#ifndef MICROSCOPE_H
#define MICROSCOPE_H

#include "nmm_Microscope.h"	// Tiger	this is the original header

#ifndef AFM_STATE_H             // Tiger        moved from nmm_Microscope.h
#include "AFMState.h"
#endif

//#if !defined(_WIN32) || defined(__CYGWIN__)
#if !defined(_WIN32)
#include <sys/time.h> // for RecordResistance() 
#endif
#include "nmb_Util.h"           // Added by Tiger, because moved some
                                // encode and decode functions here
#include <nmm_Types.h>

        // Added by Tiger       moved from nmm_Microscope.h
class nmb_Dataset;  // from nmb_Dataset.h
class nmb_Decoration;  // from nmb_Decoration.h

class Point_value;  // from Point.h
class BCPlane;  // from BCPlane.h
class Tclvar_checklist_with_entry;  // from Tcl_Linkvar.h

struct stm_stream;  // from stm_file.h

#ifndef VRPN_CONNECTION_H
#include <vrpn_Connection.h>  // for vrpn_HANDLERPARAM
#endif

class MicroscopeIO;
class GraphMod;

#include <Scanline.h>

#include <Topo.h>

// Microscope
//
// Tom Hudson, September 1997
// Code mostly from microscape.c and animate.c

// Encapsulates the microscope and all functions a client might want to
// perform on it.  Controls AFMState and MicroscopeIO classes.

// Uses references to its private members (io and modfile) rather than
// containing them so that we (or users of our library) don't have to
// include the relevant header files.

// Source code for this class is split between two files:
//   MicroscopeRcv.C  --  callbacks
//   Microscope.C     --  all other code

// Flaws:
//   Currently contains InitializeDataset() call that'll be moved elsewhere
// someday.



// NOTE!
//   Microscope publically inherits from nmm_Microscope.
// We want to test as much of the new (nmm_) functionality
// as we can before the VRPN switchover.

class Microscope : public nmm_Microscope {

  friend class MicroscopeIO;

  public:

    Microscope (const AFMInitializationState &, int replayRate);
                
      //   Constructor.

    ~Microscope (void);
      // Destructor.

// Added by Tiger       moved from nmm_Microscope.h
    AFMState state;

    TopoFile d_topoFile;

// Added by Tiger       moved from nmm_Microscope.h
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
// Above added by Tiger moved from nmm_Microscope.h

// Tiger        moved from nmm_Microscope.h
    char * encode_GetNewPointDatasets (long * len, const Tclvar_checklist *);
    char * encode_GetNewScanDatasets (long * len, const Tclvar_checklist *);
// Tiger


    int Initialize (const vrpn_bool _setRegion,
                    const vrpn_bool _setMode,
                    int (*) (stm_stream *),
                    const int _socketType, const char * _SPMhost,
                    const int _SPMport, const int _UDPport);
      // Set up to read from a live scope
      // Function parameter writes any necessary cookies on
      // an output stream, returning nonzero on failure
      // Last 4 parameters are for Michele Clark's experiments.

    int Initialize (int (*) (stm_stream *));
      // Set up to read from a streamfile (or static files)
      // Function parameter writes any necessary cookies on
      // an output stream, returning nonzero on failure

    int NewEpoch (void);
      // Forces refresh of grid (?)

    // SENDS

    int ImageMode (void);
      //   Enter imaging mode.  Sets parameters to values given in
      // state.image (P/I/D gain, setpoint, amplitude).

    int ModifyMode (void);
      //   Enter modify mode.  Sets parameters to values given in
      // state.modify (P/I/D gain, setpoint, amplitude, top & bottom
      // delay, z_pull, punch distance, speed, watchdog)

    int ExitScanlineMode (void);
    int EnterScanlineMode (void);
    int AcquireScanline (void);
    int SetScanlineModeParameters (void);
    int SetFeedbackForScanline (void);

/* AAS - replaced with next two functions make this more similar to nmm_Mic.
    int GetNewSetOfDataChannels (const int _mode,
                                 const Tclvar_checklist_with_entry * _dataTypes);
      //   Sets the list of datasets to be scanned by the microscope.
      // Microscope will respond with the list that it is actually scanning,
      // which may not match.
      //   mode is SPM_REQUEST_POINT_DATASETS or SPM_REQUEST_SCAN_DATASETS
      //   every active (marked) dataset in the checklist will be requested
*/
    int GetNewPointDatasets(const Tclvar_checklist_with_entry * _dataTypes);
    int GetNewScanDatasets(const Tclvar_checklist_with_entry * _dataTypes);

    int ResumeScan (Point_value * = NULL,
                    BCPlane * = NULL);
      //   Return to image mode *and start scanning*
      //   Non-NULL parameters tell the microscope to check a small region
      // of the surface before rescanning the entire surface, and only make
      // sense with the old AFM.

    int ResumeFullScan (void);

    int ResumeWindowScan (void);
      //   Causes the microscope to scan over the entire grid.
      // SetScanWindow() may be called to limit scanning to a smaller
      // portion of the grid;  this function undoes that operation.

    int SetScanStyle (void);
      //   Sets the microscope to scan in X or Y first, and in a raster or
      // in boustrophedonic order.  Controlled by (state.do_raster,
      // state.do_y_fastest)

    int SetRegionNM (const float _minx, const float _miny,
                     const float _maxx, const float _maxy);
    int SetSlowScan (const int _value);

    int SetModForce();


    // Tip Motion control (using settings on modify-mode variables).
    //
    // _point, if set, must be an alias for the current position
    // of the tip.  When _awaitResult is VRPN_TRUE, the function blocks
    // (safely!) until we receive packets back from the microscope that
    // indicate the tip has moved.  For TakeFeelStep, we wait until the
    // tip has moved any distance at all, while for TakeModStep, DrawLine,
    // or DrawArc, we wait until the tip has moved to within a small
    // tolerance of its destination.

    int DrawLine (const double _startx, const double _starty,
                  const double _endx, const double _endy,
                  Point_value * _point = NULL,
                  const vrpn_bool _awaitResult = VRPN_FALSE);

    int DrawArc (const double _x, const double _y,
                 const double _startAngle, const double _endAngle,
                 Point_value * _point = NULL,
                 const vrpn_bool _awaitResult = VRPN_FALSE);

    int TakeFeelStep (const float _x, const float _y,
                      Point_value * _point = NULL,
                      const vrpn_bool _awaitResult = VRPN_FALSE);

    int TakeModStep (const float _x, const float _y,
                     Point_value * _point = NULL,
                     const vrpn_bool _awaitResult = VRPN_FALSE);

    int TakeDirectZStep (const float _x, const float _y, const float _z,
                     Point_value * _point = NULL,
                     const vrpn_bool _awaitResult = VRPN_FALSE);

    int TakeScanlineStep(const float _x, const float _y,
    		const float _z, const float _angle);

    int ScanTo (const float _x, const float _y);

    
      // Sends the microscope to (x, y).




    // Ohmmeter control

    int SetOhmmeterSampleRate (const int _rate);
      //   Sets the sample rate on the #0 ohmmeter.

    int EnableAmp (const int _which, const float _offset,
                   const float _uncalOffset, const int _gain);
      //   Enables one of the amplifier channels with specified offsets
      // and gain.

    int DisableAmp (const int _which);
      // Disables one of the amplifier channels.

    int EnableVoltsource (const int _which, const float _voltage);
      // Enables one of the voltage sources at a specified voltage.

    int DisableVoltsource (const int _which);
      // Enables one of the voltage sources.

    int RecordResistance(int channel, struct timeval t, float r,
			float voltage, float range, float filter, int status); // HACK - AAS

    // RECEIVES

    int HandleReports (void);
      // handle all pending communication from the microscope

    int HandlePacket (void);
      // handle a single message from the microscope
      // used for polling




    // ODDS AND ENDS

    int InputStreamPosition (void) const;
    int OutputStreamPosition (void) const;
      // returns the current offset from the start of the stream, in bytes

    void GetCurrTime (struct timeval * t) const;
    void GetStartTime (struct timeval * t) const;

    void RestartStream();
      // resets the input stream to the beginning.
    void SkipInputStream (const Direction);
      // skip forwards or backwards in the input stream by "one frame"
      // not implemented

    void SetStreamReplayRate(int replayRate);

    int GetStreamReplayRate(); 

    void SetStreamToTime(struct timeval time);

    void ResetClock (void);

    Blunt_result * getBluntResult (void)
      { return &d_bluntResult; }

    // Register callbacks from the user interface so we can decouple some
    // UI functions that currently go through here.

    int registerPointDataHandler
               (int (* handler) (void *, const Point_results *),
                void * userdata);
    int unregisterPointDataHandler
               (int (* handler) (void *, const Point_results *),
                void * userdata);

    int registerModifyModeHandler (int (* handler) (void *),
                                   void * userdata);
    int unregisterModifyModeHandler (int (* handler) (void *),
                                     void * userdata);
    int registerImageModeHandler (int (* handler) (void *),
                                  void * userdata);
    int unregisterImageModeHandler (int (* handler) (void *),
                                    void * userdata);
    int registerScanlineModeHandler (int (* handler) (void *),
                                  void *userdata);
    int unregisterScanlineModeHandler (int (* handler) (void *),
                                  void *userdata);
    int registerScanlineDataHandler (int (* handler) (void *,
                                        const Scanline_results *),
                                  void *userdata);
    int unregisterScanlineDataHandler (int (* handler) (void *,
                                        const Scanline_results *),
                                  void *userdata);

    // Limit # packets (messages) played back per call to HandleReports()
    void setPlaybackLimit (int);

  protected:

        // Added by Tiger       moved from nmm_Microscope.h
    nmb_Dataset * d_dataset;
    nmb_Decoration * d_decoration;
        // Tiger


  private: // OBSOLETE

    enum SampleMode { Visual, Haptic };
    int SetSamples (const SampleMode);

    // STM control and query
    int SetBias (const float _bias);
      //   Sets the voltage difference to maintain between the tip and
      // the sample during normal scanning.

    int SetPulsePeak (const float _height);
      //   Sets the voltage difference to impose between the tip and the
      // sample while pulsing.

    int SetPulseDuration (const float _width);
      // Sets the length of time to hold at high voltage during a pulse.

    int QueryPulseParams (void);
      // Queries the current state of the pulse generator.




  private:


    // initialization
    int InitDevice (const vrpn_bool, const vrpn_bool,
                    const int, const char *, const int, const int);
    int InitStream (const char *);
    int Init (int (*) (stm_stream *));



    // Commands only sent by member functions
    int SetStdDevParams (void);

    int SetStdDevParams (const int, const float);
      // Sets the number of samples to be taken at each point in the grid
      // and the rate in Hz at which to sample.

    int QueryStdDevParams (void);
    int QueryScanRange (void);
    int SetGridSize (const int _x, const int _y);
      // Set the size of the grid:  # data points to collect in each dimension.

    int SetScanWindow (const int _minx, const int _miny,
                       const int _maxx, const int _maxy);
      //   Choose a subset of the entire grid to scan.  Specifies the
      // corners of a rectangular region inside the originally-specified
      // grid to limit the scan to.




    // Receive callbacks from io
    // Code is in MicroscopeRcv.C
    void RcvInTappingMode (const float, const float, const float,
                           const float, const float);
    void RcvInContactMode (const float, const float, const float,
                           const float);
    void RcvInSewingStyle (const float, const float, const float, const float,
                          const float, const float, const float);
    void RcvInSpectroscopyMode (const float,const float,const float, 
	const float,const float,const float,const float,const int,const int,
	const float,const float,const float,const float,
	const int, const float,const float,const float);
    void RcvForceParameters (const int, const float);
    void RcvBaseModParameters (const float, const float);
    void RcvForceSettings (const float, const float, const float);
    void RcvVoltsourceEnabled (const int, const float);
    void RcvVoltsourceDisabled (const int);
    void RcvAmpEnabled (const int, const float, const float, const int);
    void RcvAmpDisabled (const int);
    void RcvStartingToRelax (const int, const int);
    void RcvInModModeT (const int, const int);
    void RcvInModMode (void);
    void RcvInImgModeT (const int, const int);
    void RcvInImgMode (void);
    void RcvModForceSet (const float);
    void RcvImgForceSet (const float);
    void RcvModSet (const int, const float, const float, const float);
    void RcvImgSet (const int, const float, const float, const float);
    void RcvRelaxSet (const int, const int);
    void RcvForceSet (const float);
    void RcvForceSetFailure (const float);
    void RcvPulseParameters (const int, const float, const float, const float);
    void RcvStdDevParameters (const int, const float);
    int RcvWindowLineData (const int, const int, const int, const int,
                            const int, const float *);
    int RcvWindowLineData (void);
    void RcvWindowScanNM (const int, const int, const int, const int,
                          const float, const float);
    void RcvWindowBackscanNM (const int, const int, const int, const int,
                              const float, const float);
    void RcvPointResultNM (const float, const float, const int, const int,
                           const float, const float);
    void RcvResultData (const int, const float, const float,
                        const int, const int, const int, const float *);
    void RcvResultNM (const float, const float, const int, const int,
                      const float, const float, const float, const float);
    void RcvPulseCompletedNM (const float, const float);
    void RcvPulseFailureNM (const float, const float);
    void RcvScanRange (const float, const float, const float, const float,
                       const float, const float);
    void RcvSetRegionC (const int,
                        const float, const float, const float, const float);
    void RcvResistanceFailure (const int);
    void RcvResistance (const int, const int, const int, const float);
    void RcvResistance2(const int, const int, const int, const float,
			const float, const float, const float);
    void RcvResistance3(const int, const int, const int, const float,
			const float, const float, const float);
    void RcvResistanceWithStatus(const int, const int, const int, const float,
			const float, const float, const float,const long);
    void RcvReportSlowScan (const int);
    void RcvScanParameters (const char **);
    void RcvHelloMessage (const char *, const char *, const int, const int);
    void RcvClientHello (const char *, const char *, const int, const int);
    void RcvClearScanChannels (void);
    void RcvScanDataset (const char *, const char *, const float, const float);
    void RcvClearPointChannels (void);
    void RcvPointDataset (const char *, const char *, const int,
                         const float, const float);
    void RcvPidParameters (const float, const float, const float);
    void RcvScanrateParameter (const float);
    int RcvReportGridSize (const int, const int);
    void RcvServerPacketTimestamp (const int, const int);
    void RcvTopoFileHeader (const int, const char *);
    void RcvForceCurveData(const float _x, const float _y, const int _sec,
	const int _usec, const int _num_points, const int _num_halfcycles,
	const float * z_values, const float **curves);

    void RcvInScanlineMode (const long);
    void RcvClearScanlineChannels (void);
    void RcvScanlineDataset (const char *, const char *, const float,
                        const float);
    void RcvScanlineDataHeader (const float _x, const float _y, 
	  const float _z, const float _angle, const float _slope,
	  const float _width, const long _resolution, 
	  const long _enable_feedback, const long _check_forcelimit,
	  const float _max_force, const float _max_z_step, const float _max_xy_step,
	  const long _sec, const long _usec,
	  const long _num_channels);
    void RcvScanlineData (const long point, const long num_channels,
	  const float * value);

    // messages for Michele Clark's experiments
    // Code is in MicroscopeRcv.C
    void RcvRecvTimestamp (const struct timeval);
    void RcvFakeSendTimestamp (const struct timeval);
    void RcvUdpSeqNum (const long);


    // limit # packets played back per call to HandleReports()
    int d_playbackLimit;



    static int handle_InTappingMode (void *, vrpn_HANDLERPARAM);
    static int handle_InContactMode (void *, vrpn_HANDLERPARAM);
    static int handle_InSewingStyle (void *, vrpn_HANDLERPARAM);
    static int handle_ForceParameters (void *, vrpn_HANDLERPARAM);
    static int handle_BaseModParameters (void *, vrpn_HANDLERPARAM);
    static int handle_ForceSettings (void *, vrpn_HANDLERPARAM);
    static int handle_VoltsourceEnabled (void *, vrpn_HANDLERPARAM);
    static int handle_VoltsourceDisabled (void *, vrpn_HANDLERPARAM);
    static int handle_AmpEnabled (void *, vrpn_HANDLERPARAM);
    static int handle_AmpDisabled (void *, vrpn_HANDLERPARAM);
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
    static int handle_StdDevParameters (void *, vrpn_HANDLERPARAM);
    static int handle_WindowLineData (void *, vrpn_HANDLERPARAM);
    static int handle_WindowScanNM (void *, vrpn_HANDLERPARAM);
    static int handle_WindowBackscanNM (void *, vrpn_HANDLERPARAM);
    static int handle_PointResultNM (void *, vrpn_HANDLERPARAM);
    static int handle_PointResultData (void *, vrpn_HANDLERPARAM);
    static int handle_BottomPunchResultData (void *, vrpn_HANDLERPARAM);
    static int handle_TopPunchResultData (void *, vrpn_HANDLERPARAM);
    static int handle_ResultNM (void *, vrpn_HANDLERPARAM);
    static int handle_PulseCompletedNM (void *, vrpn_HANDLERPARAM);
    static int handle_PulseFailureNM (void *, vrpn_HANDLERPARAM);
    static int handle_ScanRange (void *, vrpn_HANDLERPARAM);
    static int handle_SetRegionCompleted (void *, vrpn_HANDLERPARAM);
    static int handle_SetRegionClipped (void *, vrpn_HANDLERPARAM);
    static int handle_ResistanceFailure (void *, vrpn_HANDLERPARAM);
    static int handle_Resistance (void *, vrpn_HANDLERPARAM);
    static int handle_Resistance2(void *, vrpn_HANDLERPARAM);
    static int handle_ReportSlowScan (void *, vrpn_HANDLERPARAM);
    static int handle_ScanParameters (void *, vrpn_HANDLERPARAM);
    static int handle_HelloMessage (void *, vrpn_HANDLERPARAM);
    static int handle_ClientHello (void *, vrpn_HANDLERPARAM);
    static int handle_ClearScanChannels (void *, vrpn_HANDLERPARAM);
    static int handle_ScanDataset (void *, vrpn_HANDLERPARAM);
    static int handle_ClearPointChannels (void *, vrpn_HANDLERPARAM);
    static int handle_PointDataset (void *, vrpn_HANDLERPARAM);
    static int handle_PidParameters (void *, vrpn_HANDLERPARAM);
    static int handle_ScanrateParameter (void *, vrpn_HANDLERPARAM);
    static int handle_ReportGridSize (void *, vrpn_HANDLERPARAM);
    static int handle_ServerPacketTimestamp (void *, vrpn_HANDLERPARAM);
    static int handle_TopoFileHeader (void *, vrpn_HANDLERPARAM);

    // messages for Michele Clark's experiments
    static int handle_RecvTimestamp (void *, vrpn_HANDLERPARAM);
    static int handle_FakeSendTimestamp (void *, vrpn_HANDLERPARAM);
    static int handle_UdpSeqNum (void *, vrpn_HANDLERPARAM);


    // OBSOLETE
    //void RcvSnapShotBegin (const int, const int);
    //void RcvSnapShotEnd (void);


// TODO:  Document and explain these better!

    void GetRasterPosition (const int, const int);
      // sets state.rasterX and state.rasterY

    void DisplayModResult (const float, const float, const float,
                           const Point_value * = NULL,
                           const vrpn_bool = VRPN_FALSE);
      // sets state.rasterX and state.rasterY


    int streamReplayRate;

    MicroscopeIO * io;

    // replacing things in microscape
    struct timeval d_nowtime;
    struct timezone d_nowzone;
    struct timeval d_next_time;

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

    pointDataHandlerEntry * d_pointDataHandlers;
    modeHandlerEntry * d_modifyModeHandlers;
    modeHandlerEntry * d_imageModeHandlers;
    modeHandlerEntry * d_scanlineModeHandlers;
    scanlineDataHandlerEntry *d_scanlineDataHandlers;

    void doImageModeCallbacks (void);
    void doModifyModeCallbacks (void);
    void doPointDataCallbacks (const Point_results *);
    void doScanlineModeCallbacks (void);
    void doScanlineDataCallbacks (const Scanline_results *);
};

#endif  // MICROSCOPE_H


