#ifndef TCL_TK_H
#define TCL_TK_H

#include <nmb_Types.h>  // for vrpn_bool

class Tcl_Interp;  // from <tcl.h>

extern int init_Tk_control_panels (const char * tcl_script_dir);
extern int poll_Tk_control_panels (void);

extern void set_Tk_command_handler (void (*) (char *, vrpn_bool *, int));

extern Tcl_Interp * get_the_interpreter (void);


#endif //TCL_TK_H
