#include <iostream.h>
#include <stdio.h>
#include <fstream.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <GL/glut.h>
#ifndef sgi_irix
#include <new.h>
#endif

#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <vrpn_Connection.h>

#include "gaFunctions.h"
#include "gaEngine_Implementation.h"

extern "C" void glEnableClientState( GLenum array );

static int TOTAL_WIDTH;
static int TOTAL_HEIGHT;
static int width;
static int height;
#define NUMBER_FILES 12
#define TEX_MAX_WIDTH 512
#define TEX_MAX_HEIGHT 512

static int p1 = 12, p2 = 12;
static textures = 0;
gaEngine_Implementation *gaEngineImplementation;

void check_gl_error() {
  GLenum e;
  e = glGetError();
  if ( e == GL_NO_ERROR )
    cout <<"no error " <<endl;
  else if ( e == GL_INVALID_ENUM )
    cout <<"invalid enum " <<endl;
  else if ( e == GL_INVALID_VALUE )
    cout <<"invalid value " <<endl;
  else if ( e == GL_INVALID_OPERATION )
    cout <<"invalid operation " <<endl;
  else if ( e == GL_STACK_OVERFLOW )
    cout <<"stack overflow " <<endl;
  else if ( e == GL_STACK_UNDERFLOW )
    cout <<"stack underflow " <<endl;
  else if ( e == GL_OUT_OF_MEMORY )
    cout <<"out of mem " <<endl;
}

void display( void ) {
  glClear( GL_COLOR_BUFFER_BIT );

  int f = 0;
  for ( int i = 0; i < 4; i++ )
    for ( int j = 0; j < 3; j++ ) {
      glPushMatrix();
      glRasterPos2i( j * (width + 10) + 10, i * (height + 10) + 10 );
      if ( textures )
 	glDrawPixels( 512, 512, GL_RGB, GL_FLOAT, gaEngineImplementation->data[f++] );
      glPopMatrix();
    }
  
  glutSwapBuffers();
}


void my_idle( void ) {
  gaEngineImplementation->mainloop();
  glutPostRedisplay();
}


void init( void ) {
  glClearColor( 0.0, 0.0, 0.0, 0.0 );
  glShadeModel( GL_FLAT );
  glViewport( 0, 0, (GLsizei)TOTAL_WIDTH, (GLsizei)TOTAL_HEIGHT );
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  glOrtho( 0, TOTAL_WIDTH, 0, TOTAL_HEIGHT, -1, 1);
  glMatrixMode( GL_MODELVIEW);
  glLoadIdentity();
}


void MenuHandler( int entry )
{
  switch( entry ) {
  case 1 : // crossover best of all
  gaEngineImplementation->evaluate();
    break;
  case 2 : // crossover best of images
  gaEngineImplementation->evaluate2();
  case 3 : // replace all with random
  gaEngineImplementation->random();
    break;
  } // end switch 
  glutPostRedisplay();
  return;
}


void keyboard( unsigned char key, int x, int y ) {
  x; y; // not used in function!
  switch( key ) {
  case 'q': {
    delete gaEngineImplementation;
    exit( 0 );
    break;
  }
  default: break;
  }
}

void mouse( int button, int state, int x, int y ) {
  static int pressed = 0;
  switch( button ) {
  case GLUT_LEFT_BUTTON:
    if ( state == GLUT_DOWN ) {
      pressed++;
      if ( pressed == 1 ) {
	p1 = (x-10)/(width + 10) + 3 * (((TOTAL_HEIGHT-y)-10)/(height+ 10));
      }
      else if ( pressed == 2 ) {
	pressed = 0;
	p2 = (x-10)/(width+10) + 3 * (((TOTAL_HEIGHT-y)-10)/(height+10));
	if ( (p1 < NUMBER_FILES) && (p2 < NUMBER_FILES) ) {

	  gaEngineImplementation->selectGenomes( p1, p2 );
	  gaEngineImplementation->reevaluateDataset( );
	  gaEngineImplementation->mainloop( );
	  glutPostRedisplay();
	}
      }
    }
    break;
  case GLUT_MIDDLE_BUTTON: {
    if ( state == GLUT_DOWN ) {
      int p = (x-10)/(width+10) + 3 * (((TOTAL_HEIGHT-y)-10)/(height+10));

      for ( int j = 0; j < 512; j++ ) {
	gaEngineImplementation->texture( p, j );
	gaEngineImplementation->mainloop();
      }

    }
    break;
  }
  case GLUT_RIGHT_BUTTON: {
    break;
  }
  default: break;
  }
}

int wait_for_textures( void *p ) {
  p; // not used in function...
  static int first = 1;

  textures = 1;

  if ( first ) {
    int argc = 1;
    char **argv = new char*[1];
    argv[0] = new char[20];
    strcpy( argv[0], "Genetic" );

    width = gaEngineImplementation->width;
    height = gaEngineImplementation->height;
    TOTAL_WIDTH = 10 + (width + 10) * 3;
    TOTAL_HEIGHT = 10 + (height + 10) * 4;
    first = 0;
    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB  );
    glutInitWindowSize( TOTAL_WIDTH, TOTAL_HEIGHT );
    glutCreateWindow( "Genetic Texture Generator" );
    
    init();
    
    glutDisplayFunc( display );
    glutIdleFunc( my_idle );
    glutMouseFunc( mouse );
    glutKeyboardFunc( keyboard );

    /* Use GLUT's menu functions to set up a few options and a quit feature */
    int menuID = glutCreateMenu( MenuHandler );
    glutAddMenuEntry( "Crossover Best of All", 1 );
    glutAddMenuEntry( "Crossover Best of Images", 2 );
    glutAddMenuEntry( "Replace All with Random", 3 );
    glutAttachMenu( GLUT_RIGHT_BUTTON );
    
    glutMainLoop();
  }
  glutPostRedisplay();
  return 1;
}


#ifndef sgi_irix
// new_handler from B. Stroustrup, "The C++ Programming Language", p. 99
static void out_of_store() {
  cerr <<"operator new failed: out of store" <<endl;
  exit(1);
}
#endif

int main( int argc, char *argv[] ) {
#ifndef sgi_irix
//   cout <<"not sgi" <<endl;
  set_new_handler(&out_of_store);
#endif

  char name [80];
  char port [10];

  if ( argc == 1 )
    sprintf(name, "nano");
  else
    strcpy(name, argv[1]);
  if (!strchr(name, ':')) {  // append a port specifier if none given
    strcat(name, ":");
    sprintf(port, "%d", gaEngine::defaultPort);
    strcat(name, port);
  }

  vrpn_Connection * connection;
  connection = vrpn_get_connection_by_name(name);

  if ( connection ) {
    gaEngineImplementation = new gaEngine_Implementation( connection );
    gaEngineImplementation->registerEvaluationCompleteHandler(
					      wait_for_textures, NULL );
    while( 1 )
      gaEngineImplementation->mainloop();
  }
  else
    cerr <<"Unable to connect" <<endl;
  return 0;
}
