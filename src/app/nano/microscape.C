
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#ifndef __CYGWIN__
#include <sys/time.h>
#endif
#include <sys/stat.h>
#include <dirent.h>

#include <tcl.h>
#include <tk.h>  // for Tk_DoOneEvent()

#include <vrpn_Connection.h>
#include <vrpn_FileConnection.h>
#include <vrpn_FileController.h>
#include <vrpn_ForceDevice.h>
#include <vrpn_Forwarder.h>
#include <vrpn_Analog.h>
#include <vrpn_Clock.h>  // for round-trip-time routines (foo_rtt())

// ############ getpid hack
#if defined (__CYGWIN__) && defined (VRPN_USE_WINSOCK_SOCKETS)

/* #include <sys/unistd.h>  // for getpid() */

// cannot include sys/unistd.h because it would cause a conflict with the
// windows-defined gethostbyname.  Instead, I'll declare getpid myslef.  This
// is really ugly and dangerous.
extern "C" {
pid_t getpid();
}


// there is also a different getpid defined in Process.h in the VC-6.0 include
// directory.  I think it has a different return type.

#endif
// ############

// base
#include <BCPlane.h>
#include <colormap.h>
#include <PPM.h>
#include <nmb_PlaneSelection.h>
#include <nmb_Dataset.h>
#include <nmb_Decoration.h>
#include <nmb_Types.h>
#include <nmb_Debug.h>
#include <Tcl_Netvar.h>
#include <nmb_Line.h>
#include <nmb_TimerList.h>

#include <nmm_Globals.h>	
#ifndef USE_VRPN_MICROSCOPE	
#include <Microscope.h>
#else
#include <nmm_MicroscopeRemote.h>
#endif
#include <nmm_Types.h>

// graphics
#include <nmg_GraphicsImpl.h>
#include <nmg_GraphicsRemote.h>
//#include <nmg_GraphicsNull.h>  // for timing tests
#include <nmg_RenderServer.h>
#include <nmg_RenderClient.h>
#include <nmg_GraphicsTimer.h>

// ui
#include <ModFile.h>
#include <GraphMod.h>
#include <nmui_Component.h>
#include <nmui_PlaneSync.h>

#ifdef	PROJECTIVE_TEXTURE
// Registration
#include "alignerUI.h"
// 
#include "RobotControl.h"
#endif

#include "x_util.h" /* qliu */
#include "x_aux.h"

#include "stm_file.h"
#include "vrml.h"
#include "interaction.h"
#include "globals.h"
#include "active_set.h"
#include "stm_cmd.h"
#include "relax.h"
#include "termio.h"
#include "updt_display.h"
#include "tcl_tk.h"
#include <Tcl_Linkvar.h>
#include <Tcl_Netvar.h>
#include "ohmmeter.h"	/* French ohmmeter */
#include "nma_Keithley2400_ui.h"  // VI Curve generator - Keithley 2400
#include "nmui_SEM.h" // EDAX SEM
#include <Topo.h>
#include "Timer.h"
#include "microscopeHandlers.h"
#include "microscape.h"

/*********** Import Objects **************/
#include "imported_obj.h"


/*********** UGRAPHICS *******************/
#include "UTree.h"
#include "URAxis.h"
#include "URPolygon.h"
#include "URTexture.h"

// UGraphics made us depend explicitly on vlib again in this file (?)
#include <v.h>

// shared memory threading libraries
//  [juliano 12/99] this *must* be included after v.h, or gcc craps out
#include <thread.h>

UTree World;
UTree Textures;


/*********** IMPORT OBJECTS **************/

static const char * import_filename_string;
   ///<the filename containing the geometry of the object we want to import
static imported_obj_list* object_list = NULL; 
   ///<a linked list of imported_objs

/*********** UGRAPHICS *******************/

#define DATA_SIZE 128

// need to cast away constness

#if defined(FLOW) || defined(linux) || defined(sgi) || defined(hpux) || defined(__CYGWIN__)
# define UGLYCAST (double *)
#else
# define UGLYCAST
#endif  // not FLOW || linux || sgi || hpux || __CYGWIN__

#if (!defined(X) || !defined(Y) || !defined(Z))
#define	X	(0)
#define	Y	(1)
#define	Z	(2)
#endif

#ifndef MAX
#define MAX(a,b) ((a)<(b)?(b):(a))
#endif

//-------------------------------------------------------------------------
// Callback functions used by the Tcl variables.

static  void    handle_contour_color_change(vrpn_int32 new_value, void *userdata);
//need this separate from color because of float new_value
/*static  void    handle_contour_opacity_change(float new_value, void *userdata);*/
static void handle_contour_width_change(vrpn_float64 new_value, void *userdata);
static void handle_alpha_dataset_change(const char *new_value, void *userdata);
static void handle_alpha_slider_change (vrpn_float64, void *);
static void handle_color_slider_change (vrpn_float64, void *);
static void handle_rulergrid_offset_change (vrpn_float64, void *);
static void handle_rulergrid_scale_change (vrpn_float64, void *);

///so we can track hand position of collaborator(s)
static void handle_collab_machine_name_change(const char *new_value, void *userdata);

static  void    handle_x_dataset_change(const char *new_value, void *userdata);

static	void    handle_colormap_change(const char *new_value, void *userdata);
static	void	handle_exportFileName_change(const char *new_value, void *userdata);
static  void    handle_exportPlaneName_change(const char *new_value, void *userdata);
static  void    handle_sumPlaneName_change(const char *new_value, void *userdata);
static  void    handle_adhPlaneName_change(const char *new_value, void *userdata);
static  void    handle_flatPlaneName_change(const char *new_value, void *userdata);
static  void	handle_lblflatPlaneName_change(const char *new_value, void *userdata);
static	void	handle_adhesion_average_change(vrpn_float64 new_value, void *userdata);
static	void	handle_filterPlaneName_change(const char *new_value, void *userdata);
static  void    handle_rulergrid_selected_change(vrpn_int32 new_value, void *userdata);
static	void	handle_rulergrid_color_change(vrpn_int32 new_value, void *userdata);
///need this separate from color because of float new_value
static  void    handle_rulergrid_opacity_change(vrpn_float64 new_value, void *userdata);
static	void	handle_friction_slider_change (vrpn_float64, void * userdata);
static  void	handle_bump_slider_change (vrpn_float64, void * userdata);
static  void    handle_buzz_slider_change (vrpn_float64, void * userdata);
static	void	handle_compliance_slider_change (vrpn_float64, void * userdata);
static  void    handle_ruler_widthx_change(vrpn_float64 new_value, void *userdata);
static  void    handle_ruler_widthy_change(vrpn_float64 new_value, void *userdata);
static	void	handle_rulergrid_angle_change(vrpn_float64 new_value, void *userdata);
static	void	handle_replay_rate_change(vrpn_int32 new_value, void *userdata);
//#ifndef USE_VRPN_MICROSCOPE
static  void    handle_set_stream_time_change(vrpn_int32 new_value, void *);
//#endif
static  void    handle_joymove (vrpn_float64, void *);
static  void    handle_recovery_time_change (vrpn_float64, void *);
static	void	handle_texture_scale_change (vrpn_float64, void *);
static	void	handle_markers_shown_change (vrpn_int32, void *);
static	void	handle_markers_height_change (vrpn_int32, void *);

static	void	handle_rewind_stream_change (vrpn_int32 new_value, void * userdata);
static	void	handle_shiny_change (vrpn_int32 new_value, void * userdata);
static	void	handle_diffuse_change (vrpn_float64 new_value, void * userdata);
static  void    handle_surface_alpha_change (vrpn_float64 new_value, void * userdata);
static	void	handle_specular_color_change (vrpn_float64 new_value, void * userdata);
static	void	handle_sphere_scale_change (vrpn_float64 new_value, void * userdata);

static	void	handle_save_xform_change (vrpn_int32 new_value, void * userdata);
static	void	handle_set_xform_change (vrpn_int32 new_value, void * userdata);
static void handle_global_icon_scale_change (vrpn_float64, void *);

/// Callback functions for latency compensation techniques.
static void handle_trueTip_change (vrpn_int32, void *);
static void handle_trueTip_scale_change (vrpn_float64, void *);
static void handle_constraint_mode_change (vrpn_int32, void *);
static void handle_constraint_kspring_change (vrpn_float64, void *);

/// Callback for screen capture
static void handle_screenImageFileName_change(const char *new_value, void *userdata);

static void handle_rulergridOrientLine_change (vrpn_int32, void *);
static void handle_rulergridPositionLine_change (vrpn_int32, void *);

/// VRPN callback functions
static void	handle_tracker2room_change(void *userdata, 
					const vrpn_TRACKERTRACKER2ROOMCB info);
static void	handle_sensor2tracker_change(void *userdata, 
					const vrpn_TRACKERCB info);

static void handle_collab_sensor2tracker_change(void *userdata,
					const vrpn_TRACKERCB info);
/**< handle_collab_sensor2tracker_change is the callback for the position
	and orientation messages sent from the nM_coord_change_server (to
	track a collaborator's hand position) */

static void handle_collab_mode_change(void *userdata,
					const vrpn_ANALOGCB info);
/**< handle_collab_mode_change is the callback for the mode
   that track a collaborator's mode */

/**** Not currently used
static void	handle_sensor2tracker_quat_change(void *userdata,
					const vrpn_TRACKERCB info);
static void	handle_forcedevice_scp_change(void *userdata, 
					const vrpn_FORCESCPCB info);
*/
static void	handle_unit2sensor_change(void *userdata,
					const vrpn_TRACKERUNIT2SENSORCB info);

///import_object handlers
static  void    handle_import_filename_change (const char *, void *);
static  void    handle_load_import_file_change (vrpn_int32, void *);
static void handle_load_button_press_change (vrpn_int32, void *);

// NANOX
/// synchronization UI handlers
// generic synchronization
//static void handle_synchronize_change (vrpn_int32, void *);
//static void handle_get_sync (vrpn_int32, void *);
//static void handle_lock_sync (vrpn_int32, void *);
// since streamfiles are time-based, we need to send a syncRequest()
static void handle_synchronize_timed_change (vrpn_int32, void *);
static void handle_timed_sync (vrpn_int32, void *);

static void handle_collab_measure_change (vrpn_float64, void *);
static void handle_center_pressed (vrpn_int32, void *);

/*******************
 * Global variables
 *******************/

// used where???
char * tcl_script_dir;

static BCPlane * oldxPlane= NULL;

// Used in Timer.h
//int	time_frame = 0;         // How much frames to time
int     framelog = 0;           // Flag to see if frame time log is on

#define NANO_FONT	(34)
#define	MAXFILES	(30)		///< Maximum number of files to load 
static vrpn_bool    head_tracked = VRPN_FALSE;              
    ///< Flag set when head tracking is active

nmb_Dataset * dataset;
nmb_Decoration * decoration;

#ifdef USE_VRPN_MICROSCOPE

nmm_Microscope_Remote * microscope;

#else

Microscope * microscope;

#endif

nmg_Graphics * graphics;

static int LOOP_NUM;
TopoFile GTF;

// used in interaction.c
int	using_mouse3button = 0;	///< Using Mouse3 button as trigger?
int	mouse3button = 0;	///< Status of mouse button 3

static float	alpha_red = 0.0;		///< Color of the alpha pattern
static float	alpha_green = 1.0;
static float	alpha_blue = 0.0;

#ifndef _WIN32
/// only used in the unix version of microscape.
static	void	update_xdisp_on_plane_change (BCPlane * p,
                                              int x, int y, void *) {

  // If x or y are negative, this means "whole grid update"
  if ( (x >=0) && (y >=0) ) {
    x_put_value(x, y, p->value(x, y));
  } else {	// whole grid update
    //XXX Nothing for now
  }
}
#endif


vrpn_Tracker_Remote * vrpnHeadTracker [NUM_USERS];
vrpn_Tracker_Remote * vrpnHandTracker [NUM_USERS];
int headSensor [NUM_USERS];
int handSensor [NUM_USERS];

// NANOX
/** collaboratingPeerServerConnection carries shared data for
 hand tracking, streamfile synchronization, and
 view sharing. */
static vrpn_Connection *
                collaboratingPeerServerConnection = NULL;
static vrpn_Connection *
                collaboratingPeerRemoteConnection = NULL;
static vrpn_Connection *
                interfaceLogConnection = NULL;
// This should eventually be generalized to a set of collaborating
// remote peers.


///These are used in tracking a remote user's hand position.
vrpn_Tracker_Remote *vrpnHandTracker_collab[NUM_USERS];
nM_coord_change *nM_coord_change_server;

/// These are used to track a remote user's mode.
vrpn_Analog_Remote *vrpnMode_collab[NUM_USERS];
vrpn_Analog_Server *vrpnMode_Local;

/// These are used for synchronizing with a remote user's streamfile playback.
static vrpn_bool isSynchronized;

// static can't be decalred for this struct?
class WellKnownPorts {

  public:

    // server ports on this machine

    static const int interfaceLog;
    static const int graphicsControl;
    static const int remoteRenderingData;
    static const int collaboratingPeerServer;
    static const int roundTripTime;

};


const int WellKnownPorts::interfaceLog (4501);

const int WellKnownPorts::graphicsControl (nmg_Graphics::defaultPort);
const int WellKnownPorts::remoteRenderingData (nmg_Graphics::defaultPort + 1);

const int WellKnownPorts::collaboratingPeerServer (4510);
const int WellKnownPorts::roundTripTime (4581);


//-----------------------------------------------------------------------
// TCL initialization section.
//-----------------------------------------------------------------------

/// Host where the user interface of microscape is running.
static Tclvar_string my_hostname("my_hostname", "localhost");

//-----------------------------------------------------------------------
/// This is the list of color map files from which mappings can be made.  It
/// should be loaded with the files in the colormap directory, and "CUSTOM".

static Tclvar_list_of_strings	colorMapNames("colorMapNames");

//-----------------------------------------------------------------------
/// This is a string that lets the user choose a color map to use.

static	char	defaultColormapDirectory[] = "/afs/unc/proj/stm/etc/colormaps";
static	char	*colorMapDir;

static ColorMap	colorMap;		///< Color map currently loaded
ColorMap	*curColorMap = NULL;	///< Pointer to the current color map
  // used in vrml.C




static Tclvar_list_of_strings export_formats("export_formats");

Tclvar_string	exportPlaneName("export_plane","");
Tclvar_string	exportFileType("export_filetype","");
Tclvar_string newExportFileName("export_filename", "");

//-----------------------------------------------------------------------
// This section deals with selecting the color to apply to the surface.
// It includes both a string to allow choosing the field to map color
// from, the min and max colors, and the min and max values that are
// mapped to the min and max colors.

static int	minC[3] = {255,255,255};
static int	maxC[3] = {255,255,255};

/// The limits on the Tk slider where min and max value are selected
Tclvar_float	color_slider_min_limit("color_slider_min_limit",0);
Tclvar_float	color_slider_max_limit("color_slider_max_limit",1);


// NANOX
/// The positions of the min and max values within the Tk slider
TclNet_float	color_slider_min("color_slider_min",0);
TclNet_float	color_slider_max("color_slider_max",1);


// NANOX
/// Streamfile controls
TclNet_int replay_rate("stream_replay_rate", 1,
			 handle_replay_rate_change, NULL);

/// Signal that the stream file should be rewound to the beginning
TclNet_int	rewind_stream("rewind_stream",0,
			handle_rewind_stream_change, NULL);

/// This is the time value to jump to in the stream file. 
TclNet_int set_stream_time ("set_stream_time", 0);
/// This is a flag (0/1) to say " jump to new time now!"
TclNet_int set_stream_time_now ("set_stream_time_now", 0,
                             handle_set_stream_time_change, NULL);

#if 1
// NANOX - XXX
/// Quick method of sharing measure line locations
TclNet_float measureRedX ("measure_red_x", 0.0,
                           handle_collab_measure_change, (void *) 0);
TclNet_float measureRedY ("measure_red_y", 0.0,
                           handle_collab_measure_change, (void *) 0);
TclNet_float measureGreenX ("measure_green_x", 0.0,
                           handle_collab_measure_change, (void *) 1);
TclNet_float measureGreenY ("measure_green_y", 0.0,
                           handle_collab_measure_change, (void *) 1);
TclNet_float measureBlueX ("measure_blue_x", 0.0,
                           handle_collab_measure_change, (void *) 2);
TclNet_float measureBlueY ("measure_blue_y", 0.0,
                           handle_collab_measure_change, (void *) 2);
#endif


/// the Tk slider for recovery time
/// XXX What is recovery time for? Adhesion?
TclNet_float recovery_time ("recovery_time", 1);

// The string that is used to determine which inputGrid field to use

Tclvar_float    compliance_slider_min("compliance_slider_min",0);
Tclvar_float    compliance_slider_max("compliance_slider_max",0);
Tclvar_float	compliance_slider_min_limit("compliance_slider_min_limit",0);
Tclvar_float	compliance_slider_max_limit("compliance_slider_max_limit",1);
///This section deals with the compliance plane
Tclvar_string compliancePlaneName
               ("compliance_comes_from", "");

Tclvar_float    friction_slider_min("friction_slider_min",0);
Tclvar_float	friction_slider_max("friction_slider_max",1);
/// The limits on the Tk slider where min and max value are selected
Tclvar_float	friction_slider_min_limit("friction_slider_min_limit",0);
Tclvar_float	friction_slider_max_limit("friction_slider_max_limit",1);

//This section deals with the friction plane
Tclvar_string frictionPlaneName
               ("friction_comes_from", "");

Tclvar_float    bump_slider_min("bump_slider_min",0);
Tclvar_float    bump_slider_max("bump_slider_max",1);
// The limits on the Tk slider where min and max value are selected
Tclvar_float    bump_slider_min_limit("bump_slider_min_limit",0);
Tclvar_float    bump_slider_max_limit("bump_slider_max_limit",1);
//This section deals with the bump plane
Tclvar_string bumpPlaneName
               ("bumpsize_comes_from", "");

Tclvar_float    buzz_slider_min("buzz_slider_min",0);
Tclvar_float    buzz_slider_max("buzz_slider_max",1);
// The limits on the Tk slider where min and max value are selected
Tclvar_float    buzz_slider_min_limit("buzz_slider_min_limit",0);
Tclvar_float    buzz_slider_max_limit("buzz_slider_max_limit",1);
//This section deals with the buzz plane
Tclvar_string buzzPlaneName
               ("buzzing_comes_from", "");

Tclvar_float    adhesion_slider_min("adhesion_slider_min",0);
Tclvar_float	adhesion_slider_max("adhesion_slider_max",1);
// The limits on the Tk slider where min and max value are selected
Tclvar_float	adhesion_slider_min_limit("adhesion_slider_min_limit",0);
Tclvar_float	adhesion_slider_max_limit("adhesion_slider_max_limit",1);

//This section deals with the adhesion plane
Tclvar_string adhesionPlaneName
               ("adhesion_comes_from", "");

//-----------------------------------------------------------------------
// This section deals with selecting the data set to map to sound.
// It includes both a string to allow choosing the field to map
// from and the scale of the mapping.

Tclvar_string	soundPlaneName("sound_comes_from","");
/*Tclvar_float sound_scale("sound_scale",1);*/

Tclvar_float sound_slider_min("sound_slider_min",0);
Tclvar_float sound_slider_max("sound_slider_max",1);
Tclvar_float sound_slider_min_limit("sound_slider_min_limit",0);
Tclvar_float sound_slider_max_limit("sound_slider_max_limit",1);

//-----------------------------------------------------------------------
// This section deals with selecting the offset in x and y and the scale of
// the ruler grid that can be overlaid on the surface.  It also has an
// integer that tells whether to do the ruler or not.  It also controls the
// orientation and specifies what image (if any) to use in place of the
// ruler.

//instead, put these in the rulergrid dialog box

/// if on, set the rulergrid position by the red line
TclNet_int rulergrid_position_line ("rulergrid_position_line", 0,
                                    handle_rulergridPositionLine_change, NULL);
/// if on, set the rulergrid angle by the green line
TclNet_int rulergrid_orient_line ("rulergrid_orient_line", 0,
                                  handle_rulergridOrientLine_change, NULL);
/// otherwise set with these sliders
TclNet_float rulergrid_xoffset ("rulergrid_x", 0);
TclNet_float rulergrid_yoffset ("rulergrid_y", 0);
TclNet_float rulergrid_scale ("rulergrid_scale", 500);
TclNet_float rulergrid_angle ("rulergrid_angle", 0);
TclNet_float ruler_width_x ("ruler_width_x", 1);
TclNet_float ruler_width_y ("ruler_width_y", 1);

///set default Rulergrid opacity to 128 (50%) instead of 255 (100%)
TclNet_float ruler_opacity ("ruler_opacity", 128);


//The quoted parameters are the variable names in tcl-space
TclNet_int ruler_r ("ruler_r", 255, NULL, NULL);
TclNet_int ruler_g ("ruler_g", 255, NULL, NULL);
TclNet_int ruler_b ("ruler_b", 55, NULL, NULL);
TclNet_int   rulergrid_changed ("rulergrid_changed", 0);
TclNet_int	rulergrid_enabled ("rulergrid_enabled",0);

//
// Genetic Textures  -- Alexandra Bokinsky
//
// The toggle button, genetic-textures parameters button, and
// the GA pop-up window checklist global variables. Used
// to activate the genetic textures and to select which variables
// are used in the GA process..
//

/// Which GA datasets to use:
Tclvar_checklist *genetic_checklist = NULL;      

/// Activates call-back to send data on-demand...
Tclvar_int gen_send_data( "gen_send_data", 0, NULL, NULL );

/// Activates the call-back to display the GA texture...
Tclvar_int gen_tex_activate( "gen_tex_activate", 0, NULL, NULL );

/// Enables the GA textues... (toggle button on main tcl window)
Tclvar_int genetic_textures_enabled ("genetic_textures_enabled", 0);



//-----------------------------------------------------------------
/// Enables the realigning textues... (toggle button on main tcl window)
// XXX This function is not included in the one screen tcl interface.
Tclvar_int display_realign_textures ("display_dataset", 0);
Tclvar_int realign_textures_enabled ("realign_dataset", 0);
Tclvar_int set_realign_center ("set_center", 0);

Tclvar_string texturePlaneName ("none" );
Tclvar_string textureConversionMapName("CUSTOM" );

Tclvar_float realign_textures_slider_min_limit("realign_textures_slider_min_limit",0);
Tclvar_float realign_textures_slider_max_limit("realign_textures_slider_max_limit", 1.0);
Tclvar_float realign_textures_slider_min("realign_textures_slider_min", 0);
Tclvar_float realign_textures_slider_max("realign_textures_slider_max", 1.0);

Tclvar_string	newRealignPlaneName("realignplane_name","");

TclNet_int shiny ("shiny", 55);
TclNet_float diffuse ("diffuse", 0.5);	//What should default be?
TclNet_float surface_alpha ("surface_alpha", 1.0);
TclNet_float specular_color ("specular_color", 1.0);

/// Tom Hudson's latency compensation techniques

Tclvar_int truetip_showing ("truetip_showing", 0);
Tclvar_float truetip_scale ("truetip_scale", 1);

Tclvar_int constraint_mode ("constraint_mode", 0);
Tclvar_float constraint_kspring ("constraint_kspring", 10.0f);

//-----------------------------------------------------------------------
/// This section deals with an image type for a screen capture
const char **screenImage_formats_list = ImageType_names;
Tclvar_list_of_strings screenImage_formats("screenImage_format_list");

Tclvar_string    screenImageFileType("screenImage_format", "");
Tclvar_string newScreenImageFileName("screenImage_filename", "");


//-----------------------------------------------------------------


//PPM	*rulerPPM = NULL;	///< Image to use for the ruler
static char * rulerPPMName = NULL;   ///< Name of image to use for the ruler
static PPM * alphaPPM = NULL;	///< Image to use for the alpha blending
static PPM * bumpPPM = NULL;	///< Image to use for the bump mapping
static PPM * noisePPM = NULL;   ///< Uniform noise image for spot noise shader 

//---------------------
/// Scaling factor for the sphere icon
Tclvar_float sphere_scale ("sphere_scale", 12.5f);

//-----------------------------------------------------------------------
// This section deals with selecting the contour lines to apply to the surface.
// It includes a string to determine when to map them from and a float
// value to determine their spacing.
TclNet_float texture_scale ("texture_spacing",10);
TclNet_float contour_width ("contour_width", 10.0);
TclNet_int contour_r ("contour_r", 255);
TclNet_int contour_g ("contour_g", 55);
TclNet_int contour_b ("contour_b", 55);
TclNet_int contour_changed ("contour_changed", 0);

//-----------------------------------------------------------------------
// This section deals with selecting the alpha for the checkerboard pattern.
// It includes both a string to allow choosing the field to map alpha
// from, the min and max colors, and the min and max values that are
// mapped to the min and max alphas.

// The limits on the Tk slider where min and max value are selected
Tclvar_float	alpha_slider_min_limit ("alpha_slider_min_limit", 0);
Tclvar_float	alpha_slider_max_limit ("alpha_slider_max_limit", 1);

// The positions of the min and max values within the Tk slider
Tclvar_float	alpha_slider_min ("alpha_slider_min", 0);
Tclvar_float	alpha_slider_max ("alpha_slider_max", 1);

// The limits on the Tk slider where min and max value are selected
Tclvar_float    x_min_scale ("x_min_scale", 0);
Tclvar_float    x_max_scale ("x_max_scale", 1);

// The positions of the min and max values within the Tk slider
Tclvar_float    x_min_value ("x_min_value", 0);
Tclvar_float    x_max_value ("x_max_value", 1);


/// The string that is used to determine which inputGrid field to use
Tclvar_string	xPlaneName ("x_comes_from", "");

//-----------------------------------------------------------------------
/// This is the list of image processing programs available.  It
/// should be loaded with the files in the image proc directory, and "none".

nmb_ListOfStrings	procProgNames;

//-----------------------------------------------------------------------
/// These are widgets which allow the use to pick the name and parameters 
/// of the image processing program, like input and output.
/// "proc" is short for "processing"

static  char    defaultFilterDir[] = "/afs/unc/proj/stm/etc/filters";
static	char	*procImageDir;

/// This is the filter program name.
Tclvar_string	procProgName ("pick_program", "");
/// The string that is used to determine which inputGrid field to use
Tclvar_string	procPlaneName ("pick_plane", "");
Tclvar_string	procParams ("proc_params","");

/// This is the output plane 
Tclvar_string	newFilterPlaneName("filterplane_name","");

/// Float scales to determine the angle and scale for programs that use them
Tclvar_float procAngle ("proc_angle", 0.0f);
Tclvar_float procScale ("proc_scale", 1.0f);


/// A list of the textures which can be used for the pxfl shader. 
nmb_ListOfStrings	textureNames;

static  char    defaultTextureDir[] = "/afs/unc/proj/stm/etc/textures";
static	char	*textureDir;

// A test list of textures. 
// Tclvar_string	textureName ("texture_name", "");

//----------------------------------------------------------------------
// This section deals with the creation of new data planes that are
// derived from other planes.  It provides variables
// for the name of the new plane and the name of the plane(s) to use
// for sources.  The names are selected from lists.

Tclvar_string	newFlatPlaneName("flatplane_name","");

//added 1-9-99 by Amy Henderson
Tclvar_string newLBLFlatPlaneName("lblflatplane_name","");

Tclvar_string	sumPlane1Name ("first_plane","");
Tclvar_string	sumPlane2Name ("second_plane","");
Tclvar_float	sumScale("sum_scale",-1.0);
Tclvar_string	newSumPlaneName ("sumplane_name","");

Tclvar_string	adhPlane1Name("first_plane","");
Tclvar_string	adhPlane2Name("last_plane","");
Tclvar_string	newAdhPlaneName("adhesionplane_name","");
static	char	lastAdhPlaneName[1000] = "";

Tclvar_string newResamplePlaneName("resample_plane_name", "");

Tclvar_float	adhNumToAvg("adhesion_average",3);


// Aron Helser Temporary, testing how to save and restore a viewpoint
Tclvar_int save_xform ("save_xform", 0);
Tclvar_int set_xform ("set_xform", 0);

//----------------------------------------------------------------------
/// These tcl vars link to a pad which rotates the view of the surface.
/// They are defined in panel_tools.tcl, and instantiated in view.tcl

Tclvar_float	joy0x("joy0(x)",0.0,
		      handle_joymove, (void*)"t");
Tclvar_float	joy0y("joy0(y)",0.0,
		      handle_joymove, (void*)"t");
Tclvar_float	joy0z("joy0(z)",0.0,
			handle_joymove, (void*)"t");
Tclvar_float	joy1x("joy1(x)",0.0,
			handle_joymove, (void*)"r");
Tclvar_float	joy1y("joy1(y)",0.0,
			handle_joymove, (void*)"r");
Tclvar_float	joy1z("joy1(z)",0.0,
			handle_joymove, (void*)"r");
Tclvar_float	joy0b("joy0(b)",0.0, NULL, NULL);
Tclvar_float	joy1b("joy1(b)",0.0, NULL, NULL);

//----------------------------------------------------------------------

// Change the number of pulse or scrape markers to show

Tclvar_int numMarkersShown ("number_of_markers_shown", 1000);

Tclvar_int markerHeight ("marker_height", 100);


Tclvar_float global_icon_scale ("global_icon_scale", 0.25);

/// Controls for the french Ohmmeter - creates many tcl widgets
Ohmmeter *the_french_ohmmeter_ui = NULL;

/// Controls for the VI curve generator -
/// uses keithley2400.tcl for most widgets.
nma_Keithley2400_ui * keithley2400_ui = NULL;


/// Controls for the SEM
nms_SEM_ui * sem_ui = NULL;

/// Controls for registration
AlignerUI * aligner_ui = NULL;

#ifdef NANO_WITH_ROBOT
RobotControl * robotControl = NULL;
#endif

/// Scales how much normal force is felt when using Direct Z Control
Tclvar_float directz_force_scale("directz_force_scale", 0.1);


// NANOX
// TCH 19 Jan 00 - Turned these into netvars that were logged on
// interfaceLogConnection but were NOT part of rootUIControl so that
// they wouldn't be shared but that while replaying logs we could
// determine which state to use.
TclNet_int share_sync_state ("share_sync_state", 0);
TclNet_int copy_inactive_state ("copy_inactive_state", 0);
TclNet_int copy_to_private_state ("copy_to_private_state", 0);
TclNet_int copy_to_shared_state ("copy_to_shared_state", 0);

///to get the name of the machine where the collaborator is whose hand
///position we want to track
TclNet_string collab_machine_name ("collab_machine_name", "");


static vrpn_bool loggingInterface = VRPN_FALSE;
static vrpn_bool replayingInterface = VRPN_FALSE;
static char loggingPath [256];
static timeval loggingTimestamp;


Tclvar_int tcl_center_pressed ("center_pressed", 0, handle_center_pressed);

//---------------------------------------------------------------------------
/// Deal with the stride between rows on the grid for tesselation.  This is
/// the step size between one row/column of the display list and the next.

TclNet_int tclstride ("tesselation_stride", 1);


// END tcl declarations
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// true globals

// used in interaction.c
float MAX_K = 1.0f;
float MIN_K = 0.2f;

int stride;       // used in VRML.C and nowhere else!

// used in minit.c;  we probably could/ought to pass these
char * bdboxName;
char * headTrackerName;
char * handTrackerName;	// = phantom server name

/************
 * Variables to turn on/off optional parts of the system
 ************/

//----------------------------------------------------------------------
// file-scoped globals

static long displayPeriod = 0L; ///< minimum interval between displays 

static int read_mode = READ_DEVICE;	///< Where get info from? 


// HACK TCH 18 May 98 - program doesn't run with glenable 0
static int glenable = 1;           ///< Default gl display is enabled 
static int tkenable = 1;           ///< Default is to use Tk control panels 

static int drawOnlyOnCenter=0;	///< Only draw a frame when the user centers?

static int ttyFD;			///< File desc for TTY device 

/***********
 * globals for drawing the graph
 ***********/

static int xmode = 0;             ///< default not draw any graph 

static TwoDLineStrip * teststrip = NULL;

static int gwi = 400;	///< Width of the graphical window
static int ghi = 400;	///< Height of the graphical window

static float xscale;	///< Scale to map window pixels to nM
static float yscale;	///< Scale to map window pixels to the Y axis
static float yoffset;	///< Offset to add to Y-axis values (already scaled)

static float testarray [1000];

static char tcl_default_dir [] = "/afs/cs.unc.edu/project/stm/bin/";


/************
 * Variables to turn on/off optional parts of the system
 ************/

//static int perf_mode = 0;			/* No perfmeter by default */
//static int do_menus = 1;			/* Menus on by default */
static int do_keybd = 1;                   ///< Keyboard on by default
//static int do_ad_device = 1;               /* A/D devices on by default */
//static int show_cpanels = 0;		/* Control panels hidden by default */
//static int show_grid = 0;		/* Measuring grid hidden by default */
int using_phantom_button = 0;
//static int ubergraphics_enabled = 0;

static vrpn_bool set_region = VRPN_FALSE; ///< Should we set the region at start?
static vrpn_bool set_mode = VRPN_FALSE; ///< Should we set tap/con at startup?

/********
 * Variables to control the scanning
 ********/

///Names of tracker and server for following a remote user's hand position
static char collab_handTrackerName [256];
static char nM_coord_change_server_name [256];
static char collab_ModeName [256];
static char local_ModeName [256];

/*********
 *  remote force device.  we send surface plane equations, surface paramters
    such as static friction coefficient, dynamic friction coefficient,
    spring coefficient, and damping coefficient through  it.
 ********/

// used in interaction.c, minit.c
vrpn_ForceDevice_Remote *forceDevice;

/*********
 * remote button for PHANToM
 ********/

vrpn_Button_Remote *phantButton;
int phantButtonState = 0;

/********
 * button and dial box
 *******/

vrpn_Button_Remote *buttonBox;
vrpn_Analog_Remote *dialBox;
int bdboxButtonState[BDBOX_NUMBUTTONS];
double bdboxDialValues[BDBOX_NUMDIALS];

/************
 * Ohmmeter
 ***********/

vrpn_Ohmmeter_Remote *ohmmeter;


/// These variables signal when someone has clicked "accept" in the 
/// image/modify tk window.

Tclvar_int changed_image_params ("accepted_image_params", 0);
Tclvar_int changed_modify_params ("accepted_modify_params", 0);
Tclvar_int changed_scanline_params ("accepted_scanline_params", 0);

enum { NO_GRAPHICS, LOCAL_GRAPHICS, SHMEM_GRAPHICS,
       DISTRIBUTED_GRAPHICS, TEST_GRAPHICS_MARSHALLING,
       RENDER_SERVER, TEXTURE_SERVER, VIDEO_SERVER,
       RENDER_CLIENT, TEXTURE_CLIENT, VIDEO_CLIENT };

/// A thread structure for multiprocessing.
/// Global so it can be shut down by signal handlers et al.

static Thread * graphicsServerThread = NULL;

// dnM things
// TCH 15 May 98
static vrpn_Connection * shmem_connection = NULL;
static vrpn_Connection * microscope_connection = NULL;

// remote rendering things
// TCH 29 December 99
static vrpn_Connection * renderServerOutputConnection = NULL;
static vrpn_Connection * renderServerControlConnection = NULL;
static vrpn_Connection * renderClientInputConnection = NULL;

#ifdef USE_VRPN_MICROSCOPE
/// File Connection	for replay vrpn log file
static vrpn_File_Connection * vrpnLogFile;
#endif

static vrpn_Connection *ohmmeter_connection = NULL;
static vrpn_File_Connection * ohmmeterLogFile;
/// connection for the Keithley 2400 vi curve generator
static vrpn_Connection *vicurve_connection = NULL;
static vrpn_File_Connection * vicurveLogFile;
static vrpn_Connection *sem_connection = NULL;
static vrpn_File_Connection *semLogFile;

static nmg_Graphics * gi = NULL;

/// Quick hack for multiple users - allow somebody to monitor
/// or to collaborate
static vrpn_Connection * monitor_forwarder_connection = NULL;
static vrpn_Connection * collab_forwarder_connection = NULL;

/// Exposing a server on port 4581 to report Round-Trip time (RTT) between the
/// application and the microscope controller.
/// TCH April 99
static vrpn_Connection * rtt_server_connection = NULL;
static vrpn_Analog_Server * rtt_server = NULL;

/*********
 * Functions defined in this file (added by KPJ to satisfy g++)...
 *********/

void Usage (char * s);
int main (int argc, char * argv []);
void handleTermInput (int ttyFD, vrpn_bool * donePtr);
int handleMouseEvents (nmb_TimerList *);
void get_Plane_Extents (float *,float *,float *);
void center (void);
void find_center_xforms ( q_vec_type * lock_userpos, q_type * lock_userrot,
 double * lock_userscale);

int vec_cmp (const q_vec_type a, const q_vec_type b);

void guessAdhesionNames (void);

#ifdef FLOW
   extern int sdi_disconnect_from_device(int);
#else
  #ifdef __cplusplus
     extern "C" int sdi_disconnect_from_device(int);
  #endif
#endif

    
// REMOTERENDERING
nmb_TimerList graphicsTimer;
nmb_TimerList collaborationTimer;

#ifdef TIMING_TEST

// x is the variable to average into,
// y the text name of the quantity to display

#define ttest0(x,y) \
  gettimeofday(&t_aft, &t_zone); \
  t_this = (t_aft.tv_sec - t_b4.tv_sec) * 1000000 +  \
           (t_aft.tv_usec - t_b4.tv_usec); \
  (x) += t_this; \
  t_loop += t_this; \
  if (((n_disp >> TIM_LN) << TIM_LN) == n_disp) \
    fprintf(stdout, "T %s %d ms, avg %d ms\n", y, t_this, (x) / n_disp); \
  gettimeofday(&t_b4, &t_zone);

#else

#define ttest0(x,y)

#endif

/// Handle exiting cleanly

void shutdown_connections (void) {

  // NANOX
  // XXX Bug. Tcl_Netvars are globals, and get deleted after connections.
  // But, they need a live connection to unregister their handlers.
  // We skip unregistration by binding with a null connection.
  replay_rate.bindConnection(NULL);
  rewind_stream.bindConnection(NULL);
  set_stream_time.bindConnection(NULL);
  set_stream_time_now.bindConnection(NULL);

  microscope->state.stm_z_scale.bindConnection(NULL);
  ((TclNet_string *) dataset->heightPlaneName)->bindConnection(NULL);
  tcl_wfr_xlate_X.bindConnection(NULL);
  tcl_wfr_xlate_Y.bindConnection(NULL);
  tcl_wfr_xlate_Z.bindConnection(NULL);
  tcl_wfr_rot_0.bindConnection(NULL);
  tcl_wfr_rot_1.bindConnection(NULL);
  tcl_wfr_rot_2.bindConnection(NULL);
  tcl_wfr_rot_3.bindConnection(NULL);
  tcl_wfr_scale.bindConnection(NULL);
  //tcl_wfr_changed.bindConnection(NULL);
  tclstride.bindConnection(NULL);

  ((TclNet_string *) dataset->colorPlaneName)->bindConnection(NULL);
  ((TclNet_string *) dataset->colorMapName)->bindConnection(NULL);
  color_slider_min.bindConnection(NULL);
  color_slider_max.bindConnection(NULL);

  measureRedX.bindConnection(NULL);
  measureRedY.bindConnection(NULL);
  measureGreenX.bindConnection(NULL);
  measureGreenY.bindConnection(NULL);
  measureBlueX.bindConnection(NULL);
  measureBlueY.bindConnection(NULL);

  tcl_lightDirX.bindConnection(NULL);
  tcl_lightDirY.bindConnection(NULL);
  tcl_lightDirZ.bindConnection(NULL);
  shiny.bindConnection(NULL);
  diffuse.bindConnection(NULL);
  surface_alpha.bindConnection(NULL);
  specular_color.bindConnection(NULL);

  texture_scale.bindConnection(NULL);
  contour_width.bindConnection(NULL);
  contour_r.bindConnection(NULL);
  contour_g.bindConnection(NULL);
  contour_b.bindConnection(NULL);
  contour_changed.bindConnection(NULL);
  ((TclNet_string *) dataset->contourPlaneName)->bindConnection(NULL);

  rulergrid_position_line.bindConnection(NULL);
  rulergrid_orient_line.bindConnection(NULL);
  rulergrid_xoffset.bindConnection(NULL);
  rulergrid_yoffset.bindConnection(NULL);
  rulergrid_scale.bindConnection(NULL);
  rulergrid_angle.bindConnection(NULL);
  ruler_width_x.bindConnection(NULL);
  ruler_width_y.bindConnection(NULL);
  ruler_opacity.bindConnection(NULL);
  ruler_r.bindConnection(NULL);
  ruler_g.bindConnection(NULL);
  ruler_b.bindConnection(NULL);
  rulergrid_changed.bindConnection(NULL);
  rulergrid_enabled.bindConnection(NULL);

  //display_realign_textures.bindConnection(NULL);

  if (interfaceLogConnection) {
    share_sync_state.bindConnection(NULL);
    copy_inactive_state.bindConnection(NULL);
      copy_to_private_state.bindConnection(NULL);
      copy_to_shared_state.bindConnection(NULL);
    collab_machine_name.bindConnection(NULL);
  }

  // output stream should be closed by microscope destructor,
  // WHICH WE MUST EXPLICITLY DELETE!

  if (microscope) {
    delete microscope;
    microscope = NULL;
  }
  if (microscope_connection) {
    delete microscope_connection;
    microscope_connection = NULL;
  }

  if (shmem_connection) {
    delete shmem_connection;
    shmem_connection = NULL;
  }

  if (graphics) {
    delete graphics;
    graphics = NULL;
  }
  if (gi) {
    delete gi;
    gi = NULL;
  }

  if (rtt_server) {
    delete rtt_server;
    rtt_server = NULL;
  }
  if (rtt_server_connection) {
    delete rtt_server_connection;
    rtt_server_connection = NULL;
  }

  if (monitor_forwarder_connection) {
    delete monitor_forwarder_connection;
    monitor_forwarder_connection = NULL;
  }
  if (collab_forwarder_connection) {
    delete collab_forwarder_connection;
    collab_forwarder_connection = NULL;
  }

  if (ohmmeter_connection) {
    delete ohmmeter_connection;
    ohmmeter_connection = NULL;
  }
  if (vicurve_connection) {
    delete vicurve_connection;
    vicurve_connection = NULL;
  }

  if (collaboratingPeerServerConnection) {
    delete collaboratingPeerServerConnection;
    collaboratingPeerServerConnection = NULL;
  }
  if (collaboratingPeerRemoteConnection) {
    delete collaboratingPeerRemoteConnection;
    collaboratingPeerRemoteConnection = NULL;
  }
  if (interfaceLogConnection) {
    delete interfaceLogConnection;
    interfaceLogConnection = NULL;
  }

}

/*********
 * Handle exiting cleanly when we get ^C
 *********/

void	handle_cntl_c(int which_signal)
{
	which_signal = which_signal;	// Keep the compiler happy

	/* Holler about it */
	fprintf(stderr,"Received ^C signal, shutting down and saving stream\n");
	if (do_keybd == 1)
	  {
	    /* Reset the raw terminal input to standard */
	    reset_raw_term(ttyFD);
	  }

	shutdown_connections();

	if (graphicsServerThread)
	  graphicsServerThread->kill();  // NOT PORTABLE TO NT!

	exit(0);
}
//---------------------------------------------------------------------------
/// Deal with changes in the stride between rows on the grid for tesselation.
void    handle_stride_change (vrpn_int32 newval, void * userdata) {

  nmg_Graphics * g = (nmg_Graphics *) userdata;
        nmb_PlaneSelection planes; planes.lookup(dataset);

        // Make sure that the value is an integer and in range.  Then
        // assign it to the stride.
  // avoid infinite loops
  if (newval < 1) {
    stride = 1;
    tclstride = 1;  // this causes a loop
  } else {
    stride = newval;
    // tclstride = newval would be infinite loop
    g->setTesselationStride(stride);
  }
}


/*Import from tube_foundry*/

///Tcl variable for getting filename containing geometry info for
///object that we want to import
Tclvar_string import_filename ("import_filename", "");
///Button which signals that we should import the object whose
///geometry is contained above
Tclvar_int load_import_file ("load_import_file",0);
Tclvar_int load_button_press ("load_button_press", 0);

static void handle_alpha_slider_change (vrpn_float64, void * userdata) {
  nmg_Graphics * g = (nmg_Graphics *) userdata;
  g->setAlphaSliderRange(alpha_slider_min, alpha_slider_max);
  //cause_grid_redraw(0.0, NULL);
}

static void handle_color_slider_change (vrpn_float64, void * userdata) {
  nmg_Graphics * g = (nmg_Graphics *) userdata;
  g->setColorSliderRange(color_slider_min, color_slider_max);
  //cause_grid_redraw(0.0, NULL);
}

static void handle_texture_scale_change (vrpn_float64 value, void * userdata) {
  nmg_Graphics * g = (nmg_Graphics *) userdata;
  g->setTextureScale(value);
  //cause_grid_redraw(0.0, NULL);
}

static void handle_rulergrid_offset_change (vrpn_float64, void * userdata) {
  nmg_Graphics * g = (nmg_Graphics *) userdata;
  g->setRulergridOffset(rulergrid_xoffset, rulergrid_yoffset);
  //cause_grid_redraw(0.0, NULL);
}

static void handle_rulergrid_scale_change (vrpn_float64, void * userdata) {
  nmg_Graphics * g = (nmg_Graphics *) userdata;
  g->setRulergridScale(rulergrid_scale);
  //cause_grid_redraw(0.0, NULL);
}

static void handle_x_value_change (vrpn_float64, void *) {
//fprintf(stderr, "In handle_x_value_change()\n");
  x_set_scale(x_min_value, x_max_value);
}


/// Handle the color change of the rulergrid
static void handle_rulergrid_color_change (vrpn_int32, void * userdata) {
  nmg_Graphics * g = (nmg_Graphics *) userdata;

  // MOVED to nmg_Graphics
  g->setRulergridColor((int)ruler_r, (int)ruler_g, (int)ruler_b);
  
//fprintf(stderr, "Ruler color %d %d %d\n",
//(int) ruler_r,(int) ruler_g, (int) ruler_b);
}

/// Handle the color change of the rulergrid
static void handle_rulergrid_opacity_change (vrpn_float64, void * userdata) {
  nmg_Graphics * g = (nmg_Graphics *) userdata;

  // MOVED to nmg_Graphics
  g->setRulergridOpacity(ruler_opacity);
}

static void handle_friction_slider_change (vrpn_float64, void * userdata) {
  nmg_Graphics * g = (nmg_Graphics *) userdata;

  g->setFrictionSliderRange(friction_slider_min, friction_slider_max);
  //cause_grid_redraw(0.0, NULL);
}

static void handle_bump_slider_change (vrpn_float64, void * userdata) {
  nmg_Graphics * g = (nmg_Graphics *) userdata;

  g->setBumpSliderRange(bump_slider_min, bump_slider_max);
  //cause_grid_redraw(0.0, NULL);
}

static void handle_buzz_slider_change (vrpn_float64, void * userdata) {
  nmg_Graphics * g = (nmg_Graphics *) userdata;

  g->setBuzzSliderRange(buzz_slider_min, buzz_slider_max);
  //cause_grid_redraw(0.0, NULL);
}


static void handle_compliance_slider_change (vrpn_float64, void * userdata) {
  nmg_Graphics * g = (nmg_Graphics *) userdata;

  g->setComplianceSliderRange(compliance_slider_min,
                                     compliance_slider_max);
  //cause_grid_redraw(0.0, NULL);
}

static void handle_recovery_time_change (vrpn_float64 value, void * userdata)
{
        vrpn_ForceDevice_Remote * fd = (vrpn_ForceDevice_Remote *) userdata;
	int ival;

	ival = (int) value;

        if (fd) {
                fd->setRecoveryTime(ival);

                printf("Recovery time: %d servo loop cycles\n",
                                        fd->getRecoveryTime());
        }
}

/// Handle the width change of the rulergrid
/// should be able to scale width less than 1, but I dont know 
/// how I should do that right now, so Im just implementing integer scaling
/// scaling is done with respect to original line width, which is 1
static void handle_ruler_widthx_change (vrpn_float64, void * userdata) {
  nmg_Graphics * g = (nmg_Graphics *) userdata;

  // MOVED to nmg_Graphics
  g->setRulergridWidths(ruler_width_x, ruler_width_y);
  //cause_grid_redraw(0.0, NULL);
}

static void handle_ruler_widthy_change (vrpn_float64, void * userdata) {
  nmg_Graphics * g = (nmg_Graphics *) userdata;

  // MOVED to nmg_Graphics
  g->setRulergridWidths(ruler_width_x, ruler_width_y);
  //cause_grid_redraw(0.0, NULL);
}


/// Change the number of pulse or scrape markers displayed
// Tom Hudson, 5 May 98

static void handle_markers_shown_change (vrpn_int32 new_value, void * /*userdata*/) {
  int v;

  v = (int) new_value;
  //  numMarkersShown = v;
  decoration->num_markers_shown = v;
  //cause_grid_redraw(0.0, NULL);
  graphics->causeGridRedraw();
}


/// Change the height of all (future?) scrape markers drawn.
// Aron Helser, 13 April 99
static void handle_markers_height_change (vrpn_int32 new_value, void * /*userdata*/) {
  int v;

  v = (int) new_value;
  microscope->state.scrapeHeight = v;
  //cause_grid_redraw(0.0, NULL);
}

//For importing from tube_foundry

static void handle_import_filename_change (const char *new_value, void *) { 
  import_filename_string = new_value;
}

static void handle_load_import_file_change (vrpn_int32 /*new_value*/, void * /*userdata*/) {
  if (object_list == NULL)
    object_list = new imported_obj_list;
  UTree* tree_ptr = &World;
  object_list->import_new_obj((char *) import_filename_string,tree_ptr);
}

static void handle_load_button_press_change (vrpn_int32 /*new_value*/, void * /*userdata*/ ) {
  char command[256];
  Tcl_Interp *tk_control_interp = get_the_interpreter();

  sprintf(command, "set tab_label %s", import_filename_string);
  if (Tcl_Eval(tk_control_interp, command) != TCL_OK) {
    fprintf(stderr, "Tcl_Eval failed in handle_load_button_press_change\n");
  }
}

// NANOX
static void getPeerRemote (const char * hostname) {
  char buf [256];
  char sfbuf [1024];

  sprintf(buf, "%s:%d", hostname, WellKnownPorts::collaboratingPeerServer);
  if (replayingInterface) {
    sprintf(sfbuf, "file:%s/SharedIFRemLog-%ld.stream", loggingPath,
            loggingTimestamp.tv_sec);
    collaboratingPeerRemoteConnection =
      vrpn_get_connection_by_name (sfbuf);
  } else {
    sprintf(sfbuf, "%s/SharedIFRemLog-%ld.stream", loggingPath,
            loggingTimestamp.tv_sec);
    collaboratingPeerRemoteConnection = vrpn_get_connection_by_name (buf,
             loggingInterface ? sfbuf : NULL,
             loggingInterface ? vrpn_LOG_INCOMING | vrpn_LOG_OUTGOING :
                        vrpn_LOG_NONE);
  }
}

static void handle_collab_machine_name_change
                   (const char * new_value,
                    void * /*userdata*/ )
{
    //char buf [256];
  char sfbuf [1024];

  if (!new_value || !strlen(new_value)) {
    // transitory excitement during startup
    return;
  }

  // Open the remote tracker object to follow the collaborator's hand
  sprintf(sfbuf, "%s/SharedIFRemLog-%ld.stream", loggingPath,
          loggingTimestamp.tv_sec);
  if (replayingInterface) {
    sprintf(collab_handTrackerName, "ccs0@file:%s", sfbuf);
    sprintf(collab_ModeName, "Cmode0@file:%s", sfbuf);
  } else {
    sprintf(collab_handTrackerName, "ccs0@%s:%d", new_value,
            WellKnownPorts::collaboratingPeerServer);
    sprintf(collab_ModeName, "Cmode0@%s:%d", new_value,
            WellKnownPorts::collaboratingPeerServer);

    //if (loggingInterface) {
      //sprintf(buf, "%s:%d", new_value,
              //WellKnownPorts::collaboratingPeerServer);
      //vrpn_get_connection_by_name (buf, sfbuf,
                                   //vrpn_LOG_INCOMING);
    //}
    getPeerRemote(new_value);  // make sure logging happens
  }

fprintf(stderr, "peer machine name: %s\n", new_value);
fprintf(stderr, "collab_ModeName: %s\n", collab_ModeName);

  vrpnHandTracker_collab[0] = new vrpn_Tracker_Remote(collab_handTrackerName);
  if (vrpnHandTracker_collab[0] != NULL) {
    vrpnHandTracker_collab[0]->register_change_handler
      ((void *)&V_TRACKER_FROM_HAND_SENSOR,
       handle_collab_sensor2tracker_change);
  }

  // Open the remote analog object to track the collaborator's mode
  vrpnMode_collab[0] = new vrpn_Analog_Remote(collab_ModeName);
  if (vrpnMode_collab[0] != NULL) {
    vrpnMode_collab[0]->register_change_handler
      (NULL, handle_collab_mode_change);
  }
}

static void handle_collab_machine_name_change2
                   (const char * new_value,
                    void * userdata)
{
  nmui_Component * uic = (nmui_Component *) userdata;
  char hnbuf [256];
  char buf [256];
  vrpn_int32 newConnection_type;
  vrpn_bool should_synchronize;

  if (!new_value || !strlen(new_value)) {
    // transitory excitement during startup
    return;
  }

  getPeerRemote(new_value);

  sprintf(buf, "%s:%d", new_value, WellKnownPorts::collaboratingPeerServer);
  if (collaboratingPeerRemoteConnection &&
      collaboratingPeerRemoteConnection->doing_okay()) {

    // XXX Only handles two-way case right now:  figures out who comes
    // first alphabetically, us or the peer.  To handle n-way either need
    // complete list of participating hosts at the beginning or need to
    // finish implementing serializer migration protocol.

    gethostname(hnbuf, 256);
fprintf(stderr, "## My name is %s, new peer is %s:  ", hnbuf, buf);
    if (strcmp(hnbuf, buf) < 0) {
fprintf(stderr, "serializer.\n");
      should_synchronize = VRPN_TRUE;
    } else {
fprintf(stderr, "client.\n");
      should_synchronize = VRPN_FALSE;
    }

    switch (should_synchronize) {
      case VRPN_TRUE:
        uic->addPeer(collaboratingPeerServerConnection, should_synchronize);
        break;
      case VRPN_FALSE:
        uic->addPeer(collaboratingPeerRemoteConnection, should_synchronize);
        break;
    }
//fprintf(stderr, "Called addPeer() on component for %s.\n", buf);
    uic->initializeConnection(collaboratingPeerRemoteConnection);
//fprintf(stderr, "## Called initializeConnection(%ld) on component for %s.\n",
//collaboratingPeerRemoteConnection, buf);


    // register callbacks if need be - HACK assume necessary
    // IF this connection isn't valid, or is dropped and reconnected later,
    // this callback renegotiates the sender and type IDs.
    newConnection_type = collaboratingPeerRemoteConnection->
               register_message_type (vrpn_got_connection);
    collaboratingPeerRemoteConnection->register_handler
      (newConnection_type, nmui_Component::handle_reconnect, uic);
  }
}

static void handle_collab_machine_name_change3
                   (const char * new_value,
                    void * userdata)
{
  nmui_PlaneSync * ps;

  ps = (nmui_PlaneSync *) userdata;

//fprintf(stderr, "In handle_collab_machine_name_change3 to %s.\n", new_value);

  if (!new_value || !strlen(new_value)) {
    // transitory excitement during startup
    return;
  }

  getPeerRemote(new_value);

  // plane sync object also needs to know the name of collaborator
  // to uniquely identify planes created.
  ps->addPeer(collaboratingPeerRemoteConnection, new_value);

  if (nM_coord_change_server) {
    nM_coord_change_server->bindConnection(collaboratingPeerRemoteConnection);
  }
}

void updateRulergridOffset (void) {
 if (rulergrid_position_line && rulergrid_enabled) {
   rulergrid_xoffset = (vrpn_float64) measureRedX;
   rulergrid_yoffset = (vrpn_float64) measureRedY;
   graphics->setRulergridOffset(rulergrid_xoffset, rulergrid_yoffset);
   graphics->causeGridRedraw();
 }
}

void updateRulergridAngle (void) {
 if (rulergrid_orient_line && rulergrid_enabled) {
   float xdiff;
   float ydiff;
   float hyp;
   xdiff = measureGreenX - rulergrid_xoffset;
   ydiff = measureGreenY - rulergrid_yoffset;
   hyp = sqrt(xdiff * xdiff + ydiff * ydiff);
   if (hyp == 0.0f) { hyp = 1.0f; }
   if (ydiff >= 0.0f) {
     // In quadrant 1 or 2
     rulergrid_angle = acos(xdiff / hyp) * 180.0f / M_PI;
   } else if (ydiff < 0.0f) {
     // In quadrant 3 or 4
     rulergrid_angle = 360 - (acos(xdiff / hyp) * 180.0f / M_PI);
   }
   graphics->setRulergridAngle(rulergrid_angle);
   graphics->causeGridRedraw();

 }
}

static void handle_rulergridPositionLine_change (vrpn_int32, void *) {
  updateRulergridOffset();
  updateRulergridAngle();  // Do we want this?
}

static void handle_rulergridOrientLine_change (vrpn_int32, void *) {
  updateRulergridAngle();
}


/// Updating both X and Y position of the line at the same time
/// doesn't work - it locks down one coord. This semaphor prevents that. 
static vrpn_bool ignoreCollabMeasureChange = 0;

// NANOX
/// Line position has changed in tcl (probably due to update from
/// collaborator) so change the position onscreen.
static void handle_collab_measure_change (vrpn_float64 /*newValue*/,
                                          void * userdata) {
    // Ignore some changes caused by tcl variables. 
    if (ignoreCollabMeasureChange) return;

  BCPlane * heightPlane = dataset->inputGrid->getPlaneByName
                 (dataset->heightPlaneName->string());
  if (!heightPlane) {
    fprintf(stderr, "Couldn't find height plane named %s.\n",
            dataset->heightPlaneName->string());
  }
  int whichLine = (int) userdata;   // hack to get the data here

  switch (whichLine) {

    case 0:
//fprintf(stderr, "Moving RED line due to Tcl change.\n");
     decoration->red.moveTo(measureRedX, measureRedY, heightPlane);
     // DO NOT doCallbacks()
     updateRulergridOffset();
     break;

    case 1:
//fprintf(stderr, "Moving GREEN line due to Tcl change.\n");
     decoration->green.moveTo(measureGreenX, measureGreenY, heightPlane);
     // DO NOT doCallbacks()
     updateRulergridAngle();
     break;

    case 2:
//fprintf(stderr, "Moving BLUE line due to Tcl change.\n");
     decoration->blue.moveTo(measureBlueX, measureBlueY, heightPlane);
     // DO NOT doCallbacks()
     break;
  }
}

// NANOX
/** If the user changes the measure line positon, update Tcl variables.
If we are collaborating, this will update our collaborator.
Because we want to set both the X and Y position of the line at 
the same time, we explicitly tell the other handler to ignore
the change we make to X, and pay attention to the change to Y. 

doMeasure() in interaction.c triggers handle_collab_measure_move.
Through the magic of Tcl_Linkvar/Tcl_Netvar, this will take care of
any necessary network synchronization or collaboration, and at the end
will call handle_collab_measure_change(), above, which executes the
final moveTo() on each nmb_Line and updates rulergrid parameters if
necessary.
*/
static void handle_collab_measure_move (float x, float y,
                                          void * userdata) {
  int whichLine = (int) userdata;   // hack to get the data here

  // TODO - a message needs to be sent so that this timer
  // gets unblocked!
  collaborationTimer.block(collaborationTimer.getListHead());

  switch (whichLine) {
    case 0:
//  fprintf(stderr, "Moving RED line , change Tcl .\n");
	ignoreCollabMeasureChange = VRPN_TRUE;
	measureRedX = x;
	ignoreCollabMeasureChange = VRPN_FALSE;
	measureRedY = y;
     break;
    case 1:
//fprintf(stderr, "Moving GREEN line , change Tcl .\n");
	ignoreCollabMeasureChange = VRPN_TRUE;
	measureGreenX = x;
	ignoreCollabMeasureChange = VRPN_FALSE;
	measureGreenY = y;
     // DO NOT doCallbacks()
     break;
    case 2:
//fprintf(stderr, "Moving BLUE line , change Tcl .\n");
	ignoreCollabMeasureChange = VRPN_TRUE;
	measureBlueX = x;
	ignoreCollabMeasureChange = VRPN_FALSE;
	measureBlueY = y;
     // DO NOT doCallbacks()
     break;
  }
}


// NANOX
/// Callback to button which rewinds the stream file to the beginning.
static void handle_rewind_stream_change (vrpn_int32 /*new_value*/, 
					     void * /*userdata*/)
{
    // rewind stream only if variable is set to 1 -> button is pressed.
    if (rewind_stream != 1) return;

    printf("Restarting stream from the beginning\n");

///*
#ifdef USE_VRPN_MICROSCOPE
    if (vrpnLogFile)
	vrpnLogFile->reset();

#else
    microscope->RestartStream();

#endif
//*/
  //set_stream_time = 0;
    if (ohmmeterLogFile)
	ohmmeterLogFile->reset();
    if (vicurveLogFile)
	vicurveLogFile->reset();

//fprintf(stderr, "Handle_rewind_stream_change setting to 0.\n");
    rewind_stream = 0;  // necessary

}


// NANOX
// synchronization UI handlers
/* UNUSED
// Checkbox - if checked, request that the peer keep us synced;
//   if unchecked, stop keeping synced.
// Currently assumes that only the most recently added peer is
//   "valid";  others are a (small?) memory/network leak.
static void handle_synchronize_change (vrpn_int32 value, void * userdata) {
  nmui_Component * sync = (nmui_Component *) userdata;
  int s;

  switch (value) {
    case 0:
      // stop synchronizing;  use local state (#0)
      s = 0;
      break;
    default:
      // use shared state (#1)
      //s = 1;  // SYNC-ROBUSTNESS
      s = sync->numPeers();
fprintf(stderr, "Synchronizing with peer #%d.\n", s);
      break;
  }

  sync->syncReplica(s);
//fprintf(stderr, "++ Synchronized to replica #%d.\n", sync->synchronizedTo());

}

// Button - if pressed, request immediate sync.
// (If synchronize_stream is checked, this is meaningless.)
// Currently assumes that only the most recently added peer is
//   "valid";  others are a (small?) memory/network leak.
static void handle_get_sync (vrpn_int32, void * userdata) {
  nmui_Component * sync = (nmui_Component *) userdata;
  int c;
  // Christmas sync
  // handles 2-way;  may handle n-way sync
  switch (sync->synchronizedTo()) {
    case 0:
      // copy shared state (#1) into local state (#0)
      //c = 1;  // SYNC-ROBUSTNESS
      c = sync->numPeers();
fprintf(stderr, "Copying peer #%d.\n", c);
      break;
    default:
      // copy local state (#0) into shared state (#1)
      c = 0;
      break;
  }
  sync->copyReplica(c);
//fprintf(stderr, "++ Copied inactive replica (#%d).\n", c);
}
*/
static int handle_timed_sync_request (void *);

struct sync_plane_struct {
    nmui_Component * component;
    nmui_PlaneSync * planesync;
};

/** Checkbox - if checked, request that the peer keep us synced;
  if unchecked, stop keeping synced.
Currently assumes that only the most recently added peer is
  "valid";  others are a (small?) memory/network leak.
*/
static void handle_synchronize_timed_change (vrpn_int32 value,
                                             void * userdata) {
  sync_plane_struct * stuff = (sync_plane_struct *)userdata;
  nmui_Component * sync = stuff->component;
  nmui_PlaneSync * plane_sync = stuff->planesync;

fprintf(stderr, "++ In handle_synchronized_timed_change() to %d\n", value);

  // First write out the current values of any volatile variables.
  // There is some risk of this doing the wrong thing, since we're
  // likely to be slightly out of sync with our replica and they may
  // get this as an update message, causing their stream to jump
  // backwards - HACK XXX.  Correct behavior may need to be implemented
  // at a lower level that knows whether or not we're the synchronizer.
  // ALL of the sync code was written for optimism, and requires some
  // tougher semantics to use centralized serialization and
  // single-shared-state.
  handle_timed_sync_request(NULL);

  switch (value) {
    case 0:
      // stop synchronizing;  use local state (#0)
fprintf(stderr, "++   ... stopped synchronizing.\n");
      sync->syncReplica(0);
      plane_sync->queueUpdates();
      graphics->enableCollabHand(VRPN_FALSE);
      nM_coord_change_server->stopSync();
      isSynchronized = VRPN_FALSE;
      break;
    default:
      // use shared state (#1)
fprintf(stderr, "++   ... sent synch request to peer.\n");

      // You'd think that this call to block() wouldn't need to be explicitly
      // unblocked, since we're going to get a syncComplete message from
      // the peer, but it does - we need to know what index to unblock
      // after the syncComplete, which requres an additional message.
      collaborationTimer.block(collaborationTimer.getListHead());

      sync->requestSync();
      sync->d_maintain = VRPN_TRUE;
      if (nM_coord_change_server->peerIsSynchronized()) {
        graphics->enableCollabHand(VRPN_TRUE);
      }
      nM_coord_change_server->startSync();
      isSynchronized = VRPN_TRUE;
      // We defer the syncReplica until after the requestSync() completes.
      break;
  } 

}

static void handle_peer_sync_change (void * userdata, vrpn_bool value) {

fprintf(stderr, "handle_peer_sync_change called, value %d\n",value);
  if (isSynchronized && value) {  // both synchronized
    graphics->enableCollabHand(VRPN_TRUE);
  } else {
    graphics->enableCollabHand(VRPN_FALSE);
  }

}

/// Linked to button in tcl UI. If pressed, copy the shared state to
/// the private state.
static void handle_copy_to_private (vrpn_int32 value, void * userdata) {
  sync_plane_struct * stuff = (sync_plane_struct *)userdata;
  nmui_Component * sync = stuff->component;
  nmui_PlaneSync * plane_sync = stuff->planesync;

  if (!sync->synchronizedTo()) {
      // we are local. We need to get current data for shared state.

    // a copyReplica needs to be deferred until the new data arrives
    // The right way to do this is probably to have a Component's
    // sync handler pack a "syncComplete" message AFTER all the
    // callbacks have been triggered (=> the sync messages are marshalled
    // for VRPN), so when that arrives we know the sync is complete and
    // we can issue a copyReplica()

    // You'd think that this call to block() wouldn't need to be explicitly
    // unblocked, since we're going to get a syncComplete message from
    // the peer, but it does - we need to know what index to unblock
    // after the syncComplete, which requres an additional message.
    collaborationTimer.block(collaborationTimer.getListHead());

    sync->requestSync();
    sync->d_maintain = VRPN_FALSE;
fprintf(stderr, "++ In handle_copy_to_private()sent synch request\n");
  } else {
      // get up to date stream time. 
      handle_timed_sync_request(NULL);

      // we are shared, copy to local state immediately.
      // shared state is in the replica from the most recent peer.
      sync->copyFromToReplica(sync->numPeers(), 0);
fprintf(stderr, "++ In handle_copy_to_private() copied immediately.\n");
    plane_sync->acceptUpdates();
    plane_sync->queueUpdates();

  }

}

/// Linked to button in tcl UI. If pressed, copy the private state to
/// the shared state.
static void handle_copy_to_shared (vrpn_int32 value, void * userdata) {
  sync_plane_struct * stuff = (sync_plane_struct *)userdata;
  nmui_Component * sync = stuff->component;
  nmui_PlaneSync * plane_sync = stuff->planesync;

  if (!sync->synchronizedTo()) {
      // we are local. We can copy to shared state immediately
      // shared state is in the replica from the most recent peer.
      sync->copyFromToReplica(0, sync->numPeers());
fprintf(stderr, "++ In handle_copy_to_shared() request sync.\n");

      // we also want to get any planes which might be from the shared state 
      plane_sync->acceptUpdates();
      plane_sync->queueUpdates();
 
  } else {
      // we are shared, want to copy from local
      // We know the current shared state, because we are shared,
      // so copy immediately.
      // shared state is in the replica from the most recent peer.
      sync->copyFromToReplica(0, sync->numPeers());
fprintf(stderr, "++ In handle_copy_to_shared() copy immediately.\n");
      // Any planes created in local state will already have been copied to
      // shared state, so no need to sync planes.

  }

}

/** Button - if pressed, request immediate sync.
(If synchronize_stream is checked, this is meaningless.)
Currently assumes that only the most recently added peer is
  "valid";  others are a (small?) memory/network leak.
*/
static void handle_timed_sync (vrpn_int32 value, void * userdata) {
  sync_plane_struct * stuff = (sync_plane_struct *)userdata;
  nmui_Component * sync = stuff->component;
  nmui_PlaneSync * plane_sync = stuff->planesync;
  int copyFrom = !sync->synchronizedTo();

  //// only run once
  //if (!value) {
    //return;
  //}

  if (copyFrom) {

    // a copyReplica needs to be deferred until the new data arrives
    // The right way to do this is probably to have a Component's
    // sync handler pack a "syncComplete" message AFTER all the
    // callbacks have been triggered (=> the sync messages are marshalled
    // for VRPN), so when that arrives we know the sync is complete and
    // we can issue a copyReplica()

    // You'd think that this call to block() wouldn't need to be explicitly
    // unblocked, since we're going to get a syncComplete message from
    // the peer, but it does - we need to know what index to unblock
    // after the syncComplete, which requres an additional message.
    collaborationTimer.block(collaborationTimer.getListHead());

    sync->requestSync();
    sync->d_maintain = VRPN_FALSE;
fprintf(stderr, "++ In handle_timed_sync() to %d;  "
"sent synch request to peer.\n", copyFrom);
  } else {

    sync->copyReplica(copyFrom);
fprintf(stderr, "++ In handle_timed_sync() to %d;  copied immediately.\n",
copyFrom);
    plane_sync->acceptUpdates();
    plane_sync->queueUpdates();

  }

}

static int handle_timed_sync_request (void *) {
  // Write the current elapsed time of the stream
  // into set_stream_time / replica[0] without messing up
  // our playback.

  set_stream_time = decoration->elapsedTime;

fprintf(stderr, "++ In handle_timed_sync_request() at %ld seconds;  "
"wrote data into replica.\n", decoration->elapsedTime);

  return 0;
}

static int handle_timed_sync_complete (void * userdata) {
  sync_plane_struct * stuff = (sync_plane_struct *)userdata;
  nmui_Component * sync = stuff->component;
  nmui_PlaneSync * plane_sync = stuff->planesync;

  //int useReplica = !sync->synchronizedTo();
  // requestSync() is only called, and so this will only be generated,
  // when we are going to the shared replica.
  //int useReplica = 1;  // SYNC-ROBUSTNESS
  int useReplica = sync->numPeers();

fprintf(stderr, "++ In handle_timed_sync_complete();  "
"getting data from replica %d.\n", useReplica);

  // Once we have the *latest* state of time-depenedent values,
  // update with that.

  if (sync->d_maintain) {

    // Make sure we save the state of any volatile variables -
    // if latency is high the previous save will be off by a few
    // seconds (which we could forget).
    handle_timed_sync_request(NULL);

    // Start synchronizing planes and create any planes from 
    // the new state.
    plane_sync->acceptUpdates();
    sync->syncReplica(useReplica);
//fprintf(stderr, "++   ... synched.\n");
  } else {
    sync->copyReplica(useReplica);

    // Create any planes from copied state, but go back to 
    // queueing new plane creations.
    plane_sync->acceptUpdates();
    plane_sync->queueUpdates();
    
//fprintf(stderr, "++   ... copied.\n");
  }

  return 0;
}


// NANOX
static void handle_center_pressed (vrpn_int32 newValue, void * /*userdata*/) {
  if (!newValue) {
    return;
  }

  handleCharacterCommand("^", &dataset->done, 0);
  tcl_center_pressed = 0;
}


static void handle_set_stream_time_change (vrpn_int32 /*value*/, void *) {
//fprintf(stderr, "handle_set_stream_time_change to %d (flag %d).\n",
//(vrpn_int32) set_stream_time, (vrpn_int32) set_stream_time_now);

  if (set_stream_time_now == 0) return;

  struct timeval newStreamTime;
  // BUG BUG BUG
  // If we're at x.y and set stream time to x.0 we'll replay the
  // whole stream file?
  newStreamTime.tv_sec = set_stream_time;
  newStreamTime.tv_usec = 999999L;
#ifdef USE_VRPN_MICROSCOPE
  if (vrpnLogFile) {
    vrpnLogFile->play_to_time(newStreamTime);
  }
#else
  microscope->SetStreamToTime(newStreamTime);
  // TCH 9 Feb 2000
  // updt_display() will set stm_new_frame IFF time has elapsed (given
  // the current rate of playback), but not clear it.
  // By setting it here we force the next call into the microscope
  // to read from the stream file, even if playback is paused -
  // this lets us change time settings while paused.
  stm_new_frame = VRPN_TRUE;
#endif
//fprintf(stderr, "Set stream time to %d.\n", time);
  set_stream_time_now = 0; 
}

static void handle_shiny_change (vrpn_int32 new_value, void * userdata) {
  nmg_Graphics * g = (nmg_Graphics *) userdata;

  g->setSpecularity(new_value);
}


static void handle_diffuse_change (vrpn_float64 new_value, void * userdata) {
  nmg_Graphics * g = (nmg_Graphics *) userdata;

  g->setDiffusePercent(new_value);
}

static void handle_surface_alpha_change (vrpn_float64 new_value, void * userdata) {
  nmg_Graphics * g = (nmg_Graphics *) userdata;

  g->setSurfaceAlpha(new_value);
}

static void handle_specular_color_change (vrpn_float64 new_value, void * userdata) {
  nmg_Graphics * g = (nmg_Graphics *) userdata;

  g->setSpecularColor(new_value);
}

static void handle_sphere_scale_change (vrpn_float64 new_value, void * userdata) {
  nmg_Graphics * g = (nmg_Graphics *) userdata;

  g->setSphereScale(new_value);
}



// Genetic Textures -- Alexandra Bokinsky
//
/// Called when the genetic_textures_enabled toggle switch on the main
/// miscroscape tcl window is pressed, either on or off.
static void handle_genetic_textures_selected_change(vrpn_int32 value, void *userdata)
{
  nmg_Graphics * g = (nmg_Graphics *) userdata;
  if (value) {
    disableOtherTextures(GENETIC);
    g->setTextureMode(nmg_Graphics::GENETIC, 
		      nmg_Graphics::MANUAL_REALIGN_COORD);
  } else {
    if (g->getTextureMode() == nmg_Graphics::GENETIC) {
      g->setTextureMode(nmg_Graphics::NO_TEXTURES,
                        nmg_Graphics::MANUAL_REALIGN_COORD);
    }
  }
  //cause_grid_redraw(0.0, NULL);
}

// This button creates a window with the genetic textures parameters
// so the user may select which data sets to use dynamically.
// All current datasets are listed as check boxes. The user
// simply check which data to send...
// The function to display the window is sent to the tcl interpreter
// from this function, because all the data sets, and the names of the
// data sets are not know until run-time, and may change during execution.
// (Also, the '.' in the dataset names must be removed from the 
// interpreted tcl commands because tcl interprets '.' )
static void handle_genetic_textures_set_parameters (vrpn_int32 , void *)
{
  char    command[256];
  int i;

  Tcl_Interp *tk_control_interp = get_the_interpreter();

  sprintf(command, "toplevel .genetic_data_sets");
  if (Tcl_Eval(tk_control_interp, command) != TCL_OK) {
    fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command,
	    tk_control_interp->result);
  }


  sprintf(command, "button .genetic_data_sets.button1 -text \"Close Genetic Texture Panel\" -bg red -command \"destroy .genetic_data_sets\"");
  if (Tcl_Eval(tk_control_interp, command) != TCL_OK) {
    fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command,
	    tk_control_interp->result);
  }

  sprintf(command, "pack .genetic_data_sets.button1");
  if (Tcl_Eval(tk_control_interp, command) != TCL_OK) {
    fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command,
	    tk_control_interp->result);
  }


  // Get a new checklist.
  genetic_checklist = new Tclvar_checklist(".genetic_data_sets");
  if (genetic_checklist == NULL) {
    fprintf(stderr,"Genetic Set Parameters(): Can't make checklist\n");
    return;
  }

  nmb_ListOfStrings *imageNames = dataset->dataImages->imageNameList();
  char *name;
  for (i = 0; i < imageNames->numEntries(); i++) {
    name = new char [strlen(imageNames->entry(i)) + 1];
    strcpy( name, imageNames->entry(i));
    char *c = strchr( name, '.' );
    if ( c ){
      *c = '_';
    }
//    printf( "%s\n", name );
    genetic_checklist->Add_checkbox( name, 1 );
    delete [] name;  
  }

  sprintf(command, "button .genetic_data_sets.button -text \"Send data now\" -bg $fc -command \"set gen_send_data 1\"");
  if (Tcl_Eval(tk_control_interp, command) != TCL_OK) {
    fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command,
	    tk_control_interp->result);
  }

  sprintf(command, "pack .genetic_data_sets.button");
  if (Tcl_Eval(tk_control_interp, command) != TCL_OK) {
    fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command,
	    tk_control_interp->result);
  }

}

// Checks which datasets are selected on the checklist and
// passes this information to the nmg_Graphics which sends it
// to the genetic alg. Only the selected datasets are send...
//
static void handle_genetic_send_data (vrpn_int32 , void * userdata)
{
  nmg_Graphics * g = (nmg_Graphics *) userdata;
  if ( genetic_checklist ) {
    int number_of_variables = 0;

    nmb_ListOfStrings *imageNames = dataset->dataImages->imageNameList();
    char *name;
    char **variable_list = new char*[imageNames->numEntries()];
    for (int i = 0; i < imageNames->numEntries(); i++) {
      name = new char [strlen(imageNames->entry(i)) + 1];
      strcpy( name, imageNames->entry(i));
      char *c = strchr( name, '.' );
      if ( c ){
        *c = '_';
      }
      if ( !strcmp( name, genetic_checklist->Checkbox_name( i ) ) &&
           -1 != genetic_checklist->Is_set( i ) ) {
        variable_list[number_of_variables] = new
          char[ strlen( imageNames->entry(i) ) + 1];
        strcpy( variable_list[number_of_variables],
                imageNames->entry(i) );
        number_of_variables++;
      }
      delete [] name;
    }

    g->sendGeneticTexturesData( number_of_variables, variable_list );
    //cause_grid_redraw(0.0, NULL);
  }
}


/// Realigning Textures Interactively:
/// Called when the realign_textures_enabled toggle switch on the main
/// miscroscape tcl window is pressed, either on or off.
static void handle_display_textures_selected_change(vrpn_int32 value, void *userdata)
{
  nmg_Graphics * g = (nmg_Graphics *) userdata;
//  printf("handle_display_textures_selected_change, %d\n", value);
  if (value){
    disableOtherTextures(MANUAL_REALIGN);
    g->setTextureMode(nmg_Graphics::COLORMAP,
		      nmg_Graphics::MANUAL_REALIGN_COORD);
  } else {
    if (g->getTextureMode() == nmg_Graphics::COLORMAP) {
      g->setTextureMode(nmg_Graphics::NO_TEXTURES,
		        nmg_Graphics::MANUAL_REALIGN_COORD);
    }
  }
  //cause_grid_redraw(0.0, NULL);
  realign_textures_enabled = 0;
  set_realign_center = 0;
}

static void handle_realign_textures_selected_change(vrpn_int32 value, void *)
{
//  printf("handle_realign_textures_selected_change, %d\n", value);
  if (realign_textures_enabled != value)
	realign_textures_enabled = value;
  else
	return;
  if ( (value == 0) && set_realign_center ) {
    mode_change = 1;
    user_mode[0] = USER_GRAB_MODE;
    set_realign_center = 0;
  }
}

static void handle_set_realign_center_change (vrpn_int32 value, void * userdata)
{
//  printf("handle_set_realign_center_change, %d\n", value);
  nmg_Graphics * g = (nmg_Graphics *) userdata;
  if ( set_realign_center != value) {
    set_realign_center = value;
  } else {
    return;
  }
  
  mode_change = 1;	/* Will make icon change */
  if ( value == 1 ) {
    user_mode[0] = USER_CENTER_TEXTURE_MODE;
    g->setSphereScale( ( dataset->inputGrid->maxX() -
				dataset->inputGrid->minX()    ) / 100.0 );
    g->setTextureCenter( 0, 0 );
  }
  else {
    user_mode[0] = USER_GRAB_MODE;
  }
}

static void handle_texture_dataset_change (const char *, void * userdata)
{
  nmg_Graphics * g = (nmg_Graphics *) userdata;
  nmb_Image *im = dataset->dataImages->getImageByName
                            (texturePlaneName.string());

  if (im != NULL) {
    float range = im->maxValue() - im->minValue();
    if ( im->minAttainableValue() > ( im->minValue() -range)) 
      realign_textures_slider_min_limit = im->minAttainableValue();
    else 
      realign_textures_slider_min_limit = (im->minValue() -range);
    if ( im->maxAttainableValue() < ( im->maxValue() +range)) 
      realign_textures_slider_max_limit = im->maxAttainableValue();
    else 
      realign_textures_slider_max_limit = (im->maxValue() +range);
    
    realign_textures_slider_min = im->minValue();
    realign_textures_slider_max = im->maxValue();
  }

  g->setRealignTextureSliderRange( realign_textures_slider_min,
					  realign_textures_slider_max );
  /* XXXXXXX - NASTY HACK:
    this hack is related to a problem with how we link C variables with
    user interface variables:
     Sometimes we want to change the user interface variable but not
        allow the user interface callback to be called (for example, when
        we want to change a bunch of variables in the user interface as
        an initialization before taking some action such as generating
        a texture
  */

  printf("creating texture: %s\n", texturePlaneName.string());
  g->createRealignTextures( texturePlaneName.string() );
}

static void handle_texture_conversion_map_change(const char *, void * userdata)
{
  nmg_Graphics * g = (nmg_Graphics *) userdata;
  g->setRealignTexturesConversionMap(textureConversionMapName.string(),
					     colorMapDir );
  g->createRealignTextures( texturePlaneName.string() );
}

static void handle_realign_textures_slider_change (vrpn_float64, void * userdata) {
  nmg_Graphics * g = (nmg_Graphics *) userdata;

  g->setRealignTextureSliderRange( realign_textures_slider_min,
					  realign_textures_slider_max );
  g->createRealignTextures( texturePlaneName.string() );
}

static void handle_realign_plane_name_change (const char *, void * userdata) {
  nmg_Graphics * g = (nmg_Graphics *) userdata;
   if (strlen(newRealignPlaneName.string()) > 0) {

     g->computeRealignPlane( texturePlaneName.string(),
				    newRealignPlaneName.string() );
     
     microscope->state.data.inputPlaneNames.addEntry(newRealignPlaneName);
     
     newRealignPlaneName = "";
   }
}



/// Handle setting or clearing of ruler selected
static	void	handle_rulergrid_selected_change(vrpn_int32 value, void *userdata)
{
  nmg_Graphics * g = (nmg_Graphics *) userdata;
  if (value) {
    disableOtherTextures(RULERGRID);
    g->setTextureMode(nmg_Graphics::RULERGRID,
		      nmg_Graphics::RULERGRID_COORD);
  } else {
    if (g->getTextureMode() == nmg_Graphics::RULERGRID) {
      g->setTextureMode(nmg_Graphics::NO_TEXTURES,
		        nmg_Graphics::RULERGRID_COORD);
    }
  }
//  cause_grid_redraw(0.0, NULL);
}

/// Handle rulergrid angle change -- recompute sin() and cos() of angle
static	void	handle_rulergrid_angle_change (vrpn_float64 value, void * userdata) {

  nmg_Graphics * g = (nmg_Graphics *) userdata;
  g->setRulergridAngle(value);
}

static void handle_replay_rate_change (vrpn_int32 value, void *) {

//  instream_rate = (int) value;
//  decoration->rateOfTime = instream_rate;
//  printf("Read rate: %d times original\n",instream_rate);

  decoration->rateOfTime = (int) value;
  printf("Read rate: %d times original\n", decoration->rateOfTime);

#ifdef USE_VRPN_MICROSCOPE

  if (vrpnLogFile) {
     vrpnLogFile->set_replay_rate(decoration->rateOfTime);
  }

#else

  if (microscope) {
    microscope->SetStreamReplayRate(decoration->rateOfTime);
  }

#endif

  if (ohmmeterLogFile) {
     ohmmeterLogFile->set_replay_rate(decoration->rateOfTime);
  }
  if (vicurveLogFile) {
     vicurveLogFile->set_replay_rate(decoration->rateOfTime);
  }

}	


static void handle_contour_color_change (vrpn_int32, void * userdata) {

  nmg_Graphics * g = (nmg_Graphics *) userdata;
  g->setContourColor(contour_r, contour_g, contour_b);

//fprintf(stderr, "Contour color now %d, %d, %d.\n",
//(int) contour_r, (int) contour_g, (int) contour_b);
}

static void     handle_contour_width_change (vrpn_float64, void * userdata) {

  nmg_Graphics * g = (nmg_Graphics *) userdata;
  g->setContourWidth(contour_width);
}

static	void	handle_alpha_dataset_change (const char *, void * userdata)
{
  nmg_Graphics * g = (nmg_Graphics *) userdata;
	BCPlane	* plane = dataset->inputGrid->getPlaneByName
                             (dataset->alphaPlaneName->string());

        printf("handle_alpha_dataset_change\n");
	if (plane != NULL) {
		alpha_slider_min_limit = plane->minAttainableValue();
		alpha_slider_min = plane->minValue();
		alpha_slider_max_limit = plane->maxAttainableValue();
		alpha_slider_max = plane->maxValue();
		g->setAlphaSliderRange(alpha_slider_min,
                                              alpha_slider_max);
	
	        g->setTextureMode(nmg_Graphics::ALPHA,
			  nmg_Graphics::RULERGRID_COORD);
//fprintf(stderr, "Setting pattern map name to %s\n",
//dataset->alphaPlaneName->string());
  // XXX REDUNDANT
                g->setPatternMapName(dataset->alphaPlaneName->string());
                g->setAlphaPlaneName(dataset->alphaPlaneName->string());

        } else {
	  fprintf(stderr, "Warning, couldn't find alpha plane: %s\n",
		dataset->alphaPlaneName->string());
	  fprintf(stderr, "  turning alpha texture off\n");
          if (g->getTextureMode() == nmg_Graphics::ALPHA) {
            g->setTextureMode(nmg_Graphics::NO_TEXTURES,
                              nmg_Graphics::RULERGRID_COORD);
          }
        }
}


static void handle_x_dataset_change (const char *, void *) {    
        BCPlane	*plane = dataset->inputGrid->getPlaneByName
                                      (xPlaneName.string());
	
#ifndef __CYGWIN__
        int retval;
        int x, y;
#endif
	
        if(oldxPlane!=NULL) {
           // remove the old call back
#ifndef __CYGWIN__
	   oldxPlane->remove_callback(update_xdisp_on_plane_change,NULL);
	   freeWindow();
#endif
        }
	oldxPlane=plane;

	if(plane==NULL ) {
	   x_min_scale = x_min_value = 0;
	   x_max_scale = x_max_value = 1;
	   xenable  = 0;
	}
        else {
#ifdef __CYGWIN__
	fprintf(stderr, "This feature is not implemented in windows NT port\n");
#else
	  xenable = 1;
          x_init(/*myargv*/ NULL);

	  synchronize_xwin(False);   /* Allow asynchronous writes */

	  x_min_scale = plane->minAttainableValue();
	  x_max_scale = plane->maxAttainableValue();
	  x_min_value = plane->minValue();
	  x_max_value = plane->maxValue();
	  x_set_scale(x_min_value, x_max_value);
	  for (x = 0; x < dataset->inputGrid->numX(); x++) {
	    for (y = 0; y < dataset->inputGrid->numY(); y++) {
		x_put_value(x,y,plane->value(x,y));
	    }
	  }
	  flush_xwin();		/* Flush the last output */
	  synchronize_xwin(True);   /* Disable asynchronous writes*/
        
	  // Put in a callback that will update the X window whenever a
	  // point changes in this plane.
	
	  retval = plane->add_callback(update_xdisp_on_plane_change, NULL);
	  if (retval) {
            fprintf(stderr, "handle_x_dataset_change:  "
                            "Couldn't register callback for X plane\n");
          }
else fprintf(stderr, "Added X plane update callback\n");
#endif
	}
}

static	void	handle_colormap_change (const char *, void * userdata) {

  nmg_Graphics * g = (nmg_Graphics *) userdata;
  g->setColorMapName(dataset->colorMapName->string());

  // If CUSTOM is selected, set the color map pointer to
  // NULL so that the gl code will go back to using the old
  // method of computing color.  XXX Integrate this better,
  // so that we always use a color map but that it is set to
  // the min/max colors when we are using CUSTOM.
  if (strcmp(dataset->colorMapName->string(), "CUSTOM") == 0) {
          curColorMap = NULL;
  } else {
          colorMap.load_from_file(dataset->colorMapName->string(), colorMapDir);
          curColorMap = &colorMap;
  }

}

/** See if the user has given a name to the export plane other
 than "".  If so, we should export a file and set the value
 back to "". If there are any errors, report them and leave name alone. 
*/
static	void	handle_exportFileName_change (const char *, void *)
{

    if (strlen(newExportFileName) > 0) {
      // Find which plane we are going to export.
      nmb_Image *im = dataset->dataImages->getImageByName
                                      (exportPlaneName.string());
      if (im == NULL) {
	fprintf(stderr,"handle_exportFileName_change:: Couldn't get image %s\n",
		exportPlaneName.string());
	return;
      }

      // find out which type of file we are writing, and write it.

      FILE *file_ptr;
      // "wb" stands for write binary - so it should work on PC platforms, too.
      file_ptr=fopen(newExportFileName.string(),"wb");
      if(file_ptr==NULL){
	  fprintf(stderr, "handle_exportFileName_change::"
                " Error in opening file %s\n",newExportFileName.string());
		return;
      }

      if (im->exportToFile(file_ptr, exportFileType.string())) {
	fprintf(stderr, 
		"handle_exportFileName_change:: Error in writing file %s\n",
		newExportFileName.string());
	return;
      }

    }
    newExportFileName = "";
}

/** Why do we need to do anything here? This just specifies
 which plane to export if we set newExportPlaneName to something...
 I see - different planes can have different lists of 
 export types. So if we change planes, we may need to change
 the list of export types. 
*/
static  void    handle_exportPlaneName_change (const char *, void *ud)
{
    nmb_Dataset *d = (nmb_Dataset *)ud;

    nmb_Image *im = d->dataImages->getImageByName(
					(const char *)exportPlaneName);
    if (!im) {
	fprintf(stderr, "handle_exportPlaneName_change:"
		" Error, couldn't find image: %s\n", 
		(const char *)exportPlaneName);
	return;
    } 

    nmb_ListOfStrings *formatList = im->exportFormatNames();
    export_formats.copyList(formatList);
    // check if its okay to leave exportFileType as it was
    // (i.e., if exportFileType is present in the new list)
    /* optionmenu widget should take care of this...
    for (int i = 0; i < formatList->numEntries(); i++) {
	if (strcmp((const char *)exportFileType, formatList->entry(i)) == 0) {
	    return;
        }
    }
    // exportFileType was not in the list
    if (formatList->numEntries() > 0) {
    	exportFileType = formatList->entry(0);
    } else {
	exportFileType = "none";
    }
    */
}

/** See if the user has given a name to the filtered plane other
 than "".  If so, we should create a new plane and set the value
 back to "". */
static	void	handle_filterPlaneName_change(const char *, void *) {

    if (strlen(newFilterPlaneName.string()) > 0) {

	// Create the new one.
	if (dataset->computeFilteredPlane(newFilterPlaneName.string(),
                                          procPlaneName.string(),
                                          procImageDir,
                                          procProgName.string(),
                                          procScale, procAngle,
                                          procParams.string())) {
            printf("Can not create filtered plane %s\n", 
                             newFilterPlaneName.string());

	// Add the plane into the list of available ones.
	} else {
		microscope->state.data.inputPlaneNames.addEntry(newFilterPlaneName);
	}

	newFilterPlaneName = (const char *) "";
    }
}






/** See if the user has given a name to the flattened plane other
 than "".  If so, we should create a new plane and set the value
 back to "". */
static	void	handle_flatPlaneName_change(const char *, void *)
{

    if (strlen(newFlatPlaneName.string()) > 0) {

	// Create the new one.
	BCPlane* new_flat_plane = 
	    dataset->computeFlattenedPlane(newFlatPlaneName.string(),
                                           dataset->heightPlaneName->string(),
 decoration->red.x(), decoration->green.x(), decoration->blue.x(),
 decoration->red.y(), decoration->green.y(), decoration->blue.y());
	if (new_flat_plane == NULL) {
            fprintf(stderr, "Can not create flatten plane %s\n", 
                             newFlatPlaneName.string());

	// Add the plane into the list of available ones.
	} else {
	    // Here we DONT just use newFlatPlaneName, because 
	    // computeFlattenedPlane will change the name,
	    // to add "from hostname"
	    microscope->state.data.inputPlaneNames.addEntry(new_flat_plane->name()->Characters());
	}

	newFlatPlaneName = (const char *) "";
    }
}

/**Added by Amy Henderson 1-9-99
 See if the user has given a name to the line-by-line flattened plane
 other than "".  If so, we should create a new plane and set the value
 back to "". */
static  void	handle_lblflatPlaneName_change(const char *, void *)
{

	
	if (strlen(newLBLFlatPlaneName.string()) > 0) {
	    // Create the new one.  
	    if (dataset->computeLBLFlattenedPlane(newLBLFlatPlaneName.string(),
                            dataset->heightPlaneName->string())) {
		printf("Can not create line-by-line flattened plane %s\n",
				newLBLFlatPlaneName.string());
	    }
	    // Add the plane into the list of available ones.
	    else {
		microscope->state.data.inputPlaneNames.addEntry(newLBLFlatPlaneName);
	    }

	    newLBLFlatPlaneName = (const char *) "";
	}
} 

/** See if the user has given a name to the sum plane other
 than "".  If so, we should create a new plane and set the value
 back to "". */
static	void	handle_sumPlaneName_change(const char *, void *)
{

    if (strlen(newSumPlaneName.string()) > 0) {

	// Create the new one from the sums.
	if (dataset->computeSumPlane(newSumPlaneName.string(),
                                     sumPlane1Name.string(),
                                     sumPlane2Name.string(), sumScale)) {
		printf("Cannot create %s from %s and %s\n",
			newSumPlaneName.string(),
			sumPlane1Name.string(), sumPlane2Name.string());

	// Add the plane into the list of available ones.
	} else {
	    microscope->state.data.inputPlaneNames.addEntry(newSumPlaneName);
	}

	newSumPlaneName = "";
    }
}

/** See if the user has given a name to the adhesion plane other
 than "".  If so, we should create a new plane and set the value
 back to "". */
static	void	handle_adhPlaneName_change(const char *, void *)
{

    if (strlen(newAdhPlaneName.string()) > 0) {

	// Create the new one.
	if (dataset->computeAdhesionFromDeflection(newAdhPlaneName.string(),
                     adhPlane1Name.string(), adhPlane2Name.string(),
                     adhNumToAvg)) {
		printf("Cannot create %s from %s and %s\n",
			newAdhPlaneName.string(),
			adhPlane1Name.string(), adhPlane2Name.string());

	// Add the plane into the list of available ones.
	} else {
		microscope->state.data.inputPlaneNames.addEntry(newAdhPlaneName);
	}

	strcpy(lastAdhPlaneName, newAdhPlaneName.string());
	newAdhPlaneName = "";

	// Redraw the grid, in case this is the one mapped to something
	cause_grid_redraw(0.0, NULL);
    }
}


static	void	handle_adhesion_average_change(vrpn_float64 val, void *)
{

	// Make sure that the number is an integer.
	adhNumToAvg = floor(val);

	// If we have created an adhesion plane, recreate it with the new
	// value.
	// XXX This assumes that the first and last plane are the same...
	if (strlen(lastAdhPlaneName)) {
	    if (dataset->computeAdhesionFromDeflection
                        (lastAdhPlaneName,
                         adhPlane1Name.string(),
                         adhPlane2Name.string(), adhNumToAvg)) {
		printf("Cannot create %s from %s and %s\n",
			newAdhPlaneName.string(),
			adhPlane1Name.string(), adhPlane2Name.string());
	    }

	    // Redraw the grid, in case this is the one mapped to something
	    cause_grid_redraw(0.0, NULL);
	}
}


/// Aron Helser Temporary, testing how to save and restore a viewpoint
static v_xform_type save_diff_xform = { { 0, 0, 0},
					{ 1, 0, 0, M_PI/4.0},
					0 };
static q_vec_type save_light_dir = { 0, 0, 0 };

/// Aron Helser Temporary, testing how to save and restore a viewpoint
static	void	handle_save_xform_change (vrpn_int32, void * userdata)
{
  nmg_Graphics * g = (nmg_Graphics *) userdata;
  v_xform_type cent_xf;
  
  // find the transforms to take us to the centered view
  find_center_xforms(&cent_xf.xlate, &cent_xf.rotate, &cent_xf.scale);

  // find the difference between the current user xform and the 
  // centered xform. Save this so we can apply it later. 
  q_vec_subtract(save_diff_xform.xlate,  v_world.users.xforms[0].xlate, cent_xf.xlate);
  //  q_subtract(save_diff_xform.rotate, cent_xf.rotate, v_world.users.xforms[0].rotate);
  q_copy(save_diff_xform.rotate,v_world.users.xforms[0].rotate);
  save_diff_xform.scale =  v_world.users.xforms[0].scale - cent_xf.scale;

  // save the light direction, too. 
  g->getLightDirection(&save_light_dir);

}

/// Aron Helser Temporary, testing how to save and restore a viewpoint
static	void	handle_set_xform_change (vrpn_int32, void * userdata)
{
  nmg_Graphics * g = (nmg_Graphics *) userdata;
  v_xform_type cent_xf;
  v_xform_type temp;
  
  // find the transforms to take us to the centered view
  find_center_xforms(&cent_xf.xlate, &cent_xf.rotate, &cent_xf.scale);

  // Add the saved difference xform to the center xform
  q_vec_add(temp.xlate, save_diff_xform.xlate, cent_xf.xlate );
  q_copy(temp.rotate, save_diff_xform.rotate);
  temp.scale = save_diff_xform.scale + cent_xf.scale;
  
  updateWorldFromRoom(&temp);

  g->setLightDirection(save_light_dir);
 
}

static void handle_global_icon_scale_change (vrpn_float64 value, void * userdata) {
  nmg_Graphics * g = (nmg_Graphics *) userdata;

  g->setIconScale(value);
}

void handle_contour_dataset_change (const char *, void * userdata)
{
  nmg_Graphics * g = (nmg_Graphics *) userdata;
        BCPlane * plane = dataset->inputGrid->getPlaneByName
                               (dataset->contourPlaneName->string());

 fprintf(stderr, "In handle_contour_dataset_change with name %s\n",
 dataset->contourPlaneName->string());

        if (plane != NULL) {
                texture_scale = (plane->maxValue() -
                                 plane->minValue()) / 10;
                g->setTextureScale(texture_scale);
                g->setTextureMode(nmg_Graphics::CONTOUR,
				  nmg_Graphics::RULERGRID_COORD);
        } else {        // No plane
fprintf(stderr, "Couldn't find plane for contour mode;  "
"turning contours off.\n");
          if (g->getTextureMode() == nmg_Graphics::CONTOUR) {
            g->setTextureMode(nmg_Graphics::NO_TEXTURES,
		  nmg_Graphics::RULERGRID_COORD);
          }
        }

  g->setContourPlaneName(dataset->contourPlaneName->string());
  g->causeGridRedraw();
}

void    handle_adhesion_dataset_change(const char *, void * userdata)
{
  nmg_Graphics * g = (nmg_Graphics *) userdata;
  BCPlane * plane = dataset->inputGrid->getPlaneByName
                     (adhesionPlaneName.string());

        if (plane != NULL)
          {
                adhesion_slider_min_limit = plane->minAttainableValue();
                adhesion_slider_min = plane->minValue();
                adhesion_slider_max_limit = plane->maxAttainableValue();
                adhesion_slider_max = plane->maxValue();
                g->setAdhesionSliderRange(adhesion_slider_min,
                                                 adhesion_slider_max);
        }
        // On PxFl, map adhesion plane to hatch map
        g->setHatchMapName(adhesionPlaneName.string());
}

void    handle_friction_dataset_change(const char *, void * userdata)
{
  nmg_Graphics * g = (nmg_Graphics *) userdata;
  BCPlane * plane = dataset->inputGrid->getPlaneByName
               (frictionPlaneName.string());

        if (plane != NULL) {
                friction_slider_min_limit = plane->minAttainableValue();
                friction_slider_min = plane->minValue();
                friction_slider_max_limit = plane->maxAttainableValue();
                friction_slider_max = plane->maxValue();
                g->setFrictionSliderRange(friction_slider_min,
                                                 friction_slider_max);
        }
        // On PxFl, map friction plane to bump map
        g->setBumpMapName(frictionPlaneName.string());
}

void    handle_bump_dataset_change(const char *, void * userdata)
{
  nmg_Graphics * g = (nmg_Graphics *) userdata;
  BCPlane * plane = dataset->inputGrid->getPlaneByName
    (bumpPlaneName.string());

        if (plane != NULL) {
                bump_slider_min_limit = plane->minAttainableValue();
                bump_slider_min = plane->minValue();
                bump_slider_max_limit = plane->maxAttainableValue();
                bump_slider_max = plane->maxValue();
                g->setBumpSliderRange(bump_slider_min,
                                                 bump_slider_max);
        }
}

void    handle_buzz_dataset_change(const char *, void * userdata)
{
  nmg_Graphics * g = (nmg_Graphics *) userdata;
  BCPlane * plane = dataset->inputGrid->getPlaneByName
               (buzzPlaneName.string());

        if (plane != NULL) {
                buzz_slider_min_limit = plane->minAttainableValue();
                buzz_slider_min = plane->minValue();
                buzz_slider_max_limit = plane->maxAttainableValue();
                buzz_slider_max = plane->maxValue();
                g->setBuzzSliderRange(buzz_slider_min,
                                                 buzz_slider_max);
        }
}


void    handle_compliance_dataset_change(const char *, void * userdata) {
  nmg_Graphics * g = (nmg_Graphics *) userdata;
  BCPlane * plane = dataset->inputGrid->getPlaneByName
           (compliancePlaneName.string());

        if (plane != NULL) {
                compliance_slider_min_limit = plane->minAttainableValue();
                compliance_slider_min = plane->minValue();
                compliance_slider_max_limit = plane->maxAttainableValue();
                compliance_slider_max = plane->maxValue();
                g->setComplianceSliderRange(compliance_slider_min,
                                                   compliance_slider_max);
        }

}



// Callbacks for latency compensation techniques.

static void handle_trueTip_change (vrpn_int32 value, void * userdata) {
  nmg_Graphics * g = (nmg_Graphics *) userdata;

  g->enableTrueTip(value);
}

static void handle_trueTip_scale_change (vrpn_float64 value, void * userdata) {
  nmg_Graphics * g = (nmg_Graphics *) userdata;

  g->setTrueTipScale(value);
}


static void handle_constraint_mode_change (vrpn_int32 value, void * userdata) {
  vrpn_ForceDevice_Remote * fd = (vrpn_ForceDevice_Remote *) userdata;
  vrpn_ForceDevice::ConstraintGeometry mode;

  switch (value) {
    case 0:  mode = vrpn_ForceDevice::NO_CONSTRAINT;  break;
    case 1:  mode = vrpn_ForceDevice::POINT_CONSTRAINT;  break;
    case 2:  mode = vrpn_ForceDevice::LINE_CONSTRAINT;  break;
  }

  if (value) {
    fd->setConstraintMode(mode);
    fd->enableConstraint(1);
  } else {
    fd->setConstraintMode(mode);
    fd->enableConstraint(0);
  }
fprintf(stderr, "Set spring constraint mode to %s.\n",
(value == 2) ? "line" : (value == 1) ? "point" : "none");
}

static void handle_constraint_kspring_change (vrpn_float64 value, void * userdata) {
  vrpn_ForceDevice_Remote * fd = (vrpn_ForceDevice_Remote *) userdata;

  fd->setConstraintKSpring(value);
fprintf(stderr, "Set spring constraint strength to %.4f.\n", value);
}


/** this changes the rotation and position of the surface based
 * on a tcl joystick pad
 * Note: variables joy0* and joy1* should range from -0.5 to 0.5
 * but if you drag the mouse off the pad, they will happily go to
 * much higher values ! */
static void handle_joymove(vrpn_float64 , void *userdata)
{
    static float offsetx, offsety, offsetz;
    static v_xform_type shadowxform,modxform;
    static q_vec_type T;
    float deltax, deltay,deltaz;
    q_type qtemp, qtemp2;
    v_xform_type temp;
    
//     printf("%f,%f,%f   %f,%f,%f\n",(float)joy0x,
// 		(float)joy0y,(float)joy0z,(float)joy1x,(float)joy1y,
// 		(float)joy1z);
    
    switch(((char*)userdata)[0]){
	case 'r':
	    switch((int)(float)joy1b){		
	    case -1: // this is a button press
		    get_Plane_Extents(&offsetx,&offsety,&offsetz);
		    v_x_invert(&shadowxform,&v_world.users.xforms[0]);
		    v_x_copy(&modxform,&shadowxform);
		    /*there is deliberately no break here so that any
			press will update the xform */
	    case 1: // this is a button move. 
		    /* because TCL joy buttons range from -.5 to 0.5
		       we only have to scale by 2*PI */
		        deltax=(float)joy1x;
			// invert y rotation because it feels more natural.
		        deltay=(float)-joy1y;
			deltaz=(float)joy1z;
// 			printf("---%2.2f, %2.2f, %2.2f\n",deltax,deltay,deltaz);
			/*calculate rotations	*/
			q_make(qtemp,0,1,0,2*M_PI*deltax);
			q_make(qtemp2,1,0,0,2*M_PI*deltay);
			q_mult(qtemp,qtemp2,qtemp);
			q_make(qtemp2,0,0,1,2*M_PI*deltaz);
			q_mult(qtemp,qtemp2,qtemp);
			
			T[0]=offsetx; T[1]=offsety;  T[2]=offsetz;	/*offset to center of grid*/
	
			// Set the temp transform : trans, rot, scale 
			v_x_set(&temp,T,qtemp,1);
			/*do rotation at offset frame position*/
			v_x_compose(&modxform,&shadowxform,&temp);
			/* undo offset frame-frame xform */	
			T[0]=-offsetx; T[1]=-offsety; T[2]=-offsetz;
			q_make(qtemp2,1,0,0,0);	// null rotation? 
			v_x_set(&temp,T,qtemp2,1);
			v_x_compose(&modxform,&modxform,&temp);
			/*recalculate world to room xform and save*/
			v_x_invert(&modxform,&modxform);
                        //v_x_copy(&v_world.users.xforms[0],&modxform);
                        updateWorldFromRoom(&modxform);
			break;
	    case 0: // this is a release
		    break;
    	    }
	    break;
	case 't':
	    switch((int)(float)joy0b){
	    case -1: // button press
		T[0]=T[1]=T[2]=0;
                get_Plane_Extents(&offsetx,&offsety,&offsetz);
                v_x_invert(&shadowxform,&v_world.users.xforms[0]);
                v_x_copy(&modxform,&shadowxform);
		// again, no break intentionally, so xforms alway updated.
	    case 1: // button move
		deltay=(float)joy0y;
                deltax=(float)joy0x;
                deltaz=(float)joy0z;
                v_x_copy(&modxform,&shadowxform);
                modxform.xlate[0]=shadowxform.xlate[0]+deltax;
                modxform.xlate[1]=shadowxform.xlate[1]+deltay;
                modxform.xlate[2]=shadowxform.xlate[2]+deltaz;
                v_x_invert(&modxform,&modxform);
                //v_x_copy(&v_world.users.xforms[0],&modxform);
                updateWorldFromRoom(&modxform);
		break;
	    case 0: // release
		break;
	    }
	    break;
	default:
	    fprintf(stderr, "handle_joymove::You shouldn't be here\n");
	    exit(-1);
	}
	
//     printf("done handle_joymove\n");
			
}



/*****************************************************************************
 VRPN tracker callbacks
 *****************************************************************************/
void    handle_tracker2room_change(void *userdata,
                                        const vrpn_TRACKERTRACKER2ROOMCB info)
{
    // userdata is VLIB transform index
    int xformIndex = *(int *)userdata;

    if (xformIndex == V_ROOM_FROM_HAND_TRACKER) {
        printf("Updating hand tracker from room xform\n");
    } else if (xformIndex == V_ROOM_FROM_HEAD_TRACKER) {
        printf("Updating head tracker from room xform\n");
    }

    if ((xformIndex == V_ROOM_FROM_HEAD_TRACKER) || 
	(xformIndex == V_ROOM_FROM_HAND_TRACKER)) {
        printf("(%g, %g, %g), (%g, %g, %g, %g)\n",
               info.tracker2room[0], info.tracker2room[1], info.tracker2room[2],
               info.tracker2room_quat[0], info.tracker2room_quat[1], 
               info.tracker2room_quat[2], info.tracker2room_quat[3]);
	
	v_update_user_xform(0, xformIndex, UGLYCAST info.tracker2room, 
			    UGLYCAST info.tracker2room_quat);

    } else {
	fprintf(stderr,"Error: unexpected xform index\n");
    }
    return;
}

void    handle_sensor2tracker_change(void *userdata, const vrpn_TRACKERCB info)
{
    // userdata is VLIB transform index
    int xformIndex = *(int *)userdata;
    if (xformIndex == V_TRACKER_FROM_HEAD_SENSOR) {
       //printf("tracker from head: ");
    } else if (xformIndex == V_TRACKER_FROM_HAND_SENSOR){
       //printf("tracker from hand: ");
    }
    if ((xformIndex == V_TRACKER_FROM_HEAD_SENSOR) ||
	(xformIndex == V_TRACKER_FROM_HAND_SENSOR)) {
        //printf("(%g, %g, %g), (%g, %g, %g, %g)\n",
        //       info.pos[0], info.pos[1], info.pos[2],
        //       info.quat[0], info.quat[1], info.quat[2], info.quat[3]);
	v_update_user_xform(0, xformIndex, UGLYCAST info.pos,
                            UGLYCAST info.quat);
    } else {
	fprintf(stderr, "Error: unexpected xform index\n");
    }
    return;
}

void handle_collab_sensor2tracker_change(void *, 
					const vrpn_TRACKERCB info) {
    graphics->setCollabHandPos(UGLYCAST info.pos, UGLYCAST info.quat);
}

void handle_collab_mode_change(void *, const vrpn_ANALOGCB info) {
    graphics->setCollabMode(info.channel[0]);
}

/**** Not currently used
void    handle_sensor2tracker_quat_change(void *userdata,
                                        const vrpn_TRACKERCB info)
{
    // userdata is VLIB transform index
    int xformIndex = *(int *)userdata;
    if ((xformIndex == V_TRACKER_FROM_HEAD_SENSOR) ||
        (xformIndex == V_TRACKER_FROM_HAND_SENSOR))
        v_update_user_rotation(0, xformIndex, UGLYCAST info.quat);
    else
        fprintf(stderr, "Error: unexpected xform index\n");
    return;
}
*/

/**** Not currently used

void    handle_forcedevice_scp_change(void *userdata, 
                                        const vrpn_FORCESCPCB info)
{
    // userdata is VLIB transform index
    int xformIndex = *(int *)userdata;
    if ((xformIndex == V_TRACKER_FROM_HEAD_SENSOR) ||
        (xformIndex == V_TRACKER_FROM_HAND_SENSOR))
        v_update_user_translation(0, xformIndex, UGLYCAST info.pos);
    else
        fprintf(stderr, "Error: unexpected xform index\n");

    return;
}
*/

void    handle_unit2sensor_change(void *userdata,
                                        const vrpn_TRACKERUNIT2SENSORCB info)
{
    // userdata is VLIB transform index
    int xformIndex = *(int *)userdata;

    if (xformIndex == V_SENSOR_FROM_HEAD_UNIT){
         printf("Updating head unit to sensor xform\n");
    } else if (xformIndex == V_SENSOR_FROM_HAND_UNIT) {
         printf("Updating hand unit to sensor xform\n");
    }

    if ((xformIndex == V_SENSOR_FROM_HEAD_UNIT) ||
        (xformIndex == V_SENSOR_FROM_HAND_UNIT)) {
        printf("(%g, %g, %g), (%g, %g, %g, %g)\n",
               info.unit2sensor[0], info.unit2sensor[1], info.unit2sensor[2],
               info.unit2sensor_quat[0], info.unit2sensor_quat[1], 
               info.unit2sensor_quat[2], info.unit2sensor_quat[3]);
        v_update_user_xform(0, xformIndex, UGLYCAST info.unit2sensor,
                        UGLYCAST info.unit2sensor_quat);
    } else {
        fprintf(stderr, "Error: unexpected xform index\n");
    }
    return;
}

void handle_phantom_connect (vrpn_Tracker_Remote * handT) {

  if (handT) {
    handT->request_t2r_xform();
    handT->request_u2s_xform();
  }

}

int register_vrpn_phantom_callbacks(void)
{
    vrpn_Tracker_Remote *handT = vrpnHandTracker[0];
    int hand_sensor = handSensor[0];
    if (handT != NULL){
        handT->register_change_handler((void *)&V_ROOM_FROM_HAND_TRACKER,
            handle_tracker2room_change);

// XXX using the SCP for the graphical position of the hand is possible but
// we definitely don't want to use for determining the next plane to send or
// for controlling the microscope (this can give you really bad oscillations).
// It may also not make sense for the graphics because it will introduce a
// small difference in where the user sees the hand and where the microscope
// is told to go.
// Thus, for now I am taking it out completely (AAS)
/*	if (forceDevice != NULL){
          handT->register_change_handler((void *)&V_TRACKER_FROM_HAND_SENSOR,
                handle_sensor2tracker_quat_change, hand_sensor);
          forceDevice->register_scp_change_handler(
                (void *)&V_TRACKER_FROM_HAND_SENSOR,
                handle_forcedevice_scp_change);
        }
        else
*/
         handT->register_change_handler((void *)&V_TRACKER_FROM_HAND_SENSOR,
              handle_sensor2tracker_change, hand_sensor);

            fprintf(stderr, "DEBUG: using remote vrpn config files for hand tracker\n");

            handT->register_change_handler((void *)&V_SENSOR_FROM_HAND_UNIT,
                                            handle_unit2sensor_change, hand_sensor);
            handT->register_change_handler((void *)&V_ROOM_FROM_HAND_TRACKER,
                                            handle_tracker2room_change);

       handle_phantom_connect(handT);
       
    }
    return 0;

}


void handle_phantom_reconnect (void *, vrpn_HANDLERPARAM) {

  handle_phantom_connect(vrpnHandTracker[0]);

}

/// Handles call back for setting the name for the file to write screen
/// captures into. At this point, the type of file is known as well.
static	void	handle_screenImageFileName_change (const char *, void *userdata)
{
   // See if the user has given a name other than ""
   if (strlen(newScreenImageFileName.string()) > 0)
   {
      nmg_Graphics *g = (nmg_Graphics *)(userdata);
      

      int i;

      fprintf(stderr, "handle_screenFileName_change:: "
                      "Saving screen to %s image '%s'\n",
                      screenImageFileType.string(),
                      newScreenImageFileName.string());

      for (i = 0; i < ImageType_count; i++)
         if (!strcmp(screenImageFileType.string(), ImageType_names[i]))
            break;

      if (ImageType_count == i) {
         fprintf(stderr, "handle_screenFileName_change:: "
                         "Error: unknown image type '%s'\n",
                         screenImageFileType.string());
      } else {
	  // Pop the window to the front.
	  // Use different methods for GLUT and for
	  // GLX windows. 
#ifdef V_GLUT
          v_gl_set_context_to_vlib_window();
	  glutPopWindow();
          glutProcessEvents_UNC(); // make the window come to the front
#else 
	  XRaiseWindow(VLIB_dpy, VLIB_win);
	  XEvent event;
	  long event_mask = 0xFFFFFFFF;//StructureNotifyMask|ExposureMask;
	  // Seems like the body of this loop never gets executed...
	  while (XCheckWindowEvent(VLIB_dpy, VLIB_win,
		event_mask, &event)) {
		if (event.type == StructureNotifyMask) {
			printf("structurenotify\n");
		} else if (event.type == ExposureMask) {
			printf("exposure\n");
		} else {
			printf("other\n");
		}
	  }
#endif
	  graphics->mainloop();

          g->createScreenImage(newScreenImageFileName.string(), (ImageType)i);
      }
   }

   newScreenImageFileName = (const char *) "";
}

int register_vrpn_callbacks(void){
    vrpn_Tracker_Remote *headT = vrpnHeadTracker[0];
    int head_sensor = headSensor[0];

    if (headT != NULL){

        headT->register_change_handler((void *)&V_TRACKER_FROM_HEAD_SENSOR,
	    handle_sensor2tracker_change, head_sensor);


            fprintf(stderr, "DEBUG:  using remote vrpn config files for head tracker\n");
            headT->register_change_handler((void *)&V_SENSOR_FROM_HEAD_UNIT,
	                                    handle_unit2sensor_change, head_sensor);
            headT->register_change_handler((void *)&V_ROOM_FROM_HEAD_TRACKER,
                                            handle_tracker2room_change);

            headT->request_t2r_xform();
            headT->request_u2s_xform(); ;
    }
    register_vrpn_phantom_callbacks();
    return 0;
}

/**
loadColorMaps
	This routine looks up the names of the color maps that we might want
 to load.  It also sets up the directory path to the color map files so that
 the routines that load them can know where to look.
*/
int	loadColorMapNames(void)
{
	DIR	*directory;
	struct	dirent *entry;

	// Set the default entry to use the custom color controls
	colorMapNames.addEntry("CUSTOM");

	// Find the directory we are supposed to use
	if ( (colorMapDir=getenv("NM_COLORMAP_DIR")) == NULL) {
		colorMapDir = defaultColormapDirectory;
	}

	// Get the list of files that are in that directory
	// Put the name of each file in that directory into the list
	if ( (directory = opendir(colorMapDir)) == NULL) {
		perror("loadColorMapNames(): Cannot open directory");
		fprintf(stderr,"  (directory name %s)\n",colorMapDir);
		return -1;
	}
	while ( (entry = readdir(directory)) != NULL) {
	    if (entry->d_name[0] != '.') {
		colorMapNames.addEntry(entry->d_name);
	    }
	}
	closedir(directory);

	return 0;
}

/**
loadProcImage
	This routine looks up the names image processing tools we might want
 to load.  It also sets up the directory path to the image proc tools so that
 the routines that load them can know where to look.
*/
int	loadProcProgNames(void)
{
	DIR	*directory;
	struct	dirent *entry;

	// Set the default entry to use the custom color controls
	procProgNames.addEntry("none");

	// Find the directory we are supposed to use
	if ( (procImageDir=getenv("NM_FILTER_DIR")) == NULL) {
		procImageDir = defaultFilterDir;
	}

	// Get the list of files that are in that directory
	// Put the name of each file in that directory into the list
	if ( (directory = opendir(procImageDir)) == NULL) {
		perror("loadProcProgNames(): Cannot open directory");
		fprintf(stderr,"  (directory name %s)\n",procImageDir);
		return -1;
	}
	while ( (entry = readdir(directory)) != NULL) {
	    if (entry->d_name[0] != '.') {
		procProgNames.addEntry(entry->d_name);
	    }
	}
	closedir(directory);

	return 0;
}

/**
loadPPMTextures
	This routine looks up the names of textures we might want
 to load.  It also sets up the directory path to the textures so that
 the routines that load them can know where to look.
*/
int	loadPPMTextures(void)
{
	DIR	*directory;
	struct	dirent *entry;

	// Set the default entries
	textureNames.addEntry("off");
	textureNames.addEntry("uniform");

	// Find the directory we are supposed to use
	if ( (textureDir=getenv("PX_TEXTURE_DIR")) == NULL) {
		textureDir = defaultTextureDir;
	} else {
	   //Make sure there is a trailing /
	   if (textureDir[strlen(textureDir)-1] != '/') {
	      char * tempdir = new char[strlen(textureDir) + 2];
	      sprintf(tempdir, "%s/", textureDir);
	      textureDir = tempdir;
	   }
	}

	// Get the list of files that are in that directory
	// Put the name of each file in that directory into the list
	if ( (directory = opendir(textureDir)) == NULL) {
		perror("loadPPMTextures(): Cannot open directory");
		fprintf(stderr,"  (directory name %s)\n",procImageDir);
		return -1;
	}
	while ( (entry = readdir(directory)) != NULL) {
	    if (entry->d_name[0] != '.') {
		textureNames.addEntry(entry->d_name);
	    }
	}
	closedir(directory);

	return 0;
}



#ifdef USE_VRPN_MICROSCOPE

void setupCallbacks (nmb_Dataset * d, nmm_Microscope_Remote * m) {

#else

void setupCallbacks (nmb_Dataset * d, Microscope * m) {

#endif

  if (!d || !m) return;

  ((Tclvar_string *) d->alphaPlaneName)->
        initializeTcl("alpha_comes_from");
  //d->alphaPlaneName->bindList(&m->state.data.inputPlaneNames);

  ((Tclvar_string *) d->colorMapName)->
        initializeTcl("color_map");
  //d->colorMapName->bindList(&colorMapNames);

  ((Tclvar_string *) d->colorPlaneName)->
        initializeTcl("color_comes_from");
  //d->colorPlaneName->bindList(&m->state.data.inputPlaneNames);
  ((Tclvar_string *) d->colorPlaneName)->addCallback
            (handle_color_dataset_change, m);

  ((Tclvar_string *) d->contourPlaneName)->
        initializeTcl("contour_comes_from");
  //d->contourPlaneName->bindList(&m->state.data.inputPlaneNames);

  ((Tclvar_string *) d->heightPlaneName)->
        initializeTcl("z_comes_from");
  //d->heightPlaneName->bindList(&m->state.data.inputPlaneNames);
  ((Tclvar_string *) d->heightPlaneName)->addCallback
            (handle_z_dataset_change, m);
}

void setupCallbacks (nmb_Dataset * d, nmg_Graphics * g) {

  if (!d || !g) return;

  ((Tclvar_string *) d->alphaPlaneName)->addCallback
            (handle_alpha_dataset_change, g);
  ((Tclvar_string *) d->colorMapName)->addCallback
            (handle_colormap_change, g);
  ((Tclvar_string *) d->contourPlaneName)->addCallback
            (handle_contour_dataset_change, g);
}

void setupCallbacks (nmb_Dataset *d) {
  // sets up callbacks that have to do with data as opposed to callbacks
  // that affect or refer to some device which produces data

    //exportPlaneName.bindList(d->dataImages->imageNameList());
  exportPlaneName = "none";
  exportPlaneName.addCallback
	(handle_exportPlaneName_change, d);

  // make the list contain all the known export file formats
  // now, the image which is being exported determines what formats it
  // can be exported as, since some format might be appropriate for certain
  // images but not for others depending on the type of data they contain
  //exportFileType.bindList(NULL); // the list is bound when an image is selected
  exportFileType = "none";

  // When newExportFileName is changed, we actually save a file.
  newExportFileName  = "";
  newExportFileName.addCallback
            (handle_exportFileName_change, NULL);

  // REALIGN TEXTURES:
  // handler for enabling/disabling the Realignment of Textures
  realign_textures_enabled.addCallback
    (handle_realign_textures_selected_change, NULL);

  texturePlaneName.initializeTcl("texture_comes_from");
  //texturePlaneName.bindList(d->dataImages->imageNameList());

  textureConversionMapName.initializeTcl("texture_conversion_map");
  //textureConversionMapName.bindList(&colorMapNames);
  // BUG BUG BUG 
  // handle_texture_conversion_map_change takes a nmg_Graphics pointer,
  // not an nmm_Microscope!
  //textureConversionMapName.addCallback
  //  (handle_texture_conversion_map_change, m);

  // DONE REALIGN TEXTURES

}

#ifdef USE_VRPN_MICROSCOPE

void setupCallbacks (nmm_Microscope_Remote * m) {

#else

void setupCallbacks (Microscope * m) {

#endif

  if (!m) return;

  // TCH 19 May 98
  // Moved all of these here since microscope isn't defined at load time
  // so we can't declare all these things statically.

  //compliancePlaneName.bindList(&m->state.data.inputPlaneNames);
  compliancePlaneName = "none";



  //frictionPlaneName.bindList(&m->state.data.inputPlaneNames);
  frictionPlaneName = "none";

  //bumpPlaneName.bindList(&m->state.data.inputPlaneNames);
  bumpPlaneName = "none";

  //buzzPlaneName.bindList(&m->state.data.inputPlaneNames);
  buzzPlaneName = "none";

  //adhesionPlaneName.bindList(&m->state.data.inputPlaneNames);
  adhesionPlaneName = "none";

  //soundPlaneName.bindList(&m->state.data.inputPlaneNames);
  soundPlaneName = "none";
  soundPlaneName.addCallback
            (handle_sound_dataset_change, m);

  // GENETIC TEXTURES -- Alexandra Bokinsky
  //

  // handler for creating the "Set Genetic Texture Parameters"
  // tcl window.
  gen_tex_activate.addCallback
    (handle_genetic_textures_set_parameters, m);

  // DONE GENETIC TEXTURES

  //xPlaneName.bindList(&m->state.data.inputPlaneNames);
  xPlaneName = "none";
  xPlaneName.addCallback
            (handle_x_dataset_change, NULL);

  //procPlaneName.bindList(&m->state.data.inputPlaneNames);
  procPlaneName = "none";
  //procParams.bindList(&m->state.data.inputPlaneNames);
  procParams = "";

  //newFilterPlaneName.bindList(&m->state.data.inputPlaneNames);
  newFilterPlaneName = "";
  newFilterPlaneName.addCallback
            (handle_filterPlaneName_change, NULL);

  //newFlatPlaneName.bindList(&m->state.data.inputPlaneNames);
  newFlatPlaneName = "";
  newFlatPlaneName.addCallback
            (handle_flatPlaneName_change, NULL);

  //newLBLFlatPlaneName.bindList(&m->state.data.inputPlaneNames);
  newLBLFlatPlaneName = "";
  newLBLFlatPlaneName.addCallback
	    (handle_lblflatPlaneName_change, NULL); 

  //sumPlane1Name.bindList(&m->state.data.inputPlaneNames);
  sumPlane1Name = "none";
  //sumPlane2Name.bindList(&m->state.data.inputPlaneNames);
  sumPlane2Name = "none";
  //newSumPlaneName.bindList(&m->state.data.inputPlaneNames);
  newSumPlaneName = "";
  newSumPlaneName.addCallback
            (handle_sumPlaneName_change, NULL);

  //adhPlane1Name.bindList(&m->state.data.inputPlaneNames);
  adhPlane1Name  = "none";
  //adhPlane2Name.bindList(&m->state.data.inputPlaneNames);
  adhPlane2Name  = "none";
  //newAdhPlaneName.bindList(&m->state.data.inputPlaneNames);
  newAdhPlaneName = "";
  newAdhPlaneName.addCallback
            (handle_adhPlaneName_change, m);

  adhNumToAvg.addCallback
            (handle_adhesion_average_change, m);

  numMarkersShown.addCallback
            (handle_markers_shown_change, m);
  markerHeight.addCallback
            (handle_markers_height_change, m);

  changed_image_params.addCallback
            (handle_image_accept, m);
  changed_modify_params.addCallback
            (handle_modify_accept, m);
  changed_scanline_params.addCallback
	    (handle_scanline_accept, m);

  x_min_value.addCallback(handle_x_value_change, NULL);
  x_max_value.addCallback(handle_x_value_change, NULL);
}

void setupCallbacks (nmg_Graphics * g) {

  if (!g) return;

  tclstride.addCallback
            (handle_stride_change, g);

  //for importing from tube_foundry
  import_filename.addCallback
            (handle_import_filename_change, g);
  load_import_file.addCallback
            (handle_load_import_file_change, g);
  load_button_press.addCallback
    (handle_load_button_press_change, g);

  alpha_slider_min.addCallback
            (handle_alpha_slider_change, g);
  alpha_slider_max.addCallback
            (handle_alpha_slider_change, g);
  color_slider_min.addCallback
            (handle_color_slider_change, g);
  color_slider_max.addCallback
            (handle_color_slider_change, g);
  texture_scale.addCallback
            (handle_texture_scale_change, g);
  rulergrid_xoffset.addCallback
            (handle_rulergrid_offset_change, g);
  rulergrid_yoffset.addCallback
            (handle_rulergrid_offset_change, g);
  rulergrid_scale.addCallback
            (handle_rulergrid_scale_change, g);
  rulergrid_angle.addCallback
            (handle_rulergrid_angle_change, g);
  rulergrid_changed.addCallback
            (handle_rulergrid_color_change, g);
  ruler_opacity.addCallback
            (handle_rulergrid_opacity_change, g);
  friction_slider_min.addCallback
            (handle_friction_slider_change, g);
  friction_slider_max.addCallback
            (handle_friction_slider_change, g);
  bump_slider_min.addCallback
			(handle_bump_slider_change, g);
  bump_slider_max.addCallback
			(handle_bump_slider_change, g);
  buzz_slider_min.addCallback
			(handle_buzz_slider_change, g);
  buzz_slider_max.addCallback
			(handle_buzz_slider_change, g);
  compliance_slider_min.addCallback
            (handle_compliance_slider_change, g);
  compliance_slider_max.addCallback
            (handle_compliance_slider_change, g);
  ruler_width_x.addCallback
            (handle_ruler_widthx_change, g);
  ruler_width_y.addCallback
            (handle_ruler_widthy_change, g);
  rulergrid_angle.addCallback
            (handle_rulergrid_angle_change, g);


  shiny.addCallback
            (handle_shiny_change, g);
  diffuse.addCallback
	    (handle_diffuse_change, g); 
  surface_alpha.addCallback
            (handle_surface_alpha_change, g);
  specular_color.addCallback
            (handle_specular_color_change, g);
  sphere_scale.addCallback
            (handle_sphere_scale_change, g);

  // Genetic textures - Alexandra Bokinsky

  // handler for enabling/disabling the Genetic Textures
  genetic_textures_enabled.addCallback
    (handle_genetic_textures_selected_change, g);

  // handler for sending the datasets:
  gen_send_data.addCallback
    (handle_genetic_send_data, g);

  // Texture realignment

  display_realign_textures.addCallback
    (handle_display_textures_selected_change, g);
  set_realign_center.addCallback
    (handle_set_realign_center_change, g);

  texturePlaneName.addCallback
    (handle_texture_dataset_change, g);
  textureConversionMapName.addCallback
    (handle_texture_conversion_map_change, g);

  realign_textures_slider_min.addCallback
    (handle_realign_textures_slider_change, g);
  realign_textures_slider_max.addCallback
    (handle_realign_textures_slider_change, g);

  newRealignPlaneName.addCallback
    (handle_realign_plane_name_change, g);

  rulergrid_enabled.addCallback
            (handle_rulergrid_selected_change, g);

  contour_changed.addCallback
            (handle_contour_color_change, g);
  contour_width.addCallback
            (handle_contour_width_change, g);

  save_xform.addCallback
            (handle_save_xform_change, g);
  set_xform.addCallback
            (handle_set_xform_change, g);

  global_icon_scale.addCallback
            (handle_global_icon_scale_change, g);

  adhesionPlaneName.addCallback
            (handle_adhesion_dataset_change, g);
  frictionPlaneName.addCallback
            (handle_friction_dataset_change, g);
  compliancePlaneName.addCallback
            (handle_compliance_dataset_change, g);
  bumpPlaneName.addCallback(handle_bump_dataset_change, g);
  buzzPlaneName.addCallback(handle_buzz_dataset_change, g);
  

  // Latency compensation - Tom Hudson

  truetip_showing.addCallback
            (handle_trueTip_change, g);
  truetip_scale.addCallback
            (handle_trueTip_scale_change, g);

  // Screen capture callback
  
  // make list of image types for screen captures
  for (int i = 0; i < ImageType_count; i++)
     screenImage_formats.addEntry(screenImage_formats_list[i]);
  //screenImageFileType.bindList(&screenImage_formats);
  screenImageFileType = (const char *)(screenImage_formats_list[0]);
  // do call back when name chnages
  newScreenImageFileName = (const char *) "";
  newScreenImageFileName.addCallback(handle_screenImageFileName_change, g);
}

void setupCallbacks (vrpn_ForceDevice_Remote * p) {

  if (!p) return;

  recovery_time.addCallback
            (handle_recovery_time_change, p);

  // Latency compensation - Tom Hudson

  constraint_mode.addCallback
            (handle_constraint_mode_change, p);
  constraint_kspring.addCallback
            (handle_constraint_kspring_change, p);

}

void setupCallbacks (nmb_Decoration * d) {
    if (!d) return;
    d->red.registerMoveCallback(handle_collab_measure_move, (void *)0);
    d->green.registerMoveCallback(handle_collab_measure_move, (void *)1);
    d->blue.registerMoveCallback(handle_collab_measure_move, (void *)2);
}

// NANOX
// Tom Hudson, September 1999
// Sets up synchronization callbacks and nmui_Component/ComponentSync
//#define MIN_SYNC

void setupSynchronization (vrpn_Connection * serverConnection,
                           vrpn_Connection * logConnection,
                           nmb_Dataset * dset,
#ifdef USE_VRPN_MICROSCOPE
                           nmm_Microscope_Remote * m) {
#else
                           Microscope * m) {
#endif

  // NOTE
  // If you add() any Netvar in this function, make sure you
  // call bindConnection(NULL) on it in shutdown_connections().

  // These really don't need to be visible globally - they don't
  // need their mainloops called, just their connection's
  // (presumably that's collaboratingPeerServerConnection)

  // Streamfile control

  nmui_Component * streamfileControls;
  streamfileControls = new nmui_Component ("Stream");

  // Whenever replay_rate, rewind_stream, or set_stream_time changes,
  // and we are synchronizing, handle_synchronization() and
  // streamfileSync will send a message over collaboratingPeerServerConnection
  // (or whatever connection was passed into this function) to synchronize
  // the same objects on the other end.

  streamfileControls->add(&replay_rate);
  streamfileControls->add(&rewind_stream);
  streamfileControls->add(&set_stream_time);
  // Don't want to synchronize this;  TCL sets it when set_stream_time
  // sync occurs.
  //streamfileControls->add(&set_stream_time_now);


  //set_stream_time.d_permitIdempotentChanges = VRPN_TRUE;


  // View control

  nmui_Component * viewControls;
  viewControls = new nmui_Component ("View");
  
  // Hierarchical decomposition of view control

  nmui_Component * viewPlaneControls;
  viewPlaneControls = new nmui_Component ("View Plane");

  viewPlaneControls->add(&m->state.stm_z_scale);
  viewPlaneControls->add((TclNet_string *) dset->heightPlaneName);
  viewPlaneControls->add(&tcl_wfr_xlate_X);
  viewPlaneControls->add(&tcl_wfr_xlate_Y);
  viewPlaneControls->add(&tcl_wfr_xlate_Z);
  viewPlaneControls->add(&tcl_wfr_rot_0);
  viewPlaneControls->add(&tcl_wfr_rot_1);
  viewPlaneControls->add(&tcl_wfr_rot_2);
  viewPlaneControls->add(&tcl_wfr_rot_3);
  viewPlaneControls->add(&tcl_wfr_scale);
  //viewPlaneControls->add(&tcl_wfr_changed);
  viewPlaneControls->add(&tclstride);

  nmui_Component * viewColorControls;
  viewColorControls = new nmui_Component ("View Color");

  viewColorControls->add((TclNet_string *) dset->colorPlaneName);
  viewColorControls->add((TclNet_string *) dset->colorMapName);
  viewColorControls->add(&color_slider_min);
  viewColorControls->add(&color_slider_max);

  nmui_Component * viewMeasureControls;
  viewMeasureControls = new nmui_Component ("View Measure");

  viewMeasureControls->add(&measureRedX);
  viewMeasureControls->add(&measureRedY);
  viewMeasureControls->add(&measureGreenX);
  viewMeasureControls->add(&measureGreenY);
  viewMeasureControls->add(&measureBlueX);
  viewMeasureControls->add(&measureBlueY);

  nmui_Component * viewLightingControls;
  viewLightingControls = new nmui_Component ("View Lighting");

  viewPlaneControls->add(&tcl_lightDirX);
  viewPlaneControls->add(&tcl_lightDirY);
  viewPlaneControls->add(&tcl_lightDirZ);
  viewColorControls->add(&shiny);
  viewColorControls->add(&diffuse);
  viewColorControls->add(&surface_alpha);
  viewColorControls->add(&specular_color);

  nmui_Component * viewContourControls;
  viewContourControls = new nmui_Component ("View Contour");

  viewContourControls->add(&texture_scale);
  viewContourControls->add(&contour_width);
  viewContourControls->add(&contour_r);
  viewContourControls->add(&contour_g);
  viewContourControls->add(&contour_b);
  viewContourControls->add(&contour_changed);
  viewContourControls->add((TclNet_string *) dset->contourPlaneName);

  nmui_Component * viewGridControls;
  viewGridControls = new nmui_Component ("View Grid");

  viewGridControls->add(&rulergrid_position_line);
  viewGridControls->add(&rulergrid_orient_line);
  viewGridControls->add(&rulergrid_xoffset);
  viewGridControls->add(&rulergrid_yoffset);
  viewGridControls->add(&rulergrid_scale);
  viewGridControls->add(&rulergrid_angle);
  viewGridControls->add(&ruler_width_x);
  viewGridControls->add(&ruler_width_y);
  viewGridControls->add(&ruler_opacity);
  viewGridControls->add(&ruler_r);
  viewGridControls->add(&ruler_g);
  viewGridControls->add(&ruler_b);
  viewGridControls->add(&rulergrid_changed);
  viewGridControls->add(&rulergrid_enabled);

  viewControls->add(viewPlaneControls);
  viewControls->add(viewColorControls);
  viewControls->add(viewMeasureControls);
  viewControls->add(viewLightingControls);
  viewControls->add(viewContourControls);
  viewControls->add(viewGridControls);

  //nmui_Component * derivedPlaneControls;
  //derivedPlaneControls = new nmui_Component ("DerivedPlanes");

  // Christmas sync

  nmui_Component * rootUIControl;
  rootUIControl = new nmui_Component ("ROOT");

  //rootUIControl->add(derivedPlaneControls);
  rootUIControl->add(viewControls);
  rootUIControl->add(streamfileControls);
  rootUIControl->bindConnection(serverConnection);

  if (logConnection) {
    rootUIControl->bindLogConnection(logConnection);

    // these should be logged but not shared;  we declare them as
    // TclNet objects but don't make them part of the UI Components.

    share_sync_state.bindLogConnection(logConnection);
    copy_inactive_state.bindLogConnection(logConnection);
      copy_to_private_state.bindLogConnection(logConnection);
      copy_to_shared_state.bindLogConnection(logConnection);
    collab_machine_name.bindLogConnection(logConnection);
  }

  // User Interface to synchronization

  // NANOX FLAT
  // Set up a utility class to make sure derived planes are synchronized
  // between all replicas.

  nmui_PlaneSync * ps;

  ps = new nmui_PlaneSync (dset, &(m->state.data), serverConnection);

  sync_plane_struct * sync_userdata = new sync_plane_struct;
  sync_userdata->component = rootUIControl;
  sync_userdata->planesync = ps;

  // Since streamfileControls are timed, the toplevel MUST use
  // the timed callbacks.  Oops.  Took an hour or more to find,
  // that one.
  share_sync_state.addCallback
      (handle_synchronize_timed_change, sync_userdata);
  copy_inactive_state.addCallback
      (handle_timed_sync, sync_userdata);

    copy_to_private_state.addCallback
        (handle_copy_to_private, sync_userdata);
    copy_to_shared_state.addCallback
        (handle_copy_to_shared, sync_userdata);

  // need to pass rootUIControl to handle_timed_sync_complete
  // so that it sees d_maintain as TRUE!
 
  streamfileControls->registerSyncRequestHandler
          (handle_timed_sync_request, streamfileControls);
  streamfileControls->registerSyncCompleteHandler
          (handle_timed_sync_complete, sync_userdata);




  // which machine collaborator is using (so we can track hand position)
  collab_machine_name.addCallback
	(handle_collab_machine_name_change, NULL);

  collab_machine_name.addCallback
	(handle_collab_machine_name_change2, rootUIControl);


  // NANOX FLAT
  // make sure flat plane utility class knows if collaborator changes.
  collab_machine_name.addCallback
        (handle_collab_machine_name_change3, ps);

}

struct MicroscapeInitializationState {

  MicroscapeInitializationState (void);

  AFMInitializationState afm;
  OhmmeterInitializationState ohm;
  nma_Keithley2400_ui_InitializationState vicurve;
  SEMInitializationState sem;

  vrpn_bool use_file_resolution;
  int num_x;
  int num_y;
  int graphics_mode;
  char graphicsHost [256];

  int num_stm_files;
  char * stm_file_names [MAXFILES];

  int num_image_files;
  char * image_file_names [MAXFILES];

  char SPMhost [STM_NAME_LENGTH];
  char * SPMhostptr;
  int SPMport;
  int UDPport;
  int socketType;

  int monitorPort;
  int collabPort;

  float x_min;
  float x_max;
  float y_min;
  float y_max;

  char remoteOutputStreamName [256];
  int writingRemoteStream;

  vrpn_bool openPeer;
  char peerName [256];
  vrpn_bool logInterface;
  char logPath [256];
  timeval logTimestamp;
  vrpn_bool replayInterface;

  // control synchronization method for collaboration
  vrpn_bool useOptimism;

  float phantomRate;
  int tesselation;

  int packetlimit;
};

MicroscapeInitializationState::MicroscapeInitializationState (void) :
  use_file_resolution(vrpn_TRUE),
  num_x (DATA_SIZE),
  num_y (DATA_SIZE),
  graphics_mode (LOCAL_GRAPHICS),  // changed from NO_GRAPHICS
  num_stm_files (0),
  num_image_files(0),
  SPMhostptr (NULL),
  SPMport (-1),
  UDPport (-1),
  socketType (SOCKET_TCP),
  monitorPort (-1),
  collabPort (-1),
  x_min (afm.xMin),
  x_max (afm.xMax),
  y_min (afm.yMin),
  y_max (afm.yMax),
  writingRemoteStream (0),
  openPeer (VRPN_FALSE),
  logInterface (VRPN_FALSE),
  replayInterface (VRPN_FALSE),
  useOptimism (VRPN_FALSE),
  phantomRate (60.0),  // standard default
  tesselation (1),
  packetlimit (0)
{

}



void ParseArgs (int argc, char ** argv,
                MicroscapeInitializationState * istate) {

  int real_params = 0;
  int time_frame;
  int i;


#ifdef FLOW
    // vlib needs special arguments for pxfl. pass them as 
    // env. var. PXFL_ARGS
    char pxflArgs [256] = "PXFL_ARGS=";
    fprintf(stderr,"ParseArgs:  pxfl_Args %s\n", pxflArgs);
#endif 

    i = 1;
    while (i < argc) {
      fprintf(stderr,"ParseArgs:  arg %i %s\n", i, argv[i]);

      if (strcmp(argv[i], "-allowdup") == 0) {
        istate->afm.allowdup = 1;
      } else if (strcmp(argv[i], "-alphacolor") == 0) {
        if (++i >= argc) Usage(argv[0]);
        alpha_red = atof(argv[i]);
        if (++i >= argc) Usage(argv[0]);
        alpha_green = atof(argv[i]);
        if (++i >= argc) Usage(argv[0]);
        alpha_blue = atof(argv[i]);
      } else if (strcmp(argv[i], "-alphaimage") == 0) {
        if (++i >= argc) Usage(argv[0]);
        alphaPPM = new PPM(argv[i]);
        if (!alphaPPM->valid) {
          fprintf(stderr, "Cannot read ppm file %s\n",argv[i]);
          exit(-1);
        }
      } else if (strcmp(argv[i], "-b") == 0) {
        using_phantom_button = 1;
      } else if (strcmp(argv[i], "-call") == 0) {
        //DO_CALLBACKS = 1;
        fprintf(stderr, "-call no longer affects anything!\n");
      } else if (!strcmp(argv[i], "-collaborator")) {
        if (++i >= argc) Usage (argv[0]);
        istate->collabPort = atoi(argv[i]);
        printf("Will open a collaborator on port %d and "
               "BLOCK UNTIL CONNECTED.\n", istate->collabPort);
      } else if (strcmp(argv[i], "-color") == 0) {
        if (++i >= argc) Usage(argv[0]);
        maxC[0] = atoi(argv[i]);
        if (++i >= argc) Usage(argv[0]);
        maxC[1] = atoi(argv[i]);
        if (++i >= argc) Usage(argv[0]);
        maxC[2] = atoi(argv[i]);
        if (++i >= argc) Usage(argv[0]);
        minC[0] = atoi(argv[i]);
        if (++i >= argc) Usage(argv[0]);
        minC[1] = atoi(argv[i]);
        if (++i >= argc) Usage(argv[0]);
        minC[2] = atoi(argv[i]);
      } else if (strcmp(argv[i], "-con") == 0) {
        istate->afm.image.mode = CONTACT;
        istate->afm.modify.mode = CONTACT;
      } else if (strcmp(argv[i], "-d") == 0) {
        if (++i >= argc) Usage(argv[0]);
        strcpy(istate->afm.deviceName, argv[i]);
      } else if (strcmp(argv[i], "-dir") == 0) {
        if (++i >= argc) Usage(argv[0]);
        //OBSOLETE (?)
        //lock_dir = atof(argv[i]);
        //lock_dir *= M_PI/180.0;
      } else if (strcmp(argv[i], "-do") == 0) {
	  if (++i >= argc) Usage(argv[0]);
	  strcpy(istate->ohm.deviceName, argv[i]);
      } else if (strcmp(argv[i], "-div") == 0) {
	  if (++i >= argc) Usage(argv[0]);
	  strcpy(istate->vicurve.deviceName, argv[i]);
	  //fprintf(stderr, "IV curve device is not available\n");
      } else if (strcmp(argv[i], "-dsem") == 0) {
          if (++i >= argc) Usage(argv[0]);
          strcpy(istate->sem.deviceName, argv[i]);
      } else if (strcmp(argv[i], "-drift") == 0) {
        istate->afm.doDriftComp = 1;
      } else if (strcmp(argv[i], "-f") == 0) {
        if (++i >= argc) Usage(argv[0]);
        read_mode = READ_FILE; 
        if (istate->num_stm_files >= MAXFILES) {
          fprintf(stderr, "Only %d files allowed, ignoring %s\n",
                  MAXFILES,argv[i]);
        }
        istate->stm_file_names[istate->num_stm_files] = argv[i];
        istate->num_stm_files++;
      } else if (strcmp(argv[i], "-grid") == 0) {
        if (++i >= argc) Usage(argv[0]);
        istate->num_x = atoi(argv[i]);
        if (++i >= argc) Usage(argv[0]);
        istate->num_y = atoi(argv[i]);
        istate->use_file_resolution = vrpn_FALSE;
      } else if (strcmp(argv[i], "-fi") == 0) {
	if (++i >= argc) Usage(argv[0]);
	if (istate->num_image_files >= MAXFILES) {
	  fprintf(stderr, "Only %d IMAGE files allowed, ignoring %s\n",
			MAXFILES,argv[i]);
	}
	istate->image_file_names[istate->num_image_files] = argv[i];
	istate->num_image_files++;
      } else if (strcmp(argv[i], "-i") == 0) {
        read_mode = READ_STREAM;
        istate->afm.readingStreamFile = 1;
        if (++i >= argc) Usage(argv[0]);
        strcpy(istate->afm.inputStreamName, argv[i]);
	// Change so the rate of replay is not a required arguement.
        if (++i >= argc) {
	    // if no next arg, use 1 as replay rate.
	    decoration->rateOfTime =1;
	} else {
	    int play_rate = atoi(argv[i]);
	    if ((play_rate == 0)&&(strcmp("0", argv[i])!=0)) {
		// next arg is not a number - let next iteration process it.
		decoration->rateOfTime =1;
		--i;
	    } else {
		// next arg is a number
		decoration->rateOfTime = play_rate;
	    }
	}
	replay_rate = decoration->rateOfTime;
      } else if ( strcmp(argv[i],"-Loop_num")==0) {
        LOOP_NUM = atoi(argv[++i]);
      } else if (!strcmp(argv[i], "-marshalltest")) {
        istate->graphics_mode = TEST_GRAPHICS_MARSHALLING;
      } else if (!strcmp(argv[i], "-renderserver")) {
        istate->graphics_mode = RENDER_SERVER;
      } else if (!strcmp(argv[i], "-renderclient")) {
	if (++i >= argc) Usage(argv[0]);
        istate->graphics_mode = RENDER_CLIENT;
      } else if (!strcmp(argv[i], "-trenderserver")) {
        istate->graphics_mode = TEXTURE_SERVER;
      } else if (!strcmp(argv[i], "-trenderclient")) {
	if (++i >= argc) Usage(argv[0]);
        istate->graphics_mode = TEXTURE_CLIENT;
        strncpy(istate->graphicsHost, argv[i], 256);
      } else if (!strcmp(argv[i], "-vrenderserver")) {
        istate->graphics_mode = VIDEO_SERVER;
      } else if (!strcmp(argv[i], "-vrenderclient")) {
	if (++i >= argc) Usage(argv[0]);
        istate->graphics_mode = VIDEO_CLIENT;
        strncpy(istate->graphicsHost, argv[i], 256);
      } else if (!strcmp(argv[i], "-phantomrate")) {
        if (++i >= argc) Usage(argv[0]);
        istate->phantomRate = atof(argv[i]);
      } else if (!strcmp(argv[i], "-tesselation")) {
        if (++i >= argc) Usage(argv[0]);
        istate->tesselation = atoi(argv[i]);
      } else if (strcmp(argv[i], "-minsep") == 0) {
        if (++i >= argc) Usage(argv[0]);
        istate->afm.stmRxTmin = atoi(argv[i]);
        if (++i >= argc) Usage(argv[0]);
        istate->afm.stmRxTsep = atoi(argv[i]);
      } else if (!strcmp(argv[i], "-monitor")) {
        if (++i >= argc) Usage (argv[0]);
        istate->monitorPort = atoi(argv[i]);
        printf("Will open a monitor on port %d and "
               "BLOCK UNTIL CONNECTED.\n", istate->monitorPort);
      } else if (!strcmp(argv[i], "-multithread")) {
        istate->graphics_mode = SHMEM_GRAPHICS;
      } else if (strcmp(argv[i], "-nocall") == 0) {
        //DO_CALLBACKS = 0;
        fprintf(stderr, "-nocall no longer affects anything!\n");
      } else if (strcmp(argv[i], "-nocpanels") == 0) {
          //do_cpanels = 0;
      } else if (strcmp(argv[i], "-nodevice") == 0) {
        //do_ad_device = 0;
      } else if (strcmp(argv[i], "-nokeybd") == 0) {
        do_keybd = 0;
      } else if (strcmp(argv[i], "-nomenus") == 0) {
        //do_menus = 0;
      } else if (strcmp(argv[i], "-o") == 0) {
        istate->afm.writingStreamFile = 1;
        if (++i >= argc) Usage(argv[0]);
        strcpy(istate->afm.outputStreamName, argv[i]);
	istate->ohm.writingLogFile = 1;
	sprintf(istate->ohm.outputLogName, "%s%s", argv[i],
		OHM_FILE_SUFFIX);
	istate->vicurve.writingLogFile = 1;
	sprintf(istate->vicurve.outputLogName, "%s%s", argv[i],
		VICURVE_FILE_SUFFIX);
      } else if (strcmp(argv[i], "-peer") == 0) {
        istate->openPeer = VRPN_TRUE;
        if (++i >= argc) Usage(argv[0]);
        strcpy(istate->peerName, argv[i]);
      } else if (!strcmp(argv[i], "-logif")) {
        istate->logInterface = VRPN_TRUE;
        if (++i >= argc) Usage(argv[0]);
        strcpy(istate->logPath, argv[i]);
      } else if (!strcmp(argv[i], "-packetlimit")) {
        if (++i >= argc) Usage(argv[0]);
        istate->packetlimit = atoi(argv[i]);
      } else if (!strcmp(argv[i], "-optimistic")) {
        istate->useOptimism = VRPN_TRUE;
      } else if (!strcmp(argv[i], "-replayif")) {
        istate->replayInterface = VRPN_TRUE;
        if (++i >= argc) Usage(argv[0]);
        strcpy(istate->logPath, argv[i]);
        if (++i >= argc) Usage(argv[0]);
        istate->logTimestamp.tv_sec = atoi(argv[i]);
      } else if (strcmp(argv[i], "-perf") == 0) {
        //perf_mode = 1;
      } else if (strcmp (argv[i], "-recv") == 0) {
        // clark -- use NANO_RECV_TIMESTAMPs for playback times
        istate->afm.useRecvTime = VRPN_TRUE;
      } else if (strcmp(argv[i], "-relax") == 0) {
        istate->afm.doRelaxComp = 1;
      } else if (strcmp(argv[i], "-relax_up_also") == 0) {
	  printf("Warning: -relax_up_also obsolete in nM\n");
        istate->afm.doRelaxUp = 1;
      } else if (strcmp(argv[i], "-showcps") == 0) {
        //show_cpanels = 1;
      } else if (strcmp(argv[i], "-showgrid") == 0) {
        //show_grid = 1;
      } else if (strcmp(argv[i], "-splat") == 0) {
        istate->afm.doSplat = 1;
      } else if (strcmp(argv[i], "-std") == 0) {
        if (++i >= argc) Usage(argv[0]);
        istate->afm.modify.std_dev_samples = atoi(argv[i]);
        if (++i >= argc) Usage(argv[0]);
        istate->afm.modify.std_dev_frequency = atof(argv[i]);
        if (++i >= argc) Usage(argv[0]);
        fprintf(stderr,
           "Warning:  std_dev_color_scale (set to \"%s\") is obsolete.\n",
           argv[i]);
        //std_dev_color_scale = atof(argv[i]);
        //decoration->std_dev_color_scale = std_dev_color_scale;
      } else if (strcmp(argv[i], "-ugraphics") == 0) {
        //ubergraphics_enabled = 1;
      } else if (strcmp(argv[i], "-verbosity") == 0) {
        if (++i >= argc) Usage(argv[0]);
        spm_verbosity = atoi(argv[i]);
      } else if (strcmp(argv[i], "-z") == 0) {
        if (++i >= argc) Usage(argv[0]);
        istate->afm.stm_z_scale = atof(argv[i]);




      } else if (strcmp(argv[i], "-fmods") == 0) {
        if (++i >= argc) Usage(argv[0]);
        istate->afm.modify.setpoint_max = atof(argv[i]);
        istate->afm.modify.amplitude_max = atof(argv[i]);
        if (++i >= argc) Usage(argv[0]);
        istate->afm.modify.setpoint_min = atof(argv[i]);
        istate->afm.modify.amplitude_min = atof(argv[i]);
        istate->afm.modify.setpoint = istate->afm.modify.setpoint_min;
      } else if (strcmp(argv[i], "-fimgs") == 0) {
        if (++i >= argc) Usage(argv[0]);
        istate->afm.image.setpoint_max = atof(argv[i]);
        istate->afm.image.amplitude_max = atof(argv[i]);
        if (++i >= argc) Usage(argv[0]);
        istate->afm.image.setpoint_min = atof(argv[i]);
        istate->afm.image.amplitude_min = atof(argv[i]);
        if (++i >= argc) Usage(argv[0]);
        istate->afm.image.setpoint = atof(argv[i]);
        istate->afm.image.amplitude = istate->afm.image.amplitude_min;
        set_mode = VRPN_TRUE;      // Send these at startup
      } else if (strcmp(argv[i], "-move") == 0) {
        if (++i >= argc) Usage(argv[0]);
        istate->afm.MaxSafeMove = atof(argv[i]);
      } else if (strcmp(argv[i], "-silent") == 0) {
	fprintf(stderr, "Warning: '-silent' is currently obsolete\n");
      } else if (strcmp(argv[i], "-tap") == 0) {
        istate->afm.image.mode = TAPPING;
        istate->afm.modify.mode = TAPPING;
      } else if (strcmp(argv[i], "-tapsew") == 0) {
        istate->afm.image.mode = TAPPING;
        istate->afm.modify.mode = CONTACT;
        istate->afm.modify.style = SEWING;
      } else if (strcmp(argv[i], "-tc") == 0) {
        istate->afm.image.mode = TAPPING;
        istate->afm.modify.mode = CONTACT;

      } else if (strcmp(argv[i], "-disp") == 0) {
        if (++i >= argc) Usage(argv[0]);
        displayPeriod = atoi(argv[i])*1000L;
      } else if (strcmp(argv[i], "-draw_when_centered") == 0) {
        drawOnlyOnCenter = 1;
      } else if (strcmp(argv[i], "-gl") == 0) {
        //glenable=1;
        if (istate->graphics_mode == NO_GRAPHICS)
          istate->graphics_mode = LOCAL_GRAPHICS;
        // LOCAL_GRAPHICS became the default 19 Jan 99
      } else if (strcmp(argv[i], "-mb3") == 0) {
        using_mouse3button = 1;
      } else if (strcmp(argv[i], "-measure") == 0) {
        if (++i >= argc) Usage(argv[0]);
        //measure_spacing = atof(argv[i]);
        fprintf(stderr, "-measure no longer affects anything!.\n");
      } else if (!strcmp(argv[i], "-nographics")) {
        istate->graphics_mode = NO_GRAPHICS;
      } else if (strcmp(argv[i], "-notk") == 0) {
        tkenable=0;
      } else if (strcmp(argv[i], "-region") == 0) {
        if (++i >= argc) Usage(argv[0]);
        istate->x_min = atof(argv[i]);
        if (++i >= argc) Usage(argv[0]);
        istate->y_min = atof(argv[i]);
        if (++i >= argc) Usage(argv[0]);
        istate->x_max = atof(argv[i]);
        if (++i >= argc) Usage(argv[0]);
        istate->y_max = atof(argv[i]);
        set_region = VRPN_TRUE;    // Set the region
        printf("Will set scan region (%g,%g) to (%g,%g)\n",
               istate->x_min, istate->y_min,
               istate->x_max, istate->y_max);
      } else if (strcmp(argv[i], "-rulerimage") == 0) {
        if (++i >= argc) Usage(argv[0]);
        rulerPPMName = new char [1 + strlen(argv[i])];
        if (!rulerPPMName) {
          fprintf(stderr, "Out of memory\n");
          exit(-1);
        }
        strcpy(rulerPPMName, argv[i]);
      } else if (strcmp(argv[i], "-sub") == 0) {
        if (++i >= argc) Usage(argv[0]);
        istate->afm.ModSubWinSz = atoi(argv[i])/2;
      } else if (strcmp(argv[i], "-xc") == 0) {
        xc_enable();
        xenable=1;

      } else if (strcmp(argv[i], "-bumpimage") == 0) {
        if (++i >= argc) Usage(argv[0]);
        bumpPPM = new PPM(argv[i]);
        if (!bumpPPM->valid) {
          fprintf(stderr, "Cannot read ppm file %s\n",argv[i]);
          exit(-1);
        }
      } else if (strcmp(argv[i], "-gverbosity") == 0) {
        if (++i >= argc) Usage(argv[0]);
        spm_graphics_verbosity = atoi(argv[i]);
      } else if (strcmp (argv[i], "-MIXport") == 0) {
        // clark 7/21/97  -- allow for split communication streams
        if (++i >= argc) Usage (argv[0]);
        istate->UDPport = atoi (argv[i]);
        printf ("UDPport for mix: %d\n", istate->UDPport);
        istate->socketType = SOCKET_MIX;
      } else if (strcmp(argv[i], "-noiseimage") == 0) {
        if (++i >= argc) Usage(argv[0]);
        noisePPM = new PPM(argv[i]);
        if (!noisePPM->valid) {
          fprintf(stderr, "Cannot read ppm file %s\n", argv[i]);
          exit(-1);
        }
      } else if (strcmp(argv[i], "-rulercolor") == 0) {
        if (++i >= argc) Usage(argv[0]);
        ruler_r = (GLubyte)atoi(argv[i]);
        if (++i >= argc) Usage(argv[0]);
        ruler_g = (GLubyte)atoi(argv[i]);
        if (++i >= argc) Usage(argv[0]);
        ruler_b = (GLubyte)atoi(argv[i]);
      } else if (strcmp(argv[i], "-scrapeheight") == 0) {
        if (++i >= argc) Usage(argv[0]);
        istate->afm.scrapeHeight = atof(argv[i]);
      } else if (strcmp (argv[i], "-SPMhost") == 0) {
        // Added by Michele Clark 6/2/97
        // Next arguments are the hostname and port # of SPM server
        if (++i >= argc) Usage (argv[0]);
        strcpy (istate->SPMhost, argv[i]);
        istate->SPMhostptr = istate->SPMhost;
        printf ("SPMhost: %s\n", istate->SPMhost);
        if (++i >= argc) Usage (argv[0]);
        istate->SPMport = atoi (argv[i]);
        printf ("SPMport: %d\n", istate->SPMport);
      } else if (strcmp(argv[i], "-timeframe") == 0) {
        if (++i >= argc) Usage(argv[0]);
        time_frame = atoi(argv[i]);
	framelog = 1;
	frametimer.startlog(time_frame);
      } else if (strcmp(argv[i], "-timerverbosity") == 0) {
        if (++i >= argc) Usage(argv[0]);
        timer_verbosity = atoi(argv[i]);
      } else if (strcmp (argv[i], "-UDPport") == 0) {
        // Added by clark 6/23/97
        // Next argument is UDP port to listen on 
        //  -- will use UDP instead of TCP
        if (++i >= argc) Usage (argv[0]);
        istate->UDPport = atoi (argv[i]);
        printf ("UDPport: %d\n", istate->UDPport);
        istate->socketType = SOCKET_UDP;
 

#ifdef FLOW
      } else if ( (strcmp(argv[i], "-b") == 0) ||
                  (strcmp(argv[i], "-w") == 0) ||
                  (strcmp(argv[i], "-h") == 0) ||
                  (strcmp(argv[i], "-x") == 0) ||
                  (strcmp(argv[i], "-y") == 0) ||
                  (strcmp(argv[i], "-c") == 0) ||
                  (strcmp(argv[i], "-g") == 0) ||
                  (strcmp(argv[i], "-s") == 0) ||
                  // same as microscape
                  //            (strcmp(argv[i], "-f") == 0) ||
                  (strcmp(argv[i], "-o") == 0) ||
                  (strcmp(argv[i], "-v") == 0) ||
                  (strcmp(argv[i], "-l") == 0) ||
                  (strcmp(argv[i], "-p") == 0) ||
                  (strcmp(argv[i], "-S") == 0) ||
                  (strcmp(argv[i], "-m") == 0) ||
                  (strcmp(argv[i], "-C") == 0) ||
                  (strcmp(argv[i], "-H") == 0) ) {
        strcat(pxflArgs, " ");
        strcat(pxflArgs, argv[i]);
        strcat(pxflArgs, " ");
        strcat(pxflArgs, argv[++i]);
        fprintf(stderr, "parsing args: pxfl Args %s\n",pxflArgs);
#endif

      } else if (argv[i][0] == '-') Usage(argv[0]);
      else {
        switch (real_params) {
        default: Usage(argv[0]);
        }
        real_params++;
      }
      i++;
    }
    if (real_params != 0) Usage(argv[0]);
}

void Usage(char* s)
{
  fprintf(stderr, "Usage: %s [-sphere] [-poly] [-dot] [-hole]\n",s);
  fprintf(stderr, "       [-d device] [-do device] [-div device]\n");
  fprintf(stderr, "       [-dsem device]\n");
  fprintf(stderr, "       [-f infile] [-z scale] [-rate rate]\n");
  fprintf(stderr, "       [-call/nocall] [-grid x y] [-perf]\n");
  fprintf(stderr, "       [-i streamfile rate][-o streamfile]\n");
  fprintf(stderr, "       [-std num rate scale] [-nokeybd] [-nodevice]");
  fprintf(stderr, "       [-region lowx lowy highx highy]\n");
  fprintf(stderr, "       [-measure dist] [-nomenus] [-nocpanels]\n");
  fprintf(stderr, "       [-fmods max min (Setpt V)] " );
  fprintf(stderr, "       [-fimgs max min (DrAmp V) setpt (V)]\n" );
  fprintf(stderr, "       [-con | -tap | -tc | -tapsew (def -tc)]\n" );
  fprintf(stderr, "       [-sub numpix] [-move dist]\n" );
  fprintf(stderr, "       [-dir deg] [-color hr hg hb lr lg lb]\n" );
  fprintf(stderr, "       [-relax] [-minsep tmin tsep] [-relax_up_also]\n");
  fprintf(stderr, "       [-splat] [-notk] [-gl] [-nographics]\n" );
  fprintf(stderr, "       [-disp period(ms)] [-x] [-xc] [-b] [-mb3]\n" );
  fprintf(stderr, "       [-draw_when_centered] [-rulerimage file.ppm]\n");
  fprintf(stderr, "       [-rulercolor r g b]");
  fprintf(stderr, "       [-scrapeheight h] [-verbosity n] [-allowdup]\n");
  fprintf(stderr, "       [-timerverbosity n] [-timeframe n]\n");
  fprintf(stderr, "       [-SPMhost host port] [-UDPport port]\n");
  fprintf(stderr, "       [-MIXport port] [-recv] [-alphacolor r g b]\n");
  fprintf(stderr, "       [-marshalltest] [-multithread] [-ugraphics]\n");
  fprintf(stderr, "       [-monitor port] [-collaborate port] [-peer name]\n");
  fprintf(stderr, "       [-renderserver] [-renderclient host]\n");
  fprintf(stderr, "       [-trenderserver] [-trenderclient host]\n");
  fprintf(stderr, "       [-vrenderserver] [-vrenderclient host]\n");
  fprintf(stderr, "       [-logif path] [-replayif path] [-packetlimit n]\n");
  fprintf(stderr, "       [-optimistic] [-phantomrate rate]\n");
  fprintf(stderr, "       [-tesselation stride]\n");
  fprintf(stderr, "       \n");
  //fprintf(stderr, "       -poly: Display poligonal surface (default)\n");
  //fprintf(stderr, "       -sphere: Display as spheres\n");
  //fprintf(stderr, "       -dot: Display as dots\n");
  //fprintf(stderr, "       -hole: Display poligonal surface with holes\n");
  fprintf(stderr, "       -d: Use given spm device (default sdi_stm0)\n");
  fprintf(stderr, "       -do: Use given ohmmeter device (default none)\n");
  fprintf(stderr, "       -div: Use given iv-curve device (default none)\n");
  fprintf(stderr, "       -dsem: Use given SEM device (default none)\n");
  fprintf(stderr, "       -f: Read from given file, not device\n");
  fprintf(stderr, "           Multiple -f options will read multiple files\n");
  fprintf(stderr, "           All must have same grid resolution and range\n");
  fprintf(stderr, "       -z: Scale of z relative to x/y (default 1)\n");
  fprintf(stderr, "       -rate: Rate of scanning (default 20 uMeters/sec)\n");
  fprintf(stderr, "       -color: high to low color range (Red to Blue def)\n");
  fprintf(stderr, "       -fmods: mod force range (def 50,0)\n");
  fprintf(stderr, "       -fimgs: img force range and setpt (def 10,0, 50)\n");
  fprintf(stderr, "       -move: max tip move of dist nm per ms (def 100)\n");
  fprintf(stderr, "       -dir: direction user facing (deg from north) \n");
  fprintf(stderr, "       -call: Use callbacks (default)\n");
  fprintf(stderr, "       -nocall: Do not use callbacks\n");
  fprintf(stderr, "       -grid: Take x by y samples for the grid\n");
  fprintf(stderr, "       -perf: Turn on the perfmeter\n");
  fprintf(stderr, "       -i: Rate at which data is to be read "
                         "from streamfile\n");
  fprintf(stderr, "       -o: Write the STM data stream to streamfile\n");
  fprintf(stderr, "       -std: Take num samples at rate per point\n");
  fprintf(stderr, "             Multiply std_dev by scale to get [0,1]\n");
  fprintf(stderr, "       -region: Scan from low to high in x and y\n");
  fprintf(stderr, "                (Default what scanner has, units are nm)\n");
  fprintf(stderr, "       -sub: Scan a square numXnum around mods (def 24)\n" );
  fprintf(stderr, "       -measure: Put a measure line every dist nm "
                         "(default 10)\n");
  fprintf(stderr, "       -nomenus: Turn off the menus\n");
  fprintf(stderr, "       -nokeybd: Turn off keyboard options\n");
  fprintf(stderr, "       -nodevice: Turn off A/D device\n");
  fprintf(stderr, "       -nocpanels: Turn off the control panels\n");
  fprintf(stderr, "       -showcps: Start the control panels invisible\n");
  fprintf(stderr, "       -showgrid: Start the measuring grid invisible\n");
					/* mod 03/15/94 mf per SW req.	*/
  fprintf(stderr, "       -relax: Perform relaxation compensation\n");
  fprintf(stderr, "       -relax_up_also: Relax when back in image mode\n");
  fprintf(stderr, "       -minsep: min time and seperation after trans\n");
  fprintf(stderr, "       -splat: Splat incoming data into grid\n");
  //fprintf(stderr, "       -snap: Snapshots save dimensioned nxXny (512^2)\n");
  fprintf(stderr, "       -disp: Don't display less than period ms apart (0)\n");
  fprintf(stderr, "       -x: Use the x window display\n");
  fprintf(stderr, "       -xc: Use the x window to display the contour map\n");
  fprintf(stderr, "       -gl: to display on openGL machine\n");
  fprintf(stderr, "       -notk: to turn off tcl/tk interface\n");
  fprintf(stderr, "       -b : Use the button on the Phantom's stylus "
                         "as a trigger\n");
  fprintf(stderr, "       -mb3 : Use the 3rd mouse button as a trigger\n");
  fprintf(stderr, "       -draw_when_centered: Only draw frame when "
                         "user centers\n");
  fprintf(stderr, "       -rulerimage: PPM file to use for the ruler image\n");
  fprintf(stderr, "       -rulercolor: use r g b values for the "
                         "rulergrid color\n");
  fprintf(stderr, "       -scrapeheight: Height above surf in nm\n");
  fprintf(stderr, "       -verbosity: How much info to print (default 0)\n");
  fprintf(stderr, "       -gverbosity: How much graphics info to print "
                         "(default 0)\n");
  fprintf(stderr, "       -timerverbosity: How much timer info to print "
                         "(default 0)\n");
  fprintf(stderr, "       -timeframe: How many frames to time (default 0)\n");
  fprintf(stderr, "       -allowdup: Allow streamfile duplication over net\n");
  fprintf(stderr, "       -SPMhost: host and TCP port of mscope "
                         "(udp = tcp+1)\n");
  fprintf(stderr, "       -UDPport: use UDP and listen on this port\n");
  fprintf(stderr, "       -MIXport: use TCP/UDP and listen for UDP "
                         "on this port\n");
  fprintf(stderr, "       -recv: playback from stream file using network "
                         "receive times\n");
  fprintf(stderr, "       -alphacolor: Set color for alpha texture "
                         "(default 0.0 1.0 0.0)\n");
  fprintf(stderr, "       -marshalltest: Test remote/server graphics pair "
                         "in single process\n");
  fprintf(stderr, "       -multithread: Spawn a second thread for the graphics "
                         "and run multithreaded\n");
  fprintf(stderr, "       -ugraphics: Run with ubergraphics\n");
  fprintf(stderr, "       -monitor:  open a VRPN Forwarder from the "
                         "microscope on this port. OBSOLETE.\n");
  fprintf(stderr, "       -collaborator:  open VRPN Forwarders both to and "
                         "from the microscope on this port. OBSOLETE.\n");
  fprintf(stderr, "       -peer:  connect to named collaborative peer.\n");

  fprintf(stderr, "       -renderserver:  start up as a server for "
                          "remote rendering.\n");
  fprintf(stderr, "       -renderclient:  start up as a client for "
                          "remote rendering,\n");
  fprintf(stderr, "       connected to the named host.\n");

  fprintf(stderr, "       -trenderserver:  start up as a server for "
                          "remote rendering.\n");
  fprintf(stderr, "       -trenderclient:  start up as a client for "
                          "remote rendering,\n");
  fprintf(stderr, "       connected to the named host.\n");

  fprintf(stderr, "       -vrenderserver:  start up as a server for "
                          "remote rendering.\n");
  fprintf(stderr, "       -vrenderclient:  start up as a client for "
                          "remote rendering,\n");
  fprintf(stderr, "       connected to the named host.\n");

  fprintf(stderr, "       -logif path:  open up stream files in specified "
                          "directory to log all use of\n"
                  "       (collaborative) interface.\n");
  fprintf(stderr, "       -replayif path:  replay the interface (\"movie "
                          "mode\") from stream files in the\n"
                  "       specified directory.\n");
  fprintf(stderr, "       -packetlimit n:  while replaying stream files "
                          "play no more than n packets\n"
                  "       before refreshing graphics (0 to disable).\n");
  fprintf(stderr, "       -optimistic:  use optimistic concurrency control.\n");

  exit(-1);
}

void KeyboardUsage (void) {
    fprintf(stderr, "\07\nCommand loop:\n");
    fprintf(stderr, "    File     'Q': Quit without saving snapshots or stream\n");
    fprintf(stderr, "             'q': quit\n");
    fprintf(stderr, "             'o': Output file nano.wrl (VRML)\n");
    fprintf(stderr, "             't': Output file nano.wrl (VRML)\n");
    //fprintf(stderr, "             's': take a data snapshot\n");
    //fprintf(stderr, "             'u': write snapshots to a UNCA file\n");
    //fprintf(stderr, "             'a': write snapshots to an ascii file\n");
    fprintf(stderr, "             'I': Info dump - current system state\n");
    fprintf(stderr, "             '$': Store screen image\n");
    fprintf(stderr, "             '!NNN!': Take NNNxNNN Snapshot\n");
    fprintf(stderr, "    Modes:   'G': Grab world mode\n");
    fprintf(stderr, "             'P': Pulse mode\n");
    fprintf(stderr, "             'E': Engrave mode\n");
    fprintf(stderr, "             '/': Sweep mode\n");
    fprintf(stderr, "             '#': Comb mode\n");
    fprintf(stderr, "             '|': Line mode\n");
    fprintf(stderr, "             '%%': Measure mode\n");
    fprintf(stderr, "             'U': Scale Up mode\n");
    fprintf(stderr, "             'D': Scale Down mode\n");
    fprintf(stderr, "             'N': Feel surface plane blunt tip (LIVE)\n");
    fprintf(stderr, "             'T': Feel surface plane line mode (LIVE)\n");
    fprintf(stderr, "             '^T': Feel surface grid planes mode (CANNED)\n");
    fprintf(stderr, "             '^8': Toggle splatting feel data into grid\n");
    fprintf(stderr, "             'F': Fly mode (default)\n");
    fprintf(stderr, "             'S': Select mode\n");
    fprintf(stderr, "             'H': Height of measure grid\n");
    fprintf(stderr, "    STM:     'A': select All (maximum scan range)\n");
    fprintf(stderr, "             'x': Scan in x fastest (default)\n");
    fprintf(stderr, "             'y': Scan in y fastest\n");
    fprintf(stderr, "             'B': Boustrophedonic scan (default)\n");
    fprintf(stderr, "             'R': Raster scan\n");
    fprintf(stderr, "             '+':   Show raster in + direction (default)\n");
    fprintf(stderr, "             '-':   Show raster in - direction\n");
    fprintf(stderr, "             '=': Toggle piezo relaxation compensation\n");
    fprintf(stderr, "             '~': Toggle drift compensation\n");
    fprintf(stderr, "             'O': Toggle Ohmmeter on and off\n");
    fprintf(stderr, "             'pnnnp': Set the imaging SetPoint\n");
    fprintf(stderr, "    Display: 'V': Visible pulses (default)\n");
    fprintf(stderr, "             'v': Invisible pulses\n");
    fprintf(stderr, "             'C': Clear existing pulses\n");
    fprintf(stderr, "             'L': Lock head position for screen viewing (default)\n");
    fprintf(stderr, "             'l': unlock head position\n");
    fprintf(stderr, "             '.': Refresh display from grid\n" );
    fprintf(stderr, "             '^': Recenter in locked head mode\n" );
    fprintf(stderr, "             '*': Position lighting\n" );
    fprintf(stderr, "  Utility:   'X': Close and re-open ARM\n");
    fprintf(stderr, "             'i': Adjust information verbosity\n");
    fprintf(stderr, "  Playback:  '0'-'9': Rate of playback\n");
    fprintf(stderr, "             '?': Where in current stream file (bytes)\n");
    fprintf(stderr, "             '>': Advance approx 1 frame (read stream)\n");
    fprintf(stderr, "             '<': Backup approx 1 frame (read stream)\n");
    fprintf(stderr, "             '@n@': Goto <n>th byte in current stream file\n");
    if(xenable){
	fprintf(stderr, "Graph Mode: 'w':Switch to graph mode\n");
	fprintf(stderr, "             mouse button 1: Add a vertex to the current line strip\n");
	fprintf(stderr, "             mouse button 2: Graph the height versus the length of the current line strip\n");
	fprintf(stderr, "Normal Mode: 'W': Switch to normal mode and graph the current line strip if any\n"); 
	fprintf(stderr, "             mouse button 1: Tell grid value at location\n");
	fprintf(stderr, "             mouse button 2: Modify following the path from press to release\n");
	fprintf(stderr, "Both Modes:  mouse button 3: Request and print point scan\n");
    } // end if(xenable)
	fprintf(stderr, "             'f': move scanline to the left\n");
	fprintf(stderr, "             'j': move scanline to the right\n");
	fprintf(stderr, "             'h': move scanline forward\n");
	fprintf(stderr, "             'g': move scanline backward\n");

}

void sharedGraphicsServer (void * data) {
  vrpn_Connection * c;
  nmg_Graphics * g;
  Semaphore * ps = ((ThreadData *) data)->ps;
  int retval;

//fprintf(stderr, "g>In graphics thread.\n");

  // INITIALIZE

  // start a server connection
//fprintf(stderr, "g>Graphics thread starting vrpn server\n");
  c = new vrpn_Synchronized_Connection (WellKnownPorts::graphicsControl);

  // start a graphics implementation
//fprintf(stderr, "g>Graphics thread starting graphics implementation\n");
  g = new nmg_Graphics_Implementation
    (dataset, minC, maxC, rulerPPMName, c);

  // release the main process to run

//fprintf(stderr, "g>Graphics thread relesing main process\n");
  retval = ps->v();
  if (retval) {
    fprintf(stderr, "Coprocess couldn't release main process to run!\n");
    exit(0);
  }

  // STEADY STATE

//fprintf(stderr, "g>Graphics thread entering mainloop\n");
  while (1) {

    TIMERVERBOSE(1, frametimer, "------------------ Frame timer ---------------");
    TIMERLOG(frametimer);

    g->mainloop();
  }
}

/**
spawnSharedGraphics()

Starts a new thread in process sharedGraphicsServer();
blocks until that thread signals completion.
*/
void spawnSharedGraphics (void) {

  ThreadData td;
  int retval;

  // allocate semaphore in td.ps
  // Initialize to zero;  we don't leave this function until
  // the coprocess has finished initializing.
//fprintf(stderr, "Allocating semaphore\n");
  td.ps = new Semaphore (0);

  // Spawn the coprocess
//fprintf(stderr, "Spawning new thread\n");
  graphicsServerThread = new Thread (sharedGraphicsServer, td);

//fprintf(stderr, "Spawned;  now starting the new thread\n");
  graphicsServerThread->go();

  // Block until it's initialized
//fprintf(stderr, "Blocking for new thread\n");
  retval = td.ps->p();
  if (retval != 1) {
    fprintf(stderr, "Main process wasn't successfully "
            "released to run by coprocess.\n");
    graphicsServerThread->kill();
    exit(0);
  }

  // Release the semaphore in case either of us wants to grab it again.
  td.ps->v();
}

// T. Hudson 8 Mar 2000
// Would like to replace createGraphics() with a proper Abstract Factory
// some day.
void createGraphics (MicroscapeInitializationState & istate) {

  char name [50], qualifiedName [50];
  int retval;

  switch (istate.graphics_mode) {

    case NO_GRAPHICS:
      fprintf(stderr, "NO GRAPHICS disabled for ease of development.\n");
      fprintf(stderr, "To reenable, update nmg_Graphics_Null, add to the\n"
                      "server_microscape/ and graphics/ makefile, and\n"
                      "change this code (in microscape.c)\n");
      exit(0);

      //fprintf(stderr, "Using NO GRAPHICS.\n");
      //graphics = new nmg_Graphics_Null(dataset, minC, maxC);
      break;

    case LOCAL_GRAPHICS:
      fprintf(stderr, "Using local GL graphics implementation.\n");
      graphics = new nmg_Graphics_Implementation(dataset,minC, maxC, 
                                                      rulerPPMName);
      graphics = new nmg_Graphics_Timer(graphics, &graphicsTimer);
      break;

    case SHMEM_GRAPHICS:
      {
        fprintf(stderr, "Starting up shared memory implementation.\n");

        spawnSharedGraphics();  // blocks until coprocess sets up

        retval = gethostname(name, 45);
        if (retval) {
          fprintf(stderr, "gethostname() failed;  "
                          "can't start up shared memory!\n");
          exit(0);
        }
        sprintf(qualifiedName, "nmg@%s:%d", name,
                WellKnownPorts::graphicsControl);

        range_ps = new Semaphore (1);

        shmem_connection = vrpn_get_connection_by_name (qualifiedName);
        graphics = new nmg_Graphics_Remote (shmem_connection);
        graphics = new nmg_Graphics_Timer (graphics, &graphicsTimer);
      }
      break; 

    case TEST_GRAPHICS_MARSHALLING:
      fprintf(stderr, "Running local GL graphics implementation PLUS "
              "nmg_Graphics_Remote\n    and VRPN as a message handler.\n"
              "    THIS MODE IS FOR TESTING ONLY.\n");
      shmem_connection = new vrpn_Synchronized_Connection
                            (WellKnownPorts::graphicsControl);
      gi = new nmg_Graphics_Implementation
               (dataset,
                minC, maxC, rulerPPMName, shmem_connection);
      graphics = new nmg_Graphics_Remote (shmem_connection);
      break;

    case RENDER_SERVER:
      fprintf(stderr, "Starting up as a rendering server "
              "(orthographic projection).\n"
              "    THIS MODE IS FOR TESTING ONLY.\n");

      renderServerOutputConnection =
              new vrpn_Synchronized_Connection
                        (WellKnownPorts::remoteRenderingData);

      //renderServerControlConnection = renderServerOutputConnection;
      renderServerControlConnection = 
              new vrpn_Synchronized_Connection
                          (WellKnownPorts::graphicsControl);

      graphics = new nmg_Graphics_RenderServer
                 (dataset, minC, maxC, renderServerOutputConnection,
                  nmg_Graphics::VERTEX_COLORS,
                  nmg_Graphics::VERTEX_DEPTH,
                  nmg_Graphics::ORTHO_PROJECTION,
                  100, 100, rulerPPMName, renderServerControlConnection);

      break;

    case TEXTURE_SERVER:
      fprintf(stderr, "Starting up as a texture rendering server "
              "(orthographic projection).\n"
              "    THIS MODE IS FOR TESTING ONLY.\n");

      renderServerOutputConnection =
              new vrpn_Synchronized_Connection
                        (WellKnownPorts::remoteRenderingData);

      //renderServerControlConnection = renderServerOutputConnection;
      renderServerControlConnection = 
              new vrpn_Synchronized_Connection
                          (WellKnownPorts::graphicsControl);

      graphics = new nmg_Graphics_RenderServer
                 (dataset, minC, maxC, renderServerOutputConnection,
                  nmg_Graphics::SUPERSAMPLED_COLORS,
                  nmg_Graphics::NO_DEPTH,
                  nmg_Graphics::ORTHO_PROJECTION,
                  512, 512, rulerPPMName, renderServerControlConnection);

      break;

    case VIDEO_SERVER:
      fprintf(stderr, "Starting up as a video rendering server "
              "(perspective projection).\n"
              "    THIS MODE IS FOR TESTING ONLY.\n");

      renderServerOutputConnection =
              new vrpn_Synchronized_Connection
                        (WellKnownPorts::remoteRenderingData);

      //renderServerControlConnection = renderServerOutputConnection;
      renderServerControlConnection = 
              new vrpn_Synchronized_Connection
                          (WellKnownPorts::graphicsControl);

      graphics = new nmg_Graphics_RenderServer
               (dataset, minC, maxC, renderServerOutputConnection,
                nmg_Graphics::SUPERSAMPLED_COLORS,
                nmg_Graphics::NO_DEPTH,
                nmg_Graphics::PERSPECTIVE_PROJECTION,
                512, 512, rulerPPMName, renderServerControlConnection);

      break;

    case RENDER_CLIENT:
      fprintf(stderr, "Starting up as a rendering client "
              "(expecting peer rendering server %p to supply images).\n",
              istate.graphicsHost);

      sprintf(qualifiedName, "nmg Graphics Renderer@%s:%d",
              istate.graphicsHost,
              WellKnownPorts::remoteRenderingData);
      renderClientInputConnection = vrpn_get_connection_by_name
                           (qualifiedName);

      sprintf(qualifiedName, "nmg Graphics Renderer@%s:%d",
              istate.graphicsHost,
              WellKnownPorts::graphicsControl);
      renderServerControlConnection = vrpn_get_connection_by_name
                           (qualifiedName);

      // By having graphics send on renderServerControlConnection
      // and gi listen on it, graphics effectively sends commands
      // to BOTH the RenderClient and the RenderServer;
      // each will execute those it needs to.

      //graphics = new nmg_Graphics_Remote (renderServerControlConnection);
      graphics = new nmg_Graphics_RenderClient
           (dataset, minC, maxC, renderClientInputConnection,
            nmg_Graphics::VERTEX_COLORS, nmg_Graphics::VERTEX_DEPTH,
            nmg_Graphics::ORTHO_PROJECTION,
            100, 100,
            renderServerControlConnection, &graphicsTimer);

      graphics = new nmg_Graphics_Timer (graphics, &graphicsTimer);

      break;

    case TEXTURE_CLIENT:
      fprintf(stderr, "Starting up as a texture rendering client "
              "(expecting peer rendering server %p to supply images).\n",
              istate.graphicsHost);

      sprintf(qualifiedName, "nmg Graphics Renderer@%s:%d",
              istate.graphicsHost,
              WellKnownPorts::remoteRenderingData);
      renderClientInputConnection = vrpn_get_connection_by_name
                           (qualifiedName);

      sprintf(qualifiedName, "nmg Graphics Renderer@%s:%d",
              istate.graphicsHost,
              WellKnownPorts::graphicsControl);
      renderServerControlConnection = vrpn_get_connection_by_name
                           (qualifiedName);

      // By having graphics send on renderServerControlConnection
      // and gi listen on it, graphics effectively sends commands
      // to BOTH the RenderClient and the RenderServer;
      // each will execute those it needs to.

      //graphics = new nmg_Graphics_Remote (renderServerControlConnection);
      graphics = new nmg_Graphics_RenderClient
           (dataset, minC, maxC, renderClientInputConnection,
            nmg_Graphics::SUPERSAMPLED_COLORS, nmg_Graphics::NO_DEPTH,
            nmg_Graphics::ORTHO_PROJECTION,
            512, 512, renderServerControlConnection, &graphicsTimer);

      graphics = new nmg_Graphics_Timer (graphics, &graphicsTimer);

      break;

    case VIDEO_CLIENT:
      fprintf(stderr, "Starting up as a video rendering client "
              "(expecting peer rendering server %p to supply images).\n",
              istate.graphicsHost);

      sprintf(qualifiedName, "nmg Graphics Renderer@%s:%d",
              istate.graphicsHost,
              WellKnownPorts::remoteRenderingData);
      renderClientInputConnection = vrpn_get_connection_by_name
                           (qualifiedName);

      sprintf(qualifiedName, "nmg Graphics Renderer@%s:%d",
              istate.graphicsHost,
              WellKnownPorts::graphicsControl);
      renderServerControlConnection = vrpn_get_connection_by_name
                           (qualifiedName);

      // By having graphics send on renderServerControlConnection
      // and gi listen on it, graphics effectively sends commands
      // to BOTH the RenderClient and the RenderServer;
      // each will execute those it needs to.

      // TODO
      graphics = new nmg_Graphics_RenderClient
           (dataset, minC, maxC, renderClientInputConnection,
            nmg_Graphics::SUPERSAMPLED_COLORS, nmg_Graphics::NO_DEPTH,
            nmg_Graphics::PERSPECTIVE_PROJECTION,
            512, 512, renderServerControlConnection, &graphicsTimer);

      graphics = new nmg_Graphics_Timer (graphics, &graphicsTimer);

      ((nmg_Graphics_Timer *) graphics)->timeViewpointChanges(VRPN_TRUE);

      break;

    default:
      fprintf(stderr, "Unimplemented graphics mode!\n");
      exit(0);
  }// end switch (istate.graphics_mode)

}

void initialize_rtt (void) {

  rtt_server_connection = new vrpn_Synchronized_Connection
           (WellKnownPorts::roundTripTime);
  rtt_server = new vrpn_Analog_Server ("microscope_rtt",
                                       rtt_server_connection);
  rtt_server->setNumChannels(1);

  fprintf(stderr, "Service named microscope_rtt is now listening "
                  "on port %d.\n", WellKnownPorts::roundTripTime);

}

void update_rtt (void) {

  static struct timeval lasttime;
  vrpn_Synchronized_Connection * scp;
  struct timeval now;
  struct timeval rtt;
  struct timezone tz;
  if ((!microscope_connection)||(read_mode == READ_STREAM)) return;

  /* gettimeofday takes two arguments */
  gettimeofday(&now,&tz);

  // quick and dirty approximately-one-second interval

  if (now.tv_sec != lasttime.tv_sec) {

    // DANGER:  forces downcast of vrpn_Connection to
    //   vrpn_Synchronized_Connection

    scp = (vrpn_Synchronized_Connection *) microscope_connection;
	if ( (scp == NULL) || (scp->pClockRemote == NULL) ) {
		fprintf(stderr,"Warning: Calling update_rtt when pClockRemote == NULL\n");
	} else {
		rtt = scp->pClockRemote->currentRTT();
		rtt_server->channels()[0] = (double) rtt.tv_sec +
                                (double) rtt.tv_usec / 1000000.0;
		rtt_server->report_changes();
		rtt_server_connection->mainloop();
	}
  }

  lasttime = now;
}

int main(int argc, char* argv[])
{
    /* Things needed for the timing information */
    MicroscapeInitializationState istate;
    struct          timeval d_time;
    struct          timeval time1,time2;
    struct          timezone zone1,zone2;
    long            start,stop;
    long            interval;
    float             timing;
    float looplen;
    long            n = 0L;
    long	    n_displays = 0L;

    int		i;

    //    int	real_params = 0;		/* Non-flag parameters */

    int new_value_of_rate_knob;
    int old_value_of_rate_knob;

    printf("Microscape version %d.%d\n",MICROSCAPE_MAJOR_VERSION,
	MICROSCAPE_MINOR_VERSION);

    decoration = new nmb_Decoration;
    if (!decoration) {
	fprintf(stderr, "Cannot initialize decorations.\n");
	return -1;
    }	


    /* Parse the command line */

    ParseArgs(argc, argv, &istate);


    if(glenable){
    	char	*envir;
	envir = getenv("TRACKER");

	if (envir == NULL)
	{
	  fprintf(stderr, "Microscape warning:  No tracker environment variable set.  ");
	  fprintf(stderr, "Using default values of null for head and hand tracking.\n");

	  /* Use the default null trackers */
	  headTrackerName = (char *)"null";
	  handTrackerName = (char *)"null";
	}
	else {
	  /* Parse the env string to pull out the name of the arm *
	   * and the name of the tracker we're using, if any.     */
	  headTrackerName = strtok(envir, " ");
	  handTrackerName = strtok(NULL, " ");

	  printf("Head tracker is %s\n", headTrackerName);
	  printf("Hand tracker is %s\n", handTrackerName);
	
	  /* We know at least the head tracker was specified, so check for a
	   * null hand tracker.
	   */
	  if (handTrackerName == NULL)
	    handTrackerName = (char *)"null";
	}

	/* Check to see if a tracker is being used for head tracking */
	if (strcmp("null", headTrackerName) != 0)
	  head_tracked = VRPN_TRUE;
	envir = getenv("BDBOX");
	if (envir == NULL)
	{
	  fprintf(stderr, "Microscape warning:  No sgi button/dial box set.\n");
	  bdboxName = (char *)"null";
	}
	else
	{
	  bdboxName = strtok(envir, " ");
	  printf("sgibdbox is %s\n", bdboxName);
	  if (bdboxName == NULL)
	    bdboxName = (char *)"null";
	}
	envir = getenv("HOST");
	fprintf(stderr, "HOST: %s\n", envir);
	sprintf(nM_coord_change_server_name, "ccs0@%s", envir);
	fprintf(stderr, "nM_coord_change_server_name: %s\n",
		nM_coord_change_server_name);
	sprintf(local_ModeName, "Cmode0@%s", envir);
	fprintf(stderr, "local_ModeName: %s\n",
		local_ModeName);
    } /* if (glenable) */

    if ( (tcl_script_dir=getenv("NM_TCL_DIR")) == NULL) {
	tcl_script_dir=tcl_default_dir;
    }

    // Load the names of usable color maps
    VERBOSE(1, "Loading color maps");
    loadColorMapNames();

    // Load the names of the image processing programs available.
    VERBOSE(1, "Loading external program names");
    loadProcProgNames();

    // Load the ppm textures available
    VERBOSE(1, "Loading PPM textures");
    loadPPMTextures();

#ifdef USE_VRPN_MICROSCOPE

    // Open a port (4581) that will report unfiltered observed RTT
    // to the microscope.
    initialize_rtt();

    long logmode = vrpn_LOG_NONE;
    long remote_logmode = vrpn_LOG_NONE;

    if (istate.afm.writingStreamFile)
      logmode |= vrpn_LOG_INCOMING;
    if (istate.afm.writingStreamFile)
      remote_logmode |= vrpn_LOG_INCOMING;

    fprintf(stderr, "About to init microscope\n");

    // If user has specified a stream file with -i option, try
    // to open it as a VRPN stream file. 
    if (istate.afm.readingStreamFile) {
	if (strcmp(istate.afm.deviceName, "null")) {
	    fprintf(stderr, "Both input stream and microscope device specified, using input stream\n");
	}
	sprintf(istate.afm.deviceName, "file:%s", istate.afm.inputStreamName);

    } 
    // Check for no microscope connection - i.e. we are opening Topo
    // or PPM files.
    if (strcmp(istate.afm.deviceName, "null")==0) {
	microscope_connection= NULL;
    } else {
    // otherwise, open the device specified. 
    fprintf(stderr, "   The connection name is %s\n", istate.afm.deviceName);
    microscope_connection = vrpn_get_connection_by_name
	(istate.afm.deviceName,
	 istate.afm.writingStreamFile ? istate.afm.outputStreamName
                                    : (char *) NULL,
	 logmode,
	 istate.writingRemoteStream ? istate.remoteOutputStreamName
                                  : (char *) NULL);
    if (!microscope_connection) {
	fprintf(stderr, "   Couldn't open connection to %s.\n",
		istate.afm.deviceName);
	exit(0);
    }
    
    // Decide whether reading vrpn log file or doing live.
    vrpnLogFile = microscope_connection->get_File_Connection();
    if (!vrpnLogFile) {
	fprintf(stderr, "   microscape.c:: in Live mode\n");
    } else {
	// Allow the user to open a stream file with "-d file:..." 
	// Set the read_mode. 
	//      But we don't need to set read_mode to READ_DEVICE in
	//      above if case, since that's default.
	read_mode = READ_STREAM;
	istate.afm.readingStreamFile = 1;
	//decoration->rateOfTime = 3;	// use 3 for rate default
	fprintf(stderr, "   microscape.c:: in Replay mode\n");
    }
  // BEFORE we call mainloop on our connection to the microscope,
  // open up a forwarder and spin until it is connected to.

  if (istate.monitorPort != -1) {

    vrpn_StreamForwarder * monitor;
    char * sname = NULL;

    sname = vrpn_copy_service_name(istate.afm.deviceName);

    fprintf(stderr, "Opening monitor connection on port %d;  "
                    "service name is %s.\n",
            istate.monitorPort,
            sname);

    monitor_forwarder_connection =
       new vrpn_Synchronized_Connection (istate.monitorPort);

    monitor = new vrpn_StreamForwarder
                        (microscope_connection, sname,
                         monitor_forwarder_connection, sname);

    nmm_Microscope::registerMicroscopeInputMessagesForForwarding(monitor);

    fprintf(stderr, "Blocking until monitor interface connects.\n");

    while (!monitor_forwarder_connection->connected())
      monitor_forwarder_connection->mainloop();

    if (sname) {
      delete [] sname;
      sname = NULL;
    }
    
  }
  if (istate.collabPort != -1) {

    vrpn_StreamForwarder * collaborator;
    vrpn_StreamForwarder * collaboratorReverse;
    char * sname = NULL;

    sname = vrpn_copy_service_name(istate.afm.deviceName);

    fprintf(stderr, "Opening collaborator connection on port %d;  "
                    "service name is %s.\n",
            istate.collabPort,
            sname);

    collab_forwarder_connection =
        new vrpn_Synchronized_Connection (istate.collabPort);

    // Hook up forwarders BOTH WAYS:
    //   collaborator sends data to a collaborator (just like monitor).
    //   collaboratorReverse sends commands from the collaborator to
    //   the microscope controller.

    collaborator = new vrpn_StreamForwarder
                        (microscope_connection, sname,
                         collab_forwarder_connection, sname);
    collaboratorReverse = new vrpn_StreamForwarder
                        (collab_forwarder_connection, sname,
                         microscope_connection, sname);

    nmm_Microscope::registerMicroscopeInputMessagesForForwarding(collaborator);
    nmm_Microscope::registerMicroscopeOutputMessagesForForwarding
                        (collaboratorReverse);

    fprintf(stderr, "Blocking until collaborator interface connects.\n");

    while (!collab_forwarder_connection->connected())
      collab_forwarder_connection->mainloop();

    if (sname) {
      delete [] sname;
      sname = NULL;
    }
  }
    }

    // XXX Due to BUG in cygwin gettimeofday, we need to sync the clocks
    // between topo and microscape before we handle any microscope message.
    // so call mainloop a few times here before creating the microscope.
    /* Temp removed so we can be sure SGI version works. 
    if ( read_mode != READ_STREAM) {
	for (int i = 0; i<50; i++) {      
	    microscope_connection->mainloop();
	    sleep(0.001);
	}
    }
    */

    microscope = new nmm_Microscope_Remote (istate.afm, microscope_connection);
    if (!microscope) {
      fprintf(stderr, "Couldn't create Microscope Remote.\n");
      exit(0);
    }

    fprintf(stderr, "Microscope initialized\n");


    if (vrpnLogFile) {
      vrpnLogFile->limit_messages_played_back(istate.packetlimit);
    }

#else

    fprintf(stderr, "About to init microscope\n");
    microscope = new Microscope (istate.afm, decoration->rateOfTime);
    fprintf(stderr, "Microscope initialized\n");

    microscope->setPlaybackLimit(istate.packetlimit);  // HACK TCH 15 Feb 2000

#endif

#ifdef  linux
    BCDebug     debug("createGrids");
    debug.turnOff();
    fprintf(stderr, "XXX -- TURNING OFF DEBUGGING INFO!!!!\n");
#endif

    // Must get hostname before initializing nmb_Dataset, but
    // should do it after VRPN starts, I think.
    char * hnbuf = new char[256];
    if (gethostname(hnbuf, 256)) {
	// get host failed! Try environment variable instead
	delete [] hnbuf;
	if ((hnbuf = getenv("HOSTNAME")) != NULL){
	    // this is a global tclvar_String, so we can see it in tcl.
	    my_hostname = hnbuf;
	    // maybe use COMPUTERNAME?
	}
    } else {
	// this is a global tclvar_String, so we can see it in tcl.
	my_hostname = hnbuf;
	delete [] hnbuf;
    }

    VERBOSE(1, "Creating grids");
    char wtl_buffer[1000];
    sprintf(wtl_buffer, "num_x = %d, num_y = %d, read_mode = %d\n", 
             istate.num_x, istate.num_y, read_mode);
    VERBOSE(2, wtl_buffer);

    dataset = new nmb_Dataset (istate.use_file_resolution,
                               istate.num_x, istate.num_y,
                               istate.x_min, istate.x_max,
                               istate.y_min, istate.y_max, read_mode,
                               (const char **) istate.stm_file_names,
                               istate.num_stm_files, 
                               (const char **) istate.image_file_names,
                               istate.num_image_files,
			       (const char *)my_hostname,
                               allocate_TclNet_string);

    VERBOSE(1, "Created dataset");

    if (!dataset) {
      fprintf(stderr, "Cannot initialize dataset.\n");
      return -1;
    }
    if (read_mode == READ_FILE)
      guessAdhesionNames();

    VERBOSE(2, "Guessed adhesion names.");


//    decoration = new nmb_Decoration;
//    if (!decoration) {
//      fprintf(stderr, "Cannot initialize decorations.\n");
//      return -1;
//    }
//    decoration->rateOfTime = instream_rate;

    microscope->InitializeDataset(dataset);
    VERBOSE(1, "Initialized dataset");
    microscope->InitializeDecoration(decoration);
    VERBOSE(1, "Initialized decoration");
    microscope->InitializeTcl(tcl_script_dir);
    VERBOSE(1, "Initialized tcl");
    setupStateCallbacks(microscope);
    VERBOSE(1, "Setup state callbacks");

    setupCallbacks(dataset, microscope);
    VERBOSE(1, "Setup dataset+microscope callbacks");
    setupCallbacks(dataset);
    VERBOSE(1, "Setup dataset callbacks");
    setupCallbacks(microscope);
    VERBOSE(1, "Setup microscope callbacks");
    setupCallbacks(decoration);
    // log modifications into a file

    ModFile * modfile = new ModFile;
    microscope->registerPointDataHandler(ModFile::ReceiveNewPoint, modfile);
    microscope->registerModifyModeHandler(ModFile::EnterModifyMode, modfile);
    microscope->registerImageModeHandler(ModFile::EnterImageMode, modfile);
    VERBOSE(1, "Created new mod file");

    // display modifications on a strip chart

    GraphMod * graphmod = new GraphMod;

    microscope->registerPointDataHandler(GraphMod::ReceiveNewPoint, graphmod);
    microscope->registerModifyModeHandler(GraphMod::EnterModifyMode, graphmod);
    microscope->registerImageModeHandler(GraphMod::EnterImageMode, graphmod);
    microscope->registerScanlineModeHandler(GraphMod::EnterScanlineMode,
						graphmod);
    microscope->registerScanlineDataHandler(GraphMod::ReceiveNewScanline,
						graphmod);

    VERBOSE(1, "Created new GraphMod");

    // Allow another result to be requested when the last is received
    // when using the Slow Line tool. 
    microscope->registerPointDataHandler(slow_line_ReceiveNewPoint, microscope);

    createGraphics(istate);

    setupCallbacks(dataset, graphics);
    setupCallbacks(graphics);

    graphics->setColorMapDirectory(colorMapDir);
    graphics->setTextureDirectory(textureDir);
    graphics->setAlphaColor(alpha_red, alpha_green, alpha_blue);

    if (rulerPPMName)
      graphics->loadRulergridImage(rulerPPMName);

    // initialize graphics
    VERBOSE(1, "Before graphics initialization");
    if (glenable)
    {
#if !defined(FLOW) && !defined(V_GLUT)  /* don't use with glut or PixelFlow */
	/*INITIALIZE EVENT MASK FOR VLIB WINDOW -- DCR Sept 29, 1997*/
	XSelectInput(VLIB_dpy,VLIB_win,ResizeRedirectMask);
#elif defined(V_GLUT)
	/* Probably need to open X-Windows or MS-Windows Input here... */
#endif

	/* Set initial user mode */
	for (i = 0; i < NUM_USERS; i++)
	   user_mode[i] = USER_GRAB_MODE;

	// Disable the screen saver if we are on the Onyx and using
	// openGL.  This keeps it from blanking out in the middle of an
	// experiment.
#ifdef	sgi
	if (glenable) {
		char buffer [1000];
		char ddname [] = ":0";
		char * dname;

		dname = getenv("V_X_DISPLAY");
		if (!dname)
			dname = ddname;
		sprintf(buffer, "DISPLAY=%s; export DISPLAY; blanktime 0",
			dname);

		system(buffer);

	}
#endif


#if !defined(FLOW) && !defined(V_GLUT)
	if( (VLIB_dpy == NULL) || ((void*)VLIB_win == NULL) ) {
		printf("VLIB display vars=NULL no virtual arm\n");	
	}
	else{ 
		printf("VLIB_win == INITIALIZED\n");
		printf("VLIB_dpy == INITIALIZED\n");
		XSelectInput(VLIB_dpy,VLIB_win,StructureNotifyMask | 
		    ButtonPressMask | ButtonReleaseMask  | PointerMotionMask );
		printf("Activating Virtual XArm\n");
	}
#endif
	
    } // end if (glenable)

  //draw measure lines
    VERBOSE(1, "Initializing measure lines");

    BCPlane *height_plane = dataset->inputGrid->getPlaneByName(dataset->heightPlaneName->string());
    if (height_plane == NULL) { 
       height_plane = dataset->ensureHeightPlane(); 
    }

  decoration->red.moveTo(height_plane->minX(), height_plane->minY(),
                          height_plane);
  decoration->green.moveTo(height_plane->maxX(), height_plane->minY(),
                            height_plane);
  decoration->blue.moveTo(height_plane->maxX(), height_plane->maxY(),
                           height_plane);
 // DO NOT doCallbacks()?


    // XXX - setting up vrpn callbacks for tracker should happen after
    // v_create_world() has been called (by new nmg_Graphics) because
    // otherwise there is no place to store the tracker reports
    // (this space is allocated in v_create_world())
    // Initialize force, tracker, a/d device, sound
    VERBOSE(1,"Before tracker enable");
    if (glenable) {
        if (peripheral_init()){
                fprintf(stderr,"Cannot initialize peripheral devices\n");
                return(-1);
        }
        else if (register_vrpn_callbacks()){
                fprintf(stderr,"Cannot setup tracker callbacks\n");
                return(-1);
        }
    }

  // NANOX

  char sfbuf [1024];

  if (istate.replayInterface) {
    sprintf(sfbuf, "file:%s/SharedIFSvrLog-%ld.stream", istate.logPath,
            istate.logTimestamp.tv_sec);
    collaboratingPeerServerConnection = 
      vrpn_get_connection_by_name (sfbuf);
  } else {

    // All interface log files opened by this session have the same
    // timestamp, taken right here.
    gettimeofday(&loggingTimestamp, NULL);

    sprintf(sfbuf, "%s/SharedIFSvrLog-%ld.stream", istate.logPath,
            loggingTimestamp.tv_sec);
    collaboratingPeerServerConnection = 
  	new vrpn_Synchronized_Connection
          (WellKnownPorts::collaboratingPeerServer,
           istate.logInterface ? sfbuf : NULL,
           istate.logInterface ? vrpn_LOG_INCOMING | vrpn_LOG_OUTGOING :
                              vrpn_LOG_NONE);

    if (!collaboratingPeerServerConnection->doing_okay()) {
      fprintf(stderr, "ERROR:  "
                      "Couldn't open collaboration server connection.\n");
      //shutdown_connections();
      //exit(0);
    }
  }

  replayingInterface = istate.replayInterface;
  loggingInterface = istate.logInterface;
  if (loggingInterface) {
    strcpy(loggingPath, istate.logPath);
  }
  if (replayingInterface) {
    strcpy(loggingPath, istate.logPath);
    loggingTimestamp = istate.logTimestamp;
  }

  if (replayingInterface) {
    sprintf(sfbuf, "file:%s/PrivateIFLog-%ld.stream", loggingPath,
            loggingTimestamp.tv_sec);
    interfaceLogConnection = vrpn_get_connection_by_name (sfbuf);
  } else if (loggingInterface) {
    sprintf(sfbuf, "%s/PrivateIFLog-%ld.stream", loggingPath,
            loggingTimestamp.tv_sec);
    interfaceLogConnection = new vrpn_Synchronized_Connection
            (WellKnownPorts::interfaceLog, sfbuf, vrpn_LOG_OUTGOING);
    if (!interfaceLogConnection->doing_okay()) {
      fprintf(stderr, "ERROR:  Couldn't open log file.\n");
      //shutdown_connections();
      //exit(0);
    }
  }

  if (collaboratingPeerServerConnection) {
printf("got collaboratingPeerServerConnection\n");
      nM_coord_change_server = 
    	  new nM_coord_change (nM_coord_change_server_name,
                               vrpnHandTracker[0],
      	                       (vrpn_Synchronized_Connection *)
                                   collaboratingPeerServerConnection);
printf("nM_coord_change_server initialized\n");

    nM_coord_change_server->registerPeerSyncChangeHandler
         (handle_peer_sync_change, NULL);
    // Set up the server to share the mode
    vrpnMode_Local = new vrpn_Analog_Server( local_ModeName,
		collaboratingPeerServerConnection );
    if (vrpnMode_Local == NULL) {
	fprintf(stderr,"ERROR: Cannot open server for Mode sharing\n");
    } else {
	vrpnMode_Local->setNumChannels(1);
    }

    // NANOX
    setupSynchronization(collaboratingPeerServerConnection,
                         interfaceLogConnection, dataset, microscope);
  }

    setupCallbacks(forceDevice);
    handTracker_update_rate = istate.phantomRate;

    VERBOSE(1,"Before Tk initialization");
    if(tkenable) {
      // init_Tk_control_panels creates the interpreter and adds most of
      // the Tk widgits
      init_Tk_control_panels(tcl_script_dir, istate.useOptimism);
      set_Tk_command_handler(handleCharacterCommand);
      VERBOSE(1, "done initialising the control panels\n");
    }

  // NANOX - TCH 14 Oct 99
  // Allow command-line specification of machine to synchronize with.
  // Must come after Tcl init.
  // nmui_Component::initializeConnection() must be called *after* the
  // connection is truly established.
  if (istate.openPeer) {
    collab_machine_name = istate.peerName;
  }

    VERBOSE(1, "Before SPM initialization");
    if (stm_init(set_region, set_mode,
                 istate.socketType, istate.SPMhostptr,
                 istate.SPMport, istate.UDPport)) {
	fprintf(stderr, "Cannot initialize the spm\n");
	return(-1);
    }


    VERBOSE(1, "Before french ohmmeter initialization");
    //if ( tkenable && (dataset->inputGrid->readMode() == READ_DEVICE) ) {
    if ( tkenable ) {
      // create the ohmmeter control panel
      // Specification of ohmmeter device: first look at command line arg
      // and then check environment variable
      if (strcmp(istate.ohm.deviceName, "null") == 0){
        char *ohm_device = getenv("OHMMETER");
      	if (ohm_device != NULL){
          strcpy(istate.ohm.deviceName, ohm_device);
        }
      }
      if (strcmp(istate.ohm.deviceName, "null") != 0) {
        printf("main: attempting to connect to ohmmeter: %s\n",
                istate.ohm.deviceName);
        ohmmeter_connection = vrpn_get_connection_by_name
	  (istate.ohm.deviceName,
	   istate.ohm.writingLogFile ? istate.ohm.outputLogName
	   : (char *) NULL,
	   vrpn_LOG_INCOMING);
        if (!ohmmeter_connection) {
	  fprintf(stderr, "Couldn't open connection to %s.\n",
		  istate.ohm.deviceName);
	  //exit(0);
        } else {
	  // Decide whether reading vrpn log file or a real device
	  ohmmeterLogFile = ohmmeter_connection->get_File_Connection();
	  if (ohmmeterLogFile){
	    istate.ohm.readingLogFile = 1;
	    // But the file name is hidden inside istate.ohm.deviceName and
	    // only vrpn_Connection knows how to parse this
	  } else {
	    // If we are reading a microscope stream file, we DONT want 
	    // to connect to a live ohmmeter - kill connection in this case.
	    if (istate.afm.readingStreamFile == VRPN_TRUE) {
	      ohmmeter_connection = NULL;
	    }
	  }

	  // We have problems if the ohmmeter is not connected initially.
	  if ((ohmmeter_connection != NULL) &&(ohmmeter_connection->connected())) {
	      ohmmeter = new vrpn_Ohmmeter_Remote(istate.ohm.deviceName, 
					ohmmeter_connection);
	  } else {
	      ohmmeter_connection = NULL;
	      ohmmeter = NULL;
	  }
	}
      } // end if (strcmp(istate.ohm.deviceName, "null") != 0)
      else {
	ohmmeter = NULL;
      }
      if (ohmmeter != NULL) {
	the_french_ohmmeter_ui = new Ohmmeter (get_the_interpreter(),
                                             tcl_script_dir, ohmmeter);
      }
    }

    VERBOSE(1, "Before Keithley 2400/VI Curve initialization");
//     if ( tkenable && (dataset->inputGrid->readMode() == READ_DEVICE) ) {
    if ( tkenable ) {
	// Specification of vi_curve device: first look at command line arg
	// and then check environment variable
	if (strcmp(istate.vicurve.deviceName, "null") == 0){
	    char *vicurve_device = getenv("NM_VICURVE");
	    if (vicurve_device != NULL) 
		strcpy(istate.vicurve.deviceName, vicurve_device);
	}
	// Make a connection
	if (strcmp(istate.vicurve.deviceName, "null") != 0) {
	    printf("main: attempting to connect to vicurve: %s\n",
		   istate.vicurve.deviceName);
	    vicurve_connection = vrpn_get_connection_by_name
		(istate.vicurve.deviceName,
		 istate.vicurve.writingLogFile ? istate.vicurve.outputLogName
		 : (char *) NULL,
		 vrpn_LOG_INCOMING);
	    if (!vicurve_connection) {
		fprintf(stderr, "Couldn't open connection to %s.\n",
			istate.vicurve.deviceName);
		//exit(0);
	    } else {
	      // Decide whether reading vrpn log file or a real device
	      vicurveLogFile = vicurve_connection->get_File_Connection();
	      if (vicurveLogFile){
		istate.vicurve.readingLogFile = 1;
		// But the file name is hidden inside istate.vicurve.deviceName and
		// only vrpn_Connection knows how to parse this
	      } else {
		// If we are reading a microscope stream file, we DONT want 
		// to connect to a live device - kill connection in this case.
		if (istate.afm.readingStreamFile == VRPN_TRUE) {
		  vicurve_connection = NULL;
		}
	      }
	      if (vicurve_connection != NULL) {
		// If we got to here, we have a connection to vi_curve -
		// create the beast.
		keithley2400_ui = new nma_Keithley2400_ui(get_the_interpreter(), 
						      tcl_script_dir, 
						      "vi_curve@dummyname.com", 
						      vicurve_connection);
		// Allow the Keithley to take IV curves when we are doing a
		// modification - start when we enter modify mode, and stop
		// when we start imaging again.
		microscope->registerModifyModeHandler(
			  nma_Keithley2400_ui::EnterModifyMode, 
			  keithley2400_ui);
		microscope->registerImageModeHandler(
			 nma_Keithley2400_ui::EnterImageMode, 
			 keithley2400_ui);
	      } else {
		keithley2400_ui = NULL;
	      }
	    }
	}
    }


    VERBOSE(1, "Before SEM initialization");
//     if ( tkenable && (dataset->inputGrid->readMode() == READ_DEVICE) ) {
    if ( tkenable ) {
        // Specification of sem device: first look at command line arg
        // and then check environment variable
        if (strcmp(istate.sem.deviceName, "null") == 0){
            char *sem_device = getenv("NM_SEM");
            if (sem_device != NULL)
                strcpy(istate.sem.deviceName, sem_device);
        }
        // Make a connection
        if (strcmp(istate.sem.deviceName, "null") != 0) {
            printf("main: attempting to connect to sem: %s\n",
                   istate.sem.deviceName);
            sem_connection = vrpn_get_connection_by_name
                (istate.sem.deviceName,
                 istate.sem.writingLogFile ? istate.sem.outputLogName
                 : (char *) NULL,
                 vrpn_LOG_INCOMING);
            if (!sem_connection) {
                fprintf(stderr, "Couldn't open connection to %s.\n",
                        istate.sem.deviceName);
                //exit(0);
            } else {
              // Decide whether reading vrpn log file or a real device
              fprintf(stderr, "Got connection\n");
              semLogFile = sem_connection->get_File_Connection();
              if (semLogFile){
                istate.sem.readingLogFile = 1;
                // But the file name is hidden inside istate.sem.deviceName and
                // only vrpn_Connection knows how to parse this
              }
              // If we got to here, we have a connection to sem -
              // create the beast.
              sem_ui = new nms_SEM_ui(get_the_interpreter(),
                                                      tcl_script_dir,
                                                      "SEM@dummyname.com",
                                                      sem_connection);
            }
        }
    }

#ifdef USE_NMB_CHANNELS
    dataset->initDeviceChannelHandlers(m);
    if (keithley2400_ui && keithley2400_ui->keithley2400) {
        dataset->initDeviceChannelHandlers(
				keithley2400_ui->keithley2400);
    }
    if (ohmmeter) {         // French ohmmeter
        dataset->initDeviceChannelHandlers(ohmmeter);
    }
#endif


#ifdef  PROJECTIVE_TEXTURE
    // Registration - displays images with glX or GLUT depending on V_GLUT
    // flag
    aligner_ui = new AlignerUI(graphics, dataset->dataImages,
      get_the_interpreter(), tcl_script_dir);

  #ifdef NANO_WITH_ROBOT
    robotControl = new RobotControl(microscope, dataset);
    robotControl->show();
  #endif
#endif


  /* open raw terminal with echo if keyboard isn't off  */
  if (do_keybd == 1){
    VERBOSE(1,"Opening raw terminal");
    if ((ttyFD = open_raw_term(1)) < 0)
      fprintf(stderr, "open_raw_term(): error opening terminal.\n");
  }
  /* Prepare signal handler for ^C,so we save the stream file and exit cleanly*/

  VERBOSE(1,"Setting ^C Handle");
  signal(SIGINT, handle_cntl_c);

  // only print the keyboard options if it is turned on
  if (do_keybd == 1) {
    KeyboardUsage();
  }

VERBOSE(1, "Creating Ugraphics");
  World.TSetName("World");
  Textures.TSetName("Texture Store\n");  

  //INIT THE WORLD
  //put an invisible axis in the world
  URAxis *temp=new URAxis;   //set the contents of the World node so it isn't EMPTY!
  if(temp==NULL){cerr << "Memory fault\n"; kill(getpid(),SIGINT);}
  temp->SetVisibility(0);    
  World.TSetContents(temp);
  
  initializeInteraction();

  // This should be activated by setupCallbacks(graphics), but doesn't
  // seem to be?
  tclstride = istate.tesselation;

/* Center the image first thing */
center();

  Tcl_Interp * interp = get_the_interpreter();

/* Start timing */
VERBOSE(1, "Starting timing");
gettimeofday(&time1,&zone1);
gettimeofday(&d_time,&zone1);
microscope->ResetClock();
//gettimeofday(&nowtime, &nowzone);

/* 
 * main interactive loop
 */
VERBOSE(1, "Entering main loop");

//while ( ! dataset->done ) {
  while( n<LOOP_NUM || !dataset->done ) {

#ifdef TIMING_TEST
#define	TIM_LN	(7)
    /* draw new image for each eye  */
    VERBOSE(4,"  Drawing eyes");
    fprintf(stderr,"loop-number= %d\n", LOOP_NUM);
    static	struct timeval 	t_b4;
    static	struct timeval 	t_aft;
    long		t_this;
    long		t_loop;
    static	long	t_avg_i = 0;
    static	long	t_avg_r = 0;
    static	long	t_avg_d = 0;
    static	long	t_avg_l = 0;
    static	long	t_avg_loop = 0;
    static	long	t_avg_tread = 0;
    static	long	t_avg_head_x = 0;
    static	long	t_avg_hand_x = 0;
    static	long	t_avg_hand_mat = 0;
    static	long	t_avg_hand_p = 0;
    static	long	t_avg_headlock = 0;
    static	long	n_disp = 1;

    t_loop = 0;

    gettimeofday(&t_aft,&t_zone);
    t_this = (t_aft.tv_sec-t_b4.tv_sec)*1000000 + (t_aft.tv_usec-t_b4.tv_usec);
    if( n_disp > 1 ) {
       t_avg_l += t_this;
       t_loop += t_this;

       if( ( ( n_disp >> TIM_LN ) << TIM_LN ) == n_disp ) {
	 fprintf(stdout, "T loop  %d ms, avg %d ms\n", t_this, t_avg_l/n_disp );
       }
    }
#endif /* TIMING_TEST */

    /* get and process input for each user  */
    for ( i = 0; i < NUM_USERS; i++ ){
#ifdef TIMING_TEST
      gettimeofday(&t_b4,&t_zone);
#endif /* TIMING_TEST */
      ttest0(t_avg_tread, "tread");

      /* update head from tracker xform	*/
      ttest0(t_avg_head_x, "head_x");
      if (ohmmeter) {
	ohmmeter->mainloop();
      }
      if (keithley2400_ui) {
	keithley2400_ui->mainloop();
      }
      if (sem_ui) {
        sem_ui->mainloop();
      }
      if (phantButton) {
	phantButton->mainloop();
      }
      if (vrpnHeadTracker[0]) {
	vrpnHeadTracker[0]->mainloop();
      }
      if (vrpnHandTracker[0]) {
	// XXX crashing here on tantalum-cs (bus error); why??
	vrpnHandTracker[0]->mainloop();
      }

//nM_coord_change_server sends hand coordinates in world space from one copy
//of microscape to another; vrpnHandTracker_collab[0] takes these messages
//and uses the coordinates/orientation it is sent to draw the icon for the
//collaborator's hand position.
//The mode is used to determine the text string that follows the user's
//hand around.
      if (nM_coord_change_server) nM_coord_change_server->mainloop();
      if (vrpnHandTracker_collab[0]) vrpnHandTracker_collab[0]->mainloop();
      if (vrpnMode_Local) {
	struct	timeval	zerotime;
	// Set the mode to the current one and send if changed
	vrpnMode_Local->channels()[0] = user_mode[0];
	vrpnMode_Local->report_changes(vrpn_CONNECTION_RELIABLE);

	// Send every couple seconds even if it hasn't changed, so that when
	// the other side connects they will have the correct mode
	// within a couple of seconds.
	// These are send unreliably, since they will come again if lost.
	{	static	unsigned long last = 0;
		struct timeval now;

		gettimeofday(&now, NULL);
		if (now.tv_sec - last >= 2) {
			last = now.tv_sec;
			vrpnMode_Local->report();
		}
	}
	zerotime.tv_sec = zerotime.tv_usec = 0;
	vrpnMode_Local->mainloop(&zerotime);
      }
      if (vrpnMode_collab[0]) { vrpnMode_collab[0]->mainloop(); }
      if (buttonBox) buttonBox->mainloop();
      if (dialBox) dialBox->mainloop();

      if (collaboratingPeerServerConnection) {
        collaboratingPeerServerConnection->mainloop();
      }

      // Not necessary for recording, but necessary for playback.
      // TCH 19 Jan 00
      if (interfaceLogConnection) {
        interfaceLogConnection->mainloop();
      }

      // NANOX
      // This should eventually be generalized to a set of collaborating
      // remote peers.
      if (collaboratingPeerRemoteConnection) {
        collaboratingPeerRemoteConnection->mainloop();
      }

      if (forceDevice && constraint_mode) {
        // TODO
        // Use callbacks or check _changed to avoid spamming network
        forceDevice->setConstraintPoint(decoration->trueTipLocation);
        forceDevice->setConstraintLinePoint(decoration->trueTipLocation);

        // TODO
        // Maintain an upwards vector in vlib space;  send that
        //forceDevice->setConstraintLineDirection
                     //(decoration->trueTipLocation);
      }

      /* good place to update displays, minimize latency from trackers */

      VERBOSE(4,"  Updating displays");
      if ((glenable) && (justCentered || !drawOnlyOnCenter)){
        //v_update_displays(displayIndexList); 
        TIMERVERBOSE(1,  frametimer, "------------- Frame timer -----------");
        TIMERLOG(frametimer);
        graphics->mainloop();
      }

    ttest0(t_avg_head_x, "head_x");

    ttest0(t_avg_hand_x, "hand_x");

      justCentered = 0;	// User did not just center since the last drawing

    ttest0(t_avg_headlock, "headlock");

      if (gi) gi->mainloop();

      TIMERVERBOSE(1, mytimer, "microscape.c:end gi->mainloop()");

      if (updt_display(displayPeriod, d_time, stm_new_frame)) {
        n_displays++;

        ttest0(t_avg_d, "display");
      } /* end if updt */

    // REMOTERENDER
    // user interface timestamp happens here

    graphicsTimer.newTimestep();

      /* routine for handling all user interaction, including:
       * sdi button box, knob box, phantom button, 
       * Tcl button box simulator, Tcl knob box simulator. */
      if(glenable) {
	VERBOSE(4,"  Calling interaction()");
	interaction(bdboxButtonState, bdboxDialValues, phantButtonState,
                    &graphicsTimer);
      }

      ttest0(t_avg_i, "inter");

      new_value_of_rate_knob = (int) (Arm_knobs[RATE_KNOB] * 10.0);

      if (!n)  // first execution
        old_value_of_rate_knob = new_value_of_rate_knob;
      else if (new_value_of_rate_knob != old_value_of_rate_knob){
        decoration->rateOfTime = new_value_of_rate_knob;

#ifdef USE_VRPN_MICROSCOPE
	if (vrpnLogFile) {
	   vrpnLogFile->set_replay_rate(decoration->rateOfTime);
	}
#else
	microscope->SetStreamReplayRate(decoration->rateOfTime);
#endif
	if (ohmmeterLogFile) {
	   ohmmeterLogFile->set_replay_rate(decoration->rateOfTime);
	}
	if (vicurveLogFile) {
	   vicurveLogFile->set_replay_rate(decoration->rateOfTime);
	}
        printf("Replay:  %d times faster\n", decoration->rateOfTime);
        old_value_of_rate_knob = new_value_of_rate_knob;
      }

      if ( (read_mode == READ_DEVICE) || (read_mode == READ_STREAM) ) {
	if (xenable)
	  synchronize_xwin(False);

#ifdef USE_VRPN_MICROSCOPE

        VERBOSE(4, "  Calling microscope->mainloop()");
	microscope->mainloop();

        if (monitor_forwarder_connection) {
          VERBOSE(4, "  Calling monitor_forwarder_connection->mainloop()");
          monitor_forwarder_connection->mainloop();
        }
        if (collab_forwarder_connection) {
          VERBOSE(4, "  Calling collab_forwarder_connection->mainloop()");
          collab_forwarder_connection->mainloop();
        }

#else

        VERBOSE(4, "  Calling microscope->HandleReports()");
	microscope->HandleReports();

#endif

	if (xenable) {
	  flush_xwin();
	  synchronize_xwin(True);
	}
      }
   
      ttest0(t_avg_r, "reports");
    } /* end for-loop (NUM_USERS) */

#ifdef TIMING_TEST
      t_avg_loop += t_loop;
      if( ( ( n_disp >> TIM_LN ) << TIM_LN ) == n_disp ) {
        fprintf(stdout, "----------------------T total   %d ms, avg %d ms\n", 
		t_loop, t_avg_loop/n_disp );
      }

      n_disp++;
      gettimeofday(&t_b4,&t_zone);
#endif /* TIMING_TEST */

    if (do_keybd == 1)
      {
	/* check for keyboard input	*/
	VERBOSE(4, "  Checking for keyboard input");
	handleTermInput(ttyFD, &dataset->done);
      }

    /* Check for exposure event of the window */
    //if((read_mode==READ_FILE) && (xenable)) { }
    // Changed TCH 6 May 98 to allow redraws regardless of
    // input type.  We don't know why it was originally limited
    // only to static images.
    // New argument is a polyline to draw.
    if (xenable) {
      VERBOSE(4, "  Checking for exposure of X window");
      restore_window(teststrip);
    }
    
    /* Check for mouse events in the X window display */
    handleMouseEvents(&graphicsTimer);

#ifdef	PROJECTIVE_TEXTURE
    if (aligner_ui) {
      aligner_ui->mainloop();
    }

  #ifdef NANO_WITH_ROBOT
    if (robotControl)
       robotControl->mainloop();
  #endif
#endif

    /* Run the Tk control panel checker if we are using them */
    if (tkenable) {
	VERBOSE(4, "  Handling Tk control panels");
	poll_Tk_control_panels();
    }

#ifdef USE_VRPN_MICROSCOPE

    // If we've rolled over a second, update the RTT measurements
    // and send out on our RTT server.

    update_rtt();

#endif


    // Have the active sets update the microscape data sets
    VERBOSE(4, "  Updating scan and point sets");

// There is no difference in code here so I took out this ifdef (AAS)
    /*
    if (microscope->state.data.scan_channels->Changed()){
      microscope->GetNewScanDatasets
                     (microscope->state.data.scan_channels->Checklist());
      microscope->state.data.scan_channels->Changed(0);
      fprintf(stderr, "microscape.c:: microscope->state.data."
		"scan_channels changed!!!\n");
    }
    if (microscope->state.data.point_channels->Changed()) {
      microscope->GetNewPointDatasets
                      (microscope->state.data.point_channels->Checklist());
      microscope->state.data.point_channels->Changed(0);
      fprintf(stderr, "microscope.c:: micrscope->state.data."
		"point_channel changed!!!\n");
    }
    */
    microscope->state.data.scan_channels->Update_microscope(microscope);
    microscope->state.data.point_channels->Update_microscope(microscope);
    // XXX What? No forcecurve channels ? - no, they are
    // limited to only 2 channels by the Topometrix dsp code and I 
    // haven't written the code to add the second channel

    /* Run the Tk event handler if we are using Tk in any of the code */
    if( tkenable || (interp != NULL) ) {
	VERBOSE(4, "  Handling Tk events");
	while (Tk_DoOneEvent(TK_DONT_WAIT)) {};
    }

    /* One more iteration done */
    VERBOSE(4, "  Done with mainloop iteration");
    n++; 

  }

  dataset->done = VRPN_TRUE; //XXXX

  /* Stop timing */
  gettimeofday(&time2,&zone2);

  if (framelog != 0) frametimer.print();

  /* Print timing info */
  start = time1.tv_sec * 1000 + time1.tv_usec/1000;
  stop = time2.tv_sec * 1000 + time2.tv_usec/1000;
  interval = stop-start;          /* In milliseconds */
  printf("Time for %ld loop iterations: %ld seconds\n",n,
        time2.tv_sec-time1.tv_sec);
  printf("Time for %ld display iterations: %ld seconds\n",n_displays,
        time2.tv_sec-time1.tv_sec);
  if (interval != 0) {
	timing = ((float)(n) / (float)(interval) * 1.0e+3);
	printf("    (%.5f loop iterations per second)\n", timing);
        looplen = ((interval / 1.0e+3) / (float) (n));
	printf("    (%.5f seconds per loop iteration)\n", looplen);
	timing = ((float)(n_displays) / (float)(interval) * 1.0e+3);
	printf("    (%.5f display iterations per second)\n", timing);
        looplen = ((interval / 1.0e+3) / (float) (n_displays));
	printf("    (%.5f seconds per display iteration)\n", looplen);
  }

  printf("---------------\n");
  printf("Graphics Timer:\n");
  graphicsTimer.report();

  if(glenable){
    /* shut down trackers and a/d devices    */
    for ( i = 0; i < NUM_USERS; i++ ) {
      if (vrpnHandTracker[i]) {
	  delete vrpnHandTracker[i];
	  vrpnHandTracker[i] = NULL;
      }
      if (vrpnHeadTracker[i]) {
	  delete vrpnHeadTracker[i];
	  vrpnHeadTracker[i] = NULL;
      }
    }
    if (phantButton) {
	delete phantButton;
	phantButton = NULL;
    }
    if (forceDevice) {
	delete forceDevice;
	forceDevice = NULL;
    }
    if (buttonBox) {
	delete buttonBox;
	buttonBox = NULL;
    }
    if (dialBox) {
	delete dialBox;
	dialBox = NULL;
    }
  }

// Turn the screen saver back on if we're using the SGI and openGL.

#ifdef	sgi
  if(glenable) {
	char buffer [1000];
	char ddname [] = ":0";
	char * dname;
	dname = getenv("V_X_DISPLAY");
	if (!dname) dname = ddname;
	sprintf(buffer, "DISPLAY=%s; export DISPLAY; blanktime 36000", dname);
	system(buffer);
  }
#endif

  if (graphicsServerThread)
    graphicsServerThread->kill();  // NOT PORTABLE TO NT!
  shutdown_connections();

  if (do_keybd == 1)
    reset_raw_term(ttyFD);

  return(0);
    
}   /* main */

/*****************************************************************************
 *
   handleTermInput - handle all keyboard events
 
    input:
    	- file descriptor for terminal
	- pointer to 'done' flag
    
    output:
    	- 'done' flag is updated if 'q' is typed at keyboard
 
    notes:
    	- doesn't check the keyboard at every pass, since this isn't really
	    necessary.
 *
 *****************************************************************************/

/* check keyboard every INPUT_CHECK_INTERVAL passes */
#define INPUT_CHECK_INTERVAL	    10

void handleTermInput(int ttyFD, vrpn_bool* donePtr)
{
    static int numTimesCalled = 0;

    static int intervals=10;
 
    /* buffer must be at least V_MAX_COMMAND_LENGTH bytes long	*/
    char    	buffer[V_MAX_COMMAND_LENGTH];
    int		nread;


    if(glenable)
      intervals=1;
    
    numTimesCalled++;

    /* is it time to really read the keyboard?  */
    if ( numTimesCalled <= intervals )
	return;
    else
	numTimesCalled = 0;

    /* nonblocking read: is there anything to read at the keyboard?	*/
    if ( (nread = read_raw_term(ttyFD, buffer) ) < 1 )
	/* no   */
	return;

    /* yes-  see what was typed and handle it   */
    handleCharacterCommand(buffer, donePtr, nread);

}	/* handleTermInput */






/*****************************************************************************
 *
   handleCharacterCommand - handle all character commands for menus/keyboard
 
    input:
    	- character to act upon
	- pointer to 'done' flag
	- number of characters to work on
    
    output:
    	- 'done' flag is updated if 'q' was the command
 
 *
 *****************************************************************************/

void handleCharacterCommand (char * character, vrpn_bool * donePtr,
                             int /* nchars */)
{
    //static	int	which_pic = 0;

    /* handle the given command. */
    switch ( *character ) {

      case 'Q':	/* quit even if there are unsaved buffers */
	*donePtr = VRPN_TRUE;
	microscope->state.saveStream = VRPN_TRUE;
	fprintf(stderr, "\nShutting down WITHOUT saving...\n");
	break;

      case '\03':	/* ^C: quit even if there are unsaved buffers */
	*donePtr = VRPN_TRUE;
	microscope->state.saveStream = VRPN_TRUE;
	fprintf(stderr, "\nShutting down WITHOUT saving...\n");
	break;

      case 'q':
	/*if (!microscope->state.snap.snapshotGrid->empty_list()) {
	    fprintf(stderr,
		    "\nThere are unsaved snapshots (use Q to exit anyway)\n");
	} else { */
	    *donePtr = VRPN_TRUE;
	    fprintf(stderr, "\nShutting down...\n");
	//}
	break;

//XXX Temporary way to write VRML file.  Make a control panel for it.
      case 'o': // Output a VRML file
	{	nmb_PlaneSelection planes;
	        planes.lookup(dataset);
		write_to_vrml_file("nano.wrl", planes,
                                   (GLdouble *) graphics->getMinColor(),
                                   (GLdouble *) graphics->getMaxColor());
	}
		break;


      case 't':

        {   BCPlane* plane;
	    plane = ((dataset->inputGrid)->getPlaneByName(dataset->heightPlaneName->string()));
	    GTF.gridToTopoData(dataset->inputGrid,plane);
	    GTF.writeTopoFile("sample.tfr");
	}
	    break;
	
      case ':':	
	{
	}
	break;
	    case '$': 
		{		/* Draw frame save */
		}
		break;

	    case 'I': {         /* Info dump */
			q_matrix_type	temp_mat;

			// XXX this needs to be gotten from vlib now
			printf("\nHead position, user 0: \n");
			printf("\nHand position, user 0: \n");
			printf("\nUser position, user 0: \n");
			printf("  xlate: ");
			q_vec_print(v_world.users.xforms[0].xlate);
			printf("  rotate: ");
			q_print(v_world.users.xforms[0].rotate);
			q_to_col_matrix( temp_mat, 
				v_world.users.xforms[0].rotate );
			q_print_matrix( temp_mat );
			printf("  scale: ");
			printf("%g\n",v_world.users.xforms[0].scale);
		}
		break;

	    case '.': printf( "Refreshing display from grid\n" );
			microscope->NewEpoch();
		break;

            case '^': 
	       //printf( "Fixing lock\n" );
			justCentered = 1;
			center();
		break;

	    case '*':
			printf( "Adjusting light position\n" );
			mode_change = 1;	/* Will make icon change */
			user_mode[0] = USER_LIGHT_MODE;
		break;

	    case 'A': {		/* Select All */

			/* Tell it where to scan */
                        microscope->SetRegionNM(microscope->state.xMin,
                                                microscope->state.yMin,
                                                microscope->state.yMax,
                                                microscope->state.yMax);

			/* Tell the user about it */
			printf("Selected All (entire scan range)\n");
		}
		break;

	    case 'G': 		/* Grab World mode */
		printf("Grab world mode\n");
		mode_change = 1;	/* Will make icon change */
		user_mode[0] = USER_GRAB_MODE;
		break;

	    case '%': 		/* Measure mode */
		printf("Measure mode\n");
		mode_change = 1;	/* Will make icon change */
		user_mode[0] = USER_MEASURE_MODE;
		break;

	    case 'U': 		/* Scale Up mode */
		printf("Scale Up mode\n");
		mode_change = 1;	/* Will make icon change */
		user_mode[0] = USER_SCALE_UP_MODE;
		break;

	    case 'D': 		/* Scale Down mode */
		printf("Scale Down mode\n");
		mode_change = 1;	/* Will make icon change */
		user_mode[0] = USER_SCALE_DOWN_MODE;
		break;

	    case 024: 		/* ^T Touch grid plane mode (Canned)*/
		printf("Touch grid mode (Canned)\n");
		mode_change = 1;	/* Will make icon change */
		user_mode[0] = USER_PLANE_MODE;
		break;

	    case 'T': 		/* T Touch Live mode */
		if( READ_DEVICE == read_mode ) {
		   printf("Touch sample mode (Live)\n");
		   mode_change = 1;	/* Will make icon change */
		   user_mode[0] = USER_PLANEL_MODE;
		   }
		else printf("Touch mode available only on live data!!!!");
		break;

	    case 'F': 		/* Fly mode */
		printf("Fly mode\n");
		mode_change = 1;	/* Will make icon change */
		user_mode[0] = USER_FLY_MODE;
		break;

	    case 'S': 		/* Select mode */
		printf("Select mode\n");
		mode_change = 1;	/* Will make icon change */
		user_mode[0] = USER_SERVO_MODE;
		break;

	    case 'H': 		/* Height of measure grid mode */
		printf("Height of measure grid mode\n");
		mode_change = 1;	/* Will make icon change */
		user_mode[0] = USER_MEAS_MOVE_MODE;
		break;

	    case 'V': 		/* Make pulses visible */
		break;

	    case 'v': 		/* Make pulses invisible */
		break;

	    case 'C': 		/* Clear existing pulses */
		if(glenable)
		  {
		    decoration->clearPulses();
		    decoration->clearScrapes();
		  }
		break;

	    case 'X': 		/* Close and reopen ARM */
		printf("_NOT_ closing and reopening ARM...\n");
		break;

	    case 'i': 		// Adjust information verbosity
		spm_verbosity = (spm_verbosity + 1) % 2;
		printf("Verbosity set to %d.\n", spm_verbosity);
		break;

	    case '0':	/* Adjust the playback rate */
	    case '1':
	    case '2':
	    case '3':
	    case '4':
	    case '5':
	    case '6':
	    case '7':
	    case '8':
	    case '9':
		decoration->rateOfTime = *character - '0';
#ifdef USE_VRPN_MICROSCOPE
	  	if (vrpnLogFile) {
		   vrpnLogFile->set_replay_rate(decoration->rateOfTime);
		}
#else
		microscope->SetStreamReplayRate(decoration->rateOfTime);
#endif
	  	if (ohmmeterLogFile) {
		   ohmmeterLogFile->set_replay_rate(decoration->rateOfTime);
		}
	  	if (vicurveLogFile) {
		   vicurveLogFile->set_replay_rate(decoration->rateOfTime);
		}
		printf("Read rate: %d times original\n", decoration->rateOfTime);
		break;


	    case 'x': {		/* Scan in x fastest */

			/* Tell it scan style */
			microscope->state.do_y_fastest = VRPN_FALSE;
			if (microscope->SetScanStyle() == -1) {
				perror("Could not write to server");
				dataset->done = VRPN_TRUE;
			}

			/* Tell the user about it */
			printf("Scan in x fastest\n");
		}
		break;

	    case 'y': {		/* Scan in y fastest */

			/* Tell it scan style */
			microscope->state.do_y_fastest = VRPN_TRUE;
			if (microscope->SetScanStyle() == -1) {
				perror("Could not write to server");
				dataset->done = VRPN_TRUE;
			}

			/* Tell the user about it */
			printf("Scan in y fastest\n");
		}
		break;

	    case 'B': {		/* Boustrophedonic scan */

			/* Tell it scan style */
			microscope->state.do_raster = VRPN_FALSE;
			/* Send the commands to the server */
			if (microscope->SetScanStyle() == -1) {
				perror("Could not write to server");
				dataset->done = VRPN_TRUE;
			}

			/* Tell the user about it */
			printf("Boustrophedonic scan\n");
		}
		break;

	    case 'R': {		/* Raster scan */

			/* Tell it scan style */
			microscope->state.do_raster = VRPN_TRUE;
                        if (microscope->SetScanStyle() == -1) {
				perror("Could not write to server");
				dataset->done = VRPN_TRUE;
			}

			/* Tell the user about it */
			printf("Raster scan\n");
		}
		break;

	    case '+':		/* Take forward raster information */
		microscope->state.raster_scan_backwards = VRPN_FALSE;
		printf("Take forward raster scan information\n");
		break;

	    case '-':		/* Take backwards raster information */
		microscope->state.raster_scan_backwards = VRPN_TRUE;
		printf("Take backwards raster scan information\n");
		break;

	    case 0177:		/* ^8 Splat data into grid */
		microscope->state.doSplat = (!microscope->state.doSplat);
		printf("%s into grid\n", 
			(microscope->state.doSplat ? "Splatting" : "Not splatting" ) );
		break;

	    case '=':		/* Calibrate piezo relaxation */
		microscope->state.doRelaxComp = (!microscope->state.doRelaxComp);
		if( !microscope->state.doRelaxComp ) {
#ifdef USE_VRPN_MICROSCOPE
			microscope->d_relax_comp.disable();
#else
			microscope->state.relaxComp = 0;
#endif
			}
		else if( !(microscope->state.stmRxTmin +
                           microscope->state.stmRxTsep ) ) {
			printf( "RELAX: " );
			printf( "Must set min (^Mnnn^M) and sep (^Nnnn^N)\n" );
			microscope->state.doRelaxComp = VRPN_FALSE;
#ifdef USE_VRPN_MICROSCOPE
			microscope->d_relax_comp.disable();
#endif
			}
		printf("Relaxation compensation %s\n", 
			(microscope->state.doRelaxComp ? "on" : "off" ) );
		printf("Relaxation compensation %s in image mode\n", 
			(microscope->state.doRelaxUp ? "happens" : "does not happen" ) );
		break;

	    case '~':		/* Drift compensation */
		microscope->state.doDriftComp = (!microscope->state.doDriftComp);
		printf("Drift compensation %s\n", 
			(microscope->state.doDriftComp ? "on" : "off" ) );
		break;

	    case 'O': {		/* Toggle Ohmmeter on and off */
		 int	rate;

		 ohmmeter_enabled = (!ohmmeter_enabled);
		 printf("Ohmmeter %s\n", (ohmmeter_enabled ? "on" : "off" ) );

		 if (ohmmeter_enabled) {
		    rate = 4;
		 } 
		 else {
		    rate = 0;
		 }
		 if (microscope->SetOhmmeterSampleRate(rate) == -1) {
		    perror("Could not write to the microscope server");
		    dataset->done = VRPN_TRUE;
		 }
		}
		break;

	    case 'w':
		/*change the mode to draw graph*/
		if((xmode==0)&&xenable) {
		    xmode=1;
		    xg_start=0;
		    // TCH 6 May 98 - moved to where xg_start is set to 1
		    //teststrip=(TwoDLineStrip *)malloc(sizeof(TwoDLineStrip));
		    //init_linestrip(teststrip);
		    printf("now in draw graph mode\n");
		}
		break;


	    case 'W':
		/*switch back*/
		if((xmode==1)&&xenable)
		  {
			printf("Leaving draw mode\n");
			xmode=0;
		  }
		break;
		
		// scanline nudge controls:
		case 'f':
			printf("nudging scanline left\n");
			microscope->state.scanline.moveScanlineRelative(0, -1);
		break;
		case 'g':
			printf("nudging scanline backward\n");
			microscope->state.scanline.moveScanlineRelative(-1, 0);
		break;
		case 'h':
			printf("nudging scanline forward\n");
			microscope->state.scanline.moveScanlineRelative(+1, 0);
		break;
		case 'j':
			printf("nudging scanline right\n");
			microscope->state.scanline.moveScanlineRelative(0, +1);
		break;

	    default:
		fprintf(stderr, "\nunrecognized command: '%c' (%o).\n", 
			*character, *character);
		break;
	}

  // TCH - HACK, but it works
  decoration->user_mode = user_mode[0];
}	/* handleCharacterCommand */



/* handle mouse button events in the x window (qliu)*/
#define M_NULL 0
#define M_TRANSLATE 1
#define M_ZOOM 2
#define M_ROTATE 3
#define	M_TRIGGER 4		// Trigger as if the PHANToM button pressed
#define	M_LINE 5		// Grab the user's line
#define M_SCALE 6
#define M_SHEAR 7
#define M_PICK 8

int handleMouseEvents (nmb_TimerList * timer)
{
  static int prev_x,prev_y; 	
  static double wanted_x = -2;
  static double wanted_y = -1;
  
#if !defined(FLOW) && !defined(V_GLUT)
  static float startx, starty;
  static v_xform_type shadowxform,modxform;
  static int mode=M_NULL;
  static float offsetx, offsety, offsetz;
  static q_vec_type T,L;
  
  // Variables for realigning textures:
  static float realign_x1, realign_x2, realign_y1, realign_y2;
  
  XEvent event;
  if(!xenable){
    float deltax, deltay;
    q_type qtemp, qtemp2;
    q_vec_type qvtemp,qvtemp2;
    
    v_xform_type temp;
    
    if (VLIB_dpy && (XPending(VLIB_dpy) > 0)) {

      XNextEvent(VLIB_dpy, &event);
      switch (event.type) {

      case ConfigureNotify: //"used for mono CRT only"

	graphics->resizeViewport(event.xconfigure.width,
				 event.xconfigure.height);
	break;

      case ButtonPress:
	  
//fprintf(stderr, "handleMouseEvent() got ButtonPress.\n");
          timer->activate(timer->getListHead());

	  qvtemp[Q_X]=event.xbutton.x;
	  qvtemp[Q_Y]=event.xbutton.y;
	  qvtemp[Q_Z]=0;
	  // windowPtr info is encapsulated in graphics
	  // this access is not allowed
	  v_x_invert(&shadowxform,&v_world.users.xforms[0]);	//room from head tracker?
	  v_x_invert(&shadowxform,&v_world.users.xforms[0]);
	  v_x_xform_vector( qvtemp2,&shadowxform,qvtemp);
	  //	printf("qvtemp %f,%f,%f\n",qvtemp[Q_X],qvtemp[Q_Y],qvtemp[Q_Z]);
	  // 	printf("qvtemp2 %f,%f,%f\n",qvtemp2[Q_X],qvtemp2[Q_Y],qvtemp2[Q_Z]);

	// Realign Textures:
	realign_x1 = event.xbutton.x;
	realign_y1 = event.xbutton.y;

	//Button1 = Rotate
	//Button2 = Translate
	//Button1 && Button2 == Zoom

	//UGRAPHICS HACK --
	//THESE ARE SET TO WORK ONLY WHEN TEXTURE REALIGN IS ENABLED OTHERWISE
	//BUTTON3 is bound to the picking operation

	//Button3 			   == Pick
	//Button3 && texrealign            == Texture Scale?
	//Button2 && Button3 && texrealign == Texture Shear?
	//Button1 && Button3 && texrealign == Texture Rotate?

	if (event.xbutton.button == Button3 &&  ( realign_textures_enabled ) ) {
	  if ( mode == M_NULL ) 
	    mode = M_SCALE;
	  else if ( mode == M_TRANSLATE )
	    mode = M_SHEAR;
	  mouse3button = 1;
	}
	else if(event.xbutton.button== Button3){
		//THIS IS HACKED TOGETHER FOR THE CRT VERSION -- YOU NEED TO WORK ON THIS
		//FOR A MORE GENERAL WORKING MODEL -- I HAVE NO IDEA WHERE TO FIND 
		//ALL THE DATA TO DO THIS -- ITS BURIED IN VLIB YOU'LL HAVE TO TRACK THROUGH
		//SEVERAL LAYERS OF #DEFINED MACROS TO FIGURE IT OUT

		mode=M_PICK;
		q_vec_type n,f,result1,result2;

		GLdouble m[4][4];		//screen to head
		glGetDoublev(GL_PROJECTION_MATRIX,&m[0][0]);
		Xform4x4 x=Xform4x4(m);
		x.transpose();
		x.invert();

		v_xform_type vx;		//head to world
		v_get_world_from_head(0,&vx);
		Xform x2=Xform(vx.xlate,vx.rotate,vx.scale);

		x=x2*x;				//compose

		//the strange equation here is simply undoing the viewport mapping
		//that is added to the mouse coordinates but not otherwise represented in 
		//the matrix stack kept by GL -- see GLViewport for info on this.
		//this will only work for the crt mode because of assumptions -- see note above
		n[0]=f[0]=event.xbutton.x*2.0/(double)ScreenWidthPixelsX-1.0;
		n[1]=f[1]=event.xbutton.y*2.0/(double)ScreenWidthPixelsY-1.0;
		n[2]=-1; f[2]=1;	//near and far planes in normalized coords

		x.apply(n,result1);
		x.apply(f,result2);
		SelectionSet *s=new SelectionSet();

		s->p1[0]=result1[0]; s->p1[1]=result1[1]; s->p1[2]=result1[2];
		s->p2[0]=result2[0]; s->p2[1]=result2[1]; s->p2[2]=result2[2];

		//this doesn't work and I think its because the coordinates of the line
		//are not transformed into object space and are only so far as world at this point
		World.Do(&URender::IntersectLine,(void*)s);

		cerr << "Selection Set: " << s->numselected << "\n";
		
	}

	else if(event.xbutton.button == Button1){
	  if(mode==M_NULL) mode=M_ROTATE;
	  if(mode==M_TRANSLATE) mode=M_ZOOM;
	}
	else if(event.xbutton.button == Button2){
	  if(mode==M_NULL) mode=M_TRANSLATE;
	  if(mode==M_ROTATE) mode=M_ZOOM;
	}

	if(user_mode[0]==USER_GRAB_MODE){
	  startx=event.xbutton.x;
	  starty=event.xbutton.y;
	  T[0]=T[1]=T[2]=0; 
	  get_Plane_Extents(&offsetx,&offsety,&offsetz);
	  v_x_invert(&shadowxform,&v_world.users.xforms[0]);
	  v_x_copy(&modxform,&shadowxform);
	}
	else if(user_mode[0]==USER_LIGHT_MODE){
	  startx=event.xbutton.x;
	  starty=event.xbutton.y;
	  graphics->getLightDirection(&T);
	  v_x_invert(&shadowxform,&v_world.users.xforms[0]);
	  v_x_copy(&modxform,&v_world.users.xforms[0]);
	  v_x_xform_vector(T,&modxform,T);
	  v_x_xform_vector(L,&shadowxform,L);
	}
      
	break;

      case ButtonRelease:

	if (event.xbutton.button == Button3){
	  mouse3button = 0;
	}
	mode = M_NULL;
	break;

      case MotionNotify:

	if( user_mode[0] == USER_GRAB_MODE ||
	    user_mode[0] == USER_CENTER_TEXTURE_MODE ) {

	  // deltax/y are used to calculate surface transformations:
	  deltax = event.xmotion.x - startx;
	  deltay = event.xmotion.y - starty;
	  
	  // Calculations for Realigning Textures:
	  // realign_* variables are used to calculate texture realignment
	  int realign_start = 1;
	  realign_x2 = event.xmotion.x;
	  realign_y2 = event.xmotion.y;
	  
	  while (XCheckWindowEvent(VLIB_dpy, VLIB_win,
				   PointerMotionMask, &event)) {

	    // surface transformations:
	    deltax = event.xmotion.x - startx;
	    deltay = event.xmotion.y - starty;
	    
	    // texture realignment
	    if ( realign_start ) {
	      realign_start = 0;
	      realign_x1 = event.xmotion.x;
	      realign_y1 = event.xmotion.y;
	    }
	    realign_x2 = event.xmotion.x;
	    realign_y2 = event.xmotion.y;
	  }
	  // texture realignment
	  float realign_dx = realign_x2 - realign_x1;
	  float realign_dy = realign_y2 - realign_y1;
	  realign_x1 = realign_x2;
	  realign_y1 = realign_y2;


	  if ( mode == M_ROTATE ){

//fprintf(stderr, "handleMouseEvent() got ACTIVE MotionNotify.\n");
          timer->activate(timer->getListHead());

	    // Texture Realignment
	    if ( realign_textures_enabled ) {
	      float theta = realign_dx * 0.017453293;
	      graphics->rotateTextures( 1, theta );
	    }

	    // Surface Transformations
	    else {
	      /*calculate rotations	*/
	      q_make(qtemp,0,1,0,2*M_PI/360*deltax);
	      q_make(qtemp2,1,0,0,2*M_PI/360*deltay);
	      
	      /*offset to center of grid*/
	      T[0]=offsetx; T[1]=offsety;  T[2]=offsetz;
	      q_mult(qtemp,qtemp2,qtemp);
	      v_x_set(&temp,T,qtemp,1);
	      
	      /*do rotation at offset frame position*/
	      v_x_compose(&modxform,&shadowxform,&temp);
	      
	      /* undo offset frame-frame xform */	
	      T[0]=-offsetx; T[1]=-offsety; T[2]=-offsetz;
	      q_make(qtemp2,1,0,0,0);	
	      v_x_set(&temp,T,qtemp2,1);
	      v_x_compose(&modxform,&modxform,&temp);
	      
	      /*recalculate world to room xform and save*/
	      v_x_invert(&modxform,&modxform);
	      //v_x_copy(&v_world.users.xforms[0],&modxform);

              // HACK?
              updateWorldFromRoom(&modxform);

	    }

	  }

	  else if ( mode == M_ZOOM ){

//fprintf(stderr, "handleMouseEvent() got ACTIVE MotionNotify.\n");
          timer->activate(timer->getListHead());

	    // Texture Realignment
	    if ( realign_textures_enabled ) {
	      float scale = (realign_dx + ( -realign_dy ) )/2.0;
	      graphics->scaleTextures( 1, scale, scale );
	    }

	    // Surface Transformations
	    else {
	      v_x_copy(&modxform,&shadowxform);
	      modxform.xlate[2]=shadowxform.xlate[2]+deltay/500;
	      /*recalculate world to room xform and save*/
	      v_x_invert(&modxform,&modxform);
	      //v_x_copy(&v_world.users.xforms[0],&modxform);
              // HACK?
              updateWorldFromRoom(&modxform);
	    }
	  }

	  else if ( mode == M_TRANSLATE ) {

//fprintf(stderr, "handleMouseEvent() got ACTIVE MotionNotify.\n");
          timer->activate(timer->getListHead());

	    // Texture Realignment
	    if ( realign_textures_enabled ) {
	      if ( set_realign_center )
		graphics->setTextureCenter( realign_dx, -realign_dy );
	      else
		graphics->translateTextures( 1, -realign_dx, realign_dy );
	    }
	    
	    // Surface Transformations
	    else {
	      v_x_copy(&modxform,&shadowxform);
	      modxform.xlate[0]=shadowxform.xlate[0]+deltax/1000;
	      modxform.xlate[1]=shadowxform.xlate[1]-deltay/1000;
	      v_x_invert(&modxform,&modxform);
	      //v_x_copy(&v_world.users.xforms[0],&modxform);
              // HACK?
              updateWorldFromRoom(&modxform);
	    }
	  }
	  
	  else if ( mode == M_SCALE ){

//fprintf(stderr, "handleMouseEvent() got ACTIVE MotionNotify.\n");
          timer->activate(timer->getListHead());

	    // Texture Realignment
	    if ( realign_textures_enabled ) {
	      graphics->scaleTextures( 1, realign_dx, -realign_dy );
	    }
	  }
	  
	  else if ( mode == M_SHEAR ) {

//fprintf(stderr, "handleMouseEvent() got ACTIVE MotionNotify.\n");
          timer->activate(timer->getListHead());

	    // Texture Realignment
	    if ( realign_textures_enabled ) {
	      graphics->shearTextures( 1, realign_dx, -realign_dy );
	    }
	  }
	  
	  else {
	    while(XCheckWindowEvent(VLIB_dpy,VLIB_win,
				    PointerMotionMask,&event));
	  }	
	}
	else if ( user_mode[0] == USER_LIGHT_MODE ) {
	  if ( mode == M_TRANSLATE ){

//fprintf(stderr, "handleMouseEvent() got ACTIVE MotionNotify.\n");
          timer->activate(timer->getListHead());

	    deltay = event.xmotion.y - starty;
	    deltax = event.xmotion.x - startx;
	    while(XCheckWindowEvent(VLIB_dpy, VLIB_win,
				    PointerMotionMask, &event)){
	      deltay=event.xmotion.y - starty;
	      deltax=event.xmotion.x - startx;
	    }
	    L[0]=T[0] + deltax * 10;
	    L[1]=T[1] - deltay * 10;
	    L[2]=T[2];
	    v_x_xform_vector(L, &shadowxform, L);
	    graphics->setLightDirection(L);
	  }
	  else{
	    while(XCheckWindowEvent(VLIB_dpy,VLIB_win,
				    PointerMotionMask,&event));
	  }	
	}
	else{ while(XCheckWindowEvent(VLIB_dpy,VLIB_win,
				      PointerMotionMask,&event));}
      default:
	break;
      }
    }
  }
  else if(xenable){

#endif /* !defined(FLOW) && !defined(V_GLUT) */

#if defined(FLOW) || defined(V_GLUT)
    if(xenable){
#endif

      int x,y;
      int xbutton_flag; /* just to process one x event */
      double	rx,ry;
      int	points_per_pixel;
      int	index_x, index_y;
      int   ac_wi;
      TwoDPoint testpoint;
      
      /* See if the last result point was one we asked for.  If so, print
       * it. */
      if ( NM_NEAR(wanted_x, microscope->state.data.inputPoint->x()) && 
	   NM_NEAR(wanted_y, microscope->state.data.inputPoint->y()) ) {
	Point_value *val = microscope->state.data.inputPoint->head();
	while (val) {
	  printf("%s at (%g, %g) is %g (%s)\n",
		 val->name()->Characters(),
		 microscope->state.data.inputPoint->x(),
		 microscope->state.data.inputPoint->y(),
		 val->value(),
		 val->units()->Characters());
	  val = val->next();
	}
	printf("\n");
	wanted_x = -2;
      }
      
      /* Respond to mouse button pressed in the window */
      xbutton_flag=button_pressed_in_window(&x,&y);
      
      if(xbutton_flag!=0)
	{
	  switch (xbutton_flag) {
	    
	  case 1:	/* Report x,y,z for button1 pressed */
	    
	    /* Find the location of the mouse point in the region */
	    mouse_xy_to_region(x,y, &rx,&ry);
	    
	    if(xmode==0)
	      {
		
		/* Find the index of this point in the grid */
		points_per_pixel = WINSIZE /
		  MAX(dataset->inputGrid->numX(), dataset->inputGrid->numY());
		index_x = x/points_per_pixel;
		index_y = y/points_per_pixel;
		if ( (index_x >= dataset->inputGrid->numX()) ||
		     (index_y >= dataset->inputGrid->numY()) ) {
		  printf("Button pressed outside of grid!\n");
		  break;
		}
		
		/* Report the value(s) here */
		BCPlane* plane = dataset->inputGrid->head();
		while (plane != NULL) {
		  printf("%s at (%g, %g) is %g (%s)\n",
			 plane->name()->Characters(), rx,ry,
			 plane->value(index_x, index_y),
			 plane->units()->Characters());
		  plane = plane->next();
		}
		printf("\n");
	      }
	    else
	      {
		/* add a vertex to the line strip we are going to graph*/
		testpoint[0]=rx;
		testpoint[1]=ry;
		if(xg_start==0)
		  {
		    xg_start=1;
		    if (teststrip) delete_linestrip(&teststrip);
		    teststrip = (TwoDLineStrip *) malloc(sizeof(TwoDLineStrip));
		    init_linestrip(teststrip);
		  }
		else
		  {
		    put_line_xwin(prev_x,prev_y, x,y);
		  }
		add_a_vertex(teststrip,testpoint);
		printf("Point (%g, %g) being added\n",rx,ry);
		fprintf(stderr, "Screen location (%d, %d)\n", x, y);
		
		prev_x=x;
		prev_y=y;
	      }
	    
	    break;
	    
	  case 2:	
	    /* Engrave for button 2 when xmode=0; else switch mode and 
	       force a graph to be drawn*/ 
	    
	    // TCH 5 May 98
	    // xmode is set/cleared by pressing w/W
	    
	    if(xmode==0) {
	      if(read_mode!=READ_DEVICE){
		printf("Button two can only be used in read device mode\n");
		return(-1);
	      }
	      
	      mouse_xy_to_region(x,y, &rx,&ry);
	      
	      microscope->ScanTo(rx, ry);
	      
	      microscope->state.data.inputPoint->setPosition(-1,-1);

#ifdef USE_VRPN_MICROSCOPE

	      do {
		microscope->mainloop();
	      } while (!(NM_NEAR(-1.0, microscope->state.data.inputPoint->x()) &&
			 NM_NEAR(-1.0, microscope->state.data.inputPoint->y())));

#else

	      do {
		if (microscope->HandlePacket() == -1) {
		  fprintf(stderr,"Can't read from SPM, bailing\n");
		  return -1;
		}
	      } while (!(NM_NEAR(-1.0, microscope->state.data.inputPoint->x()) &&
			 NM_NEAR(-1.0, microscope->state.data.inputPoint->y())));

#endif

	      microscope->ModifyMode();
	      
	    } else {
	      ac_wi=construct_float_from_linestrip(testarray,(*teststrip),
						   gwi,test);
	      if(ac_wi<0)
		{
		  printf("can not graph: line strip has less than two points\n");
		}
	      else
		{
		  xscale=teststrip->tot_dis/((float) ac_wi);
		  scale_array(testarray,ac_wi,ghi, &yscale,&yoffset);
		  showgraph(testarray, ac_wi, ghi, xscale, yscale, yoffset,
                            tcl_script_dir);
		  xg_start=0;
		  // Moved up by TCH 6 May 98 so we could keep the old one
		  // around until the new one was begun.
		  //delete_linestrip(&teststrip);
		  //teststrip=(TwoDLineStrip *)malloc(sizeof(TwoDLineStrip));
		  //init_linestrip(teststrip);
		}
	    }
	    break;
	    
	  case 3:	/* Request point scan and print it for button 3 */
	    
	    if(read_mode!=READ_DEVICE){
	      printf("Button three can only be used in read device mode\n");
	      return(-1);
	    }
	    
	    /* Find the location of the mouse point in the region */
	    mouse_xy_to_region(x,y, &rx,&ry);
	    
	    microscope->ScanTo(rx, ry);
	    
	    /* Set it so than we will print this point when it is received. */
	    microscope->state.data.inputPoint->setPosition(-1,-1);
	    wanted_x = rx;
	    wanted_y = ry;
	    
	    break;
	    
	  default:
	    /* Do nothing if don't know about this button */
	    break;
	  }
	}
      else if(button2_moved_in_window(&x,&y)) {
	/* Respond to mouse button two motion in the window 
	** This is the end of a scrape across the surface.
	** The tip is already positioned at the start with the
	** press event, now just beef up the force and go there.
	*/
	
	if(read_mode!=READ_DEVICE){
	  printf("Button two can only be used in read device mode\n");
	  return(-1);
	}
	mouse_xy_to_region(x,y, &rx,&ry);
	
	microscope->ScanTo(rx, ry);
      }
      
      else
	{
	  /* Respond to mouse button released in the window 
	  ** This is the end of a scrape across the surface.
	  ** Return to image force and resume scanning the surface
	  */
	  switch (button_released_in_window(&x,&y)) {
	    
	  case 2:
	    if(read_mode!=READ_DEVICE){
	      printf("Button two can only be used in read device mode\n");
	      return(-1);
	    }
	    
	    /* Resume previous scan pattern */
	    microscope->ResumeScan();
	    
	    break;
	    
	  case 3:	   
	    if(read_mode!=READ_DEVICE){
	      printf("Button three can only be used in read device mode\n");
	      return(-1);}
	    
	    /* restart scaning the grid */
	    printf("Not resuming full scan in this version\n");
	    
	    // TCH 5 May 98
	    // BUG BUG BUG
	    // this stops scanning & leaves the microscope hung
	    
	    break;
	    
	  default:
	    break;
	  }
	}
      
    }
    return(0);
}
  


  
  /* Quick function to compare two vectors since this isn't in quatlib.
   * Probably should be added there.
   */

int vec_cmp (const q_vec_type a, const q_vec_type b) {
  return ((a[0] == b[0]) && (a[1] == b[1]) && (a[2] == b[2]));
}


  
// globals used:
//  dataset:  from microscape.c

void find_center_xforms ( q_vec_type * lock_userpos, q_type * lock_userrot,
 double * lock_userscale)
{
  // This function doesn't make sense if we aren't doing graphics.
  if (!glenable) return;


    BCPlane * plane = dataset->inputGrid->getPlaneByName
                          (dataset->heightPlaneName->string());
    if (plane == NULL) {
        fprintf(stderr, "Error in center: could not get plane!\n");
        return;
    }

    float mid_x, mid_y, mid_z;

/*MODIFIED DANIEL ROHRER*/
    //double len_x = fabs(plane->maxX()- plane->minX());
    double len_y = fabs(plane->maxY() - plane->minY());

//     printf("max_x:  %lf\tmin_x:  %lf\nmax_y:  %lf\tmin_y:  %lf\n", 
// 	   plane->maxX(), plane->minX(), plane->maxY(), plane->minY());

    q_vec_type screenLL,
               screenUL,
               screenUR,
               left_side_vec,
               diagonal_vec,
               outward_normal,
               pos_z, pos_y,
               NULL_SCREEN_LOWER_LEFT  = V_NULL_SCREEN_LOWER_LEFT,
               NULL_SCREEN_UPPER_LEFT  = V_NULL_SCREEN_UPPER_LEFT,
               NULL_SCREEN_UPPER_RIGHT = V_NULL_SCREEN_UPPER_RIGHT;
    q_type     screenz_to_worldz,
               screeny_to_worldy;
    q_vec_type screen_mid;
    q_vec_type screenLL_v, screenUL_v, screenUR_v;

    int null_head_tracker = 0;

    // get position of display in the room from vlib(?)
    graphics->getDisplayPosition(screenLL_v, screenUL_v, screenUR_v);
// DEBUG
//   fprintf(stderr, "DEBUG nmg_Graphics_Impl::getDisplayPosition, "
//   "ll %f %f %f "
//   "ul %f %f %f "
//   "ur %f %f %f\n",
// 	  screenLL_v[0], screenLL_v[1], screenLL_v[2], 
// 	  screenUL_v[0], screenUL_v[1], screenUL_v[2], 
// 	  screenUR_v[0], screenUR_v[1], screenUR_v[2]);
    /* Check to see if vlib has the screen defined in room space, i.e, we're in
     * head-tracked mode.
     */
    if (vec_cmp(screenLL_v, NULL_SCREEN_LOWER_LEFT) &&
        vec_cmp(screenUL_v, NULL_SCREEN_UPPER_LEFT) &&
        vec_cmp(screenUR_v, NULL_SCREEN_UPPER_RIGHT) )
    {
       printf("Center: screen has 0 extent, which means null head tracker\n");
       null_head_tracker = 1;
    } else {
       printf("Center: screen has extent, which means head tracker\n");
       q_vec_copy(screenLL, screenLL_v);
       q_vec_copy(screenUL, screenUL_v);
       q_vec_copy(screenUR, screenUR_v);
    }

    /* Find the center of the lower left hand corner.  This will serve as *
     * a translation for the center of the image.                         */
    if (!null_head_tracker)
    {
       screen_mid[0] = (screenLL[0] + screenUR[0]) / 2.0;
       screen_mid[1] = (screenLL[1] + screenUR[1]) / 2.0;
       screen_mid[2] = (screenLL[2] + screenUR[2]) / 2.0;

       screen_mid[0] = (screenLL[0] + screen_mid[0]) / 2.0;
       screen_mid[1] = (screenLL[1] + screen_mid[1]) / 2.0;
       screen_mid[2] = (screenLL[2] + screen_mid[2]) / 2.0;

       /* Now find the rotation of the screen.  First find the outward
          pointing normal. */
       q_vec_subtract(left_side_vec, screenUL, screenLL);
       q_vec_subtract(diagonal_vec,  screenUR, screenLL);
       q_vec_cross_product(outward_normal, left_side_vec, diagonal_vec);

       /* Now subtract a little of the outward pointing normal so that
        * the image floats above the screen.  */
       screen_mid[0] -= 0.02*outward_normal[0];
       screen_mid[1] -= 0.02*outward_normal[1];
       screen_mid[2] -= 0.02*outward_normal[2];

       q_vec_normalize(outward_normal, outward_normal);
       q_vec_normalize(left_side_vec, left_side_vec);

       /* Now rotate the outward screen normal to the positive z-axis
          of world space */
       q_set_vec(pos_z, 0.0, 0.0, -1.0);
       q_from_two_vecs(screenz_to_worldz, outward_normal, pos_z);

       /* Now line up the sides of the screens with the y-axis. */
       q_set_vec(pos_y, 0.0, 1.0, 0.0);
       q_xform(left_side_vec, screenz_to_worldz, left_side_vec);
       q_from_two_vecs(screeny_to_worldy, left_side_vec, pos_y);
       q_mult(*lock_userrot, screeny_to_worldy, screenz_to_worldz);

       /* Determine the scale up factor                            */
       /* We want to keep the image in the lower left hand corner, *
        * so we'll use just that y-length to scale the image.      */
       *lock_userscale = fabs(len_y)/( fabs(screenLL[1] - screen_mid[1]) );

       /* Just a little less scale to make it fit nicely on the screen. */
       *lock_userscale *= 0.75;

       q_xform(screen_mid, *lock_userrot, screen_mid);
    } 
    else
    { //null_head_tracker is true
      screen_mid[0] = 0.0;
      screen_mid[1] = 1.0;
      screen_mid[2] = -1.0;

      /* No rotation needed if we have a null tracker */
      q_make(*lock_userrot, 1.0, 0.0, 0.0, M_PI/4.0);

      /* Determine the scale up factor */
      /* MODIFIED DANIEL ROHRER */
//      if(ScreenWidthMetersY > ScreenWidthMetersX){
                *lock_userscale = len_y / ScreenWidthMetersY * 0.7 / 1.0;
//      }
//      else{
//              *lock_userscale = len_x / ScreenWidthMetersX * 0.7 / 1.0;
//      }
    } // end if (!null_head_tracker)
  
    /* The middle of the screen will be the middle of the grid in X and Y    */
    /* and Z will be the height at the center of the plane (if it is nonzero)*/
    /* or else the average of the non-zero elements in the plane.            */
//     mid_x = ( plane->maxX() + plane->minX() )/2.0;
//     mid_y = ( plane->maxY() + plane->minY() )/2.0;
     mid_z = plane->scaledValue((plane->numX())/2, (plane->numY())/2);
     float minz = mid_z;
     if (mid_z == 0.0) {
         int     x,y;
         double  avg = 0.0;
         int     count = 0;

         for (x = 0; x < plane->numX(); x++) {
           for (y = 0; y < plane->numY(); y++) {
             if (plane->scaledValue(x,y) != 0) {
                 avg += plane->scaledValue(x,y);
		 if (plane->scaledValue(x,y) < minz)
		   minz = plane->scaledValue(x,y);
                 count++;
             }
           }
         }
         if (count > 0) {
                 avg /= count;
         }
         mid_z = avg;
     }
     //printf("center mid_z avg %f  min %f\n", mid_z, minz);
    get_Plane_Extents(&mid_x, &mid_y, &mid_z);
    //printf("center mid_z mm %f\n", mid_z);

    (*lock_userpos)[X] = -screen_mid[0];
    (*lock_userpos)[Y] = -screen_mid[1];
    (*lock_userpos)[Z] = -screen_mid[2];

    q_vec_scale( *lock_userpos, *lock_userscale, *lock_userpos );

    (*lock_userpos)[X] += mid_x;
    (*lock_userpos)[Y] += mid_y;
    (*lock_userpos)[Z] += mid_z;

//     screen_mid[0] = (screenLL[0] + screenUR[0]) / 2.0;
//     screen_mid[1] = (screenLL[1] + screenUR[1]) / 2.0;
//     screen_mid[2] = (screenLL[2] + screenUR[2]) / 2.0;

//     q_xform(screen_mid, *lock_userrot, screen_mid);
//     q_vec_scale(screen_mid, *lock_userscale, screen_mid);
//     q_vec_add(screen_mid, *lock_userpos, screen_mid);

  // this magic number scales down the user so that the phantom can reach
  // the entire surface when the surface is filling the whole screen. 
   // (like when the user hits the center button). This can't be
   // done in the head tracked workbench, because the phantom and
  // user's head are need to be in the same space to be registered.
    if (head_tracked==VRPN_FALSE)
      {
	fprintf(stderr,"DEBUG: non headtracked mode - scaling down user by 5.0 to let phantom reach entire surface and so that surface takes up the whole screen.\n");

	*lock_userscale *= 5.0;
      }
    else
      {
	fprintf(stderr,"DEBUG: head tracked mode - surface will not take up entire screen by default, so that phantom and viewer's head are to the same scale.\n");
      }
}


void center (void) {
  // Reset the light position to its original one
  graphics->resetLightDirection();

  v_xform_type lock;
  
  // find the transforms to take us to the centered view
  find_center_xforms(&lock.xlate, &lock.rotate, &lock.scale);
  
  /* Copy the new values to the world xform for user 0 */
  printf("Setting head xlate to (%f, %f, %f)\n",
        lock.xlate[V_X], lock.xlate[V_Y], lock.xlate[V_Z]);

/*XXX Center causes seg fault on nWC after the resize has happened
  if the rotation sets are done under Linux.  It does not fail if they
  are not done.  If you go into grab mode to clear the resize lines
  before centering, then all works well. */
  printf("Setting head rotate to (%f, %f, %f: %f)\n",
        lock.rotate[V_X], lock.rotate[V_Y], lock.rotate[V_Z],
        lock.rotate[V_W]);

  printf("Setting head scale to %f\n", lock.scale);

  updateWorldFromRoom(&lock);
}



void get_Plane_Extents(float* offsetx, float *offsety, float* offsetz){
    BCPlane* plane;

    *offsetx=(dataset->inputGrid->minX()+
	      dataset->inputGrid->maxX())/2;
    *offsety=(dataset->inputGrid->minY()+
	      dataset->inputGrid->maxY())/2;

    plane=dataset->inputGrid->getPlaneByName(dataset->heightPlaneName->string());
    if(plane==NULL){
      *offsetz=0;
      fprintf(stderr,"UNABLE TO EXTRACT Z offset\n");
    } else { 
      *offsetz=plane->scale()*
	   (plane->minNonZeroValue()+plane->maxNonZeroValue())/2;
    }
}

/** Look for planes named ".ffl" or ".FFL".  If there are any, find the last
 one;  fill in the names of the adhesion planes with the halfway and last
 such planes. */
void guessAdhesionNames (void) {
        int     max_ffl = 0;
        char    max_name[1000];
        BCPlane * p;
        char    name[1000];

        // Pass through the planes.  For each one, see if it has the string
        // .ffl or .FFL in it.  Keep track of the highest number in
        // any of these.  The number will be 0 if there are not any.
        p = dataset->inputGrid->head();
        while (p != NULL) {
                char    *ffl_loc;
                char    testname[1000];
                int     baselen;

                strncpy(testname, p->name()->Characters(), sizeof(testname));
                ffl_loc = MAX(strstr(testname,".ffl"), strstr(testname,".FFL"));
                baselen = (ffl_loc - testname) + 4;
                if (ffl_loc != NULL) {
                        strncpy(max_name, testname, baselen);
                        max_name[baselen] = '\0';
                        max_ffl = MAX(max_ffl, atoi(ffl_loc+4));
                }

                p = p->next();
        }

        // If we have a number, fill in the values with half the number
        // and the number.  This picks the second half of the range.
        if (max_ffl > 0) {
                sprintf(name,"%s%d",max_name, max_ffl/2+1);
                adhPlane1Name = name;
                sprintf(name,"%s%d",max_name,max_ffl);
                adhPlane2Name = name;
        }
}

/** The purpose of this is to just enforce mutual exclusion in the gui
 alternative would be to use a radio button group but since these are
 in separate parts of the gui we can't do this
*/
int disableOtherTextures (TextureMode m) {

  // Not compatible with collaboration.
  // #if'd out by TCH 22 Jan 00 because collaboration experiment is
  // critical path;  hope to rewrite properly soon.

//#if 0
  if (m != RULERGRID){
    rulergrid_enabled = 0;
  }
  if (m != SEM){
    if (sem_ui) {
      sem_ui->displayTexture(0);
    }
  }
  if (m != REGISTRATION){
    if (aligner_ui) {
      aligner_ui->displayTexture(0);
    }
  }
  if (m != MANUAL_REALIGN){
    display_realign_textures = 0;
  }
  if (m != GENETIC){
    genetic_textures_enabled = 0;
  }
//#endif

  return 0;
}

