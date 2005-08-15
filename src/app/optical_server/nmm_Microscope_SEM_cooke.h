
#ifndef __NMM_MICROSCOPE_SEM_COOKE_H
#define __NMM_MICROSCOPE_SEM_COOKE_H

#include <windows.h>
#include <SC2_SDKStructures.h> // needs to be before SC2_CamExport.h
#include <SC2_CamExport.h>

#include <vrpn_Connection.h>
#include "nmb_Device.h"
#include "nmm_Microscope_SEM.h"


#include "edax_defs.h"
#include "nmm_Microscope_SEM_optical.h"

// for WaitForSingleObject
DWORD WINAPI nmm_Microscope_SEM_cooke_spotCallback(LPVOID param);

class nmm_Microscope_SEM_cooke : 
/*public nmb_Device_Server, public nmm_Microscope_SEM,*/ 
public nmm_Microscope_SEM_optical
{	
public:
    nmm_Microscope_SEM_cooke( const char * name, vrpn_Connection * c,
             vrpn_bool virtualAcquisition = vrpn_FALSE );
    virtual ~nmm_Microscope_SEM_cooke( void );

    virtual vrpn_int32 mainloop(const struct timeval *timeout = NULL);

	// functions that change settings
	vrpn_int32 setResolutionByIndex( vrpn_int32 index );
	vrpn_int32 setBinning( vrpn_int32 bin );
	vrpn_int32 setExposure( vrpn_int32 millisecs );

    // functions for getting settings
    vrpn_int32 getResolution( vrpn_int32 &res_x, vrpn_int32 &res_y );
	vrpn_int32 getResolutionIndex( ) { return currentResolutionIndex;  }
	vrpn_int32 getMaxResolution( vrpn_int32& x, vrpn_int32& y );
	vrpn_int32 getBinning( );
	vrpn_int32 getContrastLevel( ) { return currentContrast; }
	vrpn_int32 getExposure( ) {  return currentExposure;  }

    vrpn_bool scanEnabled();
    vrpn_int32 getScanRegion_nm( double &x_span_nm, double &y_span_nm );
    vrpn_int32 getMaxScan( int &x_span_DAC, int &y_span_DAC );

    // data acquisition
    vrpn_int32 acquireImage(void);
    vrpn_int32 requestScan( vrpn_int32 nscans );

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

protected:

    // other subroutines:
	vrpn_int32 setupCamera( );
    
	vrpn_int32 getResolutionFromCamera( vrpn_int32 &res_x, vrpn_int32 &res_y );
	
	void printCookeValues( );
	
    void checkForParameterChanges(void);
    vrpn_int32 initializeParameterDefaults(void);

    static int VRPN_CALLBACK RcvSetResolution( void *_userdata, vrpn_HANDLERPARAM _p );
    static int VRPN_CALLBACK RcvRequestScan( void *_userdata, vrpn_HANDLERPARAM _p );
    static int VRPN_CALLBACK RcvGotConnection( void *_userdata, vrpn_HANDLERPARAM _p );
    static int VRPN_CALLBACK RcvDroppedConnection( void *_userdata, vrpn_HANDLERPARAM _p );

    vrpn_bool d_image_mode_settings_changed;
    vrpn_bool d_shared_settings_changed;
    vrpn_bool d_scan_enabled;
    vrpn_int32 d_lines_per_message;

    vrpn_int32 d_scans_to_do;

    vrpn_bool d_virtualAcquisition;
	
	// Cooke-specific parameters
	HANDLE camera;
	int boardNumber;
	PCO_General cameraGeneral;
	PCO_CameraType cameraType;
	PCO_Sensor cameraSensor;
	PCO_Description cameraDescription;
	PCO_Timing cameraTiming;
	PCO_Storage cameraStorage;
	PCO_Recording cameraRecording;
	HANDLE cameraEvent;

	// Cooke error stuff
#define  ERROR_TEXT_LEN (256)
	char errorText[ERROR_TEXT_LEN];
	
	SHORT cameraBufferNumber;
	int maxBufferSize;
	vrpn_uint8* myImageBuffer;
	vrpn_uint8* cameraImageBuffer;

	struct CookeChanges
	{
		bool resolutionChanged;
		int newResolutionIndex;

		bool binningChanged;
		int newBinning;

		bool exposureChanged;
		int newExposure;
	};

	CookeChanges requestedChanges;
	void doRequestedChangesOnCooke( );

}; // end class nmm_Microscope_SEM_cooke

#endif // __NMM_MICROSCOPE_SEM_COOKE_H