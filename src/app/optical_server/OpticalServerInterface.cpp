
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <glui.h>
#include <vrpn_Connection.h>

#include "OpticalServerInterface.h"
#include "nmm_Microscope_SEM_diaginc.h"

OpticalServerInterface* OpticalServerInterface::instance = NULL;
bool OpticalServerInterface::interfaceShutdown = false;

void OpticalServerInterface_myGlutRedisplay( )
{
	static int count = 0;
	OpticalServerInterface* iface = OpticalServerInterface::getInterface( );
	printf( "display %d\n", count++ );

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
	printf( "glut keyboard func\n" );
	OpticalServerInterface* iface = OpticalServerInterface::getInterface( );

}


void OpticalServerInterface_myGlutSpecial( int, int, int )
{
	printf( "glut special func\n" );
	OpticalServerInterface* iface = OpticalServerInterface::getInterface( );

}


void OpticalServerInterface_myGlutMouse( int button, int state, int x, int y )
{
	printf( "glut mouse func\n" );
	OpticalServerInterface* iface = OpticalServerInterface::getInterface( );

}


void OpticalServerInterface_myGlutMotion( int x, int y )
{
	printf( "glut motion func\n" );
	OpticalServerInterface* iface = OpticalServerInterface::getInterface( );

}


void OpticalServerInterface_myGlutReshape( int w, int h )
{
	printf( "glut reshape func\n" );
	OpticalServerInterface* iface = OpticalServerInterface::getInterface( );
	GLUI_Master.auto_set_viewport( );

}


void OpticalServerInterface_myGlutIdle( )
{
	OpticalServerInterface* iface = OpticalServerInterface::getInterface( );
	printf( "glut idle func\n" );
	vrpn_SleepMsecs( 100 );
}


void OpticalServerInterface_gluiChangedBinning( int id )
{
	OpticalServerInterface* iface = OpticalServerInterface::getInterface( );
	if( iface->microscope != NULL && iface->threadReady )
		iface->microscope->setBinning( iface->binningListbox->get_int_val( ) );

}


void OpticalServerInterface_gluiChangedResolution( int id )
{
	OpticalServerInterface* iface = OpticalServerInterface::getInterface( );

}


DWORD WINAPI OpticalServerInterface_mainloop( LPVOID lpParameter )
{
	printf( "OpticalServerInterface_mainloop\n" );
	OpticalServerInterface* iface = OpticalServerInterface::getInterface( );
	iface->image_window = glutCreateWindow( "Optical Microscope Server" );
	printf( " graphics window id:  %d\n", iface->image_window );
	glutDisplayFunc( OpticalServerInterface_myGlutRedisplay );
	glutMotionFunc( OpticalServerInterface_myGlutMotion );
	GLUI_Master.set_glutKeyboardFunc( OpticalServerInterface_myGlutKeyboard );
	GLUI_Master.set_glutSpecialFunc( OpticalServerInterface_myGlutSpecial );
	GLUI_Master.set_glutMouseFunc( OpticalServerInterface_myGlutMouse );
	GLUI_Master.set_glutReshapeFunc( OpticalServerInterface_myGlutReshape );
	GLUI_Master.set_glutIdleFunc( OpticalServerInterface_myGlutIdle );
	//GLUI_Master.set_glutIdleFunc( NULL );
	
	//iface->glui_window = GLUI_Master.create_glui( "Optical Microscope Controls" );
	//printf( " GLUI window id:  %d\n", iface->glui_window->get_glut_window_id( ) );
	//iface->glui_window 
	//	= GLUI_Master.create_glui_subwindow( iface->image_window, GLUI_SUBWINDOW_LEFT );
	
	//iface->glui_window->set_main_gfx_window( iface->image_window );

	/*
	GLUI_Panel* panel = iface->glui_window->add_panel( "Resolution" );
	iface->resRadioGroup 
		= iface->glui_window->add_radiogroup_to_panel( panel, NULL, -1, 
				OpticalServerInterface_gluiChangedResolution );
	char s[512];
	for( int i = 0; i <= EDAX_NUM_SCAN_MATRICES - 1; i++ )
	{
		sprintf( s, "%d x %d", EDAX_SCAN_MATRIX_X[i], EDAX_SCAN_MATRIX_Y[i] );
		iface->glui_window->add_radiobutton_to_group( iface->resRadioGroup, s );
	}
	iface->resRadioGroup->set_int_val( EDAX_DEFAULT_SCAN_MATRIX );
	iface->binningListbox = iface->glui_window->add_listbox( "Binning", NULL, -1, 
				OpticalServerInterface_gluiChangedBinning );
	iface->binningListbox->add_item( 1, "1" );
	iface->binningListbox->add_item( 2, "2" );
	iface->binningListbox->add_item( 3, "3" );
	iface->binningListbox->add_item( 4, "4" );
	*/

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D( 0.0, 1.0, 0.0, 1.0 );

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
  threadReady( false ),
  image_window( -1 ),
  glui_window( NULL ),
  binningListbox( NULL ),
  resRadioGroup( NULL ),
  microscope( NULL )
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
	glutSetWindow( this->image_window );
	glutPostRedisplay( );
}


void OpticalServerInterface::
setMicroscope( nmm_Microscope_SEM_diaginc* m )
{
	this->microscope = m;
	if( m != NULL )
	{
		int ret;
		m->getBinning( ret );
		this->setBinning( ret );
		this->setResolutionIndex( m->getResolutionIndex( ) );
	}
}


int OpticalServerInterface::
getBinning( ) 
{
	if( threadReady && binningListbox != NULL )
		return binningListbox->get_int_val( ); 
	else
		return -1;
}


int OpticalServerInterface::
getResolutionIndex( ) 
{
	if( threadReady && resRadioGroup != NULL )
		return resRadioGroup->get_int_val( ); 
	else
		return -1;
}


void OpticalServerInterface::
setBinning( int bin ) 
{
	if( threadReady && binningListbox != NULL )
		binningListbox->set_int_val( bin ); 
}


void OpticalServerInterface::
setResolutionIndex( int idx ) 
{ 
	if( threadReady && resRadioGroup != NULL )
		resRadioGroup->set_int_val( idx ); 
}


