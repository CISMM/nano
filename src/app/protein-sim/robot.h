//============================================================================
// robot.h
//============================================================================

#ifndef ROBOT_H_GUARD
#define ROBOT_H_GUARD


#include "vec2d.h"


/**************************************************************************************/
class OB {
public:
	int   type;
	Vec2d pos;		// 2D points (in the XY plane), at present
	double angle;	// for in-XY-plane rotations
	double roll;		// for tube rotating around its axis
	double leng;		// length of the tube
	double diam;		// diameter of the tube
	int   nextSeg;	// link to next segment of bendable tube (can be null)
	int   prevSeg;	// link to previous segment of bendable tube  (can be null)
	int   moved;	// 1=already moved this sim step; 0=free to be moved.

	OB( void );
	OB( Vec2d pos, double angle, double length, double diameter );
};


/**************************************************************************************/
// FUNCTIONS
void	error( char* errMsg );
void	Assert( int condition, char* msg );



#endif // ROBOT_H_GUARD