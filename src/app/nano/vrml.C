/*===3rdtech===
  Copyright (c) 2000 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
/** \file vrml.C
 *					adapted to VRML by Will Allen
 * 	This file contains the functions needed to write out a VRML file
 * containing a description of the surface that is currently mapped to
 * the display.  This includes normal and color information, and eventually
 * will include texture information and modify lines and labels.
 *	The function write_to_vrml() is the operative one.
 */

#include <iostream>
#include <fstream>
using namespace std;
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

/*
 * windows.h must be included before gl.h if 
 * compiling under windows.
 */
#ifdef _WIN32
#include <windows.h>
#endif
#include	<GL/gl.h>

#include <nmb_ColorMap.h>
#include <BCPlane.h>
#include <nmb_PlaneSelection.h>

//#include	"spm_gl.h"
#include        "microscape.h"
#include	<Tcl_Linkvar.h>

#if (!defined(X) || !defined(Y) || !defined(Z))
#define	X	(0)
#define	Y	(1)
#define	Z	(2)
#endif

//static int vrml_shiny = 55;	// Specular shininess coefficient
//static GLenum	vrml_texture_mode = GL_FALSE;	// Texture mode for surface

//---------------------------------------------------------------------------
// Vector utility routines.

static inline	void	vector_cross(GLfloat a[3], GLfloat b[3], GLfloat c[3])
{
    c[0] = a[1]*b[2] - b[1]*a[2];
    c[1] = -(a[0]*b[2] - b[0]*a[2]);
    c[2] = a[0]*b[1] - b[0]*a[1];
}

static inline	void	vector_add(GLfloat a[3], GLfloat b[3], GLfloat c[3])
{
    c[0] = a[0] + b[0];     
    c[1] = a[1] + b[1];     
    c[2] = a[2] + b[2];
}

static inline	void	vector_normalize(GLfloat a[3])
{
    double mag;   

    mag = sqrt(a[0] * a[0] + a[1]*a[1] + a[2]*a[2]); 

    if (mag == 0.0) {
        fprintf(stderr,"pg_vector_normalize:  vector has zero magnitude\n");
        a[0] = a[1] = a[2] = 1.0/sqrt(3.0);
    } else {
        mag = 1.0/mag;
        a[0] *= mag;
        a[1] *= mag;
        a[2] *= mag;
    }
}


/**	This routine finds the normal to the surface at the given grid
 * point.  It does this by averaging the normals with the four 4-connected
 * points in the grid (one away in x or y.)
 *	This routine takes into account the current stride in x and y
 * between tesselated points and grid points.
 *
 * x and y are the indices of the point.  dx and dy are the distances between
 * points in the X and Y directions.  I think dz is the Z scale.
 *
 *	This routine returns 0 on success and -1 on failure.
 */

int vrml_compute_plane_normal(const BCPlane *plane, int x,int y,
                              double dx,double dy,double dz, GLfloat Normal[3])
{
	static	GLfloat	X_norm[3] = {1.0, 0.0, 0.0};
	static	GLfloat	Y_norm[3] = {0.0, 1.0, 0.0};
	static	GLfloat	NX_norm[3] = {-1.0, 0.0, 0.0};
	static	GLfloat	NY_norm[3] = {0.0, -1.0, 0.0};
	GLfloat		diff_vec[3];
	GLfloat		local_norm[3];
	int	i;

	// Valid points must be within the grid and lie exactly on a stride
	// point.
	if ( (x < 0) || (x > plane->numX()-1) ||
	     (y < 0) || (y > plane->numY()-1) ||
	     (x % stride) || (y % stride) ){
		return(-1);
	}

	/* Initially, clear the normal */
	for (i = 0; i < 3; i++) {
		Normal[i] = 0.0;
	}

	// Find the normal with stride more in x, if it is within the grid

	if ( (x+stride) < plane->numX()) {
		diff_vec[X] = dx * stride;
		diff_vec[Y] = 0;
		diff_vec[Z] = dz * (float) (plane->valueInWorld(x+stride,y) -
					    plane->valueInWorld(x, y));
		vector_cross(diff_vec,Y_norm, local_norm);
		vector_add(local_norm,Normal, Normal);
	}

	// Find the normal with stride more in y, if it is within the grid

	if ( (y+stride) < plane->numY()) {
		diff_vec[X] = 0;
		diff_vec[Y] = dy * stride;
		diff_vec[Z] = (float) (dz * (plane->valueInWorld(x,y+stride) -
					     plane->valueInWorld(x,y)));
		vector_cross(diff_vec,NX_norm, local_norm);
		vector_add(local_norm,Normal, Normal);
	}
 
	// Find the normal with stride less in x, if it is within the grid

	if ( (x-stride) >= 0) {
		diff_vec[X] = -dx * stride;
		diff_vec[Y] = 0;
		diff_vec[Z] = (float) (dz * (plane->valueInWorld(x-stride,y) -
					     plane->valueInWorld(x, y)));
		vector_cross(diff_vec,NY_norm, local_norm);
		vector_add(local_norm,Normal, Normal);
	}

	// Find the normal with stride less in y, if it is within the grid

	if ( (y-stride) >= 0) {
		diff_vec[X] = 0;
		diff_vec[Y] = -dy * stride;
		diff_vec[Z] = (float) (dz * (plane->valueInWorld(x,y-stride) -
					     plane->valueInWorld(x, y)));
		vector_cross(diff_vec,X_norm, local_norm);
		vector_add(local_norm,Normal, Normal);
	}

	/* Normalize the normal */

	vector_normalize(Normal);
	return(0);
}


static int	fout_vrml_points(nmb_PlaneSelection planes,
		ofstream &fout, int stride)
{
	int	x,y;
	// Make sure we found a height plane
	if (planes.height == NULL) {
	    fprintf(stderr, "spm_y_strip: could not get grid!\n");
	    return -1;
	}

	for (x = 0; x < planes.height->numX(); x += stride) {// right->left
	  for (y = 0; y < planes.height->numY(); y += stride) {	//bottom->top
   	     /* Put the vertex at the correct location */
             if ((y+stride >= planes.height->numY())
		  &&(x+stride >= planes.height->numX())) { //at bottom of list
                fout<<"\n\t"<< (float)(planes.height->xInWorld(x))<<' ';
        	fout<< (float) planes.height->yInWorld(y)<<' ';
       	 	fout<< (float) planes.height->valueInWorld(x,y);
		//no comma at bottom
	     } else{			//not at bottom
        	fout<<"\n\t"<< (float) planes.height->xInWorld(x)<<' ';
        	fout<< (float) planes.height->yInWorld(y)<<' ';
        	fout<< (float) planes.height->valueInWorld(x,y)<<',';
	     }
	  }
	}
	
	return(0);
}


static int	fout_vrml_normal_dimension(nmb_PlaneSelection planes,
		ofstream &fout, int stride)
{
	int x,y;
	GLfloat Normal[3];

	double	min_x = planes.height->minX(), max_x = planes.height->maxX();
	double	min_y = planes.height->minY(), max_y = planes.height->maxY();

	for (x = 0; x <  planes.height->numX(); x += stride) {  // right->left
	  for (y = 0; y < planes.height->numY(); y += stride) {	// bottom->top

	     /* Find the normal for the vertex */
             if (vrml_compute_plane_normal(planes.height, x,y,
			(float) ((max_x-min_x)/(planes.height->numX())),  
			(float) ((max_y-min_y)/(planes.height->numY())),
			(float) 1.0,
			Normal)) {
	         fprintf(stderr,"describe_vrml_vertex(): Can't find normal!\n");
                 return -1;
	     }

	     //write out to file
             if ((y + stride >= planes.height->numY())&&
		 (x + stride >= planes.height->numX())) { //at bottom of list

                fout<<"\n\t"<< Normal[0]<<' ';
        	fout<< Normal[1]<<' ';
       	 	fout<< Normal[2];			//no comma at bottom
             } else {					//not at bottom
                fout<<"\n\t"<< Normal[0]<<' ';
        	fout<< Normal[1]<<' ';
       	 	fout<< Normal[2]<<',';
	     }
	  }
	}
	return 0;
}



static int fout_vrml_color( nmb_PlaneSelection planes,
			    GLdouble /*minColor*/[3], GLdouble /*maxColor[3]*/[3],
			    ofstream &fout, int stride )
{
    int x,y;

    if (planes.color == NULL) {	// No color plane specified
	return -1;
    }

    for (x = 0; x < planes.height->numX(); x += stride) {// right->left
      for (y = 0; y < planes.height->numY(); y += stride) {	// bottom->top
        GLfloat	Color[3] = {0.5,0.5,0.5};

	if ((y + stride >= planes.height->numY())&&
	    (x + stride >= planes.height->numX())) {
						//at bottom of list
                fout<<"\n\t"<< Color[0]<<' ';
        	fout<< Color[1]<<' ';
       	 	fout<< Color[2];		//no comma at bottom
	} else {			//not at bottom
                fout<<"\n\t"<< Color[0]<<' ';
        	fout<< Color[1]<<' ';
       	 	fout<< Color[2]<<',';
	}
     }
   }
   return 0;      
}


int     write_to_vrml_file (const char * filename, nmb_PlaneSelection planes,
                GLdouble minColor [3], GLdouble maxColor [3]) {

        cout<<"Writing to VRML file "<<filename<<"...\n";
	ofstream fout(filename);	//open filestream
        fout<<"#VRML V1.0 ascii\n";	//VRML header
        fout<<"Coordinate3{  \n\tpoint [ ";
        
        // Describe the point list (using part of describe_vrml_vertex for each)
	
	fout_vrml_points(planes, fout, stride);
	 
	fout<<"\t]\t}\n# end coord3\n"; //end coord3


        // Describe the normal list (again part of describe)
        
	fout<<"Normal  { \n\tvector[\n";	
	fout_vrml_normal_dimension(planes, fout, stride);
        fout<<"\t]\n}\nNormalBinding {\n"
     	    <<"value PER_VERTEX_INDEXED\n}\n";


        // Describe colors

	fout<<"Material  {\n\tspecularColor [ 0.7 0.7 0.7 ]\n\tdiffuseColor[\n";
	fout_vrml_color(planes, minColor, maxColor, fout, stride);
	fout<<"\t]\n}MaterialBinding {\n"
	<<"value PER_VERTEX_INDEXED\n}\n";


        
        // Indices using vrml_y_strip mostly
	// connect the vertices, and make faces     

	/*The triangle strips are mapped from the grid into the graphics space
	 *as follows:
	 *                                                                     
	 *          +-----+                                   
	 *         7|    /|6                                                 
	 *          |   / |                                                 
	 *          |  /  |                                                    
	 *          | /   |                                                     
	 *          |/    |                                                  
	 *          +-----+           (Clockwise face is forwards)
	 *         5|    /|4                                                 
	 *          |   / |           (REVERSE THIS -- counterclockwise fwd)
	 *          |  /  |                                                    
	 *          | /   |                                                     
	 *          |/    |                                                  
	 *          +-----+                                                 
	 *    ^    3|    /|2                                                 
	 *    |     |   / |                                                 
	 *    |     |  /  |                                                    
	 *    |     | /   |                                                     
	 *    |     |/    |                                                  
	 *    |     +-----+                                                 
	 *    Y    1       0                                           
	 *     X-------->                                                      
	 */                                                                     

	fout<<"IndexedFaceSet{\n";
	fout<<"\tcoordIndex[\n";

	int x,y, dx;

	// Max x,y is the largest index that should appear for
	// any vertex.  Recall that the indices here are for points
	// listed above, which have already taken stride into account.
	int maxx = (planes.height->numX() -1) / stride;
	int maxy = (planes.height->numY() -1) / stride;
	int numy = maxy+1;
//XXX This section seems to work fine.  The points also seem to work
// fine.  The normals also seem to work.  The colors still seem messed up.
// Try setting diffuse and ambient to the same?

	// x goes only through one lower than the last, since the last
	// row is covered by the strip that has one lower index.
	for (x = 0; x <= maxx-1; x += 1) {
          dx=numy*x;		//x changes by numy for each new column

	  // y goes to maxy-2 because we treat the last one specially, to
	  // handle the fact that all but the last have separators.
	  for (y = 0; y <= maxy-2; y += 1) {// top->bottom
             fout<<y+dx<<", "<<y+dx+numy<<", "<<y+dx+numy+1<<", -1,\n";
	     fout<<y+dx+numy+1<<", "<<y+dx+1<<", "<<y+dx<<", -1, \n";
          }//end y for

          // put the last triangles on the y-strip
          // On all except the last, put a -1 separator: last put \n
	  if(x == (maxx-1)) {  //at end
             fout<<y+dx<<", "<<y+dx+numy<<", "<<y+dx+numy+1<<", -1,\n";
	     fout<<y+dx+numy+1<<", "<<y+dx+1<<", "<<y+dx<<" \n";
	  } else {
             fout<<y+dx<<", "<<y+dx+numy<<", "<<y+dx+numy+1<<", -1,\n";
	     fout<<y+dx+numy+1<<", "<<y+dx+1<<", "<<y+dx<<", -1, \n";
	  }

        }//end x for

	fout<<"\t\t]\n\t}\n";
	//close vrml file
        printf("done!\n");
	return 0;
}


