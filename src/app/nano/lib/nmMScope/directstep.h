#ifndef DIRECTSTEP_H
#define DIRECTSTEP_H

//--------------------------------------
//direct step TCL

extern void handle_take_x_step(vrpn_float64, void *mptr);
extern void handle_take_y_step(vrpn_float64, void *mptr);
extern void handle_take_z_step(vrpn_float64, void *mptr);

extern void handle_step_go_to_pos(vrpn_int32, void *mptr);
//------------------------------------------------------------





//direct step variables
extern Tclvar_float step_x;
extern Tclvar_float step_y;
extern Tclvar_float step_z;
extern Tclvar_float step_x_pos;
extern Tclvar_float step_y_pos;
extern Tclvar_float step_z_pos;
// extern position_sphere(float, float, float);
extern double z_pos;
extern TclNet_int xy_lock;
#endif //DIRECTSTEP_H

