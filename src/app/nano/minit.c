#include <stdio.h>
#include <string.h>
#include <strings.h>            // bzero(), bcopy()
#include <stdlib.h>
#include <math.h>

// socket includes
#ifndef __CYGWIN__
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

#include <vrpn_Tracker.h>
#include <vrpn_Types.h>

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

#include <nmg_Globals.h>
#include <nmg_Graphics.h>

#include "x_util.h" /* qliu */
#include "relax.h"
#include "microscape.h"  // for #defines

// MOVED #ifdef FLOW includes and declarations to graphics.C

int x_init(char* argv[]);
int stm_init (const vrpn_bool, const vrpn_bool, const vrpn_bool,
              const int, const char *, const int, const int);
extern "C" void txt_init(int);

static void handle_config_fp_change (vrpn_int32, void *);
static void handle_config_ss_change (vrpn_int32, void *);
static void handle_config_cj_change (vrpn_int32, void *);

//-----------------------------------------------------------------------
// Configure triangle-display methods
Tclvar_int_with_button	config_filled_polygons
     ("Filled_triangles", ".sliders", 1, handle_config_fp_change, NULL);
Tclvar_int_with_button	config_smooth_shading
     ("Smooth_shading", ".sliders", 1, handle_config_ss_change, NULL);
Tclvar_int_with_button	config_chartjunk
     ("Chart_junk", ".screenImage", 1, handle_config_cj_change, NULL);

// MOVED to graphics.C
//GLfloat l0_position[4] = { 0.0, 1.0, 0.0, 0.0 };
//GLubyte contourImage[contourImageWidth][4];
//GLubyte checkImage[checkImageDepth][checkImageWidth][checkImageHeight][4];
//GLubyte rulerImage[rulerImageHeight][rulerImageWidth][4];

// Added by Michele Clark 6/2/97
unsigned long inet_addr();

//static
void handle_config_fp_change (vrpn_int32, void *) {
  graphics->enableFilledPolygons(config_filled_polygons);
}

//static
void handle_config_ss_change (vrpn_int32, void *) {
  graphics->enableSmoothShading(config_smooth_shading);
}

//static
void handle_config_cj_change (vrpn_int32, void *) {
  graphics->enableChartjunk(config_chartjunk);
}


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

/**********
 * Variable lighting position
 *********/
VectorType	lightpos,lightdir;

/*****************************************************************************

  x_init - init x graphics display (qliu 7/11/95) 

******************************************************************************/
int 
x_init(char* argv[])
{
#ifdef __CYGWIN__

#else
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

/* callbacks for vrpn_Button_Remote and vrpn_Analog_Remote for sgi button/dial
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

        long dropped_conn_id =
                        c->register_message_type(vrpn_dropped_connection);
        c->register_handler(dropped_conn_id, handle_phantom_conn_dropped,
                NULL);
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
peripheral_init()
{
    int	i;

    /* initialize force device (single user) */
    if (phantom_init()){
	fprintf(stderr, "Error: could not initialize force device.\n");
        vrpnHandTracker[0] = NULL;
	phantButton = NULL;
	forceDevice = NULL;
    }

    if (strcmp(headTrackerName, "null") != 0)
    {
	vrpnHeadTracker[0] = new vrpn_Tracker_Remote(headTrackerName);
	headSensor[0] = 0;
    }
    else
	vrpnHeadTracker[0] = NULL;

    printf("Head Tracker = '%s', Hand Tracker = '%s'\n",headTrackerName,
	handTrackerName);

    for (i = 0; i < BDBOX_NUMBUTTONS; i++)
	bdboxButtonState[i] = 0;
    for (i = 0; i < BDBOX_NUMDIALS; i++)
	bdboxDialValues[i] = 0.0;


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
	buttonBox->set_toggle(0,vrpn_BUTTON_TOGGLE_OFF); // Trigger
	buttonBox->set_toggle(2,vrpn_BUTTON_TOGGLE_OFF); // Modify
	buttonBox->set_toggle(14,vrpn_BUTTON_TOGGLE_OFF); // Sweep lock
	buttonBox->set_toggle(20,vrpn_BUTTON_TOGGLE_OFF); // XY lock
    }
    else {
	buttonBox = NULL;
	dialBox = NULL;
    }

    return 0;
}


// MOVED to graphics.C
//int v_setup_lighting(int nothing)

// int pgl_init() is obsolete

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

/*****************************************************************************
 *
   stm_init - initialize stm
 *
 *****************************************************************************/

int stm_init (const vrpn_bool set_region,
              const vrpn_bool set_mode, const int socketType,
              const char * SPMhost,
              const int SPMport, const int UDPport) {
    int retval;

    /* Open the STM connection and tell it to start scanning */
    if (dataset->inputGrid->readMode() == READ_DEVICE) {
      retval = microscope->Initialize(set_region, set_mode,
                                      describe_version_to_stream,
                                      socketType, SPMhost, SPMport, UDPport);
      if (retval == -1) {
        fprintf(stderr, "Microscope constructor failed in stm_init().\n");
        exit(-1);
      }
    }

    /* Open the stream input if we are using one */
    if (dataset->inputGrid->readMode() == READ_STREAM) {
      retval = microscope->Initialize(describe_version_to_stream);
      if ((!microscope)||( retval == -1)){
        fprintf(stderr, "Microscope constructor failed in stm_init().\n");
        exit(-1);
      }
    }

    return(0);

}	/* stm_init */

