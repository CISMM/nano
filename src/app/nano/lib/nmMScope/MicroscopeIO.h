#ifndef NETWORKED_MICROSCOPE_H
#define NETWORKED_MICROSCOPE_H

#include "NetworkedMicroscopeChannel.h"

//#if !defined(_WIN32) || defined(__CYGWIN__)
#if !defined(_WIN32)
#include <sys/time.h>
#else
#include <windows.h>
#include <winsock.h>  // for timeval
#endif

//#include <v.h>  // vrpn_bool

#include <Tcl_Linkvar.h>
#include <stm_file.h>

class Microscope;

// MicroscopeIO
//
// Tom Hudson, September 1997
// Code mostly from animate.c and perhaps microscape.c

// This class handles all of the Microscope's stream I/O.

// Perhaps a redesign is in order:  communication with a 'scope
// or streamfile fits together cleanly and layers well, but might
// as well be in a separate class from output for logging, since
// logging seems to force layering violations.

// Parts of this are expected to change when we convert the microscope
// to VRPN (most of the implementation), but not too much of the interface.
// Rather, we think the management will move down to the vrpn_Connection,
// and just packing & unpacking happen here.

class MicroscopeIO {

  public:

    MicroscopeIO (Microscope *);
    //MicroscopeIO (const vrpn_bool, const vrpn_bool, const vrpn_bool);
    ~MicroscopeIO (void);

    vrpn_bool IsMicroscopeOpen (void) const
      { return comm.IsMicroscopeOpen(); }

    int InitDevice (const char * _deviceName);
    int InitDevice (const int, const char * _SPMhost,
                    const int _SPMport, const int _UDPport);
    int InitStream (const char * _inputStreamName);

    int RestartStream (void);

    int RecordResistance(int meter, struct timeval t, float r,
		float voltage, float range, float filter, int status); // HACK - AAS

    int HandlePacket (void);


    int EnableOutputStream (int (*) (stm_stream *));
      // argument is a function that writes any necessary cookies
      // to the stream, returning nonzero on failure

    int InputStreamPosition (void) const;
    int OutputStreamPosition (void) const;
      // returns offset from start of stream, in bytes

    void GetCurrTime (struct timeval * t) const;
    void GetStartTime (struct timeval * t) const;

    void SkipInputStream (const int);
      // skips some distance through the input stream, forwards or backwards

    void SetStreamToTime(struct timeval t);
      //sets the stream to the time relative to the stream start time

    // OBSOLETE

    //void CutOutputStream (void);
      // effectively stops generating the output stream from a certain
      // point
      // WARNING:  Not general.  Using this command properly will require
      // modifications of the MicroscopeIO class code, due to layering
      // violations it seems to entail.
    //void ResumeOutputStream (void);
      // effectively resumes generating the output stream at the last
      // Cut position

    //void CloseOutputStream (void);


    // SENDS

    int Shutdown (void);

    int ResumeWindowScan (void);


    int SetRegionNM (const float, const float, const float, const float);

    int ScanTo (const float, const float);
    int ScanTo (const float, const float, const float);
    int ZagTo (const float, const float, const float,
               const float, const float);

    int MarkModifyMode (void);
    int MarkImageMode (void);

    int EnterTappingMode (const float, const float, const float,
                          const float, const float);
    int EnterContactMode (const float, const float, const float,
                          const float);
    int EnterDirectZControl (const float, const float, const float,
			     const float, const float);
    int EnterSewingStyle (const float, const float, const float,
                          const float, const float, const float,
                          const float);
    int EnterForceCurveStyle (const float, const float, const float,
			      const float, const float, const float,
			      const float, const int, const int,
			      const float, const float, const float,
			      const float, const int,
			      const float, const float, const float);

    int SetScanStyle (const int);
    int SetSlowScan (const int);

    int SetStdDelay (const int);
    int SetStPtDelay (const int);
    int SetRelax (const int, const int);
    //int SetStdDevParams (const int, const float);

    int SetScanWindow (const int, const int, const int, const int);
    int SetGridSize (const int, const int);

    int SetOhmmeterSampleRate (const int);
    int EnableAmp (const int, const float, const float, const int);
    int DisableAmp (const int);
    int EnableVoltsource (const int, const float);
    int DisableVoltsource (const int);

    int SetBias (const float);
    int SetPulsePeak (const float);
    int SetPulseDuration (const float);
    int QueryPulseParams (void);

    int QueryScanRange (void);
    //int QueryStdDevParams (void);

    int SetRateNM (const float);
    int SetMaxMove (const float);
   // This does nothing because AFM ignores the message.
    int SetModForce (const float, const float, const float);

    int DrawSharpLine (const float, const float, const float, const float,
                       const float);
    int DrawSweepLine (const float, const float, const float, const float,
                       const float, const float, const float);
    int DrawSweepArc (const float, const float, const float, const float,
                      const float, const float);
    int GetNewPointDatasets (const Tclvar_checklist_with_entry *);
    int GetNewScanDatasets (const Tclvar_checklist_with_entry *);

    int EnterScanlineMode(const long);
    int RequestScanLine(const float, const float, const float, const float,
    const float, const float, const long,
    const long, const long, const float, const float, const float);

  private:

    Microscope * microscope;

    NetworkedMicroscopeChannel comm;

    stm_stream * inputStream;
    stm_stream * outputStream;

    int readMode;

    char buffer [MAXBUF];
    char * bufptr;
    unsigned int bufsize;

    // for buffer manipulations during snapshots
    //char * streamCutPoint;

    // for keeping a queue of incoming data
    // from Michele Clark's experiments
    vrpn_bool ready;
    int queueLength;
    int queueTail,
        queueHead;
    struct queue {
      char data [MAXBUF];
      int len;
    } dataQ [20];


    int sec, usec;

    struct timeval stream_time;		// current timestamp for stream messages
    struct timeval stream_start_time;	// start timestamp for stream messages
    struct timeval d_stream_runtime;	// elapsed time from start time

    struct timeval temp_time;  // Michele Clark's experiments

    // Given a packet, read out each message and dispatch it to
    // its handler.  If _enableQueueing is VRPN_TRUE, WindowLineData
    // *and all subsequent messages in that packet* may be queued.
    int DispatchPacket (char *, const int, const vrpn_bool _enableQueueing);

    // Do we want to migrate this here some day?  Does it belong?
    //int DescribeVersion (stm_stream *);

    // called by HandlePacket()
    int GetPacketFromDevice (char *, const int);
    int GetPacketFromStream (char *, const int, vrpn_bool &);

    // routines to unpack messages received from comm and pass
    // them up to callbacks in Microscope class

    void RcvInTappingMode (char **);
    void RcvInContactMode (char **);
    void RcvInSewingStyle (char **);
    void RcvInSpectroscopyMode (char **);

    void RcvForceParameters (char **);
    void RcvForceSettings (char **);
    void RcvForceSetFailure (char **);
    void RcvForceSet (char **);

    void RcvBaseModParameters (char **);
    void RcvModForceSet (char **);
    void RcvModSet (char **);
    void RcvInModModeT (char **);
    void RcvInModMode (char **);

    void RcvImgForceSet (char **);
    void RcvImgSet (char **);
    void RcvInImgModeT (char **);
    void RcvInImgMode (char **);

    void RcvPulseParameters (char **);
    void RcvPulseCompletedNM (char **);
    void RcvPulseFailureNM (char **);

    void RcvVoltsourceEnabled (char **);
    void RcvVoltsourceDisabled (char **);
    void RcvAmpEnabled (char **);
    void RcvAmpDisabled (char **);
    void RcvResistanceFailure (char **);
    void RcvResistance (char **);
    void RcvResistance2 (char **);
	void RcvResistanceWithStatus (char **);

    void RcvStartingToRelax (char **);
    void RcvRelaxSet (char **);
    void RcvStdDevParameters (char **);
    void RcvWindowLineData (char **);
    void RcvWindowScanNM (char **);
    void RcvWindowBackscanNM (char **);
    void RcvPointResultNM (char **);
    void RcvResultData (const int, char **);
    void RcvResultNM (char **);
    void RcvScanRange (char **);
    void RcvSetRegionC (const int, char **);
    void RcvReportSlowScan (char **);
    void RcvScanParameters (char **);
    void RcvHelloMessage (char **);
    void RcvClientHello (char **);
    void RcvPidParameters (char **);
    void RcvScanrateParameter (char **);
    void RcvReportGridSize (char **);
    void RcvServerPacketTimestamp (char **);
    void RcvTopoFileHeader (char **);
    void RcvForceCurveData (char **);

    void RcvScanDatasets (char **);
    void RcvPointDatasets (char **);

    void RcvInScanlineMode (char **);
    void RcvScanlineData (char **);

    //void GetTimestamp (char * &, int &, int &);
    //void GetRasterPosition (char * &, int &, int &);

    // messages & functions for Michele Clark's experiments
    void RcvRecvTimestamp (char **);
    void RcvFakeSendTimestamp (char **);
    void RcvUdpSeqNum (char **);

    int EnqueueWindowLineData (const int, char * &, const int);
      // Queues window line data.
      // ASSUMES that the WLD is the last message in the packet.
      // Messages packed after it may be delayed or lost.
};

#endif  // NETWORKED_MICROSCOPE_H
