/*===3rdtech===
  Copyright (c) 2000 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <tcl.h>
#include <tk.h>

#include <BCPlane.h>
#include <Tcl_Linkvar.h>
#include <Tcl_Netvar.h>
#include <nmb_Dataset.h>
#include <nmb_Decoration.h>
#include <nmb_Globals.h>
#include <nmb_Debug.h>
#include <nmb_TimerList.h>

#include <nmg_Graphics.h>
#include <nmg_Globals.h>

#include <blt.h>

#ifndef NO_ITCL
#include <itcl.h>
#include <itk.h>
#endif

// for some reason blt.h doesn't declare this procedure.
// I copied this prototype from bltUnixMain.c, but I had to add
// the "C" so it would link with the library. 
extern "C" int Blt_Init (Tcl_Interp *interp);
extern "C" int Blt_SafeInit(Tcl_Interp *interp);

#include "microscape.h"  // for user_mode, mode_change, tcl_offsets
#include "globals.h"
#include "tcl_tk.h"


#define TCL_MODE_FILE "mainwin.tcl"
#ifdef TRUE
#undef TRUE
#endif
#define TRUE (1)
#ifdef FALSE
#undef FALSE
#endif
#define FALSE (0)


static	Tcl_Interp *tk_control_interp = NULL;
static	int	controls_on = 0;


/***********************************************************
 * here's a little function that will let outsiders accesss
 * the interpreter once it gets created in init_Tk_control_panels
 ***********************************************************/
Tcl_Interp *get_the_interpreter(void)
{
  return tk_control_interp;
}

/** Initialize the Tk control panels */
int	init_Tk_control_panels (const char * tcl_script_dir,
                                int collabMode,
                                nmb_TimerList * timer)
{
	char    command[256];
	Tk_Window       tk_control_window;
	
	VERBOSE(4, "  Initializing Tcl");
	Tcl_Interp * my_tk_control_interp = Tcl_CreateInterp();

	VERBOSE(1,"init_Tk_control_panels(): just created the tcl/tk interpreter\n");

#if defined (_WIN32) && !defined (__CYGWIN__)
        if (Tcl_InitStubs(my_tk_control_interp, TCL_VERSION, 1) == NULL) {
            fprintf(stderr, "Non matching version of tcl and tk\n");
            return -1;
        }
#endif
	/* Start a Tcl interpreter */
	VERBOSE(4, "  Starting Tcl interpreter");
	if (Tcl_Init(my_tk_control_interp) == TCL_ERROR) {
		fprintf(stderr,
			"Tcl_Init failed: %s\n",Tcl_GetStringResult(my_tk_control_interp));
		return(-1);
	}

	/* Initialize Tk using the Tcl interpreter */
	VERBOSE(4, "  Initializing Tk");
	if (Tk_Init(my_tk_control_interp) == TCL_ERROR) {
		fprintf(stderr,
			"Tk_Init failed: %s\n",Tcl_GetStringResult(my_tk_control_interp));
		return(-1);
	}
	Tcl_StaticPackage(my_tk_control_interp, "Tk", Tk_Init, Tk_SafeInit);

	/* Initialize Tcl packages */
	if (Blt_Init(my_tk_control_interp) == TCL_ERROR) {
		fprintf(stderr,
			"Package_Init failed: %s\n",Tcl_GetStringResult(my_tk_control_interp));
		return(-1);
	}
	Tcl_StaticPackage(my_tk_control_interp, "Blt", Blt_Init, Blt_SafeInit);

#ifndef NO_ITCL
	if (Itcl_Init(my_tk_control_interp) == TCL_ERROR) {
		fprintf(stderr,
			"Package_Init failed: %s\n",Tcl_GetStringResult(my_tk_control_interp));
		return(-1);
	}
	if (Itk_Init(my_tk_control_interp) == TCL_ERROR) {
		fprintf(stderr,
			"Package_Init failed: %s\n",Tcl_GetStringResult(my_tk_control_interp));
		return(-1);
	}
	Tcl_StaticPackage(my_tk_control_interp, "Itcl", Itcl_Init, Itcl_SafeInit);
	Tcl_StaticPackage(my_tk_control_interp, "Itk", Itk_Init, (Tcl_PackageInitProc *) NULL);
#endif	
        // Check to see if we have a Tk main window.
	VERBOSE(4, "  Checking Tk mainwindow");
	tk_control_window = Tk_MainWindow(my_tk_control_interp);
	if (tk_control_window == NULL) {
		fprintf(stderr,"Tk can't get main window: %s\n",
			Tcl_GetStringResult(my_tk_control_interp));
		return(-1);
	}

#ifdef NO_MSCOPE_CONNECTION
        // Set variable to indicate that this is a Viewer only
        // interface. 
	sprintf(cvalue, "1");
	Tcl_SetVar(my_tk_control_interp,"viewer_only",&cvalue[0],TCL_GLOBAL_ONLY);
#endif
#ifdef NO_PHANTOM
        // Set variable to indicate that the Phantom is disabled 
        // during compile.
	sprintf(cvalue, "1");
	Tcl_SetVar(my_tk_control_interp,"no_phantom",&cvalue[0],TCL_GLOBAL_ONLY);
#endif

        // Tell tcl script what directory it lives in. 
        sprintf(command, "%s",tcl_script_dir);
	Tcl_SetVar(my_tk_control_interp,"tcl_script_dir",command,TCL_GLOBAL_ONLY);
	/* The Tcl script that holds widget definitions is sourced from
	 * inside the main window script*/
	/* Load the Tcl script that handles main interface window
	 * and mode changes */
	VERBOSE(4, "  Loading Tcl script");
        if ((tcl_script_dir[strlen(tcl_script_dir)-1]) == '/') {
            sprintf(command, "%s%s",tcl_script_dir,TCL_MODE_FILE);
        } else {
            sprintf(command, "%s/%s",tcl_script_dir,TCL_MODE_FILE);
        }
	if (Tcl_EvalFile(my_tk_control_interp, command) != TCL_OK) {
		fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command,
			Tcl_GetStringResult(my_tk_control_interp));
		return(-1);
	}

        /* Initialize the Tclvar variables */
        VERBOSE(4, "  Calling Tclvar_init()");
        Tclnet_init(my_tk_control_interp, collabMode, timer);
        if (Tclvar_init(my_tk_control_interp)) {
                fprintf(stderr,"Tclvar_init failed.\n");
                return(-1);
        }

        // Initialize the global tcl interpreter
        // Waiting til here makes sure the error_display functions get to use a
        // fully-initialized interpreter, and don't cause a seg-fault.
        tk_control_interp = my_tk_control_interp;

        return 0;
}


/** connect some C variables to tk variables */
int     init_Tk_variables ()
{
        // We have been initialized.
	controls_on = 1;

        // Initialize the color bar display. Needs inited interpreter. 
        //tcl_colormapRedraw();

	return(0);
}


/** Check C variable that are associated with Tcl variables.  If the C
 * variables have changed, set the Tcl variables as a result. */
int	poll_Tk_control_panels(void)
{
        // Check for initialization.
	if (!controls_on) {
          return -1;
        }

	/* Call the Tclvar's main loop to allow them to send new values to
	 * the Tcl variables if they need to. */
	if (Tclvar_mainloop()) {return -1;}

	return 0;
}

