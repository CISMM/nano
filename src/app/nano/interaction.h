#ifndef INTERACTION_H
#define INTERACTION_H

// defined in interaction.c

int interaction (int bdbox_buttons [], double bdbox_dials [],
                 int phantomButton);
int set_aim_line_color (float);

// NANOX
// Set up synchronization variables with meaningful values.
void initializeInteraction (void);
void updateWorldFromRoom (void);

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

