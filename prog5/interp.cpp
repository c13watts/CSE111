// $Id: interp.cpp,v 1.3 2019-03-19 16:18:22-07 - - $

#include <memory>
#include <string>
#include <vector>
using namespace std;

#include <GL/freeglut.h>

#include "debug.h"
#include "interp.h"
#include "shape.h"
#include "util.h"

unordered_map<string, interpreter::interpreterfn>
        interpreter::interp_map{
        {"define", &interpreter::do_define},
        {"draw",   &interpreter::do_draw},
        {"border", &interpreter::border},
        {"moveby", &interpreter::move_by},
};


static unordered_map<string, void *> fontcde{
        {"Fixed-8x13",     GLUT_BITMAP_8_BY_13},
        {"Fixed-9x15",     GLUT_BITMAP_9_BY_15},
        {"Helvetica-10",   GLUT_BITMAP_HELVETICA_10},
        {"Helvetica-12",   GLUT_BITMAP_HELVETICA_12},
        {"Helvetica-18",   GLUT_BITMAP_HELVETICA_18},
        {"Times-Roman-10", GLUT_BITMAP_TIMES_ROMAN_10},
        {"Times-Roman-24", GLUT_BITMAP_TIMES_ROMAN_24},
};

unordered_map<string, interpreter::factoryfn>
        interpreter::factory_map{
        {"text",        &interpreter::make_text},
        {"ellipse",     &interpreter::make_ellipse},
        {"circle",      &interpreter::make_circle},
        {"polygon",     &interpreter::make_polygon},
        {"rectangle",   &interpreter::make_rectangle},
        {"square",      &interpreter::make_square},
        {"triangle",    &interpreter::make_triangle},
        {"equilateral", &interpreter::make_equilateral},
        {"diamond",     &interpreter::make_diamond},
};

interpreter::shape_map interpreter::objmap;

interpreter::~interpreter() {
    for (const auto &itor: objmap) {
        cout << "objmap[" << itor.first << "] = "
             << *itor.second << endl;
    }
}

void interpreter::interpret(const parameters &params) {
    DEBUGF ('i', params);
    param begin = params.cbegin();
    string command = *begin;
    auto itor = interp_map.find(command);
    if (itor == interp_map.end()) throw runtime_error("syntax error");
    interpreterfn func = itor->second;
    func(++begin, params.cend());
}

void interpreter::do_define(param begin, param end) {
    DEBUGF ('f', range(begin, end));
    string name = *begin;
    objmap.emplace(name, make_shape(++begin, end));
}

void interpreter::move_by(interpreter::param begin,
                          interpreter::param) {
    window::set_moveby(stof(*begin));
    DEBUGF('f', "move_amt" + *begin);
}

void interpreter::border(interpreter::param begin,
                         interpreter::param) {
    string s;
    s = *begin;
    window::set_color(s);
    DEBUGF('f', "color" + *begin);
    ++begin;
    window::set_border(stof(*begin));
    DEBUGF('f', "line_width" + *begin);
}


void interpreter::do_draw(param begin, param end) {
    DEBUGF ('f', range(begin, end));
    if (end - begin != 4) throw runtime_error("syntax error");
    string name = begin[1];
    shape_map::const_iterator itor = objmap.find(name);
    if (itor == objmap.end()) {
        throw runtime_error(name + ": no such shape");
    }
    try {
        rgbcolor color{begin[0]};
        try {
            vertex where{from_string<GLfloat>(begin[2]),
                         from_string<GLfloat>(begin[3])};
            window::push_back(object(itor->second, where, color));
        } catch (exception &e){
            throw runtime_error("Error with vertex param: "
            + begin[2] + " or " + begin[3]);
        }

    } catch (exception &e){
        throw runtime_error("Color " + begin[0] + " not found" );
    }
}

shape_ptr interpreter::make_shape(param begin, param end) {
    DEBUGF ('f', range(begin, end));
    string type = *begin++;
    auto itor = factory_map.find(type);
    if (itor == factory_map.end()) {
        throw runtime_error(type + ": no such shape");
    }
    factoryfn func = itor->second;
    return func(begin, end);
}

shape_ptr interpreter::make_text(param begin, param end) {
    DEBUGF ('f', range(begin, end));
    auto font = fontcde.find(*begin);
    if (font == fontcde.end()) {
        throw runtime_error("Font " + *begin + " not found");
    }
    string s;
    auto bgin = ++begin;
    for (auto it = bgin; it != end; ++it) {
        if (it != begin) {
            s += " ";
        }
        s += *it;
    }

    return make_shared<text>(font->second, s);
}

shape_ptr interpreter::make_ellipse(param begin, param end) {
    DEBUGF ('f', range(begin, end));
    auto f1 = stof(*begin);
    ++begin;
    auto f2 = stof(*begin);
    return make_shared<ellipse>(GLfloat(f1), GLfloat(f2));
}

shape_ptr interpreter::make_circle(param begin, param end) {
    DEBUGF ('f', range(begin, end));
    return make_shared<circle>(GLfloat(stof(*begin)));
}

shape_ptr interpreter::make_polygon(param begin, param end) {
    DEBUGF ('f', range(begin, end));
    vertex_list v;
    for (auto it = begin; it != end; ++it) {
        auto f1 = stof(*it);
        ++it;
        auto f2 = stof(*it);
        auto vtx = vertex{GLfloat(f1), GLfloat(f2)};
        v.push_back(vtx);
    }
    return make_shared<polygon>(v);
}

shape_ptr interpreter::make_rectangle(param begin, param end) {
    DEBUGF ('f', range(begin, end));
    auto w = stof(*begin);
    ++begin;
    auto h = stof(*begin);
    return make_shared<rectangle>(GLfloat(w), GLfloat(h));
}

shape_ptr interpreter::make_square(param begin, param end) {
    DEBUGF ('f', range(begin, end));
    return make_shared<square>(GLfloat(stof(*begin)));
}

shape_ptr interpreter::make_equilateral(interpreter::param begin,
                                        interpreter::param) {
    DEBUGF('f', *begin);

    return make_shared<equilateral>(GLfloat(stof(*begin)));
}

shape_ptr interpreter::make_diamond(interpreter::param begin,
                                    interpreter::param) {
    auto w = stof(*begin);
    ++begin;
    auto h = stof(*begin);
    return make_shared<diamond>(GLfloat(w), GLfloat(h));
}

shape_ptr interpreter::make_triangle(interpreter::param begin,
                                     interpreter::param end) {
    if ((end - begin) != 6) {
        throw runtime_error("Invalid number of vertices");
    }
    vertex_list v;
    for (auto it = begin; it != end; ++it) {
        auto f1 = stof(*it);
        ++it;
        auto f2 = stof(*it);
        auto vtx = vertex{GLfloat(f1), GLfloat(f2)};
        v.push_back(vtx);
    }

    return make_shared<triangle>(v);   
}

