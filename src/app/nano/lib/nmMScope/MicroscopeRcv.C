
#include "Microscope.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

//#include <sound.h>  // ~hmd/beta/include

#include <Topo.h>
#include <BCPlane.h>
#include <nmb_Dataset.h>
#include <nmb_Decoration.h>
#include <nmb_Types.h>
#include <nmb_Debug.h>
#include <nmb_Line.h>

#include "nmm_Types.h"
#include "relax.h"  // relax_set_min, relax_set_max
#include "drift.h"  // driftZDirty
#include "splat.h"  // ptSplat(), mkSplat()

#include "ohmmeter.h"
extern char *tcl_script_dir;
extern Ohmmeter *the_french_ohmmeter_ui;
#include "tcl_tk.h"	// getting interpreter to create ohmmeter window

#define CHECK(a) if ((a) == -1) return -1

#ifdef MAX
  #undef MAX
#endif
#define MAX(a,b) ((a > b) ? (a) : (b))

#ifdef MIN
  #undef MIN
#endif
#define MIN(a,b) ((a < b) ? (a) : (b))
#define NMB_NEAR(x0,x1) (fabs((x0) - (x1)) < 0.001)

//#include "globjects.h"  // make_selected_region_marker

// from microscape.c
extern TopoFile TF;

#if (!defined(X) || !defined(Y) || !defined(Z))
#define	X	(0)
#define	Y	(1)
#define	Z	(2)
#endif


// MicroscopeRcv.C
//
// Tom Hudson, September 1997
// Code from animate.c

// NOTHING BUT CALLBACKS

// Defines how the Microscope object responds to messages arriving
// from the actual microscope

void Microscope::RcvInTappingMode (const float _p, const float _i,
                                   const float _d, const float _setpoint,
                                   const float _amp) {
  if (state.acquisitionMode == MODIFY) {
    printf("Matching AFM modify parameters (Tapping).\n");
    printf("   S=%g  P=%g, I=%g, D=%g, A=%g\n", _setpoint,_p, _i, _d, _amp);
    if (state.modify.mode != TAPPING)
      state.modify.mode = TAPPING;
    state.modify.p_gain = _p;
    state.modify.i_gain = _i;
    state.modify.d_gain = _d;
    state.modify.setpoint = _setpoint;
    d_decoration->modSetpoint = _setpoint;
    state.modify.amplitude = _amp;
  } else if (state.acquisitionMode == IMAGE){
    printf("Matching AFM image parameters (Tapping).\n");
    printf("   S=%g  P=%g, I=%g, D=%g, A=%g\n", _setpoint,_p, _i, _d, _amp);
    if (state.image.mode != TAPPING)
      state.image.mode = TAPPING;
    state.image.p_gain = _p;
    state.image.i_gain = _i;
    state.image.d_gain = _d;
    state.image.setpoint = _setpoint;
    d_decoration->imageSetpoint = _setpoint;
    state.image.amplitude = _amp;
  } else if (state.acquisitionMode == SCANLINE){
    printf("Matching AFM scanline parameters (Tapping).\n");
    printf("   S=%g  P=%g, I=%g, D=%g, A=%g\n", _setpoint,_p, _i, _d, _amp);
    if (state.scanline.mode != TAPPING)
      state.scanline.mode = TAPPING;
    state.scanline.p_gain = _p;
    state.scanline.i_gain = _i;
    state.scanline.d_gain = _d;
    state.scanline.setpoint = _setpoint;
    d_decoration->scanlineSetpoint = _setpoint;
    state.scanline.amplitude = _amp;
  }
  else {
    fprintf(stderr, "RcvInTappingMode: Error, in unknown mode\n");
  }
}

void Microscope::RcvInContactMode (const float _p, const float _i,
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
}

void Microscope::RcvInSewingStyle (const float _setpoint, const float _bDelay,
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

void Microscope::RcvInSpectroscopyMode(const float _setpoint,
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
    printf("stdel=%f, z_st=%f, z_end=%f, z_pull=%f, forcelim=%f\n",
		_startDelay, _zStart, _zEnd, _zPullback, _forceLimit);
    printf("dist=%f, numpoints=%d, numhalfcycles=%d\n",
		_distBetweenFC, _numPoints, _numHalfcycles);
    printf("samp_spd=%f, pull_spd=%f, start_spd=%f, fdback_spd=%f\n",
		_sampleSpeed, _pullbackSpeed, _startSpeed, _feedbackSpeed);
    printf("avg_num=%d, samp_del=%f, pull_del=%f, fdback_del=%f\n",
		_avgNum, _sampleDelay, _pullbackDelay, _feedbackDelay);
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

void Microscope::RcvForceParameters (const int _modifyEnable,
                                     const float) {
  state.modify.modify_enabled = _modifyEnable;
  if (!_modifyEnable) {
    printf("Force modification disabled!\n");
  } else {
    printf("Fmods min = %g, max = %g\n",
           state.modify.setpoint_min, state.modify.setpoint_max);
  }
}

void Microscope::RcvBaseModParameters (const float _min, const float _max) {
  state.modify.setpoint_min = _min;
  state.modify.setpoint_max = _max;
  d_decoration->modSetpointMin = _min;
  d_decoration->modSetpointMax = _max;
  printf("Fmods min = %g, max = %g\n",
         _min, _max);
}

void Microscope::RcvForceSettings (const float _min, const float _max,
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

void Microscope::RcvVoltsourceEnabled (const int _voltNum,
                                       const float _voltage) {
  // DO NOTHING
  printf("Volt source %d has been enabled with %g voltage\n",
         _voltNum, _voltage);
}

void Microscope::RcvVoltsourceDisabled (const int _voltNum) {
  // DO NOTHING
  printf("Volt source %d has been disabled\n", _voltNum);
}

void Microscope::RcvAmpEnabled (const int _ampNum, const float _offset,
                                const float _percentOffset,
                                const int _gainMode) {
  // DO NOTHING
  printf("Amplifier %d has been enabled with offset %g, percent "
         "offset %g, and gain mode %d\n", _ampNum, _offset, _percentOffset,
         _gainMode);
}

void Microscope::RcvAmpDisabled (const int _ampNum) {
  // DO NOTHING
  printf("Amplifier %d has been disabled\n", _ampNum);
}

 
void Microscope::RcvStartingToRelax (const int _sec, const int _usec) {
  state.relaxComp = state.doRelaxComp;
  if (state.doRelaxComp) {
    relax_init(_sec, _usec, state.lastZ);
    printf("Beginning relaxation compensation at %d:%d\n", _sec, _usec);
  }

  if (state.doDriftComp)
    driftZDirty();
}

void Microscope::RcvInModModeT (const int _sec, const int _usec) {
  state.acquisitionMode = MODIFY;
  printf("In modify mode\n");

  state.relaxComp = state.doRelaxComp;
  if (state.doRelaxComp) {
    relax_init(_sec, _usec, state.lastZ);
    printf("Compensating mod force %f\n", (float) state.modify.setpoint);
  }

  if (state.doDriftComp)
    driftZDirty();

  doModifyModeCallbacks();
}

void Microscope::RcvInModMode (void) {
  state.acquisitionMode = MODIFY;
  printf("In modify mode\n");

  if (state.doDriftComp)
    driftZDirty();

  doModifyModeCallbacks();
}

// Not used by Topo code
void Microscope::RcvInImgModeT (const int _sec, const int _usec) {
  state.acquisitionMode = IMAGE;
  printf("In image mode\n");

  state.relaxComp = state.doRelaxComp;
  if (state.doRelaxComp) {
    relax_init(_sec, _usec, state.lastZ);
    printf("Compensating img force %f\n", (float) state.image.amplitude);
  }

  if (state.doDriftComp)
    driftZDirty();

  doImageModeCallbacks();
}

// case AFM_IN_IMG_MODE:
// case SPM_INVISIBLE_TRAIL:
void Microscope::RcvInImgMode (void) {
  state.acquisitionMode = IMAGE;
  printf("In image mode\n");

  if (state.doDriftComp)
    driftZDirty();

  // If we're only relaxing down, turn off the relaxation code
  if (!state.doRelaxUp) {
    state.relaxComp = 0;
    printf("Disabling relaxation in image mode\n");
  }

  doImageModeCallbacks();
}

void Microscope::RcvModForceSet (const float _value) {
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

void Microscope::RcvImgForceSet (const float _value) {
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

void Microscope::RcvModSet (const int _modifyEnable, const float _max,
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

void Microscope::RcvImgSet (const int _modifyEnable, const float _max,
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

void Microscope::RcvRelaxSet (const int _min, const int _sep) {
  state.stmRxTmin = _min;
  relax_set_min(_min);
  printf("Microscope::RcvRelaxSet: Tmin set @ %d\n", _min);
  state.stmRxTsep = _min;
  relax_set_sep(_sep);
  printf("Microscope::RcvRelaxSet: Tsep set @ %d\n", _sep);
}

void Microscope::RcvForceSet (const float _force) {
  printf("Throwing away:  force set at %g\n", _force);

  if (state.doDriftComp)
    driftZDirty();
}

// case AFM_IMG_FORCE_SET_FAILURE:
// case AFM_MOD_FORCE_SET_FAILURE:
// case AFM_FORCE_SET_FAILURE:
void Microscope::RcvForceSetFailure (const float) {
  state.modify.modify_enabled = VRPN_FALSE;
  printf("Force modifications disabled!\n");
}

void Microscope::RcvPulseParameters (const int _pulseEnabled,
                                     const float _bias, const float _height,
                                     const float _width) {
  if (_pulseEnabled) {
    printf("Pulses disabled!\n");
  } else {
    printf("Pulse bias = %g, peak = %g, width = %g\n",
           _bias, _height, _width);
  }
}

void Microscope::RcvStdDevParameters (const int _samples, const float _freq) {
  state.modify.std_dev_samples = _samples;
  state.modify.std_dev_frequency = _freq;
  printf("Num samples/point = %d, Frequency = %g\n", _samples, _freq);
}


int Microscope::RcvWindowLineData (const int _x, const int _y,
                                    const int _sec, const int _usec,
                                    const int _fieldCount,
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

int Microscope::RcvWindowLineData (void) {
  // blue, since we are not touching or modifying
  d_decoration->mode = nmb_Decoration::IMAGE;

  state.dlistchange = VRPN_TRUE;        //XXX Need more here
  return 0;
}

// TODO:  check _x and _y for out-of-bounds?
void Microscope::RcvWindowScanNM (const int _x, const int _y,
                                  const int _sec, const int _usec,
                                  const float _value, const float _deviation) {
  float value;
  int x, y;

  value = _value;
  x = _x;
  y = _y;
  if (state.relaxComp)
    state.relaxComp = relax_comp(_sec, _usec, &value);

  if (state.doDriftComp)
    value = drift_comp(_x, _y, value, _sec, _usec, state.do_y_fastest);

  // In raster-scan mode, check to make sure we are showing the forward
  // direction before taking the data
  //if (!state.snap.ignoreScan) {
    if (!(state.do_raster && state.raster_scan_backwards)) {
      state.data.scan_channels->Handle_old_report(x, y, _sec, _usec,
                                             value, _deviation);
      // "if" removed TCH 3 Oct 97
      //if (glenable)
        // update the record of the subgrid area updated
        d_dataset->range_of_change.AddPoint(x, y);

      // obsolete?
      if (state.subscan_count && !(--state.subscan_count))
        ResumeFullScan();

    }
  //}

  if (state.relaxComp >= 0)
    state.lastZ = value;

  if (state.cannedLineVisible)
    state.cannedLineVisible = VRPN_FALSE;

  // blue, since we are not touching or modifying
  d_decoration->mode = nmb_Decoration::IMAGE;

  state.dlistchange = VRPN_TRUE;        //XXX Need more here
}

void Microscope::RcvWindowBackscanNM (const int _x, const int _y,
                                      const int _sec, const int _usec,
                                      const float _value,
                                      const float _deviation) {
  float value;
  //int x, y;

  // Compensate for piezo relaxation
  // why not (state.doRelaxComp)?
  value = _value;
  if (state.relaxComp)
    state.relaxComp = relax_comp(_sec, _usec, &value);

  if (state.doDriftComp)
    value = drift_comp(_x, _y, value, _sec, _usec, state.do_y_fastest);

  // In the raster scan mode, check to make sure we are showing
  // the backwards direction before taking data
  //if (!state.snap.ignoreScan) {
    if (state.do_raster && state.raster_scan_backwards) {
      state.data.scan_channels->Handle_old_report(_x, _y, _sec, _usec,
                                             value, _deviation);
      state.dlistchange = VRPN_TRUE;
      // "if" removed TCH 3 Oct 97
      //if (glenable)
        // Update the record of the subgrid that has changed
        d_dataset->range_of_change.AddPoint(_x, _y);

      // If we're temporarily subscanning around and time's up,
      // resume scanning the big picture
      if (state.subscan_count && !(--state.subscan_count))
        ResumeFullScan();
    }
  //}

  if (state.relaxComp >= 0)
    state.lastZ = value;

  // set the background color to blue (not touching nor modifying)
  d_decoration->mode = nmb_Decoration::IMAGE;

  state.cannedLineVisible = VRPN_FALSE;
}

void Microscope::RcvPointResultNM (const float _x, const float _y,
                                   const int _sec, const int _usec,
                                   const float _height,
                                   const float _deviation) {
  float height;

  d_bluntResult.normal[Z] = 1.0;  // invalidate it

  height = _height;
  if (state.relaxComp)
    state.relaxComp = relax_comp(_sec, _usec, &height);

  //XXX Drift compensation taken out

  if (state.doSplat && (state.relaxComp >= 0))
    ptSplat(&state.lost_changes, d_dataset->inputGrid, state.data.inputPoint);

  if (state.acquisitionMode == MODIFY)
    d_decoration->mode = nmb_Decoration::MODIFY;  // red if modifying
  else if (state.acquisitionMode == IMAGE)
    d_decoration->mode = nmb_Decoration::FEEL;  // yellow if just touching

  if (state.readingStreamFile && !state.cannedLineVisible)
    state.cannedLineVisible = VRPN_TRUE;

  // if it's a modification result, display it
  if ((state.acquisitionMode == MODIFY || state.cannedLineVisible) &&
      (state.relaxComp >= 0))
    DisplayModResult(_x, _y, height, NULL, VRPN_FALSE);

  if (state.relaxComp >= 0)
    state.lastZ = height;
  else
    height = state.lastZ;  // ignore this point

  state.data.point_channels->Handle_old_report(_x, _y, _sec, _usec,
                                          height, _deviation);

  doPointDataCallbacks(state.data.inputPoint); // to feel what we are
                                                // doing but we don't
                                                // want to store them
                                                // in a modfile because
                                                // its being used to
                                                // store the force curve

}

// HACK to get ohmmeter data into point results
static float lastResistanceReceived = 0;
// END HACK

// case SPM_POINT_RESULT_DATA:
// case SPM_BOTTOM_PUNCH_RESULT_DATA:
// case SPM_TOP_PUNCH_RESULT_DATA: {
void Microscope::RcvResultData (const int _type,
                                const float _x, const float _y,
                                const int _sec, const int _usec,
                                const int _fieldCount,
                                const float * _fields) {
  Point_value * z_value;
  float height;
  int i;

  d_bluntResult.normal[Z] = 1.0;  // invalidate it (?)

  if (spm_verbosity >= 1) {
    printf("Point result, type %d at (%g, %g), time %d:%d\n",
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
  // the inputPoint values.
  // HACK HACK HACK
  if (state.data.point_channels->Handle_report(_x, _y, _sec, _usec,
                                          (float *) _fields, _fieldCount)) {
    fprintf(stderr, "Error handling SPM point result data\n");
    d_dataset->done = VRPN_TRUE;
    return;
  }

  if (spm_verbosity >= 1)
    state.data.inputPoint->print("  Result:");

  // Look up the value that corresponds to what is
  // mapped to the heightGrid, if we are getting that
  // data set.  This will make what we feel match what
  // we are looking at.
  z_value = state.data.inputPoint->getValueByPlaneName
                   (d_dataset->heightPlaneName->string());

  // Do relaxation compensation using the data set that is mapped to
  // height.
  if (state.relaxComp && z_value) {
    height = z_value->value();
    state.relaxComp = relax_comp(_sec, _usec, &height);
    z_value->setValue(height);
  }
  // What is going on here? We do relaxation above, then it seems like
  // we make some additional adjustment here. But if I take this out, 
  // I can't feel a surface when using -tc and freehand modification.
   if (state.relaxComp >= 0) {
     if (z_value)
       state.lastZ = z_value->value();
   } else
     z_value->setValue(state.lastZ);  // ignore this point

  //XXX Drift compensation taken out

  // splat this point onto the grid
  if (state.doSplat && (state.relaxComp >= 0))
    ptSplat(&state.lost_changes, d_dataset->inputGrid, state.data.inputPoint);

  // set the background color
  if (state.acquisitionMode == MODIFY)
    d_decoration->mode = nmb_Decoration::MODIFY;  // red if modifying
  else if (state.acquisitionMode == IMAGE)
    d_decoration->mode = nmb_Decoration::FEEL;  // yellow if just touching

  if (state.readingStreamFile && !state.cannedLineVisible)
    state.cannedLineVisible = VRPN_TRUE;

  // if it's a modification result, display it
  if ((state.acquisitionMode == MODIFY || state.cannedLineVisible) &&
      (state.relaxComp >= 0))
    DisplayModResult(state.data.inputPoint->x(),
                     state.data.inputPoint->y(),
                     0.0f, z_value, VRPN_TRUE);

  if (state.modify.style != FORCECURVE)	{// XXX HACK - we need point results
     //fprintf(stderr, "RcvResultData.\n");

     // HACK - to get ohmmeter data in point results
     static int channel_added = 0;
     char *res_channel_name = "Resistance";
     char *res_channel_unit = "Ohms";
     if (!channel_added) {
	state.data.inputPoint->addNewValue(res_channel_name, res_channel_unit);
	channel_added = 1;
     }
     Point_value *pv = state.data.inputPoint->getValueByName(res_channel_name);
     pv->setValue(lastResistanceReceived);
     // END HACK

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
  d_decoration->trueTipLocation[2] = z_value->value();
  d_decoration->trueTipLocation_changed = 1;
//fprintf(stderr, "Set true tip location to %.2f %.2f %.2f\n",
//d_decoration->trueTipLocation[0],
//d_decoration->trueTipLocation[1],
//d_decoration->trueTipLocation[2]);

}

// case STM_ZIG_RESULT_NM:
// case SPM_BLUNT_RESULT_NM:
void Microscope::RcvResultNM (const float _x, const float _y,
                              const int _sec, const int _usec,
                              const float _height, const float _normX,
                              const float _normY, const float _normZ) {
  float height;

  height = _height;
  if (state.relaxComp)
    state.relaxComp = relax_comp(_sec, _usec, &height);

  // BUG BUG BUG - should this be height or value?
  if (state.doDriftComp)
    height = drift_comp(-2, -2, height, _sec, _usec, state.do_y_fastest);

  // splat this point onto the grid
  if (state.doSplat && (state.relaxComp >= 0))
    ptSplat(&state.lost_changes, d_dataset->inputGrid, state.data.inputPoint);

  // set the background color
  if (state.acquisitionMode == MODIFY)
    d_decoration->mode = nmb_Decoration::MODIFY;  // red if modifying
  else if (state.acquisitionMode == IMAGE)
    d_decoration->mode = nmb_Decoration::FEEL;  // yellow if just touching

  if (state.readingStreamFile && !state.cannedLineVisible)
    state.cannedLineVisible = VRPN_TRUE;

  // if it's a modification result, display it
  if ((state.acquisitionMode == MODIFY || state.cannedLineVisible) &&
      (state.relaxComp >= 0))
    DisplayModResult(_x, _y, height);

  if (state.relaxComp >= 0)
    state.lastZ = height;
  else
    height = state.lastZ;  // ignore this point

  if (state.doDriftComp)
    driftZDirty();

  d_bluntResult.x = _x;
  d_bluntResult.y = _y;
  d_bluntResult.height = height;
  d_bluntResult.std_dev = 
  d_bluntResult.normal[X] = _normX;
  d_bluntResult.normal[Y] = _normY;
  d_bluntResult.normal[Z] = _normZ;

  // latency compensation
  d_decoration->trueTipLocation[0] = _x;
  d_decoration->trueTipLocation[1] = _y;
  d_decoration->trueTipLocation[2] = height;
  d_decoration->trueTipLocation_changed = 1;
}

// pulses and num_pulses are globals from openGL.c
void Microscope::RcvPulseCompletedNM (const float _xRes, const float _yRes) {

  BCPlane * heightPlane;
  PointType top, bottom;
  //double frac_x, frac_y;
  //int i;

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

void Microscope::RcvPulseFailureNM (const float, const float) {
  state.lastPulseOK = VRPN_FALSE;
}

// XXX
// "Think about what this means when the data sets can be changed"
void Microscope::RcvScanRange (const float _minX, const float _maxX,
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

// case STM_SET_REGION_COMPLETED:
// case STM_SET_REGION_CLIPPED:
void Microscope::RcvSetRegionC (const int /* _type */,
                                const float _minX, const float _minY,
                                const float _maxX, const float _maxY) {
  BCPlane * heightPlane;
  BCPlane * p;
  int x, y;

  //static PointType red_bot, blue_bot, green_bot;

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

  fprintf(stderr,"Microscope::RcvSetRegionC\n\
   new region: (%g, %g) to (%g, %g)\n",
         heightPlane->minX(), heightPlane->minY(),
         heightPlane->maxX(), heightPlane->maxY());

  VERBOSE(5,"        Making tops in region change");

  //d_decoration->red.moveTo(heightPlane->minX(), heightPlane->minY(),
                            //heightPlane);
  d_decoration->red.doCallbacks(heightPlane->minX(), heightPlane->minY(),
                            heightPlane);
  //d_decoration->green.moveTo(heightPlane->maxX(), heightPlane->minY(),
                              //heightPlane);
  d_decoration->green.doCallbacks(heightPlane->maxX(), heightPlane->minY(),
                            heightPlane);
  //d_decoration->blue.moveTo(heightPlane->maxX(), heightPlane->maxY(),
                             //heightPlane);
  d_decoration->blue.doCallbacks(heightPlane->maxX(), heightPlane->maxY(),
                            heightPlane);

  state.SetDefaultScanlineForRegion(d_dataset);

//fprintf(stderr, "region set complete\n");
}

void Microscope::RcvResistanceFailure (const int _meter) {
  // DO NOTHING
  fprintf(stderr, "Error reading resistance on meter %d\n", _meter);
}

void Microscope::RcvResistance (const int _meter, const int /* _sec */,
                                const int /* _usec */,
                                const float _resistance) {
  // DO NOTHING
  printf("Resistance on meter %d is %f\n", _meter, _resistance);

}

void Microscope::RcvResistance2(const int _chan, const int  _sec ,
				const int  _usec , const float _res,
				const float _volt, const float _range,
				const float _filt) {
  float time = _sec + _usec*0.000001;
  printf("time: %f sec, %f Ohms\n", time, _res);
  if (the_french_ohmmeter_ui == NULL)
      the_french_ohmmeter_ui = new Ohmmeter(get_the_interpreter(),
			tcl_script_dir, NULL);
  the_french_ohmmeter_ui->updateDisplay(_chan, _res, 
		MEASURING, _volt, _range, _filt);
  RcvResistance3(_chan, _sec, _usec, _res, _volt, _range, _filt);
}

// HACK - this is a temporary fix to get resistance values into the modfile
void Microscope::RcvResistance3(const int /*_chan */, const int /* _sec */,
				const int /* _usec */, const float _res,
				const float /*_volt */, const float /*_range*/,
				const float /*_filt*/) {
    lastResistanceReceived = _res;
}

void Microscope::RcvResistanceWithStatus(const int _chan, const int  _sec ,
                const int  _usec , const float _res,
                const float _volt, const float _range,
                const float _filt, const long _status) {
  float time = _sec + _usec*0.000001;
  printf("time: %f sec, %f Ohms\n", time, _res);
  if (the_french_ohmmeter_ui == NULL)
      the_french_ohmmeter_ui = new Ohmmeter(get_the_interpreter(),
            tcl_script_dir, NULL);
  the_french_ohmmeter_ui->updateDisplay(_chan, _res,
        _status, _volt, _range, _filt);
  RcvResistance3(_chan, _sec, _usec, _res, _volt, _range, _filt);
}

void Microscope::RcvReportSlowScan (const int _enable) {
  state.slowScanEnabled = _enable;
}

void Microscope::RcvScanParameters (const char **) {

  // DO NOTHING
  // This is a notification-only message which is handled
  // for now in MicroscopeIO.

}

void Microscope::RcvHelloMessage (const char * _magic, const char * _name,
                                  const int _majorVersion,
                                  const int _minorVersion) {
  if (strcmp(_magic, "nM!")) {
    fprintf(stderr, "Bad magic in microscope hello\n");
    fprintf(stderr, "  (expected \"nM!\", got \"%s\"\n", _magic);
    d_dataset->done = VRPN_TRUE;
    return;
  }
  printf("Hello from microscope %s, version %d.%d\n", _name, _majorVersion,
         _minorVersion);
}

void Microscope::RcvClientHello (const char * _magic, const char * _name,
                                 const int _majorVersion,
                                 const int _minorVersion) {
  if (strcmp(_magic, "nM!")) {
    fprintf(stderr, "Bad magic in client hello\n");
    fprintf(stderr, "  (expected \"nM!\", got \"%s\"\n", _magic);
    d_dataset->done = VRPN_TRUE;
    return;
  }
  printf("Streamfile written by %s, version %d.%d\n", _name, _majorVersion,
         _minorVersion);

}

void Microscope::RcvClearScanChannels (void) {
  // we use the same sets for 2D and 1D scanning
  RcvClearScanlineChannels();

  if (state.acquisitionMode == SCANLINE) return;

  if (state.data.scan_channels->Clear_channels()) {
    fprintf(stderr, "Can't clear scan datasets\n");
    d_dataset->done = VRPN_TRUE;
  }
}

void Microscope::RcvScanDataset (const char * _name, const char * _units,
                                 const float _offset, const float _scale) {
    // we use the same sets for 2D and 1D scanning
    RcvScanlineDataset(_name, _units, _offset, _scale);
    if (state.acquisitionMode == SCANLINE) return;

    // HACK HACK HACK
    printf("RcvScanDataset  %s (%s), offset:  %g, scale:  %g\n",
	_name, _units, _offset, _scale);
    if (state.data.scan_channels->Add_channel((char *) _name, (char *) _units,
                                        _offset, _scale)) {
        fprintf(stderr, "Can't add scan dataset\n");
        d_dataset->done = VRPN_TRUE;
    }
}

void Microscope::RcvClearPointChannels (void) {
  if (state.data.point_channels->Clear_channels()) {
    fprintf(stderr, "Can't clear point datasets\n");
    d_dataset->done = VRPN_TRUE;
  }
}

void Microscope::RcvPointDataset (const char * _name, const char * _units,
                                  const int _numSamples,
                                  const float _offset, const float _scale) {
  printf("  %s (%s), count:  %d, offset:  %g, scale:  %g\n",
         _name, _units, _numSamples, _offset, _scale);
  // HACK HACK HACK
  if (state.data.point_channels->Add_channel((char *) _name, (char *) _units,
                                        _offset, _scale, _numSamples)) {
    fprintf(stderr, "Can't add point dataset\n");
    d_dataset->done = VRPN_TRUE;
  }
}

void Microscope::RcvPidParameters (const float _p, const float _i,
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

void Microscope::RcvScanrateParameter (const float _rate) {
  if (state.acquisitionMode == MODIFY) {
	printf("Warning! Rate changed on topo in modify mode (ignoring)\n");
  } else {
	state.image.scan_rate_microns = _rate / 1000.0;
	printf("New scan rate:  %g (nM/s)\n", _rate);
  }
}

int Microscope::RcvReportGridSize (const int _x, const int _y) {
  printf("Grid size from scanner:  %dx%d\n", _x, _y);
  if ((_x != d_dataset->inputGrid->numX()) ||
      (_y != d_dataset->inputGrid->numY())) {
    fprintf(stderr, "Does not match command line grid size!\n");
    // If we don't set exit flag here, we just get a bus error later.
    d_dataset->done = VRPN_TRUE;
    exit(-1); // we get a bus error before the next iteration, so exit now.
    //return -1;
  }
  return 0;
}

void Microscope::RcvServerPacketTimestamp (const int, const int) {
  // DO NOTHING
  // This message is for use by networking code
}

extern TopoFile GTF;
void Microscope::RcvTopoFileHeader (const int _length, const char *header) {
  printf("********** RCV'D TOPO FILE HEADER **********\n");
  // DO NOTHING
  if(_length < 1536){
	printf("Unexpected Header length %d need 1536\n",_length);
  }else{
	GTF.parseHeader(header,_length);
  	printf("********** Got Topometrix file header, length %d\n", _length);
/*	handle=fopen("temp.tfr","w");
	if(handle == NULL){printf("ERROR WRITING TEMP.TFR");}
	fHdl=fileno(handle);
	write(fHdl,header,_length*sizeof(char));	
	fclose(handle);*/
  }
}


void Microscope::RcvForceCurveData(const float _x, const float _y, 
    const int _sec, const int _usec, const int _num_points, 
    const int _num_halfcycles, const float * _z_values, const float **_curves){

  int i,j;
  Point_results pnt;
  if (spm_verbosity >= 1) {
    printf("Force Curve result, at (%g, %g), time %d:%d\n", _x,_y,_sec,_usec);
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
      fprintf(stderr, "RcvForceCurveData.\n");
      doPointDataCallbacks(state.data.fc_inputPoint);
    }
  }
  // draw graphical representation
  DisplayModResult(state.data.fc_inputPoint->x(),
		   state.data.fc_inputPoint->y(),
		   0.0f, NULL, VRPN_FALSE);

}

// updates user interface
void Microscope::RcvInScanlineMode(const long enabled){
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
void Microscope::RcvClearScanlineChannels (void) {
  // this was taken out because channels are shared with those
  // used for image mode
  // update user interface:
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
void Microscope::RcvScanlineDataset(const char * _name, const char * _units,
                                 const float /*_offset*/, const float /*_scale*/) {

// this was taken out because channels are shared with those
// used for image mode

//  printf("RcvScanlineDataset:  %s (%s), offset:  %g, scale:  %g\n",
//         _name, _units, _offset, _scale);
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
void Microscope::RcvScanlineDataHeader(const float _x, const float _y, 
        const float _z, const float _angle, const float /*_slope*/,
		const float _width, const long _resolution, 
		const long _feedback_enabled, const long _checking_forcelimit,
		const float _max_force_setting, const float _max_z_step,
		const float _max_xy_step,
        const long _sec, const long _usec, const long _num_channels) {

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


	state.scanline.feedback_enabled = _feedback_enabled; 
	state.scanline.forcelimit_enabled = _checking_forcelimit;
	state.scanline.forcelimit = _max_force_setting;
	state.scanline.max_z_step = _max_z_step;
	state.scanline.max_xy_step = _max_xy_step;
}

// converts from DAC units to the appropriate units for each channel and 
// calls callbacks to handle the new data

void Microscope::RcvScanlineData(const long _point, const long _num_channels,
        const float * _value) {
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
    if (_point == state.data.currentScanlineData.length() - 1)
	doScanlineDataCallbacks(&(state.data.currentScanlineData));
}

void Microscope::RcvRecvTimestamp (const struct timeval) {
  // DO NOTHING
}

void Microscope::RcvFakeSendTimestamp (const struct timeval) {
  // DO NOTHING
}

void Microscope::RcvUdpSeqNum (const long) {
  // DO NOTHING
}
