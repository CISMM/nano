// stuff for tcl/tk graphical user interface 
#include <tcl.h>
#include <tk.h>
#include <blt.h>
#include <Tcl_Linkvar.h>

#include <stdio.h>

#include "GL/glut.h"
#include "patternEditor.h"
#include "nmb_ImageTransform.h"
#include "transformFile.h"
#include "nmr_Util.h"

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

#define MAX_PLANNING_IMAGES 10
static nmb_ListOfStrings planningImageNameList;
static char **planningImageNames;
static int numPlanningImages = 0;
nmb_ImageList *planningImages = NULL;

static nmb_Image *liveImage;
static char transformFileName[256];
static TransformFile transformFile;
static Tclvar_int timeToQuit ("time_to_quit", 0);

PatternEditor *pe = NULL;
static Tcl_Interp *tk_control_interp;

int main(int argc, char **argv)
{
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

    TopoFile defaultTopoFileSettings;
    planningImages = new nmb_ImageList(&planningImageNameList,
                          (const char **)planningImageNames, numPlanningImages,
                          defaultTopoFileSettings);

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);

    

    pe = new PatternEditor();

    double matrix[16];
    double default_matrix[16] = {1.0, 0.0, 0.0, 0.0,
                                 0.0, 1.0, 0.0, 0.0,
                                 0.0, 0.0, 1.0, 0.0,
                                 0.0, 0.0, 0.0, 1.0};
    nmb_Image *currImage;

    for (i = 0; i < planningImages->numImages(); i++){
        currImage = planningImages->getImage(i);
        currImage->normalize();
        currImage->setWorldToImageTransform(default_matrix);
        // search for this image in the list of transformations we loaded
        if (transformFile.lookupImageTransformByName(
                          currImage->name()->Characters(),
                          matrix)) {
            currImage->setWorldToImageTransform(matrix);
            printf("setting world to image transform for %s\n",
                   currImage->name()->Characters());
        }
        pe->addImage(currImage);
    }

    pe->show();

    char *tcl_script_dir;
    char command[128];

    if ((tcl_script_dir=getenv("NM_TCL_DIR")) == NULL) {
         tcl_script_dir = "./";
    }
    init_Tk();
    Tclvar_init(tk_control_interp);

    // Hide the main window.
    sprintf(command, "wm withdraw .");
    TCLEVALCHECK(tk_control_interp, command);

    // source the litho_main.tcl file
    sprintf(command, "source %s%s",tcl_script_dir,"litho_main.tcl");
    if (Tcl_Eval(tk_control_interp, command) != TCL_OK) {
        fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command,
              tk_control_interp->result);
        return 0;
    }

    while(!timeToQuit){
      glutProcessEvents_UNC();
      while (Tk_DoOneEvent(TK_DONT_WAIT)) {};
      if (Tclvar_mainloop()) {
          fprintf(stderr, "main: Tclvar_mainloop error\n");
          return -1;
      }
    }

    return 0;
}

static int usage(char *programName)
{
  fprintf(stderr, "Usage: %s [-t transform_file] [-f image_file]+\n", 
         programName);
  exit(0);
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
        return 0;
}
