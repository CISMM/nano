// stuff for tcl/tk graphical user interface 
#include <tcl.h>
#include <tk.h>
#include <blt.h>

#ifndef NO_ITCL
#include <itcl.h>
#include <itk.h>
#endif

#include <Tcl_Linkvar.h>

#include <stdio.h>

#include "GL/glut_UNC.h"

#include "patternEditor.h"
#include "nmb_TransformMatrix44.h"
#include "transformFile.h"
#include "nmr_Util.h"

#include "nmr_Registration_Proxy.h"
#include "nmm_Microscope_SEM_Remote.h"
#include "nmm_Microscope_SEM_EDAX.h"
#include "controlPanels.h"

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
static int init_Tk();
// for some reason blt.h doesn't declare this procedure.
// I copied this prototype from bltUnixMain.c, but I had to add
// the "C" so it would link with the library.
extern "C" int Blt_Init (Tcl_Interp *interp);
extern "C" int Blt_SafeInit(Tcl_Interp *interp);

#define MAX_PLANNING_IMAGES 10
static char **planningImageNames;
static int numPlanningImages = 0;
nmb_ImageList *imageData = NULL;

static char transformFileName[256];

static vrpn_bool semDeviceSet = VRPN_FALSE, alignerDeviceSet = VRPN_FALSE;
static char semDeviceName[256];
static char alignerDeviceName[256];
static vrpn_bool virtualAcquisition = vrpn_FALSE;

static TransformFile transformFile;

static Tclvar_int timeToQuit ("time_to_quit", 0);

vrpn_Connection *local_connection;
PatternEditor *patternEditor = NULL;
nmr_Registration_Proxy *aligner = NULL;
nmm_Microscope_SEM_Remote *sem = NULL;
ControlPanels *controls = NULL;

nmm_Microscope_SEM_EDAX *sem_server = NULL;
vrpn_Connection *internal_device_connection = NULL;

static Tcl_Interp *tk_control_interp;

#ifdef V_GLUT
void nullDisplayFunc() { ; }
void nullIdleFunc() { ; }
#endif

int main(int argc, char **argv)
{

    // Initialize TCL/TK so that TclLinkvar variables link up properly
    char *tcl_script_dir;
    char command[128];

    local_connection = new vrpn_Synchronized_Connection(4511);

    if ((tcl_script_dir=getenv("NM_TCL_DIR")) == NULL) {
         tcl_script_dir = "./";
    }
    init_Tk();
    Tclvar_init(tk_control_interp);

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
#ifndef _WIN32
    glutCreateWindow("the program crashes without this window");
    glutDisplayFunc(nullDisplayFunc);
    glutIdleFunc(nullIdleFunc);
#endif
    // this must come after we initialize graphics
    patternEditor = new PatternEditor();

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
      sem = new nmm_Microscope_SEM_Remote("localSEM", 
                                          internal_device_connection);
    }


    // Now we can hook up the Tcl/Tk control panels 
    // to the parts that do the work
    controls = new ControlPanels(patternEditor, aligner, sem);


    // load the images specified on the command line
    TopoFile defaultTopoFileSettings;
    imageData = new nmb_ImageList(
                          controls->imageNameList(),
                          (const char **)planningImageNames, numPlanningImages,
                          defaultTopoFileSettings);

    double matrix[16];
    double default_matrix[16] = {0.001, 0.0, 0.0, 0.0,
                                 0.0, 0.001, 0.0, 0.0,
                                 0.0, 0.0, 1.0, 0.0,
                                 0.0, 0.0, 0.0, 1.0};
    double acqDistX, acqDistY;
    nmb_Image *currImage;

    for (i = 0; i < imageData->numImages(); i++){
        currImage = imageData->getImage(i);
        currImage->normalize();
        currImage->setWorldToImageTransform(default_matrix);
        // search for this image in the list of transformations we loaded
        if (transformFile.lookupImageTransformByName(
                          currImage->name()->Characters(),
                          matrix, acqDistX, acqDistY)) {
            currImage->setWorldToImageTransform(matrix);
            currImage->setAcquisitionDimensions(acqDistX, acqDistY);
            printf("setting world to image transform for %s\n",
                   currImage->name()->Characters());
        }
        patternEditor->addImage(currImage);
    }

    patternEditor->show();

    controls->setImageList(imageData);

    ImageViewer *image_viewer = ImageViewer::getImageViewer();

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

      if (sem) {
          sem->mainloop();
      }
      if (internal_device_connection){
          internal_device_connection->mainloop();
      }
      if (sem_server) {
          sem_server->mainloop();
      }
      if (aligner) {
          aligner->mainloop();
      }

      image_viewer->mainloop();

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

int init_Tk(){
        tk_control_interp = Tcl_CreateInterp();
        printf("init_Tk(): just created the tcl/tk interpreter\n");
        /* Start a Tcl interpreter */
        if (Tcl_Init(tk_control_interp) == TCL_ERROR) {
                fprintf(stderr, "Tcl_Init failed: %s\n",
                tk_control_interp->result);
                return -1;
        }
        /* Initialize Tk using the Tcl interpreter */
        if (Tk_Init(tk_control_interp) == TCL_ERROR) {
                fprintf(stderr, "Tk_Init failed: %s\n",
                tk_control_interp->result);
                return -1;
        }
        if (Blt_Init(tk_control_interp) == TCL_ERROR) {
                fprintf(stderr, "Package_Init failed: %s\n",
                tk_control_interp->result);
                return -1;
        }
#ifndef NO_ITCL

  Tcl_StaticPackage(tk_control_interp, "Blt", Blt_Init, Blt_SafeInit);

#ifndef _WIN32
  if (Itcl_Init(tk_control_interp) == TCL_ERROR) {
    fprintf(stderr, "Package_Init failed: %s\n",
          tk_control_interp->result);
    return -1;
  }
  if (Itk_Init(tk_control_interp) == TCL_ERROR) {
    fprintf(stderr, "Package_Init failed: %s\n",
          tk_control_interp->result);
    return -1;
  }
  Tcl_StaticPackage(tk_control_interp, "Itcl", Itcl_Init, Itcl_SafeInit);
  Tcl_StaticPackage(tk_control_interp, "Itk", Itk_Init, 
                    (Tcl_PackageInitProc *) NULL);
#endif
#endif
  return 0;
}
