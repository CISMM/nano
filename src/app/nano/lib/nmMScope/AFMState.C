/*===3rdtech===
  Copyright (c) 2000 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#include "AFMState.h"

#include "nmm_Types.h"

#include "BCPlane.h"

#ifndef M_PI
#define M_PI 3.141592653589793238
#endif

/// Default grid size on startup. Kept to a minimum size.
/// 12 because that allows a tesselation_stride of up to 6 without crashing.
#define DATA_SIZE 12



/** \file AFMState.C

Tom Hudson, September, 1997
Taken from global variables declared in microscape.c, animate.c,
interaction.c, and elsewhere.

These structs gather many of the global variables scattered throughout
the code into a single hierarchy

The default parameters given here are, or were once, "reasonable",
according to the physicists.

*/

// juliano 9/19/99 added conditional around this defn to supress warning
#ifndef max  
#define max(a,b) ((a)<(b)?(b):(a))
#endif



AFMModifyInitializationState::AFMModifyInitializationState (void) :
  mode (CONTACT),
  style (SHARP),
  setpoint (0.0),
  setpoint_min (0.0),
  setpoint_max (25.0),
  amplitude (1.0),
  amplitude_min (0.0),
  amplitude_max (1.0),
  std_dev_samples (1),
  std_dev_frequency (50000.0)
{

}

/** ModifyState:
  State of the microscope during modification.
  This is explicitly kept separate from image mode state so that when
the user changes back and forth between the two she does not have to
completely reset setpoint, amplitude, and things like that.
*/
AFMModifyState::AFMModifyState (const AFMModifyInitializationState & i) :
    std_dev_samples (i.std_dev_samples ),
    std_dev_samples_cache (1),
    std_dev_frequency (i.std_dev_frequency ),

    mode_changed    (VRPN_FALSE),
    style_changed   (VRPN_FALSE),
    tool_changed    (VRPN_FALSE),
    mode_p_changed  (VRPN_FALSE),
    style_p_changed (VRPN_FALSE),
    tool_p_changed  (VRPN_FALSE),

    mode ("modifyp_mode", i.mode ),
    new_mode ("newmodifyp_mode", i.mode ),
    control ("modifyp_control", FEEDBACK ),
    new_control ("newmodifyp_control", FEEDBACK ),
    constr_xyz_param ("modifyp_constr_xyz_mode", CONSTR_XYZ_LINE ),
    new_constr_xyz_param ("newmodifyp_constr_xyz_mode", CONSTR_XYZ_LINE ),
    optimize_now_param ("modifyp_optimize_now", OPTIMIZE_NOW_LINE ),
    new_optimize_now_param ("newmodifyp_optimize_now", OPTIMIZE_NOW_LINE ),
    style ("modifyp_style", i.style ),
    new_style ("newmodifyp_style", i.style ),
    tool ("modifyp_tool", FREEHAND),
    new_tool ("newmodifyp_tool", FREEHAND),

    setpoint ("modifyp_setpoint", i.setpoint ),
    new_setpoint ("newmodifyp_setpoint", i.setpoint ),
    setpoint_min (i.setpoint_min ),
    setpoint_max (i.setpoint_max ),
    p_gain ("modifyp_p_gain", 1.0),
    new_p_gain ("newmodifyp_p_gain", 1.0),
    i_gain ("modifyp_i_gain", 0.5),
    new_i_gain ("newmodifyp_i_gain", 0.5),
    d_gain ("modifyp_d_gain", 0.0),
    new_d_gain ("newmodifyp_d_gain", 0.0),
    amplitude ("modifyp_amplitude", i.amplitude ),
    new_amplitude ("newmodifyp_amplitude", i.amplitude ),
    amplitude_min (i.amplitude_min ),
    amplitude_max (i.amplitude_max ),
    frequency("modifyp_frequency", 100.0),
    new_frequency("newmodifyp_frequency", 100.0),
    input_gain("modifyp_input_gain", 1),
    new_input_gain("newmodifyp_input_gain", 1),
    ampl_or_phase("modifyp_ampl_or_phase", 1),   // 1 for ampl, 0 for phase
    new_ampl_or_phase("newmodifyp_ampl_or_phase", 1),// 1 for ampl, 0 for phase
    drive_attenuation("modifyp_drive_attenuation", 1),
    new_drive_attenuation("newmodifyp_drive_attenuation", 1),
    phase("modifyp_phase", 0.0),
    new_phase("newmodifyp_phase", 0.0),
    scan_rate_microns ("modifyp_rate", 20.0),
    new_scan_rate_microns ("newmodifyp_rate", 20.0),

    sweep_width ("modifyp_sweep_width", 500.0),
    new_sweep_width ("newmodifyp_sweep_width", 500.0),
    region_diag (282.8),  // length of diagonal, assuming 200x200
    yaw (0.0),

    bot_delay ("modifyp_bot_delay", 1.0),
    new_bot_delay ("newmodifyp_bot_delay", 1.0),
    top_delay ("modifyp_top_delay", 1.0),
    new_top_delay ("newmodifyp_top_delay", 1.0),
    z_pull ("modifyp_z_pull", 50.0),
    new_z_pull ("newmodifyp_z_pull", 50.0),
    punch_dist ("modifyp_punchdist", 2.0),
    new_punch_dist ("newmodifyp_punchdist", 2.0),
    speed ("modifyp_speed", 200.0),
    new_speed ("newmodifyp_speed", 200.0),
    watchdog ("modifyp_watchdog", 100.0),
    new_watchdog ("newmodifyp_watchdog", 100.0),

    fc_start_delay("modifyp_start_delay", 50.0),
    new_fc_start_delay("newmodifyp_start_delay", 50.0),
    fc_z_start("modifyp_z_start", -1000.0),
    new_fc_z_start("newmodifyp_z_start", -1000.0),
    fc_z_end("modifyp_z_end", 0.0),
    new_fc_z_end("newmodifyp_z_end", 0.0),
    fc_z_pullback("modifyp_z_pullback", -1000.0),
    new_fc_z_pullback("newmodifyp_z_pullback", -1000.0),
    fc_force_limit("modifyp_force_limit", 32),
    new_fc_force_limit("newmodifyp_force_limit", 32),
    fc_movedist("modifyp_fcdist", 100.0),
    new_fc_movedist("newmodifyp_fcdist", 100.0),
    fc_num_points("modifyp_num_layers", 150),
    new_fc_num_points("newmodifyp_num_layers", 150),
    fc_num_halfcycles("modifyp_num_hcycles",2),
    new_fc_num_halfcycles("newmodifyp_num_hcycles",2),
    fc_sample_speed("modifyp_sample_speed", 0.1),
    new_fc_sample_speed("newmodifyp_sample_speed", 0.1),
    fc_pullback_speed("modifyp_pullback_speed", 1.0),
    new_fc_pullback_speed("newmodifyp_pullback_speed", 1.0),
    fc_start_speed("modifyp_start_speed", 0.1),
    new_fc_start_speed("newmodifyp_start_speed", 0.1),
    fc_feedback_speed("modifyp_feedback_speed", 1.0),
    new_fc_feedback_speed("newmodifyp_feedback_speed", 1.0),
    fc_avg_num("modifyp_avg_num", 3.0),
    new_fc_avg_num("newmodifyp_avg_num", 3.0),
    fc_sample_delay("modifyp_sample_delay", 0.0),
    new_fc_sample_delay("newmodifyp_sample_delay", 0.0),
    fc_pullback_delay("modifyp_pullback_delay", 0.0),
    new_fc_pullback_delay("newmodifyp_pullback_delay", 0.0),
    fc_feedback_delay("modifyp_feedback_delay", 200.0),
    new_fc_feedback_delay("newmodifyp_feedback_delay", 200.0),

    step_size ("modifyp_step_size", 1.0),
    new_step_size ("newmodifyp_step_size", 1.0),

    max_z_step("modifyp_max_z_step", 1.0),
    new_max_z_step("newmodifyp_max_z_step", 1.0),
    max_xy_step("modifyp_max_xy_step", 1.0),
    new_max_xy_step("newmodifyp_max_xy_step", 1.0),
    min_z_setpoint("modifyp_min_z_setpoint", -10.0),
    new_min_z_setpoint("newmodifyp_min_z_setpoint", -10.0),
    max_z_setpoint("modifyp_max_z_setpoint", 10.0),
    new_max_z_setpoint("newmodifyp_max_z_setpoint", 10.0),
    max_lat_setpoint("modifyp_max_lat_setpoint", 10.0),
    new_max_lat_setpoint("newmodifyp_max_lat_setpoint", 10.0),
    freespace_normal_force(BOGUS_FORCE),
    freespace_lat_force(BOGUS_FORCE),

    stored_points (),

    constr_line_specified(VRPN_FALSE),

    slow_line_committed(VRPN_FALSE),
    slow_line_playing("slow_line_playing", 0),
    slow_line_step("slow_line_step",0),
    slow_line_direction("slow_line_direction", FORWARD),
    slow_line_position_param(0.0),
    slow_line_currPt(NULL),
    slow_line_prevPt(NULL),
    slow_line_relax_done(VRPN_FALSE),

    modify_enabled (VRPN_FALSE),

    blunt_size ("modifyp_tri_size", 1.0),
    new_blunt_size ("newmodifyp_tri_size", 1.0),
    blunt_speed ("modifyp_tri_speed", 5000.0),
    new_blunt_speed ("newmodifyp_tri_speed", 5000.0)

{

//fprintf(stderr, "AFMModifyState constructor\n");
}

// Destructor only here to keep the HP compilers happy.
// (If none were given they'd create a default destructor and attempt to
//  inline it, but it's too big to inline, so they issue warning messages.)
AFMModifyState::~AFMModifyState (void) {

}




AFMImageInitializationState::AFMImageInitializationState (void) :
  mode (TAPPING),
  grid_resolution(DATA_SIZE),
  setpoint (50.0),
  setpoint_max (0.0),
  setpoint_min (50.0),
  amplitude (1.0),
  amplitude_min (0.0),
  amplitude_max (1.0)
{

}
/**
ImageState:
  State of the microscope during imaging.
  This is explicitly kept separate from modify mode state so that when
the user changes back and forth between the two she does not have to
completely reset setpoint, amplitude, and things like that.
*/
AFMImageState::AFMImageState (const AFMImageInitializationState & i) :
  mode_changed    (VRPN_FALSE),
  style_changed   (VRPN_FALSE),
  mode_p_changed  (VRPN_FALSE),
  style_p_changed (VRPN_FALSE),
  
  mode ("imagep_mode", i.mode ),
  new_mode( "newimagep_mode", i.mode ),
  style ("imagep_style", SHARP),
  new_style( "newimagep_style", SHARP ),
  tool ("imagep_tool", FREEHAND),
  new_tool( "newimagep_tool", FREEHAND ),
     
  grid_resolution("imagep_grid_resolution", i.grid_resolution),
  new_grid_resolution( "newimagep_grid_resolution", i.grid_resolution ),
  scan_angle("imagep_scan_angle", 0),
  new_scan_angle( "newimagep_scan_angle", 0 ),
     
  setpoint ("imagep_setpoint", i.setpoint ),
  new_setpoint( "newimagep_setpoint", i.setpoint ),
  setpoint_min (i.setpoint_min ),
  setpoint_max (i.setpoint_max ),
  p_gain ("imagep_p_gain", 1.0),
  new_p_gain( "newimagep_p_gain", 1.0 ),
  i_gain ("imagep_i_gain", 0.30),
  new_i_gain( "newimagep_i_gain", 0.30 ),
  d_gain ("imagep_d_gain", 0.0),
  new_d_gain( "newimagep_d_gain", 0.0 ),
  amplitude ("imagep_amplitude", i.amplitude ),
  new_amplitude( "newimagep_amplitude", i.amplitude ),
  amplitude_min (i.amplitude_min ),
  amplitude_max (i.amplitude_max ),
  frequency("imagep_frequency", 100.0),
  new_frequency( "newimagep_frequency", 100.0 ),
  input_gain("imagep_input_gain", 1),
  new_input_gain( "newimagep_input_gain", 1 ),
  ampl_or_phase("imagep_ampl_or_phase", 1),   // 1 for ampl, 0 for phase
  new_ampl_or_phase( "newimagep_ampl_or_phase", 1 ),
  drive_attenuation("imagep_drive_attenuation", 1),
  new_drive_attenuation( "newimagep_drive_attenuation", 1 ),
  phase("imagep_phase", 0.0),
  new_phase( "newimagep_phase", 0.0 ),
  scan_rate_microns ("imagep_rate", 1.0),
  new_scan_rate_microns( "newimagep_rate", 1.0 ),
  
  blunt_size ("imagep_tri_size", 1.0),
  blunt_speed ("imagep_tri_speed", 5000.0)
  
{
  

//fprintf(stderr, "AFMImageState constructor\n");
}

// Destructor only here to keep the HP compilers happy.
AFMImageState::~AFMImageState (void) {

}

AFMScanlineInitializationState::AFMScanlineInitializationState(void) :
  mode (TAPPING),
  feedback_enabled (VRPN_TRUE),
  forcelimit_enabled (VRPN_FALSE),
  setpoint (1.0),
  setpoint_min (50.0),
  setpoint_max (0.0),
  amplitude (0.1),
  amplitude_min (0.0),
  amplitude_max (1.0) 
{

}

AFMScanlineState::AFMScanlineState(const AFMScanlineInitializationState &i) :
    mode_changed (VRPN_FALSE),
    forcelimit_changed (VRPN_FALSE),
    mode_p_changed  (VRPN_FALSE),
    forcelimit_p_changed (VRPN_FALSE),

    mode ("scanlinep_mode", i.mode ),
    feedback_enabled ("scanlinep_use_feedback", i.feedback_enabled),
    forcelimit_enabled ("scanlinep_use_forcelimit", i.forcelimit_enabled),
    continuous_rescan("scanlinep_continuous", 1),
    start_linescan("start_linescan", 0),
    showing_position("display_scanline_position", 0),
    resolution("scanlinep_resolution", 100),
    setpoint ("scanlinep_setpoint", i.setpoint),
    setpoint_min (i.setpoint_min ),
    setpoint_max (i.setpoint_max ),
    p_gain ("scanlinep_p_gain", 1.0),
    i_gain ("scanlinep_i_gain", 0.50),
    d_gain ("scanlinep_d_gain", 0.01),
    width("scanlinep_width", 100),
    amplitude ("scanlinep_amplitude", i.amplitude ),
    amplitude_min (i.amplitude_min ),
    amplitude_max (i.amplitude_max ),
    frequency("scanlinep_frequency", 100.0),
    input_gain("scanlinep_input_gain", 1),
    ampl_or_phase("scanlinep_ampl_or_phase", 1),   // 1 for ampl, 0 for phase
    drive_attenuation("scanlinep_drive_attenuation", 1),
    phase("scanlinep_phase", 0.0),
    scan_rate_microns_per_sec ("scanlinep_rate", 20.0),
    forcelimit("scanlinep_forcelimit", 10),
    max_z_step("scanlinep_max_z_step", 1),
    max_xy_step("scanlinep_max_xy_step", 1),
    num_scanlines_to_receive(0),
    x_end("scanlinep_endpos_x",0.0), y_end("scanlinep_endpos_y",0.0), 
    z_end("scanlinep_endpos_z",0.0), angle("scanlinep_angle",0.0),
    slope_nm_per_micron("scanlinep_slope_nm_per_micron", 0.0)
{

}

AFMScanlineState::~AFMScanlineState (void) {

}

void AFMScanlineState::getStartPoint(BCPlane *p, float *x, float *y, float *z){
	// convert width, x_end, y_end and z_end into nM for the plane p
	float width_nm = 0.01*(float)width*(p->maxX() - p->minX());
	float x_end_nm, y_end_nm, z_end_nm;
	getFinishPoint(p, &x_end_nm, &y_end_nm, &z_end_nm);

    *x = x_end_nm - width_nm*cos(angle*M_PI/180.0);
    *y = y_end_nm - width_nm*sin(angle*M_PI/180.0);
    *z = (float)z_end_nm - width_nm*slope_nm_per_micron*0.001;
/*	printf("end: (%g,%g,%g), width: %g, angle: %g\n",
			(float)x_end, (float)y_end, (float)z_end, (float)width, 
			(float)angle);
*/
}

void AFMScanlineState::getFinishPoint(BCPlane *p, float *x, float *y, float *z){
	// convert percentages of scan region to nm
	*x = 0.01*(float)x_end*(p->maxX() - p->minX()) + p->minX();
    *y = 0.01*(float)y_end*(p->maxY() - p->minY()) + p->minY();
	*z = z_end;
    return;
}

/// moves scanline in the scanline coordinate system
void AFMScanlineState::moveScanlineRelative(float dist_fast_dir, 
											float dist_slow_dir) {
	float cosa = cos(angle*M_PI/180.0);
	float sina = sin(angle*M_PI/180.0);
    x_end = (float)x_end + dist_fast_dir*cosa + dist_slow_dir*sina;
	y_end = (float)y_end + dist_fast_dir*sina - dist_slow_dir*cosa;
}

/**
 Dataset:
   Lumps together a lot of the things that seemed sensible to keep
 in one place as pertaining to the data rather than the 'scope itself.
 This will change in the future.
*/
AFMDataset::AFMDataset (void) :
    inputPoint (NULL),
    fc_inputPoint(NULL),
    inputPointNames ("inputPointNames"),
    scan_channels (NULL),
    point_channels (NULL),
    forcecurve_channels(NULL) {

//fprintf(stderr, "AFMDatasetState constructor\n");

}

// Destructor only here to keep the HP compilers happy.
AFMDataset::~AFMDataset (void) {
    if (inputPoint) delete inputPoint;
    if (fc_inputPoint) delete fc_inputPoint;
    // for some reason I haven't tracked down, these call a segfault. 
    //if (scan_channels) delete scan_channels;
    //if (point_channels) delete point_channels;
    //if (forcecurve_channels) delete forcecurve_channels;
}

AFMGuardedScanState::AFMGuardedScanState(void) :
	fNormalX(0.0f), fNormalY(0.0f), fNormalZ(0.0f),
	fPlaneD(0.0f), fGuardDepth(0.0f)
{
}

AFMGuardedScanState::~AFMGuardedScanState()
{
}

AFMInitializationState::AFMInitializationState (void) :

  stm_z_scale (1.0f),

  doRelaxComp (VRPN_TRUE),
  doRelaxUp (VRPN_FALSE),
  doDriftComp (VRPN_FALSE),
  doSplat (VRPN_FALSE),
  readingStreamFile (VRPN_FALSE),
  writingStreamFile (VRPN_FALSE),
  allowdup (VRPN_FALSE),
  useRecvTime (VRPN_FALSE),

  stmRxTmin (500),
  stmRxTsep (500),

  MaxSafeMove (100.0f),
  ModSubWinSz (15),

  xMin (0.0f),
  xMax (5000.0f),
  yMin (0.0f),
  yMax (5000.0f),

  mutexPort (-1),

  incr_save(1)
{
  strcpy (deviceName, "null");
}




/// State not part of a particular mode or distinguished subset.
AFMState::AFMState (const AFMInitializationState & i) :
    StdDelay (200),
    StPtDelay (200),

    stmRxTmin (i.stmRxTmin),
    stmRxTsep (i.stmRxTsep),

    MaxSafeMove (i.MaxSafeMove),

    stm_z_scale ("z_scale", i.stm_z_scale),

    do_raster    (VRPN_TRUE),
    do_y_fastest (VRPN_FALSE),
    raster_scan_backwards (VRPN_FALSE),

    modify (i.modify),
    image (i.image),
    scanline (i.scanline),
    data (),

    // MOVED to nmb_Dataset
    //done (VRPN_FALSE),
    acquisitionMode(IMAGE),

    scanning ("spm_scanning", 0),
    autoscan ("autoscan", 0),
    withdraw_tip("withdraw_tip", 0),
    slowScanEnabled ("slowScan", 1),
    cannedLineVisible (VRPN_FALSE),
    cannedLineToggle (0),

    lost_changes (0),
    new_epoch (VRPN_FALSE),

    regionFlag (VRPN_FALSE),

    xMin (i.xMin),
    xMax (i.xMax),
    yMin (i.yMin),
    yMax (i.yMax),
    zMin (0.0f),
    zMax (10000.0f),

    doDriftComp    (i.doDriftComp),
    doRelaxComp    ("doRelaxComp", i.doRelaxComp),
    doRelaxUp      (i.doRelaxUp),
    doSplat        (i.doSplat),
    snapPlaneFit   (VRPN_TRUE),

    read_mode("spm_read_mode", READ_FILE),

    commands_suspended("spm_commands_suspended", 0),

    writingStreamFile    (i.writingStreamFile),
    writingNetworkStream (VRPN_FALSE),
    readingStreamFile    (i.readingStreamFile),
    saveStream           (VRPN_TRUE),
    allowdup             (i.allowdup),
    useRecvTime          (i.useRecvTime),

    rasterX (0),
    rasterY (0),

    first_PID_message_pending(VRPN_TRUE),  //message starts out pending

    //dlistchange (VRPN_FALSE),
    select_center_x (0.0f),
    select_center_y (0.0f),
    select_region_rad (0.0f),
    ModSubWinSz (i.ModSubWinSz),
    lastPulseOK (VRPN_TRUE),
    current_epoch (0),
    subscan_count (0),

    lastZ (0.0f),
    // this will effectively set the default to disable the jump back 
    // feature which doesn't quite work as advertised yet
    numLinesToJumpBack("num_lines_to_jump_back", 1000)

{
    // We should be able to tell where we are getting data
    // Default is READ_FILE, so check flags and such for STREAM or DEVICE
  strcpy(deviceName, i.deviceName);
  if (writingStreamFile) {
    strcpy(outputStreamName, i.outputStreamName);
  }
  if (readingStreamFile) {
    strcpy(inputStreamName, i.inputStreamName);
    read_mode = READ_STREAM;
  } else if ( strcmp(deviceName, "null") != 0) {
    read_mode = READ_DEVICE;
  }
}

AFMState::~AFMState (void) {

}


void AFMState::SetDefaultScanlineForRegion(nmb_Dataset * dataset){

    BCGrid *g = dataset->inputGrid;

    if (do_y_fastest) {
        scanline.resolution = g->numY();
        scanline.x_end = 50.0;	// 50 percent of x range
        scanline.y_end = 100.0;	// 100 percent of y range
        scanline.angle = 90.0;
        scanline.width = 100.0;	// 100 percent
    } else {
	scanline.resolution = g->numX();
        scanline.y_end = 50.0;
        scanline.x_end = 100.0;
        scanline.angle = 0.0;
        scanline.width = 100.0;
    }
    BCPlane *p = g->getPlaneByName(dataset->heightPlaneName->string());
    if (!p) {
        fprintf(stderr, "AFMState::SetDefaultScanlineForRegion - "
           "Error: tried to get height plane but couldn't\n");
        // this value is just a default so its not really critical
	// user should check it anyway before running without feedback
        // enabled
        scanline.z_end = 100.0;
        return;
    }
    // Add a safety margin of 100.0 nM
	float x, y, z;
	scanline.getFinishPoint(p, &x, &y, &z);
    scanline.z_end = p->interpolatedValueAt(x, y) + 100.0;

}

int AFMDataset::Initialize (nmb_Dataset * dataset) {
  BCPlane * p;

  inputPoint = new Point_results;
  fc_inputPoint = new Point_results;

  inputPointNames.addEntry("none");
  fc_inputPointNames.addEntry("none");


  // Add every plane in the input grid list into the possible ones
  // to map to outputs
  p = dataset->inputGrid->head();
  while (p) {
    inputPointNames.addEntry(p->name()->Characters());
    p = p->next();
  }

  // Set up the checklists for the scan and touch data sets.
  // This should be done before the microscope is initialized (so that
  // the proper values are requested from the microscope), but
  // after the grids are created (so that new planes can be added
  // if needed).

  scan_channels = new Scan_channel_selector (dataset->inputGrid,
                                             dataset->inputPlaneNames,
                                             dataset);
  point_channels = new Point_channel_selector (inputPoint,
                                               &inputPointNames,
						dataset,
                                               &incomingPointList);
  forcecurve_channels = new ForceCurve_channel_selector (fc_inputPoint,
						&fc_inputPointNames, dataset);

  return 0;
}

