#include "nmg_Triangle.h"
#include <stdlib.h>

// errorThreshold:
// This represents the amount of precision loss we will tolerate
// in computing ray-triangle intersections. A smaller number indicates lower
// tolerance of precision loss. If the estimated lower
// bound of the relative error goes above this number then we will 
// treat the situation as if there were catastropic cancellation which
// we expect to occur when a ray comes very very close to an edge 
// (or two edges if near a vertex). In this case we assume that the ray
// hits exactly any edge for which the error threshold is exceeded. This
// effectively makes edges look a little fat but since we are using
// double precision in the nmg_Plucker class I expect the effect is
// insignificant. If the threshold is set to 0.1 a ray may still fall through
// the cracks because we are dealing with only a lower bound on 
// relative error due to a one-time precision loss. Accumulated precision
// loss could make the actual relative error significantly higher. Putting
// the error threshold a bit lower causes us to classify more calculations
// as having insufficient precision and eliminates those rays that would
// fall through the cracks due to incorrect assumption of sufficient precision.

double nmg_Triangle::s_errorThreshold = 0.001;

nmg_Triangle::nmg_Triangle(): d_A(NULL), d_B(NULL), d_C(NULL)
{
  d_normal = nmg_Vector_3d(0.0, 0.0, 1.0);
  d_plane = nmg_Plane_d(d_normal, 0.0);
}

nmg_Triangle::nmg_Triangle(nmg_Point_3d *a, nmg_Point_3d *b, nmg_Point_3d *c):
d_A(a), d_B(b), d_C(c),
d_edge0(*a, *b), d_edge1(*b, *c), d_edge2(*c, *a)
{
  d_normal = (*c-*b)/(*a-*b);
  if (d_normal.x == 0 && d_normal.y == 0 && d_normal.z == 0) {
    printf("nmg_Triangle::nmg_Triangle: Error, bad triangle created\n");
    exit(-1);
  }
  // area = 0.5*d_normal.Length();
  d_normal.Normalize();
  double d = d_normal*(*d_A);
  d_plane = nmg_Plane_d(d_normal, d);
}
