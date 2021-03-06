/* The nanoManipulator and its source code have been released under the
 * Boost software license when nanoManipulator, Inc. ceased operations on
 * January 1, 2014.  At this point, the message below from 3rdTech (who
 * sublicensed from nanoManipulator, Inc.) was superceded.
 * Since that time, the code can be used according to the following
 * license.  Support for this system is now through the NIH/NIBIB
 * National Research Resource at cismm.org.

Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

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

#include <iostream>
using namespace std;

#ifndef _VEC3D_H_
#define _VEC3D_H_

#define true 1
#define false 0

#ifndef _WIN32
typedef int bool;
#define true 1
#define false 0
#endif


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
ostream & operator<<(ostream & out, Vec3d a);
istream & operator>>(istream & in, Vec3d a);

#endif
