#include <vrpn_Connection.h>
#include "nmm_Microscope_SEM_EDAX.h"
#include "nmm_EDAX.h"
#include "nmb_Image.h"
#include <stdlib.h>
#include <string.h>

#include "exposureManager.h"
#include "exposurePattern.h"

//#define USE_COLUMN_AND_STAGE
//#define USE_SCAN_TABLE
#define USE_SET_SCAN_PARAMS
#define USE_BUSYWAIT_DELAY
#define UCHAR_PIXEL
//#define VIRTUAL_SEM

#ifndef VIRTUAL_SEM
extern "C" {
#include "imgboard.h"
#include "semdef.h"

extern short int EDXCALL read_sem_params(char binary, 
                      char *fileName, PEDI32_SEM_REC edi32SgCfg);

#include "clmctrl.h"
#include "stgctrl.h"

}
#endif

nmm_Microscope_SEM_EDAX::nmm_Microscope_SEM_EDAX 
    (const char * name, vrpn_Connection * c, vrpn_bool virtualAcq) :
    nmb_Device_Server(name, c),
    nmm_Microscope_SEM(name, d_connection), 
    d_scan_enabled(vrpn_FALSE), 
    d_lines_per_message(1), 
    d_scans_to_do(0),
    d_virtualAcquisition(virtualAcq),
    d_exposureManager(new ExposureManager()),
    d_magCalibration(1e8), // (10 cm)*(nm/cm)
    d_beamCurrent_picoAmps(0.0),
    d_beamWidth_nm(0.0)
{
  if (!d_connection) {
    fprintf(stderr, "nmm_Microscope_SEM_EDAX: Fatal Error: NULL connection\n");
    return;
  }
  d_connection->register_handler(d_SetResolution_type,
                RcvSetResolution, this);
  d_connection->register_handler(d_SetPixelIntegrationTime_type,
                RcvSetPixelIntegrationTime, this);
  d_connection->register_handler(d_SetInterPixelDelayTime_type,
                RcvSetInterPixelDelayTime, this);
  d_connection->register_handler(d_RequestScan_type,
                RcvRequestScan, this);
  d_connection->register_handler(d_SetPointDwellTime_type,
                RcvSetPointDwellTime, this);
  d_connection->register_handler(d_SetBeamBlankEnable_type,
                RcvSetBeamBlankEnable, this);
  d_connection->register_handler(d_GoToPoint_type,
                RcvGoToPoint, this);
  d_connection->register_handler(d_SetRetraceDelays_type,
                RcvSetRetraceDelays, this);
  d_connection->register_handler(d_SetDACParams_type,
                RcvSetDACParams, this);
  d_connection->register_handler(d_SetExternalScanControlEnable_type,
                RcvSetExternalScanControlEnable, this);
  d_connection->register_handler(d_ClearExposePattern_type,
                RcvClearExposePattern, this);
  d_connection->register_handler(d_AddPolygon_type,
                RcvAddPolygon, this);
  d_connection->register_handler(d_AddPolyline_type,
                RcvAddPolyline, this);
  d_connection->register_handler(d_AddDumpPoint_type,
                RcvAddDumpPoint, this);
  d_connection->register_handler(d_ExposePattern_type,
                RcvExposePattern, this);
  d_connection->register_handler(d_SetBeamCurrent_type,
                RcvSetBeamCurrent, this);
  d_connection->register_handler(d_SetBeamWidth_type,
                RcvSetBeamWidth, this);

  int connect_id = d_connection->register_message_type(vrpn_got_connection);
  int disconnect_id = 
	d_connection->register_message_type(vrpn_dropped_connection);
  d_connection->register_handler(connect_id,
				RcvGotConnection, this);
  d_connection->register_handler(disconnect_id,
                                RcvDroppedConnection, this);

  initializeParameterDefaults();
  // create buffer in which to store data
#ifdef UCHAR_PIXEL
  d_scanBuffer = new vrpn_uint8[d_resolution_x*d_resolution_y];
#else
  d_scanBuffer = new vrpn_uint8[2*d_resolution_x*d_resolution_y];
#endif
  if (!d_scanBuffer){
	fprintf(stderr, "Error, out of memory allocating image buffer %dx%d\n",
		d_resolution_x, d_resolution_y);
	exit(-1);
  }
  openEDAXHardware();

  // The following is a test of column/stage control
#ifndef VIRTUAL_SEM

#if 0
  long magVal;

  // ***************
  // column controls
  // ***************
  
  result = GetMag(magVal);
  printf("GetMag returned %d\n", result);
  printf("magnification: %lu\n", magVal);
  result = SetMag(magVal);
  printf("SetMag returned %d\n", result);


  float fval;
  int ival;
  result = GetBright(fval);
  printf("GetBright returned %d\n", result);
  printf("brightness: %g\n", fval);
  result = GetCon(fval);
  printf("GetCon returned %d\n", result);
  printf("contrast: %g\n", fval);
  
  result = GetAccKv(fval);
  printf("GetAccKv returned %d\n", result);
  printf("AccKv: %g\n", fval);
  result = SetAccKv(fval);
  printf("SetAccKv returned %d\n", result);

  result = GetAccOnOff(ival);
  printf("GetAccOnOff returned %d\n", result);
  printf("AccOnOff: %d\n", ival);
  result = SetAccOnOff(ival);
  printf("SetAccOnOff returned %d\n", result);

  result = GetSpotSize(fval);
  printf("GetSpotSize returned %d\n", result);
  printf("Spot size: %g\n", fval);
  result = SetSpotSize(fval);
  printf("GetSpotSize returned %d\n", result);

  result = GetWorkDist(fval);
  printf("GetWorkDist returned %d\n", result);
  printf("Working distance: %g mm\n", fval);
  result = SetWorkDist(fval);
  printf("SetWorkDist returned %d\n", result);

//  result = DisplayColumnError
// result = GetClmDef

  CLMVECT minLim, maxLim;
  result = GetClmLimits(minLim, maxLim);
  printf("GetClmLimits returned %d\n", result);
  printf("Limits:\n");
  printf("  kv: (%g, %g)\n", minLim.kv, maxLim.kv);
  printf("  mag: (%ld, %ld)\n", minLim.mag, maxLim.mag);
  printf("  spot: (%g, %g)\n", minLim.spot, maxLim.spot);
  printf("  wd: (%g, %g)\n", minLim.wd, maxLim.wd);
  printf("  contr: (%g, %g)\n", minLim.contr, maxLim.contr);
  printf("  bright: (%g, %g)\n", minLim.bright, maxLim.bright);
  printf("  scanType: (%d, %d)\n", minLim.scanType, maxLim.scanType);
  printf("  scanLines: (%d, %d)\n", minLim.scanLines, maxLim.scanLines);
  printf("  scanTime: (%d, %d)\n", minLim.scanTime, maxLim.scanTime);
  printf("  beamOnOf: (%d, %d)\n", minLim.beamOnOf, maxLim.beamOnOf);
  printf("  kvOnOf: (%d, %d)\n", minLim.kvOnOf, maxLim.kvOnOf);
  printf("  detIndex: (%d, %d)\n", minLim.detIndex, maxLim.detIndex);

/*
  // auto brightness and contrast
  result = AutoBC();
  printf("AutoBC returned %d\n", result);
  result = AutoFocusC();
  printf("AutoFocusC returned %d\n", result);
  result = AutoFocusF();
  printf("AutoFocusF returned %d\n", result);
  result = AutoFocusStig();
  printf("AutoFocusStig returned %d\n", result);
*/
  
  // **************
  // stage controls
  // **************

//  GetStgDef
//  SetStgDef

  STGLOCA stg;
  result = ReadStageLoca(0, stg);
  printf("ReadStageLoca returned %d\n", result);
  printf("Stage position: X=%ld um, Y=%ld um, Z=%ld um\n"
         "                R=%ld, T=%ld, W=%ld\n",
                              stg.x, stg.y, stg.z, stg.r, stg.t, stg.w);
  stg.w = 2L;
  result = MoveStageLoca(0, stg);
  printf("MoveStageLoca returned %d\n", result);

  STGLOCA status;
  vrpn_bool stageIsMoving = vrpn_TRUE;
  int pollCount = 0;
  do {
    vrpn_SleepMsecs(100.0);
    result = ReadStageError(0, status);
    if (status.x == 0L) {
      stageIsMoving = vrpn_FALSE;
    } else {
      stageIsMoving = vrpn_TRUE;
    }
    pollCount++;
  } while (stageIsMoving && pollCount < 100);

  if (stageIsMoving) {
    printf("stage move test: >= 100 iterations\n");
  } else {
    printf("stage move test: %d iterations\n", (pollCount-1));
  }
 
  STGLOCA minStg, maxStg;
  result = GetLimits(0, minStg, maxStg);
  printf("GetLimits returned %d\n", result);
  printf("stage limits:\n");
  printf("  x: (%ld, %ld)\n", minStg.x, maxStg.x);
  printf("  y: (%ld, %ld)\n", minStg.y, maxStg.y);
  printf("  z: (%ld, %ld)\n", minStg.z, maxStg.z);
  printf("  r: (%ld, %ld)\n", minStg.r, maxStg.r);
  printf("  t: (%ld, %ld)\n", minStg.t, maxStg.t);
  printf("  w: (%ld, %ld)\n", minStg.w, maxStg.w);

//  GetStageReply
//  DisplayStageError

#endif
  
#endif

}

nmm_Microscope_SEM_EDAX::~nmm_Microscope_SEM_EDAX (void)
{
  closeEDAXHardware();
  closeScanControlInterface();
  delete d_exposureManager;
}

void nmm_Microscope_SEM_EDAX::checkForParameterChanges()
{

if (!d_virtualAcquisition) {
#ifndef VIRTUAL_SEM

#ifdef TRUST_EDAX
  int result;
  long mag;
  printf("calling getmag\n");
  result = GetMag(mag);
  printf("getmag returned %ld\n", mag);
  if (result == 0) {
    if (mag != d_magnification) {
      d_magnification = mag;
      reportMagnification();
    }
  } else {
    printf("error calling GetMag\n");
  }
#endif

#endif
}
}

vrpn_int32 nmm_Microscope_SEM_EDAX::mainloop(const struct timeval *timeout)
{
  checkForParameterChanges();
//  if (!d_connection || !(d_connection->connected())) {
//    return 0;
//  }

  if (d_scans_to_do > 0) {
    /*  When running controlling the server without going over the network 
        (in the same process as the client) there is a slight difference
        process as the client it is necessary to decrement before 
        calling acquireImage because
        acquireImage calls mainloop for the connection and this can result
        in a set of d_scans_to_do to 0 if the right message is received 
        breaking our assumption that d_scans_to_do is always non-negative */

    d_scans_to_do--;
    acquireImage();
  } else {
  }
  return 0;
}

vrpn_int32 nmm_Microscope_SEM_EDAX::initializeParameterDefaults()
{
  int file_found = 0;
  int i;
/*
if (!d_virtualAcquisition) {
#ifndef VIRTUAL_SEM
  LONG	Ierror;

  // either read in defaults from a file or, if file doesn't exist, 
  // set to hard-coded values
  EDI32_SEM_REC	SemCfg;
  Ierror = read_sem_params(	// read the scan generator parameters saved
		1,		// by the EDAM3 Shell
		"C:\\EDAX32\\IMG\\EDI32S1.cfg", 
		&SemCfg);
  if (Ierror != EDAX_ERROR){
	 file_found = vrpn_TRUE;
  }

#endif
}

  if (file_found) {
   if (!d_virtualAcquisition) {
#ifndef VIRTUAL_SEM
	printf("loading settings from file\n");
    
    d_gainParams[0] = (SHORT) SemCfg.xGain;
    d_offsetParams[0] = (SHORT) SemCfg.xOffset;
    
    d_gainParams[1] = (SHORT) SemCfg.yGain;
    d_offsetParams[1] = (SHORT) SemCfg.yOffset;
    
    d_gainParams[2] = (SHORT) SemCfg.zGain;
    d_offsetParams[2] = (SHORT) SemCfg.zOffset;

	printf("x,y,z (gain, offset): (%d,%d),(%d,%d),(%d,%d)\n",
            d_gainParams[0], d_offsetParams[0], 
            d_gainParams[1], d_offsetParams[1],
            d_gainParams[2], d_offsetParams[2]);

	d_horzRetrace_usec = SemCfg.horzRetrace;
	d_vertRetrace_usec = SemCfg.vertRetrace;
   
	printf("retrace delays: hor: %d, vert: %d\n",
		SemCfg.horzRetrace,SemCfg.vertRetrace);

	d_xScanDir = SemCfg.xMirror;
	d_yScanDir = SemCfg.yMirror;

	printf("scan dir: %d, %d\n", SemCfg.xMirror, SemCfg.yMirror);

	for (i = 0; i < EDAX_NUM_INPUT_CHANNELS; i++){
		d_videoPolarity[i] = SemCfg.videoPolarity[i];
	}

	d_xScanSpan = SemCfg.maxXSpan;
	d_yScanSpan = SemCfg.maxYSpan;
	d_interpixel_delay_nsec = 0;
    d_pix_integrate_nsec = 100;
    d_resolution_index = EDAX_DEFAULT_SCAN_MATRIX;
    nmm_EDAX::indexToResolution(d_resolution_index, d_resolution_x,
                                d_resolution_y);
    printf("max x,y dac spans: %d, %d\n", d_xScanSpan, d_yScanSpan);

#endif
   }
  } else {
*/
    // hard-coded defaults:
    d_gainParams[0] = EDAX_DEFAULT_X_DAC_GAIN;
    d_gainParams[1] = EDAX_DEFAULT_Y_DAC_GAIN;
    d_gainParams[2] = EDAX_DEFAULT_Z_ADC_GAIN;
    d_offsetParams[0] = EDAX_DEFAULT_X_DAC_OFFSET;
    d_offsetParams[1] = EDAX_DEFAULT_Y_DAC_OFFSET;
    d_offsetParams[2] = EDAX_DEFAULT_Z_ADC_OFFSET;

    d_horzRetrace_usec = EDAX_DEFAULT_HORZ_RETRACE;
    d_vertRetrace_usec = EDAX_DEFAULT_VERT_RETRACE;
    d_xScanDir = EDAX_DIR_NORMAL;
    d_yScanDir = EDAX_DIR_NORMAL;
    for (i = 0; i < EDAX_NUM_INPUT_CHANNELS; i++){
      d_videoPolarity[i] = EDAX_POLARITY_INVERTED;
    }
    d_xScanSpan = EDAX_DEFAULT_MAX_X_SPAN;
    d_yScanSpan = EDAX_DEFAULT_MAX_Y_SPAN;

    d_resolution_index = EDAX_DEFAULT_SCAN_MATRIX;
    nmm_EDAX::indexToResolution(d_resolution_index, d_resolution_x,
                            d_resolution_y);
    d_interpixel_delay_nsec = 0;
    d_pix_integrate_nsec = 1000;
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
  d_blankMode = EDAX_DEFAULT_BLANK_MODE;
  d_beam_location_x = 0;
  d_beam_location_y = 0;

  d_external_scan_control_enabled = 0;

  d_scanType = EDAX_FAST_SCAN;
  d_dataTransfer = EDAX_DATA_TRANSFER_BYTE;
  d_magnification = 1000;

  d_image_mode_settings_changed = vrpn_TRUE;
  d_point_mode_settings_changed = vrpn_TRUE;
  d_shared_settings_changed = vrpn_TRUE;
  return 0;
}

vrpn_int32 nmm_Microscope_SEM_EDAX::openEDAXHardware()
{
    // here is where we see if we can actually connect to the SEM
    int result = 0;
#ifndef VIRTUAL_SEM
    if (!d_virtualAcquisition) {
    ULONG boardNum = 0;
    result = InitGpuBoard(boardNum);
    if (result == EDAX_ERROR) {
        fprintf(stderr, "initializeHardware: InitGpuBoard failure\n");
        return -1;
    }
#ifdef USE_COLUMN_AND_STAGE
	printf("attemping to open the column\n");
    result = OpenColumn();
    if (result == EDAX_ERROR) {
        fprintf(stderr, "initializeHardware: OpenColumn failure\n");
        return -1;
    }

	printf("attempting to open the stage\n");
    result = OpenStage();
    if (result == EDAX_ERROR) {
        fprintf(stderr, "initializeHardware: OpenStage failure\n");
        return -1;
    }
#endif
    }
#endif

    return 0;
}

vrpn_int32 nmm_Microscope_SEM_EDAX::closeEDAXHardware()
{
    int result = 0;
#ifndef VIRTUAL_SEM
    if (!d_virtualAcquisition) {
    result = ResetGpuBoard();
    if (result == EDAX_ERROR) {
        fprintf(stderr, "closeHardware: ResetGpuBoard failure\n");
        return -1;
    }
#ifdef USE_COLUMN_AND_STAGE
    result = CloseColumn();
    if (result == EDAX_ERROR) {
        fprintf(stderr, "closeHardware: CloseColumn failure\n");
        return -1;
    }
    result = CloseStage();
    if (result == EDAX_ERROR) {
        fprintf(stderr, "closeHardware: CloseStage failure\n");
        return -1;
    }
#endif
    }
#endif
    return 0;
}

vrpn_int32 nmm_Microscope_SEM_EDAX::openScanControlInterface()
{
    int result = 0, enable = EDAX_TRUE;
    printf("EDAX is taking control of SEM scanning\n");
#ifndef VIRTUAL_SEM
    if (!d_virtualAcquisition) {
    result = SgEmia(EDAX_WRITE, &enable);
    if (result == EDAX_ERROR) {
        fprintf(stderr, "initializeHardware: SgEmia failure\n");
        return -1;
    }
    }
#else
    //printf("not\n");
#endif
    d_external_scan_control_enabled = vrpn_TRUE;
    return 0;
}

vrpn_int32 nmm_Microscope_SEM_EDAX::closeScanControlInterface()
{
    int result = 0, enable = EDAX_FALSE;
    printf("EDAX is relinquishing control of SEM scanning\n");
#ifndef VIRTUAL_SEM
    if (!d_virtualAcquisition) {
    result = SgEmia(EDAX_WRITE, &enable);
    }
#else
//    printf("not\n");
#endif
    if (result == EDAX_ERROR) {
        fprintf(stderr, "closeHardware: SgEmia failure\n");
        return -1;
    }
    d_external_scan_control_enabled = vrpn_FALSE;
    return 0;
}

vrpn_int32 nmm_Microscope_SEM_EDAX::configureSharedSettings()
{
#ifndef VIRTUAL_SEM
    if (!d_virtualAcquisition) {
    int result;
    result = SetupDACParams(d_gainParams, d_offsetParams);
    if (result == EDAX_ERROR) {
      fprintf(stderr, "setHardwareConfiguration: SetupDACParams failure\n");
      return -1;
    }
    result = SgSetRetrace(d_horzRetrace_usec, d_vertRetrace_usec);
    if (result == EDAX_ERROR) {
      fprintf(stderr, "setHardwareConfiguration: SgSetRetrace failure\n");
      return -1;
    }
    result = SgSetScanDir(d_xScanDir, d_yScanDir);
    if (result == EDAX_ERROR) {
      fprintf(stderr, "setHardwareConfiguration: SgSetScanDir failure\n");
      return -1;
    }
    result = SgSetVideoPolarity(d_videoPolarity);
    if (result == EDAX_ERROR) {
      fprintf(stderr, "setHardwareConfiguration: SgSetVideoPolarity failure\n");
      return -1;
    }
    result = SgSetMaxSpan(d_xScanSpan, d_yScanSpan);
    if (result == EDAX_ERROR) {
        fprintf(stderr, "setHardwareConfiguration: SgSetMaxSpan failure\n");
        return -1;
    }
    result = SgBeamBlank(d_blankMode);
    if (result == EDAX_ERROR) {
        fprintf(stderr, "setHardwareConfiguration: SgBeamBlank failure\n");
        return -1;
    }
    }
#endif
    
    d_shared_settings_changed = vrpn_FALSE;
    return 0;
}

vrpn_int32 nmm_Microscope_SEM_EDAX::configureForImageMode()
{
    configureSharedSettings();
    // this changes some of the settings used in point mode so we need to
    // set this flag to make them get changed back when we get a gotoPoint cmd
    d_point_mode_settings_changed = vrpn_TRUE;

    // pixel delay specified in usec, integration time is in 100 ns units
    // this call is really only to set the inter-pixel delay, the next two
    // functions are responsible for setting the pixel integration time and
    // the documentation says they should be called after this function,
    // not before
#ifndef VIRTUAL_SEM
    if (!d_virtualAcquisition) {
    int result;
    result = SpDwel(d_interpixel_delay_nsec/1000, 0, d_pix_integrate_nsec/100);
    if (result == EDAX_ERROR) {
	fprintf(stderr, "configureForImageMode: SpDwel failed\n");
	return -1;
    }
 
#ifdef USE_SET_SCAN_PARAMS
    // FAST_SCAN is limited to one ADC and one read (100 nsec integration) from
    // that ADC
    if (d_pix_integrate_nsec == 100) {
      SgSetScanParams(EDAX_FAST_SCAN, EDAX_DATA_TRANSFER_BYTE);
    } else {
      SgSetScanParams(EDAX_NORMAL_SCAN, EDAX_DATA_TRANSFER_BYTE);
    }
#endif

    // estimate ignoring integration time
    double msec_per_frame = d_resolution_y*0.5 + 
                            d_resolution_x*d_resolution_y*0.002;
    double msec_integration_per_frame = 
                 0.000001*d_resolution_x*d_resolution_y*d_pix_integrate_nsec;

/*
    printf("configureScan: estimated duration of image acquisition is ");
    printf("%g msec;\n  actual integration time is %g msec\n", 
           msec_per_frame, msec_integration_per_frame);
*/

    // division by 100 is to specify integration time in 100 ns units
    // integration time = 0 has special meaning - only gives us most
    // significant byte from ADC
    result = SetupSgScan(0, d_xScanSpan-1, 0, d_yScanSpan-1,
        d_pix_integrate_nsec/100, d_xScanSpan/d_resolution_x, 
	d_yScanSpan/d_resolution_y);


    if (result == EDAX_ERROR) {
		fprintf(stderr, "configureForImageMode: SetupSgScan failed\n");
		return -1;
    }
	
    }
#endif

    d_image_mode_settings_changed = vrpn_FALSE;
    return 0;
}

vrpn_int32 nmm_Microscope_SEM_EDAX::configureForPointMode()
{
    configureSharedSettings();
    // this changes some of the settings used in image mode so we need to
    // set this flag to make them get changed back when we start acquiring an
    // image
    d_image_mode_settings_changed = vrpn_TRUE;

#ifndef VIRTUAL_SEM
    if (!d_virtualAcquisition) {
    int result;
    //printf("setting dwell time to %d nsec\n", d_point_dwell_time_nsec);
    result = SpDwel(0, 0, d_point_dwell_time_nsec/100);
    if (result == EDAX_ERROR) {
        fprintf(stderr, "configureForPointMode: SpDwel failed\n");
        return -1;
    }

#ifdef USE_SET_SCAN_PARAMS
    SgSetScanParams(EDAX_FAST_SCAN, EDAX_DATA_TRANSFER_NONE);
#endif

    // set both x and y DAC increments to 1 so we get maximum resolution
    result = SetupSgScan(0, d_xScanSpan-1, 0, d_yScanSpan-1,
        d_point_dwell_time_nsec/100, 1, 1);

    if (result == EDAX_ERROR) {
                fprintf(stderr, "configureForPointMode: SetupSgScan failed\n");
                return -1;
    }

    result = SetSgScan(EDAX_SPOT_MODE);
    if (result != EDAX_OK) {
        fprintf(stderr, "configureForPointMode: SetSgScan failed\n");
        return -1;
    }
    }
#endif

    d_point_mode_settings_changed = vrpn_FALSE;
    return 0;
}

vrpn_int32 nmm_Microscope_SEM_EDAX::setResolution
				(vrpn_int32 res_x, vrpn_int32 res_y)
{
  // set to one of the discrete settings based on last setting and
  // the new setting
  int i;
  vrpn_bool res_valid = vrpn_false;
  i = nmm_EDAX::resolutionToIndex(res_x, res_y);
  if (i != -1) {
    res_valid = vrpn_TRUE;
    d_resolution_index = i;
  }
  if (res_valid){
    d_resolution_x = res_x;
    d_resolution_y = res_y;
    delete [] d_scanBuffer;
#ifdef UCHAR_PIXEL
    d_scanBuffer = new vrpn_uint8[d_resolution_x*d_resolution_y];
#else
    d_scanBuffer = new vrpn_uint8[2*d_resolution_x*d_resolution_y];
#endif
    d_image_mode_settings_changed = vrpn_TRUE;
  } else {
    fprintf(stderr, "setResolution: Warning, invalid resolution: %d, %d\n",
	res_x, res_y);
    return -1;
  }
  return reportResolution();
}

vrpn_int32 nmm_Microscope_SEM_EDAX::reportResolution()
{
  char *msgbuf;
  vrpn_int32 len;

  msgbuf = encode_ReportResolution(&len, d_resolution_x, d_resolution_y);
  if (!msgbuf){
    return -1;
  }

  return dispatchMessage(len, msgbuf, d_ReportResolution_type);
}

vrpn_int32 nmm_Microscope_SEM_EDAX::setPixelIntegrationTime
				(vrpn_int32 time_nsec)
{
    vrpn_int32 num_reads = (time_nsec)/100;
    if (num_reads == 0) num_reads = 1;
    d_pix_integrate_nsec = num_reads*100;
    d_image_mode_settings_changed = vrpn_TRUE;
    return reportPixelIntegrationTime();
}

vrpn_int32 nmm_Microscope_SEM_EDAX::reportPixelIntegrationTime()
{
  char *msgbuf;
  vrpn_int32 len;

  msgbuf = encode_ReportPixelIntegrationTime(&len, d_pix_integrate_nsec);
  if (!msgbuf){
    return -1;
  }

  return dispatchMessage(len, msgbuf, d_ReportPixelIntegrationTime_type);
}

vrpn_int32 nmm_Microscope_SEM_EDAX::setInterPixelDelayTime(vrpn_int32 time_nsec)
{
    vrpn_int32 num_usecs = time_nsec/1000;

/*
    if (num_usecs == 0){
        num_usecs = 1;
    }
*/
    d_interpixel_delay_nsec = num_usecs*1000;

    d_image_mode_settings_changed = vrpn_TRUE;
    return reportInterPixelDelayTime();
}

vrpn_int32 nmm_Microscope_SEM_EDAX::reportInterPixelDelayTime()
{
  char *msgbuf;
  vrpn_int32 len;

  msgbuf = encode_ReportInterPixelDelayTime(&len, d_interpixel_delay_nsec);
  if (!msgbuf){
    return -1;
  }

  return dispatchMessage(len, msgbuf, d_ReportInterPixelDelayTime_type);
}


vrpn_int32 nmm_Microscope_SEM_EDAX::requestScan(vrpn_int32 nscans)
{
	//printf("enable scan = %d\n", enable);
    if (nscans == 0) {
        d_scan_enabled = vrpn_FALSE;
        d_scans_to_do = 0;
    } else {
        d_scan_enabled = vrpn_TRUE;
        d_scans_to_do += nscans;
    }
    return 0;
}


vrpn_int32 nmm_Microscope_SEM_EDAX::getResolution(vrpn_int32 &res_x, 
	vrpn_int32 &res_y)
{
  res_x = d_resolution_x;
  res_y = d_resolution_y;
  return 0;
}

vrpn_int32 nmm_Microscope_SEM_EDAX::getPixelIntegrationTime_nsec()
{
  return d_pix_integrate_nsec;
}

vrpn_int32 nmm_Microscope_SEM_EDAX::getInterPixelDelayTime_nsec()
{
  return d_interpixel_delay_nsec;
}

vrpn_bool nmm_Microscope_SEM_EDAX::scanEnabled()
{
  return d_scan_enabled;
}

vrpn_int32 nmm_Microscope_SEM_EDAX::getPointDwellTime_nsec()
{
  return d_point_dwell_time_nsec;
}

vrpn_bool nmm_Microscope_SEM_EDAX::beamBlankEnabled()
{
  if (d_blankMode == EDAX_BLANK_ON){
    return vrpn_TRUE;
  } else {
    return vrpn_FALSE;
  }
}

vrpn_int32 nmm_Microscope_SEM_EDAX::getBeamLocation
                              (vrpn_int32 &x, vrpn_int32 &y)
{
  x = d_beam_location_x;
  y = d_beam_location_y;
  return 0;
}

vrpn_int32 nmm_Microscope_SEM_EDAX::getRetraceDelays
                              (vrpn_int32 &htime_usec, vrpn_int32 &vtime_usec)
{
  htime_usec = d_horzRetrace_usec;
  vtime_usec = d_vertRetrace_usec;
  d_shared_settings_changed = vrpn_TRUE;
  return 0;
}

vrpn_int32 nmm_Microscope_SEM_EDAX::getDACParams
                       (vrpn_int32 &xGain, vrpn_int32 &xOffset,
                        vrpn_int32 &yGain, vrpn_int32 &yOffset,
                        vrpn_int32 &zGain, vrpn_int32 &zOffset)
{
  xGain = d_gainParams[0];
  xOffset = d_offsetParams[0];
  yGain = d_gainParams[1];
  yOffset = d_offsetParams[1];
  zGain = d_gainParams[2];
  zOffset = d_offsetParams[2];
  return 0;
}

vrpn_int32 nmm_Microscope_SEM_EDAX::getExternalScanControlEnable
                                    (vrpn_int32 &enable) 
{
#ifndef VIRTUAL_SEM
  if (!d_virtualAcquisition) {
  // we set d_external_scan_control_enabled before based on the user request
  // but here we set it to whats in the hardware
  int read_result = 0;
  int call_result;
  call_result = SgEmia(EDAX_READ, &read_result);
  if (call_result == EDAX_ERROR) {
    fprintf(stderr, "getExternalScanControlEnable: Error calling SgEmia\n");
    return -1;
  }
  d_external_scan_control_enabled = read_result;
  }
#endif

  enable = d_external_scan_control_enabled;
  return 0;
}

vrpn_int32 nmm_Microscope_SEM_EDAX::getMagnification(vrpn_float32 &mag)
{
#ifndef VIRTUAL_SEM
  if (!d_virtualAcquisition) {
#ifdef USE_COLUMN_AND_STAGE
  long lmag;
  int result;
  printf("calling getmag\n");
  result = GetMag(lmag);
  printf("getmag returned %ld\n", lmag);
  if (result == 0) {
    if (lmag != d_magnification) {
      d_magnification = lmag;
    }
  } else {
    printf("Error calling GetMag\n");
  }
#endif
  }
#endif
  mag = (vrpn_float32)d_magnification;
  return 0;
}

vrpn_int32 nmm_Microscope_SEM_EDAX::acquireImage()
{
  int i;

  if (d_shared_settings_changed) {
    configureSharedSettings();
  }
  if (d_image_mode_settings_changed) {
    configureForImageMode();
  }

  if (!d_virtualAcquisition) {
#ifndef VIRTUAL_SEM
    // variables used for timing
    double t_SetSgScan, t_SetupSgColl, t_CollectSgLine, t_reportScanlineData;
    int result;
    struct timeval t0, t1, t2;
    
    gettimeofday(&t0, NULL);
    // start the scan as configured
    // this is supposedly where the scan gets started but then
    // does that require later calls to wait until the next scan starts?
    result = SetSgScan(EDAX_FULL_SCAN);

    gettimeofday(&t1, NULL);
    t0 = vrpn_TimevalDiff(t1, t0);
    t_SetSgScan = vrpn_TimevalMsecs(t0);

    if (result == EDAX_ERROR) {
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
    if (result == EDAX_ERROR) {
	fprintf(stderr, "acquireImage: SetupSgColl failed\n");
	return -1;
    }
    gettimeofday(&t1, NULL);
    t0 = vrpn_TimevalDiff(t1, t0);
    t_SetupSgColl = vrpn_TimevalMsecs(t0);

    // collect the image
/*
    result = CollectSgStrip(d_scanBuffer);
    if (result == EDAX_ERROR) {
      fprintf(stderr, "Error collecting image\n");
    }
    for (i = 0; i < d_resolution_y; i++){
      reportScanlineData(i);
      d_connection->mainloop();
    }
*/

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
      if (result == EDAX_ERROR) {
	fprintf(stderr, "Error collecting scan line for row %d\n", i);
      }
      gettimeofday(&t1, NULL);
      reportScanlineData(i);
      //d_connection->mainloop();
      gettimeofday(&t2, NULL);
      t0 = vrpn_TimevalDiff(t1, t0);
      t1 = vrpn_TimevalDiff(t2, t1);
      t_CollectSgLine += vrpn_TimevalMsecs(t0);
      t_reportScanlineData += vrpn_TimevalMsecs(t1);
    }
#endif
  } else {
  // make some fake data:
  static int count = 0;

  int dx, dy;

  for (i = 0; i < d_resolution_y; i++){
#ifdef UCHAR_PIXEL
/*
    memset(&(d_scanBuffer[i*d_resolution_x]),
        ((i+((int)(count)))%d_resolution_y)*255/d_resolution_y, d_resolution_x);
*/
    dy = i-d_resolution_y/2;
    if (dy < 0) dy = -dy;
    int j;
    for (j = 0; j < d_resolution_x; j++){
        dx = j-d_resolution_x/2;
        if (dx < 0) dx = -dx;
        ((vrpn_uint8 *)d_scanBuffer)[i*d_resolution_x + j] =
             ((dx+dy+count)%(d_resolution_y/4))*255/
             (d_resolution_y/4);
    }
#else
    int j;
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

/*
  printf("acquireImage - measured times for acquisition operations:\n");
  printf("  SetSgScan x 1 : %g msec\n", t_SetSgScan);
  printf("  SetupSgColl x 1 : %g msec\n", t_SetupSgColl);
  printf("  CollectSgLine x %d : %g msec\n", d_resolution_y,
            t_CollectSgLine);
  printf("  reportScanlineData x %d : %g msec\n", d_resolution_y,
            t_reportScanlineData);
*/

  return 0;
}

vrpn_int32 nmm_Microscope_SEM_EDAX::reportScanlineData(int line_num)
{
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
/*  if (line_num + lines_per_message > d_resolution_y){
      lines_per_message = d_resolution_y - line_num;
  }
  */
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
}

vrpn_int32 nmm_Microscope_SEM_EDAX::setPointDwellTime(vrpn_int32 time_nsec,
                                                      vrpn_bool report)
{
    d_point_dwell_time_nsec = time_nsec;
    d_point_mode_settings_changed = vrpn_TRUE;
    if (report) {
      return reportPointDwellTime();
    } else {
      return 0;
    }
}

vrpn_int32 nmm_Microscope_SEM_EDAX::reportPointDwellTime()
{
  char *msgbuf;
  vrpn_int32 len;

  msgbuf = encode_ReportPointDwellTime(&len, d_point_dwell_time_nsec);
  if (!msgbuf){
    return -1;
  }

  return dispatchMessage(len, msgbuf, d_ReportPointDwellTime_type);
}

vrpn_int32 nmm_Microscope_SEM_EDAX::setBeamBlankEnable(vrpn_int32 enable)
{
  if (enable) {
    d_blankMode = EDAX_BLANK_ON;
  } else {
    d_blankMode = EDAX_BLANK_OFF;
  }
  d_shared_settings_changed = vrpn_TRUE;
  return reportBeamBlankEnable();
}

vrpn_int32 nmm_Microscope_SEM_EDAX::reportBeamBlankEnable()
{
  vrpn_int32 enable;
  if (beamBlankEnabled()) {
    enable = 1;
  } else {
    enable = 0;
  }

  char *msgbuf;
  vrpn_int32 len;

  msgbuf = encode_ReportBeamBlankEnable(&len, enable);
  if (!msgbuf){
    return -1;
  }
  
  return dispatchMessage(len, msgbuf, d_ReportBeamBlankEnable_type);
}

vrpn_int32 nmm_Microscope_SEM_EDAX::reportMaxScanSpan()
{
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
}

vrpn_int32 nmm_Microscope_SEM_EDAX::goToPoint(vrpn_int32 xDAC, vrpn_int32 yDAC,
               vrpn_bool report)
{
  static int point_count = 0;
  static double cumulative_dwell_msec = 0;
  struct timeval t0, t_end;
  if (d_shared_settings_changed) {
    configureSharedSettings();
  }
  if (d_point_mode_settings_changed) {
    configureForPointMode();
  }

  if (xDAC < 0 || yDAC < 0 || xDAC >= d_xScanSpan || yDAC >= d_yScanSpan) {
    fprintf(stderr, "goToPoint: Error, point out of range:(%d, %d)\n",
         xDAC, yDAC);
  } 

  point_count++;

  gettimeofday(&t0, NULL);
#ifndef VIRTUAL_SEM
if (!d_virtualAcquisition) {
  LONG result;
  LONG x = xDAC;
  LONG y = yDAC;
//  LONG obsolete_parameter = 0;

#ifdef USE_SCAN_TABLE
  unsigned int np = 1;
  int ix = x, iy = y;
  result = ScanTable(np, &ix, &iy);
  if (result != EDAX_OK) {
     fprintf(stderr, "Error calling ScanTable(%ld, %ld)\n", x, y);
  }

#else

  // use y for the obsolete parameter just to check if EDAX does 
  // anything with it
  result = SpMoveEx(x,y); // this takes about 100 usec to execute
  //result = SpMove(x, y, y);

  if (result != EDAX_OK) {
     fprintf(stderr, "Error calling SpMove(%ld, %ld)\n", x, y);
  }
  
#endif
  }
#endif

  gettimeofday(&t_end, NULL);
  double delta_t = vrpn_TimevalMsecs(t_end) - vrpn_TimevalMsecs(t0);

#ifdef USE_BUSYWAIT_DELAY
  while (delta_t < 0.000001*d_point_dwell_time_nsec) {
    gettimeofday(&t_end, NULL);
    delta_t = vrpn_TimevalMsecs(t_end) - vrpn_TimevalMsecs(t0);
  }
#endif
  cumulative_dwell_msec += delta_t;

/*
  if ((point_count % 1000) == 0) {
    double avg = cumulative_dwell_msec / (double)point_count;
    printf("estimated average dwell time: %g msec\n", avg);
    cumulative_dwell_msec = 0;
    point_count = 0;
  }
*/

  d_beam_location_x = xDAC;
  d_beam_location_y = yDAC;
  if (report) {
    return reportBeamLocation();
  } else {
    return 0;
  }
}

vrpn_int32 nmm_Microscope_SEM_EDAX::reportBeamLocation()
{
  vrpn_int32 x, y;
  x = d_beam_location_x;
  y = d_beam_location_y;
  char *msgbuf;
  vrpn_int32 len;

  msgbuf = encode_ReportBeamLocation(&len, x, y);
  if (!msgbuf){
    return -1;
  }

  return dispatchMessage(len, msgbuf, d_ReportBeamLocation_type);
}

vrpn_int32 nmm_Microscope_SEM_EDAX::setRetraceDelays(
                    vrpn_int32 htime_nsec, vrpn_int32 vtime_nsec)
{
  d_horzRetrace_usec = htime_nsec/1000;
  d_vertRetrace_usec = vtime_nsec/1000;
  d_shared_settings_changed = vrpn_TRUE;
  return reportRetraceDelays();
}

vrpn_int32 nmm_Microscope_SEM_EDAX::reportRetraceDelays()
{
  vrpn_int32 htime_nsec, vtime_nsec;
  char *msgbuf;
  vrpn_int32 len;
  htime_nsec = d_horzRetrace_usec*1000;
  vtime_nsec = d_vertRetrace_usec*1000;

  msgbuf = encode_ReportRetraceDelays(&len, htime_nsec, vtime_nsec);
  if (!msgbuf){
    return -1;
  }

  return dispatchMessage(len, msgbuf, d_ReportRetraceDelays_type);
}

vrpn_int32 nmm_Microscope_SEM_EDAX::setDACParams(
                        vrpn_int32 xGain, vrpn_int32 xOffset,
                        vrpn_int32 yGain, vrpn_int32 yOffset,
                        vrpn_int32 zGain, vrpn_int32 zOffset)
{
  d_gainParams[0] = xGain;
  d_offsetParams[0] = xOffset;
  d_gainParams[1] = yGain;
  d_offsetParams[1] = yOffset;
  d_gainParams[2] = zGain;
  d_offsetParams[2] = zOffset;

  d_shared_settings_changed = vrpn_TRUE;
  return reportDACParams();
}

vrpn_int32 nmm_Microscope_SEM_EDAX::reportDACParams()
{
  vrpn_int32 xg, xo, yg, yo, zg, zo;
  char *msgbuf;
  vrpn_int32 len;
  getDACParams(xg, xo, yg, yo, zg, zo);

  msgbuf = encode_ReportDACParams(&len, xg, xo, yg, yo, zg, zo);
  if (!msgbuf){
    return -1;
  }

  return dispatchMessage(len, msgbuf, d_ReportDACParams_type);
}

vrpn_int32 nmm_Microscope_SEM_EDAX::setExternalScanControlEnable(
                                    vrpn_int32 enable)
{
  if (enable) {
    openScanControlInterface();
  } else {
    closeScanControlInterface();
  }
  return reportExternalScanControlEnable();
}

vrpn_int32 nmm_Microscope_SEM_EDAX::reportExternalScanControlEnable()
{
  vrpn_int32 enable;
  char *msgbuf;
  vrpn_int32 len;
  getExternalScanControlEnable(enable);
 
  msgbuf = encode_ReportExternalScanControlEnable(&len, enable);
  if (!msgbuf){
    return -1;
  }
  
  return dispatchMessage(len, msgbuf, d_ReportExternalScanControlEnable_type);
}

vrpn_int32 nmm_Microscope_SEM_EDAX::reportMagnification()
{
  vrpn_float32 mag;
  char *msgbuf;
  vrpn_int32 len;
  getMagnification(mag);

  msgbuf = encode_ReportMagnification(&len, mag);
  if (!msgbuf){
    return -1;
  }

  return dispatchMessage(len, msgbuf, d_ReportMagnification_type);
}


vrpn_int32 nmm_Microscope_SEM_EDAX::clearExposePattern()
{
  d_patternShapes.clear();
  d_dumpPoints.clear();
  return 0;
}

vrpn_int32 nmm_Microscope_SEM_EDAX::addPolygon(
                   vrpn_float32 exposure_uCoul_per_cm2,
                   vrpn_int32 numPoints,
                   vrpn_float32 *x_nm, vrpn_float32 *y_nm)
{
  PatternShape shape(0, exposure_uCoul_per_cm2, PS_POLYGON);
  int i;
  for (i = 0; i < numPoints; i++) {
    shape.addPoint(x_nm[i], y_nm[i]);
  }

  d_patternShapes.push_back(shape);
  return 0;
}

vrpn_int32 nmm_Microscope_SEM_EDAX::addPolyline(
                    vrpn_float32 exposure_uCoul_per_cm2,
                    vrpn_float32 lineWidth_nm, vrpn_int32 numPoints,
                    vrpn_float32 *x_nm, vrpn_float32 *y_nm)
{
  PatternShape shape(lineWidth_nm, exposure_uCoul_per_cm2, PS_POLYLINE);
  int i;
  for (i = 0; i < numPoints; i++) {
    shape.addPoint((double)(x_nm[i]), (double)(y_nm[i]));
  }

  d_patternShapes.push_back(shape);
  return 0;
}

vrpn_int32 nmm_Microscope_SEM_EDAX::addDumpPoint(
                    vrpn_float32 x_nm, vrpn_float32 y_nm)
{
  PatternPoint point((double)x_nm, (double)y_nm);
  d_dumpPoints.push_back(point);
  return 0;
}

vrpn_int32 nmm_Microscope_SEM_EDAX::exposePattern()
{
  vrpn_float32 mag;
  getMagnification(mag);

  d_exposureManager->setColumnParameters(1e-6,
                d_beamWidth_nm, d_beamCurrent_picoAmps);
  d_exposureManager->exposePattern(d_patternShapes, d_dumpPoints, this, mag);
  return 0;
}

vrpn_int32 nmm_Microscope_SEM_EDAX::setBeamCurrent(
                     vrpn_float32 current_picoAmps)
{
  d_beamCurrent_picoAmps = current_picoAmps;
  return 0;
}

vrpn_int32 nmm_Microscope_SEM_EDAX::setBeamWidth(
                     vrpn_float32 beamWidth_nm)
{
  d_beamWidth_nm = beamWidth_nm;
  return 0;
}

//static 
int nmm_Microscope_SEM_EDAX::RcvSetResolution 
	(void *_userdata, vrpn_HANDLERPARAM _p)
{
  nmm_Microscope_SEM_EDAX *me = (nmm_Microscope_SEM_EDAX *)_userdata;
  const char * bufptr = _p.buffer;
  vrpn_int32 res_x, res_y;

  if (decode_SetResolution(&bufptr, &res_x, &res_y) == -1) {
    fprintf(stderr, 
	"nmm_Microscope_SEM_EDAX::RcvSetResolution: decode failed\n");
    return -1;
  }
  if (me->setResolution(res_x, res_y) == -1) {
    fprintf(stderr, "nmm_Microscope_SEM_EDAX::RcvSetResolution: set failed\n");
    return -1;
  }
  return 0;
}

//static 
int nmm_Microscope_SEM_EDAX::RcvSetPixelIntegrationTime
		(void *_userdata, vrpn_HANDLERPARAM _p)
{
  nmm_Microscope_SEM_EDAX *me = (nmm_Microscope_SEM_EDAX *)_userdata;
  const char * bufptr = _p.buffer;
  vrpn_int32 time_nsec;

  if (decode_SetPixelIntegrationTime(&bufptr, &time_nsec) == -1) {
    fprintf(stderr,
        "nmm_Microscope_SEM_EDAX::RcvSetPixelIntegrationTime: decode failed\n");
    return -1;
  }
  if (me->setPixelIntegrationTime(time_nsec) == -1) {
    fprintf(stderr, "nmm_Microscope_SEM_EDAX::RcvSetPixelIntegrationTime:"
                    " set failed\n");
    return -1;
  }
  return 0;
}

//static 
int nmm_Microscope_SEM_EDAX::RcvSetInterPixelDelayTime
		(void *_userdata, vrpn_HANDLERPARAM _p)
{
  nmm_Microscope_SEM_EDAX *me = (nmm_Microscope_SEM_EDAX *)_userdata;   
  const char * bufptr = _p.buffer;
  vrpn_int32 time_nsec;

  if (decode_SetInterPixelDelayTime(&bufptr, &time_nsec) == -1) {
    fprintf(stderr,
        "nmm_Microscope_SEM_EDAX::RcvSetInterPixelDelayTime: decode failed\n");
    return -1;
  }
  if (me->setInterPixelDelayTime(time_nsec) == -1) {
    fprintf(stderr, "nmm_Microscope_SEM_EDAX::RcvSetInterPixelDelayTime:"
                    " set failed\n");
    return -1;
  }
  return 0;
}

//static 
int nmm_Microscope_SEM_EDAX::RcvRequestScan
		(void *_userdata, vrpn_HANDLERPARAM _p)
{
  nmm_Microscope_SEM_EDAX *me = (nmm_Microscope_SEM_EDAX *)_userdata;
  const char * bufptr = _p.buffer;
  vrpn_int32 nscans;

  if (decode_RequestScan(&bufptr, &nscans) == -1) {
    fprintf(stderr,
        "nmm_Microscope_SEM_EDAX::RcvRequestScan: decode failed\n");
    return -1;
  }
  if (me->requestScan(nscans) == -1) {
    fprintf(stderr, "nmm_Microscope_SEM_EDAX::RcvRequestScan: set failed\n");
    return -1;
  }
  return 0;
}

//static 
int nmm_Microscope_SEM_EDAX::RcvSetPointDwellTime
                (void *_userdata, vrpn_HANDLERPARAM _p)
{
  nmm_Microscope_SEM_EDAX *me = (nmm_Microscope_SEM_EDAX *)_userdata;
  const char * bufptr = _p.buffer;
  vrpn_int32 time_nsec;

  if (decode_SetPointDwellTime(&bufptr, &time_nsec) == -1) {
    fprintf(stderr,
        "nmm_Microscope_SEM_EDAX::RcvSetPointDwellTime: decode failed\n");
    return -1;
  }
  if (me->setPointDwellTime(time_nsec) == -1) {
    fprintf(stderr, 
          "nmm_Microscope_SEM_EDAX::RcvSetPointDwellTime: set failed\n");
    return -1;
  }
  return 0;
}

//static 
int nmm_Microscope_SEM_EDAX::RcvSetBeamBlankEnable
                (void *_userdata, vrpn_HANDLERPARAM _p)
{
  nmm_Microscope_SEM_EDAX *me = (nmm_Microscope_SEM_EDAX *)_userdata;
  const char * bufptr = _p.buffer;
  vrpn_int32 enable;

  if (decode_SetBeamBlankEnable(&bufptr, &enable) == -1) {
    fprintf(stderr,
        "nmm_Microscope_SEM_EDAX::RcvSetBeamBlankEnable: decode failed\n");
    return -1;
  }
  if (me->setBeamBlankEnable(enable) == -1) {
    fprintf(stderr, 
         "nmm_Microscope_SEM_EDAX::RcvSetBeamBlankEnable: set failed\n");
    return -1;
  }
  return 0;
}

//static 
int nmm_Microscope_SEM_EDAX::RcvGoToPoint
                (void *_userdata, vrpn_HANDLERPARAM _p)
{
  nmm_Microscope_SEM_EDAX *me = (nmm_Microscope_SEM_EDAX *)_userdata;
  const char * bufptr = _p.buffer;
  vrpn_int32 x, y;

  if (decode_GoToPoint(&bufptr, &x, &y) == -1) {
    fprintf(stderr,
        "nmm_Microscope_SEM_EDAX::RcvGoToPoint: decode failed\n");
    return -1;
  }
  if (me->goToPoint(x,y) == -1) {
    fprintf(stderr, "nmm_Microscope_SEM_EDAX::RcvGoToPoint: set failed\n");
    return -1;
  }
  return 0;
}

//static 
int nmm_Microscope_SEM_EDAX::RcvSetRetraceDelays
                (void *_userdata, vrpn_HANDLERPARAM _p)
{
  nmm_Microscope_SEM_EDAX *me = (nmm_Microscope_SEM_EDAX *)_userdata;
  const char * bufptr = _p.buffer;
  vrpn_int32 hdelay, vdelay;

  if (decode_SetRetraceDelays(&bufptr, &hdelay, &vdelay) == -1) {
    fprintf(stderr,
        "nmm_Microscope_SEM_EDAX::RcvSetRetraceDelays: decode failed\n");
    return -1;
  }
  if (me->setRetraceDelays(hdelay, vdelay) == -1) {
    fprintf(stderr, 
            "nmm_Microscope_SEM_EDAX::RcvSetRetraceDelays: set failed\n");
    return -1;
  }
  return 0;
}

//static 
int nmm_Microscope_SEM_EDAX::RcvSetDACParams
                (void *_userdata, vrpn_HANDLERPARAM _p)
{
  nmm_Microscope_SEM_EDAX *me = (nmm_Microscope_SEM_EDAX *)_userdata;
  const char * bufptr = _p.buffer;
  vrpn_int32 xg, xo, yg, yo, zg, zo;

  if (decode_SetDACParams(&bufptr, &xg, &xo, &yg, &yo, &zg, &zo) == -1) {
    fprintf(stderr,
        "nmm_Microscope_SEM_EDAX::RcvSetDACParams: decode failed\n");
    return -1;
  }
  if (me->setDACParams(xg, xo, yg, yo, zg, zo) == -1) {
    fprintf(stderr, "nmm_Microscope_SEM_EDAX::RcvSetDACParams: set failed\n");
    return -1;
  }
  return 0;
}

//static
int nmm_Microscope_SEM_EDAX::RcvSetExternalScanControlEnable
                (void *_userdata, vrpn_HANDLERPARAM _p)
{
  nmm_Microscope_SEM_EDAX *me = (nmm_Microscope_SEM_EDAX *)_userdata;
  const char * bufptr = _p.buffer;
  vrpn_int32 enable;

  if (decode_SetExternalScanControlEnable(&bufptr, &enable) == -1) {
    fprintf(stderr,
        "nmm_Microscope_SEM_EDAX::RcvSetExternalScanControlEnable:"
        " decode failed\n");
    return -1;
  }
  if (me->setExternalScanControlEnable(enable) == -1) {
    fprintf(stderr,
         "nmm_Microscope_SEM_EDAX::RcvSetExternalScanControlEnable:"
         " set failed\n");
    return -1;
  }
  return 0;
}

//static
int nmm_Microscope_SEM_EDAX::RcvClearExposePattern
                (void *_userdata, vrpn_HANDLERPARAM _p)
{
  nmm_Microscope_SEM_EDAX *me = (nmm_Microscope_SEM_EDAX *)_userdata;

  if (me->clearExposePattern() == -1) {
    fprintf(stderr,
         "nmm_Microscope_SEM_EDAX::clearExposePattern failed\n");
    return -1;
  }
  return 0;
}

//static
int nmm_Microscope_SEM_EDAX::RcvAddPolygon
                (void *_userdata, vrpn_HANDLERPARAM _p)
{
  nmm_Microscope_SEM_EDAX *me = (nmm_Microscope_SEM_EDAX *)_userdata;
  const char * bufptr = _p.buffer;
  vrpn_float32 exposure_uCoul_per_cm2;
  vrpn_int32 numPoints;
  vrpn_float32 *x_nm, *y_nm;

  if (decode_AddPolygonHeader(&bufptr, &exposure_uCoul_per_cm2, 
                              &numPoints) == -1) {
    fprintf(stderr,
        "nmm_Microscope_SEM_EDAX::RcvAddPolygon"
        " decode header failed\n");
    return -1;
  }
  x_nm = new vrpn_float32[numPoints];
  y_nm = new vrpn_float32[numPoints];
  if (decode_AddPolygonData(&bufptr, numPoints,
                              x_nm, y_nm) == -1) {
    fprintf(stderr,
        "nmm_Microscope_SEM_EDAX::RcvAddPolygon"
        " decode data failed\n");
    return -1;
  }

  if (me->addPolygon(exposure_uCoul_per_cm2, numPoints, x_nm, y_nm) == -1) {
    fprintf(stderr,
         "nmm_Microscope_SEM_EDAX::RcvAddPolygon"
         " set failed\n");
    return -1;
  }
  delete [] x_nm;
  delete [] y_nm;

  return 0;
}

//static
int nmm_Microscope_SEM_EDAX::RcvAddPolyline
                (void *_userdata, vrpn_HANDLERPARAM _p)
{
  nmm_Microscope_SEM_EDAX *me = (nmm_Microscope_SEM_EDAX *)_userdata;
  const char * bufptr = _p.buffer;
  vrpn_float32 exposure_uCoul_per_cm2;
  vrpn_float32 lineWidth_nm;
  vrpn_int32 numPoints;
  vrpn_float32 *x_nm, *y_nm;

  if (decode_AddPolylineHeader(&bufptr, &exposure_uCoul_per_cm2, &lineWidth_nm,
                              &numPoints) == -1) {
    fprintf(stderr,
        "nmm_Microscope_SEM_EDAX::RcvAddPolyline"
        " decode header failed\n");
    return -1;
  }
  x_nm = new vrpn_float32[numPoints];
  y_nm = new vrpn_float32[numPoints];
  if (decode_AddPolylineData(&bufptr, numPoints,
                              x_nm, y_nm) == -1) {
    fprintf(stderr,
        "nmm_Microscope_SEM_EDAX::RcvAddPolyline"
        " decode data failed\n");
    return -1;
  }

  if (me->addPolyline(exposure_uCoul_per_cm2, lineWidth_nm, numPoints, 
                      x_nm, y_nm) == -1) {
    fprintf(stderr,
         "nmm_Microscope_SEM_EDAX::RcvAddPolyline"
         " set failed\n");
    return -1;
  }
  delete [] x_nm;
  delete [] y_nm;

  return 0;
}

//static
int nmm_Microscope_SEM_EDAX::RcvAddDumpPoint
                (void *_userdata, vrpn_HANDLERPARAM _p)
{
  nmm_Microscope_SEM_EDAX *me = (nmm_Microscope_SEM_EDAX *)_userdata;
  const char * bufptr = _p.buffer;
  vrpn_float32 x_nm, y_nm;

  if (decode_AddDumpPoint(&bufptr, &x_nm, &y_nm) == -1) {
    fprintf(stderr,
        "nmm_Microscope_SEM_EDAX::RcvAddDumpPoint"
        " decode failed\n");
    return -1;
  }

  if (me->addDumpPoint(x_nm, y_nm) == -1) {
    fprintf(stderr,
         "nmm_Microscope_SEM_EDAX::RcvAddDumpPoint"
         " set failed\n");
    return -1;
  }

  return 0;
}

//static
int nmm_Microscope_SEM_EDAX::RcvExposePattern
                (void *_userdata, vrpn_HANDLERPARAM _p)
{
  nmm_Microscope_SEM_EDAX *me = (nmm_Microscope_SEM_EDAX *)_userdata;

  if (me->exposePattern() == -1) {
    fprintf(stderr, "nmm_Microscope_SEM_EDAX::RcvExposePattern failed\n");
    return -1;
  }
  return 0;
}


//static
int nmm_Microscope_SEM_EDAX::RcvSetBeamCurrent
                (void *_userdata, vrpn_HANDLERPARAM _p)
{
  nmm_Microscope_SEM_EDAX *me = (nmm_Microscope_SEM_EDAX *)_userdata;
  const char * bufptr = _p.buffer;
  vrpn_float32 beamCurrent_picoAmps;

  if (decode_SetBeamCurrent(&bufptr, &beamCurrent_picoAmps) == -1) {
    fprintf(stderr,
        "nmm_Microscope_SEM_EDAX::RcvSetBeamCurrent"
        " decode failed\n");
    return -1;
  }

  if (me->setBeamCurrent(beamCurrent_picoAmps) == -1) {
    fprintf(stderr, "nmm_Microscope_SEM_EDAX::RcvSetBeamCurrent: set failed\n");
    return -1;
  }
  return 0;
}

//static
int nmm_Microscope_SEM_EDAX::RcvSetBeamWidth
                (void *_userdata, vrpn_HANDLERPARAM _p)
{
  nmm_Microscope_SEM_EDAX *me = (nmm_Microscope_SEM_EDAX *)_userdata;
  const char * bufptr = _p.buffer;
  vrpn_float32 beamWidth_nm;

  if (decode_SetBeamWidth(&bufptr, &beamWidth_nm) == -1) {
    fprintf(stderr,
        "nmm_Microscope_SEM_EDAX::RcvSetBeamWidth"
        " decode failed\n");
    return -1;
  }

  if (me->setBeamWidth(beamWidth_nm) == -1) {
    fprintf(stderr, "nmm_Microscope_SEM_EDAX::RcvSetBeamWidth: set failed\n");
    return -1;
  }
  return 0;
}

//static
int nmm_Microscope_SEM_EDAX::RcvGotConnection
		(void *_userdata, vrpn_HANDLERPARAM _p)
{
    nmm_Microscope_SEM_EDAX *me = 
			(nmm_Microscope_SEM_EDAX *)_userdata;
  
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
    //me->openScanControlInterface();
    me->d_scans_to_do = 0;
    me->d_scan_enabled = 0;
    return 0;
}

// static
int nmm_Microscope_SEM_EDAX::RcvDroppedConnection
                (void *_userdata, vrpn_HANDLERPARAM _p)
{
        nmm_Microscope_SEM_EDAX *me =
                        (nmm_Microscope_SEM_EDAX *)_userdata;
        me->closeScanControlInterface();
        return 0;
}
