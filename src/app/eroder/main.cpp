/* 
 * Eroder 
 * by 
 * Andrea Hilchey
 * hilchey@cs.unc.edu
 * June 2002
 *
 * adapted from AFM simulator
 * written by 
 * Gokul Varadhan
 * varadhan@cs.unc.edu
 * May 2001
 *
 * 3D AFM simulator.
 */
#include <stdlib.h> //stdlib.h vs cstdlib
#include <stdio.h> //stdio.h vs cstdio
#include <iostream>
#include <fstream>
#include <vector>
#include <math.h> //math.h vs cmath
#include <GL/glut_UNC.h>
#include "Vec3d.h"
#include "ConeSphere.h"
#include "Tips.h"
#include <string.h>
#include "defns.h"
#include "Unca.h"
#include "Uncertw.h"
#include "input.h"
#include "erode.h"
#include "draw.h"
#include "lightcol.h"
#include "uncert.h"
#include "main.h"
#include <vrpn_Connection.h>
#include <nmm_SimulatedMicroscope.h>
#if !defined(_WIN32) || defined(__CYGWIN__)
#include <sys/time.h>
#else
#include <vrpn_Shared.h> // get timeval some other way
#endif


GLuint list_sphere;
GLuint list_cylinder;

static int dblBuf = GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH;

int buttonpress=-1;

int selected_triangle_side;

//added by ANDREA
double unca_minx = -1, unca_miny = -1, unca_maxx = -1, unca_maxy = -1;
double ** HeightData;
bool connection_to_nano = false;
//end

int uncertainty_mode=0;

#define NO_AFM 0
#define SEMI_SOLID_AFM 1
#define SOLID_AFM 2

ofstream fout;

int afm_scan=SEMI_SOLID_AFM;
//int afm_scan=NO_AFM;
int draw_objects=1;
double view_angle = -90.;

//int solid_afm_scan=0;
//int enable_afm=1;

// window stuff
int mainWindowID, viewWindowID, depthWindowID;
double windowWidth = xResolution;
double windowHeight = yResolution;
double orthoFrustumCenterX = xResolution/2; // area of XY plane always visible for all window aspect ratios
double orthoFrustumCenterY = yResolution/2;
double orthoFrustumWidthNominal = xResolution;//***
double orthoFrustumHeightNominal = yResolution;//***

// actual bounds of current ortho view frustum matching window aspect ratio
double orthoFrustumLeftEdge;
double orthoFrustumBottomEdge;
double orthoFrustumWidth;
double orthoFrustumHeight;

// mouse and cursor
int xMouseInWindow; // mouse position in world coords
int yMouseInWindow; // mouse position in world coords
Vec3d vMouseWorld; // mouse position in world coords (actually a 2D vector in XY plane)

int stopAFM=0;
int done_drawing_objects=0;
double thetax=0.,thetay=0.;
int tesselation = 30;

char units[100];
char* VolumeFilename;
float TipSize;

void write_to_unca(char *filename);
void displayFuncMain( void );
void displayFuncView( void );
void displayFuncDepth( void );
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
void Usage(char *progname);
void glutInitializeStuff();

//has to be put out here so variables that can be accessed later...
char * machineName = "dummy_name";
vrpn_Synchronized_Connection *connection = new vrpn_Synchronized_Connection();
nmm_SimulatedMicroscope EroderConnection(machineName, connection,xResolution,yResolution);
int counter = 0;
timeval oldtime, currenttime;
bool first = true;
double Sim_x_ratio;
double Sim_y_ratio;
bool undone = true;
double x_to_y = 1.0;
int TriangleCounter = 0;
bool once_thru = false;
FILE *file;
bool grouping;
int* group;
bool first_in_group = true;
int bcounter = 0;
char simulator_file[100];
bool new_microscope_data = false;
bool new_tip_data = false;
bool new_erosion = false;
bool safe_to_erode = false;
bool sim_window_change = false;

int main(int argc, char *argv[])
{
	nmm_SimulatedMicroscope* test = &EroderConnection;
	if ( (file = fopen("triangles.txt", "w")) == NULL) {
		fprintf(stderr,"Could not open triangles.txt for write\n");
		return -1;
	}
	
    //opengl stuff
    adjustOrthoProjectionParams();
    
    VolumeFilename = NULL;
    bool unitsGiven = false;
	Sim_x_ratio = 1.0;
	Sim_y_ratio = 1.0;

    // Parse the command line
    for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-connection")) {
            connection_to_nano = true;
            char * newName = "Eroder";//do not change this name without changing check for name
									  //in nmm_SimulatedMicroscope_Remote and nmm_SimulatedMicroscope
									  //files--certain functionalities are only triggered by a server
									  //(device) name of "Eroder"
            EroderConnection.change_machineName(newName);
        }//need to take care of -connection argument first so that ratios are set before
		 //any objects are drawn
        else if (!strcmp(argv[i], "-units")) {
            if(++i >= argc) { Usage(argv[0]); }
            strcpy(units,argv[i]);
			if(strcmp(units,"nm") != 0){
				cout << endl << "Make sure that the units you have input are 'nm'" << endl
					 << "if you want to send data to nano and display along with another file." << endl;
			}
            unitsGiven = true;
        } 
        else if (!strcmp(argv[i], "-tip_radius")) {
            if (++i > argc) { Usage(argv[0]); }
            TipSize = atof(argv[i]);
            sp.set_r(TipSize);
            ics.set_r(TipSize);//let the user specify the tip radius desired
        }
		else if (!strcmp(argv[i], "-simulator_data")) {
            if (++i > argc) { Usage(argv[0]); }
            strcpy(simulator_file,argv[i]);
			sim_window_change = load_simulator_data(simulator_file);//true if window size needs to be changed
			if(sim_window_change)	cout << "window" << endl;
			new_microscope_data = true;
        }
		else if (!strcmp(argv[i], "-unca_nano")) {
            bool breakOut = false;
            
            if (++i < argc){
            //determine if there are any arguments specific to the -unca_nano argument
            //by checking if next argument is another command line argument first--if not
            //it has to be a parameter to -unca_nano
                if (!strcmp(argv[i], "-units")) breakOut = true;
                else if (!strcmp(argv[i], "-type")) breakOut = true;
                else if (!strcmp(argv[i], "-tip_radius")) breakOut = true;
				else if (!strcmp(argv[i], "-connection")) breakOut = true;
				else if (!strcmp(argv[i], "-simulator_data")) breakOut = true;

                if(breakOut == false){
                //if more arguments, but the next argument is none of the "standard"
                //arguments, it has to be a parameter for -unca_nano

                    if ((i + 3) >= argc) { Usage(argv[0]); }//make sure there are 4 of them
                    
                    unca_minx = atof(argv[i]);
                    unca_miny = atof(argv[++i]);
                    unca_maxx = atof(argv[++i]);
                    unca_maxy = atof(argv[++i]);
                }
                else{//breakOut == true
                    unca_minx = 0;
                    unca_miny = 0;
                    unca_maxx = 1000;
                    unca_maxy = 1000;
                }
            }
        }
        
    }

    if (!unitsGiven){
        strcpy(units, "nm");
        cout << "Your units are the default, nanometers." << endl
             << "If you want to use different units, enter them now: ";
		cin >> units;

    }

  glutInit(&argc, argv);

  glutInitDisplayMode(dblBuf);
  // glutInitDisplayMode(singleBuf);


  /* The view on Main window is a view of XY plane from a pt on the +ve 
   * Z-axis. +ve X axis is towards right while +ve Y is to upwards
   */
  
  // MAIN WINDOW
  glutInitWindowSize( (int)windowWidth, (int)windowHeight );

  glutInitWindowPosition( 25, 0 );
  mainWindowID = glutCreateWindow( "Eroder - Top View" );
  adjustOrthoProjectionToWindow();

#if DISP_LIST
    make_sphere();
    make_cylinder();
    make_cone_sphere(*(tip.icsTip));
    make_uncert_sphere();
    make_uncert_cylinder();
    make_uncert_cone_sphere(*(tip.icsTip));
#endif

  // pass pointers to callback routines for main window
  glutDisplayFunc(displayFuncMain);
  glutIdleFunc(idleFuncDummy );
  glutReshapeFunc(reshapeWindow);
  glutKeyboardFunc(commonKeyboardFunc);
  glutMouseFunc(mouseFuncMain);
  glutMotionFunc( mouseMotionFuncMain );


  /* The view on Another View window is a front view from a point on the
   * -Y axis
   */

#if 1
  
  //another view window
  glutInitWindowSize( (int)windowWidth, (int)windowHeight );

  glutInitWindowPosition( 650, 0 );
  viewWindowID = glutCreateWindow( "Eroder - Front View" );
  adjustOrthoProjectionToViewWindow();
  glutMouseFunc(mouseFuncMain);
  glutMotionFunc( mouseMotionFuncMain );
  
#if DISP_LIST
    make_sphere();
    make_cylinder();
    make_cone_sphere(*(tip.icsTip));
    make_uncert_sphere();
    make_uncert_cylinder();
    make_uncert_cone_sphere(*(tip.icsTip));
#endif

  // pass pointers to callback routines for the other view window
  glutDisplayFunc(displayFuncView);
  glutIdleFunc(idleFuncDummy );
  // glutReshapeFunc(reshapeWindow);
  glutKeyboardFunc(commonKeyboardFunc);
#endif


  //depth window
  glutInitWindowSize( (int)DEPTHSIZE, (int)DEPTHSIZE );

  glutInitWindowPosition( 25, 400 );
  depthWindowID = glutCreateWindow( "Depth window" );

#if DISP_LIST
    make_sphere();
    make_cylinder();
    make_cone_sphere(*(tip.icsTip));
    make_uncert_sphere();
    make_uncert_cylinder();
    make_uncert_cone_sphere(*(tip.icsTip));
#endif

  glutDisplayFunc( displayFuncDepth );
  glutIdleFunc(idleFuncDummy );
  glutReshapeFunc( reshapeWindowFuncDummy );
  glutKeyboardFunc(commonKeyboardFunc);
  adjustOrthoProjectionToWindow();

  GLenum e = glGetError();
  if (e != GL_NO_ERROR) {
    printf("Found error %d\n",e);
  }

  // app's main loop, from which callbacks to above routines occur
  glutMainLoop();

  if (file) {fclose(file); }
  return 0; /* ANSI C requires main to return int. */
}


void write_to_unca(char *filename) {
    // Everything here is in Angstroms. Unca takes care of this.
    Unca u;
    if(unca_minx != -1 || unca_maxx != -1){
        u.set(DEPTHSIZE, DEPTHSIZE, unca_minx, unca_maxx, unca_miny, unca_maxy, 
            -Far, -Near, (double *)zHeight);
    }
    else{
        u.set(DEPTHSIZE, DEPTHSIZE, orthoFrustumLeftEdge,
            (orthoFrustumLeftEdge + orthoFrustumWidth),orthoFrustumBottomEdge,
            (orthoFrustumBottomEdge + orthoFrustumHeight), -Far, -Near, 
            (double *)zHeight);
    }
    u.writeUnca(filename);
}

void write_to_uncertw(char *filename) {
  // Everything here is in Angstroms. Unca takes care of this.
  Uncertw u = Uncertw(DEPTHSIZE, DEPTHSIZE, orthoFrustumLeftEdge,
(orthoFrustumLeftEdge + orthoFrustumWidth),orthoFrustumBottomEdge,
(orthoFrustumBottomEdge + orthoFrustumHeight), 0, 1., colorBuffer);
  u.writeUncertw(filename);
}


/***********************************************************************/
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
// as required by "doErosion". 
void displayFuncDepth( void ) {
  if(first){
    gettimeofday(&oldtime,NULL);
	first = false;
  }

  if(connection_to_nano)	EroderConnection.d_connection->mainloop();//call to receive first message
  if(EroderConnection.scanRcv){//fill in the data array used as base data for the erosion, set flag for erosion
	  if(safe_to_erode){//fill if have the correct size windows		  		  
			new_microscope_data = true;
			//cout << "microscope data received" << endl;
	  }
	  EroderConnection.scanRcv = false;//will be set to true again if new scan message received
  }

  if (!stopAFM) {
    glutSetWindow( depthWindowID );
    //in Z-buffer of Depth Window using graphics hardware.
    //HeightData is a double array 
    int length = 0;
    //will be filled in by doErosion
    //length is the the rowlength in the height array 
	
	if((new_microscope_data || new_tip_data) && safe_to_erode){//only erode and send back to nano if new microscope data
		//if(new_microscope_data)	cout << "new microscope data" << endl;
		//cout << "safe to erode = " << (int)safe_to_erode;
		
		if(connection_to_nano){//fill array, erode, and send
			gettimeofday(&currenttime,NULL);//current time
			if( fillArray(&EroderConnection) && 
				( (EroderConnection.last_filled_y == yResolution - 1) || 
				((vrpn_TimevalDiff(currenttime,EroderConnection.lastfilltime)).tv_sec >= 10) ) ){
			//true if array filled properly, and all rows filled in (ANDREA:fix second condition for partial fills later)
				HeightData = doErosion(length,EroderConnection.get_zrange(),&EroderConnection,
					EroderConnection.Sim_to_World_x);				
			}

			gettimeofday(&currenttime,NULL);//current time
			int x = 3;
			if((vrpn_TimevalDiff(currenttime,oldtime)).tv_sec >= x)
			{//send no greater than every x sec.
				EroderConnection.encode_and_sendData(HeightData,length);
				//cout << "sending erosion" << endl;
				gettimeofday(&oldtime,NULL);//give oldtime a new value to reflect that data 
											//just sent
			}
		}
		else{//just erode
			//cout << "erosion here" << endl;
			HeightData = doErosion(length,300,&EroderConnection,1.0);
		}
		new_microscope_data = false;
		new_tip_data = false;		//we have taken care of the new microscope/tip data so set both to false
		new_erosion = true;
		if(connection_to_nano)	EroderConnection.d_connection->mainloop();
		//call mainloop to be able to receive next message
	}
	else{
		if(connection_to_nano){
			EroderConnection.d_connection->mainloop();//so connection can be set up

			gettimeofday(&currenttime,NULL);//current time
			int x = 1;
			if((vrpn_TimevalDiff(currenttime,oldtime)).tv_sec >= x)
			{//send no greater than every x sec.
				EroderConnection.encode_and_sendData(HeightData,length);
				//cout << "sending erosion" << endl;
				gettimeofday(&oldtime,NULL);//give oldtime a new value to reflect that data 
											//just sent
			}
		}
	}
    
	
    // end of display frame, so flip buffers
    glutSwapBuffers();
  }
}

// This idle function marks both windows for redisplay, which will cause
// their display callbacks to be invoked.
void commonIdleFunc( void ) {
#define PERIOD 30
	EroderConnection.d_connection->mainloop();

	x_to_y = 1.0;
	int oldWidth = windowWidth;
	int oldHeight = windowHeight;

	if(EroderConnection.grid_size_rcv){
		sp.set_r(sp.r* EroderConnection.Sim_to_World_x);
        ics.set_r(ics.r* EroderConnection.Sim_to_World_x);
		//change tip radius to match scan resolution from nano
		
		windowWidth = EroderConnection.get_xsize();
		windowHeight = EroderConnection.get_ysize();

		xResolution = windowWidth;
		yResolution = windowHeight;

		undone = true;
		safe_to_erode = false;
	}
	if(sim_window_change){
		cout << "changing window" << endl;
		windowWidth = xResolution;
		windowHeight = yResolution;
		undone = true;
		safe_to_erode = false;
		sim_window_change = false;
	}
	double WidthRatio = windowWidth/oldWidth;
	double HeightRatio = windowHeight/oldHeight;
  
	if (undone){
		glutSetWindow(mainWindowID);
		glutReshapeWindow((int)(windowWidth), (int)(windowHeight));
		glutSetWindow( viewWindowID );
		glutReshapeWindow((int)(windowWidth), (int)(windowHeight));
		glutSetWindow( depthWindowID );
		glutReshapeWindow((int)(windowWidth), (int)(windowHeight));

		glutSetWindow( mainWindowID ); glutPostRedisplay();
		glutSetWindow( viewWindowID ); glutPostRedisplay();
		glutSetWindow( depthWindowID ); glutPostRedisplay();

		undone = false;
		if(EroderConnection.grid_size_rcv){
			EroderConnection.grid_size_rcv = false;//so only process this once
		}
		//cout << "setting safe_to_erode to true" << endl;
		safe_to_erode = true;//now that windows have been resized it is safe to erode

	}

	if(new_erosion){
		glutSetWindow( mainWindowID ); glutPostRedisplay();
		glutSetWindow( viewWindowID ); glutPostRedisplay();
		glutSetWindow( depthWindowID ); glutPostRedisplay();
		new_erosion = false;
	}
}

void idleFuncDummy( void ) {commonIdleFunc();}
void reshapeWindowFuncDummy( int newWindowWidth, int newWindowHeight ) 
{}

// callback routine: called when window is resized by user
void reshapeWindow( int newWindowWidth, int newWindowHeight ) {
  windowWidth = newWindowWidth;
  windowHeight = newWindowHeight;

  // viewport covers whole window
  glViewport( 0, 0, (int)windowWidth, (int)windowHeight );

  // make graphics projection match window dimensions
  adjustOrthoProjectionToWindow();
}


void adjustOrthoProjectionParams( void ) {
  // set nominal size of window before taking aspect ratio into account
  // double orthoFrustumLeftEdgeNominal = orthoFrustumCenterX - orthoFrustumWidthNominal/2.;
  double orthoFrustumBottomEdgeNominal = orthoFrustumCenterY - orthoFrustumHeightNominal/2.;

  // calculate aspect ratio of current window
  double aspectRatio = windowWidth / windowHeight;

  // set vertical extent of window to nominal area of world being viewed.
  orthoFrustumHeight = orthoFrustumHeightNominal;
  orthoFrustumBottomEdge = orthoFrustumBottomEdgeNominal;

  // view horizontal extent of world proportional to window width
  orthoFrustumWidth = orthoFrustumWidthNominal * aspectRatio;
  orthoFrustumLeftEdge = orthoFrustumCenterX - orthoFrustumWidth / 2.;
}


// adjust the ortho projection to match window aspect ratio and keep circles round.
void adjustOrthoProjectionToWindow( void ) {
  double orthoFrustumNearEdge = Near;
  /* All far pts get mapped to Far. Allow round off of 1 */
  double orthoFrustumFarEdge = Far;

  // set projection matrix to orthoscopic projection matching current window
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho( orthoFrustumLeftEdge, orthoFrustumLeftEdge + orthoFrustumWidth,
        orthoFrustumBottomEdge, orthoFrustumBottomEdge + orthoFrustumHeight, orthoFrustumNearEdge, 
		orthoFrustumFarEdge );
}

// adjust the ortho projection to match window aspect ratio and keep circles round.
void adjustOrthoProjectionToViewWindow( void ) {

#if 0
  double orthoFrustumNearEdge = -Far;
  double orthoFrustumFarEdge = -Near;
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glOrtho( orthoFrustumLeftEdge, orthoFrustumLeftEdge + orthoFrustumWidth,
        orthoFrustumNearEdge, orthoFrustumFarEdge, 
        orthoFrustumBottomEdge, orthoFrustumBottomEdge + orthoFrustumHeight);
#else
  double orthoFrustumNearEdge = -Far;
  double orthoFrustumFarEdge = -Near;
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glOrtho( orthoFrustumLeftEdge, orthoFrustumLeftEdge + orthoFrustumWidth,
        orthoFrustumNearEdge, orthoFrustumFarEdge, -100,100);
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
  
}

#if DISP_LIST
// remake disp lists
void remake_sphere() {
  glutSetWindow( mainWindowID ); make_sphere();
    make_uncert_sphere(); 
  glutSetWindow( viewWindowID ); make_sphere();
    make_uncert_sphere(); 
  glutSetWindow( depthWindowID ); make_sphere();
    make_uncert_sphere(); 
}

void remake_cylinder() {
  glutSetWindow( mainWindowID ); make_cylinder(); 
make_uncert_cylinder();
  glutSetWindow( viewWindowID ); make_cylinder(); 
make_uncert_cylinder();
  glutSetWindow( depthWindowID ); make_cylinder(); 
make_uncert_cylinder(); 
}

void remake_cone_sphere(InvConeSphereTip ics) {
  glutSetWindow( mainWindowID ); make_cone_sphere(ics); 
make_uncert_cone_sphere(ics); 
  glutSetWindow( viewWindowID ); make_cone_sphere(ics); 
make_uncert_cone_sphere(ics); 
  glutSetWindow( depthWindowID ); make_cone_sphere(ics); 
make_uncert_cone_sphere(ics); 
}
#endif

// Keyboard callback for main window.
void commonKeyboardFunc(unsigned char key, int x, int y) {
    ofstream fout2;
    fout2.open("protein_vols_backwards.txt", fstream::out | fstream::app);
    double volume;
	int counter = 0;
	int i = 0;

    switch (key) {
    case 'M' : // toggle shading model
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
   case 'f' ://find volume
	  double avgHeight, maxHeight,area;
      volume = find_volume(avgHeight,maxHeight,area);
      if(VolumeFilename != NULL){
        fout2 << VolumeFilename << ": ";
      }
      fout2 << endl << endl << "\nVol: " << volume <<"\nAvg Height: " 
		    << avgHeight << "\nMax Height: " << maxHeight << "\nArea: " << area << flush;
      cout << "Volume of all objects on plane is " << volume << " " 
            << units << "^3.\n\n" << flush;
      fout2.close();
      break;
    case 'r' :
      tip.inc_r();
	  new_tip_data = true;
#if DISP_LIST
      remake_cone_sphere(*(tip.icsTip));
#endif
      break;
    case 'R' :
      tip.dec_r();
	  new_tip_data = true;
#if DISP_LIST
      remake_cone_sphere(*(tip.icsTip));
#endif
      break;
    case 'a' ://change angle, slant of the tip
      tip.inc_theta();
	  new_tip_data = true;
#if DISP_LIST
      remake_cone_sphere(*(tip.icsTip));
#endif
      break;
    case 'A' ://change angle, slant of the tip
      tip.dec_theta();
	  new_tip_data = true;
#if DISP_LIST
      remake_cone_sphere(*(tip.icsTip));
#endif
      break;
    case '*' ://increase tesselation
      tesselation += 5;
      new_tip_data = true;
#if DISP_LIST
      remake_sphere();
      remake_cylinder();
      remake_cone_sphere(*(tip.icsTip));
      tip.icsTip->set(tip.icsTip->r, tip.icsTip->ch, tip.icsTip->theta, tesselation);
#endif
      printf("Tesselation %d\n", tesselation);
      break;
    case '/' ://decrease tesselation
      if (tesselation > 5) { tesselation -= 5; }
      new_tip_data = true;
#if DISP_LIST
      remake_sphere();
      remake_cylinder();
      remake_cone_sphere(*(tip.icsTip));
      
      tip.icsTip->set(tip.icsTip->r, tip.icsTip->ch, tip.icsTip->theta, tesselation);
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
      stopAFM=1;
      
        if (uncertainty_mode) {// write out uncert.map
            char filename[40];
            if (tip.type == SPHERE_TIP) {
                sprintf(filename,"uncert_sptip_r_%.1lfnm.UNCERTW",tip.spTip->r);
            }
            else {
                sprintf(filename,"uncert_icstip_r_%.1lfnm_ch_%.1lfnm_theta_%.1lfdeg.UNCERTW",tip.icsTip->r,tip.icsTip->ch,
                RAD_TO_DEG*tip.icsTip->theta);
            }
            cout << "Writing to file " << filename << endl;
            write_to_uncertw(filename);
            cout << "Finished writing to file " << filename << endl;
    
        }
        else {// write output to a file.
            char filename[40];
			struct timeval now;
			gettimeofday(&now,NULL);
            if (tip.type == SPHERE_TIP) {
                sprintf(filename,"time%d.%d.UNCA",(int)now.tv_sec,(int)now.tv_usec);
            }
            else {
                sprintf(filename,"time%d.%d.UNCA",(int)now.tv_sec,(int)now.tv_usec);
            }
            cout << "Writing to file " << filename << endl;
            write_to_unca(filename);
            cout << "Finished writing to file " << filename << endl;
        
        }

        stopAFM=0;
        break;
    case 'q' :
      exit(0);
      break;
    default :
      
    globalkeyboardFunc(key,x,y);
      
    }
    glutPostRedisplay(); // in case something was changed

}


void mouseFuncMain( int button, int state, int x, int y ) {

}

void mouseMotionFuncMain( int x, int y ) {

}


void Usage(char *progname)
{
  cout << endl
       <<"Running the program" << endl
       <<"-------------------" << endl
       << endl
       <<"There are several command line argument options, but none are mandatory. they are set up" << endl
       <<"so that, as long as you have the correct arguments for each parameter that you want to " << endl
       <<"specify, you can put the parameters in any order. The parameters that can be specified are" << endl
       <<"as follows:" << endl
       <<endl
       <<" 1. units used" << endl
       <<" 2. tip radius" << endl
       <<" 3. type of file to open" << endl
       <<endl
       <<"1. To specify what units you are using, type:" << endl
       <<" -units <units>" << endl
       <<endl
       <<" (remember to include the '-'!)" << endl
       <<" where <units> is a string such as 'nm'." << endl
       <<" Note that if you do not enter anything, the default units of nanometers" << endl
       <<" will be used.\n"<< endl

       <<"2. To specify what tip radius to use, type:" << endl
       <<" -tip_radius <radius>" << endl<< endl

       <<" where <radius> is a floating point value such as 7.5. If you do not specify" << endl
       <<" a tip radius, the default value of 5.0 will be used." << endl<< endl

       <<"3. To specify that the file is to be loaded into nano as a unca file, type:" << endl
       <<" -unca_nano" << endl<< endl

       <<" Optional arguments to follow specify the region, corresponding to the region for the" << endl
       <<" real afm image that the simulated scan is to be matched up against. In order, these are:" << endl
       <<" min x" << endl
       <<" min y" << endl
       <<" max x" << endl
       <<" max y" << endl<< endl

       <<" Example: -unca_nano 0 0 5000 5000" << endl<< endl

       <<"4. To specify a file to be opened, use one of the following formats:" << endl << endl
       <<" -type -p" << endl
       <<" -type -d" << endl
       <<" -type -t" << endl
       <<" -type -s" << endl
       <<" -type -dp" << endl<< endl

       <<" In order, these are for opening a protein file, dna file, triangle file, " << endl
       <<" spheres file, and both a dna and protein file at the same time. Note that you do" << endl
       <<" not have to open a file in order to run the simulation; leaving out a file type" << endl
       <<" specification opens the simulation without any objects. Various keyboard commands" << endl
       <<" can then be used to place objects where desired." << endl<< endl

       <<"Explanation of the '-type' command usage" << endl
       <<"----------------------------------------" << endl<< endl

       <<"1. To simply run the simulator, type:" << endl<< endl

       <<" ./sim if you are on an sgi" << endl
       <<" or"<<endl
       <<" ./3d_afm/Debug/3d_afm.exe if you are working in cygwin, or have the " << endl
       <<" application on your pc" << endl<< endl

       <<" Note that all future references to the application will use './sim' to lessen" << endl
       <<" confusion, but in all cases, either of the two options listed could be used." << endl<< endl

       <<"2. To get the AFM of a protein" << endl
       <<" ./sim -type -p <protein filename> <ratio>" << endl
       <<" <ratio> is a number = (Unit assumed in the file)/(1 nm) e.g 0.1 for Angstrom\n" << endl
       <<" Example : ./sim -type -p lac.data 1" << endl<< endl

       <<"Read pdb/README on how to generate a '.data' file from a PDB file." << endl
       <<"--See Protein file format below" << endl<< endl

       <<"3. To get AFM of a triangular model run" << endl
       <<" ./sim -type -t <filename> <scale>" << endl
       <<" <scale> scales the values in the given input file" << endl
       <<" Example : ./sim -type -t teddy.obj 10 " << endl<< endl

       <<"4. To run DNA simulator" << endl
       <<" ./sim -type -d <dna-filename>" << endl
       <<" Example : ./sim -type -d dna.dat" << endl<< endl

       <<"--See DNA file format below" << endl<< endl

       <<"5. To run DNA simulator and protein file" << endl
       <<" ./sim -type -dp <protein-filename> <ratio> <dna-filename>" << endl
       <<" Example : ./sim -type -dp lac-smaller.dat 1 dna2.dat " << endl<< endl

       <<"--Note that in order to run the dna simulator and the protein at the same time," << endl 
       <<"the data for the protein cannot be too large. For example, the size of the file " << endl
       <<"lac.data is too large to refresh all the data in a reasonable amount of time, but" << endl
       <<"the file lac-smaller, which has been pared down, is okay. Look at these files to " << endl
       <<"gauge an appropriate size when preparing your data file."<< endl<< endl

       <<"--Note also that the protein as a whole will also be centered in the simulation" << endl<< endl

       <<"6. To get the AFM of spheres where the position you specify is absolute" << endl
       <<" ./sim -type -s <sphere-filename> <ratio>" << endl
       <<" Example : ./sim -type -s testfile.dat 1" << endl<< endl

       <<"--See Sphere file format below" << endl<< endl<< endl


       <<"Again, for any of the above examples, tip radius and/or units can also be specified," << endl
       <<"and the order in which the three parameters (the above two and file type) are specified" << endl
       <<"is arbitrary. So, the last example, ./sim -type -s testfile.dat 1, could also be any " << endl
       <<"of the following, if we also wanted to specify tip radius and units:" << endl<< endl

       <<" ./sim -units nm -tip_radius 10.0 -type -s testfile.dat 1" << endl
       <<" ./sim -units nm -type -s testfile.dat 1 -tip_radius 10.0" << endl
       <<" ./sim -tip_radius 10.0 -units nm -type -s testfile.dat 1" << endl
       <<" ./sim -tip_radius 10.0 -type -s testfile.dat 1 -units nm" << endl
       <<" ./sim -type -s testfile.dat 1 -units nm -tip_radius 10.0" << endl
       <<" ./sim -type -s testfile.dat 1 -tip_radius 10.0 -units nm" << endl<< endl<< endl


       <<"Format for the protein file" << endl
       <<"---------------------------" << endl<< endl

       <<"x_position y_position z_position" << endl
       <<"repeat for all spheres" << endl<< endl

       <<"Example" << endl<< endl

       <<"34.0 34.0 0.0" << endl
       <<"36.0 36.0 5.0" << endl<< endl

       <<"--The above file would create a protein consisting of 2 spheres with the " << endl
       <<"default radius." << endl<< endl<< endl


       <<"Format for the sphere file" << endl
       <<"--------------------------" << endl
       <<"x_position y_position z_position radius" << endl
       <<"repeat for all spheres" << endl<< endl

       <<"Example" << endl<< endl

       <<"34.0 34.0 0.0 2.0" << endl
       <<"67.0 74.0 5.0 5.0" << endl<< endl

       <<"--The above file would create two spheres." << endl<< endl


       <<"Format for the dna file" << endl
       <<"-----------------------" << endl<< endl

       <<" Number of segments in the DNA" << endl
       <<" Length of DNA" << endl
       <<" Position of site 1" << endl
       <<" Position of site 2" << endl
       <<" Tangent at site 1" << endl
       <<" Tangent at site 2" << endl<< endl

       <<"Example (see file dna.dat)" << endl<< endl

       <<" 30" << endl
       <<" 160" << endl
       <<" 20 10 10" << endl
       <<" 110 40 50" << endl
       <<" -100 100 0" << endl
       <<" -50 -100 0" << endl<< endl<< endl<< endl<< endl



       <<"Information" << endl
       <<"-----------" << endl<< endl

       <<". This code uses display lists. They are turned on by default. To turn them off, change " << endl
       <<"the #define DISP_LIST in defns.h and recompile." << endl<< endl

       <<". For objects totally composed of spheres, use the type SPHERE instead of NTUBE. This " << endl
       <<"enable a number of optimizations. However you cannot change the length of an object " << endl
       <<"declared as a SPHERE." << endl<< endl


       <<"Key and Mouse bindings" << endl
       <<"----------------------" << endl<< endl

       <<". Left mouse click on an object selects it. The object's color turns red." << endl
       <<". Left mouse when clicked and moved while the key remains pressed moves the object along " << endl
       <<"XY plane." << endl
       <<". Right mouse click works on triangles. It is used to select a side of a triangle." << endl<< endl
	   <<"The following keys produce global changes (not object specific)" << endl
	   <<"--------------------------------------------------------------" << endl<< endl

<<". 'q' - exit" << endl
<<". 'o' - draw objects. By default, objects are not drawn, only the AFM is." << endl
<<". 'w' - write the AFM to a file." << endl
<<" In uncertainty mode, it writes out the uncertainty map." << endl
<<". 'n' - add a new nano tube" << endl
<<". 's' - add a sphere, you will get a prompt to enter a radius, which can be any" << endl
<<" reasonably-sized floating point number" << endl
<<". 'm' - add new protein to display" << endl
<<". 'M' - toggle shading mode" << endl
<<". 'f' - find the volume of all objects drawn, the volume will be printed to the" << endl
<<" screen and will also be appended to the file sphere_output.txt" 
<< endl
<<". 't' - add a new triangle" << endl
<<". 'p' - toggle the tip model, the default is the inverted ConeSphere tip model" << endl
<<". 'r', 'R' - change the tip radius" << endl
<<". 'a', 'A' - change the tip angle in case of inv ConeSphere tip." << endl
<<". 'i' - toggles between 'no scan' to 'semi solid scan' to 'solid scan'." << endl
<<". 'u' - to toggle to uncertainty mode" << endl
<<endl
<<"The following perform object specific actions" << endl
<<"---------------------------------------------" << endl
<<endl
<<". DELETE - deleted that object" << endl
<<". 'd', 'D' - change diam" << endl
<<". 'l', 'L' - change length" << endl
<<". '+', '-' - change z" << endl
<<". 'x', 'X' - rotate about X" << endl
<<". 'y', 'Y' - rotate about X" << endl
<<". 'z', 'Z' - rotate about X" << endl
<<endl
<<"The following will have effect only on nano tubes" << endl
<<"-------------------------------------------------" << endl
<<endl
<<". 'e', 'E' - change roll" << endl
<<". 'f', 'F' - change yaw" << endl
<<". 'h', 'H' - change pitch" << endl
<<endl
<<endl
<<"Important Note :" << endl
<<"----------------" << endl
<<endl
<<"We had said earlier, right click selects one side of a triangle. But since a side of a " << endl
<<"triangle is modelled as a nanotube, you can perform all the nanotube operations on that " << endl
       <<"side." << endl;

    exit(-1);
}


