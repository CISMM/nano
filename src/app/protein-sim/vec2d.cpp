/*==========================================================================
 *
 *  Copyright (C) 1997 Warren Robinett. All Rights Reserved.
 *
 *  File:       vec2d.cpp
 *  Content:    Vec2d class and its routines
 *
 *  Creates and manipulates 2D vectors.  
 *  
 *
 *  See file skel.doc for documentation, including bibliography
 *
 ***************************************************************************/

#include "vec2d.h"
#include <math.h>

// definition of Vec2d helper functions

// see p264, 267-269 Stroustrup
Vec2d	operator-( Vec2d a)	{Vec2d zero( 0., 0.);  return zero -= a;}

Vec2d	operator+( Vec2d a, Vec2d b)	{Vec2d v = a;    return v += b;}
Vec2d	operator+( Vec2d a, double s)	{Vec2d v = a;    return v += s;}
Vec2d	operator+( double s, Vec2d b)	{Vec2d v = b;    return v += s;}

Vec2d	operator-( Vec2d a, Vec2d b)	{Vec2d v = a;    return v -= b;}
Vec2d	operator-( Vec2d a, double s)	{Vec2d v = a;    return v -= s;}
Vec2d	operator-( double s, Vec2d b)	{Vec2d v = - b;  return v += s;}

Vec2d	operator*( Vec2d a, double s)	{Vec2d v = a;    return v *= s;}
Vec2d	operator/( Vec2d a, double s)	{Vec2d v = a;    return v /= s;}

bool	operator==( Vec2d a, Vec2d b)	{return a.x == b.x  &&  a.y == b.y;}
bool	operator!=( Vec2d a, Vec2d b)	{return ! (a == b);}
bool	operator<(  Vec2d a, Vec2d b)	{
	if(      a.x == b.x )  return a.y < b.y;
	else if( a.x <  b.x )  return true;
	else                   return false;
}
	// Why define < for 2D points, you ask?  Because vector<Vec2d> won't work without it.
	// operator< required to use type as element of container.
	// See p466-469 Stroustrup -- element requirements for containers.

double	dotProduct( Vec2d a, Vec2d b)	{return (a.x * b.x)  +  (a.y * b.y);}

Vec2d	Vec2d::rotate( double angle ) {
	double xIn = x;
	double yIn = y;

	x =   xIn * cos( angle ) - yIn * sin( angle );
	y =   xIn * sin( angle ) + yIn * cos( angle );

	return *this;
}


