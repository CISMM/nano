/* Gokul Varadhan
 * varadhan@cs.unc.edu
 * May 2001
 */
/*$Id$*/
#include <stdlib.h>		//stdlib.h vs cstdlib
#include <stdio.h>		//stdio.h vs cstdio
#include <iostream>
#include <fstream>
#include <vector>
#include <math.h>		//math.h vs cmath
#include <GL/glut_UNC.h>
#include "Vec3d.h"
#include "3Dobject.h"
#include "ConeSphere.h"
#include "Tips.h"
#include <string.h>
#include "defns.h"
#include "scan.h"
#include "lightcol.h"
#include "sim.h"
#include "input.h"


// raw values (normalized) from Z-buffer
float zBuffer[ DEPTHSIZE*DEPTHSIZE ];			

float colorBuffer[ DEPTHSIZE*DEPTHSIZE ];			

// array of heights: image scan data
double zHeight        [MAX_GRID][MAX_GRID];	
double** zDistance;
double** zDistanceScaled;


// scan grid resolution
int    scanResolution = DEPTHSIZE;	
// scan grid pitch (sample-to-sample spacing)
double scanStep   = 1.;		
// scan grid origin X coord (left side)
double scanXMin =  0.;		
// scan grid origin Y coord (bottom)
double scanYMin =  0.;		
//double scanLength = scanStep * scanResolution;	
//double scanXMax =   scanXMin + (scanStep * scanResolution);
//double scanYMax =   scanYMin + (scanStep * scanResolution);
//double scanNear =  -100.;	// near end of Z-buffer range
double scanNear =  -DEPTHSIZE;	// near end of Z-buffer range

double scanFar  =   0.;	// far  end of Z-buffer range

double Volume;
int numberUnits_onedim = DEPTHSIZE;
int numberPixels_onedim = DEPTHSIZE;





//saves data into .er file in the following format:
//xdim ydim
//data(0,0)		data(0,1) ...
//data(1,0)		data(1,1) ...
//...
void save_to_eroder(){
	ofstream outstream;
	char filename[100];
	int i = 0;

	cout << "Save for Eroder\nEnter filename to save to (do not include extension): ";
	scanf("%s",filename);
	char * fileminusext;//filename minus extension
	fileminusext = strtok(filename,".");
	strcat(fileminusext,".er");
	
	cout << "The filename is: " << fileminusext << endl
		 << "All files are saved as type '*.er'" << endl;;

	outstream.open(fileminusext);
    outstream << scanResolution << " " << scanResolution << endl;;

	int rownumber;
	for(int j=0; j<scanResolution; j++ ) {
		rownumber = scanResolution-j-1;//rownumber counts from the top of the grid down

		for(int i=0; i<scanResolution; i++ ) {	
			outstream.width(7);
			outstream.precision(6);
			outstream.setf(ios_base::showpoint);
			outstream.fill('0');
			outstream.setf(ios_base::right);
			outstream << zDistanceScaled[rownumber][i] << "\t";//feed data into array
		}
		outstream << endl;
	}
	outstream.close();

}


void get_z_buffer_values(double xworldratio) {

  GLint PackAlignment;
  glGetIntegerv(GL_PACK_ALIGNMENT,&PackAlignment); 
  glPixelStorei(GL_PACK_ALIGNMENT,1); 

  
  // Read (normalized) Z-buffer values from the depth window.
  // Scale them back to correct Z-values and use as 
  // Z-heights in image scan grid.  
  // static double zBuffer[ 128*128 ];
  void* zBufferPtr = &(zBuffer[0]);
  int pixelGridSize = DEPTHSIZE;		// must match window size
  // width and height the same for now

  glReadBuffer(GL_BACK);


  //create new rows for zDistance and zDistanceScaled only if they have not yet been created
  if(zDistance == NULL){
	  zDistance = new double*[MAX_GRID];
	  for(int j=0; j<scanResolution; j++ ) {
		  zDistance[j] = NULL;
		  //initialize to NULL so can check and see if we have to create the row array later
		  //(only want to do this once too...)
	  }
  }
  if(zDistanceScaled == NULL){
	  zDistanceScaled = new double*[MAX_GRID];
	  for(int j=0; j<scanResolution; j++ ) {
		  zDistanceScaled[j] = NULL;
	  }
  }
  
  
  glReadPixels( 0, 0, pixelGridSize, pixelGridSize, GL_DEPTH_COMPONENT, GL_FLOAT, zBufferPtr );

  int rownumber;
  for(int j=0; j<scanResolution; j++ ) {
	rownumber = scanResolution-j-1;//rownumber counts from the top of the grid down

	//create row array only if hasn't been done before
    if(zDistance[j] == NULL)		zDistance[j] = new double[MAX_GRID];
	if(zDistanceScaled[j] == NULL)	zDistanceScaled[j] = new double[MAX_GRID];

    for(int i=0; i<scanResolution; i++ ) {
      float zNormalized = zBuffer[rownumber*pixelGridSize + i];
	  //changed back for now
	  //changed to (scanResolution-j-1) from j so that top of zBuffer ends up in top of zDistance array
	  //zBuffer has lower values at the bottom of the grid, while zDistance should have lower values at the 
	  //top of the grid to match the way data read with microscope
      
      // -scanNear and -scanFar are the real depth values in the viewing 
      // volume (see definition of glOrtho)
      double zDepth = -scanFar + (1-(double)zNormalized)*(-scanNear + scanFar);
      // Open GL convention
      zHeight[rownumber][i] = zDepth;
      zDistance[j][i] = (1-zNormalized)*(-scanNear + scanFar);
	  zDistanceScaled[j][i] = zDistance[j][i]/xworldratio;//ANDREA:  check on this division
    }
  }

}

//returns the volume total for all the objects in the plane, in the units of the objects entered
double find_volume(double & avgHeight, double & maxHeight,double & area){

  avgHeight = 0.0;
  double sumHeight = 0.0;
  maxHeight = 0.0;
  area = 0.0;

  Volume = 0;
  double onePixelArea = pow(((double)numberUnits_onedim/(double)numberPixels_onedim),2);
  int numberPixelsInSample = 0;
  
  for(int j= 0; j<scanResolution; j++){
    for(int i=0; i<scanResolution; i++){
      Volume += (zDistance[j][i] * onePixelArea);
	  //= sum(zDistance[j][i])/(128*128) * onePixelArea * (128*128)
	  //= avg. pixel height * pixel area * number of pixels
	  //= avg. volume/pixel * number pixels = total volume
		
	  if(zDistance[j][i] > 0){
		numberPixelsInSample++;
		sumHeight += zDistance[j][i];
		if(zDistance[j][i] > maxHeight)	maxHeight = zDistance[j][i];
	  }
    }
  }

  avgHeight = sumHeight/double(numberPixelsInSample);
  area = numberPixelsInSample*onePixelArea;

  cout << "Average Height = " << avgHeight << endl;
  cout << "Max Height = " << maxHeight << endl;
  cout << "Area = " << area << endl;


  return Volume;
}


void get_color_buffer_values() {

  GLint PackAlignment;
  glGetIntegerv(GL_PACK_ALIGNMENT,&PackAlignment); 
  glPixelStorei(GL_PACK_ALIGNMENT,1); 

  glReadBuffer(GL_BACK);

  // could choose red, green or blue
  int pixelGridSize = DEPTHSIZE;		// must match window size
  glReadPixels(0,0,pixelGridSize,pixelGridSize,GL_GREEN,GL_FLOAT,colorBuffer);
}

double ** doImageScanApprox(int& row_col_length,double xworldratio) 
{
  // Render tube images (enlarged to account for tip radius)
  // into window.  
  // (We don't really care about the image, just the depth.)

  imageScanDepthRender();

  // Read (normalized) Z-buffer values from the depth window.
  get_z_buffer_values(xworldratio);

  get_color_buffer_values();

  row_col_length = scanResolution;


  return zDistanceScaled;

}


/* This is the most critical part of the afmsim. This is the one which 
 * renders the afm scan
 */
// display graphics in the depth window.
void  imageScanDepthRender()  {
  // draw into depth window
  glutSetWindow( depthWindowID );

  // Setup OpenGL state.
  glClearDepth(1.);
  glClearColor(0, 0, 0, 1);

  glDrawBuffer(GL_BACK);
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  glEnable(GL_DEPTH_TEST);

  glFinish();

  GLint UnpackAlignment;
  glGetIntegerv(GL_UNPACK_ALIGNMENT,&UnpackAlignment); 
  glPixelStorei(GL_UNPACK_ALIGNMENT,1); 


  // set projection matrix to orthoscopic projection matching current window
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(  scanXMin,   scanXMin + (scanStep * scanResolution),
	    scanYMin,   scanYMin + (scanStep * scanResolution),
	    scanNear,   scanFar   );

  // set modeling matrix to identity
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  lighting();
  setColor( WHITE );

  for( int i=0; i<numObs; i++ ) {
    if (ob[i] == UNUSED) continue;	  
    switch (tip.type) {
    case SPHERE_TIP :
      glPushMatrix();
      // go down by tip radius
      glTranslatef( 0., 0., -tip.spTip->r);  
      if (uncertainty_mode) {
		ob[i]->uncert_afm_sphere_tip(*(tip.spTip));
      }
      else {
		ob[i]->afm_sphere_tip(*(tip.spTip));
      }
      glPopMatrix();
      break;
    case INV_CONE_SPHERE_TIP :
      glPushMatrix();
      // go down by tip radius
      glTranslatef( 0., 0., -tip.icsTip->r);  

      if (uncertainty_mode) {
		ob[i]->uncert_afm_inv_cone_sphere_tip(*(tip.icsTip));
      }
      else {
		ob[i]->afm_inv_cone_sphere_tip(*(tip.icsTip));
	  }

      glPopMatrix();
      break;
    }
   }

  glFinish();

  glPixelStorei(GL_UNPACK_ALIGNMENT,UnpackAlignment); 

}

// Display the image scan grid (a depth image).
void showGrid( void ) {
  int gridColor = GREEN;

  // Display depth image surface.  
  // The variable "gridStyle" controls which of several visualizations
  // of the surface are used.
  //  shadingModel = GL_SMOOTH;
  setColor( gridColor );

  lighting();
  int rownumber;
  for(int j=0; j<scanResolution; j++ ) {
	rownumber = scanResolution-j-1;//rownumber counts from the top of the grid down

	
    for( int i=0; i<scanResolution; i++ ) {
      double x = i * scanStep  +  scanXMin;
      double y = j * scanStep  +  scanYMin;
      double dx = scanStep;
      double dy = scanStep;
	  double x1,x2,x3,x4,y1,y2,y3,y4,z1,z2,z3,z4;

	  if(i<scanResolution-1 && j<scanResolution-1){
		  // Get the the 4 (x,y,z) coords on the corners of this grid cell.
		  // Show objects above the surface only
		   x1 = x;      y1 = y;      z1 = zHeight[j][i];
		   x2 = x+dx;   y2 = y;      z2 = zHeight[j][i+1];
		   x3 = x+dx;   y3 = y+dy;   z3 = zHeight[j+1][i+1];
		   x4 = x;      y4 = y+dy;   z4 = zHeight[j+1][i];
	  }
	  else if(i ==scanResolution-1 && j<scanResolution-1){
		  // Get the the 4 (x,y,z) coords on the corners of this grid cell.
		  // Show objects above the surface only
		   x1 = x;      y1 = y;      z1 = zHeight[j  ][i-1  ];
		   x2 = x+dx;   y2 = y;      z2 = zHeight[j][i];
		   x3 = x+dx;   y3 = y+dy;   z3 = zHeight[j+1][i];
		   x4 = x;      y4 = y+dy;   z4 = zHeight[j+1][i-1];		
	  }
	  else if(i < scanResolution-1 && j==scanResolution-1){
		  // Get the the 4 (x,y,z) coords on the corners of this grid cell.
		  // Show objects above the surface only
		   x1 = x;      y1 = y;      z1 = zHeight[j-1][i];
		   x2 = x+dx;   y2 = y;      z2 = zHeight[j-1][i+1];
		   x3 = x+dx;   y3 = y+dy;   z3 = zHeight[j][i+1];
		   x4 = x;      y4 = y+dy;   z4 = zHeight[j][i];
	  }
	  else{
		  // Get the the 4 (x,y,z) coords on the corners of this grid cell.
		  // Show objects above the surface only
		   x1 = x;      y1 = y;      z1 = zHeight[j-1][i-1];
		   x2 = x+dx;   y2 = y;      z2 = zHeight[j-1][i];
		   x3 = x+dx;   y3 = y+dy;   z3 = zHeight[j][i];
		   x4 = x;      y4 = y+dy;   z4 = zHeight[j][i-1];
	  }
      // gray color values
      double gcol1 = colorBuffer[rownumber*scanResolution + i];
      //      double gcol2 = colorBuffer[j*scanResolution + i+1];
      //      double gcol3 = colorBuffer[(j+1)*scanResolution + i+1];
      //      double gcol4 = colorBuffer[(j+1)*scanResolution + i];

      if (uncertainty_mode) {
	glColor3f(gcol1, gcol1, gcol1);
      }

      // calc normal to plane through P1, P2, P3 using (P1-P3) x (P1-P2)

      Vec3d A = Vec3d(x1,y1,z1);
      Vec3d B = Vec3d(x2,y2,z2);
      Vec3d C = Vec3d(x3,y3,z3);
      Vec3d D = Vec3d(x4,y4,z4);
      Vec3d xyz = Vec3d :: crossProd(A-B,A-C);
      xyz.normalize();
      // draw the triangle 
      glBegin(GL_POLYGON);
      //      glNormal3f(xn,yn,zn);
      glNormal3f( xyz.x, xyz.y, xyz.z);
      //      glColor3f(gcol1, gcol1, gcol1);
      glVertex3f( x1, y1, z1 );
      //      glColor3f(gcol2, gcol2, gcol2);
      glVertex3f( x2, y2, z2 );
      //      glColor3f(gcol3, gcol3, gcol3);
      glVertex3f( x3, y3, z3 );
      glEnd();
      if (afm_scan==SOLID_AFM) {      
	Vec3d xyz2 = Vec3d :: crossProd(A-C,A-D);
	xyz2.normalize();
	
	glBegin(GL_POLYGON);
	glNormal3f( xyz2.x, xyz2.y, xyz2.z );
	//	glColor3f(gcol3, gcol3, gcol3);
	glVertex3f( x3, y3, z3 );
	//	glColor3f(gcol4, gcol4, gcol4);
	glVertex3f( x4, y4, z4 );
	//	glColor3f(gcol1, gcol1, gcol1);
	glVertex3f( x1, y1, z1 );
	glEnd();
      }
    }
  }
  setColor( gridColor );
  glFlush();
}





