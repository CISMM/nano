
/* drift.h - public functions and variables for drift compensation
**/

#ifndef MF_DRIFT_H
#define MF_DRIFT_H

/* variables: none
**/

/* functions:
**/
int 	driftZDirty (void);
double 	drift_comp (int, int, double, long, long, int do_y_fastest);

#endif
