/* Gokul Varadhan
 * robot.h
 * Sept 2000
 */



//============================================================================
// robot.h
//============================================================================

#ifndef ROBOT_H_GUARD
#define ROBOT_H_GUARD


#include "vec3d.h"


/**************************************************************************************/
class OB {
public:
	int   type;
	// Vec3d instead of Vec2d for 3D - Gokul
	Vec3d pos;		

	// these are in radians
	double yaw;	// for in-XY-plane rotations
	double roll;		// for tube rotating around its axis
	double pitch; 

	double leng;		// length of the tube
	double diam;		// diameter of the tube
	int   nextSeg;	// link to next segment of bendable tube (can be null)
	int   prevSeg;	// link to previous segment of bendable tube  (can be null)
	int   moved;	// 1=already moved this sim step; 0=free to be moved.

	OB( void );
	OB( Vec3d _pos, double _angle, double _roll, double _pitch, double _length, double _diameter );
	void set( int _type, Vec3d _pos, double _angle, double _roll, double _pitch, double _length, double _diameter, int _nextSeg, int _prevSeg );
        void print();
};


#define NULLOB (-1)

// object types
#define UNUSED 0
//#define TIP 1
#define TUBE 2


/**************************************************************************************/
// FUNCTIONS
void	error( char* errMsg );

#endif // ROBOT_H_GUARD
