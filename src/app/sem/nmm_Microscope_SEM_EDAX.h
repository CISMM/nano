#ifndef NMM_MICROSCOPE_SEM_EDAX_H
#define NMM_MICROSCOPE_SEM_EDAX_H

#include <vrpn_Connection.h>
#include "nmb_Device.h"
#include "nmm_Microscope_SEM.h"
#include "nmm_EDAX.h"  // a bunch of EDAX header files and definitions
#include "patternShape.h" // this file as well as others in ebeamWriter
#include "exposurePattern.h"  // needs to be put into a library

class nmm_Microscope_SEM_EDAX : 
    public nmb_Device_Server, public nmm_Microscope_SEM {

  public:
    nmm_Microscope_SEM_EDAX (const char * name, vrpn_Connection * c,
             vrpn_bool virtualAcquisition = vrpn_FALSE);
    virtual ~nmm_Microscope_SEM_EDAX (void);

    virtual vrpn_int32 mainloop(const struct timeval *timeout = NULL);

    // functions that change settings
    vrpn_int32 setResolution(vrpn_int32 res_x, vrpn_int32 res_y);
    vrpn_int32 setPixelIntegrationTime(vrpn_int32 time_nsec);
    vrpn_int32 setInterPixelDelayTime(vrpn_int32 time_nsec);
    vrpn_int32 requestScan(vrpn_int32 nscans);
    vrpn_int32 setPointDwellTime(vrpn_int32 time_nsec,
                         vrpn_bool report=vrpn_TRUE);
    vrpn_int32 setBeamBlankEnable(vrpn_int32 enable);
    vrpn_int32 goToPoint(vrpn_int32 xDAC, vrpn_int32 yDAC, 
                         ShapeType type=PS_DEFAULT);
    vrpn_int32 setRetraceDelays(vrpn_int32 htime_usec, vrpn_int32 vtime_usec);
    vrpn_int32 setDACParams(vrpn_int32 xGain, vrpn_int32 xOffset,
                            vrpn_int32 yGain, vrpn_int32 yOffset,
                            vrpn_int32 zGain, vrpn_int32 zOffset);
    vrpn_int32 setExternalScanControlEnable(vrpn_int32 enable);

    vrpn_int32 clearExposePattern();
    vrpn_int32 addPolygon(vrpn_float32 exposure_uCoul_per_cm2, 
                   vrpn_int32 numPoints,
                   vrpn_float32 *x_nm, vrpn_float32 *y_nm);
    vrpn_int32 addPolyline(
                    vrpn_float32 exposure_pCoul_per_cm,
                    vrpn_float32 exposure_uCoul_per_cm2,
                    vrpn_float32 lineWidth_nm, vrpn_int32 numPoints,
                    vrpn_float32 *x_nm, vrpn_float32 *y_nm);
    vrpn_int32 addDumpPoint(vrpn_float32 x_nm, vrpn_float32 y_nm);
    vrpn_int32 exposePattern();
    vrpn_int32 exposureTimingTest();
    vrpn_int32 setBeamCurrent(vrpn_float32 current_picoAmps);
    vrpn_int32 setBeamWidth(vrpn_float32 width_nm);
    vrpn_int32 setPointReportEnable(vrpn_int32 enable);
    vrpn_int32 setDotSpacing(vrpn_float32 spacing_nm);
    vrpn_int32 setLineSpacing(vrpn_float32 spacing_nm);
    vrpn_int32 setLinearExposure(vrpn_float32 exposure);
    vrpn_int32 setAreaExposure(vrpn_float32 exposure);
    vrpn_int32 setMagnification(vrpn_float32 mag);

    // functions for getting settings
    vrpn_int32 getResolution(vrpn_int32 &res_x, vrpn_int32 &res_y);
    vrpn_int32 getPixelIntegrationTime_nsec();
    vrpn_int32 getInterPixelDelayTime_nsec();
    vrpn_bool scanEnabled();
    vrpn_int32 getPointDwellTime_nsec();
    vrpn_bool beamBlankEnabled();
    vrpn_int32 getBeamLocation(vrpn_int32 &x, vrpn_int32 &y);
    vrpn_int32 getRetraceDelays(vrpn_int32 &htime_usec, vrpn_int32 &vtime_usec);
    vrpn_int32 getDACParams(vrpn_int32 &xGain, vrpn_int32 &xOffset,
                            vrpn_int32 &yGain, vrpn_int32 &yOffset,
                            vrpn_int32 &zGain, vrpn_int32 &zOffset);
    vrpn_int32 getExternalScanControlEnable(vrpn_int32 &enable);
    vrpn_int32 getMagnification(vrpn_float32 &mag);
    vrpn_int32 getScanRegion_nm(double &x_span_nm, double &y_span_nm);
    vrpn_int32 getMaxScan(int &x_span_DAC, int &y_span_DAC);

    void convert_nm_to_DAC(const double x_nm, const double y_nm, 
                           int &xDAC, int &yDAC);
    void convert_DAC_to_nm(const int xDAC, const int yDAC,
                           double &x_nm, double &y_nm);

    // data acquisition
    vrpn_int32 acquireImage(void);

    // functions that send messages
    vrpn_int32 reportResolution();
    vrpn_int32 reportPixelIntegrationTime();
    vrpn_int32 reportInterPixelDelayTime();
    vrpn_int32 reportScanlineData(int line_num);
    vrpn_int32 reportPointDwellTime();
    vrpn_int32 reportBeamBlankEnable();
    vrpn_int32 reportMaxScanSpan();
    vrpn_int32 reportBeamLocation();
    vrpn_int32 reportRetraceDelays();
    vrpn_int32 reportDACParams();
    vrpn_int32 reportExternalScanControlEnable();
    vrpn_int32 reportMagnification();
    vrpn_int32 reportExposureStatus(
            vrpn_int32 numPointsTotal, vrpn_int32 numPointsDone,
            vrpn_float32 timeTotal_sec, vrpn_float32 timeDone_sec);

  private:
    // subroutines used for timing test
    
    // initialize point and time counters and put into test mode
    void startTimingTest();
    // compute overhead per point and put into normal mode
    void endTimingTest();
    // switch which counters we are accumulating into
    void switchToShapeType(ShapeType type);

    // other subroutines:

    void checkForParameterChanges(void);
    vrpn_int32 initializeParameterDefaults(void);
    vrpn_int32 openEDAXHardware(void);
    vrpn_int32 closeEDAXHardware(void);
    vrpn_int32 openScanControlInterface(void);
    vrpn_int32 closeScanControlInterface(void);
    vrpn_int32 configureSharedSettings(void);
    vrpn_int32 configureForImageMode(void);
    vrpn_int32 configureForPointMode(void);

    static int RcvSetResolution (void *_userdata, vrpn_HANDLERPARAM _p);
    static int RcvSetPixelIntegrationTime
		(void *_userdata, vrpn_HANDLERPARAM _p);
    static int RcvSetInterPixelDelayTime
		(void *_userdata, vrpn_HANDLERPARAM _p);
    static int RcvRequestScan
		(void *_userdata, vrpn_HANDLERPARAM _p);
    static int RcvSetPointDwellTime
                (void *_userdata, vrpn_HANDLERPARAM _p);
    static int RcvSetBeamBlankEnable
                (void *_userdata, vrpn_HANDLERPARAM _p);
    static int RcvGoToPoint
                (void *_userdata, vrpn_HANDLERPARAM _p);
    static int RcvSetRetraceDelays
                (void *_userdata, vrpn_HANDLERPARAM _p);
    static int RcvSetDACParams
                (void *_userdata, vrpn_HANDLERPARAM _p);
    static int RcvSetExternalScanControlEnable
                (void *_userdata, vrpn_HANDLERPARAM _p);
    static int RcvClearExposePattern
                (void *_userdata, vrpn_HANDLERPARAM _p);
    static int RcvAddPolygon
                (void *_userdata, vrpn_HANDLERPARAM _p);
    static int RcvAddPolyline
                (void *_userdata, vrpn_HANDLERPARAM _p);
    static int RcvAddDumpPoint
                (void *_userdata, vrpn_HANDLERPARAM _p);
    static int RcvExposePattern
                (void *_userdata, vrpn_HANDLERPARAM _p);
    static int RcvExposureTimingTest
                (void *_userdata, vrpn_HANDLERPARAM _p);
    static int RcvSetBeamCurrent
                (void *_userdata, vrpn_HANDLERPARAM _p);
    static int RcvSetBeamWidth
                (void *_userdata, vrpn_HANDLERPARAM _p);
    static int RcvSetPointReportEnable
                (void *_userdata, vrpn_HANDLERPARAM _p);
    static int RcvSetDotSpacing
                (void *_userdata, vrpn_HANDLERPARAM _p);
    static int RcvSetLineSpacing
                (void *_userdata, vrpn_HANDLERPARAM _p);
    static int RcvSetLinearExposure
                (void *_userdata, vrpn_HANDLERPARAM _p);
    static int RcvSetAreaExposure
                (void *_userdata, vrpn_HANDLERPARAM _p);
    static int RcvSetMagnification
                (void *_userdata, vrpn_HANDLERPARAM _p);
    static int RcvGotConnection
		(void *_userdata, vrpn_HANDLERPARAM _p);
    static int RcvDroppedConnection
                (void *_userdata, vrpn_HANDLERPARAM _p);

    vrpn_bool d_image_mode_settings_changed;
    vrpn_bool d_point_mode_settings_changed;
    vrpn_bool d_shared_settings_changed;
    vrpn_int32 d_resolution_index;
    vrpn_int32 d_resolution_x;
    vrpn_int32 d_resolution_y;
    vrpn_int32 d_pix_integrate_nsec;
    vrpn_int32 d_interpixel_delay_nsec;
    vrpn_bool d_scan_enabled;
    vrpn_int32 d_lines_per_message;
    vrpn_int32 d_point_dwell_time_nsec;
    vrpn_int32 d_beam_location_x;
    vrpn_int32 d_beam_location_y;
    vrpn_bool d_external_scan_control_enabled;

    vrpn_int32 d_scans_to_do;

    vrpn_bool d_virtualAcquisition;

#ifdef _WIN32
    UCHAR *d_scanBuffer;
#else
    vrpn_uint8 *d_scanBuffer;  // UCHAR
#endif

// EDAX hardware configuration parameters
// (parameters that get passed directly to EDAX API (if in windows)):
#ifdef _WIN32
    SHORT d_gainParams[3];
    SHORT d_offsetParams[3];
    LONG d_horzRetrace_usec;
    LONG d_vertRetrace_usec;
    LONG d_xScanDir;
    LONG d_yScanDir;
    LONG d_videoPolarity[EDAX_NUM_INPUT_CHANNELS];
    LONG d_xScanSpan;
    LONG d_yScanSpan;
    LONG d_blankMode;
    LONG d_scanType;
    LONG d_dataTransfer;
    long d_magnification;
#else
// for the platform independent virtual SEM server
    vrpn_int16 d_gainParams[3]; // SHORT
    vrpn_int16 d_offsetParams[3]; // SHORT
    // LONG->vrpn_int32:
    vrpn_int32 d_horzRetrace_usec; 
    vrpn_int32 d_vertRetrace_usec;
    vrpn_int32 d_xScanDir;
    vrpn_int32 d_yScanDir;	
    vrpn_int32 d_videoPolarity[EDAX_NUM_INPUT_CHANNELS];
    vrpn_int32 d_xScanSpan;
    vrpn_int32 d_yScanSpan;
    vrpn_int32 d_blankMode;
    vrpn_int32 d_scanType;
    vrpn_int32 d_dataTransfer;
    vrpn_int32 d_magnification;
#endif

    vrpn_float32 d_beamCurrent_picoAmps;
    vrpn_float32 d_beamWidth_nm;
    vrpn_int32 d_pointReportEnable;
    vrpn_float32 d_dotSpacing_nm;
    vrpn_float32 d_lineSpacing_nm;
    vrpn_float32 d_linearExposure_pCoul_per_cm;
    vrpn_float32 d_areaExposure_uCoul_per_sq_cm;

    // variables used to calculate overhead time required to generate each
    // goToPoint() command
/*
        During the timing test, the average time to compute and move the beam to
        any given location is computed per shape type in the pattern since
        different shapes will typically have different algorithms for
        generating dwell points. During exposure, these overhead delays are 
        subtracted from the dwell time and an additional delay is inserted to
        make up the difference.
*/
    int d_pointCounter[PS_NUMSHAPETYPES];
    double d_pointCommandOverheadTotal_msec[PS_NUMSHAPETYPES];
    double d_pointCommandOverheadPerPoint_msec[PS_NUMSHAPETYPES];
    ShapeType d_currShapeType;
    struct timeval d_shapeTypeStartTime;
    vrpn_bool d_timingTestFirstPoint;
    vrpn_bool d_timingTestActive;
    
    // for status reporting
    int d_totalPatternNumPoints;
    double d_totalPatternDwellTime_sec;
    int d_completedPatternNumPoints;
    double d_completedPatternDwellTime_sec;

    ExposurePattern d_pattern;
};

#endif
