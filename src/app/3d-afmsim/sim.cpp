
//test for cvs updates

/* Gokul Varadhan
 * varadhan@cs.unc.edu
 * May 2001
 *
 * 3D AFM simulator.
 */
/*$Id$*/
#include <stdlib.h> //stdlib.h vs cstdlib
#include <stdio.h> //stdio.h vs cstdio
//#include <unistd.h>
//#include <sys/types.h>
//#include <sys/time.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <math.h> //math.h vs cmath
#include <GL/glut_UNC.h>
//#include <string.h>
#include "Vec3d.h"
#include "3Dobject.h"
#include "ConeSphere.h"
#include "Tips.h"
#include <string.h>
#include "defns.h"
#include "Unca.h"
#include "Uncertw.h"
#include "input.h"
#include "scan.h"
#include "draw.h"
#include "lightcol.h"
#include "uncert.h"

#include "sim.h"
#include <vrpn_Connection.h>
#include <nmm_SimulatedMicroscope.h>
#include <Tcl_Linkvar.h>
#include <tcl.h>
#include <tk.h>
#include <Tcl_Interpreter.h>
#include "../nano/tcl_tk.h"
#if !defined(_WIN32) || defined(__CYGWIN__)
#include <sys/time.h>
#else
#include <vrpn_Shared.h> // get timeval some other way
#endif


GLuint list_sphere;
GLuint list_cylinder;

static int dblBuf = GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH;
Ntube nullntube;

OB *ob[MAXOBS];
int numObs;
OB **group_of_obs[MAXGROUPS];
int numGroups = 0;
int number_in_group[MAXGROUPS];
int NANOTUBE_GROUP;

int selectedOb = NULLOB;
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
double windowWidth = 600.;
double windowHeight = 600.;
double orthoFrustumCenterX = DEPTHSIZE/2; // area of XY plane always visible for all window aspect ratios
double orthoFrustumCenterY = DEPTHSIZE/2;
double orthoFrustumWidthNominal = DEPTHSIZE;//***
double orthoFrustumHeightNominal = DEPTHSIZE;//***

// actual bounds of current ortho view frustum matching window aspect ratio
double orthoFrustumLeftEdge;
double orthoFrustumBottomEdge;
double orthoFrustumWidth;
double orthoFrustumHeight;

// mouse and cursor
int xMouseInWindow; // mouse position in world coords
int yMouseInWindow; // mouse position in world coords
Vec3d vMouseWorld; // mouse position in world coords (actually a 2D vector in XY plane)
Vec3d vGrabOffset; // offset from cursor position to grabbed object (in world coords actually a 2D vector in XY plane)

int stopAFM=0;
int done_drawing_objects=0;
double thetax=0.,thetay=0.;
int tesselation = 30;

char units[100];
char* VolumeFilename;
float TipSize;

static void handle_cname_change(const char *, void *);
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
void calcMouseWorldLoc( int xMouse, int yMouse, int xy_or_xz);
void grabNearestOb(int xy_or_xz);
int findNearestObToMouse(int xy_or_xz);
void findNearestTriangleSideToMouse( void );
void select_triangle_side();
void Usage(char *progname);
void glutInitializeStuff();

//has to be put out here so variables that can be accessed later...
char * machineName = "dummy_name";
vrpn_Synchronized_Connection *connection = new vrpn_Synchronized_Connection();
nmm_SimulatedMicroscope SimMicroscopeServer(machineName, connection,scanResolution);
int counter = 0;
timeval oldtime, currenttime;
bool first = true;
double Sim_x_ratio;
double Sim_y_ratio;
bool undone = true;
double x_to_y = 1.0;
bool once_thru = false;
FILE *file;
float current_scale = 1;
bool grouping;
int* group;
bool first_in_group = true;
int bcounter = 0;
int oldShift = 0;
double water = 0.0;
char* proteinName;
bool save_for_eroder = false;
float current_trans_x = 0;
float current_trans_y = 0;
float current_trans_z = 0;
float current_rot_x = 0;
float current_rot_y = 0;
float current_rot_z = 0;

Tclvar_string cname("tclname","");

int main(int argc, char *argv[])
{
	nmm_SimulatedMicroscope *test = &SimMicroscopeServer;
	for(int j = 0;j < MAXGROUPS;j++){
		number_in_group[j] = 0;
	}
	if ( (file = fopen("triangles.txt", "w")) == NULL) {
		fprintf(stderr,"Could not open triangles.txt for write\n");
		return -1;
	}
	cname = "";
    cname.addCallback(handle_cname_change, NULL);

    //opengl stuff
    adjustOrthoProjectionParams();
    bool radius = true; //for protein/spheres files--false means use default
                        //true means the file contains radii
    bool centering = true;
    
    VolumeFilename = NULL;
    bool unitsGiven = false;
	Sim_x_ratio = 1.0;
	Sim_y_ratio = 1.0;

    // Parse the command line
    for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-connection")) {
            connection_to_nano = true;
            char * newName = "AFMSimulator"/*argv[i]*/;
            SimMicroscopeServer.change_machineName(newName);
			//Sim_x_ratio = SimMicroscopeServer.return_Simpixels_to_realworld_ratio_x();
			//Sim_y_ratio = SimMicroscopeServer.return_Simpixels_to_realworld_ratio_y();
        }//need to take care of -connection argument first so that ratios are set before
		 //any objects are drawn
        else if (!strcmp(argv[i], "-units")) {
            if(++i >= argc) { Usage(argv[0]); }
            strcpy(units,argv[i]);
            cout << endl << "Make sure that the units you have input are 'nm'" << endl
                 << "if you want to send data to nano and display along with another file." << endl;
            unitsGiven = true;
        }
        else if (!strcmp(argv[i], "-type")) {
            if(++i > argc) { Usage(argv[0]); }
            if(!strcmp(argv[i], "-p")){
                if ((++i + 1) >= argc) { Usage(argv[0]); } 
                VolumeFilename = argv[i];
                addSpheresFromFile(VolumeFilename,atof(argv[++i]),!radius, 
                           centering);
            }
            else if(!strcmp(argv[i], "-d")){
                if (++i >= argc) { Usage(argv[0]); } 
                VolumeFilename = argv[i];
                init_dna(VolumeFilename);
            }
            else if(!strcmp(argv[i], "-dp")){//protein filename first then dna filename
                if ((++i + 2) >= argc) { Usage(argv[0]); }
                char * proteinFile = argv[i];
                float ratio = atof(argv[++i]);
                char * dnaFile = argv[++i];
                
                addSpheresFromFile(proteinFile, ratio, !radius, centering);
                char *proteinPortion = new char[strlen(proteinFile) + 3 + 1];
                strcpy(proteinPortion, proteinFile);
                strcat(proteinPortion, " & ");
                VolumeFilename = new char[strlen(proteinPortion) + strlen(dnaFile) +1];
                strcpy(VolumeFilename, proteinPortion);
                strcat(VolumeFilename, dnaFile);// XXX LEAKING MEMORY
                init_dna(dnaFile);

                delete proteinPortion;
            }
            else if(!strcmp(argv[i], "-t")){//triangles file
                if ((++i + 1) >= argc) { Usage(argv[0]); }
                VolumeFilename = argv[i];
                addTrianglesFromFile(VolumeFilename,atof(argv[++i]));
            }
            else if(!strcmp(argv[i], "-s")){//spheres
                if ((++i + 1) >= argc) { Usage(argv[0]); }
                VolumeFilename = argv[i];
                fout.open("sphere_output.txt", fstream::out | fstream::app);
                //append to end each time
                addSpheresFromFile(VolumeFilename,atof(argv[++i]),radius, 
                           !centering);
            }
            else{//inappropriate file type given 
                Usage(argv[0]); 
            }
        } 
        else if (!strcmp(argv[i], "-tip_radius")) {
            if (++i > argc) { Usage(argv[0]); }
            TipSize = atof(argv[i]);
            sp.set_r(TipSize);
            ics.set_r(TipSize);//let the user specify the tip radius desired
        }
		else if (!strcmp(argv[i], "-water")) {
            if (++i > argc) { Usage(argv[0]); }
            water = atof(argv[i]);
        }
		else if (!strcmp(argv[i], "-protein")) {
            if (++i > argc) { Usage(argv[0]); }
            proteinName = argv[i];
		}
        else if (!strcmp(argv[i], "-connection")) {
            connection_to_nano = true;
            char * newName = "AFMSimulator"/*argv[i]*/;
            SimMicroscopeServer.change_machineName(newName);
			Sim_x_ratio = SimMicroscopeServer.return_Simpixels_to_realworld_ratio_x();
			Sim_y_ratio = SimMicroscopeServer.return_Simpixels_to_realworld_ratio_y();
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
             << "Press 'q' if this is not what you intended." << endl;
    }

	/*if(connection_to_nano){
		while(!SimMicroscopeServer.grid_size_rcv){
			//try again
		}
		//once received
		glutInit(&argc, argv);
		glutInitializeStuff();
	}
	else{
		glutInit(&argc, argv);
		glutInitializeStuff();
	}*/
  glutInit(&argc, argv);

  glutInitDisplayMode(dblBuf);
  // glutInitDisplayMode(singleBuf);


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


  glutInitWindowPosition( 800, 0 );
  viewWindowID = glutCreateWindow( "Front View" );
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

  glutInitWindowPosition( 50, 750 );
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

static void handle_cname_change(const char *, void *)
{
    if( strlen(cname.string() ) <= 0 )
        return;
  
    cout << "tcl works" << endl;

    cname = (const char *) "";
} 

void write_to_unca(char *filename) {
    // Everything here is in Angstroms. Unca takes care of this.
    Unca u;
    if(unca_minx != -1 || unca_maxx != -1){
        u.set(DEPTHSIZE, DEPTHSIZE, unca_minx, unca_maxx, unca_miny, unca_maxy, 
            -scanFar, -scanNear, (double *)zHeight);
    }
    else{
        u.set(DEPTHSIZE, DEPTHSIZE, orthoFrustumLeftEdge,
            (orthoFrustumLeftEdge + orthoFrustumWidth),orthoFrustumBottomEdge,
            (orthoFrustumBottomEdge + orthoFrustumHeight), -scanFar, -scanNear, 
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
// as required by "doImageScanApprox". 
void displayFuncDepth( void ) {
    
  if(first){
	  gettimeofday(&oldtime,NULL);	
	  first = false;
  }

  if (!stopAFM) {
    glutSetWindow( depthWindowID );
    //in Z-buffer of Depth Window using graphics hardware.
    //HeightData is a double array 
    int length = 0;
    //will be filled in by doImageScanApprox
    //length is the size of both rows and columns in the height array 
    //(double array)
	
	if(SimMicroscopeServer.grid_size_rcv){
		HeightData = doImageScanApprox(length,SimMicroscopeServer.Sim_to_World_x);
	}
	else{
		HeightData = doImageScanApprox(length,1.0);
	}
	if(save_for_eroder){
		save_to_eroder();
		save_for_eroder = false;
	}
	
	
    if(connection_to_nano){
        gettimeofday(&currenttime,NULL);//current time
        int x = 1;
        if((vrpn_TimevalDiff(currenttime,oldtime)).tv_sec >= x)
		{//send every x sec. for now
            SimMicroscopeServer.encode_and_sendData(HeightData,length);
            gettimeofday(&oldtime,NULL);//give oldtime a new value to reflect that data 
            //just sent
        }
    }
	
/*	if(SimMicroscopeServer.triangleRcv){
		float tri_scale = 10;//SimMicroscopeServer.tri_scale;
		double ratio = SimMicroscopeServer.Sim_to_World_x;
		double x_offset = SimMicroscopeServer.get_x_offset();
		double y_offset = SimMicroscopeServer.get_y_offset();
		float total_scaling = ratio*tri_scale;

		float v1_1,v1_2,v1_3,v2_1,v2_2,v2_3,v3_1,v3_2,v3_3;
		
		while(SimMicroscopeServer.head != NULL){//add a bunch of triangles	
			v1_1 = SimMicroscopeServer.head->v1_1;
			v1_2 = SimMicroscopeServer.head->v1_2;
			v1_3 = SimMicroscopeServer.head->v1_3;
			v2_1 = SimMicroscopeServer.head->v2_1;
			v2_2 = SimMicroscopeServer.head->v2_2;
			v2_3 = SimMicroscopeServer.head->v2_3;
			v3_1 = SimMicroscopeServer.head->v3_1;
			v3_2 = SimMicroscopeServer.head->v3_2;
			
			v3_3 = SimMicroscopeServer.head->v3_3;
			addTriangle(//draw triangle with vertices v1_*,v2_*, and v3_*, where * is {x,y,z} 
				        //coords of v*			
			Vec3d(v1_1*total_scaling //+ x_offset,
				  v1_2*total_scaling //+ y_offset,
				  v1_3*total_scaling),
			Vec3d(v2_1*total_scaling //+ x_offset,
				  v2_2*total_scaling //+ y_offset,
				  v2_3*total_scaling),
			Vec3d(v3_1*total_scaling //+ x_offset,
				  v3_2*total_scaling //+ y_offset,
				  v3_3*total_scaling));
			fprintf(file,"%g,%g,%g\n%g,%g,%g\n%g,%g,%g\n\n",v1_1,v1_2,v1_3,v2_1,v2_2,v2_3,v3_1,v3_2,v3_3);
			static int count = 0;
			if (++count == 3) {
				fprintf(stderr,"\n");
				count = 0;
			}

			SimMicroscopeServer.holder = SimMicroscopeServer.head;
			SimMicroscopeServer.head = SimMicroscopeServer.head->next;
			delete SimMicroscopeServer.holder;	
			
		}
		SimMicroscopeServer.triangleRcv = false;
		
	}
*/
	if(SimMicroscopeServer.cylRcv){
		double ratio = SimMicroscopeServer.Sim_to_World_x;
		//current_scale = SimMicroscopeServer._scale;
		float scale = current_scale * ratio;

		float x,y,z,altitude,azimuth,length,radius;
		
		while(SimMicroscopeServer.cylHead != NULL){//add a bunch of cylinders	
			x = SimMicroscopeServer.cylHead->x;
			y = SimMicroscopeServer.cylHead->y;
			z = SimMicroscopeServer.cylHead->z;
			altitude = SimMicroscopeServer.cylHead->altitude;//in radians
			azimuth = SimMicroscopeServer.cylHead->azimuth;//in radians
			length = SimMicroscopeServer.cylHead->length*scale;
			radius = SimMicroscopeServer.cylHead->radius*scale;

			Vec3d npos(x*scale,y*scale,z*scale);
			cout << "altitude: " << altitude << " azimuth: " << azimuth << endl
				 << "pos: " << npos << endl;

			NANOTUBE_GROUP = numGroups;

			addNtube(NTUBE, Vec3d(x*scale,y*scale,z*scale), 
				azimuth*RAD_TO_DEG, 0*RAD_TO_DEG, altitude*RAD_TO_DEG, length, radius*2.0,&NANOTUBE_GROUP);	
			//type,pos,yaw,roll,pitch,length,diameter,group

			SimMicroscopeServer.cylHolder = SimMicroscopeServer.cylHead;
			SimMicroscopeServer.cylHead = SimMicroscopeServer.cylHead->next;
			delete SimMicroscopeServer.cylHolder;	


			int obj_group = NANOTUBE_GROUP;

			Vec3d group_center;
			for(int i = 0; i < number_in_group[obj_group];i++){
				OB * this_obj = group_of_obs[obj_group][i];
				group_center += this_obj->pos;
			}
			group_center /= number_in_group[obj_group];
			//cout << "group center: " << group_center << endl;
			
		}
		SimMicroscopeServer.cylRcv = false;
		
	}

	if(SimMicroscopeServer.scaleRcv){
		float _scale = SimMicroscopeServer._scale;
		float new_scale = _scale/current_scale;	//"un-apply" old scaling and apply new one
		current_scale = _scale;					//record what the current scale is so can change 
												//in the same way next time

		if(new_scale != 1){
			int obj_group = NANOTUBE_GROUP;

			if(group_of_obs[obj_group] != NULL){
				/*
				float x, y, z;
				x = group_of_obs[obj_group][0]->pos.x + 
					(group_of_obs[obj_group][number_in_group[obj_group] - 1]->pos.x - group_of_obs[obj_group][0]->pos.x) / 2;
				y = group_of_obs[obj_group][0]->pos.y + 
					(group_of_obs[obj_group][number_in_group[obj_group] - 1]->pos.y - group_of_obs[obj_group][0]->pos.y) / 2;
				z = group_of_obs[obj_group][0]->pos.z + 
					(group_of_obs[obj_group][number_in_group[obj_group] - 1]->pos.z - group_of_obs[obj_group][0]->pos.z) / 2;
				*/

				//find the group center
				Vec3d group_center;
				for(int j = 0; j < number_in_group[obj_group];j++){
					OB * this_obj = group_of_obs[obj_group][j];
					group_center += this_obj->pos;
				}
				group_center /= number_in_group[obj_group];
				//cout << "group center: " << group_center << endl;


				for(int i = 0; i < number_in_group[obj_group];i++){
					
					OB * this_obj = group_of_obs[obj_group][i];
					
					// need to reflect the fact that scaling now occurs at tube centers in nano
					this_obj->scale(new_scale);

					Vec3d set_pos( (this_obj->pos - group_center)*new_scale + group_center );
					this_obj->setPos(set_pos);
					/*this_obj->setPos(Vec3d((this_obj->pos.x - group_center.x)*new_scale + group_center.x,
											(this_obj->pos.y - group_center.y)*new_scale + group_center.y,
											(this_obj->pos.z - group_center.z)*new_scale + group_center.z));*/
				}
			}
		}

		SimMicroscopeServer.scaleRcv = false;

	}

	if(SimMicroscopeServer.transRcv){
		float trans_x = SimMicroscopeServer.trans_x;
		float trans_y = SimMicroscopeServer.trans_y;
		float trans_z = SimMicroscopeServer.trans_z;

		float new_trans_x = SimMicroscopeServer.trans_x - current_trans_x;
		float new_trans_y = SimMicroscopeServer.trans_y - current_trans_y;
		float new_trans_z = SimMicroscopeServer.trans_z - current_trans_z;

		current_trans_x = trans_x;
		current_trans_y = trans_y;
		current_trans_z = trans_z;

		double ratio = SimMicroscopeServer.Sim_to_World_x;

		int obj_group = NANOTUBE_GROUP;
		for(int i = 0; i < number_in_group[obj_group];i++){
			
			OB * this_obj = group_of_obs[obj_group][i];
			this_obj->setPos(Vec3d(ob[i]->pos.x + new_trans_x*ratio,
								   ob[i]->pos.y + new_trans_y*ratio,
								   ob[i]->pos.z + new_trans_z*ratio));
		}
		SimMicroscopeServer.transRcv = false;

	}
	if(SimMicroscopeServer.rotRcv){
		
		//in radians
		float x = SimMicroscopeServer.rot_x;
		float y = SimMicroscopeServer.rot_y;
		float z = SimMicroscopeServer.rot_z;
		cout << "rot rcv: \t" << "x: " << x << "\ty: " << y << "\tz: " << z << endl;

		//in radians
		float rel_x = x - current_rot_x;
		float rel_y = y - current_rot_y;
		float rel_z = z - current_rot_z;
		cout << "rel: \t\t" << "x: " << rel_x << "\ty: " << rel_y << "\tz: " << rel_z << endl;


		//in radians
		current_rot_x = x;
		current_rot_y = y;
		current_rot_z = z;

		int obj_group = NANOTUBE_GROUP;

		//start added code

		//Z stuff
		if(rel_z != 0){	
		
			//find the group center
			Vec3d group_center;
			for(int i = 0; i < number_in_group[obj_group];i++){
				OB * this_obj = group_of_obs[obj_group][i];
				group_center += this_obj->pos;
			}
			group_center /= number_in_group[obj_group];
			//cout << "group center: " << group_center << endl;

			//rotate around z

			//translate objects to position rel_z rad. rel. to group_center
			for(i = 0; i < number_in_group[obj_group];i++){
				OB * this_obj = group_of_obs[obj_group][i];
				Vec3d rel_pos = this_obj->pos - group_center;
				Vec3d new_pos = rel_pos.rotate3(Vec3d(0,0,1),rel_z) + group_center;
				this_obj->setPos(new_pos);
			}

			for(i = 0; i < number_in_group[obj_group];i++){
				Ntube * n = (Ntube *)group_of_obs[obj_group][i];
				Vec3d left;
				Vec3d right;
				left += n->pos;
				left -= n->axis*n->leng/2;
				right += n->pos;
				right += n->axis*n->leng/2;

				left = n->pos + Vec3d(left - n->pos).rotate3(Vec3d(0,0,1),rel_z);
				right = n->pos + Vec3d(right - n->pos).rotate3(Vec3d(0,0,1),rel_z);
				n->set(left,right,n->diam);		
			}
		}

		//Y stuff
		if(rel_y != 0){		

			//find the group center
			Vec3d group_center;
			for(int i = 0; i < number_in_group[obj_group];i++){
				OB * this_obj = group_of_obs[obj_group][i];
				group_center += this_obj->pos;
			}
			group_center /= number_in_group[obj_group];
			//cout << "group center: " << group_center << endl;
			
			//rotate around y

			//translate objects to position rel_y rad. rel. to group_center
			for(i = 0; i < number_in_group[obj_group];i++){
				OB * this_obj = group_of_obs[obj_group][i];
				Vec3d rel_pos = this_obj->pos - group_center;
				Vec3d new_pos = rel_pos.rotate3(Vec3d(0,1,0),rel_y) + group_center;
				this_obj->setPos(new_pos);
			}

			for(i = 0; i < number_in_group[obj_group];i++){
				Ntube * n = (Ntube *)group_of_obs[obj_group][i];
				Vec3d left;
				Vec3d right;
				left += n->pos;
				left -= n->axis*n->leng/2;
				right += n->pos;
				right += n->axis*n->leng/2;

				left = n->pos + Vec3d(left - n->pos).rotate3(Vec3d(0,1,0),rel_y);
				right = n->pos + Vec3d(right - n->pos).rotate3(Vec3d(0,1,0),rel_y);
				n->set(left,right,n->diam);
			}
		}

		//X stuff
		if(rel_x != 0){			

			//find the group center
			Vec3d group_center;
			for(int i = 0; i < number_in_group[obj_group];i++){
				OB * this_obj = group_of_obs[obj_group][i];
				group_center += this_obj->pos;
			}
			group_center /= number_in_group[obj_group];
			//cout << "group center: " << group_center << endl;

			//rotate around x

			//translate objects to position rel_x rad. rel. to group_center
			for(i = 0; i < number_in_group[obj_group];i++){
				OB * this_obj = group_of_obs[obj_group][i];
				Vec3d rel_pos = this_obj->pos - group_center;
				Vec3d new_pos = rel_pos.rotate3(Vec3d(1,0,0),rel_x) + group_center;
				this_obj->setPos(new_pos);
			}
			
			//rotate objects about themselves in x
			for(i = 0; i < number_in_group[obj_group];i++){
				Ntube * n = (Ntube *)group_of_obs[obj_group][i];
				Vec3d left;
				Vec3d right;
				left += n->pos;
				left -= n->axis*n->leng/2;
				right += n->pos;
				right += n->axis*n->leng/2;

				left = n->pos + Vec3d(left - n->pos).rotate3(Vec3d(1,0,0),rel_x);
				right = n->pos + Vec3d(right - n->pos).rotate3(Vec3d(1,0,0),rel_x);
				n->set(left,right,n->diam);
			}
		}

   
		//end added code

		SimMicroscopeServer.rotRcv = false;

	}
	

    // end of display frame, so flip buffers
    glutSwapBuffers();
  }
}

// This idle function marks both windows for redisplay, which will cause
// their display callbacks to be invoked.
void commonIdleFunc( void ) {
#define PERIOD 30

	x_to_y = 1.0;
	if(SimMicroscopeServer.grid_size_rcv){
		x_to_y = SimMicroscopeServer.Sim_to_World_y/SimMicroscopeServer.Sim_to_World_x;//=world x to world y
	}
  //ANDREA:  changes here
  if(connection_to_nano){
	if(x_to_y != 1){//alter if ratio is not 1 on nano side so that ratio is the same on this side
		if (undone){
			glutSetWindow(mainWindowID);
			glutReshapeWindow((int)(windowWidth*x_to_y), (int)(windowHeight));
			glutSetWindow( viewWindowID );
			glutReshapeWindow((int)(windowWidth*x_to_y), (int)(windowHeight));
			glutSetWindow( depthWindowID );
			glutReshapeWindow((int)(DEPTHSIZE*x_to_y), (int)(DEPTHSIZE));
			undone = false;
		}
	}
	//ANDREA:  take out this else later...  test
	/*else{
		if (undone){
			glutSetWindow(mainWindowID);
			glutReshapeWindow((int)(windowWidth*x_to_y*1.5), (int)(windowHeight));
			glutSetWindow( viewWindowID );
			glutReshapeWindow((int)(windowWidth*x_to_y*1.5), (int)(windowHeight));
			glutSetWindow( depthWindowID );
			glutReshapeWindow((int)(DEPTHSIZE*x_to_y*1.5), (int)(DEPTHSIZE));
			undone = false;
		}
		else{
			glutSetWindow( mainWindowID);
			glutReshapeWindow((int)(windowWidth*x_to_y*2.0/3.0), (int)(windowHeight));
			glutSetWindow(viewWindowID);
			glutReshapeWindow((int)(windowWidth*x_to_y*2.0/3.0), (int)(windowHeight));
			glutSetWindow(depthWindowID);
			glutReshapeWindow((int)(DEPTHSIZE*x_to_y*2.0/3.0), (int)(DEPTHSIZE));			
			undone = true;
		}
	}*/
  }
  //

  static int run_cnt=0;
  if (dna) {
    dna->run();
    run_cnt++;

    if ((run_cnt % PERIOD) == 0) {
      glutSetWindow( mainWindowID ); glutPostRedisplay();
      glutSetWindow( viewWindowID ); glutPostRedisplay();
      glutSetWindow( depthWindowID ); glutPostRedisplay();
    }
  }
  else {
    glutSetWindow( mainWindowID ); glutPostRedisplay();
    glutSetWindow( viewWindowID ); glutPostRedisplay();
    glutSetWindow( depthWindowID ); glutPostRedisplay();
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
  double orthoFrustumNearEdge = scanNear;
  /* All far pts get mapped to scanFar. Allow round off of 1 */
  double orthoFrustumFarEdge = scanFar;

  // set projection matrix to orthoscopic projection matching current window
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho( orthoFrustumLeftEdge, orthoFrustumLeftEdge + orthoFrustumWidth,
        orthoFrustumBottomEdge, orthoFrustumBottomEdge + orthoFrustumHeight, orthoFrustumNearEdge, orthoFrustumFarEdge );
}

// adjust the ortho projection to match window aspect ratio and keep circles round.
void adjustOrthoProjectionToViewWindow( void ) {

#if 0
  double orthoFrustumNearEdge = -scanFar;
  double orthoFrustumFarEdge = -scanNear;
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glOrtho( orthoFrustumLeftEdge, orthoFrustumLeftEdge + orthoFrustumWidth,
        orthoFrustumNearEdge, orthoFrustumFarEdge, 
        orthoFrustumBottomEdge, orthoFrustumBottomEdge + orthoFrustumHeight);
#else
  double orthoFrustumNearEdge = -scanFar;
  double orthoFrustumFarEdge = -scanNear;
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
  for (int i=0;i<numObs;i++) {
    if (ob[i] == UNUSED) continue;
    ob[i]->keyboardFunc(key,x,y);
  }
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
    char c;
    float radius = 0.0;
	float depth = 0.0;
	float radius1,radius2,depth1,depth2;
	float spacing = 0.0;
	float spacing2;
	float length = 0.0;
    float decimal = 0.0;
	int * dimergroup;
	bool add = true;
    double volume;
	int counter = 0;
	//float val[5];
	int i = 0;

    switch (key) {
    case 'u' : /* want to see uncertainty map : 
        * use flat shading
        */
      uncertainty_mode = !uncertainty_mode;
      break;
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
	  //deal with radius
	  cout << "Enter a radius: " << flush;      
	  scanf("%f",&radius);
      
	  //deal with length
	  cout << "Enter a length: " << flush;
	  scanf("%f",&length);
      
	  if(SimMicroscopeServer.grid_size_rcv){
		radius = radius * SimMicroscopeServer.Sim_to_World_x;
		length = length * SimMicroscopeServer.Sim_to_World_x;

		addNtube( NTUBE, Vec3d( DEPTHSIZE/2, DEPTHSIZE/2, radius), 0., 0., 0., length, radius*2.0);
	  }
      else{
		addNtube( NTUBE, Vec3d( DEPTHSIZE/2, DEPTHSIZE/2, radius), 0., 0., 0., length, radius*2.0);
	  }
      selectedOb = numObs-1;
      break;
    case 'm' ://add monomer
	  cout << endl << "Protein List" << endl;
	  cout << "1		MLH1" << endl;
	  cout << "2		PMS1" << endl;
	  cout << "3		BAM" << endl;
	  cout << "4		APFE" << endl;
	  cout << "5		throg" << endl;
	  cout << "6		helicase" << endl;
	  cout << "7		any other protein (you enter the dimensions)" << endl;
      cout << endl <<"Enter a number for the protein you want: " << flush;
      scanf("%c",&c);

	  switch(c){
	  case '1':
		  radius = 9.3075;
		  depth = -8.0049;
		  break;
	  case '2':
		  radius = 12.2399;
		  depth = -10.8306;
		  break;
	  case '3':
		  radius = 4.6438;
		  depth = -2.35942;
		  break;
	  case '4':
		  radius = 11.5845;
		  depth = -8.30443;
		  break;
	  case '5':
		  radius = 64.6241;
		  depth = -62.5739;
		  break;
	  case '6':
		  radius = 35.1499;
		  depth = -34.3325;
		  break;
	  case '7':
		  cout << "Enter a radius: " << flush;
		  scanf("%f",&radius);
		  		  
		  cout << "Enter a depth (place to put origin of protein sphere in z): " << flush;
		  scanf("%f",&depth);
		  
		  break;
	  default:
		  cout << "Please try again with a valid option number." << endl
			   << "Try 7 if your protein is not in the list given." << endl;
		  add = false;
		  break;
	  }
	
	  if(add){//add if a protein has been selected
		  if(SimMicroscopeServer.grid_size_rcv){
			radius = radius * SimMicroscopeServer.Sim_to_World_x;
			addNtube( SPHERE, Vec3d(DEPTHSIZE/2, DEPTHSIZE/2, depth*SimMicroscopeServer.Sim_to_World_x), 
				0., 0., 0., 0., radius*2.0);
		  }
		  else{
			addNtube( SPHERE, Vec3d(DEPTHSIZE/2, DEPTHSIZE/2, depth), 0., 0., 0., 0., radius*2.0);
		  }
		  selectedOb = numObs-1;
	  }

      break;
	case '2' ://add dimer
		cout << endl << "Protein List" << endl;
		cout << "1		MLH1-PMS1" << endl;
		cout << "2		helicase dimer" << endl;
		cout << "3		any other protein dimer (you enter the dimensions)" << endl;
		cout << endl << "Enter a number for the protein you want: " << flush;
		scanf("%c",&c);
		
		switch(c){
		case '1':
		  radius1 = radius2 = 35.1499;
		  depth1 = depth2 = -34.3325;
		  spacing2 = 2*7.53622;//2 times the base radius (at the surface) for one protein
		  break;
		case '2':
		  radius1 = 9.3075;
		  depth1 = -8.0049; 
		  radius2 = 12.2399;
		  depth2 = -10.8306;
		  spacing2 = 10.77;
		  break;
		case '3':
		  cout << "Enter the radius and depth for the first protein then the second." << endl;
		  for(i = 0;i<2;++i){
			  cout << "Enter a radius: " << flush;
			  scanf("%f",&radius);
			  			  
			  cout << "Enter a depth (place to put origin of protein sphere in z): " << flush;
			  scanf("%f",&depth);
			  
			  if(i==0){
				  radius1 = radius;
				  depth1 = depth;
			  }
			  else{
				  radius2 = radius;
				  depth2 = depth;
				  cout << "Enter the spacing between the proteins on the surface: " << flush;
				  scanf("%f",&spacing2);
			  }
		  }
		  break;
		default:
		  cout << "Please try again with a valid option number." << endl
			   << "Try 3 if your protein is not in the list given." << endl;
		  add = false;
		  break;
		}

		if(add){//add if a dimer has been selected
			radius = radius1;
			depth = depth1;

			dimergroup = NULL;

			if(SimMicroscopeServer.grid_size_rcv){
				for(i = 0;i<2;++i){
					radius = radius * SimMicroscopeServer.Sim_to_World_x;
					
					addNtube( SPHERE, Vec3d(DEPTHSIZE/2, DEPTHSIZE/2 + spacing*SimMicroscopeServer.Sim_to_World_x, 
						depth*SimMicroscopeServer.Sim_to_World_x), 0., 0., 0., 0., radius*2.0,dimergroup);
					dimergroup = new int();
					*dimergroup = numGroups-1;
					spacing = spacing2;
					radius = radius2;
					depth = depth2;
				}	
			}	
			else{
				for(i = 0;i<2;++i){
					addNtube( SPHERE, Vec3d(DEPTHSIZE/2, DEPTHSIZE/2 + spacing, depth), 0., 0., 0., 0., radius*2.0,
						dimergroup);
					dimergroup = new int();
					*dimergroup = numGroups-1;
					spacing = spacing2;
					radius = radius2;
					depth = depth2;
				}
			}

		  selectedOb = numObs-1;

		}
      break;
	case 'S' :
		saveAllGroups();
		break;
	case 'L':
		retrieveAllGroups();
		break;
	case 'F':
		save_for_eroder = true;
		break;
    case 's' ://add sphere
      cout << "Enter a radius: " << flush;
	  scanf("%f",&radius);
      
	  if(SimMicroscopeServer.grid_size_rcv){
		radius = radius * SimMicroscopeServer.Sim_to_World_x;
	  }
      addNtube( SPHERE, Vec3d( DEPTHSIZE/2, DEPTHSIZE/2, radius), 0., 0., 0., 0., radius*2.0);
      selectedOb = numObs-1;

      break;
	case 'h' ://add 1/2 sphere
      cout << "Enter a radius: " << flush;
	  scanf("%f",&radius);
      
	  if(SimMicroscopeServer.grid_size_rcv){
		radius = radius * SimMicroscopeServer.Sim_to_World_x;
	  }
      addNtube( SPHERE, Vec3d( DEPTHSIZE/2, DEPTHSIZE/2, 0), 0., 0., 0., 0., radius*2.0);
      selectedOb = numObs-1;

      break;
    case 'f' ://find volume
	  double avgHeight, maxHeight,area;
      volume = find_volume(avgHeight,maxHeight,area);
      if(VolumeFilename != NULL){
        fout2 << VolumeFilename << ": ";
      }
      fout2 << endl << endl << proteinName << "\nWater: " << water << "\nVol: " << volume <<"\nAvg Height: " 
		    << avgHeight << "\nMax Height: " << maxHeight << "\nArea: " << area << flush;
      cout << "Volume of all objects on plane is " << volume << " " 
            << units << "^3.\n\n" << flush;
      fout2.close();
      break;
    case 't' :
		if(SimMicroscopeServer.grid_size_rcv){
			addTriangle(Vec3d(0.,0.,DEFAULT_TRIANGLE_SIDE/3.0*SimMicroscopeServer.Sim_to_World_x),
				Vec3d(DEFAULT_TRIANGLE_SIDE*SimMicroscopeServer.Sim_to_World_x,0.0,DEFAULT_TRIANGLE_SIDE/2.0
				*SimMicroscopeServer.Sim_to_World_x),Vec3d(DEFAULT_TRIANGLE_SIDE/2.0*SimMicroscopeServer.Sim_to_World_x,
				DEFAULT_TRIANGLE_SIDE/2.0*SimMicroscopeServer.Sim_to_World_x,
				DEFAULT_TRIANGLE_SIDE/2.0*SimMicroscopeServer.Sim_to_World_x));
		}
		else{
			addTriangle(Vec3d(0.,0.,DEFAULT_TRIANGLE_SIDE/3.0),
				Vec3d(DEFAULT_TRIANGLE_SIDE,0.0,DEFAULT_TRIANGLE_SIDE/2.0),
				Vec3d(DEFAULT_TRIANGLE_SIDE/2.0,DEFAULT_TRIANGLE_SIDE/2.0,
				DEFAULT_TRIANGLE_SIDE/2.0));
		}
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
#if DISP_LIST
      remake_cone_sphere(*(tip.icsTip));
#endif
      break;
    case 'R' :
      tip.dec_r();
#if DISP_LIST
      remake_cone_sphere(*(tip.icsTip));
#endif
      break;
    case 'a' ://change angle, slant of the tip
      tip.inc_theta();
#if DISP_LIST
      remake_cone_sphere(*(tip.icsTip));
#endif
      break;
    case 'A' ://change angle, slant of the tip
      tip.dec_theta();
#if DISP_LIST
      remake_cone_sphere(*(tip.icsTip));
#endif
      break;
    case '*' ://increase tesselation
      tesselation += 5;
      
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
      if (selectedOb != NULLOB) {
    ob[selectedOb]->keyboardFunc(key,x,y);
      }
      else {
    globalkeyboardFunc(key,x,y);
      }
      break;
    }
    glutPostRedisplay(); // in case something was changed

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
    if( state == GLUT_DOWN ){
		//cout << "processing button press " << bcounter++ << endl;
		buttonpress=LEFT_BUTTON;
		int nState = GetKeyState(VK_SHIFT);//check status of shift key
	
		if(nState < 0){//indicates shift button held down
			selectedOb = findNearestObToMouse(xy_or_xz);
			cout << "grouping" << endl;
			if(nState != oldShift){//new shift -> new group
				oldShift = nState;//oldShift gives last shift val, shift down val toggles
							      //between -127 and -128, so we can tell if new shift down
							      //or old one still going...

				cout << "first in group " << ob[selectedOb]->obj_group << endl;
				if(group == NULL)	group = new int();
				//*group = changeGroup(ob[selectedOb],NULL);//change group
				*group = ob[selectedOb]->obj_group;
			}
			else{//old shift, process objects under that shift hold-down
				if(inGroup(ob[selectedOb],group)){//still holding down shift but changed mind...
					removeFromGroup(ob[selectedOb],group);
				}
				else{
					changeGroup(ob[selectedOb],group);
				}
			}
		}
		else{
			grabNearestOb(xy_or_xz);
		}
	}
    else if( state == GLUT_UP ) {}
    break;
  case GLUT_RIGHT_BUTTON: 
    /* this selects one side of the triangle so that we can perform all
     * our nanotube operations on that side. 
     */
    if( state == GLUT_DOWN ) {
		buttonpress=RIGHT_BUTTON; 
		select_triangle_side();
    }
    else if( state == GLUT_UP ) {}
    break;
  }

  glutPostRedisplay();
}


// Callback routine: called when mouse is moved while a button is down.
// Only called when cursor loc changes.
// x,y: cursor loc in window coords
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
    // moveGrabbedOb();
    if(ob[selectedOb] != NULL){
        ob[selectedOb]->moveGrabbedOb(vMouseWorld);
    }
    
    // glutPostRedisplay();
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
    vMouseWorld.x = (xMouseNormalized * orthoFrustumWidth) + orthoFrustumLeftEdge;
    vMouseWorld.y = (yMouseNormalized * orthoFrustumHeight) + orthoFrustumBottomEdge;
    vMouseWorld.z = 0; 
  }
  else {
    // calculate cursor position in ortho frustum's XY plane
    vMouseWorld.y = 0; 
    vMouseWorld.x = (xMouseNormalized * orthoFrustumWidth) + orthoFrustumLeftEdge;
    vMouseWorld.z = (yMouseNormalized * orthoFrustumHeight) + orthoFrustumBottomEdge;
  }
}


void grabNearestOb(int xy_or_xz) {
  selectedOb = findNearestObToMouse(xy_or_xz);
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
    if( dist < nearestDist && dist < thresholdDist ) {
      nearestDist = dist;
      nearestOb = i;
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


