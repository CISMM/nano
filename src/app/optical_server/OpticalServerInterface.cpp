
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <glui.h>
#include <vrpn_Connection.h>

#include "OpticalServerInterface.h"

OpticalServerInterface* OpticalServerInterface::instance = NULL;
bool OpticalServerInterface::interfaceShutdown = false;

void OpticalServerInterface_myGlutRedisplay( )
{
	//printf( "display\n" );
	OpticalServerInterface* iface = OpticalServerInterface::getInterface( );

	glutSetWindow( iface->image_window );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	if( iface->lastImage != NULL )
	{
		glutSetWindow( iface->image_window );
		glRasterPos2i( 0, 0 );
		glDrawPixels( iface->lastImageWidth, iface->lastImageHeight, 
					  GL_LUMINANCE, GL_UNSIGNED_BYTE, 
					  iface->lastImage );
	}

}


void OpticalServerInterface_myGlutKeyboard( unsigned char key, int x, int y )
{
	OpticalServerInterface* iface = OpticalServerInterface::getInterface( );

}


void OpticalServerInterface_myGlutSpecial( int, int, int )
{
	OpticalServerInterface* iface = OpticalServerInterface::getInterface( );

}


void OpticalServerInterface_myGlutMouse( int button, int state, int x, int y )
{
	OpticalServerInterface* iface = OpticalServerInterface::getInterface( );

}


void OpticalServerInterface_myGlutMotion( int x, int y )
{
	OpticalServerInterface* iface = OpticalServerInterface::getInterface( );
	//glutSetWindow( iface->image_window );
	//glutPostRedisplay( );

}


void OpticalServerInterface_myGlutReshape( int w, int h )
{
	OpticalServerInterface* iface = OpticalServerInterface::getInterface( );
	GLUI_Master.auto_set_viewport( );

}


void OpticalServerInterface_myGlutIdle( )
{
	OpticalServerInterface* iface = OpticalServerInterface::getInterface( );

}


DWORD WINAPI OpticalServerInterface_mainloop( LPVOID lpParameter )
{
	printf( "OpticalServerInterface_mainloop\n" );
	OpticalServerInterface* iface = OpticalServerInterface::getInterface( );
	iface->image_window = glutCreateWindow( "Optical Microscope Server" );
	glutDisplayFunc( OpticalServerInterface_myGlutRedisplay );
	glutMotionFunc( OpticalServerInterface_myGlutMotion );
	GLUI_Master.set_glutKeyboardFunc( OpticalServerInterface_myGlutKeyboard );
	GLUI_Master.set_glutSpecialFunc( OpticalServerInterface_myGlutSpecial );
	GLUI_Master.set_glutMouseFunc( OpticalServerInterface_myGlutMouse );
	GLUI_Master.set_glutReshapeFunc( OpticalServerInterface_myGlutReshape );
	GLUI_Master.set_glutIdleFunc( OpticalServerInterface_myGlutIdle );
	
	iface->glui_window = GLUI_Master.create_glui( "Optical Microscope Controls" );

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D( 0.0, 1.0, 0.0, 1.0 );

	iface->glui_window->set_main_gfx_window( iface->image_window );
	GLUI_Master.auto_set_viewport( );

	glClearColor( 0.0, 0.0, 0.0, 0.0 );

	iface->threadReady = true;

	glutMainLoop( );

	return 0;
}


OpticalServerInterface::
OpticalServerInterface( )
: lastImage( NULL ),
  lastImageWidth( 0 ),
  lastImageHeight( 0 ),
  threadReady( false )
{
	LPDWORD lpThreadID = NULL;
	ifaceThread = CreateThread( NULL, 0, OpticalServerInterface_mainloop, NULL, 0, lpThreadID );
}



OpticalServerInterface::
~OpticalServerInterface( )
{
	// ExitThread( 0 );
	OpticalServerInterface::interfaceShutdown = true;

}


OpticalServerInterface* OpticalServerInterface::
getInterface( )
{
	if( OpticalServerInterface::interfaceShutdown == true )
	{  return NULL;  }

	if( OpticalServerInterface::instance == NULL ) // create one
	{
		OpticalServerInterface::instance = new OpticalServerInterface( );
		while( OpticalServerInterface::instance->threadReady == false )
		{
			vrpn_SleepMsecs( 10 );
		}
	}
	return OpticalServerInterface::instance;
}


void OpticalServerInterface::
setImage( vrpn_uint8* buffer, int width, int height )
{
	this->lastImage = buffer;
	this->lastImageWidth = width;
	this->lastImageHeight = height;
	//glutSetWindow( this->image_window );
	glutPostRedisplay( );
}

