/*===3rdtech===
  Copyright (c) 2000 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
/**
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
#endif

#include "stm_file.h"
#include "stm_cmd.h"
#include <Tcl_Linkvar.h>
#include <Tcl_Netvar.h>

#include <PPM.h>
#include <nmb_PlaneSelection.h>
#include <nmb_Decoration.h>
#include <nmb_Globals.h>

#ifndef USE_VRPN_MICROSCOPE	// #ifdef #else #endif 	Added by Tiger
#include <Microscope.h>
#else
#include <nmm_MicroscopeRemote.h>
#endif

#include <nmm_Globals.h>

//  #include <nmg_Globals.h>
//  #include <nmg_Graphics.h>

#include "x_util.h" /* qliu */
#include "relax.h"
#include "microscape.h"  // for #defines
#include "butt_mode.h"   // for button defines
#include "interaction.h"

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
   fprintf(stderr, "ERROR: connection to PHANToM has been dropped\n");
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
       // This is the * button, we don't use it for anything. 
       break;
   case 1: 		/* Grab World mode */
       mode_change = 1;	/* Will make icon change */
       user_mode[0] = USER_GRAB_MODE;
       break;
   case 2:
       mode_change = 1;	/* Will make icon change */
       user_mode[0] = USER_LIGHT_MODE;
       break;
       
   case 3: 		/* T Touch Live mode */
       mode_change = 1;	/* Will make icon change */
       user_mode[0] = USER_PLANEL_MODE;
       break;
   case 4:              /* Commit button */
       tcl_commit_pressed = 1;
       break;
   case 5:              /* Center the surface */
       center();
       break;
   case 6: 		/* Measure mode */
       mode_change = 1;	/* Will make icon change */
       user_mode[0] = USER_MEASURE_MODE;
       break;
   case 7: 		/* Select mode */
       mode_change = 1;	/* Will make icon change */
       user_mode[0] = USER_SERVO_MODE;
       break;
   case 8:              /* Cancel button */
       tcl_commit_canceled = 1;
       break;
   }


}
#endif

// XXX - add a separate function to reconnect to phantom after a dropped
// connection
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
phantom_init()
{
    if (strcmp(handTrackerName, "null") != 0){
	forceDevice = new vrpn_ForceDevice_Remote(handTrackerName);
        // Already set to [0.2, 1.0] by default
        //MAX_K = 1.0f;
        //MIN_K = 0.2f;
        if (!(forceDevice->connectionAvailable())) {
           fprintf(stderr, "%s %s %s\n","Microscape WARNING: remote device",
               handTrackerName, "is not available");
           delete forceDevice;
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
        phantButton = new vrpn_Button_Remote(handTrackerName);
        // This might give me toggle behavior for the phantom button
        //phantButton->set_all_toggle(vrpn_BUTTON_TOGGLE_OFF);
        // NOPE. vrpn_Phantom derives from vrpn_Button, and it seems to
        // need vrpn_Button_Filter to handle toggle buttons. Maybe.
        if (phantButton->register_change_handler(&phantButtonState,
                handle_phant_button_change)){
                fprintf(stderr, "Error: can't register vrpn_Button handler\n");
		return -1;
	}
        vrpnHandTracker[0] = new vrpn_Tracker_Remote(handTrackerName);
        handSensor[0] = 0;

        vrpn_Connection * c = NULL;
        if (vrpnHandTracker[0])
          c = vrpnHandTracker[0]->connectionPtr();
	if (c == NULL) return -1;

        vrpn_int32 dropped_conn_id =
                        c->register_message_type(vrpn_dropped_connection);
        c->register_handler(dropped_conn_id, handle_phantom_conn_dropped,
                NULL);

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
peripheral_init(vrpn_Connection * local_device_connection)
{
    int	i;

    /* initialize force device (single user) */
    if (phantom_init()){
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
    if (local_device_connection) {
        magellanButtonBoxServer = new vrpn_Magellan("Magellan0@bogus", 
                                                    local_device_connection, 
                                                    "COM1", 9600);
        magellanButtonBox = new vrpn_Button_Remote("Magellan0@bogus",
                                                   local_device_connection);
        if ( magellanButtonBox->register_change_handler(magellanButtonState,
                                handle_magellan_button_change)) {
            fprintf(stderr, "Error: can't register magellan vrpn_Button handler\n");
        }
    }
#endif
    return 0;
}


// MOVED to graphics.C
//int v_setup_lighting(int nothing)

// int pgl_init() is obsolete

#ifndef USE_VRPN_MICROSCOPE
int     describe_version_to_stream(stm_stream *s)
{
        int     response = SPM_CLIENT_HELLO;
        int     ver;
        char    buffer[1000];
        char    *bufptr = buffer;

        // Fill up a buffer with the report to insert into the stream
        // This insertion must be architecture-independent.

        response = htonl(response);
        memcpy(bufptr, &response, sizeof(response));
        bufptr += sizeof(response);
        memcpy(bufptr, "nM!", 4);
        bufptr += 4;
        strncpy(bufptr, "microscape", STM_NAME_LENGTH);
        bufptr += STM_NAME_LENGTH;
        ver = htonl(MICROSCAPE_MAJOR_VERSION);
        memcpy(bufptr, &ver, sizeof(ver));
        bufptr += sizeof(ver);
        ver = htonl(MICROSCAPE_MINOR_VERSION);
        memcpy(bufptr, &ver, sizeof(ver));
        bufptr += sizeof(ver);

        // Write the buffer into the stream
        if (stm_write_block_to_stream(s, buffer, bufptr-buffer)) {
                fprintf(stderr,"describe_version_to_stream(): write failed\n");
                return -1;
        }

        return 0;
}

#endif
