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
				SCANLINE_DATA} msg_t;
  protected:
//    vrpn_Connection * d_connection;
//    vrpn_File_Controller * d_fileController;

//    vrpn_int32 d_myId;

    // (client-->server) messages
    vrpn_int32 d_SetResolution_type;
    vrpn_int32 d_SetPixelIntegrationTime_type;
    vrpn_int32 d_SetInterPixelDelayTime_type;
    vrpn_int32 d_RequestScan_type;

    // (server-->client) messages

    vrpn_int32 d_ReportResolution_type;
    vrpn_int32 d_ReportPixelIntegrationTime_type;
    vrpn_int32 d_ReportInterPixelDelayTime_type;
    vrpn_int32 d_WindowLineData_type;
    vrpn_int32 d_ScanlineData_type;

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
        vrpn_int32 sec, vrpn_int32 usec, vrpn_uint8 **data);
    static vrpn_int32 decode_ScanlineDataHeader (const char ** buf, 
        vrpn_int32 *start_x, vrpn_int32 *start_y, 
        vrpn_int32 *dx, vrpn_int32 *dy, vrpn_int32 *lineLength, 
        vrpn_int32 *numFields, vrpn_int32 *numLines, vrpn_int32 *sec, 
        vrpn_int32 *usec);
    static vrpn_int32 decode_ScanlineDataLine (const char ** buf,
        vrpn_int32 lineLength, vrpn_int32 numFields, vrpn_int32 numLines,
        vrpn_uint8 *data);

    // packs the message reliably and then deletes [] buf
//    int dispatchMessage (vrpn_int32 len, const char *buf, 
//                     vrpn_int32 type);
};

#endif
