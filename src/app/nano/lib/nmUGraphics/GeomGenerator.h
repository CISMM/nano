#ifndef GEOMETRY_GENERATOR_H
#define GEOMETRY_GENERATOR_H

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>

class URender;

class GeometryGenerator
{
public:
    GeometryGenerator();

    virtual int Load(URender *node, GLuint *&Dlist_array) = 0;
    virtual int ReLoad(URender *node, GLuint *&Dlist_array) = 0;
};

#endif
