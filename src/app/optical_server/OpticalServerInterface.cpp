
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <glui.h>

#include "OpticalServerInterface.h"

OpticalServerInterface* OpticalServerInterface::instance = NULL;

void OpticalServerInterface_myGlutRedisplay( )
{
	OpticalServerInterface* iface = OpticalServerInterface::getInterface( );
	printf( "redisplay\n" );

	glClear( GL_COLOR_BUFFER_BIT );
	glColor3f( 1.0f, 1.0f, 1.0f );
	glutWireSphere( 0.5, 10, 10 );

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
	iface->main_window = glutCreateWindow( "Optical Microscope Server" );
	glutDisplayFunc( OpticalServerInterface_myGlutRedisplay );
	glutMotionFunc( OpticalServerInterface_myGlutMotion );
	GLUI_Master.set_glutKeyboardFunc( OpticalServerInterface_myGlutKeyboard );
	GLUI_Master.set_glutSpecialFunc( OpticalServerInterface_myGlutSpecial );
	GLUI_Master.set_glutMouseFunc( OpticalServerInterface_myGlutMouse );
	GLUI_Master.set_glutReshapeFunc( OpticalServerInterface_myGlutReshape );
	GLUI_Master.set_glutIdleFunc( OpticalServerInterface_myGlutIdle );
	
	GLUI* glui_subwin = GLUI_Master.create_glui_subwindow( iface->main_window,
		GLUI_SUBWINDOW_LEFT );
	glui_subwin->set_main_gfx_window( iface->main_window );
	GLUI_Master.auto_set_viewport( );
	
	glClearColor( 0.0, 0.0, 0.0, 0.0 );

	glutMainLoop( );

	return 0;
}

OpticalServerInterface::
OpticalServerInterface( )
{
	LPDWORD lpThreadID = NULL;
	ifaceThread = CreateThread( NULL, 0, OpticalServerInterface_mainloop, NULL, 0, lpThreadID );
	printf( "thread id:  %d\n", ifaceThread );
}



OpticalServerInterface::
~OpticalServerInterface( )
{
	// ExitThread( 0 );

}


OpticalServerInterface* OpticalServerInterface::
getInterface( )
{
	if( OpticalServerInterface::instance == NULL ) // create one
	{
		OpticalServerInterface::instance = new OpticalServerInterface( );
	}
	return OpticalServerInterface::instance;
}



