/*$Id$*/

/* Gokul Varadhan
 * vec3d.h
 * Sept 2000
 */

/*==========================================================================
 *
 *  Copyright (C) 1997 Warren Robinett. All Rights Reserved.
 *
 *  File:       vec2d.h
 *  Content:    header file for class Vec2d
 *
 *  
 *  
 *
 *  See file skel.doc for documentation, including bibliography
 *
 ***************************************************************************/

#ifndef _VEC3D_H_
#define _VEC3D_H_

#define true 1
#define false 0

#ifndef _WIN32
typedef int bool;
#endif
#define true 1
#define false 0


class Vec3d {
public:
	double x;
	double y;
	double z;

	Vec3d normalize();
	double magnitude();
       	Vec3d	rotate3(Vec3d axis, double angle); 
	void print();
	static double dotProd(Vec3d a, Vec3d b);
	static double angleBetween(Vec3d a, Vec3d b);
	static Vec3d crossProd(Vec3d a, Vec3d b);

	Vec3d() : x(0.), y(0.), z(0.) {}
	Vec3d( double xx, double yy, double zz ) : x(xx), y(yy), z(zz) {}
	Vec3d operator+=( Vec3d b )			{ x += b.x;     y += b.y;     z +=b.z;    return *this; }
	Vec3d operator-=( Vec3d b )			{ x -= b.x;     y -= b.y;     z -=b.z;    return *this; }
	Vec3d operator+=( double scalar )	{ x += scalar;  y += scalar;  z += scalar;  return *this; }
	Vec3d operator-=( double scalar )	{ x -= scalar;  y -= scalar;  z -= scalar;  return *this; }
	Vec3d operator*=( double scalar )	{ x *= scalar;  y *= scalar;  z *= scalar;  return *this; }
	Vec3d operator/=( double scalar )	{ x /= scalar;  y /= scalar;  z /= scalar;  return *this; }
};

// non-member functions
Vec3d	operator-( Vec3d a);

Vec3d	operator+( Vec3d a, Vec3d b);
Vec3d	operator+( Vec3d a, double s);
Vec3d	operator+( double s, Vec3d b);

Vec3d	operator-( Vec3d a, Vec3d b);
Vec3d	operator-( Vec3d a, double s);
Vec3d	operator-( double s, Vec3d b);

Vec3d	operator*( Vec3d a, double s);
Vec3d	operator/( Vec3d a, double s);

bool	operator==( Vec3d a, Vec3d b);
bool	operator!=( Vec3d a, Vec3d b);
bool	operator<(  Vec3d a, Vec3d b);
#endif
