/*===3rdtech===
  Copyright (c) 2001 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#include "nmr_Registration_Impl.h"
#include "transformSolve.h"
#include "nmr_AlignerMI.h"
#include "nmr_CoarseToFineSearch.h"
#include "nmb_Transform_TScShR.h"
#include "nmb_TransformMatrix44.h"

// M_PI not defined for VC++, for some reason.
#ifndef M_PI
#define M_PI        3.14159265358979323846
#endif

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
  nmb_Image::deleteImage(d_images[SOURCE_IMAGE_INDEX]);
  nmb_Image::deleteImage(d_images[TARGET_IMAGE_INDEX]);
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
  vrpn_float32 size_x, size_y;
  vrpn_int32 row, length;
  vrpn_float32 *data;
  vrpn_float32 x, y, z;
  vrpn_bool enabled;

  switch(info.msg_type) {
    case NMR_IMAGE_PARAM:
//      printf("nmr_Registration_Impl: image params received\n");
      info.aligner->getImageParameters(whichImage, res_x, res_y, 
                    size_x, size_y);
      me->setImageParameters(whichImage, res_x, res_y, 
                             size_x, size_y);
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
        static Correspondence last_c;
        Correspondence c(2, 3);
        int corrSourceIndex, corrTargetIndex;
        d_alignerUI->getCorrespondence(c, corrSourceIndex, corrTargetIndex);
        double srcSizeX, srcSizeY, tgtSizeX, tgtSizeY;
        d_images[SOURCE_IMAGE_INDEX]->
                       getAcquisitionDimensions(srcSizeX, srcSizeY);
        d_images[TARGET_IMAGE_INDEX]->
                       getAcquisitionDimensions(tgtSizeX, tgtSizeY);
        c.scalePoints(corrSourceIndex, srcSizeX, srcSizeY, 1.0);
        c.scalePoints(corrTargetIndex, tgtSizeX, tgtSizeY, 1.0);
        static double xform_matrix[16];
        static nmb_Transform_TScShR xform;
        double center[4] = {0.0, 0.0, 0.0, 1.0};
        center[0] = 0.5*srcSizeX;
        center[1] = 0.5*srcSizeY;
        center[2] = d_images[SOURCE_IMAGE_INDEX]->getValueInterpolated(
                           0.5*d_images[SOURCE_IMAGE_INDEX]->width(),
                           0.5*d_images[SOURCE_IMAGE_INDEX]->height());
        xform.setCenter(center[0], center[1], center[2]);

        adjustTransformFromRotatedCorrespondence(c, corrSourceIndex,
                                    corrTargetIndex, xform);
        xform.getMatrix(xform_matrix);
        sendResult(xform_matrix);
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
                           vrpn_float32 xSpan, 
                           vrpn_float32 ySpan)
{
    if (whichImage == NMR_SOURCE) {
        if ((d_images[SOURCE_IMAGE_INDEX]->width() != res_x) ||
            (d_images[SOURCE_IMAGE_INDEX]->height() != res_y)) {
//              printf("changing source resolution: (%d, %d) --> (%d, %d)\n",
//                      d_images[SOURCE_IMAGE_INDEX]->width(),
//                      d_images[SOURCE_IMAGE_INDEX]->height(),
//                      res_x, res_y);
                 nmb_Image::deleteImage(d_images[SOURCE_IMAGE_INDEX]);
                 d_images[SOURCE_IMAGE_INDEX] =
                      new nmb_ImageGrid("sourceImage",
                                        "unknown_units",
                                        res_x, res_y);
        }
        d_images[SOURCE_IMAGE_INDEX]->
                  setAcquisitionDimensions(xSpan, ySpan);
    } else if (whichImage == NMR_TARGET) {
        if ((d_images[TARGET_IMAGE_INDEX]->width() != res_x) ||
            (d_images[TARGET_IMAGE_INDEX]->height() != res_y)) {
//              printf("changing target resolution: (%d, %d) --> (%d, %d)\n",
//                      d_images[TARGET_IMAGE_INDEX]->width(),
//                      d_images[TARGET_IMAGE_INDEX]->height(),
//                      res_x, res_y);
                 nmb_Image::deleteImage(d_images[TARGET_IMAGE_INDEX]);
                 d_images[TARGET_IMAGE_INDEX] = new nmb_ImageGrid("targetImage",
                                        "unknown_units",
                                        res_x, res_y);
        }
        d_images[TARGET_IMAGE_INDEX]->
                  setAcquisitionDimensions(xSpan, ySpan);
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

int nmr_Registration_Impl::setColorMap(nmr_ImageType whichImage,
                                            nmb_ColorMap * cmap)
{
    d_alignerUI->setColorMap(whichImage, cmap);
    return 0;
}

int nmr_Registration_Impl::setColorMinMax(nmr_ImageType whichImage, 
                              vrpn_float64 dmin, vrpn_float64 dmax,
                              vrpn_float64 cmin, vrpn_float64 cmax)
{
    d_alignerUI->setColorMinMax(whichImage, dmin, dmax, cmin, cmax);
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
  d_imageChangeSinceLastRegistration = vrpn_TRUE;;
  return 0;
}

int nmr_Registration_Impl::setTransformationOptions(
                           nmr_TransformationType type)
{
    d_transformType = type;
    return 0;
}

int nmr_Registration_Impl::registerImagesFromPointCorrespondence(double *xform)
{
   Correspondence c(2, 3);
   int corrSourceIndex, corrTargetIndex;
   d_alignerUI->getCorrespondence(c, corrSourceIndex, corrTargetIndex);
   double srcSizeX, srcSizeY, tgtSizeX, tgtSizeY;
   d_images[SOURCE_IMAGE_INDEX]->
                   getAcquisitionDimensions(srcSizeX, srcSizeY);
   d_images[TARGET_IMAGE_INDEX]->
                   getAcquisitionDimensions(tgtSizeX, tgtSizeY);
   c.scalePoints(corrSourceIndex, srcSizeX, srcSizeY, 1.0);
   c.scalePoints(corrTargetIndex, tgtSizeX, tgtSizeY, 1.0);

   convertTo3DSpace(c, corrSourceIndex);
   ensureThreePoints(c, corrSourceIndex, vrpn_FALSE, vrpn_TRUE);

   registerImagesFromPointCorrespondence(c, corrSourceIndex, 
                                         corrTargetIndex, xform);

   return 0;
}

int nmr_Registration_Impl::registerImagesFromPointCorrespondence(
     Correspondence &c,
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
    
/*
// debugging: try printing out the mutual information estimate to 
// see if it makes sense
    nmr_AlignerMI *mi = new nmr_AlignerMI();
    mi->setDimensionMode(REF_2D);
    if (mi->setSampleSizes(100, 100)) {
      printf("setSampleSizes error\n");
    } else {
        //printf("setSampleSizes succeeded\n");
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
        //printf("setTransformation2D succeeded\n");
    }
    if (mi->buildSampleA(d_images[SOURCE_IMAGE_INDEX],
                         d_images[TARGET_IMAGE_INDEX]) ||
        mi->buildSampleB(d_images[SOURCE_IMAGE_INDEX],
                         d_images[TARGET_IMAGE_INDEX])) {
      printf("buildSample error\n");
    } else {
        //printf("buildSample suceeded\n");
    }
    double mutual_info;
    if (mi->computeMutualInformation(mutual_info)){
      printf("computeMutualInformation error\n");
    } else {
        //printf("computeMutualInformation succeeded; value = %g\n", mutual_info);
    }
*/
    return 0;
}

int nmr_Registration_Impl::registerImagesUsingMutualInformation(
                                           nmb_Transform_TScShR &xform,
                                           vrpn_bool initialize)
{
    double xform_matrix[16];
    nmb_Transform_TScShR test_xform = xform;
    // xform is 4x4 in column-major order but T is 2x3 in row-major order
    // copy our initial search point from xform
    if (initialize) {
      double T[6];
      xform.getMatrix(xform_matrix);
      T[0] = xform_matrix[0]; T[1] = xform_matrix[4]; T[2] = xform_matrix[12];
      T[3] = xform_matrix[1]; T[4] = xform_matrix[5]; T[5] = xform_matrix[13];
      d_mutInfoAligner.setTransformation(T);
    }


    if (d_imageChangeSinceLastRegistration) {
        d_mutInfoAligner.initImages(d_images[SOURCE_IMAGE_INDEX],
                           d_images[TARGET_IMAGE_INDEX]);
        d_imageChangeSinceLastRegistration = vrpn_FALSE;
    }

/*
    double change;
    int numDirectionSearches = 0;
    const int numDirections = 6;
    int directionOrder[6] = {
      NMR_ROTATE_Z,
      NMR_SHEAR,
      NMR_TRANSLATE_X, 
      NMR_TRANSLATE_Y,
      NMR_SCALE_X,
      NMR_SCALE_Y
      };
*/

/*
    FILE *outfile = fopen("regdata.txt", "w");
    for (i = 0; i < numDirections; i++) {
      d_mutInfoAligner.plotObjective(directionOrder[i], 21, outfile);
    }
    fclose(outfile);

    d_mutInfoAligner.getTransformation(T);
*/

    xform = test_xform;

    return 0;
}

void nmr_Registration_Impl::sendResult(double *xform_matrix) 
{
  if (d_server){
    d_server->sendRegistrationResult(xform_matrix);
  }
}

void nmr_Registration_Impl::ensureThreePoints(Correspondence &c,
      int corrSourceIndex, vrpn_bool normalized, vrpn_bool lookupZ)
{
  double srcSizeX, srcSizeY, tgtSizeX, tgtSizeY;
  d_images[SOURCE_IMAGE_INDEX]->getAcquisitionDimensions(srcSizeX, srcSizeY);
  d_images[TARGET_IMAGE_INDEX]->getAcquisitionDimensions(tgtSizeX, tgtSizeY);

  // XXX: this function is serving the additional purpose of ensuring that
  // there are at least three points in the correspondence so that the
  // transformation is sufficiently constrained - this addition was necessary
  // when we switched from doing things in normalized image coordinates to
  // scaled normalized image coordinates - previous to this, the transformSolver
  // result was correct when given only 1 or 2 points since it would do a
  // pure translation when given only one point but now a single point
  // does not
  double srcMaxX, srcMaxY;
  double tgtMaxX, tgtMaxY;
  if (normalized) {
    srcMaxX = 1.0;
    srcMaxY = 1.0;
    tgtMaxX = 1.0;
    tgtMaxY = 1.0;
  } else {
    srcMaxX = srcSizeX;
    srcMaxY = srcSizeY;
    tgtMaxX = tgtSizeX;
    tgtMaxY = tgtSizeY;
  }
  corr_point_t p0, p1, p2;
  double xIndex, yIndex;
  int i;
  double offset = 0.25;
  if (c.numPoints() == 1) {
    c.addPoint(p0);
    c.getPoint(corrSourceIndex, 0, &p0);
    // ensure that the new point falls in the src image
    if (p0.x > 0.5*srcMaxX) {
      offset *= -1;
    }
    p1.x = p0.x + offset*srcMaxX;
    p1.y = p0.y;
    if (lookupZ) {
      xIndex = (double)d_images[SOURCE_IMAGE_INDEX]->width()*p1.x/srcMaxX;
      yIndex = (double)d_images[SOURCE_IMAGE_INDEX]->height()*p1.y/srcMaxY;
      p1.z = d_images[SOURCE_IMAGE_INDEX]->getValueInterpolated(xIndex, yIndex);
      c.setPoint(corrSourceIndex, 1, p1);
      p1.z = 0;
    }
    for (i = 0; i < c.numSpaces(); i++) {
      if (i != corrSourceIndex) {
        c.getPoint(i, 0, &p0);
        p1.x = p0.x + offset*tgtMaxX;
        p1.y = p0.y;
        p1.z = 0.0;
        c.setPoint(i, 1, p1);
      }
    }
  }
  double offsetFactor = 0.25;
  if (c.numPoints() == 2) {
    c.addPoint(p0);
    // ensure that the new point falls in the src image
    c.getPoint(corrSourceIndex, 0, &p0);
    c.getPoint(corrSourceIndex, 1, &p1);
    p2.x = 0.5*(p0.x + p1.x) + offsetFactor*(p1.y - p0.y);
    p2.y = 0.5*(p0.y + p1.y) - offsetFactor*(p1.x - p0.x);
    if (p2.x < 0 || p2.x > srcMaxX || p2.y < 0 || p2.y > srcMaxY) {
      offsetFactor *= -1;
      p2.x = 0.5*(p0.x + p1.x) + offsetFactor*(p1.y - p0.y);//*srcMaxX/srcMaxY;
      p2.y = 0.5*(p0.y + p1.y) - offsetFactor*(p1.x - p0.x);//*srcMaxY/srcMaxX;
    }
    if (lookupZ) {
      xIndex = (double)d_images[SOURCE_IMAGE_INDEX]->width()*p2.x/srcMaxX;
      yIndex = (double)d_images[SOURCE_IMAGE_INDEX]->height()*p2.y/srcMaxY;
      p2.z = d_images[SOURCE_IMAGE_INDEX]->getValueInterpolated(xIndex, yIndex);
      c.setPoint(corrSourceIndex, 2, p2);
      p2.z = 0;
    }
    for (i = 0; i < c.numSpaces(); i++) {
      if (i != corrSourceIndex) {
        c.getPoint(i, 0, &p0);
        c.getPoint(i, 1, &p1);
        p2.x = 0.5*(p0.x + p1.x) + offsetFactor*(p1.y - p0.y);
        p2.y = 0.5*(p0.y + p1.y) - offsetFactor*(p1.x - p0.x);
        c.setPoint(i, 2, p2);
      }
    }
  }
}

void nmr_Registration_Impl::convertTo3DSpace(Correspondence &c,
                 int corrSourceIndex)
{
  double invSrcSizeX, invSrcSizeY;
  double srcSizeX, srcSizeY;
  d_images[SOURCE_IMAGE_INDEX]->getAcquisitionDimensions(srcSizeX, srcSizeY);
  invSrcSizeX = 1.0/srcSizeX;
  invSrcSizeY = 1.0/srcSizeY;

  int pointIndex = 0;
  corr_point_t srcPoint;
  double xIndex, yIndex;
  for (pointIndex = 0; pointIndex < c.numPoints(); pointIndex++) {
    c.getPoint(corrSourceIndex, pointIndex, &srcPoint);
    xIndex = srcPoint.x*invSrcSizeX*
             (double)d_images[SOURCE_IMAGE_INDEX]->width();
    yIndex = srcPoint.y*invSrcSizeY*
             (double)d_images[SOURCE_IMAGE_INDEX]->height();
    srcPoint.z =
           d_images[SOURCE_IMAGE_INDEX]->getValueInterpolated(xIndex, yIndex);
    c.setPoint(corrSourceIndex, pointIndex, srcPoint);
  }
}

/* 
  adjustTransformFromRotatedCorrespondence:
   The idea behind this function is that we know a few things about what
   the transformation should be and we should use those things to 
   initialize our automatic search (using mutual information). One thing we
   are given is a set of corresponding landmarks input by the user. Using
   this correspondence by itself, it may be possible to compute
   a full 3D transformation matrix that is optimal (minimizing sum of squared
   distances) (the linear least squares code can easily do this with a 
   little tweaking). The problem with this is that the range of
   z in our application is very close to size of the errors in locating
   points in x and y so while the data contains 3D points there is a good
   possibility that these points will give very bad results if we try to 
   extract a 3D transformation from them.
   An alternative is to use our knowledge of the imaging parameters to
   give a reasonable starting point for the 3D component of the transformation
   (the rotation about x and y axes) and to compute the rest of the
   transformation from the correspondence. This function computes this
   hybrid transformation.

   This assumes the order of transformations for nmb_Transform_TScShR starts
   with a rotation about the x axis followed by a rotation about the y axis.

   Outline of algorithm:
   o Apply the Ry.Rx rotation (extracted from the nmb_Transform_TScShR argument
     which is passed in) to the source points in the correspondence
   o Solve for the 2D-2D transformation from the resulting correspondence.
   o Extract the 2D transformation parameters (rotation about Z, shear about z, 
     scale in x and y, and translation in x and y) from that 2D transformation.
   o Compose the Ry.Rx part with the 2D part and put the result in the
     nmb_Transform_TScShR object which was passed as an argument.
*/ 
int nmr_Registration_Impl::adjustTransformFromRotatedCorrespondence(
          Correspondence &c, int corrSourceIndex, int corrTargetIndex,
          nmb_Transform_TScShR &xform)
{
  nmb_Transform_TScShR rotation;
  double rotateX, rotateY, rotateZ, scaleX, scaleY, shearZ, translate[4];
  rotateX = xform.getRotation(NMB_X);
  rotateY = xform.getRotation(NMB_Y);

  rotation.setRotation(NMB_X, rotateX);
  rotation.setRotation(NMB_Y, rotateY);

  // convert the source points from 2D normalized image coordinates to 
  // points in 3D spatial units (presumably nanometers)
  convertTo3DSpace(c, corrSourceIndex);

  ensureThreePoints(c, corrSourceIndex, vrpn_FALSE, vrpn_TRUE);

  Correspondence rotatedC(2,3);
  rotatedC = c;

  // now rotate them
  int pointIndex = 0;
  corr_point_t srcPoint, tgtPoint;
  double centerX, centerY, centerZ;
  xform.getCenter(centerX, centerY, centerZ);
  double srcPointArr[4] = {0, 0, 0, 1};
  double rotatedPnt[4] = {0, 0, 0, 1};
  for (pointIndex = 0; pointIndex < c.numPoints(); pointIndex++) {
    c.getPoint(corrSourceIndex, pointIndex, &srcPoint);
    srcPointArr[0] = srcPoint.x;
    srcPointArr[1] = srcPoint.y;
    srcPointArr[2] = srcPoint.z;

    // now rotate it and copy it back into the correspondence
    rotation.transform(srcPointArr, rotatedPnt);
    srcPoint.x = rotatedPnt[0];
    srcPoint.y = rotatedPnt[1];
    srcPoint.z = 0.0; // rotatedPnt[2]; - we should only need 2D points
    rotatedC.setPoint(corrSourceIndex, pointIndex, srcPoint);
  }

  double xform_matrix[16];
  // now we solve for the 2D->2D transformation part
  if (registerImagesFromPointCorrespondence(rotatedC, 
      corrSourceIndex, corrTargetIndex,
      xform_matrix)) {
    printf("nmr_Registration_Impl::adjustTransform:"
           " Error computing transform from correspondence\n");
    return -1;
  }

  nmb_TransformMatrix44 transform2D;
  transform2D.setMatrix(xform_matrix);

  // The center/pivot needs to be (0,0) for this transformation because the
  // position it holds in the sequence of transformations comes after the
  // translation of the pivot point to the origin (this confused me for a
  // while but it kind of makes sense now)
  transform2D.getTScShR_2DParameters(0.0, 0.0, translate[0], translate[1],
                              rotateZ, shearZ, scaleX, scaleY);
  translate[2] = 0.0;
  translate[3] = 1.0;
  // getTScShR_2DParameters gives the parameters for nmb_Transform_TScShR
  // in the nmb_Transform_TScShR transformation order so we now have
  // T.Sc.Sh.Rz
  // what we want to end up with is
  // T.Sc.Sh.Rz.Ry.Rx

  xform.setIdentity();
  xform.setTranslation(NMB_X, translate[0]);
  xform.setTranslation(NMB_Y, translate[1]);
  xform.setTranslation(NMB_Z, translate[2]);
  xform.setRotation(NMB_X, rotateX);
  xform.setRotation(NMB_Y, rotateY);
  xform.setRotation(NMB_Z, rotateZ);
  xform.setScale(NMB_X, scaleX);
  xform.setScale(NMB_Y, scaleY);
  xform.setShear(NMB_Z, shearZ);

  return 0;
}

// a couple utility functions to let us express things in terms of the 
// transformed z axis R([0,0,1]) = [vx, vy, vz]
void nmr_Registration_Impl::convertRyRxToViewingDirection(double Ry, double Rx,
                         double &vx, double &vy, double &vz)
{
  vx = cos(Rx)*sin(Ry);
  vy = -sin(Rx);
  vz = cos(Rx)*cos(Ry);
}

void nmr_Registration_Impl::convertViewingDirectionToRyRx(
                                       double vx, double vy, double vz,
                                       double &Ry, double &Rx)
{
  double inv_mag = 1.0/sqrt(vx*vx + vy*vy + vz*vz);
  double vxn = vx*inv_mag, vyn = vy*inv_mag, vzn = vz*inv_mag;
  double inv_cos_Rx, sin_Ry, cos_Ry;
  Rx = asin(-vyn);
  // Rx could also be M_PI - Rx but I think it doesn't matter
  if (fabs(vyn) < 1) {
    inv_cos_Rx = 1.0/cos(Rx);
    sin_Ry = vxn*inv_cos_Rx;
    cos_Ry = vzn*inv_cos_Rx;
    Ry = asin(sin_Ry);
    if (cos_Ry < 0) {
      Ry = M_PI - Ry;
    }
  } else {
    Ry = 0.0;
  }
}
