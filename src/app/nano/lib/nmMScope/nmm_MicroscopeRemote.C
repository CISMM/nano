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
#if !defined(_WIN32)
#include <sys/time.h> // for RecordResistance()
#include <unistd.h>  // for sleep()
#endif

#include <vrpn_FileConnection.h>	// for vrpn_File_Connection class
#include <vrpn_RedundantTransmission.h>

#include <Topo.h>
#include <Point.h>
#include <BCPlane.h>
#include <BCGrid.h>
#include <nmb_Dataset.h>
#include <nmb_Decoration.h>  // for addScrapeMark()
#include <Tcl_Linkvar.h>
#include <nmb_Time.h>
#include <nmb_Debug.h>
#include <nmb_Types.h>
#include <nmb_Line.h>

#include "stm_cmd.h"  // for SPM_POINT_RESULT_DATA and other types
#include "drift.h"
#include "splat.h"
#include "nmm_RelaxComp.h"
#include "nmm_Sample.h"

#include <vrpn_Connection.h>
#include <vrpn_FileConnection.h>	// for vrpn_File_Connection class

#include "error_display.h"

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

#if (!defined(X) || !defined(Y) || !defined(Z1))
#define	X	(0)
#define	Y	(1)
#define	Z	(2)
#endif

#ifndef M_PI
#define M_PI 3.141592653589793238
#define M_PI_2		1.57079632679489661923
#endif

#define FC_MAX_HALFCYCLES (100)


nmm_Microscope_Remote::~nmm_Microscope_Remote (void) {
  // Shut the server down nicely
  // Check to make sure we are talking to live microscope.

  if (d_connection) {
      //XXX Unregister handlers
      unregisterSynchHandler(handle_barrierSynch, this);
      unregisterGotMutexCallback(this, handle_GotMicroscopeControl);
  }

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
  if (d_redundancy) {
    delete d_redundancy;
  }
  if (d_redReceiver) {
    delete d_redReceiver;
  }
  if (d_monitor) {
    delete d_monitor;
  }
  if (d_tsList) {
    delete d_tsList;
  }

  if (graphmod) {
    delete graphmod;
  }

}




// virtual
int nmm_Microscope_Remote::mainloop (void) {

  timeval skiptime;
  timeval last_time;

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

  if (d_redundancy) {
    // Best to call this *before* connection::mainloop
    // Wish we didn't have to worry about that...  How to rearchitect?
    d_redundancy->mainloop();
  }

  if (d_connection) {
    d_connection->mainloop();
  }

  if (d_monitor) {
    // Best to call this *after* connection::mainloop
    // Wish we didn't have to worry about that...  How to rearchitect?
    d_monitor->mainloop();
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

// Thirdtech Initialize Routine
long nmm_Microscope_Remote::Initialize (void) {

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
  //s->setMicroscope(this);
  d_sampleAlgorithm = s;
}


long nmm_Microscope_Remote::ModifyMode (void) {

  CHECK(MarkModifyMode());  // Put this event in output log

  CHECK(SetRateNM(state.modify.scan_rate_microns * 1000.0));

  if ((state.modify.style != SEWING) && (state.modify.style != FORCECURVE)) {
     if (state.modify.control != DIRECTZ) {
	switch (state.modify.mode) {
	case TAPPING:
	   return EnterOscillatingMode(state.modify.p_gain, state.modify.i_gain,
                                state.modify.d_gain, state.modify.setpoint,
                                       state.modify.amplitude,
                                       state.modify.frequency,
                                       state.modify.input_gain,
                                       state.modify.drive_attenuation,
                                       state.modify.phase,
				       state.modify.ampl_or_phase );
	case GUARDED_SCAN:
		  return EnterGuardedScanMode(state.image.p_gain,
                              state.modify.i_gain,
                              state.modify.d_gain,
                              state.modify.setpoint,
			      state.guardedscan.fNormalX,
			      state.guardedscan.fNormalY,
			      state.guardedscan.fNormalZ,
			      state.guardedscan.fPlaneD,
			      state.guardedscan.fGuardDepth,
			     	state.guardedscan.nChannel,
				state.guardedscan.bDirection);
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

  CHECK(MarkImageMode());  // Put this event in output log

  CHECK(SetRateNM(state.image.scan_rate_microns * 1000.0));

  switch (state.image.mode) {
    case TAPPING:
      return EnterOscillatingMode(state.image.p_gain,
                              state.image.i_gain,
                              state.image.d_gain,
                              state.image.setpoint,
                                  state.image.amplitude,
                                  state.image.frequency,
                                  state.image.input_gain,
                                  state.image.drive_attenuation,
                                  state.image.phase,
				  state.image.ampl_or_phase );
    case CONTACT:
      return EnterContactMode(state.image.p_gain,
                              state.image.i_gain,
                              state.image.d_gain,
                              state.image.setpoint);

	case GUARDED_SCAN:
	  return EnterGuardedScanMode(state.image.p_gain,
                                      state.image.i_gain,
                                      state.image.d_gain,
                                      state.image.setpoint,
                                      state.guardedscan.fNormalX,
                                      state.guardedscan.fNormalY,
                                      state.guardedscan.fNormalZ,
                                      state.guardedscan.fPlaneD,
                                      state.guardedscan.fGuardDepth,
                                      state.guardedscan.nChannel,
                                      state.guardedscan.bDirection);
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



long nmm_Microscope_Remote::rotateScanCoords (double _x, double _y,
					      double _scanAngle, 
					      double * out_x, double * out_y) 
{

  // Rotate about the center of the scan region -- same as
  // the Thermo software rotates it's scan when we send it a 
  // particular scan angle. 
    double sin_angle = sin(Q_DEG_TO_RAD(-_scanAngle));
    double cos_angle = cos(Q_DEG_TO_RAD(-_scanAngle));

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

long nmm_Microscope_Remote::DrawLine (double _startx, double _starty,
                          double _endx, double _endy,
                          Point_value * _point, vrpn_bool _awaitResult) {
  char * msgbuf = NULL;
  vrpn_int32 len;
  long type;
  long retval;

  double startx, starty;
  rotateScanCoords(_startx, _starty, (double)(state.image.scan_angle), &startx, &starty);
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
      if (d_monitor) {
        d_monitor->mainloop();
      }
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





long nmm_Microscope_Remote::DrawArc (double _x, double _y,
                         double _startAngle, double _endAngle,
                         Point_value * _point, vrpn_bool _awaitResult) {
  char * msgbuf = NULL;
  vrpn_int32 len;
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
      if (d_monitor) {
        d_monitor->mainloop();
      }
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




long nmm_Microscope_Remote::ScanTo (float _x, float _y) {
  char * msgbuf;
  vrpn_int32 len;

  double x,y;
  rotateScanCoords(_x, _y, (double)(state.image.scan_angle), &x, &y);

  msgbuf = encode_ScanTo(&len, x, y);
  if (!msgbuf)
    return -1;

  // TCH network adaptations Nov 2000

  // Note a message send for application-level loss tracking.
  // Send the message off, UDP/redundantly if that's enabled,
  // TCP otherwise.

  if (d_tsList) {
    d_tsList->markSend();
  }

  return dispatchRedundantMessage(len, msgbuf, d_ScanTo_type);
}

long nmm_Microscope_Remote::ScanTo (float _x, float _y, float _z) {
  char * msgbuf;
  vrpn_int32 len;

  double x,y;
  rotateScanCoords(_x, _y, (double)(state.image.scan_angle), &x, &y);

  msgbuf = encode_ScanToZ(&len, x, y, _z);
  if (!msgbuf)
    return -1;

  return dispatchMessage(len, msgbuf, d_ScanToZ_type);
}

int nmm_Microscope_Remote::TakeSampleSet (float _x, float _y) {
  char * msgbuf;
  vrpn_int32 len;
  double x, y;
  int nx, ny;
  double dx, dy;
  double ori;

  if (!d_sampleAlgorithm) {
    fprintf(stderr, "nmm_Microscope_Remote::TakeSampleSet:  "
            "unspecified sample algorithm, defaulting to 1x1.\n");
    nx = 1;
    ny = 1;
    dx = 0.0;
    dy = 0.0;
    ori = 0.0;
  } else {
    nx = d_sampleAlgorithm->numx;
    ny = d_sampleAlgorithm->numy;
    dx = d_sampleAlgorithm->dx;
    dy = d_sampleAlgorithm->dy;
    ori = d_sampleAlgorithm->orientation;
  }

  rotateScanCoords(_x, _y, (double)(state.image.scan_angle), &x, &y);

  msgbuf = encode_FeelTo(&len, x, y, nx, ny, dx, dy, ori);
  if (!msgbuf) {
    return -1;
  }

  return dispatchMessage(len, msgbuf, d_FeelTo_type);
}


long nmm_Microscope_Remote::TakeFeelStep (float _x, float _y,
                              Point_value * _point,
                              vrpn_bool _awaitResult) {

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
      if (d_monitor) {
        d_monitor->mainloop();
      }
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




long nmm_Microscope_Remote::TakeModStep (float _x, float _y,
                             Point_value * _point,
                             vrpn_bool _awaitResult) {

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
      if (d_monitor) {
        d_monitor->mainloop();
      }
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

int nmm_Microscope_Remote::TakeDirectZStep (float _x, float _y, float _z,
                             Point_value * _point,
                             vrpn_bool _awaitResult) {

  // Don't rotate coords, because ScanTo rotates.
   CHECK(ScanTo(_x, _y, _z));

  if (_awaitResult && _point) {
    // Wait until the tip moves to the destination.
    // We do this by kicking off the read loop until _point (which is
    //   an alias for the tip's current value) changes to the
    //   desired destination value

    do {
      d_connection->mainloop();
      if (d_monitor) {
        d_monitor->mainloop();
      }
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




long nmm_Microscope_Remote::SetRegionNM (float _minx, float _miny,
                             float _maxx, float _maxy) {
  char * msgbuf;
  vrpn_int32 len;

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
  vrpn_int32 len;
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




long nmm_Microscope_Remote::SetScanWindow (long _minx, long _miny,
                               long _maxx, long _maxy) {
  char * msgbuf;
  vrpn_int32 len;

  msgbuf = encode_SetScanWindow
             (&len, _minx, _miny, _maxx, _maxy);
  if (!msgbuf)
    return -1;

  return dispatchMessage(len, msgbuf, d_SetScanWindow_type);
}

long nmm_Microscope_Remote::SetRateNM (double rate) {
  char * msgbuf;
  vrpn_int32 len;

  msgbuf = encode_SetRateNM(&len, rate);
  if (!msgbuf)
    return -1;

  return dispatchMessage(len, msgbuf, d_SetRateNM_type);
}

long nmm_Microscope_Remote::MarkModifyMode (void) {
  char * msgbuf = NULL;
  vrpn_int32 len = 0;

  return dispatchMessage(len, msgbuf, d_MarkModify_type);
}

long nmm_Microscope_Remote::MarkImageMode (void) {
  char * msgbuf = NULL;
  vrpn_int32 len = 0;

  return dispatchMessage(len, msgbuf, d_MarkImage_type);
}


long nmm_Microscope_Remote::EnterOscillatingMode
        (float p, float i, float d, float set, float amp,
         vrpn_float32 frequency, vrpn_int32 input_gain,
         vrpn_int32 drive_attenuation, vrpn_float32 phase, 
	 vrpn_bool ampl_or_phase ) {
  char * msgbuf;
  vrpn_int32 len;

  msgbuf = encode_EnterOscillatingMode(&len, p, i, d, set, amp,
                                       frequency, input_gain, drive_attenuation, 
					phase, ampl_or_phase );
  if (!msgbuf)
    return -1;

  return dispatchMessage(len, msgbuf, d_EnterOscillatingMode_type);
}

long nmm_Microscope_Remote::EnterContactMode (float p, 
					      float i, float d, float set) {
  char * msgbuf;
  vrpn_int32 len;

  msgbuf = encode_EnterContactMode(&len, p, i, d, set);
  if (!msgbuf)
    return -1;

  return dispatchMessage(len, msgbuf, d_EnterContactMode_type);
}

long nmm_Microscope_Remote::EnterGuardedScanMode (float a_fP, 
					      float a_fI, float a_fD, float a_fSetpoint, 
						  float a_fNormalX, float a_fNormalY, float a_fNormalZ,
						  float a_fPlaneD, float a_fGuardDepth, int a_nChannel, int a_bDirection) {
  char * msgbuf;
  vrpn_int32 len;

  msgbuf = encode_EnterGuardedScanMode(&len, a_fP, a_fI, a_fD, a_fSetpoint,
											a_fNormalX, a_fNormalY, a_fNormalZ,
											a_fPlaneD, a_fGuardDepth, a_nChannel, a_bDirection);
  if (!msgbuf)
    return -1;

	printf("Entering guardedscan mode.\n");
  return dispatchMessage(len, msgbuf, d_EnterGuardedScanMode_type);
}

long nmm_Microscope_Remote::EnterDirectZControl (float _max_z_step, 
				       float _max_xy_step, 
				       float _min_setpoint, 
				       float _max_setpoint, 
				       float _max_lateral_force) {
  char * msgbuf;
  vrpn_int32 len;	

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
  vrpn_int32 len;

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
  vrpn_int32 len;

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
  vrpn_int32 len;

  double x,y;
  rotateScanCoords(_x, _y, (double)(state.image.scan_angle), &x, &y);

  // Need to rotate yaw as well! Subtract the scan angle.
  msgbuf = encode_ZagTo(&len, x, y, yaw-Q_DEG_TO_RAD(state.image.scan_angle), sweepWidth, regionDiag);
  if (!msgbuf)
    return -1;

  return dispatchMessage(len, msgbuf, d_ZagToCenter_type);
}

long nmm_Microscope_Remote::SetRelax (long min, long sep) {
  char * msgbuf;
  vrpn_int32 len;
  
  printf("setRelax, %ld %ld\n", min, sep);
  msgbuf = encode_SetRelax(&len, min, sep);
  if (!msgbuf)
    return -1;

  return dispatchMessage(len, msgbuf, d_SetRelax_type);
}





long nmm_Microscope_Remote::SetGridSize (long _x, long _y) {
  char * msgbuf;
  vrpn_int32 len;

  msgbuf = encode_SetGridSize(&len, _x, _y);
  if (!msgbuf)
    return -1;

  return dispatchMessage(len, msgbuf, d_SetGridSize_type);
}

long nmm_Microscope_Remote::SetScanAngle (float _angle) {
  char * msgbuf;
  vrpn_int32 len;

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




long nmm_Microscope_Remote::SetSlowScan (long _value) {
  char * msgbuf;
  vrpn_int32 len;
  
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
                                       	state.modify.drive_attenuation,
                                       	state.modify.phase,
					state.modify.ampl_or_phase );

      case GUARDED_SCAN:
        return EnterGuardedScanMode(state.modify.p_gain,
                                   state.modify.i_gain,
                                   state.modify.d_gain,
                                   state.modify.setpoint,
                                   state.guardedscan.fNormalX,
                                   state.guardedscan.fNormalY,
                                   state.guardedscan.fNormalZ,
                                   state.guardedscan.fPlaneD,
                                   state.guardedscan.fGuardDepth,
				   state.guardedscan.nChannel,
				   state.guardedscan.bDirection);
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
                                       	state.modify.drive_attenuation,
                                       	state.modify.phase,
					state.modify.ampl_or_phase )) {
            fprintf(stderr, "Error, can't enter oscillating mode\n");
            return -1;
	}
         
        break;
      case GUARDED_SCAN:
	    if(EnterGuardedScanMode(state.modify.p_gain,
                              state.modify.i_gain,
                              state.modify.d_gain,
                              state.modify.setpoint,
							  state.guardedscan.fNormalX,
							  state.guardedscan.fNormalY,
							  state.guardedscan.fNormalZ,
							  state.guardedscan.fPlaneD,
							  state.guardedscan.fGuardDepth,
							state.guardedscan.nChannel,
							state.guardedscan.bDirection)) {
			fprintf(stderr, "Error, can't enter guarded scan mode\n");
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
                           " (not contact, guarded scan, or tapping)\n");
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


  
long nmm_Microscope_Remote::RecordResistance
          (long /*meter*/, timeval /*t*/, float res,
           float /*v*/, float /*r*/, float /*f*/) {
//    char * msgbuf;
//    vrpn_int32 len;

    // Message doesn't do anything, and is ignored by Topo, so don't send
//    msgbuf = encode_RecordResistance(&len, meter, t, res, v, r, f);
//    if (!msgbuf)
//      return -1;

  // HACK - this is a fix to get resistance values into the modfile
  lastResistanceReceived = res;

  //  return dispatchMessage(len, msgbuf, d_RecordResistance_type);
  return 0;
}

int nmm_Microscope_Remote::getTimeSinceConnected(void) {

  timeval elapsedTime;

  if( !d_connection ) return 0; // we're not really connected
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

//fprintf(stderr, "nmm_Microscope_Remote::getTimeSinceConnected:  %d sec.\n",
//d_decoration->elapsedTime);

  return 0;
}

long nmm_Microscope_Remote::EnterScanlineMode(){
 
  char * msgbuf;
  vrpn_int32 len;

  msgbuf = encode_EnterScanlineMode(&len, 1);
  if (!msgbuf)
    return -1;
  return dispatchMessage(len, msgbuf, d_EnterScanlineMode_type);
}

long nmm_Microscope_Remote::ExitScanlineMode(){
 
  char * msgbuf;
  vrpn_int32 len;

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
  vrpn_int32 len;

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
                                       	state.scanline.drive_attenuation,
                                       	state.scanline.phase,
					state.scanline.ampl_or_phase );
    case CONTACT:
	return EnterContactMode(state.scanline.p_gain, state.scanline.i_gain,
			state.scanline.d_gain, state.scanline.setpoint);

	case GUARDED_SCAN:
	return EnterGuardedScanMode(state.scanline.p_gain, state.scanline.i_gain,
			state.scanline.d_gain, state.scanline.setpoint,
			state.guardedscan.fNormalX, state.guardedscan.fNormalY, state.guardedscan.fNormalZ,
			state.guardedscan.fPlaneD, state.guardedscan.fGuardDepth, state.guardedscan.nChannel, state.guardedscan.bDirection);

    default:
	return 0;
  }
}

long nmm_Microscope_Remote::JumpToScanLine(long line)
{
  char * msgbuf;
  vrpn_int32 len;
 
  msgbuf = encode_JumpToScanLine(&len, line);

  if (!msgbuf)
    return -1;

  return dispatchMessage(len, msgbuf, d_JumpToScanLine_type);
}





long nmm_Microscope_Remote::QueryScanRange (void) {
  return dispatchMessage(0, NULL, d_QueryScanRange_type);
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

long nmm_Microscope_Remote::EnableUpdatableQueue (vrpn_bool on) {
  char * msgbuf;
  vrpn_int32 len;
 
  msgbuf = encode_EnableUpdatableQueue(&len, on);
  if (!msgbuf) {
    return -1;
  }

  return dispatchMessage(len, msgbuf, d_EnableUpdatableQueue_type);
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

long nmm_Microscope_Remote::registerFeeltoHandler
                          (int (* handler) (void *),
                           void * userdata) {
  feeltoHandlerEntry * newEntry;

  newEntry = new feeltoHandlerEntry;
  if (!newEntry) {
    fprintf(stderr, "nmm_Microscope_Remote::registerFeeltoHandler:  "
                    "Out of memory.\n");
    return -1;
  }

  newEntry->handler = handler;
  newEntry->userdata = userdata;
  newEntry->next = d_feeltoHandlers;

  d_feeltoHandlers = newEntry;

  return 0;
}

long nmm_Microscope_Remote::unregisterFeeltoHandler
                          (int (* handler) (void *),
                           void * userdata) {
  feeltoHandlerEntry * victim, ** snitch;

  snitch = &d_feeltoHandlers;
  victim = *snitch;
  while (victim &&
         (victim->handler != handler) &&
         (victim->userdata != userdata)) {
    snitch = &((*snitch)->next);
    victim = *snitch;
  }

  if (!victim) {
    fprintf(stderr, "nmm_Microscope_Remote::unregisterFeeltoHandler:  "
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

/// Poll the vrpn connection until a point result returns that is close to the supplied arguments.
void nmm_Microscope_Remote::WaitForResult(float a_fX, float a_fY, Point_value* a_pPoint)
{
 do {
      d_connection->mainloop();
      if (d_monitor) {
        d_monitor->mainloop();
      }
      if ( ! d_connection->doing_okay()) {
        fprintf(stderr, "nmm_Microscope_Remote::WaitForResult(x,y):  "
                        "can't read from microscope.\n");
        return;
      }
      getTimeSinceConnected();
      VERBOSE(3, "  Waiting for result in WaitForResult(x,y)");
    } while (!NMB_NEAR(a_fX, a_pPoint->results()->x()) ||
             !NMB_NEAR(a_fY, a_pPoint->results()->y()));
}

/// Poll the vrpn connection until a point result returns that is close to the supplied arguments.
void nmm_Microscope_Remote::WaitForResult(float a_fX, float a_fY, float a_fZ, Point_value* a_pPoint)
{
   do {
      d_connection->mainloop();
      if (d_monitor) {
        d_monitor->mainloop();
      }
      if ( ! d_connection->doing_okay()) {
        fprintf(stderr, "nmm_Microscope_Remote::WaitForResult(x,y,z):  "
                        "can't read from microscope.\n");
        return;
      }
      getTimeSinceConnected();
      VERBOSE(3, "  Waiting for result in WaitForResult(x,y,z)");
    } while (!NMB_NEAR(a_fX, a_pPoint->results()->x()) ||
             !NMB_NEAR(a_fY, a_pPoint->results()->y()) ||
             !NMB_NEAR(a_fZ, a_pPoint->results()->z()));
}



// Common code from by RcvPointResultNM and RcvResultNM, and,
// with a little stretching (_z and _checkZ), RcvResultData

void nmm_Microscope_Remote::DisplayModResult (float _x, float _y,
                                   float _height,
                                   const Point_value * _z,
                                   vrpn_bool _checkZ) {
  PointType top, bottom;
  //double frac_x, frac_y;
  double fx, fy;
  long x, y;
  BCPlane * heightPlane;

  double xr,yr;
  rotateScanCoords(_x, _y, -(double)(state.image.scan_angle), &xr, &yr);

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

  //  printf("currentz: %d\n", state.modify.slow_line_currPt->z());

  // modification markers
  if (state.acquisitionMode == MODIFY) {
    if (((unsigned) x < (unsigned) d_dataset->inputGrid->numX()) &&
        ((unsigned) y < (unsigned) d_dataset->inputGrid->numY())) {
      top[2] = heightPlane->value(x, y);
      if (_checkZ) {
        if (_z) {
          // what if scale changes?
          top[2] = _z->value();// * heightPlane->scale();
        } else {
          top[2] = top[2];
        }
      } else {
        // what if scale changes?
        top[2] = _height;// * heightPlane->scale();
      }
      bottom[2] = top[2];
      //if (glenable)
      //      d_decoration->addScrapeMark(top, bottom);
      d_decoration->addScrapeMark(top, bottom, heightPlane->value(x, y));

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



void nmm_Microscope_Remote::GetRasterPosition (long _x, long _y) {
  // drives X output
  state.rasterX = _x;
  state.rasterY = _y;
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

void nmm_Microscope_Remote::doScanlineModeCallbacks (void) {
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

void nmm_Microscope_Remote::doFeeltoCallbacks (void) {
  feeltoHandlerEntry * l;

  //fprintf(stderr, "Microscope::doFeeltoCallbacks\n");

  // Force decoration->elapsedTime to be updated, so callbacks
  // can use it. 
  getTimeSinceConnected();
  l = d_feeltoHandlers;
  while (l) {
    if ((l->handler)(l->userdata)) {
      fprintf(stderr, "Microscope::doFeeltoCallbacks:  "
                      "Nonzero return value.\n");
      return;
    }
    l = l->next;
  }
}

void nmm_Microscope_Remote::swapPointList (void) {

  int i;

  // Awkward, but it's the interface that's there today.

  state.data.receivedPointList.clear();
  for (i = 0; i < state.data.incomingPointList.numEntries(); i++) {
    state.data.receivedPointList.addEntry
           (*state.data.incomingPointList.entry(i));
  }
  state.data.incomingPointList.clear();

}

