
#include <windows.h>
#include <SpotCam.h>
#include <stdlib.h>
#include <string.h>

#include <vrpn_Connection.h>
#include "nmm_Microscope_SEM_diaginc.h"
#include "nmb_Image.h"
#include "OpticalServerInterface.h"


#define VIRTUAL_ACQ_RES_X (1024)
#define VIRTUAL_ACQ_RES_Y (800)


int EDAX_SCAN_MATRIX_X[EDAX_NUM_SCAN_MATRICES] ={64, 128, 256, 512, 1024, 2048, 4096};
int EDAX_SCAN_MATRIX_Y[EDAX_NUM_SCAN_MATRICES] ={50, 100, 200, 400, 800,  1600, 3200};

//static 
int nmm_Microscope_SEM_diaginc::
resolutionToIndex(const int res_x, const int res_y)
{
    int i;
    for (i = 0; i < EDAX_NUM_SCAN_MATRICES; i++){
        if (EDAX_SCAN_MATRIX_X[i] == res_x &&
            EDAX_SCAN_MATRIX_Y[i] == res_y) {
            return i;
        }
    }
    return -1;
}


//static 
int nmm_Microscope_SEM_diaginc::
indexToResolution(const int id, int &res_x, int &res_y)
{
  if (id < 0 || id >= EDAX_NUM_SCAN_MATRICES)
     return -1;
  res_x = EDAX_SCAN_MATRIX_X[id];
  res_y = EDAX_SCAN_MATRIX_Y[id];
  return 0;
}


nmm_Microscope_SEM_diaginc::
nmm_Microscope_SEM_diaginc( const char * name, vrpn_Connection * c, vrpn_bool virtualAcq ) 
	: nmb_Device_Server(name, c),
	  nmm_Microscope_SEM(name, d_connection), 
	  d_scan_enabled(vrpn_FALSE), 
	  d_lines_per_message(1), 
	  d_scans_to_do(0),
	  d_virtualAcquisition(virtualAcq),
	  currentResolutionIndex( EDAX_DEFAULT_SCAN_MATRIX ),
	  currentBinning( 1 )
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
		maxExtents[0] = VIRTUAL_ACQ_RES_X;
		maxExtents[1] = VIRTUAL_ACQ_RES_Y;
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

	
}

nmm_Microscope_SEM_diaginc::
~nmm_Microscope_SEM_diaginc (void)
{
	OpticalServerInterface::getInterface()->setImage( NULL, 0, 0 );

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
			"and initializing the SPOT camera.  Code:  %d\n", success );
		return success;
	}
	
	// add our callback
	SpotSetCallback( nmm_Microscope_SEM_diaginc_spotCallback, (DWORD) this );

	success = setResolution( EDAX_SCAN_MATRIX_X[EDAX_DEFAULT_SCAN_MATRIX], 
							 EDAX_SCAN_MATRIX_Y[EDAX_DEFAULT_SCAN_MATRIX] );
	if( success != SPOT_SUCCESS )
	{
		fprintf( stderr, "nmm_Microscope_SEM_diaginc::setupCamera:  Error opening "
			"and initializing the SPOT camera.  Code:  %d\n", success );
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
	
	// ask for 8 bits per pixel
	int bitdepth = 8;
	success = SpotSetValue(SPOT_BITDEPTH, &bitdepth);
	if( success != SPOT_SUCCESS )
	{
		fprintf( stderr, "nmm_Microscope_SEM_diaginc::setupCamera:  Error opening "
			"and initializing the SPOT camera (bit depth).  Code:  %d\n", success );
		return success;
	}
	
	// turn off auto-exposure
	bool autoexpose = false;
	success = SpotSetValue( SPOT_AUTOEXPOSE, &autoexpose );
	if( success != SPOT_SUCCESS )
	{
		fprintf( stderr, "nmm_Microscope_SEM_diaginc::setupCamera:  Error opening "
			"and initializing the SPOT camera (auto-expose).  Code:  %d\n", success );
		return success;
	}
	
	// set an exposure time
	SPOT_EXPOSURE_STRUCT exposure;
	exposure.lExpMSec = 2000;
	exposure.lGreenExpMSec = 2000;
	exposure.lBlueExpMSec = 2000;
	exposure.nGain = 2;
	success = SpotSetValue( SPOT_EXPOSURE, &exposure );
	if( success != SPOT_SUCCESS )
	{
		fprintf( stderr, "nmm_Microscope_SEM_diaginc::setupCamera:  Error opening "
			"and initializing the SPOT camera (exposure).  Code:  %d\n", success );
		return success;
	}
	
	// set bin size
	short binning = 2;
	success = SpotSetValue( SPOT_BINSIZE, &binning );
	if( success != SPOT_SUCCESS )
	{
		fprintf( stderr, "nmm_Microscope_SEM_diaginc::setupCamera:  Error opening "
			"and initializing the SPOT camera (binning).  Code:  %d\n", success );
	   }
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
		res_x = VIRTUAL_ACQ_RES_X;
		res_y = VIRTUAL_ACQ_RES_Y;
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
		res_x = rect.right - rect.left;
		res_y = rect.bottom - rect.top;
	}

	int resInd = this->resolutionToIndex( res_x, res_y );
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
getBinning( vrpn_int32 &bin )
{
	if( d_virtualAcquisition )
	{
		bin = 1;
	}
	else
	{
		short binning;
		int ret = SpotGetValue( SPOT_BINSIZE, &binning );
		if( ret != SPOT_SUCCESS )
		{
			fprintf( stderr, "nmm_Microscope_SEM_diaginc::getBinning:  "
				"Error getting bin size.  Code:  %d\n", ret );
			bin = -1;
			return ret;
		}
		bin = binning;
	}

	if( bin != currentBinning )
	{
		OpticalServerInterface* iface = OpticalServerInterface::getInterface( );
		iface->setBinning( bin );
		currentBinning = bin;
	}

	return 0;
}


vrpn_int32 nmm_Microscope_SEM_diaginc::
getMaxResolution( vrpn_int32& x, vrpn_int32& y )
{
	if( d_virtualAcquisition )
	{
		x = VIRTUAL_ACQ_RES_X;
		y = VIRTUAL_ACQ_RES_Y;
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
setResolution( vrpn_int32 res_x, vrpn_int32 res_y )
{
	if( d_virtualAcquisition )
	{
		fprintf( stderr, "nmm_Microscope_SEM_diaginc::setResolution:  "
			"virtual acquisition on.  We're making up data and not changing the resolution.\n" );
		return reportResolution( );
	}

	// get the index for this resolution
	int resInd = this->resolutionToIndex( res_x, res_y );
	if( resInd < 0 )
	{
		fprintf( stderr, "nmm_Microscope_SEM_diaginc::setResolution:  "
			"invalid resolution requested:  %d x %d\n", res_x, res_y );
		return reportResolution( );
	}

	// get the max resolution
	int maxX = 0, maxY = 0;
	int retVal = this->getMaxResolution( maxX, maxY );
	if( retVal != 0 )
	{
		fprintf( stderr, "nmm_Microscope_SEM_diaginc::setResolution:  "
			"internal error.\n" );
		return reportResolution( );
	}

	// check that the requested resolution isn't too big
	if( res_x > maxX || res_y > maxY )
	{
		fprintf( stderr, "nmm_Microscope_SEM_diaginc::setResolution:  "
			"resolution (%d x %d) greater than max (%d x %d).\n",
			res_x, res_y, maxX, maxY );
		return reportResolution( );
	}

	// request the new resolution.  the requested area is centered in 
	// the camera's capture area.
	RECT rect;
	rect.left = ( (int) floor( maxX / 2.0f ) ) - ( (int) floor( res_x / 2.0f ) );
	rect.right = ( (int) floor( maxX / 2.0f ) ) +  ( (int) ceil( res_x / 2.0f ) );
	rect.top = ( (int) floor( maxY / 2.0f ) ) - ( (int) floor( res_y / 2.0f ) );
	rect.bottom = ( (int) floor( maxY / 2.0f ) ) +  ( (int) ceil( res_y / 2.0f ) );
	retVal = SpotSetValue( SPOT_IMAGERECT, &rect );
	if( retVal != SPOT_SUCCESS )
	{
		fprintf( stderr, "nmm_Microscope_SEM_diaginc::setResolution:  "
			"failed on the SPOT camera.  Code:  %d\n", retVal);
	}
	
	return reportResolution();
} // end setResolution(...)


vrpn_int32 nmm_Microscope_SEM_diaginc::
setBinning( vrpn_int32 bin )
{
	if( d_virtualAcquisition )
	{
		fprintf( stderr, "nmm_Microscope_SEM_diaginc::setBinning:  "
			"virtual acquisition on.  We're making up data and not changing the binning.\n" );
		return currentBinning;
	}

	// check that the requested bin size is in range
	if( bin <= 0 || bin > 4 )
	{
		fprintf( stderr, "nmm_Microscope_SEM_diaginc::setBinning:  "
			"bin size %d out of range.\n", bin );
		return currentBinning;
	}

	// request the new resolution.  the requested area is centered in 
	// the camera's capture area.
	short binsize = bin;
	int retVal = SpotSetValue( SPOT_BINSIZE, &binsize );
	if( retVal != SPOT_SUCCESS )
	{
		fprintf( stderr, "nmm_Microscope_SEM_diaginc::setBinning:  "
			"failed on the SPOT camera.  Code:  %d\n", retVal);
	}
	vrpn_int32 binning = 0;
	getBinning( binning );
	
	return currentBinning;
} // end setResolution(...)


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
	//printf("enable scan = %d\n", enable);
    if (nscans == 0) {
        d_scan_enabled = vrpn_FALSE;
        d_scans_to_do = 0;
    } else {
        d_scan_enabled = vrpn_TRUE;
        d_scans_to_do += nscans;
		acquireImage();
		d_scans_to_do--;
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


vrpn_int32 nmm_Microscope_SEM_diaginc::
acquireImage()
{
	int i, j;
	
	/*
	if (d_shared_settings_changed) {
		//  do something
	}
	if (d_image_mode_settings_changed) {
		//  do something else
	}
	*/
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
			reportScanlineData(i);
		}
		count++;
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
		memcpy( myImageBuffer, cameraImageBuffer, this->maxBufferSize );
		for (i = 0; i < resY; i++)
		{
			reportScanlineData(i);
		}
	}

	OpticalServerInterface::getInterface()->setImage( myImageBuffer, resX, resY );
	
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
int nmm_Microscope_SEM_diaginc::RcvSetResolution 
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
int nmm_Microscope_SEM_diaginc::RcvRequestScan
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
int nmm_Microscope_SEM_diaginc::RcvGotConnection
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
int nmm_Microscope_SEM_diaginc::RcvDroppedConnection
(void *_userdata, vrpn_HANDLERPARAM _p)
{
	nmm_Microscope_SEM_diaginc *me =
		(nmm_Microscope_SEM_diaginc *)_userdata;
	// me->closeScanControlInterface();
	return 0;
}


// from SpotCam.h:
// typedef VOID (WINAPI *SPOTCALLBACK)(int iStatus, long lInfo, DWORD dwUserData);
void WINAPI nmm_Microscope_SEM_diaginc_spotCallback( int iStatus, long lInfo, DWORD dwUserData )
{
	nmm_Microscope_SEM_diaginc* me = (nmm_Microscope_SEM_diaginc*) dwUserData;
	printf( "nmm_Microscope_SEM_diaginc_spotCallback:  status  %d\n", iStatus );

	if( iStatus == SPOT_STATUSLIVEIMAGEREADY )
	{
		memcpy( me->myImageBuffer, me->cameraImageBuffer, me->maxBufferSize );
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
	} // end if live image ready
	
}


// bogus function so that we appear to be an SEM
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
