// OpenGL Tutorial
// Polygons_List.c

/*************************************************************************
This example is a modification Polygons.c to use display lists.
*************************************************************************/

// gcc -o Polygons_List  Polygons_List.c -lX11 -lMesaGL -lMesaGLU -lMesatk -lm

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <GL/glut.h>

#define SPHERE 1

void reshape(int width, int height) {

        // Set the new viewport size
        glViewport(0, 0, (GLint)width, (GLint)height);

        // Choose the projection matrix to be the matrix 
        // manipulated by the following calls
        glMatrixMode(GL_PROJECTION);

        // Set the projection matrix to be the identity matrix
        glLoadIdentity();

        // Define the dimensions of the Orthographic Viewing Volume
        glOrtho(-80.0, 80.0, -80.0, 80.0, -80.0, 80.0);

        // Choose the modelview matrix to be the matrix
        // manipulated by further calls
        glMatrixMode(GL_MODELVIEW);
}

static GLUquadricObj* qobj;

void draw(void) {
  for (int i=0;i<10000;i++) {
        // Clear the RGB buffer and the depth buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Set the modelview matrix to be the identity matrix
        glLoadIdentity();
        // Translate and rotate the object
	glTranslatef((i%30),0,0);
	glRotatef(90, 1, 0.0, 0.0);

	glScalef(1,1,i%5);
        // Draw cube
        glCallList(SPHERE);
  }
  exit(0);

  // Flush the buffer to force drawing of all objects thus far
  glFlush();
}


void draw2() {
  qobj = gluNewQuadric();
  gluQuadricDrawStyle( qobj, GLU_FILL);
  gluQuadricNormals( qobj, GLU_FLAT );  
  

  for (int i=0;i<10000;i++) {

        // Clear the RGB buffer and the depth buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Set the modelview matrix to be the identity matrix
        glLoadIdentity();
        // Translate and rotate the object
	glTranslatef((i%30),0,0);
	glRotatef(90, 1, 0.0, 0.0);
	gluCylinder( qobj, 30, 20, 10*(i%5), 50, 50);
  }
  exit(0);

  // Flush the buffer to force drawing of all objects thus far
  glFlush();

}

void make_sphere() {
  qobj = gluNewQuadric();
  gluQuadricDrawStyle( qobj, GLU_FILL);
  gluQuadricNormals( qobj, GLU_FLAT );  
  
  glNewList(SPHERE, GL_COMPILE);
  gluCylinder( qobj, 30, 20, 10, 50, 50);
  glEndList();
}

#ifdef _WIN32
extern "C" void key_down(unsigned char key, int, int)
#else
GLenum key_down(int key, GLenum state)
#endif
{
        if ((key == 'q') || (key == 'Q'))
	  exit(0);
}



void main(int argc, char **argv) {

        // Set top left corner of window to be at location (0, 0)
        // Set the window size to be 500x500 pixels
  //        tkInitPosition(0, 0, 500, 500);

	glutInitWindowSize(500, 500);
	glutInitWindowPosition( 0, 0 );

        // Initialize the RGB and Depth buffers
	//        tkInitDisplayMode(TK_RGB | TK_DEPTH);
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH);

        // Open a window, name it "Polygons_List"
	//        if (tkInitWindow("Polygons_List") == GL_FALSE) {
	//                tkQuit();
	//        }
	glutCreateWindow( "Poly list" );

        // Set the clear color to black
        glClearColor(0.0, 0.0, 0.0, 0.0);

        // Set the shading model
        glShadeModel(GL_FLAT);

        // Set the polygon mode to fill
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        // Enable depth testing for hidden line removal
        glEnable(GL_DEPTH_TEST);

        // Create the display lists
	make_sphere();

        // Assign reshape() to be the function called whenever
        // an expose event occurs
	//        tkExposeFunc(reshape);

        // Assign reshape() to be the function called whenever 
        // a reshape event occurs
	//        tkReshapeFunc(reshape);
	glutReshapeFunc(reshape);

        // Assign key_down() to be the function called whenever
        // a key is pressed
	//	tkKeyDownFunc(key_down);
	glutKeyboardFunc(key_down);

        // Assign draw() to be the function called whenever a display
        // event occurs, generally after a resize or expose event
	//        tkDisplayFunc(draw);
        glutDisplayFunc(draw);

        // Pass program control to tk's event handling code
        // In other words, loop forever
	//        tkExec();
	glutMainLoop();
}
