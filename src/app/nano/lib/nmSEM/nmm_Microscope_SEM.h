#ifndef NMM_MICROSCOPE_SEM_H
#define NMM_MICROSCOPE_SEM_H

#include <math.h>
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

// by convention for an SEM, magnification refers to the scale when the image
// is displayed on a 10 centimeter wide display - this is just that width in nm
#define SEM_STANDARD_DISPLAY_WIDTH_NM (1e8) // (10 cm)*(nm/cm)
#define SEM_INV_STANDARD_DISPLAY_WIDTH_NM (1e-8)

class nmm_Microscope_SEM {
  public:
    nmm_Microscope_SEM (const char * name,
			vrpn_Connection *c = NULL);

    virtual ~nmm_Microscope_SEM (void);
    
    virtual vrpn_int32 mainloop(const struct timeval *timeout) = 0;

  typedef enum {SET_RESOLUTION,
                SET_PIXEL_INTEGRATION_TIME,
                SET_INTERPIXEL_DELAY_TIME,
                ENABLE_SCAN,
                ENABLE_POINT_REPORTING,
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
                EXTERNAL_SCAN_CONTROL_ENABLE,
                REPORT_MAGNIFICATION,
                CLEAR_EXPOSE_PATTERN,
                SET_LINE_EXPOSURE,
                SET_AREA_EXPOSURE,
                ADD_POLYLINE,
                ADD_POLYGON,
                ADD_DUMP_POINT,
                EXPOSE_PATTERN,
                EXPOSURE_TIMING_TEST,
                BEAM_CURRENT,
                BEAM_WIDTH,
                REPORT_EXPOSURE_STATUS,
		REPORT_TIMING_STATUS,
		POINT_REPORT_ENABLE,
		DOT_SPACING,
		LINE_SPACING
                } msg_t;
  protected:
//    vrpn_Connection * d_connection;
//    vrpn_File_Controller * d_fileController;

//    vrpn_int32 d_myId;

    // (client-->server) messages
	// parameters that can be set by the client
    vrpn_int32 d_SetResolution_type;
    vrpn_int32 d_SetPixelIntegrationTime_type;
    vrpn_int32 d_SetInterPixelDelayTime_type;
	vrpn_int32 d_SetPointDwellTime_type;
    vrpn_int32 d_SetBeamBlankEnable_type;
    vrpn_int32 d_SetRetraceDelays_type;
    vrpn_int32 d_SetDACParams_type;
    vrpn_int32 d_SetExternalScanControlEnable_type;
    vrpn_int32 d_SetBeamCurrent_type;
    vrpn_int32 d_SetBeamWidth_type;
    vrpn_int32 d_SetPointReportEnable_type;
    vrpn_int32 d_SetDotSpacing_type;
    vrpn_int32 d_SetLineSpacing_type;
    vrpn_int32 d_SetLinearExposure_type;
    vrpn_int32 d_SetAreaExposure_type;
    vrpn_int32 d_SetMagnification_type;

	// scan/data requests
    vrpn_int32 d_RequestScan_type;
    vrpn_int32 d_GoToPoint_type;

	// exposure or exposure calculation requests
    vrpn_int32 d_ClearExposePattern_type;
    vrpn_int32 d_AddPolygon_type;
    vrpn_int32 d_AddPolyline_type;
    vrpn_int32 d_AddDumpPoint_type;
    vrpn_int32 d_ExposePattern_type;
    vrpn_int32 d_ExposureTimingTest_type;

    // (server-->client) messages
    // parameters that can be set by the client
    vrpn_int32 d_ReportResolution_type;
    vrpn_int32 d_ReportPixelIntegrationTime_type;
    vrpn_int32 d_ReportInterPixelDelayTime_type;
    vrpn_int32 d_ReportPointDwellTime_type;
    vrpn_int32 d_ReportBeamBlankEnable_type;
    vrpn_int32 d_ReportRetraceDelays_type;
    vrpn_int32 d_ReportDACParams_type;
    vrpn_int32 d_ReportExternalScanControlEnable_type;
    vrpn_int32 d_ReportBeamCurrent_type;
    vrpn_int32 d_ReportBeamWidth_type;

	vrpn_int32 d_ReportPointReportEnable_type;
	vrpn_int32 d_ReportDotSpacing_type;
    vrpn_int32 d_ReportLineSpacing_type;

    vrpn_int32 d_ReportMagnification_type;

	// responses to scan/data/exposure requests
    vrpn_int32 d_ScanlineData_type;
    vrpn_int32 d_ReportBeamLocation_type;
    vrpn_int32 d_ReportExposureStatus_type;
	vrpn_int32 d_ReportTimingStatus_type;

	// parameters not settable by the client
    vrpn_int32 d_ReportMaxScanSpan_type;

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

    // messages for pattern exposure

    static char * encode_AddPolygon (vrpn_int32 *len,
         vrpn_float32 exposure_uCoul_per_cm2, 
         vrpn_int32 numPoints,
         vrpn_float32 *x_nm, vrpn_float32 *y_nm);

    static vrpn_int32 decode_AddPolygonHeader (const char **buf,
         vrpn_float32 *exposure_uCoul_per_cm2,
         vrpn_int32 *numPoints);
    static vrpn_int32 decode_AddPolygonData (const char **buf,
         vrpn_int32 numPoints, 
         vrpn_float32 *x_nm, vrpn_float32 *y_nm);
     
    static char * encode_AddPolyline (vrpn_int32 *len,
         vrpn_float32 exposure_pCoul_per_cm,
         vrpn_float32 exposure_uCoul_per_cm2,
         vrpn_float32 lineWidth_nm,
         vrpn_int32 numPoints,
         vrpn_float32 *x_nm, vrpn_float32 *y_nm);

    static vrpn_int32 decode_AddPolylineHeader (const char **buf,
         vrpn_float32 *exposure_pCoul_per_cm,
         vrpn_float32 *exposure_uCoul_per_cm2,
         vrpn_float32 *lineWidth_nm,
         vrpn_int32 *numPoints);
    static vrpn_int32 decode_AddPolylineData (const char **buf,
         vrpn_int32 numPoints,
         vrpn_float32 *x_nm, vrpn_float32 *y_nm);

    static char * encode_AddDumpPoint (vrpn_int32 *len,
         vrpn_float32 x_nm, vrpn_float32 y_nm);

    static vrpn_int32 decode_AddDumpPoint (const char **buf,
         vrpn_float32 *x_nm, vrpn_float32 *y_nm);


    static char * encode_SetBeamCurrent (vrpn_int32 *len,
                                            vrpn_float32 current_picoAmps);
    static vrpn_int32 decode_SetBeamCurrent (const char **buf,
                                            vrpn_float32 *current_picoAmps);

    static char * encode_SetBeamWidth (vrpn_int32 *len,
                                            vrpn_float32 beamWidth_nm);
    static vrpn_int32 decode_SetBeamWidth (const char **buf,
                                            vrpn_float32 *beamWidth_nm);

    static char * encode_SetPointReportEnable (vrpn_int32 *len,
                                               vrpn_int32 enable);
    static vrpn_int32 decode_SetPointReportEnable (const char **buf,
                                                vrpn_int32 *enable);

    static char * encode_SetDotSpacing (vrpn_int32 *len,
                           vrpn_float32 dotSpacing_nm);
    static vrpn_int32 decode_SetDotSpacing (const char **buf, 
                           vrpn_float32 *dotSpacing_nm);

    static char * encode_SetLineSpacing (vrpn_int32 *len,
                           vrpn_float32 lineSpacing_nm);
    static vrpn_int32 decode_SetLineSpacing (const char **buf,
                           vrpn_float32 *lineSpacing_nm);

    static char * encode_SetLinearExposure (vrpn_int32 *len,
                           vrpn_float32 exposure_pCoul_per_cm);
    static vrpn_int32 decode_SetLinearExposure (const char **buf,
                           vrpn_float32 *exposure_pCoul_per_cm);

    static char * encode_SetAreaExposure (vrpn_int32 *len,
                           vrpn_float32 exposure_uCoul_per_sq_cm);
    static vrpn_int32 decode_SetAreaExposure (const char **buf,
                           vrpn_float32 *exposure_uCoul_per_sq_cm);

    static char * encode_SetMagnification (vrpn_int32 *len,
                                              vrpn_float32 mag);
    static vrpn_int32 decode_SetMagnification (const char **buf,
                                              vrpn_float32 *mag);

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

    static char * encode_ReportMagnification (vrpn_int32 *len, 
                                              vrpn_float32 mag);
    static vrpn_int32 decode_ReportMagnification (const char **buf,
                                              vrpn_float32 *mag);

    static char * encode_ReportExposureStatus (vrpn_int32 *len,
                                               vrpn_int32 numPointsTotal,
                                               vrpn_int32 numPointsDone,
                                               vrpn_float32 timeTotal_sec,
                                               vrpn_float32 timeDone_sec);
    static vrpn_int32 decode_ReportExposureStatus (const char **buf,
                                               vrpn_int32 *numPointsTotal,
                                               vrpn_int32 *numPointsDone,
                                               vrpn_float32 *timeTotal_sec,
                                               vrpn_float32 *timeDone_sec);

    static char * encode_ReportTimingStatus (vrpn_int32 *len,
                                               vrpn_int32 numPointsTotal,
                                               vrpn_int32 numPointsDone,
                                               vrpn_float32 timeTotal_sec,
                                               vrpn_float32 timeDone_sec);
    static vrpn_int32 decode_ReportTimingStatus (const char **buf,
                                               vrpn_int32 *numPointsTotal,
                                               vrpn_int32 *numPointsDone,
                                               vrpn_float32 *timeTotal_sec,
                                               vrpn_float32 *timeDone_sec);


    static char * encode_ReportPointReportEnable (vrpn_int32 *len,
                                               vrpn_int32 enable);
    static vrpn_int32 decode_ReportPointReportEnable (const char **buf,
                                                vrpn_int32 *enable);

    static char * encode_ReportDotSpacing (vrpn_int32 *len,
                           vrpn_float32 dotSpacing_nm);
    static vrpn_int32 decode_ReportDotSpacing (const char **buf, 
                           vrpn_float32 *dotSpacing_nm);

    static char * encode_ReportLineSpacing (vrpn_int32 *len,
                           vrpn_float32 lineSpacing_nm);
    static vrpn_int32 decode_ReportLineSpacing (const char **buf,
                           vrpn_float32 *lineSpacing_nm);

    static void convert_nm_to_DAC(const double mag,
                                const int res_x, const int res_y,
                                const int xDACspan, const int yDACspan,
                                const double x_nm, const double y_nm,
                                int &x_DAC, int &y_DAC)
    {
      x_DAC = (int)floor(x_nm*(double)xDACspan*mag*
                         SEM_INV_STANDARD_DISPLAY_WIDTH_NM);
      y_DAC = yDACspan - 1 -
              (int)floor(y_nm*(double)yDACspan*(double)res_x*mag*
                         SEM_INV_STANDARD_DISPLAY_WIDTH_NM/
                         (double)res_y);
    }
    static void convert_DAC_to_nm(const double mag,
                                const int res_x, const int res_y,
                                const int xDACspan, const int yDACspan,
                                const int x_DAC, const int y_DAC,
                                double &x_nm, double &y_nm)
    {
      x_nm = (double)x_DAC*SEM_STANDARD_DISPLAY_WIDTH_NM/
         ((double)xDACspan*mag);
      y_nm = (double)(yDACspan - y_DAC - 1)*
         (double)res_y*SEM_STANDARD_DISPLAY_WIDTH_NM/
         ((double)yDACspan*(double)res_x*mag);
    }

//    packs the message reliably and then deletes [] buf
//    int dispatchMessage (vrpn_int32 len, const char *buf, 
//                     vrpn_int32 type);
};

#endif
