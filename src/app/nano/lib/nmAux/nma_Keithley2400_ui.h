#ifndef _NMA_KEITHLEY2400_UI_H
#define _NMA_KEITHLEY2400_UI_H

// must come before tcl/tk includes.
#include <vrpn_Connection.h>
#include <vrpn_Types.h>

#include <Tcl_Linkvar.h>
#include <Tcl_Netvar.h>

#include <nmb_Types.h>	// for vrpn_bool

#include <nma_Keithley2400.h>

#define VICURVE_FILE_SUFFIX ".vic"
#define VICURVE_TCL_FILE "keithley2400.tcl"

class nma_Keithley2400_ui_InitializationState {
public:
  nma_Keithley2400_ui_InitializationState(void);
  
  char deviceName [128];
  char inputLogName [256];
  char outputLogName [256];
  
  vrpn_bool readingLogFile;
  vrpn_bool writingLogFile;
};

class nma_Keithley2400_ui {
public:
  nma_Keithley2400_ui(Tcl_Interp *interp, const char *tcl_script_dir, 
                      const char *name, 
                      vrpn_Connection *c = NULL);
  virtual ~nma_Keithley2400_ui (void);
  
  void reset( ); // if we're reading a stream file, call this when we rewind

  // Handle getting any reports
  virtual int mainloop(const struct timeval * timeout = NULL);

  virtual void setTimeFromStream( );
  
  nma_Keithley2400 * keithley2400;
  Tcl_Interp	*tcl_interp;
  
  //--------------------------------------------------------------------
  // Declaring some integer and floating-point variables that will be
  // automatically linked to Tcl variables.
  Tclvar_int vi_stream_time;
  Tclvar_int connect_and_init;
  Tclvar_int source;
  Tclvar_int compliance;
  Tclvar_float compliance_val;
  Tclvar_float num_power_line_cycles;
  Tclvar_int sweep;
  Tclvar_float sweep_start;
  Tclvar_float sweep_stop;
  Tclvar_float sweep_stepsize;
  // If this variable is set, stepsize won't propagate changes to numpoints.
  int stepsize_update_from_c;
  
  Tclvar_int sweep_numpoints;
  Tclvar_float sweep_delay;
  Tclvar_int num_sweeps;
  Tclvar_float initial_delay;
  Tclvar_int zero_after_meas;
  
  Tclvar_int display_enable;
  
  Tclvar_int take_iv_curves;
  
  // Take IV curves over and over until this variable is turned off
  Tclvar_int take_repeat_iv_curves;
  
  // Take IV curves over and over during a modification, when this
  // class is used inside nM.
  Tclvar_int take_iv_curves_during_mod;
  
  // Save the curves we have taken now
  Tclvar_int save_curves_now;
  
  Tclvar_int wire_type;
  
  // Remember the number of data vectors that have been received,
  // so we can create unique names
  int num_data_vecs;
  
  // Record how many data vectors we think we will be receiving, 
  // so that when we get all of them, we can prompt the user
  // to save them.
  int expected_num_data_vecs;
  
  
  //--------------------------------------------------------------------------
  // Handles changes to the Tcl variables
  
  // Initial connection and initialization - send everything.
  static void handle_connect_and_init(vrpn_int32 /*_newvalue*/, void *_userdata);
  
  static void handle_int_param_change(vrpn_int32 /*_newvalue*/, void *_userdata);
  static void handle_float_param_change(vrpn_float64 /*_newvalue*/, void *_userdata);
  
  // Stepsize is not used intuitively by the Keithley - instead
  // the stepsize is used as a guide for choosing the number
  // of points in a sweep. So we link stepsize and numpoints -
  // one sets the other. We _only_ send numpoints to the Keithley.
  static void handle_stepsize_change(vrpn_float64 /*_newvalue*/, void * _userdata);
  
  // Stepsize is not used intuitively by the Keithley - instead
  // the stepsize is used as a guide for choosing the number
  // of points in a sweep. So we link stepsize and numpoints -
  // one sets the other.  We _only_ send numpoints to the Keithley.
  static void handle_numpoints_change(vrpn_int32 /*_newvalue*/, void *_userdata);
  static void handle_startstop_change(vrpn_float64 /*_newvalue*/, void *_userdata);

  static void handle_initial_delay_change( vrpn_float64 /*newvalue*/, void *userdata );
  static void handle_zero_after_meas_change( vrpn_int32 /*newvalue*/, void *userdata );
  
  int set_voltage_vector();
  
  static void handle_take_iv_curves(vrpn_int32 /*_newvalue*/, void *_userdata);
  static void handle_take_repeat_iv_curves(vrpn_int32 /*_newvalue*/, void *_userdata);
  static void handle_wire_type_change(vrpn_int32 /*_newvalue*/, void *_userdata);
  int set_tcl_callbacks();
  
  // Receive data when it arrives from the Keithley, save it, and display it.
  static void receive_ResultData(void *userdata, const vrpn_VIRESULTDATACB &info);
  // Why do we need these? nM needs to trigger the repeat curve-taking
  // ability without handling Tcl events. These functions do that.
  void stop_repeat_curves_now();
  void take_repeat_curves_now();
  
  // These handlers should be called when the microscope enter Image
  // or Modification mode. They allow the Keithley to take
  // resistance measurements during a modification. If the
  // Tclvar_int take_iv_curves_during_mod is true, EnterModifyMode
  // will call take_repeat_curves_now, and EnterImageMode will call
  // stop_repeat_curves_now.
  static int EnterModifyMode(void * userdata);
  static int EnterImageMode(void * userdata);
};

#endif
