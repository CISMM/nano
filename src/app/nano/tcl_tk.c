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
			"Tcl_Init failed: %s\n",my_tk_control_interp->result);
		return(-1);
	}

	/* Initialize Tk using the Tcl interpreter */
	VERBOSE(4, "  Initializing Tk");
	if (Tk_Init(my_tk_control_interp) == TCL_ERROR) {
		fprintf(stderr,
			"Tk_Init failed: %s\n",my_tk_control_interp->result);
		return(-1);
	}
	Tcl_StaticPackage(my_tk_control_interp, "Tk", Tk_Init, Tk_SafeInit);

#ifndef NO_ITCL
	/* Initialize Tcl packages */
	if (Blt_Init(my_tk_control_interp) == TCL_ERROR) {
		fprintf(stderr,
			"Package_Init failed: %s\n",my_tk_control_interp->result);
		return(-1);
	}
	Tcl_StaticPackage(my_tk_control_interp, "Blt", Blt_Init, Blt_SafeInit);

	if (Itcl_Init(my_tk_control_interp) == TCL_ERROR) {
		fprintf(stderr,
			"Package_Init failed: %s\n",my_tk_control_interp->result);
		return(-1);
	}
	if (Itk_Init(my_tk_control_interp) == TCL_ERROR) {
		fprintf(stderr,
			"Package_Init failed: %s\n",my_tk_control_interp->result);
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
			my_tk_control_interp->result);
		return(-1);
	}

#ifdef VIEWER
        // Set variable to indicate that this is a Viewer only
        // interface. 
	sprintf(cvalue, "1");
	Tcl_SetVar(my_tk_control_interp,"viewer_only",(char *) cvalue,TCL_GLOBAL_ONLY);
#endif
	/* The Tcl script that holds widget definitions is sourced from
	 * inside the main window script*/
	/* Load the Tcl script that handles main interface window
	 * and mode changes */
	VERBOSE(4, "  Loading Tcl script");
        if ((tcl_script_dir[strlen(tcl_script_dir)]) == '/') {
            sprintf(command, "%s%s",tcl_script_dir,TCL_MODE_FILE);
        } else {
            sprintf(command, "%s/%s",tcl_script_dir,TCL_MODE_FILE);
        }            
	if (Tcl_EvalFile(my_tk_control_interp, command) != TCL_OK) {
		fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command,
			my_tk_control_interp->result);
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
        tcl_colormapRedraw();

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

/** Take a colormap and makes an color-bar image in Tcl with specified name,
    width, height.  c_min and c_max are values between 0 and 1 which can
    squash the colormap range, making the whole colormap appear in a small
    window in the larger image. Defaults perform no squashing. 
 */
int makeColorMapImage(ColorMap * cmap, char * name, int width, int height, 
                      float c_min , float c_max ) 
{
    Tk_PhotoImageBlock colormap;
    unsigned char *colormap_pixels = new unsigned char[ height * width * 3];
    Tk_PhotoHandle image;

    char command[200];
    // This code sets up the colormap bars displayed in the colormap
    // choice menu in tcl.
    colormap.pixelPtr = colormap_pixels;
    colormap.width = width;
    colormap.height = height;
    colormap.pixelSize = 3;
    colormap.pitch = width * 3;
    colormap.offset[0] = 0; colormap.offset[1] = 1; colormap.offset[2] = 2;

    // Make an image in Tcl based on the colormap.
    float ci;
    int r, g, b, a;
    for ( int i= 0; i < height; i++) {
        for ( int j = 0; j < width; j++ ) {
            ci = 1.0 - float(i)/height;
            if ( ci <  c_min ) ci = 0;
            else if ( ci > c_max ) ci = 1.0;
            else ci = (ci - c_min)/(c_max - c_min);
            
            cmap->lookup( ci, &r, &g, &b, &a);
            
            colormap_pixels[ i*width*3 + j*3 + 0] = (unsigned char)( r );
            colormap_pixels[ i*width*3 + j*3 + 1] = (unsigned char)( g );
            colormap_pixels[ i*width*3 + j*3 + 2] = (unsigned char)( b );
        }
    }
    // "image create" will replace any existing instance of the image. 
    sprintf (command, "image create photo %s", name);
    TCLEVALCHECK( get_the_interpreter(), command);
    image = Tk_FindPhoto( get_the_interpreter(), name );
    Tk_PhotoPutBlock( image, &colormap, 0, 0, width, height );

    return 0;
}

void tcl_colormapRedraw() {
    
    // Must match the colormap widget:
    const int colormap_width = 32, colormap_height = 256;

    // Draw a colormap bar if either colorPlaneName or colorMapName 
    // aren't "none"
    if (curColorMap &&
        ( ( strcmp( dataset->colorPlaneName->string(), "none") != 0 ) ||
          ( strcmp( dataset->colorMapName->string(), "none") != 0) ) ) {
        // color_max and color_min "squash" the color map image based
        // on tcl controls. 
        makeColorMapImage(curColorMap, "colormap_image", 
                          colormap_width, colormap_height, 
                          color_min, color_max);
    }
    else {
        ColorMap constmap;
        constmap.setConst(surface_r , surface_g, surface_b, 255);
        makeColorMapImage(&constmap, "colormap_image", 
                          colormap_width, colormap_height);
    }  
}
