//Warning: this file automatically generated using the command line
// ../../../../../../vrpn/util/gen_rpc/gen_vrpn_rpc.pl nmm_MicroscopeRemote.hdef
//DO NOT EDIT! Edit the source file instead.
/*===3rdtech===
  Copyright (c) 2000 by 3rdTech, Inc.
  All Rights Reserved.
  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#ifndef NMM_MICROSCOPE_REMOTE_H
#define NMM_MICROSCOPE_REMOTE_H
#include "nmm_AFM_Report.h"
#include "nmm_SPM_Report.h"
#include "nmm_AFM_Control.h"
#include "nmm_SPM_Control.h"
#include <vrpn_Shared.h>  // for timeval/timezone
#ifndef VRPN_CONNECTION_H
#include <vrpn_Connection.h>  // for vrpn_HANDLERPARAM
#endif
#include <vrpn_RedundantTransmission.h>
#include <nmb_SharedDevice.h>
#include <Scanline.h>
#include <Topo.h>
class Tclvar_checklist;  // from <Tcl_Linkvar.h>
class nmb_Dataset;  // from <nmb_Dataset.h>
class nmb_Decoration;  // from <nmb_Decoration.h>
class Point_value;  // from <Point.h>
class BCPlane;  // from <BCPlane.h>
#include "AFMState.h"
#include "nmm_Types.h"
#include "nmm_RelaxComp.h"   // for nmm_RelaxComp
#include "nmm_QueueMonitor.h"
#include "nmm_TimestampList.h"
#include "nmm_GuardedscanClient.h"
struct nmm_Sample;  // from nmm_Sample.h
//Message definitions from file nmm_Monitor.vrpndef
//Message definitions from file nmm_SPM_Report.vrpndef
//Message definitions from file nmm_AFM_Report.vrpndef
// nmm_Microscope
//
// Tom Hudson, September 1997
// Code mostly from microscape.c and animate.c
// Encapsulates the microscope and all functions a client might want to
// perform on it.  Controls AFMState class.
// Uses references to its private members (io and modfile) rather than
// containing them so that we (or users of our library) don't have to
// include the relevant header files.
// Flaws:
//   Currently contains InitializeDataset() call that'll be moved elsewhere
// someday.
#define RELAX_MSG "relaxation done"
class nmm_Microscope_Remote : public nmb_SharedDevice_Remote,
                              public nmm_AFM_Report, public nmm_SPM_Report,
                              public nmm_AFM_Control, public nmm_SPM_Control  {
  friend class nmm_RelaxComp;
  public:
    nmm_Microscope_Remote (const AFMInitializationState &,
                           vrpn_Connection *);
      //   Constructor.
    ~nmm_Microscope_Remote (void);
      // Destructor.
    virtual int mainloop (void);
    AFMState state;
    TopoFile d_topoFile;
    nmm_RelaxComp d_relax_comp;
    // ACCESSORS
    nmm_Sample * SampleMode (void) const
      { return d_sampleAlgorithm; }
    // MANIPULATORS
    long InitializeDataset (nmb_Dataset *);
    long InitializeDecoration (nmb_Decoration *);
    long InitializeTcl (const char * tcl_script_dir);
      // TODO:  move all of these to AFMInitializationState?
      //   AT LEAST think about passing them in to the constructor.
    nmb_Dataset * Data (void)
      { return d_dataset; }
    nmb_Decoration * Decor (void)
      { return d_decoration; }
    char * encode_GetNewPointDatasets (vrpn_int32 * len,
                                       const Tclvar_list_of_strings *,
                                       Tclvar_int*[], Tclvar_int*[]);
   char * encode_GetNewScanDatasets (vrpn_int32 * len,
                                     const Tclvar_list_of_strings *, 
                                     Tclvar_int*[]);
   long Initialize (void) ;
      ///< Set up to read from a stream file or a live scope, 
      ///< depending on d_dataset->inputgrid->readMode()
    long NewEpoch (void);
      ///< Forces refresh of grid (?)
    void SetSampleMode (nmm_Sample *);
      ///< Passes in a pointer to the nmm_Sample subclass to be used by
      ///< TakeSampleSet().
    // SENDS
    long ImageMode (void);
      ///<   Enter imaging mode.  Sets parameters to values given in
      ///< state.image (P/I/D gain, setpoint, amplitude).
    long ModifyMode (void);
      ///<   Enter modify mode.  Sets parameters to values given in
      ///< state.modify (P/I/D gain, setpoint, amplitude, top & bottom
      ///< delay, z_pull, punch distance, speed, watchdog)
    long GetNewPointDatasets (const Tclvar_list_of_strings *, 
                                       Tclvar_int*[],Tclvar_int*[]);
    long GetNewScanDatasets (const Tclvar_list_of_strings *, 
                                       Tclvar_int*[]);
      ///<   Sets the list of datasets to be scanned by the microscope.
      ///< Microscope will respond with the list that it is actually scanning,
      ///< which may not match.
      ///<   every active (marked) dataset in the checklist will be requested
    long ResumeScan (Point_value * = NULL,
                    BCPlane * = NULL);
      ///<   Return to image mode *and start scanning*
      ///<   Non-NULL parameters tell the microscope to check a small region
      ///< of the surface before rescanning the entire surface, and only make
      ///< sense with the old AFM.
    long ResumeFullScan (void);
      ///<   Causes the microscope to scan over the entire grid.
      ///< SetScanWindow() may be called to limit scanning to a smaller
      ///< portion of the grid;  this function undoes that operation.
    long ResumeWindowScan (void);
    long PauseScan(void);
      ///< Pauses the scan.
    long WithdrawTip(void);
      ///< Withdraws the tip from the surface.
    long SetScanStyle (void);
      ///<   Sets the microscope to scan in X or Y first, and in a raster or
      ///< in boustrophedonic order.  Controlled by (state.do_raster,
      ///< state.do_y_fastest)
    long SetRegionNM (const float minx, const float miny,
                     const float maxx, const float maxy);
    long SetSlowScan (const long value);
    long SetModForce();
    // Tip Motion control (using settings on modify-mode variables).
    //
    // _point, if set, must be an alias for the current position
    // of the tip.  When _awaitResult is VRPN_TRUE, the function blocks
    // (safely!) until we receive packets back from the microscope that
    // indicate the tip has moved.  For TakeFeelStep, we wait until the
    // tip has moved any distance at all, while for TakeModStep, DrawLine,
    // or DrawArc, we wait until the tip has moved to within a small
    // tolerance of its destination.
    long DrawLine (const double startx, const double starty,
                  const double endx, const double endy,
                  Point_value * point = NULL,
                  const vrpn_bool awaitResult = VRPN_FALSE);
    long DrawArc (const double x, const double y,
                 const double startAngle, const double endAngle,
                 Point_value * point = NULL,
                 const vrpn_bool awaitResult = VRPN_FALSE);
    long TakeFeelStep (const float x, const float y,
                      Point_value * point = NULL,
                      const vrpn_bool awaitResult = VRPN_FALSE);
    long TakeModStep (const float x, const float y,
                     Point_value * point = NULL,
                     const vrpn_bool awaitResult = VRPN_FALSE);
    int TakeDirectZStep (const float x, const float y, const float z,
                     Point_value * point = NULL,
                     const vrpn_bool awaitResult = VRPN_FALSE);
    long ScanTo (const float x, const float y);
      ///< Sends the microscope to (x, y).
    long ScanTo (const float x, const float y, const float z);
      ///< Sends the microscope to (x, y, z).
    int TakeSampleSet (float x, float y);
      ///< Tells the microscope to take one or more samples in the
      ///< pattern specified by SetSampleAlgorithm().  The results are
      ///< made available to the user by a yet-to-be-determined mechanism,
      ///< including exposure as state.data.receivedPointList.
      ///< Returns negative values on failure.
    // Ohmmeter control
    /*
    long SetOhmmeterSampleRate (const long rate);
      ///<   Sets the sample rate on the #0 ohmmeter.
    long EnableAmp (long which, float offset,
                   float uncalOffset, long gain);
      ///<   Enables one of the amplifier channels with specified offsets
      ///< and gain.
    long DisableAmp (long which);
      ///< Disables one of the amplifier channels.
    long EnableVoltsource (long which, float voltage);
      ///< Enables one of the voltage sources at a specified voltage.
    long DisableVoltsource (long which);
      ///< Enables one of the voltage sources.
    */
    long RecordResistance(long channel, timeval t, float r,
                          float voltage, float range, float filter); 
       ///< XXX - this function was used for the French Ohmmeter
       ///< because we wanted to put data into the microscope
       ///< stream file but this should be replaced now that
       ///< we are using vrpn for logging - e.g. either
       ///< set both connections to log to the same file or
       ///< create two separate files
    int getTimeSinceConnected(void);
    // Scanline mode - parameters sent by these functions reside in
    // state.scanline and are typically set in the user interface
    long EnterScanlineMode(void);
    long ExitScanlineMode(void);
    long AcquireScanline(void);
    long SetScanlineModeParameters(void);
    long SetFeedbackForScanline(void);
    long JumpToScanLine(long line);
      ///< set which line to start scanning in image mode
    long SetGridSize (const long _x, const long _y);
      ///< Set the size of the grid:  # data points to collect in each dimension.
    long SetScanAngle (const float _angle);
      ///< Angle of the scan, in degrees 
    // ODDS AND ENDS
    void ResetClock (void);
    void SetStreamToTime (timeval time);
    /// Query and set the read mode, READ_DEVICE, READ_FILE or READ_STREAM
    int ReadMode();
    void ReadMode(int);
    long EnableUpdatableQueue (vrpn_bool);
      ///< If TRUE, only the most recent ScanTo/ScanToZ/ZagTo request
      ///< is obeyed at the microscope;  older requests are thrown away.
      ///< If FALSE, all ScanTo/ScanToZ/ZagTo requests are executed.
      ///< Should be turned off for line mode!
//      Blunt_result * getBluntResult (void)
//        { return &d_bluntResult; }
    // Register callbacks from the user interface so we can decouple some
    // UI functions that currently go through here.
    long registerPointDataHandler
               (int (* handler) (void *, const Point_results *),
                void * userdata);
    long unregisterPointDataHandler
               (int (* handler) (void *, const Point_results *),
                void * userdata);
    long registerModifyModeHandler (int (* handler) (void *),
                                   void * userdata);
    long unregisterModifyModeHandler (int (* handler) (void *),
                                     void * userdata);
    long registerImageModeHandler (int (* handler) (void *),
                                  void * userdata);
    long unregisterImageModeHandler (int (* handler) (void *),
                                    void * userdata);
    long registerScanlineModeHandler (int (* handler) (void *),
      void *userdata);
    long unregisterScanlineModeHandler (int (* handler) (void *),
                                  void *userdata);
    long registerScanlineDataHandler (int (* handler) (void *,
     const Scanline_results *),
      void *userdata);
    long unregisterScanlineDataHandler (int (* handler) (void *,
     const Scanline_results *),
                                  void *userdata);
    long registerFeeltoHandler (int (* handler) (void *),
      void *userdata);
    long unregisterFeeltoHandler (int (* handler) (void *),
                                  void *userdata);
    vrpn_int32 pointResultType (void) const;
      ///< Returns the vrpn type of a point result message;
      ///< used with nmb_DeviceSequencer/nmm_Sample and with
      ///< nmm_QueueMonitor.
    ///< Polls the vrpn connection until the point result
    ///< returns an (X,Y) or (X,Y,Z) tuple close to the
    ///< supplied arguments.
    void WaitForResult(float a_fX, float a_fY, Point_value* a_pPoint);
    void WaitForResult(float a_fX, float a_fY, float a_fZ, Point_value* a_pPoint);
  protected:
    nmb_Dataset * d_dataset;
    nmb_Decoration * d_decoration;
    char * d_tcl_script_dir;
    // to keep track of region to be sent using SetScanWindow():
    // this basically just accumulates a bounding box for all points
    // received during the latest modification
    vrpn_bool d_mod_window_initialized; // is the mod_window initialized
                                        // for the current modification
                                        // when we first switch to img mode
                                        // this refers to the previous
                                        // modification but after that it
                                        // refers to the next modification
    vrpn_int32 d_mod_window_min_x;
    vrpn_int32 d_mod_window_min_y;
    vrpn_int32 d_mod_window_max_x;
    vrpn_int32 d_mod_window_max_y;
    vrpn_int32 d_mod_window_pad; // some padding on the boundary
    void accumulatePointResults (vrpn_bool on);
      ///< If true, in addition to exposing point results on
      ///< state.data.inputPoint we put them on state.data.pointList.
  private: // OBSOLETE
    // STM control and query
    long SetBias (const float bias);
      //   Sets the voltage difference to maintain between the tip and
      // the sample during normal scanning.
    long SetPulsePeak (const float height);
      //   Sets the voltage difference to impose between the tip and the
      // sample while pulsing.
    long SetPulseDuration (const float width);
      // Sets the length of time to hold at high voltage during a pulse.
    long QueryPulseParams (void);
      // Queries the current state of the pulse generator.
  private:
    // Utility fcn to rotate scan coords.
    long rotateScanCoords (double _x, double _y,
      double _scanAngle, 
      double * out_x, double * out_y);
    // Commands only sent by member functions
    long QueryScanRange (void);
    long SetScanWindow (long _minx, long _miny,
                       long _maxx, long _maxy);
      //   Choose a subset of the entire grid to scan.  Specifies the
      // corners of a rectangular region inside the originally-specified
      // grid to limit the scan to.
    long SetRateNM (double rate);
    long MarkModifyMode (void);
    long MarkImageMode (void);
    long EnterOscillatingMode(float p, float i, float d, float set, float amp,
         vrpn_float32 frequency, vrpn_int32 input_gain,
         vrpn_bool ampl_or_phase, vrpn_int32 drive_attenuation,
         vrpn_float32 phase);
    long EnterContactMode (float p, float i, float d, float set);
 long EnterGuardedScanMode (float a_fP, float a_fI, float a_fD, float a_fSetpoint, 
        float a_fNormalX, float a_fNormalY, float a_fNormalZ,
        float a_fPlaneD, float a_fGuardDepth);
    long EnterDirectZControl (float _max_z_step, float _max_xy_step, 
         float _min_setpoint, float _max_setpoint, 
         float _max_lateral_force);
    long EnterSewingStyle (float set, float bot, float top, float zpull,
                          float punch, float speed, float watchdog);
    long EnterForceCurveStyle (float setpnt, float startdelay, float zstart, 
 float zend, float zpull, float forcelimit, float movedist, long numpnts,
 long numhalfcycles, float samplespd, float pullspd, float startspd,
 float fdbackspd, long avgnum, float sampledel, float pulldel, 
 float fdbackdel);
    long ZagTo (float x, float y, float yaw,
               float sweepWidth, float regionDiag);
    long GotConnection(void);
    long SetRelax (long min, long sep);
    int SetModForce (float newforce, float min, float max);
    vrpn_int32 d_PointResultData_type;
  char * encode_PointResultData (
      int * len,
      vrpn_float32 x,
      vrpn_float32 y,
      vrpn_int32 sec,
      vrpn_int32 usec,
      vrpn_int32 reports,
      vrpn_float32 *data
  );
  int decode_PointResultData (
      const char ** buffer,
      vrpn_float32 (*x),
      vrpn_float32 (*y),
      vrpn_int32 (*sec),
      vrpn_int32 (*usec),
      vrpn_int32 (*reports),
      vrpn_float32 *(*data)
  );
  static int handle_PointResultData (void * userdata, vrpn_HANDLERPARAM p);
  void RcvPointResultData (
      vrpn_float32 x,
      vrpn_float32 y,
      vrpn_int32 sec,
      vrpn_int32 usec,
      vrpn_int32 reports,
      vrpn_float32 *data
  );
    // Receive callbacks 
    int RcvGotConnection2 (void);
  void RcvHelloMessage (
      char nm[4],
      char scopeName[SPM_NAME_LENGTH],
      vrpn_int32 majorVersion,
      vrpn_int32 minorVersion
  );
  void RcvScanrateParameter (
      vrpn_float32 rate
  );
  void RcvPidParameters (
      vrpn_float32 P,
      vrpn_float32 I,
      vrpn_float32 D
  );
  void RcvReportGridSize (
      vrpn_int32 x,
      vrpn_int32 y
  );
  void RcvSetRegionClipped (
      vrpn_float32 xmin,
      vrpn_float32 ymin,
      vrpn_float32 xmax,
      vrpn_float32 ymax
  );
  void RcvScanRange (
      vrpn_float32 xmin,
      vrpn_float32 ymin,
      vrpn_float32 zmin,
      vrpn_float32 xmax,
      vrpn_float32 ymax,
      vrpn_float32 zmax
  );
  void RcvReportScanAngle (
      vrpn_float32 angle
  );
  void RcvReportSlowScan (
      vrpn_int32 enabled
  );
  void RcvScanning (
      vrpn_int32 on_off
  );
  void RcvScanDataset_header (
      vrpn_int32 count
  );
  void RcvScanDataset_body (
      char name[64],
      char units[64],
      vrpn_float32 offset,
      vrpn_float32 scale
  );
  void RcvWindowLineData (
      vrpn_int32 x,
      vrpn_int32 y,
      vrpn_int32 dx,
      vrpn_int32 dy,
      vrpn_int32 reports,
      vrpn_int32 fields,
      vrpn_int32 sec,
      vrpn_int32 usec,
      vrpn_float32 **data
  );
  void RcvPointDataset_header (
      vrpn_int32 count
  );
  void RcvPointDataset_body (
      char name[64],
      char units[64],
      vrpn_int32 numSamples,
      vrpn_float32 offset,
      vrpn_float32 scale
  );
  void RcvForceCurveData_header (
      vrpn_float32 x,
      vrpn_float32 y,
      vrpn_int32 numSamples,
      vrpn_int32 numHalfcycles,
      vrpn_int32 sec,
      vrpn_int32 usec
  );
  void RcvForceCurveData_body (
      vrpn_float32 z,
      vrpn_float32 *d
      , vrpn_int32 numHalfcycles
  );
  void RcvRelaxSet (
      vrpn_int32 minTime,
      vrpn_int32 sepTime
  );
  void RcvStartingToRelax (
      vrpn_int32 sec,
      vrpn_int32 usec
  );
  void RcvTopoFileHeader (
      vrpn_int32 length,
      char *header
  );
  void RcvSuspendCommands (
  );
  void RcvResumeCommands (
  );
  void RcvMaxSetpointExceeded (
  );
  void RcvInScanlineMode (
      vrpn_int32 enabled
  );
  void RcvScanlineData (
      vrpn_float32 x,
      vrpn_float32 y,
      vrpn_float32 z,
      vrpn_float32 angle,
      vrpn_float32 slope,
      vrpn_float32 width,
      vrpn_int32 resolution,
      vrpn_int32 feedback_enabled,
      vrpn_int32 checking_forcelimit,
      vrpn_float32 max_force_setting,
      vrpn_float32 max_z_step,
      vrpn_float32 max_xy_step,
      vrpn_int32 sec,
      vrpn_int32 usec,
      vrpn_int32 num_channels,
      vrpn_float32 **data
  );
  void RcvBeginFeelTo (
  );
  void RcvEndFeelTo (
      vrpn_float32 x,
      vrpn_float32 y,
      vrpn_int32 numx,
      vrpn_int32 numy,
      vrpn_float32 dx,
      vrpn_float32 dy,
      vrpn_float32 orientation
  );
  static int handle_HelloMessage (void * userdata, vrpn_HANDLERPARAM p);
  static int handle_ScanrateParameter (void * userdata, vrpn_HANDLERPARAM p);
  static int handle_PidParameters (void * userdata, vrpn_HANDLERPARAM p);
  static int handle_ReportGridSize (void * userdata, vrpn_HANDLERPARAM p);
  static int handle_SetRegionClipped (void * userdata, vrpn_HANDLERPARAM p);
  static int handle_ScanRange (void * userdata, vrpn_HANDLERPARAM p);
  static int handle_ReportScanAngle (void * userdata, vrpn_HANDLERPARAM p);
  static int handle_ReportSlowScan (void * userdata, vrpn_HANDLERPARAM p);
  static int handle_Scanning (void * userdata, vrpn_HANDLERPARAM p);
  static int handle_ScanDataset (void * userdata, vrpn_HANDLERPARAM p);
  static int handle_WindowLineData (void * userdata, vrpn_HANDLERPARAM p);
  static int handle_PointDataset (void * userdata, vrpn_HANDLERPARAM p);
  static int handle_ForceCurveData (void * userdata, vrpn_HANDLERPARAM p);
  static int handle_RelaxSet (void * userdata, vrpn_HANDLERPARAM p);
  static int handle_StartingToRelax (void * userdata, vrpn_HANDLERPARAM p);
  static int handle_TopoFileHeader (void * userdata, vrpn_HANDLERPARAM p);
  static int handle_SuspendCommands (void * userdata, vrpn_HANDLERPARAM p);
  static int handle_ResumeCommands (void * userdata, vrpn_HANDLERPARAM p);
  static int handle_MaxSetpointExceeded (void * userdata, vrpn_HANDLERPARAM p);
  static int handle_InScanlineMode (void * userdata, vrpn_HANDLERPARAM p);
  static int handle_ScanlineData (void * userdata, vrpn_HANDLERPARAM p);
  static int handle_BeginFeelTo (void * userdata, vrpn_HANDLERPARAM p);
  static int handle_EndFeelTo (void * userdata, vrpn_HANDLERPARAM p);
  void RcvInContactMode (
      vrpn_float32 P,
      vrpn_float32 I,
      vrpn_float32 D,
      vrpn_float32 setpoint
  );
  void RcvInGuardedScanMode (
      vrpn_float32 P,
      vrpn_float32 I,
      vrpn_float32 D,
      vrpn_float32 setpoint,
      vrpn_float32 fNormalX,
      vrpn_float32 fNormalY,
      vrpn_float32 fNormalZ,
      vrpn_float32 fPlaneD,
      vrpn_float32 fGuardDepth
  );
  void RcvInTappingMode (
      vrpn_float32 P,
      vrpn_float32 I,
      vrpn_float32 D,
      vrpn_float32 setpoint,
      vrpn_float32 amplitude
  );
  void RcvInOscillatingMode (
      vrpn_float32 P,
      vrpn_float32 I,
      vrpn_float32 D,
      vrpn_float32 setpoint,
      vrpn_float32 amplitude,
      vrpn_float32 frequency,
      vrpn_int32 input_gain,
      vrpn_int32 drive_attenuation,
      vrpn_float32 phase,
      vrpn_bool ampl_or_phase
  );
  void RcvInSewingStyle (
      vrpn_float32 setpoint,
      vrpn_float32 bottomDelay,
      vrpn_float32 topDelay,
      vrpn_float32 pullBackDistance,
      vrpn_float32 moveDistance,
      vrpn_float32 moveRate,
      vrpn_float32 maxDistanceToApproach
  );
  void RcvInSpectroscopyMode (
      vrpn_float32 setpoint,
      vrpn_float32 startDelay,
      vrpn_float32 zStart,
      vrpn_float32 zEnd,
      vrpn_float32 zPullback,
      vrpn_float32 forceLimit,
      vrpn_float32 distBetweenFC,
      vrpn_int32 numPoints,
      vrpn_int32 numHalfcycles,
      vrpn_float32 sampleSpeed,
      vrpn_float32 pullbackSpeed,
      vrpn_float32 startSpeed,
      vrpn_float32 feedbackSpeed,
      vrpn_int32 averageNum,
      vrpn_float32 sampleDelay,
      vrpn_float32 pullbackDelay,
      vrpn_float32 feedbackDelay
  );
  void RcvInDirectZControl (
      vrpn_float32 max_z_step,
      vrpn_float32 max_xy_step,
      vrpn_float32 min_setpoint,
      vrpn_float32 max_setpoint,
      vrpn_float32 max_lateral_force,
      vrpn_float32 freespace_normal_force,
      vrpn_float32 freespace_lat_force
  );
  void RcvInModMode (
  );
  void RcvInImgMode (
  );
  static int handle_InContactMode (void * userdata, vrpn_HANDLERPARAM p);
  static int handle_InGuardedScanMode (void * userdata, vrpn_HANDLERPARAM p);
  static int handle_InTappingMode (void * userdata, vrpn_HANDLERPARAM p);
  static int handle_InOscillatingMode (void * userdata, vrpn_HANDLERPARAM p);
  static int handle_InSewingStyle (void * userdata, vrpn_HANDLERPARAM p);
  static int handle_InSpectroscopyMode (void * userdata, vrpn_HANDLERPARAM p);
  static int handle_InDirectZControl (void * userdata, vrpn_HANDLERPARAM p);
  static int handle_InModMode (void * userdata, vrpn_HANDLERPARAM p);
  static int handle_InImgMode (void * userdata, vrpn_HANDLERPARAM p);
//obs?   void RcvClientHello (const char *, const char *, long, long);
    void ClearScanChannels (void);
    void ClearPointChannels (void);
//?      void RcvFeedbackSetForScanline(long, long,
//?  			float, float, float);
    void ClearScanlineChannels(void);
    void RcvScanlineDataset(const char *, const char *, float, 
 float);
    static int handle_GotConnection2 (void *, vrpn_HANDLERPARAM);
    static int handle_DroppedConnection2 (void *, vrpn_HANDLERPARAM);
// TODO:  Document and explain these better!
    void GetRasterPosition (long, long);
      // sets state.rasterX and state.rasterY
    void DisplayModResult (float, float, float,
                           const Point_value * = NULL,
                           vrpn_bool = VRPN_FALSE);
      // sets state.rasterX and state.rasterY
    // Has resistance channel been added to point results?
    vrpn_bool d_res_channel_added;
    // replacing things in microscape
    timeval d_nowtime;
    struct timezone d_nowzone;
    timeval d_next_time;
    int readMode;	// differentiate between Live and Replay
    // callback structures
    struct pointDataHandlerEntry {
      int (* handler) (void *, const Point_results *);
      void * userdata;
      pointDataHandlerEntry * next;
    };
    struct modeHandlerEntry {
      int (* handler) (void *);
      void * userdata;
      modeHandlerEntry * next;
    };
    struct scanlineDataHandlerEntry {
      int (* handler) (void *, const Scanline_results *);
      void * userdata;
      scanlineDataHandlerEntry * next;
    };
    struct feeltoHandlerEntry {
      int (* handler) (void *);
      void * userdata;
      feeltoHandlerEntry * next;
    };
    pointDataHandlerEntry * d_pointDataHandlers;
    modeHandlerEntry * d_modifyModeHandlers;
    modeHandlerEntry * d_imageModeHandlers;
    modeHandlerEntry * d_scanlineModeHandlers;
    scanlineDataHandlerEntry *d_scanlineDataHandlers;
    feeltoHandlerEntry * d_feeltoHandlers;
    void doImageModeCallbacks (void);
    void doModifyModeCallbacks (void);
    void doPointDataCallbacks (const Point_results *);
    void doScanlineModeCallbacks (void);
    void doScanlineDataCallbacks (const Scanline_results *);
    void doFeeltoCallbacks (void);
    nmm_Sample * d_sampleAlgorithm;
    vrpn_bool d_accumulatePointResults;
    void swapPointList (void);
      ///< Replaces state.data.receivedPointList with
      ///< state.data.incomingPointList, then empties the latter.
    static int handle_barrierSynch (void * ud, const nmb_SynchMessage * msg);
    static void handle_GotMicroscopeControl (void * ud, 
                                             nmb_SharedDevice_Remote * dev);
    vrpn_bool d_incr_save;
    ///< Incrementally save the stream file, instead of saving on exit?
  public:
    // TCH network adaptations Nov 2000
    vrpn_RedundantTransmission * d_redundancy;
    vrpn_RedundantReceiver * d_redReceiver;
    nmm_QueueMonitor * d_monitor;
    nmm_TimestampList * d_tsList;
  public:
    // Guardedscan interface
    CGuardedScanClient	m_oGuardedScan;
};
#endif  // NMM_MICROSCOPE_REMOTE_H
