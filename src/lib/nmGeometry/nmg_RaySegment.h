#ifndef NMG_RAYSEGMENT_H
#define NMG_RAYSEGMENT_H

#include <math.h>
#include <float.h> // for DBL_EPSILON
#include "nmg_Vector.h"
#include "nmg_Point.h"

// making these inline and pure C has a huge effect on performance
inline void initPlucker(const nmg_Point_3d &start, 
                        const nmg_Point_3d &end, double *p)
{
  p[0] = start.x*end.y - end.x*start.y;
  p[1] = start.x*end.z - end.x*start.z;
  p[2] = start.x - end.x; //-dir.x;
  p[3] = start.y*end.z - end.y*start.z;
  p[4] = start.z - end.z; //-dir.z;
  p[5] = end.y - start.y; //dir.y;
}

inline void initPlucker(const nmg_Point_3f &start,
                        const nmg_Point_3f &end, double *p)
{
  p[0] = start.x*end.y - end.x*start.y;
  p[1] = start.x*end.z - end.x*start.z;
  p[2] = start.x - end.x; //-dir.x;
  p[3] = start.y*end.z - end.y*start.z;
  p[4] = start.z - end.z; //-dir.z;
  p[5] = end.y - start.y; //dir.y;
}

inline double sideFromPlucker(double *p0, double *p1, 
       double &relativeErrorLowerBound)
{
  double result, addend, maxAddend;
  addend = p0[0]*p1[4];
  maxAddend = addend;
  result = addend;
  addend = p0[1]*p1[5];
  if (addend > maxAddend) maxAddend = addend;
  result += addend;
  addend = p0[2]*p1[3];
  if (addend > maxAddend) maxAddend = addend;
  result += addend;
  addend = p0[3]*p1[2];
  if (addend > maxAddend) maxAddend = addend;
  result += addend;
  addend = p0[4]*p1[0];
  if (addend > maxAddend) maxAddend = addend;
  result += addend;
  addend = p0[5]*p1[1];
  if (addend > maxAddend) maxAddend = addend;
  result += addend;
  relativeErrorLowerBound = DBL_EPSILON*fabs(maxAddend/result);
  return result;
}

class nmg_Ray_d {
 friend class nmg_Edge_d;
 friend class nmg_Edge_f;
 friend class nmg_Ray_f;
 public:
  nmg_Ray_d() {}
  inline nmg_Ray_d(const nmg_Point_3d &start, const nmg_Vector_3d &direction);
  inline double side(nmg_Ray_d &r, double &relativeError);
  inline double side(nmg_Edge_d &r, double &relativeError);
  inline double side(nmg_Ray_f &r, double &relativeError);
  inline double side(nmg_Edge_f &r, double &relativeError);
  nmg_Point_3d &start() {return d_start;}
  nmg_Vector_3d &dir() {return d_dir;}

 private:
  nmg_Point_3d d_start;
  nmg_Vector_3d d_dir;
  double d_plucker[6];
};

nmg_Ray_d::nmg_Ray_d(const nmg_Point_3d &start, const nmg_Vector_3d &dir):
  d_start(start), d_dir(dir)/*, d_plucker(start, start+dir)*/
{
  nmg_Point_3d end = start+dir;
  initPlucker(start, end, d_plucker);
}

// inline
double nmg_Ray_d::side(nmg_Ray_d &b, double &relative_error)
{
  return sideFromPlucker(d_plucker, b.d_plucker, relative_error);
}

// A ray segment is a ray with an associated length (as for a part of a
// trajectory between consecutive scattering events

class nmg_RaySegment_d : public nmg_Ray_d {
 public:
  nmg_RaySegment_d() {}
  nmg_RaySegment_d(const nmg_Point_3d &start, const nmg_Point_3d &direction,
                const double length):
        nmg_Ray_d(start, direction), d_length(length) {}
  nmg_RaySegment_d(const nmg_Ray_d &ray, const double length):
        nmg_Ray_d(ray), d_length(length) {}
  double length() {return d_length;}

 private:
  double d_length;
};

// An edge is a line segment defined by two points (as in a surface)
class nmg_Edge_d {
 friend class nmg_Ray_d;
 friend class nmg_RaySegment_d;
 public:
  nmg_Edge_d() {}
  inline nmg_Edge_d(const nmg_Point_3d &start, const nmg_Point_3d &end);
  inline double side(nmg_Ray_d &r, double &relativeError);
  inline double side(nmg_Ray_f &r, double &relativeError);
  nmg_Point_3d &start() {return d_start;}
  nmg_Vector_3d &end() {return d_end;}

 private:
  nmg_Point_3d d_start;
  nmg_Point_3d d_end;
  double d_plucker[6];
};

// inline
nmg_Edge_d::nmg_Edge_d(const nmg_Point_3d &start, const nmg_Point_3d &end):
  d_start(start), d_end(end)/*, d_plucker(start, end)*/
{
  initPlucker(start, end, d_plucker);
}

// inline
double nmg_Edge_d::side(nmg_Ray_d &b, double &relative_error)
{
  return sideFromPlucker(d_plucker, b.d_plucker, relative_error);
}

// inline
double nmg_Ray_d::side(nmg_Edge_d &b, double &relative_error)
{
  return sideFromPlucker(d_plucker, b.d_plucker, relative_error);
}

//========================================================
// float versions

class nmg_Ray_f {
 friend class nmg_Edge_d;
 friend class nmg_Edge_f;
 friend class nmg_Ray_d;
 public:
  nmg_Ray_f() {}
  inline nmg_Ray_f(const nmg_Point_3f &start, const nmg_Vector_3f &direction);
  inline nmg_Ray_f(nmg_Ray_d &ray);
  inline float side(nmg_Ray_f &r, double &relativeError);
  inline float side(nmg_Edge_f &r, double &relativeError);
  nmg_Point_3f &start() {return d_start;}
  nmg_Vector_3f &dir() {return d_dir;}

 private:
  nmg_Point_3f d_start;
  nmg_Vector_3f d_dir;
  double d_plucker[6];
};

nmg_Ray_f::nmg_Ray_f(const nmg_Point_3f &start, const nmg_Vector_3f &dir):
  d_start(start), d_dir(dir)/*, d_plucker(start, start+dir)*/
{
  nmg_Point_3f end = start+dir;
  initPlucker(start, end, d_plucker);
}

nmg_Ray_f::nmg_Ray_f(nmg_Ray_d &ray):
  d_start(ray.start()), d_dir(ray.dir())
{
  nmg_Point_3f end = d_start+d_dir;
  initPlucker(d_start, end, d_plucker);
}

// inline
float nmg_Ray_f::side(nmg_Ray_f &b, double &relative_error)
{
  return (float)sideFromPlucker(d_plucker, b.d_plucker, relative_error);
}

// A ray segment is a ray with an associated length (as for a part of a
// trajectory between consecutive scattering events

class nmg_RaySegment_f : public nmg_Ray_f {
 public:
  nmg_RaySegment_f() {}
  nmg_RaySegment_f(const nmg_Point_3f &start, const nmg_Point_3f &direction,
                const float length):
        nmg_Ray_f(start, direction), d_length(length) {}
  nmg_RaySegment_f(const nmg_Ray_f &ray, const float length):
        nmg_Ray_f(ray), d_length(length) {}
  float length() {return d_length;}

 private:
  float d_length;
};

// An edge is a line segment defined by two points (as in a surface)
class nmg_Edge_f {
 friend class nmg_Ray_d;
 friend class nmg_Ray_f;
 friend class nmg_RaySegment_f;
 public:
  nmg_Edge_f() {}
  inline nmg_Edge_f(const nmg_Point_3f &start, const nmg_Point_3f &end);
  inline float side(nmg_Ray_f &r, double &relativeError);
  nmg_Point_3f &start() {return d_start;}
  nmg_Vector_3f &end() {return d_end;}

 private:
  nmg_Point_3f d_start;
  nmg_Point_3f d_end;
  double d_plucker[6];
};

// inline
nmg_Edge_f::nmg_Edge_f(const nmg_Point_3f &start, const nmg_Point_3f &end):
  d_start(start), d_end(end)/*, d_plucker(start, end)*/
{
  initPlucker(start, end, d_plucker);
}

// inline
float nmg_Edge_f::side(nmg_Ray_f &b, double &relative_error)
{
  return (float)sideFromPlucker(d_plucker, b.d_plucker, relative_error);
}

// inline
float nmg_Ray_f::side(nmg_Edge_f &b, double &relative_error)
{
  return (float)sideFromPlucker(d_plucker, b.d_plucker, relative_error);
}

// inline
double nmg_Ray_d::side(nmg_Ray_f &b, double &relative_error)
{
  return sideFromPlucker(d_plucker, b.d_plucker, relative_error);
}

// inline
double nmg_Edge_d::side(nmg_Ray_f &b, double &relative_error)
{
  return sideFromPlucker(d_plucker, b.d_plucker, relative_error);
}

// inline
double nmg_Ray_d::side(nmg_Edge_f &b, double &relative_error)
{
  return sideFromPlucker(d_plucker, b.d_plucker, relative_error);
}

#endif
