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
    
   Developed at the University of North Carolina at Chapel Hill, supported
   by the following grants/contracts:
   
     DARPA #DAEA18-90-C-0044
     ONR #N00014-86-K-0680
     NIH #5-R24-RR-02170
   
 *
 */
#include <stdio.h>
#ifdef __CYGWIN__
#include <unistd.h> // for sleep()
#endif

#include <quat.h>

#include <vrpn_RedundantTransmission.h>
#include <vrpn_ForceDevice.h>

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
#include <nmm_MicroscopeRemote.h>
#include <nmm_Types.h>  // for point_result, enums
#include <nmm_Sample.h>

#include <nmg_GraphicsImpl.h>
#include <nmg_Globals.h>

#include <nmui_Util.h>
#include <nmui_Haptics.h>
#include <nmui_HapticSurface.h>
#include <nmui_SurfaceFeatures.h>
#include <nmui_CrossSection.h>

#include <optimize_now.h>
#include <directstep.h>
#include "normal.h"
#include "relax.h"
#include "butt_mode.h"

#include "microscopeHandlers.h"
#include "globals.h"
#include "microscape.h"  // for lots of #defines, graphicsTimer
#include "interaction.h"
#include "minit.h"  // for reset_phantom()

#include "UTree.h"	// for Ubergraphics
#include "Urender.h"

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

#ifndef max
#define max(a,b) ((a)<(b)?(b):(a))
#endif
#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
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

static q_vec_type xyz_lock_pos;  // used for constrained freehand xyz

/***************************
 * Mode of user operation
 ***************************/

static void handle_user_mode_change(vrpn_int32, void *);
Tclvar_int     user_0_mode("user_0_mode", USER_GRAB_MODE, handle_user_mode_change);
static int     mode_change = 0;                ///< Flag set when mode changes

static int	last_mode = user_0_mode; /**< Previous mode of operation */
static int	last_style = 0;		/**< Previous style of operation */

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

// Enum for valid values is in nmg_Globals.h
/// Select mode state
static RegMode prep_select_drag_mode = REG_NULL;
static RegMode select_drag_mode = REG_NULL;

/// Region mode state
static RegMode prep_region_drag_mode = REG_NULL;
static RegMode region_drag_mode = REG_DEL;
static float region_center_x = 0;
static float region_center_y = 0;
static float region_width = 0;
static float region_height = 0;
static float region_angle = 0;
static float region_base_tracker_angle = 0;

/// Cross Section mode state
static RegMode prep_xs_drag_mode = REG_NULL;
static RegMode xs_drag_mode = REG_NULL;
static float xs_base_tracker_angle = 0;
static int xs_which_active = -1;
///State for an individual cross section
typedef struct xs_state_struct {
    float center_x;
    float center_y;
    float widthL;
    float widthR;
    float angle;
} xsState;
/// 2 cross sections - Init with non-zero width to avoid divide-by-zero
static xsState xs_state[2] = { { 0,0,1,1,0 }, { 0,0,1,1,0 } };

/// Controls tcl interaction with cross sections
nmui_CrossSection xs_ui;

/** parameter locking the tip in sharp tip mode */
static void handle_xyLock (vrpn_int32, void *);
TclNet_int xy_lock ("xy_lock_pressed", 0, handle_xyLock);

static void handle_zLock( vrpn_int32, void* );
TclNet_int z_lock( "z_lock_pressed", 0, handle_zLock );

/// Trigger button from the virtual button box in Tcl
static int  tcl_trigger_just_forced_on = 0;
static int  tcl_trigger_just_forced_off = 0;
static void handle_trigger_change( vrpn_int32 val, void *userdata );

Tclvar_int  tcl_trigger_pressed("trigger_pressed",0, handle_trigger_change);

/**
 * callback function for PHANToM reset button in tcl interface.
 */
static void handle_phantom_reset( vrpn_int32 val, void *userdata);

static void handle_handTracker_update_rate (vrpn_float64, void *);



// TCH network adaptations Nov 2000
static void handle_useRedundant_change (vrpn_int32, void *);
static void handle_numRedundant_change (vrpn_int32, void *);
static void handle_redundantInterval_change (vrpn_float64, void *);
static void handle_useMonitor_change (vrpn_int32, void *);
static void handle_monitorThreshold_change (vrpn_int32, void *);
static void handle_monitorDecay_change (vrpn_float64, void *);

Tclvar_int feel_useRedundant ("feel_use_redundant", 0,
                              handle_useRedundant_change, NULL);
Tclvar_int feel_numRedundant ("feel_num_redundant", 0,
                              handle_numRedundant_change, NULL);
Tclvar_float feel_redundantInterval ("feel_redundant_interval", 0.015,
                                     handle_redundantInterval_change, NULL);

Tclvar_int feel_useMonitor ("feel_use_monitor", 0,
                            handle_useMonitor_change, NULL);
Tclvar_int feel_monitorThreshold ("feel_monitor_threshold", 150,
                                  handle_monitorThreshold_change, NULL);
Tclvar_float feel_monitorDecay ("feel_monitor_decay", 2.0,
                                handle_monitorDecay_change, NULL);

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

TclNet_int friction_linear("friction_linear", 0, handle_friction_linear_change);
Tclvar_int adhesion_linear("adhesion_linear", 0);
TclNet_int compliance_linear("compliance_linear", 0);
TclNet_int bumpscale_linear("bumpscale_linear", 0,
			    handle_bumpscale_linear_change);
TclNet_int buzzscale_linear("buzzscale_linear", 0,
			    handle_buzzscale_linear_change);

Tclvar_float handTracker_update_rate ("handTracker_update_rate", 60.0,
                                      handle_handTracker_update_rate);

/*********
 * Functions defined in this file (added by KPJ to satisfy g++)...
 *********/
void dispatch_event( int, int, nmb_TimerList *);
int qmf_lookat(q_type ,  q_vec_type,  q_vec_type,  q_vec_type);
int doLight(int, int);
int doFly(int, int);
int doScale(int, int, double);
int specify_directZ_force(int);
double touch_surface (int, q_vec_type);
int set_aim_line_color(float);
int meas_cross( float, float, float, float, float);
int doMeasure(int, int);
int doCrossSection(int, int);

int doLine(int, int);
int doFeelFromGrid(int, int);
int doFeelLive(int, int);
int doSelect(int, int);
int doRegion(int, int);
int doServoConstRes(int, int);
int doWorldGrab(int, int);
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

/** Variables for direct step so that the position reported to the microscope
window is current
*/
Tclvar_float curr_x("cur_x",0.0);
Tclvar_float curr_y("cur_y",0.0);
Tclvar_float curr_z("cur_z",0.0);


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
  forceFieldGoing (vrpn_FALSE) 
{ }

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


/** Trace routine that handles updates to user_0_mode variable from Tcl */
void handle_user_mode_change(vrpn_int32, void *) 
{
    // There was some stuff here, but it is all handled in 
    // interaction(), below. 
}

void handle_xyLock (vrpn_int32, void *) 
{
  if (!microscope) return;
  if (!microscope->haveMutex()) {
    xy_lock = 0;
  }

  // save the position the user is currently at so we have
  // the coords of their hand in the world
  q_vec_type temp;
  if( z_lock ) // save the locked z position
    { temp[2] = xyz_lock_pos[2]; }
  nmui_Util::getHandInWorld(0, xyz_lock_pos);
  if( z_lock ) // restore the locked z position
    { xyz_lock_pos[2] = temp[2]; }

  //get Z position of when xy_lock was set for direct step
  if (microscope->haveMutex() 
      && microscope->state.modify.tool == DIRECT_STEP) {
    z_pos = xyz_lock_pos[2];
  }
}

void handle_zLock( vrpn_int32, void* )
{
  if (!microscope) return;
  if( !microscope->haveMutex() )
    { z_lock = 0; }
  cout << "z_lock pressed" << endl;

  // save the position the user is currently at so we have
  // the coords of their hand in the world
  q_vec_type temp;
  if( xy_lock ) // keep the locked xy positions
    {
      temp[0] = xyz_lock_pos[0];
      temp[1] = xyz_lock_pos[1];
    }
  nmui_Util::getHandInWorld(0, xyz_lock_pos);
  if( xy_lock ) // restore the locked xy positions
    {
      xyz_lock_pos[0] = temp[0];
      xyz_lock_pos[1] = temp[1];
    }
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

  if (vrpnHandTracker) {
    vrpnHandTracker->set_update_rate(v);
  } else {

  }

}

// TCH network adaptations Nov 2000

// static
void handle_useRedundant_change (vrpn_int32 on, void *) {
  if (!microscope) return;

fprintf(stderr, "Turning FEC %s.\n", on ? "on" : "off");

  if (microscopeRedundancyController) {
    microscopeRedundancyController->enable(on);
  } else {
    fprintf(stderr, "No microscopeRedundancyController.\n");
  }
  if (microscope->d_redundancy) {
    microscope->d_redundancy->enable(on);
  } else {
    fprintf(stderr, "No microscope->Redundancy.\n");
  }
}

// static
void handle_numRedundant_change (vrpn_int32 val, void *) {
  if (!microscope) return;

fprintf(stderr, "Sending %d redundant copies at %.5f sec intervals.\n",
val, (float) feel_redundantInterval);

  if (microscopeRedundancyController) {
    microscopeRedundancyController->set
              (val, vrpn_MsecsTimeval(1000.0 * feel_redundantInterval));
  }
  if (microscope->d_redundancy) {
    microscope->d_redundancy->setDefaults
              (val, vrpn_MsecsTimeval(1000.0 * feel_redundantInterval));
  }
}

// static
void handle_redundantInterval_change (vrpn_float64, void *) {
  if (!microscope) return;

  if (microscopeRedundancyController) {
    microscopeRedundancyController->set
              (feel_numRedundant,
               vrpn_MsecsTimeval(1000.0 * (float) feel_redundantInterval));
  }
  if (microscope->d_redundancy) {
    microscope->d_redundancy->setDefaults
              (feel_numRedundant,
               vrpn_MsecsTimeval(1000.0 * (float) feel_redundantInterval));
  }
}

// static
void handle_useMonitor_change (vrpn_int32 on, void *) {
  if (!microscope) return;

fprintf(stderr, "Turning QM %s.\n", on ? "on" : "off");

  if (microscope->d_monitor) {
    microscope->d_monitor->enable(on);
  } else {
    fprintf(stderr, "No microscope->d_monitor.\n");
  }

}

// static
void handle_monitorThreshold_change (vrpn_int32 n, void *) {
  if (!microscope) return;

fprintf(stderr, "New length-2 threshold is %d.\n", n);

  if (microscope->d_monitor) {
    microscope->d_monitor->setThreshold(n, feel_monitorDecay);
  } else {
    fprintf(stderr, "No microscope->d_monitor.\n");
  }
}

// static
void handle_monitorDecay_change (vrpn_float64 n, void *) {
  if (!microscope) return;

fprintf(stderr, "New threshold decay is %.5f.\n", n);

  if (microscope->d_monitor) {
    microscope->d_monitor->setThreshold(feel_monitorThreshold, n);
  } else {
    fprintf(stderr, "No microscope->d_monitor.\n");
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
  if (!microscope) return;
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
  microscope->DrawLine(x, y, currPt->x(), currPt->y());
}

static void drawLine (void) {
  if (!microscope) return;

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

    // start modification force!
    // We're already in modifcation mode using slow_line_3d
    if (microscope->state.modify.tool != SLOW_LINE_3D) {
      microscope->ModifyMode();
    }
          
    // Wait for the mode to finish (XXX should wait for response)
    // these calls to sleep should maybe be replaced with a 
    // barrier synch as used in init_slow_line
    sleep(1);

    // Slow line tool doesn't do the modification now, it
    // waits for the user to press Play or Step controls.
    if ((microscope->state.modify.tool == SLOW_LINE)||
	(microscope->state.modify.tool == SLOW_LINE_3D)) {
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
    microscope->DrawLine(x, y, currPt->x(), currPt->y());


    //start with the second point, so previous point is valid.
    for (p.next(); p.notDone(); p.next()) {
      drawLineStep(p.peekPrev(), p.curr());
    }
    // Delete these points so they are not used again. 
    p.start();
    while(p.notDone()) {
      p.del();
    }

    // resume normal scanning operation of AFM
    printf("drawLine: done modifying, resuming scan.\n");

    //microscope->EnableUpdatableQueue(VRPN_TRUE);

    if (microscope->state.autoscan) {
	    microscope->ResumeScan();
    }
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
  if (!microscope) return;

    // This handles double callbacks, when we set tcl_commit_pressed to
    // zero below.
    if (tcl_commit_pressed != 1) return;

    BCPlane * plane = dataset->inputGrid->getPlaneByName
      (dataset->heightPlaneName->string());
	  
    // only allow commit to be activated in selected modes.
    switch (user_0_mode) {

    //if we are feeling before
    // a modify, (can be either freehand or line tool)
    case USER_PLANEL_MODE:
	// If we press commit while in Touch Live mode and using the
	// line tool, we tell the AFM to connect the dots with
	// modificaton force. The Slow Line tool goes to the first
	// point and waits.

        // will contain world coords of max value in plane
        // for OPTIMIZE_NOW mode

        double coord_x, coord_y;
	if ((microscope->state.modify.tool == LINE)||
	    (microscope->state.modify.tool == SLOW_LINE)||
	    (microscope->state.modify.tool == SLOW_LINE_3D)) {

          drawLine();
	} else if (microscope->state.modify.tool == OPTIMIZE_NOW) {
	  if (microscope->state.modify.optimize_now_param == 
	      OPTIMIZE_NOW_LINE) {
	    // optimize based on a line specified by the user
	    // These are the points we have specified so far in the polyline
	    Position_list & pos_list = microscope->state.modify.stored_points;
	    if(pos_list.peekPrev() != NULL){
	    pos_list.goToHead();
	    computeOptimizeMinMax(
		   microscope->state.modify.optimize_now_param,
		   pos_list.currX(),
		   pos_list.currY(),
		   (pos_list.peekNext())->x(),
		   (pos_list.peekNext())->y(),
		   &coord_x, &coord_y);
	    pos_list.goToTail();

	    pos_list.start();
	    while(pos_list.notDone()) {
		// get rid of the icons marking the line endpoints
 		pos_list.del();  
		//this moves the pointer forward to the next point.
 	    }

	    graphics->emptyPolyline();
	    }else {
	      fprintf (stderr, "ERROR: NEED 2 POINTS TO OPTIMIZE\n");
	      tcl_commit_pressed = 0;
	      old_commit_pressed = 0;
	    }
	  }
	  Point_value *value =
	    microscope->state.data.inputPoint->getValueByPlaneName
	    (dataset->heightPlaneName->string());
	  if (value == NULL) {
	    fprintf(stderr, "Error in handle_commit_change(): "
		    "could not get value!\n");
	    return;
	  }
	  microscope->ScanTo( coord_x, coord_y );
	  double grid_x,grid_y;
	  dataset->inputGrid->worldToGrid(coord_x,coord_y,grid_x, grid_y);
	  graphics->positionSphere( coord_x, coord_y, 
			   plane->valueInWorld(grid_x, grid_y) );

	}
	//if we aren't using line tool, don't change commit button's value,
	// because it's handled below in doFeelLive()
	// (allows user to switch between touch and modify, without
	// scanning in between.)

	break;
    case USER_SERVO_MODE: // user is in Select mode
	// set the scan region based on the last one specified by the user. 
        if ( (microscope->state.modify.tool == OPTIMIZE_NOW) && 
	     (microscope->state.modify.optimize_now_param == 
	      OPTIMIZE_NOW_AREA) ) {
	    BCPlane * plane = dataset->inputGrid->getPlaneByName
	      (dataset->heightPlaneName->string());

	    // optimize within an area specified by the user

	    printf("Optimizing based on selected area\n");
	    Position_list & pos_list = microscope->state.modify.stored_points;
	    pos_list.goToHead();
	    computeOptimizeMinMax(
		 microscope->state.modify.optimize_now_param,
		 microscope->state.select_center_x
                    - microscope->state.select_region_rad,
		 microscope->state.select_center_y
                    - microscope->state.select_region_rad,
		 microscope->state.select_center_x
                    + microscope->state.select_region_rad,
		 microscope->state.select_center_y
                    + microscope->state.select_region_rad, &coord_x, &coord_y);
	    Point_value *value =
	      microscope->state.data.inputPoint->getValueByPlaneName
	      (dataset->heightPlaneName->string());
	    if (value == NULL) {
	      fprintf(stderr, "Error in handle_commit_change(): "
		      "could not get value!\n");
	      return;
	  }
	  microscope->ScanTo( coord_x, coord_y );

	double height;
	plane -> valueAt(&height,coord_x,coord_y);

	  graphics->positionSphere( coord_x, coord_y,height );
	  printf("Moved tip location to %.2f, %.2f\n", coord_x, coord_y);
	}
	// if not in optimize_now mode, use the select tool as it was 
	// originally intended to be used.
	else if ( microscope->state.select_region_rad > 0.01 ) {  
	    // This comparison sets the smallest possible scan region 
	    // to be 0.01 nm - 0.1 Angstrom. Ought to be safe. 
	    // only do something if region has been specified.

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
  if (!microscope) return;
    BCPlane * plane;
    // This handles double callbacks, when we set tcl_commit_canceled to
    // zero below.
    if (tcl_commit_canceled != 1) return;

    // only allow cancel to be activated in selected modes.
    switch (user_0_mode) {

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
	    graphics->emptyPolyline();

	    p.start();
	    while(p.notDone()) {
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

	    // For SLOW_LINE_3D tool: clear the markers along the 
	    // rubber-band lines
	    if (microscope->state.modify.tool == SLOW_LINE_3D) {
	      decoration->num_slow_line_3d_markers = 0;
	    }

	}
	// I think we should always resume scan - if the user hits "cancel"
	// that should mean "start over", so we always begin scanning again.
	  if (microscope->state.autoscan) {
		microscope->ResumeScan();
          }
	}
	break;
    case USER_SERVO_MODE:
	//Set the region back to the old scan. 
        plane = dataset->inputGrid->getPlaneByName
            (dataset->heightPlaneName->string());
        if (plane) {
            microscope->state.select_region_rad = 0.5 * 
                (plane->maxX() - plane->minX());
            microscope->state.select_center_x = plane->minX() + 
                microscope->state.select_region_rad;
            microscope->state.select_center_y= plane->minY() + 
                microscope->state.select_region_rad;
        }
	// Icon for the region will be displayed in doSelect. 
	if (microscope->state.modify.tool == OPTIMIZE_NOW) {
	  microscope->state.modify.tool = FREEHAND;
	}
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
    if (tcl_phantom_reset_pressed != 1) return;

    if (reset_phantom())
	fprintf(stderr, "Error: could not reinitialize phantom\n");

    tcl_phantom_reset_pressed = 0;
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
      if (microscope && (microscope->state.image.tool == FEELAHEAD)) {
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
void dispatch_event(int mode, int event, nmb_TimerList * /*timer*/)
{
    int ret = 0;
    int user = 0;
    // If no hand tracker this function shouldn't do anything
    if ( vrpnHandTracker == NULL )
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
	case USER_CROSS_SECTION_MODE:
		ret = doCrossSection(user,event);
		break;
	case USER_LINE_MODE:  // no longer directly called?
		ret = doLine(user,event);
		break;
	case USER_PLANE_MODE:
		ret = doFeelFromGrid(user,event);
		break;
	case USER_PLANEL_MODE:
            if (!microscope) return;
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
	   } else if (microscope->state.modify.tool == OPTIMIZE_NOW) {
	     // user wants to specify region via a line
	     ret = doLine(user,event);
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
	case USER_CENTER_TEXTURE_MODE:  // mouse control only!
		break;
        case USER_REGION_MODE:  // change size of a selected region
            ret = doRegion(user,event);
            break;
	default:
	    fprintf(stderr,"dispatch_event(): "
		    "Event %d not implemented\n", mode);
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
  // XXX doesn't fit vrpn model, should be rewritten to match Magellan in 
  // minit.c
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
  
  VERBOSE(4, "  Entering interaction().");
  /*
   * handle button events for each user-  do each handler independent of
   *  of the other so that grabbing and flying can be done together
   */
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
	dispatch_event(user_0_mode, eventList[0], timer);
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
	Arm_knobs[i] = bdbox_dials[i] + 0.5;
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
  if (microscope) {
    if( microscope->state.modify.mode == CONTACT ) {
      if( force != old_mod_con_force ) {
	microscope->state.modify.setpoint = 
	  microscope->state.modify.setpoint_min +
	  force * (microscope->state.modify.setpoint_max 
		   - microscope->state.modify.setpoint_min);
	old_mod_con_force = force;
      }
    } else if ( microscope->state.modify.mode == TAPPING ) {
      if( force != old_mod_tap_force ) {
	microscope->state.modify.amplitude = 
	  microscope->state.modify.amplitude_min +
	  force * (microscope->state.modify.amplitude_max 
		   - microscope->state.modify.amplitude_min);
	old_mod_tap_force = force;
      }
    }
    
    force = Arm_knobs[IMG_FORCE_KNOB];
    if( (microscope->state.image.mode == CONTACT) 
	|| (microscope->state.image.mode == GUARDED_SCAN) ) {
      if( force != old_img_con_force ) {
	microscope->state.image.setpoint =
	  microscope->state.image.setpoint_min +
	  force * (microscope->state.image.setpoint_max -
		   microscope->state.image.setpoint_min);
	old_img_con_force = force;
      }
    } else if ( microscope->state.image.mode == TAPPING ) {
      if( force != old_img_tap_force ) {
	microscope->state.image.amplitude =
	  microscope->state.image.amplitude_min +
	  force * (microscope->state.image.amplitude_max -
		   microscope->state.image.amplitude_min);
		old_img_tap_force = force;
      }
    }
  }
    /* Check for immediate action button presses
    **
    ** Center command is a special case
    **/
    // XXX doesn't fit vrpn model, should be rewritten to match Magellan 
    //in minit.c
    if( PRESS_EVENT == eventList[CENTER_BT] ) {
      if (timer) { timer->activate(timer->getListHead()); }
      center();
    }

    /* code dealing with locking the tip in sharp tip mode */
    /* locks the tip in one place so you can take repeated measurements */
  if (microscope) {
    if ((PRESS_EVENT == eventList[XY_LOCK_BT]) && microscope->haveMutex()) 
      { xy_lock = 1; }
    if (RELEASE_EVENT == eventList[XY_LOCK_BT]) 
      { xy_lock = 0; }

    if( (PRESS_EVENT == eventList[Z_LOCK_BT]) && microscope->haveMutex() )
      { z_lock = 1; }
    if( RELEASE_EVENT == eventList[Z_LOCK_BT] )
      { z_lock = 0; }
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
      tcl_commit_canceled = !tcl_commit_canceled; 
      // we must call "handle_commit_cancel" explicitly, 
      // because tclvars are set to ignore changes from C.
      handle_commit_cancel(tcl_commit_canceled, NULL);
    }
    mode_change = 0;
    // Set the user mode, and indicate a mode change, if it's different
    if (last_mode != user_0_mode) {
        mode_change = 1;
    }
    // see if the user changed modes using a button
    int temp = user_0_mode;
    if( butt_mode(eventList, &temp) ) {
        mode_change = 1;
        user_0_mode = temp;
    }
    // also check to see if the modify style has changed -
    // can affect hand icon as well.
    if (microscope && (user_0_mode == USER_PLANEL_MODE)) {
      mode_change |= microscope->state.modify.style_changed;
    }

    /* If there has been a mode change, clear old stuff and set new mode */
    if (mode_change) {
	if (timer) { timer->activate(timer->getListHead()); }
      // If the user was going to go into a disabled mode, change them
      // into GRAB mode instead.  Some modes are currently not
      // implemented or not used.
      if ((user_0_mode == USER_COMB_MODE)||
	  (user_0_mode == USER_SWEEP_MODE)||
	  (user_0_mode == USER_BLUNT_TIP_MODE)||
	  (user_0_mode == USER_PULSE_MODE))
	{
	  fprintf(stderr,"Warning -- mode disabled (entering "
		  "grab mode instead)\n");
	  user_0_mode = USER_GRAB_MODE;
	}

      /* If the user is in HOLD_EVENT at the time of the
       * change, we need to send an RELEASE_EVENT to the
       * previous mode and an PRESS_EVENT to the new mode
       * so that all setup/cleanup code is executed. */
      if (eventList[TRIGGER_BT] == HOLD_EVENT) {
	dispatch_event( last_mode, RELEASE_EVENT, timer);
	dispatch_event( user_0_mode, PRESS_EVENT, timer);
      }		    

      VERBOSE(6, "    Calling graphics->setUserMode().");

      // Change icons to ones for this new mode.
    if (microscope) {
      graphics->setUserMode(last_mode, last_style,
			    user_0_mode, microscope->state.modify.style,
			    microscope->state.modify.optimize_now_param);

      /* Last mode next time around is the current mode this time around */
      last_style = microscope->state.modify.style;
    } else {
      graphics->setUserMode(last_mode, last_style,
			    user_0_mode, 0, 0);

    }
      last_mode = user_0_mode;

      /* Clear the mode change flag */
      mode_change = 0;
      if (microscope) microscope->state.modify.style_changed = 0;
    }
    else if ( first_mode == 1 ) {
      first_mode = 0;
      if (microscope) last_style = microscope->state.modify.style;
      last_mode = user_0_mode;
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
    dispatch_event(user_0_mode, eventList[0], timer);
    lastTriggerEvent = eventList[0];

    // TCH - HACK but it works
    decoration->user_mode = user_0_mode;
    
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
    static v_xform_type	    startWorldFromHand;
    static v_xform_type	    startWorldFromRoom;
    v_xform_type    	    trackerFromHand;
    static v_xform_type	    startTrackerFromHand;
    v_xform_type    	    handFromWorld;
    v_xform_type    	    worldFromHand = V_ID_XFORM;

    BCPlane* plane 
      = dataset->inputGrid->getPlaneByName(dataset->heightPlaneName->string());
    if (plane == NULL) {
      fprintf(stderr, "Error in doScale: could not get plane!\n");
      return -1;
    }     
    v_get_world_from_hand(whichUser, &worldFromHand);
    
    switch ( userEvent )
      {
      case HOLD_EVENT:
        v_xform_type temp;

        // Very similar operation to v_scale_about_hand, 
        // but we scale around the intersection of the aim line with
        // the height plane. 

        // First bit done in press event. 

        // Find a scale factor base on in/down vs. out/up movement, since the
        // last frame.
        // This is based on absolute movement of the Phantom, so 
        // you can scale a reasonable amount if the surface is big. 
        v_x_copy(&trackerFromHand,
		 &v_users[whichUser].xforms[V_TRACKER_FROM_HAND_SENSOR]);
        scale_factor = 1.0 + 10.0 
	  *( -trackerFromHand.xlate[2]
	     + startTrackerFromHand.xlate[2]);
        if (scale_factor < 0.01) scale_factor = 0.01;

        // now scale by scale amount, use square to emphasize
        // scale-down. 
        scaleXform.scale = scale_factor*scale_factor;

        // scaled_w_f_ha = w_f_h * scale   
        v_x_compose(&scaledWorldFromHand, &startWorldFromHand, &scaleXform);
        
        //scaling's done, so get back into object space 
        v_x_invert(&handFromWorld, &startWorldFromHand);
    
        // ha_f_o = ha_f_w * w_f_o  
        v_x_compose(&handFromObject, &handFromWorld, &startWorldFromRoom);
    
        // scaled_w_f_o = scaled_w_f_h * h_f_o	
        v_x_compose(&temp, &scaledWorldFromHand, &handFromObject);

	collabVerbose(5, "doScale:  updateWorldFromRoom().\n");

        updateWorldFromRoom(&temp);

        //v_x_copy(&lastWorldFromHand, &worldFromHand);
	break;
	
      case PRESS_EVENT:
	// grab the initial position to scale around 
	v_get_world_from_hand(whichUser, &startWorldFromHand);
	v_x_copy(&startTrackerFromHand,
		 &v_users[whichUser].xforms[V_TRACKER_FROM_HAND_SENSOR]);
	v_x_copy(&startWorldFromRoom, &v_world.users.xforms[whichUser]);
	// get spot to scale from in world space. 
	decoration->aimLine.getIntercept(startWorldFromHand.xlate, plane);

        // If the plane height has been exaggerated, we need to 
        // adjust the translation point by that scale to get world coords. 
        startWorldFromHand.xlate[2] *= plane->scale();
     
	// move aim line only if we aren't actively scaling. 
	decoration->aimLine.moveTo(worldFromHand.xlate[0],
				   worldFromHand.xlate[1], plane);
	break;
      case RELEASE_EVENT:
      default:
	// move aim line only if we aren't actively scaling. 
	decoration->aimLine.moveTo(worldFromHand.xlate[0],
				   worldFromHand.xlate[1], plane);
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
  haptic_manager.surface()->update(dataset, microscope);
  haptic_manager.surface()->sendForceUpdate(forceDevice);

  // Set up buzzing, bumps, friction, compliance, ...
  haptic_manager.surfaceFeatures().update(dataset, microscope);

  return haptic_manager.surface()->distanceFromSurface();
}



/** When controlling the position of the tip in 3D, this calculates
 * the force the user should feel based on internal sensor measurements.
 */
int specify_directZ_force(int whichUser) 
{
  if (!microscope) return 0;
     q_vec_type		        point;
     q_vec_type		up = { 0.0, 0.0, 1.0 };
     v_xform_type               WorldFromTracker, TrackerFromWorld;

     // If directz hasn't been initialized, specify a force field that has
     // zero force, so that when the forcefield is turned on the user won't
     // feel anything.
     if (microscope->state.modify.freespace_normal_force == BOGUS_FORCE) {
	forceDevice->setFF_Force(0.0, 0.0, 0.0);
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
	 fprintf(stderr, "Error in specify_directZ_force: "
		 "could not get plane!\n");
	 return -1;
     }
     if (value == NULL) {
	 fprintf(stderr, "Error in specify_directZ_force: "
		 "could not get value!\n");
	 return -1;
     }

     point[Z] = value->value()*plane->scale();

    double current_force;

    // Get the current value of the internal sensor, which tells us 
    // the force the tip is experiencing.
     Point_value *forcevalue =
	microscope->state.data.inputPoint->getValueByName("Internal Sensor");

     if (forcevalue == NULL) {
	 fprintf(stderr, "Error in specify_directZ_force: "
		 "could not get force value!\n");
	 return -1;
     }

     current_force = forcevalue->value();

     // Calculate the difference from the free-space value of 
     // internal sensor recorded when we entered direct-z control
     double diff_force 
       = current_force - microscope->state.modify.freespace_normal_force;

     // got user's hand position and force direction (up) in microscope space, 
     // XForm into hand-tracker space
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

    // Force field does not change as we move around.
    forceDevice->setFF_Jacobian(0,0,0,  0,0,0,  0,0,0);
    forceDevice->setFF_Radius(0.02); // 2cm radius of validity
    // this actually turns on the force field- do that later. 

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
  int		i0 = (int)( (x0-dataset->inputGrid->minX())
			    *dataset->inputGrid->derangeX());
  int		j0 = (int)( (y0-dataset->inputGrid->minY())
			    *dataset->inputGrid->derangeY());
  int		i1 = (int)( (x1-dataset->inputGrid->minX())
			    *dataset->inputGrid->derangeX());
  int		j1 = (int)( (y1-dataset->inputGrid->minY())
			    *dataset->inputGrid->derangeY());
  float		ranger_x = 1.0/dataset->inputGrid->derangeX();
  float		ranger_y = 1.0/dataset->inputGrid->derangeY();
  float		x;
  float		y;
  float		dx, dy;
  int		n = MAX( IABS(n,i1-i0), IABS(n,j1-j0) );

  BCPlane* plane 
    = dataset->inputGrid->getPlaneByName(dataset->heightPlaneName->string());
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

    Bot[X] = ( (int)((x-plane->minX())*dataset->inputGrid->derangeX()) )
      * ranger_x + plane->minX();
    Bot[Y] = ( (int)((y-plane->minY())*dataset->inputGrid->derangeY()) )
      * ranger_y + plane->minY();
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

static float xform_width(float x, float y, float angle) {
    return fabs(cos(angle)*(x) + sin(angle)*(y));
}
static float xform_height(float x, float y, float angle) {
    return fabs(cos(angle)*(y) - sin(angle)*(x));
}

/** Clamp the widths to the edges of the plane.
 Center is clamped to inside the plane, too. 
*/
static void xs_find_full_width(
    float * widthL,
    float * widthR,
    float * center_x,
    float * center_y,
    float angle,
    BCPlane * plane) 
{
    double endxR, endyR, endxL, endyL;
    // Make sure center is inside plane. 
    if (*center_x < plane->minX()) *center_x = plane->minX();
    if (*center_x > plane->maxX()) *center_x = plane->maxX();
    if (*center_y < plane->minY()) *center_y = plane->minY();
    if (*center_y > plane->maxY()) *center_y = plane->maxY();

    // Shift angle into -45 to 315 deg. range.
    while (angle > 1.75*M_PI) angle -= 2*M_PI;
    while (angle < -0.25*M_PI) angle += 2*M_PI;

    // Special case 0, 90, 180, 270 - to make sure whole line
    // is displayed when xs is on an edge. 
    if (fabs(angle - 0) < 0.001) {
        *widthR = plane->maxX()- *center_x;
        *widthL = *center_x - plane->minX(); 
    } else if (fabs(angle - M_PI/2.0) < 0.001) {
        *widthR = plane->maxY() - *center_y;
        *widthL = *center_y - plane->minY();
    } else if (fabs(angle - M_PI) < 0.001) {
        *widthR = *center_x - plane->minX();
        *widthL = plane->maxX()- *center_x;
    } else if (fabs(angle - 3*M_PI/2.0) < 0.001) {
        *widthR = *center_y - plane->minY();
        *widthL = plane->maxY() - *center_y;
    } else {
        // Other angles need more general math. 
        if (angle > 0.25*M_PI && angle < 0.75 * M_PI) {
            // Clamp to y first, then x. 
            endyR = plane->maxY();
            endxR = *center_x + (endyR - *center_y)/tan(angle);
            endyL = plane->minY();
            endxL = *center_x + (endyL - *center_y)/tan(angle);
        } else if (angle > 1.25*M_PI) {
            // Clamp to y first, then x. 
            endyR = plane->minY();
            endxR = *center_x + (endyR - *center_y)/tan(angle);
            endyL = plane->maxY();
            endxL = *center_x + (endyL - *center_y)/tan(angle);
        } else if ((angle > -0.25*M_PI) && (angle < 0.25*M_PI)) {
            // Clamp to x first, then y. 
            endxR = plane->maxX();
            endyR = *center_y + (endxR - *center_x)*tan(angle);
            endxL = plane->minX();
            endyL = *center_y + (endxL - *center_x)*tan(angle);
        } else {
            // Clamp to x first, then y. 
            endxR = plane->minX();
            endyR = *center_y + (endxR - *center_x)*tan(angle);
            endxL = plane->maxX();
            endyL = *center_y + (endxL - *center_x)*tan(angle);
        } 
        // More bounds checking, right half
        if (endxR > plane->maxX()) {
            endxR = plane->maxX();
            endyR = *center_y + (endxR - *center_x)*tan(angle);
        } else if (endxR < plane->minX()) {
            endxR = plane->minX();
            endyR = *center_y + (endxR - *center_x)*tan(angle);
        }            
        if (endyR > plane->maxY()) {
            endyR = plane->maxY();
            endxR = *center_x + (endyR - *center_y)/tan(angle);
        } else if (endyR < plane->minY()) {
            endyR = plane->minY();
            endxR = *center_x + (endyR - *center_y)/tan(angle);
        }

        // More bounds checking, left half
        if (endxL > plane->maxX()) {
            endxL = plane->maxX();
            endyL = *center_y + (endxL - *center_x)*tan(angle);
        } else if (endxL < plane->minX()) {
            endxL = plane->minX();
            endyL = *center_y + (endxL - *center_x)*tan(angle);
        }            
        if (endyL > plane->maxY()) {
            endyL = plane->maxY();
            endxL = *center_x + (endyL - *center_y)/tan(angle);
        } else if (endyL < plane->minY()) {
            endyL = plane->minY();
            endxL = *center_x + (endyL - *center_y)/tan(angle);
        }
        *widthR = sqrt((*center_x - endxR)*(*center_x - endxR) +
                       (*center_y - endyR)*(*center_y - endyR));
        *widthL = sqrt((*center_x - endxL)*(*center_x - endxL) +
                       (*center_y - endyL)*(*center_y - endyL));
    }
}

/**
*
   doCrossSection - Move cross-section indicators across the surface.
*
*/
int doCrossSection(int whichUser, int userEvent)
{ 
    v_xform_type	worldFromHand;
    q_matrix_type	hand_mat;
    q_vec_type          angles;
    float	        handx,handy, hand_angle;

    BCPlane* plane = dataset->inputGrid->getPlaneByName
        (dataset->heightPlaneName->string());
    if (plane == NULL)
    {
        fprintf(stderr, "Error in doxs: could not get plane!\n");
        return -1;
    }  

    /* Move the tip to the hand x,y location */
    /* Set its height based on data at this point */
    v_get_world_from_hand(whichUser, &worldFromHand);

    // Move the aiming line to the user's hand location
    // We don't want to clip it to the heightplane, so pass NULL
    decoration->aimLine.moveTo(worldFromHand.xlate[0],
                               worldFromHand.xlate[1], NULL);
    
    q_to_col_matrix(hand_mat, worldFromHand.rotate);
    q_col_matrix_to_euler( angles, hand_mat );
    hand_angle = -angles[YAW];
    handx = worldFromHand.xlate[0];
    handy = worldFromHand.xlate[1];

    // Assume hand isn't near anything. 
    //xs_which_active = -1;
    prep_xs_drag_mode = REG_NULL;

    // only check if we aren't currently dragging a cross section. 
    if (xs_drag_mode == REG_NULL) {
      //Check all cross sections:
      for (int i = 0; i < 2; i++) {
        // Don't examine if the xs is hidden. User can't interact with it. 
        if (xs_ui.d_hide[i] ==1) {
            graphics->hideCrossSection(i);
            continue;
        }
        // Is hand near center ?
        if ((handx - xs_state[i].center_x)*
            (handx - xs_state[i].center_x)+
            (handy - xs_state[i].center_y)*
            (handy - xs_state[i].center_y)
            < 0.01*(xs_state[i].widthL*xs_state[i].widthR)) {
            xs_which_active = i;
            prep_xs_drag_mode = REG_PREP_TRANSLATE;
        } else {
            //calculate endpoints of the cross section
            double end_L_x = xs_state[i].center_x - 
                cos(xs_state[i].angle)*xs_state[i].widthL;
            double end_L_y = xs_state[i].center_y - 
                sin(xs_state[i].angle)*xs_state[i].widthL;
            double end_R_x = xs_state[i].center_x + 
                cos(xs_state[i].angle)*xs_state[i].widthR;
            double end_R_y = xs_state[i].center_y +
                sin(xs_state[i].angle)*xs_state[i].widthR;
            // See if hand is near an endpoint. 
            if ((handx - end_L_x)*(handx - end_L_x) +
                 (handy - end_L_y)*(handy - end_L_y)
                < 0.01*(xs_state[i].widthL*xs_state[i].widthL)) {
                xs_which_active = i;
                // near endpoint, so resize the widthL. 
                // use WIDTH state as a stand-in
                prep_xs_drag_mode = REG_PREP_SIZE_WIDTH;
            } else if ((handx - end_R_x)*(handx - end_R_x) +
                 (handy - end_R_y)*(handy - end_R_y)
                 < 0.01*(xs_state[i].widthR*xs_state[i].widthR)) {
                xs_which_active = i;
                // near endpoint, so resize the widthR. 
                // use HEIGHT state as a stand-in
                prep_xs_drag_mode = REG_PREP_SIZE_HEIGHT;
            }
        }
      }
    }
    // Carefull! Nested switch statements!
    switch ( userEvent ) {

    case PRESS_EVENT:	
        switch(prep_xs_drag_mode) {
        case REG_PREP_TRANSLATE:
            xs_state[xs_which_active].center_x = handx;
            xs_state[xs_which_active].center_y = handy;
            xs_base_tracker_angle = xs_state[xs_which_active].angle + hand_angle;
            xs_drag_mode = REG_TRANSLATE;
            break;
        case REG_NULL:
            // If we're not near a feature when we click, switch
            // to creating and sizing a new xs. 
            if (xs_ui.d_hide[0] ==1) {
                xs_ui.d_hide[0] = 0;
                xs_which_active = 0;
            } else if (xs_ui.d_hide[1] ==1) {
                xs_ui.d_hide[1] = 0;
                xs_which_active = 1;
            } else {
                // Don't do anything if both are already active
                break;
            }
            // reset xs size for dragging. 
            xs_state[xs_which_active].center_x = handx;
            xs_state[xs_which_active].center_y = handy;
            xs_base_tracker_angle = hand_angle;
            xs_state[xs_which_active].angle = 0;
            if (xs_ui.d_vary_width == 0) {
                xs_drag_mode = REG_TRANSLATE;
                prep_xs_drag_mode = REG_TRANSLATE;
                // Clamp the widths to the edges of the plane.
                // Center is clamped to inside the plane, too. 
                xs_find_full_width(
                    &xs_state[xs_which_active].widthL,
                    &xs_state[xs_which_active].widthR,
                    &xs_state[xs_which_active].center_x,
                    &xs_state[xs_which_active].center_y,
                    xs_state[xs_which_active].angle,
                    plane);
                    
            } else {
                // Drag the right end of the cross section. 
                xs_drag_mode = REG_SIZE_HEIGHT;
                prep_xs_drag_mode = REG_PREP_SIZE_HEIGHT;
                xs_state[xs_which_active].widthL = 
                xs_state[xs_which_active].widthR = 1;
            }
             break;
        case REG_PREP_SIZE_WIDTH:
            xs_drag_mode = REG_SIZE_WIDTH;
            break;
        case REG_PREP_SIZE_HEIGHT:
            xs_drag_mode = REG_SIZE_HEIGHT;
            break;
        }
        break;
       
    case RELEASE_EVENT:	
        // Do same stuff for hold or release

    case HOLD_EVENT:
        switch(xs_drag_mode) {
        case REG_TRANSLATE:
            // Move the center of the xs
            xs_state[xs_which_active].center_x = handx ;
            xs_state[xs_which_active].center_y = handy ;
            // Change the angle, too. 
            if (xs_ui.d_snap_to_45) {
                // Clamp to 45 deg intervals. 
                xs_state[xs_which_active].angle = 
                    (int((xs_base_tracker_angle - hand_angle)*4.0/M_PI))
                    *M_PI/4.0 ;
            } else {
                xs_state[xs_which_active].angle = xs_base_tracker_angle - hand_angle;
            }
            if (xs_ui.d_vary_width == 0) {
                // Make edges stay glued to edges of plane. 
                xs_find_full_width(
                    &xs_state[xs_which_active].widthL,
                    &xs_state[xs_which_active].widthR,
                    &xs_state[xs_which_active].center_x,
                    &xs_state[xs_which_active].center_y,
                    xs_state[xs_which_active].angle,
                    plane);
            }
            break;
        case REG_SIZE_WIDTH:
        case REG_SIZE_HEIGHT:
            // Resize the xs
            //xs_state[xs_which_active].angle = xs_base_tracker_angle - hand_angle;
            xs_state[xs_which_active].widthL = 
            xs_state[xs_which_active].widthR = 
                sqrt((handx - xs_state[xs_which_active].center_x)*
                     (handx - xs_state[xs_which_active].center_x)+
                     (handy - xs_state[xs_which_active].center_y)*
                     (handy - xs_state[xs_which_active].center_y));
            xs_state[xs_which_active].angle = 
                atan2(handy - xs_state[xs_which_active].center_y, 
                      handx - xs_state[xs_which_active].center_x);
            if (xs_drag_mode == REG_SIZE_WIDTH) {
                // We're at the other end, add 180 degrees. 
                xs_state[xs_which_active].angle += M_PI;
            }
            if (xs_ui.d_snap_to_45) {
                // Clamp to 45 deg intervals. 
                xs_state[xs_which_active].angle = 
                    (int((xs_state[xs_which_active].angle)*4.0/M_PI))
                    *M_PI/4.0 ;
            }
            // passed as param below, so we display highlight. 
            //prep_xs_drag_mode = REG_PREP_SIZE_WIDTH;
            break;
        default:
            break;
        }
        if (userEvent == RELEASE_EVENT) {
            // Helps display highlight correctly, below
            xs_which_active = -1;
            xs_drag_mode = REG_NULL;
        }
        break;

    default:
        break;
    }

    // Avoid some unnecessary calls, but might still call if
    // cross section was created, then cleared. 
    if (xs_which_active!= -1) {
        // Display the scan xs extent as the user plans to change it. 
        // Draw the new bounding box and center handle for the xs. 
        graphics->positionCrossSection(
            xs_which_active, !xs_ui.d_hide[xs_which_active],
            xs_state[xs_which_active].center_x, 
            xs_state[xs_which_active].center_y, 
            xs_state[xs_which_active].widthL, 
            xs_state[xs_which_active].widthR, 
            Q_RAD_TO_DEG(xs_state[xs_which_active].angle), 
            (xs_drag_mode==REG_NULL)?prep_xs_drag_mode:xs_drag_mode);
        
        xs_ui.ShowCrossSection(
            dataset->inputGrid, dataset->inputPlaneNames, 
            xs_which_active, xs_ui.d_hide[xs_which_active],
            xs_state[xs_which_active].center_x, 
            xs_state[xs_which_active].center_y, 
            xs_state[xs_which_active].widthL, 
            xs_state[xs_which_active].widthR, 
            xs_state[xs_which_active].angle);
    }
    return(0);
}

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
  if (!microscope || (dataset->inputGrid->readMode() != READ_DEVICE))
    {
      user_0_mode = USER_GRAB_MODE;
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
  valid_Direct_Z_Point 
    = nmui_Util::convertPositionToNM(plane, clipPosNM);
  
  /* Move the aiming line to the user's hand location
   * re-draw the aim line and the red sphere representing the tip. */
  decoration->aimLine.moveTo(clipPos[0], clipPos[1], plane);
  if ( (microscope->state.modify.tool != OPTIMIZE_NOW) ||
       (!tcl_commit_pressed) ) {
    nmui_Util::moveSphere(clipPos, graphics);
  }
  
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
      // Request a reading from the current location,
      // and wait till tip gets there
      
      // for CONSTR_FREEHAND_XYZ: the line is specified in 3-space, not
      // 2-space, so you have to consider the z-coord as well.  
      // Therefore, this mode should do the same thing slow_line_3d does
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
      // Feel the surface, to help determine the next point. 
      // Request a reading from the current location
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
	
	// Apply force to the user based on current sample points
	touch_surface(whichUser, clipPos);
	monitor.startSurface();
      }
      break;
      
    case RELEASE_EVENT:
      {
	//Save current hand position as one point on the polyline-
	//The list is saved in microscope->state.modify.stored_points
	// don't send a stop surface when not surfacing (?)
	monitor.stopSurface();
	monitor.stopForceField();
	
	int list_id;
	if (microscope->state.modify.style == SWEEP) {
	  graphics->addPolySweepPoints(TopL, BottomL, TopR, BottomR);
	}
	else {
	  //add an icon to represent this spot as part of polyline.
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
	    (microscope->state.modify.tool == SLOW_LINE_3D)||
	    (microscope->state.modify.tool == OPTIMIZE_NOW)) {
	  graphics->setRubberLineStart(clipPos[0], clipPos[1]);
	  graphics->setRubberSweepLineStart(TopL, TopR);
	  //save this point as part of the poly-line
	  if (microscope->state.modify.tool == SLOW_LINE_3D) {
	    // Insert points in reverse order so we start
	    // modifying from the last point we add. 
	    pos_list.insertPrev(clipPosNM[0], clipPosNM[1],
				clipPosNM[2], list_id);
	  }
	  else {
	    
	    //if in optimize now, only let 2 points be on a line
	    if((microscope->state.modify.tool == OPTIMIZE_NOW) &&
	       (pos_list.peekPrev() != NULL)){
	      //if we are here, 2 pts are in the polyline already,
	      //delete head, add point, update rubberline
	      pos_list.goToHead();
	      pos_list.del();
	      pos_list.goToTail();
	      
	      pos_list.insert(clipPos[0], clipPos[1], list_id);
	      
	      //delete rubberline
	      graphics->emptyPolyline();
	      //redraw rubberline
	      for(pos_list.start();pos_list.notDone(); pos_list.next()){
		PointType * markpts = new PointType[2];
		markpts[0][0] = markpts[1][0] = pos_list.currX();
		markpts[0][1] = markpts[1][1] = pos_list.currY();
		markpts[0][2] = plane->maxAttainableValue()*plane->scale();
		markpts[1][2] = plane->minAttainableValue()*plane->scale();
		
		list_id = graphics->addPolylinePoint(markpts);
	      }
	      //we want to be at tail of list
	      pos_list.goToTail();
	    } else{
	      pos_list.insert(clipPos[0], clipPos[1], list_id);
	    }
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

    if (!microscope) return 0;
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
	    if (aboveSurf > 0) {
              monitor.startSurface();
	    }
	    break;

	  case HOLD_EVENT:
	    /* stylus continues to be pressed */
	    /* Apply force to the user based on grid */
	    aboveSurf = touch_surface(whichUser, clipPos);
	    if (monitor.surfaceGoing || (aboveSurf > 0)) {
              monitor.startSurface();
	    }
	    
	    break;
	  
	  case RELEASE_EVENT:
	    
              // Stop applying forces. 
            monitor.stopSurface();
	    
	    break;
	    
	  default:
	    break;
	  }
	
	return(0);
}


static void setupSweepIcon (int whichUser, q_vec_type clipPos,
                            BCPlane * plane) {
  if (!microscope) return;
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
int doFeelLive (int whichUser, int userEvent)
{
  BCPlane * plane;

  // static to allow xy_lock to work properly
  static q_vec_type clipPos;
  static q_vec_type clipPosNM;

  static FDOnOffMonitor monitor;

  vrpn_bool nmOK;

  /* if we are not running live, you should not be able
     to do this, so put the user into grab mode */
  if (!microscope || (dataset->inputGrid->readMode() != READ_DEVICE)) {
    user_0_mode = USER_GRAB_MODE;
    printf("SharpTip mode available only on live data!!!\n");
    return 0;
  }

  // If we are running live, but we don't have control of the
  // microscope, we can't do this, so put the user into grab mode.

  if (!microscope->haveMutex()) {
    user_0_mode = USER_GRAB_MODE;
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
  
  ////////////////////////
  // Restrict the tip position depending on various modes
  
  // set clipPos to the current hand position in world coords.
  nmui_Util::getHandInWorld(whichUser, clipPos);

  // clip to the interior of the plane
  nmui_Util::clipPosition(plane, clipPos);

  // restrict based on xy_lock
  if( xy_lock )
    {
      if (microscope->state.modify.control == DIRECTZ ) {
	clipPos[0] = xyz_lock_pos[0];
	clipPos[1] = xyz_lock_pos[1];
      }
      else { // not directZ; in feedback
	clipPos[0] = xyz_lock_pos[0];
	clipPos[1] = xyz_lock_pos[1];
	clipPos[2] = xyz_lock_pos[2];
      }
    } // end if xy_lock

  // restrict based on z_lock
  if( z_lock )
    {
      if( microscope->state.modify.control == DIRECTZ )
	{ clipPos[2] = xyz_lock_pos[2]; }
      // else in feedback, and the tip stays on the surface.
    } // end if z_lock

  // restrict based on direct step
  if (microscope->state.modify.tool == DIRECT_STEP) {    
    if( xy_lock ) {
      // if we are in xy_lock and in direct_Step mode, we want to be able to 
      // move microscope and update position on screen
      if(microscope->state.modify.direct_step_param == DIRECT_STEP_PLANE) {
	BCPlane* plane = dataset->inputGrid->getPlaneByName
	  (dataset->heightPlaneName->string());
	
	//current position of microscope
	clipPos[0] = microscope->state.data.inputPoint->x();
	clipPos[1] = microscope->state.data.inputPoint->y();
	
	//height from BCPlane
	//so that the sphere knows where to be positioned when in feedback
	double height;
	plane -> valueAt(&height,clipPos[0],clipPos[1]);
	clipPos[2] = height;
	z_pos = height;
      } 
      else {
	// Direct_STEP_3D
	clipPos[0] = microscope->state.data.inputPoint->x();
	clipPos[1] = microscope->state.data.inputPoint->y();
	clipPos[2] = z_pos;
      }
    }
    curr_x = microscope->state.data.inputPoint->x() - plane->xInWorld(0);
    curr_y = microscope->state.data.inputPoint->y() - plane->yInWorld(0);
    curr_z = z_pos;
  } // end if direct step mode
  
  // constrained freehand only allows motion along a line.
  if (microscope->state.modify.tool == CONSTR_FREEHAND) {
    if (microscope->state.modify.control == DIRECTZ ) {
      nmui_Util::
	clipPositionLineConstraint(plane, clipPos, 
				   microscope->state.modify.stored_points,
				   microscope->state.modify.tool,
				   microscope->state.modify.constr_xyz_param);
    }
    else { // feedback
      nmui_Util::
	clipPositionLineConstraint(plane, clipPos, 
				   microscope->state.modify.stored_points);
    } 
  }
  // end restricting the tip position for various modes
  ///////////////////////////


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
      /* Request a reading from the current location,
       * and wait till tip gets there */
      microscope->TakeFeelStep(clipPos[0], clipPos[1], value, 1);
      break;
    case HOLD_EVENT:
      if (microscope->state.modify.tool == FEELAHEAD) {
        // Feelahead mode IGNORES the commit button;  it never
        // leaves image mode.
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
          specify_directZ_force(whichUser);
          monitor.startForceField();
      } else {
        setupHaptics(USER_PLANEL_MODE);
	// Apply force to the user based on current sample points
	if (!microscope->d_relax_comp.is_ignoring_points() ) {
          touch_surface(whichUser, clipPos);
          monitor.startSurface();
        }
      }
      break;
	
      /* Go back to scanning upon release */
    case RELEASE_EVENT:	
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
          p.start();
          while(p.notDone()) {
      	    // get rid of the icons marking the line endpoints
      	    p.del();  //this moves the pointer forward to the next point.
          }
        }
      }

      /* Start image mode (to make the relaxation compensation code work
       * as it should, and to turn the background the right color in case
       * we don't scan more, and to stop points from coming into the list
       * of points) and resume previous scan pattern. */
      microscope->ImageMode();
      if (microscope->state.autoscan) {
	      microscope->ResumeWindowScan();
      }
      break;

    default:
      break;
  } // end switch( userEvent )

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
    float	        handx,handy;
    float	        centerx,centery;
    float		diffx,diffy, diffmax, select_rad;

    /* if we are not running live, you should not be able
	   to do this, so put the user into grab mode */
    if (!microscope || (dataset->inputGrid->readMode() != READ_DEVICE)) 
    {
        user_0_mode = USER_GRAB_MODE;
        printf("Select mode available only on live data!!!\n");
        return 0;
    }

    if (!microscope->haveMutex()) {
        user_0_mode = USER_GRAB_MODE;
        printf("Can't select when we don't have access to the microscope.\n");
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

    // Move the aiming line to the user's hand location
    // We don't want to clip it to the heightplane, so pass NULL
    decoration->aimLine.moveTo(worldFromHand.xlate[0],
                               worldFromHand.xlate[1], NULL);
    handx = worldFromHand.xlate[0];
    handy = worldFromHand.xlate[1];
    centerx = microscope->state.select_center_x;
    centery = microscope->state.select_center_y;
    select_rad = microscope->state.select_region_rad;
    // What are we going to change, center of region
    // or size of region?
    
    // Is hand near center (20% of proposed scan region) ?
    // This is exactly the same size as the handle drawn in 
    // graphics code, globjects.c:make_rubber_corner
    if ((fabs(handx-centerx) < 0.2*(select_rad))&&
        (fabs(handy-centery) < 0.2*(select_rad))){
        prep_select_drag_mode = REG_TRANSLATE;
    } else if (((fabs(handx-centerx) < 1.03*select_rad) &&
                (fabs(handy-centery) > 0.9*select_rad) &&
                (fabs(handy-centery) < 1.1*select_rad)) ||
               ((fabs(handy-centery) < 1.03*select_rad) &&
                (fabs(handx-centerx) > 0.9*select_rad) &&
                (fabs(handx-centerx) < 1.1*select_rad))){
        // Are we near an edge?
        prep_select_drag_mode = REG_SIZE;
        
    } else {
        // Not near any features. 
        prep_select_drag_mode = REG_NULL;
   }

    switch ( userEvent ) {

    case PRESS_EVENT:	
        if (prep_select_drag_mode == REG_NULL) {
            // Weren't near any features, drag a new select area
            select_drag_mode = REG_SIZE;
            microscope->state.select_center_x = handx;
            microscope->state.select_center_y = handy;
            microscope->state.select_region_rad = 0;
        } else {       
            select_drag_mode = prep_select_drag_mode;
        }
        break;

    case RELEASE_EVENT:	
        // Do same stuff for hold or release

    case HOLD_EVENT:
        if (select_drag_mode == REG_TRANSLATE) {
            // Move the center of the select region
            centerx = handx ;
            centery = handy ;
            // Keep region inside scanner range
            if ((centerx - select_rad) < 
                microscope->state.xMin) {
                centerx = microscope->state.xMin + 
                    select_rad;
            }
            if ((centerx + select_rad) > 
                microscope->state.xMax) {
                centerx = microscope->state.xMax - 
                    select_rad;
            }
            if ((centery - select_rad) < 
                microscope->state.yMin) {
                centery = microscope->state.yMin + 
                    select_rad;
            }
            if ((centery + select_rad) > 
                microscope->state.yMax) {
                centery = microscope->state.yMax - 
                    select_rad;
            }
            microscope->state.select_center_x = centerx ;
            microscope->state.select_center_y = centery ;
        } else {
            // Move the edge of the select region
            // Figure out the differential in x and y from the center
            diffx = fabs(handx - centerx);
            diffy = fabs(handy - centery);

            // Find the larger of the two and use it for both, so it is square
            diffmax = max(diffx, diffy);
            // Clip the region to make sure we stay within the boundaries
            // of the scan range.
            diffmax = min(diffmax, centerx - microscope->state.xMin); // left
            diffmax = min(diffmax, microscope->state.xMax - centerx); // right
            diffmax = min(diffmax, centery - microscope->state.yMin); // bottom
            diffmax = min(diffmax, microscope->state.yMax - centery); // top
            microscope->state.select_region_rad = diffmax;
        }
        if (userEvent == RELEASE_EVENT) {
            // Helps display highlight correctly, below
            select_drag_mode = REG_NULL;
        }
        break;

    default:
        break;
    }

    // Display the scan region extent as the user plans to change it. 
    decoration->selectedRegionMaxX = microscope->state.select_center_x 
        + microscope->state.select_region_rad;
    decoration->selectedRegionMinX = microscope->state.select_center_x 
        - microscope->state.select_region_rad;
    decoration->selectedRegionMaxY = microscope->state.select_center_y 
        + microscope->state.select_region_rad;
    decoration->selectedRegionMinY = microscope->state.select_center_y 
        - microscope->state.select_region_rad;
    // Draw the new bounding box and center handle for the region. 
    graphics->positionRubberCorner(
        decoration->selectedRegionMinX, 
        decoration->selectedRegionMinY, 
        decoration->selectedRegionMaxX, 
        decoration->selectedRegionMaxY, 
        (select_drag_mode==REG_NULL)?prep_select_drag_mode:select_drag_mode); 

    return(0);
}

static int regionOutsidePlane(BCPlane* plane ,
                              float region_center_x,
                              float region_center_y,
                              float region_width,
                              float region_height,
                              float region_angle) 
{
    // The idea is to check and see if the entire region area is
    // outside of the plane, then return true.
    while (region_angle > 3*M_PI/2) region_angle -= 2*M_PI;
    while (region_angle < -M_PI/2) region_angle += 2*M_PI;

    float A,B,C,cx1, cy1, cx2, cy2, cx3, cy3, cx4, cy4; 
    // Formula Ax + By + C = 0 of edge of region
    // Derived from y = mx +b, mx -y + b = 0, so A=m, B = -1, C = b

    // If all corners of plane fall on outside of any edge, region is
    // totally outside plane. 
    // Sign factor I don't understand, but it works. 
    int sign = 1;
    if (region_angle > M_PI/2) sign = -1;
    A = sign * tan(region_angle);
    B = -sign;
    cx1 = region_center_x +cos(region_angle)*region_width
        -sin(region_angle)*region_height;
    cy1 = region_center_y +cos(region_angle)*region_height
        +sin(region_angle)*region_width;
    C = sign *(-cx1 * tan(region_angle) + cy1);

    if ((A* plane->maxX() + B*plane->maxY() + C ) < 0 &&
        (A* plane->maxX() + B*plane->minY() + C ) < 0 &&
        (A* plane->minX() + B*plane->maxY() + C ) < 0 &&
        (A* plane->minX() + B*plane->minY() + C ) < 0 ) {
        return 1;
    }
    cx2 = region_center_x +cos(region_angle)*-region_width
        -sin(region_angle)*-region_height;
    cy2 = region_center_y +cos(region_angle)*-region_height
        +sin(region_angle)*-region_width;
    C = sign *(-cx2 * tan(region_angle) + cy2);
    
    // Other side of this line. 
    if ((A* plane->maxX() + B*plane->maxY() + C ) > 0 &&
        (A* plane->maxX() + B*plane->minY() + C ) > 0 &&
        (A* plane->minX() + B*plane->maxY() + C ) > 0 &&
        (A* plane->minX() + B*plane->minY() + C ) > 0 ) {
        return 1;
    }
    // We need these later, so calc now before angle changes.
    cx3 = region_center_x +cos(region_angle)*-region_width
        -sin(region_angle)*region_height;
    cy3 = region_center_y +cos(region_angle)*region_height
        +sin(region_angle)*-region_width;
    cx4 = region_center_x +cos(region_angle)*region_width
        -sin(region_angle)*-region_height;
    cy4 = region_center_y +cos(region_angle)*-region_height
        +sin(region_angle)*region_width;


    region_angle = region_angle + M_PI/2.0;
    if (region_angle > 3*M_PI/2) region_angle -= 2*M_PI;
    // Sign factor I don't understand, but it works. 
    sign = 1;
    if (region_angle > M_PI/2) sign = -1;
    A = sign * tan(region_angle);
    B = -sign;
    C = sign *(-cx1 * tan(region_angle) + cy1);
    
    if ((A* plane->maxX() + B*plane->maxY() + C ) > 0 &&
        (A* plane->maxX() + B*plane->minY() + C ) > 0 &&
        (A* plane->minX() + B*plane->maxY() + C ) > 0 &&
        (A* plane->minX() + B*plane->minY() + C ) > 0 ) {
        return 1;
    }
    
    C = sign *(-cx2 * tan(region_angle) + cy2);
    // Other side of this line. 
    if ((A* plane->maxX() + B*plane->maxY() + C ) < 0 &&
        (A* plane->maxX() + B*plane->minY() + C ) < 0 &&
        (A* plane->minX() + B*plane->maxY() + C ) < 0 &&
        (A* plane->minX() + B*plane->minY() + C ) < 0 ) {
        return 1;
    }

    // Now, checks for the other way around. 
    // Corner coords are contained in cx* and cy* calculated above. 
    A = 0;
    B = -1;
    C = plane->minY();
    // Simplfy checks based on these values
    if (( -cy1 + C ) > 0 &&
        ( -cy2 + C ) > 0 &&
        ( -cy3 + C ) > 0 &&
        ( -cy4 + C ) > 0 ) {
        return 1;
    }
    A = 0;
    B = -1;
    C = plane->maxY();
    // Simplfy checks based on these values
    if (( -cy1 + C ) < 0 &&
        ( -cy2 + C ) < 0 &&
        ( -cy3 + C ) < 0 &&
        ( -cy4 + C ) < 0 ) {
        return 1;
    }
    //A = infinity;
    //B = -1;
    // C used for X intercept. 
    C = plane->minX();
    // Simplfy checks based on these values
    if (( -cx1 + C ) > 0 &&
        ( -cx2 + C ) > 0 &&
        ( -cx3 + C ) > 0 &&
        ( -cx4 + C ) > 0 ) {
        return 1;
    }
    //A = infinity;
    //B = -1;
    // C used for X intercept.
    C = plane->maxX();
    // Simplfy checks based on these values
    if (( -cx1 + C ) < 0 &&
        ( -cx2 + C ) < 0 &&
        ( -cx3 + C ) < 0 &&
        ( -cx4 + C ) < 0 ) {
        return 1;
    }
    return 0;
}

/**
 * Region mode
 * doRegion - Perform rubber-banding upon press, move, release
 */
int 
doRegion(int whichUser, int userEvent)
{
    v_xform_type	worldFromHand;
    q_matrix_type	hand_mat;
    q_vec_type          angles;
    float	        handx,handy, hand_angle;

    BCPlane* plane = dataset->inputGrid->getPlaneByName
        (dataset->heightPlaneName->string());
    if (plane == NULL)
    {
        fprintf(stderr, "Error in doregion: could not get plane!\n");
        return -1;
    }  

    /* Move the tip to the hand x,y location */
    /* Set its height based on data at this point */
    v_get_world_from_hand(whichUser, &worldFromHand);

    // Move the aiming line to the user's hand location
    // We don't want to clip it to the heightplane, so pass NULL
    decoration->aimLine.moveTo(worldFromHand.xlate[0],
                               worldFromHand.xlate[1], NULL);
    
    q_to_col_matrix(hand_mat, worldFromHand.rotate);
    q_col_matrix_to_euler( angles, hand_mat );
    hand_angle = -angles[YAW];
    handx = worldFromHand.xlate[0];
    handy = worldFromHand.xlate[1];

    // Is hand near center (20% of proposed  region) ?
    // This is exactly the same size as the handle drawn in 
    // graphics code, globjects.c:make_region_box
    if ((xform_width(handx-region_center_x, handy-region_center_y, 
                     region_angle) < 0.2*(region_width))&&
        (xform_height(handx-region_center_x, handy-region_center_y, 
                      region_angle) < 0.2*(region_height))){
        prep_region_drag_mode = REG_PREP_TRANSLATE;
    } else if ((xform_height(handx - region_center_x, 
                             handy - region_center_y, 
                             region_angle) < 1.03*region_height) && 
               ((xform_width(handx-region_center_x,
                             handy-region_center_y, 
                             region_angle) > 0.9*(region_width))&&
                (xform_width(handx-region_center_x,
                             handy-region_center_y, 
                             region_angle) < 1.1*(region_width)))
               ) {
        // near width edge, so resize those edges. 
        prep_region_drag_mode = REG_PREP_SIZE_WIDTH;

    } else if ((xform_width(handx - region_center_x, 
                            handy - region_center_y, 
                            region_angle) < 1.03*region_width) &&
               ((xform_height(handx-region_center_x,
                              handy-region_center_y, 
                              region_angle) > 0.9*(region_height))&&
                (xform_height(handx-region_center_x,
                              handy-region_center_y, 
                              region_angle) < 1.1*(region_height)))
               ) {
        // near height edge, so resize those edges. 
        prep_region_drag_mode = REG_PREP_SIZE_HEIGHT;
        
    } else {
        prep_region_drag_mode = REG_NULL;
    }
    
    // Carefull! Nested switch statements!
    switch ( userEvent ) {

    case PRESS_EVENT:	
        switch(prep_region_drag_mode) {
        case REG_PREP_TRANSLATE:
            region_center_x = handx;
            region_center_y = handy;
            region_base_tracker_angle = region_angle + hand_angle;
            region_drag_mode = REG_TRANSLATE;
            break;
        case REG_NULL:
            // If we're not near a feature when we click, switch
            // to creating and sizing a new region. 
            region_drag_mode = REG_SIZE;
            prep_region_drag_mode = REG_PREP_SIZE;
            // reset region size for dragging. 
            region_center_x = handx;
            region_center_y = handy;
            region_width = 0;
            region_height = 0;
            region_base_tracker_angle = hand_angle;
            region_angle = 0;
             break;
        case REG_PREP_SIZE_WIDTH:
            region_drag_mode = REG_SIZE_WIDTH;
            break;
        case REG_PREP_SIZE_HEIGHT:
            region_drag_mode = REG_SIZE_HEIGHT;
            break;
        case REG_PREP_SIZE:
            region_drag_mode = REG_SIZE;
            break;
        }
        break;
       
    case RELEASE_EVENT:	
        // Do same stuff for hold or release

    case HOLD_EVENT:
        switch(region_drag_mode) {
        case REG_TRANSLATE:
            // Move the center of the region region
            region_center_x = handx ;
            region_center_y = handy ;
            // Change the angle, too. 
            region_angle = region_base_tracker_angle - hand_angle;
            break;
        case REG_SIZE_WIDTH:
            region_width = xform_width(handx - region_center_x,
                                       handy - region_center_y,
                                       region_angle);
            break;
        case REG_SIZE_HEIGHT:
            region_height = xform_height(handx - region_center_x,
                                         handy - region_center_y,
                                         region_angle);
            break;
        case REG_SIZE:
            // Resize the region
            //region_angle = region_base_tracker_angle - hand_angle;
            region_width = xform_width(handx - region_center_x,
                                       handy - region_center_y,
                                       region_angle);
            region_height = xform_height(handx - region_center_x,
                                         handy - region_center_y,
                                         region_angle);
            // passed as param below, so we display highlight. 
            prep_region_drag_mode = REG_PREP_SIZE;
            break;
        default:
            break;
        }
        if (userEvent == RELEASE_EVENT) {
            // Helps display highlight correctly, below
            region_drag_mode = REG_NULL;
            if (regionOutsidePlane(plane, region_center_x, region_center_y, 
                                   region_width, region_height, region_angle)){
                // Tell gfx to delete the region
                region_drag_mode = REG_DEL;
                // Reset region size, position to zero. 
                region_center_x = region_center_y = region_width =
                    region_height = region_angle = 0;
            }
        }
        break;

    default:
        break;
    }

    // Display the scan region extent as the user plans to change it. 
    // Draw the new bounding box and center handle for the region. 
    graphics->positionRegionBox(
         region_center_x, region_center_y, 
         region_width, region_height, Q_RAD_TO_DEG(region_angle), 
         (region_drag_mode==REG_NULL)?prep_region_drag_mode:region_drag_mode);
    return(0);
}


/**
 * doWorldGrab - handle world grab operation
 */
int
doWorldGrab(int whichUser, int userEvent)
{
    v_xform_type    	    worldFromHand;
    v_xform_type            roomFromHand, roomFromSensor, handFromRoom;
    static v_xform_type     oldWorldFromHand;
	static v_xform_type		oldObject;

	// for updating the object's tcl variables when rotating and translating
	extern Tclvar_float import_transx;
	extern Tclvar_float import_transy;
	extern Tclvar_float import_transz;
	extern Tclvar_float import_rotx;
	extern Tclvar_float import_roty;
	extern Tclvar_float import_rotz;

    BCPlane* plane = dataset->inputGrid->getPlaneByName
                     (dataset->heightPlaneName->string());
    if (plane == NULL)
    {
        fprintf(stderr, "Error in doWorldGrab: could not get plane!\n");
        return -1;
    }     
    // Move the aiming line to the users hand location 
    v_get_world_from_hand(whichUser, &worldFromHand);
    decoration->aimLine.moveTo(worldFromHand.xlate[0],
                            worldFromHand.xlate[1], plane);

  v_xform_type temp;
  UTree *node;

  switch ( userEvent )
    {

    case PRESS_EVENT:
	/* get snapshot of hand in world space == w_from_h  */
	v_get_world_from_hand(whichUser, &oldWorldFromHand);
	node = World.TGetNodeByName(*World.current_object);
	if (node != NULL) {
		URender &obj = node->TGetContents();
		if (obj.GetGrabObject() == 1) {
			q_copy(oldObject.rotate, obj.GetLocalXform().GetRot());
			q_vec_copy(oldObject.xlate, obj.GetLocalXform().GetTrans());
		}
	}
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

	// Change so that we can grab objects instead of the world...
	node = World.TGetNodeByName(*World.current_object);
	if (node != NULL) {
		URender &obj = node->TGetContents();
		if (obj.GetGrabObject() == 1) {
			q_type q;
			q_invert(q, oldWorldFromHand.rotate);
			q_mult(q, q, worldFromHand.rotate);
			q_mult(q, oldObject.rotate, q);

			q_vec_type v;
			q_vec_subtract(v, worldFromHand.xlate, oldWorldFromHand.xlate);
			q_vec_add(v, oldObject.xlate, v);

			node->TGetContents().GetLocalXform().SetRotate(q);
			node->TGetContents().GetLocalXform().SetTranslate(v);

			// update tcl variables
			import_transx = v[0];
			import_transy = v[1];
			import_transz = v[2];

			q_to_euler(v, q);

			import_rotx = Q_RAD_TO_DEG(v[2]);
			import_roty = Q_RAD_TO_DEG(v[1]);
			import_rotz = Q_RAD_TO_DEG(v[0]);
		}
		else {
			updateWorldFromRoom(&temp);
		}
	}
	else {
		updateWorldFromRoom(&temp);
	}

        // NANOX
collabVerbose(5, "doWorldGrab:  updateWorldFromRoom().\n");
 

	break;

    case RELEASE_EVENT:
    default:
	/* do nothing	*/
	break;
    }

return 0;
}	/* doWorldGrab */



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

