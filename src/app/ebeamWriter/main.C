/*
main.C : this file contains the main() function for the seegerizer. This
 program can either connect to a remote sem server or run the sem server in
 a separate thread and connect to it locally
*/

// stuff for tcl/tk graphical user interface 
#include <tcl.h>
#include <tk.h>

#include <Tcl_Linkvar.h>
#include <Tcl_Interpreter.h>

#include <stdio.h>

#include "GL/glut_UNC.h"

#include "patternEditor.h"
#include "nmb_TransformMatrix44.h"
#include "transformFile.h"
#include "nmr_Util.h"

#include "nmr_RegistrationUI.h"
#include "nmr_Registration_Proxy.h"
#include "nmm_Microscope_SEM_Remote.h"
#include "nmm_Microscope_SEM_EDAX.h"
#include "controlPanels.h"

#include "nmb_ImgMagick.h"

#include "thread.h"

/* arguments:

 -t <transformation file>
 -f <spmlab file or pgm>

format for transformation file (the matrix gives the world to image xform):
num_files <n>
file_name <filename_1>
t00 t01 t02 t03
t10 t11 t12 t13
t20 t21 t22 t23
t30 t31 t32 t33
file_name <filename_2>
t00 t01 t02 t03
t10 t11 t12 t13
t20 t21 t22 t23
t30 t31 t32 t33
...
file_name <filename_n)>
t00 t01 t02 t03
t10 t11 t12 t13
t20 t21 t22 t23
t30 t31 t32 t33
*/

static int parseArgs(int argc, char **argv);

#define MAX_PLANNING_IMAGES 10
static char **planningImageNames;
static int numPlanningImages = 0;
nmb_ImageManager *dataset = NULL;
nmb_ImageList *imageData = NULL;

static char transformFileName[256];

static vrpn_bool semDeviceSet = VRPN_FALSE, alignerDeviceSet = VRPN_FALSE;
static char semDeviceName[256];
static char alignerDeviceName[256];
static vrpn_bool virtualAcquisition = vrpn_FALSE;

static TransformFile transformFile;

static Tclvar_int timeToQuit ("time_to_quit", 0);
static Tclvar_int semControlsEnabled("sem_controls_enabled", 1);
static vrpn_bool needToDisableSEMControls = vrpn_FALSE;
static Tclvar_int nextLeftWindowPos("next_left_pos", 0);

vrpn_Connection *local_connection;
PatternEditor *patternEditor = NULL;
nmr_Registration_Proxy *aligner = NULL;
nmr_RegistrationUI *alignerUI = NULL;
nmm_Microscope_SEM_Remote *sem = NULL;
ControlPanels *controls = NULL;

nmm_Microscope_SEM_EDAX *sem_server = NULL;
vrpn_Connection *internal_device_connection = NULL;

#ifdef V_GLUT
void nullDisplayFunc() { ; }
void nullIdleFunc() { ; }
#endif

void handle_LeftWindowPos_change(int value, void *ud);

/**************** stuff for multithreading ******************/
static ThreadData td;
void serverThreadStart(void *ud);
Thread *serverThread;
/************************************************************/

int main(int argc, char **argv)
{
	nmb_ImgMagick::initMagick(argv[0]);

    // Initialize TCL/TK so that TclLinkvar variables link up properly
    char *tcl_script_dir;
    char command[128];

    local_connection = new vrpn_Synchronized_Connection(4511);

    if ((tcl_script_dir=getenv("NM_TCL_DIR")) == NULL) {
         tcl_script_dir = "./";
    }
    Tcl_Interp *tk_control_interp = Tcl_Interpreter::getInterpreter();
    Tclvar_init(tk_control_interp);
    Tclnet_init(tk_control_interp);

    // Hide the main window.
    sprintf(command, "wm withdraw .");
    TCLEVALCHECK(tk_control_interp, command);

    // Tell tcl script what directory it lives in. 
    sprintf(command, "%s",tcl_script_dir);
    Tcl_SetVar(tk_control_interp,"tcl_script_dir",command,TCL_GLOBAL_ONLY);
    // source the litho_main.tcl file
    if ((tcl_script_dir[strlen(tcl_script_dir)-1]) == '/') {
        sprintf(command, "%s%s",tcl_script_dir,"litho_main.tcl");
    } else {
        sprintf(command, "%s/%s",tcl_script_dir,"litho_main.tcl");
    }
    if (Tcl_EvalFile(tk_control_interp, command) != TCL_OK) {
        fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command,
                tk_control_interp->result);
        return 0;
    }

    // Figure out what image files to load and other options
    int i;
    planningImageNames = new char *[MAX_PLANNING_IMAGES];
    for (i = 0; i < MAX_PLANNING_IMAGES; i++){
        planningImageNames[i] = NULL;
    }
    sprintf(transformFileName, "default_transforms.txt");
    // fill in the list of planning images, setting their worldToImage
    // transformations from the transformation file if it is available and
    // contains the files loaded
    parseArgs(argc, argv);

    // only load the transformations if the transform file exists
    FILE *test = fopen(transformFileName, "r");
    if (test) {
       fclose(test);
       transformFile.load((char *)transformFileName);
    }

    // initialize graphics

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(100, 100);
    glutInitWindowPosition(100, 100);
#ifndef _WIN32
    glutCreateWindow("the program crashes without this window");
    glutDisplayFunc(nullDisplayFunc);
    glutIdleFunc(nullIdleFunc);
#endif
    // this must come after we initialize graphics
    int windowPositionX = 50;
    int windowPositionY = 50;
    patternEditor = new PatternEditor(windowPositionX, windowPositionY);
    nextLeftWindowPos.addCallback(handle_LeftWindowPos_change, patternEditor);

    // create the other two important objects for the program 
    // Registration and SEM

    if (alignerDeviceSet) {
      aligner = new nmr_Registration_Proxy(alignerDeviceName);
    } else { // use local implementation
      aligner = new nmr_Registration_Proxy(NULL, local_connection);
    }
    if (semDeviceSet) {
      printf("Opening SEM: %s\n", semDeviceName);
      sem = new nmm_Microscope_SEM_Remote(semDeviceName);
    } else {
      internal_device_connection = new vrpn_Synchronized_Connection(4512);
      sem_server = new nmm_Microscope_SEM_EDAX("localSEM",
                                          internal_device_connection, 
                                          virtualAcquisition);
      sem_server->setBufferEnable(vrpn_TRUE); // don't receive incoming
					      // data or parameters except
                                              // where we call sendBuffer() -
                                              // in the client/UI thread
      sem = new nmm_Microscope_SEM_Remote("localSEM", 
                                          internal_device_connection);
      sem->setBufferEnable(vrpn_TRUE); // don't send outgoing commands except
                                       // where we call sendBuffer() - in
				       // the server thread

      td.ps = new Semaphore(0);
      serverThread = new Thread(serverThreadStart, td);
      serverThread->go();
      int retval = td.ps->p();
      if (retval != 1) {
        fprintf(stderr, "main process wasn't successfully released to run\n");
        serverThread->kill();
        exit(0);
      }
      // set it to 1 so either thread can push it
      td.ps->v();
    }


    // Now we can hook up the Tcl/Tk control panels 
    // to the parts that do the work

    alignerUI = new nmr_RegistrationUI(aligner, patternEditor);
    alignerUI->setupCallbacks();
	alignerUI->d_flipXreference = 0;
	alignerUI->d_flipProjectionImageInX = 0;

	controls = new ControlPanels(patternEditor, sem, alignerUI);
    
    patternEditor->setControlPanels(controls);
    // load the images specified on the command line
    TopoFile defaultTopoFileSettings;
    imageData = new nmb_ImageList(controls->imageNameList());

    dataset = new nmb_ImageManager(imageData);

    alignerUI->changeDataset(dataset);

    controls->setImageList(imageData);
    // this needs to come after the controls get the pointer to the imageData
    // list so that as the list of image names in tcl is updated and tcl
    // selectors that index into this list generate callbacks, the C-code has
    // the corresponding list in C where it needs to search for the images
    imageData->addFileImages((const char **)planningImageNames,
                          numPlanningImages,
                          defaultTopoFileSettings);

    double matrix[16];
    double default_matrix[16] = {0.001, 0.0, 0.0, 0.0,
                                 0.0, 0.001, 0.0, 0.0,
                                 0.0, 0.0, 1.0, 0.0,
                                 0.0, 0.0, 0.0, 1.0};
    double imageRegionWidth, imageRegionHeight;
    sem->getScanRegion_nm(imageRegionWidth, imageRegionHeight);
    if (imageRegionWidth != 0 && imageRegionHeight != 0) {
      default_matrix[0] = 2.0/imageRegionWidth;
      default_matrix[5] = 2.0/imageRegionHeight;
    }

    double acqDistX, acqDistY;
    nmb_Image *currImage;

    for (i = 0; i < imageData->numImages(); i++){
        currImage = imageData->getImage(i);
        currImage->normalize();
        currImage->setWorldToImageTransform(default_matrix);
        // search for this image in the list of transformations we loaded
        if (transformFile.lookupImageTransformByName(
                          currImage->name()->c_str(),
                          matrix, acqDistX, acqDistY)) {
            currImage->setWorldToImageTransform(matrix);
            currImage->setAcquisitionDimensions(acqDistX, acqDistY);
            printf("setting world to image transform for %s\n",
                   currImage->name()->c_str());
        }
        patternEditor->addImage(currImage);
    }

    patternEditor->show();

    ImageViewer *image_viewer = ImageViewer::getImageViewer();

    semControlsEnabled = 1;

    if (sem_server) {
      sem_server->reportResolution();
      sem_server->reportPixelIntegrationTime();
      sem_server->reportInterPixelDelayTime();
      sem_server->reportMaxScanSpan();
      sem_server->reportBeamBlankEnable();
      sem_server->reportPointDwellTime();
      sem_server->reportRetraceDelays();
      sem_server->reportDACParams();
      sem_server->reportExternalScanControlEnable();
      sem_server->reportMagnification();
    }

    while(!timeToQuit){
      glutProcessEvents_UNC();
      while (Tk_DoOneEvent(TK_DONT_WAIT)) {};
      if (Tclvar_mainloop()) {
          fprintf(stderr, "main: Tclvar_mainloop error\n");
          return -1;
      }
      if (internal_device_connection) {
        sem_server->sendBuffer(); // may execute Tcl code in response to
                                  // incoming data and we can only do that
                                  // in this thread
      } else if (sem) {
        sem->mainloop();
      }

      if (aligner) {
          aligner->mainloop();
      }

      image_viewer->mainloop();

      if (needToDisableSEMControls && (semControlsEnabled != 0)) {
        semControlsEnabled = 0;
      } else if (!needToDisableSEMControls && (semControlsEnabled == 0)) {
        semControlsEnabled = 1;
      }
    }

    return 0;
}

static int usage(char *programName)
{
  char dummy;
  fprintf(stderr, "Usage: %s [-t transform_file] [-f image_file]+\n",
         programName);
  fprintf(stderr, "     [-d vrpn_sem_device] [-da vrpn_aligner_device]\n");
  fprintf(stderr, "Hit enter to quit\n");
  scanf("%c", &dummy);
  exit(0);
  return 0;
}


static int parseArgs(int argc, char **argv)
{
  int i = 1;
  while (i < argc) {
    printf("%s, ", argv[i]);
    if (strcmp(argv[i], "-f") == 0) {
       if (++i == argc) usage(argv[0]);
       printf("%s, ", argv[i]);
       planningImageNames[numPlanningImages] = new char[strlen(argv[i])+1];
       sprintf(planningImageNames[numPlanningImages], "%s", argv[i]);
       numPlanningImages++;
    } else if (strcmp(argv[i], "-t") == 0) {
       if (++i == argc) usage(argv[0]);
       sprintf(transformFileName, "%s", argv[i]);
    } else if (strcmp(argv[i], "-d") == 0) {
       if (++i == argc) usage(argv[0]);
       sprintf(semDeviceName, "%s", argv[i]);
       semDeviceSet = vrpn_TRUE;
    } else if (strcmp(argv[i], "-da") == 0) {
       if (++i == argc) usage(argv[0]);
       sprintf(alignerDeviceName, "%s", argv[i]);
       alignerDeviceSet = vrpn_TRUE;
    } else if (strcmp(argv[i], "-virtualacq") == 0) {
       virtualAcquisition = vrpn_TRUE;
    } else {
       usage(argv[0]);
    }
    i++;
  }

  printf("\n");
  return 0;
}

void handle_LeftWindowPos_change(int value, void *ud)
{
  PatternEditor *pe = (PatternEditor *)ud;
  int x = value + 20;
  int y = 50;
  pe->setWindowStartPosition(x, y);
}

void serverThreadStart(void *ud)
{
  ThreadData *threadData = (ThreadData *)ud;
  Semaphore *ps = threadData->ps;
  printf("releasing main thread\n");
  ps->v();

  while(!timeToQuit){
    if (internal_device_connection){
      internal_device_connection->mainloop();
      if (sem) {
        if (sem->numBufferedMessages() > 0) {
          // we can't disable tcl controls here through a Tclvar because
          // we can only use Tclvars in the main thread where all the other
          // tcl stuff is - instead we set one indirectly through this flag
          // this is important because we don't want the user pushing lots of
          // buttons and not seeing any results because the server is busy
          // executing a previous command
          needToDisableSEMControls = vrpn_TRUE; 
          sem->sendBuffer();
          needToDisableSEMControls = vrpn_FALSE;
        }
      }
    }
    if (sem_server) {
      sem_server->mainloop();
    }
    vrpn_SleepMsecs(10); // give the main thread a chance to do stuff
  }
}
