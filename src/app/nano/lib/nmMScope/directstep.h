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
extern TclNet_float step_x;
extern TclNet_float step_y;
extern TclNet_float step_z;

// extern position_sphere(float, float, float);
extern double z_pos;
extern TclNet_int xy_lock;
#endif //DIRECTSTEP_H

