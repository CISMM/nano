/*===3rdtech===
  Copyright (c) 2000 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#ifdef __CYGWIN__
#include <unistd.h> // for sleep()
#endif
#include <BCPlane.h>
#include <nmb_Types.h>
#include <nmb_Decoration.h>
#include <nmb_Globals.h>
#include <nmb_Line.h>

#include <nmm_Types.h>
#include <nmm_Globals.h>
#include <nmm_MicroscopeRemote.h>

#include <nmg_Graphics.h>
#include <nmg_Globals.h>

#include <microscape.h>

#include "microscopeHandlers.h"



#if (!defined(X) || !defined(Y) || !defined(Z))
#define	X	(0)
#define	Y	(1)
#define	Z	(2)
#endif

#ifndef M_PI
#define M_PI 3.141592653589793238
#define M_PI_2		1.57079632679489661923
#endif

void setupStateCallbacks (nmm_Microscope_Remote * ms) {

  ms->state.doRelaxComp.addCallback
      (handle_doRelaxComp_change, ms);
  ms->state.scanning.addCallback
      (handle_scanning_change, ms);

  ms->state.modify.mode.addCallback
    (handle_Mmode_change, ms);
  ms->state.modify.style.addCallback
    (handle_Mstyle_change, ms);
  ms->state.modify.tool.addCallback
    (handle_Mtool_change, ms);

  ms->state.modify.setpoint.addCallback
    (handle_Mmode_p_change, ms);
  ms->state.modify.p_gain.addCallback
    (handle_Mmode_p_change, ms);
  ms->state.modify.i_gain.addCallback
    (handle_Mmode_p_change, ms);
  ms->state.modify.d_gain.addCallback
    (handle_Mmode_p_change, ms);
  ms->state.modify.amplitude.addCallback
    (handle_Mmode_p_change, ms);
  ms->state.modify.frequency.addCallback
    (handle_Mmode_p_change, ms);
  ms->state.modify.input_gain.addCallback
    (handle_Mmode_p_change, ms);
  ms->state.modify.ampl_or_phase.addCallback
    (handle_Mmode_p_change, ms);
  ms->state.modify.drive_attenuation.addCallback
    (handle_Mmode_p_change, ms);
  ms->state.modify.phase.addCallback
    (handle_Mmode_p_change, ms);
  ms->state.modify.scan_rate_microns.addCallback
    (handle_Mmode_p_change, ms);

  ms->state.modify.sweep_width.addCallback
    (handle_Mstyle_p_change, ms);
  ms->state.modify.bot_delay.addCallback
    (handle_Mstyle_p_change, ms);
  ms->state.modify.top_delay.addCallback
    (handle_Mstyle_p_change, ms);
  ms->state.modify.z_pull.addCallback
    (handle_Mstyle_p_change, ms);
  ms->state.modify.punch_dist.addCallback
    (handle_Mstyle_p_change, ms);
  ms->state.modify.speed.addCallback
    (handle_Mstyle_p_change, ms);
  ms->state.modify.watchdog.addCallback
    (handle_Mstyle_p_change, ms);

  ms->state.modify.step_size.addCallback
    (handle_Mtool_p_change, ms);

  ms->state.modify.slow_line_playing.addCallback
    (handle_slow_line_playing_change, ms);
  ms->state.modify.slow_line_step.addCallback
    (handle_slow_line_step_change, ms);
  ms->state.modify.slow_line_direction.addCallback
    (handle_slow_line_direction_change, ms);

  ms->state.modify.blunt_size.addCallback
    (handle_Mstyle_p_change, ms);
  ms->state.modify.blunt_speed.addCallback
    (handle_Mstyle_p_change, ms);

  ms->state.modify.fc_start_delay.addCallback
    (handle_Mstyle_p_change, ms);
  ms->state.modify.fc_z_start.addCallback
    (handle_Mstyle_p_change, ms);
  ms->state.modify.fc_z_end.addCallback
    (handle_Mstyle_p_change, ms);
  ms->state.modify.fc_z_pullback.addCallback
    (handle_Mstyle_p_change, ms);
  ms->state.modify.fc_force_limit.addCallback
    (handle_Mstyle_p_change, ms);
  ms->state.modify.fc_movedist.addCallback
    (handle_Mstyle_p_change, ms);
  ms->state.modify.fc_num_points.addCallback
    (handle_Mstyle_p_change, ms);
  ms->state.modify.fc_num_halfcycles.addCallback
    (handle_Mstyle_p_change, ms);
  ms->state.modify.fc_sample_speed.addCallback
    (handle_Mstyle_p_change, ms);
  ms->state.modify.fc_pullback_speed.addCallback
    (handle_Mstyle_p_change, ms);
  ms->state.modify.fc_start_speed.addCallback
    (handle_Mstyle_p_change, ms);
  ms->state.modify.fc_feedback_speed.addCallback
    (handle_Mstyle_p_change, ms);
  ms->state.modify.fc_avg_num.addCallback
    (handle_Mstyle_p_change, ms);
  ms->state.modify.fc_sample_delay.addCallback
    (handle_Mstyle_p_change, ms);
  ms->state.modify.fc_pullback_delay.addCallback
    (handle_Mstyle_p_change, ms);
  ms->state.modify.fc_feedback_delay.addCallback
    (handle_Mstyle_p_change, ms);

  ms->state.image.mode.addCallback
    (handle_Imode_change, ms);
  ms->state.image.style.addCallback
    (handle_Istyle_change, ms);

  ms->state.image.grid_resolution.addCallback
    (handle_grid_resolution_change, ms);
  ms->state.image.scan_angle.addCallback
    (handle_scan_angle_change, ms);

  ms->state.image.setpoint.addCallback
    (handle_Imode_p_change, ms);
  ms->state.image.p_gain.addCallback
    (handle_Imode_p_change, ms);
  ms->state.image.i_gain.addCallback
    (handle_Imode_p_change, ms);
  ms->state.image.d_gain.addCallback
    (handle_Imode_p_change, ms);
  ms->state.image.amplitude.addCallback
    (handle_Imode_p_change, ms);
  ms->state.image.frequency.addCallback
    (handle_Imode_p_change, ms);
  ms->state.image.input_gain.addCallback
    (handle_Imode_p_change, ms);
  ms->state.image.ampl_or_phase.addCallback
    (handle_Imode_p_change, ms);
  ms->state.image.drive_attenuation.addCallback
    (handle_Imode_p_change, ms);
  ms->state.image.phase.addCallback
    (handle_Imode_p_change, ms);
  ms->state.image.scan_rate_microns.addCallback
    (handle_Imode_p_change, ms);

  ms->state.image.blunt_size.addCallback
    (handle_Istyle_p_change, ms);
  ms->state.image.blunt_speed.addCallback
    (handle_Istyle_p_change, ms);

  ms->state.stm_z_scale.addCallback
    (handle_z_scale_change, ms);
  ms->state.slowScanEnabled.addCallback
    (handle_tcl_scanEnable_change, ms);

  // Scanline parameters:
  ms->state.scanline.mode.addCallback
    (handle_SLmode_change, ms);
  ms->state.scanline.forcelimit_enabled.addCallback
    (handle_SLforcelimit_change, ms);

  ms->state.scanline.setpoint.addCallback
    (handle_SLmode_p_change, ms);
  ms->state.scanline.p_gain.addCallback
    (handle_SLmode_p_change, ms);
  ms->state.scanline.i_gain.addCallback
    (handle_SLmode_p_change, ms);
  ms->state.scanline.d_gain.addCallback
    (handle_SLmode_p_change, ms);
  ms->state.scanline.width.addCallback
    (handle_SLmode_p_change, ms);
  ms->state.scanline.amplitude.addCallback
    (handle_SLmode_p_change, ms);
  ms->state.scanline.frequency.addCallback
    (handle_SLmode_p_change, ms);
  ms->state.scanline.input_gain.addCallback
    (handle_SLmode_p_change, ms);
  ms->state.scanline.ampl_or_phase.addCallback
    (handle_SLmode_p_change, ms);
  ms->state.scanline.drive_attenuation.addCallback
    (handle_SLmode_p_change, ms);
  ms->state.scanline.phase.addCallback
    (handle_SLmode_p_change, ms);
  ms->state.scanline.scan_rate_microns_per_sec.addCallback
    (handle_SLmode_p_change, ms);

  ms->state.scanline.forcelimit.addCallback
    (handle_SLforcelimit_p_change, ms);
  ms->state.scanline.max_z_step.addCallback
    (handle_SLforcelimit_p_change, ms);
  ms->state.scanline.max_xy_step.addCallback
    (handle_SLforcelimit_p_change, ms);
  ms->state.scanline.feedback_enabled.addCallback
    (handle_SLforcelimit_change, ms);

  ms->state.scanline.start_linescan.addCallback
            (handle_linescan_start, ms);
  ms->state.scanline.width.addCallback
	    (handle_linescan_position, ms);
  ms->state.scanline.x_end.addCallback
            (handle_linescan_position, ms);
  ms->state.scanline.y_end.addCallback
            (handle_linescan_position, ms);
  ms->state.scanline.z_end.addCallback
            (handle_linescan_position, ms);
  ms->state.scanline.angle.addCallback
            (handle_linescan_position, ms);

  ms->state.scanline.showing_position.addCallback
	    (handle_scanline_position_display_change, ms);
}

void teardownStateCallbacks (nmm_Microscope_Remote * ms) {

  ms->state.doRelaxComp.removeCallback
      (handle_doRelaxComp_change, ms);
  ms->state.scanning.removeCallback
      (handle_scanning_change, ms);

  ms->state.modify.mode.removeCallback
    (handle_Mmode_change, ms);
  ms->state.modify.style.removeCallback
    (handle_Mstyle_change, ms);
  ms->state.modify.tool.removeCallback
    (handle_Mtool_change, ms);

  ms->state.modify.setpoint.removeCallback
    (handle_Mmode_p_change, ms);
  ms->state.modify.p_gain.removeCallback
    (handle_Mmode_p_change, ms);
  ms->state.modify.i_gain.removeCallback
    (handle_Mmode_p_change, ms);
  ms->state.modify.d_gain.removeCallback
    (handle_Mmode_p_change, ms);
  ms->state.modify.amplitude.removeCallback
    (handle_Mmode_p_change, ms);
  ms->state.modify.frequency.removeCallback
    (handle_Mmode_p_change, ms);
  ms->state.modify.input_gain.removeCallback
    (handle_Mmode_p_change, ms);
  ms->state.modify.ampl_or_phase.removeCallback
    (handle_Mmode_p_change, ms);
  ms->state.modify.drive_attenuation.removeCallback
    (handle_Mmode_p_change, ms);
  ms->state.modify.phase.removeCallback
    (handle_Mmode_p_change, ms);
  ms->state.modify.scan_rate_microns.removeCallback
    (handle_Mmode_p_change, ms);

  ms->state.modify.sweep_width.removeCallback
    (handle_Mstyle_p_change, ms);
  ms->state.modify.bot_delay.removeCallback
    (handle_Mstyle_p_change, ms);
  ms->state.modify.top_delay.removeCallback
    (handle_Mstyle_p_change, ms);
  ms->state.modify.z_pull.removeCallback
    (handle_Mstyle_p_change, ms);
  ms->state.modify.punch_dist.removeCallback
    (handle_Mstyle_p_change, ms);
  ms->state.modify.speed.removeCallback
    (handle_Mstyle_p_change, ms);
  ms->state.modify.watchdog.removeCallback
    (handle_Mstyle_p_change, ms);

  ms->state.modify.step_size.removeCallback
    (handle_Mtool_p_change, ms);

  ms->state.modify.slow_line_playing.removeCallback
    (handle_slow_line_playing_change, ms);
  ms->state.modify.slow_line_step.removeCallback
    (handle_slow_line_step_change, ms);
  ms->state.modify.slow_line_direction.removeCallback
    (handle_slow_line_direction_change, ms);

  ms->state.modify.blunt_size.removeCallback
    (handle_Mstyle_p_change, ms);
  ms->state.modify.blunt_speed.removeCallback
    (handle_Mstyle_p_change, ms);

  ms->state.modify.fc_start_delay.removeCallback
    (handle_Mstyle_p_change, ms);
  ms->state.modify.fc_z_start.removeCallback
    (handle_Mstyle_p_change, ms);
  ms->state.modify.fc_z_end.removeCallback
    (handle_Mstyle_p_change, ms);
  ms->state.modify.fc_z_pullback.removeCallback
    (handle_Mstyle_p_change, ms);
  ms->state.modify.fc_force_limit.removeCallback
    (handle_Mstyle_p_change, ms);
  ms->state.modify.fc_movedist.removeCallback
    (handle_Mstyle_p_change, ms);
  ms->state.modify.fc_num_points.removeCallback
    (handle_Mstyle_p_change, ms);
  ms->state.modify.fc_num_halfcycles.removeCallback
    (handle_Mstyle_p_change, ms);
  ms->state.modify.fc_sample_speed.removeCallback
    (handle_Mstyle_p_change, ms);
  ms->state.modify.fc_pullback_speed.removeCallback
    (handle_Mstyle_p_change, ms);
  ms->state.modify.fc_start_speed.removeCallback
    (handle_Mstyle_p_change, ms);
  ms->state.modify.fc_feedback_speed.removeCallback
    (handle_Mstyle_p_change, ms);
  ms->state.modify.fc_avg_num.removeCallback
    (handle_Mstyle_p_change, ms);
  ms->state.modify.fc_sample_delay.removeCallback
    (handle_Mstyle_p_change, ms);
  ms->state.modify.fc_pullback_delay.removeCallback
    (handle_Mstyle_p_change, ms);
  ms->state.modify.fc_feedback_delay.removeCallback
    (handle_Mstyle_p_change, ms);

  ms->state.image.mode.removeCallback
    (handle_Imode_change, ms);
  ms->state.image.style.removeCallback
    (handle_Istyle_change, ms);

  ms->state.image.grid_resolution.removeCallback
    (handle_grid_resolution_change, ms);
  ms->state.image.scan_angle.removeCallback
    (handle_scan_angle_change, ms);

  ms->state.image.setpoint.removeCallback
    (handle_Imode_p_change, ms);
  ms->state.image.p_gain.removeCallback
    (handle_Imode_p_change, ms);
  ms->state.image.i_gain.removeCallback
    (handle_Imode_p_change, ms);
  ms->state.image.d_gain.removeCallback
    (handle_Imode_p_change, ms);
  ms->state.image.amplitude.removeCallback
    (handle_Imode_p_change, ms);
  ms->state.image.frequency.removeCallback
    (handle_Imode_p_change, ms);
  ms->state.image.input_gain.removeCallback
    (handle_Imode_p_change, ms);
  ms->state.image.ampl_or_phase.removeCallback
    (handle_Imode_p_change, ms);
  ms->state.image.drive_attenuation.removeCallback
    (handle_Imode_p_change, ms);
  ms->state.image.phase.removeCallback
    (handle_Imode_p_change, ms);
  ms->state.image.scan_rate_microns.removeCallback
    (handle_Imode_p_change, ms);

  ms->state.image.blunt_size.removeCallback
    (handle_Istyle_p_change, ms);
  ms->state.image.blunt_speed.removeCallback
    (handle_Istyle_p_change, ms);

  ms->state.stm_z_scale.removeCallback
    (handle_z_scale_change, ms);
  ms->state.slowScanEnabled.removeCallback
    (handle_tcl_scanEnable_change, ms);

  // Scanline parameters:
  ms->state.scanline.mode.removeCallback
    (handle_SLmode_change, ms);
  ms->state.scanline.forcelimit_enabled.removeCallback
    (handle_SLforcelimit_change, ms);

  ms->state.scanline.setpoint.removeCallback
    (handle_SLmode_p_change, ms);
  ms->state.scanline.p_gain.removeCallback
    (handle_SLmode_p_change, ms);
  ms->state.scanline.i_gain.removeCallback
    (handle_SLmode_p_change, ms);
  ms->state.scanline.d_gain.removeCallback
    (handle_SLmode_p_change, ms);
  ms->state.scanline.width.removeCallback
    (handle_SLmode_p_change, ms);
  ms->state.scanline.amplitude.removeCallback
    (handle_SLmode_p_change, ms);
  ms->state.scanline.frequency.removeCallback
    (handle_SLmode_p_change, ms);
  ms->state.scanline.input_gain.removeCallback
    (handle_SLmode_p_change, ms);
  ms->state.scanline.ampl_or_phase.removeCallback
    (handle_SLmode_p_change, ms);
  ms->state.scanline.drive_attenuation.removeCallback
    (handle_SLmode_p_change, ms);
  ms->state.scanline.phase.removeCallback
    (handle_SLmode_p_change, ms);
  ms->state.scanline.scan_rate_microns_per_sec.removeCallback
    (handle_SLmode_p_change, ms);

  ms->state.scanline.forcelimit.removeCallback
    (handle_SLforcelimit_p_change, ms);
  ms->state.scanline.max_z_step.removeCallback
    (handle_SLforcelimit_p_change, ms);
  ms->state.scanline.max_xy_step.removeCallback
    (handle_SLforcelimit_p_change, ms);
  ms->state.scanline.feedback_enabled.removeCallback
    (handle_SLforcelimit_change, ms);

  ms->state.scanline.start_linescan.removeCallback
            (handle_linescan_start, ms);
  ms->state.scanline.width.removeCallback
	    (handle_linescan_position, ms);
  ms->state.scanline.x_end.removeCallback
            (handle_linescan_position, ms);
  ms->state.scanline.y_end.removeCallback
            (handle_linescan_position, ms);
  ms->state.scanline.z_end.removeCallback
            (handle_linescan_position, ms);
  ms->state.scanline.angle.removeCallback
            (handle_linescan_position, ms);

  ms->state.scanline.showing_position.removeCallback
	    (handle_scanline_position_display_change, ms);
}

#define microscope ((nmm_Microscope_Remote *) _mptr)

void handle_doRelaxComp_change (vrpn_int32 val, void * _mptr) {
    if (val == VRPN_TRUE) {
	microscope->d_relax_comp.enable(nmm_RelaxComp::DECAY);
    } else if (val == VRPN_FALSE) {
	microscope->d_relax_comp.disable();
    } else {
	fprintf(stderr, "Unexpected value for doRelaxComp, %d,"
		" should be 1 or 0", val);
    }
    return;
}

void handle_scanning_change (vrpn_int32 , void * _mptr) {
    // Only allow this button to do something when we are already scanning. 
    if (microscope->state.acquisitionMode !=IMAGE) return;
    // Pause or resume the scan. 
    if(microscope->state.scanning) {
        microscope->ResumeFullScan();
    } else {
        microscope->PauseScan();
    }
    
}

void handle_grid_resolution_change (vrpn_int32, void * _mptr) {
    // Do some bounds checking to make sure the user's specified
    // grid resolution is reasonable. 

    // Thermo can handle minimum grid resolution of 2, max of 1000
    if (microscope->state.image.grid_resolution < 2) {
        microscope->state.image.grid_resolution = 2;
    } else if (microscope->state.image.grid_resolution > 1000) {
        microscope->state.image.grid_resolution = 1000;
    }
    microscope->SetGridSize(microscope->state.image.grid_resolution,
                            microscope->state.image.grid_resolution);
    return;
}

// Change by the user interface - angle in degrees. 
void handle_scan_angle_change (vrpn_float64, void * _mptr) {
    // Boundary check - clamp to inside range -360 to 360
    while (microscope->state.image.scan_angle >= 360.0) {
        microscope->state.image.scan_angle = 
            microscope->state.image.scan_angle - 360.0;
    }
    while (microscope->state.image.scan_angle <= -360.0) {
        microscope->state.image.scan_angle = 
           microscope->state.image.scan_angle + 360.0;
    }
    microscope->SetScanAngle(microscope->state.image.scan_angle);

}
// M stands for modify 
// I stands for image
// _p_ stands for parameters
void handle_Mmode_change (vrpn_int32, void * _mptr) {
  microscope->state.modify.mode_changed = VRPN_TRUE;
}

void handle_Mstyle_change (vrpn_int32, void * _mptr) {
  microscope->state.modify.style_changed = VRPN_TRUE;
}

void handle_Mtool_change (vrpn_int32, void * _mptr) {
  microscope->state.modify.tool_changed = VRPN_TRUE;
}

void handle_Mmode_p_change (vrpn_float64, void * _mptr) {
  microscope->state.modify.mode_p_changed = VRPN_TRUE;
}

void handle_Mmode_p_change (vrpn_int32, void * _mptr) {
  microscope->state.modify.mode_p_changed = VRPN_TRUE;
}

void handle_Mstyle_p_change (vrpn_float64, void * _mptr) {
  microscope->state.modify.style_p_changed = VRPN_TRUE;
}

void handle_Mtool_p_change (vrpn_float64, void * _mptr) {
  microscope->state.modify.tool_p_changed = VRPN_TRUE;
}


void handle_Imode_change (vrpn_int32, void * _mptr) {
  microscope->state.image.mode_changed = VRPN_TRUE;
}

void handle_Istyle_change (vrpn_int32, void * _mptr) {
  microscope->state.image.style_changed = VRPN_TRUE;
}

void handle_Imode_p_change (vrpn_float64, void * _mptr) {
  microscope->state.image.mode_p_changed = VRPN_TRUE;
}
void handle_Imode_p_change (vrpn_int32, void * _mptr) {
  microscope->state.image.mode_p_changed = VRPN_TRUE;
}

void handle_Istyle_p_change (vrpn_float64, void * _mptr) {
  microscope->state.image.style_p_changed = VRPN_TRUE;
}


/// Controls whether the slow scan direction is enabled,
/// or instead the microscope just repeatedly scans the same line.
void handle_tcl_scanEnable_change (vrpn_int32 _value, void * _mptr) {
  microscope->SetSlowScan(_value);
}


void handle_SLmode_change (vrpn_int32, void * _mptr) {
  microscope->state.scanline.mode_changed = VRPN_TRUE;
}

void handle_SLforcelimit_change (vrpn_int32, void * _mptr) {
  microscope->state.scanline.forcelimit_changed = VRPN_TRUE;
}

void handle_SLmode_p_change (vrpn_float64, void * _mptr) {
  //printf("handle_SLmode_p_change\n");
  microscope->state.scanline.mode_p_changed = VRPN_TRUE;
}
void handle_SLmode_p_change (vrpn_int32, void * _mptr) {
  //printf("handle_SLmode_p_change\n");
  microscope->state.scanline.mode_p_changed = VRPN_TRUE;
}

void handle_SLforcelimit_p_change (vrpn_float64, void * _mptr) {
  microscope->state.scanline.forcelimit_p_changed = VRPN_TRUE;
}

void handle_linescan_start (vrpn_int32 val, void *_mptr) {
  // we make sure we are at the starting location of the scanline before
  // changing the feedback parameters and starting the scan
  // This check is based on the switch to modify mode parameters using
  // ModifyMode() in interaction.c

  // Don't do anything if button hasn't been pressed in Tcl.
  if (val == 0) return;

  Point_value *value =
      microscope->state.data.inputPoint->getValueByPlaneName(
                            dataset->heightPlaneName->string());
  if (value == NULL) {
      fprintf(stderr, "handle_linescan_start():  "
                      "could not get input point!\n");
      return;
  }

  BCPlane *p = dataset->inputGrid->getPlaneByName(
                            dataset->heightPlaneName->string());
  if (p == NULL) {
	fprintf(stderr, "Error in handle_linescan_start:"
		" could not get plane!\n");
	return;
  }

  float start_x, start_y, start_z;	// start of currently selected line
  microscope->state.scanline.getStartPoint(p, &start_x, &start_y, &start_z);

  microscope->EnterScanlineMode();

  // wait until we get to the starting location before changing parameters
  printf("handle_linescan_start::waiting until start point is "
	 "reached before setting scanline parameters\n");
  microscope->TakeFeelStep(start_x, start_y, value, 1);
  // sleep to allow piezo relaxation
  sleep(2);

  microscope->SetScanlineModeParameters();
  microscope->AcquireScanline();
  //  now, until acquireScanline is implemented atomically on the server,
  // it is very important that we don't send any other messages until after
  // the scanline data is received
}

void handle_linescan_position (vrpn_float64, void *_mptr) {
  float p0[3], p1[3];

  BCPlane* plane = dataset->inputGrid->getPlaneByName(
                            dataset->heightPlaneName->string());
  if (plane == NULL)
  {
        fprintf(stderr, "Error in handle_linescan_position:"
		" could not get plane!\n");
        return;
  }

  microscope->state.scanline.getStartPoint(plane, &(p0[0]), &(p0[1]), &(p0[2]));
  microscope->state.scanline.getFinishPoint(plane, &(p1[0]), &(p1[1]), &(p1[2]));
  p0[2] *= plane->scale(); // show the height of the scanline in correct
  p1[2] *= plane->scale(); // relation to the surface

/*  printf("start: %f,%f,%f, end: %f,%f,%f\n", p0[0], p0[1], p0[2], 
												p1[0], p1[1], p1[2]);
*/
  graphics->setScanlineEndpoints(p0, p1);
}


void handle_scanline_position_display_change (vrpn_int32 _value,
                                              void * /*_mptr*/) {
  graphics->displayScanlinePosition(_value);
}

/// Update the grid scale and rebuild the display lists
/// whenever the Z scale changes.
void handle_z_scale_change (vrpn_float64 /*_value*/, void * _mptr) {
  BCPlane * plane =
    dataset->inputGrid->getPlaneByName(dataset->heightPlaneName->string());
  // If user is feeling from data at the same time that she is changing
  // the zscale, she could get a strong jerk from the phantom, so put her
  // into grab mode first
  if (plane) {
      if ((user_mode[0] == USER_PLANE_MODE) || (user_mode[0] == USER_PLANEL_MODE)) {
          handleCharacterCommand("G", &dataset->done, 1);
      }
    plane->setScale(microscope->state.stm_z_scale);
    //cause_grid_redraw(_value, _mptr);
    graphics->causeGridRedraw();
  }
  // update display of scanline to show true relative (yet scaled) height of
  // scanline with respect to the surface
  handle_linescan_position(0.0, _mptr);
  
  // Bring the surface in view
  center();
}

// Called when the user clicks "accept" in the Image frame
// in the main TCL window

void handle_image_accept (vrpn_int32, void * _mptr) {
    if (microscope->state.image.mode_p_changed ||
        microscope->state.image.mode_changed) {
      if (microscope->state.acquisitionMode == IMAGE){
        printf("Sending image mode parameters to SPM. \n");
        microscope->ImageMode();
      }
      microscope->state.image.mode_changed = VRPN_FALSE;
      microscope->state.image.mode_p_changed = VRPN_FALSE;
    }

    if (microscope->state.image.style_p_changed) {
      switch ((int)microscope->state.image.style) {
      case SHARP:
//      printf("  Sharp.\n");
        break;
      case BLUNT:
//      printf("  Blunt:\n");
//      printf("    blunt size:  %f\n",(float)microscope->state.image.blunt_size);
//      printf("    blunt speed: %f\n",(float)microscope->state.image.blunt_speed);
        break;
      default:
        printf("  Unknown imaging style!!!\n");
      }
      microscope->state.image.style_p_changed = VRPN_FALSE;
    }

    if (microscope->state.image.style_changed) {
//      printf("Change imaging style to  sharp(0), blunt (1): %d\n",
//           (int)microscope->state.image.style);
      microscope->state.image.style_changed = VRPN_FALSE;
    }

    if (microscope->state.image.mode_changed) {
//      printf("Change image mode to tapping(0), contact (1): %d\n",
//           (int)microscope->state.image.mode);
      microscope->state.image.mode_changed = VRPN_FALSE;
    }
}


/// Called when the user clicks "accept" in the Modify frame
/// in the main TCL window
void handle_modify_accept (vrpn_int32, void * _mptr) {
   if (microscope->state.modify.mode_changed) {
      
      microscope->state.modify.mode_changed = VRPN_FALSE;
   }
   // If the modify mode force changes, we want it sent to
   // the microscope RIGHT NOW, but ONLY if we are already in modify mode.
   if (microscope->state.modify.mode_p_changed){
      if (microscope->state.acquisitionMode == MODIFY) {
	 microscope->SetModForce();
      }
      // update screen decorations
      decoration->modSetpoint = microscope->state.modify.setpoint;

      microscope->state.modify.mode_p_changed = VRPN_FALSE;
   } 
    if (microscope->state.modify.style_p_changed) {
      switch ((int)microscope->state.modify.style) {
      case SHARP:
//      printf("  Sharp.\n");
        break;
      case BLUNT:
//      printf("  Blunt:\n");
//      printf("    blunt size:  %f\n",(float)microscope->state.modify.blunt_size);
//      printf("    blunt speed: %f\n",(float)microscope->state.modify.blunt_speed);
        break;
      case SWEEP:
//      printf("  Sweep:\n");
//      printf("    sweep width: %f\n",(float)microscope->state.modify.sweep_width);
        break;
      case SEWING:
//      printf("  Sewing:\n");
//      printf("    bottom delay: %f\n",(float)microscope->state.modify.bot_delay);
//      printf("    top delay:    %f\n",(float)microscope->state.modify.top_delay);
//      printf("    z pullout:    %f\n",(float)microscope->state.modify.z_pull);
//      printf("    punch dist:   %f\n",(float)microscope->state.modify.punch_dist);
//      printf("    speed:        %f\n",(float)microscope->state.modify.speed);
//      printf("    watchdog:     %f\n",(float)microscope->state.modify.watchdog);
        break;
      case FORCECURVE:
/*	printf("  Force Curve:\n");
	printf("    start delay:	%f\n",
			(float)microscope->state.modify.fc_start_delay);
	printf("    z start:		%f\n",
			(float)microscope->state.modify.fc_z_start);
	printf("    z end:		%f\n",
			(float)microscope->state.modify.fc_z_end);
	printf("    z pullback:		%f\n",
			(float)microscope->state.modify.fc_z_pullback);
	printf("    force limit:	%f\n",
			(float)microscope->state.modify.fc_force_limit);
	printf("    move dist:		%f\n",
			(float)microscope->state.modify.fc_movedist);
	printf("    num samples:	%d\n",
			(int)microscope->state.modify.fc_num_points);
	printf("    num halfcycles:	%d\n",
			(int)microscope->state.modify.fc_num_halfcycles);
	printf("    sample speed:	%f\n",
			(float)microscope->state.modify.fc_sample_speed);
	printf("    pullback speed:	%f\n",
			(float)microscope->state.modify.fc_pullback_speed);
	printf("    start speed:	%f\n",
			(float)microscope->state.modify.fc_start_speed);
	printf("    feedback speed:	%f\n",
			(float)microscope->state.modify.fc_feedback_speed);
*/
	break;
      default:
        printf("  Unknown modify style!!!\n");
      }
      microscope->state.modify.style_p_changed = VRPN_FALSE;
    }

    if (microscope->state.modify.tool_p_changed) {
      switch ((int)microscope->state.modify.tool) {
      case FREEHAND:
//      printf("  Freehand.\n");
        break;
      case LINE:
//      printf("  Line:\n");
//      printf("    step size:  %f\n",(float)microscope->state.modify.step_size);

        break;
      case CONSTR_FREEHAND:
//      printf("  Constrained Freehand:\n");
        break;
      case SLOW_LINE:
	break;
      default:
        printf("  Unknown modify tool!!!\n");
      }
      microscope->state.modify.tool_p_changed = VRPN_FALSE;
    }

    if (microscope->state.modify.style_changed) {
        // Take this out so we can detect a style change in
        // interaction.c - to display correct hand icons.
        //microscope->state.modify.style_changed = VRPN_FALSE;
    }
    if (microscope->state.modify.tool_changed) {
      microscope->state.modify.tool_changed = VRPN_FALSE;
    }
    if (microscope->state.modify.mode_changed) {
      microscope->state.modify.mode_changed = VRPN_FALSE;
    }
}

void handle_scanline_accept (vrpn_int32, void * _mptr) {
    //printf("handle_scanline_accept\n");

	// it is very important that we remain in SCANLINE mode until we hear
	// back from the server that we are not in scanline mode and then
	// immediately call ImageMode() to switch to IMAGE mode on server
	// (i.e. acquisitionMode should be synchronized between the client and
	// server, given the assumption that SCANLINE-->IMAGE, MODIFY-->IMAGE
	// IMAGE->SCANLINE, and IMAGE->MODIFY are the only allowed transitions)

    if (microscope->state.scanline.mode_p_changed ||
        microscope->state.scanline.mode_changed) {
      if (microscope->state.acquisitionMode == SCANLINE) {
        printf("Sending scanline mode parameters to SPM. \n");
        microscope->SetScanlineModeParameters();
      }
      microscope->state.scanline.mode_changed = VRPN_FALSE;
      microscope->state.scanline.mode_p_changed = VRPN_FALSE;
    }

    if (microscope->state.scanline.forcelimit_p_changed ||
	microscope->state.scanline.forcelimit_changed) {
      microscope->state.scanline.forcelimit_p_changed = VRPN_FALSE;
      microscope->state.scanline.forcelimit_changed = VRPN_FALSE;
    }
}

/** This routine causes the entire grid to be redrawn at the next loop
 iteration.
 It also repositions the red, green and blue measurement lines so
 that they track the surface with all of its changes.
 The reason for the parameters is so that this can be set up as a
 Tcl_Linkvar callback, which requires them.
*/
void cause_grid_redraw (vrpn_float64, void *) {

#if 1

  graphics->causeGridRedraw();

#else

  BCPlane * plane = dataset->inputGrid->getPlaneByName
                                (dataset->heightPlaneName->string());

        // Force a redraw of the entire surface
        dataset->range_of_change.ChangeAll();

        // Move the measure lines to the correct new position
        if (plane != NULL) {
                //decoration->red_bot[X] = decoration->red_top[X];
                //decoration->red_bot[Y] = decoration->red_top[Y];
                //decoration->blue_bot[X] = decoration->blue_top[X];
                //decoration->blue_bot[Y] = decoration->blue_top[Y];
                //decoration->green_bot[X] = decoration->green_top[X];
                //decoration->green_bot[Y] = decoration->green_top[Y];
                //decoration->red_top[Z] = decoration->green_top[Z] =
                     //decoration->blue_top[Z] =
                            //plane->maxAttainableValue() * plane->scale();
                //decoration->red_bot[Z] = decoration->green_bot[Z] =
                     //decoration->blue_bot[Z] =
                            //plane->minAttainableValue() * plane->scale();
            //decoration->red_changed = 1;
            //decoration->green_changed = 1;
            //decoration->blue_changed = 1;
            decoration->red.normalize(plane);
            decoration->green.normalize(plane);
            decoration->blue.normalize(plane);
        }

#endif

}

/// Export a new .txt file whenever the user selects a new data set
void    handle_export_dataset_change(const char *new_value, void * )
{
        BCPlane *plane = dataset->inputGrid->getPlaneByName(new_value);
        char    filename[1000];
        int     x,y;
        FILE    *f;

        // Bail if we do not have a plane to write
        if (plane == NULL) { return; }

        // Find the name of the file and then open it for writing.
        sprintf(filename, "%s.txt",new_value);
        if ( (f = fopen(filename,"w")) == NULL) {
                fprintf(stderr,"Can't open %s for write", filename);
                perror("");
                return;
        }

        // Write the plane into the file
        for (y = 0; y < plane->numY(); y++) {
          for (x = 0; x < plane->numX(); x++) {
                fprintf(f,"%g ", plane->value(x,y));
          }
          fprintf(f,"\n");
        }
        // Close the file
        fclose(f);
}

void    handle_z_dataset_change(const char *, void * _mptr)
{
  BCPlane * plane = dataset->inputGrid->getPlaneByName
    (dataset->heightPlaneName->string());
  
  /* if user is feeling from data at the same time she is changing the
     grid dataset, could get a strong jerk from the phantom, so put her
     into grab mode first */
  if ((user_mode[0] == USER_PLANE_MODE) || (user_mode[0] == USER_PLANEL_MODE))
    {
      handleCharacterCommand("G", &dataset->done, 1);
    }
  
  // If the plane cannot be found, look for another one instead.
  // We NEED to have a height plane.
  if (plane == NULL) { 
    plane = dataset->ensureHeightPlane(); 
  }
  
  // Set the scale to that of the one we just selected.
  microscope->state.stm_z_scale = plane->scale();
  
  graphics->setHeightPlaneName(dataset->heightPlaneName->string());
  graphics->causeGridRebuild();
  graphics->causeGridRedraw();
}



void    handle_color_dataset_change(const char *, void * /*_mptr*/)
{
  //changed from using *NonZeroValue to *Value to match settings for
  //haptics parameters

    BCPlane * plane = dataset->inputGrid->getPlaneByName
                          (dataset->colorPlaneName->string());

    if (plane != NULL) {
	// Try to set this to a useful value for a stream file
	// minAttainableValue is much too small to be useful, except
	// for Topo files.
	// So, only use these if they are within a reasonable distance
	// of the data range.
        // Use plane->m*NonZeroValue(), as opposed to plane->m*Value(), so
        // that we get reasonable values before the 1st scan of the surface
        // finishes.
      float range = plane->maxNonZeroValue() - plane->minNonZeroValue();
//        printf("Colormap Min %f max %f range %f\n", plane->minNonZeroValue(),
//  	     plane->maxNonZeroValue(), range);
      // We set min_limit = min_value - range and max_limit = max_value +
      // range to account for drifting.  Yes, we know this is a kluge.

      /*
      printf("minAttainable = %f, maxAttainable = %f\n",
             plane->minAttainableValue(), plane->maxAttainableValue());

      if ( plane->minAttainableValue() > (plane->minNonZeroValue() - range)){
	    color_min_limit = plane->minAttainableValue();
      } else {
	color_min_limit = (plane->minNonZeroValue() - range);
      }
      if ( plane->maxAttainableValue() < (plane->maxNonZeroValue() + range)){
	color_max_limit = plane->maxAttainableValue();
      } else {
	color_max_limit = (plane->maxNonZeroValue() + range);
      }
      */
      color_min_limit = plane->minNonZeroValue();
      color_max_limit = plane->maxNonZeroValue();
    }
    else {  // so selected data set is "none"
        color_min_limit = 0;
        color_max_limit = 1;
    }

    graphics->setColorPlaneName(dataset->colorPlaneName->string());
    graphics->causeGridReColor();
}

void     handle_sound_dataset_change(const char *, void * )
{

  BCPlane * plane = dataset->inputGrid->getPlaneByName
                         (soundPlaneName.string());

  if (plane != NULL)
    {
      sound_slider_min_limit = plane->minAttainableValue();
      sound_slider_min = plane->minNonZeroValue();
      sound_slider_max_limit = plane->maxAttainableValue();
      sound_slider_max = plane->maxNonZeroValue();
    }

  else
    {
      /* turn off sound */
      sound_slider_min_limit = 0;
      sound_slider_min = 0;
      sound_slider_max_limit = 0;
      sound_slider_max = 0;
    }
}

// Controls for the SLOW LINE tool

/** Call this function right after the user has hit "commit" for the
 slow-line tool - they are ready to hit Play or Step to move the
 tip. */
void init_slow_line (void * _mptr) 
{
  microscope->state.modify.slow_line_position_param = 0.0;

  Position_list & p = microscope->state.modify.stored_points;
  p.start();
  p.next();
  microscope->state.modify.slow_line_currPt =  p.curr();
  microscope->state.modify.slow_line_prevPt = p.peekPrev();
  // send barrier message to detect end of relaxation points
//  printf("sending barrier synch request for slow line\n");
  microscope->requestSynchronization(0, 0, RELAX_MSG);
}

/** If the line has been specified, and commit has been pressed, start
 taking steps along that line. Otherwise, do nothing. The line is
 specified in interaction.c in the doLine function. */
void	handle_slow_line_playing_change (vrpn_int32, void * _mptr)
{
  if (microscope->state.modify.tool != SLOW_LINE) return;
  if (microscope->state.modify.slow_line_committed != VRPN_TRUE) return;

  // Instead of replicating code, we will just call the function that
  // takes a step, as if the user had pressed the "step" button, but
  // the "play" flag (set in Tcl) tells the point result handler to
  // keep stepping forward.
  //printf("Slow line play\n");
  //handle_slow_line_step_change(1, _mptr);

}

/** If the line has been specified, and commit has been pressed, take
one step along that line. Otherwise, do nothing. If we are in
"play" mode, and we reach the end of our line segment, turn "play"
off. Always take a step, so the slow_line_ReceiveNewPoint fcn has a
chance to request another point. */
void	handle_slow_line_step_change (vrpn_int32, void * _mptr)
{
  if (microscope->state.modify.tool != SLOW_LINE) return;
  if (microscope->state.modify.slow_line_committed != VRPN_TRUE) return;
  
  // XXX maybe take this out, so function can be used for "play"
  //if (microscope->state.modify.slow_line_playing == VRPN_TRUE)
  //  microscope->state.modify.slow_line_playing = VRPN_FALSE;
//    printf("Slow line step, position %f, dir %d\n", 
//  	 microscope->state.modify.slow_line_position_param, 
//  	 (int)microscope->state.modify.slow_line_direction);
  // calculate the step size in parameter space. 
  // Stepping from point 1 to point 2.
  float x1 =  microscope->state.modify.slow_line_prevPt->x();
  float y1 =  microscope->state.modify.slow_line_prevPt->y();
  float x2 =  microscope->state.modify.slow_line_currPt->x();
  float y2 =  microscope->state.modify.slow_line_currPt->y();
  float line_length = sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));
  float param_step = microscope->state.modify.step_size / line_length;
  
  Position_list & p = microscope->state.modify.stored_points;
  if (microscope->state.modify.slow_line_direction == FORWARD) {
    if ( microscope->state.modify.slow_line_position_param == 1.0) {
        if ( !p.peekNext() ) {
            microscope->state.modify.slow_line_playing = VRPN_FALSE;
        }
        else {
            p.next();
            microscope->state.modify.slow_line_currPt = p.curr();
            microscope->state.modify.slow_line_prevPt = p.peekPrev();
            microscope->state.modify.slow_line_position_param = 0;
        }
    } else {
      microscope->state.modify.slow_line_position_param += param_step;
    }
  } else if (microscope->state.modify.slow_line_direction == REVERSE) {
    if ( microscope->state.modify.slow_line_position_param == 0.0){
        p.prev();
        if ( !p.peekPrev() ) {
            microscope->state.modify.slow_line_playing = VRPN_FALSE;
            p.next();
        }
        else {
            microscope->state.modify.slow_line_prevPt = p.peekPrev();
            microscope->state.modify.slow_line_currPt = p.curr();
            microscope->state.modify.slow_line_position_param = 1.0;
        }
    } else {
      microscope->state.modify.slow_line_position_param -= param_step;
    }
  }

  // Do some bounds checking on the line param.  0 <= t <= 1
  if (microscope->state.modify.slow_line_position_param > 1.0) {
    microscope->state.modify.slow_line_position_param = 1.0;
  }
  if (microscope->state.modify.slow_line_position_param < 0.0) {
    microscope->state.modify.slow_line_position_param = 0.0;
  } 
  // Take a normal step. 

// Following is commented out because we already should have a request sent
// which will trigger the next request with this new point - this gets
// started from nmm_Microscope_Remote::handle_barrierSynch

  // find the position our parameter corresponds to .
/*  float x = x2*(microscope->state.modify.slow_line_position_param) +
            x1*(1.0-microscope->state.modify.slow_line_position_param);
  float y = y2*(microscope->state.modify.slow_line_position_param) +
            y1*(1.0-microscope->state.modify.slow_line_position_param);
*/
  //printf("Slow line, start %f %f, stop %f %f, step to %f %f \n", 
  //	 x1, y1, x2, y2, x, y);
  // Take the step. Note: we _always_ want to take a step, so that we
  // get back point results, the slow_line_ReceiveNewPoint fcn is
  // called, and we can request more point results.
//  microscope->TakeModStep(x,y);
  

}

/** If we are "playing", then pause, so that the next time we hit
    "play" we will go in the other direction. */
void	handle_slow_line_direction_change (vrpn_int32, void * _mptr)
{
  if (microscope->state.modify.tool != SLOW_LINE) return;
  if (microscope->state.modify.slow_line_committed != VRPN_TRUE) return;
  if (microscope->state.modify.slow_line_playing == VRPN_TRUE)
    microscope->state.modify.slow_line_playing = VRPN_FALSE;
  //printf("Slow line direction\n");

  return;
  
}

/** This is a point result callback, registered with the microscope
object. Whenever we receive point results, this function will get
called.  Only do something if we are doing a Slow Line tool
modification, and user has hit the "commit" button.  

Either take a step along the line segment, if "play" has been
pressed, or take another measurment at the same point. */
int slow_line_ReceiveNewPoint (void * _mptr, const Point_results *)
{
  if (microscope->state.modify.tool != SLOW_LINE) return 0;
  if (microscope->state.modify.slow_line_committed != VRPN_TRUE) return 0;
  if (microscope->state.modify.slow_line_relax_done != VRPN_TRUE) return 0;
  // We don't want to send out new requests if the AFM is relaxing. 
//  if (microscope->d_relax_comp.is_ignoring_points()) return 0;


  if (microscope->state.modify.slow_line_playing == VRPN_TRUE) {
    //printf("slow_line: got point results, step to next %f %f\n", 
    //	   p->x(), p->y());
    handle_slow_line_step_change(1, _mptr);
  } 

// this should always be done while in SLOW_LINE now that 
// handle_slow_line_step_change doesn't send point requests
//else {
    //printf("slow_line: got point results, stay at %f %f\n", 
    //	   p->x(), p->y());
    // Take the step to the same place we just were.  We can't just
    // use p->x() and p->y() because more than one point result could
    // be in transit to us, resulting in a loop of two or three
    // points.
    // Re-calcs the point we want to go to. 

    float x1 =  microscope->state.modify.slow_line_prevPt->x();
    float y1 =  microscope->state.modify.slow_line_prevPt->y();
    float x2 =  microscope->state.modify.slow_line_currPt->x();
    float y2 =  microscope->state.modify.slow_line_currPt->y();
    
    // Set yaw so if we sweep it will be perpendicular to the slow-line path. 
    microscope->state.modify.yaw = atan2((y2 - y1), (x2 - x1)) - M_PI_2;

    float x = x2*(microscope->state.modify.slow_line_position_param) +
              x1*(1.0-microscope->state.modify.slow_line_position_param);
    float y = y2*(microscope->state.modify.slow_line_position_param) +
              y1*(1.0-microscope->state.modify.slow_line_position_param);
 
    microscope->TakeModStep(x,y);
 // }
  return 0;
}

