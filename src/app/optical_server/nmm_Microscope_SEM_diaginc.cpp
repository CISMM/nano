
#include <windows.h>
#include <SpotCam.h>
#include <stdlib.h>
#include <string.h>

#include <vrpn_Connection.h>
#include "nmm_Microscope_SEM_diaginc.h"
#include "nmb_Image.h"

nmm_Microscope_SEM_diaginc::
nmm_Microscope_SEM_diaginc( const char * name, vrpn_Connection * c, vrpn_bool virtualAcq ) 
	: nmb_Device_Server(name, c),
	  nmm_Microscope_SEM(name, d_connection), 
	  d_scan_enabled(vrpn_FALSE), 
	  d_lines_per_message(1), 
	  d_scans_to_do(0),
#ifdef VIRTUAL_SEM
	  d_virtualAcquisition(vrpn_TRUE)
#else
	  d_virtualAcquisition(virtualAcq)
#endif
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
	if( setupCamera( ) != 0 )
	{ return; }

	initializeParameterDefaults();
	// create buffers in which to store data
	short maxExtents[2]; // width then height
	int ret = SpotGetValue( SPOT_MAXIMAGERECTSIZE, maxExtents );
	if( ret != SPOT_SUCCESS )
	{
		fprintf( stderr, "nmm_Microscope_SEM_diaginc constructor:  Error querying "
			"the SPOT camera max. resolution.  Code:  %d\n", ret );
		return;
	}
	// the maximum number of bytes is (width + 3) * height * 3
	// each line can have up to 3 extra bytes to make its length a
	// multiple of 4.  in addition, each image can have up to 3
	// bytes per pixel (red, green, blue)
	maxBufferSize = (maxExtents[0] + 3) * maxExtents[1] * 3;
	myImageBuffer = (void*) new vrpn_uint8[ maxBufferSize ];
	cameraImageBuffer = (void*) new vrpn_uint8[ maxBufferSize ];

	
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
	/*
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
	*/
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
	return 0;
}


vrpn_int32 nmm_Microscope_SEM_diaginc::
getMaxResolution( vrpn_int32& x, vrpn_int32& y )
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
	/*
	int i, j;
	
	if (d_shared_settings_changed) {
		//  do something
	}
	if (d_image_mode_settings_changed) {
		//  do something else
	}
	
	if (!d_virtualAcquisition) {
#ifndef VIRTUAL_SEM
		// variables used for timing
		double t_SetSgScan, t_SetupSgColl, t_CollectSgLine, t_reportScanlineData;
		//int result;
		struct timeval t0, t1, t2;
		
		gettimeofday(&t0, NULL);
		// start the scan as configured
		// this is supposedly where the scan gets started but then
		// does that require later calls to wait until the next scan starts?
		// result = SetSgScan(diaginc_FULL_SCAN);
		
		gettimeofday(&t1, NULL);
		t0 = vrpn_TimevalDiff(t1, t0);
		t_SetSgScan = vrpn_TimevalMsecs(t0);
		
		if (result == diaginc_ERROR) {
		fprintf(stderr, "Error starting scan\n");
		return -1;
		}
		
		gettimeofday(&t0, NULL);
		
		// integration time is in 100 ns units
		// I guess this initializes the buffer to acquire a new image
		// last two parameters: ADC = 1, number of strips to collect= 1
		// integration time = 0 has special meaning - only gives us most
		// significant byte from ADC
		result = SetupSgColl(d_resolution_index, d_pix_integrate_nsec/100, 1, 1);
		if (result == diaginc_ERROR) {
		fprintf(stderr, "acquireImage: SetupSgColl failed\n");
		return -1;
		}
		gettimeofday(&t1, NULL);
		t0 = vrpn_TimevalDiff(t1, t0);
		t_SetupSgColl = vrpn_TimevalMsecs(t0);
		
		// collect the image
		t_CollectSgLine = 0.0;
		t_reportScanlineData = 0.0;
		for (i = 0; i < d_resolution_y; i++){
			//printf("about to call CollectSgLine for line %d\n", i);
			// WARNING: I think this is where we block waiting for the data to
			// come in. This call has been known to block indefinitely in certain
			// situations such as when you don't call SetupSgColl()
			// right before this loop; I once attempted to not call it
			// every time since SetupSgColl looks like it is just a configuration
			// function but it didn't work very well
			gettimeofday(&t0, NULL);
			#ifdef UCHAR_PIXEL
			result = CollectSgLine(&(d_scanBuffer[i*d_resolution_x]));
			#else
			result = CollectSgLine(&(d_scanBuffer[2*i*d_resolution_x]));
			#endif
			//printf("finished collecting\n");
			if (result == diaginc_ERROR) {
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
#endif // not VIRTUAL_SEM
	} else { // assert(d_virtualAcquisition == true)
		// make some fake data:
		static int count = 0;
		
		//int dx, dy;
		for (i = 0; i < d_resolution_y; i++){
#ifdef UCHAR_PIXEL
			dy = i-d_resolution_y/4;
			if (dy < 0) dy = -dy;
			for (j = 0; j < d_resolution_x; j++){
				dx = j-d_resolution_x/3;
				if (dx < 0) dx = -dx;
				((vrpn_uint8 *)d_scanBuffer)[i*d_resolution_x + j] =
					((dx+dy+count)%(d_resolution_y/4))*255/
					(d_resolution_y/4);
			}
#else
			for (j = 0; j < d_resolution_x; j++){
				((vrpn_uint16 *)d_scanBuffer)[i*d_resolution_x + j] = 
					((i+(int)count)%d_resolution_y)*64000/d_resolution_y;
			}
#endif
			
			reportScanlineData(i);
			//d_connection->mainloop();
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
	*/
	return 0;
}


vrpn_int32 nmm_Microscope_SEM_diaginc::reportScanlineData(int line_num)
{
	/*
	// line data is at d_scanBuffer[line_num*d_resolution_x]
	char *msgbuf;
	vrpn_int32 len;
	struct timeval now;
#ifdef UCHAR_PIXEL
	void *data = &(d_scanBuffer[line_num*d_resolution_x]);
#else
	void *data = &(d_scanBuffer[2*line_num*d_resolution_x]);
#endif
	
	gettimeofday(&now, NULL);
	vrpn_int32 lines_per_message = d_lines_per_message;
	if (line_num + lines_per_message > d_resolution_y){
		lines_per_message = d_resolution_y - line_num;
	}

#ifdef UCHAR_PIXEL
	msgbuf = encode_ScanlineData(&len, 0, line_num, 1, 1,
		d_resolution_x, 1, lines_per_message, now.tv_sec, now.tv_usec,
		NMB_UINT8, &data);
#else
	msgbuf = encode_ScanlineData(&len, 0, line_num, 1, 1,
		d_resolution_x, 1, lines_per_message, now.tv_sec, now.tv_usec, 
		NMB_UINT16, &data);
#endif
	if (!msgbuf) { 
		return -1;
	}
	return dispatchMessage(len, msgbuf, d_ScanlineData_type);
	*/
	return 0;
}


vrpn_int32 nmm_Microscope_SEM_diaginc::reportMaxScanSpan()
{
/*
vrpn_int32 x, y;
x = d_xScanSpan;
y = d_yScanSpan;
char *msgbuf;
vrpn_int32 len;

  msgbuf = encode_ReportMaxScanSpan(&len, x, y);
  if (!msgbuf){
  return -1;
  }
  return dispatchMessage(len, msgbuf, d_ReportMaxScanSpan_type);
	*/
	return -1;
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
    me->reportMaxScanSpan();
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