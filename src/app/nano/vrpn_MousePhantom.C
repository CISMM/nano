/*===3rdtech===
  Copyright (c) 2000 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>

#include <quat.h>

#include <vrpn_Connection.h>
#include <vrpn_Button.h>
#include <vrpn_Tracker.h>
#include <vrpn_ForceDevice.h>
#include <vrpn_MousePhantom.h>

#ifdef V_GLUT
#include <GL/glut_UNC.h>
#endif

#include <Tcl_Linkvar.h>

//  #ifndef	_WIN32
//  #include <netinet/in.h>
//  #include <sys/ioctl.h>
//  #endif

static	unsigned long	duration(struct timeval t1, struct timeval t2)
{
	return (t1.tv_usec - t2.tv_usec) +
	       1000000L * (t1.tv_sec - t2.tv_sec);
}

// For a portable implementation, these params should be
// set in a VRPN message to this class, and the UI
// should be handled by the app that uses the class. 

// Globals for GUI interaction - used in get_report()
// Default value of 1.0 should be "normal"
Tclvar_float mp_rot_scale("mp_rot_scale", 1.0);
Tclvar_float mp_trans_scale("mp_trans_scale", 1.0);
Tclvar_int mp_enabled("mp_enabled", 1, vrpn_MousePhantom::handle_mp_enabled_change);

// Class static variables
int vrpn_MousePhantom::d_press_x= -1; 
int vrpn_MousePhantom::d_press_y = -1;
int vrpn_MousePhantom::d_move_x = -1; 
int vrpn_MousePhantom::d_move_y = -1;
int vrpn_MousePhantom::d_ctrl_on = 0;
int vrpn_MousePhantom::d_shift_on = 0;
vrpn_float64 vrpn_MousePhantom::d_press_pos[3] = { 0,0,0};
vrpn_float64 vrpn_MousePhantom::d_press_quat[4] = { 0,0,0,1};
vrpn_MousePhantom * vrpn_MousePhantom::d_instance = NULL;


// return the position of the phantom stylus relative to base
// coordinate system
void vrpn_MousePhantom::getPosition(double *vec, double *orient)
{
    for (int i = 0; i < 3; i++){
		vec[i] = pos[i];
		orient[i] = d_quat[i];
	}
	orient[3] = d_quat[3];

}
// static
vrpn_MousePhantom * vrpn_MousePhantom::createMousePhantom(char *name, vrpn_Connection *c, float hz)
{
    if(d_instance) {
        delete d_instance;
        d_instance = NULL;
    }
    d_instance = new vrpn_MousePhantom(name, c, hz);
    return d_instance;
}

vrpn_MousePhantom::vrpn_MousePhantom(char *name, vrpn_Connection *c, float hz)
		:vrpn_Tracker(name, c),vrpn_Button_Filter(name,c),
		 update_rate(hz) {
  num_buttons = 1;  // only has one button

  timestamp.tv_sec = 0;
  timestamp.tv_usec = 0;
  
#ifdef V_GLUT
    // Initialize mouse-like phantom
    glutMouseFunc(vrpn_MousePhantom::my_glutMouseFunc);
    glutMotionFunc(vrpn_MousePhantom::my_glutMotionFunc);
    glutKeyboardFunc(vrpn_MousePhantom::my_glutKeyboardFunc);
#endif
  //  status= TRACKER_RESETTING;
  gettimeofday(&timestamp,NULL);


  if (register_autodeleted_handler(reset_origin_m_id,
        handle_resetOrigin_change_message, this, vrpn_Tracker::d_sender_id)) {
                fprintf(stderr,"vrpn_MousePhantom:can't register handler\n");
                vrpn_Tracker::d_connection = NULL;
  }

  if (vrpn_Tracker::register_server_handlers()) {
    fprintf(stderr, "vrpn_MousePhantom: couldn't register xform request handlers\n");
  }
  if (register_autodeleted_handler
       (update_rate_id, handle_update_rate_request, this,
        vrpn_Tracker::d_sender_id)) {
                fprintf(stderr, "vrpn_MousePhantom:  "
                                "Can't register update-rate handler\n");
                vrpn_Tracker::d_connection = NULL;
  }
  d_instance = this;
}

vrpn_MousePhantom::~vrpn_MousePhantom() {
#ifdef V_GLUT
    glutMouseFunc(NULL);
    glutMotionFunc(NULL);
    glutKeyboardFunc(NULL);
#endif
    d_instance = NULL;
}

void vrpn_MousePhantom::print_report(void)
{ 
   printf("----------------------------------------------------\n");

   printf("Timestamp:%ld:%ld\n",timestamp.tv_sec, timestamp.tv_usec);
   printf("Pos      :%lf, %lf, %lf\n", pos[0],pos[1],pos[2]);
   printf("Quat     :%lf, %lf, %lf, %lf\n", d_quat[0],d_quat[1],d_quat[2],d_quat[3]);
}
void vrpn_MousePhantom::get_report(void)
{
	double x = 0,y =0,z=0;
  	double dt_vel = .10, dt_acc = .10;
        q_type temp_quat;

        // This is a "meters per pixel" translation factor.
	double trans_scale = mp_trans_scale/10000.0;
        // radians per pixel.
        double rot_scale = mp_rot_scale/200.0;
        // Move hand only if mouse button is pressed. 
        if ((d_press_x != -1)&&(d_move_x != -1)) {
            if (!d_ctrl_on) {
                // Incremental position change;
                pos[0] = d_press_pos[0] + (d_move_x - d_press_x)*trans_scale;
                // Y mouse motion affects either Y or Z hand motion
                if (!d_shift_on) {
                    // y is reversed because screen coords upside-down
                    pos[1] = d_press_pos[1] - (d_move_y - d_press_y)*trans_scale;
                } else {
                    // This sign means "pull towards, push away"
                    pos[2] = d_press_pos[2] + (d_move_y - d_press_y)*trans_scale;
                }
                //printf("P %d %d M %d %d\n",d_press_x, d_press_y,d_move_x,d_move_y);
            } else {
                // Rotate, instead of translate
                // x mouse,  rotate around y axis, angle in radians.
                if (!d_shift_on) {
                    q_make(temp_quat, 0,1,0, (d_move_x - d_press_x)*rot_scale);
                    q_mult(d_quat, temp_quat, d_press_quat);
                } else {
                    q_make(temp_quat, 0,0,1, -(d_move_x - d_press_x)*rot_scale);
                    q_mult(d_quat, temp_quat, d_press_quat);
                }
                // y mouse, rotate around x 
                    q_make(temp_quat, 1,0,0, (d_move_y - d_press_y)*rot_scale);
                    q_mult(d_quat, temp_quat, d_quat);
            }
        }

        // Left over from Phantom routine - probably should
        // figure out if we ever use it...
//          gstVector phantomVel = phantom->getVelocity();//mm/sec
//          phantomVel.getValue(x, y, z);
//          vel[0] = x/1000.0;      // convert from mm to m
//          vel[1] = y/1000.0;
//          vel[2] = z/1000.0;
//          gstVector phantomAcc = phantom->getAccel(); //mm/sec^2
//          phantomAcc.getValue(x,y,z);
//          acc[0] = x/1000.0;      // convert from mm to m
//          acc[1] = y/1000.0;
//          acc[2] = z/1000.0;

	// need to work out expression for acceleration quaternion

	//gstVector phantomAngAcc = phantom->getAngularAccel();//rad/sec^2
//          gstVector phantomAngVel = phantom->getAngularVelocity();//rad/sec
//          angVelNorm = phantomAngVel.norm();
//          phantomAngVel.normalize();

	// compute angular velocity quaternion
//          phantomAngVel.getValue(x,y,z);
        //q_make(v_quat, x, y, z, angVelNorm*dt_vel);
        // set v_quat = v_quat*d_quat
        //q_mult(v_quat, v_quat, d_quat);


//  	for(i=0;i<4;i++ ) {
//  		d_quat[i] = p_quat[i];
//  		vel_quat[i] = v_quat[i];
//  		scp_quat[i] = 0.0; // no torque with PHANToM
//  	}
//  	scp_quat[3] = 1.0;

//  	vel_quat_dt = dt_vel;
//printf("get report pos = %lf, %lf, %lf\n",pos[0],pos[1],pos[2]);
}



// mainloop:
//      send button message (done in vrpn_Button::report_changes)
//      get position from mouse (done in vrpn_MousePhantom::get_report())
//      send position message
//      send force message

void vrpn_MousePhantom::mainloop(void) {
    struct timeval current_time;
    char	msgbuf[1000];
    //	char    *buf;
    vrpn_int32	len;
    
    // Allow the base server class to do its thing
    server_mainloop();
    
    // No reports if not enabled
    if (!mp_enabled) return;
    
    //check button status done in GLUT button handler.
    
    // if button event happens, report changes
    report_changes();
    
    //set if its time to generate a new report 
    gettimeofday(&current_time, NULL);
    if(duration(current_time,timestamp) >= 1000000.0/update_rate) {
		
        //update the time
        timestamp.tv_sec = current_time.tv_sec;
        timestamp.tv_usec = current_time.tv_usec;

        // Read the current PHANToM position, velocity, accel., and force
        // Put the information into pos/quat
        get_report();

        //Encode the position/orientation if there is a connection
        if(vrpn_Tracker::d_connection) {
            len = vrpn_Tracker::encode_to(msgbuf);
            if(vrpn_Tracker::d_connection->pack_message(len,
                timestamp,vrpn_Tracker::position_m_id,
                vrpn_Tracker::d_sender_id, msgbuf, vrpn_CONNECTION_LOW_LATENCY)) {
                    fprintf(stderr,"Phantom: cannot write message: tossing\n");
            }
        }

        //Encode the velocity/angular velocity if there is a connection
        if(vrpn_Tracker::d_connection) {
            len = vrpn_Tracker::encode_vel_to(msgbuf);
            if(vrpn_Tracker::d_connection->pack_message(len,
                timestamp,vrpn_Tracker::velocity_m_id,
                vrpn_Tracker::d_sender_id, msgbuf,vrpn_CONNECTION_LOW_LATENCY)) {
                    fprintf(stderr,"Phantom: cannot write message: tossing\n");
            }
        }
        //Encode the acceleration/angular acceleration if there is a connection
/*        if (vrpn_Tracker::d_connection) {
            len = vrpn_Tracker::encode_acc_to(msgbuf);
            if(vrpn_Tracker::d_connection->pack_message(len,
                timestamp,vrpn_Tracker::acc_m_id,
                vrpn_Tracker::d_sender_id, msgbuf,vrpn_CONNECTION_LOW_LATENCY)) {
                    fprintf(stderr,"Phantom: cannot write message: tossing\n");
            }
        }
*/
  //      print_report();
    }
}

void vrpn_MousePhantom::reset(){
    pos[0] = pos[1] = pos[2] =0;
    d_quat[0] = d_quat[1] = d_quat[2] = 0;
    d_quat[3] = 1;
    return;
}

void vrpn_MousePhantom::enable () 
{
    mp_enabled = 1;
    handle_mp_enabled_change(mp_enabled, NULL);
}
void vrpn_MousePhantom::disable () 
{
    mp_enabled = 0;
    handle_mp_enabled_change(mp_enabled, NULL);
}

//static
void vrpn_MousePhantom::handle_mp_enabled_change(int, void *) 
{
#ifdef V_GLUT
    if (mp_enabled) {
        glutMouseFunc(vrpn_MousePhantom::my_glutMouseFunc);
        glutMotionFunc(vrpn_MousePhantom::my_glutMotionFunc);
        glutKeyboardFunc(vrpn_MousePhantom::my_glutKeyboardFunc);
    } else {        
        glutMouseFunc(NULL);
        glutMotionFunc(NULL);
        glutKeyboardFunc(NULL);
    }
#endif
}

// static
int vrpn_MousePhantom::handle_resetOrigin_change_message(void * userdata,
                                                vrpn_HANDLERPARAM p) {
  vrpn_MousePhantom * me = (vrpn_MousePhantom *) userdata;
  me->reset();
  return 0;

}


// static
int vrpn_MousePhantom::handle_update_rate_request (void * userdata,
                                                  vrpn_HANDLERPARAM p) {
  vrpn_MousePhantom * me = (vrpn_MousePhantom *) userdata;

  if (p.payload_len != sizeof(double)) {
    fprintf(stderr, "vrpn_MousePhantom::handle_update_rate_request:  "
                    "Got %d-byte message, expected %d bytes.\n",
            p.payload_len, sizeof(double));
    return -1;  // XXXshould this be a fatal error?
  }

  me->update_rate = ntohd(*((double *) p.buffer));
  fprintf(stderr, "vrpn_MousePhantom:  set update rate to %.2f hertz.\n",
          me->update_rate);
  return 0;
}

//static 
void vrpn_MousePhantom::my_glutMouseFunc(int btn, int state, int x, int y)
{
#ifdef V_GLUT
    // If either left or right is pressed...
    if (btn !=GLUT_MIDDLE_BUTTON) {
        // If right is pressed, act like Phantom button pressed, too. 
        if (btn ==GLUT_RIGHT_BUTTON) {
            if (state == GLUT_DOWN) {
                d_instance->buttons[0] = 1;
            } else if (state == GLUT_UP) {
                d_instance->buttons[0] = 0;
            }   
        }
        // Move the hand icon for either button.  
        if (state == GLUT_DOWN) {
            // Would normally work when called by GLUT, but
            // doesn't work when called by my_glutMotionFunc below.
//              int modifiers = glutGetModifiers();
//              d_ctrl_on = (modifiers & GLUT_ACTIVE_CTRL) != 0;
//              d_shift_on = (modifiers & GLUT_ACTIVE_SHIFT) != 0;
#ifdef _WIN32
            // Windows functions, the same one GLUT calls for _WIN32
            d_ctrl_on = (GetKeyState(VK_CONTROL) < 0);
            d_shift_on = (GetKeyState(VK_SHIFT) < 0);
#endif
//              printf("GLUT mod %d, ctrl %d, shift %d\n", 
//                     modifiers, d_ctrl_on,d_shift_on);
            d_press_x = x;
            d_press_y = y;
            // The mouse hasn't moved yet, so note that. 
            d_move_x = -1;
            d_move_y = -1;
            // Record current phantom position and orientation
            d_press_pos[0] = d_instance->pos[0];
            d_press_pos[1] = d_instance->pos[1];
            d_press_pos[2] = d_instance->pos[2];
            d_press_quat[0] = d_instance->d_quat[0];
            d_press_quat[1] = d_instance->d_quat[1];
            d_press_quat[2] = d_instance->d_quat[2];
            d_press_quat[3] = d_instance->d_quat[3];
        } else if (state == GLUT_UP) {
            // button release
            d_press_x = -1;
            d_press_y = -1;
        }
    }
#endif
}

//static 
void vrpn_MousePhantom::my_glutMotionFunc(int x, int y)
{
    // Can't do this in this callback, GLUT doesn't allow it. 
//      int modifiers;
//      modifiers = glutGetModifiers();
//      d_ctrl_on = (modifiers & GLUT_ACTIVE_CTRL) != 0;
//      d_shift_on = (modifiers & GLUT_ACTIVE_SHIFT) != 0;
#ifdef V_GLUT
#ifdef _WIN32
    // Windows functions, the same one GLUT calls for _WIN32
    int ctrl_on = (GetKeyState(VK_CONTROL) < 0);
    int shift_on = (GetKeyState(VK_SHIFT) < 0);
    //    int alt_on = (GetKeyState(VK_MENU) < 0);
    if ((ctrl_on != d_ctrl_on) || (shift_on != d_shift_on)) {
        my_glutMouseFunc( GLUT_LEFT_BUTTON, GLUT_UP, x, y);
        my_glutMouseFunc( GLUT_LEFT_BUTTON, GLUT_DOWN, x, y);
    }
#endif
#endif
    d_move_x = x;
    d_move_y = y;
}


// Argh. This doesn't work - not called when just shift or control is pressed.
//static 
void vrpn_MousePhantom::my_glutKeyboardFunc(unsigned char key, int x, int y)
{
#ifdef V_GLUT
    // If the user changes the modifiers, treat it like a press and release
    // of the left mouse button. 
//      printf("k\n");
//      int modifiers = glutGetModifiers();
//      int ctrl_on = (modifiers & GLUT_ACTIVE_CTRL) != 0;
//      int shift_on = (modifiers & GLUT_ACTIVE_SHIFT) != 0;
//      if ((ctrl_on != d_ctrl_on) || (shift_on != d_shift_on)) {
//          my_glutMouseFunc( GLUT_LEFT_BUTTON, GLUT_UP, x, y);
//          my_glutMouseFunc( GLUT_LEFT_BUTTON, GLUT_DOWN, x, y);
//      }
#endif
}
