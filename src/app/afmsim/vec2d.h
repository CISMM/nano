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

#ifndef VEC2D_INCLUDED
#define VEC2D_INCLUDED


#if 0 // don't need this under C++
typedef int bool;
#define true 1
#define false 0
#endif


class Vec2d {
public:
	double x;
	double y;

	Vec2d() : x(0.), y(0.) {}						// default constructor: default = (0.,0.)
	Vec2d( double xx, double yy ) : x(xx), y(yy) {}

//	~Vec2d();										// destructor			--implicit OK
//	Vec2d operator=( Vec2d src& );					// assignment operator	--implicit OK
//	Vec2d Vec2d( Vec2d src& );						// copy constructor		--implicit OK
		// See p271 Stroustrup: "for types where the default copy constructor has the right
		// semantics, I prefer to rely on that default."
		// "I use a reference argument for the copy constructor because I must."

	// Only put functions in the class that modify the data members (eg +=).
	// Other functions that just produce new data values are defined outside the class.
	// See p267 Stroustrup.  
	Vec2d operator+=( Vec2d b )			{ x += b.x;     y += b.y;     return *this; }
	Vec2d operator-=( Vec2d b )			{ x -= b.x;     y -= b.y;     return *this; }

	Vec2d operator+=( double scalar )	{ x += scalar;  y += scalar;  return *this; }
	Vec2d operator-=( double scalar )	{ x -= scalar;  y -= scalar;  return *this; }
	Vec2d operator*=( double scalar )	{ x *= scalar;  y *= scalar;  return *this; }
	Vec2d operator/=( double scalar )	{ x /= scalar;  y /= scalar;  return *this; }

	Vec2d	rotate( double angle );
};

// non-member functions
Vec2d	operator-( Vec2d a);

Vec2d	operator+( Vec2d a, Vec2d b);
Vec2d	operator+( Vec2d a, double s);
Vec2d	operator+( double s, Vec2d b);

Vec2d	operator-( Vec2d a, Vec2d b);
Vec2d	operator-( Vec2d a, double s);
Vec2d	operator-( double s, Vec2d b);

Vec2d	operator*( Vec2d a, double s);
Vec2d	operator/( Vec2d a, double s);

bool	operator==( Vec2d a, Vec2d b);
bool	operator!=( Vec2d a, Vec2d b);
bool	operator<(  Vec2d a, Vec2d b);

double	dotProduct( Vec2d a, Vec2d b);


#endif VEC2D_INCLUDED
