/*===3rdtech===
  Copyright (c) 2000 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#include "nmm_MicroscopeRemote.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
//#if !defined(_WIN32) || defined(__CYGWIN__)
#if !defined(_WIN32)
#include <sys/time.h> // for RecordResistance()
#include <unistd.h>  // for sleep()
#endif

#include <Topo.h>
#include <Point.h>
#include <BCPlane.h>
#include <nmb_Dataset.h>
#include <nmb_Decoration.h>  // for addScrapeMark()
#include <Tcl_Linkvar.h>
#include <stm_file.h>	// not sure if we need it.
#include <nmb_Time.h>
#include <nmb_Debug.h>
//#include <nmb_Globals.h>  // get rid of this if we're going to be modular
#include <nmb_Types.h>
#include <nmb_Line.h>

#include "drift.h"
#include "splat.h"
//#include "relax.h"  // relax_set_min, relax_set_max
                    // encode and decode functions here ?
#include "nmm_RelaxComp.h"
#include "nmm_Sample.h"

#include "vrpn_FileConnection.h"	// for vrpn_File_Connection class
#include "error_display.h"
//#include <microscape.h>  // for spm_verbosity

#if defined(_WIN32) && !defined(__CYGWIN__)
// bogus double to float conversion warning.
#pragma warning(disable:4244)
#endif

#define CHECK(a) if ((a) == -1) return -1

#ifdef MAX
  #undef MAX
#endif
#define MAX(a,b) ((a > b) ? (a) : (b))
#ifdef MIN
  #undef MIN
#endif
#define MIN(a,b) ((a < b) ? (a) : (b))
#define      NMB_NEAR(x0,x1)     (fabs(x0-x1) < 0.001)

#include <vrpn_Connection.h>

#if (!defined(X) || !defined(Y) || !defined(Z))
#define	X	(0)
#define	Y	(1)
#define	Z	(2)
#endif

#ifndef M_PI
#define M_PI 3.141592653589793238
#define M_PI_2		1.57079632679489661923
#endif

// from nmm_Microscope.h
#define FC_MAX_HALFCYCLES (100)

// I feel bad, but we need to know if there is an ohmmeter
class vrpn_Ohmmeter_Remote;
extern vrpn_Ohmmeter_Remote *ohmmeter;

// Microscope
//
// Tom Hudson, September 1997
// Code mostly from microscape.c and animate.c

// All callbacks are in MicroscopeRcv.C

nmm_Microscope_Remote::nmm_Microscope_Remote
  (const AFMInitializationState & i,
   vrpn_Connection * c) :
    nmb_SharedDevice_Remote (i.deviceName, c),
    nmm_Microscope (i.deviceName, c),
    state(i),
    d_relax_comp(this),
    d_dataset (NULL),
    d_decoration (NULL),
    d_tcl_script_dir (NULL),
    d_mod_window_initialized (vrpn_FALSE),
    d_mod_window_min_x (0),
    d_mod_window_min_y (0),
    d_mod_window_max_x (0),
    d_mod_window_max_y (0),
    d_mod_window_pad (10),
    d_res_channel_added(vrpn_FALSE),
    d_pointDataHandlers (NULL),
    d_modifyModeHandlers (NULL),
    d_imageModeHandlers (NULL),
    d_scanlineModeHandlers (NULL),
    d_scanlineDataHandlers (NULL),
    d_sampleAlgorithm (NULL),
    d_accumulatePointResults (vrpn_FALSE)
{
  gettimeofday(&d_nowtime, &d_nowzone);
  d_next_time.tv_sec = 0L;
  d_next_time.tv_usec = 0L;

  // Turn on relaxation compensation, with k/t decay model. 
/*
  This is done when in RcvGotConnection2 so we shouldn't do it here
  if (state.doRelaxComp) {
      d_relax_comp.enable(nmm_RelaxComp::DECAY);
  }

  d_relax_comp.set_ignore_time_ms(state.stmRxTmin);
  d_relax_comp.set_separation_time_ms(state.stmRxTsep);
*/

  if (!d_connection) {
    return;
  }

  markTypeAsSafe(d_QueryScanRange_type);
  markTypeAsSafe(d_QueryStdDevParams_type);
  markTypeAsSafe(d_QueryPulseParams_type);

  d_connection->register_handler(d_VoltsourceEnabled_type,
                                 handle_VoltsourceEnabled,
                                 this);
  d_connection->register_handler(d_VoltsourceDisabled_type,
                                 handle_VoltsourceDisabled,
                                 this);
  d_connection->register_handler(d_AmpEnabled_type,
                                 handle_AmpEnabled,
                                 this);
  d_connection->register_handler(d_AmpDisabled_type,
                                 handle_AmpDisabled,
                                 this);
  d_connection->register_handler(d_SuspendCommands_type,
                                 handle_SuspendCommands,
                                 this);
  d_connection->register_handler(d_ResumeCommands_type,
                                 handle_ResumeCommands,
                                 this);
  d_connection->register_handler(d_StartingToRelax_type,
                                 handle_StartingToRelax,
                                 this);
  d_connection->register_handler(d_RelaxSet_type,
                                 handle_RelaxSet,
                                 this);
//    d_connection->register_handler(d_StdDevParameters_type,
//                                   handle_StdDevParameters,
//                                   this);
  d_connection->register_handler(d_WindowLineData_type,
                                 handle_WindowLineData,
                                 this);
  d_connection->register_handler(d_WindowScanNM_type,
                                 handle_WindowScanNM,
                                 this);
  d_connection->register_handler(d_WindowBackscanNM_type,
                                 handle_WindowBackscanNM,
                                 this);
  d_connection->register_handler(d_PointResultNM_type,
                                 handle_PointResultNM,
                                 this);
  d_connection->register_handler(d_PointResultData_type,
                                 handle_PointResultData,
                                 this);
  d_connection->register_handler(d_BottomPunchResultData_type,
                                 handle_BottomPunchResultData,
                                 this);
  d_connection->register_handler(d_TopPunchResultData_type,
                                 handle_TopPunchResultData,
                                 this);
  d_connection->register_handler(d_ZigResultNM_type,
                                 handle_ResultNM,
                                 this);
  d_connection->register_handler(d_BluntResultNM_type,
                                 handle_ResultNM,
                                 this);
  d_connection->register_handler(d_ForceCurveData_type,
				 handle_ForceCurveData,
				 this);
  d_connection->register_handler(d_Scanning_type,
                                 handle_Scanning,
                                 this);
  d_connection->register_handler(d_ScanRange_type,
                                 handle_ScanRange,
                                 this);
  d_connection->register_handler(d_ReportScanAngle_type,
                                 handle_ReportScanAngle,
                                 this);
  d_connection->register_handler(d_SetRegionCompleted_type,
                                 handle_SetRegionCompleted,
                                 this);
  d_connection->register_handler(d_SetRegionClipped_type,
                                 handle_SetRegionClipped,
                                 this);
  d_connection->register_handler(d_ResistanceFailure_type,
                                 handle_ResistanceFailure,
                                 this);
  d_connection->register_handler(d_Resistance_type,
                                 handle_Resistance,
                                 this);
  d_connection->register_handler(d_Resistance2_type,
                                 handle_Resistance2,
                                 this);
  d_connection->register_handler(d_ReportSlowScan_type,
                                 handle_ReportSlowScan,
                                 this);
  d_connection->register_handler(d_ScanParameters_type,
                                 handle_ScanParameters,
                                 this);
  d_connection->register_handler(d_HelloMessage_type,
                                 handle_HelloMessage,
                                 this);
  d_connection->register_handler(d_ClientHello_type,
                                 handle_ClientHello,
                                 this);
  d_connection->register_handler(d_ScanDataset_type,
                                 handle_ScanDataset,
                                 this);
  d_connection->register_handler(d_PointDataset_type,
                                 handle_PointDataset,
                                 this);
  d_connection->register_handler(d_PidParameters_type,
                                 handle_PidParameters,
                                 this);
  d_connection->register_handler(d_ScanrateParameter_type,
                                 handle_ScanrateParameter,
                                 this);
  d_connection->register_handler(d_ReportGridSize_type,
                                 handle_ReportGridSize,
                                 this);
  d_connection->register_handler(d_ServerPacketTimestamp_type,
                                 handle_ServerPacketTimestamp,
                                 this);
  d_connection->register_handler(d_TopoFileHeader_type,
                                 handle_TopoFileHeader,
                                 this);

  d_connection->register_handler(d_RecvTimestamp_type,
                                 handle_RecvTimestamp,
                                 this);
  d_connection->register_handler(d_FakeSendTimestamp_type,
                                 handle_FakeSendTimestamp,
                                 this);
  d_connection->register_handler(d_UdpSeqNum_type,
                                 handle_UdpSeqNum,
                                 this);

  // AFM-ish

  d_connection->register_handler(d_InTappingMode_type,
                                 handle_InTappingMode,
                                 this);
  d_connection->register_handler(d_InOscillatingMode_type,
                                 handle_InOscillatingMode,
                                 this);
  d_connection->register_handler(d_InContactMode_type,
                                 handle_InContactMode,
                                 this);
  d_connection->register_handler(d_InDirectZControl_type,
                                 handle_InDirectZControl,
                                 this);
  d_connection->register_handler(d_InSewingStyle_type,
                                 handle_InSewingStyle,
                                 this);
  d_connection->register_handler(d_InSpectroscopyMode_type,
				 handle_InSpectroscopyMode,
				 this);
  d_connection->register_handler(d_ForceParameters_type,
                                 handle_ForceParameters,
                                 this);
  d_connection->register_handler(d_BaseModParameters_type,
                                 handle_BaseModParameters,
                                 this);
  d_connection->register_handler(d_ForceSettings_type,
                                 handle_ForceSettings,
                                 this);
  d_connection->register_handler(d_InModModeT_type,
                                 handle_InModModeT,
                                 this);
  d_connection->register_handler(d_InModMode_type,
                                 handle_InModMode,
                                 this);
  d_connection->register_handler(d_InImgModeT_type,
                                 handle_InImgModeT,
                                 this);
  d_connection->register_handler(d_InImgMode_type,
                                 handle_InImgMode,
                                 this);
  d_connection->register_handler(d_ModForceSet_type,
                                 handle_ModForceSet,
                                 this);
  d_connection->register_handler(d_ImgForceSet_type,
                                 handle_ImgForceSet,
                                 this);
  d_connection->register_handler(d_ModSet_type,
                                 handle_ModSet,
                                 this);
  d_connection->register_handler(d_ImgSet_type,
                                 handle_ImgSet,
                                 this);
  d_connection->register_handler(d_ForceSet_type,
                                 handle_ForceSet,
                                 this);
  d_connection->register_handler(d_ForceSetFailure_type,
                                 handle_ForceSetFailure,
                                 this);
  d_connection->register_handler(d_MaxSetpointExceeded_type,
				 handle_MaxSetpointExceeded,
                                 this);

  // STM-ish

  d_connection->register_handler(d_PulseParameters_type,
                                 handle_PulseParameters,
                                 this);
  d_connection->register_handler(d_PulseCompletedNM_type,
                                 handle_PulseCompletedNM,
                                 this);
  d_connection->register_handler(d_PulseFailureNM_type,
                                 handle_PulseFailureNM,
                                 this);

  // scanline mode
  d_connection->register_handler(d_InScanlineMode_type,
				 handle_InScanlineMode,
				 this);
  d_connection->register_handler(d_ScanlineData_type,
				 handle_ScanlineData,
				 this);

  d_connection->register_handler(d_DroppedConnection_type,
                                 handle_DroppedConnection2,
                                   this);
  registerSynchHandler(handle_barrierSynch, this);
  registerGotMutexCallback(this, handle_GotMicroscopeControl);
}


nmm_Microscope_Remote::~nmm_Microscope_Remote (void) {
  // Shut the server down nicely
  // Check to make sure we are talking to live microscope.
    /* Handled by vrpn_drop_connection message, automatic from vrpn.
  if (d_dataset && (d_dataset->inputGrid->readMode() == READ_DEVICE)) {
    if (d_connection && d_connection->doing_okay()) {
      if (Shutdown() == -1) {
	fprintf(stderr, "nmm_Microscope_Remote::~Microscope():  "
		"could not send quit command to STM server\n");
      }
      // Wait to give the server a chance to receive the message before it
      // gets a connection-closed exception
      //sleep(3);
      // TODO:  verify that destructors of members get called after
      //   this destructor;  otherwise we've got a mess to handle
    }
  }
    */
  if (d_tcl_script_dir) {
    delete [] d_tcl_script_dir;
  }

  // TODO:  clean up callback lists on d_connection
  if (!d_connection) {
    return;
  }
  d_connection->unregister_handler(d_DroppedConnection_type,
                                 handle_DroppedConnection2,
                                   this);
}




// virtual
int nmm_Microscope_Remote::mainloop (void) {

  struct timeval skiptime;
  struct timeval last_time;

  //nmb_SharedDevice::mainloop();

  // Read in the changes
  VERBOSE(5, "   setup");
  state.lost_changes = 0;
  state.new_epoch = VRPN_FALSE;

  last_time = d_nowtime;
  gettimeofday(&d_nowtime, &d_nowzone);
  time_subtract(d_nowtime, last_time, &skiptime);
  time_multiply(skiptime, d_decoration->rateOfTime, &skiptime);
  time_add(d_next_time, skiptime, &d_next_time);

  if (d_connection) {
    d_connection->mainloop();
  }

  if (d_connection && !d_connection->doing_okay()) {
    fprintf(stderr, "nmm_Microscope_Remote::mainloop():  "
                    "lost connection.\n");
    //d_dataset->done = VRPN_TRUE;
    return -1;
  }

  // Tiger  added to get time elapsed since connected,
  //        used by Live and Replay mode
  getTimeSinceConnected();

  return 0;
}




char * nmm_Microscope_Remote::encode_GetNewPointDatasets
                            (vrpn_int32 * len,
                             const Tclvar_list_of_strings * channel_list,
                             Tclvar_int* active_list[] , Tclvar_int* numsamples_list[] )
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;
  vrpn_int32 numSets = 0;
  vrpn_int32 i;

  //fprintf(stderr, "nmm_Microscope_Remote::encode_GetNewPointDatasets(): Entering...\n");

  if (!len) return NULL;

  for (i = 0; i < channel_list->numEntries(); i++)
    if (1 == (*active_list[i])) numSets++;

  *len = (vrpn_int32)(sizeof(vrpn_int32) + (STM_NAME_LENGTH + sizeof(vrpn_int32)) * numSets);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope_Remote::encode_GetNewPointDatasets:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    //fprintf(stderr, "nmm_Microscope_Remote::encode_GetNewPointDatasets(): numSets = %d\n", numSets);
    vrpn_buffer(&mptr, &mlen, numSets);
    for (i = 0; i < channel_list->numEntries(); i++)
      if (1 == (*active_list[i])) {
          // This is safe because STM_NAME_LENGTH is 64 and nmb_STRING_LENGTH is 128
        vrpn_buffer(&mptr, &mlen, channel_list->entry(i), STM_NAME_LENGTH);

        // Also send the number of samples per dataset
        vrpn_buffer(&mptr, &mlen, (vrpn_int32)*numsamples_list[i]);
      }
  }

  return msgbuf;
}

char * nmm_Microscope_Remote::encode_GetNewScanDatasets
                            (vrpn_int32 * len,
                             const Tclvar_list_of_strings * channel_list,
                             Tclvar_int* active_list[]) 
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;
  vrpn_int32 numSets = 0;
  vrpn_int32 i;

  if (!len) return NULL;

  for (i = 0; i < channel_list->numEntries(); i++)
    if (1 == (*active_list[i])) numSets++;

  //fprintf(stderr, "nmm_Microscope_Remote::encode_GetNewScanDatasets: numSets = %d\n", numSets);
  *len = (vrpn_int32)(sizeof(vrpn_int32) + STM_NAME_LENGTH * numSets);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope_Remote::encode_GetNewScanDatasets:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, numSets);
    for (i = 0; i < channel_list->numEntries(); i++) {
      if (1 == (*active_list[i])) {
          // This is safe because STM_NAME_LENGTH is 64 and nmb_STRING_LENGTH is 128
          vrpn_buffer(&mptr, &mlen, channel_list->entry(i), STM_NAME_LENGTH);
	  //fprintf(stderr, "     name: %s\n",list->Checkbox_name(i));
      }
    }
  }
  
  return msgbuf;
}

long nmm_Microscope_Remote::InitializeDataset (nmb_Dataset * ds) {
  BCPlane * plane;

  d_dataset = ds;

  state.data.Initialize(ds);
  plane = ds->ensureHeightPlane();
  plane->setScale(state.stm_z_scale);
  state.SetDefaultScanlineForRegion(ds);
    // safest to SetDefaultScanlineForRegion after ensureHeightPlane(),
    // since it assumes a height plane exists.

  state.modify.region_diag =
      sqrt(((d_dataset->inputGrid->maxX() - d_dataset->inputGrid->minX()) *
            (d_dataset->inputGrid->maxX() - d_dataset->inputGrid->minX())) +
           ((d_dataset->inputGrid->maxY() - d_dataset->inputGrid->minY()) *
            (d_dataset->inputGrid->maxY() - d_dataset->inputGrid->minY())));

  return 0;
}

long nmm_Microscope_Remote::InitializeDecoration (nmb_Decoration * dec) {
  d_decoration = dec;

  return 0;
}

long nmm_Microscope_Remote::InitializeTcl (const char * dir) {
  if (!dir)
    return -1;

  d_tcl_script_dir = new char [1 + strlen(dir)];
  if (!d_tcl_script_dir)
    return -1;

  strcpy(d_tcl_script_dir, dir);
  return 0;
}

long nmm_Microscope_Remote::Initialize ( ) {

  if (ReadMode() == READ_DEVICE) {
    // XXX Bug in VRPN. If we're already connected before we register the
    // handle_GotConnection handlers, they never get executed. So we'll call
    // them explicitly, if needed.
    if (d_connection->connected()) {
      vrpn_HANDLERPARAM p;
      p.buffer=NULL;
      handle_GotConnection2(this, p);
    } 
    // Register this callback here because it segfaults if I execute the handler
    // in the constructor, and I need to register at the same place it is
    // conditionally executed.
    d_connection->register_handler(d_GotConnection_type,
				   handle_GotConnection2,
				   this);
  } else if (ReadMode() == READ_STREAM) {
    if (!d_connection->get_File_Connection()) {
      fprintf(stderr,"nmm_Microscope_Remote::InitStream():  "
	      "could not open input log file %c\n", 0x08);
      return -1;
    }
    
  }  
// Initialization code common to both live and canned data
  // used for sweep mode
  // Moved to InitializeDataset, because this is also initialized
  // when we get the SetRegionC message
//    state.modify.region_diag =
//      sqrt(((d_dataset->inputGrid->maxX() - d_dataset->inputGrid->minX()) *
//            (d_dataset->inputGrid->maxX() - d_dataset->inputGrid->minX())) +
//           ((d_dataset->inputGrid->maxY() - d_dataset->inputGrid->minY()) *
//            (d_dataset->inputGrid->maxY() - d_dataset->inputGrid->minY())));

  return 0;
}



// These messages were only needed with the old microscope (STM),
// which had to take more samples per point when feeling to reduce
// the noise in the results.
/* OBSOLETE
long nmm_Microscope_Remote::SetSamples (const nmm_Microscope_Remote::SampleMode _mode) {

  switch (_mode) {
    case nmm_Microscope_Remote::Haptic:
      state.modify.std_dev_samples_cache = state.modify.std_dev_samples;
      state.modify.std_dev_samples = (int) (state.modify.std_dev_frequency
                                            * 0.001);
      break;
    case nmm_Microscope_Remote::Visual:
      state.modify.std_dev_samples = state.modify.std_dev_samples_cache;
      break;
  }

  CHECK(SetStdDevParams());

  return 0;
}
*/



// Tells the AFM to resume it's normal scan pattern in Image mode -
// presumably we were feeling or modifying the surface.
//   If 'value' is not NULL, we might do a scan of a small region before
//   resuming. (NOTE: this only available with the older AFM, not the new AFM)

long nmm_Microscope_Remote::ResumeScan (Point_value *,
                            BCPlane *) {
  CHECK(ImageMode());

  return ResumeWindowScan();
}




long nmm_Microscope_Remote::NewEpoch (void) {
  state.current_epoch++;
  return 0;
}


void nmm_Microscope_Remote::SetSampleMode (nmm_Sample * s) {
  s->setMicroscope(this);
  d_sampleAlgorithm = s;
}


long nmm_Microscope_Remote::ModifyMode (void) {

  CHECK(SetRateNM(state.modify.scan_rate_microns * 1000.0));

  CHECK(MarkModifyMode());  // Put this event in output log

  if ((state.modify.style != SEWING) && (state.modify.style != FORCECURVE)) {
     if (state.modify.control != DIRECTZ) {
	switch (state.modify.mode) {
	case TAPPING:
	   return EnterOscillatingMode(state.modify.p_gain, state.modify.i_gain,
                                state.modify.d_gain, state.modify.setpoint,
                                       state.modify.amplitude,
                                       state.modify.frequency,
                                       state.modify.input_gain,
                                       state.modify.ampl_or_phase,
                                       state.modify.drive_attenuation,
                                       state.modify.phase);
	case CONTACT:
	   return EnterContactMode(state.modify.p_gain, state.modify.i_gain,
                                state.modify.d_gain, state.modify.setpoint);
	default:
	   return 0;  // HACK HACK HACK
	}	
     } else { // Direct Z control
	return EnterDirectZControl(state.modify.max_z_step, 
				       state.modify.max_xy_step, 
				       state.modify.min_z_setpoint,
				       state.modify.max_z_setpoint, 
				       state.modify.max_lat_setpoint);
     }

  } else if (state.modify.style == SEWING) {
    if (state.modify.mode == TAPPING) {
      fprintf(stderr, "Oscillating while sewing is an impossible setting.\n");
      return 0;
    } else if (state.modify.control == DIRECTZ){
      fprintf(stderr, "DirectZ while sewing is an impossible setting.\n");
      return 0;
    } else {
      return EnterSewingStyle(state.modify.setpoint,
                              0.001 * state.modify.bot_delay,
                              0.001 * state.modify.top_delay,
                              state.modify.z_pull,
                              state.modify.punch_dist,
                              1000.0 * state.modify.speed,
                              state.modify.watchdog);
    }
  }
  else if (state.modify.style == FORCECURVE) {
    if (state.modify.control == DIRECTZ){
      fprintf(stderr, "DirectZ during forcecurve is an impossible setting.\n");
      return 0;
    } else {
    long numpnts = (long)(double)(state.modify.fc_num_points);
    long numcycles = (long)(double)(state.modify.fc_num_halfcycles);
    long avgnum = (long)(double)(state.modify.fc_avg_num);
       return EnterForceCurveStyle(state.modify.setpoint,
                                state.modify.fc_start_delay,
                                state.modify.fc_z_start,
                                state.modify.fc_z_end,
                                state.modify.fc_z_pullback,
                                state.modify.fc_force_limit,
                                state.modify.fc_movedist,
                                numpnts,
                                numcycles,
                                state.modify.fc_sample_speed,
                                state.modify.fc_pullback_speed,
                                state.modify.fc_start_speed,
                                state.modify.fc_feedback_speed,
				avgnum,
				state.modify.fc_sample_delay,
				state.modify.fc_pullback_delay,
				state.modify.fc_feedback_delay);
    }
  }
  
  return 0;	// HACK HACK HACK
}

long nmm_Microscope_Remote::ImageMode (void) {

  CHECK(SetRateNM(state.image.scan_rate_microns * 1000.0));

  CHECK(MarkImageMode());  // Put this event in output log

  switch (state.image.mode) {
    case TAPPING:
      return EnterOscillatingMode(state.image.p_gain,
                              state.image.i_gain,
                              state.image.d_gain,
                              state.image.setpoint,
                                  state.image.amplitude,
                                  state.image.frequency,
                                  state.image.input_gain,
                                  state.image.ampl_or_phase,
                                  state.image.drive_attenuation,
                                  state.image.phase);
    case CONTACT:
      return EnterContactMode(state.image.p_gain,
                              state.image.i_gain,
                              state.image.d_gain,
                              state.image.setpoint);
  }
  return 0;
}


long nmm_Microscope_Remote::GetNewPointDatasets
                           (const Tclvar_list_of_strings * channel_list,
                             Tclvar_int* active_list[], Tclvar_int* numsamples_list[])  {
  char * msgbuf = NULL;
  vrpn_int32 len;

  msgbuf = encode_GetNewPointDatasets(&len, channel_list, active_list, numsamples_list);
  if (!msgbuf)
    return -1;

  return dispatchMessage(len, msgbuf, d_GetNewPointDatasets_type);
}

long nmm_Microscope_Remote::GetNewScanDatasets
                           (const Tclvar_list_of_strings * channel_list,
                             Tclvar_int* active_list[])  {
  char * msgbuf = NULL;
  vrpn_int32 len;

  msgbuf = encode_GetNewScanDatasets(&len, channel_list, active_list);
  if (!msgbuf)
    return -1;

  return dispatchMessage(len, msgbuf, d_GetNewScanDatasets_type);
}

long nmm_Microscope_Remote::ResumeFullScan (void) {
  return SetScanWindow(0, 0,
                       d_dataset->inputGrid->numX() - 1,
                       d_dataset->inputGrid->numY() - 1);
}




long nmm_Microscope_Remote::ResumeWindowScan (void) {
  return dispatchMessage(0, NULL, d_ResumeWindowScan_type);
}

long nmm_Microscope_Remote::PauseScan (void) {
  return dispatchMessage(0, NULL, d_PauseScanning_type);
}

long nmm_Microscope_Remote::WithdrawTip (void) {
  return dispatchMessage(0, NULL, d_WithdrawTip_type);
}



long nmm_Microscope_Remote::rotateScanCoords (const double _x, const double _y,
					      const double _scanAngle, 
					      double * out_x, double * out_y) 
{

  // Rotate about the center of the scan region -- same as
  // the Thermo software rotates it's scan when we send it a 
  // particular scan angle. 
    double sin_angle = sinf(Q_DEG_TO_RAD(-_scanAngle));
    double cos_angle = cosf(Q_DEG_TO_RAD(-_scanAngle));

    double centerx = d_dataset->inputGrid->minX() +
    (d_dataset->inputGrid->maxX() - d_dataset->inputGrid->minX())/2.0 ;
    double centery = d_dataset->inputGrid->minY() + 
      (d_dataset->inputGrid->maxY() - d_dataset->inputGrid->minY())/2.0 ;

    double x = _x - centerx; // translate points to center
    double y = _y - centery;

    *out_x = ( cos_angle * x ) + ( -sin_angle * y );  // rotate
    *out_y = ( sin_angle * x ) + (  cos_angle * y );

    *out_x += centerx;  // translate points back
    *out_y += centery;  // translate points back
    
    return 0;
}

long nmm_Microscope_Remote::DrawLine (const double _startx, const double _starty,
                          const double _endx, const double _endy,
                          Point_value * _point, const vrpn_bool _awaitResult) {
  char * msgbuf = NULL;
  long len;
  long type;
  long retval;

  double startx, starty;
  rotateScanCoords(_startx, _starty, state.image.scan_angle, &startx, &starty);
  double endx, endy;
  rotateScanCoords(_endx, _endy, state.image.scan_angle, &endx, &endy);
  double yaw = state.modify.yaw - Q_DEG_TO_RAD(state.image.scan_angle);
  /*
  printf( "DrawLine ::  angle = %f xMin = %f xMax = %f yMin = %f yMax = %f\n",
	  state.image.scan_angle, 
	  d_dataset->inputGrid->minX(), d_dataset->inputGrid->maxX(),
	  d_dataset->inputGrid->minY(), d_dataset->inputGrid->maxY() );
  printf( "             startx = %f starty = %f rotated x = %f rotated y = %f\n",
	  _startx, _starty, startx, starty);
  printf( "             endx = %f endy = %f rotated x = %f rotated y = %f\n",
	  _endx, _endy, endx, endy);
  */
  switch (state.modify.style) {
    case SHARP:
    case SEWING:
    case FORCECURVE:
      msgbuf = encode_DrawSharpLine
                 (&len, startx, starty, endx, endy,
                  state.modify.step_size);
      type = d_DrawSharpLine_type;
      break;
    case SWEEP:
      msgbuf = encode_DrawSweepLine
	(&len, startx, starty,
	 yaw, state.modify.sweep_width,
	 endx, endy,
	 yaw, state.modify.sweep_width,
	 state.modify.step_size);
      type = d_DrawSweepLineCenter_type;
      break;
    default:
      return 0;
  }

  if (!msgbuf)
    return -1;

  retval = dispatchMessage(len, msgbuf, type);

  if (retval)
    return retval;

  if (_awaitResult && _point) {
    // Wait until the tip gets there.
    // We do this by kicking off the read loop until _point (which is
    //   an alias for the tip's current value) approaches <endx,endy>

    do {
      d_connection->mainloop();
      if ( ! d_connection->doing_okay()) {
        fprintf(stderr, "nmm_Microscope_Remote::DrawLine():  "
                        "can't read from microscope.\n");
        return -1;
      }
      // Tiger	added to get time elapsed since connected,
      //	used by Live and Replay mode
      getTimeSinceConnected();
      VERBOSE(5, "  Waiting for result in line mode");
    } while (!NMB_NEAR(_endx, _point->results()->x()) ||
             !NMB_NEAR(_endy, _point->results()->y()));
  }

  return retval;
}





long nmm_Microscope_Remote::DrawArc (const double _x, const double _y,
                         const double _startAngle, const double _endAngle,
                         Point_value * _point, const vrpn_bool _awaitResult) {
  char * msgbuf = NULL;
  long len;
  long retval;

  double x,y;
  rotateScanCoords(_x, _y, state.image.scan_angle, &x, &y);
  // Need to rotate start and end angle as well!
  double startAngle = _startAngle - Q_DEG_TO_RAD(state.image.scan_angle);
  double endAngle = _endAngle - Q_DEG_TO_RAD(state.image.scan_angle);

  switch (state.modify.style) {
    case SHARP:
    case SEWING:
    case FORCECURVE:
      return 0;
    case SWEEP:
      msgbuf = encode_DrawSweepArc
                 (&len, x, y, startAngle,
                  state.modify.sweep_width,
                  endAngle,
                  state.modify.sweep_width,
                  state.modify.step_size);
      if (!msgbuf)
        return -1;

      retval = dispatchMessage(len, msgbuf, d_DrawSweepArcCenter_type);

      break;
    default:
      return 0;
  }

  if (retval)
    return -1;

  if (_awaitResult && _point) {
    // Wait until the tip gets there.
    // We do this by kicking off the read loop until _point (which is
    //   an alias for the tip's current value) approaches <endx,endy>

    do {
      d_connection->mainloop();
      if ( ! d_connection->doing_okay()) {

        fprintf(stderr, "nmm_Microscope_Remote::DrawLine():  "
                        "can't read from microscope.\n");
        return -1;
      }
      // Tiger  added to get time elapsed since connected,
      //        used by Live and Replay mode
      getTimeSinceConnected();
      VERBOSE(5, "  Waiting for result in line mode");
    } while (!NMB_NEAR(_x, _point->results()->x()) ||
             !NMB_NEAR(_y, _point->results()->y()));
  }

  return retval;
}




long nmm_Microscope_Remote::ScanTo (const float _x, const float _y) {
  char * msgbuf;
  long len;

  double x,y;
  rotateScanCoords(_x, _y, state.image.scan_angle, &x, &y);

  msgbuf = encode_ScanTo(&len, x, y);
  if (!msgbuf)
    return -1;

  return dispatchMessage(len, msgbuf, d_ScanTo_type);
}

long nmm_Microscope_Remote::ScanTo (const float _x, const float _y, const float _z) {
  char * msgbuf;
  long len;

  double x,y;
  rotateScanCoords(_x, _y, state.image.scan_angle, &x, &y);

  msgbuf = encode_ScanTo(&len, x, y, _z);
  if (!msgbuf)
    return -1;

  return dispatchMessage(len, msgbuf, d_ScanToZ_type);
}

int nmm_Microscope_Remote::TakeSampleSet (float _x, float _y) {

  double x,y;
  rotateScanCoords(_x, _y, state.image.scan_angle, &x, &y);

  if (!d_sampleAlgorithm) {
    return -1;
  }

  d_sampleAlgorithm->sampleAt(x, y);
  return 0;
}


long nmm_Microscope_Remote::TakeFeelStep (const float _x, const float _y,
                              Point_value * _point,
                              const vrpn_bool _awaitResult) {

  // Don't rotate coords, because ScanTo/ZagTo do it. 
  CHECK(ScanTo(_x, _y));

  if (_awaitResult && _point) {
    // Wait until the tip moves.
    // We do this by kicking off the read loop until _point (which is
    //   an alias for the tip's current value) changes from the
    //   bogus value <-1, -1>

    _point->results()->setPosition(-1.0, -1.0);
    do {
      d_connection->mainloop();
      if ( ! d_connection->doing_okay()) {

        fprintf(stderr, "nmm_Microscope_Remote::TakeFeelStep():  "
                        "can't read from microscope.\n");
        return -1;
      }
      // Tiger  added to get time elapsed since connected,
      //        used by Live and Replay mode
      getTimeSinceConnected();
      VERBOSE(5, "  Waiting for result in line mode");
    } while (NMB_NEAR(-1.0, _point->results()->x()) &&
             NMB_NEAR(-1.0, _point->results()->y()));
  }

  return 0;
}




long nmm_Microscope_Remote::TakeModStep (const float _x, const float _y,
                             Point_value * _point,
                             const vrpn_bool _awaitResult) {

  // Don't rotate coords, because ScanTo/ZagTo do it. 
  switch (state.modify.style) {
    case SHARP:
    case SEWING:
    case FORCECURVE:
      CHECK(ScanTo(_x, _y));
      break;
    case SWEEP:
      CHECK(ZagTo(_x, _y, state.modify.yaw, state.modify.sweep_width,
                  state.modify.region_diag));
      break;
    default:
      return 0;
  }

  if (_awaitResult && _point) {
    // Wait until the tip moves to the destination.
    // We do this by kicking off the read loop until _point (which is
    //   an alias for the tip's current value) changes to the
    //   desired destination value

    do {
      d_connection->mainloop();
      if ( ! d_connection->doing_okay()) {

        fprintf(stderr, "nmm_Microscope_Remote::TakeModStep():  "
                        "can't read from microscope.\n");
        return -1;
      }
      // Tiger  added to get time elapsed since connected,
      //        used by Live and Replay mode
      getTimeSinceConnected();
      VERBOSE(5, "  Waiting for result in TakeModStep");
    } while (!NMB_NEAR(_x, _point->results()->x()) ||
             !NMB_NEAR(_y, _point->results()->y()));
  }

  return 0;
}

int nmm_Microscope_Remote::TakeDirectZStep (const float _x, const float _y, const float _z,
                             Point_value * _point,
                             const vrpn_bool _awaitResult) {

  // Don't rotate coords, because ScanTo rotates.
   CHECK(ScanTo(_x, _y, _z));

  if (_awaitResult && _point) {
    // Wait until the tip moves to the destination.
    // We do this by kicking off the read loop until _point (which is
    //   an alias for the tip's current value) changes to the
    //   desired destination value

    do {
      d_connection->mainloop();
      if ( ! d_connection->doing_okay()) {

        fprintf(stderr, "nmm_Microscope_Remote::TakeDirectZStep():  "
                        "can't read from microscope.\n");
        return -1;
      }
      // Tiger  added to get time elapsed since connected,
      //        used by Live and Replay mode
      getTimeSinceConnected();
      VERBOSE(5, "  Waiting for result in TakeDirectZStep()");
    } while (!NMB_NEAR(_x, _point->results()->x()) ||
             !NMB_NEAR(_y, _point->results()->y()) ||
             !NMB_NEAR(_z, _point->results()->z()));
  }
  return 0;
}




long nmm_Microscope_Remote::SetRegionNM (const float _minx, const float _miny,
                             const float _maxx, const float _maxy) {
  char * msgbuf;
  long len;

	// DEBUGGING	Tiger
  fprintf(stderr, "nmm_Microscope_Remote::SetRegionNM(): "
		   "minx = %g\t miny = %g\nmaxx = %g\t maxy = %g\n",
		   _minx, _miny, _maxx, _maxy);

  msgbuf = encode_SetRegionNM
             (&len, _minx, _miny, _maxx, _maxy);
  if (!msgbuf)
    return -1;

  return dispatchMessage(len, msgbuf, d_SetRegionNM_type);
}




// Figure out the correct scan mode for the STM based on the current
// desired features of the scan mode.
// The modes used here are defined in stm_cmd.h

long nmm_Microscope_Remote::SetScanStyle (void) {
  char * msgbuf;
  long len;
  long style;

  if (state.do_raster)
    if (state.do_y_fastest)
      style = RASTER_Y_FASTEST_POS;
    else
      style = RASTER_X_FASTEST_POS;
  else
    if (state.do_y_fastest)
      style = BOUST_Y_FASTEST;
    else
      style = BOUST_X_FASTEST;
  
  msgbuf = encode_SetScanStyle
             (&len, style);
  if (!msgbuf)
    return -1;

  return dispatchMessage(len, msgbuf, d_SetScanStyle_type);
}




long nmm_Microscope_Remote::SetScanWindow (const long _minx, const long _miny,
                               const long _maxx, const long _maxy) {
  char * msgbuf;
  long len;

  msgbuf = encode_SetScanWindow
             (&len, _minx, _miny, _maxx, _maxy);
  if (!msgbuf)
    return -1;

  return dispatchMessage(len, msgbuf, d_SetScanWindow_type);
}

long nmm_Microscope_Remote::SetRateNM (double rate) {
  char * msgbuf;
  long len;

  msgbuf = encode_SetRateNM(&len, rate);
  if (!msgbuf)
    return -1;

  return dispatchMessage(len, msgbuf, d_SetRateNM_type);
}

long nmm_Microscope_Remote::MarkModifyMode (void) {
  char * msgbuf = NULL;
  long len = 0;

  //return dispatchMessage(len, msgbuf, d_Echo_type);
	// HACK HACK HACK	added a new message type d_MarkModify_type
  return dispatchMessage(len, msgbuf, d_MarkModify_type);	// Tiger
}

long nmm_Microscope_Remote::MarkImageMode (void) {
  char * msgbuf = NULL;
  long len = 0;

  //return dispatchMessage(len, msgbuf, d_Echo_type);
        // HACK HACK HACK       added a new message type d_MarkImage_type
  return dispatchMessage(len, msgbuf, d_MarkImage_type);       // Tiger
}


long nmm_Microscope_Remote::EnterOscillatingMode
        (float p, float i, float d, float set, float amp,
         vrpn_float32 frequency, vrpn_int32 input_gain,
         vrpn_bool ampl_or_phase, vrpn_int32 drive_attenuation,
         vrpn_float32 phase) {
  char * msgbuf;
  long len;

  msgbuf = encode_EnterOscillatingMode(&len, p, i, d, set, amp,
                                       frequency, input_gain, ampl_or_phase,
                                       drive_attenuation, phase);
  //  msgbuf = encode_EnterTappingMode(&len, p, i, d, set, amp);           
  if (!msgbuf)
    return -1;

  return dispatchMessage(len, msgbuf, d_EnterOscillatingMode_type);
  //return dispatchMessage(len, msgbuf, d_EnterTappingMode_type);
}

long nmm_Microscope_Remote::EnterContactMode (float p, 
					      float i, float d, float set) {
  char * msgbuf;
  long len;

  msgbuf = encode_EnterContactMode(&len, p, i, d, set);
  if (!msgbuf)
    return -1;

  return dispatchMessage(len, msgbuf, d_EnterContactMode_type);
}

long nmm_Microscope_Remote::EnterDirectZControl (float _max_z_step, 
				       float _max_xy_step, 
				       float _min_setpoint, 
				       float _max_setpoint, 
				       float _max_lateral_force) {
  char * msgbuf;
  long len;	

  if (!strncmp(d_dataset->heightPlaneName->string(), "Z Piezo", 7)) {
      // We are not looking at Z Piezo as our height plane
      // XXX Directz doesn't work with anything but Z Piezo, yet.
      fprintf (stderr, "WARNING: EnterDirectZControl Height plane not Z Piezo, "
	       "Direct Z control will probably not work!\n");
  }
  msgbuf = encode_EnterDirectZControl(&len, _max_z_step, 
				      _max_xy_step, _min_setpoint, 
				      _max_setpoint, 
				      _max_lateral_force);
  if (!msgbuf)
    return -1;

  return dispatchMessage(len, msgbuf, d_EnterDirectZControl_type);
}

long nmm_Microscope_Remote::EnterSewingStyle (float set, 
	 float bot, float top, float zpull, float punch,
         float speed, float watchdog) {
  char * msgbuf;
  long len;

  msgbuf = encode_EnterSewingStyle(&len, set, bot, top, zpull, punch,
                                   speed, watchdog);
  if (!msgbuf)
    return -1;

  return dispatchMessage(len, msgbuf, d_EnterSewingStyle_type);
}

long nmm_Microscope_Remote::EnterForceCurveStyle
	(float setpnt, float startdelay, float zstart, float zend, 
	float zpull, float forcelimit, float movedist, long numpnts,
	long numhalfcycles, float samplespd, float pullspd, float startspd,
	float fdbackspd, long avgnum, float sampledel, float pulldel,
	float fdbackdel){

  char * msgbuf;
  long len;

  msgbuf = encode_EnterSpectroscopyMode(&len, setpnt, startdelay, zstart, 
		zend, zpull, forcelimit, movedist, numpnts, numhalfcycles,
		samplespd, pullspd, startspd, fdbackspd, avgnum,
		sampledel, pulldel, fdbackdel);
  if (!msgbuf)
    return -1;

  return dispatchMessage(len, msgbuf, d_EnterSpectroscopyMode_type);

  // _start_delay = delay at z_start (usec)
  // _z_start = distance at which to start acquiring (nm)
  // _z_end = distance at which to stop acquiring (nm)
  // _z_pullback = initial pull back distance (nm)
  // _forcelimit = maximum force at which to stop descent (nA)
  // _movedist = distance between force curves (nm)
  // _num_points = number of different z values to sample at
  // _num_halfcycles = number of down-up curves to acquire per pnt
  // _sample_speed = speed while sampling (um)
  // _pull_speed = speed while pulling back (um)
  // _start_speed = speed going to start pnt (um)
  // _fdback_speed = speed going to feedback pnt (um)
  // _avg_num = # of samples to average
  // _sample_delay = delay before sample (us)
  // _pull_delay = delay after pullback (us)
  // _fdback_delay = delay to establish feedback (us)
}

long nmm_Microscope_Remote::ZagTo
        (float _x, float _y, float yaw, float sweepWidth, float regionDiag) {
  char * msgbuf;
  long len;

  double x,y;
  rotateScanCoords(_x, _y, state.image.scan_angle, &x, &y);

  // Need to rotate yaw as well! Subtract the scan angle.
  msgbuf = encode_ZagTo(&len, x, y, yaw-Q_DEG_TO_RAD(state.image.scan_angle), sweepWidth, regionDiag);
  if (!msgbuf)
    return -1;

  return dispatchMessage(len, msgbuf, d_ZagToCenter_type);
}
/*
long nmm_Microscope_Remote::Shutdown (void) {
  return dispatchMessage(0, NULL, d_Shutdown_type);
}
*/
long nmm_Microscope_Remote::SetMaxMove (float distance) {
  char * msgbuf;
  long len;

  msgbuf = encode_SetMaxMove(&len, distance);
  if (!msgbuf)
    return -1;

  return dispatchMessage(len, msgbuf, d_SetMaxMove_type);
}

int nmm_Microscope_Remote::SetModForce (float newforce, float min, float max) {
  char * msgbuf;
  int len;

  msgbuf = encode_SetModForce(&len, newforce, min, max);
  if (!msgbuf)
    return -1;

  return dispatchMessage(len, msgbuf, d_SetModForce_type);
}

long nmm_Microscope_Remote::SetStdDelay (long delay) {
  char * msgbuf;
  long len;

  msgbuf = encode_SetStdDelay(&len, delay);
  if (!msgbuf)
    return -1;

  return dispatchMessage(len, msgbuf, d_SetStdDelay_type);
}

long nmm_Microscope_Remote::SetStPtDelay (long delay) {
  char * msgbuf;
  long len;

  msgbuf = encode_SetStPtDelay(&len, delay);
  if (!msgbuf)
    return -1;

  return dispatchMessage(len, msgbuf, d_SetStPtDelay_type);
}

long nmm_Microscope_Remote::SetRelax (long min, long sep) {
  char * msgbuf;
  long len;

  msgbuf = encode_SetRelax(&len, min, sep);
  if (!msgbuf)
    return -1;

  return dispatchMessage(len, msgbuf, d_SetRelax_type);
}





long nmm_Microscope_Remote::SetGridSize (const long _x, const long _y) {
  char * msgbuf;
  long len;

  msgbuf = encode_SetGridSize(&len, _x, _y);
  if (!msgbuf)
    return -1;

  return dispatchMessage(len, msgbuf, d_SetGridSize_type);
}

long nmm_Microscope_Remote::SetScanAngle (const float _angle) {
  char * msgbuf;
  long len;

  float angle = _angle;
  while (angle >= 360.0) {
      angle -= 360.0;
  }
  while (angle <= -360.0) {
      angle += 360.0;
  }
  
  float ang_radians = Q_DEG_TO_RAD(angle);

  printf("Setting scan angle %g\n", angle);

  msgbuf = encode_SetScanAngle(&len, ang_radians);
  if (!msgbuf)
    return -1;

  return dispatchMessage(len, msgbuf, d_SetScanAngle_type);
}




long nmm_Microscope_Remote::SetSlowScan (const long _value) {
  char * msgbuf;
  long len;
  
  // If state is identical, don't send message. 
  // XXX I found that this was commented out and when it was
  // commented out it was causing the function to get called when
  // the microscope reported back which was bad because then it just
  // kept on getting called resulting in the scan getting restarted
  // repeatedly - AAS - 
  // since this I have changed Tcl_Linkvar.C updateTcl() routines so they
  // don't do idempotent updates
  if (  state.slowScanEnabled == _value) {
//       fprintf(stderr, "nmm_MicRemote::SetSlowScan: Warning, variable is already"
//               " set to this value; you may experience infinite loop behavior\n");
     return 0;
  }

  msgbuf = encode_SetSlowScan(&len, _value);
  if (!msgbuf)
    return -1;

  return dispatchMessage(len, msgbuf, d_SetSlowScan_type);
}




long nmm_Microscope_Remote::SetModForce () {

  if ((state.modify.style != SEWING) && (state.modify.style != FORCECURVE)) {
    switch (state.modify.mode) {
      case TAPPING:
	return EnterOscillatingMode(state.modify.p_gain, state.modify.i_gain,
				state.modify.d_gain, state.modify.setpoint,
                                    state.modify.amplitude,
                                       state.modify.frequency,
                                       state.modify.input_gain,
                                       state.modify.ampl_or_phase,
                                       state.modify.drive_attenuation,
                                       state.modify.phase);

      case CONTACT:
        return EnterContactMode(state.modify.p_gain, state.modify.i_gain,
                                   state.modify.d_gain, state.modify.setpoint);
      default:
        return 0;  // HACK HACK HACK
    }
  } else if (state.modify.style == SEWING) {
    if (state.modify.mode == TAPPING) {
      fprintf(stderr, "Oscillating while sewing is an impossible setting.\n");
      return 0;
    } else {
      return EnterSewingStyle(state.modify.setpoint,
                              0.001 * state.modify.bot_delay,
                              0.001 * state.modify.top_delay,
                              state.modify.z_pull,
                              state.modify.punch_dist,
                              1000.0 * state.modify.speed,
                              state.modify.watchdog);
    }
  }
  else if (state.modify.style == FORCECURVE) {
    switch (state.modify.mode) {
      case TAPPING:
        if (EnterOscillatingMode(state.modify.p_gain, state.modify.i_gain,
                                state.modify.d_gain, state.modify.setpoint,
                                    state.modify.amplitude,
                                       state.modify.frequency,
                                       state.modify.input_gain,
                                       state.modify.ampl_or_phase,
                                       state.modify.drive_attenuation,
                                       state.modify.phase)) {
            fprintf(stderr, "Error, can't enter oscillating mode\n");
            return -1;
	}
         
        break;
      case CONTACT:
        if (EnterContactMode(state.modify.p_gain, state.modify.i_gain,
                                   state.modify.d_gain, state.modify.setpoint)){
	    fprintf(stderr, "Error, can't enter contact mode\n");
            return -1;
	}
        break;
      default:
        fprintf(stderr, "Error, unknown modify mode"
                           " (not contact or tapping)\n");
        return 0;
    }
    long numpnts = (long)(double)(state.modify.fc_num_points);
    long numcycles = (long)(double)(state.modify.fc_num_halfcycles);
    long avgnum = (long)(double)(state.modify.fc_avg_num);
    return EnterForceCurveStyle(state.modify.setpoint,
                                state.modify.fc_start_delay,
                                state.modify.fc_z_start,
                                state.modify.fc_z_end,
                                state.modify.fc_z_pullback,
                                state.modify.fc_force_limit,
                                state.modify.fc_movedist,
                                numpnts,
                                numcycles,
                                state.modify.fc_sample_speed,
                                state.modify.fc_pullback_speed,
                                state.modify.fc_start_speed,
                                state.modify.fc_feedback_speed,
                                avgnum,
                                state.modify.fc_sample_delay,
                                state.modify.fc_pullback_delay,
                                state.modify.fc_feedback_delay);
  } else {
    fprintf(stderr,"shouldn't be here\n");
    return 0;
  }
}


  
long nmm_Microscope_Remote::SetBias (const float _bias) {
  char * msgbuf;
  long len;

  msgbuf = encode_SetBias(&len, _bias);
  if (!msgbuf)
    return -1;

  return dispatchMessage(len, msgbuf, d_SetBias_type);
}




long nmm_Microscope_Remote::SetPulsePeak (const float _height) {
  char * msgbuf;
  long len;

  msgbuf = encode_SetPulsePeak(&len, _height);
  if (!msgbuf)
    return -1;

  return dispatchMessage(len, msgbuf, d_SetPulsePeak_type);
}




long nmm_Microscope_Remote::SetPulseDuration (const float _width) {
  char * msgbuf;
  long len;

  msgbuf = encode_SetPulseDuration(&len, _width);
  if (!msgbuf)
    return -1;

  return dispatchMessage(len, msgbuf, d_SetPulseDuration_type);
}




long nmm_Microscope_Remote::SetOhmmeterSampleRate (const long _rate) {
  char * msgbuf;
  long len;

  // HACK:  hardwired to ohmmeter #0
  msgbuf = encode_SetOhmmeterSampleRate(&len, 0, _rate);
  if (!msgbuf)
    return -1;

  return dispatchMessage(len, msgbuf, d_SetOhmmeterSampleRate_type);
}




long nmm_Microscope_Remote::EnableVoltsource (long _which,
                                             float _voltage) {
  char * msgbuf;
  long len;

  msgbuf = encode_EnableVoltsource(&len, _which, _voltage);
  if (!msgbuf)
    return -1;

  return dispatchMessage(len, msgbuf, d_EnableVoltsource_type);
}




long nmm_Microscope_Remote::EnableAmp (long _which, float _offset,
                           float _uncalOffset, long _gain) {
  char * msgbuf;
  long len;

  msgbuf = encode_EnableAmp
             (&len, _which, _offset, _uncalOffset, _gain);
  if (!msgbuf)
    return -1;

  return dispatchMessage(len, msgbuf, d_EnableAmp_type);
}




long nmm_Microscope_Remote::DisableAmp (long _which) {
  char * msgbuf;
  long len;

  msgbuf = encode_DisableAmp(&len, _which);
  if (!msgbuf)
    return -1;
  
  return dispatchMessage(len, msgbuf, d_DisableAmp_type);
}




long nmm_Microscope_Remote::DisableVoltsource (long _which) {
  char * msgbuf;
  long len;

  msgbuf = encode_DisableVoltsource(&len, _which);
  if (!msgbuf)
    return -1;

  return dispatchMessage(len, msgbuf, d_DisableVoltsource_type);
}

// HACK to get ohmmeter data into point results
static float lastResistanceReceived = 0;
// END HACK

long nmm_Microscope_Remote::RecordResistance
        (long meter, struct timeval t, float res,
         float v, float r, float f) {
  char * msgbuf;
  long len;

  msgbuf = encode_RecordResistance(&len, meter, t, res, v, r, f);
  if (!msgbuf)
    return -1;

  // HACK - this is a temporary fix to get resistance values into the modfile
  lastResistanceReceived = res;

  return dispatchMessage(len, msgbuf, d_RecordResistance_type);
}

int nmm_Microscope_Remote::getTimeSinceConnected(void) {

  struct timeval elapsedTime;

  switch (ReadMode()) {
    case READ_DEVICE:
      d_connection->time_since_connection_open(&elapsedTime);
      d_decoration->elapsedTime = elapsedTime.tv_sec;
      break;
    case READ_STREAM:
      vrpn_File_Connection * logFile;
      logFile = (vrpn_File_Connection *)(d_connection->get_File_Connection());
      logFile->time_since_connection_open(&elapsedTime);
      d_decoration->elapsedTime = elapsedTime.tv_sec;
      break;
  }

  return 0;
}

long nmm_Microscope_Remote::EnterScanlineMode(){
 
  char * msgbuf;
  long len;

  msgbuf = encode_EnterScanlineMode(&len, 1);
  if (!msgbuf)
    return -1;
  return dispatchMessage(len, msgbuf, d_EnterScanlineMode_type);
}

long nmm_Microscope_Remote::ExitScanlineMode(){
 
  char * msgbuf;
  long len;

  msgbuf = encode_EnterScanlineMode(&len, 0);
  if (!msgbuf)
    return -1;
  return dispatchMessage(len, msgbuf, d_EnterScanlineMode_type);
}

long nmm_Microscope_Remote::AcquireScanline(){
  printf("requesting scan line - fix this: function not atomic on server!\n");
  BCPlane *p = d_dataset->inputGrid->getPlaneByName(
		d_dataset->heightPlaneName->string());
  if (!p) {
	fprintf(stderr, "nmm_Microscope_Remote::AcquireScanline:"
                 " Error, could not get height plane\n");
	return -1;
  }

  float x, y, z, angle, slope = 0;

  // get the starting location of the scan line in nm
  state.scanline.getStartPoint(p, &x, &y, &z);
  angle = state.scanline.angle*M_PI/180.0;
  slope = state.scanline.slope_nm_per_micron;

  if (state.acquisitionMode != SCANLINE)
      if (EnterScanlineMode()) return -1;
  
  char * msgbuf;
  long len;

  msgbuf = encode_RequestScanLine(&len, x, y, z, angle, slope, 
	state.scanline.width, state.scanline.resolution, 
	state.scanline.feedback_enabled, state.scanline.forcelimit_enabled,
	state.scanline.forcelimit, state.scanline.max_z_step,
	state.scanline.max_xy_step);
  if (!msgbuf)
    return -1;
  state.scanline.num_scanlines_to_receive++;
  return dispatchMessage(len, msgbuf, d_RequestScanLine_type);
}

long nmm_Microscope_Remote::SetScanlineModeParameters(){
 
  CHECK(SetRateNM(state.scanline.scan_rate_microns_per_sec * 1000.0));

  switch(state.scanline.mode) {
    case TAPPING:
	return EnterOscillatingMode(state.scanline.p_gain, state.scanline.i_gain,
			state.scanline.d_gain, state.scanline.setpoint,
                                       state.scanline.amplitude,
                                       state.scanline.frequency,
                                       state.scanline.input_gain,
                                       state.scanline.ampl_or_phase,
                                       state.scanline.drive_attenuation,
                                       state.scanline.phase);
    case CONTACT:
	return EnterContactMode(state.scanline.p_gain, state.scanline.i_gain,
			state.scanline.d_gain, state.scanline.setpoint);
    default:
	return 0;
  }
}

long nmm_Microscope_Remote::JumpToScanLine(long line)
{
  char * msgbuf;
  long len;
 
  msgbuf = encode_JumpToScanLine(&len, line);

  if (!msgbuf)
    return -1;

  return dispatchMessage(len, msgbuf, d_JumpToScanLine_type);
}

/* OBSOLETE
long nmm_Microscope_Remote::SetStdDevParams (const long _samples,
                                            const float _freq) {
  state.modify.std_dev_samples = _samples;
  state.modify.std_dev_frequency = _freq;
  return SetStdDevParams();
}

long nmm_Microscope_Remote::SetStdDevParams (void) {
  char * msgbuf;
  long len;

  msgbuf = encode_SetStdDevParams(&len, 
                  state.modify.std_dev_samples,
                  state.modify.std_dev_frequency);
  if (!msgbuf)
    return -1;

  return dispatchMessage(len, msgbuf, d_SetStdDevParams_type);
}
*/





long nmm_Microscope_Remote::QueryScanRange (void) {
  return dispatchMessage(0, NULL, d_QueryScanRange_type);
}



/* OBSOLETE
long nmm_Microscope_Remote::QueryStdDevParams (void) {
  return dispatchMessage(0, NULL, d_QueryStdDevParams_type);
}
*/



long nmm_Microscope_Remote::QueryPulseParams (void) {
  return dispatchMessage(0, NULL, d_QueryPulseParams_type);
}






int nmm_Microscope_Remote::ReadMode()
{
    return state.read_mode;
}

void nmm_Microscope_Remote::ReadMode(int rm)
{
    if ((rm == READ_FILE) ||(rm == READ_DEVICE) ||(rm == READ_STREAM) ) {
        state.read_mode = rm;
        fprintf(stderr, "nmm_Microscope_Remote Read Mode %d.\n", rm);
    } else {
        fprintf(stderr, "nmm_Microscope_Remote Invalid Read Mode.\n");
    }
}







void nmm_Microscope_Remote::ResetClock (void) {
  gettimeofday(&d_nowtime, &d_nowzone);
  d_next_time.tv_sec = 0L;
  d_next_time.tv_usec = 0L;
}


long nmm_Microscope_Remote::registerPointDataHandler
                          (int (* handler) (void *, const Point_results *),
                           void * userdata) {
  pointDataHandlerEntry * newEntry;

  newEntry = new pointDataHandlerEntry;
  if (!newEntry) {
    fprintf(stderr, "nmm_Microscope_Remote::registerPointDataHandler:  "
                    "Out of memory.\n");
    return -1;
  }

  newEntry->handler = handler;
  newEntry->userdata = userdata;
  newEntry->next = d_pointDataHandlers;

  d_pointDataHandlers = newEntry;

  return 0;
}

long nmm_Microscope_Remote::unregisterPointDataHandler
                          (int (* handler) (void *, const Point_results *),
                           void * userdata) {
  pointDataHandlerEntry * victim, ** snitch;

  snitch = &d_pointDataHandlers;
  victim = *snitch;
  while (victim &&
         (victim->handler != handler) &&
         (victim->userdata != userdata)) {
    snitch = &((*snitch)->next);
    victim = *snitch;
  }

  if (!victim) {
    fprintf(stderr, "nmm_Microscope_Remote::unregisterPointDataHandler:  "
                    "No such handler.\n");
    return -1;
  }

  *snitch = victim->next;
  delete victim;

  return 0;
}

long nmm_Microscope_Remote::registerModifyModeHandler
                          (int (* handler) (void *),
                           void * userdata) {
  modeHandlerEntry * newEntry;

  newEntry = new modeHandlerEntry;
  if (!newEntry) {
    fprintf(stderr, "nmm_Microscope_Remote::registerModifyModeHandler:  "
                    "Out of memory.\n");
    return -1;
  }

  newEntry->handler = handler;
  newEntry->userdata = userdata;
  newEntry->next = d_modifyModeHandlers;

  d_modifyModeHandlers = newEntry;

  return 0;
}

long nmm_Microscope_Remote::unregisterModifyModeHandler
                          (int (* handler) (void *),
                           void * userdata) {
  modeHandlerEntry * victim, ** snitch;

  snitch = &d_modifyModeHandlers;
  victim = *snitch;
  while (victim &&
         (victim->handler != handler) &&
         (victim->userdata != userdata)) {
    snitch = &((*snitch)->next);
    victim = *snitch;
  }

  if (!victim) {
    fprintf(stderr, "nmm_Microscope_Remote::unregisterModifyModeHandler:  "
                    "No such handler.\n");
    return -1;
  }

  *snitch = victim->next;
  delete victim;

  return 0;
}

long nmm_Microscope_Remote::registerImageModeHandler
                          (int (* handler) (void *),
                           void * userdata) {
  modeHandlerEntry * newEntry;

  newEntry = new modeHandlerEntry;
  if (!newEntry) {
    fprintf(stderr, "nmm_Microscope_Remote::registerImageModeHandler:  "
                    "Out of memory.\n");
    return -1;
  }

  newEntry->handler = handler;
  newEntry->userdata = userdata;
  newEntry->next = d_imageModeHandlers;

  d_imageModeHandlers = newEntry;

  return 0;
}

long nmm_Microscope_Remote::unregisterImageModeHandler
                          (int (* handler) (void *),
                           void * userdata) {
  modeHandlerEntry * victim, ** snitch;

  snitch = &d_imageModeHandlers;
  victim = *snitch;
  while (victim &&
         (victim->handler != handler) &&
         (victim->userdata != userdata)) {
    snitch = &((*snitch)->next);
    victim = *snitch;
  }

  if (!victim) {
    fprintf(stderr, "nmm_Microscope_Remote::unregisterImageModeHandler:  "
                    "No such handler.\n");
    return -1;
  }

  *snitch = victim->next;
  delete victim;

  return 0;
}


long nmm_Microscope_Remote::registerScanlineModeHandler
                          (int (* handler) (void *),
                           void * userdata) {
  modeHandlerEntry * newEntry;

  newEntry = new modeHandlerEntry;
  if (!newEntry) {
    fprintf(stderr, "nmm_Microscope_Remote::registerScanlineModeHandler:  "
                    "Out of memory.\n");
    return -1;
  }

  newEntry->handler = handler;
  newEntry->userdata = userdata;
  newEntry->next = d_scanlineModeHandlers;

  d_scanlineModeHandlers = newEntry;

  return 0;
}

long nmm_Microscope_Remote::unregisterScanlineModeHandler
                          (int (* handler) (void *),
                           void * userdata) {
  modeHandlerEntry * victim, ** snitch;

  snitch = &d_scanlineModeHandlers;
  victim = *snitch;
  while (victim &&
         (victim->handler != handler) &&
         (victim->userdata != userdata)) {
    snitch = &((*snitch)->next);
    victim = *snitch;
  }

  if (!victim) {
    fprintf(stderr, "nmm_Microscope_Remote::unregisterScanlineModeHandler:  "
                    "No such handler.\n");
    return -1;
  }

  *snitch = victim->next;
  delete victim;

  return 0;
}

long nmm_Microscope_Remote::registerScanlineDataHandler
                          (int (* handler) (void *, const Scanline_results *),
                           void * userdata) {
  scanlineDataHandlerEntry * newEntry;

  newEntry = new scanlineDataHandlerEntry;
  if (!newEntry) {
    fprintf(stderr, "nmm_Microscope_Remote::registerScanlineDataHandler:  "
                    "Out of memory.\n");
    return -1;
  }

  newEntry->handler = handler;
  newEntry->userdata = userdata;
  newEntry->next = d_scanlineDataHandlers;

  d_scanlineDataHandlers = newEntry;

  return 0;
}

long nmm_Microscope_Remote::unregisterScanlineDataHandler
                          (int (* handler) (void *, const Scanline_results *),
                           void * userdata) {
  scanlineDataHandlerEntry * victim, ** snitch;

  snitch = &d_scanlineDataHandlers;
  victim = *snitch;
  while (victim &&
         (victim->handler != handler) &&
         (victim->userdata != userdata)) {
    snitch = &((*snitch)->next);
    victim = *snitch;
  }

  if (!victim) {
    fprintf(stderr, "nmm_Microscope_Remote::unregisterScanlineDataHandler:  "
                    "No such handler.\n");
    return -1;
  }

  *snitch = victim->next;
  delete victim;

  return 0;
}

vrpn_int32 nmm_Microscope_Remote::pointResultType (void) const {
  return d_PointResultData_type;
}

void nmm_Microscope_Remote::accumulatePointResults (vrpn_bool on) {
  d_accumulatePointResults = on;
}



// Common code from by RcvPointResultNM and RcvResultNM, and,
// with a little stretching (_z and _checkZ), RcvResultData

void nmm_Microscope_Remote::DisplayModResult (const float _x, const float _y,
                                   const float _height,
                                   const Point_value * _z,
                                   const vrpn_bool _checkZ) {
  PointType top, bottom;
  //double frac_x, frac_y;
  double fx, fy;
  long x, y;
  BCPlane * heightPlane;

  double xr,yr;
  rotateScanCoords(_x, _y, -state.image.scan_angle, &xr, &yr);

  heightPlane = d_dataset->inputGrid->getPlaneByName
            (d_dataset->heightPlaneName->string());

  top[0] = bottom[0] = xr;
  top[1] = bottom[1] = yr;

  d_dataset->inputGrid->worldToGrid((double)xr, (double)yr, fx, fy);
  x = (long)fx;
  y = (long)fy;

  // drives X output
  state.rasterX = x;
  state.rasterY = y;

  // modification markers
  if (state.acquisitionMode == MODIFY) {
    if (((unsigned) x < (unsigned) d_dataset->inputGrid->numX()) &&
        ((unsigned) y < (unsigned) d_dataset->inputGrid->numY())) {
      top[2] = heightPlane->scaledValue(x, y);

      if (_checkZ) {
        if (_z)
          bottom[2] = _z->value() * heightPlane->scale();
        else
          bottom[2] = top[2];
      } else {
        bottom[2] = _height * heightPlane->scale();
      }
      //if (glenable)
      d_decoration->addScrapeMark(top, bottom);

    }
  }

  // track hand for feel or mod (canned data)
  if (state.cannedLineVisible) {
    top[2] = heightPlane->maxAttainableValue() * heightPlane->scale();
    bottom[2] = heightPlane->minAttainableValue() * heightPlane->scale();
    state.cannedLineToggle = !state.cannedLineToggle;
  }

  // BUG BUG BUG
  // nothing gets done with top and bottom?
}



void nmm_Microscope_Remote::GetRasterPosition (const long _x, const long _y) {
  // drives X output
  state.rasterX = _x;
  state.rasterY = _y;
}



//static
int nmm_Microscope_Remote::handle_GotConnection2 (void * userdata,
                                      vrpn_HANDLERPARAM ) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
  return (ms->RcvGotConnection2());
}

//static
int nmm_Microscope_Remote::handle_DroppedConnection2 (void * userdata,
                                      vrpn_HANDLERPARAM ) {
    nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
    // Only display warning if we aren't quitting the program, and
    // if we are connected to a live AFM - no streamfiles!
    if ((!ms->d_dataset->done) && (ms->ReadMode() == READ_DEVICE)) {
      display_warning_dialog("Communication with ThermoMicroscopes AFM has stopped.\n"
                             "No more data will be collected until ThermoMicroscopes\n"
                             "software is re-started and communication is re-established.");
    }
    return 0;
}

//static
int nmm_Microscope_Remote::handle_InTappingMode (void * userdata,
                                      vrpn_HANDLERPARAM param) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
  float p, i, d, setpoint, amplitude;

  ms->decode_InTappingMode(&param.buffer, &p, &i, &d, &setpoint, &amplitude);
  ms->RcvInTappingMode(p, i, d, setpoint, amplitude);

  return 0;
}

//static
int nmm_Microscope_Remote::handle_InOscillatingMode (void * userdata,
                                      vrpn_HANDLERPARAM param) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
  float p, i, d, setpoint, amplitude;
  vrpn_float32 frequency; 
  vrpn_int32 input_gain;
  vrpn_bool ampl_or_phase; 
  vrpn_int32 drive_attenuation;
  vrpn_float32 phase;

  ms->decode_InOscillatingMode(&param.buffer, &p, &i, &d, &setpoint, 
                               &amplitude, &frequency, 
                               &input_gain, &ampl_or_phase,
                               &drive_attenuation, &phase);
  ms->RcvInOscillatingMode(p, i, d, setpoint, amplitude, frequency, 
                           input_gain, ampl_or_phase,
                           drive_attenuation, phase);

  return 0;
}

//static
int nmm_Microscope_Remote::handle_InContactMode (void * userdata, 
                                             vrpn_HANDLERPARAM param) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
  float p, i, d, setpoint;

  ms->decode_InContactMode(&param.buffer, &p, &i, &d, &setpoint);
  ms->RcvInContactMode(p, i, d, setpoint);

  return 0;
}

//static
int nmm_Microscope_Remote::handle_InDirectZControl (void * userdata,
                                             vrpn_HANDLERPARAM param) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
  float max_z_step, max_xy_step, min_setpoint, max_setpoint, 
     max_lateral_force, freespace_normal_force,freespace_lat_force ;

  ms->decode_InDirectZControl(&param.buffer, &max_z_step, &max_xy_step, 
				       &min_setpoint, &max_setpoint, 
				       &max_lateral_force,
			      &freespace_normal_force, &freespace_lat_force);
  ms->RcvInDirectZControl(max_z_step, max_xy_step, 
				       min_setpoint, max_setpoint, 
				       max_lateral_force,
			      freespace_normal_force, freespace_lat_force);

  return 0;
}

//static
int nmm_Microscope_Remote::handle_InSewingStyle (void * userdata,
                                             vrpn_HANDLERPARAM param) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
  float setpoint, bottomDelay, topDelay, pullBackDistance, moveDistance,
        speed, maximumPunchDistance;

  ms->decode_InSewingStyle(&param.buffer, &setpoint, &bottomDelay, &topDelay,
                          &pullBackDistance, &moveDistance,
                          &speed, &maximumPunchDistance);
  ms->RcvInSewingStyle(setpoint, bottomDelay, topDelay, pullBackDistance,
                      moveDistance, speed, maximumPunchDistance);

  return 0;
}

//static
int nmm_Microscope_Remote::handle_InSpectroscopyMode (void * userdata,
						vrpn_HANDLERPARAM param) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
  float setpoint;
  float startDelay, zStart, zEnd, zPullback, forceLimit, distBetweenFC;
  vrpn_int32 numPoints, numHalfcycles;        // Tiger changed int to long
  float sampleSpeed, pullbackSpeed, startSpeed, feedbackSpeed;
  vrpn_int32 avgNum;  // Tiger changed int to long
  float sampleDelay, pullbackDelay, feedbackDelay;

  ms->decode_InSpectroscopyMode(&param.buffer, &setpoint,
        &startDelay, &zStart, &zEnd, &zPullback, &forceLimit, &distBetweenFC,
        &numPoints, &numHalfcycles,
        &sampleSpeed, &pullbackSpeed, &startSpeed, &feedbackSpeed,
        &avgNum, &sampleDelay, &pullbackDelay, &feedbackDelay);
  ms->RcvInSpectroscopyMode(setpoint, startDelay, zStart, zEnd,
        zPullback, forceLimit, distBetweenFC, numPoints, numHalfcycles,
        sampleSpeed, pullbackSpeed, startSpeed, feedbackSpeed,
        avgNum, sampleDelay, pullbackDelay, feedbackDelay);

  return 0;
}

//static
int nmm_Microscope_Remote::handle_ForceParameters (void * userdata,
                                             vrpn_HANDLERPARAM param) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
  vrpn_int32 modifyEnable;
  float scrap;

  ms->decode_ForceParameters(&param.buffer, &modifyEnable, &scrap);
  ms->RcvForceParameters(modifyEnable, scrap);

  return 0;
}

//static
int nmm_Microscope_Remote::handle_BaseModParameters (void * userdata,
                                              vrpn_HANDLERPARAM param) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
  float min, max;

  ms->decode_BaseModParameters(&param.buffer, &min, &max);
  ms->RcvBaseModParameters(min, max);

  return 0;
}

//static
int nmm_Microscope_Remote::handle_ForceSettings (void * userdata,
                                             vrpn_HANDLERPARAM param) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
  float min, max, setpoint;

  ms->decode_ForceSettings(&param.buffer, &min, &max, &setpoint);
  ms->RcvForceSettings(min, max, setpoint);

  return 0;
}

//static
int nmm_Microscope_Remote::handle_VoltsourceEnabled (void * userdata,
                                              vrpn_HANDLERPARAM param) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
  vrpn_int32 which;
  float voltage;

  ms->decode_VoltsourceEnabled(&param.buffer, &which, &voltage);
  ms->RcvVoltsourceEnabled(which, voltage);

  return 0;
}

//static
int nmm_Microscope_Remote::handle_VoltsourceDisabled (void * userdata,
                                              vrpn_HANDLERPARAM param) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
  vrpn_int32 which;

  ms->decode_VoltsourceDisabled(&param.buffer, &which);
  ms->RcvVoltsourceDisabled(which);

  return 0;
}

//static
int nmm_Microscope_Remote::handle_AmpEnabled (void * userdata,
                           vrpn_HANDLERPARAM param) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
  vrpn_int32 which, gain;
  float offset, percentOffset;

  ms->decode_AmpEnabled(&param.buffer, &which, &offset, &percentOffset, &gain);
  ms->RcvAmpEnabled(which, offset, percentOffset, gain);

  return 0;
}

//static
int nmm_Microscope_Remote::handle_AmpDisabled (void * userdata,
                           vrpn_HANDLERPARAM param) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
  vrpn_int32 which;

  ms->decode_AmpDisabled(&param.buffer, &which);
  ms->RcvAmpDisabled(which);

  return 0;
}

//static
int nmm_Microscope_Remote::handle_SuspendCommands (void * userdata,
                                             vrpn_HANDLERPARAM param) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;

  ms->RcvSuspendCommands();

  return 0;
}
//static
int nmm_Microscope_Remote::handle_ResumeCommands (void * userdata,
                                             vrpn_HANDLERPARAM param) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;

  ms->RcvResumeCommands();

  return 0;
}

//static
int nmm_Microscope_Remote::handle_StartingToRelax (void * userdata,
                                             vrpn_HANDLERPARAM param) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
  vrpn_int32 sec, usec;

  ms->decode_StartingToRelax(&param.buffer, &sec, &usec);
  ms->RcvStartingToRelax(sec, usec);

  return 0;
}

//static
int nmm_Microscope_Remote::handle_InModModeT (void * userdata,
                           vrpn_HANDLERPARAM param) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
  vrpn_int32 sec, usec;

  ms->decode_InModModeT(&param.buffer, &sec, &usec);
  ms->RcvInModModeT(sec, usec);

  return 0;
}

//static
int nmm_Microscope_Remote::handle_InModMode
                          (void * userdata,
                           vrpn_HANDLERPARAM param) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
  ms->RcvInModMode();

  return 0;
}

//static
int nmm_Microscope_Remote::handle_InImgModeT (void * userdata,
                           vrpn_HANDLERPARAM param) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
  vrpn_int32 sec, usec;

  ms->decode_InImgModeT(&param.buffer, &sec, &usec);
  ms->RcvInImgModeT(sec, usec);

  return 0;
}

//static
int nmm_Microscope_Remote::handle_InImgMode
                          (void * userdata,
                           vrpn_HANDLERPARAM param) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
  ms->RcvInImgMode();

  return 0;
}

//static
int nmm_Microscope_Remote::handle_ModForceSet (void * userdata,
                           vrpn_HANDLERPARAM param) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
  float force;

  ms->decode_ModForceSet(&param.buffer, &force);
  ms->RcvModForceSet(force);

  return 0;
}

//static
int nmm_Microscope_Remote::handle_ImgForceSet (void * userdata,
                           vrpn_HANDLERPARAM param) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
  float force;

  ms->decode_ImgForceSet(&param.buffer, &force);
  ms->RcvImgForceSet(force);

  return 0;
}

//static
int nmm_Microscope_Remote::handle_ModSet (void * userdata,
                           vrpn_HANDLERPARAM param) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
  vrpn_int32 modifyEnable;
  float max, min, value;

  ms->decode_ModSet(&param.buffer, &modifyEnable, &max, &min, &value);
  ms->RcvModSet(modifyEnable, max, min, value);

  return 0;
}

//static
int nmm_Microscope_Remote::handle_ImgSet (void * userdata,
                           vrpn_HANDLERPARAM param) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
  vrpn_int32 modifyEnable;
  float max, min, value;

  ms->decode_ImgSet(&param.buffer, &modifyEnable, &max, &min, &value);
  ms->RcvImgSet(modifyEnable, max, min, value);

  return 0;
}

//static
int nmm_Microscope_Remote::handle_RelaxSet (void * userdata,
                           vrpn_HANDLERPARAM param) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
  vrpn_int32 min, sep;

  ms->decode_RelaxSet(&param.buffer, &min, &sep);
  ms->RcvRelaxSet(min, sep);

  return 0;
}

//static
int nmm_Microscope_Remote::handle_ForceSet (void * userdata,
                           vrpn_HANDLERPARAM param) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
  float force;

  ms->decode_ForceSet(&param.buffer, &force);
  ms->RcvForceSet(force);

  return 0;
}

//static
int nmm_Microscope_Remote::handle_ForceSetFailure (void * userdata,
                                             vrpn_HANDLERPARAM param) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
  float force;

  ms->decode_ForceSetFailure(&param.buffer, &force);
  ms->RcvForceSetFailure(force);

  return 0;
}

//static
int nmm_Microscope_Remote::handle_PulseParameters (void * userdata,
                                             vrpn_HANDLERPARAM param) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
  vrpn_int32 pulseEnabled;
  float biasVoltage, peakVoltage, width;

  ms->decode_PulseParameters(&param.buffer, &pulseEnabled, &biasVoltage,
                             &peakVoltage, &width);
  ms->RcvPulseParameters(pulseEnabled, biasVoltage, peakVoltage, width);

  return 0;
}

/* OBSOLETE
//static
int nmm_Microscope_Remote::handle_StdDevParameters (void * userdata,
                                              vrpn_HANDLERPARAM param) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
  vrpn_int32 samples;
  float freq;

  ms->decode_StdDevParameters(&param.buffer, &samples, &freq);
  ms->RcvStdDevParameters(samples, freq);

  return 0;
}
*/

//static
int nmm_Microscope_Remote::handle_WindowLineData (void * userdata,
                                             vrpn_HANDLERPARAM param) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
  float fields [MAX_CHANNELS];
  vrpn_int32 x, y, dx, dy, sec, usec, lineCount, fieldCount;
  long i;

  ms->decode_WindowLineDataHeader(&param.buffer, &x, &y, &dx, &dy,
                                  &lineCount, &fieldCount, &sec, &usec);


  for (i = 0; i < lineCount; i++) {
    ms->decode_WindowLineDataField(&param.buffer, fieldCount, fields);
    ms->RcvWindowLineData(x + i * dx, y + i * dy, sec, usec,
                          fieldCount, fields);
  }
  ms->RcvWindowLineData();
  ms->RcvWindowLineData(x, y, dx, dy, lineCount);

  return 0;
}

//static
int nmm_Microscope_Remote::handle_WindowScanNM (void * userdata,
                                             vrpn_HANDLERPARAM param) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
  vrpn_int32 x, y, sec, usec;
  float value, deviation;

  ms->decode_WindowScanNM(&param.buffer, &x, &y, &sec, &usec,
                          &value, &deviation);
  ms->RcvWindowScanNM(x, y, sec, usec, value, deviation);

  return 0;
}

//static
int nmm_Microscope_Remote::handle_WindowBackscanNM (void * userdata,
                                              vrpn_HANDLERPARAM param) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
  vrpn_int32 x, y, sec, usec;
  float value, deviation;

  ms->decode_WindowBackscanNM(&param.buffer, &x, &y, &sec, &usec,
                              &value, &deviation);
  ms->RcvWindowBackscanNM(x, y, sec, usec, value, deviation);

  return 0;
}

//static
int nmm_Microscope_Remote::handle_PointResultNM (void * userdata,
                                             vrpn_HANDLERPARAM param) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
  float x, y, height, deviation;
  vrpn_int32 sec, usec;

  ms->decode_PointResultNM(&param.buffer, &x, &y, &sec, &usec,
                           &height, &deviation);
  ms->RcvPointResultNM(x, y, sec, usec, height, deviation);

  return 0;
}

//static
int nmm_Microscope_Remote::handle_PointResultData (void * userdata,
                                        vrpn_HANDLERPARAM param) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
  vrpn_int32 sec, usec, fieldCount;
  float x, y, fields [MAX_CHANNELS];

  ms->decode_ResultData(&param.buffer, &x, &y, &sec, &usec,
                        &fieldCount, fields);
  ms->RcvResultData(SPM_POINT_RESULT_DATA, x, y, sec, usec,
                    fieldCount, fields);

  return 0;
}

//static
int nmm_Microscope_Remote::handle_BottomPunchResultData (void * userdata,
                                         vrpn_HANDLERPARAM param) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
  vrpn_int32 sec, usec, fieldCount;
  float x, y, fields [MAX_CHANNELS];

  ms->decode_ResultData(&param.buffer, &x, &y, &sec, &usec,
                        &fieldCount, fields);
  ms->RcvResultData(SPM_BOTTOM_PUNCH_RESULT_DATA, x, y, sec, usec,
                    fieldCount, fields);

  return 0;
}

//static
int nmm_Microscope_Remote::handle_TopPunchResultData (void * userdata,
                                      vrpn_HANDLERPARAM param) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
  vrpn_int32 sec, usec, fieldCount;
  float x, y, fields [MAX_CHANNELS];

  ms->decode_ResultData(&param.buffer, &x, &y, &sec, &usec,
                        &fieldCount, fields);
  ms->RcvResultData(SPM_TOP_PUNCH_RESULT_DATA, x, y, sec, usec,
                    fieldCount, fields);

  return 0;
}

//static
int nmm_Microscope_Remote::handle_ResultNM (void * userdata,
                           vrpn_HANDLERPARAM param) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
  vrpn_int32 sec, usec;
  float x, y, height, normX, normY, normZ;

  ms->decode_ResultNM(&param.buffer, &x, &y, &sec, &usec,
                      &height, &normX, &normY, &normZ);
  ms->RcvResultNM(x, y, sec, usec, height, normX, normY, normZ);

  return 0;
}

//static
int nmm_Microscope_Remote::handle_ForceCurveData (void * userdata,
                           vrpn_HANDLERPARAM param) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
  float x, y;
  vrpn_int32 num_points, num_halfcycles, sec, usec, i, j;     // changed int to long
  float ** curves = NULL;       // one for each halfcycle
  float * z_values;             // one for each point
  float z;
  float force [FC_MAX_HALFCYCLES];
  int mem_err = 0;

//  printf("starting force curve decoding\n");
  ms->decode_ForceCurveDataHeader(&param.buffer, &x, &y, &num_points,
        &num_halfcycles, &sec, &usec);

//  printf("got header: x=%f,y=%f,layers=%d,halfcyc=%d\n",x,y,num_points,
//              num_halfcycles);
  curves = new float * [num_halfcycles]; //(float **)malloc(num_halfcycles*sizeof(float *));
  if (!curves) mem_err = 1;
  else {
    for (i = 0; i < num_halfcycles; i++){
        curves[i] = new float [num_points];//(float *)malloc(num_points*sizeof(float));
      if (!curves[i]) mem_err = 1;
    }
  }
  z_values = new float[num_points];
  if (!z_values) mem_err = 1;

  //microscope->GetRasterPosition(x, y);
  //this->sec = sec;
  //this->usec = usec;

  for (i = 0; i < num_points; i++) {
    ms->decode_ForceCurveDataSingleLevel(&param.buffer,
        num_halfcycles, &z, force);
//    printf("got pnt: %d, z=%f",i, z);
    if (!mem_err){
      z_values[i] = z;
      for (j = 0; j < num_halfcycles; j++){
          curves[j][i] = force[j];
//        printf(",f=%f", force[j]);
      }
    }
//    printf("\n");
  }
  if (mem_err){
    fprintf(stderr, "nmm_MicroscopeRemote::RcvForceCurveData:  "
                    "Out of memory.\n");
    return -1;
  }
  else {
    ms->RcvForceCurveData(x, y, sec, usec, num_points, num_halfcycles,
                                z_values, (const float **) curves);
  }
//  printf("decoded force curve data successfully\n");
  for (i = 0; i < num_halfcycles; i++){
      delete [] curves[i];
  }
  delete [] curves;
  return 0;
}

//static
int nmm_Microscope_Remote::handle_PulseCompletedNM (void * userdata,
                                              vrpn_HANDLERPARAM param) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
  float x, y;

  ms->decode_PulseCompletedNM(&param.buffer, &x, &y);
  ms->RcvPulseCompletedNM(x, y);

  return 0;
}

//static
int nmm_Microscope_Remote::handle_PulseFailureNM (void * userdata,
                                             vrpn_HANDLERPARAM param) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
  float x, y;

  ms->decode_PulseFailureNM(&param.buffer, &x, &y);
  ms->RcvPulseFailureNM(x, y);

  return 0;
}

//static
int nmm_Microscope_Remote::handle_Scanning (void * userdata,
                           vrpn_HANDLERPARAM param) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
  vrpn_int32 on_off;

  ms->decode_Scanning(&param.buffer, &on_off);
  ms->RcvScanning(on_off);

  return 0;
}


//static
int nmm_Microscope_Remote::handle_ScanRange (void * userdata,
                           vrpn_HANDLERPARAM param) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
  float minX, maxX, minY, maxY, minZ, maxZ;

  ms->decode_ScanRange(&param.buffer, &minX, &maxX, &minY, &maxY, &minZ, &maxZ);
  ms->RcvScanRange(minX, maxX, minY, maxY, minZ, maxZ);

  return 0;
}

//static
int nmm_Microscope_Remote::handle_ReportScanAngle (void * userdata,
                                           vrpn_HANDLERPARAM param) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
  float angle;

  ms->decode_ReportScanAngle(&param.buffer, &angle);
  ms->RcvReportScanAngle(angle);

  return 0;
}

//static
int nmm_Microscope_Remote::handle_SetRegionCompleted (void * userdata,
                                           vrpn_HANDLERPARAM param) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
  float minX, minY, maxX, maxY;

  ms->decode_SetRegionC(&param.buffer, &minX, &minY, &maxX, &maxY);
  ms->RcvSetRegionC(STM_SET_REGION_COMPLETED, minX, minY, maxX, maxY);

  return 0;
}

//static
int nmm_Microscope_Remote::handle_SetRegionClipped (void * userdata,
                                         vrpn_HANDLERPARAM param) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
  float minX, minY, maxX, maxY;

  ms->decode_SetRegionC(&param.buffer, &minX, &minY, &maxX, &maxY);
  ms->RcvSetRegionC(STM_SET_REGION_CLIPPED, minX, minY, maxX, maxY);

  return 0;
}

//static
int nmm_Microscope_Remote::handle_ResistanceFailure (void * userdata,
                                              vrpn_HANDLERPARAM param) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
  vrpn_int32 which;

  ms->decode_ResistanceFailure(&param.buffer, &which);
  ms->RcvResistanceFailure(which);

  return 0;
}

//static
int nmm_Microscope_Remote::handle_Resistance (void * userdata,
                           vrpn_HANDLERPARAM param) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
  vrpn_int32 which, sec, usec;
  float resistance;

  ms->decode_Resistance(&param.buffer, &which, &sec, &usec, &resistance);
  ms->RcvResistance(which, sec, usec, resistance);

  return 0;
}

//static
int nmm_Microscope_Remote::handle_Resistance2(void * userdata,
                           vrpn_HANDLERPARAM param) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
  vrpn_int32 which, sec, usec;
  float resistance;
  float voltage, range, filter;

  ms->decode_Resistance2(&param.buffer, &which, &sec, &usec, &resistance,
                         &voltage, &range, &filter);
  ms->RcvResistance2(which, sec, usec, resistance, voltage, range, filter);

  return 0;
}

//static
int nmm_Microscope_Remote::handle_ReportSlowScan (void * userdata,
                                             vrpn_HANDLERPARAM param) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
  vrpn_int32 enable;

  ms->decode_ReportSlowScan(&param.buffer, &enable);
  ms->RcvReportSlowScan(enable);

  return 0;
}

//static
int nmm_Microscope_Remote::handle_ScanParameters (void * userdata,
                                             vrpn_HANDLERPARAM param) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
  char * buffer;
  vrpn_int32 length;

  ms->decode_ScanParameters(&param.buffer, &length, &buffer);
  ms->RcvScanParameters((const char **)&buffer);

  return 0;
}

//static
int nmm_Microscope_Remote::handle_HelloMessage (void * userdata,
                                             vrpn_HANDLERPARAM param) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
  char magic [4];
  char name [STM_NAME_LENGTH];
  vrpn_int32 majorVersion, minorVersion;

  ms->decode_HelloMessage(&param.buffer, magic, name,
                          &majorVersion, &minorVersion);
  ms->RcvHelloMessage(magic, name, majorVersion, minorVersion);

  return 0;
}

//static
int nmm_Microscope_Remote::handle_ClientHello (void * userdata,
                                             vrpn_HANDLERPARAM param) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
  char magic [4];
  char name [STM_NAME_LENGTH];
  vrpn_int32 majorVersion, minorVersion;

  ms->decode_ClientHello(&param.buffer, magic, name,
                         &majorVersion, &minorVersion);
  ms->RcvClientHello(magic, name, majorVersion, minorVersion);

  return 0;
}

/* Never sent by Topometrix
//static
int nmm_Microscope_Remote::handle_ClearScanChannels (void * userdata,
                                              vrpn_HANDLERPARAM) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;

  ms->RcvClearScanChannels();

  return 0;
}
*/
//static
int nmm_Microscope_Remote::handle_ScanDataset (void * userdata,
                           vrpn_HANDLERPARAM param) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
  char name [STM_NAME_LENGTH];
  char units [STM_NAME_LENGTH];
  float offset;
  float scale;
  vrpn_int32 numDatasets;
  long i;

  fprintf(stderr, "New Scan Datasets:\n");
  ms->decode_ScanDatasetHeader(&param.buffer, &numDatasets);
  ms->RcvClearScanChannels();
  for (i = 0; i < numDatasets; i++) {
    ms->decode_ScanDataset(&param.buffer, name, units, &offset, &scale);
    ms->RcvScanDataset(name, units, offset, scale);
  }

  return 0;
}

/* Never sent by Topometrix
//static
int nmm_Microscope_Remote::handle_ClearPointChannels (void * userdata,
                                              vrpn_HANDLERPARAM) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;

  ms->RcvClearPointChannels();

  return 0;
}
*/

//static
int nmm_Microscope_Remote::handle_PointDataset (void * userdata,
                                             vrpn_HANDLERPARAM param) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
  char name [STM_NAME_LENGTH];
  char units [STM_NAME_LENGTH];
  float offset;
  float scale;
  vrpn_int32 numDatasets;
  vrpn_int32 numSamples;
  long i;

  ms->decode_PointDatasetHeader(&param.buffer, &numDatasets);

  ms->RcvClearPointChannels();
  for (i = 0; i < numDatasets; i++) {
    ms->decode_PointDataset(&param.buffer, name, units, &numSamples,
                                  &offset, &scale);
    ms->RcvPointDataset(name, units, numSamples, offset, scale);
  }

  return 0;
}

//static
int nmm_Microscope_Remote::handle_PidParameters (void * userdata,
                                             vrpn_HANDLERPARAM param) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
  float p, i, d;

  ms->decode_PidParameters(&param.buffer, &p, &i, &d);
  ms->RcvPidParameters(p, i, d);

  return 0;
}

//static
int nmm_Microscope_Remote::handle_ScanrateParameter (void * userdata,
                                              vrpn_HANDLERPARAM param) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
  float rate;

  ms->decode_ScanrateParameter(&param.buffer, &rate);
  ms->RcvScanrateParameter(rate);

  return 0;
}

//static
int nmm_Microscope_Remote::handle_ReportGridSize (void * userdata,
                                             vrpn_HANDLERPARAM param) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
  vrpn_int32 x, y;

  ms->decode_ReportGridSize(&param.buffer, &x, &y);
  ms->RcvReportGridSize(x, y);

  return 0;
}

//static
int nmm_Microscope_Remote::handle_ServerPacketTimestamp (void * userdata,
                                              vrpn_HANDLERPARAM param) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
  vrpn_int32 sec, usec;

  ms->decode_ServerPacketTimestamp(&param.buffer, &sec, &usec);
  ms->RcvServerPacketTimestamp(sec, usec);

  return 0;
}

//static
int nmm_Microscope_Remote::handle_TopoFileHeader (void * userdata,
                                             vrpn_HANDLERPARAM param) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
  char * buffer;
  vrpn_int32 length;

  ms->decode_TopoFileHeader(&param.buffer, &length, &buffer);
  ms->RcvTopoFileHeader(length, buffer);

  return 0;
}

//   static 
int nmm_Microscope_Remote::handle_MaxSetpointExceeded (void *userdata, 
					       vrpn_HANDLERPARAM /*param*/ )
 {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
  ms->RcvMaxSetpointExceeded();

  return 0;
}

//static
int nmm_Microscope_Remote::handle_InScanlineMode (void *userdata, 
					vrpn_HANDLERPARAM param) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
  vrpn_int32 enabled;
  
  ms->decode_InScanlineMode(&param.buffer, &enabled);
  ms->RcvInScanlineMode(enabled);

  return 0;
}

//static
int nmm_Microscope_Remote::handle_ScanlineData (void *userdata, 
					vrpn_HANDLERPARAM param) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
  long i;
  vrpn_int32 sec, usec;
  float x, y, z, angle, slope, width;
  vrpn_int32 resolution, feedback_enabled, checking_forcelimit, num_channels;
  float max_force_setting, max_z_step, max_xy_step;
  float fields [MAX_CHANNELS];

  ms->decode_ScanlineDataHeader(&param.buffer, &x, &y, &z, &angle,
	&slope, &width, &resolution, &feedback_enabled, &checking_forcelimit, 
	&max_force_setting, &max_z_step, &max_xy_step, &sec, &usec, &num_channels);


  ms->RcvScanlineDataHeader(x, y, z, angle, slope, width, resolution,
	feedback_enabled, checking_forcelimit, max_force_setting, 
	max_z_step, max_xy_step, sec, usec, num_channels);

  for (i = 0; i < resolution; i++) {
    ms->decode_ScanlineDataPoint(&param.buffer, num_channels, fields);
    ms->RcvScanlineData(i, num_channels, fields);
  }
  return 0;
}

    // messages for Michele Clark's experiments
//static
int nmm_Microscope_Remote::handle_RecvTimestamp (void * userdata,
                                             vrpn_HANDLERPARAM param) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
  struct timeval t;

  ms->decode_RecvTimestamp(&param.buffer, &t);
  ms->RcvRecvTimestamp(t);

  return 0;
}

//static
int nmm_Microscope_Remote::handle_FakeSendTimestamp (void * userdata,
                                              vrpn_HANDLERPARAM param) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
  struct timeval t;

  ms->decode_FakeSendTimestamp(&param.buffer, &t);
  ms->RcvFakeSendTimestamp(t);

  return 0;
}

//static
int nmm_Microscope_Remote::handle_UdpSeqNum (void * userdata,
                           vrpn_HANDLERPARAM param) {
  nmm_Microscope_Remote * ms = (nmm_Microscope_Remote *) userdata;
  vrpn_int32 seqnum;

  ms->decode_UdpSeqNum(&param.buffer, &seqnum);
  ms->RcvUdpSeqNum(seqnum);

  return 0;
}

/////////////////////////////////////////////////////////////////////////

/** 
    Called when we first get a connection to the AFM. Send all initialization
    messages needed.  
*/
int nmm_Microscope_Remote::RcvGotConnection2() 
{

/*
  //printf("nmm_Microscope_Remote::RcvGotConnection2()\n");

  Can't do this until we have the mutex
  // Send off the relaxation parameters (if any)
  if (d_relax_comp.is_enabled()) {
      CHECK(SetRelax(state.stmRxTmin, state.stmRxTsep));
  } else {
      CHECK(SetRelax(0, 0));
  }

  Can't do this until we have the mutex
  // Start scanning the surface
  CHECK(ResumeFullScan());

  Can't do this until we have the mutex
  // Tell AFM to scan forward and backward, or just forward.
  CHECK(SetScanStyle());
*/

  // Ask it for the scan range in x, y, and z.
  // When this is read back, Z will be used to set min_z and max_z.
  CHECK(QueryScanRange());

  return 0;
}

// Obsolete, only occurs in old stream files, because it doesn't
// include phase information.
void nmm_Microscope_Remote::RcvInTappingMode (const float _p, const float _i,
                                   const float _d, const float _setpoint,
                                   const float _amp) {
    // Call newer rcv function, with default values for the parameters 
    // not covered by this message. 
    printf("WARNING: Old Tapping Mode message received, treating as Oscillating\n");
    RcvInOscillatingMode ( _p, _i, _d, _setpoint, _amp, 100, 1, 1, 1, 0.0);
}

void nmm_Microscope_Remote::RcvInOscillatingMode (const float _p, 
                                   const float _i,
                                   const float _d, const float _setpoint,
                                   const float _amp,
                                   const float _frequency,
                                   const vrpn_int32 _input_gain, 
                                   const vrpn_bool _ampl_or_phase,
                                   const vrpn_int32 _drive_attenuation, 
                                   const float _phase
) {
  if (state.acquisitionMode == MODIFY) {
    printf("Matching AFM modify parameters (Oscillating).\n");
    printf("   S=%g  P=%g, I=%g, D=%g, A=%g, F=%g, G=%d, P/A %d, Atten=%d, Ph=%g\n",
           _setpoint,_p, _i, _d, _amp, _frequency, _input_gain,
           _ampl_or_phase, _drive_attenuation, _phase);
    if (state.modify.mode != TAPPING) state.modify.mode = TAPPING;
    state.modify.p_gain = _p;
    state.modify.i_gain = _i;
    state.modify.d_gain = _d;
    state.modify.setpoint = _setpoint;
    d_decoration->modSetpoint = _setpoint;
    state.modify.amplitude = _amp;
    state.modify.frequency = _frequency;
    state.modify.input_gain = _input_gain;
    state.modify.ampl_or_phase = _ampl_or_phase;
    state.modify.drive_attenuation = _drive_attenuation;
    state.modify.phase = _phase;
  } else if (state.acquisitionMode == IMAGE){
    printf("Matching AFM image parameters (Oscillating).\n");
    printf("   S=%g  P=%g, I=%g, D=%g, A=%g, F=%g, G=%d, P/A %d, Atten=%d, Ph=%g\n",
           _setpoint,_p, _i, _d, _amp, _frequency, _input_gain,
           _ampl_or_phase, _drive_attenuation, _phase);
    if (state.image.mode != TAPPING) state.image.mode = TAPPING;
    state.image.p_gain = _p;
    state.image.i_gain = _i;
    state.image.d_gain = _d;
    state.image.setpoint = _setpoint;
    d_decoration->imageSetpoint = _setpoint;
    state.image.amplitude = _amp;
    state.image.frequency = _frequency;
    state.image.input_gain = _input_gain;
    state.image.ampl_or_phase = _ampl_or_phase;
    state.image.drive_attenuation = _drive_attenuation;
    state.image.phase = _phase;
    // We want the modify  defaults to match the first report of image mode.
    // We watch for the initialization message sent by the AFM to tell us
    // what the state is. So if this is the first time we get this
    // message, we will set the modify state as well.
    if (state.first_PID_message_pending ){
	state.first_PID_message_pending = VRPN_FALSE;
	// Set defaults for modify mode...
        // Don't change mode or setpoint!
        state.modify.p_gain = _p;
        state.modify.i_gain = _i;
        state.modify.d_gain = _d;
        //    state.modify.setpoint = _setpoint;
        state.modify.amplitude = _amp;
        state.modify.frequency = _frequency;
        state.modify.input_gain = _input_gain;
        state.modify.ampl_or_phase = _ampl_or_phase;
        state.modify.drive_attenuation = _drive_attenuation;
        state.modify.phase = _phase;
        // Copy scan rate, too
        state.modify.scan_rate_microns = (vrpn_float32)state.image.scan_rate_microns;
    }
  } else if (state.acquisitionMode == SCANLINE){
    printf("Matching AFM scanline parameters (Oscillating).\n");
    printf("   S=%g  P=%g, I=%g, D=%g, A=%g, F=%g, G=%d, P/A %d, Atten=%d, Ph=%g\n",
           _setpoint,_p, _i, _d, _amp, _frequency, _input_gain,
           _ampl_or_phase, _drive_attenuation, _phase);
    if (state.scanline.mode != TAPPING) state.scanline.mode = TAPPING;
    state.scanline.p_gain = _p;
    state.scanline.i_gain = _i;
    state.scanline.d_gain = _d;
    state.scanline.setpoint = _setpoint;
    d_decoration->scanlineSetpoint = _setpoint;
    state.scanline.amplitude = _amp;
    state.scanline.frequency = _frequency;
    state.scanline.input_gain = _input_gain;
    state.scanline.ampl_or_phase = _ampl_or_phase;
    state.scanline.drive_attenuation = _drive_attenuation;
    state.scanline.phase = _phase;
    state.first_PID_message_pending = VRPN_FALSE;
  }
  else {
   fprintf(stderr, "RcvInOscillatingMode: Error, in unknown mode\n");
  }

  state.first_PID_message_pending = VRPN_FALSE;
}

void nmm_Microscope_Remote::RcvInContactMode (const float _p, const float _i,
                                   const float _d, const float _setpoint) {
  if (state.acquisitionMode == MODIFY) {
    printf("Matching AFM modify parameters (Contact).\n");
    printf("   S=%g  P=%g, I=%g, D=%g\n", _setpoint,_p, _i, _d);
    if (state.modify.mode != CONTACT)
      state.modify.mode = CONTACT;
    state.modify.p_gain = _p;
    state.modify.i_gain = _i;
    state.modify.d_gain = _d;
    state.modify.setpoint = _setpoint;
    d_decoration->modSetpoint = _setpoint;
  } else if (state.acquisitionMode == IMAGE) {
    printf("Matching AFM image parameters (Contact).\n");
    printf("   S=%g  P=%g, I=%g, D=%g\n", _setpoint,_p, _i, _d);
    if (state.image.mode != CONTACT)
      state.image.mode = CONTACT;
    state.image.p_gain = _p;
    state.image.i_gain = _i;
    state.image.d_gain = _d;
    state.image.setpoint = _setpoint;
    d_decoration->imageSetpoint = _setpoint;
    // We want the modify  defaults to match the first report of image mode.
    // We watch for the initialization message sent by the AFM to tell us
    // what the state is. So if this is the first time we get this
    // message, we will set the modify state as well.
    if (state.first_PID_message_pending ){
	state.first_PID_message_pending = VRPN_FALSE;
	// Set defaults for modify mode...
        // Don't change mode or setpoint!
        state.modify.p_gain = _p;
        state.modify.i_gain = _i;
        state.modify.d_gain = _d;
        //    state.modify.setpoint = _setpoint;
        // Copy scan rate, too
        state.modify.scan_rate_microns = (vrpn_float32)state.image.scan_rate_microns;
    }
  } else if (state.acquisitionMode == SCANLINE){
    printf("Matching AFM scanline parameters (Contact).\n");
    printf("   S=%g  P=%g, I=%g, D=%g\n", _setpoint,_p, _i, _d);
    if (state.scanline.mode != CONTACT)
      state.scanline.mode = CONTACT;
    state.scanline.p_gain = _p;
    state.scanline.i_gain = _i;
    state.scanline.d_gain = _d;
    state.scanline.setpoint = _setpoint;
    d_decoration->scanlineSetpoint = _setpoint;
  }
  else {
    fprintf(stderr, "RcvInContactMode: Error, in unknown mode\n");
  }
  state.first_PID_message_pending = VRPN_FALSE;
}

void nmm_Microscope_Remote::RcvInDirectZControl (const float _max_z_step, 
				       const float _max_xy_step, 
				       const float _min_setpoint, 
				       const float _max_setpoint, 
				       const float _max_lateral_force,
				       const float _freespace_normal_force, 
				       const float _freespace_lat_force ) 
{
  if (state.acquisitionMode == MODIFY) {
    printf("Matching AFM Direct Z Control parameters.\n");
    printf("   max_xy=%g  max_z=%g, min_setpoint=%g, max_setpoint=%g, max_lat_force %g\n", 
	   _max_z_step, _max_xy_step, _min_setpoint, 
	   _max_setpoint, _max_lateral_force);
    printf("   free space normal force %g, free space lateral force %g\n",
	   _freespace_normal_force, _freespace_lat_force);
    if (state.modify.control != DIRECTZ)
       state.modify.control = DIRECTZ;
    state.modify.max_z_step = _max_z_step;
    state.modify.max_xy_step = _max_xy_step;
    state.modify.min_z_setpoint = _min_setpoint;
    state.modify.max_z_setpoint = _max_setpoint;
    state.modify.max_lat_setpoint = _max_lateral_force;
    state.modify.freespace_normal_force = _freespace_normal_force;
    state.modify.freespace_lat_force = _freespace_lat_force;
  } else {
    fprintf(stderr, "Can't do Image/DirectZ control!!\n");
  }
}

void nmm_Microscope_Remote::RcvInSewingStyle (const float _setpoint, const float _bDelay,
                                  const float _tDelay, const float _pbDist,
                                  const float _mDist, const float _rate,
                                  const float _mSafe) {
  if (state.acquisitionMode == MODIFY) {
    printf("Matching AFM modify parameters (Sewing).\n");
    if ((state.modify.mode != CONTACT) ||
        (state.modify.style != SEWING)) {
      state.modify.mode = CONTACT;
      state.modify.style = SEWING;
    }
    state.modify.setpoint = _setpoint;
    d_decoration->modSetpoint = _setpoint;
    // transmit in sec, store in ms
    state.modify.bot_delay = 1000.0 * _bDelay;
    state.modify.top_delay = 1000.0 * _tDelay;
    state.modify.z_pull = _pbDist;
    state.modify.punch_dist = _mDist;
    // convert oppositely
    state.modify.speed = 0.001 * _rate;
    state.modify.watchdog = _mSafe;
  } else {
    fprintf(stderr, "Can't do Image/Sewing mode!\n");
  }
}

void nmm_Microscope_Remote::RcvInSpectroscopyMode(const float _setpoint,
                        const float _startDelay,
                        const float _zStart, const float _zEnd,
                        const float _zPullback, const float _forceLimit,
                        const float _distBetweenFC,
                        const int _numPoints, const int _numHalfcycles,
                        const float _sampleSpeed, const float _pullbackSpeed,
                        const float _startSpeed, const float _feedbackSpeed,
                        const int _avgNum, const float _sampleDelay,
                        const float _pullbackDelay,
                        const float _feedbackDelay) {

  if (state.acquisitionMode == MODIFY) {
    printf("Matching AFM modify parameters (Spectroscopy).\n");
    printf("stdel=%g, z_st=%g, z_end=%g, z_pull=%g, forcelim=%g, dist=%g\n",
           _startDelay, _zStart, _zEnd, _zPullback, _forceLimit, _distBetweenFC);
    printf("numpoints=%d, numhalfcycles=%d, samp_spd=%g, pull_spd=%g, start_spd=%g\n",
           _numPoints, _numHalfcycles,_sampleSpeed, _pullbackSpeed, _startSpeed);
    printf("fdback_spd=%g, avg_num=%d, samp_del=%g, pull_del=%g, fdback_del=%g\n",
                _feedbackSpeed, _avgNum, _sampleDelay, _pullbackDelay, _feedbackDelay);
    if (state.modify.style != FORCECURVE) {
      state.modify.style = FORCECURVE;
    }
    state.modify.setpoint = _setpoint;
    d_decoration->modSetpoint = _setpoint;
    state.modify.fc_start_delay = _startDelay;
    state.modify.fc_z_start = _zStart;
    state.modify.fc_z_end = _zEnd;
    state.modify.fc_z_pullback = _zPullback;
    state.modify.fc_force_limit = _forceLimit;
    state.modify.fc_movedist = _distBetweenFC;
    state.modify.fc_num_points = _numPoints;
    state.modify.fc_num_halfcycles = _numHalfcycles;
    state.modify.fc_sample_speed = _sampleSpeed;
    state.modify.fc_pullback_speed = _pullbackSpeed;
    state.modify.fc_start_speed = _startSpeed;
    state.modify.fc_feedback_speed = _feedbackSpeed;
    state.modify.fc_avg_num = _avgNum;
    state.modify.fc_sample_delay = _sampleDelay;
    state.modify.fc_pullback_delay = _pullbackDelay;
    state.modify.fc_feedback_delay = _feedbackDelay;
  } else {
    fprintf(stderr, "Can't do Image/SpectroscopyMode!\n");
  }
}

/*	obsolete?
void nmm_Microscope_Remote::RcvInSpectroscopyMode(float _setpoint,
			float _startDelay, 
			float _zStart, float _zEnd,
			float _zPullback, float _forceLimit,
			float _distBetweenFC, 
			long _numPoints, long _numHalfcycles) {
					
  if (state.acquisitionMode == MODIFY) {
    printf("Matching AFM modify parameters (Spectroscopy).\n");
    printf("stdel=%g, z_st=%g, z_end=%g, z_pull=%g, forcelim=%g\n",
		_startDelay, _zStart, _zEnd, _zPullback, _forceLimit);
    printf("dist=%g, numpoints=%d, numhalfcycles=%d\n",
		_distBetweenFC, _numPoints, _numHalfcycles);
    if ((state.modify.mode != CONTACT) ||
	(state.modify.style != FORCECURVE)) {
      state.modify.mode = CONTACT;	// XXX don't think this is relevant
					// e.g. pid are not used
      state.modify.style = FORCECURVE;	
    }
    state.modify.setpoint = _setpoint;
    d_decoration->modSetpoint = _setpoint;
    state.modify.fc_start_delay = _startDelay;
    state.modify.fc_z_start = _zStart;
    state.modify.fc_z_end = _zEnd;
    state.modify.fc_z_pullback = _zPullback;
    state.modify.fc_force_limit = _forceLimit;
    state.modify.fc_movedist = _distBetweenFC;
    state.modify.fc_num_points = _numPoints;
    state.modify.fc_num_halfcycles = _numHalfcycles;
  } else {
    fprintf(stderr, "Can't do Image/SpectroscopyMode!\n");
  }
}
*/

void nmm_Microscope_Remote::RcvForceParameters (const long _modifyEnable,
                                     const float) {
  state.modify.modify_enabled = _modifyEnable;
  if (!_modifyEnable) {
    printf("Force modification disabled!\n");
  } else {
    printf("Fmods min = %g, max = %g\n",
           state.modify.setpoint_min, state.modify.setpoint_max);
  }
}

void nmm_Microscope_Remote::RcvBaseModParameters (const float _min, const float _max) {
  state.modify.setpoint_min = _min;
  state.modify.setpoint_max = _max;
  d_decoration->modSetpointMin = _min;
  d_decoration->modSetpointMax = _max;
  printf("Fmods min = %g, max = %g\n",
         _min, _max);
}

void nmm_Microscope_Remote::RcvForceSettings (const float _min, const float _max,
                                   const float _setpoint) {
  state.modify.setpoint_min = _min;
  state.modify.setpoint_max = _max;
  state.modify.setpoint = _setpoint;
  d_decoration->modSetpointMin = _min;
  d_decoration->modSetpointMax = _max;
  d_decoration->modSetpoint = _setpoint;
  printf("Fmods min = %g, max = %g, setpoint = %g\n",
         _min, _max, _setpoint);
}

void nmm_Microscope_Remote::RcvVoltsourceEnabled (const long _voltNum,
                                       const float _voltage) {
  // DO NOTHING
  printf("Volt source %ld has been enabled with %g voltage\n",
         _voltNum, _voltage);
}

void nmm_Microscope_Remote::RcvVoltsourceDisabled (const long _voltNum) {
  // DO NOTHING
  printf("Volt source %ld has been disabled\n", _voltNum);
}

void nmm_Microscope_Remote::RcvAmpEnabled (const long _ampNum, const float _offset,
                                const float _percentOffset,
                                const long _gainMode) {
  // DO NOTHING
  printf("Amplifier %ld has been enabled with offset %g, percent "
         "offset %g, and gain mode %ld\n", _ampNum, _offset, _percentOffset,
         _gainMode);
}

void nmm_Microscope_Remote::RcvAmpDisabled (const long _ampNum) {
  // DO NOTHING
  printf("Amplifier %ld has been disabled\n", _ampNum);
}

/* Helps with Thermo Image Analysis mode. When in this mode, most of 
 the widgets/dialogs that Nano needs to control the SPM aren't available
 So we avoid issuing any commands to Thermo, by disabling all device 
 controls. 
*/
void nmm_Microscope_Remote::RcvSuspendCommands() 
{
    if (ReadMode() != READ_DEVICE) return;
    state.commands_suspended = 1;
}

void nmm_Microscope_Remote::RcvResumeCommands() 
{
    if (ReadMode() != READ_DEVICE) return;
    state.commands_suspended = 0;
}

void nmm_Microscope_Remote::RcvStartingToRelax (const long _sec, const long _usec) {
  if (state.doRelaxComp) {
    d_relax_comp.start_fix(_sec, _usec, state.lastZ);
    //printf("Beginning relaxation compensation at %ld:%ld\n", _sec, _usec);
  }

  if (state.doDriftComp)
    driftZDirty();
}

void nmm_Microscope_Remote::RcvInModModeT (const long, const long) {
    fprintf(stderr, "ERROR nmm_Microscope_Remote::RcvInImgModeT disabled message received.\n");
    /*
  state.acquisitionMode = MODIFY;
  //printf("In modify mode\n");

  // Do relaxation compensation ( if it is enabled)
  d_relax_comp.start_fix(_sec, _usec, state.lastZ);
  //printf("Compensating mod force %g\n", (float) state.modify.setpoint);

  if (state.doDriftComp)
    driftZDirty();

  doModifyModeCallbacks();
    */
}

void nmm_Microscope_Remote::RcvInModMode (void) {
  state.acquisitionMode = MODIFY;
  //printf("In modify mode\n");

  d_mod_window_initialized = vrpn_FALSE;

  // I think this should only be done if we get the startingToRelax message.
  // Do relaxation compensation ( if it is enabled)
  //d_relax_comp.start_fix(_sec, _usec, state.lastZ);
  //printf("Compensating mod force %g\n", (float) state.modify.setpoint);

  if (state.doDriftComp)
    driftZDirty();

  doModifyModeCallbacks();
}

// Not used by Topo code
void nmm_Microscope_Remote::RcvInImgModeT (const long, const long) {
    fprintf(stderr, "ERROR nmm_Microscope_Remote::RcvInImgModeT disabled message received.\n");
    /*
  state.acquisitionMode = IMAGE;
  //printf("In image mode\n");

  // this is wrong, should be relax up. 
    d_relax_comp.start_fix(_sec, _usec, state.lastZ);
    //printf("Compensating img force %g\n", (float) state.image.amplitude);

  if (state.doDriftComp)
    driftZDirty();

  doImageModeCallbacks();
    */
}

// case AFM_IN_IMG_MODE:
// case SPM_INVISIBLE_TRAIL:
void nmm_Microscope_Remote::RcvInImgMode (void) {

  int previousAcquisitionMode = state.acquisitionMode;
  state.acquisitionMode = IMAGE;
  //printf("In image mode\n");

  if (state.doDriftComp)
    driftZDirty();

  // If we're only relaxing down, turn off the relaxation code
  d_relax_comp.stop_fix();
  // XXX Took out the ability to compensate for transition
  // into image mode from modify mode. 
  //if (!state.doRelaxUp) { ... }

  doImageModeCallbacks();

  d_mod_window_pad = state.numLinesToJumpBack;

  // did we just come out of modifying and did we receive at least one 
  // point result?
  if ((previousAcquisitionMode == MODIFY) && d_mod_window_initialized) {
    // add padding to the region
    d_mod_window_min_x -= d_mod_window_pad;
    d_mod_window_min_y -= d_mod_window_pad;
    d_mod_window_max_x += d_mod_window_pad;
    d_mod_window_max_y += d_mod_window_pad;
    // check to make sure we don't exceed the image boundaries
    if (d_mod_window_min_x < 0) d_mod_window_min_x = 0;
    if (d_mod_window_min_y < 0) d_mod_window_min_y = 0;
    if (d_mod_window_max_x > d_dataset->inputGrid->numX() - 1)
        d_mod_window_max_x = d_dataset->inputGrid->numX() - 1;
    if (d_mod_window_max_y > d_dataset->inputGrid->numY() - 1)
        d_mod_window_max_y = d_dataset->inputGrid->numY() - 1;

    // this function would be nice but isn't implemented and probably
    // would require someone to write new Topometrix dsp code
    //SetScanWindow(d_mod_window_min_x, d_mod_window_min_y,
    //              d_mod_window_max_x, d_mod_window_max_y);

    // instead we do this:
    // XXX - this should depend on the direction of the scanning
    // (whether its up or down in the Y direction)
    // this is hard-coded for what works with typical Thermomicroscope setup
    int lineNumber = d_dataset->inputGrid->numY()-1 - d_mod_window_max_y;
    //printf("jumping to line %d after modify\n", lineNumber);
    JumpToScanLine(lineNumber);

  }
}

void nmm_Microscope_Remote::RcvModForceSet (const float _value) {
  if (state.modify.mode == CONTACT) {
    state.modify.setpoint = _value;
    d_decoration->modSetpoint = _value;
    printf("Mod force set at %g (setpoint)\n", _value);
  } else if (state.modify.mode == TAPPING) {
    state.modify.amplitude = _value;
    printf("Mod force set at %g (amplitude)\n", _value);
  }
  if (state.doDriftComp)
    driftZDirty();
}

void nmm_Microscope_Remote::RcvImgForceSet (const float _value) {
  if (state.image.mode == CONTACT) {
    state.image.setpoint = _value;
    d_decoration->imageSetpoint = _value;
    printf("Img force set at %g (setpoint)\n", _value);
  } else if (state.image.mode == TAPPING) {
    state.image.amplitude = _value;
    printf("Img force set at %g (amplitude)\n", _value);
  }
  if (state.doDriftComp)
    driftZDirty();
}

void nmm_Microscope_Remote::RcvModSet (const long _modifyEnable, const float _max,
                            const float _min, const float _value) {
  state.modify.modify_enabled = _modifyEnable;
  if (state.modify.mode == CONTACT) {
    state.modify.setpoint_max = _max;
    state.modify.setpoint_min = _min;
    state.modify.setpoint = _value;
    d_decoration->modSetpointMin = _min;
    d_decoration->modSetpointMax = _max;
    d_decoration->modSetpoint = _value;
    printf("Mod force set at %g (setpoint)\n", _value);
  } else if (state.modify.mode == TAPPING) {
    state.modify.amplitude_max = _max;
    state.modify.amplitude_min = _min;
    state.modify.amplitude = _value;
    printf("Mod force set at %g (amplitude)\n", _value);
  }
  if (state.doDriftComp)
    driftZDirty();
}

void nmm_Microscope_Remote::RcvImgSet (const long _modifyEnable, const float _max,
                            const float _min, const float _value) {
  state.modify.modify_enabled = _modifyEnable;
  if (state.image.mode == CONTACT) {
    state.image.setpoint_max = _max;
    state.image.setpoint_min = _min;
    state.image.setpoint = _value;
    d_decoration->imageSetpointMin = _min;
    d_decoration->imageSetpointMax = _max;
    d_decoration->imageSetpoint = _value;
    printf("Img force set at %g (setpoint)\n", _value);
  } else if (state.image.mode == TAPPING) {
    state.image.amplitude_max = _max;
    state.image.amplitude_min = _min;
    state.image.amplitude = _value;
    printf("Img force set at %g (amplitude)\n", _value);
  }
  if (state.doDriftComp)
    driftZDirty();
}

void nmm_Microscope_Remote::RcvRelaxSet (const long _min, const long _sep) {
  state.stmRxTmin = _min;
  d_relax_comp.set_ignore_time_ms(_min);
  //printf("Relax ignore time set at %ld\n", _min);
  state.stmRxTsep = _min;
  d_relax_comp.set_separation_time_ms(_sep);
  //printf("Relax separation time set at %ld\n", _sep);
}

void nmm_Microscope_Remote::RcvForceSet (const float _force) {
  printf("Throwing away:  force set at %g\n", _force);

  if (_force != state.modify.setpoint_min) {
	// Used to play a sound
  }
  if (state.doDriftComp)
    driftZDirty();
}

// case AFM_IMG_FORCE_SET_FAILURE:
// case AFM_MOD_FORCE_SET_FAILURE:
// case AFM_FORCE_SET_FAILURE:
void nmm_Microscope_Remote::RcvForceSetFailure (const float) {
  state.modify.modify_enabled = VRPN_FALSE;
  printf("Force modifications disabled!\n");
}

void nmm_Microscope_Remote::RcvPulseParameters (const long _pulseEnabled,
                                     const float _bias, const float _height,
                                     const float _width) {
  if (_pulseEnabled) {
    printf("Pulses disabled!\n");
  } else {
    printf("Pulse bias = %g, peak = %g, width = %g\n",
           _bias, _height, _width);
  }
}

/* OBSOLETE
void nmm_Microscope_Remote::RcvStdDevParameters (const long _samples, const float _freq) {
  state.modify.std_dev_samples = _samples;
  state.modify.std_dev_frequency = _freq;
  printf("Num samples/point = %ld, Frequency = %g\n", _samples, _freq);
}
*/

long nmm_Microscope_Remote::RcvWindowLineData (const long _x, const long _y,
                                    const long _sec, const long _usec,
                                    const long _fieldCount,
                                    const float * _fields) {
  // HACK HACK HACK

  if (state.data.scan_channels->Handle_report(_x, _y, _sec, _usec,
                                         (float *) _fields, _fieldCount)) {
    fprintf(stderr, "Error handling window line data\n");
    d_dataset->done = VRPN_TRUE;
    return -1;
  }
  return 0;
}

long nmm_Microscope_Remote::RcvWindowLineData(const long _x, const long _y,
					      const long _dx, const long _dy,
					      const long _lineCount) {
  long xf, yf;
  double xi_2, yi_2, xf_2, yf_2;
  nmb_Image *image;
  
  image = d_dataset->dataImages->getImageByName(d_dataset->heightPlaneName->string() );

  xf = _x + (_lineCount - 1) * _dx;
  yf = _y + (_lineCount - 1) * _dy;
  
  image->pixelToWorld( (double)_x, (double)_y, xi_2, yi_2);
  image->pixelToWorld( (double)xf, (double)yf, xf_2, yf_2);

  d_decoration->sl_left[0] = (float)xi_2;
  d_decoration->sl_left[1] = (float)yi_2;
  d_decoration->sl_left[2] = 
    d_dataset->inputGrid->getPlaneByName(d_dataset->heightPlaneName->string() )->scaledValue(_x,_y);
  d_decoration->sl_right[0] = (float)xf_2;
  d_decoration->sl_right[1] = (float)yf_2;
  d_decoration->sl_right[2] = 
    d_dataset->inputGrid->getPlaneByName(d_dataset->heightPlaneName->string() )->scaledValue(xf,yf);

  BCPlane *cp = d_dataset->inputGrid->getPlaneByName(d_dataset->colorPlaneName->string() );
  // Color map drift compensation. Keep track of the average data value of the
  // first scan line
  if (cp && (_x == 0) && (_y == cp->numY() -1)) {
      d_decoration->first_line_avg = d_dataset->getFirstLineAvg(cp);
  }
  return 0;
}

long nmm_Microscope_Remote::RcvWindowLineData (void) {
  // blue, since we are not touching or modifying
  d_decoration->mode = nmb_Decoration::IMAGE;

  // XXX Experimental. Incremental save of stream file when
  // user is not touching or modifying the sample. 
  d_connection->save_log_so_far();

//    state.dlistchange = VRPN_TRUE;  OBSOLETE
  return 0;
}

// TODO:  check _x and _y for out-of-bounds?
void nmm_Microscope_Remote::RcvWindowScanNM
       (const long /*_x*/, const long /*_y*/,
        const long /*_sec*/, const long /*_usec*/,
        const float /*_value*/,
        const float /*_deviation*/) {
    fprintf(stderr, "ERROR nmm_Microscope_Remote::RcvWindowScanNM"
	    " obsolete message received.\n");
    // If we need to read stream files with this message, we should
    // translate them into the WindowLineData message
}

void nmm_Microscope_Remote::RcvWindowBackscanNM
       (const long /*_x*/, const long /*_y*/,
        const long /*_sec*/, const long /*_usec*/,
        const float /*_value*/,
        const float /*_deviation*/) {
    fprintf(stderr, "ERROR nmm_Microscope_Remote::RcvWindowBackscanNM"
	    " obsolete message received.\n");
    // If we need to read stream files with this message, we should
    // translate them into the WindowLineData message
}

void nmm_Microscope_Remote::RcvPointResultNM
       (const float /*_x*/, const float /*_y*/,
        const long /*_sec*/, const long /*_usec*/,
        const float /*_height*/,
        const float /*_deviation*/) {
    fprintf(stderr, "ERROR nmm_Microscope_Remote::RcvPointResultNM"
	    " obsolete message received.\n");
    // If we need to read stream files with this message, we should
    // translate them into the ResultData message
}

// case SPM_POINT_RESULT_DATA:
// case SPM_BOTTOM_PUNCH_RESULT_DATA:
// case SPM_TOP_PUNCH_RESULT_DATA: {
void nmm_Microscope_Remote::RcvResultData (const long _type,
                                const float _x, const float _y,
                                const long _sec, const long _usec,
                                const long _fieldCount,
                                const float * _fields) {
  Point_value * z_value;
  float height;
  long i;

  d_bluntResult.normal[Z] = 1.0;  // invalidate it (?)

  if (spm_verbosity >= 1) {
    printf("nmm_Microscope_Remote::RcvResultData: Point result,"
	   " type %ld at (%g, %g), time %ld:%ld\n",
           _type, _x, _y, _sec, _usec);
    printf("  Raw values:");
    for (i = 0; i < _fieldCount; i++)
      printf(" %g", _fields[i]);
    printf("\n");
  }

  // Ignore TOP_PUNCH results for the most part, since
  // they will have less real data than the bottom
  // punches and will mess up feel mode.
  if (_type == SPM_TOP_PUNCH_RESULT_DATA)
    return;

  // Make the report.  This has the side effect of updating
  // the inputPoint values and, if d_accumulatePointResults,
  // the incomingPointList.
  // HACK HACK HACK
  if (state.data.point_channels->Handle_report(_x, _y, _sec, _usec,
                                          (float *) _fields, _fieldCount,
                                          d_accumulatePointResults)) {
    fprintf(stderr, "Error handling SPM point result data\n");
    d_dataset->done = VRPN_TRUE;
    return;
  }

  if (spm_verbosity >= 1) {
    state.data.inputPoint->print("  Result:");
  }
  // Look up the value that corresponds to what is
  // mapped to the heightGrid, if we are getting that
  // data set.  This will make what we feel match what
  // we are looking at.
  z_value = state.data.inputPoint->getValueByPlaneName
                   (d_dataset->heightPlaneName->string());

  // Do relaxation compensation using the data set that is mapped to
  // height.
  if (z_value) {
    height = z_value->value();
    // If the height needs adjusting, do the adjustment, maybe using
    // a stored value. 
    // Otherwise fix_height will leave the height alone. 
    height = d_relax_comp.fix_height(_sec, _usec, height);
    z_value->setValue(height);
    //Store this height 
    state.lastZ = height;
  }

  //XXX Drift compensation taken out

  // splat this point onto the grid
  if (state.doSplat && !d_relax_comp.is_ignoring_points())
    ptSplat(&state.lost_changes, d_dataset->inputGrid, state.data.inputPoint);

  // set the background color
  if (state.acquisitionMode == MODIFY)
    d_decoration->mode = nmb_Decoration::MODIFY;  // red if modifying
  else{
    d_decoration->mode = nmb_Decoration::FEEL;  // yellow if just touching
						// or moving to start position
						// or relaxing
  }

  if (state.readingStreamFile && !state.cannedLineVisible)
    state.cannedLineVisible = VRPN_TRUE;

  // if it's a modification result, display it
  if ((state.acquisitionMode == MODIFY || state.cannedLineVisible) &&
      (!d_relax_comp.is_ignoring_points())) {
    // Causes the white tick marks/modify markers to show up.
    DisplayModResult(state.data.inputPoint->x(),
                     state.data.inputPoint->y(),
                     0.0f, z_value, VRPN_TRUE);
  }
  if (state.modify.style != FORCECURVE)	{
      // XXX HACK - we need point results
      if (ohmmeter !=NULL) {
          // HACK - to get ohmmeter data in point results
          char *res_channel_name = "Resistance";
          char *res_channel_unit = "Ohms";
          if (!d_res_channel_added) {
              state.data.inputPoint->addNewValue(res_channel_name, res_channel_unit);
              d_res_channel_added = vrpn_TRUE;
          }
          Point_value *pv = state.data.inputPoint->getValueByName(res_channel_name);
          if (!pv) {
              fprintf(stderr, "nmm_Microscope_Remote::RcvResultData: "
                      "Unable to get ohmmeter channel\n");
          } else {
              pv->setValue(lastResistanceReceived);
          }
          // END HACK
      }
     
     doPointDataCallbacks(state.data.inputPoint); // to feel what we are
						// doing but we don't
						// want to store them
						// in a modfile because
						// its being used to
						// store the force curve
     
  }
  // latency compensation
  d_decoration->trueTipLocation[0] = _x;
  d_decoration->trueTipLocation[1] = _y;
  if (z_value) {
      d_decoration->trueTipLocation[2] = z_value->value();
  } else {
      d_decoration->trueTipLocation[2] = 0;
  }
  d_decoration->trueTipLocation_changed = 1;

  if (state.acquisitionMode == MODIFY) {
     double grid_x, grid_y;
     d_dataset->inputGrid->worldToGrid((double)_x, (double)_y, grid_x, grid_y);
     if (!d_mod_window_initialized){
        d_mod_window_min_x = (vrpn_int32)grid_x;
        d_mod_window_min_y = (vrpn_int32)grid_y;
        d_mod_window_max_x = (vrpn_int32)grid_x;
        d_mod_window_max_y = (vrpn_int32)grid_y;
        d_mod_window_initialized = vrpn_TRUE;
     } else {
        if (grid_x < d_mod_window_min_x) 
            d_mod_window_min_x = (vrpn_int32)grid_x;
        else if (grid_x > d_mod_window_max_x) 
            d_mod_window_max_x = (vrpn_int32)grid_x;
        if (grid_y < d_mod_window_min_y) 
            d_mod_window_min_y = (vrpn_int32)grid_y;
        else if (grid_y > d_mod_window_max_y) 
            d_mod_window_max_y = (vrpn_int32)grid_y;
     }
  }
}

// case STM_ZIG_RESULT_NM:
// case SPM_BLUNT_RESULT_NM:
void nmm_Microscope_Remote::RcvResultNM
             (const float /*_x*/, const float /*_y*/,
              const long /*_sec*/, const long /*_usec*/,
              const float /*_height*/, const float /*_normX*/,
              const float /*_normY*/, const float /*_normZ*/) {
    fprintf(stderr, "ERROR nmm_Microscope_Remote::RcvResultNM"
	    " obsolete message received.\n");
    // If we need to read stream files with this message, we should
    // translate them into the ResultData message
}

// pulses and num_pulses are globals from openGL.c
// OBSOLETE
void nmm_Microscope_Remote::RcvPulseCompletedNM (const float _xRes, const float _yRes) {

  BCPlane * heightPlane;
  PointType top, bottom;
  //double frac_x, frac_y;
  //long i;

  heightPlane = d_dataset->inputGrid->getPlaneByName
                   (d_dataset->heightPlaneName->string());

  state.lastPulseOK = VRPN_TRUE;

  top[X] = bottom[X] = _xRes;
  top[Y] = bottom[Y] = _yRes;
  top[Z] = heightPlane->maxAttainableValue();
  bottom[Z] = heightPlane->minAttainableValue();

  // MOVED code to nmb_Decoration
  d_decoration->addPulseMark(top, bottom);
}

//OBSOLETE
void nmm_Microscope_Remote::RcvPulseFailureNM (const float, const float) {
  state.lastPulseOK = VRPN_FALSE;
}

// Is the microscope scanning (1), or is the scan paused (0)? 
void nmm_Microscope_Remote::RcvScanning(vrpn_int32 on_off) {
    //printf("Scanning is %s\n", (on_off ? "on": "off"));
    state.scanning = on_off;
}

// XXX
// "Think about what this means when the data sets can be changed"
void nmm_Microscope_Remote::RcvScanRange (const float _minX, const float _maxX,
                               const float _minY, const float _maxY,
                               const float _minZ, const float _maxZ) {
  BCPlane * heightPlane;

  heightPlane = d_dataset->inputGrid->getPlaneByName
                   (d_dataset->heightPlaneName->string());

  heightPlane->setMinAttainableValue(_minZ);
  heightPlane->setMaxAttainableValue(_maxZ);

  printf("Max scan range (%g, %g) to (%g, %g)\n",
         _minX, _minY, _maxX, _maxY);
  printf("  Instrument Z range is %g to %g\n", _minZ, _maxZ);

  state.xMin = _minX;
  state.xMax = _maxX;
  state.yMin = _minY;
  state.yMax = _maxY;
  state.zMin = _minZ;
  state.zMax = _maxZ;

  if (state.doDriftComp)
    driftZDirty();
}

void nmm_Microscope_Remote::RcvReportScanAngle (const float angle ) {
    state.image.scan_angle = Q_RAD_TO_DEG(angle);
    printf( "New scan angle = %g\n", (float)state.image.scan_angle );
}


// case STM_SET_REGION_COMPLETED:
// case STM_SET_REGION_CLIPPED:
void nmm_Microscope_Remote::RcvSetRegionC (const long /* _type */,
                                const float _minX, const float _minY,
                                const float _maxX, const float _maxY) {
  BCPlane * heightPlane;
  BCPlane * p;
  long x, y;

  heightPlane = d_dataset->inputGrid->getPlaneByName
                   (d_dataset->heightPlaneName->string());

  if (state.regionFlag) {
    d_decoration->selectedRegionMinX = d_dataset->inputGrid->minX();
    d_decoration->selectedRegionMinY = d_dataset->inputGrid->minY();
    d_decoration->selectedRegionMaxX = d_dataset->inputGrid->maxX();
    d_decoration->selectedRegionMaxY = d_dataset->inputGrid->maxY();
  } else
    state.regionFlag = VRPN_TRUE;

  d_dataset->inputGrid->setMinX(_minX);
  d_dataset->inputGrid->setMinY(_minY);
  d_dataset->inputGrid->setMaxX(_maxX);
  d_dataset->inputGrid->setMaxY(_maxY);
//    printf( "            nmm_Microscope_Remote::RcvSetRegionC minX %g minY %g maxX %g maxY %g\n", 
//            _minX, _minY, _maxX, _maxY );

  state.modify.region_diag =
     sqrt((d_dataset->inputGrid->maxX() - d_dataset->inputGrid->minX()) *
          (d_dataset->inputGrid->maxX() - d_dataset->inputGrid->minX()) +
          (d_dataset->inputGrid->maxY() - d_dataset->inputGrid->minY()) *
          (d_dataset->inputGrid->maxY() - d_dataset->inputGrid->minY()));

  // initialize splatting filter
  mkSplat(d_dataset->inputGrid);
  if (state.doDriftComp)
    driftZDirty();

  state.current_epoch++;

  // fill in the grid with flat values
  for (p = d_dataset->inputGrid->head(); p; p = p->next())
    for (x = 0; x < p->numX(); x++)
      for (y = 0; y < p->numY(); y++)
        p->setValue(x, y, 0);

  d_decoration->selectedRegion_changed = 1;

  fprintf(stderr, "New region (%g, %g) to (%g, %g)\n",
         heightPlane->minX(), heightPlane->minY(),
         heightPlane->maxX(), heightPlane->maxY());

  d_decoration->red.doCallbacks(heightPlane->minX(), heightPlane->minY(),
                            heightPlane);
  d_decoration->green.doCallbacks(heightPlane->maxX(), heightPlane->minY(),
                            heightPlane);
  d_decoration->blue.doCallbacks(heightPlane->maxX(), heightPlane->maxY(),
                            heightPlane);
  d_decoration->aimLine.moveTo(heightPlane->minX(), heightPlane->maxY(),
                            heightPlane);

  state.SetDefaultScanlineForRegion(d_dataset);

//fprintf(stderr, "region set complete\n");
}

void nmm_Microscope_Remote::RcvResistanceFailure (const long _meter) {
  // DO NOTHING
  fprintf(stderr, "Error reading resistance on meter %ld\n", _meter);
}

void nmm_Microscope_Remote::RcvResistance (const long _meter, const long /* _sec */,
                                const long /* _usec */,
                                const float _resistance) {
  // DO NOTHING
  printf("Resistance on meter %ld is %f\n", _meter, _resistance);

}

void nmm_Microscope_Remote::RcvResistance2(const long _chan, const long /* _sec */,
				const long /* _usec */, const float _res,
				const float _volt, const float _range,
				const float _filt) {
  printf("Ohmmeter: ch %ld, %f mV, range: >%f ohms, filt: %f sec\n",
	_chan, _volt, _range,_filt);
  printf("   measurement: %f ohms\n", _res);
}

void nmm_Microscope_Remote::RcvReportSlowScan (const long _enable) {
  if (state.slowScanEnabled != _enable) {
     state.slowScanEnabled = _enable;
  }
}

void nmm_Microscope_Remote::RcvScanParameters (const char **) {

  // DO NOTHING
  // This is a notification-only message which is handled
  // for now in MicroscopeIO.

}

void nmm_Microscope_Remote::RcvHelloMessage (const char * _magic, const char * _name,
                                  const long _majorVersion,
                                  const long _minorVersion) {
  if (strcmp(_magic, "nM!")) {
    fprintf(stderr, "Bad magic in microscope hello\n");
    fprintf(stderr, "  (expected \"nM!\", got \"%s\"\n", _magic);
    d_dataset->done = VRPN_TRUE;
    return;
  }
  printf("Hello from microscope %s, version %ld.%ld\n", _name, _majorVersion,
         _minorVersion);
}

void nmm_Microscope_Remote::RcvClientHello (const char * _magic, const char * _name,
                                 const long _majorVersion,
                                 const long _minorVersion) {
  if (strcmp(_magic, "nM!")) {
    fprintf(stderr, "Bad magic in client hello\n");
    fprintf(stderr, "  (expected \"nM!\", got \"%s\"\n", _magic);
    d_dataset->done = VRPN_TRUE;
    return;
  }
  printf("Streamfile written by %s, version %ld.%ld\n", _name, _majorVersion,
         _minorVersion);

}

void nmm_Microscope_Remote::RcvClearScanChannels (void) {
  // we use the same messages for 2D and 1D scanning but there is the
  // possibility that in SCANLINE mode we cannot provide all the requested
  // channels so these are temporarily reduced
  if (state.acquisitionMode == SCANLINE) {
      RcvClearScanlineChannels();
  } else {
      if (state.data.scan_channels->Clear_channels()) {
          fprintf(stderr, "nmm_Microscope_Remote::RcvClearScanChannels:"
             " Can't clear scan datasets\n");
          d_dataset->done = VRPN_TRUE;
      }
  }
}

void nmm_Microscope_Remote::RcvScanDataset (const char * _name, 
	const char * _units, const float _offset, const float _scale) {

  // we use the same messages for 2D and 1D scanning but there is the
  // possibility that in SCANLINE mode we cannot provide all the requested
  // channels so these are temporarily reduced
  if (state.acquisitionMode == SCANLINE) {
      RcvScanlineDataset(_name, _units, _offset, _scale);
  } else {
       fprintf(stderr, "  %s (%s), offset:  %g, scale:  %g\n",
               _name, _units, _offset, _scale);
      // HACK HACK HACK
      // Note: units are copied directly from pImg->szWorldUnit
      // on the Thermo side of things. 
      if (state.data.scan_channels->Add_channel((char *) _name,
                                 (char *) _units,_offset, _scale)) {
          fprintf(stderr, "Can't add scan dataset\n");
          d_dataset->done = VRPN_TRUE;
      }
      nmb_Image *new_image = d_dataset->dataImages->getImageByName(_name);
      new_image->setTopoFileInfo(d_topoFile);
  }
}

void nmm_Microscope_Remote::RcvClearPointChannels (void) {
  if (state.data.point_channels->Clear_channels()) {
    fprintf(stderr, "nmm_Microscope_Remote::RcvClearPointChannels: Can't clear point datasets\n");
    d_dataset->done = VRPN_TRUE;
  } else {
      printf("New Point Datasets:\n");
  } 
}

void nmm_Microscope_Remote::RcvPointDataset (const char * _name, const char * _units,
                                  const long _numSamples,
                                  const float _offset, const float _scale) {
  printf("  %s (%s), count:  %ld, offset:  %g, scale:  %g\n",
         _name, _units, _numSamples, _offset, _scale);

  if (state.data.point_channels->Add_channel((char *) _name, (char *) _units,
                                        _offset, _scale,_numSamples)) {
    fprintf(stderr, "Can't add point dataset\n");
    d_dataset->done = VRPN_TRUE;
  }
}

void nmm_Microscope_Remote::RcvPidParameters (const float _p, const float _i,
                                   const float _d) {
  printf("Feedback:  P=%g, I=%g, D=%g\n", _p, _i, _d);
  if (state.acquisitionMode == MODIFY) {
    state.modify.p_gain = _p;
    state.modify.i_gain = _i;
    state.modify.d_gain = _d;
  } else {
    state.image.p_gain = _p;
    state.image.i_gain = _i;
    state.image.d_gain = _d;
  }
}

void nmm_Microscope_Remote::RcvScanrateParameter (const float _rate) {
  if (state.acquisitionMode == MODIFY) {
	printf("Warning! Rate changed on topo in modify mode (ignoring)\n");
  } else {
	state.image.scan_rate_microns = _rate / 1000.0;
	printf("New scan rate:  %g (nM/s)\n", _rate);
  }
}

int nmm_Microscope_Remote::RcvReportGridSize (const long _x, const long _y) {
  printf("Grid size from scanner:  %ldx%ld\n", _x, _y);
  if ((_x != d_dataset->inputGrid->numX()) ||
      (_y != d_dataset->inputGrid->numY())) {
//      fprintf(stderr, "Reset grid size from %d %d!\n",
//  	    d_dataset->inputGrid->numX(),
//  	    d_dataset->inputGrid->numY());
    if (d_dataset->setGridSize(_x, _y)) {
	// Non-zero indicates error!
	fprintf(stderr, "ERROR: unable to reset grid size\n");
	// If we don't set exit flag here, we just get a bus error later.
	d_dataset->done = VRPN_TRUE;
	// New strategy, so don't abruptly exit...
	//exit(-1); // we get a bus error before the next iteration, so exit now.
    }
    // update the user interface. 
    state.image.grid_resolution = _x;
  }
  return 0;
}

void nmm_Microscope_Remote::RcvServerPacketTimestamp (const long, const long) {
  // DO NOTHING
  // This message is for use by networking code
}

void nmm_Microscope_Remote::RcvTopoFileHeader (const long _length, const char *header) {
    //printf("********** RCV'D TOPO FILE HEADER **********\n");
  if(_length < 1536){
	printf("Unexpected Header length %ld need at least 1536\n",_length);
  } else {
      d_topoFile.parseHeader(header,_length);
      //printf("********** Got Topometrix file header, length %ld\n", _length);

      nmb_ImageList *images = d_dataset->dataImages;
      nmb_Image* img;
      // Find the data type based on the plane name. 
      // Based on table in topo/fileinfo.c, switch (doc.sScanParam.iDataType) 
      switch (d_topoFile.iDataType) {
      case ZDATA_LINEARIZED_Z:
          if(d_topoFile.iDataDir == DIR_FWD) img = images->getImageByName( "Topography-Forward");
          else img = images->getImageByName( "Topography-Reverse");
          break;
      case ZDATA_SENSOR:
          if(d_topoFile.iDataDir == DIR_FWD) img = images->getImageByName( "Internal Sensor-Forward") ;
          else img = images->getImageByName( "Internal Sensor-Reverse") ;
          break;
      case ZDATA_SPECT:
          if(d_topoFile.iDataDir == DIR_FWD) img = images->getImageByName( "Z Modulation-Forward");
          else img = images->getImageByName( "Z Modulation-Reverse");
          break;
      case ZDATA_FFM:
          if(d_topoFile.iDataDir == DIR_FWD) img = images->getImageByName( "Lateral Force-Forward");
          else img = images->getImageByName( "Lateral Force-Reverse");
          break;
      case ZDATA_IN1:
          if(d_topoFile.iDataDir == DIR_FWD) img = images->getImageByName( "IN 1-Forward");
          else img = images->getImageByName( "IN 1-Reverse");
          break;
      case ZDATA_IN2:
          if(d_topoFile.iDataDir == DIR_FWD) img = images->getImageByName( "IN 2-Forward");
          else img = images->getImageByName( "IN 2-Reverse");
          if (!img) {
              if(d_topoFile.iDataDir == DIR_FWD) img = images->getImageByName( "Phase-Forward");
              else img = images->getImageByName( "Phase-Reverse");
          }
          break;
      case ZDATA_HEIGHT_ERROR:
          if(d_topoFile.iDataDir == DIR_FWD) img = images->getImageByName( "FastTrack-Forward");
          else img = images->getImageByName( "FastTrack-Reverse");
          break;
      case ZDATA_HEIGHT:
          if(d_topoFile.iDataDir == DIR_FWD) img = images->getImageByName( "Z Piezo-Forward");
          else img = images->getImageByName( "Z Piezo-Reverse");
          break;
      default:
          printf("Warning: unrecognized data type in ThermoMicro file header message.\n");
      }       
      if (img) img->setTopoFileInfo(d_topoFile);
      else printf("Warning: unable to match ThermoMicro file header to data plane, discarding.\n");
      /*	handle=fopen("temp.tfr","w");
                if(handle == NULL){printf("ERROR WRITING TEMP.TFR");}
                fHdl=fileno(handle);
                write(fHdl,header,_length*sizeof(char));	
                fclose(handle);
      */
  }
}


void nmm_Microscope_Remote::RcvForceCurveData(float _x, float _y, 
    long _sec, long _usec, long _num_points, 
    long _num_halfcycles, const float * _z_values, const float **_curves){

  long i,j;
  Point_results pnt;
  if (spm_verbosity >= 1) {
    printf("Force Curve result, at (%g, %g), time %ld:%ld\n", _x,_y,_sec,_usec);
    printf("  values:");
    for (i = 0; i < _num_points; i++){
      printf("z=%f:\t", _z_values[i]);
      for (j = 0; j < _num_halfcycles; j++)
	printf("%f\t", _curves[j][i]);
      printf("\n");
    }
  }

  // Make the report. This has the side effect of updating
  // the fc_inputPoint values.
  // HACK HACK HACK
  for (i = 0; i < _num_halfcycles; i++){
    for (j = 0; j < _num_points; j++){
      if (state.data.forcecurve_channels->Handle_report(_x,_y, _z_values[j], 
			_sec, _usec, (float *)&(_curves[i][j]), 1))
	fprintf(stderr, "Error handling SPM force curve result data\n");
      doPointDataCallbacks(state.data.fc_inputPoint);
    }
  }
  // draw graphical representation
  DisplayModResult(state.data.fc_inputPoint->x(),
		   state.data.fc_inputPoint->y(),
		   0.0f, NULL, VRPN_FALSE);

}

// updates user interface
void nmm_Microscope_Remote::RcvInScanlineMode(const long enabled){
  if (enabled){
    printf("In scanline mode\n");
    state.acquisitionMode = SCANLINE;
    doScanlineModeCallbacks();
    d_decoration->mode = nmb_Decoration::SCANLINE;
  }
  else {
    printf("exited scanline mode (proper scan region should be restored)\n");
    ImageMode();
  }
}

// updates user interface and data object
void nmm_Microscope_Remote::RcvClearScanlineChannels (void) {
  // update user interface:
// this was taken out because channels are shared with those
// used for image mode
/*  if (state.data.scanline_channels->Clear_channels()){
    fprintf(stderr, "Can't clear scanline channel selector\n");
    d_dataset->done = VRPN_TRUE;
  }
*/
  // update data storage
  if (state.data.currentScanlineData.clearChannels()){
    fprintf(stderr, "Can't clear scanline datasets\n");
    d_dataset->done = VRPN_TRUE;
  }
}

// updates user interface and data object
void nmm_Microscope_Remote::RcvScanlineDataset(const char * _name, 
	const char * _units, const float /*_offset*/, const float /*_scale*/) {
//  printf("RcvScanlineDataset:  %s (%s), offset:  %g, scale:  %g\n",
//         _name, _units, _offset, _scale);
// this was taken out because channels are shared with those
// used for image mode
/*  if (state.data.scanline_channels->Add_channel((char *)_name, (char *)_units,
                                        _offset, _scale) < 0) {
    fprintf(stderr, "Can't add scanline channel to selector\n");
    d_dataset->done = VRPN_TRUE;
  }
*/

  if (state.data.currentScanlineData.addChannel(_name, _units)) {
    fprintf(stderr, "Can't add scanline dataset\n");
    d_dataset->done = VRPN_TRUE;
  }
}

// updates data object
void nmm_Microscope_Remote::RcvScanlineDataHeader(const float _x, 
                const float _y, const float _z, 
                const float _angle, const float /*_slope*/,
		const float _width, const long _resolution, 
		const long _enable_feedback, const long _check_forcelimit,
		const float _max_force, const float _max_z_step, 
		const float _max_xy_step,
        const long _sec, const long _usec,
        const long _num_channels) {

    if (state.scanline.continuous_rescan)
        AcquireScanline();
    else
        ExitScanlineMode();

    printf("got scanline data header\n");
    state.scanline.num_scanlines_to_receive--;

    if (state.scanline.num_scanlines_to_receive < 0) {
        fprintf(stderr, "Warning: received more scanlines than expected\n");
        state.scanline.num_scanlines_to_receive = 0;
    }

    if (state.data.currentScanlineData.num_values() != _num_channels) {
        fprintf(stderr, "Error: scanline header has wrong number of channels:");
        fprintf(stderr, "  got %ld, expected %d\n",
                _num_channels, state.data.currentScanlineData.num_values());
        return;
    }
    state.data.currentScanlineData.setLength(_resolution);
    state.data.currentScanlineData.setTime(_sec, _usec);
    state.data.currentScanlineData.setEndpoints(_x, _y, _z,
        _x + sin(_angle)*_width, _y + cos(_angle)*_width, _z);


	if (state.scanline.feedback_enabled != _enable_feedback){
		fprintf(stderr, "Warning:");
		if (state.scanline.feedback_enabled) {
			fprintf(stderr, "Scanline: Feedback enabled in user interface"
				"but disabled in currently received data\n");
		}
		else {
			fprintf(stderr, "Scanline: Feedback disabled in user interface"
				"but enabled in currently received data\n");
		}
	}
	//state.scanline.feedback_enabled = _enable_feedback;
	state.scanline.forcelimit_enabled = _check_forcelimit;
	state.scanline.forcelimit = _max_force;
	state.scanline.max_z_step = _max_z_step;
	state.scanline.max_xy_step = _max_xy_step;

}

// converts from DAC units to the appropriate units for each channel and
// calls callbacks to handle the new data

void nmm_Microscope_Remote::RcvScanlineData(const long _point, 
	const long _num_channels, const float * _value) {
    if (state.data.currentScanlineData.num_values() != _num_channels) {
        fprintf(stderr, "Error: scanline data has wrong number of channels:");
        fprintf(stderr, "  got %ld, expected %d\n",
                _num_channels, state.data.currentScanlineData.num_values());
        return;
    }
    float value;
    for (int i = 0; i < _num_channels; i++){
        value = state.data.scan_channels->DAC_to_units(i, _value[i]);
        state.data.currentScanlineData.setValue(_point, i, value);
    }

    // if this is the last point in the scanline then do the callbacks
    if (_point == state.data.currentScanlineData.length() - 1){
        doScanlineDataCallbacks(&(state.data.currentScanlineData));
	d_decoration->mode = nmb_Decoration::SCANLINE;
    }
}


void nmm_Microscope_Remote::RcvMaxSetpointExceeded (void) 
{
   fprintf(stderr, "### Max Setpoint Exceeded in Direct Z Control ###\n");
}

void nmm_Microscope_Remote::RcvRecvTimestamp (const struct timeval) {
  // DO NOTHING
}

void nmm_Microscope_Remote::RcvFakeSendTimestamp (const struct timeval) {
  // DO NOTHING
}

void nmm_Microscope_Remote::RcvUdpSeqNum (const long) {
  // DO NOTHING
}


void nmm_Microscope_Remote::doImageModeCallbacks (void) {
  modeHandlerEntry * l;

  //fprintf(stderr, "nmm_Microscope_Remote::doImageModeCallbacks\n");  // Tiger

  // Force decoration->elapsedTime to be updated, so callbacks
  // can use it. 
  getTimeSinceConnected();
  l = d_imageModeHandlers;
  while (l) {
    if ((l->handler)(l->userdata)) {
      fprintf(stderr, "nmm_Microscope_Remote::doImageModeCallbacks:  "
                      "Nonzero return value.\n");
      return;
    }
    l = l->next;
  }

}

void nmm_Microscope_Remote::doModifyModeCallbacks (void) {
  modeHandlerEntry * l;

  //fprintf(stderr, "nmm_Microscope_Remote::doModifyModeCallbacks\n");

  // Force decoration->elapsedTime to be updated, so ModFile callback
  // can use it. 
  getTimeSinceConnected();
  l = d_modifyModeHandlers;
  while (l) {
    if ((l->handler)(l->userdata)) {
      fprintf(stderr, "nmm_Microscope_Remote::doModifyModeCallbacks:  "
                      "Nonzero return value.\n");
      return;
    }
    l = l->next;
  }


}

void nmm_Microscope_Remote::doPointDataCallbacks (const Point_results * p) {
  pointDataHandlerEntry * l;

  // Force decoration->elapsedTime to be updated, so callbacks
  // can use it. 
  getTimeSinceConnected();
  l = d_pointDataHandlers;
  while (l) {
    if ((l->handler)(l->userdata, p)) {
      fprintf(stderr, "nmm_Microscope_Remote::doPointDataCallbacks:  "
                      "Nonzero return value.\n");
      return;
    }
    l = l->next;
  }


}

void nmm_Microscope_Remote::doScanlineModeCallbacks (){
  modeHandlerEntry * l;

  //fprintf(stderr, "Microscope::doScanlineModeCallbacks\n");

  // Force decoration->elapsedTime to be updated, so callbacks
  // can use it. 
  getTimeSinceConnected();
  l = d_scanlineModeHandlers;
  while (l) {
    if ((l->handler)(l->userdata)) {
      fprintf(stderr, "Microscope::doScanlineModeCallbacks:  "
                      "Nonzero return value.\n");
      return;
    }
    l = l->next;
  }
}

void nmm_Microscope_Remote::doScanlineDataCallbacks (const Scanline_results *s)
{
  scanlineDataHandlerEntry * l;

  // Force decoration->elapsedTime to be updated, so ModFile callback
  // can use it. 
  getTimeSinceConnected();
  l = d_scanlineDataHandlers;
  while (l) {
    if ((l->handler)(l->userdata, s)) {
      fprintf(stderr, "Microscope::doScanlineDataCallbacks:  "
                      "Nonzero return value.\n");
      return;
    }
    l = l->next;
  }
}

// static 
int nmm_Microscope_Remote::handle_barrierSynch (void *ud, 
                                  const nmb_SynchMessage *msg)
{
   nmm_Microscope_Remote *me = (nmm_Microscope_Remote *)ud;
//   printf("got barrier synch message for slow line(?)\n");
   if (strcmp(msg->comment, RELAX_MSG) == 0) {
      if (me->state.modify.tool != SLOW_LINE) {
         fprintf(stderr, "nmm_Microscope::handle_barrierSynch: Error, not in"
                         " slow line mode\n");
         return 0;
      }
      float x1 =  me->state.modify.slow_line_prevPt->x();
      float y1 =  me->state.modify.slow_line_prevPt->y();
      float x2 =  me->state.modify.slow_line_currPt->x();
      float y2 =  me->state.modify.slow_line_currPt->y();

      float x = x2*(me->state.modify.slow_line_position_param) +
              x1*(1.0-me->state.modify.slow_line_position_param);
      float y = y2*(me->state.modify.slow_line_position_param) +
              y1*(1.0-me->state.modify.slow_line_position_param);

      // Set yaw so if we sweep it will be perpendicular to the slow-line path. 
      me->state.modify.yaw = atan2((y2 - y1), (x2 - x1)) - M_PI_2;

//      printf("sending first point request of slow line mode\n");
      me->state.modify.slow_line_relax_done = VRPN_TRUE;
      me->TakeModStep(x,y);
   }
   return 0;
}

// static
void nmm_Microscope_Remote::handle_GotMicroscopeControl(void *ud,
    nmb_SharedDevice_Remote * /*dev*/)
{
  nmm_Microscope_Remote *me = (nmm_Microscope_Remote *)ud;
  
  //printf("nmm_Microscope_Remote::Got control, sending initialization\n");
  // Send off the relaxation parameters (if any)
  if (me->state.doRelaxComp) {
      me->SetRelax(me->state.stmRxTmin, me->state.stmRxTsep);
  } else {
      me->SetRelax(0, 0);
  }

  me->QueryScanRange();

  // Tell AFM to scan forward and backward, or just forward.
  me->SetScanStyle();

  // Start scanning the surface
  me->ResumeFullScan();

  return;
}
