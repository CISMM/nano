#ifndef TUBE_GENERATOR_H
#define TUBE_GENERATOR_H

#include "GeomGenerator.h"
#include <quat.h>

class TubeGenerator : public GeometryGenerator
{
public:
    TubeGenerator();

    void SetPoints(q_vec_type *points, int num);
    void SetStride(int stride);
    void SetTesselation(int tess);

    virtual int Load(URender *node, GLuint *&Dlist_array);
    virtual int ReLoad(URender *node, GLuint *&Dlist_array);

private:
    int d_num_points;
    float d_overall_diameter;
    q_vec_type *d_tube_points;
    int d_tesselation;
    int d_stride;

    void drawSphere( double diameter);
    void drawCylinder( double diameter, double height);
    void drawTube(q_vec_type center, double diameter, double length);
};

#endif