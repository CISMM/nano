
#ifndef __NMM_MICROSCOPE_SEM_DIAGINC_H
#define __NMM_MICROSCOPE_SEM_DIAGINC_H

#include <windows.h>
#include <spotCam.h>

#include <vrpn_Connection.h>
#include "nmb_Device.h"
#include "nmm_Microscope_SEM.h"


// from SpotCam.h:
// typedef VOID (WINAPI *SPOTCALLBACK)(int iStatus, long lInfo, DWORD dwUserData);
void WINAPI nmm_Microscope_SEM_diaginc_spotCallback( int iStatus, long lInfo, DWORD dwUserData );
//SPOTCALLBACK nmm_Microscope_SEM_diaginc_spotCallback;

// for compatibility with the SEM part of nano
//  the EDAX is a particular hardware controller for the Hitachi
//  SEM and has a few fixed resolutions.  We must adhere to these
//  in order to fake being an SEM.  Some of these may be invalid
//  depending on the model of SPOT camera.
const int EDAX_NUM_SCAN_MATRICES =(7);
extern int EDAX_SCAN_MATRIX_X[EDAX_NUM_SCAN_MATRICES];
extern int EDAX_SCAN_MATRIX_Y[EDAX_NUM_SCAN_MATRICES];
const int EDAX_DEFAULT_SCAN_MATRIX =(4);	// (1024 x 800)


class nmm_Microscope_SEM_diaginc : 
public nmb_Device_Server, public nmm_Microscope_SEM 
{
	
	
public:
    nmm_Microscope_SEM_diaginc( const char * name, vrpn_Connection * c,
             vrpn_bool virtualAcquisition = vrpn_FALSE );
    virtual ~nmm_Microscope_SEM_diaginc( void );

    virtual vrpn_int32 mainloop(const struct timeval *timeout = NULL);

    // functions that change settings
    vrpn_int32 setResolution(vrpn_int32 res_x, vrpn_int32 res_y);
    vrpn_int32 requestScan(vrpn_int32 nscans);

    // functions for getting settings
    vrpn_int32 getResolution(vrpn_int32 &res_x, vrpn_int32 &res_y);
	vrpn_int32 getMaxResolution( vrpn_int32& x, vrpn_int32& y );
    vrpn_bool scanEnabled();
    vrpn_int32 getScanRegion_nm(double &x_span_nm, double &y_span_nm);
    vrpn_int32 getMaxScan(int &x_span_DAC, int &y_span_DAC);

    // data acquisition
    vrpn_int32 acquireImage(void);

    // functions that send messages
    vrpn_int32 reportResolution();
    vrpn_int32 reportScanlineData(int line_num);
    vrpn_int32 reportMaxScanSpan();

    vrpn_int32 reportPixelIntegrationTime();
    vrpn_int32 reportInterPixelDelayTime();
    vrpn_int32 reportBeamBlankEnable();
    vrpn_int32 reportPointDwellTime();
    vrpn_int32 reportBeamLocation();
    vrpn_int32 reportRetraceDelays();
    vrpn_int32 reportDACParams();
    vrpn_int32 reportExternalScanControlEnable();
    vrpn_int32 reportMagnification();

  
	static int resolutionToIndex(const int res_x, const int res_y);
	static int indexToResolution(const int id, int &res_x, int &res_y);


private:

    // other subroutines:

	vrpn_int32 setupCamera( );
	
    void checkForParameterChanges(void);
    vrpn_int32 initializeParameterDefaults(void);

    static int RcvSetResolution( void *_userdata, vrpn_HANDLERPARAM _p );
    static int RcvRequestScan( void *_userdata, vrpn_HANDLERPARAM _p );
    static int RcvGotConnection( void *_userdata, vrpn_HANDLERPARAM _p );
    static int RcvDroppedConnection( void *_userdata, vrpn_HANDLERPARAM _p );

    vrpn_bool d_image_mode_settings_changed;
    vrpn_bool d_shared_settings_changed;
    vrpn_bool d_scan_enabled;
    vrpn_int32 d_lines_per_message;

    vrpn_int32 d_scans_to_do;

    vrpn_bool d_virtualAcquisition;

	int currentResolutionIndex;
	int maxBufferSize;
	vrpn_uint8* myImageBuffer;
	vrpn_uint8* cameraImageBuffer;


	friend void WINAPI nmm_Microscope_SEM_diaginc_spotCallback( int iStatus, long lInfo, DWORD dwUserData );

}; // end class nmm_Microscope_SEM_diaginc

#endif // __NMM_MICROSCOPE_SEM_DIAGINC_H