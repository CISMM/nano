#ifndef CHARTJUNK_H
#define CHARTJUNK_H

// chartjunk
//
// Tom Hudson, April 1998

// Chartjunk is the ONLY file in the graphics package that depends
// on Microscope.  To make graphics independant of Microscope,
// comment out the call to addChartjunk() in replaceDefaultObjects()
// in globjects.c

extern int	myfont;	// Font to use

struct nmg_Funclist;  // from nmg_Funclist.h

class nmg_State;

//int scale_display (void *);
//int x_y_width_display (void *);
//int height_at_hand_display (void *);
//int rate_display (void *);
//int screenaxis_display (void *);

//int control_display (void *);
//int mode_display (void *);
//int measure_display (void *);

int addChartjunk (nmg_Funclist ** v_screen, nmg_State * state);
  // adds the above display routines to the specified funclist
  // (should be v_screen)

void initializeChartjunk (const char * = NULL);
  // loads the named font (NULL uses default)


#endif  // CHARTJUNK_H
