#include "Microscope.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
//#include <sys/time.h>

#include <Point.h>      // Added by Tiger       moved from nmm_Microscope.C
#include <BCPlane.h>
#include <nmb_Dataset.h>
#include <nmb_Decoration.h>  // for addScrapeMark()
#include <Tcl_Linkvar.h>        // Added by Tiger, moved from nmm_Microscope.C
#include <stm_file.h>   // Tiger moved from nmm_Microscope.C, not sure whether
                        // we need it.
#include <nmb_Time.h>
#include <nmb_Debug.h>
#include <nmb_Globals.h>

#include "MicroscopeIO.h"
#include "relax.h"  // relax_set_min, relax_set_max

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

#ifndef M_PI
#define M_PI 3.141592653589793238
#endif

#include <vrpn_Connection.h>


// Microscope
//
// Tom Hudson, September 1997
// Code mostly from microscape.c and animate.c

// All callbacks are in MicroscopeRcv.C



Microscope::Microscope (const AFMInitializationState & i, int replayRate):
    nmm_Microscope ("nmm Microscope AFM Remote"), // Tiger: don't have AFMState in base class now
// Added by Tiger       moved from nmm_Microscope.C
    state(i),
    d_dataset (NULL),
    d_decoration (NULL),
// Tiger
	
    io (new MicroscopeIO (this)),
    d_pointDataHandlers (NULL),
    d_modifyModeHandlers (NULL),
    d_imageModeHandlers (NULL)

{
  gettimeofday(&d_nowtime, &d_nowzone);
  d_next_time.tv_sec = 0L;
  d_next_time.tv_usec = 0L;
  streamReplayRate = replayRate;
}


Microscope::~Microscope (void) {
  // Shut the server down nicely
  // TODO:  make sure we're talking to a live microscope and not using
  //   canned data.  Previous version checked a global variable.  Does
  //   checking IsMicroscopeOpen() work?

  /*
  if (io->IsMicroscopeOpen()) {
    if (io->Shutdown() == -1) {
      fprintf(stderr, "Microscope::~Microscope():  "
                      "could not send quit command to STM server\n");
    }
    // Wait to give the server a chance to receive the message before it
    // gets a connection-closed exception
    sleep(3);
    // io's destructor will do a sdi_disconnect_from_device
    // TODO:  verify that destructors of members get called after
    //   this destructor;  otherwise we've got a mess to handle
  }
  */

  // Call the destructor, so the stream file gets closed!
  if (io) {
    delete io;
  }

  // TODO:  clean up callback lists
}




// Added by Tiger       moved from nmm_Microscope.C
char * Microscope::encode_GetNewPointDatasets
                            (long * len,  // XXX should be vrpn_int32 ???
                             const Tclvar_checklist * list) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;
  long numSets = 0;
  int i;

  if (!len) return NULL;

  for (i = 0; i < list->Num_checkboxes(); i++)	// Tiger changed Num_entries to Num_checkboxes
    if (1 == list->Is_set(i)) numSets++;

  *len = sizeof(long) + (STM_NAME_LENGTH + sizeof(long)) * numSets;
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "Microscope::encode_GetNewPointDatasets:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, numSets);
    for (i = 0; i < list->Num_checkboxes(); i++)  // Tiger changed Num_entries to Num_checkboxes
      if (1 == list->Is_set(i)) {
		// Tiger changed Entry_name to Checkbox_name
        nmb_Util::Buffer(&mptr, &mlen, list->Checkbox_name(i), STM_NAME_LENGTH);

        // Ask for 10 samples of each except Topography;
        // of that we want 90
        if (!strcmp("Topography", list->Checkbox_name(i))) // Tiger changed Entry_name to Checkbox_name
          nmb_Util::Buffer(&mptr, &mlen, (long)90);
        else
          nmb_Util::Buffer(&mptr, &mlen, (long)10);
      }
  }

  return msgbuf;
}
// Tiger

// Added by Tiger       moved from nmm_Microscope.C
char * Microscope::encode_GetNewScanDatasets
                            (long * len,  // XXX should be vrpn_int32 ???
                             const Tclvar_checklist * list) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;
  long numSets = 0;
  long i;

  if (!len) return NULL;
// NANO BEGIN
  fprintf(stderr, "Microscope::encode_GetNewScanDatasets: Entering...\n");
// NANO END

  for (i = 0; i < list->Num_checkboxes(); i++)
    if (1 == list->Is_set(i)) numSets++;

// NANO BEGIN
  fprintf(stderr, "nmm_Microscope_Remote::encode_GetNewScanDatasets: numSets = %ld\n", numSets);
// NANO END
  *len = sizeof(long) + STM_NAME_LENGTH * numSets;
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "Microscope::encode_GetNewScanDatasets:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, numSets);
    for (i = 0; i < list->Num_checkboxes(); i++)
      if (1 == list->Is_set(i))
        nmb_Util::Buffer(&mptr, &mlen, list->Checkbox_name(i), STM_NAME_LENGTH);
  }

  return msgbuf;
}
// Tiger

// Added by Tiger       moved from nmm_Microscope.C
long Microscope::InitializeDataset (nmb_Dataset * ds) {
  BCPlane * plane;

  d_dataset = ds;

  state.data.Initialize(ds);
  plane = ds->ensureHeightPlane();
  state.SetDefaultScanlineForRegion(ds);
  plane->setScale(state.stm_z_scale);

  return 0;
}
// Tiger

// Added by Tiger       moved from nmm_Microscope.C
long Microscope::InitializeDecoration (nmb_Decoration * dec) {
  d_decoration = dec;

  return 0;
}
// Tiger

// Added by Tiger       moved from nmm_Microscope.C
long Microscope::InitializeTcl (const char * dir) {
  if (!dir)
    return -1;

  d_tcl_script_dir = new char [1 + strlen(dir)];
  if (!d_tcl_script_dir)
    return -1;

  strcpy(d_tcl_script_dir, dir);
  return 0;
}
// Tiger


int Microscope::Initialize (const vrpn_bool _setRegion,
                            const vrpn_bool _setMode,
                            int (* f) (stm_stream *),
                            const int _socketType,
                            const char * _SPMhost,
                            const int _SPMport,
                            const int _UDPport) {

  CHECK(InitDevice(_setRegion, _setMode,
                   _socketType, _SPMhost, _SPMport, _UDPport));
  return Init(f);
}




int Microscope::Initialize (int (* f) (stm_stream *)) {

  CHECK(InitStream(state.inputStreamName));
  return Init(f);
}





// These messages were only needed with the old microscope (STM),
// which had to take more samples per point when feeling to reduce
// the noise in the results.
/*
int Microscope::SetSamples (const Microscope::SampleMode _mode) {

  switch (_mode) {
    case Microscope::Haptic:
      state.modify.std_dev_samples_cache = state.modify.std_dev_samples;
      state.modify.std_dev_samples = (int) (state.modify.std_dev_frequency
                                            * 0.001);
      break;
    case Microscope::Visual:
      state.modify.std_dev_samples = state.modify.std_dev_samples_cache;
      break;
  }

  CHECK(SetStdDevParams());

  return 0;
}
*/

//updates streamReplayRate whenever decoration->rateOfTime gets changed 
void Microscope::SetStreamReplayRate(int replayRate) {
    streamReplayRate = replayRate;
}

int Microscope::GetStreamReplayRate() {
    return streamReplayRate;
}

void Microscope::SetStreamToTime(struct timeval time) {
    io->SetStreamToTime(time);
}

// Tells the AFM to resume it's normal scan pattern in Image mode -
// presumably we were feeling or modifying the surface.
//   If 'value' is not NULL, we might do a scan of a small region before
//   resuming. (NOTE: this only available with the older AFM, not the new AFM)

int Microscope::ResumeScan (Point_value *,
                            BCPlane *) {
  CHECK(ImageMode());

  return io->ResumeWindowScan();
}




int Microscope::NewEpoch (void) {
  state.current_epoch++;
  return 0;
}



int Microscope::ModifyMode (void) {

  CHECK(io->SetRateNM(state.modify.scan_rate_microns * 1000.0));

  CHECK(io->MarkModifyMode());  // Put this event in output log

  if ((state.modify.style != SEWING) && (state.modify.style != FORCECURVE)) {
     if (state.modify.control != DIRECTZ) {
	switch (state.modify.mode) {
	case TAPPING:
	   return io->EnterTappingMode(state.modify.p_gain, state.modify.i_gain,
				       state.modify.d_gain, state.modify.setpoint,
				       state.modify.amplitude);
	case CONTACT:
	   return io->EnterContactMode(state.modify.p_gain, state.modify.i_gain,
                                   state.modify.d_gain, state.modify.setpoint);
	default:
	   return 0;  // HACK HACK HACK
	} 
     } else { // Direct Z control
	return io->EnterDirectZControl(state.modify.max_z_step, 
				       state.modify.max_xy_step, 
				       state.modify.min_z_setpoint,
				       state.modify.max_z_setpoint, 
				       state.modify.max_lat_setpoint);
     }
  } else if (state.modify.style == SEWING){
    if (state.modify.mode == TAPPING) {
      fprintf(stderr, "Tapping while sewing is an impossible setting.\n");
      return 0;
    } else if (state.modify.control == DIRECTZ){
      fprintf(stderr, "DirectZ while sewing is an impossible setting.\n");
      return 0;
    } else
      return io->EnterSewingStyle(state.modify.setpoint,
                                 0.001 * state.modify.bot_delay,
                                 0.001 * state.modify.top_delay,
                                 state.modify.z_pull,
                                 state.modify.punch_dist,
                                 1000.0 * state.modify.speed,
                                 state.modify.watchdog);
  }
  else if (state.modify.style == FORCECURVE)
    if (state.modify.control == DIRECTZ){
      fprintf(stderr, "DirectZ during forcecurve is an impossible setting.\n");
      return 0;
    } else
       return io->EnterForceCurveStyle(state.modify.setpoint,
				state.modify.fc_start_delay,
				state.modify.fc_z_start,
				state.modify.fc_z_end,
				state.modify.fc_z_pullback,
				state.modify.fc_force_limit,
				state.modify.fc_movedist,
				state.modify.fc_num_points,
				state.modify.fc_num_halfcycles,
				state.modify.fc_sample_speed,
				state.modify.fc_pullback_speed,
				state.modify.fc_start_speed,
				state.modify.fc_feedback_speed,
				state.modify.fc_avg_num,
				state.modify.fc_sample_delay,
				state.modify.fc_pullback_delay,
				state.modify.fc_feedback_delay);
  else
    fprintf(stderr,"shouldn't be here\n");
  return 0;
}

int Microscope::ImageMode (void) {

  CHECK(io->SetRateNM(state.image.scan_rate_microns * 1000.0));

  CHECK(io->MarkImageMode());  // Put this event in output log
  switch (state.image.mode) {
    case TAPPING:
      return io->EnterTappingMode(state.image.p_gain,
                                 state.image.i_gain,
                                 state.image.d_gain,
                                 state.image.setpoint,
                                 state.image.amplitude);
    case CONTACT:
      return io->EnterContactMode(state.image.p_gain,
                                 state.image.i_gain,
                                 state.image.d_gain,
                                 state.image.setpoint);
  }
  return 0;
}

int Microscope::ExitScanlineMode(void) {
    CHECK(io->EnterScanlineMode(VRPN_FALSE));
    return 0;
}

int Microscope::EnterScanlineMode(void) {
    CHECK(io->EnterScanlineMode(VRPN_TRUE));
	return 0;
}

int Microscope::AcquireScanline (void) {
    printf("requesting scan line - fix this function so it is atomic\n");
    float x, y, z, angle, slope = 0;
	BCPlane *p = d_dataset->inputGrid->getPlaneByName
			(dataset->heightPlaneName->string());

	if (!p) {
		fprintf(stderr, "Microscope::AcquireScanline: Error, could not get"
			" height plane\n");
		return -1;
	}

    state.scanline.getStartPoint(p, &x,&y,&z);
    angle = state.scanline.angle*M_PI/180.0;
    slope = state.scanline.slope_nm_per_micron;
    if (state.acquisitionMode != SCANLINE){
	CHECK(io->EnterScanlineMode(VRPN_TRUE));
    }
    CHECK(io->RequestScanLine(x, y, z, angle, slope, state.scanline.width,
        state.scanline.resolution, state.scanline.feedback_enabled,
		state.scanline.forcelimit_enabled, state.scanline.forcelimit,
		state.scanline.max_z_step, state.scanline.max_xy_step));
    state.scanline.num_scanlines_to_receive++;
    return 0;
}

int Microscope::SetScanlineModeParameters (void) {

  CHECK(io->SetRateNM(state.scanline.scan_rate_microns_per_sec * 1000.0));

  printf("SetScanlineModeParameters: may want to fix the code a bit here\n");
  // we should really be scanning to the start location here before changing
  // force and especially when we are going to be turning off feedback and
  // lifting off the surface - we want to have some idea of the point on 
  // the surface from which our current height is measured

  printf("setting scanline mode parameters\n");
  if (state.scanline.mode == CONTACT) {
	io->EnterContactMode(state.scanline.p_gain,
					state.scanline.i_gain,
					state.scanline.d_gain,
					state.scanline.setpoint);
  } else {	// TAPPING
	io->EnterTappingMode(state.scanline.p_gain,
                                        state.scanline.i_gain,
                                        state.scanline.d_gain,
                                        state.scanline.setpoint,
                                        state.scanline.amplitude);
  }

  return 0;
}

/* AAS - replaced with following functions
// Request point or scan datasets for every active entry in the checklist

int Microscope::GetNewSetOfDataChannels (const int _mode,
                                         const Tclvar_checklist_with_entry * _list) {
  switch (_mode) {
    case SPM_REQUEST_POINT_DATASETS:
      return io->GetNewPointDatasets(_list);
    case SPM_REQUEST_SCAN_DATASETS:
      return io->GetNewScanDatasets(_list);
    default:
      return -1;  // HACK HACK HACK
  }
}
*/

int Microscope::GetNewPointDatasets(const Tclvar_checklist_with_entry * _list)
{
	return io->GetNewPointDatasets(_list);
}

int Microscope::GetNewScanDatasets(const Tclvar_checklist_with_entry * _list)
{
	return io->GetNewScanDatasets(_list);
}

int Microscope::ResumeFullScan (void) {
  return io->SetScanWindow(0, 0, d_dataset->inputGrid->numX() - 1,
                          d_dataset->inputGrid->numY() - 1);
}

int Microscope::ResumeWindowScan (void) {
  return io->ResumeWindowScan();
}

int Microscope::DrawLine (const double _startx, const double _starty,
                          const double _endx, const double _endy,
                          Point_value * _point, const vrpn_bool _awaitResult) {
  switch (state.modify.style) {
    case SHARP:
    case SEWING:
    case FORCECURVE:
      CHECK(io->DrawSharpLine(_startx, _starty, _endx, _endy,
                             state.modify.step_size));
      break;
    case SWEEP:
      CHECK(io->DrawSweepLine(_startx, _starty, _endx, _endy,
                             state.modify.yaw, state.modify.sweep_width,
                             state.modify.step_size));
      break;
    default:
      return 0;
  }

  if (_awaitResult && _point) {
    // Wait until the tip gets there.
    // We do this by kicking off the read loop until _point (which is
    //   an alias for the tip's current value) approaches <endx,endy>

    do {
      if (io->HandlePacket() == -1) {
        fprintf(stderr, "Microscope::DrawLine():  "
                        "can't read from microscope.\n");
        return -1;
      }
      VERBOSE(5, "  Waiting for result in line mode");
    } while (!NMB_NEAR(_endx, _point->results()->x()) ||
             !NMB_NEAR(_endy, _point->results()->y()));
  }

  return 0;
}



int Microscope::DrawArc (const double _x, const double _y,
                         const double _startAngle, const double _endAngle,
                         Point_value * _point, const vrpn_bool _awaitResult) {
  switch (state.modify.style) {
    case SHARP:
    case SEWING:
    case FORCECURVE:
      return 0;
    case SWEEP:
      CHECK(io->DrawSweepArc(_x, _y, _startAngle, _endAngle,
                            state.modify.sweep_width,
                            state.modify.step_size));
      break;
    default:
      return 0;
  }

  if (_awaitResult && _point) {
    // Wait until the tip gets there.
    // We do this by kicking off the read loop until _point (which is
    //   an alias for the tip's current value) approaches <endx,endy>

    do {
      if (io->HandlePacket() == -1) {
        fprintf(stderr, "Microscope::DrawArc():  "
                        "can't read from microscope.\n");
        return -1;
      }
      VERBOSE(5, "  Waiting for result in DrawArc");
    } while (!NMB_NEAR(_x, _point->results()->x()) ||
             !NMB_NEAR(_y, _point->results()->y()));
  }

  return 0;
}




int Microscope::ScanTo (const float _x, const float _y) {
  return io->ScanTo(_x, _y);
}




int Microscope::TakeFeelStep (const float _x, const float _y,
                              Point_value * _point,
                              const vrpn_bool _awaitResult) {
  CHECK(io->ScanTo(_x, _y));

  if (_awaitResult && _point) {
    // Wait until the tip moves.
    // We do this by kicking off the read loop until _point (which is
    //   an alias for the tip's current value) changes from the
    //   bogus value <-1, -1>

    _point->results()->setPosition(-1.0, -1.0);
    do {
      if (io->HandlePacket() == -1) {
        fprintf(stderr, "Microscope::TakeFeelStep():  "
                        "can't read from microscope.\n");
        return -1;
      }
      VERBOSE(5, "  Waiting for result in TakeFeelStep");
    } while (NMB_NEAR(-1.0, _point->results()->x()) &&
             NMB_NEAR(-1.0, _point->results()->y()));
  }

  return 0;
}




int Microscope::TakeModStep (const float _x, const float _y,
                             Point_value * _point,
                             const vrpn_bool _awaitResult) {
  switch (state.modify.style) {
    case SHARP:
    case SEWING:
    case FORCECURVE:
      CHECK(io->ScanTo(_x, _y));
      break;
    case SWEEP:
      CHECK(io->ZagTo(_x, _y, state.modify.yaw, state.modify.sweep_width,
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
      if (io->HandlePacket() == -1) {
        fprintf(stderr, "Microscope::TakeModStep():  "
                        "can't read from microscope.\n");
        return -1;
      }
      VERBOSE(5, "  Waiting for result in TakeModeStep");
    } while (!NMB_NEAR(_x, _point->results()->x()) ||
             !NMB_NEAR(_y, _point->results()->y()));
  }

  return 0;
}

int Microscope::TakeDirectZStep (const float _x, const float _y, const float _z,
                             Point_value * _point,
                             const vrpn_bool _awaitResult) {
   CHECK(io->ScanTo(_x, _y, _z));

  if (_awaitResult && _point) {
    // Wait until the tip moves to the destination.
    // We do this by kicking off the read loop until _point (which is
    //   an alias for the tip's current value) changes to the
    //   desired destination value

    do {
      if (io->HandlePacket() == -1) {
        fprintf(stderr, "Microscope::TakeDirectZStep():  "
                        "can't read from microscope.\n");
        return -1;
      }
      VERBOSE(5, "  Waiting for result in TakeDirectZStep()");
    } while (!NMB_NEAR(_x, _point->results()->x()) ||
             !NMB_NEAR(_y, _point->results()->y()) ||
             !NMB_NEAR(_z, _point->results()->z()));
  }

  return 0;
}

int Microscope::SetRegionNM (const float _minx, const float _miny,
                             const float _maxx, const float _maxy) {
  return io->SetRegionNM(_minx, _miny, _maxx, _maxy);
}


// Figure out the correct scan mode for the STM based on the current
// desired features of the scan mode.
// The modes used here are defined in stm_cmd.h

int Microscope::SetScanStyle (void) {
  int style;

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
  
  return io->SetScanStyle(style);
}




int Microscope::SetScanWindow (const int _minx, const int _miny,
                               const int _maxx, const int _maxy) {
  return io->SetScanWindow(_minx, _miny, _maxx, _maxy);
}




int Microscope::SetGridSize (const int _x, const int _y) {
  return io->SetGridSize(_x, _y);
}




int Microscope::SetSlowScan (const int _value) {
  return io->SetSlowScan(_value);
}


int Microscope::SetModForce () {
   
  if ((state.modify.style != SEWING) && (state.modify.style != FORCECURVE))
    switch (state.modify.mode) {
      case TAPPING:
        return io->EnterTappingMode(state.modify.p_gain, state.modify.i_gain,
                                   state.modify.d_gain, state.modify.setpoint,
                                   state.modify.amplitude);
      case CONTACT:
        return io->EnterContactMode(state.modify.p_gain, state.modify.i_gain,
                                   state.modify.d_gain, state.modify.setpoint);
      default:
        return 0;  // HACK HACK HACK
    }
  else if (state.modify.style == SEWING){
    if (state.modify.mode == TAPPING) {
      fprintf(stderr, "Tapping while sewing is an impossible setting.\n");
      return 0;
    } else
      return io->EnterSewingStyle(state.modify.setpoint,
                                 0.001 * state.modify.bot_delay,
                                 0.001 * state.modify.top_delay,
                                 state.modify.z_pull,
                                 state.modify.punch_dist,
                                 1000.0 * state.modify.speed,
                                 state.modify.watchdog);
  }
  else if (state.modify.style == FORCECURVE)
    return io->EnterForceCurveStyle(state.modify.setpoint,
				state.modify.fc_start_delay,
				state.modify.fc_z_start,
				state.modify.fc_z_end,
				state.modify.fc_z_pullback,
				state.modify.fc_force_limit,
				state.modify.fc_movedist,
				state.modify.fc_num_points,
				state.modify.fc_num_halfcycles,
				state.modify.fc_sample_speed,
				state.modify.fc_pullback_speed,
				state.modify.fc_start_speed,
				state.modify.fc_feedback_speed,
				state.modify.fc_avg_num,
				state.modify.fc_sample_delay,
				state.modify.fc_pullback_delay,
				state.modify.fc_feedback_delay);
  else {
    fprintf(stderr,"shouldn't be here\n");
    return 0;
  }
}




int Microscope::SetBias (const float _bias) {
  return io->SetBias(_bias);
}




int Microscope::SetPulsePeak (const float _height) {
  return io->SetPulsePeak(_height);
}




int Microscope::SetPulseDuration (const float _width) {
  return io->SetPulseDuration(_width);
}




int Microscope::SetOhmmeterSampleRate (const int _rate) {
  return io->SetOhmmeterSampleRate(_rate);
}




int Microscope::EnableVoltsource (const int _which, const float _voltage) {
  return io->EnableVoltsource(_which, _voltage);
}




int Microscope::EnableAmp (const int _which, const float _offset,
                           const float _uncalOffset, const int _gain) {
  return io->EnableAmp(_which, _offset, _uncalOffset, _gain);
}




int Microscope::DisableAmp (const int _which) {
  return io->DisableAmp(_which);

}




int Microscope::DisableVoltsource (const int _which) {
  return io->DisableVoltsource(_which);
}

int Microscope::RecordResistance(int meter, struct timeval t, float res,
			float v, float r, float f, int status) {
  return io->RecordResistance(meter, t, res, v, r, f, status);
}

/* OBSOLETE
int Microscope::SetStdDevParams (const int _samples, const float _freq) {
  state.modify.std_dev_samples = _samples;
  state.modify.std_dev_frequency = _freq;
  return SetStdDevParams();
}



int Microscope::SetStdDevParams (void) {
  return io->SetStdDevParams(state.modify.std_dev_samples,
                            state.modify.std_dev_frequency);
}
*/


int Microscope::QueryScanRange (void) {
  return io->QueryScanRange();
}



/* OBSOLETE
int Microscope::QueryStdDevParams (void) {
  return io->QueryStdDevParams();
}
*/



int Microscope::QueryPulseParams (void) {
  return io->QueryPulseParams();
}






void Microscope::setPlaybackLimit (int limit) {
  d_playbackLimit = limit;
}


int Microscope::HandleReports (void) {

  int packets_played = 0;
  int retval = 0;
  struct timeval skiptime;
  struct timeval last_time;
  extern vrpn_bool stm_new_frame;  // from updt_display.c

  if (stm_new_frame) {

    // Read in the changes
    VERBOSE(5, "   setup");
    state.lost_changes = 0;
    state.new_epoch = VRPN_FALSE;

    last_time = d_nowtime;
    gettimeofday(&d_nowtime, &d_nowzone);
    time_subtract(d_nowtime, last_time, &skiptime);
//    time_multiply(skiptime, d_decoration->rateOfTime, &skiptime);
    time_multiply(skiptime, streamReplayRate, &skiptime);
    time_add(d_next_time, skiptime, &d_next_time);
    VERBOSE(5, "   handling packets with HandlePacket()");

    while ((retval = HandlePacket()) == 1) {
      VERBOSE(5, "    another packet...");
      packets_played++;
      if ((d_playbackLimit > 0) &&
          (packets_played >= d_playbackLimit)) {
//fprintf(stderr, "Played back %d packets;  truncating HandleReports().\n",
//packets_played);
        break;
      }
    }

    if (retval == -1) {
      fprintf(stderr, "Microscope::HandleReports():  "
                      "bad packet.\n");
      d_dataset->done = VRPN_TRUE;
      return -1;  // BUG ???
    }

    stm_new_frame = VRPN_FALSE;
  }

  return retval;
}



// Publically exposed so that ONE function in microscape.c can busy loop
// waiting for the microscope to change / get data back

int Microscope::HandlePacket (void) {

  return io->HandlePacket();

}



int Microscope::InputStreamPosition (void) const {
  return io->InputStreamPosition();
}

int Microscope::OutputStreamPosition (void) const {
  return io->OutputStreamPosition();
}

void Microscope::GetCurrTime (struct timeval * t) const {
  io->GetCurrTime(t);
}

void Microscope::GetStartTime (struct timeval * t) const {
  io->GetStartTime(t);
}

// resets the input stream to the beginning, so we can replay again.
void Microscope::RestartStream () {
  io->RestartStream();
}

// not implemented - see MicroscopeIO.C
void Microscope::SkipInputStream (const Direction _direction) {
  int amount;

  // Skip number of grid points times number of bytes per scan packet
  // (7 items at 4 bytes each)
  // HACK HACK HACK
  // That kind of knowledge should be down in IO

  switch (_direction) {
    case Forward:
      amount = d_dataset->inputGrid->numX() * d_dataset->inputGrid->numY() * 7 * 4;
    case Backward:
      amount = - d_dataset->inputGrid->numX() * d_dataset->inputGrid->numY() * 7 * 4;
  }

  io->SkipInputStream(amount);
}

void Microscope::ResetClock (void) {
  gettimeofday(&d_nowtime, &d_nowzone);
  d_next_time.tv_sec = 0L;
  d_next_time.tv_usec = 0L;
}


int Microscope::registerPointDataHandler
                          (int (* handler) (void *, const Point_results *),
                           void * userdata) {
  pointDataHandlerEntry * newEntry;

  newEntry = new pointDataHandlerEntry;
  if (!newEntry) {
    fprintf(stderr, "Microscope::registerPointDataHandler:  "
                    "Out of memory.\n");
    return -1;
  }

  newEntry->handler = handler;
  newEntry->userdata = userdata;
  newEntry->next = d_pointDataHandlers;

  d_pointDataHandlers = newEntry;

  return 0;
}

int Microscope::unregisterPointDataHandler
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
    fprintf(stderr, "Microscope::unregisterPointDataHandler:  "
                    "No such handler.\n");
    return -1;
  }

  *snitch = victim->next;
  delete victim;

  return 0;
}

int Microscope::registerModifyModeHandler
                          (int (* handler) (void *),
                           void * userdata) {
  modeHandlerEntry * newEntry;

  newEntry = new modeHandlerEntry;
  if (!newEntry) {
    fprintf(stderr, "Microscope::registerModifyModeHandler:  "
                    "Out of memory.\n");
    return -1;
  }

  newEntry->handler = handler;
  newEntry->userdata = userdata;
  newEntry->next = d_modifyModeHandlers;

  d_modifyModeHandlers = newEntry;

  return 0;
}

int Microscope::unregisterModifyModeHandler
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
    fprintf(stderr, "Microscope::unregisterModifyModeHandler:  "
                    "No such handler.\n");
    return -1;
  }

  *snitch = victim->next;
  delete victim;

  return 0;
}
int Microscope::registerImageModeHandler
                          (int (* handler) (void *),
                           void * userdata) {
  modeHandlerEntry * newEntry;

  newEntry = new modeHandlerEntry;
  if (!newEntry) {
    fprintf(stderr, "Microscope::registerImageModeHandler:  "
                    "Out of memory.\n");
    return -1;
  }

  newEntry->handler = handler;
  newEntry->userdata = userdata;
  newEntry->next = d_imageModeHandlers;

  d_imageModeHandlers = newEntry;

  return 0;
}

int Microscope::unregisterImageModeHandler
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
    fprintf(stderr, "Microscope::unregisterImageModeHandler:  "
                    "No such handler.\n");
    return -1;
  }

  *snitch = victim->next;
  delete victim;

  return 0;
}

int Microscope::registerScanlineModeHandler
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

int Microscope::unregisterScanlineModeHandler
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

int Microscope::registerScanlineDataHandler
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

int Microscope::unregisterScanlineDataHandler
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


int Microscope::InitDevice (const vrpn_bool _setRegion,
                            const vrpn_bool _setMode,
                            const int _socketType,
                            const char * _SPMhost,
                            const int _SPMport,
                            const int _UDPport) {
  if (!_SPMhost)
    CHECK(io->InitDevice(state.deviceName));
  if (_SPMhost)
    CHECK(io->InitDevice(_socketType, _SPMhost, _SPMport, _UDPport));


  // This is a live scope, so now we put it in the state in which
  // we want to start.


  // Set the delay parameters for when we change things
  // These are used on the DI code to control keyboard delay
  CHECK(io->SetStdDelay(state.StdDelay));
  CHECK(io->SetStPtDelay(state.StPtDelay));

  // Set the microscope for imaging mode, if we're supposed to.
  if (_setMode)
    CHECK(ImageMode());

  // Send off the relaxation parameters (if any)
  CHECK(io->SetRelax(state.stmRxTmin, state.stmRxTsep));

  // Set the safe speed limit (non-modification)
  CHECK(io->SetMaxMove(state.MaxSafeMove));

  // Ask it for the scan range in x, y, and z.
  // When this is read back, Z will be used to set min_z and max_z.
  CHECK(QueryScanRange());

  // Tell it the size of the region to scan, if we should.
  // The topometrix scanner will tell us where it is scanning when it
  // starts up, so we don't have to set one for it.
  if (_setRegion)
    CHECK(SetRegionNM(d_dataset->inputGrid->minX(),
                      d_dataset->inputGrid->minY(),
                      d_dataset->inputGrid->maxX(),
                      d_dataset->inputGrid->maxY()));

  // Tell it where to scan.
  // (Grid size == window size to begin with)
  CHECK(SetGridSize(d_dataset->inputGrid->numX(), d_dataset->inputGrid->numY()));
  CHECK(SetScanWindow(0, 0, d_dataset->inputGrid->numX() - 1,
                            d_dataset->inputGrid->numY() - 1));

  // Tell it how many samples to take at each point, at what rate
  // Removed TCH 5 May 98 - obsolete (only make sense when we're
  // taking multiple samples at each point)
  //CHECK(SetStdDevParams(state.modify.std_dev_samples,
  //                      state.modify.std_dev_frequency));
  //CHECK(QueryStdDevParams());

  CHECK(SetScanStyle());
  state.SetDefaultScanlineForRegion(d_dataset);

  return 0;
}




int Microscope::InitStream (const char * _inputStreamName) {

  return io->InitStream(_inputStreamName);

}




// Initialization code common to both live and canned data

int Microscope::Init (int (* f) (stm_stream *)) {
  relax_set_min(state.stmRxTmin);
  relax_set_sep(state.stmRxTsep);
  // used for sweep mode
  state.modify.region_diag =
    sqrt(((d_dataset->inputGrid->maxX() - d_dataset->inputGrid->minX()) *
          (d_dataset->inputGrid->maxX() - d_dataset->inputGrid->minX())) +
         ((d_dataset->inputGrid->maxY() - d_dataset->inputGrid->minY()) *
          (d_dataset->inputGrid->maxY() - d_dataset->inputGrid->minY())));

  if (state.writingStreamFile)
    return io->EnableOutputStream(f);

  return 0;
}





// Common code from by RcvPointResultNM and RcvResultNM, and,
// with a little stretching (_z and _checkZ), RcvResultData

void Microscope::DisplayModResult (const float _x, const float _y,
                                   const float _height,
                                   const Point_value * _z,
                                   const vrpn_bool _checkZ) {
  PointType top, bottom;
  //double frac_x, frac_y;
  int x, y;
  BCPlane * heightPlane;

  heightPlane = d_dataset->inputGrid->getPlaneByName
              (dataset->heightPlaneName->string());

  top[0] = bottom[0] = _x;
  top[1] = bottom[1] = _y;

  x = (int) ((_x - d_dataset->inputGrid->minX()) *
                   d_dataset->inputGrid->derangeX());
  y = (int) ((_y - d_dataset->inputGrid->minY()) *
                   d_dataset->inputGrid->derangeY());

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
      } else
        bottom[2] = _height * heightPlane->scale();

      //if (glenable)
      decoration->addScrapeMark(top, bottom);

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



void Microscope::GetRasterPosition (const int _x, const int _y) {
  // drives X output
  state.rasterX = _x;
  state.rasterY = _y;
}




#if 0
//static
nmm_Microscope::CB Microscope::callbacks [] = {
    { "", handle_InTappingMode },
    { "", handle_InContactMode },
    { "", handle_InSewingStyle },
    { "", handle_ForceParameters },
    { "", handle_BaseModParameters },
    { "", handle_ForceSettings },
    { "", handle_VoltsourceEnabled },
    { "", handle_VoltsourceDisabled },
    { "", handle_AmpEnabled },
    { "", handle_AmpDisabled },
    { "", handle_StartingToRelax },
    { "", handle_InModModeT },
    { "", handle_InModMode },
    { "", handle_InImgModeT },
    { "", handle_InImgMode },
    { "", handle_ModForceSet },
    { "", handle_ImgForceSet },
    { "", handle_ModSet },
    { "", handle_ImgSet },
    { "", handle_RelaxSet },
    { "", handle_ForceSet },
    { "", handle_ForceSetFailure },
    { "", handle_PulseParameters },
    //{ "", handle_StdDevParameters },
    { "", handle_WindowLineData },
    { "", handle_WindowScanNM },
    { "", handle_WindowBackscanNM },
    { "", handle_PointResultNM },
    { "", handle_PointResultData },
    { "", handle_BottomPunchResultData },
    { "", handle_TopPunchResultData },
    { "", handle_ResultNM },
    { "", handle_PulseCompletedNM },
    { "", handle_PulseFailureNM },
    { "", handle_ScanRange },
    { "", handle_SetRegionCompleted },
    { "", handle_SetRegionClipped },
    { "", handle_ResistanceFailure },
    { "", handle_Resistance },
    { "", handle_Resistance2},
    { "", handle_ReportSlowScan },
    { "", handle_ScanParameters },
    { "", handle_HelloMessage },
    { "", handle_ClientHello },
    { "", handle_ClearScanChannels },
    { "", handle_ScanDataset },
    { "", handle_ClearPointChannels },
    { "", handle_PointDataset },
    { "", handle_PidParameters },
    { "", handle_ScanrateParameter },
    { "", handle_ReportGridSize },
    { "", handle_ServerPacketTimestamp },
    { "", handle_TopoFileHeader },
    { "", handle_RecvTimestamp },
    { "", handle_FakeSendTimestamp },
    { "", handle_UdpSeqNum },
  { NULL, NULL }
};
#endif


//static
int Microscope::handle_InTappingMode (void * userdata,
                                      vrpn_HANDLERPARAM param) {
  Microscope * ms = (Microscope *) userdata;
  float p, i, d, setpoint, amplitude;

  ms->decode_InTappingMode(&param.buffer, &p, &i, &d, &setpoint, &amplitude);
  ms->RcvInTappingMode(p, i, d, setpoint, amplitude);

  return 0;
}

//static
int Microscope::handle_InContactMode (void * userdata,
                                             vrpn_HANDLERPARAM param) {
  Microscope * ms = (Microscope *) userdata;
  float p, i, d, setpoint;

  ms->decode_InContactMode(&param.buffer, &p, &i, &d, &setpoint);
  ms->RcvInContactMode(p, i, d, setpoint);
  //fprintf(stderr, "======handle_InContactMode: DEBUG: setpoint %f\n", setpoint);
  return 0;
}

//static
int Microscope::handle_InSewingStyle (void * userdata,
                                             vrpn_HANDLERPARAM param) {
  Microscope * ms = (Microscope *) userdata;
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
int Microscope::handle_ForceParameters (void * userdata,
                                             vrpn_HANDLERPARAM param) {
  Microscope * ms = (Microscope *) userdata;
  vrpn_int32 modifyEnable;	// Tiger change int to long
  float scrap;

  ms->decode_ForceParameters(&param.buffer, &modifyEnable, &scrap);
  ms->RcvForceParameters(modifyEnable, scrap);

  return 0;
}

//static
int Microscope::handle_BaseModParameters (void * userdata,
                                              vrpn_HANDLERPARAM param) {
  Microscope * ms = (Microscope *) userdata;
  float min, max;

  ms->decode_BaseModParameters(&param.buffer, &min, &max);
  ms->RcvBaseModParameters(min, max);

  return 0;
}

//static
int Microscope::handle_ForceSettings (void * userdata,
                                             vrpn_HANDLERPARAM param) {
  Microscope * ms = (Microscope *) userdata;
  float min, max, setpoint;

  ms->decode_ForceSettings(&param.buffer, &min, &max, &setpoint);
  ms->RcvForceSettings(min, max, setpoint);
  //  fprintf(stderr, "======handle_ForceSettings: DEBUG: setpoint %f\n", setpoint);

  return 0;
}

//static
int Microscope::handle_VoltsourceEnabled (void * userdata,
                                              vrpn_HANDLERPARAM param) {
  Microscope * ms = (Microscope *) userdata;
  vrpn_int32 which;	// Tiger change int to long
  float voltage;

  ms->decode_VoltsourceEnabled(&param.buffer, &which, &voltage);
  ms->RcvVoltsourceEnabled(which, voltage);

  return 0;
}

//static
int Microscope::handle_VoltsourceDisabled (void * userdata,
                                              vrpn_HANDLERPARAM param) {
  Microscope * ms = (Microscope *) userdata;
  vrpn_int32 which;	// Tiger change int to long

  ms->decode_VoltsourceDisabled(&param.buffer, &which);
  ms->RcvVoltsourceDisabled(which);

  return 0;
}

//static
int Microscope::handle_AmpEnabled (void * userdata, vrpn_HANDLERPARAM param) {
  Microscope * ms = (Microscope *) userdata;
  vrpn_int32 which, gain; 	// Tiger change int to long
  float offset, percentOffset;

  ms->decode_AmpEnabled(&param.buffer, &which, &offset, &percentOffset, &gain);
  ms->RcvAmpEnabled(which, offset, percentOffset, gain);

  return 0;
}

//static
int Microscope::handle_AmpDisabled (void * userdata, vrpn_HANDLERPARAM param) {
  Microscope * ms = (Microscope *) userdata;
  vrpn_int32 which;	// Tiger change int to long

  ms->decode_AmpDisabled(&param.buffer, &which);
  ms->RcvAmpDisabled(which);

  return 0;
}

//static
int Microscope::handle_StartingToRelax (void * userdata,
                                             vrpn_HANDLERPARAM param) {
  Microscope * ms = (Microscope *) userdata;
  vrpn_int32 sec, usec;	// Tiger change int to long

  ms->decode_StartingToRelax(&param.buffer, &sec, &usec);
  ms->RcvStartingToRelax(sec, usec);

  return 0;
}

//static
int Microscope::handle_InModModeT (void * userdata, vrpn_HANDLERPARAM param) {
  Microscope * ms = (Microscope *) userdata;
  vrpn_int32 sec, usec;	// Tiger change int to long

  ms->decode_InModModeT(&param.buffer, &sec, &usec);
  ms->RcvInModModeT(sec, usec);

  return 0;
}

//static
int Microscope::handle_InModMode (void * userdata,
                                  vrpn_HANDLERPARAM) {
  Microscope * ms = (Microscope *) userdata;

  ms->RcvInModMode();

  return 0;
}

//static
int Microscope::handle_InImgModeT (void * userdata, vrpn_HANDLERPARAM param) {
  Microscope * ms = (Microscope *) userdata;
  vrpn_int32 sec, usec;	// Tiger change int to long

  ms->decode_InImgModeT(&param.buffer, &sec, &usec);
  ms->RcvInImgModeT(sec, usec);

  return 0;
}

//static
int Microscope::handle_InImgMode (void * userdata,
                                  vrpn_HANDLERPARAM) {
  Microscope * ms = (Microscope *) userdata;

  ms->RcvInImgMode();

  return 0;
}

//static
int Microscope::handle_ModForceSet (void * userdata, vrpn_HANDLERPARAM param) {
  Microscope * ms = (Microscope *) userdata;
  float force;

  ms->decode_ModForceSet(&param.buffer, &force);
  ms->RcvModForceSet(force);

  return 0;
}

//static
int Microscope::handle_ImgForceSet (void * userdata, vrpn_HANDLERPARAM param) {
  Microscope * ms = (Microscope *) userdata;
  float force;

  ms->decode_ImgForceSet(&param.buffer, &force);
  ms->RcvImgForceSet(force);

  return 0;
}

//static
int Microscope::handle_ModSet (void * userdata, vrpn_HANDLERPARAM param) {
  Microscope * ms = (Microscope *) userdata;
  vrpn_int32 modifyEnable; 	// Tiger change int to long
  float max, min, value;

  ms->decode_ModSet(&param.buffer, &modifyEnable, &max, &min, &value);
  ms->RcvModSet(modifyEnable, max, min, value);

  return 0;
}

//static
int Microscope::handle_ImgSet (void * userdata, vrpn_HANDLERPARAM param) {
  Microscope * ms = (Microscope *) userdata;
  vrpn_int32 modifyEnable;	// Tiger change int to long
  float max, min, value;

  ms->decode_ImgSet(&param.buffer, &modifyEnable, &max, &min, &value);
  ms->RcvImgSet(modifyEnable, max, min, value);

  return 0;
}

//static
int Microscope::handle_RelaxSet (void * userdata, vrpn_HANDLERPARAM param) {
  Microscope * ms = (Microscope *) userdata;
  vrpn_int32 min, sep;	// Tiger change int to long

  ms->decode_RelaxSet(&param.buffer, &min, &sep);
  ms->RcvRelaxSet(min, sep);

  return 0;
}

//static
int Microscope::handle_ForceSet (void * userdata, vrpn_HANDLERPARAM param) {
  Microscope * ms = (Microscope *) userdata;
  float force;

  ms->decode_ForceSet(&param.buffer, &force);
  ms->RcvForceSet(force);

  return 0;
}

//static
int Microscope::handle_ForceSetFailure (void * userdata,
                                             vrpn_HANDLERPARAM param) {
  Microscope * ms = (Microscope *) userdata;
  float force;

  ms->decode_ForceSetFailure(&param.buffer, &force);
  ms->RcvForceSetFailure(force);

  return 0;
}

//static
int Microscope::handle_PulseParameters (void * userdata,
                                             vrpn_HANDLERPARAM param) {
  Microscope * ms = (Microscope *) userdata;
  vrpn_int32 pulseEnabled; 	// Tiger change int to long
  float biasVoltage, peakVoltage, width;

  ms->decode_PulseParameters(&param.buffer, &pulseEnabled, &biasVoltage,
                             &peakVoltage, &width);
  ms->RcvPulseParameters(pulseEnabled, biasVoltage, peakVoltage, width);

  return 0;
}

//static
/* OBSOLETE
int Microscope::handle_StdDevParameters (void * userdata,
                                              vrpn_HANDLERPARAM param) {
  Microscope * ms = (Microscope *) userdata;
  vrpn_int32 samples; 	// Tiger change int to long
  float freq;

  ms->decode_StdDevParameters(&param.buffer, &samples, &freq);
  ms->RcvStdDevParameters(samples, freq);

  return 0;
}
*/
//static
int Microscope::handle_WindowLineData (void * userdata,
                                             vrpn_HANDLERPARAM param) {
  Microscope * ms = (Microscope *) userdata;
  float fields [MAX_CHANNELS];
  vrpn_int32 x, y, dx, dy, sec, usec, lineCount, fieldCount;	// Tiger change int to long

  int i;

  ms->decode_WindowLineDataHeader(&param.buffer, &x, &y, &dx, &dy, &sec,
                                  &usec, &lineCount, &fieldCount);
  for (i = 0; i < lineCount; i++) {
    ms->decode_WindowLineDataField(&param.buffer, fieldCount, fields);
    ms->RcvWindowLineData(x + i * dx, y + i * dy, sec, usec,
                          fieldCount, fields);
  }
  ms->RcvWindowLineData();

  return 0;
}

//static
int Microscope::handle_WindowScanNM (void * userdata,
                                             vrpn_HANDLERPARAM param) {
  Microscope * ms = (Microscope *) userdata;
  vrpn_int32 x, y, sec, usec;	// Tiger change int to long
  float value, deviation;

  ms->decode_WindowScanNM(&param.buffer, &x, &y, &sec, &usec,
                          &value, &deviation);
  ms->RcvWindowScanNM(x, y, sec, usec, value, deviation);

  return 0;
}

//static
int Microscope::handle_WindowBackscanNM (void * userdata,
                                              vrpn_HANDLERPARAM param) {
  Microscope * ms = (Microscope *) userdata;
  vrpn_int32 x, y, sec, usec;	// Tiger change int to long
  float value, deviation;

  ms->decode_WindowBackscanNM(&param.buffer, &x, &y, &sec, &usec,
                              &value, &deviation);
  ms->RcvWindowBackscanNM(x, y, sec, usec, value, deviation);

  return 0;
}

//static
int Microscope::handle_PointResultNM (void * userdata,
                                             vrpn_HANDLERPARAM param) {
  Microscope * ms = (Microscope *) userdata;
  float x, y, height, deviation;
  vrpn_int32 sec, usec;	// Tiger change int to long

  ms->decode_PointResultNM(&param.buffer, &x, &y, &sec, &usec,
                           &height, &deviation);
  ms->RcvPointResultNM(x, y, sec, usec, height, deviation);

  return 0;
}

//static
int Microscope::handle_PointResultData (void * userdata,
                                        vrpn_HANDLERPARAM param) {
  Microscope * ms = (Microscope *) userdata;
  vrpn_int32 sec, usec, fieldCount;	// Tiger change int to long
  float x, y, fields [MAX_CHANNELS];

  ms->decode_ResultData(&param.buffer, &x, &y, &sec, &usec,
                        &fieldCount, fields);
  ms->RcvResultData(SPM_POINT_RESULT_DATA, x, y, sec, usec,
                    fieldCount, fields);

  return 0;
}

//static
int Microscope::handle_BottomPunchResultData (void * userdata,
                                         vrpn_HANDLERPARAM param) {
  Microscope * ms = (Microscope *) userdata;
  vrpn_int32 sec, usec, fieldCount;	// Tiger change int to long
  float x, y, fields [MAX_CHANNELS];

  ms->decode_ResultData(&param.buffer, &x, &y, &sec, &usec,
                        &fieldCount, fields);
  ms->RcvResultData(SPM_BOTTOM_PUNCH_RESULT_DATA, x, y, sec, usec,
                    fieldCount, fields);

  return 0;
}

//static
int Microscope::handle_TopPunchResultData (void * userdata,
                                      vrpn_HANDLERPARAM param) {
  Microscope * ms = (Microscope *) userdata;
  vrpn_int32 sec, usec, fieldCount;	// Tiger change int to long
  float x, y, fields [MAX_CHANNELS];

  ms->decode_ResultData(&param.buffer, &x, &y, &sec, &usec,
                        &fieldCount, fields);
  ms->RcvResultData(SPM_TOP_PUNCH_RESULT_DATA, x, y, sec, usec,
                    fieldCount, fields);

  return 0;
}

//static
int Microscope::handle_ResultNM (void * userdata, vrpn_HANDLERPARAM param) {
  Microscope * ms = (Microscope *) userdata;
  vrpn_int32 sec, usec;	// Tiger change int to long
  float x, y, height, normX, normY, normZ;

  ms->decode_ResultNM(&param.buffer, &x, &y, &sec, &usec,
                      &height, &normX, &normY, &normZ);
  ms->RcvResultNM(x, y, sec, usec, height, normX, normY, normZ);

  return 0;
}

//static
int Microscope::handle_PulseCompletedNM (void * userdata,
                                              vrpn_HANDLERPARAM param) {
  Microscope * ms = (Microscope *) userdata;
  float x, y;

  ms->decode_PulseCompletedNM(&param.buffer, &x, &y);
  ms->RcvPulseCompletedNM(x, y);

  return 0;
}

//static
int Microscope::handle_PulseFailureNM (void * userdata,
                                             vrpn_HANDLERPARAM param) {
  Microscope * ms = (Microscope *) userdata;
  float x, y;

  ms->decode_PulseFailureNM(&param.buffer, &x, &y);
  ms->RcvPulseFailureNM(x, y);

  return 0;
}

//static
int Microscope::handle_ScanRange (void * userdata, vrpn_HANDLERPARAM param) {
  Microscope * ms = (Microscope *) userdata;
  float minX, maxX, minY, maxY, minZ, maxZ;

  ms->decode_ScanRange(&param.buffer, &minX, &maxX, &minY, &maxY, &minZ, &maxZ);
  ms->RcvScanRange(minX, maxX, minY, maxY, minZ, maxZ);

  return 0;
}

//static
int Microscope::handle_SetRegionCompleted (void * userdata,
                                           vrpn_HANDLERPARAM param) {
  Microscope * ms = (Microscope *) userdata;
  float minX, minY, maxX, maxY;

  ms->decode_SetRegionC(&param.buffer, &minX, &minY, &maxX, &maxY);
  ms->RcvSetRegionC(STM_SET_REGION_COMPLETED, minX, minY, maxX, maxY);

  return 0;
}

//static
int Microscope::handle_SetRegionClipped (void * userdata,
                                         vrpn_HANDLERPARAM param) {
  Microscope * ms = (Microscope *) userdata;
  float minX, minY, maxX, maxY;

  ms->decode_SetRegionC(&param.buffer, &minX, &minY, &maxX, &maxY);
  ms->RcvSetRegionC(STM_SET_REGION_CLIPPED, minX, minY, maxX, maxY);

  return 0;
}

//static
int Microscope::handle_ResistanceFailure (void * userdata,
                                              vrpn_HANDLERPARAM param) {
  Microscope * ms = (Microscope *) userdata;
  vrpn_int32 which;	// Tiger change int to long

  ms->decode_ResistanceFailure(&param.buffer, &which);
  ms->RcvResistanceFailure(which);

  return 0;
}

//static
int Microscope::handle_Resistance (void * userdata, vrpn_HANDLERPARAM param) {
  Microscope * ms = (Microscope *) userdata;
  vrpn_int32 which, sec, usec;	// Tiger change int to long
  float resistance;

  ms->decode_Resistance(&param.buffer, &which, &sec, &usec, &resistance);
  ms->RcvResistance(which, sec, usec, resistance);

  return 0;
}

//static
int Microscope::handle_Resistance2(void * userdata, vrpn_HANDLERPARAM param) {
  Microscope * ms = (Microscope *) userdata;
  vrpn_int32 which, sec, usec;	// Tiger change int to long
  float resistance;
  float voltage, range, filter;

  ms->decode_Resistance2(&param.buffer, &which, &sec, &usec, &resistance,
                         &voltage, &range, &filter);
  ms->RcvResistance2(which, sec, usec, resistance, voltage, range, filter);

  return 0;
}

//static
int Microscope::handle_ReportSlowScan (void * userdata,
                                             vrpn_HANDLERPARAM param) {
  Microscope * ms = (Microscope *) userdata;
  vrpn_int32 enable;	// Tiger change int to long

  ms->decode_ReportSlowScan(&param.buffer, &enable);
  ms->RcvReportSlowScan(enable);

  return 0;
}

//static
int Microscope::handle_ScanParameters (void * userdata,
                                             vrpn_HANDLERPARAM param) {
  Microscope * ms = (Microscope *) userdata;
  char * buffer;
  vrpn_int32 length;	// Tiger change int to long

  ms->decode_ScanParameters(&param.buffer, &length, &buffer);
  ms->RcvScanParameters((const char **)&buffer);

  return 0;
}

//static
int Microscope::handle_HelloMessage (void * userdata,
                                             vrpn_HANDLERPARAM param) {
  Microscope * ms = (Microscope *) userdata;
  char magic [4];
  char name [STM_NAME_LENGTH];
  vrpn_int32 majorVersion, minorVersion;	// Tiger change int to long


  ms->decode_HelloMessage(&param.buffer, magic, name,
                          &majorVersion, &minorVersion);
  ms->RcvHelloMessage(magic, name, majorVersion, minorVersion);

  return 0;
}

//static
int Microscope::handle_ClientHello (void * userdata,
                                             vrpn_HANDLERPARAM param) {
  Microscope * ms = (Microscope *) userdata;
  char magic [4];
  char name [STM_NAME_LENGTH];
  vrpn_int32 majorVersion, minorVersion;	// Tiger change int to long

  ms->decode_ClientHello(&param.buffer, magic, name,
                         &majorVersion, &minorVersion);
  ms->RcvClientHello(magic, name, majorVersion, minorVersion);

  return 0;
}

//static
int Microscope::handle_ClearScanChannels (void * userdata,
                                              vrpn_HANDLERPARAM) {
  Microscope * ms = (Microscope *) userdata;

  ms->RcvClearScanChannels();

  return 0;
}

//static
int Microscope::handle_ScanDataset (void * userdata, vrpn_HANDLERPARAM param) {
  Microscope * ms = (Microscope *) userdata;
  char name [STM_NAME_LENGTH];
  char units [STM_NAME_LENGTH];
  float offset;
  float scale;
  vrpn_int32 numDatasets;	// Tiger change int to long
  int i;

  ms->decode_ScanDatasetHeader(&param.buffer, &numDatasets);
  for (i = 0; i < numDatasets; i++) {
    ms->decode_ScanDataset(&param.buffer, name, units, &offset, &scale);
    ms->RcvScanDataset(name, units, offset, scale);
  }

  return 0;
}

//static
int Microscope::handle_ClearPointChannels (void * userdata,
                                              vrpn_HANDLERPARAM) {
  Microscope * ms = (Microscope *) userdata;

  ms->RcvClearPointChannels();

  return 0;
}

//static
int Microscope::handle_PointDataset (void * userdata,
                                             vrpn_HANDLERPARAM param) {
  Microscope * ms = (Microscope *) userdata;
  char name [STM_NAME_LENGTH];
  char units [STM_NAME_LENGTH];
  float offset;
  float scale;
  vrpn_int32 numDatasets;	// Tiger change int to long
  vrpn_int32 numSamples;	// Tiger change int to long
  int i;

  ms->decode_PointDatasetHeader(&param.buffer, &numDatasets);
  for (i = 0; i < numDatasets; i++) {
    ms->decode_PointDataset(&param.buffer, name, units, &numSamples,
                                  &offset, &scale);
    ms->RcvPointDataset(name, units, numSamples, offset, scale);
  }

  return 0;
}

//static
int Microscope::handle_PidParameters (void * userdata,
                                             vrpn_HANDLERPARAM param) {
  Microscope * ms = (Microscope *) userdata;
  float p, i, d;

  ms->decode_PidParameters(&param.buffer, &p, &i, &d);
  ms->RcvPidParameters(p, i, d);

  return 0;
}

//static
int Microscope::handle_ScanrateParameter (void * userdata,
                                              vrpn_HANDLERPARAM param) {
  Microscope * ms = (Microscope *) userdata;
  float rate;

  ms->decode_ScanrateParameter(&param.buffer, &rate);
  ms->RcvScanrateParameter(rate);

  return 0;
}

//static
int Microscope::handle_ReportGridSize (void * userdata,
                                             vrpn_HANDLERPARAM param) {
  Microscope * ms = (Microscope *) userdata;
  vrpn_int32 x, y;	// Tiger change int to long

  ms->decode_ReportGridSize(&param.buffer, &x, &y);
  ms->RcvReportGridSize(x, y);

  return 0;
}

//static
int Microscope::handle_ServerPacketTimestamp (void * userdata,
                                              vrpn_HANDLERPARAM param) {
  Microscope * ms = (Microscope *) userdata;
  vrpn_int32 sec, usec;	// Tiger change int to long

  ms->decode_ServerPacketTimestamp(&param.buffer, &sec, &usec);
  ms->RcvServerPacketTimestamp(sec, usec);

  return 0;
}

//static
int Microscope::handle_TopoFileHeader (void * userdata,
                                             vrpn_HANDLERPARAM param) {
  Microscope * ms = (Microscope *) userdata;
  char * buffer;
  vrpn_int32 length;	// Tiger change int to long

  ms->decode_TopoFileHeader(&param.buffer, &length, &buffer);
  ms->RcvTopoFileHeader(length, buffer);

  return 0;
}


    // messages for Michele Clark's experiments
//static
int Microscope::handle_RecvTimestamp (void * userdata,
                                             vrpn_HANDLERPARAM param) {
  Microscope * ms = (Microscope *) userdata;
  struct timeval t;

  ms->decode_RecvTimestamp(&param.buffer, &t);
  ms->RcvRecvTimestamp(t);

  return 0;
}

//static
int Microscope::handle_FakeSendTimestamp (void * userdata,
                                              vrpn_HANDLERPARAM param) {
  Microscope * ms = (Microscope *) userdata;
  struct timeval t;

  ms->decode_FakeSendTimestamp(&param.buffer, &t);
  ms->RcvFakeSendTimestamp(t);

  return 0;
}

//static
int Microscope::handle_UdpSeqNum (void * userdata, vrpn_HANDLERPARAM param) {
  Microscope * ms = (Microscope *) userdata;
  vrpn_int32 seqnum;	// Tiger change int to long

  ms->decode_UdpSeqNum(&param.buffer, &seqnum);
  ms->RcvUdpSeqNum(seqnum);

  return 0;
}




void Microscope::doImageModeCallbacks (void) {
  modeHandlerEntry * l;

fprintf(stderr, "Microscope::doImageModeCallbacks\n");

  l = d_imageModeHandlers;
  while (l) {
    if ((l->handler)(l->userdata)) {
      fprintf(stderr, "Microscope::doImageModeCallbacks:  "
                      "Nonzero return value.\n");
      return;
    }
    l = l->next;
  }

}

void Microscope::doModifyModeCallbacks (void) {
  modeHandlerEntry * l;

fprintf(stderr, "Microscope::doModifyModeCallbacks\n");

  l = d_modifyModeHandlers;
  while (l) {
    if ((l->handler)(l->userdata)) {
      fprintf(stderr, "Microscope::doModifyModeCallbacks:  "
                      "Nonzero return value.\n");
      return;
    }
    l = l->next;
  }


}

void Microscope::doPointDataCallbacks (const Point_results * p) {
  pointDataHandlerEntry * l;

  l = d_pointDataHandlers;
  while (l) {
    if ((l->handler)(l->userdata, p)) {
      fprintf(stderr, "Microscope::doPointDataCallbacks:  "
                      "Nonzero return value.\n");
      return;
    }
    l = l->next;
  }


}

void Microscope::doScanlineModeCallbacks (void) {
  modeHandlerEntry * l;

  fprintf(stderr, "Microscope::doScanlineModeCallbacks\n");

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

void Microscope::doScanlineDataCallbacks (const Scanline_results *s) 
{
  scanlineDataHandlerEntry * l;

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

