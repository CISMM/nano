#include "nmr_Registration_Client.h"

nmr_Registration_Client::nmr_Registration_Client (const char *name, 
                                                 vrpn_Connection * c):
    nmb_Device_Client(name, c?c:vrpn_get_connection_by_name(name)),
    nmr_Registration_Interface(name, d_connection),
    d_imageParamsLastReceived(NMR_SOURCE), // this is arbitrary
    d_srcResX(0),
    d_srcResY(0),
    d_srcSizeX(0), d_srcSizeY(0),
    d_targetResX(0),
    d_targetResY(0),
    d_targetSizeX(0), d_targetSizeY(0),
    d_transformType(NMR_2D2D_AFFINE),
    d_messageHandlerList(NULL)
{
  int i;
  for (i = 0; i < 16; i++){
    d_matrix44[i] = 0.0;
  }

  if (d_connection == NULL) {
    fprintf(stderr, "nmr_Registration_Client: no connection\n");
    return;
  }
  if (d_connection->register_handler(d_ImageParameters_type, 
       RcvImageParameters, this)) {
    fprintf(stderr, "nmr_Registration_Client: can't register handler\n");
    return;
  }
  if (d_connection->register_handler(d_TransformationOptions_type, 
       RcvTransformationOptions, this)) {
    fprintf(stderr, "nmr_Registration_Client: can't register handler\n");
    return; 
  }
  if (d_connection->register_handler(d_RegistrationResult_type, 
       RcvRegistrationResult, this)) {
    fprintf(stderr, "nmr_Registration_Client: can't register handler\n");
    return; 
  }

}

nmr_Registration_Client::~nmr_Registration_Client()
{
  if (d_connection) {
    d_connection->unregister_handler(d_ImageParameters_type,
       RcvImageParameters, this);
    d_connection->unregister_handler(d_TransformationOptions_type,
       RcvTransformationOptions, this);
    d_connection->unregister_handler(d_RegistrationResult_type,
       RcvRegistrationResult, this);
  }
}

vrpn_int32 nmr_Registration_Client::mainloop(void)
{
  if (d_connection) {
     return d_connection->mainloop();
  }
  return 0;
}

int nmr_Registration_Client::setImageParameters(nmr_ImageType whichImage,
              vrpn_int32 res_x, vrpn_int32 res_y,
              vrpn_float32 sizeX, vrpn_float32 sizeY)
{
  char *msgbuf;
  vrpn_int32 len;

  msgbuf = encode_SetImageParameters(&len, (vrpn_int32)whichImage, res_x, res_y,
        sizeX, sizeY);
  if (!msgbuf) {
    return -1;
  }

  return dispatchMessage(len, msgbuf, d_SetImageParameters_type);
}

int nmr_Registration_Client::setScanline(nmr_ImageType whichImage, 
                           vrpn_int32 row,
                           vrpn_int32 line_length, vrpn_float32 *data)
{
  char *msgbuf;
  vrpn_int32 len;

  msgbuf = encode_SetImageScanlineData(&len, 
                               (vrpn_int32)whichImage, row, line_length,
                               data);
  if (!msgbuf) {
    return -1;
  }

  return dispatchMessage(len, msgbuf, d_SetImageScanlineData_type);
}

int nmr_Registration_Client::setTransformationOptions(
                                     nmr_TransformationType type)
{
  char *msgbuf;
  vrpn_int32 len;

  msgbuf = encode_SetTransformationOptions(&len, (vrpn_int32)type);

  if (!msgbuf) {
    return -1;
  }

  return dispatchMessage(len, msgbuf, d_SetTransformationOptions_type);
}

int nmr_Registration_Client::sendFiducial(nmr_ImageType whichImage,
                           vrpn_float32 x, vrpn_float32 y, vrpn_float32 z)
{
  char *msgbuf;
  vrpn_int32 len;

  msgbuf = encode_Fiducial(&len, (vrpn_int32)whichImage, x, y, z);

  if (!msgbuf) {
    return -1;
  }

  return dispatchMessage(len, msgbuf, d_Fiducial_type);
}

int nmr_Registration_Client::setRegistrationEnable(vrpn_bool enable)
{
  char *msgbuf;
  vrpn_int32 len;

  msgbuf = encode_EnableRegistration(&len, (vrpn_int32)enable);

  if (!msgbuf) {
    return -1;
  }

  return dispatchMessage(len, msgbuf, d_EnableRegistration_type);
}

int nmr_Registration_Client::setGUIEnable(vrpn_bool enable)
{
  char *msgbuf;
  vrpn_int32 len;

  msgbuf = encode_EnableGUI(&len, (vrpn_int32)enable);

  if (!msgbuf) {
    return -1;
  }

  return dispatchMessage(len, msgbuf, d_EnableGUI_type);
}

//static 
int nmr_Registration_Client::RcvImageParameters (
                void *_userdata, vrpn_HANDLERPARAM _p)
{
  nmr_Registration_Client *me = (nmr_Registration_Client *)_userdata;
  const char * bufptr = _p.buffer;
  vrpn_int32 which_image, res_x, res_y;
  vrpn_float32 sizeX, sizeY;

  if (decode_ImageParameters(&bufptr, &which_image, &res_x, &res_y,
            &sizeX, &sizeY) == -1) {
    fprintf(stderr,
        "nmr_Registration_Client::RcvImageParameters: decode failed\n");
    return -1;
  }
  if (which_image == NMR_SOURCE) {
     me->d_imageParamsLastReceived = NMR_SOURCE;
     me->d_srcResX = res_x;
     me->d_srcResY = res_y;
     me->d_srcSizeX = sizeX;
     me->d_srcSizeY = sizeY;
  } else if (which_image == NMR_TARGET) {
     me->d_imageParamsLastReceived = NMR_TARGET;
     me->d_targetResX = res_x;
     me->d_targetResY = res_y;
     me->d_targetSizeX = sizeX;
     me->d_targetSizeY = sizeY;
  } else {
    fprintf(stderr,
        "nmr_Registration_Client::RcvImageParameters: unknown image type\n");
    return -1;
  }
  return me->notifyMessageHandlers(NMR_IMAGE_PARAM, _p.msg_time);
}

//static 
int nmr_Registration_Client::RcvTransformationOptions (
                void *_userdata, vrpn_HANDLERPARAM _p)
{
  nmr_Registration_Client *me = (nmr_Registration_Client *)_userdata;
  const char * bufptr = _p.buffer;
  vrpn_int32 transformType;

  if (decode_TransformationOptions(&bufptr, &transformType) == -1) {
    fprintf(stderr,
        "nmr_Registration_Client::RcvTransformationOptions: decode failed\n");
    return -1;
  }
  if (transformType == NMR_2D2D_AFFINE) {
      me->d_transformType = NMR_2D2D_AFFINE;
  } else if (transformType == NMR_2D2D_PERSPECTIVE) {
      me->d_transformType = NMR_2D2D_PERSPECTIVE;
  } else if (transformType == NMR_3D2D) {
      me->d_transformType = NMR_3D2D;
  } else {
    fprintf(stderr,
        "nmr_Registration_Client::RcvTransformationOptions:"
        " unknown transform type\n");
    return -1;
  }

  return me->notifyMessageHandlers(NMR_TRANSFORM_OPTION, _p.msg_time);
}

//static 
int nmr_Registration_Client::RcvRegistrationResult (
                void *_userdata, vrpn_HANDLERPARAM _p) 
{
  nmr_Registration_Client *me = (nmr_Registration_Client *)_userdata;
  const char * bufptr = _p.buffer;
  vrpn_float64 matrix44[16];

  if (decode_RegistrationResult(&bufptr, matrix44) == -1) {
    fprintf(stderr,
        "nmr_Registration_Client::RcvRegistrationResult: decode failed\n");
    return -1;
  }
  int i;
  for (i = 0; i < 16; i++) {
      me->d_matrix44[i] = matrix44[i];
  }
  return me->notifyMessageHandlers(NMR_REG_RESULT, _p.msg_time);
}

int nmr_Registration_Client::registerChangeHandler (void *userdata,
        void (*handler)(void *ud, const nmr_ClientChangeHandlerData &info))
{
  msg_handler_list_t *new_entry;
  if (handler == NULL) {
    fprintf(stderr,
       "nmr_Registration_Client::registerChangeHandler: NULL handler\n");
    return -1;
  }
  if ( (new_entry = new msg_handler_list_t) == NULL) {
    fprintf(stderr,
       "nmr_Registration_Client::registerChangeHandler: out of memory\n");
    return -1;
  }
  new_entry->handler = handler;
  new_entry->userdata = userdata;
  new_entry->next = d_messageHandlerList;
  d_messageHandlerList = new_entry;

  return 0;
}

int nmr_Registration_Client::unregisterChangeHandler (void *userdata,
        void (*handler)(void *ud, const nmr_ClientChangeHandlerData &info))
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
           "nmr_Registration_Client::unregisterChangeHandler:"
           " no such handler\n");
    return -1;
  }

  // remove the entry from the list
  *snitch = victim->next;
  delete victim;

  return 0;
}

int nmr_Registration_Client::notifyMessageHandlers(nmr_MessageType type,
        const struct timeval &msg_time)
{
  nmr_ClientChangeHandlerData handler_info;
  handler_info.msg_time = msg_time;
  handler_info.msg_type = type;
  handler_info.aligner = this;

  msg_handler_list_t *handler = d_messageHandlerList;
  while (handler != NULL) {
    handler->handler(handler->userdata, handler_info);
    handler = handler->next;
  }
  return 0;
}

void nmr_Registration_Client::getImageParameters(nmr_ImageType &whichImage,
                           vrpn_int32 &res_x, vrpn_int32 &res_y,
                           vrpn_float32 &size_x, 
                           vrpn_float32 &size_y)
{ 
    whichImage = d_imageParamsLastReceived;
    if (whichImage == NMR_SOURCE){
        res_x = d_srcResX;
        res_y = d_srcResY;
        size_x = d_srcSizeX;
        size_y = d_srcSizeY;
    } else {
        res_x = d_targetResX;
        res_y = d_targetResY;
        size_x = d_targetSizeX;
        size_y = d_targetSizeY;
    }
}

void nmr_Registration_Client::getTransformationOptions(
      nmr_TransformationType &type)
{
    type = d_transformType;
}

void nmr_Registration_Client::getRegistrationResult(vrpn_float64 *matrix44)
{
    int i;
    for (i = 0; i < 16; i++){
        matrix44[i] = d_matrix44[i];
    }
}
