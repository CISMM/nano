#define UNSURE 0


/* Gokul Varadhan
 * vec3d.h
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

#include "vec3d.h"
#include <math.h>
#include <iostream.h>

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

double	dotProduct( Vec3d a, Vec3d b)	{return (a.x * b.x)  +  (a.y * b.y)  +  (a.z * b.z);}

#if UNSURE
Vec3d	Vec3d::rotate( double angle ) {
	double xIn = x;
	double yIn = y;

	x =   xIn * cos( angle ) - yIn * sin( angle );
	y =   xIn * sin( angle ) + yIn * cos( angle );

	return *this;
}
#endif

#if 0
// testing Vec3d
void main(int argc, char *argv[]) {
  Vec3d u,v,z;
  
  u = Vec3d(71.5,-2.4,5.7);
  v = Vec3d(7,2,5);
  z=u+v;
  cout << z.x << ", " << z.y << ", " << z.z << "\n";
  cout << "dotprod = " << dotProduct(u,v) << "\n";
}


#endif
