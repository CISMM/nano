/*===3rdtech===
  Copyright (c) 2000 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#ifndef TCL_TK_H
#define TCL_TK_H

#include <nmb_Types.h>  // for vrpn_bool

struct Tcl_Interp;  // from <tcl.h>
class ColorMap;

extern int init_Tk_control_panels (const char * tcl_script_dir,
                                   vrpn_bool useOptimism);
extern int poll_Tk_control_panels (void);

extern void set_Tk_command_handler (void (*) (char *, vrpn_bool *, int));

extern Tcl_Interp * get_the_interpreter (void);

int makeColorMapImage(ColorMap * cmap, char * name, int width, int height, 
                      float c_min = 0, float c_max = 1); 
void tcl_colormapRedraw();

#endif //TCL_TK_H
