#ifndef NMG_PLANE_H
#define NMG_PLANE_H

#include "nmg_Vector.h"
#include "nmg_Point.h"
#include "nmg_RaySegment.h"
#include "nmg_Types.h"

class nmg_Plane_d {
 public:
  nmg_Plane_d() {}
  // normal*point - d = 0 ==> point is in the plane
  // normal*point - d > 0 ==> point is above plane
  // normal*point - d < 0 ==> point is below plane
  nmg_Plane_d(const nmg_Vector_3d &normal, double d):d_normal(normal), d_d(d) {}
  inline bool intersectsRaySegment(nmg_RaySegment_d &ray, double &t);
  inline bool intersectsRay(nmg_Ray_d &ray, double &t);
  inline bool intersectsRaySegment(nmg_RaySegment_f &ray, double &t);
  inline bool intersectsRay(nmg_Ray_f &ray, double &t);
  inline double z(double x, double y);
  inline double height(nmg_Point_3d &point);
  inline double height(nmg_Point_3f &point);
  nmg_Vector_3d normal() {return d_normal;}
  void setNormal(nmg_Vector_3d normal) {d_normal = normal;}
  double d() {return d_d;}
  void setD(double d) {d_d = d;}

 protected:
  nmg_Vector_3d d_normal;
  double d_d;
};

bool nmg_Plane_d::intersectsRay(nmg_Ray_d &ray, double &t)
{
  bool result = true;
  double num, den;
  num = d_d - d_normal*ray.start();
  den = d_normal*ray.dir();
  if (den == 0) {
    result = false;
  } else {
    double t_temp = num/den;
    if (t_temp < 0) {
      result = false;
    } else {
      t = t_temp;
      result = true;
    }
  }
  return result;
}

bool nmg_Plane_d::intersectsRay(nmg_Ray_f &ray, double &t)
{
  bool result = true;
  double num, den;
  num = d_d - d_normal*ray.start();
  den = d_normal*ray.dir();
  if (den == 0) {
    result = false;
  } else {
    double t_temp = num/den;
    if (t_temp < 0) {
      result = false;
    } else {
      t = t_temp;
      result = true;
    }
  }
  return result;
}

bool nmg_Plane_d::intersectsRaySegment(nmg_RaySegment_d &ray, double &t)
{
  bool result = true;
  double num, den;
  num = d_d - d_normal*ray.start();
  den = d_normal*ray.dir();
  if (den == 0) {
    result = false;
  } else {
    double t_temp = num/den;
    if (t_temp < 0 || t_temp > ray.length()) {
      result = false;
    } else {
      t = t_temp;
      result = true;
    }
  }
  return result;
}

bool nmg_Plane_d::intersectsRaySegment(nmg_RaySegment_f &ray, double &t)
{
  bool result = true;
  double num, den;
  num = d_d - d_normal*ray.start();
  den = d_normal*ray.dir();
  if (den == 0) {
    result = false;
  } else {
    double t_temp = num/den;
    if (t_temp < 0 || t_temp > ray.length()) {
      result = false;
    } else {
      t = t_temp;
      result = true;
    }
  }
  return result;
}

double nmg_Plane_d::z(double x, double y)
{
  if (d_normal.z != 0.0) {
    return (-d_normal.x*x - d_normal.y*y + d_d)/d_normal.z;
  } else {
    return 0.0;
  }
}

double nmg_Plane_d::height(nmg_Point_3d &point)
{
  double h = d_normal*point - d_d;
  return h;
}

double nmg_Plane_d::height(nmg_Point_3f &point)
{
  double h = d_normal*point - d_d;
  return h;
}

//=========================================================

class nmg_Plane {
 public:
  nmg_Plane() {}
  // normal*point - d = 0 ==> point is in the plane
  // normal*point - d > 0 ==> point is above plane
  // normal*point - d < 0 ==> point is below plane
  nmg_Plane(const nmg_Vector &normal, nmg_Float d):d_normal(normal), d_d(d) {}
  inline bool intersectsRaySegment(nmg_RaySegment &ray, nmg_Float &t);
  inline bool intersectsRay(nmg_Ray &ray, nmg_Float &t);
  inline nmg_Float z(nmg_Float x, nmg_Float y);
  inline nmg_Float height(nmg_Point &point);
  nmg_Vector normal() {return d_normal;}
  void setNormal(nmg_Vector normal) {d_normal = normal;}
  nmg_Float d() {return d_d;}
  void setD(nmg_Float d) {d_d = d;}

 protected:
  nmg_Vector d_normal;
  nmg_Float d_d;
};

bool nmg_Plane::intersectsRay(nmg_Ray &ray, nmg_Float &t)
{
  bool result = true;
  double num, den;
  num = d_d - d_normal*ray.start();
  den = d_normal*ray.dir();
  if (den == 0) {
    result = false;
  } else {
    double t_temp = num/den;
    if (t_temp < 0) {
      result = false;
    } else {
      t = (float)t_temp;
      result = true;
    }
  }
  return result;
}

bool nmg_Plane::intersectsRaySegment(nmg_RaySegment &ray, nmg_Float &t)
{
  bool result = true;
  double num, den;
  num = d_d - d_normal*ray.start();
  den = d_normal*ray.dir();
  if (den == 0) {
    result = false;
  } else {
    double t_temp = num/den;
    if (t_temp < 0 || t_temp > ray.length()) {
      result = false;
    } else {
      t = (float)t_temp;
      result = true;
    }
  }
  return result;
}

nmg_Float nmg_Plane::z(nmg_Float x, nmg_Float y) 
{
  if (d_normal.z != 0.0) {
    return (-d_normal.x*x - d_normal.y*y + d_d)/d_normal.z;
  } else {
    return 0.0;
  }
}

nmg_Float nmg_Plane::height(nmg_Point &point) 
{
  double h = d_normal*point - d_d;
  return (float)h;
}

#endif
