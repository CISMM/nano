/*===3rdtech===
  Copyright (c) 2000 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#ifndef AFM_STATE_H
#define AFM_STATE_H

#include <BCGrid.h>
#include <Point.h>
#include <Scanline.h>
#include <Position.h>

#include <nmb_Subgrid.h>
#include <nmb_Dataset.h>
#include <Tcl_Linkvar.h>
#include <Tcl_Netvar.h>

#include <active_set.h>  // Scan_channel_selector
#include <nmm_Sample.h>

// Forces normally range from -64 to 64 nanoAmps  
#define BOGUS_FORCE -100

/** \file AFMState.h

Tom Hudson, September, 1997
Taken from global variables declared in microscape.c, animate.c,
interaction.c, and elsewhere.

These structs gather many of the global variables scattered throughout
the code into a single hierarchy
*/


struct AFMModifyInitializationState {

  AFMModifyInitializationState (void);

  int mode;
  int style;

  int grid_resolution;

  float setpoint;
  float setpoint_min;
  float setpoint_max;
  float amplitude;
  float amplitude_min;
  float amplitude_max;

  int std_dev_samples;
  float std_dev_frequency;
};

struct AFMModifyState {

  AFMModifyState (const AFMModifyInitializationState &);
  ~AFMModifyState (void);

  int     std_dev_samples;
  int     std_dev_samples_cache;
  float   std_dev_frequency;
    ///< number of samples to take at a point, and frequency with which to
    ///< take them, when determining standard deviation

  vrpn_bool mode_changed,
            style_changed,
            tool_changed,
            mode_p_changed,
            style_p_changed,
            tool_p_changed;
    // flags to tell the user interface that something changed.
    // these ought to be moved somewhere else if we make an interface module.

  // All variables are in pairs of the form (foo, new_foo) corresponding to
  // modifyp_foo and newmodifyp_foo in Tcl. foo is sent to the microscope
  // and new_foo is used to synchronize the user interface between collaborators

  Tclvar_int mode;
  TclNet_int new_mode;
  Tclvar_int control;
  TclNet_int new_control;
  Tclvar_int style;
  TclNet_int new_style;
  Tclvar_int tool;
  TclNet_int new_tool;
  Tclvar_int constr_xyz_param;
  TclNet_int new_constr_xyz_param;
  Tclvar_int optimize_now_param;
  TclNet_int new_optimize_now_param;
  Tclvar_int direct_step_param;
  TclNet_int new_direct_step_param;
    ///< the current mode of the microscope

  // parameters for Tapping and Contact mode
  Tclvar_float setpoint;
  TclNet_float new_setpoint;
  float        setpoint_min,     ///< control range of the "modify force" knob
               setpoint_max;
  Tclvar_float p_gain;
  TclNet_float new_p_gain;
  Tclvar_float i_gain;
  TclNet_float new_i_gain;
  Tclvar_float d_gain;
  TclNet_float new_d_gain;
  Tclvar_float amplitude;
  TclNet_float new_amplitude;
  float        amplitude_min,    ///< control range of the "modify force" knob
               amplitude_max;
  Tclvar_float frequency;
  TclNet_float new_frequency;
  Tclvar_int   input_gain;
  TclNet_int   new_input_gain;
  Tclvar_int   ampl_or_phase;
  TclNet_int   new_ampl_or_phase;
  Tclvar_int   drive_attenuation;
  TclNet_int   new_drive_attenuation;
  Tclvar_float phase;
  TclNet_float new_phase;

  Tclvar_float scan_rate_microns;
  TclNet_float new_scan_rate_microns;

  // parameters for Sweep style
  Tclvar_float sweep_width;
  TclNet_float new_sweep_width;
  float        region_diag;
  float        yaw;

  // parameters for Sew style
  Tclvar_float bot_delay;
  TclNet_float new_bot_delay;
  Tclvar_float top_delay;
  TclNet_float new_top_delay;
  Tclvar_float z_pull;
  TclNet_float new_z_pull;
  Tclvar_float punch_dist;
  TclNet_float new_punch_dist;
  Tclvar_float speed;
  TclNet_float new_speed;
  Tclvar_float watchdog;
  TclNet_float new_watchdog;

  // parameters for Force Curve style
  Tclvar_float fc_start_delay;	///< usec
  TclNet_float new_fc_start_delay;///< usec
  Tclvar_float fc_z_start;	///< nm
  TclNet_float new_fc_z_start;	///< nm
  Tclvar_float fc_z_end;	///< nm
  TclNet_float new_fc_z_end;	///< nm
  Tclvar_float fc_z_pullback;	///< nm
  TclNet_float new_fc_z_pullback;///< nm
  Tclvar_float fc_force_limit;	///< nA
  TclNet_float new_fc_force_limit;///< nA
  Tclvar_float fc_movedist;	///< nm
  TclNet_float new_fc_movedist;	///< nm
  Tclvar_float fc_num_points;	///< how many values of z to use between start
				///< and end
  TclNet_float new_fc_num_points;///< how many values of z to use between start
				///< and end
  Tclvar_float fc_num_halfcycles;///< # 'down' curves + # 'up' curves
  TclNet_float new_fc_num_halfcycles;///< # 'down' curves + # 'up' curves
  Tclvar_float fc_sample_speed; ///< um (speed while sampling)
  TclNet_float new_fc_sample_speed; ///< um (speed while sampling)
  Tclvar_float fc_pullback_speed; ///< um (speed while going to pullback height)
  TclNet_float new_fc_pullback_speed;///< um (speed while going to pullback
				 ///< height)
  Tclvar_float fc_start_speed; ///< um (speed in going to start height)
  TclNet_float new_fc_start_speed; ///< um (speed in going to start height)
  Tclvar_float fc_feedback_speed; ///< um (speed in going to feedback point)
  TclNet_float new_fc_feedback_speed; ///< um (speed in going to feedback point)
  Tclvar_float fc_avg_num;	///< # of samples per point
  TclNet_float new_fc_avg_num;	///< # of samples per point
  Tclvar_float fc_sample_delay; 	///< us
  TclNet_float new_fc_sample_delay; 	///< us
  Tclvar_float fc_pullback_delay; 	///< us
  TclNet_float new_fc_pullback_delay; 	///< us
  Tclvar_float fc_feedback_delay;	///< us
  TclNet_float new_fc_feedback_delay;	///< us

  vrpn_bool this_mod_fc;
    ///< Set if current modification is a force curve - used to ignore
    ///< point results in favor of FC results. 

  // parameter for Poly-line tool
  Tclvar_float step_size;
  TclNet_float new_step_size;

   // parameters for Direct Z control
  Tclvar_float max_z_step;
  TclNet_float new_max_z_step;
  Tclvar_float max_xy_step;
  TclNet_float new_max_xy_step;
  Tclvar_float min_z_setpoint;
  TclNet_float new_min_z_setpoint;
  Tclvar_float max_z_setpoint;
  TclNet_float new_max_z_setpoint;
  Tclvar_float max_lat_setpoint;
  TclNet_float new_max_lat_setpoint;

    /// information returned by the Topo AFM used during directZ control
  float freespace_normal_force;
  float freespace_lat_force;


  Position_list stored_points;
    ///< list of points used to actually carry out a modification
    ///< after COMMIT is pressed
  
   vrpn_bool constr_line_specified; 
       ///< CONSTR_FREEHAND tool starts contraining feeling and modification
       ///< when this is changed to VRPN_TRUE

  vrpn_bool slow_line_committed;
  ///< SLOW_LINE tool acts like line mode until the commit button is
  ///< pressed - then the user has to press PLAY or STEP to make the tip
  ///< move. This flag marks that change.

  Tclvar_int slow_line_playing;
  ///< SLOW_LINE tool, set to true when use has hit PLAY and tool takes
  ///< a step each time the data from the previous step is received.

  Tclvar_int slow_line_step;
  ///< Set to 1 when the user hits the "step" button.

  Tclvar_int slow_line_direction;
  ///< forward or reverse

  float slow_line_position_param;
  ///< ranges from 0 to 1, parameterizes the position of the tip along
  ///< the current line segment

  Position * slow_line_currPt;
  Position * slow_line_prevPt;
  ///< Location of blue marker lines specified by user. currPt is the
  ///< one we are stepping towards, and prevPt is the one we started at.

  vrpn_bool slow_line_relax_done;

  // Obsolete?

  int modify_enabled;
  ///< referenced in a bodiless if() in interaction.c::interaction()

  // parameters for Blunt style
  Tclvar_float blunt_size;
  TclNet_float new_blunt_size;
  Tclvar_float blunt_speed;
  TclNet_float new_blunt_speed;
};

struct AFMImageInitializationState {

  AFMImageInitializationState (void);

  int mode;

  int grid_resolution;  

  float setpoint;
  float setpoint_max;
  float setpoint_min;
  float amplitude;
  float amplitude_min;
  float amplitude_max;
};


struct AFMImageState {

  AFMImageState (const AFMImageInitializationState &);
  ~AFMImageState (void);

  vrpn_bool mode_changed,
            style_changed,
            mode_p_changed,
            style_p_changed;
    // flags to tell the user interface that something changed.
    // these ought to be moved somewhere else if we make an interface module.

  Tclvar_int mode;
  TclNet_int new_mode;
  Tclvar_int style;
  TclNet_int new_style;
  Tclvar_int tool;
  TclNet_int new_tool;

  Tclvar_int grid_resolution;
  TclNet_int new_grid_resolution;
  Tclvar_float scan_angle; 
  TclNet_float new_scan_angle;

  Tclvar_float setpoint;
  TclNet_float new_setpoint;
  float        setpoint_min,
               setpoint_max;
  Tclvar_float p_gain;
  TclNet_float new_p_gain;
  Tclvar_float i_gain;
  TclNet_float new_i_gain;
  Tclvar_float d_gain;
  TclNet_float new_d_gain;
  Tclvar_float amplitude;
  TclNet_float new_amplitude;
  float        amplitude_min,
               amplitude_max;
  Tclvar_float frequency;
  TclNet_float new_frequency;
  Tclvar_int   input_gain;
  TclNet_int   new_input_gain;
  Tclvar_int   ampl_or_phase;
  TclNet_int   new_ampl_or_phase;
  Tclvar_int   drive_attenuation;
  TclNet_int   new_drive_attenuation;
  Tclvar_float phase;
  TclNet_float new_phase;
  Tclvar_float scan_rate_microns;
  TclNet_float new_scan_rate_microns;


  // Obsolete?

  // parameters for Blunt style
  Tclvar_float blunt_size;
  Tclvar_float blunt_speed;

};

struct AFMScanlineInitializationState {

  AFMScanlineInitializationState (void);

  int mode;
  vrpn_bool feedback_enabled;
  vrpn_bool forcelimit_enabled;

  float setpoint;
  float setpoint_min;
  float setpoint_max;
  float amplitude;
  float amplitude_min;
  float amplitude_max;
};

struct AFMScanlineState {

  AFMScanlineState (const AFMScanlineInitializationState &);
  ~AFMScanlineState (void);

  void getStartPoint(BCPlane *p, float *x, float *y, float *z);
  void getFinishPoint(BCPlane *p, float *x, float *y, float *z);
  void moveScanlineRelative(float dist_fast_dir, float dist_slow_dir);

  vrpn_bool mode_changed,
	     forcelimit_changed,
	     mode_p_changed,
	     forcelimit_p_changed;
    // flags to tell the user interface that something changed.
    // these ought to be moved somewhere else if we make an interface module.

  Tclvar_int mode;	///< tapping or contact
  Tclvar_int feedback_enabled;
  Tclvar_int forcelimit_enabled;
  Tclvar_int continuous_rescan;
  Tclvar_int start_linescan;
  Tclvar_int showing_position;

  Tclvar_int resolution;
  Tclvar_float setpoint;
  float        setpoint_min,   ///< control range of the setpoint knob
               setpoint_max;
  Tclvar_float p_gain;
  Tclvar_float i_gain;
  Tclvar_float d_gain;
  Tclvar_float width;
  Tclvar_float amplitude;
  float        amplitude_min,   ///< control range of the amplitude knob
               amplitude_max;
  Tclvar_float frequency;
  Tclvar_int   input_gain;
  Tclvar_int   ampl_or_phase;
  Tclvar_int   drive_attenuation;
  Tclvar_float phase;
  Tclvar_float scan_rate_microns_per_sec;
  
  Tclvar_float forcelimit;
  Tclvar_float max_z_step;
  Tclvar_float max_xy_step;
  int num_scanlines_to_receive;

  Tclvar_float x_end, y_end, z_end, angle, slope_nm_per_micron;

};

// AFMDataset

// Parts of this class *should* be capable of becoming an independant
// entity in the object structure.  However, inputPoint belongs
// on a microscope;  I suspect the other objects belong here, but am
// not certain.

// GuessAdhesionNames should probably be independant, rather than a
// member function.

class AFMDataset {

  public:

    AFMDataset (void);
    ~AFMDataset (void);

    int Initialize (nmb_Dataset *);

    Point_results * inputPoint;
      ///< last point accessed by this microscope and all data values there
    Point_results * fc_inputPoint;
    Scanline_results currentScanlineData;

    Point_list receivedPointList;
      ///< Last complete results from a feel-ahead call.
    nmm_Sample receivedAlgorithm;
      ///< Sampling strategy used for the last complete feel-ahead call.
    Point_list incomingPointList;
      ///< Current, incomplete results from a feel-ahead call.

    Tclvar_list_of_strings inputPointNames;
      // should be part of user interface?

    Tclvar_list_of_strings fc_inputPointNames;
      // should be part of user interface?

    Scan_channel_selector * scan_channels;
    Point_channel_selector * point_channels;
    ForceCurve_channel_selector * forcecurve_channels;

  private:

};

struct AFMGuardedScanState {

  AFMGuardedScanState (void);
  ~AFMGuardedScanState (void);

  float fNormalX;
  float fNormalY;
  float fNormalZ;
  float fPlaneD;
  float fGuardDepth;
  int nChannel;
  int bDirection;
};

struct AFMInitializationState {

  AFMInitializationState (void);

    // device should be at least "file:" + inputstreamname
  char deviceName [261];
  char inputStreamName [256];
  char outputStreamName [256];

  float stm_z_scale;

  vrpn_bool doRelaxComp;
  vrpn_bool doRelaxUp;
  vrpn_bool doDriftComp;
  vrpn_bool doSplat;
  vrpn_bool readingStreamFile;
  vrpn_bool writingStreamFile;
  vrpn_bool allowdup;
  vrpn_bool useRecvTime;

  int stmRxTmin;
  int stmRxTsep;

  AFMModifyInitializationState modify;
  AFMImageInitializationState image;
  AFMScanlineInitializationState scanline;

  float MaxSafeMove;
  int ModSubWinSz;

  float xMin;
  float xMax;
  float yMin;
  float yMax;

  int mutexPort;

  // Incremental save of stream files to disk, on by default. 
  vrpn_bool incr_save;
};



struct AFMState {

  AFMState (const AFMInitializationState &);
  void SetDefaultScanlineForRegion(nmb_Dataset * dataset);
  ~AFMState (void);

  int StdDelay,                  // wait time for stability. Obsolete
      StPtDelay;                 // delay after setting parameters. Obsolete

    // Used in relaxation compenstation. Should be obsolete, now inside d_relaxComp.
  int stmRxTmin,                 // wait time before accepting data (ms)
      stmRxTsep;                 // time between points for comp (ms)

  float MaxSafeMove;             // fastest speed (nm/ms) that feedback
                                 //   can keep up with. Obsolete

  TclNet_float stm_z_scale;      // exageration factor for height of surface

  // parameters for direction of scan
  vrpn_bool do_raster;
  vrpn_bool do_y_fastest;
  vrpn_bool raster_scan_backwards;

  char deviceName [100];         ///< (sdi) name of microscope to try to open
  char outputStreamName [256];   ///< filenames
  char inputStreamName [256];

  AFMModifyState   modify;
  AFMImageState    image;
  AFMScanlineState scanline;
  AFMDataset       data;
  AFMGuardedScanState guardedscan;

  int acquisitionMode;///< replaces inModifyMode which replaced doing_modify_mode

  Tclvar_int scanning;  ///< is the SPM scanning right now?
  Tclvar_int autoscan;	///< Should the SPM start scanning after touch?
  Tclvar_int withdraw_tip; ///< Causes AFM tip to withdraw

  Tclvar_int slowScanEnabled;
  vrpn_bool cannedLineVisible;
  int cannedLineToggle;

  int lost_changes;
  vrpn_bool new_epoch;

  vrpn_bool regionFlag;            ///< only draw region marker if TRUE

  float xMin, xMax,              
        yMin, yMax,              
        zMin, zMax;
    ///< maximum scan range of which the spm is capable

  // optional system components
  vrpn_bool doDriftComp;           ///< compensate for drift. Always FALSE
  Tclvar_int doRelaxComp;           ///< compensate for relaxation -can change in Tcl.
  vrpn_bool doRelaxUp;             ///< do relaxation even if in imagemode. Always FALSE
  vrpn_bool doSplat;               ///< splat incoming data into grid. Always FALSE
  vrpn_bool snapPlaneFit;

  Tclvar_int read_mode;          // Are we reading device, stream, or files?

  Tclvar_int commands_suspended; // Does Thermo want us to stop sending commands?

  vrpn_bool writingStreamFile;     // was DO_OUTPUT_STREAM
  vrpn_bool writingNetworkStream;  // was DO_NETOUT_STREAM
  vrpn_bool readingStreamFile;     // Getting data from stream file?
  vrpn_bool saveStream;            // 
  vrpn_bool allowdup;
  vrpn_bool useRecvTime;           // Michele Clark's experiments

  int rasterX,                     // used to be x, y in animate.c
      rasterY;

    // Saved values from header of ForceCurveData message, so
    // body can be interpreted correctly. 
  vrpn_float32 fc_x;
  vrpn_float32 fc_y;
  vrpn_int32 fc_numSamples;
  vrpn_int32 fc_numHalfcycles;
  vrpn_int32 fc_sec;
  vrpn_int32 fc_usec;

    /// tracks when PID received from AFM/stream, so we can set
    /// modify PID to match.
  vrpn_bool first_PID_message_pending;

    //OBSOLETE
    //vrpn_bool dlistchange;
    // if true triggers update of X display
    //   in MicroscopeIO.C (was in animate.c)

    /// Used in doSelect to set the scan region. These are the last values
  float select_center_x;
  float select_center_y;
  float select_region_rad;  ///< this is half the width of the select region

  // Obsolete?

  int ModSubWinSz;               // size of subwindow scanned
                                 //   after modification
  vrpn_bool lastPulseOK;           // inverts old pulse_result
  int current_epoch;             // for synchronizing multiple renderers (?)
  int subscan_count;             // for subscanning


  // not fully grokked

  float lastZ;
    // last height read by the microscope (?)
    // used to call relax_init() and as the Z value to use
    // when ignoring new points

  // for jump to scanline feature (after a modification)
  TclNet_int numLinesToJumpBack;

};


#endif  // AFM_STATE_H

