#ifndef GLOBALS_H
#define GLOBALS_H

// Some of the globals that we haven't eliminated yet.
// Defined in globals.c

#include <nmb_Types.h>  // for vrpn_bool!

#include "Timer.h"

extern float Arm_knobs [];
extern int spm_graphics_verbosity;
extern int timer_verbosity;

extern Timer mytimer, frametimer;

extern vrpn_bool stm_new_frame;

extern int mode_change;

extern float tcl_offsets [];

extern int justCentered;
extern int ohmmeter_enabled;
extern int xenable;
extern int xg_start;

extern int     user_mode [];

#endif  // GLOBALS_H
