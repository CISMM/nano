
#include <windows.h>
#include <SpotCam.h>
#include <stdlib.h>
#include <string.h>

#include <vrpn_Connection.h>
#include "nmm_Microscope_SEM_diaginc.h"
#include "nmb_Image.h"


#define VIRTUAL_ACQ_RES_X (128)
#define VIRTUAL_ACQ_RES_Y (100)

nmm_Microscope_SEM_diaginc::
nmm_Microscope_SEM_diaginc( const char * name, vrpn_Connection * c, vrpn_bool virtualAcq ) 
	: nmb_Device_Server(name, c),
	  nmm_Microscope_SEM(name, d_connection), 
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
		if( setupCamera( ) != 0 ) { return; }
	}

	initializeParameterDefaults();

	// create buffers in which to store data
	short maxExtents[2]; // width then height
	if( d_virtualAcquisition )
	{
		maxExtents[0] = VIRTUAL_ACQ_RES_X;
		maxExtents[1] = VIRTUAL_ACQ_RES_Y;
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

	return 0;
}


vrpn_int32 nmm_Microscope_SEM_diaginc::
mainloop(const struct timeval *timeout)
{
	//  if (!d_connection || !(d_connection->connected())) {
	//    return 0;
	//  }
	if (d_scans_to_do > 0) {
		// When running controlling the server without going over the network 
		// (in the same process as the client) there is a slight difference
		// process as the client it is necessary to decrement before 
		// calling acquireImage because
		// acquireImage calls mainloop for the connection and this can result
		// in a set of d_scans_to_do to 0 if the right message is received 
		// breaking our assumption that d_scans_to_do is always non-negative 
		d_scans_to_do--;
		acquireImage();
	} else {
	}
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
	/*
	RECT rect;
	int ret = SpotGetValue( SPOT_IMAGERECT, &rect );
	*/
	fprintf( stderr, "nmm_Microscope_SEM_diaginc::setResolution:  "
		"this function is unimplemented.\n" );
	
	return reportResolution();
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
	return 0;
}

vrpn_int32 nmm_Microscope_SEM_diaginc::
getMaxScan(int &x_span_DAC, int &y_span_DAC)
{
/*
x_span_DAC = d_xScanSpan;
y_span_DAC = d_yScanSpan;
	*/
	return 0;
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
	this->getResolution( resX, resY );
	
	if (!d_virtualAcquisition) {
		// variables used for timing
		double t_CollectSgLine, t_reportScanlineData;
		//int result;
		struct timeval t0, t1, t2;
		
		// collect the image
		t_CollectSgLine = 0.0;
		t_reportScanlineData = 0.0;
		for (i = 0; i < resY; i++){
			gettimeofday(&t0, NULL);
			//result = CollectSgLine(&(d_scanBuffer[2*i*d_resolution_x]));
			if (result != SPOT_SUCCESS) {
			fprintf(stderr, "Error collecting scan line for row %d\n", i);
			}
			gettimeofday(&t1, NULL);
			reportScanlineData(i);
			gettimeofday(&t2, NULL);
			t0 = vrpn_TimevalDiff(t1, t0);
			t1 = vrpn_TimevalDiff(t2, t1);
			t_CollectSgLine += vrpn_TimevalMsecs(t0);
			t_reportScanlineData += vrpn_TimevalMsecs(t1);
		}
	} 
	else 
	{ // assert(d_virtualAcquisition == true)
		// make some fake data:
		static int count = 0;
		
		//int dx, dy;
		for (i = 0; i < resY; i++){
			for (j = 0; j < resX; j++){
				((vrpn_uint8 *)myImageBuffer)[i*(resX + resX%4) + j] = 
					((i+(int)count)%resY)*250/resY;
			}
			
			reportScanlineData(i);
		}
		
		count++;
		
	}
	
	
	//printf("acquireImage - measured times for acquisition operations:\n");
	//printf("  SetSgScan x 1 : %g msec\n", t_SetSgScan);
	//printf("  SetupSgColl x 1 : %g msec\n", t_SetupSgColl);
	//printf("  CollectSgLine x %d : %g msec\n", d_resolution_y,
	//          t_CollectSgLine);
	//printf("  reportScanlineData x %d : %g msec\n", d_resolution_y,
	//          t_reportScanlineData);

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
