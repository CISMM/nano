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


#include <fstream.h>

#include <stdlib.h>
#include <stdio.h>

#ifndef _WIN32
#include <unistd.h>
#endif

#include <math.h>

#ifndef NMRC_LIB
#include <GL/glut.h>
#else
   #ifdef V_GLUT
#include <GL/glut.h>
   #else
#include <GL/glx.h>
#include <GL/glu.h>
#include <GL/gl.h>
   #endif
#endif

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
#ifndef M_PI
#define M_PI            3.14159265358979323846
#endif

#define DEG_TO_RAD   (M_PI / 180.)
#define RAD_TO_DEG   (180. / M_PI)

#ifndef NMRC_LIB
static int dblBuf  = GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH;
//static int snglBuf = GLUT_SINGLE | GLUT_RGBA | GLUT_DEPTH;
//static char *winTitle = "Warren's CNT simulator v1.0";
#endif

#define True  1
#define False 0
#define MAXOB 100
#define MAX_GRID 128
#define MAX_PATHQ 101
#define NULLOB (-1)
// object types
#define TIP 0
#define PHANTOM 1
#define TUBE 2
#define SUBSTRATE 3
// colors
#define WHITE   0
#define RED     1
#define GREEN   2
#define BLUE    3
#define MAGENTA 4
#define YELLOW  5
#define CYAN    6
#define BLACK   7
#define ORANGE  8
// drawing styles
#define OB_SOLID      0
#define OB_WIREFRAME   1
#define OB_POINTS      2
#define OB_SILHOUETTE   3
#define OB_OUTLINE2D   4
#define OB_NONE         5
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
typedef struct {
   int   type;
   Vec2d pos;      // 2D points (in the XY plane), at present
   float angle;   // for in-XY-plane rotations
   float roll;      // for tube rotating around its axis
   float leng;      // length of the tube
   float diam;      // diameter of the tube
   int   nextSeg;   // link to next segment of bendable tube (can be null)
   int   prevSeg;   // link to previous segment of bendable tube  (can be null)
   float bendAngle;// angle bent by this segment (radians); 0=no bending
   int   moved;   // 1=already moved this sim step; 0=free to be moved.
} OB;

// Data structure containing data pertaining to collision test between 
// sphere and tube.
typedef struct {
   Vec2d vP;   // input: point P
   Vec2d vA;   // input: endpoint A of line seg
   Vec2d vB;   // input: endpoint B of line seg
   Vec2d vM;   // nearest point on line AB to P
   Vec2d vS;   // pivot point
   Vec2d vC;   // midpoint of AB
   float k;   // parameter locating point M = B + (A-B)*k
   float sphereRadius;
   float tubeRadius;
   float dist;      // distance from sphere to tube
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
   Vec2d vD;   // input: endpoint D of line seg DE
   Vec2d vE;   // input: endpoint E of line seg DE
   Vec2d vF;   // input: endpoint F of line seg FG
   Vec2d vG;   // input: endpoint G of line seg FG

   float dist;   // distance between the two line segments
   int whichCase;      // which of the 6 cases this is
   int parallelFlag;   // 1=line segs parallel
   int intersectFlag;   // 1=line segs intersect
   int overlapFlag;   // 1=parallel and mutual projection is line seg
   Vec2d vN1;   // nearest point on line seg DE to line seg FG
   Vec2d vN2;   // nearest point on line seg FG to line seg DE
   Vec2d vI;   // point of intersection of line DE and line FG
               // (Point I exists only if lines are not parallel.)
   Vec2d vI1;   // endpoint 1 of line segment which is intersection of the line segs
   Vec2d vI2;   // endpoint 2 of line segment which is intersection of the line segs
} LINE_SEGMENTS_INTERSECTION_TEST;

typedef struct {
   int numObs;
   OB ob[MAXOB];
} SIM_STATE;


/**************************************************************************************/
/**************************************************************************************/
// FUNCTION PROTOYPES
void   error( char* errMsg );
void   assert( int condition, char* msg );
int      streq( char* str1, char* str2 );
Vec2d   interpolateVecs( Vec2d v0, Vec2d v1, float fraction );
float   interpolateScalars( float s0, float s1, float fraction );
void   showStrokeString( char* str );
void   showBitmapString( char* str );
void   getState( SIM_STATE st );
void   setState( SIM_STATE* pSt );
void   showLabeledPoint( Vec2d vPoint, char* labelStr );
void   rotateRadiansAxis( float angleInRadians, 
               float axisComponentX, float axisComponentY, float axisComponentZ );
void   setMaterialColor( GLfloat r, GLfloat g, GLfloat b );
void   setColor( int colorIndex );
#ifndef NMRC_LIB
void   drawCube( float halfWidth );
void   drawTorus( float innerDiameter, float outerDiameter );
#endif
void   drawSphere( float radius );
void   showPoint( Vec2d vPoint, int color, float size = 1., float zHeight = 0. );
void   drawCone( float radius, float height );
void   drawCylinder( float diameter, float height );
void   drawPolygon( void );
void   drawLine( Vec2d pt1, Vec2d pt2, int color, float z1 = 0., float z2 = 0. );
void   drawTip( float tipDiameter );
void   drawTube( float diameter, float length, int obIndex );
void   drawSubstrate( void );
void   lighting( void );
float   norm( Vec2d v );
float   distance( Vec2d pt1, Vec2d pt2 );
int      findNearestObToMouse( void );
void   addObject( int obNum, 
         int type, Vec2d pos, float yaw, float roll, float leng, float diam,
         int nextSeg, int prevSeg, float bendAngle );
void   newOb( void );
void   deleteOb( int n );
float   centerAngle( float angle );
float   calcAngleInPlane( Vec2d vA, Vec2d vB, Vec2d vC );
float   pivotPointEquation( float k );
Vec2d   PivotPoint( float normalizedPushPoint, Vec2d vEndA, Vec2d vEndB  );
float   nearestPointOnLineToPoint( Vec2d vA, Vec2d vB, Vec2d vP );
void   collisionDraw( SPHERE_AND_TUBE_DATA cxTest );
void   lineSegmentCollisionDraw( LINE_SEGMENTS_INTERSECTION_TEST lineSegCx );
int   betweenInclusive( float value,   float bound1, float bound2 );
Vec2d   xformDEtoYaxis( Vec2d vIn, Vec2d vD, Vec2d vE );
Vec2d   inverseXformDEtoYaxis( Vec2d vIn, Vec2d vD, Vec2d vE );
float   touchDistance( float radius1, float radius2 );
SPHERE_AND_TUBE_DATA   distSphereToCylinder( Vec2d vP, Vec2d vA, Vec2d vB, 
                     float radiusSphere, float radiusCylinder );
LINE_SEGMENTS_INTERSECTION_TEST   distBetweenLineSegments( 
                           Vec2d vD, Vec2d vE, Vec2d vF, Vec2d vG );
Vec2d   translateJustOutOfContact( SPHERE_AND_TUBE_DATA cxTest );
void   translateCollisionResponse(  Vec2d vPushPt, int tube, 
                           SPHERE_AND_TUBE_DATA cxTest  );
void   calcComponentVectorsRelativeToAxis( Vec2d vVector, Vec2d vAxis, 
                            Vec2d* pvNormal, Vec2d* pvParallel );
void   rollingCollisionResponse(  Vec2d vPushPt, int tube, 
                         SPHERE_AND_TUBE_DATA cxTest  );
void   calcTubeEndpoints( int tube, Vec2d* pvEndA, Vec2d* pvEndB );
void   slidingCollisionResponse( Vec2d vPushPt, int tube, 
                         SPHERE_AND_TUBE_DATA cxTest );
void   rollingSlidingCollisionResponse( Vec2d vPushPt, int tube, 
                              SPHERE_AND_TUBE_DATA cxTest );
void   collisionResponse( Vec2d vPushPt, float radiusSphere, int pushedTube );
void   crossedTubeAxisCollisionResponse(  LINE_SEGMENTS_INTERSECTION_TEST lineSegCx,
            int pushingTube, int pushedTube  );
void   TubeTubeCollisionTestAndResponse( int pushingTube, int pushedTube );
void   positionSegmentRigid( int curSeg, int lastSeg, int directionFlag );
void   positionSegment( int curSeg, int lastSeg, int directionFlag );
void   positionSegmentedTube( int movedSegment );
void   propagateMovements( int movedTube );
void   markAllTubesMovable( void );
int   newSegmentedTube( int numSegments, float bendAngle, 
              float segmentLength, float segmentWidth, Vec2d vEndpoint  );
void   collisionStuff( void );
void   showSubstrateLines( void );
#ifndef NMRC_LIB
void   showText( void );
#endif
void   showObjectLabels( void );
void   drawObjects( void );
void   drawStuff( void );
void   simStep(void);
int      moveTipToXYLoc( float x, float y );
int      getImageHeightAtXYLoc( float x, float y, float* z );
void   crossProduct( float  x1, float  y1, float  z1,
           float  x2, float  y2, float  z2,
           float* px, float* py, float* pz );
void   normalize( float* px, float* py, float* pz );
void   showGrid( void );
void   drawFrame( void );
void   singleSimStep(void);
void   toggleSim(void);
void   adjustOrthoProjectionToWindow( void );
void   zoom( float zoomFactor );
void   pan( float screenFractionX, float screenFractionY );
void   commonKeyboardFunc(unsigned char key, int x, int y);
void   doTopMenu(int value);
void   doSubMenu(int value);
void   initMenus(void );
void   moveGrabbedOb( void );
void   grabNearestOb( void );
void   calcMouseWorldLoc( int xMouse, int yMouse );
void   mouseFuncMain( int button, int state, int x, int y );
void   mouseMotionFuncMain( int x, int y );
void   reshapeWindowFuncMain( int newWindowWidth, int newWindowHeight );
void   timerFunc(int value);
void   initObs( void );
void   displayFuncMain( void );
void   displayFuncUI( void );
void   displayFuncView( void );
void   imageScanDepthRender( void );
void   doImageScan( void );
void   displayFuncDepth( void );
void   commonIdleFunc( void );
void   displayFuncDummy( void );
void   keyboardFuncDummy(unsigned char key, int x, int y);
void   idleFuncDummy( void );
void   mouseFuncDummy( int button, int state, int x, int y );
void   mouseMotionFuncDummy( int x, int y );
void   reshapeWindowFuncDummy( int newWindowWidth, int newWindowHeight );
#ifndef NMRC_LIB
int      main(int argc, char *argv[]);
#endif
void   robotStep(void);
void   showRobotStuff( void );
void   robotExecutive( void );
Vec2d   towardVec( Vec2d vTarget, Vec2d vFrom, float speed );
void   moveTipTowardPoint( Vec2d vPoint );
void   startRobot( void );
void   popFrontPathQ( void );
void   pushBackPathQ( Vec2d vPathPoint );
Vec2d   frontPathQ( void );
vrpn_bool   emptyPathQ( void );
void   clearPathQ( void );
void   moveTipToRotateTube( void );
vrpn_bool   rotatedEnoughTest( void );
void   moveTipToTranslateTube( void );
void   showBox( OB tube, int color );
void   calcTubePointsAndVectors( 
   OB tube, Vec2d* vCenter, Vec2d* vAxis, Vec2d* vWidth,
   Vec2d* vRightClear, Vec2d* vLeftClear, Vec2d* vTopClear, Vec2d* vBottomClear );
Vec2d   normalizeVec( Vec2d v );
void   showSphere( Vec2d vPoint, int color, float radius, float z );
void   addPointToTipPath( Vec2d vPathPoint );
vrpn_bool   translatedEnoughTest( void );
vrpn_bool   rotatedEnoughTest2( void );

void buildScan1Path(void);
void analizeScan1Path(void);
void buildScan2Path(void);
void analizeScan2Path(void);
void buildScan3Path(void);
void analizeScan3Path(void);

/**************************************************************************************/
/**************************************************************************************/
// GLOBAL VARIABLES
//objects
OB nullob;      // at index loc -1 (immediately before ob[...])
OB ob[MAXOB];   // array of objects used in the simulation
int numObs = 0;   // current count of objects in sim world
int selectedOb = NULLOB;   // object currently selected by user

// image scan
float zHeight[MAX_GRID][MAX_GRID];   // array of heights: image scan data
float zBuffer[ 128*128 ];         // raw values (normalized) from Z-buffer
#ifndef NMRC_LIB
int gridSize = 64;      // scan grid resolution
#else
int gridXSize = 64;
int gridYSize = 64;
#endif
float scanLength = 128.;   // scan grid total width/height
float scanXMin =  0.;
float scanYMin =  0.;
float scanNear =  -100.;   // near end of Z-buffer range
float scanFar  =   100.;   // far  end of Z-buffer range
float scanXMax =   scanXMin + scanLength;
float scanYMax =   scanYMin + scanLength;
#ifndef NMRC_LIB
float scanStep   = scanLength / gridSize;
#else
float scanStep = 1.0;
#endif

// simulation 
SIM_STATE prevState;   // state of sim before previous simulation-step
SIM_STATE saveState;   // state of all objects in the simulation (used in undo)
int simType = SIM_SLIDING;
vrpn_bool simRunning = vrpn_true;

// robot stuff
vrpn_bool robotRunning = vrpn_false;
int robotModPhase = 0;
int robotImgPhase = 0;
float tipSpeed = 4.;
OB currentGoal;
OB measuredTubePose;
OB scannedTubePose;
OB predictedTubePose;
int currentGoalTube = 0;
Vec2d pathQ[MAX_PATHQ];
int pathQSize = 0;
Vec2d scanPath[MAX_PATHQ];
float scanPathZ[MAX_PATHQ];
int scanPathIdx = 0;
Vec2d scan1Max, scan1Min, scan2Max, scan2Min, scan3Min1, scan3Min2;
float scan1MaxZ, scan1MinZ, scan2MaxZ, scan2MinZ, scan3Min1Z, scan3Min2Z;

// display options
int renderStyle = OB_OUTLINE2D;
int gridStyle   = GRID_NONE;
int imageScanType = IMAGE_SCAN_APPROX;
GLenum drawStyle = GL_FILL;       
GLenum shadingModel = GL_FLAT;   // GL_FLAT or GL_SMOOTH
vrpn_bool lightOn[8] = { vrpn_true, vrpn_true, vrpn_false, vrpn_false, 
        vrpn_false, vrpn_false, vrpn_false, vrpn_false };
vrpn_bool namesOn = vrpn_false;
vrpn_bool displayCollisionsOn  = vrpn_true;
vrpn_bool displayPivotPointsOn = vrpn_true;
vrpn_bool linesOn = vrpn_false;
vrpn_bool displayConeOn = vrpn_false;
int frameCount = 0;
float framesPerSecond = 0.;

// window stuff
int mainWindowID;
int UIWindowID;
int viewWindowID;
int depthWindowID;
float viewYawAngle   =  M_PI/2.;
float viewPitchAngle = -M_PI/3.;   // was -M_PI/2.
float windowWidth  = 600.;
float windowHeight = 600.;
float orthoFrustumCenterX = 64.;   // area of XY plane always visible for all window aspect ratios
float orthoFrustumCenterY = 64.;
float orthoFrustumWidthNominal  = 128. + 10.;
float orthoFrustumHeightNominal = 128. + 10.;
// actual bounds of current ortho view frustum matching window aspect ratio
float orthoFrustumLeftEdge;
float orthoFrustumBottomEdge;
float orthoFrustumWidth;
float orthoFrustumHeight;

// mouse and cursor
int xMouseInWindow;   // mouse position in world coords
int yMouseInWindow;   // mouse position in world coords
Vec2d vMouseWorld;   // mouse position in world coords (as a 2D vector)
Vec2d vGrabOffset;   // offset from cursor position to grabbed object (in world coords)

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

#ifndef NMRC_LIB
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
   mainWindowID = glutCreateWindow( "Warren's CNT simulator v1.0" );
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
#endif

/**************************************************************************************/
// dummy (stub) routines for window callback routines
void displayFuncDummy( void ) {}
void keyboardFuncDummy(unsigned char key, int x, int y)   {commonKeyboardFunc(key,x,y);}
#ifndef NMRC_LIB
void idleFuncDummy( void ) {commonIdleFunc();}
#endif
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
#ifndef NMRC_LIB
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

   glutSetWindow( mainWindowID );      glutPostRedisplay();
   glutSetWindow( UIWindowID );      glutPostRedisplay();
   glutSetWindow( viewWindowID );      glutPostRedisplay();
   glutSetWindow( depthWindowID );      glutPostRedisplay();
}
#endif


/**************************************************************************************/
/**************************************************************************************/
// GRAPHICS -- TOP-LEVEL

/**************************************************************************************/
void
drawFrame( void )
{
   // draw into main window
#ifndef NMRC_LIB
   glutSetWindow( mainWindowID );
#endif

   // Setup OpenGL state.
   glClearDepth(1.0);
#ifdef NMRC_LIB
   if (g_activeControl)
      glClearColor(0.7, 0.4, 0.4, 0.0);
   else
#endif
   glClearColor(0.5, 0.5, 0.5, 0.0);   // gray background

   glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
   glEnable(GL_DEPTH_TEST);


   // draw objects, text, and other graphics.

   drawStuff();

   robotStep();

   // do collision detection and collision response
   // (may draw some graphics, too.)
   simStep();

   showGrid();

   // end of display frame, so flip buffers
#ifndef NMRC_LIB
   glutSwapBuffers();
#endif

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
// draw text labels on things
#ifndef NMRC_LIB
void
showObjectLabels( void )
{
   int i;
   char str[256];


   if( namesOn ) {
      // Put text labels next to objects
      for( i=0; i<numObs; i++ ) {
         glPushMatrix();
         glTranslatef( ob[i].pos.x, ob[i].pos.y, 0. );

         sprintf( str, "Ob #%d", i );
         showBitmapString( str );

         glPopMatrix();
      }
   }
}
#endif


/**************************************************************************************/
void
drawObjects( void )
{
   int i;

   // draw the objects
   for( i=0; i<numObs; i++ ) {
      // set colors 
      if (ob[i].type == PHANTOM)
      {
         if( i == selectedOb )
         {
            setColor( ORANGE );
         }
         else
         {
            setColor( RED );
         }
      }
	  else if( i == selectedOb )
      {
         setColor( YELLOW );
      }
	  else
      {
		 setColor( WHITE );
      }


      glPushMatrix();

      // put tube at its (x,y) position
      glTranslatef( ob[i].pos.x, ob[i].pos.y, 0. );

      // put tube flat on XY plane
      // (ie, with its lower surface touching XY plane)
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

      case PHANTOM:
         drawTube( ob[i].diam, ob[i].leng, i );
         break;

      case TUBE:      
         drawTube( ob[i].diam, ob[i].leng, i );
         break;

      case SUBSTRATE:   
         drawSubstrate();
         break;

//      case 3: drawCone( 0.5, 1.0 ); break;
//      case 4: drawTorus( 0.2, 1.0 ); break;
//      case 5: drawCylinder( 0.5, 1.0 ); break;
//      case 6: drawPolygon(); break;
//      default: drawTorus( 0.2, 1.0 ); break;

      default: error( "attempt to draw unknown object type" );
      }

      glPopMatrix();
   }
}


/**************************************************************************************/
void
drawStuff( void )
{
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

   showSubstrateLines();
      
   // draw all objects at their current locations
   drawObjects();
#ifndef NMRC_LIB
   showObjectLabels();
#endif
}


/**************************************************************************************/
// adjust the ortho projection to match window aspect ratio and keep circles round.
void
adjustOrthoProjectionToWindow( void )
{
#ifndef NMRC_LIB
   float orthoFrustumNearEdge =  -100.;
   float orthoFrustumFarEdge  =   100.;
#else
   float orthoFrustumNearEdge = scanNear;
   float orthoFrustumFarEdge  = scanFar;
#endif

   // set nominal size of window before taking aspect ratio into account
   //float orthoFrustumLeftEdgeNominal   = orthoFrustumCenterX - orthoFrustumWidthNominal/2.;
   float orthoFrustumBottomEdgeNominal = orthoFrustumCenterY - orthoFrustumHeightNominal/2.;

   // calculate aspect ratio of current window
   float aspectRatio = windowWidth / windowHeight;

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
zoom( float zoomFactor )
{
   // adjust window size in world coords
   orthoFrustumHeightNominal      *= zoomFactor;
   orthoFrustumWidthNominal       *= zoomFactor;
}


/**************************************************************************************/
void
pan( float screenFractionX, float screenFractionY )
{
   // pan viewing frustum center (which is parallel to Z-axis)
   // around in X and Y.
   orthoFrustumCenterX += (orthoFrustumWidthNominal  * screenFractionX);
   orthoFrustumCenterY += (orthoFrustumHeightNominal * screenFractionY);
}

#ifdef NMRC_LIB
void updateLiveData(void);
#endif

/**************************************************************************************/
// This routine is called only after input events.
void
displayFuncMain( void )
{
   adjustOrthoProjectionToWindow();
   // draw graphics for this frame
   drawFrame();
}


/**************************************************************************************/
// display graphics in the UI (User Interface) window.
#ifndef NMRC_LIB
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
#endif

/**************************************************************************************/
// display graphics in the UI (User Interface) window.
void displayFuncView( void ) 
{
   // set up projection matrix to look down X-axis
   adjustOrthoProjectionToWindow();            // set up projection down Z-axis
   rotateRadiansAxis( -M_PI/2., 0.0, 0.0, 1.0 );      // rotate around Z
   rotateRadiansAxis( viewPitchAngle, 0.0, 1.0, 0.0 );   // rotate view direction to X-axis
   rotateRadiansAxis( viewYawAngle,   0.0, 0.0, 1.0 );   // rotate around Z


   // draw into main window
#ifndef NMRC_LIB
   glutSetWindow( viewWindowID );

   // Setup OpenGL state.
   glClearDepth(1.0);
   glClearColor(0.5, 0.5, 0.5, 0.0);   // gray background
   glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
   glEnable(GL_DEPTH_TEST);

   // draw objects (tip and tubes)
   int saveRenderStyle = renderStyle;
   if( renderStyle == OB_OUTLINE2D )   renderStyle = OB_WIREFRAME;
   drawStuff();
   renderStyle = saveRenderStyle;
#else
   drawFrame();
#endif

   // do collision detection and collision response
   // (may draw some graphics, too.)
#ifndef NMRC_LIB
   simStep();

   showGrid();

   // end of display frame, so flip buffers
   glutSwapBuffers();
#endif
}


/*********************** added for nmRobotControl library *******************/

#ifdef NMRC_LIB
void updateLiveData(void)
{
   nmb_Image *hImage = g_dataset->dataImages->getImageByName(
                             g_dataset->heightPlaneName->string());


   gridXSize = hImage->width();
   gridYSize = hImage->height();
   scanXMin = 0.;
   scanYMin = 0.;
   scanXMax = scanXMin + gridXSize;
   scanYMax = scanYMin + gridYSize;
   scanNear = -(hImage->maxValue() - hImage->minValue()) - 100.0 ;
   scanFar  = -scanNear;

   Point_results *point = g_microscope->state.data.inputPoint;

   ob[TIP].pos = Vec2d(point->x(), point->y());
}
#endif

/**************************************************************************************/
/**************************************************************************************/
// KEYBOARD

/**************************************************************************************/
// Keyboard callback for main window.
void
commonKeyboardFunc(unsigned char key, int x, int y)
{
      // To respond to non-ASCII (special) key keypresses, see
      // http://reality.sgi.com/opengl/spec3/node54.html#1044
      // Section 7.9 (glutSpecialFunc) -- uses eg, GLUT_KEY_F1 
   x=x; y=y;
//   printf( "key code: 0x%x\n", key );

   switch (key) {

#ifndef NMRC_LIB
   case '[':   moveTipToXYLoc( 0.,  20. );   break;   
   case ']':   moveTipToXYLoc( 0., -20. );   break;   
#endif

   // save state and revert
   case 's':   setState( &saveState );      // save state
      break;   
   case 'u':   getState(  saveState );   // revert to saved state (undo)
            setState( &prevState );   // make previous state match new current state
      break;   
         

   // robot stuff
   case 'r':
      if (selectedOb != NULLOB && ob[selectedOb].type == TUBE)
      {
         currentGoalTube = selectedOb;
         startRobot();
      }
      break;
   case 'R':
      if (!(robotRunning = !robotRunning))
      {
         g_microscope->ResumeScan();
         g_activeMode = SCAN_MODE;
         g_activeControl = vrpn_false;
      }
      break;
   case 'g':   // set goal
      if( selectedOb != NULLOB && ob[selectedOb].type == PHANTOM)
      {
         currentGoal = ob[selectedOb];
      }
      break;

   case 'k':
      clearPathQ();
      scanPathIdx = 0;
      robotRunning = vrpn_false;
      robotModPhase = robotImgPhase = 0;
      g_microscope->ResumeScan();
      g_activeMode = SCAN_MODE;
      g_activeControl = vrpn_false;
      break;
#ifndef NMRC_LIB
   case 'f':   tipSpeed *= 2.;      break;
   case 'F':   tipSpeed /= 2.;      break;


   case 'G':   toggleSim();      break;
   case ' ':   singleSimStep();   break;
#endif

   case 'o':
      if(      renderStyle == OB_SOLID )      {renderStyle = OB_WIREFRAME;}
      else if( renderStyle == OB_WIREFRAME )  {renderStyle = OB_OUTLINE2D;}
      else if( renderStyle == OB_OUTLINE2D )  {renderStyle = OB_NONE;}
      else if( renderStyle == OB_NONE )      {renderStyle = OB_SOLID;}
      else                         error( "bad value for renderStyle" );      
      break;
   case 'i':
      if(      gridStyle == GRID_NONE )         {gridStyle = GRID_HALF_SOLID;}
      else if( gridStyle == GRID_HALF_SOLID )      {gridStyle = GRID_SOLID;}
      else if( gridStyle == GRID_SOLID )         {gridStyle = GRID_WIREFRAME;}
      else if( gridStyle == GRID_WIREFRAME )      {gridStyle = GRID_NONE;}
      else                              error( "bad value for gridStyle" );      
      break;
   case 'e':
      if(      imageScanType == IMAGE_SCAN_EXACT )   {imageScanType = IMAGE_SCAN_APPROX;}
      else if( imageScanType == IMAGE_SCAN_APPROX )   {imageScanType = IMAGE_SCAN_EXACT;}
      else                                  error( "bad value for imageScanType" );      
      break;

//   case '1':   lightOn[0] = ! lightOn[0];   break;
//   case '3':   lightOn[1] = ! lightOn[1];   break;


#if 0
   case 's':   
      if(      simType == SIM_NONE )       simType = SIM_TRANSLATE;
      else if( simType == SIM_TRANSLATE )   simType = SIM_ROLLING;
      else if( simType == SIM_ROLLING )   simType = SIM_SLIDING;
      else if( simType == SIM_SLIDING )   simType = SIM_ROLLING_SLIDING;
      else if( simType == SIM_ROLLING_SLIDING )   simType = SIM_NONE;
      break;
#endif


   case 'n':   namesOn  = ! namesOn;   break;
   case '.':   displayCollisionsOn  = ! displayCollisionsOn;   break;
   case ',':   displayPivotPointsOn = ! displayPivotPointsOn;   break;
   case '/':   linesOn =  ! linesOn;   break;
   case '^':   displayConeOn =  ! displayConeOn;   break;




   // create and delete objects
   case 'v':
      if (ob[selectedOb].type == PHANTOM)
         newOb();
	  break;      
   case 'x':   if( selectedOb!=NULLOB) deleteOb( selectedOb);   break;      

   // change object parameters
   case 'T':   ob[selectedOb].angle +=  5. * DEG_TO_RAD;   break;
   case 't':   ob[selectedOb].angle += -5. * DEG_TO_RAD;   break;

   case 'l':   
      if( ob[selectedOb].type == TIP )  break;
      ob[selectedOb].leng +=  1.;   
      break;
   case 'L':   
      if( ob[selectedOb].type == TIP )  break;
      ob[selectedOb].leng += (ob[selectedOb].leng > 0.) ? -1. : 0.;   
      break;

   case 'w':   ob[selectedOb].diam +=  1.;   break;
   case 'W':   ob[selectedOb].diam += (ob[selectedOb].diam > 1.) ? -1. : 0.;   break;

//   #define ANGLE_INCREMENT (M_PI/36.)
//   case 'b':   ob[selectedOb].bendAngle +=  ANGLE_INCREMENT;   break;
//   case 'B':
//      ob[selectedOb].bendAngle += (ob[selectedOb].leng > ANGLE_INCREMENT) 
//                                ? (- ANGLE_INCREMENT) : 0.;   
//      break;





   // zoom and pan using numeric keypad
   case '5':   zoom( 0.9 );   break;      // middle of numeric keypad (zoom in)
   case '0':   zoom( 1.1 );   break;      // bottom of numeric keypad (zoom out)
   case '2':   pan(  0.,  -0.1 );   break;   // down-arrow  on numeric keypad
   case '4':   pan( -0.1,  0.  );   break;   // left-arrow  on numeric keypad
   case '6':   pan(  0.1,  0.  );   break;   // right-arrow on numeric keypad
   case '8':   pan(  0.,   0.1 );   break;   // up-arrow    on numeric keypad


   case 'Y':   viewYawAngle   +=  5. * DEG_TO_RAD;   break;
   case 'y':   viewYawAngle   += -5. * DEG_TO_RAD;   break;
   case 'P':   viewPitchAngle +=  5. * DEG_TO_RAD;   break;
   case 'p':   viewPitchAngle += -5. * DEG_TO_RAD;   break;
   case 'V':   viewYawAngle = M_PI/2.;  viewPitchAngle = -M_PI/3.; break; // std view

#ifndef NMRC_LIB
   case 'q':   // quit
   case 27:   // Esc
      // Exit program.  
      exit(0);
#endif

#ifdef NMRC_LIB
   case 'A':
      g_activeControl = !g_activeControl;
      break;
   case 'd':
      g_displayPlan = !g_displayPlan;
      break;
#endif
   }

#ifndef NMRC_LIB
   glutPostRedisplay();
#else
   g_iViewer->dirtyWindow(g_iViewerWin);
#endif
}


/**************************************************************************************/
/**************************************************************************************/
// MOUSE AND CURSOR

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
   if( selectedOb != NULLOB && ob[selectedOb].type != TIP ) {
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
   float xMouseNormalized;
   float yMouseNormalized;

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
      if(      state == GLUT_DOWN )   {grabNearestOb();}
      else if( state == GLUT_UP )      {}
      break;
   case GLUT_RIGHT_BUTTON: 
      if(      state == GLUT_DOWN )   {}
      else if( state == GLUT_UP )      {}
      break;
   }

#ifndef NMRC_LIB
   glutPostRedisplay();
#else
   g_iViewer->dirtyWindow(g_iViewerWin);
#endif
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

#ifndef NMRC_LIB
   glutPostRedisplay();
#else
   g_iViewer->dirtyWindow(g_iViewerWin);
#endif
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
#ifndef NMRC_LIB
   glutPostRedisplay();
#else
   g_iViewer->dirtyWindow(g_iViewerWin);
#endif
}


/**************************************************************************************/
// searches through all objects to find the object nearest the mouse cursor.
// returns the index of the object, or NULLOB if no objects are within threshold.
int
findNearestObToMouse( void )
{
   int i;
   int nearestOb = NULLOB;
   float nearestDist = 1000000.;
   float thresholdDist = 3.;
   float dist;

   for( i=0; i<numObs; i++ ) {
      dist = distance( vMouseWorld, ob[i].pos );

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
#ifndef NMRC_LIB
void
showText( void )
{
   // Title at top middle of screen giving sim type
//   glPushMatrix();
//   glTranslatef(  -3., 9., 0. );
//
//   switch( simType ) {
//   case SIM_NONE:      showStrokeString( "DETECT COLLISIONS" );break;
//   case SIM_TRANSLATE:   showStrokeString( "TRANSLATE sim" );break;
//   case SIM_ROLLING:   showStrokeString( "ROLLING sim" );break;
//   case SIM_SLIDING:   showStrokeString( "SLIDING sim" );break;
//   case SIM_ROLLING_SLIDING:   showStrokeString( "ROLLING & SLIDING sim" );break;
//   }
//   glPopMatrix();

   // first column UI screen
   glPushMatrix();
   glTranslatef( -10., 9.5, 0. );

   showBitmapString( "CNT simulator, version 1.0" );
   showBitmapString( "Warren Robinett, Nov 99" );
   showBitmapString( "(rigid bodies, sliding only)" );
   showBitmapString( "" );

   // count frames and display count
   frameCount++;
   char str[256];
   sprintf( str, "frameCount:%8d", frameCount ); showBitmapString( str );
   sprintf( str, "frames/sec: %6.1f", framesPerSecond ); showBitmapString( str );

   sprintf( str, "simulation on:   %d", simRunning ); showBitmapString( str );
//   sprintf( str, "simulation type: %d", simType );    showBitmapString( str );
   sprintf( str, "robot on:        %d", robotRunning ); showBitmapString( str );
   sprintf( str, "image-scan type (1=exact): %d", imageScanType );    showBitmapString( str );
   sprintf( str, "number of objects: %d", numObs ); showBitmapString( str );




   if( selectedOb != NULLOB ) {
      showBitmapString( "" );
      showBitmapString( "SELECTED OBJECT (tube or tip)" );
      sprintf( str, "  object #:  %d",            selectedOb );   showBitmapString( str );
      sprintf( str, "  position:  %6.1f, %6.1f",  ob[selectedOb].pos.x, ob[selectedOb].pos.y ); showBitmapString( str );
      sprintf( str, "  yaw angle: %6.1f degrees", ob[selectedOb].angle * RAD_TO_DEG ); showBitmapString( str );
      sprintf( str, "  roll angle:%6.1f degrees", ob[selectedOb].roll  * RAD_TO_DEG ); showBitmapString( str );
      sprintf( str, "  length:    %6.1f nm",      ob[selectedOb].leng ); showBitmapString( str );
      sprintf( str, "  diameter:  %6.1f nm",      ob[selectedOb].diam ); showBitmapString( str );
   }


   //   pathQ
   for( int j=0; j<pathQSize; j++ ) {
      sprintf( str, "pathQ: %6.1f %6.1f", pathQ[j].x, pathQ[j].y ); showBitmapString( str );
   }



   glPopMatrix();



   // second column in UI screen
   glPushMatrix();
   glTranslatef( 0., 9.5, 0. );

   showBitmapString( "KEYBOARD COMMANDS" );
   showBitmapString( "g: simulation on/off" );
   showBitmapString( "SPACEBAR: single-step sim" );
   showBitmapString( "q or ESC: quit program" );
   showBitmapString( "" );

   showBitmapString( "v: create new tube" );
   showBitmapString( "x: delete selected tube" );
   showBitmapString( "w,W: change tube width (diameter)" );
   showBitmapString( "l,L: change tube length" );
   showBitmapString( "t,T: change tube yaw angle" );
   showBitmapString( "" );

   showBitmapString( "o: change object draw mode" );
   showBitmapString( "i: change image  draw mode" );
   showBitmapString( "e: exact/approx image scan" );
   showBitmapString( "/: lines on/off" );
   showBitmapString( ".: collision points on/off" );
   showBitmapString( ",: pivot points on/off" );
   showBitmapString( "^: cone on/off" );
   showBitmapString( "" );

   showBitmapString( "5,0: zoom in, out" );
   showBitmapString( "2,4,6,8: pan" );
   showBitmapString( "y,Y: change yaw   view angle" );
   showBitmapString( "p,P: change pitch view angle" );
   showBitmapString( "" );

   showBitmapString( "s: save state" );
   showBitmapString( "r: revert to saved state" );

   glPopMatrix();
}
#endif

/**************************************************************************************/
/**************************************************************************************/
// OBJECTS

/**************************************************************************************/
void
initObs( void )
{
   numObs = 0;

   // create tip object
         // tip length of 0.001 is workaround for prob with zero tip length
   addObject( numObs, TIP,  Vec2d( 0., 0.), 0., 0., 0.001, 5.,   NULLOB, NULLOB, 0. );

   // create phantom
   addObject( numObs, PHANTOM, Vec2d( 105., 125.), 0., 0.,40., 4., NULLOB, NULLOB, 0. );

   // To start with, set the previous state (used in computing movements) 
   // and saved state (used in undo)
   // to be the initial state.  
   setState( &prevState );
   setState( &saveState );

   // init goal
   currentGoal = nullob;
   currentGoalTube = 0;
}


/**************************************************************************************/
void
addObject( int obNum, 
         int type, Vec2d pos, float yaw, float roll, float leng, float diam,
         int nextSeg, int prevSeg, float bendAngle )
{
   ob[obNum].type  = type;
   ob[obNum].pos   = pos;
   ob[obNum].angle = yaw;
   ob[obNum].roll  = roll;
   ob[obNum].leng  = leng;
   ob[obNum].diam  = diam;

   ob[obNum].nextSeg    = nextSeg;
   ob[obNum].prevSeg    = prevSeg;
   ob[obNum].bendAngle  = bendAngle;

   numObs++;
}


/**************************************************************************************/
// add a new object into the world
void
newOb( void )
{
   // check whether there is room in object data structure
   if( numObs < MAXOB ) {
      // add new object to end of current object list.
      // Put it at current mouse location.
      addObject( numObs, TUBE, ob[selectedOb].pos, ob[selectedOb].angle,
                         ob[selectedOb].roll, ob[selectedOb].leng,
                         ob[selectedOb].diam, NULLOB, NULLOB, 0. );
   }
   else {
//      error( "Tried to add one too many objects" );
   }
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
   if( ob[n].type != TUBE )      return;

   // overwrite ob n with ob n+1, and shift all higher objects down one slot
   // in the array.
   for( int i = n+1; i<numObs; i++ ) {
      ob[i-1].type  = ob[i].type;
      ob[i-1].pos   = ob[i].pos;
      ob[i-1].angle = ob[i].angle;
      ob[i-1].roll  = ob[i].roll;
      ob[i-1].leng  = ob[i].leng;
      ob[i-1].diam  = ob[i].diam;
   }

   // reduce number of objects by one.
   numObs--;

   // if the object deleted was the selected object, then there
   // is now no object selected.
   if( n == selectedOb ) {
      selectedOb = NULLOB;
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

void stepImgScan(void)
{
   float height(0);
   //if (!scanPathIdx)
   //{
      for (int i(0); i < 5; i++)
      {
      getImageHeightAtXYLoc(frontPathQ().x, frontPathQ().y, &(scanPathZ[scanPathIdx]));
      height += scanPathZ[scanPathIdx];
      }
   //}

   scanPathZ[scanPathIdx] = height / 5.0;

   //getImageHeightAtXYLoc(frontPathQ().x, frontPathQ().y, &(scanPathZ[scanPathIdx]));

   scanPath[scanPathIdx++] = frontPathQ();
   ob[TIP].pos = frontPathQ();
   popFrontPathQ();
}

/**************************************************************************************/
// Issue tip movement commands based on robot's goals.
void
robotExecutive( void )
{
   if (robotImgPhase)
   {
      simRunning = 0;
      if (emptyPathQ())
      {
         switch(robotImgPhase)
         {
            case 1:
               predictedTubePose = ob[currentGoalTube];
               buildScan1Path();
               break;
            case 2:
               buildScan2Path();
               break;
            case 3:
               buildScan3Path();
               break;
         }

#ifdef NMRC_LIB
         g_iViewer->dirtyWindow(g_iViewerWin);
#endif
      }

      stepImgScan();

      if (emptyPathQ())
      {
         switch(robotImgPhase)
         {
            case 1:
               analizeScan1Path();
               break;
            case 2:
               analizeScan2Path();
               break;
            case 3:
               analizeScan3Path();
               break;
         }
         robotImgPhase = (robotImgPhase + 1) % 4;
         scanPathIdx = 0;
      }
   }
   else
   {
      simRunning = 1;
      vrpn_bool testResult(vrpn_false);
      switch( robotModPhase ) {
         case 1:
            testResult = rotatedEnoughTest();
            break;
         case 2:
            testResult = translatedEnoughTest();
            break;
         case 3:
            testResult = rotatedEnoughTest2();
            break;
      }

      if( testResult ) {
         clearPathQ();

         robotModPhase++; 

         if (robotModPhase > 3)
         {
            robotModPhase = robotRunning = 0;
#ifdef NMRC_LIB
            g_microscope->ResumeScan();
            g_activeMode = SCAN_MODE;
            g_activeControl = vrpn_false;
#endif
         }

#ifdef NMRC_LIB
      g_iViewer->dirtyWindow(g_iViewerWin);
#endif
         return;
      }


      // test whether tip has arrived at target point
      float gotThereThreshold = 1.;
      if( !emptyPathQ() &&
            distance( ob[TIP].pos, frontPathQ() ) <= gotThereThreshold ) {

         popFrontPathQ();

         if (emptyPathQ())
         {
            robotImgPhase = 1;
#ifdef NMRC_LIB
            g_iViewer->dirtyWindow(g_iViewerWin);
#endif
            return;
         }
      }


      // if the target point queue is empty, stop robot
      if( emptyPathQ() ) {
         // load new tip path
         switch( robotModPhase ) {
            case 1: moveTipToRotateTube();    break;
            case 2: moveTipToTranslateTube(); break;
            case 3: moveTipToRotateTube();    break;
         }
      }


      // We have not yet gotten to this target point, so
      // keep moving toward it.  
      moveTipTowardPoint( frontPathQ() );
   }

#ifdef NMRC_LIB
   g_iViewer->dirtyWindow(g_iViewerWin);
#endif
}


/**************************************************************************************/
// Start and stop the robot from the keyboard
void
startRobot( void )
{
   if( !robotRunning  &&  emptyPathQ() ) {
      robotImgPhase = 1;
      robotModPhase = 1;

      robotExecutive();
   }
   
   robotRunning = ! robotRunning;   
}

float angleDelta(float angle1, float angle2)
{
   while (angle1 < 0.) angle1 = 2.*M_PI + angle1;
   while (angle2 < 0.) angle2 = 2.*M_PI + angle2;

   float delta = fmod(angle1,M_PI) - fmod(angle2,M_PI);

   if (delta >=  M_PI * .5) delta =  M_PI - delta;
   if (delta <= -M_PI * .5) delta = -M_PI - delta;

   return delta;
}

/**************************************************************************************/
vrpn_bool
rotatedEnoughTest2( void )
{
   float deltaAngle = angleDelta(currentGoal.angle,ob[currentGoalTube].angle);
   float misorientationThreshold = M_PI/6.;

   return fabs(deltaAngle) <= misorientationThreshold;
}


/**************************************************************************************/
vrpn_bool
translatedEnoughTest( void )
{
   // if tube being moved is within threshold distance
   // of target position, return TRUE.
   Vec2d vTube     = ob[currentGoalTube].pos;
   Vec2d vTarget   = currentGoal.pos;
   float distToTarget = distance( vTarget, vTube );
   float distanceThreshold = ob[currentGoalTube].leng/8.;
   
   return distToTarget <= distanceThreshold;
//   return 0;
}


/**************************************************************************************/
vrpn_bool
rotatedEnoughTest( void )
{
   // Compare goal orientation with current tube orientation
   // The goal orientation has the tube normal to the path 
   // to the goal position.
   Vec2d vTube     = ob[currentGoalTube].pos;
   Vec2d vTarget   = currentGoal.pos;
   Vec2d vTowardTarget = vTarget - vTube;
   Vec2d vNormal = vTowardTarget;

   vNormal.rotate( -M_PI/2. );

   // test to see whether angles agree within threshold
   float deltaAngle = angleDelta(atan2( vNormal.y, vNormal.x ),ob[currentGoalTube].angle);

   float misorientationThreshold = M_PI/8.;

   return fabs(deltaAngle) <= misorientationThreshold;
}


/**************************************************************************************/
void
moveTipToTranslateTube( void )
{
   //predictedTubePose = ob[currentGoalTube];
      // (for now, just cheat and look in simulator data structure).
   //measuredTubePose = locateTube( predictedTubePose );
      // (for now, just cheat and look in simulator data structure).




   // figure direction to goal position
   Vec2d vTarget   = currentGoal.pos;
   Vec2d vDesiredTranslationDirection = vTarget - measuredTubePose.pos;

   // calc perpendicular to translation direction
   // and construct a desired pose fro which to calculate
   // pushes that keep the tube going toward the target.
   Vec2d vAdjustedAxis = vDesiredTranslationDirection; vAdjustedAxis.rotate(-M_PI/2.);
   float desiredOrientation = atan2( vAdjustedAxis.y, vAdjustedAxis.x );
   OB alignedPose = measuredTubePose;
   alignedPose.angle = desiredOrientation;


   // calc points, vectors, and constants needed
   Vec2d vC, vAxis, vWidth, vRightTouch, vLeftTouch, vTopTouch, vBottomTouch;
   calcTubePointsAndVectors( alignedPose, &vC, &vAxis, &vWidth,
            &vRightTouch, &vLeftTouch, &vTopTouch, &vBottomTouch );


   float turnAngle = M_PI/6.;   // how much to rotate tube each push
   Vec2d vPivotArm = vAxis * 0.5;
   float pushLength = norm(vPivotArm) * tan(turnAngle);
   Vec2d vPush = normalizeVec(vWidth) * pushLength;

   Vec2d vRight  = vRightTouch  * 1.2;
   //Vec2d vLeft   = - vRight;
   Vec2d vTop    = vRight;   vTop.rotate(M_PI/2.);
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
   Vec2d tipPos = ob[TIP].pos;
   Vec2d vPerimeter = normalizeVec(tipPos - vC) * norm(vRightTouch * 1.5);
   float misalignAngle = calcAngleInPlane( vC + vPerimeter, vC, vC + vBottom );

   addPointToTipPath( vC + vPerimeter );

   // move along the perimeter circle in 3 steps
   // to arrive at position (vC + vBottom).
   vPerimeter.rotate( misalignAngle/3. );
   addPointToTipPath( vC + vPerimeter );
   vPerimeter.rotate( misalignAngle/3. );
   addPointToTipPath( vC + vPerimeter );
//   vPerimeter.rotate( misalignAngle/3. );
//   addPointToTipPath( vC + vPerimeter );


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
   *vWidth  = Vec2d(1.,0.).rotate( tube.angle + M_PI/2. ) * (tube.diam/2.);

   // calc points at which tip is just touching the tube based on 
   // current estimates of tube length and width, and tip radius.
   float tipRadius  = ob[TIP].diam/2.;   // XXX peek into sim data for now
   float tubeRadius = tube.diam/2.;
   float touchDist = touchDistance( tipRadius, tubeRadius );

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
   //predictedTubePose = ob[currentGoalTube];
      // (for now, just cheat and look in simulator data structure).


   // Measure where tube is using a few linear image scans
   // (not an entire raster scan, just a few scans gathering
   //  height data along selected paths).  
   //measuredTubePose = locateTube( predictedTubePose );


   // calc points and vectors needed for rotating tube
   Vec2d vC, vAxis, vWidth, vRightTouch, vLeftTouch, vTopTouch, vBottomTouch;
   calcTubePointsAndVectors( measuredTubePose, &vC, &vAxis, &vWidth,
            &vRightTouch, &vLeftTouch, &vTopTouch, &vBottomTouch );

   // calculate vectors and constants needed
   float turnAngle = M_PI/6.;
   Vec2d vPivotArm = vAxis * 0.5;
   float pushLength = norm(vPivotArm) * tan(turnAngle);
   Vec2d vPush = normalizeVec(vWidth) * pushLength;

   Vec2d vRight  = vRightTouch * 1.2;
   Vec2d vLeft   = - vRight;
   Vec2d vTop    = vRight;   vTop.rotate(M_PI/2.);
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
   Vec2d tipPos = ob[TIP].pos;
   Vec2d vPerimeter = normalizeVec(tipPos - vC) * norm(vRightTouch * 1.5);
   float misalignAngle = calcAngleInPlane( vC + vPerimeter, vC, vC + vBottom );

   addPointToTipPath( vC + vPerimeter );

   // move along the perimeter circle in 3 steps
   // to arrive at position (vC + vBottom).
   vPerimeter.rotate( misalignAngle/3. );
   addPointToTipPath( vC + vPerimeter );
   vPerimeter.rotate( misalignAngle/3. );
   addPointToTipPath( vC + vPerimeter );
//   vPerimeter.rotate( misalignAngle/3. );
//   addPointToTipPath( vC + vPerimeter );

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
// measure tube position

void buildScan1Path(void)
{
   Vec2d predictedPos(predictedTubePose.pos);

   float predictedLeng(predictedTubePose.leng),
         predictedDiam(predictedTubePose.diam);

   Vec2d tubeCoreEst(cos(predictedTubePose.angle),sin(predictedTubePose.angle)),
         tubeScanCenter(predictedPos + tubeCoreEst * predictedLeng * 0.25),
         tubeScanDir(tubeCoreEst.rotate(90 * DEG_TO_RAD)),
         tubeScanStart(tubeScanCenter - tubeScanDir * predictedDiam * 2.0),
         tubeScanStep(tubeScanDir * predictedDiam * 4.0 / (float)(MAX_PATHQ-1));

   for (int i(0); i < MAX_PATHQ; i++)
   {
      addPointToTipPath(tubeScanStart + (tubeScanStep * (float)i));
   }
}

void analizeScan1Path(void)
{
   float scanStep((scanPathZ[MAX_PATHQ-1] - scanPathZ[0]) /
                  (float)(MAX_PATHQ-1));

   int i;

   for (i=0; i < MAX_PATHQ; i++)
      scanPathZ[i] -= scanStep * (float)i;

   scan1MaxZ = -100000.0;
   float scanFloor = 100000.0;

   for (i=0; i < MAX_PATHQ; i++)
   {
      if (scanPathZ[i] > scan1MaxZ)
      {
         scan1MaxZ = scanPathZ[i];
         scan1Max  = scanPath[i];
      }

      if (scanPathZ[i] < scanFloor)
      {
         scanFloor = scanPathZ[i];
      }
   }

   scan1MaxZ -= scanFloor;

   for (i=0; i < MAX_PATHQ; i++)
      scanPathZ[i] -= scanFloor;

   scan1MinZ = -100000.0;

   for (i=0; i < MAX_PATHQ; i++)
   {
      if ((scanPathZ[i] > scan1MinZ) && (scanPathZ[i] < scan1MaxZ * 0.8))
      {
         scan1MinZ = scanPathZ[i];
         scan1Min  = scanPath[i];
      }
   } 

   scannedTubePose.type = TUBE;
   scannedTubePose.roll = 0.0;
   scannedTubePose.nextSeg = 0;
   scannedTubePose.prevSeg = 0;
   scannedTubePose.bendAngle = 0.0;
   scannedTubePose.moved = 0;
}

void buildScan2Path(void)
{
   Vec2d predictedPos(predictedTubePose.pos);

   float predictedLeng(predictedTubePose.leng),
         predictedDiam(predictedTubePose.diam);

   Vec2d tubeCoreEst(cos(predictedTubePose.angle),sin(predictedTubePose.angle)),
         tubeScanCenter(predictedPos - tubeCoreEst * predictedLeng * 0.25),
         tubeScanDir(tubeCoreEst.rotate(90 * DEG_TO_RAD)),
         tubeScanStart(tubeScanCenter + tubeScanDir * predictedDiam * 2.0),
         tubeScanStep(tubeScanDir * predictedDiam * 4.0 / (float)(MAX_PATHQ-1));

   for (int i(0); i < MAX_PATHQ; i++)
   {
      addPointToTipPath(tubeScanStart - (tubeScanStep * (float)i));
   }
}

void analizeScan2Path(void)
{
   float scanStep((scanPathZ[MAX_PATHQ-1] - scanPathZ[0]) /
                  (float)(MAX_PATHQ-1));

   int i;

   for (i = 0; i < MAX_PATHQ; i++)
      scanPathZ[i] -= scanStep * (float)i;

   scan2MaxZ = -100000.0;
   float scanFloor = 100000.0;

   for (i=0; i < MAX_PATHQ; i++)
   {
      if (scanPathZ[i] > scan2MaxZ)
      {
         scan2MaxZ = scanPathZ[i];
         scan2Max  = scanPath[i];
      }

      if (scanPathZ[i] < scanFloor)
      {
         scanFloor = scanPathZ[i];
      }
   }

   scan2MaxZ -= scanFloor;
   for (i=0; i < MAX_PATHQ; i++)
      scanPathZ[i] -= scanFloor;

   scan2MinZ = -100000.0;

   for (i=0; i < MAX_PATHQ; i++)
   {
      if ((scanPathZ[i] > scan2MinZ) && (scanPathZ[i] < scan2MaxZ * 0.8))
      {
         scan2MinZ = scanPathZ[i];
         scan2Min  = scanPath[i];
      }
   } 
 
   
   Vec2d coreVector(scan2Max - scan1Max);

   scannedTubePose.diam = 0.5 * (norm(scan1Max - scan1Min) + norm(scan2Max - scan2Min));
   scannedTubePose.angle = atan2(coreVector.y, coreVector.x);
}

void buildScan3Path(void)
{
   float predictedLeng(predictedTubePose.leng);

   Vec2d coreVector(scan2Max - scan1Max),
         coreCenter((scan2Max + scan1Max) * 0.5),
         ridgeStart(coreCenter + normalizeVec(coreVector) * predictedLeng * 1.5),
         ridgeStep(normalizeVec(coreVector) * predictedLeng * 3.0 / (float)(MAX_PATHQ-1));

   for (int i(0); i < MAX_PATHQ; i++)
   {
      addPointToTipPath(ridgeStart - (ridgeStep * (float)i));
   }
}

void analizeScan3Path(void)
{
   float scanStep((scanPathZ[MAX_PATHQ-1] - scanPathZ[0]) /
                  (float)(MAX_PATHQ-1));

   int i;

   for (i = 0; i < MAX_PATHQ; i++)
      scanPathZ[i] -= scanStep * (float)i;

   float scanCeil  = -100000.0;
   float scanFloor =  100000.0;

   for (i=0; i < MAX_PATHQ; i++)
   {
      if (scanPathZ[i] > scanCeil)
      {
         scanCeil = scanPathZ[i];
      }

      if (scanPathZ[i] < scanFloor)
      {
         scanFloor = scanPathZ[i];
      }
   }

   scanCeil -= scanFloor;
   for (i=0; i < MAX_PATHQ; i++)
      scanPathZ[i] -= scanFloor;

   for (i=0; i < MAX_PATHQ; i++)
   {
      if ((scanPathZ[i] > scan2MinZ) && (scanPathZ[i] < scan2MaxZ * 0.8))
      {
         scan2MinZ = scanPathZ[i];
         scan2Min  = scanPath[i];
      }
   } 
 

   scan3Min1Z = scan3Min2Z = -100000.0;

   int half(MAX_PATHQ/2);

   for (i=0; i <= half; i++)
   {
      if ((scanPathZ[i] > scan3Min1Z) && (scanPathZ[i] < scanCeil * 0.8))
      {
         scan3Min1  = scanPath[i];
         scan3Min1Z = scanPathZ[i];
      }

      if ((scanPathZ[i+half] > scan3Min2Z) && (scanPathZ[i+half] < scanCeil * 0.8))
      {
         scan3Min2  = scanPath[i+half];
         scan3Min2Z = scanPathZ[i+half];
      }
   }

   // calc length, mid point

   scannedTubePose.leng = norm(scan3Min1 - scan3Min2);
   scannedTubePose.pos  = (scan3Min1 + scan3Min2) * 0.5;

   // return measured position OB
   measuredTubePose = scannedTubePose;
}


/**************************************************************************************/
// return point on front of queue
Vec2d
frontPathQ( void )
{
   assert( !emptyPathQ(), "access to empty pathQ" );
   return pathQ[0];   // element 0 is front of queue
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
   assert( pathQSize < MAX_PATHQ, "pathQ overflow" );

   pathQ[ pathQSize ] = vPathPoint;
   pathQSize++;
}


/**************************************************************************************/
// pop point off front of queue
void
popFrontPathQ( void )
{
   assert( !emptyPathQ(), "attempt to pop empty pathQ" );
   for( int i=0; i<pathQSize-1; i++ ) {
      pathQ[i] = pathQ[i+1];
   }
   pathQSize--;
}


/**************************************************************************************/
// check if queue empty
vrpn_bool
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
//   float speed = 4.;
   Vec2d vTowardGoal = towardVec( vTarget, ob[TIP].pos, tipSpeed );
   Vec2d vTip = ob[TIP].pos + vTowardGoal;
   moveTipToXYLoc( vTip.x,  vTip.y );

}


/**************************************************************************************/
// Calculate velocity vector toward position vToward from position vFrom
// with given speed.
Vec2d
towardVec( Vec2d vTarget, Vec2d vFrom, float speed )
{
   Vec2d vDelta = vTarget - vFrom;
   float magnitude = sqrt( vDelta.x * vDelta.x  +  vDelta.y * vDelta.y );
   assert( speed >= 0., "speed must be non-negative" );
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
   float length   = tube.leng;
   float diameter = tube.diam;

#define BOX_BORDER_WIDTH 2.0
   glPushMatrix();
   glScalef( length/2. + diameter/2. + BOX_BORDER_WIDTH, 
            diameter/2. + BOX_BORDER_WIDTH, 
           1. );
   drawPolygon(); // square centered at origin, width 2
   glPopMatrix();

   glPopMatrix();
}


/**************************************************************************************/
void
showRobotStuff( void )
{
   // display rectangle indicating goal position for tube
   showBox( currentGoal, RED );

        // display rectangle indicating simultaed position for tube
   showBox( predictedTubePose, GREEN );

   // display rectangle indicating last measured position of tube
   showBox( measuredTubePose, YELLOW );



   //display waypoints for current tip movement (pathQ)
   // and lines between them
   Vec2d vPrevPoint = ob[TIP].pos;
   for( int j=0; j<pathQSize; j++ ) {
      //float tipRadius = ob[TIP].diam/2.;
//      showSphere( pathQ[j], MAGENTA,  tipRadius  );
      drawLine( vPrevPoint, pathQ[j], MAGENTA );
      vPrevPoint = pathQ[j];
   }
   if (robotImgPhase)
   {
      vPrevPoint = ob[TIP].pos;
      float prevZ = scanPathZ[scanPathIdx-1];
      for (int j=scanPathIdx - 1; j >= 0; j--)
      {
         drawLine(vPrevPoint, scanPath[j], CYAN, prevZ, scanPathZ[j]);
         vPrevPoint = scanPath[j];
         prevZ = scanPathZ[j];
      }

      if (robotImgPhase != 1)
      {
         Vec2d smp(scan1Max * 2.0 - scan1Min);

         drawLine(scan1Max, scan1Max, YELLOW, 0.0, scan1MaxZ);
         drawLine(scan1Min, scan1Min, YELLOW, 0.0, scan1MinZ);
         drawLine(smp, smp, YELLOW, 0.0, scan1MinZ);

         showSphere(scan1Max, YELLOW, 0.2, scan1MaxZ);
         showSphere(scan1Min, YELLOW, 0.2, scan1MinZ);
         showSphere(smp, YELLOW, 0.2, scan1MinZ);

         if (robotImgPhase != 2)
         {
            Vec2d smp(scan2Max * 2.0 - scan2Min);

            drawLine(scan2Max, scan2Max, YELLOW, 0.0, scan2MaxZ);
            drawLine(scan2Min, scan2Min, YELLOW, 0.0, scan2MinZ);
            drawLine(smp, smp, YELLOW, 0.0, scan2MinZ);

            showSphere(scan2Max, YELLOW, 0.2, scan2MaxZ);
            showSphere(scan2Min, YELLOW, 0.2, scan2MinZ);
            showSphere(scan2Max * 2.0 - scan2Min, YELLOW, 0.2, scan2MinZ);
         }
      }
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
moveTipToXYLoc( float x, float y )
{
   // move tip
   Vec2d vNewTipPosition = Vec2d(x,y);
   ob[TIP].pos = vNewTipPosition;

   
#ifdef NMRC_LIB
   if (g_activeControl)
   {
      if (g_activeMode != MODIFY_MODE)
      {
         g_microscope->ModifyMode();

         g_activeMode = MODIFY_MODE;
      }

      double wX, wY;

      nmb_Image *hImage = g_dataset->dataImages->getImageByName(
                                g_dataset->heightPlaneName->string());

      hImage->pixelToWorld(x, y, wX, wY);

      g_microscope->TakeModStep(wX, wY);
   }
#endif

   // respond to new tip position
   selectedOb = TIP;  // this is need to get simStep to work right
   simStep();


   // XXX Call to nano to move the tip in contact mode

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
#ifndef NMRC_LIB
void
showLabeledPoint( Vec2d vPoint, char* labelStr )
{
   glPushMatrix();
   glTranslatef( vPoint.x, vPoint.y, 0. );

   showBitmapString( labelStr );

   glPopMatrix();
}
#endif


/**************************************************************************************/
// This routine is a substitute for glRotatef().
// It is the same except it uses radians rather than degrees for angular measure.
void
rotateRadiansAxis( float angleInRadians, 
               float axisComponentX, float axisComponentY, float axisComponentZ )
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
   case WHITE  : setMaterialColor(1.0, 1.0, 1.0); break;
   case RED    : setMaterialColor(1.0, 0.0, 0.0); break;
   case GREEN  : setMaterialColor(0.0, 1.0, 0.0); break;
   case BLUE   : setMaterialColor(0.0, 0.0, 1.0); break;
   case MAGENTA: setMaterialColor(1.0, 0.0, 1.0); break;
   case YELLOW : setMaterialColor(1.0, 1.0, 0.0); break;
   case CYAN   : setMaterialColor(0.0, 1.0, 1.0); break;
   case BLACK  : setMaterialColor(0.0, 0.0, 0.0); break;
   case ORANGE : setMaterialColor(1.0, 0.5, 0.0); break;
   }
}


/**************************************************************************************/
#ifndef NMRC_LIB
void
drawCube( float halfWidth )
{
   switch( renderStyle ) {
   default:
   case OB_SOLID:         glutSolidCube( halfWidth );   break;

   case OB_POINTS:      
   case OB_SILHOUETTE:   
   case OB_OUTLINE2D:      
   case OB_WIREFRAME:      glutWireCube(  halfWidth );   break;
   }
}


/**************************************************************************************/
void
drawTorus( float innerDiameter, float outerDiameter )
{
   switch( renderStyle ) {
   default:
   case OB_SOLID:         glutSolidTorus( innerDiameter, outerDiameter, 20, 16);   break;
                     // see p659 Woo 3ed
   case OB_POINTS:      
   case OB_SILHOUETTE:   
   case OB_WIREFRAME:      glutWireTorus(  innerDiameter, outerDiameter, 20, 16);   break;

   case OB_OUTLINE2D:      glutWireTorus(  innerDiameter, outerDiameter, 2, 16);   break;
   }
}
#endif


/**************************************************************************************/
void
drawSphere( float radius )
{
   static int firstTime = 1;
   static GLUquadricObj* qobj;
   if( firstTime ) { qobj = gluNewQuadric(); }

   switch( renderStyle ) {
   default:
   case OB_SOLID:      drawStyle = (GLenum)GLU_FILL;      goto sphere3D;
   case OB_WIREFRAME:   drawStyle = (GLenum)GLU_LINE;      goto sphere3D;
   case OB_POINTS:      drawStyle = (GLenum)GLU_POINT;      goto sphere3D;
   case OB_SILHOUETTE:   drawStyle = (GLenum)GLU_SILHOUETTE;   goto sphere3D;
   sphere3D:
      gluQuadricDrawStyle( qobj, drawStyle );
      gluQuadricNormals( qobj, (GLenum)GLU_FLAT );
      gluSphere( qobj, radius, 10, 10);
      break;

   case OB_OUTLINE2D:   
      gluQuadricDrawStyle( qobj, (GLenum)GLU_LINE );
      gluQuadricNormals( qobj, (GLenum)GLU_FLAT );
      gluCylinder( qobj, radius, radius, 0.001, 10, 1);  // draw short cylinder
      break;

   case OB_NONE:
      break;
   }
}


/**************************************************************************************/
void
showPoint( Vec2d vPoint, int color, float size /* = 1. */, float zHeight /* = 0. */ )
{
   glPushMatrix();

   glTranslatef( vPoint.x, vPoint.y, zHeight );
   setColor( color );
   drawSphere( 0.1 * size ); 

   glPopMatrix();
}


/**************************************************************************************/
void
showSphere( Vec2d vPoint, int color, float radius, float z )
{
   glPushMatrix();

   glTranslatef( vPoint.x, vPoint.y, 0. );
   setColor( color );
   drawSphere( radius ); 

   glPopMatrix();
}


/**************************************************************************************/
void
drawCone( float radius, float height )
{
   static int firstTime = 1;
   static GLUquadricObj* qobj;
   if( firstTime ) { qobj = gluNewQuadric(); }

   switch( renderStyle ) {
   default:
   case OB_SOLID:      drawStyle = (GLenum)GLU_FILL;      goto cone3D;
   case OB_WIREFRAME:   drawStyle = (GLenum)GLU_LINE;      goto cone3D;
   case OB_POINTS:      drawStyle = (GLenum)GLU_POINT;      goto cone3D;
   case OB_SILHOUETTE:   drawStyle = (GLenum)GLU_SILHOUETTE;   goto cone3D;
   cone3D:
      gluQuadricDrawStyle( qobj, drawStyle );
      gluQuadricNormals( qobj, (GLenum)GLU_FLAT );
      gluCylinder( qobj, radius, 0., height, 10, 10); 
      break;

   case OB_OUTLINE2D:   
      gluQuadricDrawStyle( qobj, (GLenum)GLU_LINE );
      gluQuadricNormals( qobj, (GLenum)GLU_FLAT );
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
drawCylinder( float diameter, float height )
{
   float radius;
   static int firstTime = 1;
   static GLUquadricObj* qobj;
   if( firstTime ) { qobj = gluNewQuadric(); }

   radius = diameter / 2.;

   switch( renderStyle ) {
   default:
   case OB_SOLID:      drawStyle = (GLenum)GLU_FILL;      goto cylinder3D;
   case OB_WIREFRAME:   drawStyle = (GLenum)GLU_LINE;      goto cylinder3D;
   case OB_POINTS:      drawStyle = (GLenum)GLU_POINT;         goto cylinder3D;
   case OB_SILHOUETTE:   drawStyle = (GLenum)GLU_SILHOUETTE;   goto cylinder3D;
   cylinder3D:
      gluQuadricDrawStyle( qobj, drawStyle );
      gluQuadricNormals( qobj, (GLenum)GLU_FLAT );
      gluCylinder( qobj, radius, radius, height, 12, 1);   // see p489 Woo 3ed
            // we want even # of slices (currently 12), 1 stack
      break;

   case OB_OUTLINE2D:   
      gluQuadricDrawStyle( qobj, (GLenum)GLU_LINE );
      gluQuadricNormals( qobj, (GLenum)GLU_FLAT );
      gluCylinder( qobj, radius, radius, height, 2, 1);  // 2 slices, 1 stack = rect
      break;

   case OB_NONE:
      break;
   }

}


/**************************************************************************************/
void
drawPolygon( void )
{
   int GLmode;      // see p43 Woo 3rd ed

   switch( renderStyle ) {
   default:
   case OB_SOLID:      GLmode = GL_POLYGON;      break;
   case OB_WIREFRAME:   GLmode = GL_LINE_LOOP;      break;
   case OB_POINTS:      GLmode = GL_POINTS;         break;
   case OB_SILHOUETTE: GLmode = GL_LINE_LOOP;      break;
   case OB_OUTLINE2D:  GLmode = GL_LINE_LOOP;      break;
   }

   glPushMatrix();
//   glScalef( 0.2, 0.2, 0.2 );

   glBegin( (GLenum)GLmode );
   glVertex3f(-1., -1.,  0.);
   glVertex3f( 1., -1.,  0.);
   glVertex3f( 1.,  1.,  0.);
   glVertex3f(-1.,  1.,  0.);
   glEnd();

   glPopMatrix();
}


/**************************************************************************************/
void
drawLine( Vec2d pt1, Vec2d pt2, int color, float z1 /* = 0. */, float z2 /* = 0. */ )
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
drawTip( float tipDiameter )
{
   float coneBaseWidth = tipDiameter * 2.0;
   float coneHeight    = tipDiameter * 3.0;

   // make sure the tip gets drawn, even if object drawing is disabled (OB_NONE)
   int saveRenderStyle = renderStyle;
   if( renderStyle == OB_NONE )   renderStyle = OB_WIREFRAME;

   glPushMatrix();
   {
      // draw sphere at tip center with appropriate radius of curvature
      drawSphere( tipDiameter / 2. ); 

      // also draw cone, if not in 2D display mode
      if( displayConeOn  &&  renderStyle != OB_OUTLINE2D ) {
         // rotate cone to point downward
         rotateRadiansAxis( M_PI, 0.0, 1.0, 0.0 );

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
drawTube( float diameter, float length, int obIndex )
{
   glPushMatrix();

      if( renderStyle == OB_OUTLINE2D ) {
         // draw a little section of 3D cylinder in the middle of tube
         // to show when rolling occurs.
         int saveRenderStyle = renderStyle;
         renderStyle = OB_SILHOUETTE;
      
         // draw cylinder with its axis parallel to X-axis
         float sectionLength = length / 10.;
         glPushMatrix();
         rotateRadiansAxis( M_PI/2., 0.0, 1.0, 0.0 );       // lay the tube over parallel to X-axis
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
      rotateRadiansAxis( M_PI/2., 0.0, 1.0, 0.0 ); // lay the tube over parallel to X-axis
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
drawSubstrate( void )
{
   drawPolygon();    // this is just a stand-in
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

         //   char str[256];
         //   sprintf( str, "%f", k );
         //   if( streq( str, "NaN" ) ) {
         //      printf( "k: %f\n",                 k );
         //      printf( "vParallel: (%f,%f)\n",    vParallel.x, vParallel.y );
         //      error( "NaN in rollingCollisionResponse" );
         //   }

/**************************************************************************************/
// if the condition is not true, invoke error routine with the message msg.
//   Sample call: assert( 0, "test" );
void
assert( int condition, char* msg )
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
         //   if( streq( "abc", "abc" ) )  error( "streq works" );
         //   if( streq( "abc", "xyz" ) )  error( "streq doesn't work" );


/**************************************************************************************/
// Return vector interpolated between two given vectors v0 and v1.
// If fraction is zero, return v0; if fraction is one return v1.
// For other fractions, return vectors between v0 and v1.
// This routine also works if fraction < 0 or fraction > 1.
Vec2d
interpolateVecs( Vec2d v0, Vec2d v1, float fraction )
{
   return (v0 * (1. - fraction))  +  (v1 * fraction);
}


/**************************************************************************************/
// Return scalar interpolated between two given scalars s0 and s1.
// If fraction is zero, return s0; if fraction is one return s1.
// For other fractions, return scalars between s0 and s1.
// This routine also works if fraction < 0 or fraction > 1.
float
interpolateScalars( float s0, float s1, float fraction )
{
   return (s0 * (1. - fraction))  +  (s1 * fraction);
}


/**************************************************************************************/
// draw char string with strokes, starting at current origin.
// String is 3D geometric object -- can be scaled, rotated, etc.
#ifndef NMRC_LIB
void
showStrokeString( char* str )
{
   glPushMatrix();
   float scaleFactor = 1. / 150.;
   glScalef( scaleFactor, scaleFactor, scaleFactor );
   
   for( char* s = str; *s!=0; s++ ) {
      glutStrokeCharacter( GLUT_STROKE_MONO_ROMAN, *s );
   }
   glPopMatrix();


   float textLineSeparation = (- 1.0);
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

   float textLineSeparation = (- 0.5);
   glTranslatef( 0., textLineSeparation, 0. );
}
#endif


/**************************************************************************************/
float
norm( Vec2d v )
{
   return sqrt( (v.x * v.x)  +  (v.y * v.y) );
}


/**************************************************************************************/
float
distance( Vec2d pt1, Vec2d pt2 )
{
   return norm( pt1 - pt2 );
}


/**************************************************************************************/
// return normalized vector
Vec2d
normalizeVec( Vec2d v )
{
   Vec2d normalizedV = v;

   float normOfV = norm( v );
   if( normOfV != 0. )    normalizedV /= normOfV;

   return normalizedV;
}


/**************************************************************************************/
// normalize argument vector
void
normalize( float* px, float* py, float* pz )
{
   // calc magnitude of vector
   float leng = sqrt( (*px)*(*px) + (*py)*(*py) + (*pz)*(*pz) );

   if( leng == 0 )
      return;

   // divide each vector component by the magnitude.
   *px /= leng;
   *py /= leng;
   *pz /= leng;
}


/**************************************************************************************/
void
crossProduct( float  x1, float  y1, float  z1,
           float  x2, float  y2, float  z2,
           float* px, float* py, float* pz )
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
#ifndef NMRC_LIB
void
timerFunc(int value)
{
   static int prevFrameCount;
   float elapsedFrames = (float)(frameCount - prevFrameCount);
   float elaspedTime   = 1.;   // seconds between timer callbacks
   int callbackIntervalInMilliseconds = (int)(elaspedTime * 1000.);

   framesPerSecond = elapsedFrames / elaspedTime;

   // Save the current frame count for next time through this routine.  
   prevFrameCount = frameCount;

   // The timer needs to call itself again some number of milliseconds
   // in the future.  This ensures that the timer is called 
   // at regular intervals.  
   glutTimerFunc( callbackIntervalInMilliseconds, timerFunc, 0 );
}
#endif


/**************************************************************************************/
/**************************************************************************************/
// COLLISION DETECTION

/**************************************************************************************/
void
collisionStuff( void )
{
   // Move the selected object only if there is one selected (it can be null).
   if( selectedOb != NULLOB && ob[selectedOb].type != PHANTOM ) {
      
      // if the cursor moved too far, break the movement into smaller steps.
      float jumpMax = 1.0;   // maximum jump distance, per step.

      // Calc previous heldob position and new position.  
      // If the jump is too big (ie it will teleport right past
      // objects it should collide with) then break the jump into
      // several smaller steps.
      Vec2d oldSelectedObPos = prevState.ob[selectedOb].pos;
      Vec2d newSelectedObPos =           ob[selectedOb].pos;

//      showPoint( oldSelectedObPos, MAGENTA, 3. );
//      showPoint( newSelectedObPos, YELLOW,  3. );

      float jumpLength = distance( newSelectedObPos, oldSelectedObPos );
      int truncateJumps = (int)(jumpLength / jumpMax);
      int numSteps;
      if( truncateJumps < 1 )   numSteps = 1;
      else                      numSteps = 1 + truncateJumps;
      assert( numSteps >= 1, "numSteps: bad value" );


      // Draw a line from old pos to new pos.  
      // Green means it was done in 1 jump; red means it took multiple steps.
      int color = (numSteps > 1) ? CYAN : YELLOW;
      drawLine( oldSelectedObPos, newSelectedObPos, color );


      // move from old position to new position in numStep jumps
      for( int i=1; i<=numSteps; i++ ) {
         // move the selectedOb to successive positions along the
         // line from the old pos to new pos.  
         float frac = ((float)i) / ((float)numSteps);
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
//   for( int obj = numObs-1;  obj >= 0;  obj-- ) {
   for( int obj=0; obj<numObs; obj++ ) {
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
// calc the difference between 2 angles in range (-M_PI,M_PI],  
// forcing result into same range.
float
centerAngle( float angle )
{
   // With input angles in the range (-M_PI,M_PI], the difference
   // can sometimes get outside this range, and must be adjusted
   // back into range (-M_PI,M_PI].  
   if(      (-M_PI) < angle  &&  angle <= M_PI )         ;
   else if( angle <= (-M_PI) )                     angle += 2*M_PI;
   else if(                    M_PI < angle     )      angle -= 2*M_PI;

   return angle;
}


/**************************************************************************************/
// calculate the angle in radians formed by line segments AB and BC.
// This calculation assumes 2D vectors in the XY plane 
// (and will not work for 3D vectors.)
float
calcAngleInPlane( Vec2d vA, Vec2d vB, Vec2d vC )
{
   Vec2d vAB = vA - vB;
   Vec2d vCB = vC - vB;

   float angleA = atan2( vAB.y, vAB.x );
   float angleC = atan2( vCB.y, vCB.x );

   // because atan2 returns angles in the range (-M_PI,M_PI], the difference
   // can sometimes get outside this range, and must be adjusted
   // back into range (-M_PI,M_PI].  
   return centerAngle( angleC - angleA );
}


/**************************************************************************************/
// Use equation from Falvo et al Nature paper (21 Jan 99)
// to calculate the sliding pivot point S = s * Axis
// from the location of the push point M = k * Axis.
// The normalized length variables x1' and x0' from the paper are here
// renamed k and s, respectively.  
// This equation is valid for 1/2 <= k <= 1.  
float
pivotPointEquation( float k )
{
   return   k - sqrt( k*k - k + 0.5 );
}


/**************************************************************************************/
Vec2d
PivotPoint( float normalizedPushPoint, Vec2d vEndA, Vec2d vEndB  )
{
   float k = normalizedPushPoint;
   float s;

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
float
nearestPointOnLineToPoint( Vec2d vA, Vec2d vB, Vec2d vP )
{
   Vec2d vAB = vA - vB;
   float dotProd = dotProduct( vAB, vP - vB );
   float lengthOfABVector = norm( vAB );

   float k;
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
   //   Return k, for use in calculating vM = ((vA-vB) * k) + vB;
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
      float tooFarAwayToDraw = 1000.;
      int color;
      if( cxTest.dist <= tooFarAwayToDraw ) {
         if( cxTest.dist <= 0. )   color = RED;     // collision
         else                 color = GREEN;
         drawLine( cxTest.vM, cxTest.vP, color );
      }

      glPopMatrix();
   }

//   // draw text labels for points
//   if( displayCollisionsOn ) {
//      // draw labeled points
//      showLabeledPoint( cxTest.vC, "C" );
//      showLabeledPoint( cxTest.vP, "P" );
//      showLabeledPoint( cxTest.vA, "A" );
//      showLabeledPoint( cxTest.vB, "B" );
//      showLabeledPoint( cxTest.vM, "M" );
//      showLabeledPoint( cxTest.vS, "S" );
//   }
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

//   showPoint( lineSegCx.vI,  BLUE );
//   showPoint( lineSegCx.vI1, CYAN );
//   showPoint( lineSegCx.vI2, CYAN );

}


/**************************************************************************************/
// Return 1 if value is in the closed interval bounded by bound1 and bound2.
// The interval is not empty.  The bounds are swapped if necessary
// to define a non-empty interval.
int
betweenInclusive( float value,   float bound1, float bound2 )
{
   // make sure that bound2 >= bound1
   if( bound2 < bound1 ) {
      // swap bound1 and bound2
      float temp = bound1;
      bound1     = bound2;
      bound2     = temp;
   }

   // Test for the value being within the interval.  
   // Being on the boundary counts as being in the interval.
   int betweenFlag = (bound1 <= value  && value  <= bound2);
   
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
   //float lengthDE = norm( vD - vE );
   float angleE = atan2( (vE-vD).y, (vE-vD).x );   // angle wrt +X axis
   
   // start with input point
   Vec2d vOut = vIn;

   // Translate so that D is at origin.
   vOut -= vD;

   // Rotate to put E on Y-axis
   vOut.rotate( - angleE + M_PI/2. );

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
   //float lengthDE = norm( vD - vE );
   float angleE = atan2( (vE-vD).y, (vE-vD).x );   // angle wrt +X axis
   
   // start with input point
   Vec2d vOut = vIn;

   // Rotate E off the Y-axis
   vOut.rotate( + angleE - M_PI/2. );

   // Translate D from the origin back to its original position.
   vOut += vD;

   // Return the transformed point.
   return vOut;
}


/**************************************************************************************/
// Calc the distance between centers for which two spheres touch.
// The centers are projected onto the XY plane, and the distance
// is measured in 2D.
float
touchDistance( float radius1, float radius2 )
{
         //   return radius1 + radius2;  // This works for pure 2D, but not 2.5D

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
                     float radiusSphere, float radiusCylinder )
{
   SPHERE_AND_TUBE_DATA cxTest;
   
   // copy radii of sphere and cylinder into struct.
   cxTest.sphereRadius = radiusSphere;
   cxTest.tubeRadius   = radiusCylinder;

   // copy input points into struct.
   cxTest.vP = vP;   // point P = center of sphere being tested for touching tube
   cxTest.vA = vA;   // endpoint A of tube axis
   cxTest.vB = vB;   // endpoint B of tube axis

   // Now calculate some points.
   // Center of the tube.
   cxTest.vC    = (vA + vB) / 2.;

   // Vector (P - B) runs from tube endpoint B to tip center P.
   // Calculate projection (M-B) of (P-B) onto tube axis (A-B) to 
   // find the nearest point M to P on the axis line AB.  
   // M will be on line AB but may not be on line segment AB.  
   // The coefficient k tells where point M is on the axis line.
   // 0<=k<=1 means point M is on  line segment AB.  
   float k = nearestPointOnLineToPoint( vA, vB, vP );  // line AB, point P
   cxTest.k = k;

   // If M is off either end of the line segment, force it back to an endpoint.
   // This gives M as the nearest point on *line-segment* AB to point P.
   // (Restating, calc the projection MB of line seg PB onto line AB, but with 
   // M set to an endpoint of AB if the point M is not within the line seg AB.)
   if(      0. < k  &&  k < 1. )   ;      //vM = vB + (vA - vB) * k);
   else if( k <= 0. )              k = 0.;   //vM = vB;
   else  /* 1. <= k */             k = 1.;   //vM = vA;

   // Calculate point M from coefficient k.
   cxTest.vM    = ((vA - vB) * k) + vB;

   // calculate the pivot point S for sliding mode from push point M.  
   cxTest.vS    = PivotPoint( k, vA, vB );

   // Since M is the nearest point on line segment AB to P,
   // the distance between M and P is also the distance between
   // P and line segment AB, the tube axis.
   float distTipCenterToTubeAxis = distance( cxTest.vP, cxTest.vM );

   // Subtract the distance (in XY plane only) when touching 
   // (distance between sphere center and tube axis)
   // to get the distance from sphere to tube.  
   // Here is how this distance D is interpreted:
   // D > 0 means the objects are not touching each other;
   // D = 0 means just touching at boundaries;
   // D < 0 means they are interpenetrating to a depth (-D).
   float touchDist = touchDistance( radiusSphere, radiusCylinder );
   cxTest.dist = distTipCenterToTubeAxis - touchDist;
      //   cxTest.dist = distTipCenterToTubeAxis - radiusSphere - radiusCylinder;


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
            //   showPoint( vDD, YELLOW );
            //   showPoint( vEE, YELLOW );
            //   showPoint( vFF, CYAN );
            //   showPoint( vGG, CYAN );

   // Is line segment FG parallel to DE?  Since DE is on the Y-axis,
   // This is the same as asking: Is FG vertical?
   // (If point F = point G, the slope is indeterminate, so treat that
   // as non-parallel.)
   if( vFF.x == vGG.x  &&  vFF.y != vGG.y ) {      
      // Yes, FG is parallel to DE.
      lineSegCx.parallelFlag = 1;




      // XXX Unfinished.  Luckily, exactly parallel lines
      // don't occur much in practice.



   }
   else {
      // No, FG is not parallel to DE.
      lineSegCx.parallelFlag = 0;
         
      // calculate the Y-intercept of line FG, which is the
      // intersection point I of lines FG and DE.  We know there
      // is an intersection since FG and DE are not parallel.
      float slopeFG = (vFF.y - vGG.y) / (vFF.x - vGG.x);
      float yIntercept = vFF.y - vFF.x * slopeFG;
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

   }



//   lineSegmentCollisionDraw( lineSegCx );


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
   float penetrationDepth = - cxTest.dist;
   Vec2d vStep = vDirection * penetrationDepth;
//   vStep *= 1.01;     // get it a little beyond contact

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
   float k = nearestPointOnLineToPoint( vAxis, vOrigin,    vVector );  
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
    //float radiusTube  = ob[tube].diam / 2.;
   Vec2d vCenterTube = ob[tube].pos;
   float lengthTube  = ob[tube].leng;
   float angleTube   = ob[tube].angle;

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
   float sphereRadius1 = cxTest1.sphereRadius;
   float tubeRadius1 = cxTest1.tubeRadius;
   cxTest = distSphereToCylinder( vP, vA, vB, sphereRadius1, tubeRadius1 );


//   return;
#endif


/**************************************************************************************/
// slide the tube across the surface with no rolling, and rotating the
// tube axis at the pivot point given by the equation from the Nature paper.
void
slidingCollisionResponse( Vec2d vPushPt, int tube, SPHERE_AND_TUBE_DATA cxTest )
{
   // mark the tube as moved to allow it to move other tubes
   ob[tube].moved = 1;
   

   // Check whether the push sphere is pushing the side of the tube or the endcap.
   // The parameter k is between 0 and 1 when the sphere pushes the side of the tube.
   if( cxTest.k < 0.  ||  cxTest.k > 1. )  {      
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
      float kM = 1.0;      // endpoint of axis
      float kC = 0.5;      // midpoint of axis
      float kS = 1. - (sqrt( 2. ) / 2.);   // approx .2929 (not 1/3)
                     // given by equation in Nature paper
      float ratioOfLengths = (kC - kS) / (kM - kS);  // exactly same as kS
      Vec2d vC = vS + (vM - vS) * ratioOfLengths;

      // Calculate new orientation of tube resulting from push.
      float tubeAngle = atan2( (vM-vC).y, (vM-vC).x );
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

      float penetrationDepth = - cxTest.dist;
      float distSphereToPivot = norm( cxTest.vS - cxTest.vP );
      float sphereRadius = cxTest.sphereRadius;
      float tubeRadius   = cxTest.tubeRadius;

      // This ratio is positive, since the numerator and denominator are positive.
      float touchDist = touchDistance( sphereRadius, tubeRadius );
      float ratio = touchDist / distSphereToPivot;
      assert( ratio >= 0., "slidingCollisionResponse: ratio negative" );

      if( ratio > 1. ) {
         // In this case, no rotation around S can get the tube out of contact
         // with the sphere, so translation is required.  
         // Translate tube outward along sphere-center P to tube-center C vector
         // Move a distance equal to penetration depth.
         Vec2d vCP    = cxTest.vC - cxTest.vP;
         Vec2d vCPdir = vCP / norm( vCP );
         Vec2d vOutwardStep   = vCPdir * penetrationDepth;
         ob[tube].pos += vOutwardStep;
      
         // Rotate tube to be tangent to sphere boundary
         // (normal to ray coming from sphere center).
         Vec2d vSphereTangent = vCPdir.rotate( M_PI/2. );
         float tubeAngle = atan2( vSphereTangent.y, vSphereTangent.x );
         ob[tube].angle = tubeAngle;
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
         
         // calculate angle PSM in range (-M_PI,M_PI].
         // This produces an angle which, if the tube is rotated around S,
         // the tube will turn away from the colliding point P.
         float currentPSMangle = calcAngleInPlane( cxTest.vP, cxTest.vS, cxTest.vM );
         // So use sign of current angle for direction to turn tube.
         float sign = (currentPSMangle >= 0.) ? 1. : -1.;

         float newPSMangle;
         newPSMangle = asin( ratio );

         float angleChangeMagnitude = fabs(newPSMangle) - fabs(currentPSMangle);
         float turningAngle = sign * angleChangeMagnitude;

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
// Translate and/or rotate the tube in response to a collision with the tip.
// (ie, the tip is pushing the tube).
void
collisionResponse( Vec2d vPushPt, float radiusSphere, int pushedTube )
{
   // calculate pushed tube endpoints A and B, and tube's radius.
   Vec2d vA;
   Vec2d vB;
   calcTubeEndpoints( pushedTube, &vA, &vB );
   float radiusCylinder = ob[pushedTube].diam / 2.;

   // calculate the collision data for sphere and tube
   SPHERE_AND_TUBE_DATA cxTest;
   cxTest = distSphereToCylinder( vPushPt, vA, vB, 
                         radiusSphere, radiusCylinder );

   // Collision response depends on sim type.
   // Pass collision data into collision response routine.
   switch( simType ) {
   case SIM_NONE:      break;
   case SIM_TRANSLATE:   translateCollisionResponse( vPushPt, pushedTube, cxTest );   break;
   case SIM_ROLLING:   rollingCollisionResponse(   vPushPt, pushedTube, cxTest );   break;
   case SIM_SLIDING:   slidingCollisionResponse(   vPushPt, pushedTube, cxTest );   break;
   case SIM_ROLLING_SLIDING: 
                 rollingSlidingCollisionResponse( vPushPt, pushedTube, cxTest ); break;
   }
}


/**************************************************************************************/
void
crossedTubeAxisCollisionResponse(  LINE_SEGMENTS_INTERSECTION_TEST lineSegCx,
   int pushingTube, int pushedTube  )
{
   // find nearest point Q to I on line seg DE.
   float lengthID = distance( lineSegCx.vI, lineSegCx.vD );
   float lengthIE = distance( lineSegCx.vI, lineSegCx.vE );
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
   vFGAxisNormal.rotate( M_PI/2. );

   // Make sure we move the pushed tube away from the axis DE.
   // The dot product of (Q-I) with a normal to the axis FG tells us 
   // whether we are moving away from or towards DE's axis.
   // There are 2 normals to choose between and we want the
   // right one -- the one that moves us away from DE.  
   // The dot product is proportional to the cosine of the angle between
   // the normal and (Q-I).  The cosine is >= 0 if the angle is in the
   // interval [-M_PI/2,M_PI/2].  
   // If the normal we picked failed the test, use the other normal.
   // All these vectors are in the XY-plane.  
   float dotProd = dotProduct( vFGAxisNormal, vStepUncrossTubes );
   float sign = (dotProd >= 0.) ? 1. : -1.;
   vFGAxisNormal *= sign;

   // calc tube radii
   float radiusPushingTube  = ob[pushingTube ].diam / 2.;
   float radiusPushedTube  = ob[pushedTube].diam / 2.;

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
   float radiusPushingTube  = ob[pushingTube ].diam / 2.;

   // Define endpoints F and G and radius of the pushed tube.
   Vec2d vF;
   Vec2d vG;
   calcTubeEndpoints( pushedTube, &vF, &vG );
   float radiusPushedTube  = ob[pushedTube].diam / 2.;

   // test for intersection between two tubes
   LINE_SEGMENTS_INTERSECTION_TEST lineSegCx = 
         distBetweenLineSegments( vD, vE,    vF, vG );

   // Calc location of point that tubes touch (or point on pushing
   // tube that penetrates most deeply into pushed tube).
   float r1 = radiusPushingTube;   // ie DE, which contains N1
   float r2 = radiusPushedTube;    // ie FG, which contains N2
   float fraction = r1 / (r1 + r2);
   Vec2d vTouchPoint = interpolateVecs( lineSegCx.vN1, lineSegCx.vN2, fraction );

   // calc z height of collision point
   float zPushingTube = r1;
   float zPushedTube  = r2;
   float zTouchPoint  = interpolateScalars( zPushingTube, zPushedTube, fraction );

   // Calc distance between tubes, taking radii into account.
   // Negative distance is penetration.
   float distTubeTube = lineSegCx.dist 
                   - touchDistance( radiusPushingTube, radiusPushedTube );
   Vec2d vPushPoint = lineSegCx.vN1;   // this point is on line seg DE


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
               //   int color = (lineSegCx.parallelFlag == 1) ? YELLOW : RED;
      }

      // move pushed tube in response to collision at push point by pushing tube
      collisionResponse( vPushPoint, radiusPushingTube, pushedTube );
   }
}


/**************************************************************************************/
/**************************************************************************************/
// PROPAGATE PUSHES

/**************************************************************************************/
void
propagateMovements( int movedTube )
{
   // mark this tube as having moved, and so not movable the rest of this sim step.
   ob[movedTube].moved = 1;




   positionSegmentedTube( movedTube );

//   return;





   // To propagate movement to other tubes, the tube which moved must
   // be tested for collision with all other tubes.
   // However, to guarantee termination of this recursion, each object
   // can only be moved one time per simulation step.
   for( int i=0; i<numObs; i++ ) {
      if (ob[i].type != TUBE) continue;

      // skip objects which have already been moved 
      // (and are therefore marked as not movable).
      if( ! ob[i].moved ) {

         // test for movedTube colliding with object i.
         TubeTubeCollisionTestAndResponse( movedTube, i );

         // if object i moved as a result of this collision test,
         // then propagate pushes to all remaining unmoved objects.
         // (Only objects that collide with the pusher object get moved.)
         if( ob[i].moved ) {
            propagateMovements( i );
         }
      }
   }
}



/**************************************************************************************/
void
markAllTubesMovable( void )
{
   // mark all tubes as movable to start the sim step.
   for( int i=2; i<numObs; i++ )
      ob[i].moved = ((ob[i].type == TUBE) ? 0 : 1);
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
   float normalLengthMoved = norm( vNormal );
   float tubeRadius = ob[tube].diam / 2.;
   float rollAngleChangeInRadians = normalLengthMoved / tubeRadius;

   // Figure out sign of roll rotation by testing which side of the axis
   // (M-P) is on.  
   // The dot product of (M-P) with a normal to the axis tells us this,
   // since it is proportional to the cosine of the angle between
   // the normal and (M-P).  The cosine is >= 0 if the angle is in the
   // interval [-M_PI/2,M_PI/2].  
   // All these vectors are in the XY-plane.  
   Vec2d vAxisNormal = (cxTest.vA - cxTest.vB).rotate( - M_PI/2. );
   float dotProd2 = dotProduct( vAxisNormal, vStep );
   float sign = (dotProd2 >= 0.) ? 1. : -1.;

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
   float angleBetweenSymmetryLines = M_PI/3.;
   float lockThresholdAngle = M_PI/36.;   // (+ or -) M_PI/36 = 5 degrees

   // Calculate the angle between the tube and the substrate.
   float substrateAngle = 0.; // orientation of graphite substrate's crystal symmetry lines
   float tubeAngleOnSubstrate = centerAngle( ob[tube].angle - substrateAngle );

   // Scale the angle so that symmetry angles are integers. 
   float scaledAngle = tubeAngleOnSubstrate / angleBetweenSymmetryLines;

   // Separate into integer (which symmetry line) and 
   // fractional (position between symmetry lines) parts.
   // Use rounding, not truncation, to get to nearest integer.
   // The fractional part will be in the interval [-1/2,1/2].
   float integerPart    = floor( scaledAngle + 0.5 );
   float fractionalPart = scaledAngle - integerPart;

   // Scale the threshold angle and test for the current tube angle
   // being within threshold of a symmetry line.
   float normalizedThreshold = lockThresholdAngle / angleBetweenSymmetryLines;
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
      float substrateAngle = 0.; // orientation of graphite substrate's crystal symmetry lines
      vSymmetry.rotate( substrateAngle );
      float symmetryLineLength = 18.;
      vSymmetry *= symmetryLineLength / 2.;

      // draw symmetry lines every 120 degrees, centered at origin.
      drawLine( vSymmetry, - vSymmetry, CYAN );
      rotateRadiansAxis( M_PI/3., 0.0, 0.0, 1.0 );
      drawLine( vSymmetry, - vSymmetry, CYAN );
      rotateRadiansAxis( M_PI/3., 0.0, 0.0, 1.0 );
      drawLine( vSymmetry, - vSymmetry, CYAN );

      glPopMatrix();
   }
}


/**************************************************************************************/
// Update the position and orientation of segment curSeg, which is
// linked to lastSeg (by either a nextSeg or prevSeg link in the
// doubly linked list).  The bend angles between segments are
// specified in the tube data structure.  
void
positionSegmentRigid( int curSeg, int lastSeg, int directionFlag )
{
   // calculate lastSeg endpoint relative to center of segment
   Vec2d vLastSegEndpointOffset = Vec2d(1.,0.);
   vLastSegEndpointOffset.rotate( ob[lastSeg].angle );
   vLastSegEndpointOffset *= (ob[lastSeg].leng / 2.);

   // Calculate curSeg's orientation from lastSeg's orientation
   // by using the bending angle for between segments.
   // (The bending angle should logically be stored in some
   // "between-segment" data structure.  By storing this angle
   // in one of the two adjacent segments, we introduce
   // an asymmetry.  
   float jointBendingAngle = (directionFlag) ? ob[curSeg ].bendAngle : 
                                    ob[lastSeg].bendAngle;

   // calculate curSeg endpoint relative to center of segment
   Vec2d vCurSegEndpointOffset = Vec2d(1.,0.);
   vCurSegEndpointOffset.rotate( ob[curSeg].angle );
   vCurSegEndpointOffset *= (ob[curSeg].leng / 2.);

   // calc offset from lastSeg center to curSeg center.
   Vec2d vOffset = vLastSegEndpointOffset + vCurSegEndpointOffset;

   // sign of offset between centers depends on which direction we are walking
   // the doubly-linked list of segments.
   if( directionFlag == 1 ) {
//      showPoint( ob[curSeg].pos, RED, 3. );

      ob[curSeg].pos =   ob[lastSeg].pos + vOffset;
      ob[curSeg].angle = ob[lastSeg].angle + jointBendingAngle;
   }
   else {
//      showPoint( ob[curSeg].pos, GREEN, 3. );

      ob[curSeg].pos =   ob[lastSeg].pos - vOffset;
      ob[curSeg].angle = ob[lastSeg].angle - jointBendingAngle;
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
   // calculate last segment's endpoints A and B.
   Vec2d vA;
   Vec2d vB;
   calcTubeEndpoints( lastSeg, &vA, &vB );

   // pick endpoint Q according to which direction (next or prev)
   // we are walking through segments.
   Vec2d vQ = (directionFlag == 1) ? vA : vB;

   // choose a point on the current segment (before it has moved),
   // currently the segment center.  
   Vec2d vC = ob[curSeg ].pos;


   // Calculate a direction from the endpoint Q to the point C.
   // This defines a ray emanating from endpoint Q.
   // Put the curSeg's center on this ray at the appropriate distance.
   // (This is the shortest distance the segment center could move
   // and still stay connected to lastSeg.)
   Vec2d vDir = (vC - vQ) / norm(vC - vQ);
   float halfLengthCurSeg = ob[curSeg ].leng / 2.;
   ob[curSeg].pos = vQ + (vDir * halfLengthCurSeg);


   // Calculate orientation of curSeg segment based on C-V direction vector.
   // Flip the orientation by M_PI radians, depending on which 
   // direction (next or prev) we are walking through segments.  
   float newAngleCurSeg;
   if( vDir.x==0. && vDir.y==0. )  newAngleCurSeg = 0.;
   else                            newAngleCurSeg = atan2( vDir.y, vDir.x );
   newAngleCurSeg += ((directionFlag==1) ? 0. : M_PI );
   ob[curSeg].angle = newAngleCurSeg;


//   // Debugging aid: use green/red to show next/prev-based updates.
//   if( directionFlag == 1 ) {showPoint( ob[curSeg].pos, RED, 3. );}
//   else                     {showPoint( ob[curSeg].pos, GREEN, 3. );}


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
newSegmentedTube( int numSegments, float bendAngle, 
              float segmentLength, float segmentWidth, Vec2d vEndpoint  )
{
   assert( numSegments >= 1, "numSegments must be >= 1" );
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
               NULLOB, NULLOB, bendAngle );

      // link segments together.
      ob[seg].prevSeg = (seg > firstSeg) ? seg-1 : NULLOB;
      ob[seg].nextSeg = (seg < lastSeg ) ? seg+1 : NULLOB;
   }

   return firstSeg;
}


/**************************************************************************************/
/**************************************************************************************/
// IMAGE SCAN


/**************************************************************************************/
int
getImageHeightAtXYLoc( float x, float y, float* z )
{
#ifdef NMRC_LIB
   if (!g_activeControl)
   {
#endif
   float tipRadius = ob[TIP].diam/2.;
   float zSubstrate = 0.;   // z-height of substrate
   
   // Calculate a z-height at this imaging point for each tube
   // and return the maximum z-value calculated.  
   // The least z-value which can be returned is the tip height 
   // for the tip scanning a bare substrate.
   float maxZ = zSubstrate;
   for( int tube=2; tube<numObs; tube++ ) {
      // Start with the point at which we want a height measurement.
      Vec2d vP = Vec2d(x,y);

      // calculate pushed tube endpoints A and B, and tube's radius.
      Vec2d vA;
      Vec2d vB;
      calcTubeEndpoints( tube, &vA, &vB );
      float radiusCylinder = ob[tube].diam / 2.;

      // calculate distance d from imaging point P to tube axis
      SPHERE_AND_TUBE_DATA cxTest;
      cxTest = distSphereToCylinder( vP, vA, vB, 
                       0., 0. );
      Vec2d vM = cxTest.vM;
      float d = distance( vM, vP );

      // measure height of tube of radius r as seen by tip of radius R
      // at image point P.
      float r = radiusCylinder;
      float R = tipRadius;
      float z = (d < r+R) ? r + sqrt( (r+R)*(r+R) - d*d ) : zSubstrate;

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
#ifdef NMRC_LIB
   else
   {
      if (g_activeMode != IMAGE_MODE)
      {
         g_microscope->ImageMode();

         g_activeMode = IMAGE_MODE;
      }

      double wX, wY;


      nmb_Image *hImage = g_dataset->dataImages->getImageByName(
                                g_dataset->heightPlaneName->string());

      hImage->pixelToWorld(x, y, wX, wY);

      Point_value *point = g_microscope->state.data.inputPoint->
                      getValueByPlaneName(g_dataset->heightPlaneName->string());

      g_microscope->TakeFeelStep(wX, wY, point, vrpn_TRUE);

      *z = point->value();

      return 0;
   }
#endif
}


/**************************************************************************************/
void
showGrid( void )
{
   if(      gridStyle == GRID_NONE )
      return;
#ifdef NMRC_LIB
   nmb_Image *hImage = g_dataset->dataImages->getImageByName(
                             g_dataset->heightPlaneName->string());
#endif

   // display grid
   shadingModel = GL_SMOOTH;
   lighting();
   setColor( GREEN );
#ifndef NMRC_LIB
   assert( gridSize < MAX_GRID, "gridSize too big" );
#endif
#ifndef NMRC_LIB
   for( int i=0; i<gridSize-1; i++ ) {
      for( int j=0; j<gridSize-1; j++ ) {
#else
   for( int i=0; i<gridXSize-1; i++ ) {
      for( int j=0; j<gridYSize-1; j++ ) {
#endif
         float x = i * scanStep  +  scanXMin;
         float y = j * scanStep  +  scanYMin;
         float dx = scanStep;
         float dy = scanStep;

         // Get the the 4 (x,y,z) coords on the corners of this grid cell.
#ifndef NMRC_LIB
         float x1 = x;     float y1 = y;     float z1 = zHeight[i  ][j  ];
         float x2 = x+dx;  float y2 = y;     float z2 = zHeight[i+1][j  ];
         float x3 = x+dx;  float y3 = y+dy;  float z3 = zHeight[i+1][j+1];
         float x4 = x;     float y4 = y+dy;  float z4 = zHeight[i  ][j+1];
#else
         float x1 = x;     float y1 = y;     float z1 = hImage->getValueInterpolated(x1,y1) - hImage->minValue();
         float x2 = x+dx;  float y2 = y;     float z2 = hImage->getValueInterpolated(x2,y2) - hImage->minValue();
         float x3 = x+dx;  float y3 = y+dy;  float z3 = hImage->getValueInterpolated(x3,y3) - hImage->minValue();
         float x4 = x;     float y4 = y+dy;  float z4 = hImage->getValueInterpolated(x4,y4) - hImage->minValue();
#endif
         Vec2d v1 = Vec2d( x1, y1 );
         Vec2d v2 = Vec2d( x2, y2 );
         //Vec2d v3 = Vec2d( x3, y3 );
         Vec2d v4 = Vec2d( x4, y4 );

         // display grid, according to current grid display mode (gridStyle)
         if(      gridStyle == GRID_NONE ) {
            // display nothing
         }
         else if( gridStyle == GRID_WIREFRAME ) {
            // draw line parallel to Y-axis (x varies)
            drawLine( v1, v2, BLUE, z1, z2 );
            // draw line parallel to X-axis (y varies)
            drawLine( v1, v4, BLUE, z1, z4 );
         }
         else if ( gridStyle == GRID_SOLID  ||  gridStyle == GRID_HALF_SOLID ) {
            // calc normal to plane through P1, P2, P3 using (P1-P3) x (P1-P2)
            float xn = 0., yn = 0., zn = 1.;
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

      }
   }

}


#ifndef NMRC_LIB
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
   glOrtho(  scanXMin,   scanXMax,
            scanYMin,   scanYMax,
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
   float substrateHeight = 0.;
   setColor( GREEN );
   glBegin(GL_POLYGON);
   glNormal3f( 0., 0., -1. );
   glVertex3f( scanXMin, scanYMin, substrateHeight );
   glVertex3f( scanXMin, scanYMax, substrateHeight );
   glVertex3f( scanXMax, scanYMax, substrateHeight );
   glVertex3f( scanXMax, scanYMin, substrateHeight );
   glEnd();

   //   Draw tubes with radii increased by tip radius.
   float tipRadius = ob[TIP].diam/2.;
   setColor( WHITE );
   for( int i=0; i<numObs; i++ ) {
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
      float radius;
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

      case PHANTOM:      
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
void
doImageScan( void )
{
   if( gridStyle == GRID_NONE )  return;


   int i;
   int j;

   if( imageScanType == IMAGE_SCAN_EXACT ) {
      // Calc grid height at each grid point.
      // ie, do an image scan.
      for( i=0; i<gridSize; i++ ) {
         for( j=0; j<gridSize; j++ ) {
            float x = i * scanStep  +  scanXMin;
            float y = j * scanStep  +  scanYMin;

            float* pz = &(zHeight[i][j]);
            getImageHeightAtXYLoc( x, y, pz );
         }
      }
   }
   else if( imageScanType == IMAGE_SCAN_APPROX ) {
      // Render tube images (enlarged to account for tip radius)
      // into window.  
      // (We don't really care about the image, just the depth.)
      imageScanDepthRender();

      // Read (normalized) Z-buffer values from the depth window.
      // Scale them back to correct Z-values and use as 
      // Z-heights in image scan grid.  
            // static float zBuffer[ 128*128 ];
      void* zBufferPtr = &(zBuffer[0]);
      int pixelGridSize = 64;      // must match window size
            // width and height the same for now
      glReadPixels( 0, 0, pixelGridSize, pixelGridSize, GL_DEPTH_COMPONENT, GL_FLOAT, zBufferPtr );
      for( i=0; i<gridSize; i++ ) {
         for( j=0; j<gridSize; j++ ) {
            float zNormalized = zBuffer[ j*pixelGridSize + i ];
            float zDepth = scanFar + zNormalized * (scanNear - scanFar);
            zHeight[i][j] = zDepth;
         }
      }
   }
}


/**************************************************************************************/
void 
displayFuncDepth( void ) 
{
   if(     gridStyle     == GRID_NONE  ||
          imageScanType == IMAGE_SCAN_EXACT) {
      // If the depth window is not being used, clear it.
      glutSetWindow( depthWindowID );
      glClearColor(0.5, 0.5, 0.5, 0.0);   // gray background
      glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
      glutSwapBuffers();
   }

   doImageScan();
}

#endif

/**************************************************************************************/
/**************************************************************************************/
