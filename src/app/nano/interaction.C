/*===3rdtech===
  Copyright (c) 2000 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
/** \file interaction.c
 *
    interaction.c - handles all interaction for user program
    
    
    Overview:
    	- handles all button presses for grabbing & flying
    
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
 */
#include <stdio.h>

#include <quat.h>

#include <vrpn_RedundantTransmission.h>

#include <BCPlane.h>
#include <nmb_Dataset.h>
#include <nmb_Decoration.h>
#include <nmb_Types.h>
#include <nmb_Globals.h>
#include <nmb_Debug.h>
#include <nmb_Line.h>
#include <Tcl_Linkvar.h>
#include <Tcl_Netvar.h>
#include <nmb_TimerList.h>

#include <nmm_Globals.h>
#ifndef USE_VRPN_MICROSCOPE
#include <Microscope.h>
#else
#include <nmm_MicroscopeRemote.h>
#endif
#include <nmm_Types.h>  // for point_result, enums
#include <nmm_Sample.h>

#include <nmg_GraphicsImpl.h>
#include <nmg_Globals.h>

#include <nmui_Util.h>
#include <nmui_Haptics.h>
#include <nmui_HapticSurface.h>
#include <nmui_SurfaceFeatures.h>

#include "normal.h"
#include "relax.h"
#include "butt_mode.h"
#include "mf.h"

#include "microscopeHandlers.h"
#include "globals.h"
#include "microscape.h"  // for lots of #defines, graphicsTimer
#include "interaction.h"

#include "vrpn_ForceDevice.h"

/***************************
 * Local Defines
 ***************************/

// M_PI not defined for VC++, for some reason. 
#ifndef M_PI
#define M_PI		3.14159265358979323846
#define M_PI_2		1.57079632679489661923
#endif

#if (!defined(X) || !defined(Y) || !defined(Z))
#define	X	(0)
#define	Y	(1)
#define	Z	(2)
#endif

#define YAW	X
#define PITCH	Y
#define ROLL	Z

//  #define MAX_MV_KNOB 	(3)
//  #define MIN_MV_KNOB	(4)
//  #define MAX_MAX_MV	(200.0)

/** IS_AIM_MODE is true for modification modes (show aiming structure) */
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
/** Configure the force-display method. This is to allow us to turn off
forces or else make the user feel from a flat plane rather than from the
actual data set being displayed. These are to be used when we take the
system to Orange High School to enable us to determine how much the
actual feeling helps the students understand the surface.
*/
Tclvar_int	config_haptic_enable("enable_haptic", 1);
Tclvar_int	config_haptic_plane("haptic_from_flat", 0);

//static nmui_SurfaceFeatures haptic_features;
static nmui_HapticsManager haptic_manager;

static q_vec_type xy_pos;  // used for constrained freehand xyz

extern Tcl_Interp * get_the_interpreter (void);

/***************************
 * Mode of user operation
 ***************************/

// moved user_mode to globals.c

int	last_mode[NUM_USERS];		/**< Previous mode of operation */
int	last_style[NUM_USERS];		/**< Previous style of operation */

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


/** parameter locking the tip in sharp tip mode */
Tclvar_int xy_lock("xy_lock_pressed");

/// Trigger button from the virtual button box in Tcl
static int	tcl_trigger_just_forced_on = 0;
static int	tcl_trigger_just_forced_off = 0;
static void	handle_trigger_change( vrpn_int32 val, void *userdata );

Tclvar_int	tcl_trigger_pressed("trigger_pressed",0, handle_trigger_change);

/**
 * callback function for PHANToM reset button in tcl interface.
 **********/
static void handle_phantom_reset( vrpn_int32 val, void *userdata);

static void handle_handTracker_update_rate (vrpn_float64, void *);

/*********
 * Variables that are linked to buttons in the
 * tcl version of the button box.  They are tracked here to allow us to
 * run even when the normal button box is not operational.
 *********/

Tclvar_int	tcl_modify_pressed("modify_pressed");
Tclvar_int	tcl_commit_pressed("commit_pressed", 0 , handle_commit_change);
int             old_commit_pressed=0;
Tclvar_int	tcl_commit_canceled("cancel_commit", 0 , handle_commit_cancel);
Tclvar_int	tcl_phantom_reset_pressed("reset_phantom", 0, 
				handle_phantom_reset);

// turn linearization of haptics stimuli on/off
static void handle_friction_linear_change(vrpn_int32 val, void *userdata);
static void handle_bumpscale_linear_change(vrpn_int32 val, void *userdata);
static void handle_buzzscale_linear_change(vrpn_int32 val, void *userdata);

Tclvar_int friction_linear("friction_linear", 0, handle_friction_linear_change);
Tclvar_int adhesion_linear("adhesion_linear", 0);
Tclvar_int compliance_linear("compliance_linear", 0);
Tclvar_int bumpscale_linear("bumpscale_linear", 0,
			    handle_bumpscale_linear_change);
Tclvar_int buzzscale_linear("buzzscale_linear", 0,
			    handle_buzzscale_linear_change);

Tclvar_float handTracker_update_rate ("handTracker_update_rate", 60.0,
                                      handle_handTracker_update_rate);

/*********
 * Functions defined in this file (added by KPJ to satisfy g++)...
 *********/
void dispatch_event(int, int, int, nmb_TimerList *);
int qmf_lookat(q_type ,  q_vec_type,  q_vec_type,  q_vec_type);
int doLight(int, int);
int doFly(int, int);
int doScale(int, int, double);
int specify_directZ_force(int);
double touch_surface (int, q_vec_type);
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

// These defaults match those set in nmGraphics/graphics.c:resetLightDir()
TclNet_float tcl_lightDirX
     ("tcl_lightDirX", 0.0, handle_lightDir_change, NULL);
TclNet_float tcl_lightDirY
     ("tcl_lightDirY", 1.0, handle_lightDir_change, NULL);
TclNet_float tcl_lightDirZ
     ("tcl_lightDirZ", 0.1, handle_lightDir_change, NULL);

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


/** @struct FDOnOffMonitor
 * This code was repeated half-a-dozen-times, but needs three distinct
 * copies of the variables, so we can't just extract it into functions;
 * it needs to be in helper objects.
 */

struct FDOnOffMonitor {
  FDOnOffMonitor (void);

  void startSurface (void);
  void stopSurface (void);
  void startForceField (void);
  void stopForceField (void);

  vrpn_bool surfaceGoing;
  vrpn_bool forceFieldGoing;
};

FDOnOffMonitor::FDOnOffMonitor (void) :
  surfaceGoing (vrpn_FALSE),
  forceFieldGoing (vrpn_FALSE) {

}

void FDOnOffMonitor::startSurface (void) {

  if (!config_haptic_enable) {
    return;
  }

  if (!forceDevice) {
    return;
  }

  if (surfaceGoing) {
    forceDevice->sendSurface();
  } else {
    forceDevice->startSurface();
    surfaceGoing = vrpn_TRUE;
  }
}

void FDOnOffMonitor::stopSurface (void) {
  if (surfaceGoing && forceDevice) {
    forceDevice->stopSurface();
    surfaceGoing = vrpn_FALSE;
  }
}

void FDOnOffMonitor::startForceField (void) {

  if (!config_haptic_enable) {
    return;
  }

  if (forceDevice) {
    forceDevice->sendForceField();
    forceFieldGoing = vrpn_TRUE;
  }
}

void FDOnOffMonitor::stopForceField (void) {
  if (forceFieldGoing && forceDevice) {
    forceDevice->stopForceField();
    forceFieldGoing = vrpn_FALSE;
  }
}

class Adaptor {

  public:

    Adaptor (void);

    ~Adaptor (void);

    // MANIPULATORS

    void updateSampleAlgorithm (nmm_Microscope_Remote *);
    void setMicroscopeRTTEstimate (double);

  private:

    nmm_Sample d_sampleAlgorithm;
    double d_rttEstimate;
};

Adaptor::Adaptor (void) :
    d_rttEstimate (0.0) {

  d_sampleAlgorithm.numx = 5;
  d_sampleAlgorithm.numy = 5;
  d_sampleAlgorithm.dx = 5.0;
  d_sampleAlgorithm.dy = 5.0;
  d_sampleAlgorithm.orientation = 0.0;
}

Adaptor::~Adaptor (void) {

}

void Adaptor::updateSampleAlgorithm (nmm_Microscope_Remote * m) {
  static nmm_Sample sampleAlgorithm;
  BCGrid * grid;
  double targetsize;

  if (!m) {
    return;
  }

  if (d_rttEstimate < 0.015) {
     // BUG:  For less than 20ms we'd like to go to freehand,
     // but there isn't any way to do that.

    d_sampleAlgorithm.numx = 2;
    d_sampleAlgorithm.numy = 2;

  } else {

    // HACK:  Assume the microscope could take 300 samples per second.
    // Figure out what it could take in a time == RTT & have it take a
    // rectangular patch with a size approximately sqrt that.

    targetsize = d_rttEstimate * 300;
    d_sampleAlgorithm.numy = floor(sqrt(targetsize));
    d_sampleAlgorithm.numx = floor(targetsize / d_sampleAlgorithm.numy);
  }

  // HACK:  arbitrarily feel over 1/20th x 1/20th the area
  // Should this instead be with a resolution = or 1/2 the grid interval?

  grid = m->Data()->inputGrid;
  targetsize = grid->maxX() - grid->minX();
  d_sampleAlgorithm.dx = targetsize / 10;
  targetsize = grid->maxY() - grid->minY();
  d_sampleAlgorithm.dy = targetsize / 10;

  d_sampleAlgorithm.orientation = 0.0;

  collabVerbose(3, "Adaptor::updateSampleAlgorithm:  "
                "Estimate %.5f seconds of RTT;\n"
                "sample grid is %d x %d points at %.2f x %.2f nm spacing.\n",
                d_rttEstimate, d_sampleAlgorithm.numx, d_sampleAlgorithm.numy,
                d_sampleAlgorithm.dx, d_sampleAlgorithm.dy);

  m->SetSampleMode(&d_sampleAlgorithm);
}

void Adaptor::setMicroscopeRTTEstimate (double rtt) {
  d_rttEstimate = rtt;
}

static Adaptor adaptor;

void updateMicroscopeRTTEstimate (double time) {
  adaptor.setMicroscopeRTTEstimate(time);
}




static void handle_friction_linear_change(vrpn_int32 val, void *) {
  haptic_manager.surfaceFeatures().useLinearFriction(val);
}

static void handle_bumpscale_linear_change(vrpn_int32 val, void *) {
  haptic_manager.surfaceFeatures().useLinearBumps(val);
}

static void handle_buzzscale_linear_change(vrpn_int32 val, void *) {
  haptic_manager.surfaceFeatures().useLinearBuzzing(val);
}

static void handle_handTracker_update_rate (vrpn_float64 v, void *) {

  if (vrpnHandTracker[0]) {
    vrpnHandTracker[0]->set_update_rate(v);
//fprintf(stderr, "Set force device update rate to %.5f\n", v);
  } else {
//fprintf(stderr, "No force device, "
                //"so can't force device update rate to %.5f\n", v);
  }

}

static void handle_trigger_change( vrpn_int32 val, void * )
{
    if (val) {
	tcl_trigger_just_forced_on = 1;
    } else {
	tcl_trigger_just_forced_off = 1;
    }
}


/**
 * Step from the previous point to the current point.
 */

static void drawLineStep (Position * prevPt, Position * currPt) {
  double x, y;

  x = prevPt->x();
  y = prevPt->y();

  // If we are in sweep mode, set the angle "yaw" correctly
  // so the sweep is perpendicular to the line we draw.
  // Also draw an arc from the previous line to the current line. 
  if (microscope->state.modify.style == SWEEP) {
      float oldyaw = microscope->state.modify.yaw;
      microscope->state.modify.yaw = atan2((currPt->y() - y),
  		                           (currPt->x() - x)) - M_PI_2;
      if ( microscope->state.modify.yaw < 0 ) {
        microscope->state.modify.yaw += (2*M_PI);
      }
      if ( oldyaw < 0 ) {
        oldyaw += (2*M_PI);
      }
      if ( (oldyaw - microscope->state.modify.yaw) > M_PI) {
        microscope->DrawArc(x,y, oldyaw - (2*M_PI),
  			  microscope->state.modify.yaw);
      } else if ( (microscope->state.modify.yaw - oldyaw) > M_PI) {
        microscope->DrawArc(x,y, oldyaw,
  			  microscope->state.modify.yaw - (2*M_PI));
      } else {
        microscope->DrawArc(x,y, oldyaw,
  			  microscope->state.modify.yaw);
      }
  }

  // Draw a line between prevPt and currPt
  // Used to wait until we got the report before we sent the next line.
  //printf("drawLineStep: line %f %f to %f %f\n", 
  //	   x, y, currPt->x(), currPt->y());
  microscope->DrawLine(x, y, currPt->x(), currPt->y());
}

static void drawLine (void) {

    // set up a reference for convenience
    Position_list & p = 
      microscope->state.modify.stored_points;
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

    microscope->EnableUpdatableQueue(VRPN_FALSE);

    /* Do a poly-line modification!!! */
    // Wait for tip to get to starting position
   if ( (microscope->state.modify.tool == LINE) ||
	 (microscope->state.modify.tool == SLOW_LINE) ) {
      Point_value *value =
	microscope->state.data.inputPoint->getValueByPlaneName
	(dataset->heightPlaneName->string());
      if (value == NULL) {
	fprintf(stderr, "drawLine():  "
		"could not get input point!\n");
        microscope->EnableUpdatableQueue(VRPN_TRUE);
	return;
      }
      printf("drawLine: points in list, doing modify.\n");
      microscope->TakeFeelStep(p.currX(), p.currY(), value, 1);
    } else if (microscope->state.modify.tool == SLOW_LINE_3D) {
      microscope->TakeDirectZStep(p.currX(), p.currY(), p.currZ());
    }
    double x, y;
    Position * currPt;
    Position * prevPt;

    // sleep to allow piezo relaxation
    sleep(2);

    // XXX - might want to move everything below up to init_slow_line
    // into init_slow_line
        
    if ((microscope->state.modify.tool == SLOW_LINE)||
	(microscope->state.modify.tool == SLOW_LINE_3D)) {
      microscope->state.modify.slow_line_relax_done = VRPN_FALSE;
    }

    //printf("drawLine: peizo relax done\n");
    // start modification force!
    // We're already in modifcation mode using slow_line_3d
    if (microscope->state.modify.tool != SLOW_LINE_3D) {
      microscope->ModifyMode();
    }
          
    // Wait for the mode to finish (XXX should wait for response)
    // these calls to sleep should maybe be replaced with a 
    // barrier synch as used in init_slow_line
    sleep(1);
    //printf("drawLine: in modify mode.\n");

    // Slow line tool doesn't do the modification now, it
    // waits for the user to press Play or Step controls.
    if ((microscope->state.modify.tool == SLOW_LINE)||
	(microscope->state.modify.tool == SLOW_LINE_3D)) {
      microscope->state.modify.slow_line_committed = VRPN_TRUE;
      // Call this function to initialize the slow_line tool
      init_slow_line(microscope);
      microscope->EnableUpdatableQueue(vrpn_TRUE);
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
    //printf("drawLine: line %f %f to %f %f\n", 
    //       x, y, currPt->x(), currPt->y());
    microscope->DrawLine(x, y, currPt->x(), currPt->y());


    //start with the second point, so previous point is valid.
    for (p.next(); p.notDone(); p.next()) {
      drawLineStep(p.peekPrev(), p.curr());
    }
    // Delete these points so they are not used again. 
    p.start();
    while(p.notDone()) {
      //printf("commit - remove func: %d\n", (p.curr())->iconID());
      p.del();
    }

    // resume normal scanning operation of AFM
    printf("drawLine: done modifying, resuming scan.\n");

    microscope->EnableUpdatableQueue(VRPN_TRUE);

    microscope->ResumeScan();
    // All done - turn off commit button
    tcl_commit_pressed = 0;	
    old_commit_pressed = 0;

}

/**
 * callback function for Commit button in tcl interface.
 * It must prevent commit from being pressed at the wrong time, 
 * and do the correct thing when commit is pressed in line mode, and 
 * select mode. The commit button also starts modification in FeelLive mode,
 * but that is taken care of in doFeelLive().
 */

void handle_commit_change( vrpn_int32 , void *) // don't use val, userdata.
{
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
	    (microscope->state.modify.tool == SLOW_LINE)||
	    (microscope->state.modify.tool == SLOW_LINE_3D)) {

          drawLine();
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
	        microscope->state.select_center_x
                   - microscope->state.select_region_rad,
		microscope->state.select_center_y
                   - microscope->state.select_region_rad,
		microscope->state.select_center_x
                   + microscope->state.select_region_rad,
		microscope->state.select_center_y
                   + microscope->state.select_region_rad);

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

/**
 * callback function for Cancel commit button in tcl interface.
 * This button backs out of an operation, instead of commiting it.
 * In polyline mode - it clears any saved points. 
 * In select mode - it sets the region invalid, and clears its icon.
 */
void handle_commit_cancel( vrpn_int32, void *) // don't use val, userdata.
{
    printf("handle_commit_cancel called, cancel: %d\n", (int)tcl_commit_canceled);
    // This handles double callbacks, when we set tcl_commit_canceled to
    // zero below.
    if (tcl_commit_canceled != 1) return;

    // only allow cancel to be activated in selected modes.
    switch (user_mode[0]) {

    //if we are feeling before
    // a modify, (can be any tool: freehand, line, constrained freehand, ...)
    case USER_PLANEL_MODE:
    case USER_GRAB_MODE:
    case USER_SCALE_UP_MODE:
    case USER_SCALE_DOWN_MODE:
    case USER_LIGHT_MODE:
    case USER_MEASURE_MODE:
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

	    // For SLOW_LINE_3D tool: clear the markers along the rubber-band lines
	    if (microscope->state.modify.tool == SLOW_LINE_3D) {
	      decoration->num_slow_line_3d_markers = 0;
	    }

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


static void handle_phantom_reset (vrpn_int32, void *)
{
  printf("handle phantom reset invoked\n");
    if (tcl_phantom_reset_pressed != 1) return;

    if (reset_phantom())
	fprintf(stderr, "Error: could not reinitialize phantom\n");

    tcl_phantom_reset_pressed = 0;
    handle_phantom_reset( (vrpn_int32)-1, NULL);

}

void setupHaptics (int mode) {

  if (config_haptic_plane) {
    haptic_manager.setSurface(haptic_manager.d_measurePlane);
    haptic_manager.surfaceFeatures().setSurfaceFeatureStrategy(NULL);
    return;
  }

  switch (mode) {

    case USER_PLANE_MODE:

      haptic_manager.setSurface(haptic_manager.d_canned);
      haptic_manager.surfaceFeatures().setSurfaceFeatureStrategy
               (haptic_manager.d_gridFeatures);
      break;

    case USER_PLANEL_MODE:

      if (microscope->state.image.tool == FEELAHEAD) {
        haptic_manager.setSurface(haptic_manager.d_feelAhead);
        haptic_manager.surfaceFeatures().setSurfaceFeatureStrategy(NULL);
        // TODO:  invent a surface feature strategy!
        return;
      } 

      // fall through...

    case USER_LINE_MODE:

      haptic_manager.setSurface(haptic_manager.d_livePlane);
      haptic_manager.surfaceFeatures().setSurfaceFeatureStrategy
                               (haptic_manager.d_pointFeatures);
      break;


  }
}


/**
 *
 * dispatch_event - dispatch the given event based on the current mode and user
 *
 */
void dispatch_event(int user, int mode, int event, nmb_TimerList * /*timer*/)
{
    int ret = 0;

    // If no hand tracker this function shouldn't do anything
    if ( vrpnHandTracker[user] == NULL )
        return;

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
	   } else if ((microscope->state.modify.tool == CONSTR_FREEHAND)
		       && (!microscope->state.modify.constr_line_specified)) {
	       ret = doLine(user, event);
	   } else if ((microscope->state.modify.tool == CONSTR_FREEHAND_XYZ)
		       && (!microscope->state.modify.constr_line_specified)) {
	       ret = doLine(user, event);
	   } else if (microscope->state.modify.tool == SLOW_LINE || 
		      microscope->state.modify.tool == SLOW_LINE_3D ) {
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


/**
 *
   interaction - handles force level changes from knob box,
                 handles button presses from Phantom or other device 
		         based on current user mode.
		 Direct action presses are handled right here.
		 Otherwise, change the "hand" icon and call dispatch_event 
		 to do the real work.
 *
 */
int interaction(int bdbox_buttons[], double bdbox_dials[], 
		int phantButtonPressed, nmb_TimerList * timer)
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
  static int first_mode = 1;
  
  if(startup==0){
    for (i = 0; i < BDBOX_NUMBUTTONS; i++) lastButtonsPressed[i]=0;
    startup=1;
  }
  
  // NULL_EVENT=0, PRESS_EVENT=1, RELEASE_EVENT=2, HOLD_EVENT=3 in microscape.h
  // compute events for button box buttons
  // XXX doesn't fit vrpn model, should be rewritten to match Magellan in minit.c
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
    int		triggerButtonPressed = 0; 
    
    // get value for trigger button event
    // first check phantom button, then mouse button, then button box 
    // trigger, tcl_trigger
    if (phantButton && (phantom_button_mode!=2)) {
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
	dispatch_event(user, user_mode[user], eventList[0], timer);
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
	// It'd be better for the Phantom to be changed to expect
	// a default value of 0, or tcl_offsets to contain this shift,
	// so there!  Or at least it would be if we weren't constraining
	// values to [0,1] but to [-.5,.5] instead.
	Arm_knobs[i] = bdbox_dials[i] + tcl_offsets[i] + 0.5;
	//Arm_knobs[i] = bdbox_dials[i] + tcl_offsets[i];
	if (Arm_knobs[i] > 1.0) {
	  Arm_knobs[i] -= 1.0;
	} else if (Arm_knobs[i] < 0.0) {
	  Arm_knobs[i] += 1.0;
	}
      }
    
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
  // XXX doesn't fit vrpn model, should be rewritten to match Magellan in minit.c
    if( PRESS_EVENT == eventList[CENTER_BT] ) {
      if (timer) { timer->activate(timer->getListHead()); }
      center();
    }

    /* code dealing with locking the tip in sharp tip mode */
    /* locks the tip in one place so you can take repeated measurements */
    if( PRESS_EVENT == eventList[XY_LOCK_BT] ) {
      // save the position the user is currently at so we have
      // the x and y coords of their hand in the world, only the z varies
      // (CONSTR_FREEHAND_XYZ mode only)
      if (microscope->state.modify.tool == CONSTR_FREEHAND_XYZ) {
	nmui_Util::getHandInWorld(user, xy_pos);
      }
      xy_lock =1;
    }
    if( RELEASE_EVENT ==eventList[XY_LOCK_BT] ) {
      xy_lock =0;
    }

    /* Handle a "commit" or "cancel" button press/release on
	 * the real button box by causing a callback to
	 * the tcl routines */
    if( PRESS_EVENT == eventList[COMMIT_BT] ) {
      tcl_commit_pressed  = !tcl_commit_pressed; 
      if (tcl_commit_pressed) {
         printf("Commit from button box, now ON\n");
      } else {
         printf("Commit from button box, now OFF\n");
      }
      // we must call "handle_commit_change" explicitly, 
      // because tclvars are set to ignore changes from C.
      handle_commit_change(tcl_commit_pressed, NULL);
    }
    if( PRESS_EVENT == eventList[CANCEL_BT] ) {
      //printf("Cancel from button box.\n");
      tcl_commit_canceled = !tcl_commit_canceled; 
      // we must call "handle_commit_cancel" explicitly, 
      // because tclvars are set to ignore changes from C.
      handle_commit_cancel(tcl_commit_canceled, NULL);
    }

    // see if the user changed modes using a button
    mode_change |= butt_mode(eventList, user_mode+user);

    // also check to see if the modify style has changed -
    // can affect hand icon as well.
    if (user_mode[user] == USER_PLANEL_MODE) {
      mode_change |= microscope->state.modify.style_changed;
    }

    /* If there has been a mode change, clear old stuff and set new mode */
    if (mode_change) {
	if (timer) { timer->activate(timer->getListHead()); }
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
	dispatch_event(user, last_mode[user], RELEASE_EVENT, timer);
	dispatch_event(user, user_mode[user], PRESS_EVENT, timer);
      }		    

      VERBOSE(6, "    Calling graphics->setUserMode().");

      // Change icons to ones for this new mode.
      graphics->setUserMode(last_mode[user], last_style[user],
			    user_mode[user], microscope->state.modify.style);

      /* Last mode next time around is the current mode this time around */
      last_style[user] = microscope->state.modify.style;
      last_mode[user] = user_mode[user];

      /* Make the associated sound */
      // Taken out for now

      /* Clear the mode change flag */
      mode_change = 0;
      microscope->state.modify.style_changed = 0;
    }
    else if ( first_mode == 1 ) {
      first_mode = 0;
      last_style[user] = microscope->state.modify.style;
      last_mode[user] = user_mode[user];
    }

    // quick approximation
    switch (eventList[0]) {
    case PRESS_EVENT:
    case RELEASE_EVENT:
    case HOLD_EVENT:
      if (timer) {
	timer->activate(timer->getListHead());
      }
      break;
    default:
      break;

    }

    VERBOSE(6, "    Calling dispatch_event().");

	/* Handle the current event, based on the user mode and user */
    dispatch_event(user, user_mode[user], eventList[0], timer);
    lastTriggerEvent = eventList[0];


  }
  // TCH - HACK but it works
  decoration->user_mode = user_mode[0];

  VERBOSE(4, "  Done with interaction().");

  return 0;
}	/* interaction */


/**
 * qmf_lookat - produce an orientation quaternion that will line up the
 *	viewing vector (+Z) from from to at, and the Y axis roughly along
 *	up.
 *
 */
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
  
//.......................................................................

/**
 *
   doLight - adjust the position of the light
 *
 */
int
doLight(int whichUser, int userEvent)
{
  //VectorType		lightpos; //, lightdir;
  q_vec_type            lightdir;
  q_vec_type		q_tmp;
  v_xform_type	worldFromPart;
  //v_xform_type	PartFromWorld;
  q_type		q_room;
  //static v_xform_type	oldWorldFromHand;
  //static q_vec_type	oldLightDir;

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
   * shine along the -y-axis in hand-space.  
   */
  q_tmp[X] = ( -lightDirection[1][0] );
  q_tmp[Y] = ( -lightDirection[1][1] );
  q_tmp[Z] = ( -lightDirection[1][2] );

  switch ( userEvent ) {
    case PRESS_EVENT:

	break;

    case HOLD_EVENT:

	// Any rotation that the hand undergoes, we 
	// will apply the same rotation to the light direction.
	
	/* Rotate light from world space to room space */
        v_get_world_from_head(whichUser, &worldFromPart);

	q_copy(q_room, worldFromPart.rotate);
	q_invert(q_room, q_room);
	q_xform(q_tmp, q_room, q_tmp);

	lightdir[X] = -q_tmp[X]; 
	lightdir[Y] = -q_tmp[Y];
	lightdir[Z] = -q_tmp[Z];

        VectorNormalize( lightdir );

	/* position the light where the top of the user's hand is */

        // NANOX
        // Don't do anything until the network confirms this move
        // to us - in handle_lightDir_change below.
        //graphics->setLightDirection(lightdir);

        // NANOX
	tcl_lightDirX = lightdir[X];
        tcl_lightDirY = lightdir[Y];
        tcl_lightDirZ = lightdir[Z];

	break;

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
  //printf("New light dir %f %f %f\n",lightdir[X] , lightdir[Y], lightdir[Z]); 
  graphics->setLightDirection(lightdir);
}

/**
 *
   doFly - fly through the world when button is pressed
	 - scale motion based on user's size in the world
	   (Allows a person to fly faster when they are huge)
 *
 */
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

/**
 *
   doScale - scale the world when button is pressed
 *
 * Scale around the intersection of the aim line with the height plane.
 * Hopefully this will keep the surface from moving out of reach.
 */
int
doScale(int whichUser, int userEvent, double scale_factor)
{
    v_xform_type            scaleXform = V_ID_XFORM; 
    v_xform_type    	    handFromObject;
    v_xform_type    	    scaledWorldFromHand;
    v_xform_type    	    handFromWorld;
    v_xform_type    	    worldFromHand = V_ID_XFORM;

    BCPlane* plane = dataset->inputGrid->getPlaneByName(dataset->heightPlaneName->string());
    if (plane == NULL) {
      fprintf(stderr, "Error in doScale: could not get plane!\n");
      return -1;
    }     
    v_get_world_from_hand(whichUser, &worldFromHand);
    decoration->aimLine.moveTo(worldFromHand.xlate[0],
			       worldFromHand.xlate[1], plane);
    
    switch ( userEvent )
      {
      case HOLD_EVENT:
        v_xform_type temp;
        v_x_copy(&temp, &v_world.users.xforms[whichUser]);
	

        // Very similar operation to v_scale_about_hand, 
        // but we scale around the intersection of the aim line with
        // the height plane. 

        // get spot to scale from in world space. 
        decoration->aimLine.getIntercept(worldFromHand.xlate, plane);

        // If the plane height has been exaggerated, we need to 
        // adjust the translation point by that scale to get world coords. 
        worldFromHand.xlate[2] *= plane->scale();
     
        // now scale by scale amount	
        scaleXform.scale = scale_factor;

        // scaled_w_f_ha = w_f_h * scale   
        v_x_compose(&scaledWorldFromHand, &worldFromHand, &scaleXform);
        
        //scaling's done, so get back into object space 
        v_x_invert(&handFromWorld, &worldFromHand);
    
        // ha_f_o = ha_f_w * w_f_o  
        v_x_compose(&handFromObject, &handFromWorld, &temp);
    
        // scaled_w_f_o = scaled_w_f_h * h_f_o	
        v_x_compose(&temp, &scaledWorldFromHand, &handFromObject);

collabVerbose(5, "doScale:  updateWorldFromRoom().\n");

        updateWorldFromRoom(&temp);
	break;
	
      case PRESS_EVENT:
      case RELEASE_EVENT:
      default:
    	/* do nothing if no button pressed  */
	break;
    }

return 0;
}	/* doScale */

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


/** @function touch_surface()
 * Replaces touch_canned_from_plane(), touch_flat_from_measurelines(),
 * and touch_live_to_plane_fit_to_line() by moving their state and
 * the differences between their algorithms into libnmui:
 * nmui_HapticSurface defines the plane, and nmui_SurfaceFeatures
 * defines the auxiliary haptic channels (buzzing, bumping, &c).
 * nmui_SurfaceFeatures should have been passed to the global
 * haptic_manager.surfaceFeatures() last time the mode was changed.
 */

// BUG must sendSurface sometimes?

double touch_surface (int, q_vec_type handpos) {

  // Set up the approximating plane or force field...

  haptic_manager.surface()->setLocation(handpos);
  haptic_manager.surface()->update(microscope);
  haptic_manager.surface()->sendForceUpdate(forceDevice);

  // Set up buzzing, bumps, friction, compliance, ...

  haptic_manager.surfaceFeatures().update(microscope);

  return haptic_manager.surface()->distanceFromSurface();
}



/** When controling the position of the tip in 3D, this calculates
 * the force the user should feel based on internal sensor measurements.
 */
int specify_directZ_force(int whichUser) 
{
     q_vec_type		        point;
     q_vec_type		up = { 0.0, 0.0, 1.0 };
     v_xform_type               WorldFromTracker, TrackerFromWorld;

     // If directz hasn't been initialized, do nothing. 
     if (microscope->state.modify.freespace_normal_force == BOGUS_FORCE) {
	 return 0;
     }
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

/// used in meas_cross 
#define IABS(i,e)	(((i)=(e)) < 0 ? (i)=-(i) : (i))
/**
 *
   meas_cross - draw out the cross section between two points.  draws only
     nearest neighbor calc of grid point values at grid x,y's.
 *
*/
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
  if( (Top[X] <= plane->maxX())
      &&
      (Top[X] >= plane->minX())
      &&
      (Top[Y] <= plane->maxY())
      &&
      (Top[Y] >= plane->minY())
      ) {
      double z_val;
      plane->valueAt( &z_val, x0, y0 );
      Top[Z] = (float)(z_val*z_scale);
  } else {
      Top[Z] = 0.0;
  }
  
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
        ) {
        
        double z_val;
    	plane->valueAt( &z_val, Bot[X], Bot[Y] );
        Bot[Z]=(float)(z_val*z_scale);
    } else {
        Bot[Z] = 0.0;
    }
    Top[X] = Bot[X]; Top[Y] = Bot[Y]; Top[Z] = Bot[Z];
  }
  return 0;
}

#undef IABS


#define RED   1    ///< index of red line, for convenience in doMeasure
#define GREEN 2    ///< index of green line, for convenience in doMeasure
#define BLUE  3    ///< index of blue line, for convenience in doMeasure
/**
*
   doMeasure - Moving the red, green, blue measure lines around.
*
*/
int doMeasure(int whichUser, int userEvent)
{ 
        q_vec_type clipPos;
	double          rdxy,gdxy,bdxy;
	static          int              hold_color;
	static          int              ishold = 0;

        BCPlane* plane = dataset->inputGrid->getPlaneByName
                     (dataset->heightPlaneName->string());
        if (plane == NULL)
        {
            fprintf(stderr, "Error in doMeasure: could not get plane!\n");
            return -1;
        }
//fprintf(stderr, "doMeasure:  hpn %s.\n", dataset->heightPlaneName->string());

	// Move the tip to the hand x,y location 
	// Set its height based on data at this point
	nmui_Util::getHandInWorld(whichUser, clipPos);
        nmui_Util::clipPosition(plane, clipPos);

        decoration->red.normalize(plane);
        decoration->green.normalize(plane);
        decoration->blue.normalize(plane);

        // Speed may be pointless here, but we were doing three sqrt(),
        // which are completely unnecessary, since sqrt() preserves
        // ordering.

        rdxy = ( (clipPos[0] - decoration->red.x()) *
                     (clipPos[0] - decoration->red.x()) +
                     (clipPos[1] - decoration->red.y()) *
                     (clipPos[1] - decoration->red.y()) );
          
        gdxy = ( (clipPos[0] - decoration->green.x()) *
                     (clipPos[0] - decoration->green.x()) +
                     (clipPos[1] - decoration->green.y()) *
                     (clipPos[1] - decoration->green.y()) );
         
        bdxy = ( (clipPos[0] - decoration->blue.x()) *
                     (clipPos[0] - decoration->blue.x()) +
                     (clipPos[1] - decoration->blue.y()) *
                     (clipPos[1] - decoration->blue.y()) );
      
        if (ishold == 0) {
          graphics->setHandColor(BLUE);
          if ((rdxy <= bdxy) && (rdxy <= gdxy)) {
             graphics->setHandColor(RED);
          } else if (gdxy <= bdxy) {
             graphics->setHandColor(GREEN);
          }
        }

        // NANOX
        // Don't call moveTo() - instead, just call doCallbacks().
        // That will send the changes through Tcl, which will do
        // network synchronization if necessary, and the Tcl callbacks
        // will finally call moveTo().

        switch(userEvent) {
        
	   case PRESS_EVENT:

             hold_color = graphics->getHandColor();

           case HOLD_EVENT: 

	     ishold = 1;
	     if (hold_color == RED) { 
               decoration->red.doCallbacks(clipPos[0], clipPos[1], plane);
	     }
	     else if (hold_color == GREEN) {
               decoration->green.doCallbacks(clipPos[0], clipPos[1], plane);
	     }
	     else if (hold_color == BLUE){
               decoration->blue.doCallbacks(clipPos[0], clipPos[1], plane);
	     }
	     break;

	   case RELEASE_EVENT:

	     ishold = 0;

           default:

             break;
        }
    return 0;
}
#undef RED
#undef GREEN
#undef BLUE



/**
 *
   doLine - Let the user specify points in a poly-line while feeling.
            When they press commit, do this (see handle_commit_pressed):
            Increase force and move from one point to another, then
	       decrease force, engraving straight line from point A to
	       point B.
	    Pause at the start point before moving to allow the offset
		due to piezo relaxation to go away.
 *
 */
int doLine(int whichUser, int userEvent)
{
	v_xform_type	worldFromHand;
        q_vec_type      clipPos, clipPosNM;
        static FDOnOffMonitor monitor;
        q_matrix_type	hand_mat;
	q_vec_type	angles;
	PointType 	TopL, BottomL, TopR, BottomR; // sweepline markers
	vrpn_bool       valid_Direct_Z_Point;

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
	nmui_Util::getHandInWorld(whichUser, clipPos);
        nmui_Util::clipPosition(plane, clipPos);

	// If this is 3d slowline, we need to have the Z converted into
	// NM (divide by the plane scale).  As noted in doFeelLive(), if
	// the plane scale approaches zero, then nmOK becomes false;
	
	q_vec_copy(clipPosNM, clipPos);
	valid_Direct_Z_Point = nmui_Util::convertPositionToNM(plane, clipPosNM);

        /* Move the aiming line to the user's hand location */
	// re-draw the aim line and the red sphere representing the tip.
        //nmui_Util::moveAimLine(clipPos);
        decoration->aimLine.moveTo(clipPos[0], clipPos[1], plane);
        nmui_Util::moveSphere(clipPos, graphics);


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
  	    	microscope->state.modify.yaw =
 		  atan2((pos_list.currY()-clipPos[1]),
  			(pos_list.currX()-clipPos[0])) + M_PI_2;
	    } else {
		// otherwise, set the angle based on hand position.

		v_get_world_from_hand(whichUser, &worldFromHand);
		q_to_col_matrix(hand_mat, worldFromHand.rotate);
		q_col_matrix_to_euler( angles, hand_mat );
		microscope->state.modify.yaw = angles[YAW] + M_PI_2;
	    }

	    TopL[X] = BottomL[X] = clipPos[0] +
	      (microscope->state.modify.sweep_width 
	       * cos( microscope->state.modify.yaw ))/2.0;
	    TopL[Y] = BottomL[Y] = clipPos[1] +
	      (microscope->state.modify.sweep_width 
	       * sin( microscope->state.modify.yaw ))/2.0;
	    
	    TopR[X] = BottomR[X] = clipPos[0] -
	      (microscope->state.modify.sweep_width 
	       * cos( microscope->state.modify.yaw ))/2.0;
	    TopR[Y] = BottomR[Y] = clipPos[1] -
	      (microscope->state.modify.sweep_width 
	       * sin( microscope->state.modify.yaw ))/2.0;
	    
            double z_val;
            plane->valueAt( &z_val, clipPos[0], clipPos[1] );
	    BottomL[Z] =BottomR[Z] = (float)(z_val*plane->scale());

	    TopL[Z] = TopR[Z] = plane->maxAttainableValue() *
		  plane->scale();
	    
	    //draw vertical green line representing sweep width.
	    graphics->positionSweepLine(TopL, BottomL, TopR, BottomR);
	}

        setupHaptics(USER_LINE_MODE);

VERBOSE(8, "      doLine:  starting case statement.");    

	switch ( userEvent ) 
	    {
	    // Allow user to feel around to find first location in polyline
	    case PRESS_EVENT:
		/* Request a reading from the current location,
		 * and wait till tip gets there */

	      // for CONSTR_FREEHAND_XYZ: the line is specified in 3-space, not
	      // 2-space, so you have to consider the z-coord as well.  Therefore
	      // this mode should do the same thing slow_line_3d does
	      if ( (microscope->state.modify.tool == SLOW_LINE_3D) ||
		   (microscope->state.modify.tool == CONSTR_FREEHAND_XYZ) ) {
		if ( pos_list.empty() ) {
		    microscope->TakeFeelStep(clipPos[0], clipPos[1], value, 1);
		    microscope->ModifyMode();
		}
		else{
		  microscope->TakeDirectZStep(clipPosNM[0], clipPosNM[1],
					    clipPosNM[2], value, 0);
		}
	      }
	      else{    
		microscope->TakeFeelStep(clipPos[0], clipPos[1], value, 1);
      	      }

		// update the position of the rubber-band line
		graphics->setRubberLineEnd(clipPos[0], clipPos[1]);
		graphics->setRubberSweepLineEnd(TopL, TopR);
		break;

	case HOLD_EVENT:
	    /* Feel the surface, to help determine the next point. */
	    /* Request a reading from the current location */
	  
	  // same logic as noted above for the press event for constr_free_xyz
	    if ((microscope->state.modify.tool == SLOW_LINE_3D) ||
		(microscope->state.modify.tool == CONSTR_FREEHAND_XYZ)) {
	      if(!tcl_commit_pressed) {
		if (valid_Direct_Z_Point) {
		  microscope->TakeDirectZStep(clipPosNM[0], clipPosNM[1], 
					  clipPosNM[2]);
		}
	      }
	    }
	    else{
	      if(!tcl_commit_pressed) {
	        microscope->TakeFeelStep(clipPos[0], clipPos[1], value, 1);
	      }
	    }

	    // update the position of the rubber-band line
	    graphics->setRubberLineEnd(clipPos[0], clipPos[1]);
	    graphics->setRubberSweepLineEnd(TopL, TopR);
	    if ( ((microscope->state.modify.tool == SLOW_LINE_3D) ||
		  (microscope->state.modify.tool == CONSTR_FREEHAND_XYZ) ) &&
		(tcl_commit_pressed) ) {

	      // Stop using the plane to apply force (?)
              monitor.stopSurface();
	      specify_directZ_force(whichUser);
              monitor.startForceField();
	    }
	    else {
	    
	    /* Apply force to the user based on current sample points */
	    // XXX needs new test with new nmm_relaxComp object
	    //if( microscope->state.relaxComp >= 0 ) {
              touch_surface(whichUser, clipPos);
              monitor.startSurface();
	      //}
	    }
	    break;

	    case RELEASE_EVENT:
	      {
		/* Save current hand position as one point on the polyline-
		 * The list is saved in microscope->state.modify.stored_points*/
		//printf("Line mode release event\n");


		/* don't send a stop surface when not surfacing (?)
		 */
                monitor.stopSurface();
                monitor.stopForceField();

		int list_id;
 		if (microscope->state.modify.style == SWEEP) {
 		    graphics->addPolySweepPoints(TopL, BottomL, TopR, BottomR);
 		}
  		else {
		    //add an icon to represent this spot as part of the polyline.
		    PointType * markpts = new PointType[2];
		    markpts[0][0] = markpts[1][0] = clipPos[0];
		    markpts[0][1] = markpts[1][1] = clipPos[1];
		    markpts[0][2] = plane->maxAttainableValue()*plane->scale();
		    markpts[1][2] = plane->minAttainableValue()*plane->scale();
		    
		    list_id = graphics->addPolylinePoint(markpts);
		}
		// Now we do some different things depending on
		// whether this is LINE tool or CONSTR_FREEHAND
		// tool. LINE or SLOW_LINE tool can keep adding points
		// indefinitely - modification gets done automatically
		// when "commit" is pressed. CONSTR_FREEHAND switches
		// after two points are specified so that the next
		// PRESS event will be handled by doFeelLive -
		// modification freehand when we hit "commit".
		if ((microscope->state.modify.tool == LINE)||
		    (microscope->state.modify.tool == SLOW_LINE)||
		    (microscope->state.modify.tool == SLOW_LINE_3D)) {
		   graphics->setRubberLineStart(clipPos[0], clipPos[1]);
		   graphics->setRubberSweepLineStart(TopL, TopR);
		   //save this point as part of the poly-line
		   if (microscope->state.modify.tool == SLOW_LINE_3D) {
		     // Insert points in reverse order so we start
		     // modifying from the last point we add. 
		     pos_list.insertPrev(clipPosNM[0], clipPosNM[1],
				     clipPosNM[2], list_id);
		     // add a visual marker to the decorations already present
		     // on screen
		     //decoration->addSlowLine3dMarker( clipPos[0], clipPos[1],
		     //			      clipPos[2]);
		   }
		   else {
		     pos_list.insert(clipPos[0], clipPos[1], list_id);
		   }
		} else if ((microscope->state.modify.tool == CONSTR_FREEHAND) ||
			   (microscope->state.modify.tool == CONSTR_FREEHAND_XYZ)) {
		   // If this is the second point, set a flag so we
		   // switch to doFeelLive on next event.
		   if (!pos_list.empty()) {
		      microscope->state.modify.constr_line_specified = VRPN_TRUE;
		      // also leave rubber line stretched between two
		      // endpoints.
		   } else {
		      graphics->setRubberLineStart(clipPos[0], clipPos[1]);
		      graphics->setRubberSweepLineStart(TopL, TopR);
		   }
		   //save this point as part of the poly-line
		   if (microscope->state.modify.tool == CONSTR_FREEHAND) {
		     pos_list.insert(clipPos[0], clipPos[1], list_id);
		   } else {
		     pos_list.insert(clipPos[0], clipPos[1], clipPos[2], list_id);
		   }
		   
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

/**
 *
   doFeelFromGrid - Control tip in x and y, updating grid in vicinity of hand.
     Feel contours of grid surface, NOT the real data.
     "Demo mode" for touching the grid.
 *
 */
int doFeelFromGrid(int whichUser, int userEvent)
{
        q_vec_type clipPos;
        static FDOnOffMonitor monitor;
	double          aboveSurf;

	BCPlane* plane = dataset->inputGrid->getPlaneByName
                     (dataset->heightPlaneName->string());
	if (plane == NULL)
	{
	    fprintf(stderr, "Error in doFeelFromGrid: could not get plane!\n");
	    return -1;
	}     

	/* Find the x,y location of hand in grid space */
	nmui_Util::getHandInWorld(whichUser, clipPos);
        nmui_Util::clipPosition(plane, clipPos);

	/* Move the aiming line to the user's hand location */
        //nmui_Util::moveAimLine(clipPos);
        decoration->aimLine.moveTo(clipPos[0], clipPos[1], plane);
        nmui_Util::moveSphere(clipPos, graphics);

    setupHaptics(USER_PLANE_MODE);

	switch ( userEvent ) 
	  {
	  case PRESS_EVENT:
	    /* stylus has just been pressed */
	    /* if user is below the surface on the initial press, don't send
	       the surface, because the user will get a strong upward force. */
	    aboveSurf = touch_surface(whichUser, clipPos);
//fprintf(stderr, "doFeelFromGrid() PRESS %.5f above surface.\n", aboveSurf);
	    if (aboveSurf > 0) {
              monitor.startSurface();
	    }
	    break;

	  case HOLD_EVENT:
	    /* stylus continues to be pressed */
	    /* Apply force to the user based on grid */
	    aboveSurf = touch_surface(whichUser, clipPos);
//fprintf(stderr, "doFeelFromGrid() HOLD %.5f above surface.\n", aboveSurf);
	    if (monitor.surfaceGoing || (aboveSurf > 0)) {
              monitor.startSurface();
	    }
	    
	    break;
	  
	  case RELEASE_EVENT:	/* Go back to scanning upon release */
	    
	    /* ArmLib doesn't like a stop surface when it's not surfacing 
	    **/
            monitor.stopSurface();
	    
	    break;
	    
	  default:
	    break;
	  }
	
	return(0);
}


static void setupSweepIcon (int whichUser, q_vec_type clipPos,
                            BCPlane * plane) {
  PointType TopL, BottomL, TopR, BottomR;
  v_xform_type worldFromHand;
  q_matrix_type	hand_mat;
  q_vec_type angles;
  double z_val;

  /* now set up the sweep direction and length, 
   * based on hand position (hand_mat).
   */

  v_get_world_from_hand(whichUser, &worldFromHand);
  q_to_col_matrix(hand_mat, worldFromHand.rotate);
  q_col_matrix_to_euler( angles, hand_mat );
  microscope->state.modify.yaw = angles[YAW] + M_PI_2;
    
  TopL[X] = BottomL[X] = clipPos[0] +
      (microscope->state.modify.sweep_width 
       * cos( microscope->state.modify.yaw ))/2.0;
  TopL[Y] = BottomL[Y] = clipPos[1] +
      (microscope->state.modify.sweep_width 
       * sin( microscope->state.modify.yaw ))/2.0;
      
  TopR[X] = BottomR[X] = clipPos[0] -
      (microscope->state.modify.sweep_width 
       * cos( microscope->state.modify.yaw ))/2.0;
  TopR[Y] = BottomR[Y] = clipPos[1] -
      (microscope->state.modify.sweep_width 
       * sin( microscope->state.modify.yaw ))/2.0;
      
  plane->valueAt( &z_val, clipPos[0], clipPos[1] );

  BottomL[Z] =BottomR[Z] = (float)(z_val*plane->scale());

  TopL[Z] = TopR[Z] = plane->maxAttainableValue() * plane->scale();

  //draw vertical green line representing sweep width.
  graphics->positionSweepLine(TopL, BottomL, TopR, BottomR);

}

/**
 *
   doFeelLive - Control tip in x and y, microscope returns z at each point.
	We use the last two returned points to determine a plane and present
	that plane to the user as a surface.
        When user presses "commit" button, switch to appropriate modify mode.
	Now includes ability to handle Direct Z control, where user
	moves the tip in x, y and z.
 *
 */
int doFeelLive (int whichUser, int userEvent)  {
  BCPlane * plane;

  // static to allow xy_lock to work properly
  static q_vec_type clipPos;
  static q_vec_type clipPosNM;

  static FDOnOffMonitor monitor;

  vrpn_bool nmOK;

  /* if we are not running live, you should not be able
     to do this, so put the user into grab mode */
  if (dataset->inputGrid->readMode() != READ_DEVICE) {
    handleCharacterCommand("G", &dataset->done, 1);
    printf("SharpTip mode available only on live data!!!\n");
    return 0;
  }

  // If we are running live, but we don't have control of the
  // microscope, we can't do this, so put the user into grab mode.

  if (!microscope->haveMutex()) {
    handleCharacterCommand("G", &dataset->done, 1);
    printf("Can't touch when we don't have access to the microscope.\n");
    return 0;
  }

  // Get the input plane and point
  plane = dataset->inputGrid->getPlaneByName
                      (dataset->heightPlaneName->string());
  if (!plane) {
    fprintf(stderr, "Error in doFeelLive: could not get plane!\n");
    return -1;
  }  

   // Get proper name for dataset -- CCWeigle 09/14/99
   // The following lifted from Point_results::getValueByPlaneName()
   char fullname[100];

   fullname[sizeof(fullname)-1] = '\0';
   strncpy(fullname, dataset->heightPlaneName->string(), sizeof(fullname)-1);
   if (!strrchr(fullname, '-')) {
      fprintf(stderr, "doFeelLive(): problem with plane name %s\n",
              (char*)dataset->heightPlaneName->string());
      return -1;
   }
   *strrchr(fullname, '-') = '\0';

   //
   // According to the comments, Channel_selector::Set does everything we need
   // 

   Point_channel_selector * point_selector =
         microscope->state.data.point_channels;

   if (point_selector->Is_set(fullname) != 1) {
       if (-1 == point_selector->Set(fullname)) {
	   fprintf(stderr, "doFeelLive(): couldn't activate dataset %s\n",
              fullname);
	   return -1;
       }
   }

   Point_value * value =
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
         nmui_Util::getHandInWorld(whichUser, clipPos);
         nmui_Util::clipPositionLineConstraint(plane, clipPos, 
                   microscope->state.modify.stored_points);
     } else if (microscope->state.modify.tool == CONSTR_FREEHAND_XYZ) {
         nmui_Util::getHandInWorld(whichUser, clipPos);
         nmui_Util::clipPositionLineConstraint(plane, clipPos, 
                   microscope->state.modify.stored_points,
		   microscope->state.modify.tool,
		   microscope->state.modify.constr_xyz_param);
     } else {
         nmui_Util::getHandInWorld(whichUser, clipPos);
         nmui_Util::clipPosition(plane, clipPos);
     }
  } else if (microscope->state.modify.tool == CONSTR_FREEHAND_XYZ) {
    nmui_Util::getHandInWorld(whichUser, clipPos);
    clipPos[0] = xy_pos[0];
    clipPos[1] = xy_pos[1];
  }

  // Used only for direct Z control - we need to have the Z converted
  // into NM (divided by plane scale).  If plane scale approaches
  // zero, nmOK becomes false

  q_vec_copy(clipPosNM, clipPos);
  nmOK = nmui_Util::convertPositionToNM(plane, clipPosNM);

  // Move the aiming line to the user's hand location
  decoration->aimLine.moveTo(clipPos[0], clipPos[1], plane);
  nmui_Util::moveSphere(clipPos, graphics);


  // if the style is sweep, set up additional icon for sweep width
  if (microscope->state.modify.style == SWEEP) {
    setupSweepIcon(whichUser, clipPos, plane);
  }

  switch ( userEvent ) {
	    
    case PRESS_EVENT:

#if 0
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
        _float world_upf [3];

        v_x_invert(&room_from_world, &v_world.users.xforms[whichUser]);
        q_set_vec(world_up, 0.0, 1.0, 0.0);
        v_x_xform_vector(world_up, &room_from_world, world_up);

        world_upf[0] = world_up[0];
        world_upf[1] = world_up[1];
        world_upf[2] = world_up[2];
        forceDevice->setConstraintLineDirection(world_upf);
      }
#endif

      if (microscopeRedundancyController) {
        // Instead of using TCP to transmit reliably,
        // send everything via UDP 3 times at 25 ms intervals.
        //microscopeRedundancyController->set(2, vrpn_MsecsTimeval(0.025));
      }

      /* Request a reading from the current location,
       * and wait till tip gets there */

      microscope->TakeFeelStep(clipPos[0], clipPos[1], value, 1);

      break;

    case HOLD_EVENT:

      if (microscope->state.modify.tool == FEELAHEAD) {

        // Feelahead mode IGNORES the commit button;  it never
        // leaves image mode.

fprintf(stderr, "Feeling to %.2f, %.2f.\n", clipPos[0], clipPos[1]);

        adaptor.updateSampleAlgorithm(microscope);

        microscope->TakeSampleSet(clipPos[0], clipPos[1]);

      } else if (!tcl_commit_pressed) { 

        // Commit button is not pressed - we are feeling the surface

        if (old_commit_pressed) { // We were modifying last time
      	        // this means we should stop using modification force.
          microscope->ImageMode();
	  old_commit_pressed = tcl_commit_pressed;
	}

	/* Request a reading from the current location */

	microscope->TakeFeelStep(clipPos[0], clipPos[1]);
	    
      } else {

        // Commit button is pressed - we are modifying the surface

	if (!old_commit_pressed) { // We weren't commited last time
		// this means we should start using modification force.
	  microscope->ModifyMode();
	  old_commit_pressed = tcl_commit_pressed;
	}

	/* Request a reading from the current location, 
	 * using the current modification style */

        if ( microscope->state.modify.control == DIRECTZ) {
          if (nmOK) {
            microscope->TakeDirectZStep(clipPosNM[0], clipPosNM[1], 
                                               clipPosNM[2]);
          }
	} else {
          microscope->TakeModStep(clipPos[0], clipPos[1]);
	}
      }
	    
      if (( microscope->state.modify.control == DIRECTZ) &&
          (tcl_commit_pressed)) {

          // Stop using the plane to apply force
	    
          monitor.stopSurface();

          // Start using the forcefield to apply a constant force.
          // Apply force to the user based on current measured force 
          // XXX needs new test with new nmm_relaxComp object

          specify_directZ_force(whichUser);
          monitor.startForceField();
	   
      } else {

        setupHaptics(USER_PLANEL_MODE);

	    /* Apply force to the user based on current sample points */
	    // XXX needs new test with new nmm_relaxComp object
        if (!microscope->d_relax_comp.is_ignoring_points() ) {
          touch_surface(whichUser, clipPos);
          monitor.startSurface();
        }
      }

      break;
	
    case RELEASE_EVENT:	/* Go back to scanning upon release */

	    // Stop applying force.
      monitor.stopSurface();
      monitor.stopForceField();

      /* turn off the commit button if user releases the trigger. */
      tcl_commit_pressed = 0;
      old_commit_pressed = 0;
      if ((microscope->state.modify.tool == CONSTR_FREEHAND) ||
	    (microscope->state.modify.tool == CONSTR_FREEHAND_XYZ)) {

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
          // This gets Done when we enter Image mode.
          //graphics->emptyPolyline();
            
          p.start();

          while(p.notDone()) {

      	    //printf("cancel - remove func: %d\n", (p.curr())->iconID());
      	    // get rid of the icons marking the line endpoints
      	    p.del();  //this moves the pointer forward to the next point.
          }
        }
      }

      if (microscopeRedundancyController) {
        microscopeRedundancyController->set(0, vrpn_MsecsTimeval(0.0));
      }

      /* Start image mode and resume previous scan pattern */
      microscope->ResumeScan();

      break;

    default:
      break;
  }

  return(0);
}


/**
 *
   Select mode
   doSelect - Perform rubber-banding upon press, move, release
 *
 */
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


/**
 *
   doWorldGrab - handle world grab operation
 *
 */
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
    // Move the aiming line to the users hand location 
    v_get_world_from_hand(whichUser, &worldFromHand);
    //nmui_Util::moveAimLine(worldFromHand.xlate);
    decoration->aimLine.moveTo(worldFromHand.xlate[0],
                            worldFromHand.xlate[1], plane);

  v_xform_type temp;

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
	v_x_compose(&temp, &oldWorldFromHand, &handFromRoom);

        // NANOX
collabVerbose(5, "doWorldGrab:  updateWorldFromRoom().\n");

        updateWorldFromRoom(&temp);

	break;

    case RELEASE_EVENT:
    default:
	/* do nothing	*/
	break;
    }

return 0;
}	/* doWorldGrab */

/**
 *
   doMeasureGridGrab - Control height of measuring grid

   * It is unclear what this function is supposed to do, even if it
   * was implemented...
   * It currently consists of an empty switch statement.
 */
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

collabVerbose(5, "initializeInteraction:  updateWorldFromRoom().\n");

  updateWorldFromRoom();

  haptic_manager.d_canned = new nmui_HSCanned;
  haptic_manager.d_measurePlane = new nmui_HSMeasurePlane (decoration);
  haptic_manager.d_livePlane = new nmui_HSLivePlane;
  haptic_manager.d_feelAhead = new nmui_HSFeelAhead;
  haptic_manager.d_directZ = new nmui_HSDirectZ (dataset, microscope);

  haptic_manager.d_gridFeatures = new nmui_GridFeatures
                                       (haptic_manager.d_canned);
  haptic_manager.d_pointFeatures = new nmui_PointFeatures;

}

void linkMicroscopeToInterface (nmm_Microscope_Remote * microscope) {

  if (microscope && haptic_manager.d_feelAhead) {
    microscope->registerFeeltoHandler
        (nmui_HSFeelAhead::newPointListReceivedCallback,
         haptic_manager.d_feelAhead);
  }

}

/**
  These two functions work together to make sure that all changes
 of the world-room transform work properly with collaboration.
 Everything that used to write v_world.users.xforms[0] instead
 calls updateWorldFromRoom(), which triggers Tcl_Netvar updates.
 If we're using a centralized serializer and we're not it, we
 then send that message out over the network;  on some future
 frame it'll come back to us & update then through the same
 path as non-centralized or non-serialized:
 handle_worldFromRoom_change(), which actually writes into
 v_world.users.xforms[0]
*/

void updateWorldFromRoom (v_xform_type * t) {

  graphicsTimer.activate(graphicsTimer.getListHead());

  if (!t) {
    t = &v_world.users.xforms[0];
  }

  tcl_wfr_xlate_X = t->xlate[0];
  tcl_wfr_xlate_Y = t->xlate[1];
  tcl_wfr_xlate_Z = t->xlate[2];
  tcl_wfr_rot_0 = t->rotate[0];
  tcl_wfr_rot_1 = t->rotate[1];
  tcl_wfr_rot_2 = t->rotate[2];
  tcl_wfr_rot_3 = t->rotate[3];

  //lock_xform = 0;
  // Only trigger vlib things once, on the last assignment.

  tcl_wfr_scale = t->scale;
}


// NANOX
void handle_worldFromRoom_change (vrpn_float64, void *) {

  // We'd like to only send this message once per frame,
  // but a naive implementation (i.e. this one) will send it
  // 8 times per frame - every time ONE of the TCL variables changes.
  // We can't reenable the flag that only generates this message
  // once, since that fails if we don't allow idempotent changes
  // to the tclvar it's limited to;  another option would be to
  // set a flag here and check it once per frame somewhere in mainloop
  // (right before calling nmg_Graphics::mainloop(), one supposes).
  // The only-one-once flag would also not do anything for the collaborative
  // case, since there will be eight calls to updateWorldFromRoom() generated
  // by the eight separate messages.  Another argument for coarse-grained
  // distributed shared objects instead of fine-grained.  The counterargument
  // is to start off fine-grained and flexible, then as the system "sets"
  // convert to coarse-grained where necessary for performance;
  // vrpn_SharedObject doesn't have any way of doing composites or any
  // decent migration path to coarse-grained YET.

  v_xform_type xform;

  xform.xlate[0] = tcl_wfr_xlate_X;
  xform.xlate[1] = tcl_wfr_xlate_Y;
  xform.xlate[2] = tcl_wfr_xlate_Z;
  xform.rotate[0] = tcl_wfr_rot_0;
  xform.rotate[1] = tcl_wfr_rot_1;
  xform.rotate[2] = tcl_wfr_rot_2;
  xform.rotate[3] = tcl_wfr_rot_3;
  xform.scale = tcl_wfr_scale;

  graphics->setViewTransform(xform);
}

int clear_polyline( void * userdata ) {
  nmg_Graphics_Implementation *g = (nmg_Graphics_Implementation *)userdata;

  // delete stored points (so we don't use them again!)
  // get rid of the rubber-band line
  g->emptyPolyline();
  return 0;
}



