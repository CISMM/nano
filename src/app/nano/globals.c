#include "globals.h"

//#include <stdio.h>

// HACK HACK HACK
// copied from microscape.h

#define NUM_USERS 1

// from microscape.c:  TCH 2 Sept 98

// ************************* definition taken from armlib ***************
// all uses of this variable should be removed at some point
float Arm_knobs[9];             /* Knobs from the ARM */
// **********************************************************************

int     spm_graphics_verbosity = 0;     ///< How much information to print?
int     timer_verbosity = 0;    ///< How much information about timer to print

Timer   mytimer, frametimer;

vrpn_bool stm_new_frame = VRPN_FALSE;

int justCentered = 0;
int ohmmeter_enabled = 0;
int xenable = 0;
int xg_start = 0;






