#ifndef NMM_MICROSCOPE_H
#define NMM_MICROSCOPE_H

#ifndef VRPN_CONNECTION_H		// Tiger added it. It was in AFMState.h
#include <vrpn_Connection.h>  // for vrpn_MESSAGEHANDLER
#endif

class vrpn_File_Controller;  // from <vrpn_FileController.h>
class vrpn_StreamForwarder;  // <vrpn_Forwarder.h>
class vrpn_ConnectionForwarder;

class vrpn_Connection;

#include <nmb_Device.h>

// nmm_Microscope
//
// Tom Hudson, May 1998

// Base class for microscope controllers.
// Speaks VRPN as the network protocol.
// Declares and initializes network ID for objects of this class
// and for each message type.
// Defines encode() and decode() functions for each message type.


class nmm_Microscope {

  public:

    nmm_Microscope (const char * name,
                    vrpn_Connection * = NULL);
      // Constructor.
      //   If the Microscope is a server, the Connection should be set up
      // to listen on a port;  if the Microscope is a remote, the
      // connection should have connected to the appropriate server.

    virtual ~nmm_Microscope (void);
      // Destructor. HP compiler doesn't like this declared with "= 0;"

//    virtual long mainloop (void);

    enum Direction { Forward, Backward };

    // MANIPULATORS

    static void registerMicroscopeMessagesForForwarding
                (vrpn_StreamForwarder * forwarder);
    static void registerMicroscopeMessagesForForwarding
                (vrpn_ConnectionForwarder * forwarder,
                 const char * sourceServiceName,
                 const char * destinationServiceName);

    static void registerMicroscopeOutputMessagesForForwarding
                (vrpn_StreamForwarder * forwarder);
    static void registerMicroscopeOutputMessagesForForwarding
                (vrpn_ConnectionForwarder * forwarder,
                 const char * sourceServiceName,
                 const char * destinationServiceName);
    static void registerMicroscopeInputMessagesForForwarding
                (vrpn_StreamForwarder * forwarder);
    static void registerMicroscopeInputMessagesForForwarding
                (vrpn_ConnectionForwarder * forwarder,
                 const char * sourceServiceName,
                 const char * destinationServiceName);

  protected:

//    vrpn_Connection * d_connection; moved to nmb_Device
//    vrpn_File_Controller * d_fileController; moved to nmb_Device

    char * d_tcl_script_dir;

    // VRPN stuff

//    long d_myId; moved to nmb_Device

    static long d_numOutputMessageNames;
    static char * d_outputMessageName [];
    static long d_numInputMessageNames;
    static char * d_inputMessageName [];

    // general messages

    long d_SetRegionNM_type;  // client ==> server
    long d_ScanTo_type;
    long d_ScanToZ_type;
    long d_ZagTo_type;
    long d_SetScanStyle_type;
    long d_SetSlowScan_type;
    long d_SetStdDelay_type;
    long d_SetStPtDelay_type;
    long d_SetRelax_type;
    long d_RecordResistance_type;
    long d_SetStdDevParams_type;
    long d_SetScanWindow_type;
    long d_ResumeWindowScan_type;
    long d_SetGridSize_type;
    long d_SetOhmmeterSampleRate_type;
    long d_EnableAmp_type;
    long d_DisableAmp_type;
    long d_EnableVoltsource_type;
    long d_DisableVoltsource_type;
    long d_SetRateNM_type;
    long d_SetMaxMove_type;
    long d_SetModForce_type;
    long d_DrawSharpLine_type;
    long d_DrawSweepLine_type;
    long d_DrawSweepArc_type;
    long d_GetNewPointDatasets_type;
    long d_GetNewScanDatasets_type;
    long d_Echo_type;
    long d_MarkModify_type;
    long d_MarkImage_type;
    long d_Shutdown_type;
    long d_QueryScanRange_type;
    long d_QueryStdDevParams_type;
    long d_QueryPulseParams_type;

    // scanline mode - should probably be called linescan mode
    long d_EnterScanlineMode_type;
    long d_RequestScanLine_type;

    long d_JumpToScanLine_type;

    long d_VoltsourceEnabled_type;  // server ==> client
    long d_VoltsourceDisabled_type;
    long d_AmpEnabled_type;
    long d_AmpDisabled_type;
    long d_StartingToRelax_type;
    long d_RelaxSet_type;
    long d_StdDevParameters_type;
    long d_WindowLineData_type;
    long d_WindowScanNM_type;
    long d_WindowBackscanNM_type;
    long d_PointResultNM_type;
    long d_PointResultData_type;
    long d_BottomPunchResultData_type;
    long d_TopPunchResultData_type;
      // PointResultData, BottomPunchResultData, and TopPunchResultData
      // are all handled by encode_ResultData() and decode_ResultData()
    long d_ZigResultNM_type;
    long d_BluntResultNM_type;
      // ZigResultNM and BluntResultNM are both handled
      // by encode_ResultNM() and decode_ResultNM()
    long d_ScanRange_type;
    long d_SetRegionCompleted_type;
    long d_SetRegionClipped_type;
    long d_ResistanceFailure_type;
    long d_Resistance_type;
    long d_Resistance2_type;
    long d_ReportSlowScan_type;
    long d_ScanParameters_type;
    long d_HelloMessage_type;
    long d_ClientHello_type;
    long d_ScanDataset_type;
    long d_PointDataset_type;
    long d_PidParameters_type;
    long d_ScanrateParameter_type;
    long d_ReportGridSize_type;
    long d_ServerPacketTimestamp_type;
    long d_TopoFileHeader_type;
    long d_ForceCurveData_type;

    long d_InScanlineMode_type;
    long d_ScanlineData_type;
    
    long d_MaxSetpointExceeded_type;

    // messages for Michele Clark's experiments
    long d_RecvTimestamp_type;
    long d_FakeSendTimestamp_type;
    long d_UdpSeqNum_type;

    // AFM-ish

    long d_EnterTappingMode_type;  // from client
    long d_EnterContactMode_type;
    long d_EnterDirectZControl_type;
    long d_EnterSewingStyle_type;
    long d_SetContactForce_type;
    long d_QueryContactForce_type;
    long d_EnterSpectroscopyMode_type;

    long d_ForceSet_type;  // from server
    long d_ForceSetFailure_type;
    long d_ForceParameters_type;
    long d_ModForceSet_type;
    long d_ImgForceSet_type;
    long d_ModForceSetFailure_type;
    long d_ImgForceSetFailure_type;
    long d_ImgSet_type;
    long d_ModSet_type;
    long d_InModMode_type;
    long d_InImgMode_type;
    long d_InModModeT_type;
    long d_InImgModeT_type;
    long d_InTappingMode_type;
    long d_InContactMode_type;
    long d_InDirectZControl_type;
    long d_InSewingStyle_type;
    long d_InSpectroscopyMode_type;

    long d_BaseModParameters_type;  // from client to itself
    long d_ForceSettings_type;

    // STM-ish

    long d_SampleApproach_type;  // from client
    long d_SetBias_type;
    long d_SampleApproachNM_type;
    long d_SetPulsePeak_type;
    long d_SetPulseDuration_type;
    long d_PulsePoint_type;
    long d_PulsePointNM_type;

    long d_TunnellingAttained_type;  // from server
    long d_TunnellingAttainedNM_type;
    long d_TunnellingFailure_type;
    long d_ApproachComplete_type;
    long d_PulseCompleted_type;
    long d_PulseFailure_type;
    long d_PulseParameters_type;
    long d_PulseCompletedNM_type;
    long d_PulseFailureNM_type;




    // encode_ functions take as parameters all the data carried in
    // the message.  They allocate a char array and marshall the message's
    // data in it, returning its length in their first argument (long * len).
    // On error, these functions return a NULL pointer and a 0 length.
    // It is the caller's responsibility to delete [] the array returned
    // after using it.

    // decode_ functions take a pointer to the buffer to unmarshall
    // and variables to unmarshall longo.
    // They advance this pointer past the end of the unmarshalled data.
    // Passing a NULL into these routines is an error.
    // They return 0 on success, -1 on error.

    // Variable-length messages with a regular repeating format
    // (e.g. WindowLineData, ScanDatasets) have two decode_ functions.
    // The first reads the fixed header, which includes a count of the
    // number of records that follow.  The second reads one record.


    char * encode_SetRegionNM (long * len,
                  float minx, float miny, float maxx, float maxy);
    long decode_SetRegionNM (const char ** buf,
               float * minx, float * miny, float * maxx, float * maxy);
    char * encode_ScanTo (long * len, float x, float y);
    long decode_ScanTo (const char ** buf, float * x, float * y);
    char * encode_ScanTo (long * len, float x, float y, float z);
    long decode_ScanTo (const char ** buf, float * x, float * y, float * z);
    char * encode_ZagTo (long * len, float x, float y, float yaw,
                  float sweepWidth, float regionDiag);
    long decode_ZagTo (const char ** buf, float * x, float * y, float * yaw,
               float * sweepWidth, float * regionDiag);
    char * encode_SetScanStyle (long * len, vrpn_int32 value);
    long decode_SetScanStyle (const char ** buf, vrpn_int32 * value);
    char * encode_SetSlowScan (long * len, vrpn_int32 value);
    long decode_SetSlowScan (const char ** buf, vrpn_int32 * value);
    char * encode_SetStdDelay (long * len, vrpn_int32 delay);
    long decode_SetStdDelay (const char ** buf, vrpn_int32 * delay);
    char * encode_SetStPtDelay (long * len, vrpn_int32 delay);
    long decode_SetStPtDelay (const char ** buf, vrpn_int32 * delay);
    char * encode_SetRelax (long * len, vrpn_int32 min, vrpn_int32 sep);
    long decode_SetRelax (const char ** buf, vrpn_int32 * min, 
                          vrpn_int32 * sep);
    char * encode_RecordResistance (long * len, vrpn_int32 meter, 
                  struct timeval time,
                  float resistance, float v, float r, float f);
    long decode_RecordResistance (const char ** buf, vrpn_int32 * meter,
               struct timeval * time, float * resistance, float * v,
               float * r, float * f);
    /* OBSOLETE */
    char * encode_SetStdDevParams (long * len, vrpn_int32 samples, float freq);
    long decode_SetStdDevParams (const char ** buf,
               vrpn_int32 * samples, float * freq);

    char * encode_SetScanWindow (long * len,
           vrpn_int32 minx, vrpn_int32 miny, vrpn_int32 maxx, vrpn_int32 maxy);
    long decode_SetScanWindow (const char ** buf,
           vrpn_int32 * minx, vrpn_int32 * miny, vrpn_int32 * maxx, 
           vrpn_int32 * maxy);
    char * encode_SetGridSize (long * len, vrpn_int32 x, vrpn_int32 y);
    long decode_SetGridSize (const char ** buf, vrpn_int32 * x, vrpn_int32 * y);
    char * encode_SetOhmmeterSampleRate (long * len, vrpn_int32 which, 
                                         vrpn_int32 rate);
    long decode_SetOhmmeterSampleRate (const char ** buf,
               vrpn_int32 * which, vrpn_int32 * rate);
    char * encode_EnableAmp (long * len,
                  vrpn_int32 which, float offset, float percentOffset, 
                  vrpn_int32 gain);
    long decode_EnableAmp (const char ** buf,
               vrpn_int32 * which, float * offset, float * percentOffset,
	       vrpn_int32 * gain);
    char * encode_DisableAmp (long * len, vrpn_int32 which);
    long decode_DisableAmp (const char ** buf, vrpn_int32 * which);
    char * encode_EnableVoltsource (long * len, vrpn_int32 which, 
                                    float voltage);
    long decode_EnableVoltsource (const char ** buf,
               vrpn_int32 * which, float * voltage);
    char * encode_DisableVoltsource (long * len, vrpn_int32 which);
    long decode_DisableVoltsource (const char ** buf, vrpn_int32 * which);
    char * encode_SetRateNM (long * len, float rate);
    long decode_SetRateNM (const char ** buf, float * rate);
    char * encode_SetMaxMove (long * len, float distance);
    long decode_SetMaxMove (const char ** buf, float * distance);
    char * encode_SetModForce (int * len, float newforce, float min, float max);
    int decode_SetModForce (const char ** buf, float * distance, 
                         float * min, float * max);

    char * encode_DrawSharpLine (long * len,
                  float startx, float starty, float endx, float endy,
                  float stepSize);
    long decode_DrawSharpLine (const char ** buf,
               float * startx, float * starty, float * endx, float * endy,
               float * stepSize);
    char * encode_DrawSweepLine (long * len,
                  float startx, float starty, float startYaw,
                  float startSweepWidth, float endx, float endy, float endYaw,
                  float endSweepWidth, float stepSize);
    long decode_DrawSweepLine (const char ** buf,
               float * startx, float * starty, float * startYaw,
               float * startSweepWidth, float * endx, float * endy,
               float * endYaw, float * endSweepWidth, float * stepSize);
    char * encode_DrawSweepArc (long * len,
                  float x, float y, float startAngle, float startSweepWidth,
                  float endAngle, float endSweepWidth, float stepSize);
    long decode_DrawSweepArc (const char ** buf,
               float * x, float * y, float * startAngle,
               float * startSweepWidth, float * endAngle,
               float * endSweepWidth, float * stepSize);
/*  Tiger	moved it to nmm_MicroscopeRemote
    char * encode_GetNewPointDatasets (long * len, const Tclvar_checklist *);
*/
    long decode_GetNewPointDatasetHeader (const char ** buf, vrpn_int32 * numSets);
    long decode_GetNewPointDataset (const char ** buf,
               char * name, vrpn_int32 * numSamples);
/*  Tiger	moved it to nmm_MicroscopeRemote
    char * encode_GetNewScanDatasets (long * len, const Tclvar_checklist *);
*/
    long decode_GetNewScanDatasetHeader (const char ** buf, vrpn_int32 * numSets);
    long decode_GetNewScanDataset (const char ** buf, char * name);


    char * encode_VoltsourceEnabled (long * len, vrpn_int32, float);
    long decode_VoltsourceEnabled (const char ** buf, vrpn_int32 *, float *);
    char * encode_VoltsourceDisabled (long * len, vrpn_int32);
    long decode_VoltsourceDisabled (const char ** buf, vrpn_int32 *);
    char * encode_AmpEnabled (long * len, vrpn_int32, float, float, vrpn_int32);
    long decode_AmpEnabled (const char ** buf, vrpn_int32 *, float *, 
                            float *, vrpn_int32 *);
    char * encode_AmpDisabled (long * len, vrpn_int32);
    long decode_AmpDisabled (const char ** buf, vrpn_int32 *);
    char * encode_StartingToRelax (long * len, vrpn_int32, vrpn_int32);
    long decode_StartingToRelax (const char ** buf, vrpn_int32 *, vrpn_int32 *);
    char * encode_RelaxSet (long * len, vrpn_int32, vrpn_int32);
    long decode_RelaxSet (const char ** buf, vrpn_int32 *, vrpn_int32 *);
    /* OBSOLETE */
    char * encode_StdDevParameters (long * len, vrpn_int32, float);
    long decode_StdDevParameters (const char ** buf, vrpn_int32 *, float *);
    char * encode_WindowLineData (long * len, vrpn_int32 x, vrpn_int32 y, 
               vrpn_int32 dx, vrpn_int32 dy, vrpn_int32, vrpn_int32,
               vrpn_int32 *, vrpn_int32 sec, vrpn_int32 usec,
               unsigned short ** data);
    char * encode_WindowLineData (long * len, vrpn_int32 x, vrpn_int32 y, 
               vrpn_int32 dx, vrpn_int32 dy, vrpn_int32, vrpn_int32,
               vrpn_int32 sec, vrpn_int32 usec,
               float ** data);
    long decode_WindowLineDataHeader (const char ** buf, vrpn_int32 *,
               vrpn_int32 *, vrpn_int32 *, vrpn_int32 *,
               vrpn_int32 *, vrpn_int32 *, vrpn_int32 *, vrpn_int32 *);
    long decode_WindowLineDataField (const char ** buf, 
               vrpn_int32, float *);
    char * encode_WindowScanNM (long * len, vrpn_int32, vrpn_int32, vrpn_int32,
                      vrpn_int32, float, float);
    long decode_WindowScanNM (const char ** buf, vrpn_int32 *, 
                      vrpn_int32 *, vrpn_int32 *, vrpn_int32 *,
                      float *, float *);
    char * encode_WindowBackscanNM (long * len, vrpn_int32, vrpn_int32, 
                      vrpn_int32, vrpn_int32, float, float);
    long decode_WindowBackscanNM (const char ** buf, vrpn_int32 *, 
                      vrpn_int32 *, vrpn_int32 *,
                      vrpn_int32 *, float *, float *);
    char * encode_PointResultNM (long * len, float, float, vrpn_int32, 
                      vrpn_int32, float, float);
    long decode_PointResultNM (const char ** buf, float *, float *, 
                      vrpn_int32 *, vrpn_int32 *, float *, float *);
    char * encode_ResultData (long * len, float, float, vrpn_int32, 
                      vrpn_int32, vrpn_int32, float *);
    long decode_ResultData (const char ** buf, float *, float *, vrpn_int32 *,
                            vrpn_int32 *, vrpn_int32 *, float *);
    char * encode_ResultNM (long * len, float, float, vrpn_int32, vrpn_int32, 
                            float, float, float, float);
    long decode_ResultNM (const char ** buf, float *, float *, vrpn_int32 *, 
                      vrpn_int32 *, float *, float *, float *, float *);
    char * encode_ScanRange (long * len, float, float, float, float, float,
                             float);
    long decode_ScanRange (const char ** buf, float *, float *, float *,
                           float *, float *, float *);
    char * encode_SetRegionC (long * len, float, float, float, float);
    long decode_SetRegionC (const char ** buf, float *, float *,
                            float *, float *);
    char * encode_ResistanceFailure (long * len, vrpn_int32);
    long decode_ResistanceFailure (const char ** buf, vrpn_int32 *);
    char * encode_Resistance (long * len, vrpn_int32, vrpn_int32, 
                              vrpn_int32, float);
    long decode_Resistance (const char ** buf, vrpn_int32 *, vrpn_int32 *, 
                              vrpn_int32 *, float *);
    char * encode_Resistance2 (long * len, vrpn_int32, vrpn_int32, vrpn_int32, 
			      float, float, float, float);
    long decode_Resistance2 (const char ** buf, vrpn_int32 *, 
                             vrpn_int32 *, vrpn_int32 *, float *,
                             float *, float *, float *);
    long decode_ResistanceWithStatus(const char ** buf, vrpn_int32 *, 
                vrpn_int32 *, vrpn_int32 *,
                float *, float *, float *, float *, vrpn_int32 *);
    char * encode_ResistanceWithStatus (long * len, vrpn_int32, vrpn_int32,
                vrpn_int32, float, float, float, float, vrpn_int32);

    char * encode_ReportSlowScan (long * len, vrpn_int32);
    long decode_ReportSlowScan (const char ** buf, vrpn_int32 *);
    char * encode_ScanParameters (long * len, char *);  // TODO
    long decode_ScanParameters (const char ** buf, vrpn_int32 * length,
                                char ** buffer);
      // Allocates *length characters in *buffer and fills it in with
      // the scan parameters.  It is the caller's responsibility to delete [].

    char * encode_HelloMessage (long * len, char *, char *, vrpn_int32, 
                              vrpn_int32);
    long decode_HelloMessage (const char ** buf, char *, char *, vrpn_int32 *,
                              vrpn_int32 *);
    char * encode_ClientHello (long * len, char *, char *, vrpn_int32, 
                              vrpn_int32);
    long decode_ClientHello (const char ** buf, char *, char *, vrpn_int32 *,
                              vrpn_int32 *);
    long decode_ScanDatasetHeader (const char ** buf, vrpn_int32 *);
    long decode_ScanDataset (const char ** buf, char *, char *, float *,
                             float *);
    long decode_PointDatasetHeader (const char ** buf, vrpn_int32 *);
    long decode_PointDataset (const char ** buf, char *, char *, vrpn_int32 *,
                              float *, float *);
    char * encode_PidParameters (long * len, float, float, float);
    long decode_PidParameters (const char ** buf, float *, float *,
		float *);
    char * encode_ScanrateParameter (long * len, float);
    long decode_ScanrateParameter (const char ** buf, float *);
    char * encode_ReportGridSize (long * len, vrpn_int32, vrpn_int32);
    long decode_ReportGridSize (const char ** buf, vrpn_int32 *, vrpn_int32 *);
    char * encode_ServerPacketTimestamp (long * len, vrpn_int32, vrpn_int32);
    long decode_ServerPacketTimestamp (const char ** buf, vrpn_int32 *, 
                   vrpn_int32 *);
// Implemented by Tiger
    char * encode_TopoFileHeader (long * len, char * buf, vrpn_int32 size);
// Tiger
    long decode_TopoFileHeader (const char ** buf, vrpn_int32 *, char **);
      // Allocates *length characters in *buffer and fills it in with
      // the scan parameters.  It is the caller's responsibility to delete [].

    // similar structure to WindowLineData: num_points = reports,
    //				    num_halfcycles = fields
    char * encode_ForceCurveData (long * len, float x, float y, 
               vrpn_int32 num_points,
               vrpn_int32 num_halfcycles, vrpn_int32 sec, vrpn_int32 usec,
               float *z, float **data);
    long decode_ForceCurveDataHeader (const char ** buf, float *x, float *y,
		vrpn_int32 *num_points, vrpn_int32 *num_halfcycles, 
		vrpn_int32 *sec, vrpn_int32 *usec);
    long decode_ForceCurveDataSingleLevel (const char ** buf,
		vrpn_int32 num_halfcycles, float *z, float * data);

    // Scanline mode (client-->server):
    char * encode_EnterScanlineMode(long *len, vrpn_int32);
    long decode_EnterScanlineMode(const char **buf,
                                vrpn_int32 *);
    char * encode_RequestScanLine(long *len, 	float, float, float,
                              float, float, float, 
                              vrpn_int32, vrpn_int32, vrpn_int32,
                              float, float, float);
    long decode_RequestScanLine(const char ** buf, 
                              float *, float *, float *,
                              float *, float *, float *,
                              vrpn_int32 *, vrpn_int32 *, vrpn_int32 *,
                              float *, float *, float *);

    // Scanline mode (server-->client)
    char * encode_InScanlineMode(long *len, vrpn_int32);
    long decode_InScanlineMode(const char **buf, vrpn_int32 *);
    char * encode_ScanlineData(long *len, 	float, float, float, 
			float, float, float,
			vrpn_int32, vrpn_int32, vrpn_int32,
			float, float, float,
			vrpn_int32, vrpn_int32, vrpn_int32, vrpn_int32 *,
			float *);
    long decode_ScanlineDataHeader(const char ** buf, 
			float *, float *, float *,
			float *, float *, float *,
			vrpn_int32 *, vrpn_int32 *, vrpn_int32 *,
			float *, float *, float *,
			vrpn_int32 *, vrpn_int32 *, vrpn_int32 *);
    long decode_ScanlineDataPoint(const char ** buf, vrpn_int32, float *);

    // messages for Michele Clark's experiments
    char * encode_RecvTimestamp (long * len, struct timeval);
    long decode_RecvTimestamp (const char ** buf, struct timeval *);
    char * encode_FakeSendTimestamp (long * len, struct timeval);
    long decode_FakeSendTimestamp (const char ** buf, struct timeval *);
    char * encode_UdpSeqNum (long * len, vrpn_int32);
    long decode_UdpSeqNum (const char ** buf, vrpn_int32 *);

    char * encode_JumpToScanLine (long *len, vrpn_int32 line_number);
    long decode_JumpToScanLine (const char ** buf, vrpn_int32 *line_number);

    // AFM-ish

    char * encode_EnterTappingMode (long * len, float, float, float,
                                          float, float);
    long decode_EnterTappingMode (const char ** buf, float *, float *,
                                  float *, float *, float *);
    char * encode_EnterContactMode (long * len, float, float, float,
                                          float);
    long decode_EnterContactMode (const char ** buf, float *, float *,
                                  float *, float *);
    char * encode_EnterDirectZControl (long * len, float, float, float,
                                          float, float);
    long decode_EnterDirectZControl (const char ** buf, float *, float *,
                                  float *, float *, float *);
    char * encode_EnterSewingStyle (long * len, float, float, float,
                                          float, float, float, float);
    long decode_EnterSewingStyle (const char ** buf, float *,float *,float *,
                                  float *, float *, float *,
				  float *);

    char * encode_EnterSpectroscopyMode (long * len, float, float, float, float,
	float, float, float, vrpn_int32, vrpn_int32,float,float,float,float,
	vrpn_int32, float, float, float);
    long decode_EnterSpectroscopyMode (const char ** buf,float *,float *,
                float *,
		float *, float *, float *, float *, vrpn_int32 *, vrpn_int32 *,
		float *, float *, float *, float *,
		vrpn_int32 *, float *, float *, float *);

    char * encode_InTappingMode (long * len, float, float, float,
		float, float);
    long decode_InTappingMode (const char ** buf, float *, float *, float *,
                float *, float *);
    char * encode_InContactMode (long * len, float, float, float,
		float);
    long decode_InContactMode (const char ** buf, float *, float *, float *,
                float *);
    char * encode_InDirectZControl (long * len, float, float, float,
				float, float, float, float);
    long decode_InDirectZControl (const char ** buf, float *, float *, float *,
                               float *, float *, float *, float *);
    char * encode_InSewingStyle (long * len, float, float, float,
				float, float, float,
                                float);
    long decode_InSewingStyle (const char ** buf, float *, float *, float *,
                              float *, float *, float *, float *);
    char * encode_InSpectroscopyMode (long * len,float,float,float,float,float,
		float, float, vrpn_int32, vrpn_int32,float,float,float,float,
		vrpn_int32, float, float,float);
    long decode_InSpectroscopyMode (const char ** buf, 
		float *, float *, float *, float *,
		float *, float *, float *, vrpn_int32 *, vrpn_int32 *,
		float *, float *, float *, float *,
		vrpn_int32 *, float *, float *, float *);
    char * encode_ForceParameters (long * len, vrpn_int32, float);
    long decode_ForceParameters (const char ** buf, vrpn_int32 *, float *);
    char * encode_BaseModParameters (long * len, float, float);
    long decode_BaseModParameters (const char ** buf, float *, float *);
    char * encode_ForceSettings (long * len, float, float, float);
    long decode_ForceSettings (const char ** buf, float *, float *, float *);
    char * encode_InModModeT (long * len, vrpn_int32, vrpn_int32);
    long decode_InModModeT (const char ** buf, vrpn_int32 *, vrpn_int32 *);
    char * encode_InImgModeT (long * len, vrpn_int32, vrpn_int32);
    long decode_InImgModeT (const char ** buf, vrpn_int32 *, vrpn_int32 *);
    char * encode_ModForceSet (long * len, float);
    long decode_ModForceSet (const char ** buf, float *);
    char * encode_ImgForceSet (long * len, float);
    long decode_ImgForceSet (const char ** buf, float *);
    char * encode_ModSet (long * len, vrpn_int32, float, float, float);
    long decode_ModSet (const char ** buf, vrpn_int32 *, float *, float *, 
               float *);
    char * encode_ImgSet (long * len, vrpn_int32, float, float, float);
    long decode_ImgSet (const char ** buf, vrpn_int32 *, float *, float *, 
               float *);
    char * encode_ForceSet (long * len, float);
    long decode_ForceSet (const char ** buf, float *);
    char * encode_ForceSetFailure (long * len, float);
    long decode_ForceSetFailure (const char ** buf, float *);


    // STM-ish

    char * encode_SampleApproach (long * len, float);
    long decode_SampleApproach (const char ** buf, float *);
    char * encode_SetBias (long * len, float);
    long decode_SetBias (const char ** buf, float *);
    char * encode_SampleApproachNM (long * len, float);
    long decode_SampleApproachNM (const char ** buf, float *);
    char * encode_SetPulsePeak (long * len, float);
    long decode_SetPulsePeak (const char ** buf, float *);
    char * encode_SetPulseDuration (long * len, float);
    long decode_SetPulseDuration (const char ** buf, float *);
    char * encode_PulsePoint (long * len, float, float);
    long decode_PulsePoint (const char ** buf, float *, float *);
    char * encode_PulsePointNM (long * len, float, float);
    long decode_PulsePointNM (const char ** buf, float *, float *);

    char * encode_PulseParameters (long * len, vrpn_int32, float, float, float);
    long decode_PulseParameters (const char ** buf, vrpn_int32 *, float *, 
                                 float *, float *);
    char * encode_PulseCompletedNM (long * len, float, float);
    long decode_PulseCompletedNM (const char ** buf, float *, float *);
    char * encode_PulseFailureNM (long * len, float, float);
    long decode_PulseFailureNM (const char ** buf, float *, float *);
    char * encode_PulseCompleted (long * len, float, float);
    long decode_PulseCompleted (const char ** buf, float *, float *);
    char * encode_PulseFailure (long * len, float, float);
    long decode_PulseFailure (const char ** buf, float *, float *);
    char * encode_TunnellingAttained (long * len, float);
    long decode_TunnellingAttained (const char ** buf, float *);
    char * encode_TunnellingAttainedNM (long * len, float);
    long decode_TunnellingAttainedNM (const char ** buf, float *);

// moved to nmb_Device:
//    long dispatchMessage (long len, const char * buf, vrpn_int32 type);
      // packs the message reliably and then deletes [] buf

};


#endif  // NMM_MICROSCOPE_H
