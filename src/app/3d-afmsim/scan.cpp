/* Gokul Varadhan
 * varadhan@cs.unc.edu
 * May 2001
 */
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
#include "scan.h"
#include "lightcol.h"
#include "sim.h"
#include "input.h"

// raw values (normalized) from Z-buffer
float zBuffer[ 128*128 ];			

float colorBuffer[ 128*128 ];			

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
//double scanNear =  -100.;	// near end of Z-buffer range
double scanNear =  -128.;	// near end of Z-buffer range

double scanFar  =   0.;	// far  end of Z-buffer range

void get_z_buffer_values() {

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
  glReadPixels( 0, 0, pixelGridSize, pixelGridSize, GL_DEPTH_COMPONENT, GL_FLOAT, zBufferPtr );
  for(int j=0; j<scanResolution; j++ ) {
    for(int i=0; i<scanResolution; i++ ) {
      double zNormalized = zBuffer[ j*pixelGridSize + i ];
      // -scanNear and -scanFar are the real depth values in the viewing 
      // volume (see definition of glOrtho)
      double zDepth = -scanFar + (1-zNormalized)*(-scanNear + scanFar);
      // Open GL convention
      zHeight[j][i] = zDepth;
    }
  }
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

void  doImageScanApprox() 
{
  // Render tube images (enlarged to account for tip radius)
  // into window.  
  // (We don't really care about the image, just the depth.)

  imageScanDepthRender();

  // Read (normalized) Z-buffer values from the depth window.
  get_z_buffer_values();

  get_color_buffer_values();
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
  for( int j=0; j<scanResolution-1; j++ ) {
    for( int i=0; i<scanResolution-1; i++ ) {
      double x = i * scanStep  +  scanXMin;
      double y = j * scanStep  +  scanYMin;
      double dx = scanStep;
      double dy = scanStep;

      // Get the the 4 (x,y,z) coords on the corners of this grid cell.
      // Show objects above the surface only
      double x1 = x;     double y1 = y;     double z1 = zHeight[j  ][i  ];
      double x2 = x+dx;  double y2 = y;     double z2 = zHeight[j][i+1  ];
      double x3 = x+dx;  double y3 = y+dy;  double z3 = zHeight[j+1][i+1];
      double x4 = x;     double y4 = y+dy;  double z4 = zHeight[j+1][i];

      // gray color values
      double gcol1 = colorBuffer[j*scanResolution + i];
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





