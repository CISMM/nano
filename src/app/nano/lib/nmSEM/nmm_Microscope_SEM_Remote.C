#include "nmm_Microscope_SEM_Remote.h"

nmm_Microscope_SEM_Remote::nmm_Microscope_SEM_Remote
    (const char * name, vrpn_Connection *cn):
    nmb_Device_Client(name, cn?cn:vrpn_get_connection_by_name(name)),
    nmm_Microscope_SEM (name, d_connection),
    d_resolutionX(512), d_resolutionY(400),
    d_pixelIntegrationTime_nsec(0), 
    d_interpixelDelayTime_nsec(0),
    d_startX(0), d_startY(0), d_dX(0), d_dY(0),
    d_lineLength(512), d_numFields(0), d_numLines(0), d_pixelType(NMB_UINT8),
    d_dataBuffer(NULL),
    d_dataBufferSize(0),
    d_pointDwellTime_nsec(0),
    d_beamBlankEnabled(0),
    d_maxScanX(4096), d_maxScanY(3200),
    d_pointScanX(0), d_pointScanY(0),
    d_hRetraceDelay_nsec(0), d_vRetraceDelay_nsec(0),
    d_xGain(0), d_xOffset(0), 
    d_yGain(0), d_yOffset(0), 
    d_zGain(0), d_zOffset(0),
    d_externalScanControlEnabled(0),
    d_magnification(1000),
    d_magCalibration(1e8), // (10 cm)*(nm/cm) - since 10cm is the standard
                           // SEM display width
    d_numPointsTotal(0),
    d_numPointsDone(0),
    d_timeTotal_sec(0.0),
    d_timeDone_sec(0.0),
    d_messageHandlerList(NULL)
{
  if (d_connection == NULL){
    fprintf(stderr, "nmm_Microscope_SEM_Remote: No connection\n");
    return;
  }
  if (d_connection->register_handler(d_ReportResolution_type,
		RcvReportResolution, this)) {
    fprintf(stderr,
           "nmm_Microscope_SEM_Remote: can't register resolution handler\n");
    return;
  }
  if (d_connection->register_handler(d_ReportPixelIntegrationTime_type,
                RcvReportPixelIntegrationTime, this)) {
    fprintf(stderr,
           "nmm_Microscope_SEM_Remote: can't register integration handler\n");
    return;
  }
  if (d_connection->register_handler(d_ReportInterPixelDelayTime_type,
                RcvReportInterPixelDelayTime, this)) {
    fprintf(stderr,
           "nmm_Microscope_SEM_Remote: can't register delay handler\n");
    return;
  }
  if (d_connection->register_handler(d_ScanlineData_type,
                RcvScanlineData, this)) {
    fprintf(stderr,
           "nmm_Microscope_SEM_Remote: can't register scanline data handler\n");
    return;
  }
  if (d_connection->register_handler(d_ReportPointDwellTime_type,
                RcvReportPointDwellTime, this)) {
    fprintf(stderr,
           "nmm_Microscope_SEM_Remote: can't register point dwell handler\n");
    return;
  }
  if (d_connection->register_handler(d_ReportBeamBlankEnable_type,
                RcvReportBeamBlankEnable, this)) {
    fprintf(stderr,
           "nmm_Microscope_SEM_Remote: can't register beam blank handler\n");
    return;
  }
  if (d_connection->register_handler(d_ReportMaxScanSpan_type,
                RcvReportMaxScanSpan, this)) {
    fprintf(stderr,
           "nmm_Microscope_SEM_Remote: can't register max scan handler\n");
    return;
  }
  if (d_connection->register_handler(d_ReportBeamLocation_type,
                RcvReportBeamLocation, this)) {
    fprintf(stderr,
           "nmm_Microscope_SEM_Remote: can't register beam location handler\n");
    return;
  }
  if (d_connection->register_handler(d_ReportRetraceDelays_type,
                RcvReportRetraceDelays, this)) {
    fprintf(stderr,
           "nmm_Microscope_SEM_Remote: can't register retrace delay handler\n");
    return;
  }
  if (d_connection->register_handler(d_ReportDACParams_type,
                RcvReportDACParams, this)) {
    fprintf(stderr,
           "nmm_Microscope_SEM_Remote: can't register DAC param handler\n");
    return;
  }
  if (d_connection->register_handler(d_ReportExternalScanControlEnable_type,
                RcvReportExternalScanControlEnable, this)) {
    fprintf(stderr,
       "nmm_Microscope_SEM_Remote: can't register external control handler\n");
    return;
  }
  if (d_connection->register_handler(d_ReportMagnification_type,
                RcvReportMagnification, this)) {
    fprintf(stderr,
       "nmm_Microscope_SEM_Remote: can't register mag handler\n");
    return;
  }
  if (d_connection->register_handler(d_ReportExposureStatus_type,
                RcvReportExposureStatus, this)) {
    fprintf(stderr,
       "nmm_Microscope_SEM_Remote: can't register exposure status handler\n");
    return;
  }

  if (!d_connection->connected()) {
    fprintf(stderr,
	   "nmm_Microscope_SEM_Remote: Warning, not connected\n");
  } else {
    fprintf(stderr,
           "nmm_Microscope_SEM_Remote: connected to %s\n", name);
  }

}

nmm_Microscope_SEM_Remote::~nmm_Microscope_SEM_Remote()
{
  if (d_dataBuffer) {
    delete d_dataBuffer;
  }
}

vrpn_int32 nmm_Microscope_SEM_Remote::mainloop(const struct timeval * timeout)
{
  if (d_connection){
    if ((d_connection->mainloop(timeout)) == -1)
       return -1;
  }
  return 0;
}

int nmm_Microscope_SEM_Remote::setResolution(vrpn_int32 res_x, vrpn_int32 res_y)
{
  char *msgbuf;
  vrpn_int32 len;

  msgbuf = encode_SetResolution(&len, res_x, res_y);
  if (!msgbuf){
    return -1;
  }
  
  return dispatchMessage(len, msgbuf, d_SetResolution_type);
}

int nmm_Microscope_SEM_Remote::setPixelIntegrationTime(vrpn_int32 time_nsec)
{
  char *msgbuf;
  vrpn_int32 len;

  msgbuf = encode_SetPixelIntegrationTime(&len, time_nsec);
  if (!msgbuf){
    return -1;
  }

  return dispatchMessage(len, msgbuf, d_SetPixelIntegrationTime_type);
}

int nmm_Microscope_SEM_Remote::setInterPixelDelayTime(vrpn_int32 time_nsec)
{
  char *msgbuf;
  vrpn_int32 len;

  msgbuf = encode_SetInterPixelDelayTime(&len, time_nsec);
  if (!msgbuf){
    return -1;
  }

  return dispatchMessage(len, msgbuf, d_SetInterPixelDelayTime_type);
}

int nmm_Microscope_SEM_Remote::requestScan(vrpn_int32 nscans)
{
  char *msgbuf;
  vrpn_int32 len;

  msgbuf = encode_RequestScan(&len, nscans);
  if (!msgbuf){
    return -1;
  }

  return dispatchMessage(len, msgbuf, d_RequestScan_type);
}

int nmm_Microscope_SEM_Remote::setPointDwellTime(vrpn_int32 time_nsec)
{
  char *msgbuf;
  vrpn_int32 len;

  msgbuf = encode_SetPointDwellTime(&len, time_nsec);
  if (!msgbuf){
    return -1;
  }

  return dispatchMessage(len, msgbuf, d_SetPointDwellTime_type);
}

int nmm_Microscope_SEM_Remote::setBeamBlankEnable(vrpn_int32 enable)
{
  char *msgbuf;
  vrpn_int32 len;

  msgbuf = encode_SetBeamBlankEnable(&len, enable);
  if (!msgbuf){
    return -1;
  }

  return dispatchMessage(len, msgbuf, d_SetBeamBlankEnable_type);
}

int nmm_Microscope_SEM_Remote::goToPoint(vrpn_int32 x, vrpn_int32 y)
{
  char *msgbuf;
  vrpn_int32 len;

  msgbuf = encode_GoToPoint(&len, x, y);
  if (!msgbuf){
    return -1;
  }

  return dispatchMessage(len, msgbuf, d_GoToPoint_type);
}

int nmm_Microscope_SEM_Remote::setRetraceDelays(
               vrpn_int32 h_time_nsec, vrpn_int32 v_time_nsec)
{
  char *msgbuf;
  vrpn_int32 len;

  msgbuf = encode_SetRetraceDelays(&len, h_time_nsec, v_time_nsec);
  if (!msgbuf){
    return -1;
  }

  return dispatchMessage(len, msgbuf, d_SetRetraceDelays_type);
}

int nmm_Microscope_SEM_Remote::setDACParams(
                     vrpn_int32 x_gain, vrpn_int32 x_offset,
                     vrpn_int32 y_gain, vrpn_int32 y_offset,
                     vrpn_int32 z_gain, vrpn_int32 z_offset)
{
  char *msgbuf;
  vrpn_int32 len;

  msgbuf = encode_SetDACParams(&len, x_gain, x_offset, y_gain, y_offset,
                                     z_gain, z_offset);
  if (!msgbuf){
    return -1;
  }

  return dispatchMessage(len, msgbuf, d_SetDACParams_type);
}

int nmm_Microscope_SEM_Remote::setExternalScanControlEnable(vrpn_int32 enable)
{
  char *msgbuf;
  vrpn_int32 len;

  msgbuf = encode_SetExternalScanControlEnable(&len, enable);
  if (!msgbuf){
    return -1;
  }

  return dispatchMessage(len, msgbuf, d_SetExternalScanControlEnable_type);
}

int nmm_Microscope_SEM_Remote::clearExposePattern()
{
  char *msgbuf = NULL;
  vrpn_int32 len = 0;

  return dispatchMessage(len, msgbuf, d_ClearExposePattern_type);
}

int nmm_Microscope_SEM_Remote::addPolygon(
               vrpn_float32 exposure_uCoul_per_cm2, vrpn_int32 numPoints,
               vrpn_float32 *x_nm, vrpn_float32 *y_nm)
{
  char *msgbuf;
  vrpn_int32 len;

  msgbuf = encode_AddPolygon(&len, exposure_uCoul_per_cm2, numPoints,
                             x_nm, y_nm);
  if (!msgbuf){
    return -1;
  }

  return dispatchMessage(len, msgbuf, d_AddPolygon_type);
}

int nmm_Microscope_SEM_Remote::addPolyline(
                    vrpn_float32 exposure_pCoul_per_cm,
                    vrpn_float32 exposure_uCoul_per_cm2,
                    vrpn_float32 lineWidth_nm, vrpn_int32 numPoints,
                    vrpn_float32 *x_nm, vrpn_float32 *y_nm)
{
  char *msgbuf;
  vrpn_int32 len;

  msgbuf = encode_AddPolyline(&len, exposure_pCoul_per_cm,
                              exposure_uCoul_per_cm2, lineWidth_nm,
                              numPoints, x_nm, y_nm);
  if (!msgbuf){
    return -1;
  }

  return dispatchMessage(len, msgbuf, d_AddPolyline_type);
}

int nmm_Microscope_SEM_Remote::addDumpPoint(
                    vrpn_float32 x_nm, vrpn_float32 y_nm)
{
  char *msgbuf;
  vrpn_int32 len;

  msgbuf = encode_AddDumpPoint(&len, x_nm, y_nm);
  if (!msgbuf){
    return -1;
  }

  return dispatchMessage(len, msgbuf, d_AddDumpPoint_type);
}

int nmm_Microscope_SEM_Remote::exposePattern()
{
  char *msgbuf = NULL;
  vrpn_int32 len = 0;

  return dispatchMessage(len, msgbuf, d_ExposePattern_type);
}

int nmm_Microscope_SEM_Remote::exposureTimingTest()
{
  char *msgbuf = NULL;
  vrpn_int32 len = 0;

  return dispatchMessage(len, msgbuf, d_ExposureTimingTest_type);
}

int nmm_Microscope_SEM_Remote::setBeamCurrent(vrpn_float32 current_picoAmps)
{
  char *msgbuf;
  vrpn_int32 len;

  msgbuf = encode_SetBeamCurrent(&len, current_picoAmps);
  if (!msgbuf){
    return -1;
  }

  return dispatchMessage(len, msgbuf, d_SetBeamCurrent_type);
}

int nmm_Microscope_SEM_Remote::setBeamWidth(vrpn_float32 beamWidth_nm)
{
  char *msgbuf;
  vrpn_int32 len;

  msgbuf = encode_SetBeamWidth(&len, beamWidth_nm);
  if (!msgbuf){
    return -1;
  }

  return dispatchMessage(len, msgbuf, d_SetBeamWidth_type);
}

int nmm_Microscope_SEM_Remote::setPointReportEnable(vrpn_int32 enable)
{
  char *msgbuf;
  vrpn_int32 len;

  msgbuf = encode_SetPointReportEnable(&len, enable);
  if (!msgbuf){
    return -1;
  }

  return dispatchMessage(len, msgbuf, d_SetPointReportEnable_type);
}

int nmm_Microscope_SEM_Remote::setDotSpacing(vrpn_float32 spacing_nm)
{
  char *msgbuf;
  vrpn_int32 len;

  msgbuf = encode_SetDotSpacing(&len, spacing_nm);
  if (!msgbuf){
    return -1;
  }

  return dispatchMessage(len, msgbuf, d_SetDotSpacing_type);
}

int nmm_Microscope_SEM_Remote::setLineSpacing(vrpn_float32 spacing_nm)
{
  char *msgbuf;
  vrpn_int32 len;

  msgbuf = encode_SetLineSpacing(&len, spacing_nm);
  if (!msgbuf){
    return -1;
  }

  return dispatchMessage(len, msgbuf, d_SetLineSpacing_type);
}

int nmm_Microscope_SEM_Remote::setLinearExposure(vrpn_float32 pCoul_per_cm)
{
  char *msgbuf;
  vrpn_int32 len;

  msgbuf = encode_SetLinearExposure(&len, pCoul_per_cm);
  if (!msgbuf){
    return -1;
  }

  return dispatchMessage(len, msgbuf, d_SetLinearExposure_type);
}

int nmm_Microscope_SEM_Remote::setAreaExposure(vrpn_float32 uCoul_per_sq_cm)
{
  char *msgbuf;
  vrpn_int32 len;

  msgbuf = encode_SetAreaExposure(&len, uCoul_per_sq_cm);
  if (!msgbuf){
    return -1;
  }

  return dispatchMessage(len, msgbuf, d_SetAreaExposure_type);
}

int nmm_Microscope_SEM_Remote::setMagnification(vrpn_float32 mag)
{
  char *msgbuf;
  vrpn_int32 len;

  msgbuf = encode_SetMagnification(&len, mag);
  if (!msgbuf){
    return -1;
  }

  return dispatchMessage(len, msgbuf, d_SetMagnification_type);
}

int nmm_Microscope_SEM_Remote::registerChangeHandler(void *userdata,
        nmm_Microscope_SEM_Remote_ChangeHandler_t handler)
{
  msg_handler_list_t *new_entry;
  if (handler == NULL) {
    fprintf(stderr, 
       "nmm_Microscope_SEM_Remote::registerChangeHandler: NULL handler\n");
    return -1;
  }
  if ( (new_entry = new msg_handler_list_t) == NULL) {
    fprintf(stderr,
       "nmm_Microscope_SEM_Remote::registerChangeHandler: out of memory\n");
    return -1;
  }
  new_entry->handler = handler;
  new_entry->userdata = userdata;
  new_entry->next = d_messageHandlerList;
  d_messageHandlerList = new_entry;

  return 0;
}

int nmm_Microscope_SEM_Remote::unregisterChangeHandler(void *userdata,
        nmm_Microscope_SEM_Remote_ChangeHandler_t handler)
{
  msg_handler_list_t *victim, **snitch;

  // Find a handler with this registry in the list
  snitch = &d_messageHandlerList;
  victim = *snitch;
  while ( (victim != NULL) &&
          ( (victim->handler != handler) ||
            (victim->userdata != userdata) )) {
    snitch = &( (*snitch)->next );
    victim = victim->next;
  }

  // make sure we found one
  if (victim == NULL) {
    fprintf(stderr,
           "nmm_Microscope_SEM_Remote::unregisterChangeHandler:"
           " no such handler\n");
    return -1;
  }

  // remove the entry from the list
  *snitch = victim->next;
  delete victim;

  return 0;
}

void nmm_Microscope_SEM_Remote::getResolution
                           (vrpn_int32 &res_x, vrpn_int32 &res_y)
{
  res_x = d_resolutionX; res_y = d_resolutionY;
}

void nmm_Microscope_SEM_Remote::getPixelIntegrationTime
                           (vrpn_int32 &time_nsec)
{
  time_nsec = d_pixelIntegrationTime_nsec;
}

void nmm_Microscope_SEM_Remote::getInterPixelDelayTime(vrpn_int32 &time_nsec)
{
  time_nsec = d_interpixelDelayTime_nsec;
}

void nmm_Microscope_SEM_Remote::getScanlineData
                      (vrpn_int32 &start_x, vrpn_int32 &start_y,
                       vrpn_int32 &dx, vrpn_int32 &dy, vrpn_int32 &line_length,
                       vrpn_int32 &num_fields, vrpn_int32 &num_lines,
                       nmb_PixelType &pix_type,
                       void **data)
{
  start_x = d_startX; start_y = d_startY;
  dx = d_dX; dy = d_dY; line_length = d_lineLength;
  num_fields = d_numFields;
  num_lines = d_numLines;
  pix_type = d_pixelType;
  *data = d_dataBuffer;
}

void nmm_Microscope_SEM_Remote::getPointDwellTime(vrpn_int32 &time_nsec)
{
  time_nsec = d_pointDwellTime_nsec;
}

void nmm_Microscope_SEM_Remote::getBeamBlankEnabled(vrpn_int32 &enabled)
{
  enabled = d_beamBlankEnabled;
}

void nmm_Microscope_SEM_Remote::getMaxScan(vrpn_int32 &x, vrpn_int32 &y)
{
  x = d_maxScanX;
  y = d_maxScanY;
}

void nmm_Microscope_SEM_Remote::getBeamLocation(vrpn_int32 &x, vrpn_int32 &y)
{
  x = d_pointScanX;
  y = d_pointScanY;
}

void nmm_Microscope_SEM_Remote::getRetraceDelays
               (vrpn_int32 &h_time_nsec, vrpn_int32 &v_time_nsec)
{
  h_time_nsec = d_hRetraceDelay_nsec;
  v_time_nsec = d_vRetraceDelay_nsec;
}

void nmm_Microscope_SEM_Remote::getDACParams
               (vrpn_int32 &x_gain, vrpn_int32 &x_offset,
                      vrpn_int32 &y_gain, vrpn_int32 &y_offset,
                      vrpn_int32 &z_gain, vrpn_int32 &z_offset)
{
  x_gain = d_xGain;
  x_offset = d_xOffset;
  y_gain = d_yGain;
  y_offset = d_yOffset;
  z_gain = d_zGain;
  z_offset = d_zOffset;
}

void nmm_Microscope_SEM_Remote::getExternalScanControlEnable(
                                vrpn_int32 &enabled)
{
  enabled = d_externalScanControlEnabled;
}

void nmm_Microscope_SEM_Remote::getMagnification(vrpn_float32 &mag)
{
  mag = d_magnification;
}

void nmm_Microscope_SEM_Remote::getScanRegion_nm(
                          double &width_nm, double &height_nm)
{
  width_nm = d_magCalibration/d_magnification;
  height_nm = width_nm*(double)d_resolutionY/(double)d_resolutionX;
}

void nmm_Microscope_SEM_Remote::getExposureStatus(vrpn_int32 &numPointsTotal,
       vrpn_int32 &numPointsDone, vrpn_float32 &timeTotal_sec,
       vrpn_float32 &timeDone_sec)
{
  numPointsTotal = d_numPointsTotal;
  numPointsDone = d_numPointsDone;
  timeTotal_sec = d_timeTotal_sec;
  timeDone_sec = d_timeDone_sec;
}

void nmm_Microscope_SEM_Remote::convert_nm_to_DAC(const double x_nm, 
                           const double y_nm,
                           int &xDAC, int &yDAC)
{
  nmm_Microscope_SEM::convert_nm_to_DAC((double)d_magnification,
                   d_resolutionX, d_resolutionY,
                   d_maxScanX, d_maxScanY,
                   x_nm, y_nm, xDAC, yDAC);
}

void nmm_Microscope_SEM_Remote::convert_DAC_to_nm(const int xDAC, 
                                                  const int yDAC,
                           double &x_nm, double &y_nm)
{
  nmm_Microscope_SEM::convert_DAC_to_nm((double)d_magnification,
                   d_resolutionX, d_resolutionY,
                   d_maxScanX, d_maxScanY,
                   xDAC, yDAC, x_nm, y_nm);
}

//static
int nmm_Microscope_SEM_Remote::RcvReportResolution (void *_userdata, 
    vrpn_HANDLERPARAM _p)
{
  nmm_Microscope_SEM_Remote *me = (nmm_Microscope_SEM_Remote *)_userdata;
  const char * bufptr = _p.buffer;
  vrpn_int32 res_x, res_y;

  if (decode_ReportResolution(&bufptr, &res_x, &res_y) == -1) {
    fprintf(stderr,
        "nmm_Microscope_SEM_Remote::RcvReportResolution: decode failed\n");
    return -1;
  }
  me->d_resolutionX = res_x;
  me->d_resolutionY = res_y;
  return me->notifyMessageHandlers(REPORT_RESOLUTION, _p.msg_time);
}

//static
int nmm_Microscope_SEM_Remote::RcvReportPixelIntegrationTime
    (void *_userdata, vrpn_HANDLERPARAM _p)
{
  nmm_Microscope_SEM_Remote *me = (nmm_Microscope_SEM_Remote *)_userdata;
  const char * bufptr = _p.buffer;
  vrpn_int32 time_nsec;

  if (decode_ReportPixelIntegrationTime(&bufptr, &time_nsec) == -1) {
    fprintf(stderr,
        "nmm_Microscope_SEM_Remote::RcvReportPixelIntegrationTime: "
        "decode failed\n");
    return -1;
  }
  me->d_pixelIntegrationTime_nsec = time_nsec;
  return me->notifyMessageHandlers(REPORT_PIXEL_INTEGRATION_TIME, _p.msg_time);
}

//static
int nmm_Microscope_SEM_Remote::RcvReportInterPixelDelayTime
    (void *_userdata, vrpn_HANDLERPARAM _p)
{
  nmm_Microscope_SEM_Remote *me = (nmm_Microscope_SEM_Remote *)_userdata;
  const char * bufptr = _p.buffer;
  vrpn_int32 time_nsec;

  if (decode_ReportInterPixelDelayTime(&bufptr, &time_nsec) == -1) {
    fprintf(stderr,
        "nmm_Microscope_SEM_Remote::RcvReportInterPixelDelayTime: "
        "decode failed\n");
    return -1;
  }
  me->d_interpixelDelayTime_nsec = time_nsec;
  return me->notifyMessageHandlers(REPORT_INTERPIXEL_DELAY_TIME, _p.msg_time);
}

//static
int nmm_Microscope_SEM_Remote::RcvScanlineData(void *_userdata, 
    vrpn_HANDLERPARAM _p)
{
  vrpn_int32 start_x, start_y, dx, dy, line_length, num_fields, num_lines;
  void *data;
  nmm_Microscope_SEM_Remote *me = (nmm_Microscope_SEM_Remote *)_userdata;
  const char * bufptr = _p.buffer;
  vrpn_int32 sec, usec;
  vrpn_int32 pixelTypeInt32;
  nmb_PixelType pixelType;
  int pixelSize;

  if (decode_ScanlineDataHeader(&bufptr, &start_x, &start_y,
           &dx, &dy, &line_length, &num_fields, &num_lines, &sec, &usec,
           &pixelTypeInt32) 
           == -1) {
    fprintf(stderr,
        "nmm_Microscope_SEM_Remote::RcvScanlineData: header decode failed\n");
    return -1;
  }

  switch(pixelTypeInt32) {
    case NMB_UINT8:
      pixelType = NMB_UINT8;
      pixelSize = sizeof(vrpn_uint8);
      break;
    case NMB_UINT16:
      pixelType = NMB_UINT16;
      pixelSize = sizeof(vrpn_uint16);
      break;
    case NMB_FLOAT32:
      pixelType = NMB_FLOAT32;
      pixelSize = sizeof(vrpn_float32);
      break;
    default:
      fprintf(stderr, "SEM_remote::Scanline data: unknown pixel type\n");
      return -1;
  }

  if (me->d_dataBufferSize != pixelSize*line_length*num_fields*num_lines){
      if (me->d_dataBuffer){
        delete [] (me->d_dataBuffer);
      }
      me->d_dataBufferSize = pixelSize*num_fields*line_length*num_lines;
      me->d_dataBuffer = new char[me->d_dataBufferSize];
  }
  data = (void *)(me->d_dataBuffer);

  if (decode_ScanlineDataLine (&bufptr, line_length, num_fields,
                               num_lines, pixelType, data) == -1) {
    fprintf(stderr,
      "nmm_Microscope_SEM_Remote::RcvScanlineData: decode failed\n");
    return -1;
  }

  me->d_startX = start_x;
  me->d_startY = start_y;
  me->d_dX = dx;
  me->d_dY = dy;
  me->d_lineLength = line_length;
  me->d_numFields = num_fields;
  me->d_numLines = num_lines;
  me->d_pixelType = pixelType;
  return me->notifyMessageHandlers(SCANLINE_DATA, _p.msg_time);
}

//static
int nmm_Microscope_SEM_Remote::RcvReportPointDwellTime
    (void *_userdata, vrpn_HANDLERPARAM _p)
{
  nmm_Microscope_SEM_Remote *me = (nmm_Microscope_SEM_Remote *)_userdata;
  const char * bufptr = _p.buffer;
  vrpn_int32 time_nsec;

  if (decode_ReportPointDwellTime(&bufptr, &time_nsec) == -1) {
    fprintf(stderr,
        "nmm_Microscope_SEM_Remote::RcvPointDwellTime: "
        "decode failed\n");
    return -1;
  }
  me->d_pointDwellTime_nsec = time_nsec;
  return me->notifyMessageHandlers(POINT_DWELL_TIME, _p.msg_time);
}

//static 
int nmm_Microscope_SEM_Remote::RcvReportBeamBlankEnable
     (void *_userdata, vrpn_HANDLERPARAM _p)
{
  nmm_Microscope_SEM_Remote *me = (nmm_Microscope_SEM_Remote *)_userdata;
  const char * bufptr = _p.buffer;
  vrpn_int32 enable;

  if (decode_ReportBeamBlankEnable(&bufptr, &enable) == -1) {
    fprintf(stderr,
        "nmm_Microscope_SEM_Remote::RcvBeamBlankEnable: "
        "decode failed\n");
    return -1;
  }
  me->d_beamBlankEnabled = enable;
  return me->notifyMessageHandlers(BEAM_BLANK_ENABLE, _p.msg_time);
}

//static 
int nmm_Microscope_SEM_Remote::RcvReportMaxScanSpan
                (void *_userdata, vrpn_HANDLERPARAM _p)
{
  nmm_Microscope_SEM_Remote *me = (nmm_Microscope_SEM_Remote *)_userdata;
  const char * bufptr = _p.buffer;
  vrpn_int32 x, y;

  if (decode_ReportMaxScanSpan(&bufptr, &x, &y) == -1) {
    fprintf(stderr,
        "nmm_Microscope_SEM_Remote::RcvMaxScanSpan: "
        "decode failed\n");
    return -1;
  }
  me->d_maxScanX = x;
  me->d_maxScanY = y;
  return me->notifyMessageHandlers(MAX_SCAN_SPAN, _p.msg_time);
}

//static 
int nmm_Microscope_SEM_Remote::RcvReportBeamLocation
                (void *_userdata, vrpn_HANDLERPARAM _p)
{
  nmm_Microscope_SEM_Remote *me = (nmm_Microscope_SEM_Remote *)_userdata;
  const char * bufptr = _p.buffer;
  vrpn_int32 x, y;

  if (decode_ReportBeamLocation(&bufptr, &x, &y) == -1) {
    fprintf(stderr,
        "nmm_Microscope_SEM_Remote::RcvBeamLocation: "
        "decode failed\n");
    return -1;
  }
  me->d_pointScanX = x;
  me->d_pointScanY = y;
  return me->notifyMessageHandlers(BEAM_LOCATION, _p.msg_time);
}

//static 
int nmm_Microscope_SEM_Remote::RcvReportRetraceDelays
               (void *_userdata, vrpn_HANDLERPARAM _p)
{
  nmm_Microscope_SEM_Remote *me = (nmm_Microscope_SEM_Remote *)_userdata;
  const char * bufptr = _p.buffer;
  vrpn_int32 h_delay, v_delay;

  if (decode_ReportRetraceDelays(&bufptr, &h_delay, &v_delay) == -1) {
    fprintf(stderr,
        "nmm_Microscope_SEM_Remote::RcvReportRetraceDelays: "
        "decode failed\n");
    return -1;
  }
  me->d_hRetraceDelay_nsec = h_delay;
  me->d_vRetraceDelay_nsec = v_delay;
  return me->notifyMessageHandlers(RETRACE_DELAYS, _p.msg_time);
}

//static 
int nmm_Microscope_SEM_Remote::RcvReportDACParams
               (void *_userdata, vrpn_HANDLERPARAM _p)
{
  nmm_Microscope_SEM_Remote *me = (nmm_Microscope_SEM_Remote *)_userdata;
  const char * bufptr = _p.buffer;
  vrpn_int32 xg, xo, yg, yo, zg, zo;

  if (decode_ReportDACParams(&bufptr, &xg, &xo, &yg, &yo, &zg, &zo) == -1) {
    fprintf(stderr,
        "nmm_Microscope_SEM_Remote::RcvReportDACParams: "
        "decode failed\n");
    return -1;
  }
  me->d_xGain = xg;
  me->d_xOffset = xo;
  me->d_yGain = yg;
  me->d_yOffset = yo;
  me->d_zGain = zg;
  me->d_zOffset = zo;
  return me->notifyMessageHandlers(DAC_PARAMS, _p.msg_time);
}

//static
int nmm_Microscope_SEM_Remote::RcvReportExternalScanControlEnable
     (void *_userdata, vrpn_HANDLERPARAM _p)
{
  nmm_Microscope_SEM_Remote *me = (nmm_Microscope_SEM_Remote *)_userdata;
  const char * bufptr = _p.buffer;
  vrpn_int32 enable;

  if (decode_ReportExternalScanControlEnable(&bufptr, &enable) == -1) {
    fprintf(stderr,
        "nmm_Microscope_SEM_Remote::RcvReportExternalScanControlEnable: "
        "decode failed\n");
    return -1;
  }
  me->d_externalScanControlEnabled = enable;
  return me->notifyMessageHandlers(EXTERNAL_SCAN_CONTROL_ENABLE, _p.msg_time);
}

//static
int nmm_Microscope_SEM_Remote::RcvReportMagnification
     (void *_userdata, vrpn_HANDLERPARAM _p)
{
  nmm_Microscope_SEM_Remote *me = (nmm_Microscope_SEM_Remote *)_userdata;
  const char * bufptr = _p.buffer;
  vrpn_float32 mag;

  if (decode_ReportMagnification(&bufptr, &mag) == -1) {
    fprintf(stderr,
        "nmm_Microscope_SEM_Remote::RcvReportMagnification: "
        "decode failed\n");
    return -1;
  }
  me->d_magnification = mag;
  return me->notifyMessageHandlers(REPORT_MAGNIFICATION, _p.msg_time);
}

//static
int nmm_Microscope_SEM_Remote::RcvReportExposureStatus
     (void *_userdata, vrpn_HANDLERPARAM _p)
{
  nmm_Microscope_SEM_Remote *me = (nmm_Microscope_SEM_Remote *)_userdata;
  const char * bufptr = _p.buffer;
  vrpn_int32 numPointsTotal, numPointsDone;
  vrpn_float32 timeTotal_sec, timeDone_sec;

  if (decode_ReportExposureStatus(&bufptr, &numPointsTotal, &numPointsDone,
                                  &timeTotal_sec, &timeDone_sec) == -1) {
    fprintf(stderr,
        "nmm_Microscope_SEM_Remote::RcvReportExposureStatus: "
        "decode failed\n");
    return -1;
  }
  me->d_numPointsTotal = numPointsTotal;
  me->d_numPointsDone = numPointsDone;
  me->d_timeTotal_sec = timeTotal_sec;
  me->d_timeDone_sec = timeDone_sec;
  return me->notifyMessageHandlers(REPORT_EXPOSURE_STATUS, _p.msg_time);
}

int nmm_Microscope_SEM_Remote::notifyMessageHandlers(
        nmm_Microscope_SEM::msg_t type, const struct timeval &msg_time)
{
  nmm_Microscope_SEM_ChangeHandlerData handler_info;
  handler_info.msg_time = msg_time;
  handler_info.msg_type = type;
  handler_info.sem = this;

  msg_handler_list_t *handler = d_messageHandlerList;
  while (handler != NULL) {
    handler->handler(handler->userdata, handler_info);
    handler = handler->next;
  }
  return 0;
}
