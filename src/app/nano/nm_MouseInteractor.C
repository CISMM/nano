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
#include <nm_MouseInteractor.h>

#ifdef V_GLUT
#include <GL/glut_UNC.h>
#endif

#include <Tcl_Linkvar.h>
#include <Tcl_Netvar.h>
#include "interaction.h"

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
Tclvar_float mi_rot_scale("mi_rot_scale", 1.0);
Tclvar_float mi_trans_scale("mi_trans_scale", 1.0);
Tclvar_int mi_enabled("mi_enabled", 1, nm_MouseInteractor::handle_mi_enabled_change);

// Class static variables
int nm_MouseInteractor::d_last_move_x= -1; 
int nm_MouseInteractor::d_last_move_y = -1;
int nm_MouseInteractor::d_move_x = -1; 
int nm_MouseInteractor::d_move_y = -1;
int nm_MouseInteractor::d_ctrl_on = 0;
int nm_MouseInteractor::d_shift_on = 0;
vrpn_float64 nm_MouseInteractor::d_press_pos[3] = { 0,0,0};
vrpn_float64 nm_MouseInteractor::d_press_quat[4] = { 0,0,0,1};
nm_MouseInteractor * nm_MouseInteractor::d_instance = NULL;


// return the position of the phantom stylus relative to base
// coordinate system
void nm_MouseInteractor::getPosition(double *vec, double *orient)
{
    for (int i = 0; i < 3; i++){
		vec[i] = pos[i];
		orient[i] = d_quat[i];
	}
	orient[3] = d_quat[3];

}
// static
nm_MouseInteractor * nm_MouseInteractor::createMouseInteractor(char *name, vrpn_Connection *c, float hz)
{
    if(d_instance) {
        delete d_instance;
        d_instance = NULL;
    }
    d_instance = new nm_MouseInteractor(name, c, hz);
    return d_instance;
}

nm_MouseInteractor::nm_MouseInteractor(char *name, vrpn_Connection *c, float hz)
		:vrpn_Tracker(name, c),vrpn_Button_Filter(name,c),
		 update_rate(hz) {
  num_buttons = 1;  // only has one button

  timestamp.tv_sec = 0;
  timestamp.tv_usec = 0;
  
#ifdef V_GLUT
    // Initialize mouse-like phantom
    glutMouseFunc(nm_MouseInteractor::my_glutMouseFunc);
    glutMotionFunc(nm_MouseInteractor::my_glutMotionFunc);
    glutKeyboardFunc(nm_MouseInteractor::my_glutKeyboardFunc);
#endif
  //  status= TRACKER_RESETTING;
  gettimeofday(&timestamp,NULL);


  if (register_autodeleted_handler(reset_origin_m_id,
        handle_resetOrigin_change_message, this, vrpn_Tracker::d_sender_id)) {
                fprintf(stderr,"nm_MouseInteractor: can't register handler\n");
                vrpn_Tracker::d_connection = NULL;
  }

  if (vrpn_Tracker::register_server_handlers()) {
    fprintf(stderr, "nm_MouseInteractor: couldn't register xform request handlers\n");
  }
  if (register_autodeleted_handler
       (update_rate_id, handle_update_rate_request, this,
        vrpn_Tracker::d_sender_id)) {
                fprintf(stderr, "nm_MouseInteractor: "
                                "Can't register update-rate handler\n");
                vrpn_Tracker::d_connection = NULL;
  }
  d_instance = this;
}

nm_MouseInteractor::~nm_MouseInteractor() {
#ifdef V_GLUT
    glutMouseFunc(NULL);
    glutMotionFunc(NULL);
    glutKeyboardFunc(NULL);
#endif
    d_instance = NULL;
}

void nm_MouseInteractor::print_report(void)
{ 
   printf("----------------------------------------------------\n");

   printf("Timestamp:%ld:%ld\n",timestamp.tv_sec, timestamp.tv_usec);
   printf("Pos      :%lf, %lf, %lf\n", pos[0],pos[1],pos[2]);
   printf("Quat     :%lf, %lf, %lf, %lf\n", d_quat[0],d_quat[1],d_quat[2],d_quat[3]);
   q_vec_type angles;
   q_to_euler(angles, d_quat);
   printf("Euler    :%lf, %lf, %lf\n", angles[0],angles[1],angles[2]);

}
void nm_MouseInteractor::get_report(void)
{
    q_type temp_quat;
    int motion_mode; // 0 = translate, 1 = rotate, 
    // 2 = absolute orbit (light), 3 = shift inversion, position (scale).

    // This is a "meters per pixel" translation factor.
    double trans_scale = mi_trans_scale/10000.0;
    // radians per pixel.
    double rot_scale = mi_rot_scale/200.0;

    // Move hand only if mouse button is pressed. 
    if ((d_last_move_x != -1)&&(d_move_x != -1)) {
        switch(user_0_mode) {
        case USER_GRAB_MODE:
        case USER_LIGHT_MODE:
            if (d_ctrl_on) 
                motion_mode = 0;
            else 
                motion_mode = 1;
            break;
            //motion_mode = 2;
            //break;
        case USER_SCALE_UP_MODE:
            motion_mode = 3;
            break;
        default:
            if (d_ctrl_on) 
                motion_mode = 1;
            else 
                motion_mode = 0;
            break;
        }
        switch (motion_mode) {
        case 0:
            // position change from press position;
            pos[0] = d_press_pos[0] + (d_move_x - d_last_move_x)*trans_scale;
            // Y mouse motion affects either Y or Z hand motion
            if (!d_shift_on) {
                // y is reversed because screen coords upside-down
                pos[1] = d_press_pos[1] - (d_move_y - d_last_move_y)*trans_scale;
            } else {
                // This sign means "pull towards, push away"
                pos[2] = d_press_pos[2] + (d_move_y - d_last_move_y)*trans_scale;
            }
            break;
        case 1:
            // Rotate, instead of translate
            // x mouse,  rotate around y (or z) axis, angle in radians.
            if (!d_shift_on) {
                q_make(temp_quat, 0,1,0, (d_move_x - d_last_move_x)*rot_scale);
                q_mult(d_quat, temp_quat, d_press_quat);
            } else {
                q_make(temp_quat, 0,0,1, -(d_move_x - d_last_move_x)*rot_scale);
                q_mult(d_quat, temp_quat, d_press_quat);
            }
            // y mouse, rotate around x 
            q_make(temp_quat, 1,0,0, (d_move_y - d_last_move_y)*rot_scale);
            q_mult(d_quat, temp_quat, d_quat);
            break;
        case 2:
            {
            q_vec_type x_axis, new_x_axis, y_axis, z_axis;
            q_type x_axis_rot;
            // Remove y-axis rotation first, it has undesired side-effects.
//              q_to_euler(angles, d_quat);
//              angles[0] += -(d_move_x - d_last_move_x)*rot_scale;
//              angles[2] += (d_move_y - d_last_move_y)*rot_scale;
//              angles[1] = 0;
//              q_from_euler(d_quat, angles[0],angles[1],angles[2]);

             //q_vec_set(y_axis, 0, 1, 0);
            //q_xform(y_axis, d_press_quat, y_axis);
            //q_invert(y_axis

            // We want to rotate about the z axis first, but it's relative
            // to the  previous quat.
            q_vec_set(z_axis, 0,0,1);
            q_xform(z_axis, d_press_quat, z_axis);
            q_vec_set(x_axis, 1, 0, 0);
            q_xform(x_axis, d_press_quat, x_axis);

            q_make(temp_quat, z_axis[0],z_axis[1],z_axis[2], -(d_move_x - d_last_move_x)*rot_scale);
            q_mult(d_quat, temp_quat, d_press_quat);

            // Now about the x axis, again relative. 

            q_make(temp_quat, x_axis[0],x_axis[1],x_axis[2], (d_move_y - d_last_move_y)*rot_scale);
            q_mult(d_quat, temp_quat, d_quat);
          
            // So our x and z axis rotations always correspond to x and y
            // mouse motions, we want the x-axis to remain horizontal,
            // always. So the x-axis transformed by our new quat dotted with
            // the absolute y-axis of the tracker should be cos (90 deg) = 0,
            // i.e. the Y component is zero.  AND if the light is on the
            // front, the x-axis should point right.
            q_vec_set(x_axis, 1, 0, 0);
            q_xform(x_axis, d_quat, x_axis);
            q_vec_set(y_axis, 0,1,0);
            q_xform(y_axis, d_quat, y_axis);
            q_vec_set(z_axis, 0,0,1);
            q_xform(z_axis, d_quat, z_axis);

            q_copy(new_x_axis, x_axis);
            new_x_axis[1] = 0;
            // Light is on the front if y_axis dot (0,0,1) is positive. 
            // Shortcut for dot product with (0,0,1) - look at z
//              if ((y_axis[2] > 0 && x_axis[0] < 0 ) ||
//                  (y_axis[2] < 0 && x_axis[0] > 0 )) {
//                  q_vec_invert(new_x_axis, new_x_axis);
//              } 
            // Find quat to rotate old to new. 
            q_from_two_vecs(x_axis_rot, x_axis, new_x_axis);
            // Add this rotation into our result quat. 
            q_mult(d_quat, x_axis_rot, d_quat);
            }
            break;
        case 3:
            // Incremental position change;
            pos[0] = d_press_pos[0] + (d_move_x - d_last_move_x)*trans_scale;
            // Y mouse motion affects Y or Z hand motion, reversed from above.
            if (d_shift_on) {
                // y is reversed because screen coords upside-down
                pos[1] = d_press_pos[1] - (d_move_y - d_last_move_y)*trans_scale;
            } else {
                // This sign means "pull towards, push away"
                pos[2] = d_press_pos[2] + (d_move_y - d_last_move_y)*trans_scale;
            }
            break;
        }
    }
    if (user_0_mode == USER_MEASURE_MODE) {
        // Clamp the position to be on the surface
//          q_vec_type clipPos;
//          BCPlane* plane = dataset->inputGrid->getPlaneByName
//                       (dataset->heightPlaneName->string());
//          if (plane == NULL)
//          {
//              fprintf(stderr, "Error in doMeasure: could not get plane!\n");
//              return -1;
//          }
//          // Set its height based on data at this point
//          nmui_Util::getHandInWorld(whichUser, clipPos);
//          nmui_Util::clipPosition(plane, clipPos);
//          clipPos[2] = plane->valueAt(clipPos[0], clipPos[1]);

//          v_xform_type worldFromHand;
//          v_get_world_from_hand(user, &worldFromHand);
//          q_vec_copy(position, worldFromHand.xlate);
        
    }
    // Only for incremental, not if we calc from press position. 
//      d_last_move_x = d_move_x;
//      d_last_move_y = d_move_y;
    
}



// mainloop:
//      send button message (done in vrpn_Button::report_changes)
//      get position from mouse (done in nm_MouseInteractor::get_report())
//      send position message
//      send force message
void nm_MouseInteractor::mainloop(void) {
    struct timeval current_time;
    char	msgbuf[1000];
    //	char    *buf;
    vrpn_int32	len;
    
    // Allow the base server class to do its thing
    server_mainloop();
    
    // No reports if not enabled
    if (!mi_enabled) return;
    
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

//          //Encode the velocity/angular velocity if there is a connection
//          if(vrpn_Tracker::d_connection) {
//              len = vrpn_Tracker::encode_vel_to(msgbuf);
//              if(vrpn_Tracker::d_connection->pack_message(len,
//                  timestamp,vrpn_Tracker::velocity_m_id,
//                  vrpn_Tracker::d_sender_id, msgbuf,vrpn_CONNECTION_LOW_LATENCY)) {
//                      fprintf(stderr,"Phantom: cannot write message: tossing\n");
//              }
//          }
    }
}

void nm_MouseInteractor::reset(){
    pos[0] = pos[1] = pos[2] =0;
    d_quat[0] = d_quat[1] = d_quat[2] = 0;
    d_quat[3] = 1;
    return;
}

void nm_MouseInteractor::enable () 
{
    mi_enabled = 1;
    handle_mi_enabled_change(mi_enabled, NULL);
}
void nm_MouseInteractor::disable () 
{
    mi_enabled = 0;
    handle_mi_enabled_change(mi_enabled, NULL);
}

//static
void nm_MouseInteractor::handle_mi_enabled_change(int, void *) 
{
#ifdef V_GLUT
    if (mi_enabled) {
        glutMouseFunc(nm_MouseInteractor::my_glutMouseFunc);
        glutMotionFunc(nm_MouseInteractor::my_glutMotionFunc);
        glutKeyboardFunc(nm_MouseInteractor::my_glutKeyboardFunc);
    } else {        
        glutMouseFunc(NULL);
        glutMotionFunc(NULL);
        glutKeyboardFunc(NULL);
    }
#endif
}

// static
int nm_MouseInteractor::
handle_resetOrigin_change_message(void * userdata, vrpn_HANDLERPARAM /*p*/) {
  nm_MouseInteractor * me = (nm_MouseInteractor *) userdata;
  me->reset();
  return 0;

}


// static
int nm_MouseInteractor::
handle_update_rate_request (void * userdata, vrpn_HANDLERPARAM p) {
  nm_MouseInteractor * me = (nm_MouseInteractor *) userdata;

  if (p.payload_len != sizeof(double)) {
    fprintf(stderr, "nm_MouseInteractor::handle_update_rate_request:  "
                    "Got %d-byte message, expected %d bytes.\n",
            p.payload_len, sizeof(double));
    return -1;  // XXXshould this be a fatal error?
  }

  me->update_rate = ntohd(*((double *) p.buffer));
//    fprintf(stderr, "nm_MouseInteractor:  set update rate to %.2f hertz.\n",
//            me->update_rate);
  return 0;
}

//static 
void nm_MouseInteractor::my_glutMouseFunc(int btn, int state, int x, int y)
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
            // Tried to use GLUT built-in functions, couldn't. 
#ifdef _WIN32
            // Windows functions, the same one GLUT calls for _WIN32
            d_ctrl_on = (GetKeyState(VK_CONTROL) < 0);
            d_shift_on = (GetKeyState(VK_SHIFT) < 0);
#endif
            d_last_move_x = x;
            d_last_move_y = y;
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
            //d_instance->print_report();
        } else if (state == GLUT_UP) {
            // button release
            d_last_move_x = -1;
            d_last_move_y = -1;
            //d_instance->print_report();
        }
    }
#endif
}

//static 
void nm_MouseInteractor::my_glutMotionFunc(int x, int y)
{
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
void nm_MouseInteractor::my_glutKeyboardFunc(unsigned char /*key*/, int /*x*/, int /*y*/)
{
#ifdef V_GLUT

#endif
}
