#ifndef NMM_MICROSCOPE_REMOTE_H
#define NMM_MICROSCOPE_REMOTE_H

#include "nmm_Microscope.h"
#include "nmb_Device.h"

#include "AFMState.h"

//#ifndef _WIN32
//#include <sys/time.h> // for timeval?
//#endif
#include <vrpn_Shared.h>  // for timeval/timezone

class Tclvar_checklist;  // from <Tcl_Linkvar.h>
struct stm_stream;  // from <stm_file.h>

class nmb_Dataset;  // from <nmb_Dataset.h>
class nmb_Decoration;  // from <nmb_Decoration.h>
class Point_value;  // from <Point.h>
class BCPlane;  // from <BCPlane.h>

#include "nmm_Types.h"
#include "nmm_RelaxComp.h"   // for nmm_RelaxComp

class nmm_Sample;  // from nmm_Sample.h

#ifndef VRPN_CONNECTION_H
#include <vrpn_Connection.h>  // for vrpn_HANDLERPARAM
#endif

#include <Scanline.h>

#include <Topo.h>

// Microscope
//
// Tom Hudson, September 1997
// Code mostly from microscape.c and animate.c

// Encapsulates the microscope and all functions a client might want to
// perform on it.  Controls AFMState class.

// Uses references to its private members (io and modfile) rather than
// containing them so that we (or users of our library) don't have to
// include the relevant header files.

// Source code for this class is split between two files:
//   MicroscopeRcv.C  --  callbacks
//   Microscope.C     --  all other code

// Flaws:
//   Currently contains InitializeDataset() call that'll be moved elsewhere
// someday.


class nmm_Microscope_Remote : public nmb_Device_Client, public nmm_Microscope {
    friend class nmm_RelaxComp;
  public:

    nmm_Microscope_Remote (const AFMInitializationState &,
                           vrpn_Connection *);
      //   Constructor.

    ~nmm_Microscope_Remote (void);
      // Destructor.

    virtual long mainloop (void);

    AFMState state;

    TopoFile d_topoFile;

    nmm_RelaxComp d_relax_comp;

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

    char * encode_GetNewPointDatasets (long * len, const Tclvar_checklist *);
    char * encode_GetNewScanDatasets (long * len, const Tclvar_checklist *);

    long Initialize (const vrpn_bool setRegion,
                    const vrpn_bool setMode,
                    int (*) (stm_stream *),
                    const long socketType, const char * SPMhost,
                    const long SPMport, const long UDPport);
      /**< Set up to read from a live scope.
       * Function parameter writes any necessary cookies on
       * an output stream, returning nonzero on failure
       * Last 4 parameters are for Michele Clark's experiments.  OBSOLETE?
       */

    long Initialize (int (*) (stm_stream *));
      /**< Set up to read from a streamfile (or static files)
       * Function parameter writes any necessary cookies on
       * an output stream, returning nonzero on failure  OBSOLETE?
       */

    long NewEpoch (void);
      /**< Forces refresh of grid (?) */

    void SetSampleMode (nmm_Sample *);
      /**< Passes in a pointer to the nmm_Sample subclass to be used by
       * TakeSampleSet().
       */


    // SENDS

    long ImageMode (void);
      /**<   Enter imaging mode.  Sets parameters to values given in
       * state.image (P/I/D gain, setpoint, amplitude).
       */

    long ModifyMode (void);
      /**<   Enter modify mode.  Sets parameters to values given in
       * state.modify (P/I/D gain, setpoint, amplitude, top & bottom
       * delay, z_pull, punch distance, speed, watchdog)
       */

    //long GetNewSetOfDataChannels (const long mode,
                                 //const Tclvar_checklist * dataTypes);
    long GetNewPointDatasets (const Tclvar_checklist * dataTypes);
    long GetNewScanDatasets (const Tclvar_checklist * dataTypes);
      /**<   Sets the list of datasets to be scanned by the microscope.
       * Microscope will respond with the list that it is actually scanning,
       * which may not match.
       *   every active (marked) dataset in the checklist will be requested
       */


    long ResumeScan (Point_value * = NULL,
                    BCPlane * = NULL);
      /**<   Return to image mode *and start scanning*
       *   Non-NULL parameters tell the microscope to check a small region
       * of the surface before rescanning the entire surface, and only make
       * sense with the old AFM.
       */

    long ResumeFullScan (void);

    long ResumeWindowScan (void);
      /**<   Causes the microscope to scan over the entire grid.
       * SetScanWindow() may be called to limit scanning to a smaller
       * portion of the grid;  this function undoes that operation.
       */

    long SetScanStyle (void);
      /**<   Sets the microscope to scan in X or Y first, and in a raster or
       * in boustrophedonic order.  Controlled by (state.do_raster,
       * state.do_y_fastest)
       */

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
      // Sends the microscope to (x, y).

    long ScanTo (const float x, const float y, const float z);
      // Sends the microscope to (x, y, z).

    int TakeSampleSet (float x, float y);
      /**< Uses the algorithm specified by SetSampleAlgorithm()
       * to take one or more samples in a carefully-controlled
       * pattern.  The results are made available to the user
       * by a yet-to-be-determined mechanism, probably some combination
       * of exposure on AFMState and a callback with an augmented Point_list.
       * Returns negative values on failure.
       */



    // Ohmmeter control

    long SetOhmmeterSampleRate (const long rate);
      //   Sets the sample rate on the #0 ohmmeter.

    long EnableAmp (long which, float offset,
                   float uncalOffset, long gain);
      //   Enables one of the amplifier channels with specified offsets
      // and gain.

    long DisableAmp (long which);
      // Disables one of the amplifier channels.

    long EnableVoltsource (long which, float voltage);
      // Enables one of the voltage sources at a specified voltage.

    long DisableVoltsource (long which);
      // Enables one of the voltage sources.


    long RecordResistance(long channel, struct timeval t, float r,
			float voltage, float range, float filter); 
			// XXX - this function was used for the French Ohmmeter
			// because we wanted to put data into the microscope
 			// stream file but this should be replaced now that
			// we are using vrpn for logging - e.g. either
			// set both connections to log to the same file or
			// create two separate files

    int getTimeSinceConnected(void);

    // Scanline mode - parameters sent by these functions reside in
    // state.scanline and are typically set in the user interface
    long EnterScanlineMode(void);
    long ExitScanlineMode(void);
    long AcquireScanline(void);
    long SetScanlineModeParameters(void);
    long SetFeedbackForScanline(void);
 
    // set which line to start scanning in image mode
    long JumpToScanLine(long line);

    // ODDS AND ENDS

    void ResetClock (void);

    void SetStreamToTime (struct timeval time);


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

    vrpn_int32 pointResultType (void) const;
      ///< Returns the vrpn type of a point result message;
      ///< used with nmb_DeviceSequencer/nmm_Sample.

    void accumulatePointResults (vrpn_bool);
      ///< If true, in addition to exposing point results on
      ///< state.data.inputPoint we put them on state.data.pointList.

  protected:

    nmb_Dataset * d_dataset;
    nmb_Decoration * d_decoration;

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

  private: // OBSOLETE

    enum SampleMode { Visual, Haptic };
    long SetSamples (const SampleMode);

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


    // initialization
    long InitDevice (const vrpn_bool, const vrpn_bool,
                    const long, const char *, const long, const long);
    long InitStream (const char *);
    long Init (int (*) (stm_stream *));



    // Commands only sent by member functions
/* OBSOLETE
    long SetStdDevParams (void);

    long SetStdDevParams (const long, const float);
      // Sets the number of samples to be taken at each point in the grid
      // and the rate in Hz at which to sample.

    long QueryStdDevParams (void);
*/
    long QueryScanRange (void);
    long SetGridSize (const long _x, const long _y);
      // Set the size of the grid:  # data points to collect in each dimension.

    long SetScanWindow (const long _minx, const long _miny,
                       const long _maxx, const long _maxy);
      //   Choose a subset of the entire grid to scan.  Specifies the
      // corners of a rectangular region inside the originally-specified
      // grid to limit the scan to.

    long SetRateNM (double rate);
    long MarkModifyMode (void);
    long MarkImageMode (void);
    long EnterTappingMode (float p, float i, float d, float set, float amp);
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
    long Shutdown (void);
    long GotConnection(void);
    long SetMaxMove (float distance);
    long SetStdDelay (long delay);
    long SetStPtDelay (long delay);
    long SetRelax (long min, long sep);
    int SetModForce (float newforce, float min, float max);




    // Receive callbacks from io
    // Code is in MicroscopeRcv.C
    void RcvInTappingMode (const float, const float, const float,
                           const float, const float);
    void RcvInContactMode (const float, const float, const float,
                           const float);
    void RcvInDirectZControl (const float, const float, const float,
                           const float, const float, 
                           const float, const float);
    void RcvInSewingStyle (const float, const float, const float, const float,
                          const float, const float, const float);
    void RcvInSpectroscopyMode (const float,const float,const float,
        const float,const float,const float,const float,const int,const int,
        const float,const float,const float,const float,
        const int, const float,const float,const float);

    void RcvForceParameters (const long, const float);
    void RcvBaseModParameters (const float, const float);
    void RcvForceSettings (const float, const float, const float);
    void RcvVoltsourceEnabled (const long, const float);
    void RcvVoltsourceDisabled (const long);
    void RcvAmpEnabled (const long, const float, const float, const long);
    void RcvAmpDisabled (const long);
    void RcvStartingToRelax (const long, const long);
    void RcvInModModeT (const long, const long);
    void RcvInModMode (void);
    void RcvInImgModeT (const long, const long);
    void RcvInImgMode (void);
    void RcvModForceSet (const float);
    void RcvImgForceSet (const float);
    void RcvModSet (const long, const float, const float, const float);
    void RcvImgSet (const long, const float, const float, const float);
    void RcvRelaxSet (const long, const long);
    void RcvForceSet (const float);
    void RcvForceSetFailure (const float);
    void RcvPulseParameters (const long, const float, const float, const float);
    //void RcvStdDevParameters (const long, const float);
    long RcvWindowLineData (const long, const long, const long, const long,
                            const long, const float *);
    long RcvWindowLineData (void);
    void RcvForceCurveData (float, float, long, long,
                            long, long, const float *, const float **);
    void RcvWindowScanNM (const long, const long, const long, const long,
                          const float, const float);
    void RcvWindowBackscanNM (const long, const long, const long, const long,
                              const float, const float);
    void RcvPointResultNM (const float, const float, const long, const long,
                           const float, const float);
    void RcvResultData (const long, const float, const float,
                        const long, const long, const long, const float *);
    void RcvResultNM (const float, const float, const long, const long,
                      const float, const float, const float, const float);
    void RcvPulseCompletedNM (const float, const float);
    void RcvPulseFailureNM (const float, const float);
    void RcvScanRange (const float, const float, const float, const float,
                       const float, const float);
    void RcvSetRegionC (const long,
                        const float, const float, const float, const float);
    void RcvResistanceFailure (const long);
    void RcvResistance (const long, const long, const long, const float);
    void RcvResistance2(const long, const long, const long, const float,
			const float, const float, const float);
    void RcvReportSlowScan (const long);
    void RcvScanParameters (const char **);
    void RcvHelloMessage (const char *, const char *, const long, const long);
    void RcvClientHello (const char *, const char *, const long, const long);
    void RcvClearScanChannels (void);
    void RcvScanDataset (const char *, const char *, const float, const float);
    void RcvClearPointChannels (void);
    void RcvPointDataset (const char *, const char *, const long,
                         const float, const float);
    void RcvPidParameters (const float, const float, const float);
    void RcvScanrateParameter (const float);
    int RcvReportGridSize (const long, const long);

    void RcvMaxSetpointExceeded(void);
    void RcvServerPacketTimestamp (const long, const long);
    void RcvTopoFileHeader (const long, const char *);

    void RcvInScanlineMode(const long);
    void RcvFeedbackSetForScanline(const long, const long,
			const float, const float, const float);
    void RcvClearScanlineChannels(void);
    void RcvScanlineDataset(const char *, const char *, const float, 
	const float);
    void RcvScanlineDataHeader(const float, const float, const float,
	const float, const float, const float, const long, const long, const long, 
	const float, const float, const float, const long, const long,
	const long);
    void RcvScanlineData(const long, const long, const float *);

    // messages for Michele Clark's experiments
    // Code is in MicroscopeRcv.C
    void RcvRecvTimestamp (const struct timeval);
    void RcvFakeSendTimestamp (const struct timeval);
    void RcvUdpSeqNum (const long);



    static int handle_GotConnection2 (void *, vrpn_HANDLERPARAM);
    static int handle_InTappingMode (void *, vrpn_HANDLERPARAM);
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
    //static int handle_StdDevParameters (void *, vrpn_HANDLERPARAM);
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
    static int handle_ClearScanChannels (void *, vrpn_HANDLERPARAM);  // UNUSED
    static int handle_ScanDataset (void *, vrpn_HANDLERPARAM);
    static int handle_ClearPointChannels (void *, vrpn_HANDLERPARAM);  // UNUSED
    static int handle_PointDataset (void *, vrpn_HANDLERPARAM);
    static int handle_PidParameters (void *, vrpn_HANDLERPARAM);
    static int handle_ScanrateParameter (void *, vrpn_HANDLERPARAM);
    static int handle_ReportGridSize (void *, vrpn_HANDLERPARAM);
    static int handle_ServerPacketTimestamp (void *, vrpn_HANDLERPARAM);
    static int handle_TopoFileHeader (void *, vrpn_HANDLERPARAM);

    static int handle_MaxSetpointExceeded (void *, vrpn_HANDLERPARAM);

    // scanline mode
    static int handle_InScanlineMode (void *, vrpn_HANDLERPARAM);
    static int handle_ScanlineData (void *, vrpn_HANDLERPARAM);

    // messages for Michele Clark's experiments
    static int handle_RecvTimestamp (void *, vrpn_HANDLERPARAM);
    static int handle_FakeSendTimestamp (void *, vrpn_HANDLERPARAM);
    static int handle_UdpSeqNum (void *, vrpn_HANDLERPARAM);

    // OBSOLETE
    //void RcvSnapShotBegin (const long, const long);
    //void RcvSnapShotEnd (void);
    //void RcvInSpectroscopyMode (float, float, float, float, float,
                                //float, float, long, long);


// TODO:  Document and explain these better!

    void GetRasterPosition (const long, const long);
      // sets state.rasterX and state.rasterY

    void DisplayModResult (const float, const float, const float,
                           const Point_value * = NULL,
                           const vrpn_bool = VRPN_FALSE);
      // sets state.rasterX and state.rasterY


    // replacing things in microscape
    struct timeval d_nowtime;
    struct timezone d_nowzone;
    struct timeval d_next_time;

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

    nmm_Sample * d_sampleAlgorithm;
    vrpn_bool d_accumulatePointResults;
};

#endif  // NMM_MICROSCOPE_REMOTE_H
