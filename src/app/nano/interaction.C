 /*****************************************************************************
 *
    interaction.c - handles all interaction for user program
    
    
    Overview:
    	- handles all button presses for grabbing & flying
	- fly with left button;  grab with right
    
    Revision History:

    Author	    	Date	  Comments
    ------	    	--------  ----------------------------
    Aron Helser         02/97     Re-wrote doFeelLive, eliminated others
    Mark Finch		03/29/94  Adding Uzi Mode
    Rich Holloway	06/18/91  Changed to work w/ adlib
    Rich Holloway	06/10/91  Simplified more for vlib v3.0
    Rich Holloway	04/18/91  Ported to pxpl5 & simplified
    Rich Holloway	01/16/91  Initial version


   Developed at the University of North Carolina at Chapel Hill, supported
   by the following grants/contracts:
   
     DARPA #DAEA18-90-C-0044
     ONR #N00014-86-K-0680
     NIH #5-R24-RR-02170
   
 *
 *****************************************************************************/

#include <stdio.h>

/*#include "tracker.h"*/
#include <quat.h>

#include <BCPlane.h>
#include <nmb_Dataset.h>
#include <nmb_Decoration.h>
#include <nmb_Types.h>
#include <nmb_Globals.h>
#include <nmb_Debug.h>
#include <nmb_Line.h>
#include <Tcl_Linkvar.h>
#include <Tcl_Netvar.h>

#include <nmm_Globals.h>
#ifndef USE_VRPN_MICROSCOPE	// #ifndef #else #endif	Added by Tiger
#include <Microscope.h>
#else
#include <nmm_MicroscopeRemote.h>
#endif
#include <nmm_Types.h>  // for Blunt_result, enums

#include <nmg_GraphicsImpl.h>
#include <nmg_Globals.h>

#include <nmui_Util.h>
//#include <nmui_IFMode.h>
//#include <nmui_IFMode_FeelFromGrid.h>
//#include <nmui_IFMode_Fly.h>
//#include <nmui_IFMode_Light.h>
//#include <nmui_IFMode_Scale.h>

#include "normal.h"
#include "relax.h"
#include "butt_mode.h"
#include "mf.h"

#include "microscopeHandlers.h"
#include "globals.h"
#include "microscape.h"  // for lots of #defines
#include "interaction.h"

#include "vrpn_ForceDevice.h"

/***************************
 * Local Defines
 ***************************/

#if (!defined(X) || !defined(Y) || !defined(Z))
#define	X	(0)
#define	Y	(1)
#define	Z	(2)
#endif

#define YAW	X
#define PITCH	Y
#define ROLL	Z

#define MAX_MV_KNOB 	(3)
#define MIN_MV_KNOB	(4)
#define MAX_MAX_MV	(200.0)

// a scale to slow down the change in sweep width when
// phantom pen is rotated.
#define SWEEP_WIDTH_SCALE (0.05)

/* IS_AIM_MODE is true for modification modes (show aiming structure) */
#define	IS_AIM_MODE(m)	( 			\
		( (m) == USER_PULSE_MODE )	\
		||				\
		( (m) == USER_LINE_MODE )	\
		||				\
		( (m) == USER_SWEEP_MODE )	\
		||				\
		( (m) == USER_PLANE_MODE )	\
		||				\
		( (m) == USER_BLUNT_TIP_MODE )	\
		||				\
		( (m) == USER_COMB_MODE )	\
		||				\
		( (m) == USER_PLANEL_MODE )	\
		)

#define IS_CORN_MODE(m) (                       \
                ( (m) == USER_SWEEP_MODE )        \
                ||                              \
                ( (m) == USER_COMB_MODE )       \
                ||                              \
                ( (m) == USER_LINE_MODE )       \
                ||                              \
                ( (m) == USER_SERVO_MODE )      \
                )

#ifndef MAX
#define MAX(a,b) ((a)<(b)?(b):(a))
#endif

//------------------------------------------------------------------------
// Configure the force-display method. This is to allow us to turn off
// forces or else make the user feel from a flat plane rather than from the
// actual data set being displayed. These are to be used when we take the
// system to Orange High School to enable us to determine how much the
// actual feeling helps the students understand the surface.

Tclvar_int_with_button	config_haptic_enable("Enable Haptic", ".sliders", 1);
Tclvar_int_with_button	config_haptic_plane("Haptic from flat", ".sliders", 0);

/***************************
 * Mode of user operation
 ***************************/

// moved user_mode to globals.c

int	last_mode[NUM_USERS];		/* Previous mode of operation */

char    *user_hand_icons[] = {  (char *)"v_arrow",
				(char *)"myRoboHand",
				(char *)"vx_up_icon",
				(char *)"vx_down_icon",
				(char *)"mypushpin",
				(char *)"mylightning",
				(char *)"v_coord_sys",
				(char *)"myHand",
				(char *)"myHand",
				(char *)"mfHair",
				(char *)"myHand",
				(char *)"mfTip",
				(char *)"mfTip",
				(char *)"myHand",
				(char *)"mfTip"};

char    *MODE_SOUNDS[] = {      (char *)"fly_mode",
				(char *)"grab_world_mode",
				(char *)"scale_up_mode",
				(char *)"scale_down_mode",
				(char *)"servo_mode",
				(char *)"move_mode",
				(char *)"move_grid_mode",
				(char *)"move_mode",
				(char *)"move_mode",
				(char *)"floop",
				(char *)"floop",
				(char *)"pick_mode",
				(char *)"pick_mode",
				(char *)"pick_mode",
				(char *)"pick_mode"};


/******************************
 *	Force modifications enabled, parameters
 ******************************/

int	plane_line_set = 0;		/* 1 if plane_line initialized	*/


/******************************
 *      These are the pulse parameters and the information about the
 * current pending pulse request, if any.
 ******************************/

int     pulse_enabled = 0;              /* 1 if pulses are enabled, 0 if not */

/*parameters locking the width in sweep mode (qliu 6/29/95)*/
Tclvar_int lock_width("sweep_lock_pressed");

/*parameter locking the tip in sharp tip mode */
Tclvar_int xy_lock("xy_lock_pressed");

// Trigger button from the virtual button box in Tcl
static int	tcl_trigger_just_forced_on = 0;
static int	tcl_trigger_just_forced_off = 0;
static void	handle_trigger_change( vrpn_int32 val, void *userdata );
Tclvar_int	tcl_trigger_pressed("trigger_pressed",0, handle_trigger_change);

/**********
 * callback function for Commit button in tcl interface.
 **********/
static void handle_commit_change( vrpn_int32 val, void *userdata);
/**********
 * callback function for Cancel commit button in tcl interface.
 **********/
static void handle_commit_cancel( vrpn_int32 val, void *userdata);
/**********
 * callback function for PHANToM reset button in tcl interface.
 **********/
static void handle_phantom_reset( vrpn_int32 val, void *userdata);


/*********
 * Variables that are linked to buttons in the
 * tcl version of the button box.  They are tracked here to allow us to
 * run even when the normal button box is not operational.
 *********/

Tclvar_int	tcl_modify_pressed("modify_pressed");
Tclvar_int	tcl_commit_pressed("commit_pressed", 0 , handle_commit_change);
int             old_commit_pressed=0;
//Tclvar_int	tcl_switchlock_pressed("modify_pressed");
Tclvar_int	tcl_commit_canceled("cancel_commit", 0 , handle_commit_cancel);
Tclvar_int	tcl_phantom_reset_pressed("reset_phantom", 0, 
				handle_phantom_reset);

/*********
 * Functions defined in this file (added by KPJ to satisfy g++)...
 *********/
void dispatch_event(int, int, int);
int interaction(int bdbox_buttons[], double bdbox_dials[], int bPressed);
int qmf_lookat(q_type ,  q_vec_type,  q_vec_type,  q_vec_type);
int doLight(int, int);
int doFly(int, int);
int doScale(int, int, double);
void specify_surface_compliance(int, int);
void specify_surface_friction(int, int);
void specify_surface_adhesion(int, int);
void specify_surface_bumpsize(int, int);
void specify_surface_buzzamplitude(int, int);
void specify_point_friction(void);
void specify_point_adhesion(void);
void specify_point_bumpsize(void);
void specify_point_buzzamplitude(void);
int specify_directZ_force(int);
double touch_canned_from_plane(int, q_vec_type handpos);
double touch_flat_from_measurelines(int, q_vec_type handpos);
double touch_live_to_plane_fit_to_line(int, q_vec_type handpos);
int plane_norm(int);
int set_aim_line_color(float);
int meas_cross( float, float, float, float, float);
int doMeasure(int, int);

int doLine(int, int);
int doFeelFromGrid(int, int);
int doFeelLive(int, int);
int doSelect(int, int);
int doServoConstRes(int, int);
int doWorldGrab(int, int);
int doMeasureGridGrab(int,  int);
int add_new_hand(int, int);
int doPositionScanline(int, int);

//New stuff by Renee
//These variables are watched by Tcl sliders in the adhesion and friction
//parameters pop-up dialog windows.  Those are in dataset.tcl
Tclvar_float adhesion_constant("adhesion_constant", 12000, NULL, NULL);
Tclvar_float lateral_spring_K("lateral_spring_K", 5500, NULL, NULL);
Tclvar_float adhesion_peak("adhesion_peak", 0, NULL, NULL);
Tclvar_float adhesion_min("adhesion_min", 0, NULL, NULL);
Tclvar_float adhesion_decrease_per("adhesion_decrease_per", .03, NULL, NULL);

/**********
 * Callback function for the trigger button on the virtual button box
 * in Tcl.  It needs to force the button to be on when it is pressed
 * and force the button to be off when it is released.  This forcing
 * is done in the trigger handling code.  We just tell it here that it
 * should be done.  This callback should only occur when the change is
 * caused by the user pressing the Tcl control button, not if it was
 * changed by the user code.
 **********/


// NANOX
// TODO:  find meaningful defaults XXX

void handle_lightDir_change (vrpn_float64, void *);

TclNet_float tcl_lightDirX
     ("tcl_lightDirX", 0.0, handle_lightDir_change, NULL);
TclNet_float tcl_lightDirY
     ("tcl_lightDirY", 1.0, handle_lightDir_change, NULL);
TclNet_float tcl_lightDirZ
     ("tcl_lightDirZ", 0.5, handle_lightDir_change, NULL);

void handle_worldFromRoom_change (vrpn_float64, void *);

TclNet_float tcl_wfr_xlate_X
     ("tcl_wfr_xlate_X", 0.0, handle_worldFromRoom_change, NULL);
TclNet_float tcl_wfr_xlate_Y
     ("tcl_wfr_xlate_Y", 0.0, handle_worldFromRoom_change, NULL);
TclNet_float tcl_wfr_xlate_Z
     ("tcl_wfr_xlate_Z", 0.0, handle_worldFromRoom_change, NULL);
TclNet_float tcl_wfr_rot_0
     ("tcl_wfr_rot_0", 0.0, handle_worldFromRoom_change, NULL);
TclNet_float tcl_wfr_rot_1
     ("tcl_wfr_rot_1", 0.0, handle_worldFromRoom_change, NULL);
TclNet_float tcl_wfr_rot_2
     ("tcl_wfr_rot_2", 0.0, handle_worldFromRoom_change, NULL);
TclNet_float tcl_wfr_rot_3
     ("tcl_wfr_rot_3", 1.0, handle_worldFromRoom_change, NULL);
TclNet_float tcl_wfr_scale
     ("tcl_wfr_scale", 1.0, handle_worldFromRoom_change, NULL);

static void handle_trigger_change( vrpn_int32 val, void * )
{
    if (val) {
	tcl_trigger_just_forced_on = 1;
    } else {
	tcl_trigger_just_forced_off = 1;
    }
}

/**********
 * callback function for Commit button in tcl interface.
 * It must prevent commit from being pressed at the wrong time, 
 * and do the correct thing when commit is pressed in line mode, and 
 * select mode. The commit button also starts modification in FeelLive mode,
 * but that is taken care of in doFeelLive().
 **********/

static void handle_commit_change( vrpn_int32 , void *) // don't use val, userdata.
{

  //    printf("handle_commit_change called, commit: %d\n", (int)tcl_commit_pressed);
    // This handles double callbacks, when we set tcl_commit_pressed to
    // zero below.
    if (tcl_commit_pressed != 1) return;

    // only allow commit to be activated in selected modes.
    switch (user_mode[0]) {

    //if we are feeling before
    // a modify, (can be either freehand or line tool)
    case USER_PLANEL_MODE:
	// If we press commit while in Touch Live mode and using the
	// line tool, we tell the AFM to connect the dots with
	// modificaton force. The Slow Line tool goes to the first
	// point and waits.
	if ((microscope->state.modify.tool == LINE)||
	    (microscope->state.modify.tool == SLOW_LINE)) {
	    // set up a reference for convenience
	    Position_list & p = microscope->state.modify.stored_points;
	    // Find out if the list of points has at least two points in it
	    p.start();
	    if (p.curr() == NULL) {
		// no stored points - leave
		tcl_commit_pressed = 0;	
		old_commit_pressed = 0;
		// Treat it as a canceled operation.
		tcl_commit_canceled =1;
		return;
	    } else if (p.peekNext() == NULL) {
		// less than 2 points - clean up and leave
		tcl_commit_pressed = 0;	
		old_commit_pressed = 0;
		// Treat it as a canceled operation.
		tcl_commit_canceled =1;
		return;
	    }
	    /* Do a poly-line modification!!! */
	    // Wait for tip to get to starting position
	    Point_value *value =
		microscope->state.data.inputPoint->getValueByPlaneName
                            (dataset->heightPlaneName->string());
	    if (value == NULL) {
		fprintf(stderr, "handle_commit_change():  "
                                "could not get input point!\n");
		return;
	    }
	    printf("handle_commit_change: points in list, doing modify.\n");
	    microscope->TakeFeelStep(p.currX(), p.currY(), value, 1);

	    double x, y;
	    Position * currPt;
	    Position * prevPt;
	
	    // sleep to allow piezo relaxation
	    sleep(2);
	    //printf("handle_commit_change: peizo relax done\n");
	    // start modification force!
	    microscope->ModifyMode();

	    // Wait for the mode to finish (XXX should wait for response)
	    sleep(1);
	    //printf("handle_commit_change: in modify mode.\n");

	    // Slow line tool doesn't do the modification now, it
	    // waits for the user to press Play or Step controls.
	    if (microscope->state.modify.tool == SLOW_LINE) {
	      microscope->state.modify.slow_line_committed = VRPN_TRUE;
	      // Call this function to initialize the slow_line tool
	      init_slow_line(microscope);
	      return;
	    }
	    p.next();
	    // Draw the first line 
	    currPt = p.curr();
	    prevPt = p.peekPrev();
	    x = prevPt->x();
	    y = prevPt->y();
	    // If we are in sweep mode, set the angle "yaw" correctly
	    // so the sweep is perpendicular to the line we draw.
	    microscope->state.modify.yaw = atan2((currPt->y() - y),
						 (currPt->x() - x)) - M_PI_2;
	
	    // Draw a line between prevPt and currPt
	    //printf("handle_commit_change: line %f %f to %f %f\n", 
	    //       x, y, currPt->x(), currPt->y());
	    microscope->DrawLine(x, y, currPt->x(), currPt->y());
	

	    //start with the second point, so previous point is valid.
	    for (p.next(); p.notDone(); p.next()) {
		// step from the previous point to the current point.
		currPt = p.curr();
		prevPt = p.peekPrev();
		x = prevPt->x();
		y = prevPt->y();
		// If we are in sweep mode, set the angle "yaw" correctly
		// so the sweep is perpendicular to the line we draw.
		// Also draw an arc from the previous line to the current line. 
		if (microscope->state.modify.style == SWEEP) {
		    float oldyaw = microscope->state.modify.yaw;
		    microscope->state.modify.yaw =
                           atan2((currPt->y() - y),
				 (currPt->x() - x)) - M_PI_2;
		    // only draw an arc if this is an outside turn. 
                    microscope->DrawArc(x, y, oldyaw,
                                        microscope->state.modify.yaw);
		}
		// Draw a line between prevPt and currPt
		// Wait until we get the report before we send the next line
		//printf("handle_commit_change: line %f %f to %f %f\n", 
		//	   x, y, currPt->x(), currPt->y());
		microscope->DrawLine(x, y, currPt->x(), currPt->y());
	    }
	
	    // delete stored points (so we don't use them again!)
	    // get rid of the rubber-band line
	    graphics->emptyPolyline();
	    p.start();
	    while(p.notDone()) {
		//printf("commit - remove func: %d\n", (p.curr())->iconID());
		p.del();
	    }

	    // resume normal scanning operation of AFM
	    printf("handle_commit_change: done modifying, resuming scan.\n");
	    microscope->ResumeScan();
	    // All done - turn off commit button
	    tcl_commit_pressed = 0;	
	    old_commit_pressed = 0;
	}
	//if we aren't using line tool, don't change commit button's value,
	// because it's handled below in doFeelLive()
	// (allows user to switch between touch and modify, without
	// scanning in between.)

	break;
    case USER_SERVO_MODE: // user is in Select mode
	// set the scan region based on the last one specified by the user. 
	if ( microscope->state.select_region_rad > 0.01 ) {  
	    // This comparison sets the smallest possible scan region 
	    // to be 0.01 nm - 0.1 Angstrom. Ought to be safe. 
	    // only do something if region has been specified.
	    printf ( "Setting region, size %f\n", microscope->state.select_region_rad);
	    // here's how the region is specified. 
	    //x_min = centerx - rad;
	    //x_max = centerx + rad;
	    //y_min = centery - rad;
	    //y_max = centery + rad;
	    
	    microscope->SetRegionNM(
	        microscope->state.select_center_x - microscope->state.select_region_rad,
		microscope->state.select_center_y - microscope->state.select_region_rad,
		microscope->state.select_center_x + microscope->state.select_region_rad,
		microscope->state.select_center_y + microscope->state.select_region_rad);

	}
	//Clear the var so we know we made a change. 
	microscope->state.select_region_rad = 0.0;
	
	// All done - turn off commit button
	tcl_commit_pressed = 0;
	old_commit_pressed = 0;

	break;
    default:
	tcl_commit_pressed = 0;	
	old_commit_pressed = 0;
	return;
    }
}

/**********
 * callback function for Cancel commit button in tcl interface.
 * This button backs out of an operation, instead of commiting it.
 * In polyline mode - it clears any saved points. 
 * In select mode - it sets the region invalid, and clears its icon.
 **********/

static void handle_commit_cancel( vrpn_int32, void *) // don't use val, userdata.
{
    //printf("handle_commit_cancel called, cancel: %d\n", (int)tcl_commit_canceled);
    // This handles double callbacks, when we set tcl_commit_canceled to
    // zero below.
    if (tcl_commit_canceled != 1) return;

    // only allow cancel to be activated in selected modes.
    switch (user_mode[0]) {

    //if we are feeling before
    // a modify, (can be any tool: freehand, line, constrained freehand, ...)
    case USER_PLANEL_MODE:
	{
	Position_list & p = microscope->state.modify.stored_points;
	if (!p.empty()) {
	    // delete stored points (so we don't use them again!)
	    // get rid of the rubber band line.
	   //printf("Clearing stored polyline points.\n");
	    graphics->emptyPolyline();

	    p.start();
	    while(p.notDone()) {
		//printf("cancel - remove func: %d\n", (p.curr())->iconID());
		// get rid of the icons marking the line endpoints
 		p.del();  //this moves the pointer forward to the next point.
 	    }

	    // For CONSTR_FREEHAND tool:
	    // Turn off constraint line, if it was on. Otherwise next
	    // phantom button press and release will do the wrong thing.
	    if (microscope->state.modify.constr_line_specified) {
	       microscope->state.modify.constr_line_specified = VRPN_FALSE;
	    }
	    // For SLOW_LINE tool: We are no longer sitting still,
	    // waiting for the user - we're about to resume the scan.
	    microscope->state.modify.slow_line_committed = VRPN_FALSE;

	    fprintf(stderr, "handle_commit_cancel: Aborting modify, resuming scan.\n");
	}
	// I think we should always resume scan - if the user hits "cancel"
	// that should mean "start over", so we always begin scanning again.
	microscope->ResumeScan();
	}
	break;
    case USER_SERVO_MODE:
	//Clear the var so we know we shouldn't use this region. 
	microscope->state.select_region_rad = 0.0;
	
	// clear the icon for the region as well, by making it small.
	// Then if the user drags again, it will re-appear.
	graphics->positionRubberCorner(microscope->state.xMin,
			   microscope->state.yMin,
			   microscope->state.xMin,
			   microscope->state.yMin);
	break;
    default:
	break;
    }
    // I think that a cancel should always turn off the commit button...
    tcl_commit_pressed = 0;	
    old_commit_pressed = 0;

    tcl_commit_canceled = 0;
}


static void handle_phantom_reset( vrpn_int32, void *) // don't use val, userdata.
{
    if (tcl_phantom_reset_pressed != 1) return;

    if (reset_phantom())
	fprintf(stderr, "Error: could not reinitialize phantom\n");

    tcl_phantom_reset_pressed = 0;
}

/*****************************************************************************
 *
   dispatch_event - dispatch the given event based on the current mode and user
 *
 *****************************************************************************/

void	dispatch_event(int user, int mode, int event)
{
    int ret = 0;

    switch(mode) {
	case USER_LIGHT_MODE:
		ret = doLight(user,event);
		break;
	case USER_FLY_MODE:  // inaccessible but not obsolete
		ret = doFly(user,event);
		break;
	case USER_MEASURE_MODE:
		ret = doMeasure(user,event);
		break;
	case USER_LINE_MODE:  // no longer directly called?
		ret = doLine(user,event);
		break;
	case USER_PLANE_MODE:
		ret = doFeelFromGrid(user,event);
		break;
	case USER_PLANEL_MODE:
	   // CONSTR_FREEHAND tool uses doLine first, then doFeelLive
	   // after it's constraint line is specified.
	   if (microscope->state.modify.tool == LINE) {
	       ret = doLine(user, event);
	   } else if ((microscope->state.modify.tool == CONSTR_FREEHAND) && 
		      (!microscope->state.modify.constr_line_specified)) {
	       ret = doLine(user, event);
	   } else if (microscope->state.modify.tool == SLOW_LINE) {
	       if (microscope->state.modify.slow_line_committed == VRPN_TRUE) {
		   ret = doWorldGrab(user,event);
		   // Do something useful, like move the surface view. We
		   // don't want the phantom to do anything with the AFM
		   // tip while the user is using the PLAY and STEP
		   // buttons to move the tip.
	       } else {
		   ret = doLine(user, event);
		   // User is specifying the line to step along.
	       }
	   } else {
	       ret = doFeelLive(user,event);
	   }
	   ret = 0;	// Keep from stopping if mode fails
		break;
	case USER_SCANLINE_MODE:
	   	ret = doPositionScanline(user, event);
	   	break;
	case USER_SCALE_UP_MODE:
		ret = doScale(user,event,0.96);
		break;
	case USER_SCALE_DOWN_MODE:
		ret = doScale(user,event,1.04);
		break;
	case USER_SERVO_MODE:  // change size of scan area
		ret = doSelect(user,event);
		break;
	case USER_GRAB_MODE:
		ret = doWorldGrab(user,event);
		break;
	case USER_MEAS_MOVE_MODE:  // obsolete?
		ret = doMeasureGridGrab(user,event);
		break;
	case USER_CENTER_TEXTURE_MODE:  // mouse control only!
		break;
	default:
	    fprintf(stderr,"dispatch_event(): Event not implemented\n");
	    break;
    }

    if (ret) {
	fprintf(stderr,"dispatch_event(): Handler failed\n");
	dataset->done = VRPN_TRUE;
    }
}


/*****************************************************************************
 *
   interaction - handles force level changes from knob box,
                 handles button presses from Phantom or other device 
		         based on current user mode.
		 Direct action presses are handled right here.
		 Otherwise, change the "hand" icon and call dispatch_event 
		 to do the real work.
 *
 *****************************************************************************/

int interaction(int bdbox_buttons[], double bdbox_dials[], 
		int phantButtonPressed)
{
    int		user;
    int i;
    /* eventList is static so unsupported buttons/dials will be init-ed and
    ** stay 0 (no event) */
    static int	lastButtonsPressed[BDBOX_NUMBUTTONS];
    static int  startup=0;
    static int	lastTriggerButtonPressed = 0;
    static int  eventList[BDBOX_NUMBUTTONS+1];
    static int  lastTriggerEvent = NULL_EVENT;
    static float old_mod_tap_force = -1;
    static float old_mod_con_force = -1;
    static float old_img_tap_force = -1;
    static float old_img_con_force = -1;

    if(startup==0){
	for (i = 0; i < BDBOX_NUMBUTTONS; i++) lastButtonsPressed[i]=0;
	startup=1;
    }

    // NULL_EVENT=0, PRESS_EVENT=1, RELEASE_EVENT=2, HOLD_EVENT=3 in microscape.h
    // compute events for button box buttons
    if (buttonBox){
	for (i = 0; i < BDBOX_NUMBUTTONS; i++){
	    if (!lastButtonsPressed[i]){
		if (bdbox_buttons[i]){
			eventList[i] = PRESS_EVENT;
			lastButtonsPressed[i]=1;
		}
		else eventList[i] = NULL_EVENT;
	    }
	    else {
		if (bdbox_buttons[i]) eventList[i] = HOLD_EVENT;
		else{ eventList[i] = RELEASE_EVENT;
			lastButtonsPressed[i]=0;
		}
	    }
	}
    }

VERBOSE(4, "  Entering interaction() loop.");
/*
 * handle button events for each user-  do each handler independent of
 *  of the other so that grabbing and flying can be done together
 */
for ( user = 0; user < NUM_USERS; user++ ) {
	int             used_events;
	int		triggerButtonPressed = 0; 

	// get value for trigger button event
	// first check phantom button, then mouse button, then button box 
	// trigger, tcl_trigger
	if (phantButton && using_phantom_button) {
		triggerButtonPressed = phantButtonPressed;
	} else if (using_mouse3button) {
		triggerButtonPressed = mouse3button;
	} else if (buttonBox) {
		triggerButtonPressed = bdbox_buttons[TRIGGER_BT];
	}
	if (tcl_trigger_just_forced_on) {
	    triggerButtonPressed = 1;
	    tcl_trigger_just_forced_on = 0;
	}
	if (tcl_trigger_just_forced_off) {
	    triggerButtonPressed = 0;
	    tcl_trigger_just_forced_off = 0;
	}

	if (!lastTriggerButtonPressed){
	    if (triggerButtonPressed) eventList[TRIGGER_BT] = PRESS_EVENT;
	    else eventList[TRIGGER_BT] = NULL_EVENT;
	} else {
	    if (triggerButtonPressed) eventList[TRIGGER_BT] = HOLD_EVENT;
	    else eventList[TRIGGER_BT] = RELEASE_EVENT;
	}
	lastTriggerButtonPressed = triggerButtonPressed;

	// If the user has forced a trigger, then make sure that the
	// button is pressed or held until they release it by pressing
	// the tcl control again.
	if ( (tcl_trigger_pressed) && (!tcl_trigger_just_forced_on) &&
	     (eventList[0] != RELEASE_EVENT) ) {
		// normally we would pass through a PRESS_EVENT here
		// but some things (like GRAB mode) will not handle
		// a HOLD_EVENT correctly if it is not preceded by
		// a PRESS event (AAS)
		if (eventList[0] != HOLD_EVENT) {
		    dispatch_event(user, user_mode[user], eventList[0]);
		}
		eventList[0] = HOLD_EVENT;
	}
	if (tcl_trigger_just_forced_on) {
	  switch (eventList[0]) {
	    case NULL_EVENT:	eventList[0] = PRESS_EVENT; break;
	    case PRESS_EVENT: break;
	    case HOLD_EVENT: break;
	    case RELEASE_EVENT: eventList[0] = HOLD_EVENT; break;
	  }
	  tcl_trigger_just_forced_on = 0;
	}
	if (tcl_trigger_just_forced_off) {
	  // Don't repeat a release event, but if there was not one
	  // last time, generate one for a NULL event.  If the button
	  // has just been pressed somewhere else, then cancel the
	  // release and turn it into a hold event.

	  switch (eventList[0]) {
	    case NULL_EVENT:	{if (lastTriggerEvent == NULL_EVENT) 
				eventList[0] = RELEASE_EVENT; } break;
	    case PRESS_EVENT: eventList[0] = HOLD_EVENT; break;
	    case HOLD_EVENT: break;
	    case RELEASE_EVENT: break;
	  }
	  tcl_trigger_just_forced_off = 0;
	}

	// Copy the current state of the trigger (whatever caused it)
	// into the tcl variable, so it will track.
	if ((eventList[0]== PRESS_EVENT) || (eventList[0]== HOLD_EVENT)) {
          // TCH 6 Oct 99 - cut down on operator = () calls
          if (tcl_trigger_pressed != 1) {
		tcl_trigger_pressed = 1;
          }
	} else {
          // TCH 6 Oct 99 - cut down on operator = () calls
          if (tcl_trigger_pressed != 0) {
		tcl_trigger_pressed = 0;
          }
	}
	// copy dial values to "Arm_knobs" - 
	for (i = 0; i < BDBOX_NUMDIALS; i++)
	{
	  // Offset of 0.5? that's confusing, so I'm taking it out.
	  // Yeah, it's confusing, but it makes the default value of
	  // all the knobs be 50, instead of 0, so the phantom feels
	  // right. So leave it in.
	  Arm_knobs[i] = bdbox_dials[i] + tcl_offsets[i] + 0.5;
	  //Arm_knobs[i] = bdbox_dials[i] + tcl_offsets[i];
	    if (Arm_knobs[i] > 1.0)
	    	Arm_knobs[i] -= 1.0;
	    else if (Arm_knobs[i] < 0.0)
		Arm_knobs[i] += 1.0;
	}

    /*******************************************************************/
    /** Here is where the vtk update occurs.  Note that if the A/D    **/
    /** device events are not "used" by the control panel (i.e. if    **/
    /** the buttons don't affect anything in the control panel), then **/
    /** the vtk_update_control_panel() returns VTK_NO, and the events **/
    /** can be legitimately used to perform other functions.          **/
    /*******************************************************************/
    if (do_cpanels) {
	    /* check the pulse control panel */
	    if (pulse_enabled) {
	    } else {
		used_events = VTK_NO;
	    }

	    if ( used_events == VTK_NO ) {      /* Okay to use events */
	    }

	    /* check the force control panel */
	    if (used_events == VTK_NO) {
	      if (microscope->state.modify.modify_enabled) {
	      } else {
		used_events = VTK_NO;
	      }
	    }

	    /* Check the std dev control panel */
	    if ( used_events == VTK_NO ) {      /* Okay to use events */
	    }
    } else {
	    used_events = VTK_NO;
    }

    if ( used_events == VTK_NO ) {	/* Okay to use events */
	float	force;			/* force level [0..1] */

	// Make sure the old_* static variable are initialized correctly.
	if( old_mod_con_force == -1) {
	  old_mod_con_force =Arm_knobs[MOD_FORCE_KNOB];
	  old_mod_tap_force =Arm_knobs[MOD_FORCE_KNOB];
	  old_img_con_force =Arm_knobs[IMG_FORCE_KNOB];
	  old_img_tap_force =Arm_knobs[IMG_FORCE_KNOB];
	}

	/* check if there's been a force level change from the knobs 
	** if so, store the new level so it will get sent to the microscope 
	** Notice it depends on which mode we're in, tapping or contact,
	** AND whether we are imaging or modifying.
	**/
	force = Arm_knobs[MOD_FORCE_KNOB];
	if( microscope->state.modify.mode == CONTACT ) {
 	    if( force != old_mod_con_force ) {
		//printf("Changing setpoint based on mod force knob\n");
		microscope->state.modify.setpoint = microscope->state.modify.setpoint_min +
		    force * (microscope->state.modify.setpoint_max - microscope->state.modify.setpoint_min);
		old_mod_con_force = force;
	    }
	} else if ( microscope->state.modify.mode == TAPPING ) {
 	    if( force != old_mod_tap_force ) {
		//printf("Changing amplitude based on mod force knob\n");
		microscope->state.modify.amplitude = microscope->state.modify.amplitude_min +
		    force * (microscope->state.modify.amplitude_max - microscope->state.modify.amplitude_min);
		old_mod_tap_force = force;
	    }
	}

	force = Arm_knobs[IMG_FORCE_KNOB];
	if( microscope->state.image.mode == CONTACT ) {
 	    if( force != old_img_con_force ) {
		//printf("Changing setpoint based on image force knob\n");
		microscope->state.image.setpoint =
                  microscope->state.image.setpoint_min +
		    force * (microscope->state.image.setpoint_max -
                             microscope->state.image.setpoint_min);
		old_img_con_force = force;
	    }
	} else if ( microscope->state.image.mode == TAPPING ) {
 	    if( force != old_img_tap_force ) {
		//printf("Changing amplitude based on image force knob\n");
		microscope->state.image.amplitude =
                  microscope->state.image.amplitude_min +
		    force * (microscope->state.image.amplitude_max -
                             microscope->state.image.amplitude_min);
		old_img_tap_force = force;
	    }
	}

	/* Check for immediate action button presses
	**
	** Center command is a special case
	**/
	if( PRESS_EVENT == eventList[CENT_BT] ) {
	  center();
		}

	/* Clear pulses
	**/
	if(PRESS_EVENT == eventList[CLEAR_BT] ) {
		handleCharacterCommand("C", &dataset->done, 1);
	}

	/* Select All (maximum scan range)
	**/
	if(PRESS_EVENT == eventList[ALL_BT] ) {
		handleCharacterCommand("A", &dataset->done, 1);
	}

	/* code dealing with locking the sweep width (qliu 6/29/95)*/
	if(PRESS_EVENT == eventList[LOCK_BT] )
	  lock_width=1;	
	if(RELEASE_EVENT == eventList[LOCK_BT] )
	  lock_width=0;

        /* code dealing with locking the tip in sharp tip mode */
	/* locks the tip in one place so you can take repeated measurements */
        if( PRESS_EVENT == eventList[XY_LOCK_BT] )
          xy_lock =1;
        if( RELEASE_EVENT ==eventList[XY_LOCK_BT] )  
          xy_lock =0;

	/* Handle a "commit" or "cancel" button press/release on
	 * the real button box by causing a callback to
	 * the tcl routines */
        if( PRESS_EVENT == eventList[COMMIT_BT] ) {
	    //printf("Commit from button box.\n");
	    // causes "handle_commit_change" to be called
	    tcl_commit_pressed  = !tcl_commit_pressed; 
	}
        if( PRESS_EVENT == eventList[CANCEL_BT] ) {
	    //printf("Cancel from button box.\n");
	    // causes "handle_commit_cancel" to be called
	    tcl_commit_canceled = !tcl_commit_canceled; 
	}
	if( PRESS_EVENT == eventList[PH_RSET_BT] ) {
	    // causes "handle_phantom_reset" to be called
	    tcl_phantom_reset_pressed = 1;
	}
	// testing for the remaining buttons on the button box.
        if( PRESS_EVENT == eventList[NULL1_BT] )
            printf("NULL1 from button box.\n");
        if( PRESS_EVENT == eventList[NULL2_BT] ) 
	    printf("NULL2 from button box.\n");
        if( PRESS_EVENT == eventList[NULL3_BT] ) 
	    printf("NULL3 from button box.\n");
        if( PRESS_EVENT == eventList[NULL4_BT] ) 
	    printf("NULL4 from button box.\n");
        if( PRESS_EVENT == eventList[NULL5_BT] ) 
	    printf("NULL5 from button box.\n");
	if( PRESS_EVENT == eventList[NULL6_BT] )
	    printf("NULL6 from button box.\n");
        if( PRESS_EVENT == eventList[NULL7_BT] ) 
	    printf("NULL7 from button box.\n");
	    
	/* see if the user changed modes using a button
	**/
	mode_change |= butt_mode(eventList, user_mode+user);
	// also check to see if the modify style has changed -
	// can affect hand icon as well.
	if (user_mode[user] == USER_PLANEL_MODE)
	    mode_change |= microscope->state.modify.style_changed;

	/* If there has been a mode change, clear old stuff and set new mode */
	if (mode_change) {
	    //fprintf(stderr, "Mode change.\n");
	    // If the user was going to go into a disabled mode, change them
	    // into GRAB mode instead.  Some modes are currently not
	    // implemented or not used.
		if ((user_mode[user] == USER_COMB_MODE)||
		    (user_mode[user] == USER_SWEEP_MODE)||
		    (user_mode[user] == USER_BLUNT_TIP_MODE)||
		    (user_mode[user] == USER_PULSE_MODE))
		    {
			fprintf(stderr,"Warning -- mode disabled (entering grab mode instead)\n");
			user_mode[user] = USER_GRAB_MODE;
		}

		/* If the user is in HOLD_EVENT at the time of the
		 * change, we need to send an RELEASE_EVENT to the
		 * previous mode and an PRESS_EVENT to the new mode
		 * so that all setup/cleanup code is executed. */
		if (eventList[TRIGGER_BT] == HOLD_EVENT) {
		    dispatch_event(user, last_mode[user], RELEASE_EVENT);
		    dispatch_event(user, user_mode[user], PRESS_EVENT);
		}		    

VERBOSE(6, "    Calling graphics->setUserMode().");

		// Change icons to ones for this new mode.
		graphics->setUserMode(last_mode[user],
		                      user_mode[user],
                                      microscope->state.modify.style);

		/* Make the associated sound */
		// Taken out for now

		/* Clear the mode change flag */
		mode_change = 0;
		microscope->state.modify.style_changed = 0;
	}

VERBOSE(6, "    Calling dispatch_event().");

	/* Handle the current event, based on the user mode and user */
	dispatch_event(user, user_mode[user], eventList[0]);
	lastTriggerEvent = eventList[0];

    } /* End of excluded events due to control panel taking them */

    /* Last mode next time around is the current mode this time around */
    last_mode[user] = user_mode[user];
}
  // TCH - HACK but it works
  decoration->user_mode = user_mode[0];

VERBOSE(4, "  Done with interaction().");

return 0;
}	/* interaction */


/*=======================================================================
** qmf_lookat - produce an orientation quaternion that will line up the
**	viewing vector (+Z) from from to at, and the Y axis roughly along
**	up.
**-----------------------------------------------------------------------
**/
int 
qmf_lookat(q_type orient,  q_vec_type from,  q_vec_type at,  q_vec_type up )
{
   q_matrix_type	row_mat;
   double		len;

   /* if anything goes wrong, return the identity quat
   **/
   q_make( orient, 0.0, 0.0, 0.0, 0.0 );

   row_mat[0][3] = 
   row_mat[1][3] = 
   row_mat[2][3] = 
   row_mat[3][2] = 
   row_mat[3][1] = 
   row_mat[3][0] = 0.0;

   row_mat[3][3] = 1.0;

   /* Z (view) axis from from to at
   **/
   q_vec_subtract( row_mat[2], at, from );
   if( ( len = q_vec_magnitude( row_mat[2] ) ) < Q_EPSILON ){
	   fprintf( stderr, "qmf_lookat: from/at identical\n" );
	   return -1;
	   }
   q_vec_scale( row_mat[2], 1.0/len, row_mat[2] );

   /* X axis perpindicular to up and Z
   **/
   q_vec_cross_product( row_mat[0], up, row_mat[2] );
   if( ( len = q_vec_magnitude( row_mat[0] ) ) < Q_EPSILON ){
	   fprintf( stderr, 
		"qmf_lookat: up parallel to (at-from) identical\n" );
	   return -1;
	   }
   q_vec_scale( row_mat[0], 1.0/len, row_mat[0] );

   /* Y axis all that's left
   **/
   q_vec_cross_product( row_mat[1], row_mat[2], row_mat[0] );

   q_from_row_matrix( orient, row_mat );

   return 0;
   }
  
/*.......................................................................
**/

/*****************************************************************************
 *
   doLight - adjust the position of the light
 *
 *****************************************************************************/

int
doLight(int whichUser, int userEvent)
{
  //VectorType		lightpos; //, lightdir;
  q_vec_type            lightdir;
  q_vec_type		q_tmp;
  v_xform_type	worldFromPart;
  v_xform_type	PartFromWorld;
  q_type		q_room;

  q_matrix_type lightDirection;
  q_xyz_quat_type xyzQuat;

  v_get_world_from_hand(whichUser, &worldFromPart);

  /* Copy the xyz-quaternion portion of v_xform_type
   * into a q_xyz_quat_type to pass to 
   * q_xyz_quat_to_row_matrix
   */
  q_copy(xyzQuat.quat, worldFromPart.rotate);
  q_vec_copy(xyzQuat.xyz, worldFromPart.xlate);

  q_xyz_quat_to_row_matrix(lightDirection, &xyzQuat);

  /* Extract the -y-transformation from the matrix.  
   * This represents the direction that the light will
   * shine from.  The light ray out of the lighthand
   * cube is along the -y-axis, so the light needs to 
   * shine along the -y-axis in hand-space.  This transform
   * transforms the hand-space -y-direction to world-space.
   */
  q_tmp[X] = ( -lightDirection[1][0] );
  q_tmp[Y] = ( -lightDirection[1][1] );
  q_tmp[Z] = ( -lightDirection[1][2] );

  switch ( userEvent ) {

    case HOLD_EVENT:

	/* Rotate force from world space to room space */
        v_get_world_from_head(whichUser, &worldFromPart);
	v_x_invert( &PartFromWorld, &worldFromPart );

	q_copy(q_room, worldFromPart.rotate);
	q_invert(q_room, q_room);
	q_xform(q_tmp, q_room, q_tmp);

	lightdir[X] = -q_tmp[X]; 
	lightdir[Y] = -q_tmp[Y];
	lightdir[Z] = -q_tmp[Z];

        VectorNormalize( lightdir );

	/* position the light where the top of the user's hand is */

        graphics->setLightDirection(lightdir);

        // NANOX
        tcl_lightDirX = lightdir[X];
        tcl_lightDirY = lightdir[Y];
        tcl_lightDirZ = lightdir[Z];

	break;

    case PRESS_EVENT:
    case RELEASE_EVENT:
    default:
    	/* do nothing if no button pressed  */
	break;
    }

return 0;
}	/* doLight */

// NANOX
void handle_lightDir_change (vrpn_float64, void *) {
  q_vec_type lightdir;

  lightdir[X] = tcl_lightDirX;
  lightdir[Y] = tcl_lightDirY;
  lightdir[Z] = tcl_lightDirZ;

  graphics->setLightDirection(lightdir);
}

/*****************************************************************************
 *
   doFly - fly through the world when button is pressed
	 - scale motion based on user's size in the world
	   (Allows a person to fly faster when they are huge)
 *
 *****************************************************************************/

int
doFly(int whichUser, int userEvent)
{

switch ( userEvent )
    {

    case HOLD_EVENT:
	/* this will do flying at about 1 meter per second  */
	v_fly(whichUser, v_world.users.xforms[whichUser].scale / UPDATE_RATE );
	break;

    case PRESS_EVENT:
    case RELEASE_EVENT:
    default:
    	/* do nothing if no button pressed  */
	break;
    }

return 0;
}	/* doFly */

/*****************************************************************************
 *
   doScale - scale the world when button is pressed
 *
 *****************************************************************************/

int
doScale(int whichUser, int userEvent, double scale_factor)
{
    switch ( userEvent )
    {
      case HOLD_EVENT:
	v_scale_about_hand(whichUser,
		&v_world.users.xforms[whichUser], scale_factor);
        updateWorldFromRoom();
	break;

      case PRESS_EVENT:
      case RELEASE_EVENT:
      default:
    	/* do nothing if no button pressed  */
	break;
    }

return 0;
}	/* doScale */


/*****************************************************************************
 *
   specify_surface_compliance - ??????
 *
 *****************************************************************************/

void specify_surface_compliance(int x, int y)
{
    static int firstExecution = 1;
    double Kspring = Arm_knobs[FORCE_KNOB]*(MAX_K-MIN_K)+MIN_K;

    if (firstExecution)
        printf("Setting surface compliance...\n");
 
    BCPlane* plane = dataset->inputGrid->getPlaneByName
                      (compliancePlaneName.string());
    if (plane == NULL) {
	if (forceDevice)
        	forceDevice->setSurfaceKspring(Kspring);
	if (firstExecution) 
		printf("Kspring = %g\n", Kspring);
        firstExecution = 0;
        return;
    }

    if (compliance_slider_max != compliance_slider_min) {

        if (firstExecution)
            printf("Basing Kspring on compliance data set\n");

        Kspring = (plane->value(x,y) - compliance_slider_min) /
                 (compliance_slider_max - compliance_slider_min);
	printf("kspring: %f\n", Kspring);
        Kspring = min(1,Kspring);   /*click scale to be in [0,1] */
        Kspring = max(0,Kspring);
	// XXX Magic number!!
        Kspring *=1.3;
        Kspring =  Kspring *(MAX_K-MIN_K)+MIN_K;

        if (firstExecution) 
		printf("Kspring = %g\n", Kspring);
	if (forceDevice)
        	forceDevice->setSurfaceKspring(Kspring);
    } 
    else if (forceDevice){
    	forceDevice->setSurfaceKspring(Kspring);
    }

    firstExecution = 0;
}

void specify_sound(int x, int y)
{
  static int firstExecution = 1;
  float frequency;
  float maxfreq = 2000;
  float minfreq = 80;
  float freq_diff = 1920;

  BCPlane *plane;

  if (firstExecution)
    printf("Setting sound...\n");
   plane = dataset->inputGrid->getPlaneByName(soundPlaneName.string());
   if (plane != NULL) 
     {
       if (sound_slider_max != sound_slider_min)
 	{
 	  frequency = ((plane->value(x,y) - sound_slider_min) /
 		       (sound_slider_max - sound_slider_min));
 	}
       else
 	frequency = 0;
       frequency = frequency*(freq_diff) + minfreq;
       if (frequency < 0)
 	frequency = 0;
       if (frequency > maxfreq)
 	  frequency = maxfreq;
       printf("frequency: %f\n", frequency);
     }
   firstExecution = 0;
}


void specify_surface_friction(int x, int y)
{
  static int firstExecution = 1;
  double kS;
  BCPlane *plane;

  if (firstExecution)
    printf("Setting surface friction...\n");
  
  
  // Find out which plane to use.  If there is none, set the friction
  // based on the knob setting.
  plane = dataset->inputGrid->getPlaneByName(frictionPlaneName.string());
  if (plane == NULL) {
    kS = max(0.0, Arm_knobs[FRICTION_KNOB] ); /* [0,1]  */
    if (forceDevice){
        forceDevice->setSurfaceFstatic(kS); 
        forceDevice->setSurfaceFdynamic(0.5 * kS);
    }
    firstExecution = 0;
    return;
  }  
  
// If the friction range is zero length, set static friction cosntant based on
// knob.
 
  if (friction_slider_max != friction_slider_min) {
   
    
    if (firstExecution)
      printf("Basing friction (kS) on auxiliary data set...\n");
    
    kS = (plane->value(x,y) - friction_slider_min) / 
      (friction_slider_max - friction_slider_min);
    
    kS = min(1,kS);   /*click scale to be in [0,1] */
    kS = max(0,kS);
    
    if (firstExecution) printf("kS = %g\n", kS);
    if (forceDevice) forceDevice->setSurfaceFstatic(kS);
  } else {
    kS = max(0.0, Arm_knobs[FRICTION_KNOB]); /*[0, 1]*/
    if (forceDevice) forceDevice->setSurfaceFstatic(kS); /*[0, 1]*/
  }
  if (forceDevice) forceDevice->setSurfaceFdynamic(0.5 * kS);
  firstExecution = 0;
}


void specify_surface_bumpsize(int x, int y)
{
  static int firstExecution = 1;
  double wavelength;
  BCPlane *plane;

  if (firstExecution)
    printf("Setting surface bump size...\n");
 
 
  // Find out which plane to use.  If there is none, set the bump amplitude
  // to zero
  plane = dataset->inputGrid->getPlaneByName(bumpPlaneName.string());
  if (plane == NULL) {
    if (forceDevice){
        forceDevice->setSurfaceTextureAmplitude(0);
    }
    firstExecution = 0;
    return;
  } 

// If the bump range is zero length, disable bumps

  if (bump_slider_max != bump_slider_min) {
  
   
    if (firstExecution)
      printf("Basing bump size (wavelength) on auxiliary data set...\n");

/*
    wavelength = (plane->value(x,y) - bump_slider_min) /
      (bump_slider_max - bump_slider_min);
*/
	wavelength = (plane->value(x,y) - plane->minValue())/
		(plane->maxValue() - plane->minValue());

    wavelength = min(1,wavelength);   /*clamp scale to be in [0,1] */
    wavelength = max(0,wavelength);
	wavelength = 0.0004 + wavelength*0.004;	// XXX magic number

    if (firstExecution) printf("haptic texture wavelength = %g\n", wavelength);
    if (forceDevice) {
//		printf("setting wavelength %f meters\n", wavelength);
		forceDevice->setSurfaceTextureWavelength(wavelength);
		// use a constant ratio (0.01) between amplitude and wavelength to
		// keep maximum slope constant
		forceDevice->setSurfaceTextureAmplitude(0.1*wavelength);
	}
  } else {
    if (forceDevice) forceDevice->setSurfaceTextureAmplitude(0);
  }
  firstExecution = 0;
}

void specify_surface_buzzamplitude(int x, int y)
{
  static int firstExecution = 1;
  double amp;
  BCPlane *plane;

  if (firstExecution)
    printf("Setting surface buzz amplitude...\n");


  // Find out which plane to use.  If there is none, set the buzz amplitude
  // to zero
  plane = dataset->inputGrid->getPlaneByName(buzzPlaneName.string());
  if (plane == NULL) {
    if (forceDevice){
        forceDevice->setSurfaceBuzzAmplitude(0);
    }
    firstExecution = 0;
    return;
  }

// If the buzz range is zero length, disable buzzing

  if (buzz_slider_max != buzz_slider_min) {


    if (firstExecution)
      printf("Basing buzzing (amplitude) on auxiliary data set...\n");

/*
    amp = (plane->value(x,y) - buzz_slider_min) /
      (buzz_slider_max - buzz_slider_min);
*/

	amp = (plane->value(x,y) - plane->minValue()) /
		(plane->maxValue() - plane->minValue());
    amp = min(1,amp);   /*click scale to be in [0,1] */
    amp = max(0,amp);

	amp = 0.0001 + 0.0009*amp;
    if (firstExecution) printf("buzzing amplitude = %g\n", amp);
    if (forceDevice) {
//		printf("setting amplitude %f meters\n", amp);
		// use the same frequency used in our psychophysical experiment
        forceDevice->setSurfaceBuzzFrequency(115.0);
        forceDevice->setSurfaceBuzzAmplitude(amp);
    }
  } else {
    if (forceDevice) forceDevice->setSurfaceBuzzAmplitude(0);
  }
  firstExecution = 0;
}



/*****************************************************************************
 *
   specify_surface_adhesion - called by touch_canned_from_plane
 *
 *****************************************************************************/
void specify_surface_adhesion(int x, int y)
{

    static int firstExecution = 1;

    if (firstExecution)
        printf("Setting surface adhesion...\n");

    int k_adhesion_const = adhesion_constant;

    // Use a name that selects from which grid to use
    // for adhesion, and also a minmax scale to map that to each of its
    // parameters.

    BCPlane* plane = dataset->inputGrid->getPlaneByName
                      (adhesionPlaneName.string());
    if (plane == NULL) {
      firstExecution = 0;
      return;
    }

    if (adhesion_slider_max != adhesion_slider_min) {
        double scale, k_adhesion;

        if (firstExecution)
            printf("Basing adhesion (k_adhesion) on auxiliary data set...\n");

        scale = (plane->value(x,y) - adhesion_slider_min) /
                 (adhesion_slider_max - adhesion_slider_min);
        scale = min(1,scale);   /*click scale to be in [0,1] */
        scale = max(0,scale);

        k_adhesion = k_adhesion_const * scale;  
        if (firstExecution)
             printf("scale = %g, k_adhesion = %g\n", scale, k_adhesion);

    }

    firstExecution = 0;
}



/*****************************************************************************
  modified 10/14/99 by Russ Taylor
	- Check for haptic enable/disable and don't display if not
	- Check for haptic feel from flat
	- Return signed distance above (negative for below) plane

   touch_canned_from_plane
	      - Let the user feel on the stored data, doing "Phong haptic
		shading" to interpolate the normal from the nearest grid
		locations.
	      - Handle differences in scale between user and world
	      - Handle differences in orientation between user and world
	      - Uses info from grid only to compute mag and direction.
 *****************************************************************************/

double touch_canned_from_plane(int whichUser, q_vec_type handpos)
{
  double x = handpos[X];
  double y = handpos[Y];

  // If the user has selected feel_from_flat, then call that routine
  // instead.
  if (config_haptic_plane) {
	return touch_flat_from_measurelines(whichUser, handpos);
  }

  BCPlane* plane = dataset->inputGrid->getPlaneByName
                     (dataset->heightPlaneName->string());
  if (plane == NULL)
  {
      fprintf(stderr,
	"Error in touch_canned_from_plane: could not get plane!\n");
      return -1;
  }

  double	rx = ( (x - plane->minX())
		  / (plane->maxX()- plane->minX())*(plane->numX()-1) );
  double	ry = ( (y - plane->minY())
		  / (plane->maxY()- plane->minY())*(plane->numY()-1) );
  int		ix = (int)rx;
  int		iy = (int)ry;
  double	a = rx - ix;
  double	b = ry - iy;
  q_vec_type	pnorm;
  q_vec_type	point;
  v_xform_type	WorldFromTracker, TrackerFromWorld;
  VectorType	Norm00, Norm01, Norm10, Norm11;
  double	z;
  double	d;

  int zero_xnorm = 0, zero_ynorm = 0;

  point[X] = x;
  point[Y] = y;

  if (ix >= (plane->numX()-1)) {
    ix = plane->numX()-2;
    zero_xnorm = 1;
    point[X] = ix*(plane->maxX()-plane->minX())*(plane->numX()-1)+plane->minX();
  }
  if (iy >= (plane->numY()-1)) {
    iy = plane->numY()-2;
    zero_ynorm = 1;
    point[Y] = iy*(plane->maxY()-plane->minY())*(plane->numY()-1)+plane->minY();
  }
  if (ix <= 0){
    ix = 0;
    zero_xnorm = 1;
    point[X] = ix*(plane->maxX()-plane->minX())*(plane->numX()-1)+plane->minX();
  }
  if (iy <= 0){
    iy = 0;
    zero_ynorm = 1;
    point[Y] = iy*(plane->maxY()-plane->minY())*(plane->numY()-1)+plane->minY();
  }

  z = plane->scaledValue(ix,   iy  )*(1-a)*(1-b)
    + plane->scaledValue(ix+1, iy  )*(  a)*(1-b)
    + plane->scaledValue(ix,   iy+1)*(1-a)*(  b)
    + plane->scaledValue(ix+1, iy+1)*(  a)*(  b);

  point[Z] = z;

  Compute_Norm(ix,iy,Norm00, plane);
  Compute_Norm(ix+1,iy,Norm10, plane);
  Compute_Norm(ix+1,iy+1,Norm11, plane);
  Compute_Norm(ix,iy+1,Norm01, plane);

  VectorScale(Norm00,(1-a)*(1-b));
  VectorCopy(pnorm,Norm00);
  VectorScale(Norm10,(  a)*(1-b));
  VectorAdd(pnorm,Norm10,pnorm);
  VectorScale(Norm01,(1-a)*(  b));
  VectorAdd(pnorm,Norm01,pnorm);
  VectorScale(Norm11,(  a)*(  b));
  VectorAdd(pnorm,Norm11,pnorm);

  if (zero_xnorm) pnorm[X] = 0;
  if (zero_ynorm) pnorm[Y] = 0;

  /* Rotate, Xlate and scale point from world space to ARM space.
  ** The normal direction just needs rotating.
  **/
  v_x_compose(&WorldFromTracker,
	  &v_world.users.xforms[whichUser],
	  &v_users[whichUser].xforms[V_ROOM_FROM_HAND_TRACKER]);

  v_x_invert(&TrackerFromWorld, &WorldFromTracker);

  v_x_xform_vector( point, &TrackerFromWorld, point );

  q_xform(pnorm, TrackerFromWorld.rotate, pnorm);

  VectorNormalize(pnorm);

  d = -( pnorm[X]*point[X]+pnorm[Y]*point[Y]+pnorm[Z]*point[Z] );


  //set plane equation

  if (forceDevice) forceDevice->set_plane(pnorm[X],pnorm[Y],pnorm[Z],d);

  specify_surface_compliance(ix,iy);
  specify_surface_friction(ix, iy);
  specify_surface_bumpsize(ix,iy);
  specify_surface_buzzamplitude(ix,iy);

  // Compute and return signed distance above plane (plane in world space)
  // This is the dot product of the vector from the plane's point to the
  // hand location and the plane's unit normal vector.

  q_vec_type	  p_to_h;
  v_x_xform_vector( handpos, &TrackerFromWorld, handpos );
  q_vec_subtract( p_to_h, handpos, point );
  return q_vec_dot_product( p_to_h, pnorm );
}


/*****************************************************************************
  created 10/14/99 by Russ Taylor.
	      -	Returns the signed distance of the user's hand position
		above the plane.

   touch_flat_from_measurelines
	      - Let the user feel on a flat plane that is the plane
		between the three current measure-line intersections
		with the surface. This is useful for experiments that
		want to determine how much force helps during modification
		attempts or feeling on the surface (because this gives
		only a reference plane and not the actual feel of the
		surface).
	      - Handle differences in scale between user and world
	      - Handle differences in orientation between user and world
 *****************************************************************************/

double touch_flat_from_measurelines(int whichUser, q_vec_type handpos)
{
  //---------------------------------------------------------------------
  // Get the height plane, which we'll use to find the height at the
  // locations of the measure lines

  BCPlane* plane = dataset->inputGrid->getPlaneByName
                     (dataset->heightPlaneName->string());
  if (plane == NULL)
  {
      fprintf(stderr,
	"Error in touch_flat_from_measurelines: could not get plane!\n");
      return -1;
  }

  //---------------------------------------------------------------------
  // Find the location of the three measure lines (where they
  // intersect with the plane we are feeling on).

  q_vec_type	red, green, blue;

  red[X] = decoration->red.x();
  red[Y] = decoration->red.y();
  red[Z] = plane->valueAt(red[X], red[Y]) * plane->scale();

  green[X] = decoration->green.x();
  green[Y] = decoration->green.y();
  green[Z] = plane->valueAt(green[X], green[Y]) * plane->scale();

  blue[X] = decoration->blue.x();
  blue[Y] = decoration->blue.y();
  blue[Z] = plane->valueAt(blue[X], blue[Y]) * plane->scale();

  //---------------------------------------------------------------------
  // Pick one of the points (red) as the plane origin, then find the
  // normal using cross-products for the vectors to the other two
  // points. Make sure we get the up-pointing normal.

  q_vec_type	point;
  q_vec_type	r_to_g, r_to_b;
  q_vec_type	pnorm;

  point[X] = red[X];
  point[Y] = red[Y];
  point[Z] = red[Z];

  q_vec_subtract( r_to_g, green, red );
  q_vec_subtract( r_to_b, blue, red );
  if ((q_vec_magnitude( r_to_g ) == 0.0) || (q_vec_magnitude( r_to_b ) == 0.0)){
	fprintf(stderr,"Error in touch_flat_from_measurelines: "
		"red, green or blue measure lines are overlapping\n");
	return -1;
  }
  q_vec_cross_product( pnorm, r_to_g, r_to_b );
  if (pnorm[Z] < 0) {	// Flip it over if it is pointing down
	q_vec_scale( pnorm, -1.0, pnorm );
  }

  //---------------------------------------------------------------------
  // Rotate, Xlate and scale point from world space to ARM space.
  // The normal direction just needs rotating. Compute the plane
  // equation that corresponds to the given point and normal.

  v_xform_type	WorldFromTracker, TrackerFromWorld;
  double	d;

  v_x_compose(&WorldFromTracker,
	  &v_world.users.xforms[whichUser],
	  &v_users[whichUser].xforms[V_ROOM_FROM_HAND_TRACKER]);

  v_x_invert(&TrackerFromWorld, &WorldFromTracker);

  v_x_xform_vector( point, &TrackerFromWorld, point );

  q_xform(pnorm, TrackerFromWorld.rotate, pnorm);

  VectorNormalize(pnorm);

  d = -( pnorm[X]*point[X]+pnorm[Y]*point[Y]+pnorm[Z]*point[Z] );

  //---------------------------------------------------------------------
  // Set plane equation.

  if (forceDevice) {
	forceDevice->set_plane(pnorm[X],pnorm[Y],pnorm[Z],d);
  }

  // Compute and return signed distance above plane (plane in world space)
  // This is the dot product of the vector from the plane's point to the
  // hand location and the plane's unit normal vector.

  q_vec_type	  p_to_h;
  v_x_xform_vector( handpos, &TrackerFromWorld, handpos );
  q_vec_subtract( p_to_h, handpos, point );
  return q_vec_dot_product( p_to_h, pnorm );
}

/*****************************************************************************
 *
  modified 10/14/99 by Russ Taylor
	- Check for haptic enable/disable and don't display if not
	- Check for haptic feel from flat
	- Returns the signed distance of the user's hand position
	  above the plane.

   touch_live_to_plane_fit_to_line
	      - Apply force to user based on difference in Z value
		between this point result and the last one
	      - Handle differences in scale between user and world
	      - Handle differences in orientation between user and world
	      - Uses vector from last point to current point for normal
		in plane of Z and line of travel.
	      - MUST BE IN LOCK STEP BEFORE CALLING !!!
 *
 *****************************************************************************/

double touch_live_to_plane_fit_to_line(int whichUser, q_vec_type handpos)
{
  // If the user has selected feel_from_flat, then call that routine
  // instead.
  if (config_haptic_plane) {
	return touch_flat_from_measurelines(whichUser, handpos);
  }

     q_vec_type		        pnorm;
     q_vec_type		        point;
     static q_vec_type		up = { 0.0, 0.0, 1.0 };
     q_vec_type		        at;
     static q_vec_type	        last_point;
     static q_vec_type	        last_pnorm;
     v_xform_type               WorldFromTracker, TrackerFromWorld;
     double			d;

     point[X] = microscope->state.data.inputPoint->x();
     point[Y] = microscope->state.data.inputPoint->y();

     // Scale the Z value by the scale factor of the currently-displayed
     // data set.  XXX This assumes that the one mapped to height display is
     // also mapped in touch mode, and that mapping for this has been
     // set up.
     BCPlane* plane = dataset->inputGrid->getPlaneByName
                     (dataset->heightPlaneName->string());
     Point_value *value =
	microscope->state.data.inputPoint->getValueByPlaneName
                     (dataset->heightPlaneName->string());

     if (plane == NULL) {
	 fprintf(stderr,
	"Error in touch_live_to_plane_fit_to_line: could not get plane!\n");
	 return -1;
     }
     if (value == NULL) {
	 fprintf(stderr,
	"Error in touch_live_to_plane_fit_to_line: could not get value!\n");
	 return -1;
     }

     point[Z] = value->value()*plane->scale();

     /* start off with the force normal assumed straight up
     **/
     if( !plane_line_set ) {
	 q_vec_copy( last_pnorm, up );
	 q_vec_copy( last_point, point );
	 }

     q_vec_copy( pnorm, last_pnorm );

     /* don't let the reuse of variables fool you. the normal is:
     **	norm = ((p-last_p)X(0,0,1))X(p-last_p).
     ** if we don't have a last point or haven't moved, leave the
     ** normal direction alone.
     **/
     if( plane_line_set ) {
	 q_vec_subtract( at, point, last_point );
	 if( 10.0 < ( at[X]*at[X]+
		     at[Y]*at[Y] ) ) {
	     q_vec_cross_product( pnorm, at, up );
	     q_vec_cross_product( pnorm, pnorm, at );
	     q_vec_copy( last_point, point );
	     }
	 }

     VectorNormalize(pnorm);

     q_vec_copy( last_pnorm, pnorm );
     plane_line_set = 1;

     /* got x,y,z, and norm in microscope space, XForm into ARM space
     **/

     /* Rotate, Xlate and scale point from world space to ARM space.
     ** The normal direction just needs rotating.
     **/
     v_x_compose(&WorldFromTracker,
	     &v_world.users.xforms[whichUser],
	     &v_users[whichUser].xforms[V_ROOM_FROM_HAND_TRACKER]);

     v_x_invert(&TrackerFromWorld, &WorldFromTracker);

     v_x_xform_vector( point, &TrackerFromWorld, point );

     q_xform(pnorm, TrackerFromWorld.rotate, pnorm);

     d = -( pnorm[X]*point[X]+pnorm[Y]*point[Y]+pnorm[Z]*point[Z] );

    if (!forceDevice) return 0;

    forceDevice->set_plane(pnorm[X],pnorm[Y],pnorm[Z],d);
      
    forceDevice->setSurfaceKspring(Arm_knobs[FORCE_KNOB]*(MAX_K-MIN_K)+MIN_K);
    specify_point_friction();
    specify_point_adhesion();
    specify_point_bumpsize();
    specify_point_buzzamplitude();

     /* Send the plane to the haptic server */
     forceDevice->sendSurface();

     // Compute and return signed distance above plane (plane in world space)
     // This is the dot product of the vector from the plane's point to the
     // hand location and the plane's unit normal vector.

     q_vec_type	  p_to_h;
     v_x_xform_vector( handpos, &TrackerFromWorld, handpos );
     q_vec_subtract( p_to_h, handpos, point );
     return q_vec_dot_product( p_to_h, pnorm );
}

/*****************************************************************************
 *
   specify_point_friction - called by touch_live_to_plane_fit_to_line
 *
 *****************************************************************************/

void specify_point_friction(void)
{
   static int firstExecution=1;

   if(firstExecution) {
      printf("Setting surface friction...\n");
      firstExecution = 0;
   }

   if (!forceDevice) return;
   forceDevice->setSurfaceFdynamic(max(0.0, Arm_knobs[FRICTION_KNOB]));


    // Scale the value by the scale factor of the currently-displayed
    // data set.  If there is no data set, use the knob value.
    Point_value *value =
        microscope->state.data.inputPoint->getValueByPlaneName
                     (frictionPlaneName.string());

    if (value == NULL) { // No friction specified -- use knob
       forceDevice->setSurfaceFstatic(max(0.0, Arm_knobs[FRICTION_KNOB])); /* [0, 1] */        
         return;
    }

    // If the range for friction is of zero length, base the friction
    // on the knob value. 
    double kS;

    if(friction_slider_min!=friction_slider_max) {       
        if (firstExecution)
           printf("Basing friction (kS) on friction plane...\n");
 
        kS = ( value->value()  - friction_slider_min) /
                (friction_slider_max - friction_slider_min);
        kS = min(1,kS);   /*click scale to be in [0,1] */
        kS = max(0,kS);

        if (firstExecution)
           printf("kS = %g\n", kS);

       forceDevice->setSurfaceFstatic(kS);
    } else {
      kS = max(0.0, Arm_knobs[FRICTION_KNOB]);
      forceDevice->setSurfaceFstatic(kS);
   }

}   

/*****************************************************************************
 *
   specify_point_bumpsize - called by touch_live_to_plane_fit_to_line
 *
 *****************************************************************************/

void specify_point_bumpsize(void)
{
   static int firstExecution=1;

   if(firstExecution) {
      printf("Setting point bumpsize...\n");
      firstExecution = 0;
   }

   if (!forceDevice) return;

    // Scale the value by the scale factor of the currently-displayed
    // data set.  If there is no data set, disable bump texture.
    Point_value *value =
			microscope->state.data.inputPoint->getValueByPlaneName
                                (bumpPlaneName.string());

    if (value == NULL) { // No bump size specified, disable
       forceDevice->setSurfaceTextureAmplitude(0);
       return;
    }

    // If the range for bumps is of zero length, disable bumps
    double wavelength;

    if(bump_slider_min!=bump_slider_max) {
        if (firstExecution)
           printf("Basing haptic texture (wavelength) on bump plane...\n");

        wavelength = ( value->value()  - bump_slider_min) /
                (bump_slider_max - bump_slider_min);
        wavelength = min(1,wavelength);   /*click scale to be in [0,1] */
        wavelength = max(0,wavelength);

        if (firstExecution)
           printf("wavelength = %g\n", wavelength);

	   // convert from cm to meters
       forceDevice->setSurfaceTextureWavelength(wavelength*0.01);
	   forceDevice->setSurfaceTextureAmplitude(0.1*wavelength*0.01);
    } else {
      forceDevice->setSurfaceTextureAmplitude(0);
   }
}  

/*****************************************************************************
 *
   specify_point_buzzamplitude - called by touch_live_to_plane_fit_to_line
 *
 *****************************************************************************/

void specify_point_buzzamplitude(void)
{
   static int firstExecution=1;

   if(firstExecution) {
      printf("Setting point buzzsize...\n");
      firstExecution = 0;
   }

   if (!forceDevice) return;

    // Scale the value by the scale factor of the currently-displayed
    // data set.  If there is no data set, disable buzzing
    Point_value *value =
            microscope->state.data.inputPoint->getValueByPlaneName
                (buzzPlaneName.string());

    if (value == NULL) { // No buzz amplitude specified, disable
       forceDevice->setSurfaceBuzzAmplitude(0);
       return;
    }

    // If the range for amp is of zero length, disable buzzing
    double amp;

    if(buzz_slider_min!=buzz_slider_max) {
        if (firstExecution)
           printf("Basing buzzing (amplitude) on buzz plane...\n");

        amp = ( value->value()  - buzz_slider_min) /
                (buzz_slider_max - buzz_slider_min);
        amp = min(1,amp);   /*click scale to be in [0,1] */
        amp = max(0,amp);

        if (firstExecution)
           printf("amplitude = %g\n", amp);

       // see comments in specify_surface_buzzamplitude
       forceDevice->setSurfaceBuzzAmplitude(amp*0.002);
       forceDevice->setSurfaceBuzzFrequency(115.0);
    } else {
      forceDevice->setSurfaceBuzzAmplitude(0);
   }
} 

/*****************************************************************************
 *
   specify_point_adhesion - called by touch_live_to_plane_fit_to_line
 *
 *****************************************************************************/

void specify_point_adhesion(void)
{
   static int firstExecution=1;

   if(firstExecution) {
      printf("Setting point adhesion...\n");
      firstExecution = 0;
   }

   //int k_adhesion_const=adhesion_constant;

   // Scale the value by the scale factor of the currently-displayed
   // data set.  If there is no value, set the adhesion to 0.
   Point_value *value =
        microscope->state.data.inputPoint->getValueByPlaneName
                     (adhesionPlaneName.string());

   if (value == NULL)
         return;

   // If the adhesion range is of zero length, set the adhesion to 0.
   if(adhesion_slider_min != adhesion_slider_max) {
        double scale /*, k_adhesion */ ;

        scale = ( value->value()  - adhesion_slider_min) /
                (adhesion_slider_max - adhesion_slider_min);
        scale = min(1,scale);   /*click to be in [0,1] */
        scale = max(0,scale);
   }
}






/*****************************************************************************
 *
   plane_norm - Apply force to user based on difference in Z value
	      - Handle differences in scale between user and world
	      - Handle differences in orientation between user and world
	      - Uses Normal from microscope.
	      - MUST BE IN LOCK STEP BEFORE CALLING !!!
 *
 *****************************************************************************/

int plane_norm(int whichUser)
{
     Blunt_result * blunt_result;
     q_vec_type			point;
     q_vec_type			pnorm;
     v_xform_type            	WorldFromTracker, TrackerFromWorld;
     double			d;

     blunt_result = microscope->getBluntResult();

     /* if we don't have a valid norm, do nothing.  the Z component
     **	of the surface normal must always be positive.
     **/
     if( blunt_result->normal[Z] <= 0.0 ) {
	fprintf(stderr,"Z component of normal was negative, ignoring!\n");
	return -1;
     }

     point[X] = blunt_result->x;
     point[Y] = blunt_result->y;

     BCPlane* plane = dataset->inputGrid->getPlaneByName
                     (dataset->heightPlaneName->string());
     if (plane == NULL)
     {
	 fprintf(stderr, "Error in plane_norm: could not get plane!\n");
	 return -1;
     }     
     point[Z] = blunt_result->height*plane->scale();

     /* got x,y,z, and norm in microscope space, XForm into ARM space
     **/

     /* Rotate, Xlate and scale point from world space to ARM space.
     ** The normal direction just needs rotating.
     **/
     v_x_compose(&WorldFromTracker,
	     &v_world.users.xforms[whichUser],
	     &v_users[whichUser].xforms[V_ROOM_FROM_HAND_TRACKER]);

     v_x_invert(&TrackerFromWorld, &WorldFromTracker);

     v_x_xform_vector( point, &TrackerFromWorld, point );

     q_xform(pnorm, TrackerFromWorld.rotate, blunt_result->normal);

     d = 1.0/sqrt( pnorm[X]*pnorm[X]
		+  pnorm[Y]*pnorm[Y]
		+  pnorm[Z]*pnorm[Z] );
     pnorm[X] *= d;
     pnorm[Y] *= d;
     pnorm[Z] *= d;

     d = -( pnorm[X]*point[X]
	 +  pnorm[Y]*point[Y]
	 +  pnorm[Z]*point[Z] );

     forceDevice->set_plane(pnorm[X],pnorm[Y],pnorm[Z],d);

 
     /* Find the spring constant based on the force dial */
     forceDevice->setSurfaceKspring(Arm_knobs[FORCE_KNOB]*(MAX_K-MIN_K)+MIN_K);

     /* Send the plane to the haptic server */
     forceDevice->sendSurface();

     return 0;
}

int specify_directZ_force(int whichUser) 
{
     q_vec_type		        point;
     q_vec_type		up = { 0.0, 0.0, 1.0 };
     v_xform_type               WorldFromTracker, TrackerFromWorld;

     point[X] = microscope->state.data.inputPoint->x();
     point[Y] = microscope->state.data.inputPoint->y();

     // Scale the Z value by the scale factor of the currently-displayed
     // data set.  XXX This assumes that the one mapped to height display is
     // also mapped in touch mode, and that mapping for this has been
     // set up.
     BCPlane* plane = dataset->inputGrid->getPlaneByName
                     (dataset->heightPlaneName->string());
     Point_value *value =
	microscope->state.data.inputPoint->getValueByPlaneName
                     (dataset->heightPlaneName->string());

     if (plane == NULL) {
	 fprintf(stderr, "Error in specify_directZ_force: could not get plane!\n");
	 return -1;
     }
     if (value == NULL) {
	 fprintf(stderr, "Error in specify_directZ_force: could not get value!\n");
	 return -1;
     }

     point[Z] = value->value()*plane->scale();

    double current_force;

    // Get the current value of the internal sensor, which tells us 
    // the force the tip is experiencing.
     Point_value *forcevalue =
	microscope->state.data.inputPoint->getValueByName("Internal Sensor");

     if (forcevalue == NULL) {
	 fprintf(stderr, "Error in specify_directZ_force: could not get force value!\n");
	 return -1;
     }

     current_force = forcevalue->value();

     // Calculate the difference from the free-space value of 
     // internal sensor recorded when we entered direct-z control
     double diff_force = current_force - microscope->state.modify.freespace_normal_force;

     printf("specify_directZ_force: internal sensor difference: %g\n",
	    diff_force);
     // got user's hand position and force direction (up) in microscope space, 
     //XForm into hand-tracker space
     // Rotate, Xlate and scale point from world space to hand-tracker space.
     // The normal direction just needs rotating.
     v_x_compose(&WorldFromTracker,
	     &v_world.users.xforms[whichUser],
	     &v_users[whichUser].xforms[V_ROOM_FROM_HAND_TRACKER]);

     v_x_invert(&TrackerFromWorld, &WorldFromTracker);

     v_x_xform_vector( point, &TrackerFromWorld, point );

     q_xform(up, TrackerFromWorld.rotate, up);
     


    if (!forceDevice) return 0;

    // This should be where the user is currently located.
    forceDevice->setFF_Origin(point[Q_X], point[Q_Y], point[Q_Z]);
    // This is the force measured by the microscope, * scale factor, 
    // * the direction (up = positive z).
    forceDevice->setFF_Force(diff_force*up[Q_X]*directz_force_scale, 
			     diff_force*up[Q_Y]*directz_force_scale, 
			     diff_force*up[Q_Z]*directz_force_scale);
    printf("   Final force vector: %g %g %g\n", 
	   diff_force*up[Q_X]*directz_force_scale, 
			     diff_force*up[Q_Y]*directz_force_scale, 
			     diff_force*up[Q_Z]*directz_force_scale);
    // Force field does not change as we move around.
    forceDevice->setFF_Jacobian(0,0,0,  0,0,0,  0,0,0);
    forceDevice->setFF_Radius(0.02); // 2cm radius of validity
    // this actually turns on the force field- do that later. 
    //forceDevice->sendForceField();

    return 0;
}

/*****************************************************************************
 *
   meas_cross - draw out the cross section between two points.  draws only
     nearest neighbor calc of grid point values at grid x,y's.
 *
 *****************************************************************************/
#define IABS(i,e)	(((i)=(e)) < 0 ? (i)=-(i) : (i))

int 
meas_cross( float x0, float y0, float x1, float y1, float z_scale)
{
  PointType 	Top, Bot;
  int		i0 = (int)( (x0-dataset->inputGrid->minX())*dataset->inputGrid->derangeX());
  int		j0 = (int)( (y0-dataset->inputGrid->minY())*dataset->inputGrid->derangeY());
  int		i1 = (int)( (x1-dataset->inputGrid->minX())*dataset->inputGrid->derangeX());
  int		j1 = (int)( (y1-dataset->inputGrid->minY())*dataset->inputGrid->derangeY());
  float		ranger_x = 1.0/dataset->inputGrid->derangeX();
  float		ranger_y = 1.0/dataset->inputGrid->derangeY();
  float		x;
  float		y;
  float		dx, dy;
  int		n = MAX( IABS(n,i1-i0), IABS(n,j1-j0) );

  BCPlane* plane = dataset->inputGrid->getPlaneByName
                     (dataset->heightPlaneName->string());
  if (plane == NULL)
  {
      fprintf(stderr, "Error in meas_cross: could not get plane!\n");
      return -1;
  }   

  dx = (x1-x0)/n;
  dy = (y1-y0)/n;

  Top[X] = x0; Top[Y] = y0; 
  if( 
	(Top[X] <= plane->maxX())
	&&
	(Top[X] >= plane->minX())
	&&
	(Top[Y] <= plane->maxY())
	&&
	(Top[Y] >= plane->minY())
    )
  	Top[Z] = plane->valueAt( x0, y0 )*z_scale;
  else
	Top[Z] = 0.0;

  for( x = x0+dx, y = y0+dy; n--; x += dx, y += dy ) {

    Bot[X] = ( (int)((x-plane->minX())*dataset->inputGrid->derangeX()) )*ranger_x + plane->minX();
    Bot[Y] = ( (int)((y-plane->minY())*dataset->inputGrid->derangeY()) )*ranger_y + plane->minY();
    if( 
	(Bot[X] <= plane->maxX())
	&&
	(Bot[X] >= plane->minX())
	&&
	(Bot[Y] <= plane->maxY())
	&&
	(Bot[Y] >= plane->minY())
      )
    	Bot[Z] = plane->valueAt( Bot[X], Bot[Y] )*z_scale;
    else
	Bot[Z] = 0.0;
    Top[X] = Bot[X]; Top[Y] = Bot[Y]; Top[Z] = Bot[Z];
    }
  return 0;
  }
#undef IABS

/*****************************************************************************
*
   doMeasure - Moving the red, green, blue measure lines around.
*
******************************************************************************/

#define RED   1
#define GREEN 2
#define BLUE  3

int doMeasure(int whichUser, int userEvent)
{ 
        q_vec_type clipPos;
	double          rdxy,gdxy,bdxy;
	static          int              hold_color;
	static          int              ishold = 0;

	/* Move the tip to the hand x,y location */
	/* Set its height based on data at this point */
        nmui_Util::clipPosition(whichUser, clipPos);


//fprintf(stderr, "doMeasure:  hpn %s.\n", dataset->heightPlaneName->string());
        BCPlane* plane = dataset->inputGrid->getPlaneByName
                     (dataset->heightPlaneName->string());
        if (plane == NULL)
        {
            fprintf(stderr, "Error in doMeasure: could not get plane!\n");
            return -1;
        }

        decoration->red.normalize(plane);
        decoration->green.normalize(plane);
        decoration->blue.normalize(plane);


        rdxy = sqrt( (clipPos[0] - decoration->red.x()) *
                     (clipPos[0] - decoration->red.x()) +
                     (clipPos[1] - decoration->red.y()) *
                     (clipPos[1] - decoration->red.y()) );
          
        gdxy = sqrt( (clipPos[0] - decoration->green.x()) *
                     (clipPos[0] - decoration->green.x()) +
                     (clipPos[1] - decoration->green.y()) *
                     (clipPos[1] - decoration->green.y()) );
         
        bdxy = sqrt( (clipPos[0] - decoration->blue.x()) *
                     (clipPos[0] - decoration->blue.x()) +
                     (clipPos[1] - decoration->blue.y()) *
                     (clipPos[1] - decoration->blue.y()) );
      
        if (ishold == 0) {
            graphics->setHandColor(BLUE);
            if ((rdxy <= bdxy) && (rdxy <= gdxy))
               graphics->setHandColor(RED);
            else if (gdxy <= bdxy)
               graphics->setHandColor(GREEN);
        }

        switch(userEvent) {
        
	   case PRESS_EVENT: hold_color = graphics->getHandColor();
           case HOLD_EVENT: 
	     ishold =1;
	     if(hold_color == RED) { 
               decoration->red.moveTo(clipPos[0], clipPos[1], plane);
               decoration->red.doCallbacks();
	     }
	     else if(hold_color == GREEN) {
               decoration->green.moveTo(clipPos[0], clipPos[1], plane);
               decoration->green.doCallbacks();
	     }
	     else if(hold_color == BLUE){
               decoration->blue.moveTo(clipPos[0], clipPos[1], plane);
               decoration->blue.doCallbacks();
	     }
	     //if the rulergrid position is being set
	     if (rulergrid_position_line == 1 && rulergrid_enabled == 1 &&
		 hold_color == RED)
	       {
		 rulergrid_xoffset = decoration->red.x();
		 rulergrid_yoffset = decoration->red.y();
	       }
	     //if the rulergrid orientation is being set
	     if (rulergrid_orient_line == 1 && rulergrid_enabled == 1 &&
		 hold_color == GREEN)
	       {
		 float xdiff = decoration->green.x() - rulergrid_xoffset;
		 float ydiff = decoration->green.y() - rulergrid_yoffset;
		 //hypotenuse
		 float hyp = sqrt(xdiff*xdiff + ydiff*ydiff);
		 //make sure you never divide by zero
		 //will happen if red and green line are at the same position
		 //dope!!!
		 if (hyp == 0)
		   hyp = 1;
		 //be careful about the quadrants for sin and cos
		 //rulergrid_angle sets the angle for the Tcl variable
		 if (ydiff >= 0)
		   {
		     //in quadrant 1 or 2
		     rulergrid_angle = acos(xdiff/hyp)*180/M_PI;
		     graphics->setRulergridAngle(rulergrid_angle);
		     //printf("sin %f cos %f angle %f\n", rulergrid_sin, 
		     //    rulergrid_cos, acos(xdiff/hyp)*180/M_PI);
		   }
		 else if (xdiff < 0 && ydiff < 0)
		   {
		     //in quadrant 3
		     rulergrid_angle = 360 - (acos(xdiff/hyp)*180/M_PI);
		     graphics->setRulergridAngle(rulergrid_angle);
		     //printf("sin %f cos %f angle %f\n", rulergrid_sin, 
		     //    rulergrid_cos, 360 - acos(xdiff/hyp)*180/M_PI);
		   }
		 else if (xdiff >= 0 && ydiff < 0)
		   {
		     // quadrant 4
		     rulergrid_angle = 360 - (acos(xdiff/hyp)*180/M_PI);
		     graphics->setRulergridAngle(rulergrid_angle);
		     //printf("sin %f cos %f angle %f\n", rulergrid_sin, 
		     //    rulergrid_cos, 360 - acos(xdiff/hyp)*180/M_PI);
		   }

		 //cause the whole surface to redraw
		 cause_grid_redraw(0.0, NULL);		 
	       }
	     break;
	   case RELEASE_EVENT:
	     ishold =0;
           default: break;
        }
    return 0;
}
#undef RED
#undef GREEN
#undef BLUE



/*****************************************************************************
 *
   doLine - Let the user specify points in a poly-line while feeling.
            When they press commit, do this (see handle_commit_pressed):
            Increase force and move from one point to another, then
	       decrease force, engraving straight line from point A to
	       point B.
	    Pause at the start point before moving to allow the offset
		due to piezo relaxation to go away.
 *
 *****************************************************************************/

int doLine(int whichUser, int userEvent)
{
	v_xform_type	worldFromHand;
   	PointType 	Top, Bot;
        q_vec_type clipPos;
 	static int	SurfaceGoing = 0;
	double		hypot_len = microscope->state.modify.region_diag;
        q_matrix_type	hand_mat;
	q_vec_type	angles;

VERBOSE(8, "      In doLine().");

	/* if we are not running live, you should not be able
	   to do this, so put the user into grab mode -- Renee */
	if (dataset->inputGrid->readMode() != READ_DEVICE)
	  {
	    handleCharacterCommand("G", &dataset->done, 1);
	    printf("Line mode available only on live data!!!\n");
	    return 0;
	  }

	BCPlane* plane = dataset->inputGrid->getPlaneByName
                     (dataset->heightPlaneName->string());
	if (plane == NULL)
	{
	    fprintf(stderr, "Error in doLine: could not get plane!\n");
	    return -1;
	}     

	Point_value *value =
		microscope->state.data.inputPoint->getValueByPlaneName
                     (dataset->heightPlaneName->string());
	if (value == NULL) {
		fprintf(stderr, "Error in doLine(): could not get value!\n");
		return -1;
	}

	/* Find the x,y location of hand in grid space */
        nmui_Util::clipPosition(whichUser, clipPos);

        /* Move the aiming line to the user's hand location */
	// re-draw the aim line and the red sphere representing the tip.
        //nmui_Util::moveAimLine(clipPos);
        decoration->aimLine.moveTo(clipPos[0], clipPos[1], plane);
        nmui_Util::moveSphere(clipPos);


	// These are the points we have specified so far in the polyline
	Position_list & pos_list = microscope->state.modify.stored_points;

	// if the style is sweep, set up additional icon for sweep width
	if (microscope->state.modify.style == SWEEP) {
	    // set up the sweep direction 
	    // if the user has already specified a point, make
	    // the width perpendicular to the line they are drawing.
	    if (!pos_list.empty()) {
	    	pos_list.goToTail();

		// Add pi/2 because it matches mod...
	    	microscope->state.modify.yaw =atan2((pos_list.currY()-clipPos[1]),
	           (pos_list.currX()-clipPos[0])) + M_PI_2;
	    } else {
		// otherwise, set the angle based on hand position.

		v_get_world_from_hand(whichUser, &worldFromHand);
		q_to_col_matrix(hand_mat, worldFromHand.rotate);
		q_col_matrix_to_euler( angles, hand_mat );
		microscope->state.modify.yaw = angles[YAW] + M_PI_2;
	    }

	    /* code dealing with locking the width (qliu 7/5/95) */
	    if (lock_width == 0) {
		// Change sweep width based on phantom pen roll.
		microscope->state.modify.sweep_width = - SWEEP_WIDTH_SCALE*
		                         hypot_len * sin(angles[ROLL]);
		// set the sweep position based on the phantom pen yaw.
        	Top[X] = clipPos[0];
        	Top[Y] = clipPos[1];
		Bot[X] = clipPos[0] + microscope->state.modify.sweep_width *
                                  cos(microscope->state.modify.yaw);
		Bot[Y] = clipPos[1] + microscope->state.modify.sweep_width *
                                  sin(microscope->state.modify.yaw);
		Bot[Z] = plane->valueAt(clipPos[0], clipPos[1]) * plane->scale();
		Top[Z] = Bot[Z] - hypot_len * cos(angles[ROLL]);

	    } else {  // sweep width is locked, look up value.
		Top[X] = Bot[X] = clipPos[0] + microscope->state.modify.sweep_width 
		    * cos( microscope->state.modify.yaw );
		Top[Y] = Bot[Y] = clipPos[1] + microscope->state.modify.sweep_width 
		    * sin( microscope->state.modify.yaw );
		Bot[Z] = plane->valueAt( clipPos[0], clipPos[1] ) * plane->scale();
		Top[Z] = plane->maxAttainableValue() * plane->scale();
	    }
	    //draw vertical green line representing sweep width.
	    graphics->positionSweepLine(Top, Bot);
	    
	}

VERBOSE(8, "      doLine:  starting case statement.");

	switch ( userEvent ) 
	    {
	    // Allow user to feel around to find first location in polyline
	    case PRESS_EVENT:
		/* Request a reading from the current location,
		 * and wait till tip gets there */
		microscope->TakeFeelStep(clipPos[0], clipPos[1], value, 1);
		
		// need this so we can use touch_live_to_plane_fit_to_line
		// to feel surface later.
		plane_line_set = 0;

		// update the position of the rubber-band line
		graphics->setRubberLineEnd(clipPos[0], clipPos[1]);
		break;

	case HOLD_EVENT:
	    /* Feel the surface, to help determine the next point. */
	    /* Request a reading from the current location */
	    microscope->TakeFeelStep(clipPos[0], clipPos[1]);
	    
	    // update the position of the rubber-band line
	    graphics->setRubberLineEnd(clipPos[0], clipPos[1]);
	    
	    /* Apply force to the user based on current sample points */
	    if( microscope->state.relaxComp >= 0 ) {
		touch_live_to_plane_fit_to_line(whichUser, clipPos);
		if( !SurfaceGoing ) {
		    if (forceDevice  && config_haptic_enable) {
		    	forceDevice->startSurface();
			SurfaceGoing = 1;
		    }
		}
	    }
	    
	    break;

	    case RELEASE_EVENT:
	      {
		/* Save current hand position as one point on the polyline-
		 * The list is saved in microscope->state.modify.stored_points*/
		//printf("Line mode release event\n");


		/* don't send a stop surface when not surfacing (?)
		**/
		if( SurfaceGoing ) {
		    if (forceDevice) {
		    	forceDevice->stopSurface();
			SurfaceGoing = 0;
		    }
		}

		//add an icon to represent this spot as part of the polyline.
		PointType * markpts = new PointType[2];
		markpts[0][0] = markpts[1][0] = clipPos[0];
		markpts[0][1] = markpts[1][1] = clipPos[1];
		markpts[0][2] = plane->maxAttainableValue()*plane->scale();
		markpts[1][2] = plane->minAttainableValue()*plane->scale();

		// HACK
		// list_id is always 0 or -1
		// because it (apparently) no longer matters
		// but we haven't rewritten Position/Position_list
		// to reflect that fact
		int list_id;
		list_id = graphics->addPolylinePoint(markpts);

		
		// Now we do some different things depending on
		// whether this is LINE tool or CONSTR_FREEHAND
		// tool. LINE or SLOW_LINE tool can keep adding points
		// indefinitely - modification gets done automatically
		// when "commit" is pressed. CONSTR_FREEHAND switches
		// after two points are specified so that the next
		// PRESS event will be handled by doFeelLive -
		// modification freehand when we hit "commit".
		if ((microscope->state.modify.tool == LINE)||
		    (microscope->state.modify.tool == SLOW_LINE)){
		   graphics->setRubberLineStart(clipPos[0], clipPos[1]);
		   //save this point as part of the poly-line
		   pos_list.insert(clipPos[0], clipPos[1], list_id);
		} else if (microscope->state.modify.tool == CONSTR_FREEHAND) {
		   // If this is the second point, set a flag so we
		   // switch to doFeelLive on next event.
		   if (!pos_list.empty()) {
		      microscope->state.modify.constr_line_specified = VRPN_TRUE;
		      // also leave rubber line stretched between two
		      // endpoints.
		   } else {
		      graphics->setRubberLineStart(clipPos[0], clipPos[1]);
		   }
		   //save this point as part of the poly-line
		   pos_list.insert(clipPos[0], clipPos[1], list_id);
		   
		   // Notice: we stay in "feel" mode and don't resume scanning.
		} else {
		   fprintf(stderr,"doLine: bad tool spec - shouldn't be here.\n");
		}
              }
	      break;

	    default:
			break;
	    }

	return(0);
}


int doPositionScanline(int whichUser, int userEvent)
{
    if ((userEvent != PRESS_EVENT) && (userEvent != HOLD_EVENT)) return 0;

    v_xform_type worldFromHand;

    BCPlane* plane = dataset->inputGrid->getPlaneByName(
	(char*)dataset->heightPlaneName);
    if (plane == NULL)
    {
        fprintf(stderr, "Error in doPositionScanline: could not get plane!\n");
        return -1;
    }

    q_vec_type position;
    q_type rotation;
    q_vec_type scan_dir = {1.0, 0.0, 0.0};
    q_vec_type scan_vec;

    v_get_world_from_hand(whichUser, &worldFromHand);
    q_vec_copy(position, worldFromHand.xlate);
    q_copy(rotation, worldFromHand.rotate);

// maybe the user will want to scan off the edge of the grid (?)
/*
    if (position[0] < plane->minX()) position[0] = plane->minX();
    if (position[0] > plane->maxX()) position[0] = plane->maxX();
    if (position[1] < plane->minY()) position[1] = plane->minY();
    if (position[1] > plane->maxY()) position[1] = plane->maxY();
*/

    q_xform(scan_dir, rotation, scan_dir);
    scan_dir[Q_Z] = 0.0;
    q_vec_normalize(scan_dir, scan_dir);
    scan_vec[Q_X] = scan_dir[Q_X]*microscope->state.scanline.width;
    scan_vec[Q_Y] = scan_dir[Q_Y]*microscope->state.scanline.width;
    scan_vec[Q_Z] = 0.0;

    float p0[3], p1[3];
    p1[0] = position[0];
    p1[1] = position[1];
    p1[2] = position[2];

/*
    // some code to clip the
    // scanline to the intersection with the border of the scan region
    float s_param[4];
    s_param[0] = (plane->minX() - p0[0])/(scan_vec[0]);
    s_param[1] = (plane->minY() - p0[1])/(scan_vec[1]);
    s_param[2] = (plane->maxX() - p0[0])/(scan_vec[0]);
    s_param[3] = (plane->maxY() - p0[1])/(scan_vec[1]);

    // the s-value in the range 0..1 which is minimum tells us where to clip
    float min_s = 1.0;
    int i;
    for (i = 0; i < 4; i++)
    	if (s_param[i] > 0 && s_param[i] < min_s)
	    min_s = s_param[i];
*/
    p0[0] = position[0] - scan_vec[0];
    p0[1] = position[1] - scan_vec[1];
    p0[2] = position[2] - scan_vec[2];
    
	// convert x and y from nm to % of scan region
    microscope->state.scanline.x_end = 100.0*(p1[0] - plane->minX())/
										(plane->maxX() - plane->minX());
    microscope->state.scanline.y_end = 100.0*(p1[1] - plane->minY())/
										(plane->maxY() - plane->minY());
    microscope->state.scanline.z_end = p1[2]/(plane->scale());
    // note: if z scale is > 1.0 then we are effectively giving the
    // user a finer adjustment over the height of the scanline
    float angle = asin(scan_dir[Q_Y]);
    if (scan_dir[Q_X] < 0) angle = M_PI - angle;
    microscope->state.scanline.angle = angle*180.0/M_PI;

    graphics->setScanlineEndpoints(p0, p1);

    return 0;

}

/*****************************************************************************
 *
   doFeelFromGrid - Control tip in x and y, updating grid in vicinity of hand.
     Feel contours of grid surface, NOT the real data.
 *
 *****************************************************************************/

int doFeelFromGrid(int whichUser, int userEvent)
{
        q_vec_type clipPos;
	static		int	SurfaceGoing = 0;
	double          aboveSurf;

	BCPlane* plane = dataset->inputGrid->getPlaneByName
                     (dataset->heightPlaneName->string());
	if (plane == NULL)
	{
	    fprintf(stderr, "Error in doFeelFromGrid: could not get plane!\n");
	    return -1;
	}     

	/* Find the x,y location of hand in grid space */
        nmui_Util::clipPosition(whichUser, clipPos);

	/* Move the aiming line to the user's hand location */
        //nmui_Util::moveAimLine(clipPos);
        decoration->aimLine.moveTo(clipPos[0], clipPos[1], plane);
        nmui_Util::moveSphere(clipPos);

	switch ( userEvent ) 
	  {
	  case PRESS_EVENT:
	    /* stylus has just been pressed */
	    /* if user is below the surface on the initial press, don't send
	       the surface, because the user will get a strong upward force. */
	    aboveSurf= touch_canned_from_plane(whichUser, clipPos);
	    if( !SurfaceGoing && aboveSurf>0) {
	      if (forceDevice && config_haptic_enable) {
              	forceDevice->startSurface();
		SurfaceGoing = 1;
	      }
	    }
	    break;

	  case HOLD_EVENT:
	    /* stylus continues to be pressed */
	    /* Apply force to the user based on grid */
	    aboveSurf = touch_canned_from_plane(whichUser, clipPos);
	    if (SurfaceGoing || aboveSurf>0) {
	      if (forceDevice && config_haptic_enable) {
              	forceDevice->sendSurface();
		SurfaceGoing = 1;
	      }
	    }
	    
	    break;
	  
	  case RELEASE_EVENT:	/* Go back to scanning upon release */
	    
	    /* ArmLib doesn't like a stop surface when it's not surfacing 
	    **/
	    if( SurfaceGoing ) {
	      if (forceDevice) {
              	forceDevice->stopSurface();
		SurfaceGoing = 0;
	      }
	    }
	    
	    break;
	    
	  default:
	    break;
	  }
	
	return(0);
}

/*****************************************************************************
 *
   doFeelLive - Control tip in x and y, microscope returns z at each point.
	We use the last two returned points to determine a plane and present
	that plane to the user as a surface.
        When user presses "commit" button, switch to appropriate modify mode.
 *
 *****************************************************************************/

int doFeelLive(int whichUser, int userEvent)  
{

        q_matrix_type	hand_mat;
	q_vec_type	angles;
	double		hypot_len = microscope->state.modify.region_diag;

	v_xform_type	worldFromHand;
	// static to allow xy_lock to work properly
        static q_vec_type clipPos;
	PointType	Top, Bot;
	static		int	SurfaceGoing = 0;
	static		int	ForceFieldGoing = 0;

	/* if we are not running live, you should not be able
	   to do this, so put the user into grab mode */
	if (dataset->inputGrid->readMode() != READ_DEVICE)
	  {
	    handleCharacterCommand("G", &dataset->done, 1);
	    printf("SharpTip mode available only on live data!!!\n");
	    return 0;
	  }

	/* Get the input plane and point */
	BCPlane* plane = dataset->inputGrid->getPlaneByName
                     (dataset->heightPlaneName->string());
	if (plane == NULL)
	{
	    fprintf(stderr, "Error in doFeelLive: could not get plane!\n");
	    return -1;
	}  

   // Get proper name for dataset -- CCWeigle 09/14/99
   // The following lifted from Point_results::getValueByPlaneName()
   char fullname[100];

   fullname[sizeof(fullname)-1] = '\0';
   strncpy(fullname, (char*)dataset->heightPlaneName, sizeof(fullname)-1);
   if (NULL == strrchr(fullname, '-'))
   {
      fprintf(stderr, "doFeelLive(): problem with plane name %s\n", (char*)dataset->heightPlaneName);
      return -1;
   }
   *strrchr(fullname, '-') = '\0';

   //
   // According to the comments, Channel_selector::Set does everything we need
   // 

   Point_channel_selector *point_selector =
         microscope->state.data.point_channels;

   if (point_selector->Is_set(fullname) != 1) {
       if (-1 == point_selector->Set(fullname)) {
	   fprintf(stderr, "doFeelLive(): couldn't activate dataset %s\n", fullname);
	   return -1;
       }
       
       // no need to update the microscope - done automatically. 
       //point_selector->Update_microscope(microscope);
   }
	Point_value *value =
		microscope->state.data.inputPoint->getValueByPlaneName
                     (dataset->heightPlaneName->string());
	if (value == NULL) {
      fprintf(stderr, "doFeelLive(): could not get value ... this is bad\n");
      return -1;
	}


	// Find the x,y location of hand in grid space
	// xy_lock fixes the hand in one position, until it is released
	if (!xy_lock) {
	   if (microscope->state.modify.tool == CONSTR_FREEHAND) {
	      // Constrained freehand only allows motion along a line
	      nmui_Util::clipPositionLineConstraint(whichUser, 
                       clipPos, microscope->state.modify.stored_points);
	   } else {
	      nmui_Util::clipPosition(whichUser, clipPos);
	   }
	}
	/* Move the aiming line to the user's hand location */
        //nmui_Util::moveAimLine(clipPos);
        decoration->aimLine.moveTo(clipPos[0], clipPos[1], plane);
        nmui_Util::moveSphere(clipPos);


	// if the style is sweep, set up additional icon for sweep width
	if (microscope->state.modify.style == SWEEP) {
	    /* now set up the sweep direction and length, 
	     * based on hand position (hand_mat).
	     */
	    v_get_world_from_hand(whichUser, &worldFromHand);
	    q_to_col_matrix(hand_mat, worldFromHand.rotate);
	    q_col_matrix_to_euler( angles, hand_mat );
	    microscope->state.modify.yaw = angles[YAW] + M_PI_2;
	    
	    /* code dealing with locking the width (qliu 7/5/95) */
	    if(lock_width==0) {
		// Change sweep width based on phantom pen rotation.
		microscope->state.modify.sweep_width = - SWEEP_WIDTH_SCALE *
		                         hypot_len * sin( angles[ROLL] );
		// set the sweep position based on the phantom pen yaw.
		Top[X] = clipPos[0];
		Top[Y] = clipPos[1];
		Bot[X] = clipPos[0] + microscope->state.modify.sweep_width
                                * cos( microscope->state.modify.yaw );
		Bot[Y] = clipPos[1] + microscope->state.modify.sweep_width
                                * sin( microscope->state.modify.yaw );
		Bot[Z] = plane->valueAt( clipPos[0], clipPos[1] ) * plane->scale();
		Top[Z] = Bot[Z] - hypot_len * cos( angles[ROLL] );
	    } else {
		Top[X] = Bot[X] = clipPos[0] + microscope->state.modify.sweep_width
                                         * cos( microscope->state.modify.yaw );
		Top[Y] = Bot[Y] = clipPos[1] + microscope->state.modify.sweep_width
                                         * sin( microscope->state.modify.yaw );
		Bot[Z] = plane->valueAt( clipPos[0], clipPos[1] ) * plane->scale();
		Top[Z] = plane->maxAttainableValue() * plane->scale();
	    }
            graphics->positionSweepLine(Top, Bot);
	}
	switch ( userEvent ) {
	    
	case PRESS_EVENT:

          // ASSUMPTION:  we won't have a grab occur until after the
          // release event, so if we figure out an up vector now it'll
          // still be valid when we're done.
          // ASSUMPTION:  [0, 1, 0] in world space is a reasonable
          // up vector for the plane.  This will, of course, have to
          // be completely rewritten for uberGraphics.
          // For plane constraint.
          // TODO:  only do this if the constraint is actually on and
          // in line mode!
          // TCH 24 May 1999

          if (forceDevice) {
            v_xform_type room_from_world;
            q_vec_type world_up;
            float world_upf [3];

            v_x_invert(&room_from_world, &v_world.users.xforms[whichUser]);
            q_set_vec(world_up, 0.0, 1.0, 0.0);
            v_x_xform_vector(world_up, &room_from_world, world_up);

            world_upf[0] = world_up[0];
            world_upf[1] = world_up[1];
            world_upf[2] = world_up[2];
            forceDevice->setConstraintLineDirection(world_upf);
          }

	/* Request a reading from the current location,
         * and wait till tip gets there */
	  microscope->TakeFeelStep(clipPos[0], clipPos[1], value, 1);

	  plane_line_set = 0;

	break;

	case HOLD_EVENT:
	    /* determine whether we should switch into or out of modify force,
	     * based on the commit button and our last state */

	if (!tcl_commit_pressed) { 
	  // Commit button is not pressed - we are feeling the surface
	    if (old_commit_pressed) { // We were modifying last time
		// this means we should stop using modification force.
                microscope->ImageMode();
		old_commit_pressed = tcl_commit_pressed;
	    }
	    /* Request a reading from the current location */
	    microscope->TakeFeelStep(clipPos[0], clipPos[1]);
	    
	} else { // Commit button is pressed - we are modifying the surface
	    if (!old_commit_pressed) { // We weren't commited last time
		// this means we should start using modification force.
	       microscope->ModifyMode();
	       old_commit_pressed = tcl_commit_pressed;
	    }

	    /* Request a reading from the current location, 
	     * using the current modification style */
		 if ( microscope->state.modify.control == DIRECTZ) {
		  // direct Z control requires different parameters.
		  microscope->TakeDirectZStep(clipPos[0], clipPos[1], 
					      clipPos[2]);
	       } else {
		  microscope->TakeModStep(clipPos[0], clipPos[1]);
	       }
	}
	    
	if (( microscope->state.modify.control == DIRECTZ)&&(tcl_commit_pressed)) {
	    // Stop using the plane to apply force
	    
	    if( SurfaceGoing ) {
		if (forceDevice) {
		    forceDevice->stopSurface();
		    SurfaceGoing = 0;
		}
	    }
	    // Start using the forcefield to apply a constant force.
	    // Apply force to the user based on current measured force 
	    // XXX needs new test with new nmm_relaxComp object
	    if( microscope->state.relaxComp >= 0 ) {
		specify_directZ_force(whichUser);
		if (forceDevice && config_haptic_enable) {
		    forceDevice->sendForceField();
		    ForceFieldGoing = 1;
		}
	    }
	   
	} else {
	    /* Apply force to the user based on current sample points */
	    // XXX needs new test with new nmm_relaxComp object
	    if( microscope->state.relaxComp >= 0 ) {
		touch_live_to_plane_fit_to_line(whichUser, clipPos);
		if( !SurfaceGoing ) {
		    if (forceDevice && config_haptic_enable) {
		    	forceDevice->startSurface();
			SurfaceGoing = 1;
		    }
		}
	    }
	}
	break;
	
	case RELEASE_EVENT:	/* Go back to scanning upon release */

	    // Stop applying force.
	if( SurfaceGoing ) {
	   if (forceDevice) {
	   	forceDevice->stopSurface();
		SurfaceGoing = 0;
	   }
	}
	if( ForceFieldGoing ) {
	    if (forceDevice) {
		forceDevice->stopForceField();
		ForceFieldGoing = 0;
	    }
	}

	/* turn off the commit button if user releases the trigger. */
	tcl_commit_pressed = 0;
	old_commit_pressed = 0;
	if (microscope->state.modify.tool == CONSTR_FREEHAND) {
	   // Turn off constraint line, if it was on.
	   if (microscope->state.modify.constr_line_specified) {
	      microscope->state.modify.constr_line_specified = VRPN_FALSE;
	   }
	   // Remove any points from the constraint line list.
	   Position_list & p = microscope->state.modify.stored_points;
	   if (!p.empty()) {
	      // delete stored points (so we don't use them again!)
	      // get rid of the rubber band line.
	      //printf("Clearing stored polyline points.\n");
	      graphics->emptyPolyline();
	      
	      p.start();
	      while(p.notDone()) {
		 //printf("cancel - remove func: %d\n", (p.curr())->iconID());
		 // get rid of the icons marking the line endpoints
		 p.del();  //this moves the pointer forward to the next point.
	      }
	   }
	   
	}
	/* Start image mode and resume previous scan pattern */
        microscope->ResumeScan();

	break;

	default:
	    break;
	}

	return(0);
}


/*****************************************************************************
 *
   Select mode
   doSelect - Perform rubber-banding upon press, move, release
 *
 *****************************************************************************/

int 
doSelect(int whichUser, int userEvent)
{
	v_xform_type	worldFromHand;
	float	        centerx,centery;
	float		diffx,diffy, diffmax;
	float		x_min,y_min;
	float		x_max,y_max;

	/* if we are not running live, you should not be able
	   to do this, so put the user into grab mode */
	if (dataset->inputGrid->readMode() != READ_DEVICE)
	  {
	    handleCharacterCommand("G", &dataset->done, 1);
	    printf("Select mode available only on live data!!!\n");
	    return 0;
	  }

	BCPlane* plane = dataset->inputGrid->getPlaneByName
                     (dataset->heightPlaneName->string());
	if (plane == NULL)
	{
	    fprintf(stderr, "Error in doSelect: could not get plane!\n");
	    return -1;
	}  

	/* Move the tip to the hand x,y location */
	/* Set its height based on data at this point */
	v_get_world_from_hand(whichUser, &worldFromHand);

        /* Move the aiming line to the user's hand location */
        //nmui_Util::moveAimLine(worldFromHand.xlate);
        decoration->aimLine.moveTo(worldFromHand.xlate[0],
                                worldFromHand.xlate[1], plane);

	centerx = microscope->state.select_center_x;
	centery = microscope->state.select_center_y;

	// Figure out the differential in x and y from the center
	diffx = fabs(worldFromHand.xlate[0] - centerx);
	diffy = fabs(worldFromHand.xlate[1] - centery);

	// Find the larger of the two and use it for both, so it is square
	diffmax = max(diffx, diffy);

	// Clip the differential to make sure we stay within the boundaries
	// of the scan range.
	diffmax = min(diffmax, centerx - microscope->state.xMin);  // Don't go past left
	diffmax = min(diffmax, microscope->state.xMax - centerx);  // Don't go past right
	diffmax = min(diffmax, centery - microscope->state.yMin);  // Don't go past bottom
	diffmax = min(diffmax, microscope->state.yMax - centery);  // Don't go past top

	switch ( userEvent ) {

	    case PRESS_EVENT:	/* Store away hand location */
		centerx = worldFromHand.xlate[0];
		centery = worldFromHand.xlate[1];

		// Make sure we are within the maximum range
		centerx = max(centerx, microscope->state.xMin);
		centerx = min(centerx, microscope->state.xMax);
		centery = max(centery, microscope->state.yMin);
		centery = min(centery, microscope->state.yMax);

		microscope->state.select_center_x = centerx ;
		microscope->state.select_center_y = centery ;

		break;

	    case RELEASE_EVENT:	/* Find hand location, request area */
		x_min = centerx - diffmax;
		x_max = centerx + diffmax;
		y_min = centery - diffmax;
		y_max = centery + diffmax;

		/* save the region so when the Commit button is pressed, the
		 * new region can be sent to the microscope */
		microscope->state.select_center_x = centerx ;
		microscope->state.select_center_y = centery ;
		microscope->state.select_region_rad = diffmax ;

		break;

	    case HOLD_EVENT:
		x_min = centerx - diffmax;
		x_max = centerx + diffmax;
		y_min = centery - diffmax;
		y_max = centery + diffmax;
		graphics->positionRubberCorner(x_min,y_min,x_max,y_max);

		break;

	    default:
		break;
	}

	return(0);
}


/*****************************************************************************
 *
   doWorldGrab - handle world grab operation
 *
 *****************************************************************************/

int
doWorldGrab(int whichUser, int userEvent)
{
    v_xform_type    	    worldFromHand;
    v_xform_type            roomFromHand, roomFromSensor, handFromRoom;
    static v_xform_type     oldWorldFromHand;

  BCPlane* plane = dataset->inputGrid->getPlaneByName
                     (dataset->heightPlaneName->string());
  if (plane == NULL)
  {
      fprintf(stderr, "Error in doWorldGrab: could not get plane!\n");
      return -1;
  }     
    /* Move the aiming line to the users hand location */
    v_get_world_from_hand(whichUser, &worldFromHand);
    //nmui_Util::moveAimLine(worldFromHand.xlate);
    decoration->aimLine.moveTo(worldFromHand.xlate[0],
                            worldFromHand.xlate[1], plane);

switch ( userEvent )
    {

    case PRESS_EVENT:
	/* get snapshot of hand in world space == w_from_h  */
	v_get_world_from_hand(whichUser, &oldWorldFromHand);
	break;

    case HOLD_EVENT:
	/* get hand from room       */
	v_x_compose(&roomFromSensor,
		&v_users[whichUser].xforms[V_ROOM_FROM_HAND_TRACKER],
		&v_users[whichUser].xforms[V_TRACKER_FROM_HAND_SENSOR]);
	v_x_compose(&roomFromHand,
		&roomFromSensor,
		&v_users[whichUser].xforms[V_SENSOR_FROM_HAND_UNIT]);

	v_x_invert(&handFromRoom, &roomFromHand);

	/* this gives new world_from_room   */
	v_x_compose(&v_world.users.xforms[whichUser],
		&oldWorldFromHand, &handFromRoom);


        // NANOX
        updateWorldFromRoom();

	break;

    case RELEASE_EVENT:
    default:
	/* do nothing	*/
	break;
    }

return 0;
}	/* doWorldGrab */

/*****************************************************************************
 *
   doMeasureGridGrab - Control height of measuring grid

   * It is unclear what this function is supposed to do, even if it
   * was implemented...
 *
 *****************************************************************************/

int 
doMeasureGridGrab(int /*whichUser*/,  int userEvent)
{
	//v_xform_type	worldFromHand;
	//double		hand_height;

  switch ( userEvent ) {

    case HOLD_EVENT:

	/* Find the z location of hand in grid space */
	//v_get_world_from_hand(whichUser, &worldFromHand);
	//hand_height = worldFromHand.xlate[Z];

	/* Adjust the grid height to match */
	// XXX not implemented
	break;

    case PRESS_EVENT:
    case RELEASE_EVENT:
    default:
	break;
  }

	return(0);
}

// NANOX
void initializeInteraction (void) {

  q_vec_type lightdir;

  graphics->getLightDirection(&lightdir);

  tcl_lightDirX = lightdir[X];
  tcl_lightDirY = lightdir[Y];
  tcl_lightDirZ = lightdir[Z];

  updateWorldFromRoom();

//fprintf(stderr, "Starting world xlate is:  (%.5f %.5f %.5f)\n",
//(vrpn_float64) tcl_wfr_xlate_X, (vrpn_float64) tcl_wfr_xlate_Y,
//(vrpn_float64) tcl_wfr_xlate_Z);
//fprintf(stderr, "Starting world rot is:  (%.5f %.5f %.5f %.5f)\n",
//(vrpn_float64) tcl_wfr_rot_0, (vrpn_float64) tcl_wfr_rot_1,
//(vrpn_float64) tcl_wfr_rot_2, (vrpn_float64) tcl_wfr_rot_3);
//fprintf(stderr, "Starting world scale is %.5f\n",
//(vrpn_float64) tcl_wfr_scale);
}

static int lock_xform = 0;

void updateWorldFromRoom (void) {

  lock_xform = 1;

  tcl_wfr_xlate_X = v_world.users.xforms[0].xlate[0];
  tcl_wfr_xlate_Y = v_world.users.xforms[0].xlate[1];
  tcl_wfr_xlate_Z = v_world.users.xforms[0].xlate[2];
  tcl_wfr_rot_0 = v_world.users.xforms[0].rotate[0];
  tcl_wfr_rot_1 = v_world.users.xforms[0].rotate[1];
  tcl_wfr_rot_2 = v_world.users.xforms[0].rotate[2];
  tcl_wfr_rot_3 = v_world.users.xforms[0].rotate[3];
  tcl_wfr_scale = v_world.users.xforms[0].scale;

//fprintf(stderr, "Set TCL world xlate to:  (%.5f %.5f %.5f)\n",
//(vrpn_float64) tcl_wfr_xlate_X, (vrpn_float64) tcl_wfr_xlate_Y,
//(vrpn_float64) tcl_wfr_xlate_Z);
//fprintf(stderr, "Set TCL world rot to:  (%.5f %.5f %.5f %.5f)\n",
//(vrpn_float64) tcl_wfr_rot_0, (vrpn_float64) tcl_wfr_rot_1,
//(vrpn_float64) tcl_wfr_rot_2, (vrpn_float64) tcl_wfr_rot_3);
//fprintf(stderr, "Set TCL world scale to %.5f\n",
//(vrpn_float64) tcl_wfr_scale);
  lock_xform = 0;
}


// NANOX
void handle_worldFromRoom_change (vrpn_float64, void *) {
//#if 0
  if (lock_xform) {
    return;
  }

  v_world.users.xforms[0].xlate[0] = tcl_wfr_xlate_X;
  v_world.users.xforms[0].xlate[1] = tcl_wfr_xlate_Y;
  v_world.users.xforms[0].xlate[2] = tcl_wfr_xlate_Z;
  v_world.users.xforms[0].rotate[0] = tcl_wfr_rot_0;
  v_world.users.xforms[0].rotate[1] = tcl_wfr_rot_1;
  v_world.users.xforms[0].rotate[2] = tcl_wfr_rot_2;
  v_world.users.xforms[0].rotate[3] = tcl_wfr_rot_3;
  v_world.users.xforms[0].scale = tcl_wfr_scale;
//#endif
//fprintf(stderr, "Set VLIB world xlate to:  (%.5f %.5f %.5f)\n",
//(vrpn_float64) tcl_wfr_xlate_X, (vrpn_float64) tcl_wfr_xlate_Y,
//(vrpn_float64) tcl_wfr_xlate_Z);
//fprintf(stderr, "Set VLIB world rot to:  (%.5f %.5f %.5f %.5f)\n",
//(vrpn_float64) tcl_wfr_rot_0, (vrpn_float64) tcl_wfr_rot_1,
//(vrpn_float64) tcl_wfr_rot_2, (vrpn_float64) tcl_wfr_rot_3);
//fprintf(stderr, "Set VLIB world scale to %.5f\n",
//(vrpn_float64) tcl_wfr_scale);
}

/*============================================================================
 end interaction.c
 ===========================================================================*/

