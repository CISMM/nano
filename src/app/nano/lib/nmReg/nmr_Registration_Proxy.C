#include "nmr_Registration_Proxy.h"

nmr_Registration_Proxy::nmr_Registration_Proxy(const char *name,
                                               vrpn_Connection *c):
  d_server(NULL),d_local_impl(NULL),d_remote_impl(NULL),d_local(vrpn_TRUE),

  d_imageParamsLastReceived(NMR_SOURCE),
  d_res_x(0), d_res_y(0),
  d_size_x(0.0), d_size_y(0.0),
  d_messageHandlerList(NULL)
{
    if (!name && !c) {
      fprintf(stderr, "nmr_Registration_Proxy::nmr_Registration_Proxy:"
           " programmer error: needs either a name or connection\n");
      return;
    }
    
    if (!c) {
        d_remote_impl = new nmr_Registration_Client(name);
        d_remote_impl->registerChangeHandler((void *) this, 
 		handle_registration_change);
        d_local = vrpn_FALSE;
    } else {
        d_server = new nmr_Registration_Server("reg_server", c);
        d_local_impl = new nmr_Registration_Impl(d_server);
        d_remote_impl = new nmr_Registration_Client("reg_server", c);
        d_remote_impl->registerChangeHandler((void *) this,
                handle_registration_change);
        d_local = vrpn_TRUE;
    }
}

nmr_Registration_Proxy::~nmr_Registration_Proxy()
{
    if (d_local_impl){
        delete d_local_impl;
        delete d_server;
        delete d_remote_impl;
    }
    if (d_remote_impl){
        delete d_remote_impl;
    }
}

vrpn_int32 nmr_Registration_Proxy::mainloop(void)
{
    if (d_local){
        // call mainloops multiple times to make sure everything gets 
        // propagated
        d_local_impl->mainloop();
        d_remote_impl->mainloop();
        d_local_impl->mainloop();
    } else {
        d_remote_impl->mainloop();
    }
    return 0;
}

vrpn_int32 nmr_Registration_Proxy::registerImages()
{
    d_remote_impl->setRegistrationEnable(vrpn_TRUE);
/*
    if (d_local){
        struct timeval now;
        d_local_impl->registerImagesFromPointCorrespondence(d_matrix44);
        gettimeofday(&now, NULL);
        notifyMessageHandlers(NMR_REG_RESULT, now);
    } else {
        d_remote_impl->setRegistrationEnable(vrpn_TRUE);
    }
*/
    return 0;
}

vrpn_int32 nmr_Registration_Proxy::setGUIEnable(vrpn_bool enable)
{
    d_remote_impl->setGUIEnable(enable);
/*
    if (d_local){
        d_local_impl->setGUIEnable(enable);
    } else {
        d_remote_impl->setGUIEnable(enable);
    }
*/
    return 0;
}

vrpn_int32 nmr_Registration_Proxy::setImage(nmr_ImageType whichImage,
          nmb_Image *im)
{
  int i,j;
  vrpn_float32 *data;
  double xSize = 0, ySize = 0;
/*
  if (d_local){
    im->getAcquisitionDimensions(xSize, ySize);
    d_local_impl->setImageParameters(whichImage, im->width(), im->height(),
                 xSize, ySize, 1.0);
    data = new vrpn_float32[im->width()];
    for (i = 0; i < im->height(); i++) {
        for (j = 0; j < im->width(); j++) {
            data[j] = im->getValue(j, i);
        }
        d_local_impl->setScanline(whichImage, i, im->width(), data);
    }
    delete [] data;
  } else {
*/
    printf("nmr_Registration_Proxy::setImage: "
           "sending image parameters to remote: (%dx%d)\n",
           im->width(), im->height());
    im->getAcquisitionDimensions(xSize, ySize);
    d_remote_impl->setImageParameters(whichImage, im->width(), im->height(),
                         xSize, ySize);
    d_remote_impl->mainloop();
    printf("nmr_Registration_Proxy::setImage: "
           "sending image data to remote\n");
    data = new vrpn_float32[im->width()];
    for (i = 0; i < im->height(); i++) {
        for (j = 0; j < im->width(); j++) {
            data[j] = im->getValue(j, i);
        }
        d_remote_impl->setScanline(whichImage, i, im->width(), data);
        d_remote_impl->mainloop();
    }
    printf("nmr_Registration_Proxy::setImage: "
           "finished sending image data to remote\n");
    delete [] data;
//  }
  return 0;
}

// static
void nmr_Registration_Proxy::handle_registration_change(void *ud,
                          const nmr_ClientChangeHandlerData &info)
{
  nmr_Registration_Proxy *me = (nmr_Registration_Proxy *)ud;

  // set our data
  switch(info.msg_type) {
    case NMR_IMAGE_PARAM:
      info.aligner->getImageParameters(me->d_imageParamsLastReceived, 
               me->d_res_x, me->d_res_y, 
               me->d_size_x, me->d_size_y);
      break;
    case NMR_TRANSFORM_OPTION:
      //printf("nmr_Registration_Proxy::got transform option\n");
      info.aligner->getTransformationOptions(me->d_transformType);
      break;
    case NMR_REG_RESULT:
      //printf("nmr_Registration_Proxy::got transform\n");
      info.aligner->getRegistrationResult(me->d_matrix44);
      break;
  }

  // notify our user callbacks
  me->notifyMessageHandlers(info.msg_type, info.msg_time);
}

int nmr_Registration_Proxy::registerChangeHandler (void *userdata,
        void (*handler)(void *ud, const nmr_ProxyChangeHandlerData &info))
{
  msg_handler_list_t *new_entry;
  if (handler == NULL) {
    fprintf(stderr,
       "nmr_Registration_Proxy::registerChangeHandler: NULL handler\n");
    return -1;
  }
  if ( (new_entry = new msg_handler_list_t) == NULL) {
    fprintf(stderr,
       "nmr_Registration_Proxy::registerChangeHandler: out of memory\n");
    return -1;
  }
  new_entry->handler = handler;
  new_entry->userdata = userdata;
  new_entry->next = d_messageHandlerList;
  d_messageHandlerList = new_entry;

  return 0;
}

int nmr_Registration_Proxy::unregisterChangeHandler (void *userdata,
        void (*handler)(void *ud, const nmr_ProxyChangeHandlerData &info))
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
           "nmr_Registration_Proxy::unregisterChangeHandler:"
           " no such handler\n");
    return -1;
  }

  // remove the entry from the list
  *snitch = victim->next;
  delete victim;

  return 0;
}

int nmr_Registration_Proxy::notifyMessageHandlers(nmr_MessageType type,
        const struct timeval &msg_time)
{
  nmr_ProxyChangeHandlerData handler_info;
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

void nmr_Registration_Proxy::getImageParameters(nmr_ImageType &whichImage,
       vrpn_int32 &res_x, vrpn_int32 &res_y,
       vrpn_float32 &size_x, vrpn_float32 &size_y)
{
    whichImage = d_imageParamsLastReceived;
    res_x = d_res_x; res_y = d_res_y;
    size_x = d_size_x; size_y = d_size_y;
}

void nmr_Registration_Proxy::getTransformationOptions(
                              nmr_TransformationType &type)
{
    type = d_transformType;
}

void nmr_Registration_Proxy::getRegistrationResult(vrpn_float64 *matrix44)
{
    for (int i = 0; i < 16; i++){
        matrix44[i] = d_matrix44[i];
    }
}

void nmr_Registration_Proxy::getRegistrationResult(
                                      nmb_TransformMatrix44 &xform)
{
  xform.setMatrix(d_matrix44);
}
