/*===3rdtech===
  Copyright (c) 2000 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#ifndef _H_MICROSCOPE_HANDLERS
#define _H_MICROSCOPE_HANDLERS

#include <vrpn_Types.h>

class nmm_Microscope_Remote;

// These handlers take a pointer to a Microscope as their userdata.
// Thus, they need not access the global 'microscope'.
// This helps us prepare for multiple simultaneous Microscopes.

void setupStateCallbacks (nmm_Microscope_Remote *);
void teardownStateCallbacks (nmm_Microscope_Remote *);

void handle_doRelaxComp_change (vrpn_int32 val, void * _mptr);
void handle_grid_resolution_change (vrpn_int32, void * _mptr);
void handle_scan_angle_change (vrpn_float64, void * _mptr);
void handle_scanning_change (vrpn_int32, void * _mptr);
void handle_withdraw_tip_change (vrpn_int32, void * _mptr);

void handle_Mmode_change (vrpn_int32, void * _mptr);
void handle_Mstyle_change (vrpn_int32, void * _mptr);
void handle_Mtool_change (vrpn_int32, void * _mptr);
void handle_Mmode_p_change (vrpn_float64, void * _mptr);
void handle_Mmode_p_change (vrpn_int32, void * _mptr);
void handle_Mstyle_p_change (vrpn_float64, void * _mptr);
void handle_Mtool_p_change (vrpn_float64, void * _mptr);
void handle_Imode_change (vrpn_int32, void * _mptr);
void handle_Istyle_change (vrpn_int32, void * _mptr);
void handle_Imode_p_change (vrpn_float64, void * _mptr);
void handle_Imode_p_change (vrpn_int32, void * _mptr);
void handle_Istyle_p_change (vrpn_float64, void * _mptr);

void handle_tcl_scanEnable_change (vrpn_int32, void * _mptr);

void handle_SLmode_change (vrpn_int32, void * _mptr);
void handle_SLforcelimit_change (vrpn_int32, void * _mptr);
void handle_SLmode_p_change (vrpn_float64, void * _mptr);
void handle_SLmode_p_change (vrpn_int32, void * _mptr);
void handle_SLforcelimit_p_change (vrpn_float64, void * _mptr);
void handle_linescan_start (vrpn_int32, void *_mptr);
void handle_linescan_position (vrpn_float64 , void *_mptr);
void handle_scanline_position_display_change(vrpn_int32, void *_mptr);

void handle_z_scale_change (vrpn_float64, void * _mptr);

void handle_image_accept (vrpn_int32, void * _mptr);
void handle_modify_accept (vrpn_int32, void * _mptr);
void handle_scanline_accept (vrpn_int32, void * _mptr);

void cause_grid_redraw (vrpn_float64, void *);  // argument not used!

void handle_export_dataset_change (const char *, void * _mptr);
void handle_z_dataset_change (const char *, void * _mptr);

// Slow line tool callbacks.
void init_slow_line (void * _mptr);
void handle_slow_line_playing_change (vrpn_int32 new_value, void * _mptr);
void handle_slow_line_step_change (vrpn_int32 new_value, void * _mptr);
void handle_slow_line_direction_change (vrpn_int32 new_value, void * _mptr);
int slow_line_ReceiveNewPoint (void * _mptr, const Point_results * p);

int handle_feelahead_numX_change (vrpn_int32, void *);
int handle_feelahead_numY_change (vrpn_int32, void *);
int handle_feelahead_distX_change (vrpn_int32, void *);
int handle_feelahead_distY_change (vrpn_int32, void *);

#endif  // _H_MICROSCOPE_HANDLERS



