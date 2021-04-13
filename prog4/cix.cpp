// $Id: cix.cpp,v 1.9 2019-04-05 15:04:28-07 - - $

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <regex>
#include <fstream>
#include <cstdint>
#include <cstring>

using namespace std;

#include <libgen.h>
#include <sys/types.h>
#include <unistd.h>

#include "protocol.h"
#include "logstream.h"
#include "sockets.h"

logstream outlog(cout);
struct cix_exit : public exception {
};

unordered_map <string, cix_command> command_map{
        {"exit", cix_command::EXIT},
        {"help", cix_command::HELP},
        {"ls",   cix_command::LS},
        {"get",  cix_command::GET},
        {"put",  cix_command::PUT},
        {"rm",   cix_command::RM}
};

static const char help[] = R"||(
exit         - Exit the program.  Equivalent to EOF.
get filename - Copy remote file to local host.
help         - Print help summary.
ls           - List names of files on remote server.
put filename - Copy local file to remote host.
rm filename  - Remove file from remote server.
)||";

void cix_help() {
    cout << help;
}

void cix_ls(client_socket &server) {
    cix_header header;
    header.command = cix_command::LS;
    outlog << "sending header " << header << endl;
    send_packet(server, &header, sizeof header);
    recv_packet(server, &header, sizeof header);
    outlog << "received header " << header << endl;
    if (header.command != cix_command::LSOUT) {
        outlog << "sent LS, server did not return LSOUT" << endl;
        outlog << "server returned " << header << endl;
    } else {
        auto buffer = make_unique<char[]>(header.nbytes + 1);
        recv_packet(server, buffer.get(), header.nbytes);
        outlog << "received " << header.nbytes << " bytes" << endl;
        buffer[header.nbytes] = '\0';
        cout << buffer.get();
    }
}

void cix_get(client_socket &server, const string &filename){
 outlog << "cix_get " << filename << endl;
    cix_header header;
    header.command = cix_command::GET;
    memset (header.filename, 0, FILENAME_SIZE);
    strncpy(header.filename, filename.c_str(), filename.length());
    outlog << "sending header " << header << endl;
    send_packet(server, &header, sizeof header);
    outlog << "sent " << header.nbytes << " bytes" << endl;

    recv_packet(server,&header,sizeof header);
    outlog << "received header " << header << endl;

    if(header.command == cix_command::NAK){
        cout << "Error getting file " << filename << endl;
        cout << "Error: " << header.nbytes << " " <<
             strerror(header.nbytes) << endl;
        return;
    } else if(header.command == cix_command::FILEOUT) {
        char *buffer = new char[header.nbytes];
        cout << "Get file " << header.filename << " OK" << endl;
        recv_packet(server, buffer, header.nbytes);
        outlog << "received " << header.nbytes << " bytes" << endl;

        ofstream myFile;
        myFile.open(header.filename, ios::out | ios::binary);

        if ((myFile.rdstate() & std::ofstream::failbit) != 0) {
            outlog << "Error opening " << header.filename << endl;
            cout << "Error opening " << header.filename << endl;
            delete[] buffer;
            return;
        }

        myFile.write(buffer, header.nbytes);
        if (myFile.bad()) {
            outlog << "Error writing " << header.filename << endl;
            cout << "Error writing " << header.filename << endl;
            delete[] buffer;
            return;
        }

        myFile.close();
        delete[] buffer;
    }
}

void cix_rm(client_socket &server, const string &filename){
  outlog << "cix_rm " << filename << endl;
    cix_header header;
    header.command = cix_command::RM;
    memset (header.filename, 0, FILENAME_SIZE);
    strncpy(header.filename, filename.c_str(), filename.length());

    outlog << "sending header " << header << endl;
    send_packet(server, &header, sizeof header);

    recv_packet(server,&header,sizeof header);
    outlog << "received header " << header << endl;

    if (header.command == cix_command::NAK){
        cout << "Error removing file " << header.filename << endl;
        cout << "Error: " << header.nbytes << " " <<
             strerror(header.nbytes) << endl;
    } else{
        cout << "Removing file " << header.filename
        << " OK" << endl;
    }
}

void cix_put(client_socket &server, const string &filename){
    outlog << "cix_put " << filename << endl;
    int length;
    char * buffer;

    ifstream is;
    is.open (filename, ios::binary );
    if ( (is.rdstate() & std::ifstream::failbit ) != 0 ) {
        outlog << "Error opening " << filename << endl;
        return;
    }
    is.seekg (0, ios::end);
    length = is.tellg();
    outlog << filename << " length: " << length << endl;
    is.seekg (0, ios::beg);
    buffer = new char [length];
    is.read (buffer,length);
    is.close();

    cix_header header;
    header.command = cix_command::PUT;
    header.nbytes = length;
    memset (header.filename, 0, FILENAME_SIZE);
    strncpy(header.filename, filename.c_str(), filename.length());
    outlog << "sending header " << header << endl;
    send_packet (server, &header, sizeof header);
    send_packet (server, buffer, length);
    outlog << "sent " << length << " bytes" << endl;

    recv_packet(server, &header, sizeof header);
    outlog << "received header " << header << endl;
    outlog << "received " << header.nbytes << " bytes" << endl;

    if (header.command == cix_command::NAK){
        cout << "PUT for file " << header.filename<< " failed"
        << endl;
        cout << "Error: " << header.nbytes << " " <<
        strerror(header.nbytes) << endl;

    } else if(header.command == cix_command::ACK) {
        cout << "PUT for file " << header.filename<< " OK" << endl;
    }

    delete[] buffer;
}

void usage() {
    cerr << "Usage: " << outlog.execname() << " [host] [port]"
         << endl;
    throw cix_exit();
}

int main(int argc, char **argv) {
    regex command{R"(^\s*[a-zA-Z]+\s+[^\/\s]+\s*$)"};
    regex cmd_regex{R"(^\s*([a-z]+?)(\s+([^\/\s]{1,58}?))?\s*$)"};
    outlog.execname(basename(argv[0]));
    outlog << "starting" << endl;
    vector <string> args(&argv[1], &argv[argc]);
    if (args.size() > 2) usage();
    string host = get_cix_server_host(args, 0);
    in_port_t port = get_cix_server_port(args, 1);
    outlog << to_string(hostinfo()) << endl;
    try {
        outlog << "connecting to " << host << " port " << port
               << endl;
        client_socket server(host, port);
        outlog << "connected to " << to_string(server) << endl;
        for (;;) {
            string line;
            getline(cin, line);
            if (cin.eof()) throw cix_exit();

            smatch result;
            auto match = regex_search(line, result, cmd_regex);

            if (!match) {
                if (line == ""){
                    continue;
                }
                cout << line << ": invalid command or filename"
                       << endl;
                continue;
            }

            outlog << "command " << line << endl;
            const auto &itor = command_map.find(result[1]);
            cix_command cmd = itor == command_map.end()
                              ? cix_command::ERROR : itor->second;

            switch (cmd) {
                case cix_command::EXIT:
                    throw cix_exit();
                    break;
                case cix_command::HELP:
                    cix_help();
                    break;
                case cix_command::LS:
                    cix_ls(server);
                    break;
                case cix_command::GET:
                    cix_get(server, result[3]);
                    break;
                case cix_command::RM:
                    cix_rm(server, result[3]);
                    break;
                case cix_command::PUT:
                    cix_put(server, result[3]);
                    break;
                default:
                    outlog << line << ": invalid command" << endl;
                    break;
            }
        }
    } catch (socket_error &error) {
        outlog << error.what() << endl;
    } catch (cix_exit &error) {
        outlog << "caught cix_exit" << endl;
    }
    outlog << "finishing" << endl;
    return 0;
}

