#ifndef NMM_MICROSCOPE_SEM_EDAX_H
#define NMM_MICROSCOPE_SEM_EDAX_H

#include <vrpn_Connection.h>
#include "nmb_Device.h"
#include "nmm_Microscope_SEM.h"
#include "nmm_EDAX.h"  // a bunch of EDAX header files and definitions

class nmm_Microscope_SEM_EDAX : 
    public nmb_Device_Server, public nmm_Microscope_SEM {

  public:
    nmm_Microscope_SEM_EDAX (const char * name, vrpn_Connection * c);
    virtual ~nmm_Microscope_SEM_EDAX (void);

    virtual vrpn_int32 mainloop(void);

    // functions that change settings
    vrpn_int32 setResolution(vrpn_int32 res_x, vrpn_int32 res_y);
    vrpn_int32 setPixelIntegrationTime(vrpn_int32 time_nsec);
    vrpn_int32 setInterPixelDelayTime(vrpn_int32 time_nsec);
    vrpn_int32 requestScan(vrpn_int32 nscans);
    vrpn_int32 setPointDwellTime(vrpn_int32 time_nsec);
    vrpn_int32 setBeamBlankEnable(vrpn_int32 enable);
    vrpn_int32 goToPoint(vrpn_int32 xDAC, vrpn_int32 yDAC);
    vrpn_int32 setRetraceDelays(vrpn_int32 htime_usec, vrpn_int32 vtime_usec);
    vrpn_int32 setDACParams(vrpn_int32 xGain, vrpn_int32 xOffset,
                            vrpn_int32 yGain, vrpn_int32 yOffset,
                            vrpn_int32 zGain, vrpn_int32 zOffset);
    vrpn_int32 setExternalScanControlEnable(vrpn_int32 enable);

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

  private:
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
#endif
};

#endif
