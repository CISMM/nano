#ifndef NMM_MICROSCOPE_SEM_H
#define NMM_MICROSCOPE_SEM_H

#include <vrpn_Connection.h>
#include <vrpn_FileController.h>
#include <vrpn_Types.h>

/* nmm_Microscope_SEM
      This is a temporary base class for the SEM until we work out 
   something a little more general based on nmm_Microscope - e.g. something
   that works for all scanning/imaging-type microscopes

   Perhaps this can serve as a guide for factoring out what should be
   shared.
*/

class nmm_Microscope_SEM {
  public:
    nmm_Microscope_SEM (const char * name,
			vrpn_Connection *c = NULL);

    virtual ~nmm_Microscope_SEM (void);
    
    virtual vrpn_int32 mainloop(void) = 0;

  typedef enum {SET_RESOLUTION,
                SET_PIXEL_INTEGRATION_TIME,
                SET_INTERPIXEL_DELAY_TIME,
                ENABLE_SCAN,
                REPORT_RESOLUTION,
                REPORT_PIXEL_INTEGRATION_TIME,
                REPORT_INTERPIXEL_DELAY_TIME,
                WINDOW_LINE_DATA,
		SCANLINE_DATA,
                POINT_DWELL_TIME,
                BEAM_BLANK_ENABLE,
                MAX_SCAN_SPAN,
                BEAM_LOCATION,
                RETRACE_DELAYS,
                DAC_PARAMS,
                EXTERNAL_SCAN_CONTROL_ENABLE} msg_t;
  protected:
//    vrpn_Connection * d_connection;
//    vrpn_File_Controller * d_fileController;

//    vrpn_int32 d_myId;

    // (client-->server) messages
    vrpn_int32 d_SetResolution_type;
    vrpn_int32 d_SetPixelIntegrationTime_type;
    vrpn_int32 d_SetInterPixelDelayTime_type;
    vrpn_int32 d_RequestScan_type;
    vrpn_int32 d_SetPointDwellTime_type;
    vrpn_int32 d_SetBeamBlankEnable_type;
    vrpn_int32 d_GoToPoint_type;
    vrpn_int32 d_SetRetraceDelays_type;
    vrpn_int32 d_SetDACParams_type;
    vrpn_int32 d_SetExternalScanControlEnable_type;

    // (server-->client) messages

    vrpn_int32 d_ReportResolution_type;
    vrpn_int32 d_ReportPixelIntegrationTime_type;
    vrpn_int32 d_ReportInterPixelDelayTime_type;
    vrpn_int32 d_WindowLineData_type;
    vrpn_int32 d_ScanlineData_type;
    vrpn_int32 d_ReportPointDwellTime_type;
    vrpn_int32 d_ReportBeamBlankEnable_type;
    vrpn_int32 d_ReportMaxScanSpan_type;
    vrpn_int32 d_ReportBeamLocation_type;
    vrpn_int32 d_ReportRetraceDelays_type;
    vrpn_int32 d_ReportDACParams_type;
    vrpn_int32 d_ReportExternalScanControlEnable_type;

    // message encode, decode functions
    // (client-->server)
    static char * encode_SetResolution (vrpn_int32 *len, vrpn_int32 x, 
        vrpn_int32 y);
    static vrpn_int32 decode_SetResolution(const char **buf, 
					vrpn_int32 *x, vrpn_int32 *y);

    static char * encode_SetPixelIntegrationTime (vrpn_int32 *len, 
					vrpn_int32 time_nsec);
    static vrpn_int32 decode_SetPixelIntegrationTime (const char **buf,
					vrpn_int32 *time_nsec);

    static char * encode_SetInterPixelDelayTime (vrpn_int32 *len,
					vrpn_int32 time_nsec);
    static vrpn_int32 decode_SetInterPixelDelayTime (const char **buf,
					vrpn_int32 *time_nsec);

    static char * encode_RequestScan (vrpn_int32 *len,
					vrpn_int32 nscans);
    static vrpn_int32 decode_RequestScan (const char **buf, vrpn_int32 *nscans);

    static char * encode_SetPointDwellTime (vrpn_int32 *len, 
                                            vrpn_int32 time_nsec);
    static vrpn_int32 decode_SetPointDwellTime (const char **buf,
                                            vrpn_int32 *time_nsec);

    static char * encode_SetBeamBlankEnable (vrpn_int32 *len,
                                             vrpn_int32 enable);
    static vrpn_int32 decode_SetBeamBlankEnable (const char **buf,
                                            vrpn_int32 *enable);

    static char * encode_GoToPoint (vrpn_int32 *len,
                              vrpn_int32 x, vrpn_int32 y);
    static vrpn_int32 decode_GoToPoint (const char **buf,
                                 vrpn_int32 *x, vrpn_int32 *y);

    static char * encode_SetRetraceDelays (vrpn_int32 *len,
                           vrpn_int32 h_time_nsec, vrpn_int32 v_time_nsec);
    static vrpn_int32 decode_SetRetraceDelays (const char **buf,
                           vrpn_int32 *h_time_nsec, vrpn_int32 *v_time_nsec);

    static char * encode_SetDACParams (vrpn_int32 *len,
                 vrpn_int32 x_gain, vrpn_int32 x_offset,
                 vrpn_int32 y_gain, vrpn_int32 y_offset,
                 vrpn_int32 z_gain, vrpn_int32 z_offset);
    static vrpn_int32 decode_SetDACParams (const char **buf,
                 vrpn_int32 *x_gain, vrpn_int32 *x_offset,
                 vrpn_int32 *y_gain, vrpn_int32 *y_offset,
                 vrpn_int32 *z_gain, vrpn_int32 *z_offset);

    static char * encode_SetExternalScanControlEnable (vrpn_int32 *len,
                                             vrpn_int32 enable);
    static vrpn_int32 decode_SetExternalScanControlEnable (const char **buf,
                                            vrpn_int32 *enable);

    // (server-->client)
    static char * encode_ReportResolution (vrpn_int32 *len, 
					vrpn_int32 x, vrpn_int32 y);
    static vrpn_int32 decode_ReportResolution(const char **buf, 
                                        vrpn_int32 *x, vrpn_int32 *y);

    static char * encode_ReportPixelIntegrationTime (vrpn_int32 *len, 
                                        vrpn_int32 time_nsec);
    static vrpn_int32 decode_ReportPixelIntegrationTime (const char **buf,
                                        vrpn_int32 *time_nsec);

    static char * encode_ReportInterPixelDelayTime (vrpn_int32 *len,
                                        vrpn_int32 time_nsec);
    static vrpn_int32 decode_ReportInterPixelDelayTime (const char **buf,
                                        vrpn_int32 *time_nsec);

    static char * encode_WindowLineData (vrpn_int32 * len, 
        vrpn_int32 start_x, vrpn_int32 start_y, vrpn_int32 dx, vrpn_int32 dy,
        vrpn_int32 lineLength, vrpn_int32 numFields, vrpn_int32 *offset, 
        vrpn_int32 sec, vrpn_int32 usec, vrpn_uint16 **);
    static char * encode_WindowLineData (vrpn_int32 * len,
        vrpn_int32 start_x, vrpn_int32 start_y, vrpn_int32 dx, vrpn_int32 dy,
        vrpn_int32 lineLength, vrpn_int32 numFields, vrpn_int32 *offset,
        vrpn_int32 sec, vrpn_int32 usec, vrpn_uint8 **);
    static vrpn_int32 decode_WindowLineDataHeader (const char ** buf, 
        vrpn_int32 *start_x, vrpn_int32 *start_y, 
        vrpn_int32 *dx, vrpn_int32 *dy, vrpn_int32 *lineLength, 
        vrpn_int32 *numFields, vrpn_int32 *sec, vrpn_int32 *usec);
    static vrpn_int32 decode_WindowLineDataField (const char ** buf,
        vrpn_int32 numFields, vrpn_float32 * data);


    static char * encode_ScanlineData (vrpn_int32 * len,
        vrpn_int32 start_x, vrpn_int32 start_y, vrpn_int32 dx, vrpn_int32 dy,
        vrpn_int32 lineLength, vrpn_int32 numFields, vrpn_int32 numLines,
        vrpn_int32 sec, vrpn_int32 usec, vrpn_int32 pixelType, void **data);
    static vrpn_int32 decode_ScanlineDataHeader (const char ** buf, 
        vrpn_int32 *start_x, vrpn_int32 *start_y, 
        vrpn_int32 *dx, vrpn_int32 *dy, vrpn_int32 *lineLength, 
        vrpn_int32 *numFields, vrpn_int32 *numLines, vrpn_int32 *sec, 
        vrpn_int32 *usec, vrpn_int32 *pixelType);
    static vrpn_int32 decode_ScanlineDataLine (const char ** buf,
        vrpn_int32 lineLength, vrpn_int32 numFields, vrpn_int32 numLines,
        vrpn_int32 pixelType, void *data);

    static char * encode_ReportPointDwellTime (vrpn_int32 *len,
                                            vrpn_int32 time_nsec);
    static vrpn_int32 decode_ReportPointDwellTime (const char **buf,
                                            vrpn_int32 *time_nsec);

    static char * encode_ReportBeamBlankEnable (vrpn_int32 *len,
                                            vrpn_int32 enable);
    static vrpn_int32 decode_ReportBeamBlankEnable (const char **buf,
                                            vrpn_int32 *enable);

    static char * encode_ReportMaxScanSpan (vrpn_int32 *len,
                              vrpn_int32 x, vrpn_int32 y);
    static vrpn_int32 decode_ReportMaxScanSpan (const char **buf,
                              vrpn_int32 *x, vrpn_int32 *y);

    static char * encode_ReportBeamLocation (vrpn_int32 *len,
                              vrpn_int32 x, vrpn_int32 y);
    static vrpn_int32 decode_ReportBeamLocation (const char **buf,
                              vrpn_int32 *x, vrpn_int32 *y);

    static char * encode_ReportRetraceDelays (vrpn_int32 *len,
                           vrpn_int32 h_time_nsec, vrpn_int32 v_time_nsec);
    static vrpn_int32 decode_ReportRetraceDelays (const char **buf,
                           vrpn_int32 *h_time_nsec, vrpn_int32 *v_time_nsec);

    static char * encode_ReportDACParams (vrpn_int32 *len,
                 vrpn_int32 x_gain, vrpn_int32 x_offset,
                 vrpn_int32 y_gain, vrpn_int32 y_offset,
                 vrpn_int32 z_gain, vrpn_int32 z_offset);
    static vrpn_int32 decode_ReportDACParams (const char **buf,
                 vrpn_int32 *x_gain, vrpn_int32 *x_offset,
                 vrpn_int32 *y_gain, vrpn_int32 *y_offset,
                 vrpn_int32 *z_gain, vrpn_int32 *z_offset);

    static char * encode_ReportExternalScanControlEnable (vrpn_int32 *len,
                                            vrpn_int32 enable);
    static vrpn_int32 decode_ReportExternalScanControlEnable (const char **buf,
                                            vrpn_int32 *enable);

//    packs the message reliably and then deletes [] buf
//    int dispatchMessage (vrpn_int32 len, const char *buf, 
//                     vrpn_int32 type);
};

#endif
