#include <vrpn_Connection.h>
#include "nmm_Microscope_SEM_EDAX.h"
#include "nmm_EDAX.h"
#include <stdlib.h>

//#define VIRTUAL_SEM

#ifndef VIRTUAL_SEM
extern "C" {
#include "imgboard.h"
#include "semdef.h"

extern short int EDXCALL read_sem_params(char binary, char *fileName, PEDI32_SEM_REC edi32SgCfg);

}
#endif

int nmm_Microscope_SEM_EDAX::d_matrixSizeX[EDAX_NUM_SCAN_MATRICES] = 
						EDAX_SCAN_MATRIX_X;
int nmm_Microscope_SEM_EDAX::d_matrixSizeY[EDAX_NUM_SCAN_MATRICES] = 
						EDAX_SCAN_MATRIX_Y;

nmm_Microscope_SEM_EDAX::nmm_Microscope_SEM_EDAX 
    (const char * name, vrpn_Connection * c) :
    nmb_Device_Server(name, c),
    nmm_Microscope_SEM(name, d_connection), 
    d_scan_enabled(vrpn_FALSE), 
    d_lines_per_message(1), 
    d_scans_to_do(0)
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
  int connect_id = d_connection->register_message_type(vrpn_got_connection);
  int disconnect_id = 
	d_connection->register_message_type(vrpn_dropped_connection);
  d_connection->register_handler(connect_id,
				RcvGotConnection, this);
  d_connection->register_handler(disconnect_id,
                                RcvDroppedConnection, this);

  initializeParameterDefaults();
  // create buffer in which to store data
#ifdef _WIN32
  d_scanBuffer = new UCHAR[d_resolution_x*d_resolution_y];
#else
  d_scanBuffer = new vrpn_uint8[d_resolution_x*d_resolution_y];
#endif
  if (!d_scanBuffer){
	fprintf(stderr, "Error, out of memory allocating image buffer %dx%d\n",
		d_resolution_x, d_resolution_y);
	exit(-1);
  }
  openEDAXHardware();
  setHardwareConfiguration();
}

nmm_Microscope_SEM_EDAX::~nmm_Microscope_SEM_EDAX (void)
{
}

vrpn_int32 nmm_Microscope_SEM_EDAX::mainloop(void)
{
  if (!d_connection || !(d_connection->connected())) {
    return 0;
  }
  if (d_scans_to_do > 0) {
    acquireImage();
    d_scans_to_do--;
  }
  return 0;
}

vrpn_int32 nmm_Microscope_SEM_EDAX::initializeParameterDefaults()
{
  int file_found = 0;
  int i;
#ifndef VIRTUAL_SEM
  LONG	Ierror;
  // either read in defaults from a file or, if file doesn't exist, 
  // set to hard-coded values
  EDI32_SEM_REC	SemCfg;
  Ierror = read_sem_params(				// read the scan generator parameters saved
		1,									// by the EDAM3 Shell
		"C:\\EDAX32\\IMG\\EDI32S1.cfg", 
		&SemCfg);
  if (Ierror != EDAX_ERROR){
	 file_found = vrpn_TRUE;
  }
#endif
  if (file_found) {
#ifndef VIRTUAL_SEM
	printf("loading settings from file\n");
	SHORT	GainSettings[3];
	SHORT	OffsetSettings[3];
    
    d_gainParams[0] = (SHORT) SemCfg.xGain;
    d_offsetParams[0] = (SHORT) SemCfg.xOffset;
    
    d_gainParams[1] = (SHORT) SemCfg.yGain;
    d_offsetParams[1] = (SHORT) SemCfg.yOffset;
    
    d_gainParams[2] = (SHORT) SemCfg.zGain;
    d_offsetParams[2] = (SHORT) SemCfg.zOffset;

	printf("x,y,z (gain, offset): (%d,%d),(%d,%d),(%d,%d)\n",
            GainSettings[0], OffsetSettings[0], 
            GainSettings[1], OffsetSettings[1],
            GainSettings[2], OffsetSettings[2]);

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
	d_interpixel_delay_nsec = 200;
    d_pix_integrate_nsec = 100;
    d_resolution_index = EDAX_DEFAULT_SCAN_MATRIX;
    d_resolution_x = d_matrixSizeX[d_resolution_index];
    d_resolution_y = d_matrixSizeY[d_resolution_index];
	printf("max x,y dac spans: %d, %d\n", d_xScanSpan, d_yScanSpan);

#endif
  } else {
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
      d_videoPolarity[i] = EDAX_POLARITY_NORMAL;
    }
    d_xScanSpan = EDAX_DEFAULT_MAX_X_SPAN;
    d_yScanSpan = EDAX_DEFAULT_MAX_Y_SPAN;

    d_resolution_index = EDAX_DEFAULT_SCAN_MATRIX;
    d_resolution_x = d_matrixSizeX[d_resolution_index];
    d_resolution_y = d_matrixSizeY[d_resolution_index];
    d_interpixel_delay_nsec = 200;
    d_pix_integrate_nsec = 100;
  }
#ifdef VIRTUAL_SEM
	printf("Warning: you are using the VIRTUAL SEM\n");
#endif
  d_settings_changed_since_last_scan = vrpn_TRUE;
  return 0;
}

vrpn_int32 nmm_Microscope_SEM_EDAX::openEDAXHardware()
{
    // here is where we see if we can actually connect to the SEM
    int result = 0;
#ifndef VIRTUAL_SEM
    ULONG boardNum = 0;
    result = InitGpuBoard(boardNum);
#endif
    if (result == EDAX_ERROR) {
        fprintf(stderr, "initializeHardware: InitGpuBoard failure\n");
        return -1;
    }
    return 0;
}

vrpn_int32 nmm_Microscope_SEM_EDAX::closeEDAXHardware()
{
    int result = 0;
#ifndef VIRTUAL_SEM
	result = ResetGpuBoard();
#endif
    if (result == EDAX_ERROR) {
        fprintf(stderr, "closeHardware: ResetGpuBoard failure\n");
        return -1;
    }
    return 0;
}

vrpn_int32 nmm_Microscope_SEM_EDAX::openScanControlInterface()
{
    int result = 0, enable = EDAX_TRUE;
#ifndef VIRTUAL_SEM
    result = SgEmia(EDAX_WRITE, &enable);
#endif
    if (result == EDAX_ERROR) {
        fprintf(stderr, "initializeHardware: SgEmia failure\n");
        return -1;
    }
    return 0;
}

vrpn_int32 nmm_Microscope_SEM_EDAX::closeScanControlInterface()
{
    int result = 0, enable = EDAX_FALSE;
#ifndef VIRTUAL_SEM
    result = SgEmia(EDAX_WRITE, &enable);
#endif
    if (result == EDAX_ERROR) {
        fprintf(stderr, "closeHardware: SgEmia failure\n");
        return -1;
    }
    return 0;
}

vrpn_int32 nmm_Microscope_SEM_EDAX::setHardwareConfiguration()
{
    int result;
#ifndef VIRTUAL_SEM
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
#endif
    d_resolution_index = EDAX_DEFAULT_SCAN_MATRIX;
    d_resolution_x = d_matrixSizeX[d_resolution_index];
    d_resolution_y = d_matrixSizeY[d_resolution_index];
    
    configureScan();
	return 0;
}

vrpn_int32 nmm_Microscope_SEM_EDAX::configureScan()
{
    d_settings_changed_since_last_scan = vrpn_TRUE;
    // pixel delay specified in usec, integration time is in 100 ns units
    // this call is really only to set the inter-pixel delay, the next two
    // functions are responsible for setting the pixel integration time and
    // the documentation says they should be called after this function,
    // not before
#ifndef VIRTUAL_SEM

	int result;
    result = SpDwel(d_interpixel_delay_nsec/1000, 0, d_pix_integrate_nsec/100);
	if (result == EDAX_ERROR) {
		fprintf(stderr, "configureScan: SpDwel failed\n");
		return -1;
	}

    // estimate ignoring integration time
    double msec_per_frame = d_resolution_y*0.5 + 
                            d_resolution_x*d_resolution_y*0.002;
    double msec_integration_per_frame = 
                 0.000001*d_resolution_x*d_resolution_y*d_pix_integrate_nsec;

    printf("configureScan: estimated duration of image acquisition is ");
    printf("%g msec;\n  actual integration time is %g msec\n", 
           msec_per_frame, msec_integration_per_frame);
    // division by 100 is to specify integration time in 100 ns units
    result = SetupSgScan(0, d_xScanSpan-1, 0, d_yScanSpan-1,
        d_pix_integrate_nsec/100, d_xScanSpan/d_resolution_x, 
		d_yScanSpan/d_resolution_y);


	if (result == EDAX_ERROR) {
		fprintf(stderr, "configureScan: SetupSgScan failed\n");
		return -1;
	}
	
#endif
	return 0;
}

vrpn_int32 nmm_Microscope_SEM_EDAX::setResolution
				(vrpn_int32 res_x, vrpn_int32 res_y)
{
  // set to one of the discrete settings based on last setting and
  // the new setting
  int i;
  vrpn_bool res_valid = vrpn_false;
  for (i = 0; i < EDAX_NUM_SCAN_MATRICES; i++) {
    if (d_matrixSizeX[i] == res_x && d_matrixSizeY[i] == res_y){
      d_resolution_index = i;
      res_valid = vrpn_true;
      break;
    }
  }
  if (res_valid){
    d_resolution_x = res_x;
    d_resolution_y = res_y;
    delete d_scanBuffer;
#ifdef _WIN32
    d_scanBuffer = new UCHAR[d_resolution_x*d_resolution_y];
#else
    d_scanBuffer = new vrpn_uint8[d_resolution_x*d_resolution_y];
#endif
    configureScan();
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

/* **********************************************************************
   text of an email from Mike Lupu (software guy at EDAX) with regard to 
   delays in scanning:

The hardware speed limitation comes from:
 
 (1) Moving the beam and allowing time at each location for the beam to
settle - dependent on microscope performance.
        Range: ~2-4 microsecs for the locations on the same scan line, ~500
microsecs for a new line.

 (2) Reading/digitizing one signal is ~100 nanoseconds. The integration
factor selectable in the user interface defines how many ADC reads per pixel
are used in averaging. Selecting 0 has a special meaning: data are buffered
and transferred as one byte (the most significant) per pixel, instead of
two.

 (3) Data transfer to the host.

*************************************************************************/


vrpn_int32 nmm_Microscope_SEM_EDAX::setPixelIntegrationTime
				(vrpn_int32 time_nsec)
{
    vrpn_int32 num_reads = (time_nsec)/100;
    if (num_reads == 0) num_reads = 1;
    d_pix_integrate_nsec = num_reads*100;
    configureScan();
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

    if (num_usecs == 0){
        num_usecs = 1;
    }
    d_interpixel_delay_nsec = num_usecs*1000;

    configureScan();
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


vrpn_int32 nmm_Microscope_SEM_EDAX::acquireImage()
{
  int i;
  double t_SetSgScan, t_SetupSgColl, t_CollectSgLine, t_reportScanlineData;

#ifndef VIRTUAL_SEM
  int result;
  struct timeval t0, t1, t2, t3;
  
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
    result = CollectSgLine(&(d_scanBuffer[i*d_resolution_x]));
	//printf("finished collecting\n");
	if (result == EDAX_ERROR) {
		fprintf(stderr, "Error collecting scan line for row %d\n", i);
	}
    gettimeofday(&t1, NULL);
    //reportWindowLineData(i);
    reportScanlineData(i);
    gettimeofday(&t2, NULL);
    t0 = vrpn_TimevalDiff(t1, t0);
    t1 = vrpn_TimevalDiff(t2, t1);
    t_CollectSgLine += vrpn_TimevalMsecs(t0);
    t_reportScanlineData += vrpn_TimevalMsecs(t1);
  }

#else		// ifdef VIRTUAL_SEM
  // make some fake data:
  static int count = 0;
  int j;

  for (i = 0; i < d_resolution_y; i++){
    memset(&(d_scanBuffer[i*d_resolution_x]), 
	((i+((int)(count)))%d_resolution_y)*255/d_resolution_y, d_resolution_x);
    //reportWindowLineData(i);
    reportScanlineData(i);
  }

  count++;
#endif

  if (d_settings_changed_since_last_scan) {
     printf("acquireImage - measured times for acquisition operations:\n");
     printf("  SetSgScan x 1 : %g msec\n", t_SetSgScan);
     printf("  SetupSgColl x 1 : %g msec\n", t_SetupSgColl);
     printf("  CollectSgLine x %d : %g msec\n", d_resolution_y,
               t_CollectSgLine);
     printf("  reportScanlineData x %d : %g msec\n", d_resolution_y,
               t_reportScanlineData);
  }

  d_settings_changed_since_last_scan = vrpn_FALSE;
  return 0;
}

vrpn_int32 nmm_Microscope_SEM_EDAX::reportWindowLineData(int line_num)
{
  // line data is at d_scanBuffer[line_num*d_resolution_x]
  char *msgbuf;
  vrpn_int32 len;
  struct timeval now;
  vrpn_int32 offset = 0;
  vrpn_uint8 *data = &(d_scanBuffer[line_num*d_resolution_x]);

  gettimeofday(&now, NULL);

  msgbuf = encode_WindowLineData(&len, 0, line_num, 1, 0,
               d_resolution_x, 1, &offset, now.tv_sec, now.tv_usec, 
               &data);
  if (!msgbuf){
    return -1;
  }
  return dispatchMessage(len, msgbuf, d_WindowLineData_type);
}

vrpn_int32 nmm_Microscope_SEM_EDAX::reportScanlineData(int line_num)
{
  // line data is at d_scanBuffer[line_num*d_resolution_x]
  char *msgbuf;
  vrpn_int32 len;
  struct timeval now;
  vrpn_uint8 *data = &(d_scanBuffer[line_num*d_resolution_x]);

  gettimeofday(&now, NULL);
  vrpn_int32 lines_per_message = d_lines_per_message;
/*  if (line_num + lines_per_message > d_resolution_y){
      lines_per_message = d_resolution_y - line_num;
  }
  */
  msgbuf = encode_ScanlineData(&len, 0, line_num, 1, 1,
               d_resolution_x, 1, lines_per_message, now.tv_sec, now.tv_usec, 
               &data);
  if (!msgbuf) { 
    return -1;
  }
  return dispatchMessage(len, msgbuf, d_ScanlineData_type);
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
int nmm_Microscope_SEM_EDAX::RcvGotConnection
		(void *_userdata, vrpn_HANDLERPARAM _p)
{
	nmm_Microscope_SEM_EDAX *me = 
			(nmm_Microscope_SEM_EDAX *)_userdata;
  
	me->reportResolution();
    me->reportPixelIntegrationTime();
    me->reportInterPixelDelayTime();
	printf("Taking control of SEM scanning\n");
    me->openScanControlInterface();
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
		printf("Relinquishing control of SEM scanning\n");
        me->closeScanControlInterface();
        return 0;
}
