#ifndef NMM_MICROSCOPE_SEM_REMOTE_H
#define NMM_MICROSCOPE_SEM_REMOTE_H

#include <vrpn_Connection.h>

#include <nmb_Device.h>
#include <nmb_Image.h>

#include "nmm_Microscope_SEM.h"

class nmm_Microscope_SEM_Remote;

// This is what gets passed to handlers that are registered with the
// nmm_Microscope_SEM_Remote:
typedef struct {
    struct timeval msg_time;
    nmm_Microscope_SEM::msg_t msg_type;	// tells you what data has changed
    nmm_Microscope_SEM_Remote *sem;
} nmm_Microscope_SEM_ChangeHandlerData;

typedef void (*nmm_Microscope_SEM_Remote_ChangeHandler_t)(void *userdata,
                          const nmm_Microscope_SEM_ChangeHandlerData &info);

class nmm_Microscope_SEM_Remote : public nmb_Device_Client, 
                                  public nmm_Microscope_SEM {

  public:
    nmm_Microscope_SEM_Remote(const char *name, vrpn_Connection *c = NULL);
    ~nmm_Microscope_SEM_Remote(void);

    virtual vrpn_int32 mainloop(const struct timeval * timeout = NULL);

    // message-sending functions

    // sets the resolution of images
    int setResolution (vrpn_int32 res_x, vrpn_int32 res_y);
    // for image acquisition sets the duration for integrating the signal
    // at each pixel
    int setPixelIntegrationTime (vrpn_int32 time_nsec);
    // for image acquisition, sets how frequently the beam is moved 
    // from one pixel to the next
    int setInterPixelDelayTime (vrpn_int32 time_nsec);
    // tells the server to acquire a given number of images so they don't
    // have to be requested separately; sending 0 stops acquisition after
    // the current scan is complete
    int requestScan (vrpn_int32 nscans);
    // for e-beam lithography, sets the dwell time
    int setPointDwellTime(vrpn_int32 time_nsec);
    // enables/disables beam blanking between points
    int setBeamBlankEnable(vrpn_int32 enable);
    // put beam at a particular point for the set dwell time
    int goToPoint(vrpn_int32 x, vrpn_int32 y);
    // set retrace delays for image scanning
    int setRetraceDelays(vrpn_int32 h_time_nsec, vrpn_int32 v_time_nsec);
    // set gain and offset for scan voltages and the input channel
    int setDACParams(vrpn_int32 x_gain, vrpn_int32 x_offset,
                     vrpn_int32 y_gain, vrpn_int32 y_offset,
                     vrpn_int32 z_gain, vrpn_int32 z_offset);
    // enable/disable our control of the beam location - should disable
    // when not in use so other scan hardware can be used if needed
    int setExternalScanControlEnable(vrpn_int32 enable);

    // clear the exposure pattern
    int clearExposePattern();
    // add a polygon shape to the pattern
    int addPolygon(vrpn_float32 exposure_uCoul_per_cm2, vrpn_int32 numPoints,
                   vrpn_float32 *x_nm, vrpn_float32 *y_nm);
    // add a polyline shape to the pattern
    int addPolyline(vrpn_float32 exposure_pCoul_per_cm,
                    vrpn_float32 exposure_uCoul_per_cm2,
                    vrpn_float32 lineWidth_nm, vrpn_int32 numPoints,
                    vrpn_float32 *x_nm, vrpn_float32 *y_nm);
    // set a point to which the beam can go when the program must pause
    // exposure (as when it needs to do some calculations)
    int addDumpPoint(vrpn_float32 x_nm, vrpn_float32 y_nm);
    // draw the pattern with the electron beam
    int exposePattern();
    // do a timing test for the pattern
    int exposureTimingTest();
    // set the beam current
    int setBeamCurrent(vrpn_float32 current_picoAmps);
    // set the beam width
    int setBeamWidth(vrpn_float32 width_nm);
    // set point reporting enable/disable
    int setPointReportEnable(vrpn_int32 enable);
    // set dot spacing
    int setDotSpacing(vrpn_float32 spacing_nm);
    // set line spacing
    int setLineSpacing(vrpn_float32 spacing_nm);
    // set linear exposure
    int setLinearExposure(vrpn_float32 exposure_pCoul_per_cm);
    // set area exposure
    int setAreaExposure(vrpn_float32 exposure_uCoul_per_sq_cm);
    // set magnification
    int setMagnification(vrpn_float32 mag);

    // message callback registration
    int registerChangeHandler(void *userdata,
	nmm_Microscope_SEM_Remote_ChangeHandler_t handler);
    int unregisterChangeHandler(void *userdata,
        nmm_Microscope_SEM_Remote_ChangeHandler_t handler);

    // data access functions intended to be called in message callback
    void getResolution(vrpn_int32 &res_x, vrpn_int32 &res_y);
    void getPixelIntegrationTime(vrpn_int32 &time_nsec);
    void getInterPixelDelayTime(vrpn_int32 &time_nsec);
    void getScanlineData(vrpn_int32 &start_x, vrpn_int32 &start_y,
		vrpn_int32 &dx, vrpn_int32 &dy, vrpn_int32 &line_length,
		vrpn_int32 &num_fields, vrpn_int32 &num_lines, 
                nmb_PixelType &pix_type,
                void **data);
    void getPointDwellTime(vrpn_int32 &time_nsec);
    void getBeamBlankEnabled(vrpn_int32 &enabled);
    void getMaxScan(vrpn_int32 &x, vrpn_int32 &y);
    void getBeamLocation(vrpn_int32 &x, vrpn_int32 &y);
    void getRetraceDelays(vrpn_int32 &h_time_nsec, vrpn_int32 &v_time_nsec);
    void getDACParams(vrpn_int32 &x_gain, vrpn_int32 &x_offset,
                      vrpn_int32 &y_gain, vrpn_int32 &y_offset,
                      vrpn_int32 &z_gain, vrpn_int32 &z_offset);
    void getExternalScanControlEnable(vrpn_int32 &enabled);
    void getMagnification(vrpn_float32 &mag);
    void getScanRegion_nm(double &width_nm, double &height_nm);
    void getExposureStatus(vrpn_int32 &numPointsTotal, 
       vrpn_int32 &numPointsDone, vrpn_float32 &timeTotal_sec, 
       vrpn_float32 &timeDone_sec);

    void convert_nm_to_DAC(const double x_nm, const double y_nm,
                           int &xDAC, int &yDAC);
    void convert_DAC_to_nm(const int xDAC, const int yDAC,
                           double &x_nm, double &y_nm);


    // other
    int connected() {return d_connection->connected();}

  protected:
    static int RcvReportResolution (void *_userdata, vrpn_HANDLERPARAM _p);
    static int RcvReportPixelIntegrationTime
                (void *_userdata, vrpn_HANDLERPARAM _p);
    static int RcvReportInterPixelDelayTime
                (void *_userdata, vrpn_HANDLERPARAM _p);
    static int RcvScanlineData
		(void *_userdata, vrpn_HANDLERPARAM _p);
    static int RcvReportPointDwellTime(void *_userdata, vrpn_HANDLERPARAM _p);
    static int RcvReportBeamBlankEnable(void *_userdata, vrpn_HANDLERPARAM _p);
    static int RcvReportMaxScanSpan
                (void *_userdata, vrpn_HANDLERPARAM _p);
    static int RcvReportBeamLocation(void *_userdata, vrpn_HANDLERPARAM _p);
    static int RcvReportRetraceDelays(void *_userdata, vrpn_HANDLERPARAM _p);
    static int RcvReportDACParams(void *_userdata, vrpn_HANDLERPARAM _p);
    static int RcvReportExternalScanControlEnable(void *_userdata,
                                                  vrpn_HANDLERPARAM _p);
    static int RcvReportMagnification(void *_userdata, vrpn_HANDLERPARAM _p);
    static int RcvReportExposureStatus(void *_userdata, vrpn_HANDLERPARAM _p);

    int notifyMessageHandlers(nmm_Microscope_SEM::msg_t type,
        const struct timeval &msg_time);

    /* ---------------------------------------------------------------
       data from messages:
       -------------------
    */
    vrpn_int32 d_resolutionX, d_resolutionY;
    vrpn_int32 d_pixelIntegrationTime_nsec;
    vrpn_int32 d_interpixelDelayTime_nsec;
    // window line data
    vrpn_int32 d_startX, d_startY, d_dX, d_dY;
    vrpn_int32 d_lineLength;
    vrpn_int32 d_numFields;
    vrpn_int32 d_numLines;
    nmb_PixelType d_pixelType;
    char *d_dataBuffer;
    int d_dataBufferSize;
    vrpn_int32 d_pointDwellTime_nsec;
    vrpn_int32 d_beamBlankEnabled;
    vrpn_int32 d_maxScanX, d_maxScanY;
    vrpn_int32 d_pointScanX, d_pointScanY;
    vrpn_int32 d_hRetraceDelay_nsec, d_vRetraceDelay_nsec;
    vrpn_int32 d_xGain, d_xOffset, d_yGain, d_yOffset, d_zGain, d_zOffset;
    vrpn_int32 d_externalScanControlEnabled;
    vrpn_float32 d_magnification; // [(width of "display" in nm)/
                                  //  (width of scan in nm)]
    vrpn_float32 d_magCalibration; // [width of "display" in nm];default = 10cm
    vrpn_int32 d_numPointsTotal;
    vrpn_int32 d_numPointsDone;
    vrpn_float32 d_timeTotal_sec;
    vrpn_float32 d_timeDone_sec;

    /* ---------------------------------------------------------------
       message callback management
       ---------------------------
       each time we get a message in one of the connection callbacks (Rcv*),
       we call all the callbacks in this list
    */
    typedef struct _msg_handler {
        void                                        *userdata;
        nmm_Microscope_SEM_Remote_ChangeHandler_t   handler;
        struct _msg_handler                         *next;
    } msg_handler_list_t;

    msg_handler_list_t *d_messageHandlerList;

};

#endif
