#include "nmr_Registration_Impl.h"
#include "transformSolve.h"

nmr_Registration_Impl::nmr_Registration_Impl(nmr_Registration_Server *server):
  d_server(server)
{
    // set up things so we do stuff when server gets messages
  if (d_server) {
      d_server->registerChangeHandler((void *)this, serverMessageHandler);
  }
  d_images[SOURCE_IMAGE_INDEX] = new nmb_ImageGrid("sourceImage",
                                       "unknown_units",
                                       100, 100);
  d_images[TARGET_IMAGE_INDEX] = new nmb_ImageGrid("targetImage",
                                       "unknown_units",
                                       100, 100);

  d_alignerUI = new nmr_Registration_ImplUI(this);

}

nmr_Registration_Impl::~nmr_Registration_Impl()
{
  delete d_images[SOURCE_IMAGE_INDEX];
  delete d_images[TARGET_IMAGE_INDEX];
  delete d_alignerUI;
  if (d_server) {
    // this causes seg fault on cygwin
    //d_server->unregisterChangeHandler((void *)this, serverMessageHandler);
  }
}

void nmr_Registration_Impl::mainloop()
{
    d_alignerUI->mainloop();
}

// static
void nmr_Registration_Impl::serverMessageHandler(void *ud, 
                           const nmr_ServerChangeHandlerData &info)
{
  nmr_Registration_Impl *me = (nmr_Registration_Impl *)ud;

  nmr_ImageType whichImage;
  nmr_TransformationType xformtype;
  vrpn_int32 res_x, res_y;
  vrpn_bool heightfield;
  vrpn_int32 row, length;
  vrpn_float32 *data;
  vrpn_float32 x, y, z;
  vrpn_bool enabled;

  switch(info.msg_type) {
    case NMR_IMAGE_PARAM:
//      printf("nmr_Registration_Impl: image params received\n");
      info.aligner->getImageParameters(whichImage, res_x, res_y, heightfield);
      me->setImageParameters(whichImage, res_x, res_y, heightfield);
      break;
    case NMR_TRANSFORM_OPTION:
      info.aligner->getTransformationOptions(xformtype);
      me->setTransformationOptions(xformtype);
      break;
    case NMR_SCANLINE:
//      printf("nmr_Registration_Impl: scanline received\n");
      info.aligner->getScanline(whichImage, row, length, &data);
      me->setScanline(whichImage, row, length, data);
      break;
    case NMR_FIDUCIAL:
      info.aligner->getFiducial(whichImage, x, y, z);
      me->setFiducial(whichImage, x, y, z);
      break;
    case NMR_ENABLE_REGISTRATION:
      info.aligner->getRegistrationEnable(enabled);
      me->setRegistrationEnable(enabled);
      break;
    case NMR_ENABLE_GUI:
      info.aligner->getGUIEnable(enabled);
      me->setGUIEnable(enabled);
    default:
      return;
  }
}

nmb_Image *nmr_Registration_Impl::getImage(nmr_ImageType type)
{
    switch (type) {
      case NMR_SOURCE:
        return d_images[SOURCE_IMAGE_INDEX];
      case NMR_TARGET:
        return d_images[TARGET_IMAGE_INDEX];
      default:
        return NULL;
    }
}

int nmr_Registration_Impl::setRegistrationEnable(vrpn_bool enable)
{
    if (enable) {
        Correspondence c;
        int corrSourceIndex, corrTargetIndex;
        d_alignerUI->getCorrespondence(c, corrSourceIndex, corrTargetIndex);
        registerImages(c, corrSourceIndex, corrTargetIndex);
    }
    return 0;
}

int nmr_Registration_Impl::setGUIEnable(vrpn_bool enable)
{
    d_alignerUI->enable(enable);
    return 0;
}

int nmr_Registration_Impl::setImageParameters(nmr_ImageType whichImage,
                           vrpn_int32 res_x, vrpn_int32 res_y,
                           vrpn_bool treat_as_height_field)
{
    if (whichImage == NMR_SOURCE) {
        if ((d_images[SOURCE_IMAGE_INDEX]->width() != res_x) ||
            (d_images[SOURCE_IMAGE_INDEX]->height() != res_y)) {
//              printf("changing source resolution: (%d, %d) --> (%d, %d)\n",
//                      d_images[SOURCE_IMAGE_INDEX]->width(),
//                      d_images[SOURCE_IMAGE_INDEX]->height(),
//                      res_x, res_y);
                 delete d_images[SOURCE_IMAGE_INDEX];
                 d_images[SOURCE_IMAGE_INDEX] =
                      new nmb_ImageGrid("sourceImage",
                                        "unknown_units",
                                        res_x, res_y);
        }
        d_images[SOURCE_IMAGE_INDEX]->setHeightField(treat_as_height_field);
    } else if (whichImage == NMR_TARGET) {
        if ((d_images[TARGET_IMAGE_INDEX]->width() != res_x) ||
            (d_images[TARGET_IMAGE_INDEX]->height() != res_y)) {
//              printf("changing target resolution: (%d, %d) --> (%d, %d)\n",
//                      d_images[TARGET_IMAGE_INDEX]->width(),
//                      d_images[TARGET_IMAGE_INDEX]->height(),
//                      res_x, res_y);
                 delete d_images[TARGET_IMAGE_INDEX];
                 d_images[TARGET_IMAGE_INDEX] = new nmb_ImageGrid("targetImage",
                                        "unknown_units",
                                        res_x, res_y);
        }
        d_images[TARGET_IMAGE_INDEX]->setHeightField(treat_as_height_field);
    } else {
        fprintf(stderr, "RegistrationImpl::setImageParameters:"
                        " Error, unknown image type\n");
        return -1;
    }
    return 0;
}

int nmr_Registration_Impl::setFiducial(nmr_ImageType whichImage,
		vrpn_float32 x, vrpn_float32 y, vrpn_float32 z)
{
    d_alignerUI->setFiducial(whichImage, x, y, z);
    return 0;
}

int nmr_Registration_Impl::setScanline(nmr_ImageType whichImage,
         vrpn_int32 row, vrpn_int32 length, vrpn_float32 *data)
{
  nmb_Image *im = getImage(whichImage);
  if (im->width() < length || im->height() <= row) {
       fprintf(stderr, 
            "nmr_RegImpl::serverHandler: pixel index out of range\n");
       return -1;
  }
  int i;
  for (i = 0; i < length; i++) {
       im->setValue(i, row, data[i]);
  }
  d_alignerUI->newScanline(whichImage, row, im);
  return 0;
}

int nmr_Registration_Impl::setTransformationOptions(
                           nmr_TransformationType type)
{
    d_transformType = type;
    return 0;
}

int nmr_Registration_Impl::registerImages(double *xform)
{
   Correspondence c;
   int corrSourceIndex, corrTargetIndex;
   d_alignerUI->getCorrespondence(c, corrSourceIndex, corrTargetIndex);
   c.print();
   registerImages(c, corrSourceIndex, corrTargetIndex, xform);
   printf("nmr_Registration_Impl: registration result:\n");
   int i;
   for (i = 0; i < 16; i++) {
       printf("%g, ", xform[i]);
   }
   printf("\n");
   return 0;
}

int nmr_Registration_Impl::registerImages(Correspondence &c,
     int corrSourceIndex, int corrTargetIndex, double *xform)
{
    double xform_matrix[16];
    double error;

    transformSolver(xform_matrix, &error, c, corrSourceIndex, corrTargetIndex,
           getImage(NMR_SOURCE)->isHeightField());
    if (xform) {
        for (int i = 0; i < 16; i++){
            xform[i] = xform_matrix[i];
        }
    }
    if (d_server){
      d_server->sendRegistrationResult(xform_matrix);
    }
    return 0;
}
