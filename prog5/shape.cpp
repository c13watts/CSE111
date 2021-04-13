// $Id: shape.cpp,v 1.2 2019-02-28 15:24:20-08 - - $

#include <typeinfo>
#include <unordered_map>
#include <GL/freeglut.h>

using namespace std;

#include "shape.h"
#include "util.h"
#include "math.h"
#include "graphics.h"


static unordered_map<void *, string> fontname{
        {GLUT_BITMAP_8_BY_13,        "Fixed-8x13"},
        {GLUT_BITMAP_9_BY_15,        "Fixed-9x15"},
        {GLUT_BITMAP_HELVETICA_10,   "Helvetica-10"},
        {GLUT_BITMAP_HELVETICA_12,   "Helvetica-12"},
        {GLUT_BITMAP_HELVETICA_18,   "Helvetica-18"},
        {GLUT_BITMAP_TIMES_ROMAN_10, "Times-Roman-10"},
        {GLUT_BITMAP_TIMES_ROMAN_24, "Times-Roman-24"},
};

static unordered_map<string, void *> fontcode{
        {"Fixed-8x13",     GLUT_BITMAP_8_BY_13},
        {"Fixed-9x15",     GLUT_BITMAP_9_BY_15},
        {"Helvetica-10",   GLUT_BITMAP_HELVETICA_10},
        {"Helvetica-12",   GLUT_BITMAP_HELVETICA_12},
        {"Helvetica-18",   GLUT_BITMAP_HELVETICA_18},
        {"Times-Roman-10", GLUT_BITMAP_TIMES_ROMAN_10},
        {"Times-Roman-24", GLUT_BITMAP_TIMES_ROMAN_24},
};

ostream &operator<<(ostream &out, const vertex &where) {
    out << "(" << where.xpos << "," << where.ypos << ")";
    return out;
}

shape::shape() {
    DEBUGF ('c', this);
}



text::text(void *glut_bitmap_font_, const string &textdata_) :
        glut_bitmap_font(glut_bitmap_font_), textdata(textdata_) {
    DEBUGF('c', this->textdata);
    DEBUGF ('c', this);
}

ellipse::ellipse(GLfloat width, GLfloat height) :
        dimension({width, height}) {
    DEBUGF('c', this->dimension);
    DEBUGF ('c', this);
}

circle::circle(GLfloat diameter) : ellipse(diameter, diameter) {
    DEBUGF ('c', this);
}


polygon::polygon(const vertex_list &vertices_) : vertices(vertices_) {
    DEBUGF ('c', this);
    float xtotal = 0;
    float ytotal = 0;
    for (auto v:vertices_) {
        xtotal += v.xpos;
        ytotal += v.ypos;
    }
    float xavg = xtotal / vertices.size();
    float yavg = ytotal / vertices.size();

    for (auto v:vertices) {
        v.xpos = v.xpos - xavg;
        v.ypos = v.ypos - yavg;
        DEBUGF('c', v);
    }

}

vertex_list rect_vertices(GLfloat width, GLfloat height) {
    vertex_list v;
    v.push_back(vertex{-width/2, -height/2});
    v.push_back(vertex{width/2, -height/2});
    v.push_back(vertex{width/2, height/2});
    v.push_back(vertex{-width/2, height/2});
    return v;
}

rectangle::rectangle(GLfloat width, GLfloat height) :
        polygon(rect_vertices(width, height)) {
    DEBUGF ('c', this << "(" << width << "," << height << ")");
}

square::square(GLfloat width) : rectangle(width, width) {
    DEBUGF ('c', this);
}

vertex_list equi_vertices(GLfloat width) {
    vertex_list v;
    DEBUGF('c',"equi_v");
    v.push_back(vertex{0, 0});
    v.push_back(vertex{width, 0});
    v.push_back(vertex{width / 2,
        GLfloat(sqrt((width * width)-(width/2)*(width/2)))});
    return v;
}

equilateral::equilateral(GLfloat width) :
        triangle(equi_vertices(width)) {
    DEBUGF('c', this->vertices);

}

triangle::triangle(const vertex_list &vtxs) : polygon(vtxs) {
    DEBUGF('c', this->vertices);
}

vertex_list diamond_v(GLfloat width, GLfloat height){
    vertex_list v;
    v.push_back(vertex{-width/2,0});
    v.push_back(vertex{0,-height/2});
    v.push_back(vertex{width/2,0});
    v.push_back(vertex{0,height/2});
    return v;
}

void polygon::draw_num(const vertex & center, int num) const {
    float xpos = center.xpos;
    float ypos = center.ypos;
    glRasterPos2f (xpos, ypos);
    glColor3ubv(rgbcolor(window::get_color()).ubvec);
    glutBitmapString (fontcode["Fixed-8x13"],
     reinterpret_cast<const unsigned char*>(to_string(num).c_str()));

}
void ellipse::draw_num(const vertex & center, int num) const {
    float xpos = center.xpos;
    float ypos = center.ypos;
    glRasterPos2f (xpos, ypos);
    glColor3ubv(rgbcolor(window::get_color()).ubvec);
    glutBitmapString (fontcode["Fixed-8x13"],
      reinterpret_cast<const unsigned char*>(to_string(num).c_str()));
}

void text::draw_num(const vertex &, int ) const {

}


diamond::diamond(const GLfloat width, const GLfloat height):
polygon(diamond_v(width,height)) {

}

void text::draw(const vertex &center, const rgbcolor &color) const {
    DEBUGF ('d', this << "(" << center << "," << color << ")");
    glColor3ubv (color.ubvec);
    float xpos = center.xpos;
    float ypos = center.ypos;
    glRasterPos2f (xpos, ypos);
    glutBitmapString (this->glut_bitmap_font,
    reinterpret_cast<const unsigned char*>(this->textdata.c_str()));
}

void text::draw_outline(const vertex &, const rgbcolor &) const {

}

void ellipse::draw(const vertex &center, const rgbcolor &color)
const {
    DEBUGF ('d', this << "(" << center << "," << color << ")");
    glBegin (GL_POLYGON);
    glColor3ubv (color.ubvec);
    const float delta = 2 * M_PI / 32;
    float width = this->dimension.xpos;
    float height = this->dimension.ypos;
    for (float theta = 0; theta < 2 * M_PI; theta += delta) {
        float xpos = width * cos (theta) + center.xpos;
        float ypos = height * sin (theta) + center.ypos;
        glVertex2f (xpos, ypos);
    }
    glEnd();
}

void ellipse::draw_outline(const vertex &center,
                            const rgbcolor &color) const {
    DEBUGF ('d', this << "(" << center << "," << color << ")");
    glBegin (GL_LINE_LOOP);
    glColor3ubv (color.ubvec);
    const float delta = 2 * M_PI / 32;
    float width = this->dimension.xpos;
    float height = this->dimension.ypos;
    for (float theta = 0; theta < 2 * M_PI; theta += delta) {
        float xpos = width * cos (theta) + center.xpos;
        float ypos = height * sin (theta) + center.ypos;
        glVertex2f (xpos, ypos);
    }
    glEnd();
}

void
polygon::draw(const vertex &center, const rgbcolor &color) const {
    DEBUGF ('d', this << "(" << center << "," << color << ")");
    glBegin (GL_POLYGON);
    glColor3ubv(color.ubvec);
    for (auto v:this->vertices) {
        auto xpos = v.xpos + center.xpos;
        auto ypos = v.ypos + center.ypos;
        glVertex2f(xpos, ypos);
    }
    glEnd();
}

void polygon::draw_outline(const vertex &center,
                            const rgbcolor &color) const {
    DEBUGF ('d', this << "(" << center << "," << color << ")");
    glBegin (GL_LINE_LOOP);
    glColor3ubv(color.ubvec);
    for (auto v:this->vertices) {
        auto xpos = v.xpos + center.xpos;
        auto ypos = v.ypos + center.ypos;
        glVertex2f(xpos, ypos);
    }
    glEnd();
}




void shape::show(ostream &out) const {
    out << this << "->" << demangle(*this) << ": ";
}

void text::show(ostream &out) const {
    shape::show(out);
    out << glut_bitmap_font << "(" << fontname[glut_bitmap_font]
        << ") \"" << textdata << "\"";
}

void ellipse::show(ostream &out) const {
    shape::show(out);
    out << "{" << dimension << "}";
}

void polygon::show(ostream &out) const {
    shape::show(out);
    out << "{" << vertices << "}";
}

ostream &operator<<(ostream &out, const shape &obj) {
    obj.show(out);
    return out;
}

