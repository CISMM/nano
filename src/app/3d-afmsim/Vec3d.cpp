/*$Id$*/

/* Gokul Varadhan
 * varadhan@cs.unc.edu
 * Sept 2000
 */

/*==========================================================================
 *
 *  Copyright (C) 1997 Warren Robinett. All Rights Reserved.
 *
 *  File:       vec2d.cpp
 *  Content:    Vec3d class and its routines
 *
 *  Creates and manipulates 2D vectors.  
 *  
 *
 *  See file skel.doc for documentation, including bibliography
 *
 ***************************************************************************/

#include "Vec3d.h"
#include <math.h>
#include <iostream>
#include <fstream>

// definition of Vec3d helper functions

// see p264, 267-269 Stroustrup
Vec3d	operator-( Vec3d a)	{Vec3d zero( 0., 0., 0.);  return zero -= a;}

Vec3d	operator+( Vec3d a, Vec3d b)	{Vec3d v = a;    return v += b;}
Vec3d	operator+( Vec3d a, double s)	{Vec3d v = a;    return v += s;}
Vec3d	operator+( double s, Vec3d b)	{Vec3d v = b;    return v += s;}

Vec3d	operator-( Vec3d a, Vec3d b)	{Vec3d v = a;    return v -= b;}
Vec3d	operator-( Vec3d a, double s)	{Vec3d v = a;    return v -= s;}
Vec3d	operator-( double s, Vec3d b)	{Vec3d v = - b;  return v += s;}

Vec3d	operator*( Vec3d a, double s)	{Vec3d v = a;    return v *= s;}
Vec3d	operator/( Vec3d a, double s)	{Vec3d v = a;    return v /= s;}

bool	operator==( Vec3d a, Vec3d b)	{return a.x == b.x  &&  a.y == b.y && a.z == b.z;}
bool	operator!=( Vec3d a, Vec3d b)	{return ! (a == b);}
bool	operator<(  Vec3d a, Vec3d b)	{
  if (a.x == b.x ) {
    if (a.y == b.y)
      return a.z < b.z;
    else if ( a.y <  b.y )  
      return true;
    else                   
      return false;
  }
  else if (a.x <  b.x)  
    return true;
  else                   
    return false;
}

ostream & operator<<(ostream & out, Vec3d a){
  out << "[" << a.x << ", " << a.y << ", " << a.z << "]" << endl;
  return out;
}

Vec3d Vec3d::normalize() {
  Vec3d x;
  double t;

  t = sqrt(this->x*this->x + this->y*this->y + this->z*this->z);
  if (t == 0) 
    return Vec3d(0,0,0);

  x = (*this)/t;
  return x;
}

double Vec3d :: magnitude() {
  return sqrt(this->x*this->x + this->y*this->y + this->z*this->z);
}

// rotate the vector vec about axis by angle 
Vec3d Vec3d::rotate3(Vec3d axis, double angle) {
  Vec3d ax = axis.normalize();
  Vec3d v;
  double ca,sa,ha, n1, n2, n3, R11,R12,R13,R21,R22,R23,R31,R32,R33;
  
  n1 = ax.x;
  n2 = ax.y;
  n3 = ax.z;

  ca = cos(angle);
  sa = sin(angle);
  ha = 1-ca;

  // first the rotation matrix
  R11 = n1*n1*ha+ca;
  R12 = n1*n2*ha - n3*sa;
  R13 = n1*n3*ha + n2*sa;
  R21 = n2*n1*ha + n3*sa;
  R22 = n2*n2*ha + ca;
  R23 = n2*n3*ha - n1*sa;
  R31 = n3*n1*ha - n2*sa;
  R32 = n3*n2*ha + n1*sa;
  R33 = n3*n3*ha + ca;

  v.x = R11*this->x + R12*this->y + R13*this->z;
  v.y = R21*this->x + R22*this->y + R23*this->z;
  v.z = R31*this->x + R32*this->y + R33*this->z;

  return v;
}


void Vec3d::print() {
  cout << "vec= (" << x << ", " << y << ", " << z << ")\n";
}


double Vec3d :: dotProd(Vec3d a, Vec3d b) {
  return (a.x * b.x)  +  (a.y * b.y)  +  (a.z * b.z);
}

double Vec3d :: angleBetween(Vec3d a, Vec3d b) {
  Vec3d t1 = a.normalize();
  Vec3d t2 = b.normalize();
  double theta = dotProd(t1,t2);
  if (theta < -1) 
    theta = -1;
  else if (theta > 1)
    theta = 1;

  return acos(theta);
}


Vec3d Vec3d :: crossProd(Vec3d a, Vec3d b) {
  return Vec3d (a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x);
}


#if 0
// testing Vec3d
void main(int argc, char *argv[]) {
  Vec3d u,v,z;
  
  u = Vec3d(71.5,-2.4,5.7);
  v = Vec3d(7,2,5);
  z=u+v;
  cout << z.x << ", " << z.y << ", " << z.z << "\n";
  cout << "dotprod = " << dotProd(u,v) << "\n";
}


#endif
