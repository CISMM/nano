/*===3rdtech===
  Copyright (c) 2000 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#ifndef NMM_MICROSCOPE_REMOTE_H
#define NMM_MICROSCOPE_REMOTE_H

#include "nmm_Microscope.h"

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

struct nmm_Sample;  // from nmm_Sample.h


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
                              public nmm_Microscope {

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

    // Query and set the read mode, READ_DEVICE, READ_FILE or READ_STREAM
    int ReadMode();
    void ReadMode(int);
    
    long EnableUpdatableQueue (vrpn_bool);
      ///< If TRUE, only the most recent ScanTo/ScanToZ/ZagTo request
      ///< is obeyed at the microscope;  older requests are thrown away.
      ///< If FALSE, all ScanTo/ScanToZ/ZagTo requests are executed.
      ///< Should be turned off for line mode!

    Blunt_result * getBluntResult (void)
      { return &d_bluntResult; }


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
    long SetMaxMove (float distance);
    long SetStdDelay (long delay);
    long SetStPtDelay (long delay);
    long SetRelax (long min, long sep);
    int SetModForce (float newforce, float min, float max);




    // Receive callbacks 
    int RcvGotConnection2 (void);
    void RcvInTappingMode (float, float, float, float, float);
    void RcvInOscillatingMode (float _p, 
                               float _i,
                               float _d, float _setpoint,
                               float _amp,
                               float _frequency,
                               vrpn_int32 _input_gain, 
                               vrpn_bool _ampl_or_phase,
                               vrpn_int32 _drive_attenuation, 
                               float _phase);
    void RcvInContactMode (float, float, float, float);
    void RcvInDirectZControl (float, float, float,
                           float, float, 
                           float, float);
    void RcvInSewingStyle (float, float, float, float,
                          float, float, float);
    void RcvInSpectroscopyMode (float,float,float,
        float,float,float,float,int,int,
        float,float,float,float,
        int, float,float,float);

    void RcvForceParameters (long, float);
    void RcvBaseModParameters (float, float);
    void RcvForceSettings (float, float, float);
    void RcvVoltsourceEnabled (long, float);
    void RcvVoltsourceDisabled (long);
    void RcvAmpEnabled (long, float, float, long);
    void RcvAmpDisabled (long);
    void RcvSuspendCommands();
    void RcvResumeCommands();
    void RcvStartingToRelax (long, long);
    void RcvInModModeT (long, long);
    void RcvInModMode (void);
    void RcvInImgModeT (long, long);
    void RcvInImgMode (void);
    void RcvModForceSet (float);
    void RcvImgForceSet (float);
    void RcvModSet (long, float, float, float);
    void RcvImgSet (long, float, float, float);
    void RcvRelaxSet (long, long);
    void RcvForceSet (float);
    void RcvForceSetFailure (float);
    void RcvPulseParameters (long, float, float, float);
    long RcvWindowLineData (long, long, long, long,
                            long, const float *, vrpn_bool);
    long RcvWindowLineData (long, long, long, long,
			    long);
    long RcvWindowLineData (void);
    void RcvForceCurveData (float, float, long, long,
                            long, long, const float *, const float **);
    void RcvWindowScanNM (long, long, long, long,
                          float, float);
    void RcvWindowBackscanNM (long, long, long, long,
                              float, float);
    void RcvPointResultNM (float, float, long, long,
                           float, float);
    void RcvResultData (long, float, float,
                        long, long, long, const float *);
    void RcvResultNM (float, float, long, long,
                      float, float, float, float);
    void RcvPulseCompletedNM (float, float);
    void RcvPulseFailureNM (float, float);
    void RcvScanning (vrpn_int32);
    void RcvScanRange (float, float, float, float,
                       float, float);
    void RcvReportScanAngle (float);
    void RcvSetRegionC (long,
                        float, float, float, float);
    void RcvResistanceFailure (long);
    void RcvResistance (long, long, long, float);
    void RcvResistance2(long, long, long, float,
			float, float, float);
    void RcvReportSlowScan (long);
    void RcvScanParameters (const char **);
    void RcvHelloMessage (const char *, const char *, long, long);
    void RcvClientHello (const char *, const char *, long, long);
    void RcvClearScanChannels (void);
    void RcvScanDataset (const char *, const char *, float, float);
    void RcvClearPointChannels (void);
    void RcvPointDataset (const char *, const char *, long,
                         float, float);
    void RcvPidParameters (float, float, float);
    void RcvScanrateParameter (float);
    int RcvReportGridSize (long, long);

    void RcvMaxSetpointExceeded(void);
    void RcvServerPacketTimestamp (long, long);
    void RcvTopoFileHeader (long, const char *);

    void RcvInScanlineMode(long);
    void RcvFeedbackSetForScanline(long, long,
			float, float, float);
    void RcvClearScanlineChannels(void);
    void RcvScanlineDataset(const char *, const char *, float, 
	float);
    void RcvScanlineDataHeader(float, float, float,
	float, float, float, long, long, long, 
	float, float, float, long, long,
	long);
    void RcvScanlineData(long, long, const float *);

    // messages for Michele Clark's experiments
    // Code is in MicroscopeRcv.C
    void RcvRecvTimestamp (timeval);
    void RcvFakeSendTimestamp (timeval);
    void RcvUdpSeqNum (long);




    static int handle_GotConnection2 (void *, vrpn_HANDLERPARAM);
    static int handle_DroppedConnection2 (void *, vrpn_HANDLERPARAM);
    static int handle_InTappingMode (void *, vrpn_HANDLERPARAM);
    static int handle_InOscillatingMode (void *, vrpn_HANDLERPARAM);
    static int handle_InContactMode (void *, vrpn_HANDLERPARAM);
    static int handle_InDirectZControl (void *, vrpn_HANDLERPARAM);
    static int handle_InSewingStyle (void *, vrpn_HANDLERPARAM);
    static int handle_InSpectroscopyMode (void *, vrpn_HANDLERPARAM);
    static int handle_ForceParameters (void *, vrpn_HANDLERPARAM);
    static int handle_BaseModParameters (void *, vrpn_HANDLERPARAM);
    static int handle_ForceSettings (void *, vrpn_HANDLERPARAM);
    static int handle_VoltsourceEnabled (void *, vrpn_HANDLERPARAM);
    static int handle_VoltsourceDisabled (void *, vrpn_HANDLERPARAM);
    static int handle_AmpEnabled (void *, vrpn_HANDLERPARAM);
    static int handle_AmpDisabled (void *, vrpn_HANDLERPARAM);
    static int handle_SuspendCommands (void *, vrpn_HANDLERPARAM);
    static int handle_ResumeCommands (void *, vrpn_HANDLERPARAM);
    static int handle_StartingToRelax (void *, vrpn_HANDLERPARAM);
    static int handle_InModModeT (void *, vrpn_HANDLERPARAM);
    static int handle_InModMode (void *, vrpn_HANDLERPARAM);
    static int handle_InImgModeT (void *, vrpn_HANDLERPARAM);
    static int handle_InImgMode (void *, vrpn_HANDLERPARAM);
    static int handle_ModForceSet (void *, vrpn_HANDLERPARAM);
    static int handle_ImgForceSet (void *, vrpn_HANDLERPARAM);
    static int handle_ModSet (void *, vrpn_HANDLERPARAM);
    static int handle_ImgSet (void *, vrpn_HANDLERPARAM);
    static int handle_RelaxSet (void *, vrpn_HANDLERPARAM);
    static int handle_ForceSet (void *, vrpn_HANDLERPARAM);
    static int handle_ForceSetFailure (void *, vrpn_HANDLERPARAM);
    static int handle_PulseParameters (void *, vrpn_HANDLERPARAM);
    static int handle_WindowLineData (void *, vrpn_HANDLERPARAM);
    static int handle_WindowScanNM (void *, vrpn_HANDLERPARAM);
    static int handle_WindowBackscanNM (void *, vrpn_HANDLERPARAM);
    static int handle_PointResultNM (void *, vrpn_HANDLERPARAM);
    static int handle_PointResultData (void *, vrpn_HANDLERPARAM);
    static int handle_BottomPunchResultData (void *, vrpn_HANDLERPARAM);
    static int handle_TopPunchResultData (void *, vrpn_HANDLERPARAM);
    static int handle_ResultNM (void *, vrpn_HANDLERPARAM);
    static int handle_ForceCurveData (void *, vrpn_HANDLERPARAM);
    static int handle_PulseCompletedNM (void *, vrpn_HANDLERPARAM);
    static int handle_PulseFailureNM (void *, vrpn_HANDLERPARAM);
    static int handle_Scanning (void *, vrpn_HANDLERPARAM);
    static int handle_ScanRange (void *, vrpn_HANDLERPARAM);
    static int handle_ReportScanAngle (void *, vrpn_HANDLERPARAM);
    static int handle_SetRegionCompleted (void *, vrpn_HANDLERPARAM);
    static int handle_SetRegionClipped (void *, vrpn_HANDLERPARAM);
    static int handle_ResistanceFailure (void *, vrpn_HANDLERPARAM);
    static int handle_Resistance (void *, vrpn_HANDLERPARAM);
    static int handle_Resistance2(void *, vrpn_HANDLERPARAM);
    static int handle_ReportSlowScan (void *, vrpn_HANDLERPARAM);
    static int handle_ScanParameters (void *, vrpn_HANDLERPARAM);
    static int handle_HelloMessage (void *, vrpn_HANDLERPARAM);
    static int handle_ClientHello (void *, vrpn_HANDLERPARAM);
    static int handle_ClearScanChannels (void *, vrpn_HANDLERPARAM);  // UNUSED
    static int handle_ScanDataset (void *, vrpn_HANDLERPARAM);
    static int handle_ClearPointChannels (void *, vrpn_HANDLERPARAM);  // UNUSED
    static int handle_PointDataset (void *, vrpn_HANDLERPARAM);
    static int handle_PidParameters (void *, vrpn_HANDLERPARAM);
    static int handle_ScanrateParameter (void *, vrpn_HANDLERPARAM);
    static int handle_ReportGridSize (void *, vrpn_HANDLERPARAM);
    static int handle_ServerPacketTimestamp (void *, vrpn_HANDLERPARAM);
    static int handle_TopoFileHeader (void *, vrpn_HANDLERPARAM);
    static int handle_BeginFeelTo (void *, vrpn_HANDLERPARAM);
    static int handle_EndFeelTo (void *, vrpn_HANDLERPARAM);


    static int handle_MaxSetpointExceeded (void *, vrpn_HANDLERPARAM);

    // scanline mode
    static int handle_InScanlineMode (void *, vrpn_HANDLERPARAM);
    static int handle_ScanlineData (void *, vrpn_HANDLERPARAM);

    // messages for Michele Clark's experiments
    static int handle_RecvTimestamp (void *, vrpn_HANDLERPARAM);
    static int handle_FakeSendTimestamp (void *, vrpn_HANDLERPARAM);
    static int handle_UdpSeqNum (void *, vrpn_HANDLERPARAM);


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

    Blunt_result d_bluntResult;


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

  public:

    // TCH network adaptations Nov 2000

    vrpn_RedundantTransmission * d_redundancy;
    vrpn_RedundantReceiver * d_redReceiver;
    nmm_QueueMonitor * d_monitor;
    nmm_TimestampList * d_tsList;
};

#endif  // NMM_MICROSCOPE_REMOTE_H
