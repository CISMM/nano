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
#include <Tcl_Interpreter.h>
#include <Tcl_Linkvar.h>
#include <Tcl_Netvar.h>
#include <nmb_Dataset.h>
#include <nmb_Decoration.h>
#include <nmb_Globals.h>
#include <nmb_Debug.h>
#include <nmb_TimerList.h>

#include <nmg_Graphics.h>
#include <nmg_Globals.h>

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


static	int	controls_on = 0;

/** Initialize the Tk control panels */
int	init_Tk_control_panels (const char * tcl_script_dir,
                                int collabMode,
                                nmb_TimerList * timer)
{
	char    command[256];
	
	VERBOSE(4, "  Initializing Tcl");
	Tcl_Interp * my_tk_control_interp = Tcl_Interpreter::getInterpreter();
        if (!my_tk_control_interp) return -1;
#ifdef NO_MSCOPE_CONNECTION
        // Set variable to indicate that this is a Viewer only
        // interface. 
	sprintf(command, "1");
	Tcl_SetVar(my_tk_control_interp,"viewer_only",&command[0],TCL_GLOBAL_ONLY);
#endif
#ifdef NO_PHANTOM
        // Set variable to indicate that the Phantom is disabled 
        // during compile.
	sprintf(command, "1");
	Tcl_SetVar(my_tk_control_interp,"no_phantom",&command[0],TCL_GLOBAL_ONLY);
#endif
#ifdef THIRDTECH
        // Set variable to indicate that the Phantom is disabled 
        // during compile.
	sprintf(command, "1");
	Tcl_SetVar(my_tk_control_interp,"thirdtech_ui",&command[0],TCL_GLOBAL_ONLY);
#endif

        // Set the microscope flavor
        if (nmb_MicroscopeFlavor == Asylum) {
          sprintf(command, "Asylum");
        } else if (nmb_MicroscopeFlavor == Topometrix) {
          sprintf(command, "Topometrix");
        } else {
          fprintf(stderr, "init_Tk_control_panels(): Unknown microscope flavor\n");
          return -1;
        }
	Tcl_SetVar(my_tk_control_interp,"microscopeflavor",&command[0],TCL_GLOBAL_ONLY);

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

