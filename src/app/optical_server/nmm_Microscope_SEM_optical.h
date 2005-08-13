

#ifndef __NMM_MICROSCOPE_SEM_OPTICAL_H
#define __NMM_MICROSCOPE_SEM_OPTICAL_H

#include "nmb_Device.h"
#include "nmm_Microscope_SEM.h"

class nmm_Microscope_SEM_optical: public nmb_Device_Server, public nmm_Microscope_SEM
{
public:
	nmm_Microscope_SEM_optical( const char * name, vrpn_Connection * c, vrpn_bool virtualAcq = vrpn_FALSE );
	virtual ~nmm_Microscope_SEM_optical( );

    virtual vrpn_int32 mainloop(const struct timeval *timeout = NULL) = 0;
    
	// functions that change settings
    vrpn_int32 setResolution( vrpn_int32 res_x, vrpn_int32 res_y );
	virtual vrpn_int32 setResolutionByIndex( vrpn_int32 index ) = 0;
	virtual vrpn_int32 setBinning( vrpn_int32 bin ) = 0;
	vrpn_int32 setContrastLevel( vrpn_int32 level );
	virtual vrpn_int32 setExposure( vrpn_int32 millisecs ) = 0;

    // functions for getting settings
    virtual vrpn_int32 getResolution( vrpn_int32 &res_x, vrpn_int32 &res_y ) = 0;
	vrpn_int32 getResolutionIndex( ) { return currentResolutionIndex;  }
	virtual vrpn_int32 getMaxResolution( vrpn_int32& x, vrpn_int32& y ) = 0;
	virtual vrpn_int32 getBinning( ) = 0;
	vrpn_int32 getContrastLevel( ) { return currentContrast; }
	vrpn_int32 getExposure( ) {  return currentExposure;  }

protected:
	nmm_Microscope_SEM_optical( );


	int currentResolutionIndex;  // image resolution = camera resolution / binning
	int currentBinning;
	int currentContrast;
	int currentExposure;

};


#endif __NMM_MICROSCOPE_SEM_OPTICAL_H