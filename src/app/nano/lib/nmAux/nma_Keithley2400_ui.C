/*===3rdtech===
  Copyright (c) 2000 by 3rdTech, Inc.
  All Rights Reserved.

 This file may not be distributed without the permission of 
 3rdTech, Inc. 
 ===3rdtech===*/
// Important note: When printing the value of a Tclvar_int or Tclvar_float
// using *printf, make sure to cast it to the proper type, otherwise you 
// will get a printout of the pointer value!

#include <stdio.h>

// must come before tcl/tk includes.
#include <vrpn_Connection.h>

extern "C" {
#include <tcl.h>
#include <tk.h>
#include <blt.h>
}
#include <Tcl_Linkvar.h>
#include <Tcl_Netvar.h>
#include <BCGrid.h>
#include <error_display.h>

#include "nma_Keithley2400_ui.h"

nma_Keithley2400_ui_InitializationState::nma_Keithley2400_ui_InitializationState() :
  readingLogFile (VRPN_FALSE),
  writingLogFile (VRPN_FALSE)
{
  strcpy(deviceName, "null");
}


nma_Keithley2400_ui::nma_Keithley2400_ui( Tcl_Interp *interp, 
                                         const char *tcl_script_dir,
                                         const char *name, vrpn_Connection *c ) : 
   tcl_interp(interp),
   
   // Note: These WON'T be the default values - those come from the 
   // keithley2400 object and are set in set_tcl_callbacks();
   vi_stream_time( "vi(stream_time)", 0),
   connect_and_init("vi(connect_and_init)",0),
   source("vi(source)", 0),
   compliance("vi(compliance)", 0),
   compliance_val("vi(compliance_val)", 0.1),
   num_power_line_cycles("vi(num_power_line_cycles)", 1),
   sweep("vi(sweep)", 0),
   sweep_start("vi(sweep_start)", -1),
   sweep_stop("vi(sweep_stop)", 1),
   sweep_stepsize("vi(sweep_stepsize)", 0.1),
   stepsize_update_from_c(0),
   
   sweep_numpoints("vi(sweep_numpoints)", 21),
   sweep_delay("vi(sweep_delay)", 0.01),
   num_sweeps("vi(num_sweeps)", 1),
   initial_delay("vi(initial_delay)", 0 ),
   zero_after_meas("vi(zero_after_meas)", 0 ),
   
   display_enable("vi(display_enable)", 1),
   
   take_iv_curves("vi(take_iv_curves)", 0),
   take_repeat_iv_curves("vi(take_repeat_iv_curves)", 0),
   take_iv_curves_during_mod("vi(curves_during_mod)", 0),    
   save_curves_now("vi(save_curves_now)", 0),
   wire_type("vi(wire_type)",2),
   num_data_vecs(0),
   expected_num_data_vecs(0)
{
  char    command[256];
  keithley2400 = new nma_Keithley2400(name, c);
  
  /* Load the Tcl script that handles main interface window */
  sprintf(command, "source {%s/%s}",tcl_script_dir,VICURVE_TCL_FILE);
  TCLEVALCHECK2(tcl_interp, command);
  
  // set up the callbacks for all tcl variables.
  // Must be done after keithley2400 is initialized.
  set_tcl_callbacks();
  
  // if we're reading a stream file, enable the replay controls
  if ( c->get_File_Connection() != NULL )
  {
    sprintf( command, "set spm_read_mode %d", READ_STREAM );
    int retval = Tcl_Eval( tcl_interp, command );
    if (retval != TCL_OK) 
    {
      display_error_dialog( "Internal: Tcl_Eval failed trying to "
        "enable replay controls for the vi curve  %s.\n",
        tcl_interp->result);
    }
  } 
  
  // Set up the callback to handle data when it is received from 
  // the Keithley
  keithley2400->register_result_handler(this, receive_ResultData);
} // end constructor 


// virtual
nma_Keithley2400_ui::~nma_Keithley2400_ui (void)
{
  if (keithley2400) {
    delete keithley2400;
  }
}


void nma_Keithley2400_ui::reset( )
{
  // only do this if we're reading from a stream file
  if( keithley2400->isReadingStreamFile( ) )
  {
    this->num_data_vecs = 0;
    this->expected_num_data_vecs = 0;

    char command [] = "clear_curves_now";
    // This procedure is defined in the tcl file.
    TCLEVALCHECK2(tcl_interp, command);
  }
}


void nma_Keithley2400_ui::setTimeFromStream( )
{
  vi_stream_time = keithley2400->getTimeSinceConnected( );
}


int nma_Keithley2400_ui::mainloop(const struct timeval * timeout)
{
  setTimeFromStream();
  return keithley2400->mainloop(timeout);
}


//--------------------------------------------------------------------------
// Handles changes to the Tcl variables

// Initial connection and initialization - send everything.
void nma_Keithley2400_ui::
handle_connect_and_init(vrpn_int32 /*_newvalue*/, void *_userdata) {
  nma_Keithley2400_ui * me = (nma_Keithley2400_ui *)_userdata;
  
  me->keithley2400->send_Device();
  me->keithley2400->send_Clear( );
  me->keithley2400->send_AllSettings();
  
}

void nma_Keithley2400_ui::
handle_int_param_change(vrpn_int32 /*_newvalue*/, void *_userdata) {
  nma_Keithley2400_ui * me = (nma_Keithley2400_ui *)_userdata;
  
  me->keithley2400->d_source = me->source;
  me->keithley2400->d_compliance = me->compliance;
  me->keithley2400->d_sweep = me->sweep;
  me->keithley2400->d_sweep_numpoints = me->sweep_numpoints;
  me->keithley2400->d_display_enable = me->display_enable;
  me->keithley2400->send_AllSettings();
}

void nma_Keithley2400_ui::
handle_wire_type_change(vrpn_int32 /*_newvalue*/, void *_userdata) {
  nma_Keithley2400_ui * me = (nma_Keithley2400_ui *)_userdata;
  
  me->keithley2400->d_wire_type = me->wire_type;
  me->keithley2400->send_OutputOff();
  me->keithley2400->send_WireType();
  me->keithley2400->send_OutputOn();
}

void nma_Keithley2400_ui::
handle_float_param_change(vrpn_float64 /*_newvalue*/, void *_userdata) {
  nma_Keithley2400_ui * me = (nma_Keithley2400_ui *)_userdata;
  
  me->keithley2400->d_compliance_val = me->compliance_val;
  me->keithley2400->d_num_power_line_cycles = me->num_power_line_cycles;
  me->keithley2400->d_sweep_start = me->sweep_start;
  me->keithley2400->d_sweep_stop = me->sweep_stop;
  me->keithley2400->d_sweep_delay = me->sweep_delay;
  me->keithley2400->send_AllSettings();
}

// XXX This procedure doesn't work properly, so I made
// Stepsize read-only in Tcl. It seems to start a callback loop
// where handle_stepsize_change and handle_numpoints_change get
// called repeatedly and they end up with the wrong values.

// Stepsize is not used intuitively by the Keithley - instead
// the stepsize is used as a guide for choosing the number
// of points in a sweep. So we link stepsize and numpoints -
// one sets the other. We _only_ send numpoints to the Keithley.
void nma_Keithley2400_ui::
handle_stepsize_change(vrpn_float64 /*_newvalue*/, void * _userdata) {
  nma_Keithley2400_ui * me = (nma_Keithley2400_ui *)_userdata;
  
  // Trigger a change in the Keithley settings, 
  // by changing the tcl variable sweep_numpoints, but
  // only if the change came from the user via the tcl interface
  // and not from a change to numpoints, start or stop.
  if (me->stepsize_update_from_c) {
    me->stepsize_update_from_c = 0;
    
  } else {
    // This truncates the number of points to insure it is an integer
    // For instance, if (stop-start) = 2, and step = 0.7, we would
    // take 3.857 steps. We truncate to 3, just as the Keithley does.
    int new_num_points 
      = (int)( ( me->keithley2400->d_sweep_stop 
		 - me->keithley2400->d_sweep_start )
	       / me->sweep_stepsize + 1 );
    // The truncation above means the stepsize might get bigger
    // In the example, 3 steps means a stepsize of 1. 
    me->sweep_stepsize 
      = ( ( me->keithley2400->d_sweep_stop 
	    - me->keithley2400->d_sweep_start ) 
	  /  (new_num_points - 1 ) );
    
    me->sweep_numpoints = new_num_points;
  }
}

// Stepsize is not used intuitively by the Keithley - instead
// the stepsize is used as a guide for choosing the number
// of points in a sweep. So we link stepsize and numpoints -
// one sets the other.  We _only_ send numpoints to the Keithley.
void nma_Keithley2400_ui::
handle_numpoints_change(vrpn_int32 /*_newvalue*/, void *_userdata) {
  nma_Keithley2400_ui * me = (nma_Keithley2400_ui *)_userdata;
  
  // Make sure stepsize knows we are updating it from C code
  me->stepsize_update_from_c = 1;
  // Calc a new step size from the new number of points.
  me->sweep_stepsize = ( ( me->keithley2400->d_sweep_stop 
			   - me->keithley2400->d_sweep_start ) 
			 / ( me->sweep_numpoints - 1 ) );
  
  // Trigger a change in the Keithley settings, if need be.
  if (me->keithley2400->d_sweep_numpoints != me->sweep_numpoints) {
    me->keithley2400->d_sweep_numpoints =me->sweep_numpoints;
    me->keithley2400->send_AllSettings();
    
  }
}

void nma_Keithley2400_ui::
handle_startstop_change(vrpn_float64 /*_newvalue*/, void *_userdata) {
  nma_Keithley2400_ui * me = (nma_Keithley2400_ui *)_userdata;
  
  // Make sure stepsize knows we are updating it from C code
  me->stepsize_update_from_c = 1;
  // Calc a new step size from the new number of points.
  me->sweep_stepsize = ((me->sweep_stop - me->sweep_start)/
    (me->keithley2400->d_sweep_numpoints -1));
  
  // Trigger a change in the Keithley settings.
  me->keithley2400->d_sweep_stop =me->sweep_stop;
  me->keithley2400->d_sweep_start =me->sweep_start;
  me->keithley2400->send_AllSettings();
}


void nma_Keithley2400_ui::
handle_initial_delay_change( vrpn_float64 /*newvalue*/, void *userdata )
{
  nma_Keithley2400_ui * me = (nma_Keithley2400_ui *) userdata;
  me->keithley2400->d_initial_delay = me->initial_delay;
  // don't send all settings to the keithley, since this is used 
  // internally in Keithley2400
}


void nma_Keithley2400_ui::
handle_zero_after_meas_change( vrpn_int32 /*newvalue*/, void *userdata )
{
  nma_Keithley2400_ui * me = (nma_Keithley2400_ui *) userdata;
  me->keithley2400->d_zero_after_meas = me->zero_after_meas;
  // don't send all settings to the keithley, since this is used 
  // internally in Keithley2400
}

  
void nma_Keithley2400_ui::
handle_take_iv_curves(vrpn_int32 /*_newvalue*/, void *_userdata) {
  nma_Keithley2400_ui * me = (nma_Keithley2400_ui *)_userdata;
  me->expected_num_data_vecs = me->num_data_vecs + me->num_sweeps;
  for (int i = 0; i < me->num_sweeps; i++) {
    me->keithley2400->send_DoCurve();
  }
}

void nma_Keithley2400_ui::
handle_take_repeat_iv_curves(vrpn_int32 /*_newvalue*/, void *_userdata) {
  nma_Keithley2400_ui * me = (nma_Keithley2400_ui *)_userdata;
  
  // Set the number of expected data vectors
  // this is only 1 more that we already have, right now.
  // receive_ResultData will update this
  me->expected_num_data_vecs = me->num_data_vecs + 1;
  
  
  // Get the ball rolling - take the first IV Curve
  // After this, rcv_Result asks for more data.
  if ((int)me->take_repeat_iv_curves) {
    me->keithley2400->d_repeat_curve_active = 1;
    me->keithley2400->send_DoCurve();
  } else {
    me->keithley2400->d_repeat_curve_active = 0;
  }
}

int nma_Keithley2400_ui::set_tcl_callbacks() 
{
  vi_stream_time = keithley2400->getTimeSinceConnected( );
  connect_and_init.addCallback(handle_connect_and_init, this);
  source = keithley2400->d_source;
  source.addCallback(handle_int_param_change, this);
  compliance = keithley2400->d_compliance;
  compliance.addCallback(handle_int_param_change, this);
  compliance_val = keithley2400->d_compliance_val;
  compliance_val.addCallback(handle_float_param_change, this);
  num_power_line_cycles = keithley2400->d_num_power_line_cycles;
  num_power_line_cycles.addCallback(handle_float_param_change, this);
  sweep = keithley2400->d_sweep;
  sweep.addCallback(handle_int_param_change, this);
  sweep_start = keithley2400->d_sweep_start;
  sweep_start.addCallback(handle_startstop_change, this);
  sweep_stop = keithley2400->d_sweep_stop;
  sweep_stop.addCallback(handle_startstop_change, this);
  sweep_stepsize.addCallback(handle_stepsize_change, this);
  sweep_numpoints.addCallback(handle_numpoints_change, this);
  // Set this after the callback is set to initialize stepsize.
  sweep_numpoints = keithley2400->d_sweep_numpoints;
  sweep_delay = keithley2400->d_sweep_delay;
  sweep_delay.addCallback(handle_float_param_change, this);
  initial_delay = keithley2400->d_initial_delay;
  initial_delay.addCallback( handle_initial_delay_change, this );
  zero_after_meas = keithley2400->d_zero_after_meas;
  zero_after_meas.addCallback( handle_zero_after_meas_change, this );
  num_sweeps = 1;
  display_enable = keithley2400->d_display_enable;
  display_enable.addCallback(handle_int_param_change, this);
  take_iv_curves.addCallback(handle_take_iv_curves, this);
  take_repeat_iv_curves.addCallback(handle_take_repeat_iv_curves, this);
  wire_type.addCallback(handle_wire_type_change, this);
  return 0;
}

// Receive data when it arrives from the Keithley, save it, and display it.
void nma_Keithley2400_ui::
receive_ResultData(void *_userdata, const vrpn_VIRESULTDATACB &info)
{
  nma_Keithley2400_ui * me = (nma_Keithley2400_ui *)_userdata;
  Tcl_Interp	*interp = me->tcl_interp;
  Blt_Vector *vecPtr = NULL;
  char x_vec_name[30];
  char y_vec_name[30];
  
  me->setTimeFromStream( );  // make sure time is updated
  sprintf(x_vec_name,"vi_volt_vec%d", (int) me->vi_stream_time );
  sprintf(y_vec_name,"vi_curr_vec%d", (int) me->vi_stream_time );
  
  // I'm pretty sure we need to use Tcl_Alloc rather than
  // new or malloc to get the memory.
  if(info.num_values%2 !=0)
    fprintf(stderr, "nma_Keithley2400_ui::receive_ResultData: "
            "odd number of values - should be volt-curr pairs?!?!\n");
  int vec_len = info.num_values/2;
  double * x_tcl_data = (double *)Tcl_Alloc(sizeof(double) *vec_len);
  double * y_tcl_data = (double *)Tcl_Alloc(sizeof(double) *vec_len);
  for (vrpn_int32 i = 0; i< vec_len; i++) {
    x_tcl_data[i] = info.data[2*i];
    y_tcl_data[i] = info.data[2*i+1];
  }
  
  // First, create a vector (of zero length), or get the existing vector
  if (Blt_VectorExists(interp, x_vec_name)) {
    if (Blt_GetVector (interp, x_vec_name, &vecPtr)!= TCL_OK) {
      fprintf(stderr, "Can't get vector %s\n", x_vec_name);
      return;
    }
  } else {
    if (Blt_CreateVector (interp, x_vec_name, 0, &vecPtr)!= TCL_OK) {
      fprintf(stderr, "Can't create vector %s\n", x_vec_name);
      return;
    }
  }
  // Now set the contents of the vector
  // By setting the last parameter to TCL_DYNAMIC, we tell
  // Tcl to free the storage later, when needed. 
  if (Blt_ResetVector (vecPtr, x_tcl_data, vec_len, 
    vec_len,TCL_DYNAMIC)!= TCL_OK) {
    fprintf(stderr,"Can't assign values to vector %s\n", x_vec_name);
    return;
  }
  
  // First, create a vector (of zero length), or get the existing vector
  if (Blt_VectorExists(interp, y_vec_name)) {
    if (Blt_GetVector (interp, y_vec_name, &vecPtr)!= TCL_OK) {
      fprintf(stderr, "Can't get vector %s\n", y_vec_name);
      return;
    }
  } else {
    if (Blt_CreateVector (interp, y_vec_name, 0, &vecPtr)!= TCL_OK) {
      fprintf(stderr, "Can't create vector %s\n", y_vec_name);
      return;
    }
  }
  // Now set the contents of the vector
  // By setting the last parameter to TCL_DYNAMIC, we tell
  // Tcl to free the storage later, when needed. 
  if (Blt_ResetVector (vecPtr, y_tcl_data, vec_len, 
    vec_len,TCL_DYNAMIC)!= TCL_OK) {
    fprintf(stderr,"Can't assign values to vector %s\n", y_vec_name);
    return;
  }
  
  // Graph this data vector.
  // This procedure is defined in the tcl file.
  char command[100];
  sprintf(command, "vi_add_chart_element %s %s %d %d", 
    x_vec_name, y_vec_name, me->num_data_vecs, (int) (me->vi_stream_time) );
  TCLEVALCHECK2(interp, command);
  
  // We've successfully processed another data vector.
  me->num_data_vecs++;
  
  // If we are taking repeated curves, we expect another
  // curve.
  if( me->keithley2400->d_repeat_curve_active == 1) {
    me->expected_num_data_vecs = me->num_data_vecs +1;
  }
  // If we're done receiving a set of data, prompt the user to
  // save it.
  if (me->num_data_vecs == me->expected_num_data_vecs) {
    me->save_curves_now = 1;
  }
}


void nma_Keithley2400_ui::stop_repeat_curves_now()
{
  take_repeat_iv_curves = 0;
  handle_take_repeat_iv_curves(0, this);
}
void nma_Keithley2400_ui::take_repeat_curves_now()
{
  take_repeat_iv_curves = 1;
  handle_take_repeat_iv_curves(1, this);
}

//static
int nma_Keithley2400_ui::EnterModifyMode(void * userdata)
{
  // If we are connected to the Keithley 2400 VI curve generator
  // and we are supposed to , take repeated VI curves. 
  if (userdata == NULL) return 0;
  nma_Keithley2400_ui *me = (nma_Keithley2400_ui *)userdata;
  if (me->take_iv_curves_during_mod) {
    me->take_repeat_curves_now();
  }
  return 0;
}
//static
int nma_Keithley2400_ui::EnterImageMode(void * userdata)
{
  // If we are connected to the Keithley 2400 VI curve generator
  // and we are supposed to , stop taking repeated VI curves. 
  if (userdata == NULL) return 0;
  nma_Keithley2400_ui *me = (nma_Keithley2400_ui *)userdata;
  if (me->take_iv_curves_during_mod) {
    me->stop_repeat_curves_now();
  }
  return 0;
}
