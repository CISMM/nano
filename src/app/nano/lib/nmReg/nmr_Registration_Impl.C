/*===3rdtech===
  Copyright (c) 2001 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#include "nmr_Registration_Impl.h"
#include "transformSolve.h"
#include "nmb_Transform_TScShR.h"
#include "nmb_TransformMatrix44.h"

// M_PI not defined for VC++, for some reason.
#ifndef M_PI
static double M_PI = 3.14159265358979323846;
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
  d_images[SOURCE_IMAGE_INDEX] = NULL;
  d_images[TARGET_IMAGE_INDEX] = NULL;

  d_alignerUI = new nmr_Registration_ImplUI();
  d_alignerUI->registerCorrespondenceHandler(handle_CorrespondenceChange,
                                             (void *) this);
  d_autoUpdateAlignment = vrpn_TRUE;
}

nmr_Registration_Impl::~nmr_Registration_Impl()
{
  if (d_images[SOURCE_IMAGE_INDEX]) {
	nmb_Image::deleteImage(d_images[SOURCE_IMAGE_INDEX]);
  }
  if (d_images[TARGET_IMAGE_INDEX]) {
	nmb_Image::deleteImage(d_images[TARGET_IMAGE_INDEX]);
  }
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
  vrpn_bool flip_x, flip_y;
  vrpn_int32 row, length;
  vrpn_float32 *data;
  vrpn_int32 replace, numFiducial;
  vrpn_float32 x_src[NMR_MAX_FIDUCIAL], y_src[NMR_MAX_FIDUCIAL], 
         z_src[NMR_MAX_FIDUCIAL], x_tgt[NMR_MAX_FIDUCIAL], 
         y_tgt[NMR_MAX_FIDUCIAL], z_tgt[NMR_MAX_FIDUCIAL];
  vrpn_int32 mode;
  vrpn_bool enabled;
  vrpn_int32 window;
  vrpn_bool addAndDelete, move;

  vrpn_int32 numLevels;
  vrpn_float32 resLevels[NMR_MAX_RESOLUTION_LEVELS];
  vrpn_int32 maxIterations;
  vrpn_float32 stepSize;
  vrpn_int32 resolutionIndex;
  vrpn_float32 *transformParameters = 
                    new vrpn_float32[nmb_numTransformParameters];

  switch(info.msg_type) {
    case NMR_IMAGE_PARAM:
//      printf("nmr_Registration_Impl: image params received\n");
      info.aligner->getImageParameters(whichImage, res_x, res_y, 
                    size_x, size_y, flip_x, flip_y);
      me->setImageParameters(whichImage, res_x, res_y, 
                             size_x, size_y, flip_x, flip_y);
      break;
    case NMR_TRANSFORM_OPTION:
      info.aligner->getTransformationOptions(xformtype);
      me->setTransformationOptions(xformtype);
      break;
    case NMR_TRANSFORM_PARAM:
      info.aligner->getTransformationParameters(transformParameters);
      me->setTransformationParameters(transformParameters);
      break;
    case NMR_SCANLINE:
//      printf("nmr_Registration_Impl: scanline received\n");
      info.aligner->getScanline(whichImage, row, length, &data);
      me->setScanline(whichImage, row, length, data);
      break;
    case NMR_FIDUCIAL:
      info.aligner->getFiducial(replace, numFiducial, 
                                x_src, y_src, z_src, x_tgt, y_tgt, z_tgt);
      // x and y are in range 0..1, z is in nm
      me->setFiducial(replace, numFiducial,
                      x_src, y_src, z_src, x_tgt, y_tgt, z_tgt);
	  me->reportFiducial();
      break;
    case NMR_ENABLE_AUTOUPDATE:
      info.aligner->getAutoUpdateEnable(enabled);
      me->enableAutoUpdate(enabled);
      break;
    case NMR_AUTOALIGN:
      info.aligner->getAutoAlign(mode);
      me->autoAlign(mode);
      break;
    case NMR_ENABLE_GUI:
      info.aligner->getGUIEnable(enabled, window);
      me->setGUIEnable(enabled, window);
      break;
	case NMR_ENABLE_EDIT:
	  info.aligner->getEditEnable(addAndDelete, move);
	  me->setEditEnable(addAndDelete, move);
	  break;
    case NMR_SET_RESOLUTIONS:
      info.aligner->getResolutions(numLevels, resLevels);
      me->setResolutions(numLevels, resLevels);
      break;
    case NMR_SET_ITERATION_LIMIT:
      info.aligner->getIterationLimit(maxIterations);
      me->setIterationLimit(maxIterations);
      break;
    case NMR_SET_STEPSIZE:
      info.aligner->getStepSize(stepSize);
      me->setStepSize(stepSize);
      break;
    case NMR_SET_CURRENT_RESOLUTION:
      info.aligner->getCurrentResolution(resolutionIndex);
      me->setCurrentResolution(resolutionIndex);
      break;
    default:
      break;
  }
  delete [] transformParameters;
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

int nmr_Registration_Impl::setResolutions(vrpn_int32 numLevels, 
                                          vrpn_float32 *stddev)
{
  if (numLevels > NMR_MAX_RESOLUTION_LEVELS) return -1;

  d_numLevels = numLevels;
  int i;
  for (i = 0; i < numLevels; i++) {
    d_stddev[i] = stddev[i];
  }
  return 0;
}

int nmr_Registration_Impl::setIterationLimit(vrpn_int32 maxIterations)
{
  d_maxIterations = maxIterations;
  return 0;
}

int nmr_Registration_Impl::setStepSize(vrpn_float32 stepSize)
{
  d_stepSize = stepSize;
  return 0;
}

int nmr_Registration_Impl::setCurrentResolution(vrpn_int32 resolutionIndex)
{
  d_resolutionIndex = resolutionIndex;
  return 0;
}

int nmr_Registration_Impl::enableAutoUpdate(vrpn_bool enable)
{
  d_autoUpdateAlignment = enable;
  return 0;
}

int nmr_Registration_Impl::autoAlign(vrpn_int32 mode)
{
  double xform_matrix[16];

  switch (mode) {
    case NMR_AUTOALIGN_FROM_MANUAL:
      d_autoAlignmentResult = d_manualAlignmentResult;
      break;
    case NMR_AUTOALIGN_FROM_DEFAULT:
      d_autoAlignmentResult = d_defaultTransformation;
      break;
    case NMR_AUTOALIGN_FROM_AUTO:
      break;
  }
  registerImagesUsingMutualInformation(d_autoAlignmentResult);
  d_autoAlignmentResult.getMatrix(xform_matrix);
  sendResult(NMR_AUTOMATIC, xform_matrix);
  return 0;
}

/*
    if (enable) {
        static Correspondence last_c;
        Correspondence c(2, 3);
        int corrSourceIndex, corrTargetIndex;
        d_alignerUI->getCorrespondence(c, corrSourceIndex, corrTargetIndex);
        if (!last_c.equals(c)) {
          printf("Warning: reinitializing auto-align with manual alignment\n");
          last_c = c;
          double srcSizeX, srcSizeY, tgtSizeX, tgtSizeY;
          d_images[SOURCE_IMAGE_INDEX]->
                       getAcquisitionDimensions(srcSizeX, srcSizeY);
          d_images[TARGET_IMAGE_INDEX]->
                       getAcquisitionDimensions(tgtSizeX, tgtSizeY);
          c.scalePoints(corrSourceIndex, srcSizeX, srcSizeY, 1.0);
          c.scalePoints(corrTargetIndex, tgtSizeX, tgtSizeY, 1.0);
          double center[4] = {0.0, 0.0, 0.0, 1.0};
          center[0] = 0.5*srcSizeX;
          center[1] = 0.5*srcSizeY;
          center[2] = d_images[SOURCE_IMAGE_INDEX]->getValueInterpolated(
                         0.5*d_images[SOURCE_IMAGE_INDEX]->width(),
                         0.5*d_images[SOURCE_IMAGE_INDEX]->height());
          xform.setCenter(center[0], center[1], center[2]);

          // can insert a guess about Rx and Ry here (put into xform)

          adjustTransformFromRotatedCorrespondence(c, corrSourceIndex,
                                  corrTargetIndex, xform);
          double *feature_x, *feature_y;
          int numPnts = c.numPoints();
          feature_x = new double[numPnts];
          feature_y = new double[numPnts];
          int i;
          corr_point_t pnt;
          for (i = 0; i < c.numPoints(); i++) {
            c.getPoint(corrSourceIndex, i, &pnt);
            feature_x[i] = pnt.x;
            feature_y[i] = pnt.y;
          }
          //d_mutInfoAligner.setRefFeaturePoints(numPnts, feature_x, feature_y);
          delete [] feature_x;
          delete [] feature_y;

        } else {
          printf("Warning: continuing auto-align from "
                 "result of previous auto-align\n");
        }
        
        // now we are ready to adjust xform using an automatic algorithm 
        registerImagesUsingMutualInformation(xform);
        xform.getMatrix(xform_matrix);
        sendResult(xform_matrix);
    }
    return 0;
}
*/

int nmr_Registration_Impl::setGUIEnable(vrpn_bool enable, vrpn_int32 window)
{
    d_alignerUI->enable(enable, window);
    return 0;
}

int nmr_Registration_Impl::setEditEnable(vrpn_bool enableAddAndDelete, 
										 vrpn_bool enableMove)
{
	d_alignerUI->enableEdit(enableAddAndDelete, enableMove);
	return 0;
}

int nmr_Registration_Impl::setImageParameters(nmr_ImageType whichImage,
                           vrpn_int32 res_x, vrpn_int32 res_y,
                           vrpn_float32 xSpan, 
                           vrpn_float32 ySpan, vrpn_bool flipX, vrpn_bool flipY)
{
    d_alignerUI->setImageOrientation(whichImage, flipX, flipY);
    if (whichImage == NMR_SOURCE) {
        if (!d_images[SOURCE_IMAGE_INDEX] ||
			(d_images[SOURCE_IMAGE_INDEX]->width() != res_x) ||
            (d_images[SOURCE_IMAGE_INDEX]->height() != res_y)) {
//              printf("changing source resolution: (%d, %d) --> (%d, %d)\n",
//                      d_images[SOURCE_IMAGE_INDEX]->width(),
//                      d_images[SOURCE_IMAGE_INDEX]->height(),
//                      res_x, res_y);
			if (d_images[SOURCE_IMAGE_INDEX]) {
                 nmb_Image::deleteImage(d_images[SOURCE_IMAGE_INDEX]);
				 d_images[SOURCE_IMAGE_INDEX] = NULL;
			}
			if (res_x != 0 && res_y != 0) {
				d_images[SOURCE_IMAGE_INDEX] =
				  new nmb_ImageGrid("sourceImage",
									"unknown_units",
									res_x, res_y);
			} else {
				// the image has become NULL so we shouldn't expect any
				// data and we should update the display now
				d_alignerUI->clearImage(NMR_SOURCE);
			}
        }
        setSourceImageDimensions(xSpan, ySpan);
    } else if (whichImage == NMR_TARGET) {
        if (!d_images[TARGET_IMAGE_INDEX] ||
			(d_images[TARGET_IMAGE_INDEX]->width() != res_x) ||
            (d_images[TARGET_IMAGE_INDEX]->height() != res_y)) {
//              printf("changing target resolution: (%d, %d) --> (%d, %d)\n",
//                      d_images[TARGET_IMAGE_INDEX]->width(),
//                      d_images[TARGET_IMAGE_INDEX]->height(),
//                      res_x, res_y);
			if (d_images[TARGET_IMAGE_INDEX]) {
				nmb_Image::deleteImage(d_images[TARGET_IMAGE_INDEX]);
				d_images[TARGET_IMAGE_INDEX] = NULL;
			}
			if (res_x != 0 && res_y != 0) {
				d_images[TARGET_IMAGE_INDEX] = 
					 new nmb_ImageGrid("targetImage",
										"unknown_units",
										res_x, res_y);
			} else {
				// the image has become NULL so we shouldn't expect any
				// data and we should update the display now
				d_alignerUI->clearImage(NMR_TARGET);
			}
        }
        setTargetImageDimensions(xSpan, ySpan);
    } else {
        fprintf(stderr, "RegistrationImpl::setImageParameters:"
                        " Error, unknown image type\n");
        return -1;
    }
    return 0;
}

// x and y are in range 0..1, z is in nm
int nmr_Registration_Impl::setFiducial(vrpn_int32 replace, vrpn_int32 num,
		vrpn_float32 *x_src, vrpn_float32 *y_src, vrpn_float32 *z_src,
                vrpn_float32 *x_tgt, vrpn_float32 *y_tgt, vrpn_float32 *z_tgt)
{
    d_alignerUI->setFiducial(replace, num,
                  x_src, y_src, z_src, x_tgt, y_tgt, z_tgt);
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
  d_imageChangeSinceLastRegistration = vrpn_TRUE;
  return 0;
}

int nmr_Registration_Impl::setTransformationOptions(
                           nmr_TransformationType type)
{
    d_transformType = type;
    return 0;
}

int nmr_Registration_Impl::setTransformationParameters(vrpn_float32 *parameters)
{
  int i;
  for (i = 0; i < nmb_numTransformParameters; i++) {
    d_defaultTransformation.setParameter(nmb_transformParameterOrder[i],
                                         parameters[i]);
  }
  double xform_matrix[16];
  d_defaultTransformation.getMatrix(xform_matrix);
  sendResult(NMR_DEFAULT, xform_matrix);
  return 0;
}

int nmr_Registration_Impl::
            registerImagesFromPointCorrespondenceAssumingDefaultRotation()
{
	if (!d_images[SOURCE_IMAGE_INDEX] || !d_images[TARGET_IMAGE_INDEX]) {
		return 0;
	}
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

	nmb_Transform_TScShR solution = d_defaultTransformation;
	int errorIndicator = adjustTransformFromRotatedCorrespondence(
				   c, corrSourceIndex, corrTargetIndex, 
				   solution);
	// make sure this worked before replacing the last computed transformation
	if (!errorIndicator) {
		d_manualAlignmentResult = solution;
	}
	return errorIndicator;
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
    if (result == -1) {
	return -1;
    }
    if (xform) {
        for (int i = 0; i < 16; i++){
            xform[i] = xform_matrix[i];
        }
    }
    
    return 0;
}

int nmr_Registration_Impl::registerImagesUsingMutualInformation(
                                           nmb_Transform_TScShR &xform)
{
	if (!d_images[SOURCE_IMAGE_INDEX] || !d_images[TARGET_IMAGE_INDEX]) {
		return 0;
	}
    if (d_imageChangeSinceLastRegistration) {
        double centerX, centerY, centerZ;
        xform.getCenter(centerX, centerY, centerZ);
        nmb_Transform_TScShR identity;
        identity.setCenter(centerX, centerY, centerZ);
        d_mutInfoAligner.initImages(d_images[SOURCE_IMAGE_INDEX],
                           d_images[TARGET_IMAGE_INDEX],
                           d_numLevels, d_stddev);
        d_mutInfoAligner.setTransform(identity);
//      d_mutInfoAligner.optimizeVarianceParameters();
        // some useful output if you want to visualize whats going on 
/*
        FILE *outputFile = fopen("objectivePlot.txt", "w");
        d_mutInfoAligner.plotObjective(outputFile);
        fclose(outputFile);
*/
        d_imageChangeSinceLastRegistration = vrpn_FALSE;
    }

    d_mutInfoAligner.setTransform(xform);
    fprintf(stderr, "registerImagesUsingMutualInformation: Warning,"
            "automatically saving histogram TIFF files\n");
	d_mutInfoAligner.printJointHistograms(xform);
	
    bool useGradientDescent = false;
	if (useGradientDescent) {
		d_mutInfoAligner.takeGradientSteps(d_resolutionIndex,
	                                       d_maxIterations, d_stepSize);
    } else {
		d_mutInfoAligner.multiResPatternSearch(d_maxIterations, d_stepSize);
	}

    d_mutInfoAligner.getTransform(xform);

    return 0;
}

void nmr_Registration_Impl::setSourceImageDimensions(vrpn_float32 srcSizeX,
                                                     vrpn_float32 srcSizeY)
{
	if (d_images[SOURCE_IMAGE_INDEX]) {
		d_images[SOURCE_IMAGE_INDEX]->
					  setAcquisitionDimensions(srcSizeX, srcSizeY);

		double resX, resY;
		resX = (double)d_images[SOURCE_IMAGE_INDEX]->width();
		resY = (double)d_images[SOURCE_IMAGE_INDEX]->height();
		double center[3];
		center[0] = 0.5*srcSizeX;
		center[1] = 0.5*srcSizeY;
		center[2] = d_images[SOURCE_IMAGE_INDEX]->getValueInterpolated(
							 0.5*resX,
							 0.5*resY);
		d_defaultTransformation.setCenter(center[0], center[1], center[2]);
		d_manualAlignmentResult.setCenter(center[0], center[1], center[2]);
		d_autoAlignmentResult.setCenter(center[0], center[1], center[2]);
	}
}

void nmr_Registration_Impl::setTargetImageDimensions(vrpn_float32 tgtSizeX,
                                                     vrpn_float32 tgtSizeY)
{
	if (d_images[TARGET_IMAGE_INDEX]) {
		d_images[TARGET_IMAGE_INDEX]->
			setAcquisitionDimensions(tgtSizeX, tgtSizeY);
	}
}

void nmr_Registration_Impl::sendResult(int whichTransform, 
                                       double *xform_matrix) 
{
  if (d_server){
    d_server->sendRegistrationResult(whichTransform, xform_matrix);
  }


  // the following is a potentially useful calculation but isn't used 
  // at the moment.
    nmb_TransformMatrix44 transform;
    transform.setMatrix(xform_matrix);
    double centerX, centerY, tx, ty, phi, shz, scx, scy;
    double srcSizeX = 0, srcSizeY = 0;
	if (d_images[SOURCE_IMAGE_INDEX]) {
		d_images[SOURCE_IMAGE_INDEX]->
						   getAcquisitionDimensions(srcSizeX, srcSizeY);
	}
    centerX = 0.5*srcSizeX;
    centerY = 0.5*srcSizeY;
    transform.getTScShR_2DParameters(centerX,centerY,tx,ty,phi,shz,scx,scy);
    double minX = 0, maxX = srcSizeX;
    double minY = 0, maxY = srcSizeY;
    double x_atMaxOffset = 0, y_atMaxOffset = 0;
    double maxOffset, meanOffset;

    maxOffset = nmr_Util::maximumOffset2D(transform, minX, maxX, minY, maxY,
                              x_atMaxOffset, y_atMaxOffset);
    meanOffset = nmr_Util::approxMeanOffset2D(transform,
                              minX, maxX, minY, maxY);
//    printf("sending (tx,ty,phi,shz,scx,scy, cx, cy, maxOffset, meanOffset) = "
//           "%g, %g, %g, %g, %g, %g, %g, %g, %g, %g\n",
//           tx, ty, phi, shz, scx, scy, centerX, centerY, maxOffset, meanOffset);

}

void nmr_Registration_Impl::reportFiducial()
{
	vrpn_int32 num, replace = 1;
    vrpn_float32 x_src[NMR_MAX_FIDUCIAL], y_src[NMR_MAX_FIDUCIAL],
                 z_src[NMR_MAX_FIDUCIAL];
    vrpn_float32 x_tgt[NMR_MAX_FIDUCIAL], y_tgt[NMR_MAX_FIDUCIAL],
                 z_tgt[NMR_MAX_FIDUCIAL];

	d_alignerUI->getFiducial(num, x_src, y_src, z_src, 
		x_tgt, y_tgt, z_tgt);
	if (d_server) {
		d_server->reportFiducial(replace, num, x_src, y_src, z_src, 
			x_tgt, y_tgt, z_tgt);
	}
}

//static
void nmr_Registration_Impl::handle_CorrespondenceChange(Correspondence &c,
                                                          void *ud)
{
  nmr_Registration_Impl *me = (nmr_Registration_Impl *)ud;
  if (me->d_autoUpdateAlignment) {
    double xform_matrix[16];
    if (me->registerImagesFromPointCorrespondenceAssumingDefaultRotation()
         == 0) {
      me->d_manualAlignmentResult.getMatrix(xform_matrix);
      me->sendResult(NMR_MANUAL, xform_matrix);
    }
  }
  me->reportFiducial();
}

int nmr_Registration_Impl::getManualAlignmentResult(nmb_Transform_TScShR
                                                    &transform)
{
  transform = d_manualAlignmentResult;
  return 0;
}

int nmr_Registration_Impl::getAutoAlignmentResult(nmb_Transform_TScShR
                                                    &transform)
{
  transform = d_autoAlignmentResult;
  return 0;
}

int nmr_Registration_Impl::getDefaultTransformation(nmb_Transform_TScShR
                                                    &transform)
{
  transform = d_defaultTransformation;
  return 0;
}

vrpn_int32 nmr_Registration_Impl::setImage(
                        nmr_ImageType whichImage, nmb_Image *im,
                        vrpn_bool flip_x, vrpn_bool flip_y)
{
  int i,j;
  vrpn_float32 *data;
  double xSize = 0, ySize = 0;
  im->getAcquisitionDimensions(xSize, ySize);
  setImageParameters(whichImage, im->width(), im->height(),
                         xSize, ySize, flip_x, flip_y);
  data = new vrpn_float32[im->width()];
  for (i = 0; i < im->height(); i++) {
      for (j = 0; j < im->width(); j++) {
          data[j] = im->getValue(j, i);
      }
      setScanline(whichImage, i, im->width(), data);
  }
  delete [] data;
  return 0;
}

void nmr_Registration_Impl::ensureThreePoints(Correspondence &c,
      int corrSourceIndex, vrpn_bool normalized, vrpn_bool lookupZ)
{
  double srcSizeX=1, srcSizeY=1, tgtSizeX=1, tgtSizeY=1;
  if (d_images[SOURCE_IMAGE_INDEX]) {
	d_images[SOURCE_IMAGE_INDEX]->getAcquisitionDimensions(srcSizeX, srcSizeY);
  }
  if (d_images[TARGET_IMAGE_INDEX]) {
	d_images[TARGET_IMAGE_INDEX]->getAcquisitionDimensions(tgtSizeX, tgtSizeY);
  }

  double srcMaxX, srcMaxY;
  double tgtMaxX;//, tgtMaxY;
  if (normalized) {
    srcMaxX = 1.0;
    srcMaxY = 1.0;
    tgtMaxX = 1.0;
//    tgtMaxY = 1.0;
  } else {
    srcMaxX = srcSizeX;
    srcMaxY = srcSizeY;
    tgtMaxX = tgtSizeX;
//    tgtMaxY = tgtSizeY;
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
	p1.z = 0;
    if (lookupZ && d_images[SOURCE_IMAGE_INDEX]) {
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
	p2.z = 0;
    if (lookupZ && d_images[SOURCE_IMAGE_INDEX]) {
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

// converts 2D points in the source image to 3D using the 
// height values in the source image
// 3D points that do not lie on the surface represented by the source image
// will not be converted (e.g. from AFM probe position-based fiducials)
void nmr_Registration_Impl::convertTo3DSpace(Correspondence &c,
                 int corrSourceIndex)
{

  if (!d_images[SOURCE_IMAGE_INDEX]) {
	  return;
  }

  double invSrcSizeX, invSrcSizeY;
  double srcSizeX = 1, srcSizeY = 1;

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
    if (srcPoint.is2D) {
      srcPoint.z =
           d_images[SOURCE_IMAGE_INDEX]->getValueInterpolated(xIndex, yIndex);
      srcPoint.is2D = vrpn_FALSE;
    }
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
  double rotate2D_Z, rotate3D_X, rotate3D_Z, scaleX, scaleY, shearZ, translate[4];
  rotate3D_X = xform.getRotation(NMB_THETA);
  rotate3D_Z = xform.getRotation(NMB_PHI);

  rotation.setRotation(NMB_THETA, rotate3D_X);
  rotation.setRotation(NMB_PHI, rotate3D_Z);

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
  rotation.setCenter(centerX, centerY, centerZ);
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
//  transform2D.getTScShR_2DParameters(0.0, 0.0, translate[0], translate[1],
//                              rotateZ, shearZ, scaleX, scaleY);
  transform2D.getTScShR_2DParameters(centerX, centerY, 
                              translate[0], translate[1],
                              rotate2D_Z, shearZ, scaleX, scaleY);
  translate[2] = 0.0;
  translate[3] = 1.0;
  // getTScShR_2DParameters gives the parameters for nmb_Transform_TScShR
  // in the nmb_Transform_TScShR transformation order so we now have
  // T.Sc.Sh.Rz
  // what we want to end up with is
  // T.Sc.Sh.Rz.Ry.Rx

  xform.setIdentity();
  xform.setCenter(centerX, centerY, centerZ);
  xform.setTranslation(NMB_X, translate[0]);
  xform.setTranslation(NMB_Y, translate[1]);
  xform.setTranslation(NMB_Z, translate[2]);
  xform.setRotation(NMB_PSI, rotate2D_Z);
  xform.setRotation(NMB_THETA, rotate3D_X);
  xform.setRotation(NMB_PHI, rotate3D_Z);
  xform.setScale(NMB_X, scaleX);
  xform.setScale(NMB_Y, scaleY);
  xform.setShear(NMB_Z, shearZ);

  return 0;
}
