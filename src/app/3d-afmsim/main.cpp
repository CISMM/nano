/*$Id$*/
#include <stdlib.h>		//stdlib.h vs cstdlib
#include <stdio.h>		//stdio.h vs cstdio
#include <iostream.h>
#include <vector>
#include <math.h>		//math.h vs cmath
#include <GL/glut.h>
#include "vec3d.h"
#include "3Dobject.h"
#include "ConeSphere.h"
#include "Tips.h"
#include <string.h>
#include "defns.h"
#include "Unca.h"

GLuint list_sphere;
GLuint list_cylinder;

#define MAXOBS 100000
#define MAXVERTICES 10000

#define MAX_GRID 128 // changed this to 128 from 64
// drawing styles
#define OB_SOLID		0
#define OB_WIREFRAME	1
#define OB_POINTS		2
#define OB_SILHOUETTE	3
#define OB_OUTLINE2D	4
#define OB_NONE			5
#define DEPTHSIZE MAX_GRID


typedef int Bool;
static int dblBuf  = GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH;
//static int singleBuf  = GLUT_SINGLE | GLUT_RGBA | GLUT_DEPTH;
Ntube nullntube;

OB *ob[MAXOBS];
Vec3d vertex[MAXVERTICES];

int numObs;

int selectedOb = NULLOB;
int buttonpress=-1;
int selected_triangle_side;

#define NO_AFM 0
#define SEMI_SOLID_AFM 1
#define SOLID_AFM 2

int afm_scan=SEMI_SOLID_AFM;
int draw_objects=0;


//int solid_afm_scan=0;
//int enable_afm=1;

int rotate_and_write_to_file=0;
int done_monster_process_x=0, done_monster_process_y=0;

// display options
int renderStyle = OB_OUTLINE2D;
GLenum shadingModel = GL_FLAT;   // GL_FLAT or GL_SMOOTH
Bool lightOn[8] = { 1, 1, 0, 0, 0, 0, 0, 0 };
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
// raw values (normalized) from Z-buffer
float zBuffer[ 128*128 ];			
// array of heights: image scan data
double zHeight        [MAX_GRID][MAX_GRID];	

// scan grid resolution
int    scanResolution = 128;	
// scan grid pitch (sample-to-sample spacing)
double scanStep   = 1.;		
// scan grid origin X coord (left side)
double scanXMin =  0.;		
// scan grid origin Y coord (bottom)
double scanYMin =  0.;		
//double scanLength = scanStep * scanResolution;	
//double scanXMax =   scanXMin + (scanStep * scanResolution);
//double scanYMax =   scanYMin + (scanStep * scanResolution);
double minZval=0;
// these might get changed in initSpheresFromFile

//double scanNear =  -100.;	// near end of Z-buffer range
double scanNear =  -128.;	// near end of Z-buffer range

double scanFar  =   0.;	// far  end of Z-buffer range
// mouse and cursor
int xMouseInWindow;	// mouse position in world coords
int yMouseInWindow;	// mouse position in world coords
Vec3d vMouseWorld;	// mouse position in world coords (actually a 2D vector in XY plane)
Vec3d vGrabOffset;	// offset from cursor position to grabbed object (in world coords actually a 2D vector in XY plane)

Vec3d centroid;


int stopAFM=0;
int done_afm_scan=0;
int done_drawing_objects=0;
double thetax=0.,thetay=0.;
double cone_sphere_list_radius;
int tesselation = 30;

/* Here are our AFM tips */
// third arg is the default

// all units in nm
// XXX Edit the following lines to adjust the radius and tilt of the tip
SphereTip sp(5.);
InvConeSphereTip ics(5.,1000.,DEG_TO_RAD*30.,tesselation);
Tip tip(sp,ics,tesselation,INV_CONE_SPHERE_TIP);
//Tip tip(sp,ics,SPHERE_TIP);


// FUNCTION PROTOYPES
void	setMaterialColor( GLfloat r, GLfloat g, GLfloat b );
void	setColor( int colorIndex );
// draws unit sphere
//void	drawSphere(double radius);
//void	drawCylinder( double diameter, double height );
//void	drawTube( double diameter, double length);
void	lighting( void );
void	addbject(int type, Vec3d pos, double yaw, double roll, double pitch, double leng, double diam,
		 int nextSeg, int prevSeg);
void	addTriangle(Vec3d a, Vec3d b, Vec3d c);
void	drawObjects( void );
void	drawFrame( void );
void	adjustOrthoProjectionParams( void );
void	adjustOrthoProjectionToWindow( void );
void	adjustOrthoProjectionToViewWindow( void );
void	reshapeWindow( int newWindowWidth, int newWindowHeight );
void	initObs( void );
void	displayFuncMain( void );
void	displayFuncView( void );
void	commonIdleFunc( void );
void	idleFuncDummy( void );
int	main(int argc, char *argv[]);
void	doImageScanApprox( void );
void	imageScanDepthRender( void );
void	showGrid( void );
void	displayFuncDepth( void );
void	reshapeWindowFuncDummy( int newWindowWidth, int newWindowHeight );
void	commonKeyboardFunc(unsigned char key, int x, int y);
void	mouseFuncMain( int button, int state, int x, int y );
void	mouseMotionFuncMain(int x, int y );
void	calcMouseWorldLoc( int xMouse, int yMouse );
double  norm_xy( Vec3d v );
double  vec_xy_Distance( Vec3d pt1, Vec3d pt2 );
int     findNearestObToMouse( void );
void    moveGrabbedOb( void );
void    grabNearestOb( void );
void    select_triangle_side(void);
void write_to_unca(char *filename);

/* We have a type field. Later, I plan to distingush spheres from ntubes */
void
addNtube(int type, Vec3d pos, double yaw, double roll, double pitch, double leng, double diam,
	 int nextSeg, int prevSeg )
{
  ob[numObs] = new Ntube(type,pos,yaw,roll,pitch,leng,diam,nextSeg,prevSeg);
  selectedOb = numObs;
  numObs++;
}

void addTriangle(Vec3d a, Vec3d b, Vec3d c) {
  ob[numObs] = new Triangle(a,b,c);
  selectedOb = numObs;
  numObs++;
}

// Draw the objects.
void
drawObjects( void )
{
  int i;
  
  // draw the objects
  for( i=0; i<numObs; i++ ) {
    if (ob[i] == UNUSED) continue;

    // we want different colors for our objects
    if ((i % 3) == 0)
      setColor( YELLOW );
    else if ((i % 3) == 1)
      setColor( GREEN );
    else 
      setColor( BLUE );
    // current object is red
    if ( i == selectedOb ) setColor( RED );


    ob[i]->draw();
    
    
    // to be able to select individual side of a triangle, we do this
    if ((buttonpress == RIGHT_BUTTON) && (glutGetWindow() == mainWindowID)) {
      if (ob[i]->type == TRIANGLE) {
	Ntube temp;
	Triangle tri = *(Triangle *) ob[selectedOb];
	setColor(MAGENTA);
	//    cout << selected_triangle_side << endl;
	switch (selected_triangle_side) {
	case 1 : 
	  temp = tri.ab;
	  break;
	case 2 : 
	  temp = tri.bc;
	  break;
	case 3 : 
	  temp = tri.ca;
	  break;
	default :
	  cout << "Error (main.c++)\n";
	  exit(0);
	  break;
	}
	temp.setDiam(2);
	temp.draw();
      }
    }
  }
}


void
drawFrame( void )
{
  // Setup OpenGL state.
  glClearDepth(1.0);
  glClearColor(0.5, 0.5, 0.5, 0.0);   // gray background

  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  glEnable(GL_DEPTH_TEST);

  shadingModel = GL_SMOOTH;
  lighting();
  glPointSize( 2. );    // se p51 Woo 3rd ed
  glLineWidth( 2. );

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  if (draw_objects) {
    drawObjects(); // Draw objects
  }

  if (afm_scan!=NO_AFM) {
    // Draw the image scan grid.
    showGrid();
  }
  // end of display frame, so flip buffers
  glutSwapBuffers();
}

#if DISP_LIST
// display list for a sphere
void make_sphere() {
  static GLUquadricObj* qobj;

  qobj = gluNewQuadric();
  gluQuadricDrawStyle( qobj, GLU_FILL);
  gluQuadricNormals( qobj, GLU_FLAT );
  
  // Create a display list for a sphere
  glNewList(SPHERE_LIST, GL_COMPILE);
  // draw a sphere of radius 1
  gluSphere( qobj, 1, tesselation, tesselation);
  // End definition of circle
  glEndList();
}

// display list for a sphere
void make_cylinder() {
  static GLUquadricObj* qobj;

  qobj = gluNewQuadric();
  gluQuadricDrawStyle( qobj, GLU_FILL);
  gluQuadricNormals( qobj, GLU_FLAT );
  
  // Create a display list for a sphere
  glNewList(CYLINDER_LIST, GL_COMPILE);
  // draw a cylinder of top and base radius of 1 and height 1
  gluCylinder( qobj, 1, 1, 1, tesselation, tesselation);
  glEndList();
}

void make_cone_sphere() {
  static GLUquadricObj* qobj;

  printf("Building ConeSphere display list (tesselation %d)\n",
	tesselation);

  qobj = gluNewQuadric();
  gluQuadricDrawStyle( qobj, GLU_FILL);
  gluQuadricNormals( qobj, GLU_FLAT );

  // here we consider the AFM of a sphere of 0 radius.

  double tipRadius = ics.r; // the radius for the tip
  double theta = ics.theta; // theta for the tip
  double radius=0.; 
  
  double bignum = 200; // some big no for now

  double cone_height = bignum + (radius+tipRadius)/sin(theta);
  ConeSphere c = ConeSphere(radius+tipRadius, cone_height, theta);
  //  c.print();
  //  exit(0);

  cone_sphere_list_radius = tipRadius;  
  // Create a display list for a sphere
  glNewList(CONE_SPHERE_LIST, GL_COMPILE);
  gluSphere( qobj, tipRadius, tesselation, tesselation);
  glPushMatrix();
  glTranslatef(0,0,-bignum);
  gluCylinder( qobj, c.cr, c.topRadius, c.topHeight, tesselation, tesselation);
  glPopMatrix();
  glEndList();
}

#endif


void
drawSphere( double diameter)
{
#if (DISP_LIST == 0)
  static int firstTime = 1;
  static GLUquadricObj* qobj;
  if( firstTime ) { 
    qobj = gluNewQuadric();
    gluQuadricDrawStyle( qobj, GLU_FILL);
    gluQuadricNormals( qobj, GLU_FLAT );
    firstTime=0;
  }
  gluSphere( qobj, diameter/2., tesselation, tesselation);
#else
  glPushMatrix();
  glScalef(diameter/2., diameter/2., diameter/2);
  glCallList(SPHERE_LIST);
  glPopMatrix();
#endif
}

void
drawCylinder( double diameter, double height)
{
#if (DISP_LIST == 0)
  static int firstTime = 1;
  static GLUquadricObj* qobj;
  if( firstTime ) { 
    qobj = gluNewQuadric();
    gluQuadricDrawStyle( qobj, GLU_FILL);
    gluQuadricNormals( qobj, GLU_FLAT );
    firstTime=0;
  }
  gluCylinder( qobj, diameter/2., diameter/2., height, tesselation, tesselation);
#else
  glPushMatrix();
  glScalef(diameter/2.,diameter/2.,height);
  glCallList(CYLINDER_LIST);
  glPopMatrix();
#endif
}


/* Here are our object. Note we want our objects to be above the surface
 * i.e z >= 0
 */
void
initObs( void )
{
  // We start with no objects.
  numObs = 0;

  // a nano tube
  // addNtube(NTUBE,  Vec3d( 25., 30., 45.), 0., 40., 0., 20, 10.,   NULLOB, NULLOB);
  addNtube( NTUBE,  Vec3d( 50., 60., 50.), 0., 0., 0., 20, 10.,   NULLOB, NULLOB);
  //addNtube( SPHERE,  Vec3d( 50., 60., 50.), 0., 0., 0., 0., 10.,   NULLOB, NULLOB);

  //  addTriangle(Vec3d(55.8418,20.437,20), Vec3d(81.6421,45.6232,35), Vec3d (42.5162,53.9397,35));
  //  addTriangle(Vec3d(60,20,20), Vec3d(80,50,35), Vec3d (40,50,35));
  //  addTriangle(Vec3d(20,20,20), Vec3d(80,20,20), Vec3d (20,80,20));
  //  tri[numtri-1].print();
}

/* Give me the unit in terms of nm. So if the unit assumed in the file is 
 * Angstrom, give me 0.1 (since 1 A = 0.1 nm)
 */
void addSpheresFromFile (char *filename, double no_of_nm_in_one_unit) {
  double x,y,z;
  int stop;
  double minx=0.,miny=0.,minz=0., maxx=0., maxy=0., maxz=0.;

  FILE *file = fopen(filename,"r"); 

  cout << "Loading file " << filename << endl;

  stop=0;
  while (!stop) {
    fscanf(file,"%lf",&x);
    if (!feof(file)) {
      fscanf(file,"%lf",&y);
      fscanf(file,"%lf",&z);
      // unit conversion - everything will be in nm from now on.
      x *= no_of_nm_in_one_unit;
      y *= no_of_nm_in_one_unit;
      z *= no_of_nm_in_one_unit;
      // assume a radius of 1.5 A
      double rad = 1.5*no_of_nm_in_one_unit;
      // need to do some profiling for later.
      minx = ((!minx) || (x < minx)) ? x : minx;
      miny = ((!miny) || (z < miny)) ? y : miny;
      minz = ((!minz) || (z < minz)) ? z : minz;
      maxx = ((!maxx) || (x > maxx)) ? x : maxx;
      maxy = ((!maxy) || (y > maxy)) ? y : maxy;
      maxz = ((!maxz) || (z > maxz)) ? z : maxz;
      addNtube( SPHERE,  Vec3d( x, y, z), 0., 0., 0., 1., rad,   NULLOB, NULLOB);
    }
    else
      stop=1;
  }

  cout << "Done no of spheres = " << (numObs-1) << endl;


  /* now use the profiled data to translate and scale the values to lie in
   * our orthogonal volume
   */
  /*Place the "XY centroid" of the body at the centre. Also let the lowest
   * pt lie just on the surface.
   */
  Vec3d translate = Vec3d(-(minx+maxx)/2.,-(miny+maxy)/2.,-minz) +
    Vec3d(orthoFrustumLeftEdge+orthoFrustumWidth/2.,orthoFrustumBottomEdge+orthoFrustumHeight/2.,0.);
  for (int i=0;i<numObs;i++) {
    ob[i]->translate(translate);
  }
  centroid = Vec3d((minx+maxx)/2.,(miny+maxy)/2.,(minz+maxz)/2.)+translate;
    
}

// give me the scale
void addTrianglesFromFile(char *filename, double scale) {
  FILE *file = fopen(filename,"r"); 
  int stop=0;
  char c;
  double x,y,z;
  int v1,v2,v3;
  double minx=0.,miny=0.,minz=0., maxx=0., maxy=0., maxz=0.;

  cout << "Loading file " << filename << endl;

  int cnt=1; // waste the first element
  while (!stop) {
    fscanf(file,"%c",&c);
    if (!feof(file)) {
      if (c == 'v') {
	// read the vertices
	fscanf(file,"%lf",&x);
	fscanf(file,"%lf",&y);
	fscanf(file,"%lf",&z);
	if (cnt < MAXVERTICES) {
	  vertex[cnt] = Vec3d(x,y,z);
	  cnt++;
	}
	else {
	  cout << "Error: Too many vertices\n";
	  exit(0);
	}
      }
      else if (c == 'f') {
	// read the vertex numbers
	fscanf(file,"%d",&v1);
	fscanf(file,"%d",&v2);
	fscanf(file,"%d",&v3);
	addTriangle(vertex[v1],  vertex[v2],  vertex[v3]);  
	// need to do some profiling for later.
	minx = ((!minx) || (vertex[v1].x < minx)) ? vertex[v1].x : minx;
	maxx = ((!maxx) || (vertex[v1].x > maxx)) ? vertex[v1].x : maxx;
	miny = ((!miny) || (vertex[v1].y < miny)) ? vertex[v1].y : miny;
	maxy = ((!maxy) || (vertex[v1].y > maxy)) ? vertex[v1].y : maxy;
	minz = ((!minz) || (vertex[v1].z < minz)) ? vertex[v1].z : minz;
	maxz = ((!maxz) || (vertex[v1].z > maxz)) ? vertex[v1].z : maxz;


      }
    }
    else {
      stop = 1;
    }
  }

  cout << "Done vertices = " << (cnt-1) << " triangles = " << (numObs-1) << endl;
  /* now use the profiled data to translate and scale the values to lie in
   * our orthogonal volume
   */
  /*Place the "XY centroid" of the body at the centre. Also let the lowest
   * pt lie just on the surface.
   */
#if 1
  Vec3d negcentroid = Vec3d(-(minx+maxx)/2.,-(miny+maxy)/2.,-(minz+maxz)/2.);
  Vec3d center = Vec3d(orthoFrustumLeftEdge+orthoFrustumWidth/2.,orthoFrustumBottomEdge+orthoFrustumHeight/2.,0.);
  for (int i=0;i<numObs;i++) {
    ob[i]->translate(negcentroid);
    ob[i]->scale(scale);
    // lift up so that body just sits on the surface
    ob[i]->translate(Vec3d(0,0,scale*(maxz-minz)/2.));
    // positon the centroid of the body at the centre.
    ob[i]->translate(center);
  }
#endif
}

/* These are meant to rotate the protein molecule along X and Y by 20 degrees.
 * and dump the AFM images
 * assuming here protein is made up of spheres
 *
 */
void monster_process_x() {
  char filename[100];
  double minz=0.;

  // stop AFM for now
  stopAFM=1;

  // write output to a file.
  if (tip.type == SPHERE_TIP) {
    sprintf(filename,"rotated_anglex_%.1lf_sptip_r_%.1lf.UNCA",thetax,tip.spTip.r);
  }
  else {
    sprintf(filename,"rotated_anglex_%.1lf_icstip_r_%.1lf_ch_%.1lf_theta_%.1lf.UNCA",thetax,tip.icsTip.r,tip.icsTip.ch,RAD_TO_DEG*tip.icsTip.theta);
  }
  
  cout << "Writing to file " << filename << endl;
  write_to_unca(filename);
  cout << "Finished writing ..\n";
  
  if (thetax >= XMAX) {
    done_monster_process_x=1;
    stopAFM=0;
    return;
  }
  thetax += XLEAST_COUNT;

  for (int i=0;i<numObs;i++) {
    if (ob[i] == UNUSED) continue;
    Vec3d pos = ob[i]->pos;
    ob[i]->setPos(centroid + Vec3d(pos-centroid).rotate3(Vec3d(1,0,0),DEG_TO_RAD*XLEAST_COUNT));
    minz = ((!minz) || (ob[i]->pos.z < minz)) ? ob[i]->pos.z : minz;
  }

  /* not done yet - have to place the protein such that its bottom most point
   * just touches the surface 
   */
  for (i=0;i<numObs;i++) {
    if (ob[i] == UNUSED) continue;
    ob[i]->setPos_z(ob[i]->pos.z-minz);
  }
  stopAFM=0;
}

void monster_process_y() {
  char filename[100];
  double minz=0.;

  stopAFM=1;

  // write output to a file.
  if (tip.type == SPHERE_TIP) {
    sprintf(filename,"rotated_angley_%.1lf_sptip_r_%.1lf.UNCA",thetay,tip.spTip.r);
  }
  else {
    sprintf(filename,"rotated_angley_%.1lf_icstip_r_%.1lf_ch_%.1lf_theta_%.1lf.UNCA",thetay,tip.icsTip.r,tip.icsTip.ch,RAD_TO_DEG*tip.icsTip.theta);
  }
  
  cout << "Writing to file " << filename << endl;
  write_to_unca(filename);
  cout << "Finished writing ..\n";
  
  if (thetay >= YMAX) {
    done_monster_process_y=1;
    exit(0);
  }
  thetay += YLEAST_COUNT;

  for (int i=0;i<numObs;i++) {
    if (ob[i] == UNUSED) continue;
    Vec3d pos = ob[i]->pos;
    ob[i]->setPos(centroid + Vec3d(pos-centroid).rotate3(Vec3d(0,1,0),DEG_TO_RAD*YLEAST_COUNT));
    minz = ((!minz) || (ob[i]->pos.z < minz)) ? ob[i]->pos.z : minz;
  }

  /* not done yet - have to place the protein such that its bottom most point
   * just touches the surface 
   */
  for (i=0;i<numObs;i++) {
    if (ob[i] == UNUSED) continue;
    ob[i]->setPos_z(ob[i]->pos.z-minz);
  }
  stopAFM=0;
}


int
main(int argc, char *argv[])
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
    else {
      cout << "\nUsage : ./sim [-p filename unit [-w]] for proteins\n";
      cout << "        ./sim [-t filename scale] for triangulated models\n\nSee README\n\n";
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
  glutInitWindowPosition( 0, 0 );
  mainWindowID = glutCreateWindow( "3D AFM simulator - Top View" );
  adjustOrthoProjectionToWindow();

#if DISP_LIST
  make_sphere();
  make_cylinder();
  make_cone_sphere();
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

#if DISP_LIST
  make_sphere();
  make_cylinder();
  make_cone_sphere();
#endif

  // pass pointers to callback routines for the other view window
  glutDisplayFunc(displayFuncView);
  glutIdleFunc(idleFuncDummy );
  //  glutReshapeFunc(reshapeWindow);
  glutKeyboardFunc(commonKeyboardFunc);
#endif

  // Depth WINDOW
  glutInitWindowSize( (int)DEPTHSIZE, (int)DEPTHSIZE );
  glutInitWindowPosition( 0, 650 );
  depthWindowID = glutCreateWindow( "Depth window" );

#if DISP_LIST
  make_sphere();
  make_cylinder();
  make_cone_sphere();
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
void
displayFuncMain( void )
{
  if (!stopAFM) {
    glutSetWindow( mainWindowID );
    // draw graphics for this frame
    drawFrame();
  }
#if 0
  if (done_afm_scan) {
    //    stopAFM = 1;
    //    write_to_unca("try.out");
    cout <<"done\n";
    exit(0);
  }
#endif
}

void
displayFuncView( void )
{
  if (!stopAFM) {
    glutSetWindow( viewWindowID );
    glPushMatrix();
    glRotatef(-90, 1.0, 0.0, 0.0 ); 
    drawFrame();
    glPopMatrix();
  }
}

// This is the callback for rendering into the depth buffer window,
// as required by "doImageScanApprox".  
void 
displayFuncDepth( void ) 
{
  if (!stopAFM) {
    glutSetWindow( depthWindowID );
    // in Z-buffer of Depth Window using graphics hardware.
    doImageScanApprox();
  }
}

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

// This idle function marks both windows for redisplay, which will cause
// their display callbacks to be invoked.
void
commonIdleFunc( void )
{
  glutSetWindow( mainWindowID );		glutPostRedisplay();
  glutSetWindow( viewWindowID );		glutPostRedisplay();
  glutSetWindow( depthWindowID );		glutPostRedisplay();
}

void idleFuncDummy( void ) {commonIdleFunc();}
void reshapeWindowFuncDummy( int newWindowWidth, int newWindowHeight ) {}

// callback routine: called when window is resized by user
void
reshapeWindow( int newWindowWidth, int newWindowHeight )
{
  windowWidth  = newWindowWidth;
  windowHeight = newWindowHeight;

  // viewport covers whole window
  glViewport( 0, 0, (int)windowWidth, (int)windowHeight );

  // make graphics projection match window dimensions
  adjustOrthoProjectionToWindow();
}


void
adjustOrthoProjectionParams( void ) {
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
void
adjustOrthoProjectionToWindow( void )
{
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
void
adjustOrthoProjectionToViewWindow( void )
{
  double orthoFrustumNearEdge =  -scanFar;
  double orthoFrustumFarEdge  =   -scanNear;
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glOrtho(  orthoFrustumLeftEdge,   orthoFrustumLeftEdge   + orthoFrustumWidth,
	    orthoFrustumNearEdge,   orthoFrustumFarEdge, 
	    orthoFrustumBottomEdge, orthoFrustumBottomEdge + orthoFrustumHeight);
}

void
error( char* errMsg )
{
  printf( "\nError: %s\n", errMsg );
  exit( 1 );
}

int cnt=0;
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
  int pixelGridSize = DEPTHSIZE;		// must match window size
  // width and height the same for now
  glReadPixels( 0, 0, pixelGridSize, pixelGridSize, GL_DEPTH_COMPONENT, GL_FLOAT, zBufferPtr );
  for( j=0; j<scanResolution; j++ ) {
    for( i=0; i<scanResolution; i++ ) {
      double zNormalized = zBuffer[ j*pixelGridSize + i ];
      //      double zDepth = scanFar + zNormalized * (scanNear - scanFar);
      // changed this : mapping the far plane to a height zero

      //      double zDepth = minZval + (1-zNormalized)*abs(scanNear - scanFar);
      double zDepth = scanFar + (1-zNormalized)*abs(scanNear - scanFar);
      zHeight[i][j] = zDepth;
      //      zHeight[i][j] = 0.;
    }
  }
  done_afm_scan=1;
  cnt++;
#if 1
  if (rotate_and_write_to_file) {
    if (cnt >= 2) {// for startup transients to finish !!!
      if (!done_monster_process_x) {
	monster_process_x();
      }
      else {
	monster_process_y();
      }
    }
  }
#endif
}


/* This is the most critical part of the afmsim. This is the one which 
 * renders the afm scan
 */
// display graphics in the depth window.
void 
imageScanDepthRender( void )  {
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

  setColor( WHITE );

  for( int i=0; i<numObs; i++ ) {
    if (ob[i] == UNUSED) continue;	  
    switch (tip.type) {
    case SPHERE_TIP :
      glPushMatrix();
      // go down by tip radius
      glTranslatef( 0., 0., -tip.spTip.r);  
      ob[i]->afm_sphere_tip(tip.spTip);
      glPopMatrix();
      break;
    case INV_CONE_SPHERE_TIP :
      glPushMatrix();
      // go down by tip radius
      glTranslatef( 0., 0., -tip.icsTip.r);  
      ob[i]->afm_inv_cone_sphere_tip(tip.icsTip);
      glPopMatrix();
      break;
    }
  }

  // end of display frame, so flip buffers
  glutSwapBuffers();
  //glFlush();
  //glFinish();

  renderStyle = saveRenderStyle;
}


// Display the image scan grid (a depth image).
void
showGrid( void )
{
  int gridColor = GREEN;

  // Display depth image surface.  
  // The variable "gridStyle" controls which of several visualizations
  // of the surface are used.
  shadingModel = GL_SMOOTH;
  lighting();
  setColor( gridColor );

  for( int i=0; i<scanResolution-1; i++ ) {
    for( int j=0; j<scanResolution-1; j++ ) {
      double x = i * scanStep  +  scanXMin;
      double y = j * scanStep  +  scanYMin;
      double dx = scanStep;
      double dy = scanStep;

      // Get the the 4 (x,y,z) coords on the corners of this grid cell.
      // Show objects above the surface only
      double x1 = x;     double y1 = y;     double z1 = zHeight[i  ][j  ];
      double x2 = x+dx;  double y2 = y;     double z2 = zHeight[i+1][j  ];
      double x3 = x+dx;  double y3 = y+dy;  double z3 = zHeight[i+1][j+1];
      double x4 = x;     double y4 = y+dy;  double z4 = zHeight[i  ][j+1];

      // calc normal to plane through P1, P2, P3 using (P1-P3) x (P1-P2)

      Vec3d A = Vec3d(x1,y1,z1);
      Vec3d B = Vec3d(x2,y2,z2);
      Vec3d C = Vec3d(x3,y3,z3);
      Vec3d D = Vec3d(x4,y4,z4);
      Vec3d xyz = Vec3d :: crossProd(A-B,A-C);
      xyz.normalize();

#if 0
      if (z1 > 5) {
	glBegin(GL_POINTS);
	glVertex3f( x1, y1, z1 );
	//	cout << x1 << " " << y1 << endl;
	glEnd();
      }
#endif
	

#if 1
      // draw the triangle 
      glBegin(GL_POLYGON);
      //      glNormal3f(xn,yn,zn);
      glNormal3f( xyz.x, xyz.y, xyz.z);
      glVertex3f( x1, y1, z1 );
      glVertex3f( x2, y2, z2 );
      glVertex3f( x3, y3, z3 );
      glEnd();
      if (afm_scan==SOLID_AFM) {      
	Vec3d xyz2 = Vec3d :: crossProd(A-C,A-D);
	xyz2.normalize();
	
	glBegin(GL_POLYGON);
	glNormal3f( xyz2.x, xyz2.y, xyz2.z );
	glVertex3f( x3, y3, z3 );
	glVertex3f( x4, y4, z4 );
	glVertex3f( x1, y1, z1 );
	glEnd();
      }
#endif
    }

  }

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
void
commonKeyboardFunc(unsigned char key, int x, int y)
{
  switch (key) {
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
    addNtube( NTUBE,  Vec3d( 0., 0., (DEFAULT_DIAM/2.)), 0., 0., 0., DEFAULT_LENGTH, DEFAULT_DIAM,   NULLOB, NULLOB);
    selectedOb = numObs-1;
    break;
  case 's' :
    addNtube( SPHERE,  Vec3d( 0., 0., (DEFAULT_DIAM/2.)), 0., 0., 0., 0., DEFAULT_DIAM,   NULLOB, NULLOB);
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
    make_cone_sphere();
    tip.icsTip.set(tip.icsTip.r, tip.icsTip.ch, tip.icsTip.theta, tesselation);
#endif
    printf("Tesselation %d\n", tesselation);
    break;
  case '/' ://decrease tesselation
    if (tesselation > 5) { tesselation -= 5; }
#if DISP_LIST
    make_sphere();
    make_cylinder();
    make_cone_sphere();
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
void
mouseFuncMain( int button, int state, int x, int y )
{
  calcMouseWorldLoc( x, y );

  switch( button ) {
  case GLUT_LEFT_BUTTON: 
    if(      state == GLUT_DOWN )	{buttonpress=LEFT_BUTTON;grabNearestOb();}
    else if( state == GLUT_UP )		{}
    break;
  case GLUT_RIGHT_BUTTON: 
    /* this selects one side of the triangle so that we can perform all
     * our nanotube operations on that side. 
     */
    if(      state == GLUT_DOWN )	{buttonpress=RIGHT_BUTTON; select_triangle_side();}
    else if( state == GLUT_UP )		{}
    break;
  }

  glutPostRedisplay();
}


// Callback routine: called when mouse is moved while a button is down.
// Only called when cursor loc changes.
// x,y:    cursor loc in window coords
// see p658 Woo 3rd ed
void
mouseMotionFuncMain( int x, int y )
{

  if (buttonpress == LEFT_BUTTON) {
    // Map mouse cursor window coords to world coords.
    // Since we're using an orthoscopic projection parallel to the Z-axis,
    // we can map (x,y) in window coords to (x,y,0) in world coords.
    calcMouseWorldLoc( x, y );
    
    // Move the grabbed object, if any, to match mouse movement.
    moveGrabbedOb();
    
    //	glutPostRedisplay();
  }
}


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
  vMouseWorld.z = 0; 
}

Vec3d oldMousePos;

// search for nearest ob to cursor, and set grab offset vector.
void
grabNearestOb( void )
{
  selectedOb =  findNearestObToMouse();
  // calculate grab offset vector
  if( selectedOb != NULLOB ) {
    vGrabOffset = ob[selectedOb]->pos   - vMouseWorld;
  }
  moveGrabbedOb();
}

void
moveGrabbedOb( void )
{
  // move the selected object, if any, to correspond with the mouse movement
  if( selectedOb != NULLOB) {
    ob[selectedOb]->setPos(Vec3d((vMouseWorld.x + vGrabOffset.x),
				 (vMouseWorld.y + vGrabOffset.y),
				 ob[selectedOb]->pos.z));
    
  }
}

// searches through all objects to find the object nearest the mouse cursor.
// returns the index of the object, or NULLOB if no objects are within threshold.
int
findNearestObToMouse( void ) {
  int i;
  int nearestOb = NULLOB;
  double nearestDist = 1000000.;
  double thresholdDist = 20.;
  double dist;
  
  for( i=0; i<numObs; i++ ) {
    if (ob[i] == UNUSED) continue;
    dist = vec_xy_Distance( vMouseWorld, ob[i]->pos );
    if( dist < nearestDist  &&  dist < thresholdDist ) {
      nearestDist = dist;
      nearestOb   = i;
    }
  }
  return nearestOb;
}

void
findNearestTriangleSideToMouse( void ) {
  int i;
  double nearestDist = 1000000.;
  double dist;
  int nearestTriangle;
  
  for( i=0; i<numObs; i++ ) {
    if (ob[i] == UNUSED) continue;
    if (ob[i]->type == TRIANGLE) {
      Triangle *tri = (Triangle *) ob[i];
      double dist1 = vec_xy_Distance( vMouseWorld, tri->ab.pos );
      double dist2 = vec_xy_Distance( vMouseWorld, tri->bc.pos );
      double dist3 = vec_xy_Distance( vMouseWorld, tri->ca.pos );

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

double
vec_xy_Distance( Vec3d pt1, Vec3d pt2 )
{
  return norm_xy( pt1 - pt2 );
}

// norm in x and y only
double
norm_xy( Vec3d v )
{
  return sqrt( (v.x * v.x)  +  (v.y * v.y) );
}

  
