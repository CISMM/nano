//XXX Issues:
//	active_set.c required by AFMState.o
//	libnmb.a: filter.o requires SDI library
//	GTF is an extern from the library that user code must create
//      Either X or GLUT blocks while windows are being resized.

#include <stdio.h>
#include <math.h>
#include <string.h>

// HACK for select()
#ifndef	__CYGWIN__
     #include <unistd.h>
     #include <sys/types.h>
     #include <bstring.h>
     #include <sys/time.h>
#endif



#include <GL/glut.h>  // typically from ~walk/include

#include <glui.h>  // typically from ~walk/include

#include <vrpn_Connection.h>
#include <vrpn_FileConnection.h>

#include <BCGrid.h>
#include <BCPlane.h>
#include <Topo.h>

#include <nmb_Dataset.h>
#include <nmb_Decoration.h>
#include <nmb_Selector.h>

#include <nmm_MicroscopeRemote.h>

struct rgb {
  unsigned char r;
  unsigned char g;
  unsigned char b;
};

struct DataWindow {
  int id;
  int pixelWidth, pixelHeight;
  rgb * frameBuffer;
};

// This is an external definition that is required by the BCGrid base class.
TopoFile	GTF;

//----------------------------------------------------------------------------
// Constants

const	int	MAX_SCOPES = 9;	// How many scopes at most?

//----------------------------------------------------------------------------
// Structures used to store the data, and the microscopes themselves

static int g_numMicroscopes = 0;
  // How many microscopes are open (0 to start)
static vrpn_Connection * g_connections [MAX_SCOPES];
  // Network connections to microscopes
static nmm_Microscope_Remote * g_microscopes [MAX_SCOPES];
  // Controllers (proxies) for microscopes
static AFMInitializationState g_initStates [MAX_SCOPES];
  // Initialization data for microscopes

static int g_mNum [MAX_SCOPES];
  // Array that maps array index to human-understood integer ID
  // (Ought to use a string instead)  HACK

static nmb_Decoration * g_decorations [MAX_SCOPES];
static nmb_Dataset * g_datasets [MAX_SCOPES];
  // Data structures used by microscope proxies.
  // nmb_Decoration holds incidental data
  // nmb_Dataset holds sample data

static DataWindow g_dataWindows [MAX_SCOPES];
  // Data for managing windows displaying data.

//----------------------------------------------------------------------------
// Variables used for the user interface

static nmm_Microscope_Remote * g_currentMicroscope;
  // Send commands to this microscope.
  // If NULL, send commands to all microscopes.

static GLUI_RadioGroup * g_scopeGroup;
  // UI for g_currentMicroscope.

//----------------------------------------------------------------------------
// Macros for the user interface

#define MICROSCOPE_DO(a) \
  if (g_currentMicroscope) { \
    g_currentMicroscope->a; \
  } else { \
    int i; \
    for (i = 0; i < g_numMicroscopes; i++) { \
      g_microscopes[i]->a; \
    } \
  }

//----------------------------------------------------------------------------
// Functions

int handle_point_data (void * userdata, const Point_results * r) {
  int microscopeNumber = * (int *) userdata;

  printf("Got point results from microscope #%d:  (%.5lf, %.5lf, %.5lf)\n",
    microscopeNumber, r->x(), r->y(), r->z());

  return 0;
}

void	Usage(char *progname)
{
  fprintf(stderr, "Usage: %s afm-source [afm-source]*\n", progname);
  fprintf(stderr, "  afm-source can be any of the URLs understood by VRPN;\n"
                  "  Most commonly:\n");
  fprintf(stderr, "    <DNS name>.\n");
  fprintf(stderr, "    file:<file name>.\n");
}

void cleanup (void) {
  int i;

  for (i = 0; i < g_numMicroscopes; i++) {
    delete [] g_dataWindows[i].frameBuffer;
    delete g_datasets[i];
    delete g_decorations[i];
    delete g_microscopes[i];
    delete g_connections[i];
  }

}



// Update the display of data received from the microscope.
// Total kluge for demo purposes.

void displayDataWindow (void) {
  BCPlane * heightPlane;
  float minZ, maxZ, deltaZ;
  float d;
  int minX, minY, maxX, maxY;
  int x, y;
  int whichIndex;
  unsigned char c;

  whichIndex = glutGetWindow() - 1;
  DataWindow & dw = g_dataWindows[whichIndex];

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Do it
  g_datasets[whichIndex]->range_of_change.GetBoundsAndClear
         (&minX, &maxX, &minY, &maxY);

//fprintf(stderr, "Change is (%d, %d) to (%d, %d).\n",
//minX, minY, maxX, maxY);

  heightPlane = g_datasets[whichIndex]->inputGrid->getPlaneByName
     (g_datasets[whichIndex]->heightPlaneName->string());

  if (!heightPlane) {
    fprintf(stderr, "NULL height plane.\n");
    return;
  }

  minZ = heightPlane->minValue();
  maxZ = heightPlane->maxValue();
  deltaZ = 1.0f / (maxZ - minZ);

  if (minZ > maxZ) {
    fprintf(stderr, "Uninitialized plane.\n");
    return;
  }

//fprintf(stderr, "With maxZ %.5f, minZ %.5f, scale factor is %.5f.\n",
//maxZ, minZ, deltaZ);


  // For grids as large as 300x300, recomputing the entire thing isn't
  // significantly slower and may help give us a better gray scale.
  // More important than that would be a smarter minValue()/maxValue()
  // routine - if the timestamp is (0,0) ignore the false 0.0 value.
  //   (Implemented on CTimedPlane 17 Sep 99 by TCH;
  //    still does not fix everything...)

  //for (x = minX; x <= maxX; x++) {
  //  for (y = minY; y <= maxY; y++) {

  for (x = 0; x < heightPlane->numX(); x++) {
    for (y = 0; y < heightPlane->numY(); y++) {

      d = heightPlane->value(x, y);
      d = (d - minZ) * deltaZ;
      c = 255 * d;

      dw.frameBuffer[x * heightPlane->numY() + y].r = c;
      dw.frameBuffer[x * heightPlane->numY() + y].g = c;
      dw.frameBuffer[x * heightPlane->numY() + y].b = c;

    }
  }

//fprintf(stderr, "At (%d, %d) had value %.5f, normalized to %.5f, "
//"producing color %d.\n", x - 1, y - 1, heightPlane->value(x - 1, y - 1),
//d, c);

  glDrawPixels(heightPlane->numX(), heightPlane->numY(),
               GL_RGB, GL_UNSIGNED_BYTE,
               dw.frameBuffer);

  glutSwapBuffers();

//fprintf(stderr, "Redisplayed data window for microscope #%d.\n",
//whichIndex + 1);

}



void reshapeDataWindow (int w, int h) {
  int whichIndex;

  whichIndex = glutGetWindow() - 1;

  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0, w, 0, h);

  g_dataWindows[whichIndex].pixelWidth = w;
  g_dataWindows[whichIndex].pixelHeight = h;

fprintf(stderr, "Resized data window for microscope #%d to (%d x %d).\n",
whichIndex + 1, w, h);

  // We currently use a fixed-size "frame buffer";  if we wanted to scale
  // our display to whatever window size the user specified we'd have to
  // be more intelligent, and would (at least when the frame buffer grew)
  // have to reallocate it.

  //if (g_dataWindows[whichIndex].frameBuffer) {
  //  delete [] g_dataWindows[whichIndex].frameBuffer;
  //}
  //g_dataWindows[whichIndex].frameBuffer = new unsigned char [w * h];

}



// Idle function for GLUT - we want to call this as often as possible.
// Processes any messages received from the microscopes;  if there's
// new data, lets GLUT know.
// When we're using GLUI we must explicitly set the window in the idle
// function;  that's OK, we were doing that anyway.

void checkMicroscopes (void) {
  int i;

  for (i = 0; i < g_numMicroscopes; i++) {
    g_microscopes[i]->mainloop();

    if (g_datasets[i]->range_of_change.Changed()) {
      glutSetWindow(g_dataWindows[i].id);
      glutPostRedisplay();
    }
  }

  //timeval timeout;
  //timeout.tv_sec = 0L;
  //timeout.tv_usec = 1000L;
  //select (0, NULL, NULL, NULL, &timeout);

}



// Initializes a window to display data from the microscope.

void setupGLUTwindow (int whichIndex) {
  char mname [100];

  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
  glutInitWindowPosition(0, 0);
  glutInitWindowSize(350, 350);

  sprintf(mname, "Microscope #%d", whichIndex + 1);
  g_dataWindows[whichIndex].id = glutCreateWindow(mname);
  g_dataWindows[whichIndex].pixelWidth = 350;
  g_dataWindows[whichIndex].pixelHeight = 350;

  glutDisplayFunc(displayDataWindow);
  glutReshapeFunc(reshapeDataWindow);

  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
}


// User Interface

void handle_selectScope (int) {
  int which = g_scopeGroup->get_int_val();

  if (!which) {
    // select all microscopes
    g_currentMicroscope = NULL;
fprintf(stderr, "Sending to all microscopes.\n");
  } else {
    g_currentMicroscope = g_microscopes[which - 1];
fprintf(stderr, "Sending to microscope %d.\n", which);
  }
}

void handle_quit (int) {
  fprintf(stderr, "Cleaning up and shutting down.\n");
  cleanup();
  exit(0);
}

void handle_resumeScan (int) {
  MICROSCOPE_DO(ResumeScan());
}


void handle_stopScan (int) {
  MICROSCOPE_DO(state.modify.setpoint = 0.0f);
  MICROSCOPE_DO(ModifyMode());
}

void setReplayRate (vrpn_Connection * c, float rate) {
  vrpn_File_Connection * fc;

  fc = c->get_File_Connection();

  if (fc) {
    fc->set_replay_rate(rate);
  } else {
    fprintf(stderr, "Warning:  Tried to set rate on a live scope.\n");
  }
}


void handle_startPlayback (int) {
  int which = g_scopeGroup->get_int_val();
  int i;

  if (!which) {
    for (i = 0; i < g_numMicroscopes; i++) {
      setReplayRate(g_connections[i], 10.0f);
    }
  } else {
fprintf(stderr, "Starting playback on microscope %d.\n", which);
    setReplayRate(g_connections[which - 1], 10.0f);
  }
}

void handle_stopPlayback (int) {
  int which = g_scopeGroup->get_int_val();
  int i;

  if (!which) {
fprintf(stderr, "Stopping playback on all microscopes.\n");
    for (i = 0; i < g_numMicroscopes; i++) {
      setReplayRate(g_connections[i], 0.0f);
    }
  } else {
fprintf(stderr, "Stopping playback on microscope %d.\n", which);
    setReplayRate(g_connections[which - 1], 0.0f);
  }
}


// Initializes a window to display a simple set of controls
// using GLUI.

void setupGLUIpane (int argc, char ** argv) {
  int i;

  GLUI * glui = GLUI_Master.create_glui("Microscope Control");

  // We don't HAVE a main graphics window.  Do we still need to
  // call glui->set_main_gfx_window()?
  
  // Pun time:  the "scope" panel controls whether our commands
  // are sent to one scope or all of them - their scope

  GLUI_Panel * scopePanel;
  scopePanel = glui->add_panel("Scope");

  g_scopeGroup = glui->add_radiogroup_to_panel(scopePanel, NULL, -1,
                                               handle_selectScope);

  glui->add_radiobutton_to_group(g_scopeGroup, "All Microscopes");

  // Add a scope-specific radiobutton for each
  for (i = 1; i < argc; i++) {
    glui->add_radiobutton_to_group(g_scopeGroup, argv[i]);
  }

  GLUI_Panel * commandPanel;
  commandPanel = glui->add_panel("Commands");

  glui->add_button_to_panel(commandPanel, "Start (Live) Scanning", -1,
                            handle_resumeScan);
//  glui->add_button_to_panel(commandPanel, "Stop (Live) Scanning", -1,
//                            handle_stopScan);

  // TODO add a bar to the panel

  glui->add_button_to_panel(commandPanel, "Start Playback", -1,
                            handle_startPlayback);
  glui->add_button_to_panel(commandPanel, "Stop Playback", -1,
                            handle_stopPlayback);


  glui->add_button("Quit", -1, handle_quit);

}


void main (int argc, char ** argv) {
  int	i;

  // INITIALIZATION

  // Check command line
  if (argc < 2) {
    Usage(argv[0]);
    exit(0);
  }
  if (argc > MAX_SCOPES + 1) {
    fprintf(stderr,"Can be at most %d scopes\n",MAX_SCOPES);
    exit(0);
  }

  // Initialize one microscope for each one listed on the command line
  for (i = 1; i < argc; i++) {
    AFMInitializationState & istate = g_initStates[g_numMicroscopes];

    printf("Opening scope #%d as %s...\n", i, argv[i]);

    // Open the network connection to the server
    // Hacked so we can (try to) open multiple connections to the same file.

    if (!strncmp(argv[i], "file:", 5)) {
      g_connections[g_numMicroscopes] = new vrpn_File_Connection(argv[i]);
    } else {
      g_connections[g_numMicroscopes] = vrpn_get_connection_by_name(argv[i]);
    }

    // Set up the initialization state for the scope

    strncpy(istate.deviceName, argv[i], sizeof(istate.deviceName) - 1);
    istate.stm_z_scale = 1.0;
    istate.doRelaxComp = 0;
    istate.doRelaxUp = 0;
    istate.doDriftComp = 0;
    istate.stmRxTmin = 0.2;
    istate.stmRxTsep = 0.5;
    istate.doSplat = 0;
    if (!strncmp(argv[i], "file:", 5)) {
      istate.readingStreamFile = 1;
    } else {
      istate.readingStreamFile = 0;
    }

    istate.writingStreamFile = 0;

    // Connect the scope proxy to its server over the network connection
    //   we already opened.

    g_microscopes[g_numMicroscopes] =
    	new nmm_Microscope_Remote (istate,
      	                           g_connections[g_numMicroscopes]);

    // Assume the sample is being scanned at a resolution of no more than
    //   300x300.
    // [xy]{Min,Max} shouldn't be significant.
    // istate.readingStreamFile may not matter any more but being sure
    //   would take a bunch of work.
    // No files are specified to read.
    // allocate_nmb_Selector keeps us from trying to construct Tcl
    //   controllers.

    g_datasets[g_numMicroscopes] =
        new nmb_Dataset (vrpn_FALSE, 300, 300,
                         istate.xMin, istate.xMax, istate.yMin, istate.yMax,
                         istate.readingStreamFile ? READ_STREAM : READ_DEVICE,
                         NULL, 0, allocate_nmb_Selector);

    g_decorations[g_numMicroscopes] = new nmb_Decoration;

    g_microscopes[g_numMicroscopes]->
        InitializeDataset(g_datasets[g_numMicroscopes]);
    g_microscopes[g_numMicroscopes]->
        InitializeDecoration(g_decorations[g_numMicroscopes]);

    //g_microscopes[g_numMicroscopes]->
    //  registerPointDataHandler(handle_point_data,
    //                           &g_mNum[g_numMicroscopes]);
    g_mNum[g_numMicroscopes] = g_numMicroscopes;

    g_dataWindows[g_numMicroscopes].frameBuffer = new rgb [300 * 300];

    setupGLUTwindow(g_numMicroscopes);

    g_numMicroscopes++;
  }

  setupGLUIpane(argc, argv);

  // Using GLUI we register the idle function with GLUI instead of GLUT

  //glutIdleFunc(checkMicroscopes);
  GLUI_Master.set_glutIdleFunc(checkMicroscopes);

  // HACK - if we're replaying streams (for a demo), run them 10x as fast

  for (i = 0; i < g_numMicroscopes; i++) {
    vrpn_File_Connection * fc;

    fc = g_connections[i]->get_File_Connection();

    if (fc) {
      fc->set_replay_rate(10.0f);
      fprintf(stderr, "Microscope %d stream replaying at 10x speed.\n",
              g_mNum[i]);
    }
  }

  // STEADY-STATE

  glutMainLoop();

}

