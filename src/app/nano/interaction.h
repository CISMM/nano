#ifndef INTERACTION_H
#define INTERACTION_H

//This causes a redefinition of v_xform_type, which causes trouble on CYGWIN
//where "trouble" = "crashes the compiler"
// - CCW 02/17/00
//
//struct v_xform_type;  // from <v.h>

// defined in interaction.c

int interaction (int bdbox_buttons [], double bdbox_dials [],
                 int phantomButton, nmb_TimerList * timer);
int set_aim_line_color (float);

// NANOX
// Set up synchronization variables with meaningful values.

void initializeInteraction (void);

// NANOX
// Send our new proposed world_from_room transform
// (v_world.users[whichUser].xforms[0]) out into the net to be
// synchronized if necessary.  There SHOULD be a callback that makes
// sure that the new values get written into Tcl;  we need to verify
// that it's actually firing.  If src is NULL reads directly out of
// vlib (v_world.users[whichUser].xforms[0]).

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

#endif  // INTERACTION_H

