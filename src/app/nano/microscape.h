/** \file microscape.h 
    All user #defines and typedefs

    If it doesn't need to be visible outside its source file,
    don't declare it here!
 *
 *****************************************************************************/
#ifndef	MICROSCAPE_H
#define	MICROSCAPE_H

#include <iostream.h>

// iostream must come before this, in order to compile with Flow!
#ifdef	hpux
#define	signed
#endif

#include <nmb_Types.h>
#include <display.h>

#include <nmb_TimerList.h>

// Removed many header files to speed up compilation

#include <vrpn_ForceDevice.h>
#include <vrpn_Analog.h>
#include "nM_coord_change.h"  //so we can track collaborator's hand position

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include <nmb_Types.h>

#include "Timer.h"

#ifndef TCL_LINKVAR_H
#include <Tcl_Linkvar.h>
#include <Tcl_Netvar.h>
#endif

class ColorMap;
class PPM;
class nmb_Dataset;
class nmb_Decoration;
//class Microscope;
class Xform;		//added from ugraphics
#ifndef NO_MAGELLAN
class vrpn_Magellan;
class vrpn_Tracker_AnalogFly;
class vrpn_Text_Receiver;
#endif
class vrpn_Phantom;
class vrpn_MousePhantom;

#define	MICROSCAPE_MAJOR_VERSION	(10)
#define	MICROSCAPE_MINOR_VERSION	(0)


#define HEAD	0
#define HAND	1

#define NUM_USERS   	1
#define NUM_OBJECTS 	100
#define NUM_ELEMENTS	20

/* rough guess at update rate	*/
#define	UPDATE_RATE 	20

#define DEFAULT_BG_COLOR    {0, 40, 120}

/** This function is used to determine whether two coordinates (in x or
 y) are near each other.  It is basically the floating-point equivalent
 of '==', but allowing a little tolerance.
 The tolerance given here is 0.01 nanometers, which is 1/10 Angstrom. */
inline	int NM_NEAR(double x0,double x1) { return (fabs(x0-x1) < 0.01); }


// used by vrpn.c
extern	ColorMap	*curColorMap;	// microscape.c


/************************
 * External stuff
 ************************/

extern int      stride;                 // vrml.C

#define	NUM_KNOBS	(5)
extern	float		MIN_K;  /* make these variable, different */
extern	float		MAX_K;  /* for each device		  */

/* vrpn stuff */
#define BDBOX_NUMBUTTONS 32
#define BDBOX_NUMDIALS 8

#ifndef NO_MAGELLAN
#define MAGELLAN_NUMBUTTONS 9
#endif

// button events - set by interaction
#define NULL_EVENT	0	///< button not pressed at least twice in a row
#define PRESS_EVENT	1	///< button just pressed
#define RELEASE_EVENT	2	///< button just released
#define HOLD_EVENT	3	///< button pressed at least twice in a row

extern  vrpn_Phantom * phantServer;
extern  vrpn_MousePhantom * mousePhantomServer;
extern  vrpn_ForceDevice_Remote *forceDevice;
extern  vrpn_Tracker_Remote *vrpnHeadTracker;
extern  vrpn_Tracker_Remote *vrpnHandTracker;
extern  int             headSensor;
extern  int             handSensor;
extern  vrpn_Button_Remote *phantButton;
extern  int phantButtonState;
extern  vrpn_Button_Remote *buttonBox;
extern  vrpn_Analog_Remote *dialBox;
extern  int bdboxButtonState[BDBOX_NUMBUTTONS];
extern  double bdboxDialValues[BDBOX_NUMDIALS];

#ifndef NO_MAGELLAN
extern  vrpn_Magellan *magellanButtonBoxServer;
extern  vrpn_Tracker_AnalogFly *magellanTrackerServer;

extern  vrpn_Button_Remote *magellanButtonBox;
extern  int magellanButtonState[MAGELLAN_NUMBUTTONS];
extern  vrpn_Analog_Remote *magellanPuckAnalog;
extern  vrpn_Tracker_Remote *magellanPuckTracker;
extern  vrpn_Text_Receiver * magellanTextRcvr;
extern  vrpn_bool magellanPuckActive;

void shutdown_Magellan();
#endif
/* end vrpn stuff */

// used in interaction.c
extern int mouse3button;
extern int using_mouse3button;
extern int do_cpanels;

#define VTK_NO 0


// Only list things here if they need to be shared with other files!

extern	Tclvar_int phantom_button_mode;	// microscape.c

//---------------------------------------------------------------------------
/// These select the plane to map color from and the scale of the mapping. 
extern  Tclvar_float            color_min_limit;
extern  Tclvar_float            color_max_limit;
extern  TclNet_float            color_min, color_max;
extern  TclNet_float            data_min, data_max;
extern TclNet_int surface_r;
extern TclNet_int surface_g;
extern TclNet_int surface_b;

//--------------------------------------------------------------------------
///These select the plane to map compliance from and teh scale of the mapping.
extern  Tclvar_float            compliance_slider_min_limit;
extern  Tclvar_float            compliance_slider_max_limit;
extern  Tclvar_float            compliance_slider_min,compliance_slider_max;
extern  Tclvar_string         compliancePlaneName;



//--------------------------------------------------------------------------
///These select the plane to map friction from and teh scale of the mapping.
extern  Tclvar_float            friction_slider_min_limit;
extern  Tclvar_float            friction_slider_max_limit;
extern  Tclvar_float            friction_slider_min, friction_slider_max;
extern  Tclvar_string         frictionPlaneName;

//--------------------------------------------------------------------------
///These select the plane to map bump size from and the scale of the mapping.
extern  Tclvar_float            bump_slider_min_limit;
extern  Tclvar_float            bump_slider_max_limit;
extern  Tclvar_float            bump_slider_min, bump_slider_max;
extern  Tclvar_string         bumpPlaneName;

//--------------------------------------------------------------------------
///This selects the plane to map opacity to.
extern  Tclvar_string           bumpPlaneName;

//--------------------------------------------------------------------------
///These select the plane to map buzz amplitude from and the scale of the mapping
extern  Tclvar_float            buzz_slider_min_limit;
extern  Tclvar_float            buzz_slider_max_limit;
extern  Tclvar_float            buzz_slider_min, buzz_slider_max;
extern  Tclvar_string         buzzPlaneName;

//--------------------------------------------------------------------------
///These select the plane to map friction from and the scale of the mapping.
extern  Tclvar_float            adhesion_slider_min_limit;
extern  Tclvar_float            adhesion_slider_max_limit;
extern  Tclvar_float            adhesion_slider_min, adhesion_slider_max;
extern  Tclvar_string         adhesionPlaneName;

//-------------------------------------------------------------------------
///This value is used when no plane is assigned
extern Tclvar_float             default_spring_k;

//--------------------------------------------------------------------------
///For mapping sound to dataset
extern Tclvar_float              sound_slider_min_limit;
extern Tclvar_float              sound_slider_max_limit;
extern Tclvar_float              sound_slider_min, sound_slider_max;
extern Tclvar_string           soundPlaneName;

//---------------------------------------------------------------------------
/// This selects the plane to map x from. 
extern  Tclvar_string         xPlaneName;

//---------------------------------------------------------------------------
/// The scale factor for force applied in Direct Z Control
extern  Tclvar_float           directz_force_scale;

/// Guardedscan TCL/TK interface variables
extern  Tclvar_int   guarded_plane_acquire;
extern	Tclvar_float guarded_plane_depth;

/*********
 * Functions defined in one file and used in another (added by KPJ to satisfy
 * g++)...
 *********/

/* defined in microscape.c */
extern void handleCharacterCommand(char*, vrpn_bool* , int);
extern void center (void);
extern void get_Plane_Centers(float*,float*,float*);
extern void set_channel_for_ohmeter(char* channel_name);
extern void cause_grid_redraw(float new_value, void *userdata);
extern int register_vrpn_phantom_callbacks(void);

// XXX - this has to do with the user interface but there isn't
// a global user interface object so I put it here temporarily (AAS)
enum TextureMode {RULERGRID, CONTOUR, ALPHA, SEM, REGISTRATION, MANUAL_REALIGN};
extern int disableOtherTextures (TextureMode m);

// things defined in global.h:  spm_graphics_verbosity, timer_verbosity,
// mytimer, frametimer, stm_new_frame, mode_change, tcl_offsets, user_mode,
// justCentered, ohmmeter_enabled, xenable, xg_start

extern nmb_TimerList graphicsTimer;
extern nmb_TimerList collaborationTimer;


// Tom's MCN experiments - Oct 2000
class vrpn_RedundantRemote;
extern vrpn_RedundantRemote * microscopeRedundancyController;

#endif	/* MICROSCAPE_H */


