#ifndef WARRENCNTSIM_H
#define WARRENCNTSIM_H

/**************************************************************************************/
/**************************************************************************************/
// Carbon nanotube (CNT) Simulator Program.  
// Warren Robinett
// Computer Science Dept.
// University of North Carolina at Chapel Hill
//     development begun Aug 99
//     version 1.0 released Nov 99.
/**************************************************************************************/
// References:
// pXXX Woo 3rd ed = OpenGL Programming Guide, 3rd Ed., Mason Woo, 1997
// pXXX Angel = Interactive Computer Graphics with OpenGL, Edward Angel, 2000

/**************************************************************************************/
/**************************************************************************************/
// INCLUDE FILES

/* #define GLUT */  // Uncomment this to use GLUT
/* Always define GLUT under Win32 */
#if defined(_WIN32) && !defined(GLUT)
#define GLUT
#endif

#include <stdlib.h>
#include <stdio.h>

#ifndef _WIN32
#include <unistd.h>
#endif

#include <math.h>
//#ifdef GLUT
#include <GL/glut.h>
//#endif

#include "vec2d.h"

// AFMSIM is #defined if compiling with the AFM simulator
// (created by Jake Kitchener et al)
#ifdef AFMSIM
#ifndef SIMULATOR_SERVER_H
#include "simulator_server.h"
#endif
#endif


/**************************************************************************************/
/**************************************************************************************/
// #DEFINES
#define PI            3.14159265358979323846
#define DEG_TO_RAD	(PI / 180.)
#define RAD_TO_DEG	(180. / PI)

static int dblBuf  = GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH;
//static int snglBuf = GLUT_SINGLE | GLUT_RGBA | GLUT_DEPTH;
//static char *winTitle = "Warren's CNT simulator v1.0";

#define True  1
#define False 0
#define MAXOB 100
#define MAX_GRID 128
#define NULLOB (-1)
// object types
#define TIP 0
#define SUBSTRATE 1
#define TUBE 2
// colors
#define WHITE	0
#define RED		1
#define GREEN	2
#define BLUE	3
#define MAGENTA	4
#define YELLOW	5
#define CYAN	6
// drawing styles
#define OB_SOLID		0
#define OB_WIREFRAME	1
#define OB_POINTS		2
#define OB_SILHOUETTE	3
#define OB_OUTLINE2D	4
#define OB_NONE			5
// grid display style
#define GRID_WIREFRAME 0
#define GRID_SOLID 1
#define GRID_HALF_SOLID 2
#define GRID_NONE 3
// image scan computation method
#define IMAGE_SCAN_APPROX 0
#define IMAGE_SCAN_EXACT  1
// simulation types
#define SIM_NONE 0
#define SIM_TRANSLATE 1
#define SIM_ROLLING 2
#define SIM_SLIDING 3
#define SIM_ROLLING_SLIDING 4


/**************************************************************************************/
/**************************************************************************************/
// TYPEDEFS
typedef int Bool;
typedef struct {
	int   type;
	Vec2d pos;		// 2D points (in the XY plane), at present
	float angle;	// for in-XY-plane rotations
	float roll;		// for tube rotating around its axis
	float leng;		// length of the tube
	float diam;		// diameter of the tube
	int   nextSeg;	// link to next segment of bendable tube (can be null)
	int   prevSeg;	// link to previous segment of bendable tube  (can be null)
	float bendAngle;// angle bent by this segment (radians); 0=no bending
	int   moved;	// 1=already moved this sim step; 0=free to be moved.
} OB;

// Data structure containing data pertaining to collision test between 
// sphere and tube.
typedef struct {
	Vec2d vP;	// input: point P
	Vec2d vA;	// input: endpoint A of line seg
	Vec2d vB;	// input: endpoint B of line seg
	Vec2d vM;	// nearest point on line AB to P
	Vec2d vS;	// pivot point
	Vec2d vC;	// midpoint of AB
	float k;	// parameter locating point M = B + (A-B)*k
	float sphereRadius;
	float tubeRadius;
	float dist;		// distance from sphere to tube
} SPHERE_AND_TUBE_DATA;

// Data structure resulting from intersection test between two line
// segments in the plane.  
// The 6 cases are:
// Case 1: line segs non-parallel, disjoint.
// Case 2: line segs non-parallel, intersecting.
// Case 3: line segs parallel, disjoint, mutual projection is line segment.
// Case 4: line segs parallel, disjoint, mutual projection is 1 or 0 points.
// Case 5: line segs parallel, intersecting, intersection = 1 point.  
// Case 6: line segs parallel, intersecting, intersection = line segment.  
typedef struct {
	Vec2d vD;	// input: endpoint D of line seg DE
	Vec2d vE;	// input: endpoint E of line seg DE
	Vec2d vF;	// input: endpoint F of line seg FG
	Vec2d vG;	// input: endpoint G of line seg FG

	float dist;	// distance between the two line segments
	int whichCase;		// which of the 6 cases this is
	int parallelFlag;	// 1=line segs parallel
	int intersectFlag;	// 1=line segs intersect
	int overlapFlag;	// 1=parallel and mutual projection is line seg
	Vec2d vN1;	// nearest point on line seg DE to line seg FG
	Vec2d vN2;	// nearest point on line seg FG to line seg DE
	Vec2d vI;	// point of intersection of line DE and line FG
	            // (Point I exists only if lines are not parallel.)
	Vec2d vI1;	// endpoint 1 of line segment which is intersection of the line segs
	Vec2d vI2;	// endpoint 2 of line segment which is intersection of the line segs
} LINE_SEGMENTS_INTERSECTION_TEST;

typedef struct {
	int numObs;
	OB ob[MAXOB];
} SIM_STATE;


/**************************************************************************************/
/**************************************************************************************/
// FUNCTION PROTOYPES
void	error( char* errMsg );
void	assert( int condition, char* msg );
int		streq( char* str1, char* str2 );
Vec2d	interpolateVecs( Vec2d v0, Vec2d v1, float fraction );
float	interpolateScalars( float s0, float s1, float fraction );
void	showStrokeString( char* str );
void	showBitmapString( char* str );
void	getState( SIM_STATE st );
void	setState( SIM_STATE* pSt );
void	showLabeledPoint( Vec2d vPoint, char* labelStr );
void	rotateRadiansAxis( float angleInRadians, 
				   float axisComponentX, float axisComponentY, float axisComponentZ );
void	setMaterialColor( GLfloat r, GLfloat g, GLfloat b );
void	setColor( int colorIndex );
void	drawCube( float halfWidth );
void	drawTorus( float innerDiameter, float outerDiameter );
void	drawSphere( float radius );
void	showPoint( Vec2d vPoint, int color, float size = 1., float zHeight = 0. );
void	drawCone( float radius, float height );
void	drawCylinder( float diameter, float height );
void	drawPolygon( void );
void	drawLine( Vec2d pt1, Vec2d pt2, int color, float z1 = 0., float z2 = 0. );
void	drawTip( float tipDiameter );
void	drawTube( float diameter, float length, int obIndex );
void	drawSubstrate( void );
void	lighting( void );
float	norm( Vec2d v );
float	distance( Vec2d pt1, Vec2d pt2 );
int		findNearestObToMouse( void );
void	addObject( int obNum, 
		   int type, Vec2d pos, float yaw, float roll, float leng, float diam,
		   int nextSeg, int prevSeg, float bendAngle );
void	newOb( void );
void	deleteOb( int n );
float	centerAngle( float angle );
float	calcAngleInPlane( Vec2d vA, Vec2d vB, Vec2d vC );
float	pivotPointEquation( float k );
Vec2d	PivotPoint( float normalizedPushPoint, Vec2d vEndA, Vec2d vEndB  );
float	nearestPointOnLineToPoint( Vec2d vA, Vec2d vB, Vec2d vP );
void	collisionDraw( SPHERE_AND_TUBE_DATA cxTest );
void	lineSegmentCollisionDraw( LINE_SEGMENTS_INTERSECTION_TEST lineSegCx );
int	betweenInclusive( float value,   float bound1, float bound2 );
Vec2d	xformDEtoYaxis( Vec2d vIn, Vec2d vD, Vec2d vE );
Vec2d	inverseXformDEtoYaxis( Vec2d vIn, Vec2d vD, Vec2d vE );
float	touchDistance( float radius1, float radius2 );
SPHERE_AND_TUBE_DATA	distSphereToCylinder( Vec2d vP, Vec2d vA, Vec2d vB, 
	                  float radiusSphere, float radiusCylinder );
LINE_SEGMENTS_INTERSECTION_TEST	distBetweenLineSegments( 
									Vec2d vD, Vec2d vE, Vec2d vF, Vec2d vG );
Vec2d	translateJustOutOfContact( SPHERE_AND_TUBE_DATA cxTest );
void	translateCollisionResponse(  Vec2d vPushPt, int tube, 
								   SPHERE_AND_TUBE_DATA cxTest  );
void	calcComponentVectorsRelativeToAxis( Vec2d vVector, Vec2d vAxis, 
								    Vec2d* pvNormal, Vec2d* pvParallel );
void	rollingCollisionResponse(  Vec2d vPushPt, int tube, 
								 SPHERE_AND_TUBE_DATA cxTest  );
void	calcTubeEndpoints( int tube, Vec2d* pvEndA, Vec2d* pvEndB );
void	slidingCollisionResponse( Vec2d vPushPt, int tube, 
								 SPHERE_AND_TUBE_DATA cxTest );
void	rollingSlidingCollisionResponse( Vec2d vPushPt, int tube, 
										SPHERE_AND_TUBE_DATA cxTest );
void	collisionResponse( Vec2d vPushPt, float radiusSphere, int pushedTube );
void	crossedTubeAxisCollisionResponse(  LINE_SEGMENTS_INTERSECTION_TEST lineSegCx,
				int pushingTube, int pushedTube  );
void	TubeTubeCollisionTestAndResponse( int pushingTube, int pushedTube );
void	positionSegmentRigid( int curSeg, int lastSeg, int directionFlag );
void	positionSegment( int curSeg, int lastSeg, int directionFlag );
void	positionSegmentedTube( int movedSegment );
void	propagateMovements( int movedTube );
void	markAllTubesMovable( void );
int	newSegmentedTube( int numSegments, float bendAngle, 
				  float segmentLength, float segmentWidth, Vec2d vEndpoint  );
void	collisionStuff( void );
void	showSubstrateLines( void );
void	showText( void );
void	showObjectLabels( void );
void	drawObjects( void );
void	drawStuff( void );
void	simStep(void);
int		moveTipToXYLoc( float x, float y );
int		getImageHeightAtXYLoc( float x, float y, float* z );
void	crossProduct( float  x1, float  y1, float  z1,
			  float  x2, float  y2, float  z2,
			  float* px, float* py, float* pz );
void	normalize( float* px, float* py, float* pz );
void	showGrid( void );
void	drawFrame( void );
void	singleSimStep(void);
void	toggleSim(void);
void	adjustOrthoProjectionToWindow( void );
void	zoom( float zoomFactor );
void	pan( float screenFractionX, float screenFractionY );
void	commonKeyboardFunc(unsigned char key, int x, int y);
void	doTopMenu(int value);
void	doSubMenu(int value);
void	initMenus(void );
void	moveGrabbedOb( void );
void	grabNearestOb( void );
void	calcMouseWorldLoc( int xMouse, int yMouse );
void	mouseFuncMain( int button, int state, int x, int y );
void	mouseMotionFuncMain( int x, int y );
void	reshapeWindowFuncMain( int newWindowWidth, int newWindowHeight );
void	timerFunc(int value);
void	initObs( void );
void	displayFuncMain( void );
void	displayFuncUI( void );
void	displayFuncView( void );
void	imageScanDepthRender( void );
void	doImageScan( void );
void	displayFuncDepth( void );
void	commonIdleFunc( void );
void	displayFuncDummy( void );
void	keyboardFuncDummy(unsigned char key, int x, int y);
void	idleFuncDummy( void );
void	mouseFuncDummy( int button, int state, int x, int y );
void	mouseMotionFuncDummy( int x, int y );
void	reshapeWindowFuncDummy( int newWindowWidth, int newWindowHeight );
int		main(int argc, char *argv[]);

#endif  // WARRENCNTSIM_H
