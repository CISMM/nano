/*===3rdtech===
  Copyright (c) 2000 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
/**
 * @file minit.c
 * This file is really Devices.C, now. It handles the initialization 
 * and callbacks for external input devices for the NanoManipulator. 
 */
#include <stdio.h>
#include <string.h>
#if !defined (_WIN32) || defined (__CYGWIN__)
#include <strings.h>            // bzero(), bcopy()
#endif
#include <stdlib.h>
#include <math.h>

// socket includes
#ifndef _WIN32
#include <netinet/in.h>
#include <sys/time.h>	// time before types
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>              // gethostbyname() 
#include <arpa/inet.h> // inet_addr()
/*#include "/usr/include/arpa/inet.h" // inet_addr() */ /* NEVER EVER use absolute paths in #include! */
#include <unistd.h>             // gethostname() 
#include <errno.h>
#endif

#include <vrpn_Types.h>
#include <vrpn_Tracker.h>
#ifndef NO_MAGELLAN
#include <vrpn_Magellan.h>
#include <vrpn_Tracker_AnalogFly.h>
#endif

#include "stm_file.h"
#include "stm_cmd.h"
#include <Tcl_Linkvar.h>
#include <Tcl_Netvar.h>

#include <PPM.h>
#include <nmb_PlaneSelection.h>
#include <nmb_Decoration.h>
#include <nmb_Globals.h>

#include <nmm_MicroscopeRemote.h>

#include <nmm_Globals.h>

//  #include <nmg_Globals.h>
//  #include <nmg_Graphics.h>


#include "x_util.h" /* qliu */
#include "relax.h"
#include "microscape.h"  // for #defines
#include "butt_mode.h"   // for button defines
#include "interaction.h"
// Must be included after Tcl headers or "list" produces a conflict. 
#ifndef NO_PHANTOM_SERVER
#include <vrpn_Phantom.h>
#endif

// MOVED #ifdef FLOW includes and declarations to graphics.C

int x_init(char* argv[]);
//  int stm_init (const vrpn_bool, const vrpn_bool, const vrpn_bool,
//                const int, const char *, const int, const int);
//  extern "C" void txt_init(int);

extern int handle_phantom_reconnect (void *, vrpn_HANDLERPARAM);


// Added by Michele Clark 6/2/97
unsigned long inet_addr();


#ifdef FLOW
  extern int sdi_start_server(char *, char *, char *);
  extern int sdi_connect_to_device(char *);
#else
  #ifdef __cplusplus
     extern "C" int sdi_start_server(char *, char *, char *);
     extern "C" int sdi_connect_to_device(char *);
  #endif
#endif

#define NANO_FONT	(34)

const char MAGELLAN_NAME[] = "Magellan0";
// * in front means "use the connection I give you" to the AnalogFly
char AF_MAGELLAN_NAME[] = "*Magellan0";
static vrpn_Tracker_AnalogFlyParam *magellan_param;

/*****************************************************************************

  x_init - init x graphics display (qliu 7/11/95) 

******************************************************************************/
int 
x_init(char* argv[])
{
#ifndef NO_XWINDOWS
    if (createWindow(0,argv)) 
	return(-1);
    clearWindow();
    if (createAndInstallGrayMap() != 1) {
	fprintf(stderr,"x_init(): Error in createAndInstallGrayMap()\n");
	return(-1);
    }

    // TCH 7 May 98
    // Decouple microscope from X windows by rendering newly added marks
    // with a callback instead of a direct call to put_dot
    decoration->registerNewScrapeCallback(x_draw_marker, NULL);
    decoration->registerNewPulseCallback(x_draw_marker, NULL);

#endif
    return(0);
}

/*********
 * callback for vrpn_Button_Remote for phantom stylus button (registered below)
   there is only one button so we just put the state into the state variable
  *********/
void handle_phant_button_change(void *userdata, const vrpn_BUTTONCB b){
   *(int *)userdata = b.state;
}

int handle_phantom_conn_dropped(void * /*userdata*/, vrpn_HANDLERPARAM /*p*/){
    //fprintf(stderr, "ERROR: connection to PHANToM has been dropped\n");
   //vrpn_Connection *c = (vrpn_Connection *)userdata;
   return 0;
}

void handle_phantom_error(void * /*userdata*/, const vrpn_FORCEERRORCB ferr){
   fprintf(stderr, "Error: phantom server has failed: ");
   switch(ferr.error_code){
      case FD_VALUE_OUT_OF_RANGE:
	fprintf(stderr, "Parameter out of range\n");
	break;
      case FD_DUTY_CYCLE_ERROR:
	fprintf(stderr, "Duty cycle taking too long\n");
	break;
      case FD_FORCE_ERROR:
	fprintf(stderr, "Exceeded max. force or velocity or temperature\n");
	break;
      default:
	fprintf(stderr, "Unknown error\n");
	break;
   }
}

/** handler for the tclvar_int phantom_button_mode.
 * if 0, set the phantom button to be a press and hold - momentary
 * if 1, set the phantom to be click on, click off - toggle
 * If 2, means use button box as phantom button, (handled in interaction.c)
 * The current phantom button state is passed in, so we can turn the
 * toggle on if the phantom button is currently held down. 
 */
void handle_phantom_button_mode_change( vrpn_int32, void * userdata) 
{
    int phant_button_state = *(int *)userdata;

    if (phantom_button_mode == 0) {
        phantButton->set_all_momentary();
    } else if (phantom_button_mode == 1) {
        if (phant_button_state) {
            phantButton->set_all_toggle(vrpn_BUTTON_TOGGLE_ON);
        } else {
            phantButton->set_all_toggle(vrpn_BUTTON_TOGGLE_OFF);
        }
    }
}

/** callbacks for vrpn_Button_Remote and vrpn_Analog_Remote for sgi button/dial
   box (registered below)
 */
void handle_bdbox_button_change(void *userdata, const vrpn_BUTTONCB b){
   int *bdboxButtonStates = (int *)userdata;
   bdboxButtonStates[b.button] = b.state;
}

void handle_bdbox_dial_change(void *userdata, const vrpn_ANALOGCB info){
   double *bdboxDialValues = (double *)userdata;
   for (int i = 0; i < info.num_channel; i++){
   	bdboxDialValues[i] = info.channel[i];
   }
}

#ifndef NO_MAGELLAN
/** callbacks for vrpn_Button_Remote for magellan button box (registered below)
 */
void handle_magellan_button_change(void *userdata, const vrpn_BUTTONCB b){
   int * magellanButtonStates = (int *)userdata;
   magellanButtonStates[b.button] = b.state;
   //printf("Magellan button %d -> %d\n",  b.button, b.state);
   // if it's a button release event, ignore it. 
   if (b.state == 0) return;
   switch (b.button) {
   case 0:
       // This is the * button, we use it to turn on and off the puck
       magellanPuckActive = !magellanPuckActive;
       break;
   case 1: 		/* Grab World mode */
       mode_change = 1;	/* Will make icon change */
       user_mode[0] = USER_GRAB_MODE;
       break;
   case 2:
       mode_change = 1;	/* Will make icon change */
       // Cycling button - cycles through Light, Measure, Scale up, Scale down.
       if (user_mode[0] == USER_LIGHT_MODE) {
           user_mode[0] = USER_MEASURE_MODE;
       } else if (user_mode[0] == USER_MEASURE_MODE) {
           user_mode[0] = USER_SCALE_UP_MODE;
       } else if (user_mode[0] == USER_SCALE_UP_MODE) {
           user_mode[0] = USER_SCALE_DOWN_MODE;
       } else {
           user_mode[0] = USER_LIGHT_MODE;
       }
       break;
       
   case 3: 		
       mode_change = 1;	
       // Cycling button -cycles through Touch Live and Select
       if (user_mode[0] == USER_PLANEL_MODE) {
           user_mode[0] = USER_SERVO_MODE;
       } else {
           user_mode[0] = USER_PLANEL_MODE;
       }
       break;
   case 4:              /* Commit button */
       tcl_commit_pressed = 1;
      // we must call "handle_commit_change" explicitly, 
      // because tclvars are set to ignore changes from C.
      handle_commit_change(tcl_commit_pressed, NULL);
       break;
   case 5:              /* Center the surface */
       center();
       break;
   case 6: 		/* Touch Stored mode */
       mode_change = 1;	
       user_mode[0] = USER_PLANE_MODE;
       break;
   case 7: 		/* XY lock */
       xy_lock = !xy_lock;
       break;
   case 8:              /* Cancel button */
       tcl_commit_canceled = 1;
      // we must call "handle_commit_cancel" explicitly, 
      // because tclvars are set to ignore changes from C.
      handle_commit_cancel(tcl_commit_canceled, NULL);
       break;
   }


}
/** callbacks for vrpn_Tracker_Remote for magellan (registered below)
 */
void handle_magellan_puck_change(void *userdata, const vrpn_TRACKERCB tr) 
{
   vrpn_bool puck_active = VRPN_FALSE;
   static vrpn_bool old_puck_active = VRPN_FALSE;
   static q_xyz_quat_type old_puck_transform;
   q_xyz_quat_type puck_transform, puck_diff_transform;

   static v_xform_type old_world_from_room;
   v_xform_type curr_world_from_room, new_world_from_room, puck_rotate_xform,
       curr_room_from_world;

   static float offsetx, offsety, offsetz;
   q_vec_type plane_center;

   double trans_scale = 1.0;

   if (userdata) puck_active = *((vrpn_bool *)userdata);

   switch (user_mode[0]) {
   case USER_GRAB_MODE:
   case USER_LIGHT_MODE:
   case USER_PLANE_MODE:
   case USER_SCALE_UP_MODE:
   case USER_SCALE_DOWN_MODE:
       if (puck_active) {
           if (!old_puck_active) {
              // This "activates" the puck, sets up later interaction.  Save
               // the current puck transforms, so we can do diffs with them.
               q_vec_scale( old_puck_transform.xyz, trans_scale, 
                            (double *)tr.pos);
               q_copy ( old_puck_transform.quat, (double *)tr.quat);
               q_xyz_quat_invert( &old_puck_transform, &old_puck_transform);

               // Save the center of the plane, basis for rotations. 
               get_Plane_Extents(&offsetx,&offsety,&offsetz);
//                 printf("offsets %g %g %g\n", offsetx, offsety, offsetz);
//                 printf("old wfr ");
//                 v_x_print( &old_world_from_room);
           } else {
               // Save the old world_from_room transform as a basis for puck
               // action. Move to activate section above if we aren't doing
               // incremental xforms.
               v_get_world_from_head(0, &old_world_from_room);

               // We are active, and changing the surface position
               v_init_xform(&new_world_from_room);
               q_vec_scale( puck_transform.xyz, trans_scale, 
                           (double *)tr.pos);
               q_copy ( puck_transform.quat, (double *)tr.quat);

               // old_puck_transform is inverted so when we compose it
               // below, it "undoes" the starting transform, to get diff. 
               q_xyz_quat_compose(&puck_diff_transform, 
                                  &old_puck_transform, &puck_transform);

               v_xform_type vxtemp;
               // Identity transforms
               q_type q_ident = Q_ID_QUAT;
               q_vec_type v_ident = Q_NULL_VECTOR;

               // ROTATE the surface, same as the puck.
               // Make rotation happen about center of the plane. 
               q_vec_set(plane_center, offsetx, 
                         offsety, 
                         offsetz);

               // Plane center is in world coords, we need it in room coords
               v_get_world_from_head(0, &curr_world_from_room);
               v_x_invert(&curr_room_from_world, &curr_world_from_room);
               
               // xform plane_center vector into room coords. 
               v_x_xform_vector(plane_center, &curr_room_from_world, plane_center);

//                 v_x_print(&curr_world_from_room);
//                 q_vec_print(plane_center);

               // translate plane center to zero, rotate
               v_x_set(&puck_rotate_xform, plane_center,
                       puck_diff_transform.quat, 1.0);
               q_vec_invert(plane_center, plane_center);
               v_x_set(&vxtemp, plane_center, q_ident, 1.0);
               // translate plane center back to proper position
               v_x_compose(&puck_rotate_xform, &puck_rotate_xform, 
                           &vxtemp);

//                 v_x_print(&puck_rotate_xform);
               
               v_x_compose(&new_world_from_room, 
                           &old_world_from_room, &puck_rotate_xform);

               
               // TRANSLATE the surface based on puck translation. 
               // Inversion necessary for natural movement. Don't know why. 
               q_vec_invert(puck_diff_transform.xyz, puck_diff_transform.xyz);
               v_x_set(&vxtemp, puck_diff_transform.xyz, q_ident, 1.0);
               v_x_compose(&new_world_from_room, &new_world_from_room, &vxtemp);
//                 v_x_print(&new_world_from_room);
               
               // May be necessary because of instability for large 
               // rotations
//                 q_normalize(new_world_from_room.rotate, 
//                             new_world_from_room.rotate);

               updateWorldFromRoom(&new_world_from_room);
//                 printf ("\n");

               // INCREMENTAL transform. If we do this, transforms are
               // updated frame-to-frame. If we don't, they are update
               // based on puck position when we activate the puck. 
               // We seem to need this for stability. Non-incremental 
               // transforms explode when we get near 360 rotation. 
               q_vec_scale( old_puck_transform.xyz, trans_scale, 
                            (double *)tr.pos);
               q_copy ( old_puck_transform.quat, (double *)tr.quat);
               q_xyz_quat_invert( &old_puck_transform, &old_puck_transform);
           }
       }
       break;
   default:
       break;
   }
   old_puck_active = puck_active;
}
#endif

// This will reset the transforms on the Premium model, 
// and shouldn't do any harm with a Desktop model. 
int
reset_phantom()
{
  if(vrpnHandTracker[0]!=NULL) {
      vrpnHandTracker[0]->reset_origin();
      vrpnHandTracker[0]->request_t2r_xform();
      vrpnHandTracker[0]->request_u2s_xform();
  }
  return 0;
}

int
phantom_init(vrpn_Connection * local_device_connection)
{
    if (strcmp(handTrackerName, "null") != 0){
        // Are we going to set up a server, too? Check the name.
        // If it includes an @, we look on another machine. Otherwise, local. 
        vrpn_bool local_Phantom = VRPN_FALSE;
#ifndef NO_PHANTOM_SERVER
        char * bp= NULL;
        bp = strchr(handTrackerName, '@');
        if (bp == NULL) {
            // If there is no local connection, we can't do anything.
            if (local_device_connection == NULL) return 0;
            // Set up a local server
            local_Phantom = VRPN_TRUE;
            // Sleep to get phantom in reset positon
            // Should notify user...
            //vrpn_SleepMsecs(2000);
            // 60 update a second, I guess. 
            phantServer = new vrpn_Phantom(handTrackerName, 
                                           local_device_connection, 60);
            if (phantServer==NULL) return -1;
        }
#endif
        if (local_Phantom) {
            forceDevice = new vrpn_ForceDevice_Remote(handTrackerName, 
                                                      local_device_connection);
        } else {
            forceDevice = new vrpn_ForceDevice_Remote(handTrackerName);
        }
        // Already set to [0.2, 1.0] by default
        //MAX_K = 1.0f;
        //MIN_K = 0.2f;
        if ((forceDevice == NULL) || !(forceDevice->connectionAvailable())) {
            fprintf(stderr, "%s %s %s\n","Microscape WARNING: remote device",
                    handTrackerName, "is not available");
            if (forceDevice) delete forceDevice;
            forceDevice = NULL;
            return -1;
        }
        forceDevice->setSurfaceKdamping(0);
	if (forceDevice->register_error_handler(&forceDevice, 
		handle_phantom_error)){
		fprintf(stderr, 
			"Error: can't register vrpn_ForceDevice handler\n");
		return -1;
	}
        if (local_Phantom) {
            phantButton = new vrpn_Button_Remote(handTrackerName, 
                                                 local_device_connection);
        } else {
            phantButton = new vrpn_Button_Remote(handTrackerName);
        }
        if (phantButton->register_change_handler(&phantButtonState,
                handle_phant_button_change)){
                fprintf(stderr, "Error: can't register vrpn_Button handler\n");
		return -1;
	}
        // Set up to change from momentary to toggle for phantom button
        phantom_button_mode.addCallback(handle_phantom_button_mode_change, 
                                        &phantButtonState);
        // Get the right initial setting.
        handle_phantom_button_mode_change(phantom_button_mode, 
                                          &phantButtonState);
        if (local_Phantom) {
            vrpnHandTracker[0] = new vrpn_Tracker_Remote(handTrackerName, 
                                                 local_device_connection);
        } else {
            vrpnHandTracker[0] = new vrpn_Tracker_Remote(handTrackerName);
        }
        handSensor[0] = 0;

        vrpn_Connection * c = NULL;
        if (vrpnHandTracker[0])
          c = vrpnHandTracker[0]->connectionPtr();
	if (c == NULL) return -1;

//          vrpn_int32 dropped_conn_id =
//                          c->register_message_type(vrpn_dropped_connection);
//          c->register_handler(dropped_conn_id, handle_phantom_conn_dropped,
//                  NULL);
        
        vrpn_int32 new_conn_id =
            c->register_message_type(vrpn_got_connection);
        c->register_handler(new_conn_id, handle_phantom_reconnect, NULL);
    }
    else {
        vrpnHandTracker[0] = NULL;
        phantButton = NULL;
        forceDevice = NULL;
    }
    return 0;
}

/*****************************************************************************
 *
   peripheral_init - initialize force, tracker, buttons, sound devices
 *
 *****************************************************************************/
int
peripheral_init(vrpn_Connection * local_device_connection,
                vrpn_bool do_magellan)
{
    int	i;

    /* initialize force device (single user) */
    if (phantom_init(local_device_connection)){
	fprintf(stderr, "Error: could not initialize force device.\n");
        vrpnHandTracker[0] = NULL;
	phantButton = NULL;
	forceDevice = NULL;
    }

    if (strcmp(headTrackerName, "null") != 0) {
	vrpnHeadTracker[0] = new vrpn_Tracker_Remote(headTrackerName);
	headSensor[0] = 0;
    } else {
	vrpnHeadTracker[0] = NULL;
    }
//      printf("Head Tracker = '%s', Hand Tracker = '%s'\n",headTrackerName,
//  	handTrackerName);

    for (i = 0; i < BDBOX_NUMBUTTONS; i++) {
	bdboxButtonState[i] = 0;
    }
    for (i = 0; i < BDBOX_NUMDIALS; i++) {
	bdboxDialValues[i] = 0.0;
    }

    // Open the SGI button and dials box.  Set the buttons we want to
    // act as toggles to toggle.
    buttonBox = NULL;
    dialBox = NULL;
    if (strcmp(bdboxName, "null") != 0)
    {
	buttonBox = new vrpn_Button_Remote(bdboxName);
	dialBox = new vrpn_Analog_Remote(bdboxName);
	if (buttonBox->register_change_handler(bdboxButtonState,
		handle_bdbox_button_change))
		fprintf(stderr, "Error: can't register vrpn_Button handler\n");
	if (dialBox->register_change_handler(bdboxDialValues,
		handle_bdbox_dial_change))
		fprintf(stderr, "Error: can't register vrpn_Analog handler\n");
	buttonBox->set_toggle(TRIGGER_BT,vrpn_BUTTON_TOGGLE_OFF); // Trigger
	buttonBox->set_toggle(XY_LOCK_BT,vrpn_BUTTON_TOGGLE_OFF); // XY lock
    }
    else {
	buttonBox = NULL;
	dialBox = NULL;
    }

#ifndef NO_MAGELLAN
    // Open the Magellan puck and button box.
    if (local_device_connection && do_magellan) {
        magellanButtonBoxServer = new vrpn_Magellan(MAGELLAN_NAME, 
                                                    local_device_connection, 
                                                    "COM1", 9600);
        // This server listens to the analog output of the Magellan puck
        // and makes it seem like a tracker. 
        // First set up parameters
        // Defaults for the axes, threshold = 0, power=1, scale =1 are OK. 
        double scale = 2.0;
        magellan_param = new vrpn_Tracker_AnalogFlyParam;
        magellan_param->x.name = AF_MAGELLAN_NAME;
        magellan_param->x.channel = 0;
        magellan_param->x.scale = scale;
        magellan_param->y.name = AF_MAGELLAN_NAME;
        magellan_param->y.channel = 1;
        magellan_param->y.scale = scale;
        magellan_param->z.name = AF_MAGELLAN_NAME;
        magellan_param->z.channel = 2;
        magellan_param->z.scale = scale;
        magellan_param->sx.name = AF_MAGELLAN_NAME;
        magellan_param->sx.channel = 3;
        magellan_param->sx.scale = scale;
        magellan_param->sy.name = AF_MAGELLAN_NAME;
        magellan_param->sy.channel = 4;
        magellan_param->sy.scale = scale;
        magellan_param->sz.name = AF_MAGELLAN_NAME;
        magellan_param->sz.channel = 5;
        magellan_param->sz.scale = scale;
        
        // Next make a tracker
        // Update rate is 20.0 times per second
         magellanTrackerServer = new vrpn_Tracker_AnalogFly(MAGELLAN_NAME,
                                                     local_device_connection,
                                                     magellan_param, 20.0);
        // Client for the buttons
        magellanButtonBox = new vrpn_Button_Remote(MAGELLAN_NAME,
                                                   local_device_connection);
        if ( magellanButtonBox->register_change_handler(magellanButtonState,
                                handle_magellan_button_change)) {
            fprintf(stderr, "Error: can't register magellan vrpn_Button handler\n");
        }
        // This won't be used for anything, unless we need to get
        // direct analog output from the puck. 
        magellanPuckAnalog = new vrpn_Analog_Remote(MAGELLAN_NAME,
                                       local_device_connection);
//          if ( magellanPuckAnalog->register_change_handler(&magellanPuckActive,
//                                  handle_magellan_puck_change)) {
//              fprintf(stderr, "Error: can't register magellan vrpn_Analog handler\n");
//          }
        magellanPuckTracker = new vrpn_Tracker_Remote(MAGELLAN_NAME,
                                       local_device_connection);
        if ( magellanPuckTracker->register_change_handler(&magellanPuckActive,
                                handle_magellan_puck_change)) {
            fprintf(stderr, "Error: can't register magellan vrpn_Tracker handler\n");
        }
    }
#endif
    return 0;
}


