#define UNSURE 0


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

#ifndef VEC3D_H_GUARD
#define VEC3D_H_GUARD

typedef int bool;
#define true 1
#define false 0


class Vec3d {
public:
	double x;
	double y;
	double z;

	Vec3d() : x(0.), y(0.), z(0.) {}

	// default constructor: default = (0.,0.,0.)
	Vec3d( double xx, double yy, double zz ) : x(xx), y(yy), z(zz) {}

	Vec3d operator+=( Vec3d b )			{ x += b.x;     y += b.y;     z +=b.z;    return *this; }

	Vec3d operator-=( Vec3d b )			{ x -= b.x;     y -= b.y;     z -=b.z;    return *this; }

	Vec3d operator+=( double scalar )	{ x += scalar;  y += scalar;  z += scalar;  return *this; }

	Vec3d operator-=( double scalar )	{ x -= scalar;  y -= scalar;  z -= scalar;  return *this; }

	Vec3d operator*=( double scalar )	{ x *= scalar;  y *= scalar;  z *= scalar;  return *this; }

	Vec3d operator/=( double scalar )	{ x /= scalar;  y /= scalar;  z /= scalar;  return *this; }

#if UNSURE
       	Vec3d	rotate( double angle ); 
#endif
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

double	dotProduct( Vec3d a, Vec3d b);


#endif VEC3D_H_GUARD
