// $Id: main.cpp,v 1.11 2018-01-25 14:19:29-08 - - $

#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>
#include <unistd.h>
#include <fstream>
#include <regex>

using namespace std;

#include "listmap.h"
#include "xpair.h"
#include "util.h"

using str_str_map = listmap<string,string>;
using str_str_pair = str_str_map::value_type;

void scan_options (int argc, char** argv) {
   opterr = 0;
   for (;;) {
      int option = getopt (argc, argv, "@:");
      if (option == EOF) break;
      switch (option) {
         case '@':
            debugflags::setflags (optarg);
            break;
         default:
            complain() << "-" << char (optopt) << ": invalid option"
                       << endl;
            break;
      }
   }
}

void equalsVal(string val, str_str_map &strMap) {

    for (str_str_map::iterator itor = strMap.begin();
         itor != strMap.end(); ++itor) {
        auto p = *itor;
        if (p.second == val) {
            cout << p.first << " = " << p.second << endl;
        }
    }
}

void erase(string key, str_str_map &strMap) {
    for (str_str_map::iterator itor = strMap.begin();
         itor != strMap.end(); ++itor) {
        auto p = *itor;
        if (p.first == key) {
            strMap.erase(itor);
//            auto p1 = &itor;
//            delete(p1);
            return;
        }
    }
}

void printAllPairs(str_str_map &strMap) {
    for (str_str_map::iterator itor = strMap.begin();
         itor != strMap.end(); ++itor) {
        auto p = *itor;
        cout << p.first << " = " << p.second << endl;
    }
}

void updateMap(string key, string val, str_str_map &strMap) {
    for (str_str_map::iterator itor = strMap.begin();
         itor != strMap.end(); ++itor) {
        auto p = *itor;
        if (p.first == key) {
            p.second = val;
            cout << key << " = " << val << endl;
            return;
        }
    }
    cout << key << " = " << val << endl;
    str_str_pair pair(key, val);
    strMap.insert(pair);
}

void printPair(string key, str_str_map &strMap) {
    for (str_str_map::iterator itor = strMap.begin();
         itor != strMap.end(); ++itor) {
        auto p = *itor;
        if (p.first == key) {
            cout << p.first << " = " << p.second << endl;
            return;
        }
    }
    cout << key << ": key not found" << endl;
}

void process_file(istream &s, string name, str_str_map &strMap) {
    regex comment_regex{R"(^\s*(#.*)?$)"};
    regex key_value_regex{R"(^\s*(.*?)\s*=\s*(.*?)\s*$)"};
    regex trimmed_regex{R"(^\s*([^=]+?)\s*$)"};
    string line;
    int i = 1;
    while (getline(s, line)) {
        cout << name << ": " << i++ << ": " << line << endl;

        smatch result;
        if (regex_search(line, result, comment_regex)) {
            continue;
        }
        if (regex_search(line, result, key_value_regex)) {
//            cout << "key  : \"" << result[1] << "\"" << endl;
//            cout << "value: \"" << result[2] << "\"" << endl;
            if (result[1] == "" and result[2] != "") {
                //'=val' ; print all keyval pair with val
                equalsVal(result[2], strMap);

            } else if (result[1] != "" and result[2] == "") {
                //'key=' ; deletes key val pair
                erase(result[1], strMap);

            } else if (result[1] == "" and result[2] == "") {
                //'=' ; print out all key val pairs
                printAllPairs(strMap);

            } else if (result[1] != "" and result[2] != "") {
                // 'key=val' ; add key val pair to map, update if exists
                updateMap(result[1], result[2], strMap);
            }

        } else if (regex_search(line, result, trimmed_regex)) {
            //cout << "query: \"" << result[1] << "\"" << endl;
            // 'key' - print out keyval pair if found.
            printPair(result[1], strMap);
        }
    }
}

int main (int argc, char** argv) {
   sys_info::execname (argv[0]);
   scan_options (argc, argv);

   str_str_map strMap;
    if (argc == 1) {
        process_file(cin, "-", strMap);
    } else {
        for (char **argp = &argv[optind]; argp != &argv[argc]; ++argp) {
            ifstream myfile;
            myfile.open(*argp);

            if (myfile.fail()) {
                cerr << "Can't open file " << *argp << endl;
                continue;
            }

            istream &s = myfile;
            process_file(s, *argp, strMap);
            myfile.close();

        }

    }

    for (str_str_map::iterator itor = strMap.begin();
         itor != strMap.end(); ) {
         itor =  strMap.erase(itor);
    }
   return EXIT_SUCCESS;
}

