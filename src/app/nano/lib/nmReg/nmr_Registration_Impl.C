#include "nmr_Registration_Impl.h"
#include "transformSolve.h"
#include "nmr_AlignerMI.h"

nmr_Registration_Impl::nmr_Registration_Impl(nmr_Registration_Server *server):
  d_alignerUI(NULL),
  d_server(server),
  d_transformType(NMR_2D2D_AFFINE)
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
        // 2 images, 3 correspondence points
        Correspondence c(2,3);
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
   Correspondence c(2,3);
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

    // default to what the user specified but if only 1 or 2 points are
    // given then default to the largest subset of affine 
    // transformations possibly defined by that number of points as long
    nmr_TransformationType transformType = d_transformType;
    if (c.numPoints() == 1 && 
        (transformType == NMR_2D2D_AFFINE || 
         transformType == NMR_TRANSLATION_ROTATION_SCALE)) {
        transformType = NMR_TRANSLATION;
    } else if (c.numPoints() == 2 && 
               (transformType == NMR_2D2D_AFFINE)) {
        transformType = NMR_TRANSLATION_ROTATION_SCALE;
    }

    int result = transformSolver(xform_matrix, &error, c, 
	corrSourceIndex, corrTargetIndex,
        transformType);
    if (result == -1)
	return -1;

    if (xform) {
        for (int i = 0; i < 16; i++){
            xform[i] = xform_matrix[i];
        }
    }
// debugging: try printing out the mutual information estimate to 
// see if it makes sense
    nmr_AlignerMI *mi = new nmr_AlignerMI();
    mi->setDimensionMode(REF_2D);
    if (mi->setSampleSizes(100, 100)) {
      printf("setSampleSizes error\n");
    } else {
      printf("setSampleSizes succeeded\n");
    }
    // compute a quick and dirty estimate of what the 
    // Parzen window variance should be
    double src_range = d_images[SOURCE_IMAGE_INDEX]->maxValue() -
                       d_images[SOURCE_IMAGE_INDEX]->minValue();
    double tgt_range = d_images[TARGET_IMAGE_INDEX]->maxValue() -
                       d_images[TARGET_IMAGE_INDEX]->minValue();
    double sigmaTest = 0.1*(tgt_range);
    double sigmaRef = 0.1*(src_range);
    mi->setTestVariance(sigmaTest);
    mi->setRefVariance(sigmaRef);
    mi->setCovariance(sigmaRef, sigmaTest);

    double T[6];
    T[0] = xform_matrix[0]; T[1] = xform_matrix[4]; T[2] = xform_matrix[12];
    T[3] = xform_matrix[1]; T[4] = xform_matrix[5]; T[5] = xform_matrix[13];

    // scale down from 0..src.width, 0..src.height to 0..1, 0..1
    double src_width = d_images[SOURCE_IMAGE_INDEX]->width();
    double src_height = d_images[SOURCE_IMAGE_INDEX]->height();
    T[0] /= src_width;
    T[1] /= src_height;
    T[3] /= src_width;
    T[4] /= src_height;

    // result is 0..1, 0..1 but we want 0..tgt.width, 0..tgt.height so scale
    // the result
    T[0] *= d_images[TARGET_IMAGE_INDEX]->width();
    T[1] *= d_images[TARGET_IMAGE_INDEX]->width();
    T[2] *= d_images[TARGET_IMAGE_INDEX]->width();
    T[3] *= d_images[TARGET_IMAGE_INDEX]->height();
    T[4] *= d_images[TARGET_IMAGE_INDEX]->height();
    T[5] *= d_images[TARGET_IMAGE_INDEX]->height();
    if (mi->setTransformation2D(T)) {
      printf("setTransformation2D error\n");
    } else {
      printf("setTransformation2D succeeded\n");
    }
    if (mi->buildSampleA(d_images[SOURCE_IMAGE_INDEX],
                         d_images[TARGET_IMAGE_INDEX]) ||
        mi->buildSampleB(d_images[SOURCE_IMAGE_INDEX],
                         d_images[TARGET_IMAGE_INDEX])) {
      printf("buildSample error\n");
    } else {
      printf("buildSample suceeded\n");
    }
    double mutual_info;
    if (mi->computeMutualInformation(mutual_info)){
      printf("computeMutualInformation error\n");
    } else {
      printf("computeMutualInformation succeeded; value = %g\n", mutual_info);
    }

    if (d_server){
      d_server->sendRegistrationResult(xform_matrix);
    }
    return 0;
}
