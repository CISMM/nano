#include "nmm_Microscope_SEM_Remote.h"

nmm_Microscope_SEM_Remote::nmm_Microscope_SEM_Remote
    (const char * name, vrpn_Connection *cn):
    nmb_Device_Client(name, cn?cn:vrpn_get_connection_by_name(name)),
    nmm_Microscope_SEM (name, d_connection),
    d_resolutionX(0), d_resolutionY(0),
    d_pixelIntegrationTime_nsec(0), 
    d_interpixelDelayTime_nsec(0),
    d_startX(0), d_startY(0), d_dX(0), d_dY(0),
    d_lineLength(0), d_numFields(0), d_numLines(0), d_pixelType(NMB_UINT8),
    d_dataBuffer(NULL),
    d_dataBufferSize(0),
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

vrpn_int32 nmm_Microscope_SEM_Remote::mainloop(void)
{
  if (d_connection){
    if ((d_connection->mainloop()) == -1)
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


