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

    // functions for getting settings
    vrpn_int32 getResolution(vrpn_int32 &res_x, vrpn_int32 &res_y);
    vrpn_int32 getPixelIntegrationTime_nsec();
    vrpn_int32 getInterPixelDelayTime_nsec();
    vrpn_bool scanEnabled();

    // data acquisition
    vrpn_int32 acquireImage(void);

    // functions that send messages
    vrpn_int32 reportResolution();
    vrpn_int32 reportPixelIntegrationTime();
    vrpn_int32 reportInterPixelDelayTime();
    vrpn_int32 reportWindowLineData(int line_num);
	vrpn_int32 reportScanlineData(int line_num);

  private:
    vrpn_int32 initializeParameterDefaults(void);
    vrpn_int32 openEDAXHardware(void);
    vrpn_int32 closeEDAXHardware(void);
    vrpn_int32 openScanControlInterface(void);
    vrpn_int32 closeScanControlInterface(void);
    vrpn_int32 setHardwareConfiguration(void);
    vrpn_int32 configureScan(void);

    static int RcvSetResolution (void *_userdata, vrpn_HANDLERPARAM _p);
    static int RcvSetPixelIntegrationTime
		(void *_userdata, vrpn_HANDLERPARAM _p);
    static int RcvSetInterPixelDelayTime
		(void *_userdata, vrpn_HANDLERPARAM _p);
    static int RcvRequestScan
		(void *_userdata, vrpn_HANDLERPARAM _p);
    static int RcvGotConnection
		(void *_userdata, vrpn_HANDLERPARAM _p);
    static int RcvDroppedConnection
                (void *_userdata, vrpn_HANDLERPARAM _p);

    vrpn_bool d_settings_changed_since_last_scan;
    vrpn_int32 d_resolution_index;
    vrpn_int32 d_resolution_x;
    vrpn_int32 d_resolution_y;
    vrpn_int32 d_pix_integrate_nsec;
    vrpn_int32 d_interpixel_delay_nsec;
    vrpn_bool d_scan_enabled;
    vrpn_int32 d_lines_per_message;

    vrpn_int32 d_scans_to_do;

#ifdef _WIN32
    UCHAR *d_scanBuffer;
#else
    vrpn_uint8 *d_scanBuffer;  // UCHAR
#endif

    static int d_matrixSizeX[EDAX_NUM_SCAN_MATRICES];
    static int d_matrixSizeY[EDAX_NUM_SCAN_MATRICES];

    // EDAX hardware configuration parameters:
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
#else
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
#endif
};

#endif
