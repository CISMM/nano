/*$Id$*/
#include <stdlib.h>		//stdlib.h vs cstdlib
#include <stdio.h>		//stdio.h vs cstdio
#include <iostream.h>
#include <vector>
#include <math.h>		//math.h vs cmath
#include <GL/glut.h>
#include "Vec3d.h"
#include "3Dobject.h"
#include "ConeSphere.h"
#include "Tips.h"
#include <string.h>
#include "defns.h"
#include "Unca.h"
#include "input.h"
#include "scan.h"
#include "draw.h"
#include "lightcol.h"
#include "myglu.h"
#include "sim.h"

GLuint list_sphere;
GLuint list_cylinder;

static int dblBuf  = GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH;
//static int singleBuf  = GLUT_SINGLE | GLUT_RGBA | GLUT_DEPTH;
Ntube nullntube;

OB *ob[MAXOBS];
int numObs;

int selectedOb = NULLOB;
int buttonpress=-1;
int selected_triangle_side;

int uncertainty_mode=0;

#define NO_AFM 0
#define SEMI_SOLID_AFM 1
#define SOLID_AFM 2

int afm_scan=SEMI_SOLID_AFM;
//int afm_scan=NO_AFM;
int draw_objects=1;
double view_angle = -90.;

//int solid_afm_scan=0;
//int enable_afm=1;

int rotate_and_write_to_file=0;
int done_monster_process_x=0, done_monster_process_y=0;

// window stuff
int mainWindowID, viewWindowID, depthWindowID;
double windowWidth  = 600.;
double windowHeight = 600.;
double orthoFrustumCenterX = 64.;	// area of XY plane always visible for all window aspect ratios
double orthoFrustumCenterY = 64.;
double orthoFrustumWidthNominal  = 128.;
double orthoFrustumHeightNominal = 128.;

// actual bounds of current ortho view frustum matching window aspect ratio
double orthoFrustumLeftEdge;
double orthoFrustumBottomEdge;
double orthoFrustumWidth;
double orthoFrustumHeight;

// mouse and cursor
int xMouseInWindow;	// mouse position in world coords
int yMouseInWindow;	// mouse position in world coords
Vec3d vMouseWorld;	// mouse position in world coords (actually a 2D vector in XY plane)
Vec3d vGrabOffset;	// offset from cursor position to grabbed object (in world coords actually a 2D vector in XY plane)

int stopAFM=0;
int done_drawing_objects=0;
double thetax=0.,thetay=0.;
int tesselation = 30;


void write_to_unca(char *filename);
void displayFuncMain( void );
void displayFuncView( void );
void  displayFuncDepth( void );
void commonIdleFunc( void );
void idleFuncDummy( void );
void reshapeWindowFuncDummy( int newWindowWidth, int newWindowHeight);
void reshapeWindow( int newWindowWidth, int newWindowHeight);
void adjustOrthoProjectionParams( void );
void adjustOrthoProjectionToWindow( void );
void adjustOrthoProjectionToViewWindow( void );
void globalkeyboardFunc(unsigned char key, int x, int y);
void commonKeyboardFunc(unsigned char key, int x, int y);
void mouseFuncMain( int button, int state, int x, int y);
void mouseMotionFuncMain( int x, int y);
void calcMouseWorldLoc( int xMouse, int yMouse, int xy_or_xz);
void grabNearestOb(int xy_or_xz);
int findNearestObToMouse(int xy_or_xz);
void findNearestTriangleSideToMouse( void );
void select_triangle_side();


int main(int argc, char *argv[])
{
  adjustOrthoProjectionParams();

  if (argc > 1) {// load from a file
    if (0==strcmp(argv[1],"-p")) {//ntube file
      addSpheresFromFile(argv[2],atof(argv[3]));
      //      addSpheresFromFile(argv[2],atof(argv[3]));
      if (0==strcmp(argv[4],"-w")) {
	rotate_and_write_to_file=1;
      }
    }
    else if (0==strcmp(argv[1],"-t")) {//triangles file
      addTrianglesFromFile(argv[2],atof(argv[3]));
    }
    else if (0==strcmp(argv[1],"-d")) {//dna file
      init_dna(argv[2]);
    }
    else {
      cout << "\nUsage : ./sim [-p filename unit [-w]] for proteins\n";
      cout << "        ./sim [-t filename scale] for triangulated models\n";
      cout << "        ./sim [-d dna-filename]\n\nSee README\n\n";
      exit(0);
    }
  }
  else {
    initObs();
  }


  // Deal with command line.
  glutInit(&argc, argv);
  glutInitDisplayMode(dblBuf);
  //  glutInitDisplayMode(singleBuf);

  /* The view on Main window is a view of XY plane from a pt on the +ve 
   * Z-axis. +ve X axis is towards right while +ve Y is to upwards
   */
  
  // MAIN WINDOW
  glutInitWindowSize( (int)windowWidth, (int)windowHeight );
  glutInitWindowPosition( 50, 0 );
  mainWindowID = glutCreateWindow( "3D AFM simulator - Top View" );
  adjustOrthoProjectionToWindow();

#if DISP_LIST
    make_sphere();
    make_cylinder();
    make_cone_sphere(ics);
#endif

  // pass pointers to callback routines for main window
  glutDisplayFunc(displayFuncMain);
  glutIdleFunc(idleFuncDummy );
  glutReshapeFunc(reshapeWindow);
  glutKeyboardFunc(commonKeyboardFunc);
  glutMouseFunc(mouseFuncMain);
  glutMotionFunc(   mouseMotionFuncMain );

  /* The view on Another View window is a front view from a point on the
   *  -Y axis
   */

#if 1
  
  // another view WINDOW
  glutInitWindowSize( (int)windowWidth, (int)windowHeight );
  glutInitWindowPosition( 800, 0 );
  viewWindowID = glutCreateWindow( "Front View" );
  adjustOrthoProjectionToViewWindow();
  glutMouseFunc(mouseFuncMain);
  glutMotionFunc(   mouseMotionFuncMain );

#if DISP_LIST
    make_sphere();
    make_cylinder();
    make_cone_sphere(ics);
#endif

  // pass pointers to callback routines for the other view window
  glutDisplayFunc(displayFuncView);
  glutIdleFunc(idleFuncDummy );
  //  glutReshapeFunc(reshapeWindow);
  glutKeyboardFunc(commonKeyboardFunc);
#endif

  // Depth WINDOW
  glutInitWindowSize( (int)DEPTHSIZE, (int)DEPTHSIZE );
  glutInitWindowPosition( 50, 650 );
  depthWindowID = glutCreateWindow( "Depth window" );

#if DISP_LIST
    make_sphere();
    make_cylinder();
    make_cone_sphere(ics);
#endif

  glutDisplayFunc( displayFuncDepth );
  glutIdleFunc(idleFuncDummy );
  glutReshapeFunc(    reshapeWindowFuncDummy );
  glutKeyboardFunc(commonKeyboardFunc);
  adjustOrthoProjectionToWindow();


  // app's main loop, from which callbacks to above routines occur
  glutMainLoop();

  return 0;               /* ANSI C requires main to return int. */
}


void write_to_unca(char *filename) {
  double orthoFrustumNearEdge =  scanNear;
  /* All far pts get mapped to scanFar. Allow round off of 1 */
  double orthoFrustumFarEdge  =   scanFar+1;
  
  // Everythinghere is in Angstroms. Unca takes care of this.
  Unca u = Unca(DEPTHSIZE, DEPTHSIZE, orthoFrustumLeftEdge,(orthoFrustumLeftEdge + orthoFrustumWidth),orthoFrustumBottomEdge,(orthoFrustumBottomEdge + orthoFrustumHeight), orthoFrustumNearEdge, orthoFrustumFarEdge, (double *)zHeight);
  u.writeUnca(filename);
}


/**************************************************************************************/
// This routine is called only after input events.
void displayFuncMain( void ) {
  if (!stopAFM) {
    glutSetWindow( mainWindowID );
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    // draw graphics for this frame
    drawFrame();
    // end of display frame, so flip buffers
    glutSwapBuffers();
  }
}

void displayFuncView( void ) {
  if (!stopAFM) {
    glutSetWindow( viewWindowID );
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    glPushMatrix();
    glRotatef(view_angle, 1.0, 0.0, 0.0 ); 
    drawFrame();
    glPopMatrix();
    // end of display frame, so flip buffers
    glutSwapBuffers();
  }
}

// This is the callback for rendering into the depth buffer window,
// as required by "doImageScanApprox".  
void  displayFuncDepth( void ) {
  if (!stopAFM) {
    glutSetWindow( depthWindowID );
    // in Z-buffer of Depth Window using graphics hardware.
    doImageScanApprox();
    // end of display frame, so flip buffers
    glutSwapBuffers();
  }
}

// This idle function marks both windows for redisplay, which will cause
// their display callbacks to be invoked.
void commonIdleFunc( void ) {
#define PERIOD 30

  static int run_cnt=0;
  if (dna) {
    dna->run();
    run_cnt++;
  }
  if ((run_cnt % PERIOD) == 0) {
    glutSetWindow( mainWindowID );		glutPostRedisplay();
    glutSetWindow( viewWindowID );		glutPostRedisplay();
    glutSetWindow( depthWindowID );		glutPostRedisplay();
  }
}

void idleFuncDummy( void ) {commonIdleFunc();}
void reshapeWindowFuncDummy( int newWindowWidth, int newWindowHeight ) {}

// callback routine: called when window is resized by user
void reshapeWindow( int newWindowWidth, int newWindowHeight ) {
  windowWidth  = newWindowWidth;
  windowHeight = newWindowHeight;

  // viewport covers whole window
  glViewport( 0, 0, (int)windowWidth, (int)windowHeight );

  // make graphics projection match window dimensions
  adjustOrthoProjectionToWindow();
}


void adjustOrthoProjectionParams( void ) {
  // set nominal size of window before taking aspect ratio into account
  //	double orthoFrustumLeftEdgeNominal   = orthoFrustumCenterX - orthoFrustumWidthNominal/2.;
  double orthoFrustumBottomEdgeNominal = orthoFrustumCenterY - orthoFrustumHeightNominal/2.;

  // calculate aspect ratio of current window
  double aspectRatio = windowWidth / windowHeight;

  // set vertical extent of window to nominal area of world being viewed.
  orthoFrustumHeight = orthoFrustumHeightNominal;
  orthoFrustumBottomEdge = orthoFrustumBottomEdgeNominal;

  // view horizontal extent of world proportional to window width
  orthoFrustumWidth = orthoFrustumWidthNominal * aspectRatio;
  orthoFrustumLeftEdge   = orthoFrustumCenterX - orthoFrustumWidth / 2.;
}


// adjust the ortho projection to match window aspect ratio and keep circles round.
void adjustOrthoProjectionToWindow( void ) {
  double orthoFrustumNearEdge =  scanNear;
  /* All far pts get mapped to scanFar. Allow round off of 1 */
  double orthoFrustumFarEdge  =   scanFar;

  // set projection matrix to orthoscopic projection matching current window
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(  orthoFrustumLeftEdge,   orthoFrustumLeftEdge   + orthoFrustumWidth,
	    orthoFrustumBottomEdge, orthoFrustumBottomEdge + orthoFrustumHeight, orthoFrustumNearEdge,   orthoFrustumFarEdge );
}

// adjust the ortho projection to match window aspect ratio and keep circles round.
void adjustOrthoProjectionToViewWindow( void ) {

#if 0
  double orthoFrustumNearEdge =  -scanFar;
  double orthoFrustumFarEdge  =   -scanNear;
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glOrtho(  orthoFrustumLeftEdge,   orthoFrustumLeftEdge   + orthoFrustumWidth,
	    orthoFrustumNearEdge,   orthoFrustumFarEdge, 
	    orthoFrustumBottomEdge, orthoFrustumBottomEdge + orthoFrustumHeight);
#else
  double orthoFrustumNearEdge =  -scanFar;
  double orthoFrustumFarEdge  =   -scanNear;
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glOrtho(  orthoFrustumLeftEdge,   orthoFrustumLeftEdge   + orthoFrustumWidth,
	    orthoFrustumNearEdge,   orthoFrustumFarEdge, 
	    -100,100);
#endif
}

void error( char* errMsg ) {
  printf( "\nError: %s\n", errMsg );
  exit( 1 );
}

/* this does global transormations (e,g rotate or translate the entire world)
 * as opposed to commonKeyboardFunc which mostly does object level 
 * transformations.
 * This is called when selectedOb is NULLOB
 */
void globalkeyboardFunc(unsigned char key, int x, int y) {
  for (int i=0;i<numObs;i++) {
    if (ob[i] == UNUSED) continue;
    ob[i]->keyboardFunc(key,x,y);
  }
}

// Keyboard callback for main window.
void commonKeyboardFunc(unsigned char key, int x, int y) {
  switch (key) {
  case 'u' : /* want to see uncertainty map : 
	      * use flat shading
	      */
    uncertainty_mode = !uncertainty_mode;
    break;
  case 'm' : // toggle shading model
    if (shadingModel == GL_FLAT) {
      shadingModel = GL_SMOOTH;
      printf("Using smooth shading model\n");
    }
    else
      if (shadingModel == GL_SMOOTH) {
	shadingModel = GL_FLAT;
	printf("Using flat shading model\n");
      }
    break;
  case 'v' :/* rotation in view angle
	     */
    view_angle += 5.;
    glutSetWindow( viewWindowID );
    glutPostRedisplay();
    break;
  case 'V' :/* rotation in view angle
	     */
    view_angle -= 5.;
    glutSetWindow( viewWindowID );
    glutPostRedisplay();
    break;
  case KEY_DELETE :
    if (selectedOb != NULLOB) {
      ob[selectedOb] = UNUSED;
    }
    break;
  case 'o' :
    if (draw_objects) {
      draw_objects = 0;
    }
    else {
      draw_objects = 1;
    }
    break;
  case 'n' :
    addNtube( NTUBE,  Vec3d( 0., 0., (DEFAULT_DIAM/2.)), 0., 0., 0., DEFAULT_LENGTH, DEFAULT_DIAM);
    selectedOb = numObs-1;
    break;
  case 's' :
    addNtube( SPHERE,  Vec3d( 0., 0., (DEFAULT_DIAM/2.)), 0., 0., 0., 0., DEFAULT_DIAM);
    selectedOb = numObs-1;
    break;
  case 't' :
    addTriangle(Vec3d(0.,0.,0.),Vec3d(DEFAULT_TRIANGLE_SIDE,0.,0.),Vec3d(DEFAULT_TRIANGLE_SIDE/2.,DEFAULT_TRIANGLE_SIDE/2.,DEFAULT_TRIANGLE_SIDE/2.));
    selectedOb = numObs-1;
    break;
    // dealing with tips
  case 'p' : // changes tip model
    tip.change_tip_model();
    break;
    /* In the foll cases, if the tip params are changed (for inv cone tip case)
       * we need to do the precomputation again for all tubes
       */
      // radius of the tip
  case 'r' :
    tip.inc_r();
    break;
  case 'R' :
    tip.dec_r();
    break;
  case 'a' ://change angle, slant of the tip
    tip.inc_theta();
    break;
  case 'A' ://change angle, slant of the tip
    tip.dec_theta();
    break;
  case '*' ://increase tesselation
    tesselation += 5;

#if DISP_LIST
    make_sphere();
    make_cylinder();
    make_cone_sphere(ics);
    tip.icsTip.set(tip.icsTip.r, tip.icsTip.ch, tip.icsTip.theta, tesselation);
#endif
    printf("Tesselation %d\n", tesselation);
    break;
  case '/' ://decrease tesselation
    if (tesselation > 5) { tesselation -= 5; }

#if DISP_LIST
    make_sphere();
    make_cylinder();
    make_cone_sphere(ics);
    tip.icsTip.set(tip.icsTip.r, tip.icsTip.ch, tip.icsTip.theta, tesselation);
#endif

    printf("Tesselation %d\n", tesselation);
    break;
  case 'i':
    if (afm_scan == NO_AFM) {
      afm_scan = SEMI_SOLID_AFM;
    }
    else if (afm_scan == SEMI_SOLID_AFM) {
      afm_scan = SOLID_AFM;
    }
    else
      afm_scan = NO_AFM;
    break;
  case 'w':
#if 1
    stopAFM=1;
  // write output to a file.
    {
	char filename[40];
	if (tip.type == SPHERE_TIP) {
	  sprintf(filename,"sptip_r_%.1lfnm.UNCA",tip.spTip.r);
	}
	else {
	  sprintf(filename,"icstip_r_%.1lfnm_ch_%.1lfnm_theta_%.1lfdeg.UNCA",tip.icsTip.r,tip.icsTip.ch,RAD_TO_DEG*tip.icsTip.theta);
	}
	cout << "Writing to file " << filename << endl;
	write_to_unca(filename);
	cout << "Finished writing to file " << filename << endl;
	stopAFM=0;
    }
#endif
    break;
  case 'q' :
    exit(0);
    break;
  default :
    if (selectedOb != NULLOB) {
      ob[selectedOb]->keyboardFunc(key,x,y);
    }
    else {
      globalkeyboardFunc(key,x,y);
    }
    break;
  }
  glutPostRedisplay();	// in case something was changed
}

// Callback routine: called for mouse button events.
void mouseFuncMain( int button, int state, int x, int y ) {

  int xy_or_xz;

  int win = glutGetWindow();

  if (win == mainWindowID) {
    xy_or_xz = XY_GRAB;
  }
  else if (win == viewWindowID) {
    xy_or_xz = XZ_GRAB;
  }

  calcMouseWorldLoc( x, y, xy_or_xz);

  switch( button ) {
  case GLUT_LEFT_BUTTON: 
    if(      state == GLUT_DOWN )	{buttonpress=LEFT_BUTTON;grabNearestOb(xy_or_xz);}
    else if( state == GLUT_UP )		{}
    break;
  case GLUT_RIGHT_BUTTON: 
    /* this selects one side of the triangle so that we can perform all
     * our nanotube operations on that side. 
     */
    if(      state == GLUT_DOWN )	{buttonpress=RIGHT_BUTTON; select_triangle_side();
    }
    else if( state == GLUT_UP )		{}
    break;
  }

  glutPostRedisplay();
}


// Callback routine: called when mouse is moved while a button is down.
// Only called when cursor loc changes.
// x,y:    cursor loc in window coords
// see p658 Woo 3rd ed
void mouseMotionFuncMain( int x, int y ) {
  int xy_or_xz;

  int win = glutGetWindow();

  if (win == mainWindowID) {
    xy_or_xz = XY_GRAB;
  }
  else if (win == viewWindowID) {
    xy_or_xz = XZ_GRAB;
  }

  if (buttonpress == LEFT_BUTTON) {
    // Map mouse cursor window coords to world coords.
    // Since we're using an orthoscopic projection parallel to the Z-axis,
    // we can map (x,y) in window coords to (x,y,0) in world coords.
    calcMouseWorldLoc( x, y, xy_or_xz);
    
    // Move the grabbed object, if any, to match mouse movement.
    //    moveGrabbedOb();
    ob[selectedOb]->moveGrabbedOb(vMouseWorld);

    
    //	glutPostRedisplay();
  }
}


// Calculate where the cursor maps to in world coordinates, 
// based on the window width and height and the edges of
// the frustum of the orthoscopic projection.
void calcMouseWorldLoc( int xMouse, int yMouse, int xy_or_xz ) {
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

  if (xy_or_xz == XY_GRAB) {
    // calculate cursor position in ortho frustum's XY plane
    vMouseWorld.x = (xMouseNormalized * orthoFrustumWidth)  + orthoFrustumLeftEdge;
    vMouseWorld.y = (yMouseNormalized * orthoFrustumHeight) + orthoFrustumBottomEdge;
    vMouseWorld.z = 0; 
  }
  else {
    // calculate cursor position in ortho frustum's XY plane
    vMouseWorld.y = 0; 
    vMouseWorld.x = (xMouseNormalized * orthoFrustumWidth)  + orthoFrustumLeftEdge;
    vMouseWorld.z = (yMouseNormalized * orthoFrustumHeight) + orthoFrustumBottomEdge;
  }
}


void grabNearestOb(int xy_or_xz) {
  selectedOb =  findNearestObToMouse(xy_or_xz);
  if (selectedOb == NULLOB) {
    return;
  }

  ob[selectedOb]->grabOb(vMouseWorld, xy_or_xz);
  ob[selectedOb]->moveGrabbedOb(vMouseWorld);
}

int findNearestObToMouse(int xy_or_xz) {
  int i;
  int nearestOb = NULLOB;
  double nearestDist = 1000000.;
  double thresholdDist = 20.;
  double dist;
  
  for( i=0; i<numObs; i++ ) {
    if (ob[i] == UNUSED) continue;
    if (xy_or_xz == XY_GRAB) {
      dist = ob[i]->xy_distance(vMouseWorld);
    }
    else {
      dist = ob[i]->xz_distance(vMouseWorld);
    }
    if( dist < nearestDist  &&  dist < thresholdDist ) {
      nearestDist = dist;
      nearestOb   = i;
    }
  }
  return nearestOb;
}

/* XXXX : Add xy or yz mode */
void findNearestTriangleSideToMouse( void ) {
  int i;
  double nearestDist = 1000000.;
  double dist;
  int nearestTriangle;
  
  for( i=0; i<numObs; i++ ) {
    if (ob[i] == UNUSED) continue;
    if (ob[i]->type == TRIANGLE) {
      Triangle *tri = (Triangle *) ob[i];
      double dist1 = tri->ab.xy_distance( vMouseWorld);
      double dist2 = tri->bc.xy_distance( vMouseWorld);
      double dist3 = tri->ca.xy_distance( vMouseWorld);

      if (dist2 < dist3) {
	if (dist1 < dist2) {
	  dist = dist1;
	  selected_triangle_side = 1;
	}
	else {
	  dist = dist2;
	  selected_triangle_side = 2;
	}
      }
      else {
	if (dist1 < dist3) {
	  dist = dist1;
	  selected_triangle_side = 1;
	}
	else {
	  dist = dist3;
	  selected_triangle_side = 3;
	}
      }
      if (dist < nearestDist) {
	nearestTriangle = i;
	nearestDist = dist;
      }
    }
  }

  selectedOb = nearestTriangle;
}

void select_triangle_side() {
  findNearestTriangleSideToMouse();
}





