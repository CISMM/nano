/**************************************************************************************/
/**************************************************************************************/
// Carbon nanotube (CNT) Simulator Program.  
// Warren Robinett
// Computer Science Dept.
// University of North Carolina at Chapel Hill
//     development begun Aug 99
//     version 1.0 released Nov 99.
//     version 2.0 released Jun 00
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

#include <stdlib.h>		//stdlib.h vs cstdlib
#include <stdio.h>		//stdio.h vs cstdio

#ifdef _WIN32
  #include <iostream>
  using namespace std;
#else
  #include <iostream.h>
#endif

//#include <string>
#include <vector>


#ifndef _WIN32
  #include <unistd.h>		//unistd.h vs cunistd
#endif

#include <math.h>		//math.h vs cmath

#include <GL/glut.h>

#include "vec2d.h"

// AFMSIM is #defined if compiling with the AFM simulator
// (created by Jake Kitchener et al)
#ifdef AFMSIM
#ifndef SIMULATOR_SERVER_H
#include "simulator_server.h"
#endif
#endif

// Image Analysis includes
#include "ppm.h"
#include "cnt_ia.h"
#include "cnt_glue.h"
#include "robot.h"

  

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
#define MAX_GRID 64   // was 128
#define MAX_PATHQ 100
#define NULLOB (-1)
// object types
#define UNUSED 0
#define TIP 1
#define TUBE 2
// colors
#define WHITE	0
#define RED		1
#define GREEN	2
#define BLUE	3
#define MAGENTA	4
#define YELLOW	5
#define CYAN	6
#define BLACK	7
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
// PPM file interpretation types
#define PPM_INTERPT_RGB_AS_FLAG   1
#define PPM_INTERPT_RGB_AS_HEIGHT 2


/**************************************************************************************/
/**************************************************************************************/
// TYPEDEFS
typedef int Bool;

// Data structure containing data pertaining to collision test between 
// sphere and tube.
class SPHERE_AND_TUBE_DATA {
public:
	Vec2d vP;	// input: point P
	Vec2d vA;	// input: endpoint A of line seg
	Vec2d vB;	// input: endpoint B of line seg
	Vec2d vM;	// nearest point on line AB to P
	Vec2d vS;	// pivot point
	Vec2d vC;	// midpoint of AB
	double k;	// parameter locating point M = B + (A-B)*k
	double sphereRadius;
	double tubeRadius;
	double dist;		// distance from sphere to tube
};

// Data structure resulting from intersection test between two line
// segments in the plane.  
// The 6 cases are:
// Case 1: line segs non-parallel, disjoint.
// Case 2: line segs non-parallel, intersecting.
// Case 3: line segs parallel, disjoint, mutual projection is line segment.
// Case 4: line segs parallel, disjoint, mutual projection is 1 or 0 points.
// Case 5: line segs parallel, intersecting, intersection = 1 point.  
// Case 6: line segs parallel, intersecting, intersection = line segment.  
class LINE_SEGMENTS_INTERSECTION_TEST {
public:
	Vec2d vD;	// input: endpoint D of line seg DE
	Vec2d vE;	// input: endpoint E of line seg DE
	Vec2d vF;	// input: endpoint F of line seg FG
	Vec2d vG;	// input: endpoint G of line seg FG

	double dist;	// distance between the two line segments
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
};

class SIM_STATE {
public:
	int numObs;
	OB ob[MAXOB];
};


/**************************************************************************************/
/**************************************************************************************/
// FUNCTION PROTOYPES
int		streq( char* str1, char* str2 );
Vec2d	interpolateVecs( Vec2d v0, Vec2d v1, double fraction );
double	interpolateScalars( double s0, double s1, double fraction );
void	showStrokeString( char* str );
void	showBitmapString( char* str );
void	getState( SIM_STATE st );
void	setState( SIM_STATE* pSt );
void	showLabeledPoint( Vec2d vPoint, char* labelStr );
void	rotateRadiansAxis( double angleInRadians, 
				   double axisComponentX, double axisComponentY, double axisComponentZ );
void	setMaterialColor( GLfloat r, GLfloat g, GLfloat b );
void	setColor( int colorIndex );
void	drawCube( double halfWidth );
void	drawTorus( double innerDiameter, double outerDiameter );
void	drawSphere( double radius );
void	showPoint( Vec2d vPoint, int color, double size = 1., double zHeight = 0. );
void	drawCone( double radius, double height );
void	drawCylinder( double diameter, double height );
void	drawUnitSquare( void );
void	drawHollowUnitSquare( void );
void	drawHollowRect( Vec2d v1, Vec2d v2 );
void	drawLine( Vec2d pt1, Vec2d pt2, int color, double z1 = 0., double z2 = 0. );
void	drawTip( double tipDiameter );
void	drawTube( double diameter, double length, int obIndex );
void	lighting( void );
double	norm( Vec2d v );
double	vecDistance( Vec2d pt1, Vec2d pt2 );
int		findNearestObToMouse( void );
void	addObject( int obNum, 
		   int type, Vec2d pos, double yaw, double roll, double leng, double diam,
		   int nextSeg, int prevSeg);
void	deleteOb( int n );
double	centerAngle( double angle );
double	calcAngleInPlane( Vec2d vA, Vec2d vB, Vec2d vC );
double	pivotPointEquation( double k );
Vec2d	PivotPoint( double normalizedPushPoint, Vec2d vEndA, Vec2d vEndB  );
double	nearestPointOnLineToPoint( Vec2d vA, Vec2d vB, Vec2d vP );
void	collisionDraw( SPHERE_AND_TUBE_DATA cxTest );
void	lineSegmentCollisionDraw( LINE_SEGMENTS_INTERSECTION_TEST lineSegCx );
int	betweenInclusive( double value,   double bound1, double bound2 );
Vec2d	xformDEtoYaxis( Vec2d vIn, Vec2d vD, Vec2d vE );
Vec2d	inverseXformDEtoYaxis( Vec2d vIn, Vec2d vD, Vec2d vE );
double	touchDistance( double radius1, double radius2 );
SPHERE_AND_TUBE_DATA	distSphereToCylinder( Vec2d vP, Vec2d vA, Vec2d vB, 
	                  double radiusSphere, double radiusCylinder );
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
void	collisionResponse( Vec2d vPushPt, double radiusSphere, int pushedTube );
void	crossedTubeAxisCollisionResponse(  LINE_SEGMENTS_INTERSECTION_TEST lineSegCx,
				int pushingTube, int pushedTube  );
void	TubeTubeCollisionTestAndResponse( int pushingTube, int pushedTube );
void	positionSegment( int curSeg, int lastSeg, int directionFlag );
void	positionSegmentedTube( int movedSegment );
void	propagateMovements( int movedTube );
void	markAllTubesMovable( void );
int	newSegmentedTube( int numSegments, 
				  double segmentLength, double segmentWidth, Vec2d vEndpoint  );
void	collisionStuff( void );
void	showSubstrateLines( void );
void	showText( void );
void	showObjectLabels( void );
void	drawObjects( void );
void	drawStuff( void );
void	simStep(void);
int		moveTipToXYLoc( double x, double y, double setPoint );
int		getImageHeightAtXYLoc( double x, double y, double* z );
void	crossProduct( double  x1, double  y1, double  z1,
			  double  x2, double  y2, double  z2,
			  double* px, double* py, double* pz );
void	normalize( double* px, double* py, double* pz );
void	showGrid( void );
void	drawFrame( void );
void	singleSimStep(void);
void	toggleSim(void);
void	adjustOrthoProjectionToWindow( void );
void	zoom( double zoomFactor );
void	pan( double screenFractionX, double screenFractionY );
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
void	robotStep(void);
void	showRobotStuff( void );
void	robotExecutive( void );
Vec2d	towardVec( Vec2d vTarget, Vec2d vFrom, double speed );
void	moveTipTowardPoint( Vec2d vPoint );
void	startRobot( void );
void	popFrontPathQ( void );
void	pushBackPathQ( Vec2d vPathPoint );
Vec2d	frontPathQ( void );
Bool	emptyPathQ( void );
void	clearPathQ( void );
void	moveTipToRotateTube( void );
int		rotatedEnoughTest( void );
void	moveTipToTranslateTube( void );
void	showBox( OB tube, int color );
void	calcTubePointsAndVectors( 
	OB tube, Vec2d* vCenter, Vec2d* vAxis, Vec2d* vWidth,
	Vec2d* vRightClear, Vec2d* vLeftClear, Vec2d* vTopClear, Vec2d* vBottomClear );
Vec2d	normalizeVec( Vec2d v );
void	showSphere( Vec2d vPoint, int color, double radius );
OB		locateTube( OB estimatedTubePose );
void	addPointToTipPath( Vec2d vPathPoint );
int		translatedEnoughTest( void );
int		rotatedEnoughTest2( void );
void	repelTubeRadiallyFromPoint( Vec2d vPushPt, int tube, 
				SPHERE_AND_TUBE_DATA cxTest );
SPHERE_AND_TUBE_DATA	calcCxData( 
				Vec2d vPushPt, double radiusSphere, int pushedTube );
int		findNearestSegmentToPoint( Vec2d vPushPt, int tube );
void	deleteSegmentedTube( int seg );
void	addSegmentAtEnd( int seg );
void	extractFoundTubes( void );
void	doImageScanExact( void );
void	doImageScanApprox( void );
void	clearFrameBufferDepthRender( void ); 
void	convertFromImageCoordsToWorldCoords( OB &tube );
void	centerImageScanGridInCurrentView( void );
void	writeGridToPPMFile( void );
void	readGridFromPPMFile( char* fileName, double imageArray[MAX_GRID][MAX_GRID], 
					 int interp = PPM_INTERPT_RGB_AS_FLAG );


/**************************************************************************************/
/**************************************************************************************/
// GLOBAL VARIABLES
// simulation 
OB nullob;		// at index loc -1 (immediately before ob[...])
OB ob[MAXOB];	// array of objects used in the simulation
int numObs = 0;	// current count of objects in sim world
int selectedOb = NULLOB;	// object currently selected by user
int tip = 0;	// the tip is ob #0 (implemented as a tube of zero length)
SIM_STATE prevState;	// state of sim before previous simulation-step
SIM_STATE saveState;	// state of all objects in the simulation (used in undo)
int simType = SIM_SLIDING;
Bool simRunning = 1;
double minRadiusOfCurvature = 64.;	// max bend for flexible tubes (linked segments)

// image scan
Bool autoImageScan = 0;
double zHeight        [MAX_GRID][MAX_GRID];	// array of heights: image scan data
double medialAxisImage[MAX_GRID][MAX_GRID];	// array of medial axis flags: 1=medial
double maskImage      [MAX_GRID][MAX_GRID];	// array of mask image flags: 1=tube pixel
float zBuffer[ 128*128 ];			// raw values (normalized) from Z-buffer
int    scanResolution = 64;	// scan grid resolution
double scanStep   = 2.;		// scan grid pitch (sample-to-sample spacing)
double scanXMin =  0.;		// scan grid origin X coord (left side)
double scanYMin =  0.;		// scan grid origin Y coord (bottom)
	//double scanLength = scanStep * scanResolution;	// scan grid total width/height
	//double scanXMax =   scanXMin + (scanStep * scanResolution);
	//double scanYMax =   scanYMin + (scanStep * scanResolution);
double scanNear =  -100.;	// near end of Z-buffer range
double scanFar  =   100.;	// far  end of Z-buffer range

// recognition
vector<OB> foundTubeVec(0);	// vector of recognized tubes

// robot stuff
Bool robotRunning = 0;
int robotPhase = 1;
double tipSpeed = 4.;
OB currentGoal;
OB measuredTubePose;
OB predictedTubePose;
int currentGoalTube = 1;
Vec2d pathQ[MAX_PATHQ];
int pathQSize = 0;

// display options
int renderStyle = OB_OUTLINE2D;
int gridStyle   = GRID_NONE;
int imageScanType = IMAGE_SCAN_APPROX;
GLenum drawStyle = GL_FILL;       
GLenum shadingModel = GL_FLAT;   // GL_FLAT or GL_SMOOTH
Bool lightOn[8] = { 1, 1, 0, 0, 0, 0, 0, 0 };
Bool namesOn = 0;
Bool displayCollisionsOn  = 1;
Bool displayPivotPointsOn = 1;
Bool linesOn = 0;
Bool displayConeOn = 0;
Bool displayBendLimitOn = 0;
int frameCount = 0;
double framesPerSecond = 0.;

// window stuff
int mainWindowID;
int UIWindowID;
int viewWindowID;
int depthWindowID;
double viewYawAngle   =  PI/2.;
double viewPitchAngle = -PI/3.;	// was -PI/2.
double windowWidth  = 600.;
double windowHeight = 600.;
double orthoFrustumCenterX = 64.;	// area of XY plane always visible for all window aspect ratios
double orthoFrustumCenterY = 64.;
double orthoFrustumWidthNominal  = 128. + 10.;
double orthoFrustumHeightNominal = 128. + 10.;
// actual bounds of current ortho view frustum matching window aspect ratio
double orthoFrustumLeftEdge;
double orthoFrustumBottomEdge;
double orthoFrustumWidth;
double orthoFrustumHeight;

// mouse and cursor
int xMouseInWindow;	// mouse position in world coords
int yMouseInWindow;	// mouse position in world coords
Vec2d vMouseWorld;	// mouse position in world coords (as a 2D vector)
Vec2d vGrabOffset;	// offset from cursor position to grabbed object (in world coords)

/**************************************************************************************/
// CODE FOR ROUTINES.  Categories of routines:
	// MAIN AND TOP-LEVEL ROUTINES
	// GRAPHICS -- TOP-LEVEL
	// KEYBOARD
	// MOUSE AND CURSOR
	// USER INTERFACE
	// OBJECTS
	// SIMULATION
	// GRAPHICS PRIMITIVES
	// UTILITY ROUTINES
	// TIMING
	// COLLISION DETECTION
	// COLLISION RESPONSE
	// PROPAGATE PUSHES
	// OTHER SIMS AND DEFORMABLE OBJECTS
	// IMAGE SCAN


/**************************************************************************************/
/**************************************************************************************/
// MAIN AND TOP-LEVEL ROUTINES

/**************************************************************************************/
// Main routine for interactive graphical simulator for carbon nanotubes (CNT).
int
main(int argc, char *argv[])
{
#ifdef AFMSIM
	// AFMSIM is #defined when the CNT simulator is being linked with
	// the AFM simulator.  If so, call initJake to initialize the
	// AFM simulator code.  
	initJake( 128., 128. );	
			// args: width & height of scan region 
			// The scan performed by the AFM simulator will runs over this
			// region of the XY-plane: [-width/2,width/2] x [-height/2,height/2]
			// (ie, the scan is centered at the origin).  
#endif

	// Deal with command line.
	glutInit(&argc, argv);
	glutInitDisplayMode(dblBuf);

	// Depth WINDOW
	glutInitWindowSize( (int)64, (int)64 );
	glutInitWindowPosition( 0, 650 );
	depthWindowID = glutCreateWindow( "Depth window for CNT simulator." );

	glutDisplayFunc( displayFuncDepth );
	glutIdleFunc(                idleFuncDummy );
	glutKeyboardFunc(        keyboardFuncDummy );
	glutMouseFunc(              mouseFuncDummy );
	glutMotionFunc(       mouseMotionFuncDummy );
	glutReshapeFunc(    reshapeWindowFuncDummy );


	// UI WINDOW
	glutInitWindowSize( (int)600, (int)600 );
	glutInitWindowPosition( 610, 0 );
	UIWindowID = glutCreateWindow( "UI window for CNT simulator." );

	glutDisplayFunc( displayFuncUI);
	glutIdleFunc(                idleFuncDummy );
	glutKeyboardFunc(        keyboardFuncDummy );
	glutMouseFunc(              mouseFuncDummy );
	glutMotionFunc(       mouseMotionFuncDummy );
	glutReshapeFunc(    reshapeWindowFuncDummy );


	// side view WINDOW
	glutInitWindowSize( (int)600, (int)600 );
	glutInitWindowPosition( 610, 390 );
	viewWindowID = glutCreateWindow( "View window for CNT simulator." );
	adjustOrthoProjectionToWindow();

	glutDisplayFunc(displayFuncView);
	glutIdleFunc(                idleFuncDummy );
	glutKeyboardFunc(        keyboardFuncDummy );
	glutMouseFunc(              mouseFuncDummy );
	glutMotionFunc(       mouseMotionFuncDummy );
	glutReshapeFunc(    reshapeWindowFuncDummy );



	// MAIN WINDOW
	glutInitWindowSize( (int)windowWidth, (int)windowHeight );
	glutInitWindowPosition( 0, 0 );
	mainWindowID = glutCreateWindow( "Warren's CNT simulator v1.4" );
	adjustOrthoProjectionToWindow();

	// pass pointers to callback routines for main window
	glutDisplayFunc(  displayFuncMain);
	glutIdleFunc(                idleFuncDummy );
	glutKeyboardFunc(        keyboardFuncDummy );
	glutMouseFunc(    mouseFuncMain );
	glutMotionFunc(   mouseMotionFuncMain );
	glutReshapeFunc(  reshapeWindowFuncMain );


	// start the timer, which measured elasped real time
	glutTimerFunc( 1000, timerFunc, 0 );


	// Other inits.
	initObs();

	// app's main loop, from which callbacks to above routines occur
	glutMainLoop();

	return 0;               /* ANSI C requires main to return int. */
}


/**************************************************************************************/
// dummy (stub) routines for window callback routines
void displayFuncDummy( void ) {}
void keyboardFuncDummy(unsigned char key, int x, int y)   {commonKeyboardFunc(key,x,y);}
void idleFuncDummy( void ) {commonIdleFunc();}
void mouseFuncDummy( int button, int state, int x, int y ) {}
void mouseMotionFuncDummy( int x, int y ) {}
void reshapeWindowFuncDummy( int newWindowWidth, int newWindowHeight ) {}

#if 0
	// pass pointers to callback routines for window
	glutDisplayFunc(          displayFuncDummy );
	glutIdleFunc(                idleFuncDummy );
	glutKeyboardFunc(        keyboardFuncDummy );
	glutMouseFunc(              mouseFuncDummy );
	glutMotionFunc(       mouseMotionFuncDummy );
	glutReshapeFunc(    reshapeWindowFuncDummy );
#endif


/**************************************************************************************/
// This idle function marks both windows for redisplay, which will cause
// their display callbacks to be invoked.
void
commonIdleFunc( void )
{
#ifdef AFMSIM
	// AFMSIM is #defined when the CNT simulator is being linked with
	// the AFM simulator.  If so, call jakeMain each iteration of the
	// main loop to service the image data requests and tip movement
	// commands.  
	jakeMain();
#endif

	glutSetWindow( mainWindowID );		glutPostRedisplay();
	glutSetWindow( UIWindowID );		glutPostRedisplay();
	glutSetWindow( viewWindowID );		glutPostRedisplay();
	glutSetWindow( depthWindowID );		glutPostRedisplay();
}


/**************************************************************************************/
/**************************************************************************************/
// GRAPHICS -- TOP-LEVEL

/**************************************************************************************/
void
drawFrame( void )
{
	// draw into main window
	glutSetWindow( mainWindowID );

	// Setup OpenGL state.
	glClearDepth(1.0);
	glClearColor(0.5, 0.5, 0.5, 0.0);   // gray background

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glEnable(GL_DEPTH_TEST);

	// set up graphics
	if(      renderStyle == OB_OUTLINE2D)  shadingModel = GL_FLAT;
	else if( renderStyle == OB_WIREFRAME)  shadingModel = GL_SMOOTH;
	else if( renderStyle == OB_POINTS)     shadingModel = GL_FLAT;
	else if( renderStyle == OB_SILHOUETTE) shadingModel = GL_SMOOTH;
	else if( renderStyle == OB_SOLID)      shadingModel = GL_SMOOTH;
	lighting();
	glPointSize( 2. );    // se p51 Woo 3rd ed
	glLineWidth( 2. );

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();


	// User input (mouse and keyboard): callbacks are assumed to have been
	// called prior to the callback for this graphics frame.

	// Do robot path planning, tip movement, and graphics.
	robotStep();

	// Do collision detection and collision response
	// (may draw some graphics, too.)
	simStep();

	// Draw objects (tip and tubes).
	drawStuff();

	// Draw the image scan grid.
	showGrid();

	// end of display frame, so flip buffers
	glutSwapBuffers();

#define DEBUG
#ifdef DEBUG
	{                     /* For help debugging, report any
					         OpenGL errors that occur per frame. */
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR)
		fprintf(stderr, "GL error: %s\n", gluErrorString(err));
	}
#endif
}


/**************************************************************************************/
void
drawStuff( void )
{
	showSubstrateLines();
		
	// draw all objects at their current locations
	drawObjects();
	showObjectLabels();
}


/**************************************************************************************/
// draw text labels on things
void
showObjectLabels( void )
{
	int i;
	char str[256];


	if( namesOn ) {
		// Put text labels next to objects
		for( i=0; i<numObs; i++ ) {
			if( ob[i].type==UNUSED )   continue;

			glPushMatrix();
			glTranslatef( ob[i].pos.x, ob[i].pos.y, 0. );

			sprintf( str, "Ob #%d", i );
			showBitmapString( str );

			glPopMatrix();
		}
	}
}


/**************************************************************************************/
// Draw the objects (tip and tubes) in the sim world.
void
drawObjects( void )
{
	int i;

	// draw the objects
	for( i=0; i<numObs; i++ ) {
		// set colors 
		if( i == selectedOb )  setColor( YELLOW );
		else                   setColor( WHITE );


		glPushMatrix();

		// put tube at its (x,y) position
		glTranslatef( ob[i].pos.x, ob[i].pos.y, 0. );

		// put tube flat on XY plane (ie, with its lower surface touching XY plane)
		glTranslatef( 0., 0., ob[i].diam/2. );  

		// set tube yaw angle (in-plane rotation angle)
		rotateRadiansAxis( ob[i].angle, 0.0, 0.0, 1.0 ); 

		// set roll angle around tube axis
		rotateRadiansAxis( ob[i].roll,  1.0, 0.0, 0.0 );

		// draw the tube or other object
		switch( ob[i].type ) {
		case TIP:		
			drawTip( ob[i].diam );
			break;

		case TUBE:		
			drawTube( ob[i].diam, ob[i].leng, i );
			break;

		case UNUSED:
			// do nothing.
					// draw deleted tubes in black
					//setColor( BLACK );			
					//drawTube( ob[i].diam, ob[i].leng, i );
			break;

		default: error( "attempt to draw unknown object type" );
		}

		glPopMatrix();
	}
}


/**************************************************************************************/
// adjust the ortho projection to match window aspect ratio and keep circles round.
void
adjustOrthoProjectionToWindow( void )
{
	double orthoFrustumNearEdge =  -100.;
	double orthoFrustumFarEdge  =   100.;

	// set nominal size of window before taking aspect ratio into account
	double orthoFrustumLeftEdgeNominal   = orthoFrustumCenterX - orthoFrustumWidthNominal/2.;
	double orthoFrustumBottomEdgeNominal = orthoFrustumCenterY - orthoFrustumHeightNominal/2.;

	// calculate aspect ratio of current window
	double aspectRatio = windowWidth / windowHeight;

	// set vertical extent of window to nominal area of world being viewed.
	orthoFrustumHeight = orthoFrustumHeightNominal;
	orthoFrustumBottomEdge = orthoFrustumBottomEdgeNominal;

	// view horizontal extent of world proportional to window width
	orthoFrustumWidth = orthoFrustumWidthNominal * aspectRatio;
	orthoFrustumLeftEdge   = orthoFrustumCenterX - orthoFrustumWidth / 2.;

	// set projection matrix to orthoscopic projection matching current window
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(  orthoFrustumLeftEdge,   orthoFrustumLeftEdge   + orthoFrustumWidth,
		      orthoFrustumBottomEdge, orthoFrustumBottomEdge + orthoFrustumHeight,
		      orthoFrustumNearEdge,   orthoFrustumFarEdge );
}


/**************************************************************************************/
void
zoom( double zoomFactor )
{
	// adjust window size in world coords
	orthoFrustumHeightNominal      *= zoomFactor;
	orthoFrustumWidthNominal       *= zoomFactor;

	// make graphics projection match window dimensions
	adjustOrthoProjectionToWindow();
}


/**************************************************************************************/
void
pan( double screenFractionX, double screenFractionY )
{
	// pan viewing frustum center (which is parallel to Z-axis)
	// around in X and Y.
	orthoFrustumCenterX += (orthoFrustumWidthNominal  * screenFractionX);
	orthoFrustumCenterY += (orthoFrustumHeightNominal * screenFractionY);

	// make graphics projection match window dimensions
	adjustOrthoProjectionToWindow();
}


/**************************************************************************************/
// Center the image scan grid in the area currently being viewed.
void
centerImageScanGridInCurrentView( void )
{
	const double insetFactor = 0.8;		// inset the scan grid so it can be seen
		// Defining parameters for image scan grid:
		// int    scanResolution = 64;	// scan grid resolution
		// double scanStep   = 2.;		// scan grid pitch (sample-to-sample spacing)
		// double scanXMin =  0.;		// scan grid origin X coord (left side)
		// double scanYMin =  0.;		// scan grid origin Y coord (bottom)
	// Calc origin (lower left corner) of scan grid
	scanXMin = orthoFrustumCenterX - (orthoFrustumWidthNominal/2.)  * insetFactor;
	scanYMin = orthoFrustumCenterY - (orthoFrustumHeightNominal/2.) * insetFactor;

	// Calc pixel-to-pixel spacing in image scan grid.
	scanStep = (orthoFrustumWidthNominal / scanResolution) * insetFactor;
}


/**************************************************************************************/
// This routine is called only after input events.
void
displayFuncMain( void )
{
	// draw graphics for this frame
	drawFrame();
}


/**************************************************************************************/
// display graphics in the UI (User Interface) window.
void displayFuncUI( void ) 
{
	glutSetWindow( UIWindowID );

	// set projection matrix to orthoscopic projection matching current window
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(  -10.,   10.,
		      -10.,   10.,
		      -10.,   10. );

	// Setup OpenGL state.
	glClearDepth(1.0);
	glClearColor(0.5, 0.5, 0.0, 0.0);   // gray background

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glEnable(GL_DEPTH_TEST);

	showText();

	// end of display frame, so flip buffers
	glutSwapBuffers();
}


/**************************************************************************************/
// display graphics in the UI (User Interface) window.
void displayFuncView( void ) 
{
	// set up projection matrix to look down X-axis
	adjustOrthoProjectionToWindow();				// set up projection down Z-axis
	rotateRadiansAxis( -PI/2., 0.0, 0.0, 1.0 );		// rotate around Z
	rotateRadiansAxis( viewPitchAngle, 0.0, 1.0, 0.0 );	// rotate view direction to X-axis
	rotateRadiansAxis( viewYawAngle,   0.0, 0.0, 1.0 );	// rotate around Z


	// draw into main window
	glutSetWindow( viewWindowID );

	// Setup OpenGL state.
	glClearDepth(1.0);
	glClearColor(0.5, 0.5, 0.5, 0.0);   // gray background
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glEnable(GL_DEPTH_TEST);

	// draw objects (tip and tubes)
	int saveRenderStyle = renderStyle;
	if( renderStyle == OB_OUTLINE2D )	renderStyle = OB_WIREFRAME;
	drawStuff();
	renderStyle = saveRenderStyle;

	// do collision detection and collision response
	// (may draw some graphics, too.)
//	simStep();

	showGrid();

	// end of display frame, so flip buffers
	glutSwapBuffers();
}


/**************************************************************************************/
/**************************************************************************************/
// KEYBOARD

/**************************************************************************************/
// Keyboard callback for main window.
void
commonKeyboardFunc(unsigned char key, int x, int y)
{
	const int CTRL = 0x1F;	// example: CTRL & 'C' gives Control-C (in ASCII)
	static double pasteTubeLength = 4.;
	static double pasteTubeDiam   = 2.;

		// To respond to non-ASCII (special) key keypresses, see
		// http://reality.sgi.com/opengl/spec3/node54.html#1044
		// Section 7.9 (glutSpecialFunc) -- uses eg, GLUT_KEY_F1 
	x=x; y=y;
//	printf( "key code: 0x%x\n", key );

	switch (key) {

//	case '[':	moveTipToXYLoc( 0.,  20., 0. );	break;   
//	case ']':	moveTipToXYLoc( 0., -20., 0. );	break;   

	// save state and revert
	case CTRL & 'S':	setState( &saveState );		// save state
		break;   
	case CTRL & 'Z':	getState(  saveState );	// revert to saved state (undo)
		setState( &prevState );	// make previous state match new current state
		selectedOb = NULLOB;
		break;   
			



	case CTRL & 'P':	  // sort of like printing
		writeGridToPPMFile();	
		break;
	case CTRL & 'O':	
		readGridFromPPMFile( "medial.ppm", medialAxisImage, PPM_INTERPT_RGB_AS_FLAG );	
		readGridFromPPMFile( "mask.ppm",   maskImage,       PPM_INTERPT_RGB_AS_FLAG );	
		break;



	case 'x':
		readGridFromPPMFile( "simin.ppm", zHeight, PPM_INTERPT_RGB_AS_HEIGHT );
		break;



	case 'I':
			// Do image scan to construct depth image over image grid.
			doImageScanExact();
		break;

	case 'f':   // find tube(s) in image 
			// Do image anaysis to locate tubes within grid.
			extractFoundTubes();
		break;

	case 'F':	autoImageScan = ! autoImageScan;	break;	// auto-find


	// Move image scan grid around.
//	case ']':	scanStep *= 2.;		break;
//	case '[':	scanStep /= 2.;		break;
//	case '}':	scanXMin += scanStep * (scanResolution/4);		break;
//	case '{':	scanXMin -= scanStep * (scanResolution/4);		break;
//	case '}':	scanYMin += scanStep * (scanResolution/4);		break;
//	case '{':	scanYMin -= scanStep * (scanResolution/4);		break;



	// robot stuff
	case 'r':	startRobot();	break;
	case 'g':	// set goal
		if( selectedOb != NULLOB ) {
			currentGoal = ob[selectedOb];
			currentGoalTube = selectedOb;
		}
		break;

	case ']':	tipSpeed *= 2.;		break;
	case '[':	tipSpeed /= 2.;		break;
	case 'k':	clearPathQ();		break;


	
	case 'G':	toggleSim();		break;
	case ' ':	singleSimStep();	break;

	case 'o':
		if(      renderStyle == OB_SOLID )      {renderStyle = OB_WIREFRAME;}
		else if( renderStyle == OB_WIREFRAME )  {renderStyle = OB_OUTLINE2D;}
		else if( renderStyle == OB_OUTLINE2D )  {renderStyle = OB_NONE;}
		else if( renderStyle == OB_NONE )		{renderStyle = OB_SOLID;}
		else								 error( "bad value for renderStyle" );		
		break;
	case 'i':
		if(      gridStyle == GRID_NONE )			{gridStyle = GRID_HALF_SOLID;}
		else if( gridStyle == GRID_HALF_SOLID )		{gridStyle = GRID_SOLID;}
		else if( gridStyle == GRID_SOLID )			{gridStyle = GRID_WIREFRAME;}
		else if( gridStyle == GRID_WIREFRAME )		{gridStyle = GRID_NONE;}
		else										error( "bad value for gridStyle" );		
		break;
	case 'e':
		if(      imageScanType == IMAGE_SCAN_EXACT )	{imageScanType = IMAGE_SCAN_APPROX;}
		else if( imageScanType == IMAGE_SCAN_APPROX )	{imageScanType = IMAGE_SCAN_EXACT;}
		else										    error( "bad value for imageScanType" );		
		break;




//	case '1':	lightOn[0] = ! lightOn[0];	break;
//	case '3':	lightOn[1] = ! lightOn[1];	break;


#if 0
	case 's':	
		if(      simType == SIM_NONE )	    simType = SIM_TRANSLATE;
		else if( simType == SIM_TRANSLATE )	simType = SIM_ROLLING;
		else if( simType == SIM_ROLLING )	simType = SIM_SLIDING;
		else if( simType == SIM_SLIDING )	simType = SIM_ROLLING_SLIDING;
		else if( simType == SIM_ROLLING_SLIDING )	simType = SIM_NONE;
		break;
#endif


	// Display flags: turn graphical annotations on/off
	case 'n':	namesOn  = ! namesOn;	break;
	case '.':	displayCollisionsOn  = ! displayCollisionsOn;	break;
	case ',':	displayPivotPointsOn = ! displayPivotPointsOn;	break;
	case '/':	linesOn =  ! linesOn;	break;
	case '^':	displayConeOn =  ! displayConeOn;	break;
	case '<':	displayBendLimitOn =  ! displayBendLimitOn;	break;




	// Create (paste), copy, and cut (delete) objects.
	case CTRL & 'C':	// copy: capture segment params
		if( selectedOb ) {
			pasteTubeLength = ob[ selectedOb ].leng;
			pasteTubeDiam   = ob[ selectedOb ].diam;
		}
		break;
	case CTRL & 'V':	// paste single-segment (stiff) tube
		newSegmentedTube( 1, pasteTubeLength, pasteTubeDiam, vMouseWorld  );	
		break;	
	case 'V':	// paste flexible, multi-segment tube.
		newSegmentedTube( 9, pasteTubeLength, pasteTubeDiam, vMouseWorld  );	
		break;	
	case CTRL & 'X':	// cut one segment
		if( selectedOb!=NULLOB) {
			pasteTubeLength = ob[ selectedOb ].leng;
			pasteTubeDiam   = ob[ selectedOb ].diam;

			deleteOb( selectedOb);
		}
		break;		
	case 'X':	// cut whole flexible tube
		if( selectedOb!=NULLOB) {
			pasteTubeLength = ob[ selectedOb ].leng;
			pasteTubeDiam   = ob[ selectedOb ].diam;

			deleteSegmentedTube( selectedOb);
		}
		break;		
	case 'A':	// add new segment to existing flexible tube
		if( selectedOb!=NULLOB)  addSegmentAtEnd( selectedOb);	
		break;		






	case 'b':	
		minRadiusOfCurvature /= 2.;	
		if( minRadiusOfCurvature < 1. )   minRadiusOfCurvature = 0.;
		break;
	case 'B':	
		minRadiusOfCurvature *= 2.;	
		if( minRadiusOfCurvature == 0. )   minRadiusOfCurvature = 1.;
	break;

	// change object parameters
	case 'T':	ob[selectedOb].angle +=  5. * DEG_TO_RAD;	break;
	case 't':	ob[selectedOb].angle += -5. * DEG_TO_RAD;	break;

	case 'l':	
		if( ob[selectedOb].type == TIP )  break;
		ob[selectedOb].leng +=  1.;	
		break;
	case 'L':	
		if( ob[selectedOb].type == TIP )  break;
		ob[selectedOb].leng += (ob[selectedOb].leng > 0.) ? -1. : 0.;	
		break;

	case 'w':	ob[selectedOb].diam +=  1.;	break;
	case 'W':	ob[selectedOb].diam += (ob[selectedOb].diam > 1.) ? -1. : 0.;	break;


	// zoom and pan using numeric keypad
	case '1':	zoom( 1.25 );	break;		// zoom out
	case '2':	zoom( 0.8 );	break;		// zoom in
	case '3':	pan( -0.25,  0.  );	break;	// pan left
	case '4':	pan(  0.25,  0.  );	break;	// pan right
	case '5':	pan(  0.,  -0.25 );	break;	// pan down
	case '6':	pan(  0.,   0.25 );	break;	// pan up

	case '0':	centerImageScanGridInCurrentView();	break;	// center grid in view





	case 'Y':	viewYawAngle   +=  5. * DEG_TO_RAD;	break;
	case 'y':	viewYawAngle   += -5. * DEG_TO_RAD;	break;
	case 'P':	viewPitchAngle +=  5. * DEG_TO_RAD;	break;
	case 'p':	viewPitchAngle += -5. * DEG_TO_RAD;	break;
//	case 'V':	viewYawAngle = PI/2.;  viewPitchAngle = -PI/3.; break; // std view

	case 'q':	// quit
	case 27:	// Esc
		// Exit program.  
		exit(0);
	}

	glutPostRedisplay();	// in case something was changed
}


/**************************************************************************************/
/**************************************************************************************/
// MOUSE AND CURSOR

/**************************************************************************************/
// Callback routine: called for mouse button events (pressed and/or released).
// button: GLUT_LEFT_BUTTON, GLUT_RIGHT_BUTTON, GLUT_MIDDLE_BUTTON
// state:  GLUT_UP (released), GLUT_DOWN (depressed)
// x,y:    cursor loc at mouse event in window coords
// see p658 Woo 3rd ed
void
mouseFuncMain( int button, int state, int x, int y )
{
	calcMouseWorldLoc( x, y );

	switch( button ) {
	case GLUT_LEFT_BUTTON: 
		if(      state == GLUT_DOWN )	{grabNearestOb();}
		else if( state == GLUT_UP )		{}
		break;
	case GLUT_RIGHT_BUTTON: 
		if(      state == GLUT_DOWN )	{}
		else if( state == GLUT_UP )		{}
		break;
	}

	glutPostRedisplay();
}


/**************************************************************************************/
// Callback routine: called when mouse is moved while a button is down.
// Only called when cursor loc changes.
// x,y:    cursor loc in window coords
// see p658 Woo 3rd ed
void
mouseMotionFuncMain( int x, int y )
{
	// Map mouse cursor window coords to world coords.
	// Since we're using an orthoscopic projection parallel to the Z-axis,
	// we can map (x,y) in window coords to (x,y,0) in world coords.
	calcMouseWorldLoc( x, y );

	// Move the grabbed object, if any, to match mouse movement.
	moveGrabbedOb();

//	glutPostRedisplay();
}


/**************************************************************************************/
void
moveGrabbedOb( void )
{
	// move the selected object, if any, to correspond with the mouse movement
	if( selectedOb != NULLOB ) {
		ob[selectedOb].pos   = vMouseWorld + vGrabOffset;
	}
}


/**************************************************************************************/
// search for nearest ob to cursor, and set grab offset vector.
void
grabNearestOb( void )
{
	// search through all objects to find the one nearest to the cursor.
	selectedOb = findNearestObToMouse();  // may be NULLOB

	// calculate grab offset vector
	if( selectedOb != NULLOB ) {
		vGrabOffset = ob[selectedOb].pos   - vMouseWorld;
	}

	moveGrabbedOb();
}


/**************************************************************************************/
// Calculate where the cursor maps to in world coordinates, 
// based on the window width and height and the edges of
// the frustum of the orthoscopic projection.
void
calcMouseWorldLoc( int xMouse, int yMouse ) 
{
	double xMouseNormalized;
	double yMouseNormalized;

	// write the cursor loc in window coords to global var
	xMouseInWindow = xMouse;
	yMouseInWindow = yMouse;	

	// calculate normalized cursor position in window: [0,1]
	xMouseNormalized = xMouseInWindow / windowWidth;
	yMouseNormalized = yMouseInWindow / windowHeight;

	// invert normalized Y due to up being - in mouse coords, but + in ortho coords.
	yMouseNormalized = 1. - yMouseNormalized;

	// calculate cursor position in ortho frustum's XY plane
	vMouseWorld.x = (xMouseNormalized * orthoFrustumWidth)  + orthoFrustumLeftEdge;
	vMouseWorld.y = (yMouseNormalized * orthoFrustumHeight) + orthoFrustumBottomEdge;
}


/**************************************************************************************/
// callback routine: called when window is resized by user
void
reshapeWindowFuncMain( int newWindowWidth, int newWindowHeight )
{
	windowWidth  = newWindowWidth;
	windowHeight = newWindowHeight;

	// viewport covers whole window
	glViewport( 0, 0, (int)windowWidth, (int)windowHeight );

	// make graphics projection match window dimensions
	adjustOrthoProjectionToWindow();
}


/**************************************************************************************/
// searches through all objects to find the object nearest the mouse cursor.
// returns the index of the object, or NULLOB if no objects are within threshold.
int
findNearestObToMouse( void )
{
	int i;
	int nearestOb = NULLOB;
	double nearestDist = 1000000.;
	double thresholdDist = 10.;
	double dist;

	for( i=0; i<numObs; i++ ) {
		if( ob[i].type==UNUSED )   continue;

		dist = vecDistance( vMouseWorld, ob[i].pos );

		if( dist < nearestDist  &&  dist < thresholdDist ) {
			nearestDist = dist;
			nearestOb   = i;
		}
	}

	return nearestOb;
}


/**************************************************************************************/
/**************************************************************************************/
// USER INTERFACE

/**************************************************************************************/
void
showText( void )
{
	// Title at top middle of screen giving sim type
//	glPushMatrix();
//	glTranslatef(  -3., 9., 0. );
//
//	switch( simType ) {
//	case SIM_NONE:		showStrokeString( "DETECT COLLISIONS" );break;
//	case SIM_TRANSLATE:	showStrokeString( "TRANSLATE sim" );break;
//	case SIM_ROLLING:	showStrokeString( "ROLLING sim" );break;
//	case SIM_SLIDING:	showStrokeString( "SLIDING sim" );break;
//	case SIM_ROLLING_SLIDING:	showStrokeString( "ROLLING & SLIDING sim" );break;
//	}
//	glPopMatrix();

	// first column UI screen
	glPushMatrix();
	glTranslatef( -10., 9.5, 0. );

	showBitmapString( "CNT simulator and robot, version 2.0" );
	showBitmapString( "Warren Robinett, June 2000" );
	showBitmapString( "rigid and flexible nanotubes" );
	showBitmapString( "" );

	// count frames and display count
	frameCount++;
	char str[256];
	sprintf( str, "frameCount:%8d", frameCount ); showBitmapString( str );
	sprintf( str, "frames/sec: %6.1f", framesPerSecond ); showBitmapString( str );

	sprintf( str, "simulation on:   %d", simRunning ); showBitmapString( str );
//	sprintf( str, "simulation type: %d", simType );    showBitmapString( str );
	sprintf( str, "robot on:        %d", robotRunning ); showBitmapString( str );
	sprintf( str, "auto image scan: %d", autoImageScan );    showBitmapString( str );
	sprintf( str, "image-scan type (1=exact): %d", imageScanType );    showBitmapString( str );
	sprintf( str, "number of objects: %d", numObs ); showBitmapString( str );
	sprintf( str, "minRadiusOfCurvature: %5.1f", minRadiusOfCurvature ); showBitmapString( str );




	if( selectedOb != NULLOB ) {
		showBitmapString( "" );
		showBitmapString( "SELECTED OBJECT (tube or tip)" );
		sprintf( str, "  object #:  %d",            selectedOb );   showBitmapString( str );
		sprintf( str, "  position:  %6.1f, %6.1f",  ob[selectedOb].pos.x, ob[selectedOb].pos.y ); showBitmapString( str );
		sprintf( str, "  yaw angle: %6.1f degrees", ob[selectedOb].angle * RAD_TO_DEG ); showBitmapString( str );
//		sprintf( str, "  roll angle:%6.1f degrees", ob[selectedOb].roll  * RAD_TO_DEG ); showBitmapString( str );
		sprintf( str, "  length:    %6.1f nm",      ob[selectedOb].leng ); showBitmapString( str );
		sprintf( str, "  diameter:  %6.1f nm",      ob[selectedOb].diam ); showBitmapString( str );

		sprintf( str, "  nextSeg:  %d ", ob[selectedOb].nextSeg ); showBitmapString( str );
		sprintf( str, "  prevSeg:  %d ", ob[selectedOb].prevSeg ); showBitmapString( str );
	}


	//	pathQ
	for( int j=0; j<pathQSize; j++ ) {
		sprintf( str, "pathQ: %6.1f %6.1f", pathQ[j].x, pathQ[j].y ); showBitmapString( str );
	}



	glPopMatrix();



	// second column in UI screen
	glPushMatrix();
	glTranslatef( 0., 9.5, 0. );

	showBitmapString( "KEYBOARD COMMANDS" );
//	showBitmapString( "g: simulation on/off" );
//	showBitmapString( "SPACEBAR: single-step sim" );
	showBitmapString( "q or ESC: quit program" );
	showBitmapString( "" );

	showBitmapString( "CTRL-S: save state" );			 // save
	showBitmapString( "CTRL-Z: revert to saved state" );  // undo

	showBitmapString( "f: find tubes in image" );
	showBitmapString( "F: auto-find (every frame)" );
	showBitmapString( "i: change image  draw mode" );
	showBitmapString( "e: exact/approx image scan" );
	showBitmapString( "CTRL-P: 'print' image to PPM file" );
	showBitmapString( "" );

	showBitmapString( "r: start/stop robot" );
	showBitmapString( "g: set robot's goal" );
	showBitmapString( "[,]: speed up, slow down robot" );
	showBitmapString( "k: kill robot plan" );
	showBitmapString( "" );

	showBitmapString( "o: change object draw mode" );
	showBitmapString( "CTRL-X: cut selected segment" );
	showBitmapString( "CTRL-V: paste new stiff tube" );
	showBitmapString( "CTRL-C: copy segment params" );
	showBitmapString( "V: paste new flexible tube" );
	showBitmapString( "X: cut selected tube" );
	showBitmapString( "A: add segment to tube" );
	showBitmapString( "b,B: change min radius of curvature" );
	showBitmapString( "w,W: change tube width (diameter)" );
	showBitmapString( "l,L: change tube length" );
	showBitmapString( "t,T: turn tube" );
	showBitmapString( "" );

	showBitmapString( "/: lines on/off" );
	showBitmapString( ".: collision points on/off" );
	showBitmapString( ",: pivot points on/off" );
	showBitmapString( "^: cone on/off" );
	showBitmapString( "<: bend limits on/off" );
	showBitmapString( "" );

	showBitmapString( "1,2: zoom out, in" );
	showBitmapString( "3,4,5,6: pan" );
	showBitmapString( "0: center image scan grid" );
//	showBitmapString( "y,Y: change yaw   view angle" );
//	showBitmapString( "p,P: change pitch view angle" );
	showBitmapString( "" );

	glPopMatrix();
}


/**************************************************************************************/
/**************************************************************************************/
// OBJECTS

/**************************************************************************************/
void
initObs( void )
{
	// We start with no objects.
	numObs = 0;
	
	// Create tip object.
	Assert( numObs == tip, "tip not object #0" ); 
	addObject( numObs, TIP,  Vec2d( 0., 0.), 0., 0., 0.001, 10.,   NULLOB, NULLOB );
			// tip length of 0.001 is workaround for prob with zero tip length

	// Create rigid tubes.
//	addObject( numObs, TUBE, Vec2d(  60.,  40.), 0., 0.,10., 1.,   NULLOB, NULLOB );
//	addObject( numObs, TUBE, Vec2d(  60.,  60.), 0., 0.,20., 2.,   NULLOB, NULLOB );
	addObject( numObs, TUBE, Vec2d(  64.,  64.), 0., 0.,60., 10.,  NULLOB, NULLOB );

//	addObject( numObs, TUBE, Vec2d(  64.,  120.), 0., 0.,10., 4.,  NULLOB, NULLOB );


	// Make segmented (flexible) tubes.
//	int segTube1 = newSegmentedTube( 21, 4., 2., Vec2d(32.,32.)  );
		//	int segTube2 = newSegmentedTube( 3, 20., 5., Vec2d(32.,32.)  );


	// init goal
	Assert( numObs >= 1, "no tubes exist to use as goal" );
	currentGoalTube = 1;
	currentGoal = ob[1];
	currentGoal.pos += Vec2d(0.,40.);	// move goal away from target tube


	// To start with, set the previous state (used in computing movements) 
	// and saved state (used in undo)
	// to be the initial state.  
	setState( &prevState );
	setState( &saveState );
}


/**************************************************************************************/
void
addObject( int obNum, 
		   int type, Vec2d pos, double yaw, double roll, double leng, double diam,
		   int nextSeg, int prevSeg )
{
	ob[obNum].type  = type;
	ob[obNum].pos   = pos;
	ob[obNum].angle = yaw;
	ob[obNum].roll  = roll;
	ob[obNum].leng  = leng;
	ob[obNum].diam  = diam;

	ob[obNum].nextSeg    = nextSeg;
	ob[obNum].prevSeg    = prevSeg;

	numObs++;
}


/**************************************************************************************/
// Delete object n from object data structure.
void
deleteOb( int n )
{
	// check object index for validity
	if( n < 0  ||  n >= numObs ) {
		error( "Attempt to delete non-existent object" );
	}

	// don't allow deletion of tip object.
	if( ob[n].type == TIP )		return;



	// unlink segments that link to this segment
	int seg;
	if( (seg=ob[n].nextSeg) != NULLOB ) {
		Assert( ob[seg].prevSeg == n, "links inconsistent" );
		ob[seg].prevSeg = NULLOB;
	}
	if( (seg=ob[n].prevSeg) != NULLOB ) {
		Assert( ob[seg].nextSeg == n, "links inconsistent" );
		ob[seg].nextSeg = NULLOB;
	}

	// Mark this tube segment as unused.
	ob[n].type = UNUSED;
	ob[n].nextSeg = NULLOB;
	ob[n].prevSeg = NULLOB;


	// if the object deleted was the selected object, then there
	// is now no object selected.
	if( n == selectedOb ) {
		selectedOb = NULLOB;
	}
}


/**************************************************************************************/
void
deleteSegmentedTube( int seg )
{
	// Go to an end segment of the tube. 
	for(   ; ob[seg].prevSeg != NULLOB; seg = ob[seg].prevSeg )   {}

	// Delete all the segments linked into this multi-segment tube.
	for( int savedLink = seg;  savedLink != NULLOB;  seg = savedLink ) {
		savedLink = ob[seg].nextSeg;

		deleteOb( seg );
	}

}


/**************************************************************************************/
void
addSegmentAtEnd( int seg )
{
	if( numObs < MAXOB-1 ) {
		// Go to an end segment of the tube. 
		for(   ; ob[seg].prevSeg != NULLOB; seg = ob[seg].prevSeg )   {}

		// create new segment
		int newSeg = numObs;
		addObject( newSeg, TUBE, Vec2d(0.,0.), 
					0., 0., ob[seg].leng, ob[seg].diam,   
					NULLOB, NULLOB );

		// link it onto end of existing tube.
		ob[seg].prevSeg    = newSeg;
		ob[newSeg].nextSeg = seg;
	}
}


/**************************************************************************************/
/**************************************************************************************/
// ROBOT

/**************************************************************************************/
void
robotStep(void)
{
	//display current goal and planned tip path	
	showRobotStuff();


	// If robot is awake, issue tip movement commands
	if( robotRunning )   {
		robotExecutive();
	}
}


/**************************************************************************************/
// Issue tip movement commands based on robot's goals.
void
robotExecutive( void )
{
	Bool testResult;
	switch( robotPhase ) {
	case 1:	testResult = rotatedEnoughTest();		break;
	case 2:	testResult = translatedEnoughTest();	break;
	case 3:	testResult = rotatedEnoughTest2();		break;
	}

	if( testResult ) {
		clearPathQ();

		predictedTubePose = ob[currentGoalTube];
			// (for now, just cheat and look in simulator data structure).
		measuredTubePose = locateTube( predictedTubePose );
			// (for now, just cheat and look in simulator data structure).

		switch( robotPhase ) {
		case 1: 
			robotPhase++; 
			break;

		case 2: 		
			robotPhase++; 
			break;

		case 3: 		
			robotRunning = 0;
			return;

			break;
		}

	}


	// test whether tip has arrived at target point
	double gotThereThreshold = 1.;
	if( !emptyPathQ()  &&
		vecDistance( ob[tip].pos, frontPathQ() ) <= gotThereThreshold ) {
		// pop queue to move tip to next target point
		if( !emptyPathQ() ) {
			popFrontPathQ();
		}
	}


	// if the target point queue is empty, stop robot
	if( emptyPathQ() ) {
		// load new tip path
		switch( robotPhase ) {
		case 1: moveTipToRotateTube();    break;
		case 2: moveTipToTranslateTube(); break;
		case 3: moveTipToRotateTube();    break;
		}
	}


	// We have not yet gotten to this target point, so
	// keep moving toward it.  
	moveTipTowardPoint( frontPathQ() );
}


/**************************************************************************************/
// Start and stop the robot from the keyboard
void
startRobot( void )
{
	if( !robotRunning  &&  emptyPathQ() ) {
		moveTipToRotateTube();

		robotPhase = 1;
	}
	
	robotRunning = ! robotRunning;	
}


/**************************************************************************************/
int
rotatedEnoughTest2( void )
{
	double angleTube     = ob[currentGoalTube].angle;
	double angleTarget   = currentGoal.angle;

	// test to see whether angles agree within threshold
	double deltaAngle = fmod( fabs(angleTube - angleTarget), 2.*PI );
	double misorientationThreshold = PI/6.;

	return deltaAngle <= misorientationThreshold;

//	return 0;
}


/**************************************************************************************/
int
translatedEnoughTest( void )
{
	// if tube being moved is within threshold distance
	// of target position, return TRUE.
	Vec2d vTube     = ob[currentGoalTube].pos;
	Vec2d vTarget   = currentGoal.pos;
	double distToTarget = vecDistance( vTarget, vTube );
	double distanceThreshold = ob[currentGoalTube].leng/8.;
	
	return distToTarget <= distanceThreshold;
//	return 0;
}


/**************************************************************************************/
int
rotatedEnoughTest( void )
{
	// Compare goal orientation with current tube orientation
	// The goal orientation has the tube normal to the path 
	// to the goal position.
	Vec2d vTube     = ob[currentGoalTube].pos;
	Vec2d vTarget   = currentGoal.pos;
	Vec2d vTowardTarget = vTarget - vTube;
	Vec2d vNormal = vTowardTarget;    vNormal.rotate( -PI/2. );
	double angleTarget = atan2( vNormal.y, vNormal.x );

	double angleTube   = ob[currentGoalTube].angle;

	// test to see whether angles agree within threshold
	double deltaAngle = fmod( fabs(angleTube - angleTarget), 2.*PI );
		// replace 2.*PI with PI to recognize tube symmetry and stop
		// at either orientation PI radians apart.
		// To make this work, pushing code must also deal with this symmetry.


	double misorientationThreshold = PI/8.;

	return deltaAngle <= misorientationThreshold;
}


/**************************************************************************************/
void
moveTipToTranslateTube( void )
{
	predictedTubePose = ob[currentGoalTube];
		// (for now, just cheat and look in simulator data structure).
	measuredTubePose = locateTube( predictedTubePose );
		// (for now, just cheat and look in simulator data structure).




	// figure direction to goal position
	Vec2d vTarget   = currentGoal.pos;
	Vec2d vDesiredTranslationDirection = vTarget - measuredTubePose.pos;

	// calc perpendicular to translation direction
	// and construct a desired pose fro which to calculate
	// pushes that keep the tube going toward the target.
	Vec2d vAdjustedAxis = vDesiredTranslationDirection; vAdjustedAxis.rotate(-PI/2.);
	double desiredOrientation = atan2( vAdjustedAxis.y, vAdjustedAxis.x );
	OB alignedPose = measuredTubePose;
	alignedPose.angle = desiredOrientation;


	// calc points, vectors, and constants needed
	Vec2d vC, vAxis, vWidth, vRightTouch, vLeftTouch, vTopTouch, vBottomTouch;
	calcTubePointsAndVectors( alignedPose, &vC, &vAxis, &vWidth,
				&vRightTouch, &vLeftTouch, &vTopTouch, &vBottomTouch );


	double turnAngle = PI/6.;	// how much to rotate tube each push
	Vec2d vPivotArm = vAxis * 0.5;
	double pushLength = norm(vPivotArm) * tan(turnAngle);
	Vec2d vPush = normalizeVec(vWidth) * pushLength;

	Vec2d vRight  = vRightTouch  * 1.2;
	Vec2d vLeft   = - vRight;
	Vec2d vTop    = vRight;   vTop.rotate(PI/2.);
	Vec2d vBottom = - vTop;


	// Set up tip path.

	// First, we have to deal with the fact that the tip could
	// starting from any position, so we have to get it to a known
	// location without hitting the tube.

	// Move along a ray from the initial tip position
	// through the tube's center 
	// to a point on a circle surrounding the tube 
	// and centered at the tube center. 
	// The radius of the circle is great enough that the tip can move
	// along the circle without touching the tube.
	Vec2d tipPos = ob[tip].pos;
	Vec2d vPerimeter = normalizeVec(tipPos - vC) * norm(vRightTouch * 1.5);
	double misalignAngle = calcAngleInPlane( vC + vPerimeter, vC, vC + vBottom );

	addPointToTipPath( vC + vPerimeter );

	// move along the perimeter circle in 3 steps
	// to arrive at position (vC + vBottom).
	vPerimeter.rotate( misalignAngle/3. );
	addPointToTipPath( vC + vPerimeter );
	vPerimeter.rotate( misalignAngle/3. );
	addPointToTipPath( vC + vPerimeter );
//	vPerimeter.rotate( misalignAngle/3. );
//	addPointToTipPath( vC + vPerimeter );


	// go to below right side of tube
	addPointToTipPath( vC + vBottom + vPivotArm );

	// push up on right side of tube
	addPointToTipPath( vC + vBottomTouch + vPush + vPivotArm );	

	// go up to above left side, avoiding tube
	addPointToTipPath( vC + vBottom );

	// push up on left side of tube
	addPointToTipPath( vC + vBottomTouch + vPush - vPivotArm );	

	// go to below left side of tube
	addPointToTipPath( vC + vBottom - vPivotArm );
}


/**************************************************************************************/
// calculate the center point, axis vectors, and width vector 
// for the tube in question.  Also touch points on ends and sides.
void
calcTubePointsAndVectors( OB tube, Vec2d* vCenter, Vec2d* vAxis, Vec2d* vWidth,
	Vec2d* vRightClear, Vec2d* vLeftClear, Vec2d* vTopClear, Vec2d* vBottomClear )
{
	*vCenter = tube.pos;
	*vAxis  = Vec2d(1.,0.).rotate( tube.angle ) * (tube.leng/2.);
	*vWidth  = Vec2d(1.,0.).rotate( tube.angle + PI/2. ) * (tube.diam/2.);

	// calc points at which tip is just touching the tube based on 
	// current estimates of tube length and width, and tip radius.
	double tipRadius  = ob[tip].diam/2.;   // XXX peek into sim data for now
	double tubeRadius = tube.diam/2.;
	double touchDist = touchDistance( tipRadius, tubeRadius );

	// points at which tip just touches end of tube
	*vRightClear = normalizeVec(*vAxis) * (tube.leng/2. + touchDist);
	*vLeftClear    = - *vRightClear;

	// points at which tip just touches side of tube
	*vTopClear     = normalizeVec(*vWidth) * touchDist;
	*vBottomClear  = - *vTopClear;
}


/**************************************************************************************/
// Set up tip path to rotate tube around its center.
// Rotates the tube counter-clockwise only (as viewed from +Z direction).
// Does not consider know about tube symmetry in which a tube
// rotated 180 degrees about its center has the same shape.
// The two ends of the tube are considered to be different.
void
moveTipToRotateTube( void )
{
	// Predict where tube being robotically manipulated currently is
	// Use the simulator for this.
	predictedTubePose = ob[currentGoalTube];
		// (for now, just cheat and look in simulator data structure).


	// Measure where tube is using a few linear image scans
	// (not an entire raster scan, just a few scans gathering
	//  height data along selected paths).  
	measuredTubePose = locateTube( predictedTubePose );



	// calc points and vectors needed for rotating tube
	Vec2d vC, vAxis, vWidth, vRightTouch, vLeftTouch, vTopTouch, vBottomTouch;
	calcTubePointsAndVectors( measuredTubePose, &vC, &vAxis, &vWidth,
				&vRightTouch, &vLeftTouch, &vTopTouch, &vBottomTouch );

	// calculate vectors and constants needed
	double turnAngle = PI/6.;	// how much to rotate tube each push
	Vec2d vPivotArm = vAxis * 0.5;
	double pushLength = norm(vPivotArm) * tan(turnAngle);
	Vec2d vPush = normalizeVec(vWidth) * pushLength;

	Vec2d vRight  = vRightTouch  * 1.2;
	Vec2d vLeft   = - vRight;
	Vec2d vTop    = vRight;   vTop.rotate(PI/2.);
	Vec2d vBottom = - vTop;


	// Set up tip path to rotate tube around its center.

	// First, we have to deal with the fact that the tip could
	// starting from any position, so we have to get it to a known
	// location without hitting the tube.

	// Move along a ray from the initial tip position
	// through the tube's center 
	// to a point on a circle surrounding the tube 
	// and centered at the tube center. 
	// The radius of the circle is great enough that the tip can move
	// along the circle without touching the tube.
	Vec2d tipPos = ob[tip].pos;
	Vec2d vPerimeter = normalizeVec(tipPos - vC) * norm(vRightTouch * 1.5);
	double misalignAngle = calcAngleInPlane( vC + vPerimeter, vC, vC + vBottom );

	addPointToTipPath( vC + vPerimeter );

	// move along the perimeter circle in 3 steps
	// to arrive at position (vC + vBottom).
	vPerimeter.rotate( misalignAngle/3. );
	addPointToTipPath( vC + vPerimeter );
	vPerimeter.rotate( misalignAngle/3. );
	addPointToTipPath( vC + vPerimeter );
//	vPerimeter.rotate( misalignAngle/3. );
//	addPointToTipPath( vC + vPerimeter );

	// go to below right side of tube
	addPointToTipPath( vC + vBottom + vPivotArm );

	// push up on right side of tube
	addPointToTipPath( vC + vBottomTouch + vPush + vPivotArm );	

	// go up to above left side, avoiding tube
	addPointToTipPath( vC + vBottom );
	addPointToTipPath( vC + vBottom + vLeft );
	addPointToTipPath( vC + vTop    + vLeft );
	addPointToTipPath( vC + vTop    - vPivotArm );

	// push down on left side of tube
	addPointToTipPath( vC + vTopTouch - vPush    - vPivotArm );	

	// retract tip after pushing
	addPointToTipPath( vC + vTop            );
}


/**************************************************************************************/
// Attempt to measure tube position.
// Do an image image scan over the grid region, creating an depth image.
// Use image analysis to find some number of tubes in the image,
// characterized by parameters of position, orientation, length and width.
// If no tubes are found, stop the robot.
// Otherwise, select a tube and return it. 
OB
locateTube( OB estimatedTubePose )
{
	// Do image scan to get image grid (2D array of Z height values).
	doImageScanExact();

	// Do image anaysis to locate tubes.
	extractFoundTubes();

	// If the image analysis found any tubes, return the first one.
		// For now, don't worry about which is 
		// the right one if there are several.
	if( foundTubeVec.size() >= 1 )	{
		return foundTubeVec[ 0 ];
	}
	else {
		// If no tube is found in the recognition step, the robot stops.  
		robotRunning = 0;
		// Also clear the target points in the planned tip path.
//		clearPathQ();

		// Return goofy pose for visual indication that the reconizer
		// worked but found zero tubes.
		OB goofyPose( Vec2d(64.,64.), PI/4., 0.001, 10. );  // 
		return goofyPose;
	}
			//	return ob[currentGoalTube];
			// (just let the robot peek into sim data structure for now)
}


/**************************************************************************************/
// return point on front of queue
Vec2d
frontPathQ( void )
{
	Assert( !emptyPathQ(), "access to empty pathQ" );
	return pathQ[0];	// element 0 is front of queue
}


/**************************************************************************************/
// add point to tip path
void
addPointToTipPath( Vec2d vPathPoint )
{
	pushBackPathQ( vPathPoint );
}


/**************************************************************************************/
// push point onto back of queue
void
pushBackPathQ( Vec2d vPathPoint )
{
	Assert( pathQSize < MAX_PATHQ, "pathQ overflow" );

	pathQ[ pathQSize ] = vPathPoint;
	pathQSize++;
}


/**************************************************************************************/
// pop point off front of queue
void
popFrontPathQ( void )
{
	Assert( !emptyPathQ(), "attempt to pop empty pathQ" );
	for( int i=0; i<pathQSize-1; i++ ) {
		pathQ[i] = pathQ[i+1];
	}
	pathQSize--;
}


/**************************************************************************************/
// check if queue empty
Bool
emptyPathQ( void )
{
	return pathQSize == 0;	
}


/**************************************************************************************/
// clear the queue
void
clearPathQ( void )
{
	pathQSize = 0;	
}


/**************************************************************************************/
// Move tip toward target point in steps of size "speed".
void
moveTipTowardPoint( Vec2d vTarget )
{
//	double speed = 4.;
	Vec2d vTowardGoal = towardVec( vTarget, ob[tip].pos, tipSpeed );
	Vec2d vTip = ob[tip].pos + vTowardGoal;
	moveTipToXYLoc( vTip.x,  vTip.y, 0. );

}


/**************************************************************************************/
// Calculate velocity vector toward position vToward from position vFrom
// with given speed.
Vec2d
towardVec( Vec2d vTarget, Vec2d vFrom, double speed )
{
	Vec2d vDelta = vTarget - vFrom;
	double magnitude = sqrt( vDelta.x * vDelta.x  +  vDelta.y * vDelta.y );
	Assert( speed >= 0., "speed must be non-negative" );

	speed = min( speed, magnitude );
	return (vDelta / magnitude) * speed;
}


/**************************************************************************************/
void
showBox( OB tube, int color )
{
	// display rectangle indicating goal position for tube
	setColor( color );

	glPushMatrix();

	// put tube at its (x,y) position
	glTranslatef( tube.pos.x, tube.pos.y, 0. );

	// set tube yaw angle (in-plane rotation angle)
	rotateRadiansAxis( tube.angle, 0.0, 0.0, 1.0 ); 

	// draw box at tube position
	double length   = tube.leng;
	double diameter = tube.diam;

#define BOX_BORDER_WIDTH 0.0
	glPushMatrix();
	glScalef( length/2. + diameter/2. + BOX_BORDER_WIDTH, 
		      diameter/2. + BOX_BORDER_WIDTH, 
			  1. );
	drawHollowUnitSquare(); // square centered at origin, width 2
	glPopMatrix();

	glPopMatrix();
}


/**************************************************************************************/
// Display info relevant to robot:
// goals, recognized tubes, planned tip paths, etc.
void
showRobotStuff( void )
{
	// display rectangle indicating goal position for tube
	showBox( currentGoal, RED );

	// Show all tubes found during image analysis.
	if( foundTubeVec.size() == 0 ) {
		// Use "noneFoundPose" for visual indication that the recognizer
		// worked but found zero tubes.
		OB noneFoundPose( Vec2d(64.,64.), PI/4., 0.001, 5. );  // 
		showBox( noneFoundPose, BLACK );
	}
	else {
		// One or more tubes were found.  Show them.
		for( int i=0; i<foundTubeVec.size(); ++i ) {
			showBox( foundTubeVec[ i ], BLACK );
		}
	}

	// display rectangle indicating last measured position of tube
	OB pose = measuredTubePose;
	pose.diam *= 1.1;		// make bigger so it can be seen
	showBox( pose, YELLOW );


	// Display planned tip path:
	// waypoints for current tip movement (pathQ)
	// and lines between them.
	Vec2d vPrevPoint = ob[tip].pos;
	for( int j=0; j<pathQSize; j++ ) {
		drawLine( vPrevPoint, pathQ[j], BLACK );
		vPrevPoint = pathQ[j];
			//double tipRadius = ob[tip].diam/2.;
			//showSphere( pathQ[j], MAGENTA,  tipRadius  );	
	}
}



/**************************************************************************************/
/**************************************************************************************/
// SIMULATION

/**************************************************************************************/
void
simStep(void)
{
	if (simRunning) {		
		// detect collisions and calculate resulting object movements
		// put this calc in here so it can draw labels and graphics as it goes along.  
		collisionStuff();
	}
}


/**************************************************************************************/
void
singleSimStep(void)
{
	if (simRunning == 0 ) {  
		// Advance one simulation step if sim currently halted.
		simRunning = 1;
		drawFrame();
		simRunning = 0;
	}
}


/**************************************************************************************/
void
toggleSim(void)
{
	simRunning = !simRunning;
}


/**************************************************************************************/
// move tip to new position
int
moveTipToXYLoc( double x, double y, double setPoint )
{
	// move tip
	int tip = 0;
	Vec2d vNewTipPosition = Vec2d(x,y);
	ob[tip].pos = vNewTipPosition;

	

	// respond to new tip position
	selectedOb = tip;  // this is need to get simStep to work right
	simStep();



	return 0;
}


/**************************************************************************************/
// set current state to contents of data arg
void
getState( SIM_STATE st )
{
	numObs = st.numObs;
	for( int i=0; i<numObs; i++ ) {
		ob[i] = st.ob[i];
	}
}


/**************************************************************************************/
// copy current state to loc pointed to by data arg.
// example call: setState( &prevState );
void
setState( SIM_STATE* pSt )
{
	pSt->numObs = numObs;
	for( int i=0; i<numObs; i++ ) {
		pSt->ob[i] = ob[i];
	}
}


/**************************************************************************************/
/**************************************************************************************/
// GRAPHICS PRIMITIVES

/**************************************************************************************/
void
showLabeledPoint( Vec2d vPoint, char* labelStr )
{
	glPushMatrix();
	glTranslatef( vPoint.x, vPoint.y, 0. );

	showBitmapString( labelStr );

	glPopMatrix();
}


/**************************************************************************************/
// This routine is a substitute for glRotatef().
// It is the same except it uses radians rather than degrees for angular measure.
void
rotateRadiansAxis( double angleInRadians, 
				   double axisComponentX, double axisComponentY, double axisComponentZ )
{
	glRotatef( angleInRadians * RAD_TO_DEG, 
		       axisComponentX, axisComponentY, axisComponentZ );
}


/**************************************************************************************/
void
setMaterialColor( GLfloat r, GLfloat g, GLfloat b )
{
	if( shadingModel == GL_FLAT ) {
		glColor3f( r, g, b );
	}
	else {
		GLfloat mat_ambient[]    = { 0.2*r, 0.2*g, 0.2*b, 1.0 };
		GLfloat mat_diffuse[]    = { 0.8*r, 0.8*g, 0.8*b, 1.0 };
		GLfloat mat_specular[]   = { 1.0*r, 1.0*g, 1.0*b, 1.0 };
		GLfloat mat_shininess[]  = { 30.0 };

		glMaterialfv( GL_FRONT_AND_BACK,  GL_AMBIENT,  mat_ambient );
		glMaterialfv( GL_FRONT_AND_BACK,  GL_DIFFUSE,  mat_diffuse );
		glMaterialfv( GL_FRONT_AND_BACK,  GL_SPECULAR,  mat_specular );
		glMaterialfv( GL_FRONT_AND_BACK,  GL_SHININESS, mat_shininess );
	}
}


/**************************************************************************************/
void
setColor( int colorIndex )
{
	switch( colorIndex ) {
	case 0: setMaterialColor(1.0, 1.0, 1.0); break;  /* white */
	case 1: setMaterialColor(1.0, 0.0, 0.0); break;  /* red */
	case 2: setMaterialColor(0.0, 1.0, 0.0); break;  /* green */
	case 3: setMaterialColor(0.0, 0.0, 1.0); break;  /* blue */
	case 4: setMaterialColor(1.0, 0.0, 1.0); break;  /* magenta */
	case 5: setMaterialColor(1.0, 1.0, 0.0); break;  /* yellow */
	case 6: setMaterialColor(0.0, 1.0, 1.0); break;  /* cyan */
	case 7: setMaterialColor(0.0, 0.0, 0.0); break;  /* black */
	}
}


/**************************************************************************************/
void
drawCube( double halfWidth )
{
	switch( renderStyle ) {
	default:
	case OB_SOLID:			glutSolidCube( halfWidth );	break;

	case OB_POINTS:		
	case OB_SILHOUETTE:	
	case OB_OUTLINE2D:		
	case OB_WIREFRAME:		glutWireCube(  halfWidth );	break;
	}
}


/**************************************************************************************/
void
drawTorus( double innerDiameter, double outerDiameter )
{
	switch( renderStyle ) {
	default:
	case OB_SOLID:			glutSolidTorus( innerDiameter, outerDiameter, 20, 16);	break;
							// see p659 Woo 3ed
	case OB_POINTS:		
	case OB_SILHOUETTE:	
	case OB_WIREFRAME:		glutWireTorus(  innerDiameter, outerDiameter, 20, 16);	break;

	case OB_OUTLINE2D:		glutWireTorus(  innerDiameter, outerDiameter, 2, 16);	break;
	}
}


/**************************************************************************************/
void
drawSphere( double radius )
{
	static int firstTime = 1;
	static GLUquadricObj* qobj;
	if( firstTime ) { qobj = gluNewQuadric(); }

	switch( renderStyle ) {
	default:
	case OB_SOLID:		drawStyle = GLU_FILL;		goto sphere3D;
	case OB_WIREFRAME:	drawStyle = GLU_LINE;		goto sphere3D;
	case OB_POINTS:		drawStyle = GLU_POINT;		goto sphere3D;
	case OB_SILHOUETTE:	drawStyle = GLU_SILHOUETTE;	goto sphere3D;
	sphere3D:
		gluQuadricDrawStyle( qobj, drawStyle );
		gluQuadricNormals( qobj, GLU_FLAT );
		gluSphere( qobj, radius, 10, 10);
		break;

	case OB_OUTLINE2D:	
		gluQuadricDrawStyle( qobj, GLU_LINE );
		gluQuadricNormals( qobj, GLU_FLAT );
		gluCylinder( qobj, radius, radius, 0.001, 10, 1);  // draw short cylinder
		break;

	case OB_NONE:
		break;
	}
}


/**************************************************************************************/
void
showPoint( Vec2d vPoint, int color, double size /* = 1. */, double zHeight /* = 0. */ )
{
	glPushMatrix();

	glTranslatef( vPoint.x, vPoint.y, zHeight );
	setColor( color );
	drawSphere( 0.1 * size ); 

	glPopMatrix();
}


/**************************************************************************************/
void
showSphere( Vec2d vPoint, int color, double radius )
{
	glPushMatrix();

	glTranslatef( vPoint.x, vPoint.y, 0. );
	setColor( color );
	drawSphere( radius ); 

	glPopMatrix();
}


/**************************************************************************************/
void
drawCone( double radius, double height )
{
	static int firstTime = 1;
	static GLUquadricObj* qobj;
	if( firstTime ) { qobj = gluNewQuadric(); }

	switch( renderStyle ) {
	default:
	case OB_SOLID:		drawStyle = GLU_FILL;		goto cone3D;
	case OB_WIREFRAME:	drawStyle = GLU_LINE;		goto cone3D;
	case OB_POINTS:	drawStyle = GLU_POINT;		goto cone3D;
	case OB_SILHOUETTE:drawStyle = GLU_SILHOUETTE;	goto cone3D;
	cone3D:
		gluQuadricDrawStyle( qobj, drawStyle );
		gluQuadricNormals( qobj, GLU_FLAT );
		gluCylinder( qobj, radius, 0., height, 10, 10); 
		break;

	case OB_OUTLINE2D:	
		gluQuadricDrawStyle( qobj, GLU_LINE );
		gluQuadricNormals( qobj, GLU_FLAT );
		gluCylinder( qobj, radius, radius, 0.001, 10, 1);  // draw short cylinder
		break;

	case OB_NONE:
		break;
	}
}


/**************************************************************************************/
// draw cylinder with given diameter and height.
// The cylinder's axis is on the Z-axis, with one end at the origin
// and the other end on the +Z side.
void
drawCylinder( double diameter, double height )
{
	double radius;
	static int firstTime = 1;
	static GLUquadricObj* qobj;
	if( firstTime ) { qobj = gluNewQuadric(); }

	radius = diameter / 2.;

	switch( renderStyle ) {
	default:
	case OB_SOLID:		drawStyle = GLU_FILL;		goto cylinder3D;
	case OB_WIREFRAME:	drawStyle = GLU_LINE;		goto cylinder3D;
	case OB_POINTS:	drawStyle = GLU_POINT;			goto cylinder3D;
	case OB_SILHOUETTE:drawStyle = GLU_SILHOUETTE;	goto cylinder3D;
	cylinder3D:
		gluQuadricDrawStyle( qobj, drawStyle );
		gluQuadricNormals( qobj, GLU_FLAT );
		gluCylinder( qobj, radius, radius, height, 12, 1);   // see p489 Woo 3ed
				// we want even # of slices (currently 12), 1 stack
		break;

	case OB_OUTLINE2D:	
		gluQuadricDrawStyle( qobj, GLU_LINE );
		gluQuadricNormals( qobj, GLU_FLAT );
		gluCylinder( qobj, radius, radius, height, 2, 1);  // 2 slices, 1 stack = rect
		break;

	case OB_NONE:
		break;
	}

}


/**************************************************************************************/
void
drawUnitSquare( void )
{
	int GLmode;		// see p43 Woo 3rd ed

	switch( renderStyle ) {
	default:
	case OB_SOLID:		GLmode = GL_POLYGON;		break;
	case OB_WIREFRAME:	GLmode = GL_LINE_LOOP;		break;
	case OB_POINTS:		GLmode = GL_POINTS;			break;
	case OB_SILHOUETTE: GLmode = GL_LINE_LOOP;		break;
	case OB_OUTLINE2D:  GLmode = GL_LINE_LOOP;		break;
	}

	glPushMatrix();
//	glScalef( 0.2, 0.2, 0.2 );

	glBegin( GLmode );
	glVertex3f(-1., -1.,  0.);
	glVertex3f( 1., -1.,  0.);
	glVertex3f( 1.,  1.,  0.);
	glVertex3f(-1.,  1.,  0.);
	glEnd();

	glPopMatrix();
}


/**************************************************************************************/
void
drawHollowUnitSquare( void )
{
	glPushMatrix();

	int GLmode = GL_LINE_LOOP;  // see p43 Woo 3rd ed

	glBegin( GLmode );
	glVertex3f(-1., -1.,  0.);
	glVertex3f( 1., -1.,  0.);
	glVertex3f( 1.,  1.,  0.);
	glVertex3f(-1.,  1.,  0.);
	glEnd();

	glPopMatrix();
}


/**************************************************************************************/
// Draw a hollow rectangle with the points v1 and v2 as opposite corners
void
drawHollowRect( Vec2d v1, Vec2d v2 )
{
	glPushMatrix();

	int GLmode = GL_LINE_LOOP;  // see p43 Woo 3rd ed

	glBegin( GLmode );
	glVertex3f( v1.x,  v1.y,  0.);
	glVertex3f( v2.x,  v1.y,  0.);
	glVertex3f( v2.x,  v2.y,  0.);
	glVertex3f( v1.x,  v2.y,  0.);
	glEnd();

	glPopMatrix();
}

//		drawHollowRect( Vec2d( scanXMin, scanYMin ), 
//			      Vec2d( scanXMin + gridSide, scanYMin + gridSide ) );

/**************************************************************************************/
void
drawLine( Vec2d pt1, Vec2d pt2, int color, double z1 /* = 0. */, double z2 /* = 0. */ )
{
	// draw line between the two points
	setColor( color );
	glBegin( GL_LINE_STRIP );  // see p43 Woo 3rd ed
	glVertex3f( pt1.x, pt1.y,  z1 );
	glVertex3f( pt2.x, pt2.y,  z2 );
	glEnd();
}


/**************************************************************************************/
void
drawTip( double tipDiameter )
{
	double coneBaseWidth = tipDiameter * 2.0;
	double coneHeight    = tipDiameter * 3.0;

	// make sure the tip gets drawn, even if object drawing is disabled (OB_NONE)
	int saveRenderStyle = renderStyle;
	if( renderStyle == OB_NONE )	renderStyle = OB_WIREFRAME;

	glPushMatrix();
	{
		// draw sphere at tip center with appropriate radius of curvature
		drawSphere( tipDiameter / 2. ); 

		// also draw cone, if not in 2D display mode
		if( displayConeOn  &&  renderStyle != OB_OUTLINE2D ) {
			// rotate cone to point downward
			rotateRadiansAxis( PI, 0.0, 1.0, 0.0 );

			// translate cone point to center of tip sphere
			glTranslatef( 0., 0., - coneHeight );

			drawCone( coneBaseWidth, coneHeight );
		}
	}
	glPopMatrix();

	// restore rendering mode
	renderStyle = saveRenderStyle;
}


/**************************************************************************************/
void
drawTube( double diameter, double length, int obIndex )
{
	glPushMatrix();

		if( renderStyle == OB_OUTLINE2D ) {
			// draw a little section of 3D cylinder in the middle of tube
			// to show when rolling occurs.
			int saveRenderStyle = renderStyle;
			renderStyle = OB_SILHOUETTE;
		
			// draw cylinder with its axis parallel to X-axis
			double sectionLength = length / 10.;
			glPushMatrix();
			rotateRadiansAxis( PI/2., 0.0, 1.0, 0.0 );       // lay the tube over parallel to X-axis
			glTranslatef( 0., 0., - sectionLength /2. );  // make middle of cylinder the center
			drawCylinder( diameter, sectionLength ); // tube axis starts parallel to Z
			glPopMatrix();
			
			renderStyle = saveRenderStyle;
			
			// undo roll angle around tube axis for 2D view
			// (we just want to see the 2D outline of the tube)
			rotateRadiansAxis( - ob[obIndex].roll,  1.0, 0.0, 0.0 );
		}





		// draw cylinder with its axis parallel to X-axis
		glPushMatrix();
		rotateRadiansAxis( PI/2., 0.0, 1.0, 0.0 ); // lay the tube over parallel to X-axis
		glTranslatef( 0., 0., - length /2. );  // make middle of cylinder the center
		drawCylinder( diameter, length );      // tube axis starts parallel to Z
		glPopMatrix();

		// draw spherical endcap on tube
		glPushMatrix();
		glTranslatef(   length /2., 0., 0. );
		drawSphere( diameter / 2. ); 
		glPopMatrix();

		// draw spherical endcap on tube's other end
		glPushMatrix();
		glTranslatef( - length /2., 0., 0. );
		drawSphere( diameter / 2. ); 
		glPopMatrix();

	glPopMatrix();

}


/**************************************************************************************/
void
lighting( void )
{
	if( shadingModel == GL_FLAT ) {
		glShadeModel(GL_FLAT);
		glDisable( GL_LIGHTING );
	}
	else {
		GLfloat light_ambient[]  = { 0.5, 0.5, 0.5, 1.0 };
		GLfloat light_diffuse[]  = { 1.0, 1.0, 1.0, 1.0 };
		GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
		GLfloat light_position0[] = {  1.0, 1.0, 1.0, 0.0 };
		GLfloat light_position1[] = { -1.0, 1.0, 0.0, 0.0 };
		

		glShadeModel(GL_SMOOTH);

		glLightfv( GL_LIGHT0,  GL_AMBIENT,  light_ambient );
		glLightfv( GL_LIGHT0,  GL_DIFFUSE,  light_diffuse );
		glLightfv( GL_LIGHT0,  GL_SPECULAR, light_specular );
		glLightfv( GL_LIGHT0,  GL_POSITION, light_position0 );

		glLightfv( GL_LIGHT1,  GL_AMBIENT,  light_ambient );
		glLightfv( GL_LIGHT1,  GL_DIFFUSE,  light_diffuse );
		glLightfv( GL_LIGHT1,  GL_SPECULAR, light_specular );
		glLightfv( GL_LIGHT1,  GL_POSITION, light_position1 );

		if( lightOn[0] )  glEnable(  GL_LIGHT0 );
		else              glDisable( GL_LIGHT0 );
		if( lightOn[1] )  glEnable(  GL_LIGHT1 );
		else              glDisable( GL_LIGHT1 );

		glLightModeli( GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE );
		glEnable( GL_LIGHTING );
	}
}


/**************************************************************************************/
/**************************************************************************************/
// UTILITY ROUTINES

/**************************************************************************************/
void
error( char* errMsg )
{
	printf( "\nError: %s\n", errMsg );
	exit( 1 );
}

			//	char str[256];
			//	sprintf( str, "%f", k );
			//	if( streq( str, "NaN" ) ) {
			//		printf( "k: %f\n",                 k );
			//		printf( "vParallel: (%f,%f)\n",    vParallel.x, vParallel.y );
			//		error( "NaN in rollingCollisionResponse" );
			//	}

/**************************************************************************************/
// if the condition is not true, invoke error routine with the message msg.
//	Sample call: Assert( 0, "test" );
void
Assert( int condition, char* msg )
{
	if( ! condition )   error( msg );
}


/**************************************************************************************/
// Test 2 character strings for equality.  
int
streq( char* str1, char* str2 )
{
	// strcmp does lexicographic comparison.  Result=0 means strings are the same.
	return   (strcmp( str1, str2 ) == 0);
}
			//   Code used to test streq()
			//	if( streq( "abc", "abc" ) )  error( "streq works" );
			//	if( streq( "abc", "xyz" ) )  error( "streq doesn't work" );


/**************************************************************************************/
// Return vector interpolated between two given vectors v0 and v1.
// If fraction is zero, return v0; if fraction is one return v1.
// For other fractions, return vectors between v0 and v1.
// This routine also works if fraction < 0 or fraction > 1.
Vec2d
interpolateVecs( Vec2d v0, Vec2d v1, double fraction )
{
	return (v0 * (1. - fraction))  +  (v1 * fraction);
}


/**************************************************************************************/
// Return scalar interpolated between two given scalars s0 and s1.
// If fraction is zero, return s0; if fraction is one return s1.
// For other fractions, return scalars between s0 and s1.
// This routine also works if fraction < 0 or fraction > 1.
double
interpolateScalars( double s0, double s1, double fraction )
{
	return (s0 * (1. - fraction))  +  (s1 * fraction);
}


/**************************************************************************************/
// draw char string with strokes, starting at current origin.
// String is 3D geometric object -- can be scaled, rotated, etc.
void
showStrokeString( char* str )
{
	glPushMatrix();
	double scaleFactor = 1. / 150.;
	glScalef( scaleFactor, scaleFactor, scaleFactor );
	
	for( char* s = str; *s!=0; s++ ) {
		glutStrokeCharacter( GLUT_STROKE_MONO_ROMAN, *s );
	}
	glPopMatrix();


	double textLineSeparation = (- 1.0);
	glTranslatef( 0., textLineSeparation, 0. );
}


/**************************************************************************************/
// draw char string as bitmap, starting at current origin. 
// String has constant size in pixels -- is not 3D object
// See p102 Angel.
void
showBitmapString( char* str )
{
	glRasterPos2f( 0., 0. );
	
	for( char* s = str; *s!=0; s++ ) {
		glutBitmapCharacter( GLUT_BITMAP_8_BY_13, *s );
	}

	double textLineSeparation = (- 0.5);
	glTranslatef( 0., textLineSeparation, 0. );
}


/**************************************************************************************/
double
norm( Vec2d v )
{
	return sqrt( (v.x * v.x)  +  (v.y * v.y) );
}


/**************************************************************************************/
double
vecDistance( Vec2d pt1, Vec2d pt2 )
{
	return norm( pt1 - pt2 );
}


/**************************************************************************************/
// return normalized vector
Vec2d
normalizeVec( Vec2d v )
{
	Vec2d normalizedV = v;

	double normOfV = norm( v );
	if( normOfV != 0. )    normalizedV /= normOfV;

	return normalizedV;
}


/**************************************************************************************/
// normalize argument vector
void
normalize( double* px, double* py, double* pz )
{
	// calc magnitude of vector
	double leng = sqrt( (*px)*(*px) + (*py)*(*py) + (*pz)*(*pz) );

	if( leng == 0 )
		return;

	// divide each vector component by the magnitude.
	*px /= leng;
	*py /= leng;
	*pz /= leng;
}


/**************************************************************************************/
void
crossProduct( double  x1, double  y1, double  z1,
			  double  x2, double  y2, double  z2,
			  double* px, double* py, double* pz )
{
	*px = y1*z2 - z1*y2;
	*py = z1*x2 - x1*z2;
	*pz = x1*y2 - y1*x2;
}


/**************************************************************************************/
/**************************************************************************************/
// TIMING

/**************************************************************************************/
// Each time the timer callback is called, calculate the current frame rate.
void
timerFunc(int value)
{
	static int prevFrameCount;
	double elapsedFrames = (double)(frameCount - prevFrameCount);
	double elaspedTime   = 1.;	// seconds between timer callbacks
	int callbackIntervalInMilliseconds = (int)(elaspedTime * 1000.);

	framesPerSecond = elapsedFrames / elaspedTime;

	// Save the current frame count for next time through this routine.  
	prevFrameCount = frameCount;

	// The timer needs to call itself again some number of milliseconds
	// in the future.  This ensures that the timer is called 
	// at regular intervals.  
	glutTimerFunc( callbackIntervalInMilliseconds, timerFunc, 0 );
}


/**************************************************************************************/
/**************************************************************************************/
// COLLISION DETECTION

/**************************************************************************************/
void
collisionStuff( void )
{
	// Move the selected object only if there is one selected (it can be null).
	if( selectedOb != NULLOB ) {
		
		// if the cursor moved too far, break the movement into smaller steps.
		double jumpMax = 1.0;   // maximum jump distance, per step.

		// Calc previous heldob position and new position.  
		// If the jump is too big (ie it will teleport right past
		// objects it should collide with) then break the jump into
		// several smaller steps.
		Vec2d oldSelectedObPos = prevState.ob[selectedOb].pos;
		Vec2d newSelectedObPos =           ob[selectedOb].pos;

//		showPoint( oldSelectedObPos, MAGENTA, 3. );
//		showPoint( newSelectedObPos, YELLOW,  3. );

		double jumpLength = vecDistance( newSelectedObPos, oldSelectedObPos );
		int truncateJumps = (int)(jumpLength / jumpMax);
		int numSteps;
		if( truncateJumps < 1 )   numSteps = 1;
		else                      numSteps = 1 + truncateJumps;
		Assert( numSteps >= 1, "numSteps: bad value" );


		// Draw a line from old pos to new pos.  
		// Green means it was done in 1 jump; red means it took multiple steps.
		int color = (numSteps > 1) ? CYAN : YELLOW;
		drawLine( oldSelectedObPos, newSelectedObPos, color );


		// move from old position to new position in numStep jumps
		for( int i=1; i<=numSteps; i++ ) {
			// move the selectedOb to successive positions along the
			// line from the old pos to new pos.  
			double frac = ((double)i) / ((double)numSteps);
			ob[selectedOb].pos = 
				interpolateVecs( oldSelectedObPos, newSelectedObPos, frac );


			showPoint( ob[selectedOb].pos, MAGENTA, 3. );


			// Start the recursive movement propagation with the
			// object (selectedOb) being moved by the user with the mouse.
			markAllTubesMovable();
			propagateMovements( selectedOb );
		}
	}


#if 0
	// if any tubes are intersecting, we need to move them apart
//	for( int obj = numObs-1;  obj >= 0;  obj-- ) {
	for( int obj=0; obj<numObs; obj++ ) {
		if( ob[obj].type==UNUSED )   continue;

		// For each object, if other obs are colliding with it, move
		// the colliding ob (and propagate secondary pushes, etc.
		// (We are not actually moving the object, we are just testing
		// it for collisions, and then propagating the movements
		// those collisions generate.)
		// This is not guarranteed to eliminate all collisions, but it will help.
		markAllTubesMovable();
		propagateMovements( obj );
	}
#endif


	// save the sim state for next pass through this routine
	setState( &prevState );
}


/**************************************************************************************/
// calc the difference between 2 angles in range (-PI,PI],  
// forcing result into same range.
double
centerAngle( double angle )
{
	// With input angles in the range (-PI,PI], the difference
	// can sometimes get outside this range, and must be adjusted
	// back into range (-PI,PI].  
	if(      (-PI) < angle  &&  angle <= PI )			;
	else if( angle <= (-PI) )							angle += 2*PI;
	else if(                    PI < angle     )		angle -= 2*PI;

	return angle;
}


/**************************************************************************************/
// calculate the angle in radians formed by line segments AB and BC.
// This calculation assumes 2D vectors in the XY plane 
// (and will not work for 3D vectors.)
double
calcAngleInPlane( Vec2d vA, Vec2d vB, Vec2d vC )
{
	Vec2d vAB = vA - vB;
	Vec2d vCB = vC - vB;

	double angleA = atan2( vAB.y, vAB.x );
	double angleC = atan2( vCB.y, vCB.x );

	// because atan2 returns angles in the range (-PI,PI], the difference
	// can sometimes get outside this range, and must be adjusted
	// back into range (-PI,PI].  
	return centerAngle( angleC - angleA );
}


/**************************************************************************************/
// Use equation from Falvo et al Nature paper (21 Jan 99)
// to calculate the sliding pivot point S = s * Axis
// from the location of the push point M = k * Axis.
// The normalized length variables x1' and x0' from the paper are here
// renamed k and s, respectively.  
// This equation is valid for 1/2 <= k <= 1.  
double
pivotPointEquation( double k )
{
	return   k - sqrt( k*k - k + 0.5 );
}


/**************************************************************************************/
Vec2d
PivotPoint( double normalizedPushPoint, Vec2d vEndA, Vec2d vEndB  )
{
	double k = normalizedPushPoint;
	double s;

	// use equation from Falvo et al Nature paper (21 Jan 99)
	// to calculate the sliding pivot point S = s * Axis.
	// This equation is valid for 1/2 <= k <= 1, so 
	// we reflect around k = 1/2 for values of k between 0 and 1/2.
	if(      0.5 <= k  &&  k <= 1.0 ) {
		s = pivotPointEquation( k );
	}
	else if( 0.0 <= k  &&  k <  0.5 ) {
		k = 1. - k;
		s = pivotPointEquation( k );
		s = 1. - s;
	}
	else {
		error( "PivotPoint: k not normalized." );
	}

	// calculate the location of point S, the pivot point, and return it.
	Vec2d vStemp = ((vEndA - vEndB) * s) + vEndB;
	return vStemp;
}


/**************************************************************************************/
// Project vector P-B onto A-B,
// returning the ratio k of the projection vector to the length of A-B.
// Thus the projection vector = (A-B) * k + B.
double
nearestPointOnLineToPoint( Vec2d vA, Vec2d vB, Vec2d vP )
{
	Vec2d vAB = vA - vB;
	double dotProd = dotProduct( vAB, vP - vB );
	double lengthOfABVector = norm( vAB );

	double k;
	if( lengthOfABVector == 0. ) {
		// Avoid divide-by-zero in the case that A-B has zero length.
		// In this case, the value of k doesn't matter (we set k to zero), 
		// since it is going to be multiplied by a vector of length zero.  
		k = 0.;
	}
	else {
		// This is the normal case, for vectors of non-zero length.
		k = dotProd / (lengthOfABVector * lengthOfABVector);
	}

	// return the point on line AB nearest to point P.
	// (If A and B were the same, return that point.)
	//	Return k, for use in calculating vM = ((vA-vB) * k) + vB;
	return k;
}


/**************************************************************************************/
void
collisionDraw( SPHERE_AND_TUBE_DATA cxTest )
{
	if( linesOn ) {
		glPushMatrix();

		// draw tube axis
		drawLine( cxTest.vA, cxTest.vB, BLUE );

		// Draw line from tip center (P) to nearest point on tube axis (M)
		// if P is within certain distance of tube.
		double tooFarAwayToDraw = 1000.;
		int color;
		if( cxTest.dist <= tooFarAwayToDraw ) {
			if( cxTest.dist <= 0. )   color = RED;	  // collision
			else					  color = GREEN;
			drawLine( cxTest.vM, cxTest.vP, color );
		}

		glPopMatrix();
	}

//	// draw text labels for points
//	if( displayCollisionsOn ) {
//		// draw labeled points
//		showLabeledPoint( cxTest.vC, "C" );
//		showLabeledPoint( cxTest.vP, "P" );
//		showLabeledPoint( cxTest.vA, "A" );
//		showLabeledPoint( cxTest.vB, "B" );
//		showLabeledPoint( cxTest.vM, "M" );
//		showLabeledPoint( cxTest.vS, "S" );
//	}
}


/**************************************************************************************/
void
lineSegmentCollisionDraw( LINE_SEGMENTS_INTERSECTION_TEST lineSegCx )
{
	// draw line between nearest points
	drawLine( lineSegCx.vN1, lineSegCx.vN2, YELLOW );
	showPoint( lineSegCx.vN1, YELLOW );
	showPoint( lineSegCx.vN2, CYAN );
	

	int color = lineSegCx.intersectFlag ? RED : WHITE;
	showPoint( lineSegCx.vI, color, 2. );



//	showPoint( lineSegCx.vI,  BLUE );
//	showPoint( lineSegCx.vI1, CYAN );
//	showPoint( lineSegCx.vI2, CYAN );

}


/**************************************************************************************/
// Return 1 if value is in the closed interval bounded by bound1 and bound2.
// The interval is not empty.  The bounds are swapped if necessary
// to define a non-empty interval.
int
betweenInclusive( double value,   double bound1, double bound2 )
{
	// make sure that bound2 >= bound1
	if( bound2 < bound1 ) {
		// swap bound1 and bound2
		double temp = bound1;
		bound1     = bound2;
		bound2     = temp;
	}

	// Test for the value being within the interval.  
	// Being on the boundary counts as being in the interval.
	int betweenFlag = (bound1 <= value  &&
		               value  <= bound2);
	
	return betweenFlag;
}


/**************************************************************************************/
// Translate and rotate the input point with an angle-preserving
// transform that puts D at the origin, and E on the Y-axis.
// and return the transformed point.
Vec2d
xformDEtoYaxis( Vec2d vIn, Vec2d vD, Vec2d vE )
{
	// calc parameters for xforms derived from positions of D and E.
	double lengthDE = norm( vD - vE );
	double angleE = atan2( (vE-vD).y, (vE-vD).x );	// angle wrt +X axis
	
	// start with input point
	Vec2d vOut = vIn;

	// Translate so that D is at origin.
	vOut -= vD;

	// Rotate to put E on Y-axis
	vOut.rotate( - angleE + PI/2. );

	// Return the transformed point.
	return vOut;
}


/**************************************************************************************/
// Perform the inverse transform of:
// Translate and rotate the input point with an angle-preserving
// transformation that puts D at the origin, and E on the Y-axis.
Vec2d
inverseXformDEtoYaxis( Vec2d vIn, Vec2d vD, Vec2d vE )
{
	// calc parameters for xforms derived from positions of D and E.
	double lengthDE = norm( vD - vE );
	double angleE = atan2( (vE-vD).y, (vE-vD).x );	// angle wrt +X axis
	
	// start with input point
	Vec2d vOut = vIn;

	// Rotate E off the Y-axis
	vOut.rotate( + angleE - PI/2. );

	// Translate D from the origin back to its original position.
	vOut += vD;

	// Return the transformed point.
	return vOut;
}


/**************************************************************************************/
// Calc the distance between centers for which two spheres touch.
// The centers are projected onto the XY plane, and the distance
// is measured in 2D.
double
touchDistance( double radius1, double radius2 )
{
			//	return radius1 + radius2;  // This works for pure 2D, but not 2.5D

	// Put two spheres of different radii resting on the
	// XY plane and make them touch.  Project their centers
	// onto the XY plane and measure the distance.
	// Return this distance.  

		// This distance = 2 * sqrt(r1 * r2) is derived as follows.
		// The larger sphere of radius r2 is centered on the Y-axis at (0,r2)
		// and is therefore tangent to the X-axis at the origin.  
		// We move the center of the smaller sphere of radius r1 along 
		// the line y=r1 (so the small sphere touches the X-axis)
		// until it also touches the big sphere.  This happens when
		// the center of the smaller sphere is at distance r1+r2
		// from the big sphere's center, ie, on the curve
		// y = r1 - sqrt( (r1+r2)^2 - x^2 ).  
		// Intersecting these two curves gives 
		// r1 = r2 - sqrt( (r1+r2)^2 - x^2 ) 
		// which can be solved for x, giving x = 2 * sqrt(r1 *r 2),
		// which is the distance between the projection of the centers 
		// onto the X-axis.  
	return   2. * sqrt( radius1 * radius2 );   // works
}


/**************************************************************************************/
// Measure the distance between a point P and a line segment AB.
// Interpret the point as the center of a sphere, and
// the line segment as the axis of a cylinder with hemispherical endcaps,
// and use the radii of the sphere and cylinder to calculate
// whether the two objects are touching, and if so, the depth of penetration.
// The calculated distance between the two bodies has this meaning:
// D > 0 means the objects are not touching each other;
// D = 0 means just touching at boundaries;
// D < 0 means they are interpenetrating to a depth (-D).
// Return a struct holding the collision data, including various points.

SPHERE_AND_TUBE_DATA
distSphereToCylinder( Vec2d vP, Vec2d vA, Vec2d vB, 
	                  double radiusSphere, double radiusCylinder )
{
	SPHERE_AND_TUBE_DATA cxTest;
	
	// copy radii of sphere and cylinder into struct.
	cxTest.sphereRadius = radiusSphere;
	cxTest.tubeRadius   = radiusCylinder;

	// copy input points into struct.
	cxTest.vP = vP;	// point P = center of sphere being tested for touching tube
	cxTest.vA = vA;	// endpoint A of tube axis
	cxTest.vB = vB;	// endpoint B of tube axis

	// Now calculate some points.
	// Center of the tube.
	cxTest.vC    = (vA + vB) / 2.;

	// Vector (P - B) runs from tube endpoint B to tip center P.
	// Calculate projection (M-B) of (P-B) onto tube axis (A-B) to 
	// find the nearest point M to P on the axis line AB.  
	// M will be on line AB but may not be on line segment AB.  
	// The coefficient k tells where point M is on the axis line.
	// 0<=k<=1 means point M is on  line segment AB.  
	double k = nearestPointOnLineToPoint( vA, vB, vP );  // line AB, point P
	cxTest.k = k;

	// If M is off either end of the line segment, force it back to an endpoint.
	// This gives M as the nearest point on *line-segment* AB to point P.
	// (Restating, calc the projection MB of line seg PB onto line AB, but with 
	// M set to an endpoint of AB if the point M is not within the line seg AB.)
	if(      0. < k  &&  k < 1. )   ;		//vM = vB + (vA - vB) * k);
	else if( k <= 0. )              k = 0.;	//vM = vB;
	else  /* 1. <= k */             k = 1.;	//vM = vA;

	// Calculate point M from coefficient k.
	cxTest.vM    = ((vA - vB) * k) + vB;

	// calculate the pivot point S for sliding mode from push point M.  
	cxTest.vS    = PivotPoint( k, vA, vB );

	// Since M is the nearest point on line segment AB to P,
	// the distance between M and P is also the distance between
	// P and line segment AB, the tube axis.
	double distTipCenterToTubeAxis = vecDistance( cxTest.vP, cxTest.vM );

	// Subtract the distance (in XY plane only) when touching 
	// (distance between sphere center and tube axis)
	// to get the distance from sphere to tube.  
	// Here is how this distance D is interpreted:
	// D > 0 means the objects are not touching each other;
	// D = 0 means just touching at boundaries;
	// D < 0 means they are interpenetrating to a depth (-D).
	double touchDist = touchDistance( radiusSphere, radiusCylinder );
	cxTest.dist = distTipCenterToTubeAxis - touchDist;
		//	cxTest.dist = distTipCenterToTubeAxis - radiusSphere - radiusCylinder;


	// draw stuff related to collision testing
	collisionDraw( cxTest );


	// return the struct containing all the collision data just calculated.
	return cxTest;
}


/**************************************************************************************/
// Calculates the distance between two line segments DE and FG in the plane,
// and calculates the nearest points on the 2 segments, N1 on DE and N2 on FG.
// If the line segments intersect, calculates the intersection point I.
LINE_SEGMENTS_INTERSECTION_TEST
distBetweenLineSegments( Vec2d vD, Vec2d vE, Vec2d vF, Vec2d vG )
{
	LINE_SEGMENTS_INTERSECTION_TEST lineSegCx;

	// put endpoints of line segments DE and FG into the data structure
	lineSegCx.vD = vD;
	lineSegCx.vE = vE;
	lineSegCx.vF = vF;
	lineSegCx.vG = vG;

	// Make sure all data structure fields have a value.
	lineSegCx.dist = 0.;
	lineSegCx.whichCase = 1;
	lineSegCx.parallelFlag = 0;
	lineSegCx.intersectFlag = 0;
	lineSegCx.overlapFlag = 0;
	lineSegCx.vN1 = Vec2d( 0.,0.);
	lineSegCx.vN2 = Vec2d( 0.,0.);
	lineSegCx.vI =  Vec2d( 0.,0.);
	lineSegCx.vI1 = Vec2d( 0.,0.);
	lineSegCx.vI2 = Vec2d( 0.,0.);


	// Translate, rotate, and scale the points D, E, F, and G together
	// so that D is at the origin, and E is at (0,1) on the Y-axis.
	// In this canonical orientation with DE on the Y-axis, it will be 
	// easier to figure out if FG is parallel to DE (Is F.x == G.x?),
	// and, if not, to calculate the intersection of lines DE and FG
	// (the Y-intercept of line FG).
	Vec2d vDD = xformDEtoYaxis( vD,   vD, vE );
	Vec2d vEE = xformDEtoYaxis( vE,   vD, vE );
	Vec2d vFF = xformDEtoYaxis( vF,   vD, vE );
	Vec2d vGG = xformDEtoYaxis( vG,   vD, vE );
				//	showPoint( vDD, YELLOW );
				//	showPoint( vEE, YELLOW );
				//	showPoint( vFF, CYAN );
				//	showPoint( vGG, CYAN );

	// Is line segment FG parallel to DE?  Since DE is on the Y-axis,
	// This is the same as asking: Is FG vertical?
	if( vFF.x == vGG.x ) {      
		// Yes, FG is parallel to DE.
		// But not for long.
				//lineSegCx.parallelFlag = 1;

		// (If point F = point G, the slope is indeterminate, but this
		// is also handled by the tweak below.
				//if( vFF.y == vGG.y ) {}

		// Just tweak one of the endpoints, so that the parallelism
		// is broken.  Then the code for non-parallel line segments
		// will handle this data just fine.  
		// A small error, on the order of the tweak size "epsilon",
		// will be introduced into the position and distance calculations.  
		// This will not be significant for the CNT simulation.
		double epsilon = 0.0001;
		Assert( vFF.x + epsilon  !=  vFF.x, 
				"epsilon too small: failed to break parallelism" );
		vFF.x += epsilon;
	}



//	else {

	// No, FG is not parallel to DE.
	lineSegCx.parallelFlag = 0;
		
	// calculate the Y-intercept of line FG, which is the
	// intersection point I of lines FG and DE.  We know there
	// is an intersection since FG and DE are not parallel.
	double slopeFG = (vFF.y - vGG.y) / (vFF.x - vGG.x);
	double yIntercept = vFF.y - vFF.x * slopeFG;
	Vec2d vII = Vec2d( 0., yIntercept );

	// Do the inverse transform to get point I back into the original
	// coord sys.
	// Write intersection point I into the data structure.
	Vec2d vI = inverseXformDEtoYaxis( vII,   vD, vE );
	lineSegCx.vI  = vI;
	lineSegCx.vI1 = vI;
	lineSegCx.vI2 = vI;

	// Calculate whether the two line segments intersect by
	// figuring out whether the point I falls on both
	// line segment DE and line segment FG.
	int IonDEflag = betweenInclusive(vII.y,    vDD.y, vEE.y );
	int IonFGflag = betweenInclusive(vII.x,    vFF.x, vGG.x );
	lineSegCx.intersectFlag = (IonDEflag && IonFGflag);
				// int color = lineSegCx.intersectFlag ? RED : WHITE;
				// showPoint( lineSegCx.vI, color );
				// showPoint( vII, color );


	// Distinguish between the two cases with non-parallel line segments.
	if( lineSegCx.intersectFlag == 0 ) {
		// Case 1: line segs non-parallel, disjoint.
		lineSegCx.whichCase = 1;

		// NEED TO FIGURE THE NEAREST POINTS HERE
		// To test the 2 line segments DE and FG for bumping into one another,
		// there are 4 point-vs-line tests to do:
		//     D vs FG
		//     E vs FG
		//     F vs DE
		//     G vs DE
		// Test each of the four endpoints against the other line segment
		// to calculate a distance from point to line segment.
		// find distance of point D to line segment FG.
		// find distance of point E to line segment FG.
		// find distance of point F to line segment DE.
		// find distance of point G to line segment DE.
		// Note that cxTestN.dist = dist( cxTestN.vP, cxTestN.vM )
		SPHERE_AND_TUBE_DATA cxTest1 = distSphereToCylinder( vD,   vF,vG, 0., 0. );			
		SPHERE_AND_TUBE_DATA cxTest2 = distSphereToCylinder( vE,   vF,vG, 0., 0. );
		SPHERE_AND_TUBE_DATA cxTest3 = distSphereToCylinder( vF,   vD,vE, 0., 0. );
		SPHERE_AND_TUBE_DATA cxTest4 = distSphereToCylinder( vG,   vD,vE, 0., 0. );

		// find the minimum of the four distances:
		// cxTest1.dist, cxTest2.dist, cxTest3.dist, cxTest4.dist.
		// This will determine the distance between the line segments
		// and also give the nearest points N1 and N2 on each line segment.
		SPHERE_AND_TUBE_DATA cxTestNearest12 = cxTest1;
		if( cxTest2.dist <= cxTestNearest12.dist ) cxTestNearest12 = cxTest2;

		SPHERE_AND_TUBE_DATA cxTestNearest34 = cxTest3;
		if( cxTest4.dist <= cxTestNearest34.dist ) cxTestNearest34 = cxTest4;

		// N1 is supposed to be on DE and N2 on FG
		if( cxTestNearest12.dist <= cxTestNearest34.dist ) {
			// in this case, P was either D or E in the test.
			// M is the nearest point to P on line seg FG.
			lineSegCx.vN1  = cxTestNearest12.vP;
			lineSegCx.vN2  = cxTestNearest12.vM;
			lineSegCx.dist = cxTestNearest12.dist;
		}
		else {
			// In this case, P was either F or G in the test.
			// M is the nearest point to P on line seg DE.
			lineSegCx.vN1  = cxTestNearest34.vM;
			lineSegCx.vN2  = cxTestNearest34.vP;
			lineSegCx.dist = cxTestNearest34.dist;
		}
	}
	else {
		// Case 2: line segs non-parallel, intersecting.
		lineSegCx.whichCase = 2;
		lineSegCx.dist = 0.;
		lineSegCx.vN1 = lineSegCx.vI;
		lineSegCx.vN2 = lineSegCx.vI;
	}

//	}



//	lineSegmentCollisionDraw( lineSegCx );


	return lineSegCx;
}


/**************************************************************************************/
/**************************************************************************************/
// COLLISION RESPONSE

/**************************************************************************************/
// Calculate how far to move tube to get just beyond the pushing sphere.
Vec2d
translateJustOutOfContact( SPHERE_AND_TUBE_DATA cxTest )
{
	// Calc vector from push point to nearest-point-on-axis.
	// This is the direction to move.
	Vec2d vMP = (cxTest.vM - cxTest.vP);
	Vec2d vDirection = vMP / norm(vMP);

	// The penetration depth is the distance to move to take the tube
	// out of contact with the pushing sphere.
	double penetrationDepth = - cxTest.dist;
	Vec2d vStep = vDirection * penetrationDepth;
//	vStep *= 1.01;     // get it a little beyond contact

	return vStep;
}


/**************************************************************************************/
// translate the tube away from the tip with no rotation of the tube.
void
translateCollisionResponse(  Vec2d vPushPt, int tube, SPHERE_AND_TUBE_DATA cxTest  )
{
	// Translate the tube away from pushing sphere center.
	Vec2d vStep = translateJustOutOfContact( cxTest );
	ob[tube].pos += vStep;

	// mark the tube as moved to allow it to move other tubes
	ob[tube].moved = 1;
}


/**************************************************************************************/
// Decompose vVector into components normal and parallel to axis vector.
// The two computed vectors always sum to the input vVector.  
// Sample call: calcComponentVectorsRelativeToAxis(vStep, vAxis, &vNormal, &vParallel);
void
calcComponentVectorsRelativeToAxis( Vec2d vVector, Vec2d vAxis, 
								    Vec2d* pvNormal, Vec2d* pvParallel )
{
	// Calculate the components of vVector that are parallel and normal to the axis.
	// Specifically, calculate projection of vVector onto the vAxis.
	// k is coefficient telling where projected endpoint is on axis.
	Vec2d vOrigin(0.,0.);
	double k = nearestPointOnLineToPoint( vAxis, vOrigin,    vVector );  
										// line AB, point B + vVector
	*pvParallel = vAxis * k;
	*pvNormal   = vVector - *pvParallel;
}


/**************************************************************************************/
// Calculate the endpoints of the given tube.
// Sample call: calcTubeEndpoints( pushedTube, &vA, &vB );
void
calcTubeEndpoints( int tube, Vec2d* pvEndA, Vec2d* pvEndB )
{
	// calculate tube endpoints A and B.
	double radiusTube  = ob[tube].diam / 2.;
	Vec2d vCenterTube = ob[tube].pos;
	double lengthTube  = ob[tube].leng;
	double angleTube   = ob[tube].angle;

	// calc tube axis vector
	Vec2d vXUnit(1.,0.);
	Vec2d vAxisTube   = vXUnit.rotate( angleTube ) * lengthTube;

	// calc tube endpoints
	*pvEndA = vCenterTube + (vAxisTube / 2.);
	*pvEndB = vCenterTube - (vAxisTube / 2.);
}




#if 0
		Vec2d vDiff = vStep - (vAxialStep + vNormalStep);
		printf( "vStep:        %7.4f, %7.4f\n",  vStep.x,       vStep.y ); 
		printf( "vAxialStep:   %7.4f, %7.4f\n",  vAxialStep.x,  vAxialStep.y ); 
		printf( "vNormalStep:  %7.4f, %7.4f\n",  vNormalStep.x, vNormalStep.y ); 
		printf( "vDiff:        %7.4f, %7.4f\n",  vDiff.x,       vDiff.y ); 
		printf( "\n" ); 
#endif

#if 0
	// Move tube by axial component of penetration vector
	// and recompute sphere-tube intersection data for new tube position.
	ob[tube].pos += vAxialStep;

	Vec2d vA;
	Vec2d vB;
	calcTubeEndpoints( tube, &vA, &vB );
	Vec2d vP = cxTest1.vP;
	double sphereRadius1 = cxTest1.sphereRadius;
	double tubeRadius1 = cxTest1.tubeRadius;
	cxTest = distSphereToCylinder( vP, vA, vB, sphereRadius1, tubeRadius1 );


//	return;
#endif


/**************************************************************************************/
// slide the tube across the surface with no rolling, and rotating the
// tube axis at the pivot point given by the equation from the Nature paper.
void
slidingCollisionResponse( Vec2d vPushPt, int tube, SPHERE_AND_TUBE_DATA cxTest )
{
	// mark the tube as moved to allow it to move other tubes
	ob[tube].moved = 1;

	// Determine whether this is a segment of a flexible tube
	// or an independent, single rigid tube.
	int isRigidTube = ( ob[tube].nextSeg == NULLOB  &&
						ob[tube].prevSeg == NULLOB     );
	int isFlexibleTube = ! isRigidTube;
	


	// Case for flexible tube segment.
	if ( isFlexibleTube ) {
		// Translate tube outward along sphere-center P to tube-center C vector
		// Move a distance equal to penetration depth.
		// Rotate tube to be tangent to sphere boundary
		// (normal to ray coming from sphere center).
		repelTubeRadiallyFromPoint( vPushPt, tube, cxTest );
			//showPoint( ob[tube].pos,        GREEN, 20. );
	}


	// Cases for rigid tube.
	// Check whether the push sphere is pushing the side of the tube or the endcap.
	// The parameter k is between 0 and 1 when the sphere pushes the side of the tube.
	else if( cxTest.k < 0.  ||  cxTest.k > 1. )  {		
		// In this case, the push sphere (centered at P) is pushing 
		// on the tube's endcap (centered at M).

		// The collision response is:
		// First, move the whole tube by the axial component of the
		// penetration vector vStep.  Then use the component of the penetration
		// vector normal to the tube axis to move the endpoint M sideways,
		// but leaving the pivot point S stationary.  
		// From the new positions of S and M, calculate the new position
		// of C (ie the tube's position) and the new yaw angle
		// of the tube (yaw angle of M-C).  

		// Calc the translation vector needed to move the endcap away 
		// from the pushing sphere (this is the penetration vector).
		// Decompose vStep into components normal and parallel to tube axis 
		// (both components are in the XY plane).
		Vec2d vStep = translateJustOutOfContact( cxTest );
		Vec2d vAxis = cxTest.vA - cxTest.vB;
		Vec2d vNormalStep;
		Vec2d vAxialStep;
		calcComponentVectorsRelativeToAxis( vStep, vAxis, &vNormalStep, &vAxialStep );

		// Move points M (endpoint) and S (pivot point)
		Vec2d vM = cxTest.vM;
		vM       += vAxialStep;
		vM       += vNormalStep;

		Vec2d vS = cxTest.vS;
		vS       += vAxialStep;

		// Use known values of k for various points along tube axis
		// to compute the new position of C.  
		double kM = 1.0;		// endpoint of axis
		double kC = 0.5;		// midpoint of axis
		double kS = 1. - (sqrt( 2. ) / 2.);	// approx .2929 (not 1/3)
							// given by equation in Nature paper
		double ratioOfLengths = (kC - kS) / (kM - kS);  // exactly same as kS
		Vec2d vC = vS + (vM - vS) * ratioOfLengths;

		// Calculate new orientation of tube resulting from push.
		double tubeAngle = atan2( (vM-vC).y, (vM-vC).x );
				//showPoint( ob[tube].pos, YELLOW );
				//showPoint( vM, GREEN );
				//showPoint( vS, GREEN );
				//showPoint( vC, GREEN );

		// update tube position and yaw angle with new values resulting
		// from this push on tube's endcap.
		ob[tube].pos   = vC;
		ob[tube].angle = tubeAngle;
	}

	else {  /* cxTest.k >= 0.  &&  cxTest.k <= 1. */
		// In this case, the push sphere (centered at P) is pushing 
		// on the tube's cylindrical wall (M is nearest point to P on 
		// the tube's axis).
		// The preferred collision response is to pivot the tube around
		// pivot point S enough so that there is no penetration
		// between push sphere and tube.
		// However, in some situation (eg, very short tubes, or large push
		// spheres), it is not possible to get the tube out of contact
		// with the push sphere by any rotation around S.  In this case,
		// we move the pushed tube radially away from the push sphere center
		// and orient it tangential to the push sphere.

		double sphereRadius = cxTest.sphereRadius;
		double tubeRadius   = cxTest.tubeRadius;

		// This ratio is positive, since the numerator and denominator are positive.
		double touchDist = touchDistance( sphereRadius, tubeRadius );
		double distSphereToPivot = norm( cxTest.vS - cxTest.vP );
		double ratio = touchDist / distSphereToPivot;
		Assert( ratio >= 0., "slidingCollisionResponse: ratio negative" );

		if( ratio > 1. ) {
			// In this case, no rotation around S can get the tube out of contact
			// with the sphere, so translation is required. 
			
			// Translate tube outward along sphere-center P 
			// to tube-center C vector.
			// Move a distance equal to penetration depth.
			repelTubeRadiallyFromPoint( vPushPt, tube, cxTest );			
		}
		else {      // 0 <= ratio <= 1
			// Figure angle change needed to rotate tube far enough away from
			// point P so that there is no more collision.  

			// Calculate the PSM angle needed to move point M to a
			// distance (sphereRadius + tubeRadius) from point P.
			// We have 3 points to start with:
			// S = the pivot point which should remain fixed in position
			// P = the center of the pushing sphere, and
			// M = the nearest point on the tube axis to the pushing sphere.
			// We want to rotate the tube away from the sphere, and get it out of contact.
			
			// calculate angle PSM in range (-PI,PI].
			// This produces an angle which, if the tube is rotated around S,
			// the tube will turn away from the colliding point P.
			double currentPSMangle = calcAngleInPlane( cxTest.vP, cxTest.vS, cxTest.vM );
			// So use sign of current angle for direction to turn tube.
			double sign = (currentPSMangle >= 0.) ? 1. : -1.;

			double newPSMangle;
			newPSMangle = asin( ratio );

			double angleChangeMagnitude = fabs(newPSMangle) - fabs(currentPSMangle);
			double turningAngle = sign * angleChangeMagnitude;

			// rotate tube away from tip (rotate around Z, leaving tube still on XY plane)
			// (This moves point S relative to tube center C.)
			ob[tube].angle += turningAngle;

			// Calculate the (S - C) vector for the old and new orientations of the tube,
			// and get the difference, which is the movement of S caused by the rotation.
			Vec2d vOldSC = (cxTest.vS - cxTest.vC);
			Vec2d vNewSC = (cxTest.vS - cxTest.vC).rotate(turningAngle);
			Vec2d vMovementOfS = vNewSC - vOldSC;
			
			// translate tube to keep S in same place
			ob[tube].pos -= vMovementOfS;
		}
	}

	// display sliding mode pivot point when actual pivot rotations occur
	if( displayPivotPointsOn ) {
		showPoint( cxTest.vS, YELLOW, 5. );
	}
}


/**************************************************************************************/
// Move the tube center away from the push point.  
// Move a distance equal to the penetration depth.  
// Orient the tube perpendicular to the movement vector
// (tangent to a sphere around the push point).
void
repelTubeRadiallyFromPoint( Vec2d vPushPt, int tube, 
						    SPHERE_AND_TUBE_DATA cxTest )
{
	// Translate tube outward along sphere-center P to tube-center C vector
	// Move a distance equal to penetration depth.
	Vec2d vCP    = cxTest.vM - cxTest.vP;    // M not C now
	Vec2d vCPdir = vCP / norm( vCP );
	double penetrationDepth = - cxTest.dist;
	Vec2d vOutwardStep   = vCPdir * penetrationDepth;
	ob[tube].pos += vOutwardStep;
		
//	// Rotate tube to be tangent to sphere boundary
//	// (normal to ray coming from sphere center).
//	Vec2d vSphereTangent = vCPdir; vSphereTangent.rotate( PI/2. );
//	double tubeAngle = atan2( vSphereTangent.y, vSphereTangent.x );
//	ob[tube].angle = tubeAngle;
}


/**************************************************************************************/
// Translate and/or rotate the tube in response to a collision with the tip.
// (ie, the tip is pushing the tube).
void
collisionResponse( Vec2d vPushPt, double radiusSphere, int pushedTube )
{
	// calculate the collision data for sphere and tube
	SPHERE_AND_TUBE_DATA cxTest = 
			calcCxData( vPushPt, radiusSphere, pushedTube );

	// Collision response depends on sim type.
	// Pass collision data into collision response routine.
	switch( simType ) {
//	case SIM_NONE:		break;
//	case SIM_TRANSLATE:	translateCollisionResponse( vPushPt, pushedTube, cxTest );	break;
//	case SIM_ROLLING:	rollingCollisionResponse(   vPushPt, pushedTube, cxTest );	break;
	case SIM_SLIDING:	slidingCollisionResponse(   vPushPt, pushedTube, cxTest );	break;
//	case SIM_ROLLING_SLIDING: 
//		           rollingSlidingCollisionResponse( vPushPt, pushedTube, cxTest ); break;
	default:  error( "unknown simType" );
	}
}


/**************************************************************************************/
SPHERE_AND_TUBE_DATA
calcCxData( Vec2d vPushPt, double radiusSphere, int pushedTube )
{
	// calculate pushed tube endpoints A and B, and tube's radius.
	Vec2d vA;
	Vec2d vB;
	calcTubeEndpoints( pushedTube, &vA, &vB );
	double radiusCylinder = ob[pushedTube].diam / 2.;

	// calculate the collision data for sphere and tube
	SPHERE_AND_TUBE_DATA cxTest;
	cxTest = distSphereToCylinder( vPushPt, vA, vB, 
	                      radiusSphere, radiusCylinder );
	return cxTest;
}


/**************************************************************************************/
void
crossedTubeAxisCollisionResponse(  LINE_SEGMENTS_INTERSECTION_TEST lineSegCx,
	int pushingTube, int pushedTube  )
{
	// find nearest point Q to I on line seg DE.
	double lengthID = vecDistance( lineSegCx.vI, lineSegCx.vD );
	double lengthIE = vecDistance( lineSegCx.vI, lineSegCx.vE );
	Vec2d vQ = (lengthID <= lengthIE) ? lineSegCx.vD : lineSegCx.vE;

	// move pushed tube so that axes are no longer crossed
	// This puts axis FG on top of the point Q (which is an endpoint of DE).
	Vec2d vStepUncrossTubes = vQ - lineSegCx.vI;
	ob[pushedTube].pos += vStepUncrossTubes;


	// Now move tube FG in the direction normal to its axis
	// for a distance equal to the sum of the tube radii.

	// Calculate unit normal to line seg FG (pushed tube's axis)
	Vec2d vFGAxis = lineSegCx.vF - lineSegCx.vG;
	Vec2d vFGAxisNormal = vFGAxis / norm(vFGAxis);
	vFGAxisNormal.rotate( PI/2. );

	// Make sure we move the pushed tube away from the axis DE.
	// The dot product of (Q-I) with a normal to the axis FG tells us 
	// whether we are moving away from or towards DE's axis.
	// There are 2 normals to choose between and we want the
	// right one -- the one that moves us away from DE.  
	// The dot product is proportional to the cosine of the angle between
	// the normal and (Q-I).  The cosine is >= 0 if the angle is in the
	// interval [-PI/2,PI/2].  
	// If the normal we picked failed the test, use the other normal.
	// All these vectors are in the XY-plane.  
	double dotProd = dotProduct( vFGAxisNormal, vStepUncrossTubes );
	double sign = (dotProd >= 0.) ? 1. : -1.;
	vFGAxisNormal *= sign;

	// calc tube radii
	double radiusPushingTube  = ob[pushingTube ].diam / 2.;
	double radiusPushedTube  = ob[pushedTube].diam / 2.;

	// Move the pushed tube.  The two tubes should be just touching now.  
	ob[pushedTube].pos += vFGAxisNormal * (radiusPushingTube + radiusPushedTube);


	// mark the tube as moved to allow it to move other tubes
	ob[pushedTube].moved = 1;
}


/**************************************************************************************/
// Test 2 tubes for collision with one another, with one tube designated
// as pusher and the other as pushed.
// When a collision is detected, respond by moving the pushed tube away
// from the point of contact.  
void
TubeTubeCollisionTestAndResponse( int pushingTube, int pushedTube )
{
	// if these two tube segments are linked, then they are not
	// considered to be colliding, even though they overlap.
	if( ob[pushingTube].nextSeg == pushedTube  )   return;
	if( ob[pushingTube].prevSeg == pushedTube  )   return;
	if( ob[pushedTube ].nextSeg == pushingTube )   return;
	if( ob[pushedTube ].prevSeg == pushingTube )   return;
	
	
	
	// Define endpoints D and E and radius of the pushing tube.
	Vec2d vD;
	Vec2d vE;
	calcTubeEndpoints( pushingTube, &vD, &vE );
	double radiusPushingTube  = ob[pushingTube ].diam / 2.;

	// Define endpoints F and G and radius of the pushed tube.
	Vec2d vF;
	Vec2d vG;
	calcTubeEndpoints( pushedTube, &vF, &vG );
	double radiusPushedTube  = ob[pushedTube].diam / 2.;

	// test for intersection between two tubes
	LINE_SEGMENTS_INTERSECTION_TEST lineSegCx = 
			distBetweenLineSegments( vD, vE,    vF, vG );

	// Calc location of point that tubes touch (or point on pushing
	// tube that penetrates most deeply into pushed tube).
	double r1 = radiusPushingTube;   // ie DE, which contains N1
	double r2 = radiusPushedTube;    // ie FG, which contains N2
	double fraction = r1 / (r1 + r2);
	Vec2d vTouchPoint = interpolateVecs( lineSegCx.vN1, lineSegCx.vN2, fraction );

	// calc z height of collision point
	double zPushingTube = r1;
	double zPushedTube  = r2;
	double zTouchPoint  = interpolateScalars( zPushingTube, zPushedTube, fraction );

	// Calc distance between tubes, taking radii into account.
	// Negative distance is penetration.
	double distTubeTube = lineSegCx.dist 
						 - touchDistance( radiusPushingTube, radiusPushedTube );
	Vec2d vPushPoint = lineSegCx.vN1;	// this point is on line seg DE


	// Test for collision between tubes.
	// If there is a collision, response by moving pushed tube.  
	if( lineSegCx.intersectFlag ) {
		// the axes of the two tubes are intersecting: so
		// move the pushed tube off the pushing tube.
		if( displayCollisionsOn ) {
			showPoint( vTouchPoint, BLUE, 5. );
		}

		crossedTubeAxisCollisionResponse( lineSegCx, pushingTube, pushedTube );
	}
	else if( distTubeTube <= 0. ) {
		// This is a collision between an endcap of a tube and either
		// the side of the other tube or the endcap of the other tube.
		if( displayCollisionsOn ) {
			showPoint( vTouchPoint, RED, 5., zTouchPoint );
					//	int color = (lineSegCx.parallelFlag == 1) ? YELLOW : RED;
		}

		// move pushed tube in response to collision at push point by pushing tube.
		collisionResponse( vPushPoint, radiusPushingTube, pushedTube );
	}
}


/**************************************************************************************/
/**************************************************************************************/
// PROPAGATE PUSHES

/**************************************************************************************/
void
markAllTubesMovable( void )
{
	// mark all tubes as movable to start the sim step.
	for( int i=0; i<numObs; i++ ) {
		if( ob[i].type==UNUSED )   continue;

		ob[i].moved = 0;
	}

	// Mark the tip as not movable.
	ob[tip].moved = 1;
}


/**************************************************************************************/
void
propagateMovements( int movedTube )
{
	// mark this tube as having moved, and so not movable the rest of this sim step.
	ob[movedTube].moved = 1;

	// after moving a segment of a flexible multi-segment tube,
	// first reposition the linked segments.
	positionSegmentedTube( movedTube );


	// To propagate movement to other tubes, the tube which moved must
	// be tested for collision with all other tubes.
	// However, to guarantee termination of this recursion, each object
	// can only be moved one time per simulation step.
	for( int i=0; i<numObs; i++ ) {
		// skip deleted tubes or segments
		if( ob[i].type==UNUSED )   continue;

		// skip objects which have already been moved 
		// (and are therefore marked as not movable).
		if( ob[i].moved )   continue;




		// Make the nearest segment to movedTube the tube segment to move.
		int nearestSeg = findNearestSegmentToPoint( 
				ob[ movedTube ].pos, i );



		// test for movedTube colliding with object i.
		TubeTubeCollisionTestAndResponse( movedTube, nearestSeg );

		// if object i moved as a result of this collision test,
		// then propagate pushes to all remaining unmoved objects.
		// (Only objects that collide with the pusher object get moved.)
		if( ob[ nearestSeg ].moved ) {
			propagateMovements( nearestSeg );
		}

	}
}


/**************************************************************************************/
// For all the segments linked to segment "tube", find
// the one (nearestSeg) closest to the push point "vPushPt".
// Limitation: uses center-to-center distance, not tube-tube dist.
int
findNearestSegmentToPoint( Vec2d vPushPt, int tube )
{
	int seg;
	int nearestSeg = tube;
	double nearestDistPushPtToSeg = norm( vPushPt - ob[tube].pos );

	// walk both directions (nextSeg and prevSeg) along the
	// linked tube segments, if this is a flexible tube.
	// If it is a single rigid tube, the links are null
	// and the input segment is returned as output.
	double distPushPtToSeg;
	for( seg=tube; seg!=NULLOB; seg=ob[seg].nextSeg ) {
		distPushPtToSeg = norm( vPushPt - ob[seg].pos );
		if( distPushPtToSeg < nearestDistPushPtToSeg ) {
			nearestSeg = seg;
			nearestDistPushPtToSeg = distPushPtToSeg;
		}
	}
	for( seg=tube; seg!=NULLOB; seg=ob[seg].prevSeg ) {
		distPushPtToSeg = norm( vPushPt - ob[seg].pos );
		if( distPushPtToSeg < nearestDistPushPtToSeg ) {
			nearestSeg = seg;
			nearestDistPushPtToSeg = distPushPtToSeg;
		}
	}

	// Return the nearest segment to push point.
	return nearestSeg;
}


/**************************************************************************************/
/**************************************************************************************/
// OTHER SIMS AND DEFORMABLE OBJECTS

/**************************************************************************************/
// roll tube along the surface without changing the angle of the tube axis.
void
rollingCollisionResponse(  Vec2d vPushPt, int tube, SPHERE_AND_TUBE_DATA cxTest  )
{
	// Translate the tube away from pushing sphere center.
	Vec2d vStep = translateJustOutOfContact( cxTest );
	ob[tube].pos += vStep;

	// Decompose step into components normal and parallel to tube axis.
	Vec2d vAxis = cxTest.vA - cxTest.vB;
	Vec2d vNormal;
	Vec2d vParallel;
	calcComponentVectorsRelativeToAxis( vStep, vAxis, &vNormal, &vParallel );

	// Figure out how much to roll tube, using only the component normal to tube axis.
	double normalLengthMoved = norm( vNormal );
	double tubeRadius = ob[tube].diam / 2.;
	double rollAngleChangeInRadians = normalLengthMoved / tubeRadius;

	// Figure out sign of roll rotation by testing which side of the axis
	// (M-P) is on.  
	// The dot product of (M-P) with a normal to the axis tells us this,
	// since it is proportional to the cosine of the angle between
	// the normal and (M-P).  The cosine is >= 0 if the angle is in the
	// interval [-PI/2,PI/2].  
	// All these vectors are in the XY-plane.  
	Vec2d vAxisNormal = (cxTest.vA - cxTest.vB).rotate( - PI/2. );
	double dotProd2 = dotProduct( vAxisNormal, vStep );
	double sign = (dotProd2 >= 0.) ? 1. : -1.;

	// Rotate tube around its axis as it rolls along the substrate.
	ob[tube].roll += sign * rollAngleChangeInRadians;

	// mark the tube as moved to allow it to move other tubes
	ob[tube].moved = 1;
}


/**************************************************************************************/
// Do combined rolling and sliding simulation.
void
rollingSlidingCollisionResponse( Vec2d vPushPt, int tube, SPHERE_AND_TUBE_DATA cxTest )
{
	// Set fixed parameters governing locking into an orientation.  
	double angleBetweenSymmetryLines = PI/3.;
	double lockThresholdAngle = PI/36.;   // (+ or -) PI/36 = 5 degrees

	// Calculate the angle between the tube and the substrate.
	double substrateAngle = 0.; // orientation of graphite substrate's crystal symmetry lines
	double tubeAngleOnSubstrate = centerAngle( ob[tube].angle - substrateAngle );

	// Scale the angle so that symmetry angles are integers. 
	double scaledAngle = tubeAngleOnSubstrate / angleBetweenSymmetryLines;

	// Separate into integer (which symmetry line) and 
	// fractional (position between symmetry lines) parts.
	// Use rounding, not truncation, to get to nearest integer.
	// The fractional part will be in the interval [-1/2,1/2].
	double integerPart    = floor( scaledAngle + 0.5 );
	double fractionalPart = scaledAngle - integerPart;

	// Scale the threshold angle and test for the current tube angle
	// being within threshold of a symmetry line.
	double normalizedThreshold = lockThresholdAngle / angleBetweenSymmetryLines;
	if(     fractionalPart <=   normalizedThreshold  &&
            fractionalPart >= - normalizedThreshold )    {

		// The tube is (almost) lined up with a symmetry line, so
		// snap the tube into alignment with the symmetry line.
		// (The tube will stay aligned unless something knocks it loose.)
		ob[tube].angle = integerPart * angleBetweenSymmetryLines;

		// Since the tube is aligned, respond to push with rolling movement.
		rollingCollisionResponse( vPushPt, tube, cxTest );
	}
	else {
		// The tube is not aligned, so respond to push with sliding movement.
		slidingCollisionResponse( vPushPt, tube, cxTest );
	}
}


/**************************************************************************************/
void
showSubstrateLines( void )
{
	if( simType == SIM_ROLLING_SLIDING ) {
		glPushMatrix();

		// Set orientation of substrate's graphite crystal symmetry lines
		Vec2d vSymmetry(1.,0.);
		double substrateAngle = 0.; // orientation of graphite substrate's crystal symmetry lines
		vSymmetry.rotate( substrateAngle );
		double symmetryLineLength = 18.;
		vSymmetry *= symmetryLineLength / 2.;

		// draw symmetry lines every 120 degrees, centered at origin.
		drawLine( vSymmetry, - vSymmetry, CYAN );
		rotateRadiansAxis( PI/3., 0.0, 0.0, 1.0 );
		drawLine( vSymmetry, - vSymmetry, CYAN );
		rotateRadiansAxis( PI/3., 0.0, 0.0, 1.0 );
		drawLine( vSymmetry, - vSymmetry, CYAN );

		glPopMatrix();
	}
}


/**************************************************************************************/
// Update the position and orientation of segment curSeg, which is
// linked to lastSeg (by either a nextSeg or prevSeg link in the
// doubly linked list).  This is a flexible coupling where the
// two segments join, and this code implements a rope-like 
// following behavior for the dragged segment curSeg.
void
positionSegment( int curSeg, int lastSeg, int directionFlag )
{
	//Calc max bend angle between segments from the min allowed
	// radius of curvature and the segment length.
	double segmentLength = ob[lastSeg ].leng;
	double maxBendAngle = atan2( segmentLength, minRadiusOfCurvature );
			//	double maxBendAngle = PI/8.;
	
	// calculate last segment's endpoints A and B and center C.
	Vec2d vCLast = ob[lastSeg ].pos;
	Vec2d vALast;
	Vec2d vBLast;
	calcTubeEndpoints( lastSeg, &vALast, &vBLast );
	// pick endpoint Q according to which direction (next or prev)
	// we are walking through segments.
	Vec2d vQ  = (directionFlag == 1) ? vALast : vBLast;

	// Get the center on the current segment (before it has moved).
	// Also calc endpoints A and B and 
	Vec2d vC = ob[curSeg ].pos;
	Vec2d vA;
	Vec2d vB;
	calcTubeEndpoints( curSeg, &vA, &vB );
	Vec2d vQ2 = (directionFlag == 1) ? vA : vB;

	//                 A       C       B
	// O=======*=======O=======*=======O
	// Alast   Clast   Blast
	// ----->          Q               Q2

	// Calculate a direction from the endpoint Q to the point Q2. (was C)
	// This defines a unit vector emanating from endpoint Q.




	Vec2d vDir = (vC - vQ) / norm(vC - vQ);   // old way curSeg pivoted in middle (C)
//	Vec2d vDir = (vQ2 - vQ) / norm(vQ2 - vQ); // new way: pivot curSeg around far end
		// Both of these work. 



	// Calc the zero-curvature (straight out of last seg) direction.
	Vec2d vDirLast = (vCLast - vQ) / norm(vCLast - vQ);
	Vec2d vDirStraight = - vDirLast;

	// Find (cosine of) bending angle between segments (with no bend = 0 angle).
	double dotProd = dotProduct( vDirStraight, vDir );
	double eps = 1e-6;
	Assert( dotProd >= -1.-eps  &&  dotProd <= 1.+eps, "dot product of unit vectors too big" );



	// The  cosine decreases with increasing angle, so calc the min cosine allowed.
	double minDotProd = cos( maxBendAngle );

	// if the bending angle is too large, adjust the direction to curSeg's center
	int constraintActiveFlag = (minRadiusOfCurvature > 0.  &&   // =0 means no constraint
		                        dotProd <= minDotProd);
	if( constraintActiveFlag ) {

		// Which side of vDirStraight is vDir on?
		// The dot product is proportional to the cosine of the angle between
		// the two vectors.  The cosine is >= 0 if the angle is in the
		// interval [-PI/2,PI/2].  
		Vec2d vDirNormal = vDirStraight; vDirNormal.rotate( PI/2. ); // rot +90 degrees
		double dotProd2 = dotProduct( vDirNormal, vDir );
		Assert( dotProd2 >= -1.-eps  &&  dotProd2 <= 1.+eps, 
					"dotProd2: dot product of unit vectors too big" );
		double sign = (dotProd2 >= 0.) ? 1. : -1.;  // 1 = to right of straight vector

		// set vDir to max bend angle from lastSeg
		double deltaAngle = maxBendAngle * sign;
		Vec2d vDirNew = vDirStraight;
		vDirNew.rotate( deltaAngle );
		vDir = vDirNew;
				//drawLine( vQ, vQ + vDirNew*10.,      BLUE );	
				//drawLine( vQ, vQ + vDirStraight*10., GREEN );	
	}


	// Put the curSeg's center on this ray at the appropriate distance.
	// (This is the shortest distance the segment center could move
	// and still stay connected to lastSeg.)
	double halfLengthCurSeg = ob[curSeg ].leng / 2.;
	Vec2d vCNew = vQ + (vDir * halfLengthCurSeg);
	ob[curSeg].pos = vCNew;

	// Calculate orientation of curSeg segment based on vDir direction vector.
	// Flip the orientation by PI radians, depending on which 
	// direction (next or prev) we are walking through segments.  
	double newAngleCurSeg;
	if( vDir.x==0. && vDir.y==0. )  newAngleCurSeg = 0.;
	else                            newAngleCurSeg = atan2( vDir.y, vDir.x );

	newAngleCurSeg += ((directionFlag==1) ? 0. : PI );
	ob[curSeg].angle = newAngleCurSeg;

	
	// If the curvature constraint affected the bend angle, draw the angle.
	// Create slightly more accepting constraint test for display purposes,
	// so that bend at threshold will be displayed.
	double eps2 = 1e-6;
	int constraintActiveFlag2 = (minRadiusOfCurvature > 0.  &&   // =0 means no constraint
		                        dotProd <= minDotProd + eps2);
	if( displayBendLimitOn  &&  constraintActiveFlag2 ) {
		drawLine( vQ, vCLast,   RED );	
		drawLine( vQ, vCNew,    RED );	
	} 
			// Debugging aid: use blue/green to show next/prev-based updates.
			//	if( directionFlag == 1 ) {showPoint( ob[curSeg].pos, BLUE, 3. );}
			//	else                     {showPoint( ob[curSeg].pos, GREEN, 3. );}
}


/**************************************************************************************/
// position all the segments of a segmented tube relative to the given segment,
// using the segment lengths and bend angles.
void
positionSegmentedTube( int movedSegment )
{
	// walk the doubly linked segment list in the forward (nextSeg) direction
	// from the starting segment.
	int curSeg = movedSegment;
	int lastSeg;
	for(    lastSeg = curSeg, curSeg = ob[curSeg].nextSeg;  
	        curSeg != NULLOB;  
	        lastSeg = curSeg, curSeg = ob[curSeg].nextSeg ) {
		positionSegment( curSeg, lastSeg, 1 ); // 1=forward direction (next not prev)

		// mark this segment as having moved, and so not movable the rest 
		// of this sim step.
		ob[curSeg].moved = 1;
	}

	// walk the doubly linked segment list in the backwards (prevSeg) direction
	// from the starting segment.
	curSeg = movedSegment;
	for(    lastSeg = curSeg, curSeg = ob[curSeg].prevSeg;  
	        curSeg != NULLOB;  
	        lastSeg = curSeg, curSeg = ob[curSeg].prevSeg ) {
		positionSegment( curSeg, lastSeg, 0 ); // 1=forward direction (next not prev)

		// mark this segment as having moved, and so not movable the rest 
		// of this sim step.
		ob[curSeg].moved = 1;
	}

	return;
}


/**************************************************************************************/
// create a multi-segmented flexible tube.  
int
newSegmentedTube( int numSegments, 
				  double segmentLength, double segmentWidth, Vec2d vEndpoint  )
{
	Assert( numSegments >= 1, "numSegments must be >= 1" );
	if( numObs + numSegments >= MAXOB ) {
		printf( "No obs left.\n" );
		return NULLOB;
	}
	
	// create a bunch of segments.
	int firstSeg = numObs;
	int lastSeg  = numObs + numSegments - 1;

	for( int i=0; i<numSegments; i++ ) {
		int seg = numObs;

		Vec2d vOffset = Vec2d(segmentLength,0.) * i;
		addObject( seg, TUBE, vEndpoint + vOffset, 
					0., 0., segmentLength, segmentWidth,   
					NULLOB, NULLOB );

		// link segments together.
		ob[seg].prevSeg = (seg > firstSeg) ? seg-1 : NULLOB;
		ob[seg].nextSeg = (seg < lastSeg ) ? seg+1 : NULLOB;
	}

	return firstSeg;
}


/**************************************************************************************/
/**************************************************************************************/
// IMAGE SCAN
// Simulate an imaging scan by AFM tip over a rectangular grid region.
// This produces an image (a depth image): a 2D array of Z-heights.
// Two methods are supported for constructing the image data:
// 1. Routine doImageScanExact  (slow, exact method).  For each point in
//        the grid, find the lowest point the tip can take without
//        penetrating any tubes (or the substrate).  This is a triply-nested
//        loop that slows down with increasing grid resolution and
//        increasing tube count.
// 2. Routine doImageScanApprox (fast, approximate method).  Use 3D graphics
//        hardware to speed up the image computation.  Since the tip is
//        modeled as a sphere and the tube as an SSL (sphere swept along a line),
//        the "solvent-accessible surface" for the tip is just a
//        larger SSL (Minkowski sum of tip's sphere and tube's SSL).
//        So render each tube into graphics frame buffer as enlarged SSL.
//        Pixel dimensions of frame buf are set up to match grid resolution.
//        Use Z-buffer to get the top (nearest) surface at each sample point.
//        (We only care about the pixel's Z-value and ignore its color value.)
//        This method uses the graphics hardware to calculate a bunch of pixels
//        in parallel and with optimized hardware.  It also wastes no effort
//        generating pixels far from the tube.  On the negative side, it
//        uses a polygonal approximation to the true SSL surface.   .
// An image scan may be invoked in several ways:
// 1. By a keyboard command.
// 2. By the robot when it needs to get an image for finding tubes.
// 3. By "auto-image mode (controlled by flag "autoImageScan"), which
//        performs an image scan over the grid region every frame,
//        so that the depth image is constantly updated as tubes move around.


/**************************************************************************************/
// This is the callback for rendering into the depth buffer window,
// as required by "doImageScanApprox".  
void 
displayFuncDepth( void ) 
{

	if( imageScanType == IMAGE_SCAN_APPROX) {
		if( autoImageScan ) {
			// Do image scan to construct depth image over image grid
			// in Z-buffer of Depth Window using graphics hardware.
			doImageScanApprox();
			// Do image anaysis to locate tubes within grid.
			extractFoundTubes();
		}
		else {
			// Depth window is not being used, so clear it.
			clearFrameBufferDepthRender();
		}
	}
	else {  // imageScanType == IMAGE_SCAN_EXACT
		// Call the code for exact image scan here to show how
		// IMAGE_SCAN_EXACT and IMAGE_SCAN_APPROX are alternatives.
		// Calling it here instead of in the main graphics loop
		// introduces a one-frame delay between imaging and tube recognition.

		// Depth window is not being used, so clear it.
		clearFrameBufferDepthRender();

		if( autoImageScan ) {
			// Do image scan to construct depth image over image grid
			// using geometric calculation, not graphics hardware.
			doImageScanExact();
			// Do image anaysis to locate tubes within grid.
			extractFoundTubes();
		}
	}
}


/**************************************************************************************/
// Do an image scan of the currently selected type.
void
doImageScan( void )
{
	if( imageScanType == IMAGE_SCAN_EXACT ) {
		doImageScanExact();
	}
	else if( imageScanType == IMAGE_SCAN_APPROX ) {
		doImageScanApprox();
	}
	else error( "unknown value of imageScanType" );
}


/**************************************************************************************/
// Simulate an imaging scan by AFM tip over a rectangular grid region.
// This produces an image (a depth image): a 2D array of Z-heights.
// (Two methods are supported for constructing the image data.)
// This one is:
// 1. Routine doImageScanExact  (slow, exact method).  For each point in
//        the grid, find the lowest point the tip can take without
//        penetrating any tubes (or the substrate).  This is a triply-nested
//        loop that slows down with increasing grid resolution and
//        increasing tube count.
void 
doImageScanExact( void ) 
{
	int i;
	int j;

	// Calc grid height at each grid point.
	// ie, do an image scan.
	for( i=0; i<scanResolution; i++ ) {
		for( j=0; j<scanResolution; j++ ) {
			double x = i * scanStep  +  scanXMin;
			double y = j * scanStep  +  scanYMin;

			double* pz = &(zHeight[i][j]);
			getImageHeightAtXYLoc( x, y, pz );
		}
	}
}


/**************************************************************************************/
// For any (x,y) position that the AFM tip could take (not necessarily within 
// the grid or at a grid sample point), calculate the lowest Z-height
// of the tip such that it does not penetrate any tubes (or the substrate).
// This is the value the AFM tip would return when imaging at this point (x,y).

// Since the tip is modeled as a sphere and each tube as an SSL 
// (sphere swept along a line), the "solvent-accessible surface" for the tip 
// with respect to one tube
// is just a larger SSL (Minkowski sum of tip's sphere and tube's SSL).
// So calculate the Z-value of this larger SSL at (x,y).
// Since the tube is assumed to lie flat on the (planar) substrate,
// this is done by first calculating a tip-to-tube distance in the XY plane.
// The SSL's cross-section is circular in the vertical plane containing
// the tip center and the SSL's nearest point.  So the equation of a circle
// (solve z^2 + d^2 = r^2 for z, giving z = sqrt( r^2 - d^2) ) 
// can be used to look up the Z-value of the SSL for a particular tip position.
// This works if the tip was within a certain distance of the SSL;
// if not, then the tip would contact the substrate, rather than the tube,
// so use the substrate's Z-height.
// The tip could possibly touch multiple tubes at a given tip position (x,y),
// so use the maximum Z-value over all tubes (and the substrate) to
// guarrantee that the tip penetrates none of them.  This maximum Z
// will be the tip Z-position at which it touches, but does not penetrate,
// the combined surface of all the tubes and the substrate.
int
getImageHeightAtXYLoc( double x, double y, double* z )
{
	int tip = 0;
	double tipRadius = ob[tip].diam/2.;
	double zSubstrate = 0.;	// z-height of substrate
	
	// Calculate a z-height at this imaging point for each tube
	// and return the maximum z-value calculated.  
	// The least z-value which can be returned is the tip height 
	double maxZ = zSubstrate;
	for( int tube=0; tube<numObs; tube++ ) {
		if( ob[tube].type==UNUSED )   continue;

		// the tip itself does not contribute to the image, so skip it.
		if(tube == tip)   continue;

		// Start with the point at which we want a height measurement.
		Vec2d vP = Vec2d(x,y);

		// calculate pushed tube endpoints A and B, and tube's radius.
		Vec2d vA;
		Vec2d vB;
		calcTubeEndpoints( tube, &vA, &vB );
		double radiusCylinder = ob[tube].diam / 2.;

		// calculate distance d from imaging point P to tube axis
		SPHERE_AND_TUBE_DATA cxTest;
		cxTest = distSphereToCylinder( vP, vA, vB, 
							  0., 0. );
		Vec2d vM = cxTest.vM;
		double d = vecDistance( vM, vP );

		// measure height of tube of radius r as seen by tip of radius R
		// at image point P.
		double r = radiusCylinder;
		double R = tipRadius;
		double z = (d < r+R) ? r + sqrt( (r+R)*(r+R) - d*d ) : zSubstrate;

		// displace in Z so that substrate with no tubes on it is always 
		// at zero height, and tops of tubes match scan surface.
		z -= tipRadius;

		// We want the maximum of all the heights computed for point P.
		if( z > maxZ )   maxZ = z;
	}

	// return the maximum z-height for all tubes.
	*z = maxZ;

	return 0;
}


/**************************************************************************************/
// Simulate an imaging scan by AFM tip over a rectangular grid region.
// This produces an image (a depth image): a 2D array of Z-heights.
// (Two methods are supported for constructing the image data.)
// This one is:
// 2. Routine doImageScanApprox (fast, approximate method).  Use 3D graphics
//        hardware to speed up the image computation.  Since the tip is
//        modeled as a sphere and the tube as an SSL (sphere swept along a line),
//        the "solvent-accessible surface" for the tip is just a
//        larger SSL (Minkowski sum of tip's sphere and tube's SSL).
//        So render each tube into graphics frame buffer as enlarged SSL.
//        Pixel dimensions of frame buf are set up to match grid resolution.
//        Use Z-buffer to get the top (nearest) surface at each sample point.
//        (We only care about the pixel's Z-value and ignore its color value.)
//        This method uses the graphics hardware to calculate a bunch of pixels
//        in parallel and with optimized hardware.  It also wastes no effort
//        generating pixels far from the tube.  On the negative side, it
//        uses a polygonal approximation to the true SSL surface.   .
void 
doImageScanApprox( void ) 
{
	int i;
	int j;

	// Render tube images (enlarged to account for tip radius)
	// into window.  
	// (We don't really care about the image, just the depth.)
	imageScanDepthRender();

	// Read (normalized) Z-buffer values from the depth window.
	// Scale them back to correct Z-values and use as 
	// Z-heights in image scan grid.  
			// static double zBuffer[ 128*128 ];
	void* zBufferPtr = &(zBuffer[0]);
	int pixelGridSize = 64;		// must match window size
			// width and height the same for now
	glReadPixels( 0, 0, pixelGridSize, pixelGridSize, GL_DEPTH_COMPONENT, GL_FLOAT, zBufferPtr );
	for( i=0; i<scanResolution; i++ ) {
		for( j=0; j<scanResolution; j++ ) {
			double zNormalized = zBuffer[ j*pixelGridSize + i ];
			double zDepth = scanFar + zNormalized * (scanNear - scanFar);
			zHeight[i][j] = zDepth;
		}
	}
}


/**************************************************************************************/
// Clear graphics and Z-buffer in the depth window.
void 
clearFrameBufferDepthRender( void ) 
{
	glutSetWindow( depthWindowID );
	glClearColor(0.5, 0.5, 0.5, 0.0);   // gray background
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glutSwapBuffers();
}


/**************************************************************************************/
// display graphics in the depth window.
void 
imageScanDepthRender( void ) 
{
	// draw into depth window
	glutSetWindow( depthWindowID );

	// Setup OpenGL state.
	glClearDepth(1.0);
	glClearColor(0.5, 0.5, 0.5, 0.0);   // gray background
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glEnable(GL_DEPTH_TEST);

	// set projection matrix to orthoscopic projection matching current window
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(  scanXMin,   scanXMin + (scanStep * scanResolution),
		      scanYMin,   scanYMin + (scanStep * scanResolution),
		      scanNear,   scanFar   );

	// set modeling matrix to identity
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// set drawing parameters
	int saveRenderStyle = renderStyle;
	renderStyle = OB_SOLID;
	shadingModel = GL_SMOOTH;
	lighting();

	// draw in substrate over scan region at constant z-height.
	double substrateHeight = 0.;
	double scanXMax = scanXMin + (scanStep * scanResolution);
	double scanYMax = scanYMin + (scanStep * scanResolution);
	setColor( GREEN );
	glBegin(GL_POLYGON);
	glNormal3f( 0., 0., -1. );
	glVertex3f( scanXMin, scanYMin, substrateHeight );
	glVertex3f( scanXMin, scanYMax, substrateHeight );
	glVertex3f( scanXMax, scanYMax, substrateHeight );
	glVertex3f( scanXMax, scanYMin, substrateHeight );
	glEnd();

	//	Draw tubes with radii increased by tip radius.
	int tip = 0;
	double tipRadius = ob[tip].diam/2.;
	setColor( WHITE );
	for( int i=0; i<numObs; i++ ) {
		if( ob[i].type==UNUSED )   continue;

		glPushMatrix();

		// put tube at its (x,y) position
		glTranslatef( ob[i].pos.x, ob[i].pos.y, 0. );
		// put tube flat on XY plane (ie, with its lower surface touching XY plane)
		glTranslatef( 0., 0., ob[i].diam/2. );  
		// set tube yaw angle (in-plane rotation angle)
		rotateRadiansAxis( ob[i].angle, 0.0, 0.0, 1.0 ); 
		// set roll angle around tube axis
		rotateRadiansAxis( ob[i].roll,  1.0, 0.0, 0.0 );

		switch( ob[i].type ) {
		double radius;
		case TUBE:		
			// Lower surface to match real surface height for ridges and plains.
			glTranslatef( 0., 0., -tipRadius );  

			// Draw SSL with radius of tube plus radius of tip.
			// This surface is the "solvent-accessible surface" for
			// a "solvent" sphere with the radius of the tip (for this tube).
			// The topmost of all these surfaces for all tubes is
			// the image scan -- the farthest the tip can be lowered
			// until it touches something for each (x,y) grid point.  
			radius = ob[i].diam/2. + tipRadius;
			drawTube( radius * 2., ob[i].leng, i );
			break;

		case TIP:		
			break;

		default: error( "attempt to draw unknown object type" );
		}

		glPopMatrix();
	}

	// end of display frame, so flip buffers
	glutSwapBuffers();

	renderStyle = saveRenderStyle;
}


/**************************************************************************************/
// Display the image scan grid (a depth image).
// There are several different ways of displaying the grid:
// wireframe, solid surface, solid with holes, 
// or just a box to show how far the grid extends.
void
showGrid( void )
{
	int gridColor = GREEN;

	if(      gridStyle == GRID_NONE ) {
		// Draw a hollow box around the image scan grid region.
		setColor( gridColor );
		double gridSide = scanStep * scanResolution; // grid is square: both sides =
		drawHollowRect( Vec2d( scanXMin, scanYMin ), 
			      Vec2d( scanXMin + gridSide, scanYMin + gridSide ) );
		return;
	}



	// Display depth image surface.  
	// The variable "gridStyle" controls which of several visualizations
	// of the surface are used.
	shadingModel = GL_SMOOTH;
	lighting();
	Assert( scanResolution == MAX_GRID, "scanResolution should = MAX_GRID" );


	setColor( gridColor );


	for( int i=0; i<scanResolution-1; i++ ) {
		for( int j=0; j<scanResolution-1; j++ ) {

			// Set color according to intermediate values
			// of image analysis processing.
			gridColor = GREEN;
			if(maskImage      [i][j])   gridColor = CYAN;
			if(medialAxisImage[i][j])   gridColor = BLUE;
			setColor( gridColor );

			double x = i * scanStep  +  scanXMin;
			double y = j * scanStep  +  scanYMin;
			double dx = scanStep;
			double dy = scanStep;

			// Get the the 4 (x,y,z) coords on the corners of this grid cell.
			double x1 = x;     double y1 = y;     double z1 = zHeight[i  ][j  ];
			double x2 = x+dx;  double y2 = y;     double z2 = zHeight[i+1][j  ];
			double x3 = x+dx;  double y3 = y+dy;  double z3 = zHeight[i+1][j+1];
			double x4 = x;     double y4 = y+dy;  double z4 = zHeight[i  ][j+1];
			Vec2d v1 = Vec2d( x1, y1 );
			Vec2d v2 = Vec2d( x2, y2 );
			Vec2d v3 = Vec2d( x3, y3 );
			Vec2d v4 = Vec2d( x4, y4 );

			// display grid, according to current grid display mode (gridStyle)
			if( gridStyle == GRID_WIREFRAME ) {
				// draw line parallel to Y-axis (x varies)
				drawLine( v1, v2, gridColor, z1, z2 );
				// draw line parallel to X-axis (y varies)
				drawLine( v1, v4, gridColor, z1, z4 );
			}
			else if ( gridStyle == GRID_SOLID  ||  gridStyle == GRID_HALF_SOLID ) {
				// calc normal to plane through P1, P2, P3 using (P1-P3) x (P1-P2)
				double xn = 0., yn = 0., zn = 1.;
				crossProduct( x1-x2, y1-y2, z1-z2, x1-x3, y1-y3, z1-z3, &xn, &yn, &zn );
				normalize( &xn, &yn, &zn );
				
				// for solid draw both triangles; for half-solid only one
				glBegin(GL_POLYGON);
				glNormal3f( xn, yn, zn );
				glVertex3f( x1, y1, z1 );
				glVertex3f( x2, y2, z2 );
				glVertex3f( x3, y3, z3 );
				glEnd();

				if ( gridStyle == GRID_SOLID ) {
					// calc normal to plane through P3, P4 and P1
					crossProduct( x3-x4, y3-y4, z3-z4, x3-x1, y3-y1, z3-z1, &xn, &yn, &zn );
					normalize( &xn, &yn, &zn );

					glBegin(GL_POLYGON);
					glNormal3f( xn, yn, zn );
					glVertex3f( x3, y3, z3 );
					glVertex3f( x4, y4, z4 );
					glVertex3f( x1, y1, z1 );
					glEnd();
				}
			}
			// else if( gridStyle == GRID_NONE ) {}  // do nothing

		}
	}

}


/**************************************************************************************/
/**************************************************************************************/
// TUBE RECONITION VIA IMAGE ANALYSIS

/**************************************************************************************/
// Write current simulated image scan to disk as PPM file.
void 
writeGridToPPMFile( void ) 
{
	// The PPM in-memory data format is a flat 1D array of
	// bytes, grouped first into 3-byte chunks (24-bit pixels)
	// and then into horizontal lines (X varies fastest).



	Assert( scanResolution == MAX_GRID, "scanResolution should = MAX_GRID" );
	static unsigned char PPMFormatImage[ MAX_GRID * MAX_GRID * 3 ];
//	static unsigned char PPMFormatImage[ scanResolution * scanResolution * 3 ];
		// how to allocate using variable array length?


	int imageWidth  = scanResolution;
	int imageHeight = scanResolution;

	// Find max and min Z values in image.
	double z;
	double maxZ = -1.0e6;
	double minZ =  1.0e6;
	int xIndex;
	int yIndex;
	for( yIndex=0; yIndex<scanResolution; yIndex++ ) {
		for( xIndex=0; xIndex<scanResolution; xIndex++ ) {
			z = zHeight[xIndex][yIndex];
			if( z > maxZ )   maxZ = z;
			if( z < minZ )   minZ = z;
		}
	}
	double rangeZ = maxZ - minZ;
	if( rangeZ == 0. )   rangeZ = 1.0e-6;

	// Write pixel values into PPM format array.
	// Scale 
	for( yIndex=0; yIndex<scanResolution; yIndex++ ) {
		for( xIndex=0; xIndex<scanResolution; xIndex++ ) {
			z = zHeight[xIndex][yIndex];				// raw Z value
			double normalizedZ = (z - minZ) / rangeZ;	// 0.0 to 1.0
			unsigned char scaledZ = (unsigned char)(normalizedZ * 255.); // 0   to 255

			// Write scaled Z value into all three (R, G, B) color components.
			PPMFormatImage[ (yIndex*imageWidth + xIndex) * 3 + 0 ] = scaledZ;
			PPMFormatImage[ (yIndex*imageWidth + xIndex) * 3 + 1 ] = scaledZ;
			PPMFormatImage[ (yIndex*imageWidth + xIndex) * 3 + 2 ] = scaledZ;
		}
	}

	// Write the PPM image to a disk file.
	WritePPM( "simout.ppm", PPMFormatImage, imageWidth, imageHeight );
}



//readGridFromPPMFile( "medial.ppm", medialAxisImage, PPM_INTERPT_RGB_AS_FLAG );

/**************************************************************************************/
// Read PPM file from disk into 2D array.
void 
readGridFromPPMFile( char* fileName, double imageArray[MAX_GRID][MAX_GRID], 
					 int interp /* = PPM_INTERPT_RGB_AS_FLAG */ ) 
{
//	cout << "Attempting to read PPM file...\n";
	Assert( scanResolution == MAX_GRID, "scanResolution should = MAX_GRID" );

	// Read the named PPM file from disk into the array.
	// The last 3 args are modified by the function.
	static unsigned char* PPMFormatImage = NULL;
//	static unsigned char PPMFormatImage[ MAX_GRID * MAX_GRID * 3 ];
		// LoadPPM will allocate an array of chars of appropriate size
		// if the 2nd arg is passed in as NULL.
	int imageWidth;
	int imageHeight;
	LoadPPM( fileName, PPMFormatImage, imageWidth, imageHeight );

	// Check that the image resolution is as expected.
	Assert( imageWidth   == scanResolution, "PPM file resolution mismatch" );
	Assert( imageHeight  == scanResolution, "PPM file resolution mismatch" );

	// Translate from RGB values in flat PPM-format array to
	// interpreted data in 2D image array.
	int xIndex;
	int yIndex;
	for( yIndex=0; yIndex<scanResolution; yIndex++ ) {
		for( xIndex=0; xIndex<scanResolution; xIndex++ ) {
			// Extract all three (R, G, B) color components.
			// The PPM in-memory data format is a flat 1D array of
			// bytes, grouped first into 3-byte chunks (24-bit pixels)
			// and then into horizontal lines (X varies fastest).
			unsigned char R = PPMFormatImage[ (yIndex*imageWidth + xIndex) * 3 + 0 ];
			unsigned char G = PPMFormatImage[ (yIndex*imageWidth + xIndex) * 3 + 1 ];
			unsigned char B = PPMFormatImage[ (yIndex*imageWidth + xIndex) * 3 + 2 ];

			double pixelValue;

			// Classify each RGB pixel as to its meaning.
			switch( interp ) {
			case PPM_INTERPT_RGB_AS_FLAG:
				// Classify RGB value as flag (0 or 1).
				// (All RGB components are 0 or 255 for medial-axis or mask pixels.)
				pixelValue = (R>127 || G>127 || B>127) ? 1. : 0.;
				break;

			case PPM_INTERPT_RGB_AS_HEIGHT:
				pixelValue = R;		// just use value of red component
				pixelValue /= 25.;  // scale down 0-255 to about 0-10.
				break;

			default:
				error( "unknown PPM file interpretation type." );
				break;
			}

			// Copy extracted data into 2D image array.
			imageArray[xIndex][yIndex] = pixelValue;
		}
	}

	// Delete PPM-format char array newed in LoadPPM
	if( PPMFormatImage != NULL ) {
		delete [] PPMFormatImage;	// Why []? See Myers 50 Ways p23 (item 5)
		PPMFormatImage = NULL;
	}
}


/**************************************************************************************/
// Find tubes in image.
// Convert from pixel coords to sim coords (nominally nm).
void 
extractFoundTubes( void ) 
{
	// Write out image file to simout.ppm.
	writeGridToPPMFile();

	// Do image analysis to find rigid tubes,
	// characterized by parameters 
	//     * position of tube center (X,Y),
	//     * length L
	//     * width  W
	//     * orientation on the surface (angle A)
	// Files are used for I/O to this algorithm for now.
	// The input file is "simout.ppm".
	// The output file is "param.dat".
	findTubesInImage();

	// Extract tube params from text file and return
	// a vector of tubes (coords in pixel units).
	foundTubeVec = extractFoundTubesFromFile( "param.dat" );


	// Map from pixel coords to nm.
	for( int i=0; i<foundTubeVec.size(); ++i ) {
		convertFromImageCoordsToWorldCoords( foundTubeVec[i] );
	}
}


/**************************************************************************************/
// Convert this tube's parameters from image coords back to world coords.  
// The image is assumed to be an NxN image where N is "scanResolution".
// The image analysis routines operate in image coords, so
// when a tube is recognized, it's parameters must be mapped
// back to world coords.  
// Here are the parameters defining the location of the image scan grid
// in world coords (nominally nanometers):
//		int    scanResolution;	// scan grid resolution
//		double scanStep;		// scan grid pitch (sample-to-sample spacing)
//		double scanXMin;		// scan grid origin X coord (left side)
//		double scanYMin;		// scan grid origin Y coord (bottom)
void
convertFromImageCoordsToWorldCoords( OB &tube )
{
	// Lengths are changed only by the magnification factor.
	tube.leng  *= scanStep;
	tube.diam  *= scanStep;

	// Positions must also have the world-space origin added in.
	Vec2d scanOrigin( scanXMin, scanYMin );
	tube.pos   = scanOrigin + (tube.pos * scanStep);
}

	


/**************************************************************************************/
// member functions for class OB
/**************************************************************************************/


#if 0
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
#endif



/**************************************************************************************/
// default constructor for OB
OB::OB( void ) 
	: type(TUBE), 
	pos(Vec2d(0.,0.)), angle(0.), roll(0.), leng(1.), diam(1.), 
	nextSeg(NULLOB), prevSeg(NULLOB), moved(0)
{}


/**************************************************************************************/
// constructor for OB
OB::OB( Vec2d pos, double angle, double length, double diameter )
	: type(TUBE), 
	pos(pos), angle(angle), roll(0.), leng(length), diam(diameter), 
	nextSeg(NULLOB), prevSeg(NULLOB), moved(0)
{}



/**************************************************************************************/
/**************************************************************************************/


