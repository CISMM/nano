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
#include "Tcl_Linkvar.h"
#include "Tcl_Netvar.h"
#include <nmb_Dataset.h>
#include <nmb_Decoration.h>
#include <nmb_Globals.h>
#include <nmb_Debug.h>

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

//#include "x_util.h"  // for x_set_scale()
//#include "microscape.h"  // for user_mode, mode_change, tcl_offsets,
                         // Arm_knobs, ...
#include "globals.h"
#include "tcl_tk.h"


//#define TCL_WIDGETS_FILE "russ_widgets.tcl"
#define TCL_MODE_FILE "mainwin.tcl"
#ifdef TRUE
#undef TRUE
#endif
#define TRUE (1)
#ifdef FALSE
#undef FALSE
#endif
#define FALSE (0)


static	Tcl_Interp *tk_control_interp;
static	int	old_user_mode;
//static	double	old_x_min_scale, old_x_max_scale;
static	int	controls_on = 0;
static  int  old_knobs[8];
static  int  knobs_set_from_c;

static void (* command_handler) (char *, vrpn_bool *, int);

/***********************************************************
 * here's a little function that will let outsiders accesss
 * the interpreter once it gets created in init_Tk_control_panels
 ***********************************************************/
Tcl_Interp *get_the_interpreter(void)
{
  return tk_control_interp;
}

void set_Tk_command_handler (void (* f) (char *, vrpn_bool *, int)) {
  command_handler = f;
}

/** Trace routine that handles updates to user_0_mode variable from Tcl */
char *handle_Tk_mode_change(ClientData /*clientData*/,
			    Tcl_Interp *interp, char *name1, char *name2, 
			    int /*flags*/)
{
	char	*cvalue;
	int	value;

	/* Look up the new value of the variable */
	cvalue = Tcl_GetVar2(interp, name1, name2, TCL_GLOBAL_ONLY);
	if (cvalue == NULL) {
		fprintf(stderr,"Warning!  Can't read %s from Tcl\n",name1);
	} else if (Tcl_GetInt(interp, cvalue, &value) != TCL_OK) {
		fprintf(stderr,"Warning!  %s not integer from Tcl\n",name1);
	}

	/* Set the user mode, and indicate a mode change, if it's different */
	if (old_user_mode != value) {
		user_mode[0] = value;
		old_user_mode = value;
		mode_change = 1;
	}

	// TCH - HACK, but it works
	decoration->user_mode = user_mode[0];

	return NULL;
}

char *handle_knob_change(ClientData /*clientData*/,
			 Tcl_Interp *interp, char *name1, char *name2, int /*flags*/)
{
	char	*cvalue;
	int value;
	int index;
	
	// this doesn't work - only shields knob[0]
	if (knobs_set_from_c) 
	{
	  knobs_set_from_c =0;
	  return NULL;
	  
	}
	
	/* Look up the new value of the variable */
	cvalue = Tcl_GetVar2(interp, name1, name2, TCL_GLOBAL_ONLY);
	if (cvalue == NULL)
	  fprintf(stderr,"Warning!  Can't read %s from Tcl\n",name1);
	else 
	{
	  if (Tcl_GetInt(interp, cvalue, &value) != TCL_OK)
		fprintf(stderr,"Warning!  Error reading %s(%s)\n",name1,name2);
	  else
	  {
	    Tcl_GetInt(interp, name2, &index);
	    if (index <=7 && index >=0)
	    {
	      tcl_offsets[index] += (float)(value - old_knobs[index])/100.0;
	      if (tcl_offsets[index] > 1.0)
		tcl_offsets[index] -= 1.0;
	      else if (tcl_offsets[index] < -1.0)
		tcl_offsets[index] += 1.0;
	      Arm_knobs[index] = ((float)value)/100.0;
// 	      printf("knob(%d) from %d to %d in tcl, offset: %g\n",index,
// 		     old_knobs[index], value,tcl_offsets[index]);
	      old_knobs[index] = value;
	    }
	  }
	}


	
	return NULL;
}

char *handle_surface_change(ClientData clientData,
	Tcl_Interp *interp, char *name1, char *name2, int flags)
{
  char	*cvalue;
  int   value;
 
  double minColor [3];
  double maxColor [3];

  /* Keep the compiler happy. */ 
  name1 = name1; name2 = name2; flags = flags; clientData = clientData;

  cvalue = Tcl_GetVar(interp, "minR", TCL_GLOBAL_ONLY);
  Tcl_GetInt(interp, cvalue, &value);
  minColor[0] = (double)value/255.0;
  cvalue = Tcl_GetVar(interp, "minG", TCL_GLOBAL_ONLY);
  Tcl_GetInt(interp, cvalue, &value);
  minColor[1] = (double)value/255.0;
  cvalue = Tcl_GetVar(interp, "minB", TCL_GLOBAL_ONLY);
  Tcl_GetInt(interp, cvalue, &value);
  minColor[2] = (double)value/255.0;
  graphics->setMinColor(minColor);

  cvalue = Tcl_GetVar(interp, "maxR", TCL_GLOBAL_ONLY);
  Tcl_GetInt(interp, cvalue, &value);
  maxColor[0] = (double)value/255.0;
  cvalue = Tcl_GetVar(interp, "maxG", TCL_GLOBAL_ONLY);
  Tcl_GetInt(interp, cvalue, &value);
  maxColor[1] = (double)value/255.0;
  cvalue = Tcl_GetVar(interp, "maxB", TCL_GLOBAL_ONLY);
  Tcl_GetInt(interp, cvalue, &value);
  maxColor[2] = (double)value/255.0;
  graphics->setMaxColor(maxColor);

  cvalue = Tcl_GetVar(interp, "polish", TCL_GLOBAL_ONLY);
  Tcl_GetInt(interp, cvalue, &value);
  graphics->setSpecularity(value);

  /* need to force a redraw of the entire surface */
  dataset->range_of_change.ChangeAll();
  
  return NULL;
}

char * handle_term_input (ClientData, Tcl_Interp * interp,
                          char * name1, char *, int)
{
	char	*cvalue;

	/* Look up the new value of the variable */
	cvalue = Tcl_GetVar(interp, name1, TCL_GLOBAL_ONLY);
	if (cvalue == NULL) 
	{
	  fprintf(stderr,"Warning!  Can't read %s from Tcl\n",name1);
	}
	else
	  /* note:  pointer to done flag is usually second argument */
	  /* but this is handled by setting user_mode[0] to -1  */
	{
	  if (command_handler)
	    (*command_handler)(cvalue, &dataset->done, strlen(cvalue));
	}

	return NULL;
}

/* Trace routine that handles updates to X window scale variables from Tcl */
// replaced with handle_x_value_change() in microscape.c and all the
// standard automatic TclLinkvar stuff
//char * handle_Tk_x_value_change

/** Initialize the Tk control panels and connect them up to the variables
 * they will be controlling */
int	init_Tk_control_panels (const char * tcl_script_dir,
                                vrpn_bool useOptimism)
{
	char    command[256];
	char	cvalue[100];
	char    index[2];
	Tk_Window       tk_control_window;
	int i, new_val;
	
	const double * minColor;
	const double * maxColor;


	VERBOSE(4, "  Initializing Tcl");
	tk_control_interp = Tcl_CreateInterp();

	printf("init_Tk_control_panels(): just created the tcl/tk interpreter\n");

	/* Start a Tcl interpreter */
	VERBOSE(4, "  Starting Tcl interpreter");
	if (Tcl_Init(tk_control_interp) == TCL_ERROR) {
		fprintf(stderr,
			"Tcl_Init failed: %s\n",tk_control_interp->result);
		return(-1);
	}

	/* Initialize Tk using the Tcl interpreter */
	VERBOSE(4, "  Initializing Tk");
	if (Tk_Init(tk_control_interp) == TCL_ERROR) {
		fprintf(stderr,
			"Tk_Init failed: %s\n",tk_control_interp->result);
		return(-1);
	}
	Tcl_StaticPackage(tk_control_interp, "Tk", Tk_Init, Tk_SafeInit);

	/* Initialize Tcl packages */
	if (Blt_Init(tk_control_interp) == TCL_ERROR) {
		fprintf(stderr,
			"Package_Init failed: %s\n",tk_control_interp->result);
		return(-1);
	}
	Tcl_StaticPackage(tk_control_interp, "Blt", Blt_Init, Blt_SafeInit);

#ifndef NO_ITCL
	if (Itcl_Init(tk_control_interp) == TCL_ERROR) {
		fprintf(stderr,
			"Package_Init failed: %s\n",tk_control_interp->result);
		return(-1);
	}
	if (Itk_Init(tk_control_interp) == TCL_ERROR) {
		fprintf(stderr,
			"Package_Init failed: %s\n",tk_control_interp->result);
		return(-1);
	}
	Tcl_StaticPackage(tk_control_interp, "Itcl", Itcl_Init, Itcl_SafeInit);
	Tcl_StaticPackage(tk_control_interp, "Itk", Itk_Init, (Tcl_PackageInitProc *) NULL);
#endif	
	/* Start a Tk mainwindow to hold the widgets */
	VERBOSE(4, "  Starting Tk mainwindow");
	tk_control_window = Tk_MainWindow(tk_control_interp);
	if (tk_control_window == NULL) {
		fprintf(stderr,"Tk can't make window: %s\n",
			tk_control_interp->result);
		return(-1);
	}

	/* The Tcl script that holds widget definitions is sourced from
	 * inside the main window script*/
	/* Load the Tcl script that handles main interface window
	 * and mode changes */
	VERBOSE(4, "  Loading Tcl script");
	sprintf(command, "source %s%s",tcl_script_dir,TCL_MODE_FILE);
	if (Tcl_Eval(tk_control_interp, command) != TCL_OK) {
		fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command,
			tk_control_interp->result);
		return(-1);
	}

	/* Link the variables that need linking and the callback routines
	 * to handle updates to the mode from either end. */
	VERBOSE(4, "  Tracing more Tcl variables");
	if (Tcl_TraceVar(tk_control_interp, "user_0_mode",
		TCL_TRACE_WRITES | TCL_GLOBAL_ONLY,
		handle_Tk_mode_change, (ClientData) NULL) != TCL_OK) {
		fprintf(stderr, "Tcl_TraceVar(%s) failed: %s\n", "user_0_mode",
			tk_control_interp->result);
		return(-1);
	}
	old_user_mode = user_mode[0];
	sprintf(cvalue,"%d",user_mode[0]);
	Tcl_SetVar(tk_control_interp, "user_0_mode", cvalue, TCL_GLOBAL_ONLY);

	/* setup callback for changes to tcl terminal input variable to */
	/* simulate an option entered by the keyboard */
	if (Tcl_TraceVar(tk_control_interp, "term_input",
		TCL_TRACE_WRITES | TCL_GLOBAL_ONLY,
		handle_term_input, (ClientData) NULL) != TCL_OK) {
		fprintf(stderr, "Tcl_TraceVar(%s) failed: %s\n", "term_input",
			tk_control_interp->result);
		return(-1);
	}

	
	/* Initialize the values of the knobs in tcl, before we
	 * setup callbacks. */
	for (i=0;i<8;i++) {
	  new_val = (int) (100 * Arm_knobs[i]);
	  old_knobs[i] = new_val;
	  sprintf(index,"%d",i);
	  sprintf(cvalue,"%d",new_val);
	  Tcl_SetVar2(tk_control_interp,"knobs",index,cvalue,TCL_GLOBAL_ONLY);
	}

	/* setup callback for changes to tcl knob variable */
	if (Tcl_TraceVar2(tk_control_interp, "knobs",NULL,
		TCL_TRACE_WRITES | TCL_GLOBAL_ONLY,
		handle_knob_change, (ClientData) NULL) != TCL_OK) {
		fprintf(stderr, "Tcl_TraceVar(%s) failed: %s\n", "knobs",
			tk_control_interp->result);
		return(-1);
	}

	/* setup callback for changes to lighting and material */
	/* this callback does nothing except force a redraw of the entire surface */
	/* you still need to link the tcl and c lighting variables */
	/* only works for the GL version currently XXX */
	if (Tcl_TraceVar2(tk_control_interp, "surface_changed",NULL,
		TCL_TRACE_WRITES | TCL_GLOBAL_ONLY,
		handle_surface_change, (ClientData) NULL) != TCL_OK) {
		fprintf(stderr, "Tcl_TraceVar(%s) failed: %s\n", "surface_change",
			tk_control_interp->result);
		return(-1);
	}

	minColor = graphics->getMinColor();
	maxColor = graphics->getMaxColor();

	/* initialize Tk variables for color settings */
	sprintf(cvalue, "%d", (int)minColor[0]);
	Tcl_SetVar(tk_control_interp,"minR",(char *) cvalue,TCL_GLOBAL_ONLY);
	sprintf(cvalue, "%d", (int)minColor[1]);
	Tcl_SetVar(tk_control_interp,"minG",(char *) cvalue,TCL_GLOBAL_ONLY);
	sprintf(cvalue, "%d", (int)minColor[2]);
	Tcl_SetVar(tk_control_interp,"minB",(char *) cvalue,TCL_GLOBAL_ONLY);
	sprintf(cvalue, "%d", (int)maxColor[0]);
	Tcl_SetVar(tk_control_interp,"maxR",(char *) cvalue,TCL_GLOBAL_ONLY);
	sprintf(cvalue, "%d", (int)maxColor[1]);
	Tcl_SetVar(tk_control_interp,"maxG",(char *) cvalue,TCL_GLOBAL_ONLY);
	sprintf(cvalue, "%d", (int)maxColor[2]);
	Tcl_SetVar(tk_control_interp,"maxB",(char *) cvalue,TCL_GLOBAL_ONLY);

	sprintf(cvalue, "%d", graphics->getSpecularity());
	Tcl_SetVar(tk_control_interp,"polish",(char *) cvalue,TCL_GLOBAL_ONLY);
	controls_on = 1;


	/* Initialize the Tclvar variables */
	VERBOSE(4, "  Calling Tclvar_init()");
        Tclnet_init(tk_control_interp, useOptimism);
	if (Tclvar_init(tk_control_interp)) {
		fprintf(stderr,"Tclvar_init failed.\n");
		return(-1);
	}

	
	return(0);
}


/** Check C variable that are associated with Tcl variables.  If the C
 * variables have changed, set the Tcl variables as a result. */
int	poll_Tk_control_panels(void)
{
	char	cvalue[100];
	char    index[2];
	
	int  i,new_val;
	
	if (!controls_on) return(-1);

	/* Check user mode and see if it has changed.  If so, set in Tcl */
	if (user_mode[0] != old_user_mode) {
		sprintf(cvalue,"%d",user_mode[0]);
		old_user_mode = user_mode[0];
		Tcl_SetVar(tk_control_interp, "user_0_mode", cvalue,
			TCL_GLOBAL_ONLY);
	}

	/* see if knobs have changed*/
	for (i=0;i<8;i++)
	{
	  new_val = (int) (100 * Arm_knobs[i]);
	  if (new_val != old_knobs[i]) 
	  {
// 	    printf("knob(%d) from %d to %d in C, offset: %g\n",
// 		   i,old_knobs[i],new_val,tcl_offsets[i]);
	    old_knobs[i] = new_val;
	    sprintf(index,"%d",i);
	    sprintf(cvalue,"%d",new_val);
	    Tcl_SetVar2(tk_control_interp,"knobs",index,cvalue,TCL_GLOBAL_ONLY);
	    // this doesn't work - only shields knob[0]
	    knobs_set_from_c = 1;
	  }
	}

	/* Call the Tclvar's main loop to allow them to send new values to
	 * the Tcl variables if they need to. */
	if (Tclvar_mainloop()) {return -1;}

	return 0;
}




