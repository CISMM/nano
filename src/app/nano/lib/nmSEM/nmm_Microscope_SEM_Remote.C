#include "nmm_Microscope_SEM_Remote.h"

nmm_Microscope_SEM_Remote::nmm_Microscope_SEM_Remote
    (const char * name, vrpn_Connection *cn):
    nmm_Microscope_SEM (name, cn?cn:vrpn_get_connection_by_name(name)),
    d_messageHandlerList(NULL),
    d_resolutionX(0), d_resolutionY(0),
    d_pixelIntegrationTime_nsec(0), 
    d_interpixelDelayTime_nsec(0),
    d_startX(0), d_startY(0), d_dX(0), d_dY(0),
    d_lineLength(0), d_numFields(0), d_numLines(0),
    d_data(NULL), d_uint8_data(NULL)
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
  if (d_connection->register_handler(d_WindowLineData_type,
                RcvWindowLineData, this)) {
    fprintf(stderr,
           "nmm_Microscope_SEM_Remote: can't register line data handler\n");
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
  if (d_data) {
    delete d_data;
  }
  if (d_uint8_data) {
    delete d_uint8_data;
  }
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

void nmm_Microscope_SEM_Remote::getWindowLineData
                      (vrpn_int32 &start_x, vrpn_int32 &start_y,
                       vrpn_int32 &dx, vrpn_int32 &dy, vrpn_int32 &line_length,
                       vrpn_int32 &num_fields, vrpn_float32 **data)
{
  start_x = d_startX; start_y = d_startY;
  dx = d_dX; dy = d_dY; line_length = d_lineLength;
  num_fields = d_numFields;
  *data = d_data;
}

void nmm_Microscope_SEM_Remote::getScanlineData
                      (vrpn_int32 &start_x, vrpn_int32 &start_y,
                       vrpn_int32 &dx, vrpn_int32 &dy, vrpn_int32 &line_length,
                       vrpn_int32 &num_fields, vrpn_int32 &num_lines,
                       vrpn_uint8 **data)
{
  start_x = d_startX; start_y = d_startY;
  dx = d_dX; dy = d_dY; line_length = d_lineLength;
  num_fields = d_numFields;
  num_lines = d_numLines;
  *data = d_uint8_data;
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
int nmm_Microscope_SEM_Remote::RcvWindowLineData(void *_userdata, 
    vrpn_HANDLERPARAM _p)
{
  int i, j;
  vrpn_int32 start_x, start_y, dx, dy, line_length, num_fields;
  vrpn_float32 *data, *pixel_data;
  nmm_Microscope_SEM_Remote *me = (nmm_Microscope_SEM_Remote *)_userdata;
  const char * bufptr = _p.buffer;
  vrpn_int32 sec, usec;

  if (decode_WindowLineDataHeader(&bufptr, &start_x, &start_y,
           &dx, &dy, &line_length, &num_fields, &sec, &usec) == -1) {
    fprintf(stderr,
        "nmm_Microscope_SEM_Remote::RcvWindowLineData: header decode failed\n");
    return -1;
  }
  //printf("length = %d\n", num_fields*line_length);
  data = new vrpn_float32[num_fields*line_length];
  if (!data){
	fprintf(stderr, "RcvWindowLineData: out of memory\n");
	return -1;
  }
  pixel_data = new vrpn_float32[num_fields];
  if (!pixel_data){
        delete data;
	fprintf(stderr, "RcvWindowLineData: out of memory\n");
	return -1;
  }
  for (i = 0; i < line_length; i++){
    if (decode_WindowLineDataField (&bufptr, num_fields, pixel_data) == -1) {
      fprintf(stderr,
         "nmm_Microscope_SEM_Remote::RcvWindowLineData: pixel decode failed\n");
      return -1;
    }
    for (j = 0; j < num_fields; j++){
      data[line_length*j + i] = pixel_data[j];
    }
  }
  

  delete [] pixel_data;
  me->d_startX = start_x;
  me->d_startY = start_y;
  me->d_dX = dx;
  me->d_dY = dy;
  me->d_lineLength = line_length;
  me->d_numFields = num_fields;
  if (me->d_data) {
    // delete old data before setting new data
    delete [] (me->d_data);
  }
  me->d_data = data;
  return me->notifyMessageHandlers(WINDOW_LINE_DATA, _p.msg_time);
}

//static
int nmm_Microscope_SEM_Remote::RcvScanlineData(void *_userdata, 
    vrpn_HANDLERPARAM _p)
{
  vrpn_int32 start_x, start_y, dx, dy, line_length, num_fields, num_lines;
  vrpn_uint8 *data;
  nmm_Microscope_SEM_Remote *me = (nmm_Microscope_SEM_Remote *)_userdata;
  const char * bufptr = _p.buffer;
  vrpn_int32 sec, usec;

  if (decode_ScanlineDataHeader(&bufptr, &start_x, &start_y,
           &dx, &dy, &line_length, &num_fields, &num_lines, &sec, &usec) 
           == -1) {
    fprintf(stderr,
        "nmm_Microscope_SEM_Remote::RcvScanlineData: header decode failed\n");
    return -1;
  }

  int oldbufsize = me->d_lineLength*me->d_numFields*me->d_numLines;
  if (oldbufsize != line_length*num_fields*num_lines){
      if (me->d_uint8_data){
        delete [] (me->d_uint8_data);
      }
      me->d_uint8_data = new vrpn_uint8[num_fields*line_length*num_lines];
  }
  data = me->d_uint8_data;

  if (decode_ScanlineDataLine (&bufptr, line_length, num_fields,
                               num_lines, data) == -1) {
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


