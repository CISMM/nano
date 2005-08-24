
#include "Tcl_Linkvar.h"
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <PCO_err.h>
#include <SC2_defs.h>

// for text error messages
#define PCO_ERRT_H_CREATE_OBJECT
#include <PCO_errt.h>


#include <vrpn_Connection.h>
#include "nmm_Microscope_SEM_cooke.h"
#include "nmb_Image.h"
#include "OpticalServerInterface.h"
#include "edax_defs.h"


nmm_Microscope_SEM_cooke::
nmm_Microscope_SEM_cooke( const char * name, vrpn_Connection * c, vrpn_bool virtualAcq ) 
	: nmm_Microscope_SEM_optical( name, c, virtualAcq ),
	  d_scan_enabled(vrpn_FALSE), 
	  d_lines_per_message(1), 
	  d_scans_to_do(0),
	  d_virtualAcquisition(virtualAcq)
{
	// VRPN initialization
	if (!d_connection) {
		fprintf(stderr, "nmm_Microscope_SEM_cooke: Fatal Error: NULL connection\n");
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
	
	// set the 'size' paramater of the various Cooke structs to the expected size
	cameraGeneral.wSize = sizeof(cameraGeneral);
	cameraGeneral.strCamType.wSize = sizeof(cameraGeneral.strCamType);
	cameraType.wSize = sizeof(cameraType);
	cameraSensor.wSize = sizeof(cameraSensor);
	cameraSensor.strDescription.wSize = sizeof(cameraSensor.strDescription);
	cameraDescription.wSize = sizeof(cameraDescription);
	cameraTiming.wSize = sizeof(cameraTiming);
	cameraStorage.wSize = sizeof(cameraStorage);
	cameraRecording.wSize = sizeof(cameraRecording);

	// Cooke camera initialization
	if( !d_virtualAcquisition )
	{
		if( setupCamera( ) != PCO_NOERROR ) { return; }
	}
	
	// pretend-SEM initialization
	initializeParameterDefaults();
	
	// create buffers in which to store data
	vrpn_int32 maxExtents[2]; // width then height
	if( d_virtualAcquisition )
	{
		maxExtents[0] = EDAX_SCAN_MATRIX_X[EDAX_NUM_SCAN_MATRICES-1];
		maxExtents[1] = EDAX_SCAN_MATRIX_Y[EDAX_NUM_SCAN_MATRICES-1];
		currentResolutionIndex = 1;
	}
	else
	{
		int ret = this->getMaxResolution( maxExtents[0], maxExtents[1] );
		if( ret != PCO_NOERROR )
		{
			PCO_GetErrorText( ret, errorText, ERROR_TEXT_LEN );
			fprintf( stderr, "nmm_Microscope_SEM_cooke constructor:  Error querying "
				"the camera max. resolution.  Code:  0x%x (%s)\n", ret, errorText );
			return;
		}
	}
	maxBufferSize = maxExtents[0] * maxExtents[1] * 2;
	if( maxBufferSize % 0x1000 )
    {  // firewire interface needs the buffer to be (multiple of 4096) + 8192
      maxBufferSize = maxBufferSize / 0x1000;
      maxBufferSize += 2;
      maxBufferSize *= 0x1000;
    }
    else
      maxBufferSize += 0x1000;
    
	myImageBuffer = new vrpn_uint8[ maxBufferSize ];
	cameraImageBuffer = new vrpn_uint8[ maxBufferSize ];
	cameraBufferNumber = -1;  // -1 to allocate a new buffer
	cameraEvent = 0;  // 0 to allocate a new handle

	if( !d_virtualAcquisition )
	{
		int success = PCO_AllocateBuffer( camera, &cameraBufferNumber, maxBufferSize, 
										  (WORD**) &cameraImageBuffer, &cameraEvent );
		if( success != PCO_NOERROR )
		{
			PCO_GetErrorText( success, errorText, ERROR_TEXT_LEN );
			fprintf( stderr, "nmm_Microscope_SEM_cooke constructor:  Error allocating "
					"the image buffer.  Code:  0x%x (%s)\n", success, errorText );
			return;
		}
	}

	// no changes, yet
	requestedChanges.resolutionChanged 
		= requestedChanges.binningChanged 
		= requestedChanges.exposureChanged 
		= false;
}

nmm_Microscope_SEM_cooke::
~nmm_Microscope_SEM_cooke (void)
{
	OpticalServerInterface::getInterface()->setImage( NULL, 0, 0 );

	
	// let the camera clean up after itself
	PCO_SetRecordingState( camera, 0 );
	PCO_FreeBuffer( camera, cameraBufferNumber );
	PCO_CloseCamera( &camera );

	delete[] myImageBuffer;
	delete[] cameraImageBuffer;
}


vrpn_int32 nmm_Microscope_SEM_cooke::
setupCamera( )
{
	// open the PCO camera
	int success = PCO_OpenCamera( &camera, 0 /* board 0 ?*/ );
	if( success != PCO_NOERROR )
	{
		PCO_GetErrorText( success, errorText, ERROR_TEXT_LEN );
		fprintf( stderr, "nmm_Microscope_SEM_cooke::setupCamera:  Error opening "
			"the Cooke camera (open).  Code:  0x%x (%s)\n", success, errorText );
		return success;
	}

	// make sure the camera isn't recording
	unsigned short recstate = 0;
	success = PCO_GetRecordingState( camera, &recstate );
	if( success == PCO_NOERROR && recstate != 0 )
	{
		success = PCO_SetRecordingState( camera, 0 );
		if( success != PCO_NOERROR )
		{
			PCO_GetErrorText( success, errorText, ERROR_TEXT_LEN );
			fprintf( stderr, "nmm_Microscope_SEM_cooke::setupCamera:  Error telling"
				"the camera to stop recording.  Code:  0x%x (%s)\n", success, errorText );
			return success;
		}
	}

	// reset the camera settings to default
	success = PCO_ResetSettingsToDefault( camera );
	if( success != PCO_NOERROR )
	{
		PCO_GetErrorText( success, errorText, ERROR_TEXT_LEN );
		fprintf( stderr, "nmm_Microscope_SEM_cooke::setupCamera:  Error setting "
			"camera to default state.  Code:  0x%x (%s)\n", success, errorText );
		return success;
	}

	// validate settings
	success = PCO_ArmCamera( camera );
	if( success != PCO_NOERROR )
	{
		PCO_GetErrorText( success, errorText, ERROR_TEXT_LEN );
		fprintf( stderr, "nmm_Microscope_SEM_cooke::setupCamera:  Error "
			"validating settings.  Code:  0x%x (%s)\n", success, errorText );
		return success;
	}

	// get the camera information
	success = PCO_GetGeneral( camera, &cameraGeneral );
	if( success != PCO_NOERROR )
	{
		PCO_GetErrorText( success, errorText, ERROR_TEXT_LEN );
		fprintf( stderr, "nmm_Microscope_SEM_cooke::setupCamera:  Error getting "
			"camera general information.  Code:  0x%x (%s)\n", success, errorText );
		return success;
	}
	success = PCO_GetCameraType( camera, &cameraType );
	if( success != PCO_NOERROR )
	{
		PCO_GetErrorText( success, errorText, ERROR_TEXT_LEN );
		fprintf( stderr, "nmm_Microscope_SEM_cooke::setupCamera:  Error getting "
			"camera type.  Code:  0x%x (%s)\n", success, errorText );
		return success;
	}
	switch( cameraType.wCamType )
	{
    case CAMERATYPE_PCO1200HS:
		printf("PCO.Camera 1200 hs found \n");
		break;
    case CAMERATYPE_PCO1300:
		printf("PCO.Camera 1300 found \n");
		break;
    case CAMERATYPE_PCO1600:
		printf("PCO.Camera 1600 found \n");
		break;
    case CAMERATYPE_PCO2000:
		printf("PCO.Camera 2000 found \n");
		break;
    case CAMERATYPE_PCO4000:
		printf("PCO.Camera 4000 found \n");
		break;
    default:
		printf("PCO.Camera undefined type");
	}
	
	success = PCO_GetSensorStruct( camera, &cameraSensor );
	if( success != PCO_NOERROR )
	{
		PCO_GetErrorText( success, errorText, ERROR_TEXT_LEN );
		fprintf( stderr, "nmm_Microscope_SEM_cooke::setupCamera:  Error getting "
			"camera sensor information.  Code:  0x%x (%s)\n", success, errorText );
		return success;
	}
	success = PCO_GetCameraDescription( camera, &cameraDescription );
	if( success != PCO_NOERROR )
	{
		PCO_GetErrorText( success, errorText, ERROR_TEXT_LEN );
		fprintf( stderr, "nmm_Microscope_SEM_cooke::setupCamera:  Error getting "
			"camera description.  Code:  0x%x (%s)\n", success, errorText );
		return success;
	}
	standardMaxResX = cameraDescription.wMaxHorzResStdDESC;
	standardMaxResY = cameraDescription.wMaxVertResStdDESC;
	printf( "\tstandard resolution:  %d x %d\n", 
			cameraDescription.wMaxHorzResStdDESC, cameraDescription.wMaxVertResStdDESC );
	printf( "\tmaximum binning:  %d x %d\n",
			cameraDescription.wMaxBinHorzDESC, cameraDescription.wMaxBinVertDESC );
	printf( "\tbinning steps:  %s x %s\n",  cameraDescription.wBinHorzSteppingDESC == 1 ? "linear" : "exponential",
			cameraDescription.wBinVertSteppingDESC == 1 ? "linear" : "exponential" );

	success = PCO_GetTimingStruct( camera, &cameraTiming );
	if( success != PCO_NOERROR )
	{
		PCO_GetErrorText( success, errorText, ERROR_TEXT_LEN );
		fprintf( stderr, "nmm_Microscope_SEM_cooke::setupCamera:  Error getting "
			"camera timing information.  Code:  0x%x (%s)\n", success, errorText );
		return success;
	}
	success = PCO_GetRecordingStruct( camera, &cameraRecording );
	if( success != PCO_NOERROR )
	{
		PCO_GetErrorText( success, errorText, ERROR_TEXT_LEN );
		fprintf( stderr, "nmm_Microscope_SEM_cooke::setupCamera:  Error getting "
			"camera recording information.  Code:  0x%x (%s)\n", success, errorText );
		return success;
	}

	// see how many ADCs this is set up to use
	WORD adcs = 0;
	success = PCO_GetADCOperation( camera, &adcs );
	if( success != PCO_NOERROR )
	{
		PCO_GetErrorText( success, errorText, ERROR_TEXT_LEN );
		fprintf( stderr, "nmm_Microscope_SEM_cooke::setupCamera:  Error getting "
			"number of ADCs.  Code:  0x%x (%s)\n", success, errorText );
		return success;
	}
	fprintf( stdout, "\t using %d ADC(s).\n", adcs );

	// make sure double image mode if off
	// (per email from sufang zhao at cooke)
	success = PCO_SetDoubleImageMode( camera, 0 );
	if( success != PCO_NOERROR )
	{
		PCO_GetErrorText( success, errorText, ERROR_TEXT_LEN );
		fprintf( stderr, "nmm_Microscope_SEM_cooke::setupCamera:  Error insuring "
			"double-image mode is off.  Code:  0x%x (%s)\n", success, errorText );
		return success;
	}

	// set the sensor to use only standard pixels
	success = PCO_SetSensorFormat( camera, 0x0000 );
	if( success != PCO_NOERROR )
	{
		PCO_GetErrorText( success, errorText, ERROR_TEXT_LEN );
		fprintf( stderr, "nmm_Microscope_SEM_cooke::setupCamera:  Error setting "
			"sensor format.  Code:  0x%x (%s)\n", success, errorText );
		return success;
	}

	// set bin size
	short binning = 1;
	if( cameraDescription.wMaxBinHorzDESC >= 2 
		&& cameraDescription.wMaxBinVertDESC >= 2 )
		binning = 2;
	success = PCO_SetBinning( camera, binning, binning );
	if( success != PCO_NOERROR )
	{
		PCO_GetErrorText( success, errorText, ERROR_TEXT_LEN );
		fprintf( stderr, "nmm_Microscope_SEM_cooke::setupCamera:  Error setting "
			"binning to %d.  Code:  0x%x (%s)\n", binning, success, errorText );
		return success;
	}
	
	// set the capture area
	int res_x = EDAX_SCAN_MATRIX_X[EDAX_DEFAULT_SCAN_MATRIX];
	int res_y = EDAX_SCAN_MATRIX_Y[EDAX_DEFAULT_SCAN_MATRIX];
	int maxX = 0, maxY = 0;
	getMaxResolution( maxX, maxY );
	maxX = (int) floor( maxX / binning );
	maxY = (int) floor( maxY / binning );
	// check that the requested resolution isn't too big
	if( res_x > maxX || res_y > maxY )
	{
		fprintf( stderr, "nmm_Microscope_SEM_cooke::setupCamera:  "
			"resolution (%d x %d) x %d binning greater than max (%d x %d).\n",
			res_x, res_y, currentBinning, maxX, maxY );
		return -1;
	}

	WORD left = 1;
	WORD right = res_x;
	WORD top = 1;
	WORD bottom = res_y;
	int tx = (maxX - res_x) / 2, 
		ty = (maxY - res_y) / 2;
	left += tx;  right += tx;
	top += ty;  bottom += ty;
	// WORD left = 1 + 156, right = res_x + 156, top = 1 + 100, bottom = res_y + 100;
	fprintf( stdout, "setting ROI to %d x %d:  (%d,%d) to (%d,%d).  Max is %d x %d.\n",
			res_x, res_y, left, top, right, bottom, maxX, maxY );
	success = PCO_SetROI( camera, left, top, right, bottom );
	if( success != PCO_NOERROR )
	{
		PCO_GetErrorText( success, errorText, ERROR_TEXT_LEN );
		fprintf( stderr, "nmm_Microscope_SEM_cooke::setupCamera:  Error setting "
			"ROI to %d x %d:  (%d,%d) to (%d,%d).  Code:  0x%x (%s)\n", 
			res_x, res_y, left, top, right, bottom, success, errorText );
		return success;
	}
	success = PCO_CamLinkSetImageParameters( camera, res_x, res_y );
	if( success != PCO_NOERROR )
	{
		PCO_GetErrorText( success, errorText, ERROR_TEXT_LEN );
		fprintf( stderr, "nmm_Microscope_SEM_cooke::setupCamera:  Error setting "
			"CamLink board for images (%d x %d).  Code:  0x%x (%s)\n", 
			res_x, res_y, success, errorText );
		return success;
	}
	int actualX, actualY;
	getResolutionFromCamera( actualX, actualY );
	getMaxResolution( maxX, maxY );
	printf( "camera reports %d x %d resolution now.\n", actualX, actualY );
	printf( "  and %d x %d max resolution.\n", maxX, maxY );


	// set trigger mode
	// from the example code -- do we care?
	success = PCO_SetTriggerMode( camera, 0 /* auto-trigger */ );
	if( success != PCO_NOERROR )
	{
		PCO_GetErrorText( success, errorText, ERROR_TEXT_LEN );
		fprintf( stderr, "nmm_Microscope_SEM_cooke::setupCamera:  Error "
			"setting trigger mode.  Code:  0x%x (%s)\n", success, errorText );
		return success;
	}

	// set an exposure time
	this->currentExposure = 100;
	WORD timebase = 2;  // Timebase: 0-ns; 1-us; 2-ms  
	success = PCO_SetDelayExposureTime( camera, 
										0, // delay
										this->currentExposure, 
										2, // Timebase: 0-ns; 1-us; 2-ms 
										2 // Timebase: 0-ns; 1-us; 2-ms 
										);
	if( success != PCO_NOERROR )
	{
		fprintf( stderr, "nmm_Microscope_SEM_cooke::setupCamera:  Error setting "
			"exposure.  Code:  0x%x\n", success );
		return success;
	}

	// validate settings
	success = PCO_ArmCamera( camera );
	if( success != PCO_NOERROR )
	{
		PCO_GetErrorText( success, errorText, ERROR_TEXT_LEN );
		fprintf( stderr, "nmm_Microscope_SEM_cooke::setupCamera:  Error "
			"validating settings.  Code:  0x%x (%s)\n", success, errorText );
		return success;
	}
	printf( "after arming, camera reports %d x %d resolution now.\n", actualX, actualY );
	printf( "  and %d x %d max resolution.\n", maxX, maxY );

	// start recording
	success = PCO_SetRecordingState( camera, 1 );
	if( success != PCO_NOERROR )
	{
		PCO_GetErrorText( success, errorText, ERROR_TEXT_LEN );
		fprintf( stderr, "nmm_Microscope_SEM_cooke::acquireImage:  Error starting "
				"recording.  Code:  0x%x (%s)\n", success, errorText );
		return success;
	}


	// ask for 8 bits per pixel
	
	// turn off auto-exposure
	
	// add our callback

	// Set the color to monochrome mode (turn off filter wheels)
	
	return 0;
}


vrpn_int32 nmm_Microscope_SEM_cooke::
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
	doRequestedChangesOnCooke( );
		acquireImage();
	//} else {
	//}
	return 0;
}

vrpn_int32 nmm_Microscope_SEM_cooke::
initializeParameterDefaults()
{
/*
int file_found = 0;
int i;

  // hard-coded defaults:
  d_resolution_index = cooke_DEFAULT_SCAN_MATRIX;
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
	  d_blankMode = cooke_DEFAULT_BLANK_MODE;
	  d_beam_location_x = 0;
	  d_beam_location_y = 0;
	  
		d_external_scan_control_enabled = 0;
		
		  d_scanType = cooke_FAST_SCAN;
		  d_dataTransfer = cooke_DATA_TRANSFER_BYTE;
		  d_magnification = 1000;
		  
			d_image_mode_settings_changed = vrpn_TRUE;
			d_point_mode_settings_changed = vrpn_TRUE;
			d_shared_settings_changed = vrpn_TRUE;
	*/
	return 0;
}



vrpn_int32 nmm_Microscope_SEM_cooke::
getResolution( vrpn_int32 &res_x, vrpn_int32 &res_y )
{
	int x = 0, y = 0;
	indexToResolution( currentResolutionIndex, x, y );
	res_x = x;  res_y = y;
	return 0;
}



vrpn_int32 nmm_Microscope_SEM_cooke::
getResolutionFromCamera( vrpn_int32 &res_x, vrpn_int32 &res_y )
{
	if( d_virtualAcquisition )
	{
		getResolution( res_x, res_y );
	}
	else
	{
		WORD left, right, top, bottom;
		int ret = PCO_GetROI( camera, &left, &top, &right, &bottom );
		if( ret != PCO_NOERROR )
		{
			PCO_GetErrorText( ret, errorText, ERROR_TEXT_LEN );
			fprintf( stderr, "nmm_Microscope_SEM_cooke::getResolutionFromCamera:  Error "
					"Code:  0x%x (%s)\n", ret, errorText );
			res_x = -1;  res_y = -1;
			return ret;
		}
		res_x = right - left + 1;
		res_y = bottom - top + 1;
	}

	int resInd = resolutionToIndex( res_x, res_y );
	if( resInd >= 0 )
	{  currentResolutionIndex = resInd;  }
	else 
	{  
		// how did it go so very, very wrong??
		printf( "Internal error in nmm_Microscope_SEM_cooke::getResolutionFromCamera:  "
				"camera and internal resolution don't match.\n" );
	}
	return 0;
}


vrpn_int32 nmm_Microscope_SEM_cooke::
getBinning(  )
{
	if( d_virtualAcquisition )
	{
		return currentBinning;
	}
	else
	{
		WORD binx, biny;
		int ret = PCO_GetBinning( camera, &binx, &biny );
		if( ret != PCO_NOERROR )
		{
			PCO_GetErrorText( ret, errorText, ERROR_TEXT_LEN );
			fprintf( stderr, "nmm_Microscope_SEM_cooke::getBinning:  Error "
					"Code:  0x%x (%s)\n", ret, errorText );
			return -1;
		}
		
		if( binx != this->currentBinning )
		{
			OpticalServerInterface* iface = OpticalServerInterface::getInterface( );
			if( iface != NULL ) iface->setBinning( binx );
			currentBinning = binx;
		}
	}
	return currentBinning;
}


vrpn_int32 nmm_Microscope_SEM_cooke::
getMaxResolution( vrpn_int32& x, vrpn_int32& y )
{
	if( d_virtualAcquisition )
	{
		x = EDAX_SCAN_MATRIX_X[EDAX_NUM_SCAN_MATRICES-1];
		y = EDAX_SCAN_MATRIX_Y[EDAX_NUM_SCAN_MATRICES-1];
	}
	else
	{
		x = standardMaxResX;
		y = standardMaxResY;
		/*
		// it turns out that PCO_GetSizes returned bogus values
		// for max resolution, either b/c of sdk bugs, or 
		// firmware in need of upgrade, or the CameraLink i-face.
		// use the "standard max res" values from GetCameraDescription
		// instead.
		//  as of 8/24/05, dtm
		WORD xreal, yreal, xmax, ymax;
		int ret = PCO_GetSizes( camera, &xreal, &yreal, &xmax, &ymax );
		if( ret != PCO_NOERROR )
		{
			PCO_GetErrorText( ret, errorText, ERROR_TEXT_LEN );
			fprintf( stderr, "nmm_Microscope_SEM_cooke::getMaxResolution:  Error "
				"Code:  0x%x (%s)\n", ret, errorText );
			return -1;
		}
		x = xmax;
		y = ymax;
		*/
	}
	return 0;
}


vrpn_int32 nmm_Microscope_SEM_cooke::
setResolutionByIndex( vrpn_int32 idx )
{
	if( idx < 0 || idx > EDAX_NUM_SCAN_MATRICES - 1 )
	{
		fprintf( stderr, "nmm_Microscope_SEM_cooke::setResolutionbyIndex:  "
			"invalid resolution requested:  %d\n", idx );
		OpticalServerInterface::getInterface()->setResolutionIndex( currentResolutionIndex );	
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


vrpn_int32 nmm_Microscope_SEM_cooke::
setBinning( vrpn_int32 bin )
{
	int retVal = PCO_NOERROR;

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


vrpn_int32 nmm_Microscope_SEM_cooke::
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


vrpn_int32 nmm_Microscope_SEM_cooke::
reportResolution()
{
	char *msgbuf;
	vrpn_int32 len;
	vrpn_int32 x, y;
	
	vrpn_int32 ret = getResolution( x, y );
	if( ret != 0 ) 
	{ 
		fprintf( stderr, "nmm_Microscope_SEM_cooke::reportResolution:  "
			"unable to get resolution from the SPOT camera.  Code:  0x%x\n", ret );
		return ret; 
	}
	
	msgbuf = encode_ReportResolution(&len, x, y);
	if (!msgbuf)
	{
		return -1;
	}
	
	return dispatchMessage(len, msgbuf, d_ReportResolution_type);
}


vrpn_int32 nmm_Microscope_SEM_cooke::
requestScan(vrpn_int32 nscans)
{
    if (nscans == 0) {
        d_scan_enabled = vrpn_FALSE;
        d_scans_to_do = 0;
    } else {
        d_scan_enabled = vrpn_TRUE;
        d_scans_to_do = 1;
		//acquireImage();
		//d_scans_to_do--;
    }
    return 0;
}



vrpn_bool nmm_Microscope_SEM_cooke::
scanEnabled()
{
	return d_scan_enabled;
}



vrpn_int32 nmm_Microscope_SEM_cooke::
getScanRegion_nm( double &x_span_nm, double &y_span_nm )
{
/*
x_span_nm = SEM_STANDARD_DISPLAY_WIDTH_NM/(double)d_magnification;
y_span_nm = x_span_nm*(double)d_resolution_y/(double)d_resolution_x;
	*/
	return -1;
}

vrpn_int32 nmm_Microscope_SEM_cooke::
getMaxScan(int &x_span_DAC, int &y_span_DAC)
{
/*
x_span_DAC = d_xScanSpan;
y_span_DAC = d_yScanSpan;
	*/
	return -1;
}


void nmm_Microscope_SEM_cooke::
printCookeValues( )
{
	/*
	// auto-expose

	// binning

	// color enable

	// color enable 2

	// exposure times

	*/
}


vrpn_int32 nmm_Microscope_SEM_cooke::
acquireImage()
{
	int i, j;
	
	int result = PCO_NOERROR;
	int resX = 0, resY = 0;
	result = this->getResolutionFromCamera( resX, resY );
	if( result != PCO_NOERROR )
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
			if( d_scans_to_do > 0 )  reportScanlineData(i);
		}
		count++;
		if( d_scans_to_do > 0 )  d_scans_to_do--;
	}
	else // the real thing
	{
		// make sure the camera is recording
		PCO_ArmCamera( camera );
		PCO_SetRecordingState( camera, 1 );

		int success = PCO_AddBufferEx( camera, 0, 0, cameraBufferNumber, resX, resY, 16 );
		if( success != PCO_NOERROR )
		{
			PCO_GetErrorText( success, errorText, ERROR_TEXT_LEN );
			fprintf( stderr, "nmm_Microscope_SEM_cooke::acquireImage:  Error adding "
				"the image buffer.  Code:  0x%x (%s)\n", success, errorText );
			return success;
		}
		
		success = WaitForSingleObject( cameraEvent, 500 ); // Wait until picture arrives
		ResetEvent( cameraEvent );
		if( success != WAIT_OBJECT_0 )
		{
			fprintf( stderr, "nmm_Microscope_SEM_cooke::acquireImage:  Error adding "
				"the image buffer.  Code:  0x%x\n", success );
			return -1;
		}

		WORD max = 0xFF00 >> currentContrast;
		WORD value = 0;
		for( int row = 0; row < resY; row++ )
		{
			int line = row * resX;
			for( int col = 0; col < resX; col++ )
			{
				value = cameraImageBuffer[2*(line+col)+1];
				if( value > max )
					myImageBuffer[line+col] = 0xFF;
				else
					myImageBuffer[line+col] 
					= (vrpn_uint8) ( value << currentContrast );
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

	OpticalServerInterface::getInterface()->setImage( myImageBuffer, resX, resY );
	
	return 0;
}


vrpn_int32 nmm_Microscope_SEM_cooke::reportScanlineData(int line_num)
{
	vrpn_int32 resX = 0, resY = 0;
	if( this->getResolution( resX, resY ) != 0 )  { return -1; }
	char *msgbuf;
	vrpn_int32 len;
	void* data = &(myImageBuffer[ line_num * resX ]);
	
	vrpn_int32 lines_per_message = d_lines_per_message;
	if (line_num + lines_per_message > resY){
		lines_per_message = resY - line_num;
	}

	struct timeval now;
	gettimeofday( &now, NULL );  // also used for encoding, not just timing
	msgbuf = encode_ScanlineData(&len, 0, line_num, 1, 1,
		resX, 1, lines_per_message, now.tv_sec, now.tv_usec,
		NMB_UINT8, &data);
	if (!msgbuf) { 
		return -1;
	}
	return dispatchMessage(len, msgbuf, d_ScanlineData_type);
}


vrpn_int32 nmm_Microscope_SEM_cooke::reportMaxScanSpan()
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
int VRPN_CALLBACK nmm_Microscope_SEM_cooke::RcvSetResolution 
(void *_userdata, vrpn_HANDLERPARAM _p)
{
	nmm_Microscope_SEM_cooke *me = (nmm_Microscope_SEM_cooke *)_userdata;
	const char * bufptr = _p.buffer;
	vrpn_int32 res_x, res_y;
	
	if (decode_SetResolution(&bufptr, &res_x, &res_y) == -1) {
		fprintf(stderr, 
			"nmm_Microscope_SEM_cooke::RcvSetResolution: decode failed\n");
		return -1;
	}
	if (me->setResolution(res_x, res_y) == -1) {
		fprintf(stderr, "nmm_Microscope_SEM_cooke::RcvSetResolution: set failed\n");
		return -1;
	}
	return 0;
}


//static 
int VRPN_CALLBACK nmm_Microscope_SEM_cooke::RcvRequestScan
(void *_userdata, vrpn_HANDLERPARAM _p)
{
	nmm_Microscope_SEM_cooke *me = (nmm_Microscope_SEM_cooke *)_userdata;
	const char * bufptr = _p.buffer;
	vrpn_int32 nscans;
	
	if (decode_RequestScan(&bufptr, &nscans) == -1) {
		fprintf(stderr,
			"nmm_Microscope_SEM_cooke::RcvRequestScan: decode failed\n");
		return -1;
	}
	if (me->requestScan(nscans) == -1) {
		fprintf(stderr, "nmm_Microscope_SEM_cooke::RcvRequestScan: set failed\n");
		return -1;
	}
	return 0;
}



//static
int VRPN_CALLBACK nmm_Microscope_SEM_cooke::RcvGotConnection
(void *_userdata, vrpn_HANDLERPARAM _p)
{
    nmm_Microscope_SEM_cooke *me = 
		(nmm_Microscope_SEM_cooke *)_userdata;
	
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
int VRPN_CALLBACK nmm_Microscope_SEM_cooke::RcvDroppedConnection
(void *_userdata, vrpn_HANDLERPARAM _p)
{
	nmm_Microscope_SEM_cooke *me =
		(nmm_Microscope_SEM_cooke *)_userdata;
	// me->closeScanControlInterface();
	return 0;
}


// a helper function to change various parameters in a
// way synchronized with image capture.
void nmm_Microscope_SEM_cooke::
doRequestedChangesOnCooke( )
{
	if( !requestedChanges.binningChanged
		&& !requestedChanges.exposureChanged
		&& !requestedChanges.resolutionChanged )
	{
		return;
	}

	// make sure the camera isn't recording
	unsigned short recstate = 0;
	int success = PCO_GetRecordingState( camera, &recstate );
	if( success == PCO_NOERROR && recstate != 0 )
	{
		success = PCO_SetRecordingState( camera, 0 );
		if( success != PCO_NOERROR )
		{
			PCO_GetErrorText( success, errorText, ERROR_TEXT_LEN );
			fprintf( stderr, "nmm_Microscope_SEM_cooke::doRequestedChangesOnCooke:  Error telling"
				"the camera to stop recording.  Code:  0x%x (%s)\n", success, errorText );
			return;
		}
	}

	
	if( requestedChanges.binningChanged )
	{
		requestedChanges.binningChanged = false;

		int newBin = requestedChanges.newBinning;

		// check that the current resolution is reasonable with the new binning
		if( newBin > currentBinning )
		{
			double resFactor = (double) newBin / (double) currentBinning;
			int resX = 0, resY = 0, maxX = 0, maxY = 0;
			int retVal = this->getResolutionFromCamera( resX, resY );
			if( retVal != PCO_NOERROR )
			{
				fprintf( stderr, "nmm_Microscope_SEM_cooke::doRequestedChangesOnCooke:  "
					"(binning) error fixing the resolution (1).\n" );
				OpticalServerInterface::getInterface()->setBinning( currentBinning );
				return;
			}
			retVal = this->getMaxResolution( maxX, maxY );
			if( retVal != PCO_NOERROR )
			{
				fprintf( stderr, "nmm_Microscope_SEM_cooke::doRequestedChangesOnCooke:  "
					"(binning) error fixing the resolution (2).\n" );
				OpticalServerInterface::getInterface()->setBinning( currentBinning );
				return;
			}
			if( resX * resFactor > maxX || resY * resFactor > maxY )
			{
				fprintf( stderr, "nmm_Microscope_SEM_cooke::doRequestedChangesOnCooke:  "
					"required area too large for camera: (%d x %d) x %f needed.  "
					"(%d x %d) available.\n", resX, resY, resFactor, maxX, maxY );
				OpticalServerInterface::getInterface()->setBinning( currentBinning );
				return;
			}
		}

		short binAsShort = newBin;
		success = PCO_SetBinning( camera, binAsShort, binAsShort );
		if( success != PCO_NOERROR )
		{
			PCO_GetErrorText( success, errorText, ERROR_TEXT_LEN );
			fprintf( stderr, "nmm_Microscope_SEM_cooke::doRequestedChangesOnCooke:  Error setting "
					"binning to %d.  Code:  0x%x (%s)\n", binAsShort, success, errorText );
			OpticalServerInterface::getInterface()->setBinning( currentBinning );
			return;
		}

		// reset the ROI
		int res_x = EDAX_SCAN_MATRIX_X[ this->getResolutionIndex() ];
		int res_y = EDAX_SCAN_MATRIX_Y[ this->getResolutionIndex() ];
		int maxX = 0, maxY = 0;
		getMaxResolution( maxX, maxY );
		maxX = (int) floor( maxX / newBin );
		maxY = (int) floor( maxY / newBin );
		// check that the current resolution isn't too big
		if( res_x > maxX || res_y > maxY )
		{
			fprintf( stderr, "nmm_Microscope_SEM_cooke::doRequestedChangesOnCooke:  "
				"error repositioning the ROI at resolution (%d x %d) x %d binning:  "
				"greater than max (%d x %d).\n",
				res_x, res_y, currentBinning, maxX, maxY );
			return;
		}
		
		WORD left = 1;
		WORD right = res_x;
		WORD top = 1;
		WORD bottom = res_y;
		int tx = (maxX - res_x) / 2, 
			ty = (maxY - res_y) / 2;
		left += tx;  right += tx;
		top += ty;  bottom += ty;
		success = PCO_SetROI( camera, left, top, right, bottom );
		if( success != PCO_NOERROR )
		{
			PCO_GetErrorText( success, errorText, ERROR_TEXT_LEN );
			fprintf( stderr, "nmm_Microscope_SEM_cooke::doRequestedChangesOnCooke:  Error setting "
				"ROI to %d x %d:  (%d,%d) to (%d,%d).  Code:  0x%x (%s)\n", 
				res_x, res_y, left, top, right, bottom, success, errorText );
			return;
		}
		
		// validate settings
		success = PCO_ArmCamera( camera );
		if( success != PCO_NOERROR )
		{
			PCO_GetErrorText( success, errorText, ERROR_TEXT_LEN );
			fprintf( stderr, "nmm_Microscope_SEM_cooke::doRequestedChangesOnCooke:  Error "
				"validating settings (arm).  Code:  0x%x (%s)\n", success, errorText );
			OpticalServerInterface::getInterface()->setBinning( currentBinning );
			return;
		}
		
		vrpn_int32 binning = getBinning( );
		OpticalServerInterface::getInterface()->setBinning( binning );
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
			fprintf( stderr, "nmm_Microscope_SEM_cooke::doRequestedChangesOnSpot:  "
				"internal error (resolution).\n" );
			reportResolution( );
			OpticalServerInterface::getInterface()->setResolutionIndex( currentResolutionIndex );	
			return;
		}
		maxX = floor( maxX / currentBinning );
		maxY = floor( maxY / currentBinning );

		// get the old ROI in case we need to revert
		WORD oldLeft, oldRight, oldTop, oldBottom;
		success = PCO_GetROI( camera, &oldLeft, &oldTop, &oldRight, &oldBottom );
		if( success != PCO_NOERROR )
		{
			PCO_GetErrorText( success, errorText, ERROR_TEXT_LEN );
			fprintf( stderr, "nmm_Microscope_SEM_cooke::doRequestedChangesOnCooke:  Error "
				"getting ROI.  Code:  0x%x (%s)\n", success, errorText );
			reportResolution( );
			OpticalServerInterface::getInterface()->setResolutionIndex( currentResolutionIndex );
			return;
		}

		// check that the requested resolution isn't too big
		if( res_x > maxX || res_y > maxY )
		{
			fprintf( stderr, "nmm_Microscope_SEM_cooke::doRequestedChangesOnSpot:  "
				"resolution (%d x %d) x %d greater than max (%d x %d).\n",
				res_x, res_y, currentBinning, maxX, maxY );
			reportResolution( );
			OpticalServerInterface::getInterface()->setResolutionIndex( currentResolutionIndex );	
			return;
		}
		
		// request the new resolution.  the requested area is centered in 
		// the camera's capture area.
		WORD left = 1;
		WORD right = res_x;
		WORD top = 1;
		WORD bottom = res_y;
		int tx = (maxX - res_x) / 2, 
			ty = (maxY - res_y) / 2;
		left += tx;  right += tx;
		top += ty;  bottom += ty;
		success = PCO_SetROI( camera, left, top, right, bottom );
		if( success != PCO_NOERROR )
		{
			PCO_GetErrorText( success, errorText, ERROR_TEXT_LEN );
			fprintf( stderr, "nmm_Microscope_SEM_cooke::doRequestedChangesOnCooke:  Error setting "
				"ROI to %d x %d:  (%d,%d) to (%d,%d).  Code:  0x%x (%s)\n", 
				res_x, res_y, left, top, right, bottom, success, errorText );
			reportResolution( );
			OpticalServerInterface::getInterface()->setResolutionIndex( currentResolutionIndex );
			return;
		}

		// validate settings
		success = PCO_ArmCamera( camera );
		if( success != PCO_NOERROR )
		{
			PCO_GetErrorText( success, errorText, ERROR_TEXT_LEN );
			fprintf( stderr, "nmm_Microscope_SEM_cooke::doRequestedChangesOnCooke:  Error "
				"validating settings (arm).  Code:  0x%x (%s)\n", success, errorText );
			// try to go back to the old ROI
			PCO_SetROI( camera, oldLeft, oldTop, oldRight, oldBottom );
			reportResolution( );
			OpticalServerInterface::getInterface()->setResolutionIndex( currentResolutionIndex );
			return;
		}
		
		success = PCO_CamLinkSetImageParameters( camera, res_x, res_y );
		if( success != PCO_NOERROR )
		{
			PCO_GetErrorText( success, errorText, ERROR_TEXT_LEN );
			fprintf( stderr, "nmm_Microscope_SEM_cooke::doRequestedChangesOnCooke:  Error setting "
				"CamLink board for images (%d x %d).  Code:  0x%x (%s)\n", 
				res_x, res_y, success, errorText );
			// try to go back to the old ROI
			PCO_SetROI( camera, oldLeft, oldTop, oldRight, oldBottom );
			reportResolution( );
			OpticalServerInterface::getInterface()->setResolutionIndex( currentResolutionIndex );
			return;
		}
		currentResolutionIndex = requestedChanges.newResolutionIndex;
		
		reportResolution( );
		OpticalServerInterface::getInterface()->setResolutionIndex( currentResolutionIndex );	
	}
	
	
	if( requestedChanges.exposureChanged )
	{
		requestedChanges.exposureChanged = false;
		
		success = PCO_SetDelayExposureTime( camera, 
			0, // delay
			requestedChanges.newExposure, 
			2, // Timebase: 0-ns; 1-us; 2-ms 
			2 // Timebase: 0-ns; 1-us; 2-ms 
			);
		if( success != PCO_NOERROR )
		{
			fprintf( stderr, "nmm_Microscope_SEM_cooke::setupCamera:  Error setting "
				"exposure.  Code:  0x%x\n", success );
			OpticalServerInterface::getInterface()->setExposure( currentExposure );
			return;
		}
		// validate settings
		success = PCO_ArmCamera( camera );
		if( success != PCO_NOERROR )
		{
			PCO_GetErrorText( success, errorText, ERROR_TEXT_LEN );
			fprintf( stderr, "nmm_Microscope_SEM_cooke::doRequestedChangesOnCooke:  Error "
				"validating settings (arm).  Code:  0x%x (%s)\n", success, errorText );
			// try to go back to the old settings
			PCO_SetDelayExposureTime( camera, 0, currentExposure, 2, 2 );
			OpticalServerInterface::getInterface()->setExposure( currentExposure );
			return;
		}
		
		currentExposure = requestedChanges.newExposure;
		OpticalServerInterface::getInterface()->setExposure( currentExposure );
	}

	// start recording
	success = PCO_SetRecordingState( camera, 1 );
	if( success != PCO_NOERROR )
	{
		PCO_GetErrorText( success, errorText, ERROR_TEXT_LEN );
		fprintf( stderr, "nmm_Microscope_SEM_cooke::acquireImage:  Error starting "
				"recording.  Code:  0x%x (%s)\n", success, errorText );
		return;
	}


}



//////////////////////////////////////////////////
// bogus functions so that we appear to be an SEM
/////////
vrpn_int32 nmm_Microscope_SEM_cooke::reportPixelIntegrationTime()
{
  char *msgbuf;
  vrpn_int32 len;

  msgbuf = encode_ReportPixelIntegrationTime(&len, -1);
  if (!msgbuf){
    return -1;
  }

  return dispatchMessage(len, msgbuf, d_ReportPixelIntegrationTime_type);
}


vrpn_int32 nmm_Microscope_SEM_cooke::reportInterPixelDelayTime()
{
  char *msgbuf;
  vrpn_int32 len;

  msgbuf = encode_ReportInterPixelDelayTime(&len, 0);
  if (!msgbuf){
    return -1;
  }

  return dispatchMessage(len, msgbuf, d_ReportInterPixelDelayTime_type);
}


vrpn_int32 nmm_Microscope_SEM_cooke::reportPointDwellTime()
{
  char *msgbuf;
  vrpn_int32 len;

  msgbuf = encode_ReportPointDwellTime(&len, 0);
  if (!msgbuf){
    return -1;
  }

  return dispatchMessage(len, msgbuf, d_ReportPointDwellTime_type);
}


vrpn_int32 nmm_Microscope_SEM_cooke::reportBeamBlankEnable()
{
  char *msgbuf;
  vrpn_int32 len;

  msgbuf = encode_ReportBeamBlankEnable(&len, 0);
  if (!msgbuf){
    return -1;
  }
  
  return dispatchMessage(len, msgbuf, d_ReportBeamBlankEnable_type);
}


vrpn_int32 nmm_Microscope_SEM_cooke::reportMagnification()
{
  char *msgbuf;
  vrpn_int32 len;

  msgbuf = encode_ReportMagnification(&len, 1);
  if (!msgbuf){
    return -1;
  }

  return dispatchMessage(len, msgbuf, d_ReportMagnification_type);
}


vrpn_int32 nmm_Microscope_SEM_cooke::reportExternalScanControlEnable()
{
  char *msgbuf;
  vrpn_int32 len;
 
  msgbuf = encode_ReportExternalScanControlEnable(&len, 0);
  if (!msgbuf){
    return -1;
  }
  
  return dispatchMessage(len, msgbuf, d_ReportExternalScanControlEnable_type);
}


vrpn_int32 nmm_Microscope_SEM_cooke::reportDACParams()
{
  char *msgbuf;
  vrpn_int32 len;

  msgbuf = encode_ReportDACParams(&len, 0, 0, 0, 0, 0, 0);
  if (!msgbuf){
    return -1;
  }

  return dispatchMessage(len, msgbuf, d_ReportDACParams_type);
}


vrpn_int32 nmm_Microscope_SEM_cooke::reportRetraceDelays()
{
  char *msgbuf;
  vrpn_int32 len;

  msgbuf = encode_ReportRetraceDelays(&len, 0, 0);
  if (!msgbuf){
    return -1;
  }

  return dispatchMessage(len, msgbuf, d_ReportRetraceDelays_type);
}
