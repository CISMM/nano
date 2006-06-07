#ifndef DIRECTSTEP_H
#define DIRECTSTEP_H

//--------------------------------------
//direct step TCL

extern void handle_take_x_step(vrpn_float64, void *mptr);
extern void handle_take_y_step(vrpn_float64, void *mptr);
extern void handle_take_z_step(vrpn_float64, void *mptr);

extern void handle_step_go_to_pos(vrpn_int32, void *mptr);

extern void handle_keep_stepping( vrpn_int32, void* mptr );
//------------------------------------------------------------



//direct step variables
extern TclNet_float step_x;
extern TclNet_float step_y;
extern TclNet_float step_z;


extern double z_pos;
extern TclNet_int xy_lock;

void set_axis(q_type rot);

extern void directStep_mainloop( );

#endif //DIRECTSTEP_H

