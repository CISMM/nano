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
#include <GL/glut_UNC.h>
#include "Vec3d.h"
#include "3Dobject.h"
#include "ConeSphere.h"
#include "Tips.h"
#include "sim.h"
#include "defns.h"
#include "lightcol.h"
#include "scan.h"

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
#endif

void drawSphere( double diameter) {
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

#if DISP_LIST

// display list for a cylinder
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
#endif

void drawCylinder( double diameter, double height) {
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

#if DISP_LIST
void make_cone_sphere(InvConeSphereTip ics) {
  static GLUquadricObj* qobj;

  //  printf("Building ConeSphere display list (tesselation %d)\n",tesselation);

  static int first_time=1;
  if (first_time) {
    first_time=0;
    qobj = gluNewQuadric();
    gluQuadricDrawStyle( qobj, GLU_FILL);
    gluQuadricNormals( qobj, GLU_FLAT );
  }

  // here we consider the AFM of a sphere of 0 radius with a tip of radius 1.
  double theta = ics.theta; // theta for the tip
  
  double bignum = 200; // some big no for now
  
  double cone_height = bignum + (1.)/sin(theta);
  ConeSphere c = ConeSphere(1., cone_height, theta);
  
  // Create a display list for a sphere
  glNewList(CONE_SPHERE_LIST, GL_COMPILE);
  gluSphere( qobj, 1., tesselation, tesselation);
  glPushMatrix();
  glTranslatef(0,0,-bignum);
  gluCylinder( qobj, c.cr, c.topRadius, c.topHeight, tesselation, tesselation);
  glPopMatrix();
  glEndList();

  glFlush();
  glFinish();

}
#endif

/* draw Cone Sphere code is part of the ConeSphere class */


// Draw the objects.
void drawObjects( void ) {
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

    // draw the object
    ob[i]->draw();


    // to be able to select individual side of a triangle, we do this
    //    if ((buttonpress == RIGHT_BUTTON) && (glutGetWindow() == mainWindowID)) {
    if ((buttonpress == RIGHT_BUTTON)) {
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

void drawFrame( void ) {
  // Setup OpenGL state.
  glClearDepth(1.0);
  //  glClearColor(0.5, 0.5, 0.5, 1);
  if (uncertainty_mode) {
    glClearColor(0., 0., 0., 1);
  }
  else {
    glClearColor(0.5, 0.5, 0.5, 1);
  }

  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  glEnable(GL_DEPTH_TEST);

  lighting();
  glPointSize( 2. );    // se p51 Woo 3rd ed
  glLineWidth( 2. );

  if (draw_objects) {
    drawObjects(); // Draw objects
  }

  if (afm_scan!=NO_AFM) {
    // Draw the image scan grid.
    showGrid();
  }
}
