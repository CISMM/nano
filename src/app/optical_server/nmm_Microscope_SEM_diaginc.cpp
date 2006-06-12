
#include "Tcl_Linkvar.h"
#include <windows.h>
#include <SpotCam.h>
#include <stdlib.h>
#include <string.h>

#include <vrpn_Connection.h>
#include "nmm_Microscope_SEM_diaginc.h"
#include "nmb_Image.h"
#include "OpticalServerInterface.h"
#include "edax_defs.h"

nmm_Microscope_SEM_diaginc::
nmm_Microscope_SEM_diaginc( const char * name, vrpn_Connection * c, vrpn_bool virtualAcq ) 
	: nmm_Microscope_SEM_optical( name, c, virtualAcq ), 
	  d_scan_enabled(vrpn_FALSE), 
	  d_lines_per_message(1), 
	  d_scans_to_do(0),
	  d_virtualAcquisition(virtualAcq)
{
	// VRPN initialization
	if (!d_connection) {
		fprintf(stderr, "nmm_Microscope_SEM_diaginc: Fatal Error: NULL connection\n");
		return;
	}
	d_connection->register_handler(d_SetResolution_type,
		RcvSetResolution, this);
	d_connection->register_handler(d_RequestScan_type,
		RcvRequestScan, this);
	
	int connect_id = d_connection->register_message_type(vrpn_got_connection);
	int disconnect_id = 
		d_connection->register_message_type(vrpn_dropped_connection);
	d_connection->register_handler(connect_id, RcvGotConnection, this);
	d_connection->register_handler(disconnect_id, RcvDroppedConnection, this);
	
	// SPOT camera initialization
	if( !d_virtualAcquisition )
	{
		if( setupCamera( ) != SPOT_SUCCESS ) { return; }
	}
	
	initializeParameterDefaults();
	
	// create buffers in which to store data
	short maxExtents[2]; // width then height
	if( d_virtualAcquisition )
	{
		maxExtents[0] = EDAX_SCAN_MATRIX_X[EDAX_NUM_SCAN_MATRICES-1];
		maxExtents[1] = EDAX_SCAN_MATRIX_Y[EDAX_NUM_SCAN_MATRICES-1];
		currentResolutionIndex = 1;
	}
	else
	{
		int ret = SpotGetValue( SPOT_MAXIMAGERECTSIZE, maxExtents );
		if( ret != SPOT_SUCCESS )
		{
			fprintf( stderr, "nmm_Microscope_SEM_diaginc constructor:  Error querying "
					"the SPOT camera max. resolution.  Code:  %d\n", ret );
			return;
		}
	}

	// the maximum number of bytes is (width + 3) * height * 3
	// each line can have up to 3 extra bytes to make its length a
	// multiple of 4.  in addition, each image can have up to 3
	// bytes per pixel (red, green, blue)
	maxBufferSize = (maxExtents[0] + 3) * maxExtents[1] * 3;
	myImageBuffer = new vrpn_uint8[ maxBufferSize ];
	cameraImageBuffer = new vrpn_uint8[ maxBufferSize ];


	// no changes, yet
	requestedChanges.resolutionChanged = requestedChanges.binningChanged 
		= requestedChanges.exposureChanged = false;

}

nmm_Microscope_SEM_diaginc::
~nmm_Microscope_SEM_diaginc (void)
{
	OpticalServerInterface* iface = OpticalServerInterface::getInterface( );
	if( iface != NULL ) iface->setImage( NULL, 0, 0 );

	// remove spot cam callback
	SpotSetCallback( NULL, 0 );
	
	// let the spot camera clean up after itself
	SpotExit( );

	delete[] myImageBuffer;
	delete[] cameraImageBuffer;
}


vrpn_int32 nmm_Microscope_SEM_diaginc::
setupCamera( )
{
	// initialize the SPOT camera
	int success = SpotInit( );
	if( success != SPOT_SUCCESS )
	{
		fprintf( stderr, "nmm_Microscope_SEM_diaginc::setupCamera:  Error opening "
			"and initializing the SPOT camera (init).  Code:  %d\n", success );
		return success;
	}
	
	// add our callback
	SpotSetCallback( nmm_Microscope_SEM_diaginc_spotCallback, (DWORD) this );

	// ask for 8 bits per pixel
	int bitdepth = 8;
	success = SpotSetValue(SPOT_BITDEPTH, &bitdepth);
	if( success != SPOT_SUCCESS )
	{
		fprintf( stderr, "nmm_Microscope_SEM_diaginc::setupCamera:  Error opening "
			"and initializing the SPOT camera (bit depth).  Code:  %d\n", success );
		return success;
	}
	
	// set bin size
	short binning = 1;
	success = SpotSetValue( SPOT_BINSIZE, &binning );
	if( success != SPOT_SUCCESS )
	{
		fprintf( stderr, "nmm_Microscope_SEM_diaginc::setupCamera:  Error opening "
			"and initializing the SPOT camera (binning).  Code:  %d\n", success );
	}

	// set resolution
	// check that the requested resolution isn't too big
	int res_x = EDAX_SCAN_MATRIX_X[OPTICAL_SERVER_DEFAULT_SCAN_MATRIX];
	int res_y = EDAX_SCAN_MATRIX_Y[OPTICAL_SERVER_DEFAULT_SCAN_MATRIX];
	int camX = res_x * binning, camY = res_y * binning;
	int maxX = 0, maxY = 0;
	getMaxResolution( maxX, maxY );
	if( camX > maxX || camY > maxY )
	{
		fprintf( stderr, "nmm_Microscope_SEM_diaginc::setupCamera:  "
			"resolution (%d x %d) greater than max (%d x %d) x %d.\n",
			res_x, res_y, maxX, maxY, currentBinning );
		return -1;
	}
	
	RECT rect;
	rect.left = ( (int) floor( maxX / 2.0f ) ) - ( (int) floor( camX / 2.0f ) );
	rect.right = ( (int) floor( maxX / 2.0f ) ) +  ( (int) ceil( camX / 2.0f ) );
	rect.top = ( (int) floor( maxY / 2.0f ) ) - ( (int) floor( camY / 2.0f ) );
	rect.bottom = ( (int) floor( maxY / 2.0f ) ) +  ( (int) ceil( camY / 2.0f ) );
	success = SpotSetValue( SPOT_IMAGERECT, &rect );
	if( success != SPOT_SUCCESS )
	{
		fprintf( stderr, "nmm_Microscope_SEM_diaginc::setupCamera:  Error opening "
			"and initializing the SPOT camera (resolution).  Code:  %d\n", success );
		return success;
	}
	
	// turn off auto-exposure
	BOOL autoexpose = false;
	success = SpotSetValue( SPOT_AUTOEXPOSE, &autoexpose );
	if( success != SPOT_SUCCESS )
	{
		fprintf( stderr, "nmm_Microscope_SEM_diaginc::setupCamera:  Error opening "
			"and initializing the SPOT camera (auto-expose).  Code:  %d\n", success );
		return success;
	}
	
	// Set the color to monochrome mode (turn off filter wheels)
	SPOT_COLOR_ENABLE_STRUCT2 colorstruct;
	colorstruct.bEnableRed = false;
	colorstruct.bEnableGreen = false;
	colorstruct.bEnableBlue = false;
	colorstruct.bEnableClear = false;
	success = SpotSetValue(SPOT_COLORENABLE2, &colorstruct);
	if( success != SPOT_SUCCESS )
	{
		fprintf( stderr, "nmm_Microscope_SEM_diaginc::setupCamera:  Error opening "
			"and initializing the SPOT camera (monochrome).  Code:  %d\n", success );
		return success;
	}
	
	// set an exposure time
	this->currentExposure = 100;
	SPOT_EXPOSURE_STRUCT exposure;
	exposure.lExpMSec = 0;
	exposure.lGreenExpMSec = this->currentExposure;
	exposure.lBlueExpMSec = 0;
	exposure.nGain = 2;
	success = SpotSetValue( SPOT_EXPOSURE, &exposure );
	if( success != SPOT_SUCCESS )
	{
		fprintf( stderr, "nmm_Microscope_SEM_diaginc::setupCamera:  Error opening "
			"and initializing the SPOT camera (exposure).  Code:  %d\n", success );
		return success;
	}

	/*
	SPOT_EXPOSURE_STRUCT2 exposure2;
	exposure2.dwExpDur = this->currentExposure;
	exposure2.dwRedExpDur = 0;
	exposure2.dwGreenExpDur = 0;
	exposure2.dwBlueExpDur = 0;
	exposure2.nGain = 2;
	success = SpotSetValue( SPOT_EXPOSURE2, &exposure2 );
	if( success != SPOT_SUCCESS )
	{
		fprintf( stderr, "nmm_Microscope_SEM_diaginc::setupCamera:  Error opening "
			"and initializing the SPOT camera (exposure2).  Code:  %d\n", success );
		return success;
	}
	*/

	return 0;
}


vrpn_int32 nmm_Microscope_SEM_diaginc::
mainloop(const struct timeval *timeout)
{
	//  if (!d_connection || !(d_connection->connected())) {
	//    return 0;
	//  }
	//if (d_scans_to_do > 0) {
		// When running controlling the server without going over the network 
		// (in the same process as the client) there is a slight difference
		// process as the client it is necessary to decrement before 
		// calling acquireImage because
		// acquireImage calls mainloop for the connection and this can result
		// in a set of d_scans_to_do to 0 if the right message is received 
		// breaking our assumption that d_scans_to_do is always non-negative 
		//d_scans_to_do--;
		acquireImage();
	//} else {
	//}
	return 0;
}

vrpn_int32 nmm_Microscope_SEM_diaginc::
initializeParameterDefaults()
{
/*
int file_found = 0;
int i;

  // hard-coded defaults:
  d_resolution_index = diaginc_DEFAULT_SCAN_MATRIX;
  d_interpixel_delay_nsec = 0;
  d_pix_integrate_nsec = 40000; 
  // 10000nsec --> about 1 uCoul/cm^2 at 10pA, mag=1000, res=1024x800
  //  }
  
    if (d_virtualAcquisition) {
	printf("Warning: you are using the VIRTUAL SEM\n");
    } else {
	#ifdef USE_SCAN_TABLE
	printf("Warning: this server uses ScanTable instead of SpMove\n");
	printf("Dwell time is fixed at 3 msec for point-by-point scanning\n");
	#endif
    }
	
	  d_point_dwell_time_nsec = 1000;
	  d_blankMode = diaginc_DEFAULT_BLANK_MODE;
	  d_beam_location_x = 0;
	  d_beam_location_y = 0;
	  
		d_external_scan_control_enabled = 0;
		
		  d_scanType = diaginc_FAST_SCAN;
		  d_dataTransfer = diaginc_DATA_TRANSFER_BYTE;
		  d_magnification = 1000;
		  
			d_image_mode_settings_changed = vrpn_TRUE;
			d_point_mode_settings_changed = vrpn_TRUE;
			d_shared_settings_changed = vrpn_TRUE;
	*/
	return 0;
}



vrpn_int32 nmm_Microscope_SEM_diaginc::
getResolution( vrpn_int32 &res_x, vrpn_int32 &res_y )
{
	if( d_virtualAcquisition )
	{
		int x = 0, y = 0;
		indexToResolution( currentResolutionIndex, x, y );
		res_x = x;  res_y = y;
	}
	else
	{
		RECT rect;
		int ret = SpotGetValue( SPOT_IMAGERECT, &rect );
		if( ret != SPOT_SUCCESS )
		{
			fprintf( stderr, "nmm_Microscope_SEM_diaginc::getResolution:  "
				"Error getting resolution.  Code:  %d\n", ret );
			res_x = -1;  res_y = -1;
			return ret;
		}
		res_x = (rect.right - rect.left) / currentBinning;
		res_y = (rect.bottom - rect.top) / currentBinning;
	}

	int resInd = resolutionToIndex( res_x, res_y );
	if( resInd >= 0 )
	{  currentResolutionIndex = resInd;  }
	else 
	{  
		// how did it go so very, very wrong??
		printf( "Internal error in nmm_Microscope_SEM_diaginc::getResolution:  "
				"camera and internal resolution don't match.\n" );
	}
	return 0;
}


vrpn_int32 nmm_Microscope_SEM_diaginc::
getBinning(  )
{
	if( d_virtualAcquisition )
	{
		return currentBinning;
	}
	else
	{
		short binning;
		int ret = SpotGetValue( SPOT_BINSIZE, &binning );
		if( ret != SPOT_SUCCESS )
		{
			fprintf( stderr, "nmm_Microscope_SEM_diaginc::getBinning:  "
				"Error getting bin size.  Code:  %d\n", ret );
			return -1;
		}
		
		if( binning != this->currentBinning )
		{
			OpticalServerInterface* iface = OpticalServerInterface::getInterface( );
			if( iface != NULL ) iface->setBinning( binning );
			currentBinning = binning;
		}
	}
	return currentBinning;
}


vrpn_int32 nmm_Microscope_SEM_diaginc::
getMaxResolution( vrpn_int32& x, vrpn_int32& y )
{
	if( d_virtualAcquisition )
	{
		x = EDAX_SCAN_MATRIX_X[EDAX_NUM_SCAN_MATRICES-1];
		y = EDAX_SCAN_MATRIX_Y[EDAX_NUM_SCAN_MATRICES-1];
	}
	else
	{
		short maxExtents[2]; // width then height
		int ret = SpotGetValue( SPOT_MAXIMAGERECTSIZE, maxExtents );
		if( ret != SPOT_SUCCESS )
		{
			fprintf( stderr, "nmm_Microscope_SEM_diaginc::getMaxResolution:  Error querying "
				"the SPOT camera max. resolution.  Code:  %d\n", ret );
			return -1;
		}
		x = maxExtents[0];
		y = maxExtents[1];
	}
	return 0;
}


vrpn_int32 nmm_Microscope_SEM_diaginc::
setResolutionByIndex( vrpn_int32 idx )
{
	if( idx < 0 || idx > EDAX_NUM_SCAN_MATRICES - 1 )
	{
		fprintf( stderr, "nmm_Microscope_SEM_diaginc::setResolutionbyIndex:  "
			"invalid resolution requested:  %d\n", idx );
		OpticalServerInterface* iface = OpticalServerInterface::getInterface( );
		if( iface != NULL ) iface->setResolutionIndex( currentResolutionIndex );	
		return currentResolutionIndex;
	}
	vrpn_int32 res_x = 0, res_y = 0;
	indexToResolution( idx, res_x, res_y );

	// get the max resolution
	int maxX = 0, maxY = 0;
	int retVal = this->getMaxResolution( maxX, maxY );
	if( retVal != 0 )
	{
		fprintf( stderr, "nmm_Microscope_SEM_diaginc::setResolutionByIndex:  "
			"internal error.\n" );
		OpticalServerInterface* iface = OpticalServerInterface::getInterface( );
		if( iface != NULL ) iface->setResolutionIndex( currentResolutionIndex );	
		return currentResolutionIndex;
	}

	// check that the requested resolution isn't too big
	int camX = res_x * currentBinning, camY = res_y * currentBinning;
	if( camX > maxX || camY > maxY )
	{
		fprintf( stderr, "nmm_Microscope_SEM_diaginc::setResolutionByIndex:  "
			"resolution (%d x %d) greater than max (%d x %d) x %d.\n",
			res_x, res_y, maxX, maxY, currentBinning );
		OpticalServerInterface* iface = OpticalServerInterface::getInterface( );
		if( iface != NULL ) iface->setResolutionIndex( currentResolutionIndex );	
		return currentResolutionIndex;
	}

	if( d_virtualAcquisition )
	{
		currentResolutionIndex = idx;
	}
	else
	{
		requestedChanges.resolutionChanged = true;
		requestedChanges.newResolutionIndex = idx;

	}
	
	return idx;
} // end of setResolutionByIndex(...)


vrpn_int32 nmm_Microscope_SEM_diaginc::
setBinning( vrpn_int32 bin )
{
	int retVal = SPOT_SUCCESS;

	// check that the requested bin size is in range
	if( bin <= 0 || bin > 4 )
	{
		fprintf( stderr, "nmm_Microscope_SEM_diaginc::setBinning:  "
			"bin size %d out of range.\n", bin );
		return currentBinning;
	}

	// figure out what new resolution we need, and if it's possible
	double resFactor = (double) bin / (double) currentBinning;
	int resX = 0, resY = 0, maxX = 0, maxY = 0, camX = 0, camY  = 0;
	retVal = this->getResolution( resX, resY );
	if( retVal != SPOT_SUCCESS )
	{
		fprintf( stderr, "nmm_Microscope_SEM_diaginc::setBinning:  "
			"error fixing the resolution (1).\n" );
		return currentBinning;
	}
	camX = resX * resFactor;  
	camY = resY * resFactor;
	retVal = this->getMaxResolution( maxX, maxY );
	if( retVal != SPOT_SUCCESS )
	{
		fprintf( stderr, "nmm_Microscope_SEM_diaginc::setBinning:  "
			"error fixing the resolution (2).\n" );
		return currentBinning;
	}
	if( camX > maxX || camY > maxY )
	{
		fprintf( stderr, "nmm_Microscope_SEM_diaginc::setBinning:  "
			"required area too large for camera: (%d x %d) needed.  "
			"(%d x %d) available.\n", camX, camY, maxX, maxY );
		return currentBinning;
	}

	// request the new binning and resolution.  the requested area is centered in 
	// the camera's capture area.
	if( d_virtualAcquisition )
	{
		currentBinning = bin;
	}
	else
	{
		requestedChanges.newBinning = bin;
		requestedChanges.binningChanged = true;
	}

	return currentBinning;
} // end setBinning(...)


vrpn_int32 nmm_Microscope_SEM_diaginc::
setExposure( vrpn_int32 millisecs )
{
	if( d_virtualAcquisition )
	{
		currentExposure = millisecs;
	}
	else
	{
		requestedChanges.newExposure = millisecs;
		requestedChanges.exposureChanged = true;
	}
	return currentExposure;
}

	
vrpn_int32 nmm_Microscope_SEM_diaginc::
reportResolution()
{
	char *msgbuf;
	vrpn_int32 len;
	vrpn_int32 x, y;
	
	vrpn_int32 ret = getResolution( x, y );
	if( ret != 0 ) 
	{ 
		fprintf( stderr, "nmm_Microscope_SEM_diaginc::reportResolution:  "
			"unable to get resolution from the SPOT camera.  Code:  %d\n", ret );
		return ret; 
	}
	
	msgbuf = encode_ReportResolution(&len, x, y);
	if (!msgbuf)
	{
		return -1;
	}
	
	return dispatchMessage(len, msgbuf, d_ReportResolution_type);
}


vrpn_int32 nmm_Microscope_SEM_diaginc::
requestScan(vrpn_int32 nscans)
{
	printf("enable scan = %d\n", nscans);
    if (nscans == 0) {
        d_scan_enabled = vrpn_FALSE;
        d_scans_to_do = 0;
    } else {
        d_scan_enabled = vrpn_TRUE;
        d_scans_to_do += nscans;
		//acquireImage();
		//d_scans_to_do--;
    }
    return 0;
}



vrpn_bool nmm_Microscope_SEM_diaginc::
scanEnabled()
{
	return d_scan_enabled;
}



vrpn_int32 nmm_Microscope_SEM_diaginc::
getScanRegion_nm( double &x_span_nm, double &y_span_nm )
{
/*
x_span_nm = SEM_STANDARD_DISPLAY_WIDTH_NM/(double)d_magnification;
y_span_nm = x_span_nm*(double)d_resolution_y/(double)d_resolution_x;
	*/
	return -1;
}

vrpn_int32 nmm_Microscope_SEM_diaginc::
getMaxScan(int &x_span_DAC, int &y_span_DAC)
{
/*
x_span_DAC = d_xScanSpan;
y_span_DAC = d_yScanSpan;
	*/
	return -1;
}


void nmm_Microscope_SEM_diaginc::
printSpotValues( )
{
	// auto-expose
	BOOL autoexpose;
	if( SpotGetValue( SPOT_AUTOEXPOSE, &autoexpose ) != SPOT_SUCCESS )
	{
		printf( "nmm_Microscope_SEM_diaginc::printSpotValues:  "
				 "failed to get auto-expose.\n" );
	}
	printf( "  SPOT:  autoexpose:  %s\n", ( (autoexpose != 0) ? "true" : "false" ) );

	// binning
	short bins;
	if( SpotGetValue( SPOT_BINSIZE, &bins ) != SPOT_SUCCESS )
	{
		printf( "nmm_Microscope_SEM_diaginc::printSpotValues:  "
				 "failed to get bins.\n" );
	}
	printf( "  SPOT:  binning:  %hd\n", bins );

	// color enable
	SPOT_COLOR_ENABLE_STRUCT colors;
	if( SpotGetValue( SPOT_COLORENABLE, &colors ) != SPOT_SUCCESS )
	{
		printf( "nmm_Microscope_SEM_diaginc::printSpotValues:  "
				 "failed to get colors\n" );
	}
	printf( "  SPOT:  color enable:  red(%s), green(%s), blue(%s)\n", 
			( (colors.bEnableRed != 0) ? "true" : "false" ),
			( (colors.bEnableGreen != 0) ? "true" : "false" ),
			( (colors.bEnableBlue != 0) ? "true" : "false" ) );

	// color enable 2
	SPOT_COLOR_ENABLE_STRUCT2 colors2;
	if( SpotGetValue( SPOT_COLORENABLE2, &colors2 ) != SPOT_SUCCESS )
	{
		printf( "nmm_Microscope_SEM_diaginc::printSpotValues:  "
				 "failed to get colors2.\n" );
	}
	printf( "  SPOT:  color enable 2:  red(%s), green(%s), blue(%s), clear(%s)\n", 
			( (colors2.bEnableRed != 0) ? "true" : "false" ),
			( (colors2.bEnableGreen != 0) ? "true" : "false" ),
			( (colors2.bEnableBlue != 0) ? "true" : "false" ),
			( (colors2.bEnableClear != 0) ? "true" : "false" ) );

	// exposure times
	SPOT_EXPOSURE_STRUCT exposure;
	if( SpotGetValue( SPOT_EXPOSURE, &exposure ) != SPOT_SUCCESS )
	{
		printf( "nmm_Microscope_SEM_diaginc::printSpotValues:  "
				 "failed to get exposure.\n" );
	}
	printf( "  SPOT:  exposure:  red/clear/exp(%ld), green(%ld), blue(%ld), gain(%hd)\n", 
			exposure.lRedExpMSec, exposure.lGreenExpMSec, exposure.lBlueExpMSec, 
			exposure.nGain );

	// exposure2 times
	SPOT_EXPOSURE_STRUCT2 exposure2;
	if( SpotGetValue( SPOT_EXPOSURE2, &exposure2 ) != SPOT_SUCCESS )
	{
		printf( "nmm_Microscope_SEM_diaginc::printSpotValues:  "
				 "failed to get exposure2.\n" );
	}
	printf( "  SPOT:  exposure 2:  red(%ld), green(%ld), blue(%ld), clear/exp(%ld), gain(%hd)\n", 
			exposure2.dwRedExpDur, exposure2.dwGreenExpDur, exposure2.dwBlueExpDur, 
			exposure2.dwClearExpDur, exposure2.nGain );


}


vrpn_int32 nmm_Microscope_SEM_diaginc::
acquireImage()
{
	int i, j;
	
	int result = SPOT_SUCCESS;
	int resX = 0, resY = 0;
	result = this->getResolution( resX, resY );
	if( result != SPOT_SUCCESS )
	{
		return -1;
	}
	
	if( d_virtualAcquisition )
	{ 
		static int count = 0;
		for (i = 0; i < resY; i++)
		{
			for (j = 0; j < resX; j++)
			{
				((vrpn_uint8 *)myImageBuffer)[i*(resX + resX%4) + j] = 
					((i+(int)count)%resY)*250/resY;
			}
			if( d_scans_to_do > 0 )
				reportScanlineData(i);
		}
		count++;
		if( d_scans_to_do > 0 )
			d_scans_to_do--;
	}
	else // the real thing
	{
		// collect the image
		int retVal = SpotGetImage( 0, // bits per plane ( 0 = use set value )
								   false, // quick image capture
								   0, // lines to skip
								   cameraImageBuffer, // image buffer
								   NULL, // ptr to red histogram
								   NULL, // ptr to green histogram
								   NULL  // ptr to blue histogram
								   );
		if( retVal != SPOT_SUCCESS )
		{
			fprintf( stderr, "nmm_Microscope_SEM_diaginc::acquireImage:  "
				"failed on the SPOT camera.  Code:  %d\n", retVal);
			return -1;
		}

		vrpn_uint8 max = 0xFF >> currentContrast;
		for( int row = 0; row < resY; row++ )
		{
			int line = row * resX;
			for( int col = 0; col < resX; col++ )
			{
				if( cameraImageBuffer[line+col] > max )
					myImageBuffer[line+col] = 0xFF;
				else
					myImageBuffer[line+col] 
					= (vrpn_uint8) ( cameraImageBuffer[line+col] << currentContrast );
			}
		}
		if( d_scans_to_do > 0 ) 
		{
			for (i = 0; i < resY; i++)
			{
				reportScanlineData(i);
			}
			d_scans_to_do--;
		}
	}

	OpticalServerInterface* iface = OpticalServerInterface::getInterface( );
	if( iface != NULL ) iface->setImage( myImageBuffer, resX, resY );
	
	return 0;
}


vrpn_int32 nmm_Microscope_SEM_diaginc::reportScanlineData(int line_num)
{
	// line data is at d_scanBuffer[line_num*(d_resolution_x + d_resolution_x%4)]
	//  the spot camera can have "zero to three padding bytes per image line
	//  so that the number of bytes per line is an integer multiple of four."
	vrpn_int32 resX = 0, resY = 0;
	if( this->getResolution( resX, resY ) != 0 )  { return -1; }
	char *msgbuf;
	vrpn_int32 len;
	struct timeval now;
	void* data = &(myImageBuffer[ line_num * (resX + resX%4) ]);
	
	gettimeofday(&now, NULL);
	vrpn_int32 lines_per_message = d_lines_per_message;
	if (line_num + lines_per_message > resY){
		lines_per_message = resY - line_num;
	}

	msgbuf = encode_ScanlineData(&len, 0, line_num, 1, 1,
		resX, 1, lines_per_message, now.tv_sec, now.tv_usec,
		NMB_UINT8, &data);
	if (!msgbuf) { 
		return -1;
	}
	return dispatchMessage(len, msgbuf, d_ScanlineData_type);
}


vrpn_int32 nmm_Microscope_SEM_diaginc::reportMaxScanSpan()
{
	vrpn_int32 x, y;
	this->getMaxResolution( x, y );
	char *msgbuf;
	vrpn_int32 len;
	
	msgbuf = encode_ReportMaxScanSpan(&len, x, y);
	if (!msgbuf)
	{
		return -1;
	}
	return dispatchMessage(len, msgbuf, d_ReportMaxScanSpan_type);
}



//static 
int VRPN_CALLBACK nmm_Microscope_SEM_diaginc::RcvSetResolution 
(void *_userdata, vrpn_HANDLERPARAM _p)
{
	nmm_Microscope_SEM_diaginc *me = (nmm_Microscope_SEM_diaginc *)_userdata;
	const char * bufptr = _p.buffer;
	vrpn_int32 res_x, res_y;
	
	if (decode_SetResolution(&bufptr, &res_x, &res_y) == -1) {
		fprintf(stderr, 
			"nmm_Microscope_SEM_diaginc::RcvSetResolution: decode failed\n");
		return -1;
	}
	if (me->setResolution(res_x, res_y) == -1) {
		fprintf(stderr, "nmm_Microscope_SEM_diaginc::RcvSetResolution: set failed\n");
		return -1;
	}
	return 0;
}


//static 
int VRPN_CALLBACK nmm_Microscope_SEM_diaginc::RcvRequestScan
(void *_userdata, vrpn_HANDLERPARAM _p)
{
	nmm_Microscope_SEM_diaginc *me = (nmm_Microscope_SEM_diaginc *)_userdata;
	const char * bufptr = _p.buffer;
	vrpn_int32 nscans;
	
	if (decode_RequestScan(&bufptr, &nscans) == -1) {
		fprintf(stderr,
			"nmm_Microscope_SEM_diaginc::RcvRequestScan: decode failed\n");
		return -1;
	}
	if (me->requestScan(nscans) == -1) {
		fprintf(stderr, "nmm_Microscope_SEM_diaginc::RcvRequestScan: set failed\n");
		return -1;
	}
	return 0;
}



//static
int VRPN_CALLBACK nmm_Microscope_SEM_diaginc::RcvGotConnection
(void *_userdata, vrpn_HANDLERPARAM _p)
{
    nmm_Microscope_SEM_diaginc *me = 
		(nmm_Microscope_SEM_diaginc *)_userdata;
	
    me->reportResolution();
    me->reportPixelIntegrationTime();
    me->reportInterPixelDelayTime();
    me->reportMaxScanSpan();
    me->reportBeamBlankEnable();
    me->reportPointDwellTime();
    me->reportRetraceDelays();
    me->reportDACParams();
    me->reportExternalScanControlEnable();
    me->reportMagnification();
    me->d_scans_to_do = 0;
    me->d_scan_enabled = 0;
    return 0;
}


// static
int VRPN_CALLBACK nmm_Microscope_SEM_diaginc::RcvDroppedConnection
(void *_userdata, vrpn_HANDLERPARAM _p)
{
	nmm_Microscope_SEM_diaginc *me =
		(nmm_Microscope_SEM_diaginc *)_userdata;
	// me->closeScanControlInterface();
	return 0;
}


// a helper function to change the resolution
// should ONLY be called from the idle notice in the spot callback, below
void nmm_Microscope_SEM_diaginc::
doRequestedChangesOnSpot( )
{
	if( requestedChanges.binningChanged )
	{
		requestedChanges.binningChanged = false;

		int bin = requestedChanges.newBinning;
		// check that the requested bin size is in range
		if( bin <= 0 || bin > 4 )
		{
			fprintf( stderr, "nmm_Microscope_SEM_diaginc::setBinning:  "
				"bin size %d out of range.\n", bin );
			return;
		}
		
		// figure out what new resolution we need, and if it's possible
		double resFactor = (double) bin / (double) currentBinning;
		int resX = 0, resY = 0, maxX = 0, maxY = 0, camX = 0, camY  = 0;
		int retVal = this->getResolution( resX, resY );
		if( retVal != SPOT_SUCCESS )
		{
			fprintf( stderr, "nmm_Microscope_SEM_diaginc::doRequestedChangesOnSpot:  "
				"(binning) error fixing the resolution (1).\n" );
			return;
		}
		camX = resX * resFactor;  
		camY = resY * resFactor;
		retVal = this->getMaxResolution( maxX, maxY );
		if( retVal != SPOT_SUCCESS )
		{
			fprintf( stderr, "nmm_Microscope_SEM_diaginc::doRequestedChangesOnSpot:  "
				"(binning) error fixing the resolution (2).\n" );
			return;
		}
		if( camX > maxX || camY > maxY )
		{
			fprintf( stderr, "nmm_Microscope_SEM_diaginc::doRequestedChangesOnSpot:  "
				"required area too large for camera: (%d x %d) needed.  "
				"(%d x %d) available.\n", camX, camY, maxX, maxY );
			return;
		}

		short binsize = bin;
		retVal = SpotSetValue( SPOT_BINSIZE, &binsize );
		if( retVal != SPOT_SUCCESS )
		{
			fprintf( stderr, "nmm_Microscope_SEM_diaginc::doRequestedChangesOnSpot:  "
				"failed on the SPOT camera (binning).  Code:  %d\n", retVal);
			return;
		}
		RECT rect;
		rect.left = ( (int) floor( maxX / 2.0f ) ) - ( (int) floor( camX / 2.0f ) );
		rect.right = ( (int) floor( maxX / 2.0f ) ) +  ( (int) ceil( camX / 2.0f ) );
		rect.top = ( (int) floor( maxY / 2.0f ) ) - ( (int) floor( camY / 2.0f ) );
		rect.bottom = ( (int) floor( maxY / 2.0f ) ) +  ( (int) ceil( camY / 2.0f ) );
		retVal = SpotSetValue( SPOT_IMAGERECT, &rect );
		if( retVal != SPOT_SUCCESS )
		{
			fprintf( stderr, "nmm_Microscope_SEM_diaginc::setBinning:  "
				"resolution failed on the SPOT camera.  Code:  %d\n", retVal);
			// try to set the binning back
			binsize = currentBinning;
			SpotSetValue( SPOT_BINSIZE, &binsize );
		}
		vrpn_int32 binning = getBinning( );
		OpticalServerInterface* iface = OpticalServerInterface::getInterface( );
		if( iface != NULL ) iface->setBinning( binning );
	}

	if( requestedChanges.resolutionChanged )
	{
		requestedChanges.resolutionChanged = false;
		vrpn_int32 res_x = 0, res_y = 0;
		indexToResolution( requestedChanges.newResolutionIndex, res_x, res_y );
		
		// get the max resolution
		int maxX = 0, maxY = 0;
		int retVal = getMaxResolution( maxX, maxY );
		if( retVal != 0 )
		{
			fprintf( stderr, "nmm_Microscope_SEM_diaginc::doRequestedChangesOnSpot:  "
				"internal error (resolution).\n" );
			reportResolution( );
			OpticalServerInterface* iface = OpticalServerInterface::getInterface( );
			if( iface != NULL ) iface->setResolutionIndex( currentResolutionIndex );	
			return;
		}
		
		// check that the requested resolution isn't too big
		int camX = res_x * currentBinning, camY = res_y * currentBinning;
		if( camX > maxX || camY > maxY )
		{
			fprintf( stderr, "nmm_Microscope_SEM_diaginc::doRequestedChangesOnSpot:  "
				"resolution (%d x %d) greater than max (%d x %d) x %d.\n",
				res_x, res_y, maxX, maxY, currentBinning );
			reportResolution( );
			OpticalServerInterface* iface = OpticalServerInterface::getInterface( );
			if( iface != NULL ) iface->setResolutionIndex( currentResolutionIndex );	
			return;
		}
		
		// request the new resolution.  the requested area is centered in 
		// the camera's capture area.
		RECT rect;
		rect.left = ( (int) floor( maxX / 2.0f ) ) - ( (int) floor( camX / 2.0f ) );
		rect.right = ( (int) floor( maxX / 2.0f ) ) +  ( (int) ceil( camX / 2.0f ) );
		rect.top = ( (int) floor( maxY / 2.0f ) ) - ( (int) floor( camY / 2.0f ) );
		rect.bottom = ( (int) floor( maxY / 2.0f ) ) +  ( (int) ceil( camY / 2.0f ) );
		retVal = SpotSetValue( SPOT_IMAGERECT, &rect );
		if( retVal != SPOT_SUCCESS )
		{
			fprintf( stderr, "nmm_Microscope_SEM_diaginc::doRequestedChangesOnSpot:  "
				"failed on the SPOT camera (resolution).  Code:  %d\n", retVal);
		}
		
		reportResolution( );
		OpticalServerInterface* iface = OpticalServerInterface::getInterface( );
		if( iface != NULL ) iface->setResolutionIndex( currentResolutionIndex );	
	}
	
	
	if( requestedChanges.exposureChanged )
	{
		requestedChanges.exposureChanged = false;

		SPOT_EXPOSURE_STRUCT exposure;
		exposure.lExpMSec = 0;
		exposure.lGreenExpMSec = requestedChanges.newExposure;
		exposure.lBlueExpMSec = 0;
		exposure.nGain = 2;
		int success = SpotSetValue( SPOT_EXPOSURE, &exposure );
		if( success != SPOT_SUCCESS )
		{
			fprintf( stderr, "nmm_Microscope_SEM_diaginc::doRequestedChangesOnSpot:  Error setting "
					"exposure (%d ms) in the SPOT camera.  Code:  %d\n", requestedChanges.newExposure, success );
			OpticalServerInterface* iface = OpticalServerInterface::getInterface( );
			if( iface != NULL ) iface->setExposure( currentExposure );
			return;
		}
		currentExposure = requestedChanges.newExposure;
		OpticalServerInterface* iface = OpticalServerInterface::getInterface( );
		if( iface != NULL ) iface->setExposure( currentExposure );
	}

}



// from SpotCam.h:
// typedef VOID (WINAPI *SPOTCALLBACK)(int iStatus, long lInfo, DWORD dwUserData);
void WINAPI nmm_Microscope_SEM_diaginc_spotCallback( int iStatus, long lInfo, DWORD dwUserData )
{
	nmm_Microscope_SEM_diaginc* me = (nmm_Microscope_SEM_diaginc*) dwUserData;
	if( iStatus == SPOT_STATUSLIVEIMAGEREADY )
	{
		//printf( "nmm_Microscope_SEM_diaginc_spotCallback:  status  %d (live image ready)\n", iStatus );

		/* do this in acquireImage
		if( me->d_scans_to_do > 0 )
		{
			// When running controlling the server without going over the network 
			// (in the same process as the client) there is a slight difference
			// process as the client it is necessary to decrement before 
			// calling acquireImage because
			// acquireImage calls mainloop for the connection and this can result
			// in a set of d_scans_to_do to 0 if the right message is received 
			// breaking our assumption that d_scans_to_do is always non-negative 
			
			me->d_scans_to_do--;
			me->acquireImage();
		}
		*/
	} // end if live image ready
	if( iStatus == SPOT_STATUSIDLE )
	{
		//printf( "nmm_Microscope_SEM_diaginc_spotCallback:  status  %d (idle)\n", iStatus );
		me->doRequestedChangesOnSpot( );
	}
	
}


//////////////////////////////////////////////////
// bogus functions so that we appear to be an SEM
/////////
vrpn_int32 nmm_Microscope_SEM_diaginc::reportPixelIntegrationTime()
{
  char *msgbuf;
  vrpn_int32 len;

  msgbuf = encode_ReportPixelIntegrationTime(&len, -1);
  if (!msgbuf){
    return -1;
  }

  return dispatchMessage(len, msgbuf, d_ReportPixelIntegrationTime_type);
}


vrpn_int32 nmm_Microscope_SEM_diaginc::reportInterPixelDelayTime()
{
  char *msgbuf;
  vrpn_int32 len;

  msgbuf = encode_ReportInterPixelDelayTime(&len, 0);
  if (!msgbuf){
    return -1;
  }

  return dispatchMessage(len, msgbuf, d_ReportInterPixelDelayTime_type);
}


vrpn_int32 nmm_Microscope_SEM_diaginc::reportPointDwellTime()
{
  char *msgbuf;
  vrpn_int32 len;

  msgbuf = encode_ReportPointDwellTime(&len, 0);
  if (!msgbuf){
    return -1;
  }

  return dispatchMessage(len, msgbuf, d_ReportPointDwellTime_type);
}


vrpn_int32 nmm_Microscope_SEM_diaginc::reportBeamBlankEnable()
{
  char *msgbuf;
  vrpn_int32 len;

  msgbuf = encode_ReportBeamBlankEnable(&len, 0);
  if (!msgbuf){
    return -1;
  }
  
  return dispatchMessage(len, msgbuf, d_ReportBeamBlankEnable_type);
}


vrpn_int32 nmm_Microscope_SEM_diaginc::reportMagnification()
{
  char *msgbuf;
  vrpn_int32 len;

  msgbuf = encode_ReportMagnification(&len, 1);
  if (!msgbuf){
    return -1;
  }

  return dispatchMessage(len, msgbuf, d_ReportMagnification_type);
}


vrpn_int32 nmm_Microscope_SEM_diaginc::reportExternalScanControlEnable()
{
  char *msgbuf;
  vrpn_int32 len;
 
  msgbuf = encode_ReportExternalScanControlEnable(&len, 0);
  if (!msgbuf){
    return -1;
  }
  
  return dispatchMessage(len, msgbuf, d_ReportExternalScanControlEnable_type);
}


vrpn_int32 nmm_Microscope_SEM_diaginc::reportDACParams()
{
  char *msgbuf;
  vrpn_int32 len;

  msgbuf = encode_ReportDACParams(&len, 0, 0, 0, 0, 0, 0);
  if (!msgbuf){
    return -1;
  }

  return dispatchMessage(len, msgbuf, d_ReportDACParams_type);
}


vrpn_int32 nmm_Microscope_SEM_diaginc::reportRetraceDelays()
{
  char *msgbuf;
  vrpn_int32 len;

  msgbuf = encode_ReportRetraceDelays(&len, 0, 0);
  if (!msgbuf){
    return -1;
  }

  return dispatchMessage(len, msgbuf, d_ReportRetraceDelays_type);
}
