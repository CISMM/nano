#ifndef _DRAW_H_
#define _DRAW_H_

#include "defns.h"


extern double cone_sphere_list_radius;

#if DISP_LIST
void make_sphere();
void make_cylinder();
// requires tip info
void make_cone_sphere(InvConeSphereTip ics);
#endif

void drawSphere( double diameter);
void drawCylinder( double diameter, double height);
void drawObjects( void );
void drawFrame( void );

#endif
