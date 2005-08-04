
#include "Tcl_Linkvar.h"
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <glui.h>
#include <vrpn_Connection.h>

#include "OpticalServerInterface.h"
#include "nmm_Microscope_SEM_diaginc.h"

static char *Version_string = "01.01";

OpticalServerInterface* OpticalServerInterface::instance = NULL;
bool OpticalServerInterface::interfaceShutdown = false;

void OpticalServerInterface_myGlutRedisplay( )
{
	static int count = 0;
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
	glutSwapBuffers( );
}


void OpticalServerInterface_myGlutKeyboard( unsigned char key, int x, int y )
{
	printf( "glut keyboard func\n" );
	OpticalServerInterface* iface = OpticalServerInterface::getInterface( );
	if( key == '+' )
	{
		int idx = iface->microscope->getResolutionIndex( );
		if( idx < EDAX_NUM_SCAN_MATRICES - 1 ) iface->microscope->setResolutionByIndex( ++idx );
	}
	else if( key == '-' )
	{
		int idx = iface->microscope->getResolutionIndex( );
		if( idx > 0 ) iface->microscope->setResolutionByIndex( --idx );
	}
	else if( key == '*' )
	{
		int bin = iface->getBinning( );
		if( bin < 4 ) iface->setBinning( ++bin );
	}
	else if( key =='/' )
	{
		int bin = iface->getBinning( );
		if( bin > 1 ) iface->setBinning( --bin );
	}

}


void OpticalServerInterface_myGlutSpecial( int, int, int )
{
	//printf( "glut special func\n" );
	OpticalServerInterface* iface = OpticalServerInterface::getInterface( );

}


void OpticalServerInterface_myGlutMouse( int button, int state, int x, int y )
{
	//printf( "glut mouse func\n" );
	OpticalServerInterface* iface = OpticalServerInterface::getInterface( );

}


void OpticalServerInterface_myGlutMotion( int x, int y )
{
	//printf( "glut motion func\n" );
	OpticalServerInterface* iface = OpticalServerInterface::getInterface( );

}


void OpticalServerInterface_myGlutReshape( int w, int h )
{
	//printf( "glut reshape func\n" );
	OpticalServerInterface* iface = OpticalServerInterface::getInterface( );
	GLUI_Master.auto_set_viewport( );

}


void OpticalServerInterface_myGlutIdle( )
{
  //printf( "glut idle func\n" );
  //------------------------------------------------------------
  // This must be done in any Tcl app, to allow Tcl/Tk to handle
  // events.  This must happen at the beginning of the idle function
  // so that the camera-capture and video-display routines are
  // using the same values for the global parameters.

  while (Tk_DoOneEvent(TK_DONT_WAIT)) {};

  //------------------------------------------------------------
  // This is called once every time through the main loop.  It
  // pushes changes in the C variables over to Tcl.

  if (Tclvar_mainloop()) {
    fprintf(stderr,"Tclvar Mainloop failed\n");
  }

  vrpn_SleepMsecs( 10 );
}


void OpticalServerInterface::handle_binning_changed(char *new_value, void *userdata)
{
  OpticalServerInterface *me = (OpticalServerInterface *)userdata;
  if( me->microscope != NULL && me->threadReady ) 
  {
	  if( me->microscope->getBinning( ) != atoi( new_value ) )
		  me->microscope->setBinning( atoi(new_value) );
  }
}

void OpticalServerInterface::handle_resolution_changed(char *new_value, void *userdata)
{
  OpticalServerInterface *me = (OpticalServerInterface *)userdata;
  if( me->microscope != NULL && me->threadReady ) 
  {
    int i, x, y;
    sscanf(new_value, "%d %dx%d", &i, &x, &y);
	if( me->microscope->getResolutionIndex( ) != i )
		me->microscope->setResolution( x, y );
  }
}


void OpticalServerInterface::handle_contrast_changed(char *new_value, void *userdata)
{
  OpticalServerInterface *me = (OpticalServerInterface *)userdata;
  if( me->microscope != NULL && me->threadReady ) 
  {
	  if( me->microscope->getContrastLevel( ) != atoi( new_value ) )
		  me->microscope->setContrastLevel( atoi(new_value) );
  }
}


void OpticalServerInterface::handle_exposure_changed( float new_value, void* userdata )
{
  OpticalServerInterface *me = (OpticalServerInterface *)userdata;
  if( me->microscope != NULL && me->threadReady )
  {
	  int e = (int) new_value;
	  *(me->d_exposure) = (float) e;
	  me->microscope->setExposure( e );
  }
}


DWORD WINAPI OpticalServerInterface_mainloop( LPVOID lpParameter )
{
  printf( "OpticalServerInterface_mainloop\n" );
  OpticalServerInterface* iface = OpticalServerInterface::getInterface( );
  glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA );
  iface->image_window = glutCreateWindow( "Optical Microscope Server" );
  printf( " graphics window id:  %d\n", iface->image_window );
  glutDisplayFunc( OpticalServerInterface_myGlutRedisplay );
  glutMotionFunc( OpticalServerInterface_myGlutMotion );
  glutKeyboardFunc( OpticalServerInterface_myGlutKeyboard );
  glutSpecialFunc( OpticalServerInterface_myGlutSpecial );
  glutMouseFunc( OpticalServerInterface_myGlutMouse );
  glutReshapeFunc( OpticalServerInterface_myGlutReshape );
  glutIdleFunc( OpticalServerInterface_myGlutIdle );

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D( 0.0, 1.0, 0.0, 1.0 );
  glClearColor( 0.4, 0.0, 0.4, 0.0 );


  //------------------------------------------------------------------
  // Generic Tcl startup.  Getting and interpreter and mainwindow.

  char		command[256];
  Tcl_Interp	*tk_control_interp;
  Tk_Window       tk_control_window;
  tk_control_interp = Tcl_CreateInterp();

  /* Start a Tcl interpreter */
  if (Tcl_Init(tk_control_interp) == TCL_ERROR) {
          fprintf(stderr,
                  "Tcl_Init failed: %s\n",tk_control_interp->result);
          return(-1);
  }

  /* Start a Tk mainwindow to hold the widgets */
  if (Tk_Init(tk_control_interp) == TCL_ERROR) {
	  fprintf(stderr,
	  "Tk_Init failed: %s\n",tk_control_interp->result);
	  return(-1);
  }
  tk_control_window = Tk_MainWindow(tk_control_interp);
  if (tk_control_window == NULL) {
          fprintf(stderr,"%s\n", tk_control_interp->result);
          return(-1);
  }

  //------------------------------------------------------------------
  // Loading the particular definition files we need.  russ_widgets is
  // required by the Tclvar_float_with_scale class.  simple_magnet_drive
  // is application-specific and sets up the controls for the integer
  // and float variables.

  /* Load the Tcl scripts that handle widget definition and
   * variable controls */
  sprintf(command, "source russ_widgets.tcl");
  if (Tcl_Eval(tk_control_interp, command) != TCL_OK) {
          fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command,
                  tk_control_interp->result);
          return(-1);
  }

  //------------------------------------------------------------------
  // Put the version number into the main window.

  sprintf(command, "label .versionlabel -text Optical_v:_%s", Version_string);
  if (Tcl_Eval(tk_control_interp, command) != TCL_OK) {
          fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command,
                  tk_control_interp->result);
          return(-1);
  }
  sprintf(command, "pack .versionlabel");
  if (Tcl_Eval(tk_control_interp, command) != TCL_OK) {
          fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command,
                  tk_control_interp->result);
          return(-1);
  }

  //------------------------------------------------------------------
  // Add the controls that we need.

  // binning
  iface->d_binningList = new Tclvar_list_of_strings();
  iface->d_binningList->Add_entry("1");
  iface->d_binningList->Add_entry("2");
  iface->d_binningList->Add_entry("3");
  iface->d_binningList->Add_entry("4");
  iface->d_binningSelector = new Tclvar_selector("binning", "", iface->d_binningList, "1", iface->handle_binning_changed, iface);

  // resolution
  iface->d_resolutionList = new Tclvar_list_of_strings();
  char s[512];
  for( int i = 0; i <= EDAX_NUM_SCAN_MATRICES - 1; i++ ) {
    sprintf( s, "%i %dx%d", i, EDAX_SCAN_MATRIX_X[i], EDAX_SCAN_MATRIX_Y[i] );
    iface->d_resolutionList->Add_entry(s);
  }
  sprintf( s, "%dx%d", EDAX_SCAN_MATRIX_X[EDAX_DEFAULT_SCAN_MATRIX], EDAX_SCAN_MATRIX_Y[EDAX_DEFAULT_SCAN_MATRIX] );
  iface->d_resolutionSelector = new Tclvar_selector("resolution", "", iface->d_resolutionList, s, iface->handle_resolution_changed, iface);

  // contrast
  iface->d_contrastList = new Tclvar_list_of_strings();
  iface->d_contrastList->Add_entry( "0" );
  iface->d_contrastList->Add_entry( "1" );
  iface->d_contrastList->Add_entry( "2" );
  iface->d_contrastList->Add_entry( "3" );
  iface->d_contrastList->Add_entry( "4" );
  iface->d_contrastList->Add_entry( "5" );
  iface->d_contrastList->Add_entry( "6" );
  iface->d_contrastList->Add_entry( "7" );
  iface->d_contrastList->Add_entry( "8" );
  iface->d_contrastSelector = new Tclvar_selector("contrast", "", iface->d_contrastList, "0", iface->handle_contrast_changed, iface );

  // exposure time
  iface->d_exposure = new Tclvar_float_with_scale( "exposure_in_ms", "", 10, 250, 100, iface->handle_exposure_changed, iface );

  
  //------------------------------------------------------------------
  // This routine must be called in order to initialize all of the
  // variables that came into scope before the interpreter was set
  // up, and to tell the variables which interpreter to use.  It is
  // called once, after the interpreter exists.

  // Initialize the variables using the interpreter
  if (Tclvar_init(tk_control_interp)) {
	  fprintf(stderr,"Can't do init!\n");
	  return -1;
  }

  //------------------------------------------------------------------
  // Ready to rock and roll
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
  microscope( NULL ),
  binning( 2 ),
  resolutionIndex( EDAX_DEFAULT_SCAN_MATRIX ),
  contrast( 0 )
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
		this->setBinning( m->getBinning( ) );
		this->setResolutionIndex( m->getResolutionIndex( ) );
	}
}


void OpticalServerInterface::
setBinning( int bin ) 
{ 
	binning = bin;
	char binStr[512];
	sprintf( binStr, "%d", bin );
	*d_binningSelector = binStr;
}


void OpticalServerInterface::
setExposure( int exposure )
{
	*d_exposure = exposure;
}


void OpticalServerInterface::
setResolutionIndex( int idx ) 
{ 
	resolutionIndex = idx;
	char idxStr[512];
	if( idx >= 0 && idx <= EDAX_NUM_SCAN_MATRICES - 1 )
	{
		sprintf( idxStr, "%d %dx%d", idx, EDAX_SCAN_MATRIX_X[idx], EDAX_SCAN_MATRIX_Y[idx] );
	}
	else
	{
		sprintf( idxStr, "%d ?x?", idx );
	}
	*d_resolutionSelector = idxStr;
}

void OpticalServerInterface::
setContrast( int contrast )
{
	if( contrast < 0 || contrast > 8 ) return;
	this->contrast = contrast;
	char str[512];
	sprintf( str, "%d", contrast );
	*d_contrastSelector = str;
}