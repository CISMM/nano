#ifndef INTERACTION_H
#define INTERACTION_H

#include <v.h>
//struct v_xform_type;  // from <v.h>

// defined in interaction.c

int interaction (int bdbox_buttons [], double bdbox_dials [],
                 int phantomButton, nmb_TimerList * timer);
int set_aim_line_color (float);

extern Tclvar_int tcl_commit_pressed;
extern Tclvar_int tcl_commit_canceled;
void handle_commit_change( vrpn_int32, void *);
void handle_commit_cancel( vrpn_int32, void *);

// NANOX
/// Set up synchronization variables with meaningful values.

void initializeInteraction (void);
void linkMicroscopeToInterface (nmm_Microscope_Remote *);

int clear_polyline( void * userdata );


void updateMicroscopeRTTEstimate (double time);

// NANOX
/** Send our new proposed world_from_room transform
(v_world.users[whichUser].xforms[0]) out into the net to be
 synchronized if necessary.  There SHOULD be a callback that makes
 sure that the new values get written into Tcl;  we need to verify
 that it's actually firing.  If src is NULL reads directly out of
 vlib (v_world.users[whichUser].xforms[0]). */

void updateWorldFromRoom (v_xform_type * src = NULL);

extern TclNet_float tcl_lightDirX;
extern TclNet_float tcl_lightDirY;
extern TclNet_float tcl_lightDirZ;
extern TclNet_float tcl_wfr_xlate_X;
extern TclNet_float tcl_wfr_xlate_Y;
extern TclNet_float tcl_wfr_xlate_Z;
extern TclNet_float tcl_wfr_rot_0;
extern TclNet_float tcl_wfr_rot_1;
extern TclNet_float tcl_wfr_rot_2;
extern TclNet_float tcl_wfr_rot_3;
extern TclNet_float tcl_wfr_scale;

extern Tclvar_float handTracker_update_rate;

extern Tclvar_int xy_lock;

/************************
 * Which knob performs which function
 ************************/

#define FORCE_KNOB              (1)
#define MOD_FORCE_KNOB          (2) /* arbitrary knob */
#define IMG_FORCE_KNOB          (6) /* arbitrary knob */
#define FRICTION_KNOB           (5)
#define RATE_KNOB               (3)
#define RECOVERY_KNOB           (0) /* DIM recover cylces */

#endif  // INTERACTION_H

