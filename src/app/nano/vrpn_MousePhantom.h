#ifndef VRPN_MOUSEPHANTOM_H
#define VRPN_MOUSEPHANTOM_H
/*===3rdtech===
  Copyright (c) 2000 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/

#include "vrpn_Button.h"
#include "vrpn_Tracker.h"
#include "vrpn_ForceDevice.h"

class vrpn_MousePhantom: public vrpn_Tracker,
					public vrpn_Button_Filter {
protected:
    static int d_press_x, d_press_y;
    static int d_move_x, d_move_y;
    static int d_ctrl_on, d_shift_on;
    static vrpn_float64 d_press_pos[3], d_press_quat[4]; 
    ///< Pose when button pressed
    
    /// Static GLUT callbacks need object pointer
    static vrpn_MousePhantom * d_instance;

    /// Protected constructor helps with singleton pattern.
    vrpn_MousePhantom(char *name, vrpn_Connection *c, float hz=1.0);
    float update_rate;
    struct timeval timestamp;
    
    virtual void get_report(void);
    
    
    // from vrpn_Tracker
    static int handle_update_rate_request (void *, vrpn_HANDLERPARAM);
    static int handle_resetOrigin_change_message(void * userdata,
                                                 vrpn_HANDLERPARAM p);

public:
    static vrpn_MousePhantom * createMousePhantom(char *name, 
                                          vrpn_Connection *c, float hz);
    ~vrpn_MousePhantom();
    virtual void mainloop(void);
    virtual void reset();
    void getPosition(double *vec, double *quat);
    virtual void print_report(void);
    void enable();
    void disable();

    static void my_glutMouseFunc(int button, int state,
                                int x, int y);
    static void my_glutMotionFunc(int x, int y);
    static void my_glutKeyboardFunc(unsigned char key, int x, int y);
    static void handle_mp_enabled_change(int, void *);

};

#endif




