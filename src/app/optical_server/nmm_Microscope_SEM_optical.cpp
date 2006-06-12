

#include "OpticalServerInterface.h"

#include "nmm_Microscope_SEM_optical.h"
#include "edax_defs.h"

nmm_Microscope_SEM_optical::
nmm_Microscope_SEM_optical( const char * name, vrpn_Connection * c, vrpn_bool virtualAcq )
	: nmb_Device_Server(name, c),
	  nmm_Microscope_SEM(name, d_connection),
	  currentResolutionIndex( OPTICAL_SERVER_DEFAULT_SCAN_MATRIX ),
	  currentBinning( 1 )
{

}


nmm_Microscope_SEM_optical::
~nmm_Microscope_SEM_optical( )
{
}


vrpn_int32 nmm_Microscope_SEM_optical::
setResolution( vrpn_int32 res_x, vrpn_int32 res_y )
{
	// get the index for this image resolution
	int idx = resolutionToIndex( res_x, res_y );
	return this->setResolutionByIndex( idx );
}


vrpn_int32 nmm_Microscope_SEM_optical::
setContrastLevel( vrpn_int32 level )
{
	if( level < 0 || level > 8 )
	{  return -1;  }
	currentContrast = level;
	OpticalServerInterface* iface = OpticalServerInterface::getInterface( );
	if( iface != NULL ) iface->setContrast( level );
	return currentContrast;
}


