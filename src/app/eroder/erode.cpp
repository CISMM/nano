
#include <stdlib.h>		//stdlib.h vs cstdlib
#include <stdio.h>		//stdio.h vs cstdio
#include <iostream>
#include <fstream>
#include <vector>
#include <math.h>		//math.h vs cmath
#include <GL/glut_UNC.h>
#include "Vec3d.h"
#include "ConeSphere.h"
#include "Tips.h"
#include <string.h>
#include "defns.h"
#include "lightcol.h"
#include "main.h"
#include "input.h"
#include "erode.h"


extern void drawSphere(double diamter);
extern void drawCylinder(double diamter, double height);
/*
// raw values (normalized) from Z-buffer
float zBuffer[ DEPTHSIZE*DEPTHSIZE ];			

float colorBuffer[ DEPTHSIZE*DEPTHSIZE ];			

// array of heights: image scan data
double zHeight        [MAX_GRID][MAX_GRID];	
*/
float* zBuffer=NULL;
float* colorBuffer=NULL;
double**zHeight=NULL;

double** zDistance=NULL;
double** zDistanceScaled=NULL;


// scan grid Resolution
int    xResolution = DEPTHSIZE;	
int    yResolution = DEPTHSIZE;	
// scan grid pitch (sample-to-sample spacing)
double Step   = 1.;		
// scan grid origin X coord (left side)
double Xmin =  0.;		
// scan grid origin Y coord (bottom)
double Ymin =  0.;		
//double scanLength = Step * Resolution;	
//double scanXMax =   Xmin + (Step * Resolution);
//double scanYMax =   Ymin + (Step * Resolution);
//double Near =  -100.;	// Near end of Z-buffer range
double Near =  -DEPTHSIZE;	// Near end of Z-buffer range

double Far  =   0.;	// Far  end of Z-buffer range

double Volume;
int numberUnits_onedim = DEPTHSIZE;
int numberPixels_onedim = DEPTHSIZE;//fix these later

double **MicroscopeHeightArray=NULL;
bool matlab = false;

int array_x_size = xResolution;
int array_y_size = yResolution;

//prints data to stdout or file associated with outstream
//data is followed by spacing_type so that data can be tab delimited, space delimited, etc.
void print_to_file(ostream& outstream, float value, char* spacing_type){
	outstream.width(7);
	outstream.precision(6);
	outstream.setf(ios_base::showpoint);
	outstream.fill('0');
	outstream.setf(ios_base::right);
	outstream << value << spacing_type;

}

//loads data saved into a .er file by save_for_eroder() function in simulator
//if data is not saved by simulator, any other application (or manual writing)
//should save data in the format:
//xdim ydim
//data(0,0) data(0,1) ...
//data(1,0) data(1,1) ...
//...
//data values should be either blank or tab separated
bool load_simulator_data(char* file){
	ifstream fin;
	fin.open(file);
	cout << "Name of file is: " << file << endl;

	int xdim,ydim;

	fin >> xdim >> ydim;
	cout << "xdim,ydim: " << xdim << " " << ydim << endl;
	
	bool retval = false;
	if(xdim != xResolution || ydim != yResolution)	retval = true;//true if incoming data has different size
	xResolution = xdim;
	yResolution = ydim;//change eroder sizes to match incoming data

	MicroscopeHeightArray = new double*[yResolution];
	for(int k=0; k<yResolution; k++ ) {
	  MicroscopeHeightArray[k] = new double[xResolution];
	}

	ofstream outstream;
	char filename[100];
	strcpy(filename,"microscopeDataReadOut.er");
	outstream.open(filename);
	outstream << xResolution << " " << yResolution << endl;

	int rownumber;
	for(int j=0; j<yResolution; j++ ) {
		rownumber = yResolution-j-1;//rownumber counts from the top of the grid down

		for(int i=0; i<xResolution; i++ ) {				
			fin >> MicroscopeHeightArray[rownumber][i];//feed data into array
			if(i < xResolution-1)	print_to_file(outstream, MicroscopeHeightArray[rownumber][i], " ");
			else					print_to_file(outstream, MicroscopeHeightArray[rownumber][i], "\n");
		}
	}

	return retval;
}

//fills MicroscopeHeightArray with values received over nano_eroder_connection
//prints values to file "microscopeData.er"
bool fillArray(nmm_SimulatedMicroscope* nano_eroder_connection){
	bool retval = nano_eroder_connection->FillDataArray(&MicroscopeHeightArray,xResolution,yResolution,
		((float)(-Near + Far))/nano_eroder_connection->get_zrange() );
	if(retval){//only print if correctly filled
		ofstream outstream;
		char filename[100];
		strcpy(filename,"microscopeData.er");
		outstream.open(filename);
		outstream << xResolution << " " << yResolution << endl;
		int rownumber;
		for(int y = 0;y < yResolution;++y){
			rownumber = yResolution-y-1;//rownumber counts up from the bottom
			for(int x = 0;x < xResolution;++x){
				if(x < xResolution-1)	print_to_file(outstream, MicroscopeHeightArray[rownumber][x], " ");
				else					print_to_file(outstream, MicroscopeHeightArray[rownumber][x], "\n");
			}
		}
	}
	return retval;//true/false if filled correctly or not
}

void get_z_buffer_values(double xworldratio) {

  GLint PackAlignment;
  glGetIntegerv(GL_PACK_ALIGNMENT,&PackAlignment); 
  glPixelStorei(GL_PACK_ALIGNMENT,1); 

  // Read (normalized) Z-buffer values from the depth window.
  // Scale them back to correct Z-values and use as 
  // Z-heights in image scan grid.  
 
  void* zBufferPtr;


  //check that arrays are correctly size after new size information received from nano
  if(array_x_size != xResolution || array_y_size != yResolution){	
	  //if not, delete current arrays, and mark ptrs with NULL so mem. allocation occurs
	  //with new sizes
	  if (zBuffer!= NULL)	delete [] zBuffer;
	  if (colorBuffer!= NULL)	delete [] colorBuffer;
	  for(int j=0; j<array_y_size; j++ ) {
		  if(zHeight != NULL){
			  if (zHeight[j]!= NULL){
				  delete [] zHeight[j];
			  }
		  }
		  if(zDistance != NULL){
			  if (zDistance[j]!= NULL){
				  delete [] zDistance[j];
			  }
		  }
		  if(zDistanceScaled != NULL){
			  if (zDistanceScaled[j]!= NULL){
				  delete [] zDistanceScaled[j];
			  }
		  }
	  }
	  if (zHeight!= NULL)	delete [] zHeight;
	  if (zDistance!= NULL)	delete [] zDistance;
	  if (zDistanceScaled!= NULL)	delete [] zDistanceScaled;

	  zBuffer = NULL;
	  colorBuffer = NULL;
	  zHeight = NULL;
	  zDistance = NULL;
	  zDistanceScaled = NULL;
  }
  array_x_size = xResolution;
  array_y_size = yResolution;


  if(zBuffer == NULL){
	  zBuffer = new float[xResolution*yResolution];
  }
  zBufferPtr = &(zBuffer[0]);
  if(colorBuffer == NULL){
	  colorBuffer = new float[xResolution*yResolution];
  }

  glReadBuffer(GL_BACK);

  if(zHeight == NULL){
	  zHeight = new double*[yResolution];
	  for(int j=0; j<yResolution; j++ ) {
		  zHeight[j] = new double[xResolution];
		  //initialize to NULL so can check and see if we have to create the row array later
		  //(only want to do this once too...)
	  }
  }

  //create new rows for zDistance and zDistanceScaled only if they have not yet been created
  if(zDistance == NULL){
	  zDistance = new double*[yResolution];
	  for(int j=0; j<yResolution; j++ ) {
		  zDistance[j] = new double[xResolution];
		  //initialize to NULL so can check and see if we have to create the row array later
		  //(only want to do this once too...)
	  }
  }
  if(zDistanceScaled == NULL){
	  zDistanceScaled = new double*[yResolution];
	  for(int j=0; j<yResolution; j++ ) {
		  zDistanceScaled[j] = new double[xResolution];
	  }
  }
  
  
  glReadPixels( 0, 0, xResolution, yResolution, GL_DEPTH_COMPONENT, GL_FLOAT, zBufferPtr );

  /*ofstream outstream;
  char filename[100];
  strcpy(filename,"zHeightInit.er");	
  outstream.open(filename);
  outstream << xResolution << " " << yResolution << endl;*/

  int rownumber;
  for(int j=0; j<yResolution; j++ ) {
	rownumber = yResolution-j-1;//rownumber counts from the top of the grid down

    for(int i=0; i<xResolution; i++ ) {
      float zNormalized = zBuffer[rownumber*xResolution + i];
	  //changed back for now
	  //changed to (Resolution-j-1) from j so that top of zBuffer ends up in top of zDistance array
	  //zBuffer has lower values at the bottom of the grid, while zDistance should have lower values at the 
	  //top of the grid to match the way data read with microscope
      
      // -Near and -Far are the real depth values in the viewing 
      // volume (see definition of glOrtho)
      double zDepth = -Far + (1-(double)zNormalized)*(-Near + Far);
      // Open GL convention
      zHeight[rownumber][i] = zDepth;
	  //print_to_file(outstream,zHeight[rownumber][i]," ");
      zDistance[j][i] = (1-zNormalized)*(-Near + Far);
	  zDistanceScaled[j][i] = zDistance[j][i]/xworldratio;
    }
  }
  //outstream.close();

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
  
  for(int j= 0; j<yResolution; j++){
    for(int i=0; i<xResolution; i++){
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
  glReadPixels(0,0,xResolution,yResolution,GL_GREEN,GL_FLOAT,colorBuffer);
}

void changeBufferSize(){
  void* zBufferPtr;

  if(array_x_size != xResolution || array_y_size != yResolution){
	  if (zBuffer!= NULL)	delete [] zBuffer;
	  if (colorBuffer!= NULL)	delete [] colorBuffer;
  
	  zBuffer = NULL;
	  colorBuffer = NULL;
  }

  if(zBuffer == NULL){
	  zBuffer = new float[xResolution*yResolution];
	  zBufferPtr = &(zBuffer[0]);
  }
  if(colorBuffer == NULL){
	  colorBuffer = new float[xResolution*yResolution];
  }
}

double ** doErosion(int& row_length,double zrange,nmm_SimulatedMicroscope* nano_eroder_connection) 
{
  //think this should be 1.0 since didn't scale the z heights when read into the eroder,
  //so shouldn't scale them on the way out either, if did scale on the way in, values
  //would be mult. by xworldratio below, then div. by it for sending out (in invert_zHeight_values)
  double xworldratio = 1.0;//= (-Near+Far)/zrange;
  //changeBufferSize();
  showGrid();

  imageErosionRender();//'scan' inverted scan image

  invert_zHeight_values(xworldratio);//get the zBuffer values, and plug into zHeight
                                     //values inverted from what they normally are

  get_color_buffer_values();

  row_length = xResolution;

  for(int y = 0;y < yResolution; ++y){
	  for(int x = 0;x < xResolution; ++x){
		  zDistanceScaled[y][x] = zDistanceScaled[y][x] /*+ nano_eroder_connection->get_zoffset()*/;
	  }
  }


  return zDistanceScaled;
}

void invert_zHeight_values(double xworldratio){
	GLint PackAlignment;
	glGetIntegerv(GL_PACK_ALIGNMENT,&PackAlignment); 
	glPixelStorei(GL_PACK_ALIGNMENT,1); 

	void* zBufferPtr;


	//check that arrays are correctly size after new size information received from nano
	if(array_x_size != xResolution || array_y_size != yResolution){	
	//if not, delete current arrays, and mark ptrs with NULL so mem. allocation occurs
	//with new sizes
		delete [] zBuffer;
		delete [] colorBuffer;
		for(int j=0; j<array_y_size; j++ ) {
			  if(zHeight != NULL){
				  if (zHeight[j]!= NULL){
					  delete [] zHeight[j];
				  }
			  }
			  if(zDistance != NULL){
				  if (zDistance[j]!= NULL){
					  delete [] zDistance[j];
				  }
			  }
			  if(zDistanceScaled != NULL){
				  if (zDistanceScaled[j]!= NULL){
					  delete [] zDistanceScaled[j];
				  }
			  }
		}
		delete [] zHeight;
		delete [] zDistance;
		delete [] zDistanceScaled;

		zBuffer = NULL;
		colorBuffer = NULL;
		zHeight = NULL;
		zDistance = NULL;
		zDistanceScaled = NULL;
	}
	array_x_size = xResolution;
	array_y_size = yResolution;



	if(zBuffer == NULL){
	  //cout << "creating new zBuffer of size " << xResolution*yResolution << endl;
	  zBuffer = new float[xResolution*yResolution];
	}
	zBufferPtr = &(zBuffer[0]);
	if(colorBuffer == NULL){
	  colorBuffer = new float[xResolution*yResolution];
	}

	// Read (normalized) Z-buffer values from the depth window.
	// Scale them back to correct Z-values and use as 
	// Z-heights in image scan grid.  

	glReadBuffer(GL_BACK);

	if(zHeight == NULL){
	  zHeight = new double*[yResolution];
	  for(int j=0; j<yResolution; j++ ) {
		  zHeight[j] = new double[xResolution];
		  //initialize to NULL so can check and see if we have to create the row array later
		  //(only want to do this once too...)
	  }
	}

	//create new rows for zDistance and zDistanceScaled only if they have not yet been created
	if(zDistance == NULL){
		zDistance = new double*[yResolution];
		for(int j=0; j<yResolution; j++ ) {
			zDistance[j] = new double[xResolution];
			//initialize to NULL so can check and see if we have to create the row array later
			//(only want to do this once too...)
		}
	}
	if(zDistanceScaled == NULL){
		zDistanceScaled = new double*[yResolution];
		for(int j=0; j<yResolution; j++ ) {
			zDistanceScaled[j] = new double[xResolution];
		}
	}

	glReadPixels( 0, 0, xResolution, yResolution, GL_DEPTH_COMPONENT, GL_FLOAT, zBufferPtr );

	//save heights to file 
	cout << "zDistance start" << endl;
	ofstream outstream;
	char filename[100];
	if(matlab)	strcpy(filename,"zDistanceScaledmatlab.er");
	else		strcpy(filename,"zDistanceScaled.er");	
	outstream.open(filename);
	if(matlab)	outstream << "[";
	else		outstream << xResolution << " " << yResolution << endl;;

	int rownumber;
	for(int j=0; j<yResolution; j++ ) {
		rownumber = yResolution-j-1;

		for(int i=0; i<xResolution; i++ ) {
			float zNormalized = zBuffer[rownumber*xResolution + i];
			
			double zDepth = -Far + zNormalized*(-Near + Far);
			
			zHeight[rownumber][i] = zDepth;
			zDistance[j][i] = zNormalized*(-Near + Far);
			zDistanceScaled[j][i] = zDistance[j][i]/xworldratio;

			if(matlab && (i != (xResolution-1)))		print_to_file(outstream,zDistanceScaled[j][i],",");
			else if(matlab && (i == (xResolution-1)))	print_to_file(outstream,zDistanceScaled[j][i],"");	
			//else										print_to_file(outstream,zDistanceScaled[j][i]," ");			
		}
		if(matlab && (j != (yResolution-1)))			outstream << ";" << endl;
		else if(matlab && (j == (yResolution-1)))		outstream << "]" << endl;
		//else											outstream << endl;
	}
	outstream.close();
	cout << "zDistance done" << endl;
}


// display graphics in the depth window.
void  imageErosionRender()  {
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
	glOrtho(  Xmin,   Xmin + (Step * xResolution),
		Ymin,   Ymin + (Step * yResolution),
		Near,   Far   );

	// set modeling matrix to identity
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	lighting();
	setColor( WHITE );

//erosion code goes here

	//start added code

	ofstream outstream;
	char filename[100] = "peakHeight.er";
	int i = 0;

	outstream.open(filename);

	outstream << xResolution << " " << yResolution << endl;;

	//draw inverted tip s.t. tip apex at each inverted z value from the simulator
	int y;
	double zvalue;
	double tipRadius= 0;
	double theta = 0;
	double cone_height = 0;

	for(int j=0; j<yResolution; j++ ) {

		y = yResolution-j-1;//rownumber counts from the top of the grid down
						   //y corresponds to rownumber, x corresponds to columnnumber

		for(int x=0; x<xResolution; x++ ) {

			zvalue = ( 1.0 - MicroscopeHeightArray[y][x]/(-Near + Far) )*(-Near + Far);//invert scan

			switch (tip.type) {
			case SPHERE_TIP :
				glPushMatrix();
				// go down by tip radius
				glTranslatef( 0., 0., -tip.spTip->r);  
				tipRadius = tip.spTip->r; // the radius for the tip

				glPushMatrix();
				glTranslatef(x,y,zvalue);
				drawSphere(2*tip.spTip->r);
				glPopMatrix();

				glPopMatrix();
				break;
			case INV_CONE_SPHERE_TIP :
				glPushMatrix();
				// go down by tip radius
				glTranslatef( 0.0, 0.0, -tip.icsTip->r);  
				tipRadius = tip.icsTip->r; // the radius for the tip

				glPushMatrix();
				theta = tip.icsTip->theta; // theta for the tip
				//default theta, sin(theta), cos(theta): 0.349066rad 0.34202 0.939693
				cone_height = zvalue + tipRadius/sin(theta) /*- tipRadius*/;
				ConeSphere c = ConeSphere(tipRadius, cone_height, theta);
				print_to_file(outstream,c.peakHeight - tipRadius," ");//<< c.sphereHeight << "\t";
				//end output stuff
				glTranslatef(x,y,zvalue);
				c.draw();//draw the tip at every point
				glPopMatrix();

				glPopMatrix();
				break;
			}			
		}
		outstream << endl;
    }
	//end added code

	glFinish();

	glPixelStorei(GL_UNPACK_ALIGNMENT,UnpackAlignment); 

	outstream.close();

}


// Display the image scan grid (a depth image).
void showGrid( void ) {
  int gridColor = GREEN;

  if( (zHeight == NULL) || (array_x_size != xResolution) || (array_y_size != yResolution) ){
	  
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
	glOrtho(  Xmin,   Xmin + (Step * xResolution),
		Ymin,   Ymin + (Step * yResolution),
		Near,   Far   );


	// set modeling matrix to identity
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	lighting();
	setColor( WHITE );

  
	  get_z_buffer_values(1.0);
	  get_color_buffer_values();
  }//will display a zero height plane

  // Display depth image surface.  
  // The variable "gridStyle" controls which of several visualizations
  // of the surface are used.
  //  shadingModel = GL_SMOOTH;
  setColor( gridColor );

  lighting();
  int rownumber;
  for(int j=0; j<yResolution; j++ ) {
	rownumber = yResolution-j-1;//rownumber counts from the top of the grid down

    for( int i=0; i<xResolution-1; i++ ) {
      double x = i * Step  +  Xmin;
      double y = j * Step  +  Ymin;
      double dx = Step;
      double dy = Step;

      double x1,x2,x3,x4,y1,y2,y3,y4,z1,z2,z3,z4;

	  if(i<xResolution-1 && j<yResolution-1){
		  // Get the the 4 (x,y,z) coords on the corners of this grid cell.
		  // Show objects above the surface only
		   x1 = x;      y1 = y;      z1 = zHeight[j][i];
		   x2 = x+dx;   y2 = y;      z2 = zHeight[j][i+1];
		   x3 = x+dx;   y3 = y+dy;   z3 = zHeight[j+1][i+1];
		   x4 = x;      y4 = y+dy;   z4 = zHeight[j+1][i];
	  }
	  else if(i ==xResolution-1 && j<yResolution-1){
		  // Get the the 4 (x,y,z) coords on the corners of this grid cell.
		  // Show objects above the surface only
		   x1 = x;      y1 = y;      z1 = zHeight[j  ][i-1  ];
		   x2 = x+dx;   y2 = y;      z2 = zHeight[j][i];
		   x3 = x+dx;   y3 = y+dy;   z3 = zHeight[j+1][i];
		   x4 = x;      y4 = y+dy;   z4 = zHeight[j+1][i-1];		
	  }
	  else if(i < xResolution-1 && j==yResolution-1){
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
      double gcol1 = colorBuffer[rownumber*xResolution + i];
      //      double gcol2 = colorBuffer[j*Resolution + i+1];
      //      double gcol3 = colorBuffer[(j+1)*Resolution + i+1];
      //      double gcol4 = colorBuffer[(j+1)*Resolution + i];

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





