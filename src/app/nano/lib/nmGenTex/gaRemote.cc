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

#include "input.h"

#include "gaEngine_Remote.h"

extern "C" void glEnableClientState( GLenum array );

static textures = 0;
gaEngine_Remote *gaEngineClient;

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
  glPushMatrix();
  glRasterPos2i( 0, 0 );
  if ( textures )
    glDrawPixels( 512, 512, GL_RGB, GL_FLOAT, gaEngineClient->data[0] );
  glPopMatrix();
  
  glutSwapBuffers();
}


void my_idle( void ) {
  gaEngineClient->mainloop();
  glutPostRedisplay();
}


void init( void ) {
  glClearColor( 0.0, 0.0, 0.0, 0.0 );
  glShadeModel( GL_FLAT );
  glViewport( 0, 0, (GLsizei)DATA_WIDTH, (GLsizei)DATA_HEIGHT );
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  glOrtho( 0, DATA_WIDTH, 0, DATA_HEIGHT, -1, 1);
  glMatrixMode( GL_MODELVIEW);
  glLoadIdentity();
}

void keyboard( unsigned char key, int x, int y ) {
  switch( key ) {
  case 'q': {
    int i;
    
    delete gaEngineClient;

    exit( 0 );
    break;
  }
  default: break;
  }
}

void mouse( int button, int state, int x, int y ) {
  static int pressed = 0;
  switch( button ) {
  case GLUT_LEFT_BUTTON: {
    break;
  }
  case GLUT_MIDDLE_BUTTON: {
    break;
  }
  case GLUT_RIGHT_BUTTON: {
    break;
  }
  default: break;
  }
}

int wait_for_textures( void *p ) {
  textures = 1;
  glutPostRedisplay();
  return 1;
}

int send_data( void *p ) {

  gaEngineClient->number_of_variables( NUMBER_OF_VARIABLES );
  gaEngineClient->mainloop();
  gaEngineClient->variableList( NUMBER_OF_VARIABLES, VARIABLE_LIST );
  gaEngineClient->mainloop();
  gaEngineClient->dimensions( NUMBER_OF_INPUTS, DATA_WIDTH, DATA_HEIGHT );
  gaEngineClient->mainloop();
  for ( int i = 0; i < NUMBER_OF_INPUTS; i++ )
    for ( int j = 0; j < DATA_HEIGHT; j++ ) {
      gaEngineClient->dataSet( j, DATA_WIDTH, INPUTS[i] );
      gaEngineClient->mainloop();
    }
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

//   int pid = fork();
//   if ( pid == -1 ) {
//     cerr <<"Cannot spawn server" <<endl;
//     exit( 0 );
//   }
//   if ( pid == 0 ) {
//     execl( "./pc_linux/gaServer", "gaServer", NULL );
//   }
//   else {

    if ( argc == 1 ) {
      char file_name[20];
      cout <<"Enter the data input file: ";
      cin  >> file_name;
      read_inputs( file_name );
    }
    else {
      read_inputs( argv[1] );
    }
    

    // this may be broken.  See --baseport option in nano (microscape.c),
    // which stores the port number in WellKnownPorts
    gaEngineClient = new gaEngine_Remote
         (new vrpn_Synchronized_Connection (gaEngine::defaultPort));
    gaEngineClient->registerEvaluationCompleteHandler
         (wait_for_textures, NULL);
    gaEngineClient->registerConnectionCompleteHandler( send_data, NULL );
    
   
    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB  );
    glutInitWindowSize( DATA_WIDTH, DATA_HEIGHT );
    glutCreateWindow( argv[0] );
    
    init();
    
    glutDisplayFunc( display );
    glutIdleFunc( my_idle );
    //     glutReshapeFunc( reshape );
    glutMouseFunc( mouse );
    glutKeyboardFunc( keyboard );
    
    glutMainLoop();
//   }
  return 0;
}
