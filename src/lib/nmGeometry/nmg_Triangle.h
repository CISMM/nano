#ifndef NMG_TRIANGLE_H
#define NMG_TRIANGLE_H

#include "nmg_Point.h"
#include "nmg_Plane.h"
#include "nmg_RaySegment.h"

class nmg_Triangle {
 public:
  nmg_Triangle();
  nmg_Triangle(nmg_Point_3d *a, nmg_Point_3d *b, nmg_Point_3d *c);
  nmg_Vector_3d *normal() { return &d_normal; }
  nmg_Point_3d *A() {return d_A;}
  nmg_Point_3d *B() {return d_B;}
  nmg_Point_3d *C() {return d_C;}

  nmg_Edge_d &edge0() {return d_edge0;}
  nmg_Edge_d &edge1() {return d_edge1;}
  nmg_Edge_d &edge2() {return d_edge2;}
  nmg_Plane_d &plane() {return d_plane;}
  inline bool intersectsRaySegment(nmg_RaySegment &ray, bool startingInside,
                                   double &tAtHit, 
                                   bool &hit0, bool &hit1, bool &hit2);

  inline bool intersectsRay(nmg_Ray &ray, bool startingInside,
                            double &tAtHit,
                            bool &hit0, bool &hit1, bool &hit2);

 protected:
  nmg_Point_3d *d_A, *d_B, *d_C;
  nmg_Edge_d d_edge0, d_edge1, d_edge2;
  nmg_Vector_3d d_normal;
  nmg_Plane_d d_plane;
  static double s_errorThreshold;
};

bool nmg_Triangle::intersectsRaySegment(nmg_RaySegment &ray, 
                          bool startingInside,
                          double &tAtHit, 
                          bool &hit0, bool &hit1, bool &hit2)
{
  double dotProd = ray.dir()*d_normal;
  if ((dotProd == 0) ||
      (startingInside && dotProd < 0) ||
      (!startingInside && dotProd > 0)) {
    return false;
  }

  double err0, err1, err2;
  double side0 = d_edge0.side(ray, err0);
  double side1 = d_edge1.side(ray, err1);
  double side2 = d_edge2.side(ray, err2);

  if (err0 > s_errorThreshold) {
    side0 = 0.0;
  }
  if (err1 > s_errorThreshold) {
    side1 = 0.0;
  }
  if (err2 > s_errorThreshold) {
    side2 = 0.0;
  }

  if (side0 == 0) {
    hit0 = true;
  } else {
    hit0 = false;
  }
  if (side1 == 0) {
    hit1 = true;
  } else {
    hit1 = false;
  }
  if (side2 == 0) {
    hit2 = true;
  } else {
    hit2 = false;
  }

  if (startingInside) { 
    if (side0 > 0 || side1 > 0 || side2 > 0) {
      return false;
    }
  } else {
    if (side0 < 0 || side1 < 0 || side2 < 0) {
      return false;
    }
  }
    
  double t;
  if (!d_plane.intersectsRaySegment(ray, t) ||
      t >= ray.length()) {
    return false;
  }
  tAtHit = t;
  return true;
}

bool nmg_Triangle::intersectsRay(nmg_Ray &ray,
                          bool startingInside,
                          double &tAtHit,
                          bool &hit0, bool &hit1, bool &hit2)
{
  double dotProd = ray.dir()*d_normal;
  if ((dotProd == 0) ||
      (startingInside && dotProd < 0) ||
      (!startingInside && dotProd > 0)) {
    return false;
  }

  double err0, err1, err2;
  double side0 = d_edge0.side(ray, err0);
  double side1 = d_edge1.side(ray, err1);
  double side2 = d_edge2.side(ray, err2);

  if (err0 > s_errorThreshold) {
    side0 = 0.0;
  }
  if (err1 > s_errorThreshold) {
    side1 = 0.0;
  }
  if (err2 > s_errorThreshold) {
    side2 = 0.0;
  }

  if (side0 == 0) {
    hit0 = true;
  } else {
    hit0 = false;
  }
  if (side1 == 0) {
    hit1 = true;
  } else {
    hit1 = false;
  }
  if (side2 == 0) {
    hit2 = true;
  } else {
    hit2 = false;
  }

  if (startingInside) {
    if (side0 > 0 || side1 > 0 || side2 > 0) {
      return false;
    }
  } else {
    if (side0 < 0 || side1 < 0 || side2 < 0) {
      return false;
    }
  }

  double t;
  if (!d_plane.intersectsRay(ray, t)) {
    return false;
  }
  tAtHit = t;
  return true;
}

#endif
