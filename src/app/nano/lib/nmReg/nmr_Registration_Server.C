#include "nmr_Registration_Server.h"

nmr_Registration_Server::nmr_Registration_Server(const char *name,
                                                vrpn_Connection *c):
    nmb_Device_Server(name, c),
    nmr_Registration_Interface(name, d_connection),
    d_imageParamsLastReceived(NMR_SOURCE),
    d_srcResX(0), d_srcResY(0), 
    d_srcSizeX(0), d_srcSizeY(0),
    d_srcFlipX(vrpn_FALSE), d_srcFlipY(vrpn_FALSE),
    d_targetResX(0), d_targetResY(0), 
    d_targetSizeX(0), d_targetSizeY(0),
    d_targetFlipX(vrpn_FALSE), d_targetFlipY(vrpn_FALSE),
    d_imageScanlineLastReceived(NMR_SOURCE),
    d_row(0), d_length(0), d_lengthAllocated(0), d_scanlineData(NULL),
    d_transformType(NMR_2D2D_AFFINE),
    d_GUIEnabled(vrpn_FALSE),
    d_numLevels(0),
    d_resolutionIndex(0),
    d_maxIterations(0),
    d_stepSize(1),
    d_autoAlignEnabled(vrpn_FALSE),   
    d_x_src(0), d_y_src(0), d_z_src(0), 
    d_x_tgt(0), d_y_tgt(0), d_z_tgt(0), d_messageHandlerList(NULL)
{

  if (d_connection == NULL) {
    fprintf(stderr, "nmr_Registration_Server: no connection\n");
    return;
  }

  if (d_connection->register_handler(d_SetImageParameters_type,
       RcvSetImageParameters, this)) {
    fprintf(stderr, "nmr_Registration_Server: can't register handler\n");
    return;
  }
  if (d_connection->register_handler(d_SetImageScanlineData_type,
       RcvSetScanline, this)) {
    fprintf(stderr, "nmr_Registration_Server: can't register handler\n");
    return;
  }
  if (d_connection->register_handler(d_SetTransformationOptions_type,
       RcvSetTransformationOptions, this)) {
    fprintf(stderr, "nmr_Registration_Server: can't register handler\n");
    return;
  }
  if (d_connection->register_handler(d_EnableGUI_type,
       RcvEnableGUI, this)) {
    fprintf(stderr, "nmr_Registration_Server: can't register handler\n");
    return;
  }
  if (d_connection->register_handler(d_Fiducial_type,
       RcvFiducial, this)) {
    fprintf(stderr, "nmr_Registration_Server: can't register handler\n");
    return;
  }
  if (d_connection->register_handler(d_SetResolutions_type,
       RcvSetResolutions, this)) {
    fprintf(stderr, "nmr_Registration_Server: can't register handler\n");
    return;
  }
  if (d_connection->register_handler(d_SetIterationLimit_type,
       RcvSetIterationLimit, this)) {
    fprintf(stderr, "nmr_Registration_Server: can't register handler\n");
    return;
  }
  if (d_connection->register_handler(d_SetStepSize_type,
       RcvSetStepSize, this)) {
    fprintf(stderr, "nmr_Registration_Server: can't register handler\n");
    return;
  }
  if (d_connection->register_handler(d_SetCurrentResolution_type,
       RcvSetCurrentResolution, this)) {
    fprintf(stderr, "nmr_Registration_Server: can't register handler\n");
    return;
  }
  if (d_connection->register_handler(d_SetAutoAlignEnable_type,
       RcvSetAutoAlignEnable, this)) {
    fprintf(stderr, "nmr_Registration_Server: can't register handler\n");
    return;
  }


}

nmr_Registration_Server::~nmr_Registration_Server()
{
  if (d_connection == NULL) {
    return;
  }
  if (d_connection->unregister_handler(d_SetImageParameters_type,
       RcvSetImageParameters, this)) {
    fprintf(stderr, "nmr_Registration_Server: can't unregister handler\n");
  }
  if (d_connection->unregister_handler(d_SetImageScanlineData_type,
       RcvSetScanline, this)) {
    fprintf(stderr, "nmr_Registration_Server: can't unregister handler\n");
  }
  if (d_connection->unregister_handler(d_SetTransformationOptions_type,
       RcvSetTransformationOptions, this)) {
    fprintf(stderr, "nmr_Registration_Server: can't unregister handler\n");
  }
  if (d_connection->unregister_handler(d_EnableGUI_type,
       RcvEnableGUI, this)) {
    fprintf(stderr, "nmr_Registration_Server: can't unregister handler\n");
  }
  if (d_connection->unregister_handler(d_Fiducial_type,
       RcvFiducial, this)) {
    fprintf(stderr, "nmr_Registration_Server: can't unregister handler\n");
  }
  if (d_connection->unregister_handler(d_SetResolutions_type,
       RcvSetResolutions, this)) {
    fprintf(stderr, "nmr_Registration_Server: can't unregister handler\n");
  }
  if (d_connection->unregister_handler(d_SetIterationLimit_type,
       RcvSetIterationLimit, this)) {
    fprintf(stderr, "nmr_Registration_Server: can't unregister handler\n");
  }
  if (d_connection->unregister_handler(d_SetStepSize_type,
       RcvSetStepSize, this)) {
    fprintf(stderr, "nmr_Registration_Server: can't unregister handler\n");
  }
  if (d_connection->unregister_handler(d_SetCurrentResolution_type,
       RcvSetCurrentResolution, this)) {
    fprintf(stderr, "nmr_Registration_Server: can't unregister handler\n");
  }
  if (d_connection->unregister_handler(d_SetAutoAlignEnable_type,
       RcvSetAutoAlignEnable, this)) {
    fprintf(stderr, "nmr_Registration_Server: can't unregister handler\n");
  }
}

int nmr_Registration_Server::setImageParameters(nmr_ImageType whichImage,
            vrpn_int32 res_x, vrpn_int32 res_y,
            vrpn_float32 size_x, vrpn_float32 size_y, 
            vrpn_bool flip_x, vrpn_bool flip_y)
{
    if (whichImage == NMR_SOURCE) {
        d_imageParamsLastReceived = NMR_SOURCE;
        d_srcResX = res_x;
        d_srcResY = res_y;
        d_srcSizeX = size_x;
        d_srcSizeY = size_y;
        d_srcFlipX = flip_x;
        d_srcFlipY = flip_y;
    } else if (whichImage == NMR_TARGET) {
        d_imageParamsLastReceived = NMR_TARGET;
        d_targetResX = res_x;
        d_targetResY = res_y;
        d_targetSizeX = size_x;
        d_targetSizeY = size_y;
        d_targetFlipX = flip_x;
        d_targetFlipY = flip_y;
    } else {
        fprintf(stderr, "RegistrationImpl::setImageParameters:"
                        " Error, unknown image type\n");
        return -1;
    }
    return 0;
}

int nmr_Registration_Server::setScanline(nmr_ImageType whichImage, 
                           vrpn_int32 row,
                           vrpn_int32 line_length, vrpn_float32 *data)
{
    int expected_length = 0;
    if (whichImage == NMR_SOURCE) {
        expected_length = d_srcResX;
    } else if (whichImage == NMR_TARGET) {
        expected_length = d_targetResX;
    }

    if (line_length != expected_length) {
        fprintf(stderr, "Server::setScanline: bad image dimensions\n");
        return -1;
    }

    d_row = row;
    d_length = line_length;
    memcpy(data, d_scanlineData, line_length*sizeof(vrpn_float32));
    return 0;
}

int nmr_Registration_Server::setTransformationOptions(
                           nmr_TransformationType type)
{
    d_transformType = type;
    return 0;
}

int nmr_Registration_Server::setGUIEnable(vrpn_bool enable)
{
    d_GUIEnabled = enable;
    return 0;
}

int nmr_Registration_Server::setFiducial(
                   vrpn_float32 x_src, vrpn_float32 y_src, vrpn_float32 z_src,
                   vrpn_float32 x_tgt, vrpn_float32 y_tgt, vrpn_float32 z_tgt)
{
    d_x_src = x_src; d_y_src = y_src; d_z_src = z_src;
    d_x_tgt = x_tgt; d_y_tgt = y_tgt; d_z_tgt = z_tgt;
    return 0;
}

int nmr_Registration_Server::setResolutions(vrpn_int32 numLevels, 
                                            vrpn_float32 *stddev)
{
  d_numLevels = numLevels;
  int i;
  for (i = 0; i < numLevels; i++) {
    d_stddev[i] = stddev[i];
  }
  return 0;
}

int nmr_Registration_Server::setIterationLimit(vrpn_int32 maxIterations)
{
  d_maxIterations = maxIterations;
  return 0;
}

int nmr_Registration_Server::setStepSize(vrpn_float32 stepSize)
{
  d_stepSize = stepSize;
  return 0;
}

int nmr_Registration_Server::setCurrentResolution(vrpn_int32 resolutionIndex)
{
  d_resolutionIndex = resolutionIndex;
  return 0;
}

int nmr_Registration_Server::setAutoAlignEnable(vrpn_bool enable)   
{
    d_autoAlignEnabled = enable;
    return 0;
}

/* Rcv functions:
    decode parameters
    call set function
    notify handlers

*/

//static 
int nmr_Registration_Server::RcvSetImageParameters (void *_userdata, 
                                          vrpn_HANDLERPARAM _p)
{
  nmr_Registration_Server *me = (nmr_Registration_Server *)_userdata;
  const char * bufptr = _p.buffer;
  vrpn_int32 which_image, res_x, res_y;
  vrpn_float32 size_x, size_y;
  vrpn_bool flip_x, flip_y;

  if (decode_SetImageParameters(&bufptr, &which_image, &res_x, &res_y,
               &size_x, &size_y, &flip_x, &flip_y) == -1) {
      fprintf(stderr,
         "nmr_Registration_Server::RcvSetImageParameters: decode failed\n");
      return -1;
  }
  if (which_image == NMR_SOURCE) {
     me->setImageParameters(NMR_SOURCE, res_x, res_y, 
                            size_x, size_y, flip_x, flip_y);
  } else if (which_image == NMR_TARGET) {
     me->setImageParameters(NMR_TARGET, res_x, res_y, 
                            size_x, size_y, flip_x, flip_y);
  } else {
    fprintf(stderr,
        "nmr_Registration_Server::RcvSetImageParameters: unknown image type\n");
    return -1;
  }

  return me->notifyMessageHandlers(NMR_IMAGE_PARAM, _p.msg_time);
}

//static 
int nmr_Registration_Server::RcvSetScanline (void *_userdata, 
                                          vrpn_HANDLERPARAM _p)
{
  nmr_Registration_Server *me = (nmr_Registration_Server *)_userdata;
  const char * bufptr = _p.buffer;
  vrpn_int32 which_image, row, length;
  int max_line_length;

  max_line_length = me->d_srcResX;
  if (me->d_targetResX > max_line_length) {
    max_line_length = me->d_targetResX;
  }


  if (me->d_lengthAllocated < max_line_length) {
     if (me->d_scanlineData) {
        delete [] me->d_scanlineData;
     }
     me->d_lengthAllocated = max_line_length;
     me->d_scanlineData = new vrpn_float32[max_line_length];
  }

  if (decode_SetImageScanlineData(&bufptr, &which_image, &row, &length,
      me->d_scanlineData, max_line_length) == -1) {
      fprintf(stderr,
         "nmr_Registration_Server::RcvSetScanline: decode failed\n");
      return -1;
  }
  if ((which_image == NMR_SOURCE && length != me->d_srcResX) ||
      (which_image == NMR_TARGET && length != me->d_targetResX)) {
      fprintf(stderr, 
         "nmr_Registration_Server::RcvSetScanline: unexpected line length\n");
      return -1;
  }

  // we aren't calling setScanline here to avoid a bunch of extra copying of 
  // data
  switch (which_image) {
    case NMR_SOURCE:
      me->d_imageScanlineLastReceived = NMR_SOURCE;
      break;
    case NMR_TARGET:
      me->d_imageScanlineLastReceived = NMR_TARGET;
      break;
    default:
      fprintf(stderr,
          "nmr_Registration_Server::RcvSetScanline: bad image type\n");
      return -1;
  }
  me->d_row = row;
  me->d_length = length;

  return me->notifyMessageHandlers(NMR_SCANLINE, _p.msg_time);
}

//static 
int nmr_Registration_Server::RcvSetTransformationOptions(void *_userdata, 
                                          vrpn_HANDLERPARAM _p)
{
  nmr_Registration_Server *me = (nmr_Registration_Server *)_userdata;
  const char * bufptr = _p.buffer;
  vrpn_int32 transformType;

  if (decode_SetTransformationOptions(&bufptr, &transformType) == -1) {
    fprintf(stderr,
        "nmr_Registration_Server::RcvSetTransformationOptions:"
        " decode failed\n");
    return -1;
  }
  switch (transformType) {
     case NMR_2D2D_AFFINE:
        me->setTransformationOptions(NMR_2D2D_AFFINE);
        break;
     case NMR_2D2D_PERSPECTIVE:
        me->setTransformationOptions(NMR_2D2D_PERSPECTIVE);
        break;
     case NMR_3D2D:
        me->setTransformationOptions(NMR_3D2D);
        break;
     default:
        fprintf(stderr,
           "nmr_Registration_Client::RcvTransformationOptions:"
            " unknown transform type\n");
        return -1;
  }

  return me->notifyMessageHandlers(NMR_TRANSFORM_OPTION, _p.msg_time);
}

//static
int nmr_Registration_Server::RcvEnableGUI (void *_userdata,
                                          vrpn_HANDLERPARAM _p)
{
  nmr_Registration_Server *me = (nmr_Registration_Server *)_userdata;
  const char * bufptr = _p.buffer;
  vrpn_int32 enable;

  if (decode_EnableGUI(&bufptr, &enable) == -1) {
    fprintf(stderr,
        "nmr_Registration_Server::RcvEnableRegistration:"
        " decode failed\n");
    return -1;
  }

  me->setGUIEnable(enable);

  return me->notifyMessageHandlers(NMR_ENABLE_GUI, _p.msg_time);
}

//static 
int nmr_Registration_Server::RcvFiducial (void *_userdata, 
                                          vrpn_HANDLERPARAM _p)
{
  nmr_Registration_Server *me = (nmr_Registration_Server *)_userdata;
  const char * bufptr = _p.buffer;
  vrpn_float32 x_src, y_src, z_src, x_tgt, y_tgt, z_tgt;

  if (decode_Fiducial(&bufptr, &x_src, &y_src, &z_src, 
                               &x_tgt, &y_tgt, &z_tgt) == -1) {
    fprintf(stderr,
        "nmr_Registration_Server::RcvFiducial:"
        " decode failed\n");
    return -1;
  }
  me->setFiducial(x_src, y_src, z_src, x_tgt, y_tgt, z_tgt);

  return me->notifyMessageHandlers(NMR_FIDUCIAL, _p.msg_time);
}

//static 
int nmr_Registration_Server::RcvSetResolutions(void *_userdata, 
                                               vrpn_HANDLERPARAM _p)
{
  nmr_Registration_Server *me = (nmr_Registration_Server *)_userdata;
  const char * bufptr = _p.buffer;
  vrpn_int32 numLevels;
  vrpn_float32 stddev[NMR_MAX_RESOLUTION_LEVELS];
  

  if (decode_SetResolutions(&bufptr, &numLevels, stddev) == -1) {
    fprintf(stderr,
        "nmr_Registration_Server::RcvSetResolutions:"
        " decode failed\n");
    return -1;
  }

  me->setResolutions(numLevels, stddev);

  return me->notifyMessageHandlers(NMR_SET_RESOLUTIONS, _p.msg_time);
}

//static 
int nmr_Registration_Server::RcvSetIterationLimit(void *_userdata, 
                                                  vrpn_HANDLERPARAM _p)
{
  nmr_Registration_Server *me = (nmr_Registration_Server *)_userdata;
  const char * bufptr = _p.buffer;
  vrpn_int32 maxIterations;

  if (decode_SetIterationLimit(&bufptr, &maxIterations) == -1) {
    fprintf(stderr,
        "nmr_Registration_Server::RcvSetIterationLimit:"
        " decode failed\n");
    return -1;
  }

  me->setIterationLimit(maxIterations);

  return me->notifyMessageHandlers(NMR_SET_ITERATION_LIMIT, _p.msg_time);
}

//static 
int nmr_Registration_Server::RcvSetStepSize(void *_userdata, 
                                            vrpn_HANDLERPARAM _p)
{
  nmr_Registration_Server *me = (nmr_Registration_Server *)_userdata;
  const char * bufptr = _p.buffer;
  vrpn_float32 stepSize;

  if (decode_SetStepSize(&bufptr, &stepSize) == -1) {
    fprintf(stderr,
        "nmr_Registration_Server::RcvSetStepSize:"
        " decode failed\n");
    return -1;
  }

  me->setStepSize(stepSize);

  return me->notifyMessageHandlers(NMR_SET_STEPSIZE, _p.msg_time);
}

//static 
int nmr_Registration_Server::RcvSetCurrentResolution(void *_userdata, 
                                                     vrpn_HANDLERPARAM _p)
{
  nmr_Registration_Server *me = (nmr_Registration_Server *)_userdata;
  const char * bufptr = _p.buffer;
  vrpn_int32 resolutionIndex;

  if (decode_SetCurrentResolution(&bufptr, &resolutionIndex) == -1) {
    fprintf(stderr,
        "nmr_Registration_Server::RcvSetCurrentResolution:"
        " decode failed\n");
    return -1;
  }

  me->setCurrentResolution(resolutionIndex);

  return me->notifyMessageHandlers(NMR_SET_CURRENT_RESOLUTION, _p.msg_time);
}


//static
int nmr_Registration_Server::RcvSetAutoAlignEnable (void *_userdata,
                                          vrpn_HANDLERPARAM _p)
{
  nmr_Registration_Server *me = (nmr_Registration_Server *)_userdata;
  const char * bufptr = _p.buffer;
  vrpn_int32 enable;

  if (decode_SetAutoAlignEnable(&bufptr, &enable) == -1) {
    fprintf(stderr,
        "nmr_Registration_Server::RcvSetAutoAlignEnable:"
        " decode failed\n");
    return -1;
  }

  me->setAutoAlignEnable(enable);

  return me->notifyMessageHandlers(NMR_ENABLE_AUTOALIGN, _p.msg_time);
}

int nmr_Registration_Server::registerChangeHandler (void *userdata,
        void (*handler)(void *ud, const nmr_ServerChangeHandlerData &info))
{
  msg_handler_list_t *new_entry;
  if (handler == NULL) {
    fprintf(stderr,
       "nmr_Registration_Server::registerChangeHandler: NULL handler\n");
    return -1;
  }
  if ( (new_entry = new msg_handler_list_t) == NULL) {
    fprintf(stderr,
       "nmr_Registration_Server::registerChangeHandler: out of memory\n");
    return -1;
  }
  new_entry->handler = handler;
  new_entry->userdata = userdata;
  new_entry->next = d_messageHandlerList;
  d_messageHandlerList = new_entry;

  return 0;
}

int nmr_Registration_Server::unregisterChangeHandler (void *userdata,
        void (*handler)(void *ud, const nmr_ServerChangeHandlerData &info))
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
           "nmr_Registration_Server::unregisterChangeHandler:"
           " no such handler\n");
    return -1;
  }

  // remove the entry from the list
  *snitch = victim->next;
  delete victim;

  return 0;
}

int nmr_Registration_Server::notifyMessageHandlers(nmr_MessageType type,
        const struct timeval &msg_time)
{
  nmr_ServerChangeHandlerData handler_info;
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

void nmr_Registration_Server::getImageParameters(nmr_ImageType &whichImage,
         vrpn_int32 &res_x, vrpn_int32 &res_y,
         vrpn_float32 &size_x, vrpn_float32 &size_y,
         vrpn_bool &flip_x, vrpn_bool &flip_y)
{
    whichImage = d_imageParamsLastReceived;
    if (whichImage == NMR_SOURCE){
        res_x = d_srcResX;
        res_y = d_srcResY;
        size_x = d_srcSizeX;
        size_y = d_srcSizeY;
        flip_x = d_srcFlipX;
        flip_y = d_srcFlipY;
    } else {
        res_x = d_targetResX;
        res_y = d_targetResY;
        size_x = d_targetSizeX;
        size_y = d_targetSizeY;
        flip_x = d_targetFlipX;
        flip_y = d_targetFlipY;
    }
}

void nmr_Registration_Server::getAutoAlignEnable(vrpn_bool &enabled)
{
    enabled = d_autoAlignEnabled;
}

void nmr_Registration_Server::getGUIEnable(vrpn_bool &enabled)
{
    enabled = d_GUIEnabled;
}

void nmr_Registration_Server::getTransformationOptions(
      nmr_TransformationType &type)
{
    type = d_transformType;
}

void nmr_Registration_Server::getScanline(nmr_ImageType &whichImage,
                           vrpn_int32 &row, vrpn_int32 &length,
                           vrpn_float32 **data)
{
    whichImage = d_imageScanlineLastReceived;
    row = d_row;
    length = d_length;
    *data = d_scanlineData;
}

void nmr_Registration_Server::getFiducial(
              vrpn_float32 &x_src, vrpn_float32 &y_src, vrpn_float32 &z_src,
              vrpn_float32 &x_tgt, vrpn_float32 &y_tgt, vrpn_float32 &z_tgt)
{
    x_src = d_x_src; y_src = d_y_src; z_src = d_z_src;
    x_tgt = d_x_tgt; y_tgt = d_y_tgt; z_tgt = d_z_tgt;
}

void nmr_Registration_Server::getResolutions(vrpn_int32 &numLevels, 
   vrpn_float32 *stddev)
{
  numLevels = d_numLevels;
  int i;
  for (i = 0; i < numLevels; i++) {
    stddev[i] = d_stddev[i];
  }
}

void nmr_Registration_Server::getIterationLimit(vrpn_int32 &maxIterations)
{
  maxIterations = d_maxIterations;
}

void nmr_Registration_Server::getStepSize(vrpn_float32 &stepSize)
{
  stepSize = d_stepSize;
}

void nmr_Registration_Server::getCurrentResolution(vrpn_int32 &resolutionIndex)
{
  resolutionIndex = d_resolutionIndex;
}

int nmr_Registration_Server::sendRegistrationResult(double xform[16])
{
  char *msgbuf;
  vrpn_int32 len;

  msgbuf = encode_RegistrationResult(&len, xform);

  if (!msgbuf) {
    return -1;
  }

  return dispatchMessage(len, msgbuf, d_RegistrationResult_type);
}
