/*===3rdtech===
  Copyright (c) 2000 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#include "nmr_RegistrationUI.h"
#include "nmr_Util.h"
//#include <nmb_Dataset.h>
//#include <microscape.h> // for disableOtherTextures
#include <nmui_ColorMap.h>
#include <nmr_AlignerMI.h>
#include <nmb_Transform_TScShR.h>

#include "matrixcl.h"
using namespace math;
typedef matrix<double> Matrix;

#include <algorithm>

#ifndef M_PI
static double M_PI = 3.14159265358979323846;
#endif

/*
this is annoying because it takes too long to calculate the image pyramid
vrpn_int32 nmr_RegistrationUI::s_defaultNumResolutionLevels = 15;
vrpn_float32 nmr_RegistrationUI::s_defaultStdDev[] = 
               {0.0, 0.6, 1.0, 1.2, 1.4, 
                1.7, 2.0, 2.4, 3,  3.6,
                4.5,   6,  9, 13, 18};
*/
vrpn_int32 nmr_RegistrationUI::s_defaultNumResolutionLevels = 5;
vrpn_float32 nmr_RegistrationUI::s_defaultStdDev[] =
                {0.0,1.0,2.0,4.0,8.0};

vrpn_int32 nmr_RegistrationUI::s_numAutoAlignModes = 3;
nmr_AutoAlignMode nmr_RegistrationUI::s_autoAlignModes[] = 
                {NMR_AUTOALIGN_FROM_MANUAL, NMR_AUTOALIGN_FROM_DEFAULT,
                        NMR_AUTOALIGN_FROM_AUTO};
char * nmr_RegistrationUI::s_autoAlignModeNames[] =
                {"Manual Init", "Default Init", "Auto Init"};

vrpn_int32 nmr_RegistrationUI::s_numTransformationSources = 3;
nmr_RegistrationType nmr_RegistrationUI::s_transformationSources[] =
                {NMR_MANUAL, NMR_DEFAULT, NMR_AUTOMATIC};
char * nmr_RegistrationUI::s_transformationSourceNames[] =
                {"Manual", "Default", "Auto"};

nmr_RegistrationUI::nmr_RegistrationUI
  ( nmr_Registration_Proxy *aligner, 
    nmb_ImageDisplay *display):

   d_registrationImageName3D("reg_surface_cm(color_comes_from)", "none"),
   d_refresh3D("reg_refresh_3D", 0),
   d_fiducialSpotTracker3D("reg_fiducial_spot_tracker_3D", "none"),
   d_optimizeSpotTrackerRadius3D("reg_spot_tracker_optimize_radius_3D", 0),
   d_spotTrackerRadius3D("reg_spot_tracker_radius_3D", 5.0),
   d_spotTrackerRadiusAccuracy3D("reg_spot_tracker_radius_accuracy_3D", 0.05),
   d_spotTrackerPixelAccuracy3D("reg_spot_tracker_pixel_accuracy_3D", 0.05),
   d_registrationImageName2D("reg_projection_cm(color_comes_from)", "none"),
   d_refresh2D("reg_refresh_2D", 0),
   d_fiducialSpotTracker2D("reg_fiducial_spot_tracker_2D", "none"),
   d_optimizeSpotTrackerRadius2D("reg_spot_tracker_optimize_radius_2D", 0),
   d_spotTrackerRadius2D("reg_spot_tracker_radius_2D", 5.0),
   d_spotTrackerRadiusAccuracy2D("reg_spot_tracker_radius_accuracy_2D", 0.05),
   d_spotTrackerPixelAccuracy2D("reg_spot_tracker_pixel_accuracy_2D", 0.05),
   d_flipProjectionImageInX("reg_proj_flipX", 0),
   d_newResampleImageName("resample_image_name", ""),
   d_newResamplePlaneName("resample_plane_name", ""),
   d_registrationEnabled("reg_window_open", 0),
   d_constrainToTopography("reg_constrain_to_topography", 0),
   d_invertWarp("reg_invert_warp", 0),
   d_textureDisplayEnabled("reg_display_texture", 0),
   d_textureAlpha("reg_texture_alpha", 1),
   d_resampleResolutionX("resample_resolution_x", 100),
   d_resampleResolutionY("resample_resolution_y", 100),
   d_resampleRatio("reg_resample_ratio", 0),
   d_registrationColorMap3D("reg_surface_cm(color_map)", "none"),
   d_registrationColorMap2D("reg_projection_cm(color_map)", "none"),

   d_autoAlignRequested("auto_align_requested", 0),
   d_numIterations("auto_align_num_iterations", 100),
   d_stepSize("auto_align_step_size", 1.0),
   d_resolutionLevel("auto_align_resolution", "0"),
   d_numResolutionLevels(0),
   d_resolutionLevelList("auto_align_resolution_list"),
   d_saveRegistrationMarkers("save_registration_markers", 0), //new
   d_loadRegistrationMarkers("load_registration_markers", 0), //new
   d_saveReport("save_report", 0), //new
   d_runRansac("run_ransac", 0), //new
   d_calculatePoints("run_calculatePoints", 0), //new
   d_drawTopographyPoints("run_drawTopographyPoints", 0), //new
   d_saveTopographyPoints("run_saveTopographyPoints", 0), //new
   d_drawProjectionPoints("run_drawProjectionPoints", 0), //new
   d_saveProjectionPoints("run_saveProjectionPoints", 0), //new
   d_runDrawRansac("draw_ransac", 0), //new

   d_scaleX("reg_scaleX", 1.0),
   d_scaleY("reg_scaleY", 1.0),
   d_translateX("reg_translateX", 0.0),
   d_translateY("reg_translateY", 0.0),
   d_translateZ("reg_translateZ", 0.0),
   d_rotate2D_Z("reg_rotate2D_Z", 0.0), 
   d_rotate3D_X("reg_rotate3D_X", 0.0), 
   d_rotate3D_Z("reg_rotate3D_Z", 0.0),
   d_shearZ("reg_shearZ", 0.0),

   d_autoAlignMode("auto_align_mode", s_autoAlignModeNames[0]),
   d_autoAlignModeList("auto_align_mode_list"),

   d_transformationSource("reg_transformation_source", 
                          s_transformationSourceNames[0]),
   d_transformationSourceList("reg_transformation_source_list"),

   d_imageDisplay(display),
   d_dataset(NULL),
   d_aligner(aligner),
   d_3DImageCMap(NULL),
   d_2DImageCMap(NULL),
   d_last2DImage(NULL),
   d_last3DImage(NULL),
   d_flipXreference(vrpn_FALSE), 
   d_flipYreference(vrpn_FALSE),
   d_flipYadjustable(vrpn_FALSE),
   d_lastTransformTypeSent(NMR_DEFAULT),

   d_colormap2DCallbackDisabled(false)
{
    d_scaledProjImFromScaledTopoIm = 
        new nmb_TransformMatrix44[s_numTransformationSources];

    d_aligner->registerChangeHandler((void *)this, handle_registrationChange);
    d_3DImageCMap = new nmui_ColorMap("reg_surface_cm", 
              &d_registrationImageName3D,
              &d_registrationColorMap3D);
    d_3DImageCMap->setSurfaceColor(255,255,255);
    d_2DImageCMap = new nmui_ColorMap("reg_projection_cm", 
              &d_registrationImageName2D,
              &d_registrationColorMap2D);
    d_2DImageCMap->setSurfaceColor(255,255,255);

    d_numResolutionLevels = s_defaultNumResolutionLevels;
    int i;
    d_resolutionLevelList.clearList();
    char listEntry[128];
    for (i = 0; i < s_defaultNumResolutionLevels; i++) {
      d_stddev[i] = s_defaultStdDev[i];
      sprintf(listEntry, "%g", d_stddev[i]);
      d_resolutionLevelList.addEntry(listEntry);
    }
    d_aligner->setResolutions(d_numResolutionLevels, d_stddev);
    sendTransformationParameters();

    d_autoAlignModeList.clearList();
    for (i = 0; i < s_numAutoAlignModes; i++) {
      d_autoAlignModeList.addEntry(s_autoAlignModeNames[i]);
    }

    d_transformationSourceList.clearList();
    for (i = 0; i < s_numTransformationSources; i++) {
      d_transformationSourceList.addEntry(s_transformationSourceNames[i]);
    }
}

nmr_RegistrationUI::~nmr_RegistrationUI()
{
  delete [] d_scaledProjImFromScaledTopoIm;
}

void nmr_RegistrationUI::setupCallbacks() 
{
    d_autoAlignRequested.addCallback
         (handle_autoAlignRequested_change, (void *)this);
    d_scaleX.addCallback(handle_transformationParameter_change, this);
    d_scaleY.addCallback(handle_transformationParameter_change, this);
    d_translateX.addCallback(handle_transformationParameter_change, this);
    d_translateY.addCallback(handle_transformationParameter_change, this);
	d_translateZ.addCallback(handle_transformationParameter_change, this);
    d_rotate2D_Z.addCallback(handle_transformationParameter_change, this);
    d_rotate3D_X.addCallback(handle_transformationParameter_change, this);
    d_rotate3D_Z.addCallback(handle_transformationParameter_change, this);
    d_shearZ.addCallback(handle_transformationParameter_change, this);
	d_saveRegistrationMarkers.addCallback(handle_saveRegistrationMarkers_change, (void *)this); //new
	d_loadRegistrationMarkers.addCallback(handle_loadRegistrationMarkers_change, (void *)this); //new
	d_saveReport.addCallback(handle_saveReport_change, (void *)this); //new
	d_runRansac.addCallback(handle_runRansac_change, (void *)this); //new
    d_calculatePoints.addCallback(handle_calculatePoints_change, (void *)this); //new
	d_drawTopographyPoints.addCallback(handle_drawTopographyPoints_change, (void *)this); //new
	d_saveTopographyPoints.addCallback(handle_saveTopographyPoints_change, (void *)this); //new
	d_drawProjectionPoints.addCallback(handle_drawProjectionPoints_change, (void *)this); //new
	d_saveProjectionPoints.addCallback(handle_saveProjectionPoints_change, (void *)this); //new
	d_runDrawRansac.addCallback(handle_runDrawRansac_change, (void *)this); //new

    d_registrationEnabled.addCallback
         (handle_registrationEnabled_change, (void *)this);
    d_textureDisplayEnabled.addCallback
         (handle_textureDisplayEnabled_change, (void *)this);
    d_textureAlpha.addCallback
         (handle_textureAlpha_change, (void *)this);
    d_newResampleImageName.addCallback
         (handle_resampleImageName_change, (void *)this);
    d_newResamplePlaneName.addCallback
         (handle_resamplePlaneName_change, (void *)this);

    d_registrationImageName3D.addCallback
       (handle_registrationImage3D_change, (void *)this);
	d_refresh3D.addCallback
		(handle_refresh3D_change, (void *)this);
	d_fiducialSpotTracker3D.addCallback
		(handle_fiducialSpotTracker3D_change, (void *) this);
	d_optimizeSpotTrackerRadius3D.addCallback
		(handle_optimizeSpotTrackerRadius3D_change, (void *) this);
	d_spotTrackerRadius3D.addCallback
		(handle_spotTrackerRadius3D_change, (void *) this);
	d_spotTrackerRadiusAccuracy3D.addCallback
		(handle_spotTrackerRadiusAccuracy3D_change, (void *) this);
	d_spotTrackerPixelAccuracy3D.addCallback
		(handle_spotTrackerPixelAccuracy3D_change, (void *) this);
    d_registrationImageName2D.addCallback
       (handle_registrationImage2D_change, (void *)this);
	d_refresh2D.addCallback
		(handle_refresh2D_change, (void *)this);
	d_fiducialSpotTracker2D.addCallback
		(handle_fiducialSpotTracker2D_change, (void *)this);
	d_optimizeSpotTrackerRadius2D.addCallback
		(handle_optimizeSpotTrackerRadius2D_change, (void *) this);
	d_spotTrackerRadius2D.addCallback
		(handle_spotTrackerRadius2D_change, (void *) this);
	d_spotTrackerRadiusAccuracy2D.addCallback
		(handle_spotTrackerRadiusAccuracy2D_change, (void *) this);
	d_spotTrackerPixelAccuracy2D.addCallback
		(handle_spotTrackerPixelAccuracy2D_change, (void *) this);

	d_flipProjectionImageInX.addCallback
       (handle_flipProjectionImageInX_change, (void *)this);

    d_registrationColorMap3D.addCallback
       (handle_registrationColorMap3D_change, (void *)this);
    d_registrationColorMap2D.addCallback
       (handle_registrationColorMap2D_change, (void *)this);

    d_3DImageCMap->addMinMaxCallback(handle_registrationMinMax3D_change, this);
    d_2DImageCMap->addMinMaxCallback(handle_registrationMinMax2D_change, this);
    d_transformationSource.addCallback
       (handle_textureTransformMode_change, (void *)this);

}

void nmr_RegistrationUI::teardownCallbacks() 
{
    d_autoAlignRequested.removeCallback
         (handle_autoAlignRequested_change, (void *)this);
    d_scaleX.removeCallback(handle_transformationParameter_change, this);
    d_scaleY.removeCallback(handle_transformationParameter_change, this);
    d_translateX.removeCallback(handle_transformationParameter_change, this);
    d_translateY.removeCallback(handle_transformationParameter_change, this);
	d_translateZ.removeCallback(handle_transformationParameter_change, this);
    d_rotate2D_Z.removeCallback(handle_transformationParameter_change, this);
    d_rotate3D_X.removeCallback(handle_transformationParameter_change, this);
    d_rotate3D_Z.removeCallback(handle_transformationParameter_change, this);
    d_shearZ.removeCallback(handle_transformationParameter_change, this);
	d_saveRegistrationMarkers.removeCallback(handle_saveRegistrationMarkers_change, (void *)this); //new
	d_loadRegistrationMarkers.removeCallback(handle_loadRegistrationMarkers_change, (void *)this); //new		
	d_saveReport.removeCallback(handle_saveReport_change, (void *)this); //new
	d_runRansac.removeCallback(handle_runRansac_change, (void *)this); //new
    d_calculatePoints.removeCallback(handle_calculatePoints_change, (void *)this); //new
	d_drawTopographyPoints.removeCallback(handle_drawTopographyPoints_change, (void *)this); //new
	d_saveTopographyPoints.removeCallback(handle_saveTopographyPoints_change, (void *)this); //new
	d_drawProjectionPoints.removeCallback(handle_drawProjectionPoints_change, (void *)this); //new
	d_saveProjectionPoints.removeCallback(handle_saveProjectionPoints_change, (void *)this); //new
	d_runDrawRansac.removeCallback(handle_runDrawRansac_change, (void *)this); //new

    d_registrationEnabled.removeCallback
         (handle_registrationEnabled_change, (void *)this);
    d_textureDisplayEnabled.removeCallback
         (handle_textureDisplayEnabled_change, (void *)this);
    d_textureAlpha.removeCallback
         (handle_textureAlpha_change, (void *)this);
    d_newResampleImageName.removeCallback
         (handle_resampleImageName_change, (void *)this);
    d_newResamplePlaneName.removeCallback
         (handle_resamplePlaneName_change, (void *)this);

    d_registrationImageName3D.removeCallback
       (handle_registrationImage3D_change, (void *)this);
	d_refresh3D.removeCallback
		(handle_refresh3D_change, (void *)this);
	d_fiducialSpotTracker3D.removeCallback
		(handle_fiducialSpotTracker3D_change, (void *)this);
    d_optimizeSpotTrackerRadius3D.removeCallback
        (handle_optimizeSpotTrackerRadius3D_change, (void *)this);
    d_spotTrackerRadius3D.removeCallback
        (handle_spotTrackerRadius3D_change, (void *)this);
    d_spotTrackerRadiusAccuracy3D.removeCallback
        (handle_spotTrackerRadiusAccuracy3D_change, (void *)this);
    d_spotTrackerPixelAccuracy3D.removeCallback
		(handle_spotTrackerPixelAccuracy3D_change, (void *)this);
    d_registrationImageName2D.removeCallback
       (handle_registrationImage2D_change, (void *)this);
	d_refresh2D.removeCallback
		(handle_refresh2D_change, (void *)this);
	d_fiducialSpotTracker2D.removeCallback
		(handle_fiducialSpotTracker2D_change, (void *)this);
    d_optimizeSpotTrackerRadius2D.removeCallback
		(handle_optimizeSpotTrackerRadius2D_change, (void *) this);
	d_spotTrackerRadius2D.removeCallback
		(handle_spotTrackerRadius2D_change, (void *) this);
	d_spotTrackerRadiusAccuracy2D.removeCallback
		(handle_spotTrackerRadiusAccuracy2D_change, (void *) this);
	d_spotTrackerPixelAccuracy2D.removeCallback
		(handle_spotTrackerPixelAccuracy2D_change, (void *) this);

    d_registrationColorMap3D.removeCallback
       (handle_registrationColorMap3D_change, (void *)this);
    d_registrationColorMap2D.removeCallback
       (handle_registrationColorMap2D_change, (void *)this);

    d_3DImageCMap->removeMinMaxCallback(handle_registrationMinMax3D_change, this);
    d_2DImageCMap->removeMinMaxCallback(handle_registrationMinMax2D_change, this);
    d_transformationSource.removeCallback
        (handle_textureTransformMode_change, (void *)this);
}

void nmr_RegistrationUI::changeDataset(nmb_ImageManager *dataset)
{
  d_dataset = dataset;
}

void nmr_RegistrationUI::handleRegistrationChange
          (const nmr_ProxyChangeHandlerData &info)
{

  switch(info.msg_type) {
    case NMR_IMAGE_PARAM:
      nmr_ImageType which_image;
      vrpn_int32 res_x, res_y;
      vrpn_float32 size_x, size_y;
      vrpn_bool flip_x, flip_y;
      d_aligner->getImageParameters(which_image, res_x, res_y, 
                                    size_x, size_y, flip_x, flip_y);
      break;
    case NMR_TRANSFORM_OPTION:
      nmr_TransformationType xform_type;
      d_aligner->getTransformationOptions(xform_type);
      break;
    case NMR_REG_RESULT:
      //printf("got transformation");
      /* *******************************************************************
         Store the raw result from the registration code and update graphics
         if necessary
       * *******************************************************************/
      double scaledProjImFromScaledTopoIm_matrix[16];
      vrpn_int32 whichTransform = 0;
      d_aligner->getRegistrationResult(whichTransform,
                                       scaledProjImFromScaledTopoIm_matrix);
      int displayedTransformIndex = getDisplayedTransformIndex();
      int sentTransformIndex =
                convertTransformSourceTypeToTransformIndex(whichTransform);
      d_scaledProjImFromScaledTopoIm[sentTransformIndex].setMatrix(
              scaledProjImFromScaledTopoIm_matrix);
     
      //printf(", type=%d\n", whichTransform);
      //d_scaledProjImFromScaledTopoIm[sentTransformIndex].print(); 
      if (displayedTransformIndex == sentTransformIndex) {
        // compose the new transformation with some others and send the
        // result off to the graphics code
        updateTextureTransform();
      }
      switch (whichTransform) {
        case NMR_MANUAL:
		  d_lastTransformTypeSent = NMR_MANUAL;
          setAutoAlignMode(NMR_AUTOALIGN_FROM_MANUAL);
          setTransformationSource(NMR_MANUAL);
          break;
        case NMR_DEFAULT:
		  d_lastTransformTypeSent = NMR_DEFAULT;
          //setAutoAlignMode(NMR_AUTOALIGN_FROM_DEFAULT);
          //setTransformationSource(NMR_DEFAULT);
          break;
        case NMR_AUTOMATIC:
		  d_lastTransformTypeSent = NMR_AUTOMATIC;
          setTransformationSource(NMR_AUTOMATIC);
          setAutoAlignMode(NMR_AUTOALIGN_FROM_AUTO);
          break;
      }
      break;
  }
}

void nmr_RegistrationUI::updateTextureTransform()
{
  if (!d_imageDisplay) return;

  // we know d_scaledProjImFromScaledTopoIm and we want to get
  // projImFromTopoWorld
  nmb_TransformMatrix44 projImFromTopoWorld;

  // first we lookup the images that this transformation applies to and
  // get from them the additional transformations we need to compose
  if (!d_dataset) return;

  nmb_Image *topographyImage = d_dataset->dataImages()->getImageByName
                      (d_registrationImageName3D.string());
  nmb_Image *projectionImage = d_dataset->dataImages()->getImageByName
                      (d_registrationImageName2D.string());
  if (!topographyImage || !projectionImage) {
     fprintf(stderr, "updateTextureTransform: can't find image\n");
     return;
  }
  nmb_TransformMatrix44 projImFromScaledProjIm, scaledTopoImFromTopoWorld;

  projectionImage->getScaledImageToImageTransform(
                                      projImFromScaledProjIm);

  topographyImage->getWorldToScaledImageTransform(
                                      scaledTopoImFromTopoWorld);

  /* ****************************************************************
     ProjImFromTopoWorld = ProjImFromScaledProjIm *
                           ScaledProjImFromScaledTopoImage *
                           ScaledTopoImageFromTopoWorld
   * ****************************************************************/
  projImFromTopoWorld = projImFromScaledProjIm;
  int displayedTransformIndex = getDisplayedTransformIndex();
  projImFromTopoWorld.compose(
                   d_scaledProjImFromScaledTopoIm[displayedTransformIndex]);
  projImFromTopoWorld.compose(scaledTopoImFromTopoWorld);

  double projImFromTopoWorld_matrix[16];
  // send the transformation to the graphics code
  projImFromTopoWorld.getMatrix(projImFromTopoWorld_matrix);

  bool transformIsSelfReferential =
		(strcmp(d_registrationImageName2D, d_registrationImageName3D) == 0);
  d_imageDisplay->updateDisplayTransform(projectionImage, 
                                         projImFromTopoWorld_matrix,
										 transformIsSelfReferential);
}

int nmr_RegistrationUI::getDisplayedTransformIndex()
{
  int i;
  for (i = 0; i < s_numTransformationSources; i++) {
    if (strcmp(s_transformationSourceNames[i],
                   d_transformationSource.string()) == 0) {
       return i;
    }
  }
  fprintf(stderr, "getCurrentTransformIndex: Error, not found\n");
  return 0;
}

int nmr_RegistrationUI::convertTransformSourceTypeToTransformIndex(int type)
{
  int i;
  for (i = 0; i < s_numTransformationSources; i++) {
    if (type == s_transformationSources[i]) {
       return i;
    }
  }
  fprintf(stderr, "convertTransformSourceTypeToTransformIndex: "
                  "Error, not found\n");
  return 0;
}

// static
void nmr_RegistrationUI::handle_registrationChange
          (void *ud, const nmr_ProxyChangeHandlerData &info)
{
    nmr_RegistrationUI *me = (nmr_RegistrationUI *)ud;
    me->handleRegistrationChange(info);
}

// static 
void nmr_RegistrationUI::handle_resampleImageName_change(const char *name, 
                                                         void *ud)
{
    nmr_RegistrationUI *me = (nmr_RegistrationUI *)ud;
    me->createResampleImage(name);

}

// static 
void nmr_RegistrationUI::handle_resamplePlaneName_change(const char *name, 
                                                         void *ud)
{
    nmr_RegistrationUI *me = (nmr_RegistrationUI *)ud;
    me->createResamplePlane(name);

}

// static
void nmr_RegistrationUI::handle_registrationImage3D_change(const char *name,
                                                           void *ud)
{
    //printf("nmr_RegistrationUI::handle_registrationImage3D_change\n");
    nmr_RegistrationUI *me = (nmr_RegistrationUI *)ud;

    // Don't do anything if reg window is not open - instead we will
    // be called explicitly in handle_registrationEnabled_change below. 
    if (!me->d_dataset || !me->d_registrationEnabled) return;
    nmb_Image *im = me->d_dataset->dataImages()->getImageByName(name);
    if (!im) {
        fprintf(stderr, "nmr_RegistrationUI::image not found: %s\n", name);
        return;
    }
    // If different, send changes. 
    if ( me->d_last3DImage != im) {

        // send image off to the proxy
        me->d_aligner->setImage(NMR_SOURCE, im, me->d_flipXreference, 
			me->d_flipYreference);
        // We have a choice, and I'm not sure which is right. Either
        // Set the new image to use the existing colormap params:
        double dmin,dmax,cmin,cmax;
        me->d_3DImageCMap->getDataColorMinMax(&dmin, &dmax, &cmin, &cmax);
        me->d_3DImageCMap->setColorMinMaxLimit(0,1);
        me->d_aligner->setColorMinMax(NMR_SOURCE, dmin, dmax, cmin, cmax);
        // Or reset the colormap params to their default:
        //me->d_3DImageCMap->setColorMinMaxLimit(0,1);
        me->d_last3DImage = im;
        if (me->d_last2DImage && me->d_lastTransformTypeSent != NMR_DEFAULT) {
          //me->updateTextureTransform();
        }
    }
}

//static 
void nmr_RegistrationUI::handle_refresh3D_change(vrpn_int32 value, void *ud)
{
	nmr_RegistrationUI *me = (nmr_RegistrationUI *)ud;
    if (value && me->d_last3DImage ) {

        // send image off to the proxy
        me->d_aligner->setImage(NMR_SOURCE, me->d_last3DImage, 
			me->d_flipXreference, 
			me->d_flipYreference);
	}
}

// static
void nmr_RegistrationUI::handle_fiducialSpotTracker3D_change(const char *name, void *ud)
{
    nmr_RegistrationUI *me = (nmr_RegistrationUI *)ud;
    nmr_FiducialSpotTracker tracker = me->getFiducialSpotTrackerByName(name);
    me->d_aligner->setFiducialSpotTracker(NMR_SOURCE, tracker);
}

// static
void nmr_RegistrationUI::handle_optimizeSpotTrackerRadius3D_change(vrpn_int32 enable, void *ud)
{
    nmr_RegistrationUI *me = (nmr_RegistrationUI *)ud;
    me->d_aligner->setOptimizeSpotTrackerRadius(NMR_SOURCE, enable ? vrpn_TRUE : vrpn_FALSE);
}

// static
void nmr_RegistrationUI::handle_spotTrackerRadius3D_change(vrpn_float64 radius, void *ud)
{
    nmr_RegistrationUI *me = (nmr_RegistrationUI *)ud;
    me->d_aligner->setSpotTrackerRadius(NMR_SOURCE, radius);
}

// static
void nmr_RegistrationUI::handle_spotTrackerRadiusAccuracy3D_change(vrpn_float64 accuracy, void *ud)
{
    nmr_RegistrationUI *me = (nmr_RegistrationUI *)ud;
    me->d_aligner->setSpotTrackerRadiusAccuracy(NMR_SOURCE, accuracy);
}

// static
void nmr_RegistrationUI::handle_spotTrackerPixelAccuracy3D_change(vrpn_float64 accuracy, void *ud)
{
    nmr_RegistrationUI *me = (nmr_RegistrationUI *)ud;
    me->d_aligner->setSpotTrackerPixelAccuracy(NMR_SOURCE, accuracy);
}

// static
void nmr_RegistrationUI::handle_fiducialSpotTracker2D_change(const char *name, void *ud)
{
    nmr_RegistrationUI *me = (nmr_RegistrationUI *)ud;
    nmr_FiducialSpotTracker tracker = me->getFiducialSpotTrackerByName(name);
    me->d_aligner->setFiducialSpotTracker(NMR_TARGET, tracker);
}

// static
void nmr_RegistrationUI::handle_optimizeSpotTrackerRadius2D_change(vrpn_int32 enable, void *ud)
{
    nmr_RegistrationUI *me = (nmr_RegistrationUI *)ud;
    me->d_aligner->setOptimizeSpotTrackerRadius(NMR_TARGET, enable ? vrpn_TRUE : vrpn_FALSE);
}

// static
void nmr_RegistrationUI::handle_spotTrackerRadius2D_change(vrpn_float64 radius, void *ud)
{
    nmr_RegistrationUI *me = (nmr_RegistrationUI *)ud;
    me->d_aligner->setSpotTrackerRadius(NMR_TARGET, radius);
}

// static
void nmr_RegistrationUI::handle_spotTrackerRadiusAccuracy2D_change(vrpn_float64 accuracy, void *ud)
{
    nmr_RegistrationUI *me = (nmr_RegistrationUI *)ud;
    me->d_aligner->setSpotTrackerRadiusAccuracy(NMR_TARGET, accuracy);
}

// static
void nmr_RegistrationUI::handle_spotTrackerPixelAccuracy2D_change(vrpn_float64 accuracy, void *ud)
{
    nmr_RegistrationUI *me = (nmr_RegistrationUI *)ud;
    me->d_aligner->setSpotTrackerPixelAccuracy(NMR_TARGET, accuracy);
}

// static
void nmr_RegistrationUI::handle_registrationImage2D_change(const char *name,
                                                           void *ud)
{
    //printf("nmr_RegistrationUI::handle_registrationImage2D_change\n");
    nmr_RegistrationUI *me = (nmr_RegistrationUI *)ud;

    // Don't do anything if reg window is not open - instead we will
    // be called explicitly in handle_registrationEnabled_change below. 
    if (!me->d_dataset || !me->d_registrationEnabled) return;
    nmb_Image *im = me->d_dataset->dataImages()->getImageByName(name);
    if (!im) {
        fprintf(stderr, "nmr_RegistrationUI::image not found: %s\n", name);
        return;
    }

    // If different, send changes. 
    if ( me->d_last2DImage != im) {
        // send image off to the proxy
        me->d_aligner->setImage(NMR_TARGET, im, me->d_flipProjectionImageInX, 
			me->d_flipYadjustable);
        
        // We have a choice, and I'm not sure which is right. Either
        // Set the new image to use the existing colormap params:
        double dmin,dmax,cmin,cmax;
        me->d_2DImageCMap->getDataColorMinMax(&dmin, &dmax, &cmin, &cmax);

		me->d_colormap2DCallbackDisabled = true;
        me->d_2DImageCMap->setColorMinMaxLimit(0,1);
		me->d_colormap2DCallbackDisabled = false;
		me->handle_registrationMinMax2D_change(0, me);

        me->d_aligner->setColorMinMax(NMR_TARGET, dmin, dmax, cmin, cmax);
        // Or reset the colormap params to their default:
        //me->d_2DImageCMap->setColorMinMaxLimit(0,1);
 
        me->d_last2DImage = im;
       
        if (me->d_imageDisplay && me->d_textureDisplayEnabled) {
          // set up texture in graphics
          if (im) {
            me->d_imageDisplay->setDisplayColorMap(im,
                  me->d_2DImageCMap->getColorMapName(), "");
            me->d_imageDisplay->setDisplayColorMapRange(im,
                                                          dmin, dmax, 
                                                          cmin,cmax);
            me->d_imageDisplay->addImageToDisplay(im);
            me->d_imageDisplay->updateImage(im);
          }

          if (me->d_last3DImage && me->d_lastTransformTypeSent != NMR_DEFAULT) {
            me->updateTextureTransform();
          }
        }
    }
}

//static 
void nmr_RegistrationUI::handle_refresh2D_change(vrpn_int32 value, void *ud)
{
	nmr_RegistrationUI *me = (nmr_RegistrationUI *)ud;
	if (value && me->d_last2DImage ) {
        // send image off to the proxy
        me->d_aligner->setImage(NMR_TARGET, me->d_last2DImage, 
			me->d_flipProjectionImageInX, 
			me->d_flipYadjustable);
	}
}

void nmr_RegistrationUI::handle_flipProjectionImageInX_change(vrpn_int32 value, void *ud)
{
	nmr_RegistrationUI *me = (nmr_RegistrationUI *)ud;
	if (me->d_last2DImage) {
		me->d_aligner->setImage(NMR_TARGET, me->d_last2DImage, me->d_flipProjectionImageInX, 
			me->d_flipYadjustable);
	}
}

// static
void nmr_RegistrationUI::handle_registrationColorMap3D_change(const char *name,
                                                           void *ud)
{
    nmr_RegistrationUI *me = (nmr_RegistrationUI *)ud;
    nmb_ColorMap * cmap =  me->d_3DImageCMap->currentColorMap();
    if (!cmap) {
        fprintf(stderr, "nmr_RegistrationUI::colormap not found: %s\n", name);
        return;
    }
    // send changes off to the proxy
    me->d_aligner->setColorMap(NMR_SOURCE, cmap);
}

// static
void nmr_RegistrationUI::handle_registrationColorMap2D_change(const char *name,
                                                           void *ud)
{
    nmr_RegistrationUI *me = (nmr_RegistrationUI *)ud;
    nmb_ColorMap * cmap =  me->d_2DImageCMap->currentColorMap();
    if (!cmap) {
        fprintf(stderr, "nmr_RegistrationUI::colormap not found: %s\n", name);
        return;
    }
    // send changes off to the proxy
    me->d_aligner->setColorMap(NMR_TARGET, cmap);

    if (!me->d_dataset) return;
    nmb_Image *im = me->d_dataset->dataImages()->getImageByName(
                                me->d_registrationImageName2D.string());

    if (me->d_imageDisplay) {
      me->d_imageDisplay->setDisplayColorMap(im, 
          me->d_2DImageCMap->getColorMapName(), "");
      if(im) me->d_imageDisplay->updateImage(im);
    }

}

//static 
void nmr_RegistrationUI::handle_registrationMinMax3D_change(vrpn_float64, void *ud) {
    nmr_RegistrationUI *me = (nmr_RegistrationUI *)ud;
    double dmin,dmax,cmin,cmax;
    me->d_3DImageCMap->getDataColorMinMax(&dmin, &dmax, &cmin, &cmax);
    // send changes off to the proxy
    me->d_aligner->setColorMinMax(NMR_SOURCE, dmin, dmax, cmin, cmax);

}

//static 
void nmr_RegistrationUI::handle_registrationMinMax2D_change(vrpn_float64, void *ud) {
    nmr_RegistrationUI *me = (nmr_RegistrationUI *)ud;
	if (me->d_colormap2DCallbackDisabled) {
		return;
	}
    double dmin,dmax,cmin,cmax;
     me->d_2DImageCMap->getDataColorMinMax(&dmin, &dmax, &cmin, &cmax);
    // send changes off to the proxy
    me->d_aligner->setColorMinMax(NMR_TARGET, dmin, dmax, cmin, cmax);

    if (!me->d_dataset) return;
    nmb_Image *im = me->d_dataset->dataImages()->getImageByName(
                                me->d_registrationImageName2D.string());

    if (me->d_imageDisplay) {
      me->d_imageDisplay->setDisplayColorMapRange(im, dmin, dmax, cmin, cmax);
	  if(im) me->d_imageDisplay->updateImage(im);
    }
}

// static
void nmr_RegistrationUI::handle_textureTransformMode_change(const char *name,
                                                           void *ud)
{
  nmr_RegistrationUI *me = (nmr_RegistrationUI *)ud;

  if (me->d_textureDisplayEnabled) {
    me->updateTextureTransform();
  }
}

// static
void nmr_RegistrationUI::handle_textureDisplayEnabled_change(
      vrpn_int32 value, void *ud)
{
    nmr_RegistrationUI *me = (nmr_RegistrationUI *)ud;
    if (!(me->d_imageDisplay)) return;

    if (!(me->d_dataset)) return;

    nmb_Image *im = me->d_dataset->dataImages()->getImageByName(
                                me->d_registrationImageName2D.string());
    if (!im) return;

    if (value) {
      // set up texture in graphics
      double dmin=0,dmax=1,cmin=0,cmax=1;
      me->d_imageDisplay->setDisplayColorMap(im,
         me->d_2DImageCMap->getColorMapName(), "");
      me->d_imageDisplay->setDisplayColorMapRange(im, dmin, dmax,
                                                cmin,cmax);
      me->d_imageDisplay->addImageToDisplay(im);
      me->updateTextureTransform();
      me->d_imageDisplay->updateImage(im);
    } 
    else {
      me->d_imageDisplay->removeImageFromDisplay(im);
    }
}

// static
void nmr_RegistrationUI::handle_textureAlpha_change(
      vrpn_float64 alpha, void *ud)
{
    nmr_RegistrationUI *me = (nmr_RegistrationUI *)ud;

    if (!(me->d_imageDisplay)) return;

    if (!(me->d_dataset)) return;

    nmb_Image *im = me->d_dataset->dataImages()->getImageByName(
                                me->d_registrationImageName2D.string());
    if (!im) return;

    me->d_imageDisplay->updateColorMapTextureAlpha(alpha);
    
    me->d_imageDisplay->updateImage(im);
}

void nmr_RegistrationUI::autoAlignImages()
{
  vrpn_float32 stddev = atof(d_resolutionLevel.string());
  vrpn_int32 levelIndex;
  for (levelIndex = 0; levelIndex < d_numResolutionLevels; levelIndex++) {
    if (stddev == d_stddev[levelIndex]) break;
  }
  if (levelIndex == d_numResolutionLevels) {
    fprintf(stderr, "Error: autoAlignImages: can't find resolution\n");
    return; 
  }

  int modeIndex;
  for (modeIndex = 0; modeIndex < s_numAutoAlignModes; modeIndex++) {
    if (strcmp(d_autoAlignMode.string(), 
               s_autoAlignModeNames[modeIndex]) == 0) {
      break;
    }
  }
  if (modeIndex == s_numAutoAlignModes) {
    fprintf(stderr, "Error: autoAlignImages: can't find mode\n");
    return;
  }
  nmr_AutoAlignMode mode = s_autoAlignModes[modeIndex];

  d_aligner->setIterationLimit((vrpn_int32)d_numIterations);
  d_aligner->setStepSize((vrpn_float32)d_stepSize);
  d_aligner->setCurrentResolution(levelIndex);
  d_aligner->autoAlignImages(mode);
}

void nmr_RegistrationUI::setTransformationSource(nmr_RegistrationType source) 
{
  int i;
  for (i = 0; i < s_numTransformationSources; i++) {
    if (source == s_transformationSources[i]) {
      d_transformationSource = s_transformationSourceNames[i];
    }
  }
}

void nmr_RegistrationUI::setAutoAlignMode(nmr_AutoAlignMode mode)
{
  int i;
  for (i = 0; i < s_numAutoAlignModes; i++) {
    if (mode == s_autoAlignModes[i]) {
      d_autoAlignMode = s_autoAlignModeNames[i];
    }
  }
}

nmr_FiducialSpotTracker nmr_RegistrationUI::getFiducialSpotTrackerByName(const char *trackerName) {
    if (!strcmp(trackerName, "None"))
        return NMR_NO_TRACKER;
    else if (!strcmp(trackerName, "Local Max"))
        return NMR_LOCAL_MAX_TRACKER;
    else if (!strcmp(trackerName, "Cone"))
        return NMR_CONE_TRACKER;
    else if (!strcmp(trackerName, "Disk"))
        return NMR_DISK_TRACKER;
    else if (!strcmp(trackerName, "FIONA"))
        return NMR_FIONA_TRACKER;
    else if (!strcmp(trackerName, "Symmetric"))
        return NMR_SYMMETRIC_TRACKER;
    else {
        fprintf(stderr, 
            "getFiducialSpotTrackerByName encountered unknown tracker name '%s'. Defaulting to Cone Tracker\n", 
            trackerName);
        return NMR_CONE_TRACKER;
    }
}

// static
void nmr_RegistrationUI::handle_autoAlignRequested_change(
      vrpn_int32 value, void *ud)
{
    nmr_RegistrationUI *me = (nmr_RegistrationUI *)ud;
    if (value) {
      me->autoAlignImages();
    
      static vrpn_bool first_time = vrpn_TRUE;
      if (first_time) {
        first_time = vrpn_FALSE;
        // some debugging code that inserts the blurred images used for
        // alignment into the image list so you can easily view them:
//         if (!me->d_dataset) return;
//         nmb_Image *im_3D = me->d_dataset->dataImages()->getImageByName
//                             (me->d_registrationImageName3D.string());
//         nmb_Image *im_2D = me->d_dataset->dataImages()->getImageByName
//                             (me->d_registrationImageName2D.string());
//         nmr_AlignerMI aligner;
//         aligner.initImages(im_3D, im_2D, 
//                            me->d_numResolutionLevels, me->d_stddev, 
//                            NULL, me->d_dataset->dataImages());
      }
    }
}

// static
void nmr_RegistrationUI::handle_transformationParameter_change(
      vrpn_float64 /*value*/, void *ud)
{
  nmr_RegistrationUI *me = (nmr_RegistrationUI *)ud;
  me->sendTransformationParameters();
}

void nmr_RegistrationUI::handle_saveRegistrationMarkers_change(vrpn_int32 value, void *ud) //(vrpn_float64 /*value*/, void *ud)
{
	// If it is static
//	nmr_RegistrationUI *me = dynamic_cast<nmr_RegistrationUI *>(ud);
	nmr_RegistrationUI *me = static_cast<nmr_RegistrationUI *>(ud);
	// Check me for NULL

	Correspondence	c;
	int	src, tgt;
//	me->d_aligner->d_local_impl->d_alignerUI->getCorrespondence(c, src, tgt);
	me->d_aligner->get_d_local_impl()->get_d_alignerUI()->getCorrespondence(c, src, tgt);
//	me->d_aligner->get_d_local_impl()->get_d_alignerUI()->get_d_ce();

	corr_point_t point_topography;
	corr_point_t point_projection;
	unsigned i;

	FILE * pFile;
	pFile = fopen ("output/registration_markers.txt","w");
 
	//number of markers in an image
	fprintf(pFile,"%d\n", c.numPoints());

/*
	vector<float> list_x;
	vector<float> list_y;

	for(int a = 0; a < c.numPoints(); a++)
	{
		int isthere = 0;
		for(int b = 0; b < list_x.size(); b++)
		{
			if(imgvalues_x[a] == list_x[b] && imgvalues_y[a] == list_y[b])
			{
				isthere++;
			}
		}
		if(isthere == 0)
		{
			list_x.push_back(imgvalues_x[a]);
			list_y.push_back(imgvalues_y[a]);
		}
	}
	for(int i = 0; i < list_x.size(); i++)
	{
		fprintf(pFile,"%g %g ", list_x[i], list_y[i]);
	}
*/

	// Topography Image Point
	for (i = 0; i < c.numPoints(); i++) {
		c.getPoint(src, i, &point_topography);
		fprintf(pFile,"%g %g ", point_topography.x, point_topography.y);
	}
	fprintf(pFile,"\n");

	// Projection Image Point
	for (i = 0; i < c.numPoints(); i++) {
		c.getPoint(tgt, i, &point_projection);
		fprintf(pFile,"%g %g ", point_projection.x, point_projection.y);
	}
	fprintf(pFile,"\n");

	fclose (pFile);

} //new

void nmr_RegistrationUI::handle_loadRegistrationMarkers_change(vrpn_int32 value, void *ud) //(vrpn_float64 /*value*/, void *ud)
{
	float x,y;

	FILE * pFile;
	pFile = fopen ("output/registration_markers.txt","r");

	if (pFile==NULL) 
	{
		perror ("Error opening file");
	}
	else
	{
		rewind (pFile);
		int num;
		fscanf (pFile, "%d", &num);

		if(num <= NMR_MAX_FIDUCIAL)
		{
			vrpn_float32 x_src[NMR_MAX_FIDUCIAL], y_src[NMR_MAX_FIDUCIAL], z_src[NMR_MAX_FIDUCIAL],
				x_tgt[NMR_MAX_FIDUCIAL], y_tgt[NMR_MAX_FIDUCIAL], z_tgt[NMR_MAX_FIDUCIAL];

			vrpn_int32 replace = 1;


			printf ("%d ",num);
			printf ("Topography: ");
			for(int i = 0; i<num; i++)
			{
				fscanf (pFile, "%f %f", &x, &y);
				x_src[i] = x;
				y_src[i] = y;
				z_src[i] = 0;
		//		printf ( "%f %f ", x, y);
				if( i == 0 || i == (num-1))
				{
					printf ("source %d %f %f\n", i, x_src[i], y_src[i]);
				}
			}
			printf ("\n");

			printf ("Projection: ");
			for(int i = 0; i<num; i++)
			{
				fscanf (pFile, "%f %f", &x, &y);
				x_tgt[i] = x;
				y_tgt[i] = y;
				z_tgt[i] = 0;
		//		printf ( "%f %f ", x, y);
				if( i == 0 || i == (num-1))
				{
					printf ("target %d %f %f\n", i, x_tgt[i], y_tgt[i]);
				}
			}
			printf ("\n");

			nmr_RegistrationUI *me = static_cast<nmr_RegistrationUI *>(ud);
			me->d_aligner->get_d_local_impl()->get_d_alignerUI()->setFiducial(replace,num,x_src,y_src,z_src,x_tgt,y_tgt,z_tgt);
		}
		else
		{
			printf ("Error: Number of markers in the file is bigger than NMR_MAX_FIDUCIAL\n");
		}
		fclose (pFile);
	}
} //new

void nmr_RegistrationUI::handle_saveReport_change(vrpn_int32 value, void *ud) //(vrpn_float64 /*value*/, void *ud)
{
	nmr_RegistrationUI *me = static_cast<nmr_RegistrationUI *>(ud);

	FILE * pFile;
	pFile = fopen ("output/methods section.txt","w");
 
	fprintf(pFile,"rms error between images: %f\n", me->d_aligner->get_d_local_impl()->get_rmsError());

	fclose (pFile);

} //new

void matrixMultiplication(float resultMatrix[6][6], float leftMatrix[6][6], float rightMatrix[6][6])
{
    for ( int r = 0; r < 6; r++ )
    {
	   for ( int c = 0; c < 6; c++ )
	   {
		  for ( int i = 0; i < 6; i++ )
		  {
			  resultMatrix[r][c] +=  leftMatrix[r][i] * rightMatrix[i][c];
//			  printf("%f %f %f \n",resultMatrix[r][c],leftMatrix[r][i],rightMatrix[i][c]);
		  }
	  }
	}
}

float detrm(float a[6][6],float k)
{
        float s=1,det=0;
		float b[6][6]={0};
        int i,j,m,n,c;
        if(k==1)
        {
                return(a[0][0]);
        }
        else
        {
                det=0;
                for(c=0;c<k;c++)
                {
                        m=0;
                        n=0;
                        for(i=0;i<k;i++)
                        {
                                for(j=0;j<k;j++)
                                {
                                        b[i][j]=0;
                                        if(i!=0&&j!=c)
                                        {
                                                b[m][n]=a[i][j];
                                                if(n<(k-2))
                                                        n++;
                                                else
                                                {
                                                        n=0;
                                                        m++;
                                                }
                                        }
                                }
                        }
                        det=det+s*(a[0][c]*detrm(b,k-1));
                        s=-1*s;
                }
        }
        return det;
}

void trans(float inverseMatrix[6][6],float num[6][6],float fac[6][6],float r)
{
        int i,j;
		float b[6][6] = {0};
		float inv[6][6] = {0};
		float d;
        for(i=0;i<r;i++)
        {
                for(j=0;j<r;j++)
                {
                        b[i][j]=fac[j][i];
                }
        }
        d=detrm(num,r);
        inv[i][j]=0;

        for(i=0;i<r;i++)
        {
                for(j=0;j<r;j++)
                {
                        if (d==1 || b[i][j]==0)
                                inverseMatrix[i][j] = b[i][j];
                        else
                                inverseMatrix[i][j] = b[i][j]/d;
                }
        }
}

void cofact(float inverseMatrix[6][6], float num[6][6],float f)
{
        float b[6][6]={0};
		float fac[6][6]={0};
        int p,q,m,n,i,j;
        for(q=0;q<f;q++)
        {
                for(p=0;p<f;p++)
                {
                        m=0;
                        n=0;
                        for(i=0;i<f;i++)
                        {
                                for(j=0;j<f;j++)
                                {
                                        b[i][j]=0;
                                        if(i!=q&&j!=p)
                                        {
                                                b[m][n]=num[i][j];
                                                if(n<(f-2))
                                                        n++;
                                                else
                                                {
                                                        n=0;
                                                        m++;
                                                }
                                        }
                                }
                        }
                        fac[q][p]=pow(-1.0,q+p)*detrm(b,f-1);
                }
        }
        trans(inverseMatrix,num,fac,f);
}


void nmr_RegistrationUI::handle_calculatePoints_change(vrpn_int32 value, void *ud) //(vrpn_float64 /*value*/, void *ud)
{

	nmr_RegistrationUI *me = static_cast<nmr_RegistrationUI *>(ud);
	vector< vector< vector <float> > > rawRansac = me->d_aligner->get_d_local_impl()->get_d_alignerUI()->readPixels();

#if(0)
	float x,y;

	FILE * pFile;
	pFile = fopen ("output/ransac_markers.txt","r");

	if (pFile==NULL) 
	{
		perror ("Error opening file");
	}
	else
	{
		rewind (pFile);
		int num;
		fscanf (pFile, "%d", &num);

		if(num <= NMR_MAX_FIDUCIAL)
		{
			vrpn_float32 x_src[NMR_MAX_FIDUCIAL], y_src[NMR_MAX_FIDUCIAL], z_src[NMR_MAX_FIDUCIAL],
				x_tgt[NMR_MAX_FIDUCIAL], y_tgt[NMR_MAX_FIDUCIAL], z_tgt[NMR_MAX_FIDUCIAL];

			vrpn_int32 replace = 1;


			printf ("%d ",num);
			printf ("Topography: ");
			for(int i = 0; i<num; i++)
			{
				fscanf (pFile, "%f %f", &x, &y);
				x_src[i] = x;
				y_src[i] = y;
				z_src[i] = 0;
		//		printf ( "%f %f ", x, y);
				if( i == 0 || i == (num-1))
				{
					printf ("source %d %f %f\n", i, x_src[i], y_src[i]);
				}
			}
			printf ("\n");

			printf ("Projection: ");
			for(int i = 0; i<num; i++)
			{
				fscanf (pFile, "%f %f", &x, &y);
				x_tgt[i] = x;
				y_tgt[i] = y;
				z_tgt[i] = 0;
		//		printf ( "%f %f ", x, y);
				if( i == 0 || i == (num-1))
				{
					printf ("target %d %f %f\n", i, x_tgt[i], y_tgt[i]);
				}
			}
			printf ("\n");

			nmr_RegistrationUI *me = static_cast<nmr_RegistrationUI *>(ud);
			me->d_aligner->get_d_local_impl()->get_d_alignerUI()->setFiducial(replace,num,x_src,y_src,z_src,x_tgt,y_tgt,z_tgt);
		}
		else
		{
			printf ("Error: Number of markers in the file is bigger than NMR_MAX_FIDUCIAL\n");
		}
		fclose (pFile);
	}
#endif
} //new

/*ewfwef
{
	float x,y;

	FILE * pFile;
	pFile = fopen ("output/ransac_points_topography.txt","r");

	if (pFile==NULL) 
	{
		perror ("Error opening file");
	}
	else
	{
		rewind (pFile);
		int num;
		fscanf (pFile, "%d", &num);

		if(num <= NMR_MAX_FIDUCIAL)
		{
			vrpn_float32 x_src[NMR_MAX_FIDUCIAL], y_src[NMR_MAX_FIDUCIAL], z_src[NMR_MAX_FIDUCIAL],
				x_tgt[NMR_MAX_FIDUCIAL], y_tgt[NMR_MAX_FIDUCIAL], z_tgt[NMR_MAX_FIDUCIAL];

			vrpn_int32 replace = 1;

			vector< vector <float> > wh = me->d_aligner->get_d_local_impl()->get_d_alignerUI()->getWidthHeight();

			printf ("%d ",num);
			printf ("Topography: ");
			for(int i = 0; i<num; i++)
			{
				fscanf (pFile, "%f %f", &x, &y);
				x_src[i] = x/(wh[0][0]-1);
				y_src[i] = y/(wh[0][1]-1);
				z_src[i] = 0;
				if( i == 0 || i == (num-1))
				{
					printf ("source %d %f %f\n", i, x_src[i], y_src[i]);
				}
			}
			printf ("\n");

			nmr_RegistrationUI *me = static_cast<nmr_RegistrationUI *>(ud);
			me->d_aligner->get_d_local_impl()->get_d_alignerUI()->setFiducial(replace,num,x_src,y_src,z_src,x_tgt,y_tgt,z_tgt);

		}
		else
		{
			printf ("Error: Number of markers in the file is bigger than NMR_MAX_FIDUCIAL\n");
		}
		fclose (pFile);
	}
}*/

void nmr_RegistrationUI::handle_drawTopographyPoints_change(vrpn_int32 value, void *ud)
{
	float x,y;

	FILE * pFile;
	pFile = fopen ("output/ransac_points_topography.txt","r");

	if (pFile==NULL) 
	{
		perror ("Error opening file");
	}
	else
	{
		rewind (pFile);
		int width;
		int height;
		fscanf (pFile, "%d", &width);
		fscanf (pFile, "%d", &height);

		vrpn_float32 x_src[NMR_MAX_FIDUCIAL], y_src[NMR_MAX_FIDUCIAL], z_src[NMR_MAX_FIDUCIAL],
			x_tgt[NMR_MAX_FIDUCIAL], y_tgt[NMR_MAX_FIDUCIAL], z_tgt[NMR_MAX_FIDUCIAL];

		vrpn_int32 replace = 1;

		printf ("handle_drawTopographyPoints_change: ");

		int i = 0;
		for(; !feof(pFile); i++)
		{
			fscanf (pFile, "%f %f", &x, &y);
//			x_src[i] = x/(width-1);
//			y_src[i] = y/(height-1);
			x_src[i] = x;
			y_src[i] = y;
			z_src[i] = 0;

			x_tgt[i] = -1;
			y_tgt[i] = -1;
			z_tgt[i] = -1;

			if( i == 0)
			{
				printf ("source %d %f %f\n", i, x_src[i], y_src[i]);
			}
		}
		printf ("\n");

		printf ("LAST source %d %f %f\n", i-1, x_src[i-1], y_src[i-1]);

		if(i < NMR_MAX_FIDUCIAL)
		{
			nmr_RegistrationUI *me = static_cast<nmr_RegistrationUI *>(ud);
			me->d_aligner->get_d_local_impl()->get_d_alignerUI()->setFiducial(replace,(i-1),x_src,y_src,z_src,x_tgt,y_tgt,z_tgt);
		}
		else
		{
			printf ("Error: Number of markers in the file is bigger than NMR_MAX_FIDUCIAL\n");
		}
		fclose (pFile);
	}
}

void nmr_RegistrationUI::handle_saveTopographyPoints_change(vrpn_int32 value, void *ud)
{
	// If it is static
	nmr_RegistrationUI *me = static_cast<nmr_RegistrationUI *>(ud);
	// Check me for NULL

	Correspondence	c;
	int	src, tgt;
	me->d_aligner->get_d_local_impl()->get_d_alignerUI()->getCorrespondence(c, src, tgt);

	corr_point_t point_topography;
	unsigned i;

	FILE * pFile;
	pFile = fopen ("output/processed_ransac_topography.txt","w");
 
	//number of markers in an image
	fprintf(pFile,"%d\n", c.numPoints());

	// Topography Image Point
	for (i = 0; i < c.numPoints(); i++) {
		c.getPoint(src, i, &point_topography);
		fprintf(pFile,"%g %g ", point_topography.x, point_topography.y);
	}
	fprintf(pFile,"\n");

	fclose (pFile);
}

void nmr_RegistrationUI::handle_drawProjectionPoints_change(vrpn_int32 value, void *ud)
{
	float x,y;

	FILE * pFile;
	pFile = fopen ("output/ransac_points_projection.txt","r");

	if (pFile==NULL) 
	{
		perror ("Error opening file");
	}
	else
	{
		rewind (pFile);
		int width;
		int height;
		fscanf (pFile, "%d", &width);
		fscanf (pFile, "%d", &height);

		vrpn_float32 x_src[NMR_MAX_FIDUCIAL], y_src[NMR_MAX_FIDUCIAL], z_src[NMR_MAX_FIDUCIAL],
			x_tgt[NMR_MAX_FIDUCIAL], y_tgt[NMR_MAX_FIDUCIAL], z_tgt[NMR_MAX_FIDUCIAL];

		vrpn_int32 replace = 1;

		printf ("handle_drawProjectionPoints_change: ");

		int i = 0;
		for(; !feof(pFile); i++)
		{
			fscanf (pFile, "%f %f", &x, &y);
			x_src[i] = -1;
			y_src[i] = -1;
			z_src[i] = -1;

			x_tgt[i] = x;
			y_tgt[i] = y;
			z_tgt[i] = 0;

			if( i == 0)
			{
				printf ("target %d %f %f\n", i, x_tgt[i], y_tgt[i]);
			}
		}
		printf ("\n");

		printf ("LAST target %d %f %f\n", i-1, x_tgt[i-1], y_tgt[i-1]);

		if(i < NMR_MAX_FIDUCIAL)
		{
			nmr_RegistrationUI *me = static_cast<nmr_RegistrationUI *>(ud);
			me->d_aligner->get_d_local_impl()->get_d_alignerUI()->setFiducial(replace,(i-1),x_src,y_src,z_src,x_tgt,y_tgt,z_tgt);
		}
		else
		{
			printf ("Error: Number of markers in the file is bigger than NMR_MAX_FIDUCIAL\n");
		}
		fclose (pFile);
	}
}

void nmr_RegistrationUI::handle_saveProjectionPoints_change(vrpn_int32 value, void *ud)
{
	// If it is static
	nmr_RegistrationUI *me = static_cast<nmr_RegistrationUI *>(ud);
	// Check me for NULL

	Correspondence	c;
	int	src, tgt;
	me->d_aligner->get_d_local_impl()->get_d_alignerUI()->getCorrespondence(c, src, tgt);

	corr_point_t point_projection;
	unsigned i;

	FILE * pFile;
	pFile = fopen ("output/processed_ransac_projection.txt","w");
 
	//number of markers in an image
	fprintf(pFile,"%d\n", c.numPoints());

	// Projection Image Point
	for (i = 0; i < c.numPoints(); i++) {
		c.getPoint(tgt, i, &point_projection);
		fprintf(pFile,"%g %g ", point_projection.x, point_projection.y);
	}
	fprintf(pFile,"\n");

	fclose (pFile);
}

void nmr_RegistrationUI::handle_runRansac_change(vrpn_int32 value, void *ud)
{
	nmr_RegistrationUI *me = static_cast<nmr_RegistrationUI *>(ud);


///////////////////////

/*	float x,y;

	FILE * pFile;
	pFile = fopen ("output/test.txt","r");

	vector< vector< vector <float> > > test_rawransac;

	if (pFile==NULL) 
	{
		perror ("Error opening file");
	}
	else
	{
		rewind (pFile);
		int num;
		fscanf (pFile, "%d", &num);

		if(num <= NMR_MAX_FIDUCIAL)
		{
			vrpn_int32 replace = 1;

			printf ("%d ",num);

			vector< vector <float> > test_topo;
			vector< vector <float> > test_proj;

			printf ("TEST_Topography: ");
			for(int i = 0; i<146; i++)
			{
				fscanf (pFile, "%f %f", &x, &y);
				vector <float> test_pair;
				test_pair.push_back(x);
				test_pair.push_back(y);
				test_topo.push_back(test_pair);
			}
			printf ("\n");

			printf ("TEST_Projection: ");
			for(int i = 0; i<146; i++)
			{
				fscanf (pFile, "%f %f", &x, &y);
				vector <float> test_pair;
				test_pair.push_back(x);
				test_pair.push_back(y);
				test_proj.push_back(test_pair);
			}
			test_rawransac.push_back(test_topo);
			test_rawransac.push_back(test_proj);
		}
		fclose (pFile);
	}*/
///////////////////////

	//	me->d_aligner->get_d_local_impl()->get_d_alignerUI()->readPixels();
//	vector< vector< vector <float> > > rawRansac = me->d_aligner->get_d_local_impl()->get_d_alignerUI()->readPixels();

	vector< vector <float> > wh = me->d_aligner->get_d_local_impl()->get_d_alignerUI()->getWidthHeight();

#if(1)
	vector< vector< vector <float> > > rawRansac;
	
	FILE * processedTopoFile;
	processedTopoFile = fopen ("output/processed_ransac_topography.txt","r");

	vector< vector <float> > topo_rawRansac;

	if (processedTopoFile==NULL) 
	{
		perror ("Error opening file");
	}
	else
	{
		float x,y;
		rewind (processedTopoFile);
		int num;
		fscanf (processedTopoFile, "%d", &num);

		for(int i = 0; i<num; i++)
		{
			fscanf (processedTopoFile, "%f %f", &x, &y);
			vector <float> tempPoint;
//			tempPoint.push_back(x*(wh[0][0]-1));
//			tempPoint.push_back(y*(wh[0][1]-1));
			tempPoint.push_back(x);
			tempPoint.push_back(y);
			topo_rawRansac.push_back(tempPoint);
		}
	}
	fclose (processedTopoFile);


	FILE * processedProjFile;
	processedProjFile = fopen ("output/processed_ransac_projection.txt","r");

	vector< vector <float> > proj_rawRansac;

	if (processedProjFile==NULL) 
	{
		perror ("Error opening file");
	}
	else
	{
		float x,y;
		rewind (processedProjFile);
		int num;
		fscanf (processedProjFile, "%d", &num);

		for(int i = 0; i<num; i++)
		{
			fscanf (processedProjFile, "%f %f", &x, &y);
			vector <float> tempPoint;
//			tempPoint.push_back(x*(wh[1][0]-1));
//			tempPoint.push_back(y*(wh[1][1]-1));
			tempPoint.push_back(x);
			tempPoint.push_back(y);
			proj_rawRansac.push_back(tempPoint);
		}
	}
	fclose (processedProjFile);

	rawRansac.push_back(topo_rawRansac);
	rawRansac.push_back(proj_rawRansac);
#endif
//	vector< vector< vector <float> > > rawRansac = test_rawransac;

/*	float tmpResultMatrix[6][6] = {0};
	float inverseMatrix[6][6] = {0};

	float a[6][6];
	a[0][0] = 7; a[0][1] = 0; a[0][2] = 2; a[0][3] = 38; a[0][4] = 0; a[0][5] = 0;
	a[1][0] = 0; a[1][1] = 1; a[1][2] = 0; a[1][3] = 0; a[1][4] = 5; a[1][5] = 0;
	a[2][0] = 0; a[2][1] = 12; a[2][2] = 1; a[2][3] = 0; a[2][4] = 0; a[2][5] = 24;
	a[3][0] = 3; a[3][1] = 0; a[3][2] = 0; a[3][3] = 1; a[3][4] = 0; a[3][5] = 46;
	a[4][0] = 0; a[4][1] = 0; a[4][2] = 2; a[4][3] = 0; a[4][4] = 1; a[4][5] = 0;
	a[5][0] = 0; a[5][1] = 0; a[5][2] = 0; a[5][3] = 0; a[5][4] = 0; a[5][5] = 1;

	float d;
	d=detrm(a,6);
	if(d==0)
	{
		printf("\nMATRIX IS NOT INVERSIBLE\n");
	}
    else
	{
		cofact(inverseMatrix,a,6);

		for(int i = 0; i<6; i++)
		{
			for(int j = 0; j<6; j++)
			{
				printf("%f ",inverseMatrix[i][j]);
			}
			printf("\n");
		}
	}

	matrixMultiplication(tmpResultMatrix, inverseMatrix, a);
	printf("\n\n");

	for(int i = 0; i<6; i++)
	{
		for(int j = 0; j<6; j++)
		{
			printf("%f ",tmpResultMatrix[i][j]);
		}
		printf("\n");
	}*/
/////////////////////////////////////////////////////////////////////
	//rawRansac[a][b][c]
	// a: 0 --> topography; 1 --> projection
	// b: index of a data point
	// c: 0 --> x-coordinate of the point; 1 --> y-coordinate of the point

	//!!different from the regular way, data point coordinates are NOT divided to 511 to keep them between 0-1. 
	//Here, coordinates are between 0-511 (just to be sure they don't go unnoticed (i.e. get too small) during affine transformation).

//    nmr_FiducialSpotTracker tracker = NMR_LOCAL_MAX_TRACKER;
  //  me->d_aligner->setFiducialSpotTracker(NMR_SOURCE, tracker); //sadece source icin bu; target icin de yapman lazim

	vector< vector< float > > distanceMatrixTopography;
	vector< vector< float > > distanceMatrixProjection;

	for(int topo_i = 0; topo_i < rawRansac[0].size(); topo_i++)
	{
		vector<float> line_i;
		for(int topo_j = 0; topo_j < rawRansac[0].size(); topo_j++)
		{
			float dist = sqrt(pow((rawRansac[0][topo_i][0]-rawRansac[0][topo_j][0]),2) + pow((rawRansac[0][topo_i][1]-rawRansac[0][topo_j][1]),2));
			line_i.push_back(dist);
		}
		distanceMatrixTopography.push_back(line_i);
	}

	for(int proj_i = 0; proj_i < rawRansac[1].size(); proj_i++)
	{
		vector<float> line_i;
		for(int proj_j = 0; proj_j < rawRansac[1].size(); proj_j++)
		{
			float dist = sqrt(pow((rawRansac[1][proj_i][0]-rawRansac[1][proj_j][0]),2) + pow((rawRansac[1][proj_i][1]-rawRansac[1][proj_j][1]),2));
			line_i.push_back(dist);
		}
		distanceMatrixProjection.push_back(line_i);
	}

//	bool found = false;

////////////////////////////////////////////////////begin////////////////////////////////////////////
	vector< vector< vector <float> > > closestneighbors_topo;
	vector<float> distanceratio_closestneighbors_topo;

	//closestneighbors_topo[a][b][c]
	// a: index of a triplet (i.e. point "rawRansac[0][a]" with its two closest neighbors)
	// b: index of one of the points in the triplet
	// b: 0--> the point of interest 1-->closest point to the point of interest 2--> second closest to the point of interest
	// c: 0--> x coordinate, 1--> y coordinate

	//distanceratio_closestneighbors_topo[a]
	// a: ratio of "rawRansac[0][a]"s two closest neigbors --> (closest point's distance to "rawRansac[0][a]") / (second closest point's distance to "rawRansac[0][a]") 

	for(int topo_index = 0; topo_index < rawRansac[0].size(); topo_index++)
	{
		vector< vector <float> > triplet;
		vector<float> topo_index_close_1(2,100000); 
		vector<float> topo_index_close_2(2,100000);
		int indexvalue_topo_close_1 = -1; 
		int indexvalue_topo_close_2 = -1;

		float topo_index_close_1_dist = 100000, topo_index_close_2_dist = 100000;

		for(int i = 0; i < rawRansac[0].size(); i++)
		{
			if(topo_index != i)
			{
				float dist = distanceMatrixTopography[i][topo_index];
				if(dist < topo_index_close_1_dist)
				{
					topo_index_close_2 = topo_index_close_1;
					topo_index_close_1 = rawRansac[0][i];

					indexvalue_topo_close_2 = indexvalue_topo_close_1;
					indexvalue_topo_close_1 = i;

					topo_index_close_2_dist = topo_index_close_1_dist;
					topo_index_close_1_dist = dist;
				}
				else if(dist < topo_index_close_2_dist)
				{
					topo_index_close_2 = rawRansac[0][i];

					indexvalue_topo_close_2 = i;

					topo_index_close_2_dist = dist;
				}
			}
		}
		if(topo_index_close_1[0] == 100000 || topo_index_close_1[1] == 100000 ||  topo_index_close_2[0] == 100000 ||  topo_index_close_2[1] == 100000 || indexvalue_topo_close_1 == -1 || indexvalue_topo_close_2 == -1)
		{
			printf("error at handle_runRansac_change: %d %d\n", topo_index, rawRansac[0].size());
			printf("topo_index_close_1_dist: %f   topo_index_close_2_dist: %f\n", topo_index_close_1_dist, topo_index_close_2_dist);
		}

		triplet.push_back(rawRansac[0][topo_index]);
		triplet.push_back(topo_index_close_1);
		triplet.push_back(topo_index_close_2);
		closestneighbors_topo.push_back(triplet);

		distanceratio_closestneighbors_topo.push_back(distanceMatrixTopography[indexvalue_topo_close_1][topo_index]/distanceMatrixTopography[indexvalue_topo_close_2][topo_index]);
//		printf("%d dene: %f %f %f %f %f %f\n\n", topo_index, closestneighbors_topo[topo_index][0][0], closestneighbors_topo[topo_index][0][1], closestneighbors_topo[topo_index][1][0], closestneighbors_topo[topo_index][1][1], closestneighbors_topo[topo_index][2][0], closestneighbors_topo[topo_index][2][1]);
	}

	
	
	vector< vector< vector <float> > > closestneighbors_proj;
	vector<float> distanceratio_closestneighbors_proj;

	//closestneighbors_proj[a][b][c]
	// a: index of a triplet (i.e. point "rawRansac[1][a]" with its two closest neighbors)
	// b: index of one of the points in the triplet
	// b: 0--> the point of interest 1-->closest point to the point of interest 2--> second closest to the point of interest
	// c: 0--> x coordinate, 1--> y coordinate

	//distanceratio_closestneighbors_proj[a]
	// a: ratio of "rawRansac[1][a]"s two closest neigbors --> (closest point's distance to "rawRansac[1][a]") / (second closest point's distance to "rawRansac[1][a]") 

	for(int proj_index = 0; proj_index < rawRansac[1].size(); proj_index++)
	{
		vector< vector <float> > triplet;
		vector<float> proj_index_close_1(2,100000); 
		vector<float> proj_index_close_2(2,100000);
		int indexvalue_proj_close_1 = -1; 
		int indexvalue_proj_close_2 = -1;

		float proj_index_close_1_dist = 100000, proj_index_close_2_dist = 100000;

		for(int i = 0; i < rawRansac[1].size(); i++)
		{
			if(proj_index != i)
			{
				float dist = distanceMatrixProjection[i][proj_index];
				if(dist < proj_index_close_1_dist)
				{
					proj_index_close_2 = proj_index_close_1;
					proj_index_close_1 = rawRansac[1][i];

					indexvalue_proj_close_2 = indexvalue_proj_close_1;
					indexvalue_proj_close_1 = i;

					proj_index_close_2_dist = proj_index_close_1_dist;
					proj_index_close_1_dist = dist;
				}
				else if(dist < proj_index_close_2_dist)
				{
					proj_index_close_2 = rawRansac[1][i];
					
					indexvalue_proj_close_2 = i;

					proj_index_close_2_dist = dist;
				}
			}
		}
		if(proj_index_close_1[0] == 100000 || proj_index_close_1[1] == 100000 ||  proj_index_close_2[0] == 100000 ||  proj_index_close_2[1] == 100000 || indexvalue_proj_close_1 == -1 || indexvalue_proj_close_2 == -1)
		{
			printf("error at handle_runRansac_change: %d %d\n", proj_index, rawRansac[0].size());
			printf("proj_index_close_1_dist: %f   proj_index_close_2_dist: %f\n", proj_index_close_1_dist, proj_index_close_2_dist);
		}

		triplet.push_back(rawRansac[1][proj_index]);
		triplet.push_back(proj_index_close_1);
		triplet.push_back(proj_index_close_2);
		closestneighbors_proj.push_back(triplet);

		distanceratio_closestneighbors_proj.push_back(distanceMatrixProjection[indexvalue_proj_close_1][proj_index]/distanceMatrixProjection[indexvalue_proj_close_2][proj_index]);
//		printf("%d dene: %f %f %f\n",proj_index,distanceMatrixProjection[indexvalue_proj_close_1][proj_index], distanceMatrixProjection[indexvalue_proj_close_2][proj_index], distanceratio_closestneighbors_proj[proj_index]);
//		printf("%d dene: %f %f %f %f %f %f\n\n", proj_index, closestneighbors_proj[proj_index][0][0], closestneighbors_proj[proj_index][0][1], closestneighbors_proj[proj_index][1][0], closestneighbors_proj[proj_index][1][1], closestneighbors_proj[proj_index][2][0], closestneighbors_proj[proj_index][2][1]);
	}

	int sayaca = 0;
	int hele = 0;
	float overall_minimum_median = 100000;
	
	FILE * identityFile;
	identityFile = fopen ("identity.txt","w");

	FILE * ransacFile;
	ransacFile = fopen ("output/ransac_markers.txt","w");

	vector< vector < float > > ransacmarkers_topo;
	vector< vector < float > > ransacmarkers_proj;

	for(int i = 0; i < distanceratio_closestneighbors_topo.size(); i++)
	{
		for(int j = 0; j < distanceratio_closestneighbors_proj.size(); j++)
		{
			if(distanceratio_closestneighbors_topo[i] <= distanceratio_closestneighbors_proj[j]*1.02 && distanceratio_closestneighbors_topo[i] >= distanceratio_closestneighbors_proj[j]*0.98)
			{
//				printf("%d esit\n", sayaca);
				sayaca++;

	//closestneighbors_topo[a][b][c]
	// a: index of a triplet (i.e. point "rawRansac[0][a]" with its two closest neighbors)
	// b: index of one of the points in the triplet
	// b: 0--> the point of interest 1-->closest point to the point of interest 2--> second closest to the point of interest
	// c: 0--> x coordinate, 1--> y coordinate	 


				float topo_matrix_M[6][6] = {0};
				topo_matrix_M[0][0] = closestneighbors_topo[i][0][0]; topo_matrix_M[0][1] = closestneighbors_topo[i][0][1]; topo_matrix_M[0][4] = 1;
				topo_matrix_M[1][2] = closestneighbors_topo[i][0][0]; topo_matrix_M[1][3] = closestneighbors_topo[i][0][1]; topo_matrix_M[1][5] = 1;
				topo_matrix_M[2][0] = closestneighbors_topo[i][1][0]; topo_matrix_M[2][1] = closestneighbors_topo[i][1][1]; topo_matrix_M[2][4] = 1;
				topo_matrix_M[3][2] = closestneighbors_topo[i][1][0]; topo_matrix_M[3][3] = closestneighbors_topo[i][1][1]; topo_matrix_M[3][5] = 1;
				topo_matrix_M[4][0] = closestneighbors_topo[i][2][0]; topo_matrix_M[4][1] = closestneighbors_topo[i][2][1]; topo_matrix_M[4][4] = 1;
				topo_matrix_M[5][2] = closestneighbors_topo[i][2][0]; topo_matrix_M[5][3] = closestneighbors_topo[i][2][1]; topo_matrix_M[5][5] = 1;


				float topo_matrix_transpose_M[6][6] = {0};
			    for(int trans_i=0; trans_i < 6; trans_i++)
				{
					for(int trans_j=0; trans_j < 6; trans_j++)
					{
                        topo_matrix_transpose_M[trans_i][trans_j]=topo_matrix_M[trans_j][trans_i];
					}
				}

				float topo_matrix_r[6];
				topo_matrix_r[0] = closestneighbors_proj[j][0][0];
				topo_matrix_r[1] = closestneighbors_proj[j][0][1];
				topo_matrix_r[2] = closestneighbors_proj[j][1][0];
				topo_matrix_r[3] = closestneighbors_proj[j][1][1];
				topo_matrix_r[4] = closestneighbors_proj[j][2][0];
				topo_matrix_r[5] = closestneighbors_proj[j][2][1];

				float topo_matrix_TM_M_multiplied[6][6] = {0};

				matrixMultiplication(topo_matrix_TM_M_multiplied, topo_matrix_transpose_M, topo_matrix_M);
/*				if(sayaca == 1)
				{
					printf("topo_matrix_transpose_M\n");
					printf("%f %f %f %f %f %f\n", topo_matrix_transpose_M[0][0], topo_matrix_transpose_M[0][1], topo_matrix_transpose_M[0][2], topo_matrix_transpose_M[0][3], topo_matrix_transpose_M[0][4], topo_matrix_transpose_M[0][5]);
					printf("%f %f %f %f %f %f\n", topo_matrix_transpose_M[1][0], topo_matrix_transpose_M[1][1], topo_matrix_transpose_M[1][2], topo_matrix_transpose_M[1][3], topo_matrix_transpose_M[1][4], topo_matrix_transpose_M[1][5]);
					printf("%f %f %f %f %f %f\n", topo_matrix_transpose_M[2][0], topo_matrix_transpose_M[2][1], topo_matrix_transpose_M[2][2], topo_matrix_transpose_M[2][3], topo_matrix_transpose_M[2][4], topo_matrix_transpose_M[2][5]);
					printf("%f %f %f %f %f %f\n", topo_matrix_transpose_M[3][0], topo_matrix_transpose_M[3][1], topo_matrix_transpose_M[3][2], topo_matrix_transpose_M[3][3], topo_matrix_transpose_M[3][4], topo_matrix_transpose_M[3][5]);
					printf("%f %f %f %f %f %f\n", topo_matrix_transpose_M[4][0], topo_matrix_transpose_M[4][1], topo_matrix_transpose_M[4][2], topo_matrix_transpose_M[4][3], topo_matrix_transpose_M[4][4], topo_matrix_transpose_M[4][5]);
					printf("%f %f %f %f %f %f\n", topo_matrix_transpose_M[5][0], topo_matrix_transpose_M[5][1], topo_matrix_transpose_M[5][2], topo_matrix_transpose_M[5][3], topo_matrix_transpose_M[5][4], topo_matrix_transpose_M[5][5]);
					printf("\ntopo_matrix_M\n");
					printf("%f %f %f %f %f %f\n", topo_matrix_M[0][0], topo_matrix_M[0][1], topo_matrix_M[0][2], topo_matrix_M[0][3], topo_matrix_M[0][4], topo_matrix_M[0][5]);
					printf("%f %f %f %f %f %f\n", topo_matrix_M[1][0], topo_matrix_M[1][1], topo_matrix_M[1][2], topo_matrix_M[1][3], topo_matrix_M[1][4], topo_matrix_M[1][5]);
					printf("%f %f %f %f %f %f\n", topo_matrix_M[2][0], topo_matrix_M[2][1], topo_matrix_M[2][2], topo_matrix_M[2][3], topo_matrix_M[2][4], topo_matrix_M[2][5]);
					printf("%f %f %f %f %f %f\n", topo_matrix_M[3][0], topo_matrix_M[3][1], topo_matrix_M[3][2], topo_matrix_M[3][3], topo_matrix_M[3][4], topo_matrix_M[3][5]);
					printf("%f %f %f %f %f %f\n", topo_matrix_M[4][0], topo_matrix_M[4][1], topo_matrix_M[4][2], topo_matrix_M[4][3], topo_matrix_M[4][4], topo_matrix_M[4][5]);
					printf("%f %f %f %f %f %f\n", topo_matrix_M[5][0], topo_matrix_M[5][1], topo_matrix_M[5][2], topo_matrix_M[5][3], topo_matrix_M[5][4], topo_matrix_M[5][5]);
					printf("\ntopo_matrix_TM_M_multiplied\n");
					printf("%f %f %f %f %f %f\n", topo_matrix_TM_M_multiplied[0][0], topo_matrix_TM_M_multiplied[0][1], topo_matrix_TM_M_multiplied[0][2], topo_matrix_TM_M_multiplied[0][3], topo_matrix_TM_M_multiplied[0][4], topo_matrix_TM_M_multiplied[0][5]);
					printf("%f %f %f %f %f %f\n", topo_matrix_TM_M_multiplied[1][0], topo_matrix_TM_M_multiplied[1][1], topo_matrix_TM_M_multiplied[1][2], topo_matrix_TM_M_multiplied[1][3], topo_matrix_TM_M_multiplied[1][4], topo_matrix_TM_M_multiplied[1][5]);
					printf("%f %f %f %f %f %f\n", topo_matrix_TM_M_multiplied[2][0], topo_matrix_TM_M_multiplied[2][1], topo_matrix_TM_M_multiplied[2][2], topo_matrix_TM_M_multiplied[2][3], topo_matrix_TM_M_multiplied[2][4], topo_matrix_TM_M_multiplied[2][5]);
					printf("%f %f %f %f %f %f\n", topo_matrix_TM_M_multiplied[3][0], topo_matrix_TM_M_multiplied[3][1], topo_matrix_TM_M_multiplied[3][2], topo_matrix_TM_M_multiplied[3][3], topo_matrix_TM_M_multiplied[3][4], topo_matrix_TM_M_multiplied[3][5]);
					printf("%f %f %f %f %f %f\n", topo_matrix_TM_M_multiplied[4][0], topo_matrix_TM_M_multiplied[4][1], topo_matrix_TM_M_multiplied[4][2], topo_matrix_TM_M_multiplied[4][3], topo_matrix_TM_M_multiplied[4][4], topo_matrix_TM_M_multiplied[4][5]);
					printf("%f %f %f %f %f %f\n", topo_matrix_TM_M_multiplied[5][0], topo_matrix_TM_M_multiplied[5][1], topo_matrix_TM_M_multiplied[5][2], topo_matrix_TM_M_multiplied[5][3], topo_matrix_TM_M_multiplied[5][4], topo_matrix_TM_M_multiplied[5][5]);
				}*/

////////////////////////////////////////

				float inverseMatrix[6][6] = {0};

				float determinant;
				determinant = detrm(topo_matrix_TM_M_multiplied,6);
				if(determinant == 0)
				{
					printf("\nMATRIX IS NOT INVERSIBLE\n");
					hele++;
				}
				else
				{
		//			printf("%d esit %d\n", sayaca, hele);
			//		cofact(inverseMatrix,topo_matrix_TM_M_multiplied,6);
					Matrix topo_tm_m(6,6);
					topo_tm_m(0,0) = topo_matrix_TM_M_multiplied[0][0];topo_tm_m(0,1) = topo_matrix_TM_M_multiplied[0][1];
					topo_tm_m(0,2) = topo_matrix_TM_M_multiplied[0][2];topo_tm_m(0,3) = topo_matrix_TM_M_multiplied[0][3];
					topo_tm_m(0,4) = topo_matrix_TM_M_multiplied[0][4];topo_tm_m(0,5) = topo_matrix_TM_M_multiplied[0][5];

					topo_tm_m(1,0) = topo_matrix_TM_M_multiplied[1][0];topo_tm_m(1,1) = topo_matrix_TM_M_multiplied[1][1];
					topo_tm_m(1,2) = topo_matrix_TM_M_multiplied[1][2];topo_tm_m(1,3) = topo_matrix_TM_M_multiplied[1][3];
					topo_tm_m(1,4) = topo_matrix_TM_M_multiplied[1][4];topo_tm_m(1,5) = topo_matrix_TM_M_multiplied[1][5];

					topo_tm_m(2,0) = topo_matrix_TM_M_multiplied[2][0];topo_tm_m(2,1) = topo_matrix_TM_M_multiplied[2][1];
					topo_tm_m(2,2) = topo_matrix_TM_M_multiplied[2][2];topo_tm_m(2,3) = topo_matrix_TM_M_multiplied[2][3];
					topo_tm_m(2,4) = topo_matrix_TM_M_multiplied[2][4];topo_tm_m(2,5) = topo_matrix_TM_M_multiplied[2][5];

					topo_tm_m(3,0) = topo_matrix_TM_M_multiplied[3][0];topo_tm_m(3,1) = topo_matrix_TM_M_multiplied[3][1];
					topo_tm_m(3,2) = topo_matrix_TM_M_multiplied[3][2];topo_tm_m(3,3) = topo_matrix_TM_M_multiplied[3][3];
					topo_tm_m(3,4) = topo_matrix_TM_M_multiplied[3][4];topo_tm_m(3,5) = topo_matrix_TM_M_multiplied[3][5];

					topo_tm_m(4,0) = topo_matrix_TM_M_multiplied[4][0];topo_tm_m(4,1) = topo_matrix_TM_M_multiplied[4][1];
					topo_tm_m(4,2) = topo_matrix_TM_M_multiplied[4][2];topo_tm_m(4,3) = topo_matrix_TM_M_multiplied[4][3];
					topo_tm_m(4,4) = topo_matrix_TM_M_multiplied[4][4];topo_tm_m(4,5) = topo_matrix_TM_M_multiplied[4][5];

					topo_tm_m(5,0) = topo_matrix_TM_M_multiplied[5][0];topo_tm_m(5,1) = topo_matrix_TM_M_multiplied[5][1];
					topo_tm_m(5,2) = topo_matrix_TM_M_multiplied[5][2];topo_tm_m(5,3) = topo_matrix_TM_M_multiplied[5][3];
					topo_tm_m(5,4) = topo_matrix_TM_M_multiplied[5][4];topo_tm_m(5,5) = topo_matrix_TM_M_multiplied[5][5];

					Matrix matinverse = !topo_tm_m;

					inverseMatrix[0][0] = matinverse(0,0); inverseMatrix[0][1] = matinverse(0,1); inverseMatrix[0][2] = matinverse(0,2);
					inverseMatrix[0][3] = matinverse(0,3); inverseMatrix[0][4] = matinverse(0,4); inverseMatrix[0][5] = matinverse(0,5);

					inverseMatrix[1][0] = matinverse(1,0); inverseMatrix[1][1] = matinverse(1,1); inverseMatrix[1][2] = matinverse(1,2);
					inverseMatrix[1][3] = matinverse(1,3); inverseMatrix[1][4] = matinverse(1,4); inverseMatrix[1][5] = matinverse(1,5);

					inverseMatrix[2][0] = matinverse(2,0); inverseMatrix[2][1] = matinverse(2,1); inverseMatrix[2][2] = matinverse(2,2);
					inverseMatrix[2][3] = matinverse(2,3); inverseMatrix[2][4] = matinverse(2,4); inverseMatrix[2][5] = matinverse(2,5);

					inverseMatrix[3][0] = matinverse(3,0); inverseMatrix[3][1] = matinverse(3,1); inverseMatrix[3][2] = matinverse(3,2);
					inverseMatrix[3][3] = matinverse(3,3); inverseMatrix[3][4] = matinverse(3,4); inverseMatrix[3][5] = matinverse(3,5);

					inverseMatrix[4][0] = matinverse(4,0); inverseMatrix[4][1] = matinverse(4,1); inverseMatrix[4][2] = matinverse(4,2);
					inverseMatrix[4][3] = matinverse(4,3); inverseMatrix[4][4] = matinverse(4,4); inverseMatrix[4][5] = matinverse(4,5);

					inverseMatrix[5][0] = matinverse(5,0); inverseMatrix[5][1] = matinverse(5,1); inverseMatrix[5][2] = matinverse(5,2);
					inverseMatrix[5][3] = matinverse(5,3); inverseMatrix[5][4] = matinverse(5,4); inverseMatrix[5][5] = matinverse(5,5);



					float identityCheckMatrix[6][6] = {0};
					matrixMultiplication(identityCheckMatrix, inverseMatrix, topo_matrix_TM_M_multiplied);
					fprintf(identityFile,"***%d***\n", sayaca);
					for(int iden_i = 0; iden_i < 6; iden_i++)
					{
						for(int iden_j = 0; iden_j < 6; iden_j++)
						{
							fprintf(identityFile,"%f ", identityCheckMatrix[iden_i][iden_j]);
						}
						fprintf(identityFile,"\n");
					}
					fprintf(identityFile,"\n\n\n");
	
					float topo_matrix_TM_r_multiplied[6] = {0};
						
					for ( int ind_r = 0; ind_r < 6; ind_r++ )
					{
						topo_matrix_TM_r_multiplied[ind_r] = 0.0;
						for ( int ind_i = 0; ind_i < 6; ind_i++ )
						{
							topo_matrix_TM_r_multiplied[ind_r] +=  topo_matrix_transpose_M[ind_r][ind_i] * topo_matrix_r[ind_i];
						}
					}

					float topo_matrix_v[6] = {0};
					for ( int ind_r = 0; ind_r < 6; ind_r++ )
					{
						topo_matrix_v[ind_r] = 0.0;
						for ( int ind_i = 0; ind_i < 6; ind_i++ )
						{
							topo_matrix_v[ind_r] +=  inverseMatrix[ind_r][ind_i] * topo_matrix_TM_r_multiplied[ind_i];
						}
					}

/*					if(sayaca == 1)
					{
						printf("\ninverseMatrix\n");
						printf("%f %f %f %f %f %f\n", inverseMatrix[0][0], inverseMatrix[0][1], inverseMatrix[0][2], inverseMatrix[0][3], inverseMatrix[0][4], inverseMatrix[0][5]);
						printf("%f %f %f %f %f %f\n", inverseMatrix[1][0], inverseMatrix[1][1], inverseMatrix[1][2], inverseMatrix[1][3], inverseMatrix[1][4], inverseMatrix[1][5]);
						printf("%f %f %f %f %f %f\n", inverseMatrix[2][0], inverseMatrix[2][1], inverseMatrix[2][2], inverseMatrix[2][3], inverseMatrix[2][4], inverseMatrix[2][5]);
						printf("%f %f %f %f %f %f\n", inverseMatrix[3][0], inverseMatrix[3][1], inverseMatrix[3][2], inverseMatrix[3][3], inverseMatrix[3][4], inverseMatrix[3][5]);
						printf("%f %f %f %f %f %f\n", inverseMatrix[4][0], inverseMatrix[4][1], inverseMatrix[4][2], inverseMatrix[4][3], inverseMatrix[4][4], inverseMatrix[4][5]);
						printf("%f %f %f %f %f %f\n", inverseMatrix[5][0], inverseMatrix[5][1], inverseMatrix[5][2], inverseMatrix[5][3], inverseMatrix[5][4], inverseMatrix[5][5]);
						printf("\ntopo_matrix_r\n");
						printf("%f %f %f %f %f %f\n", topo_matrix_r[0], topo_matrix_r[1], topo_matrix_r[2], topo_matrix_r[3], topo_matrix_r[4], topo_matrix_r[5]);
						printf("\ntopo_matrix_TM_r_multiplied\n");
						printf("%f %f %f %f %f %f\n", topo_matrix_TM_r_multiplied[0], topo_matrix_TM_r_multiplied[1], topo_matrix_TM_r_multiplied[2], topo_matrix_TM_r_multiplied[3], topo_matrix_TM_r_multiplied[4], topo_matrix_TM_r_multiplied[5]);
						printf("\ntopo_matrix_v\n");
						printf("%f %f %f %f %f %f\n", topo_matrix_v[0], topo_matrix_v[1], topo_matrix_v[2], topo_matrix_v[3], topo_matrix_v[4], topo_matrix_v[5]);
					}
*/


/*					for(int inv_i = 0; i<6; i++)
					{
						for(int inv_j = 0; j<6; j++)
						{
							printf("%f ",inverseMatrix[inv_i][inv_j]);
						}
						printf("\n");
					}
				
					float testResultMatrix[6][6] = {0};
					matrixMultiplication(testResultMatrix, inverseMatrix, topo_matrix_TM_M_multiplied);
					printf("\n\n");
					for(int test_i = 0; i<6; i++)
					{
						for(int test_j = 0; j<6; j++)
						{
							printf("%f ",testResultMatrix[test_i][test_j]);
						}
						printf("\n");
					}*/

	//				int no_of_correspondences = 0;
					vector<float> sum_of_errors;


//					if(rawRansac[0].size() <= rawRansac[1].size())
//					{
						vector< vector < float > > temp_ransacmarkers_proj;

						for(int den_i = 0; den_i < rawRansac[0].size(); den_i++)
						{
							float tmp_proj_x = topo_matrix_v[0]*rawRansac[0][den_i][0] + topo_matrix_v[1]*rawRansac[0][den_i][1] + topo_matrix_v[4];
							float tmp_proj_y = topo_matrix_v[2]*rawRansac[0][den_i][0] + topo_matrix_v[3]*rawRansac[0][den_i][1] + topo_matrix_v[5];
							float smallest_distance = 100000;
							int minimum_ind = 0;

							vector < float > temp_unit_proj(2,0);
							for(int den_j = 0; den_j < rawRansac[1].size(); den_j++)
							{
								float temp_distance = sqrt(pow((rawRansac[1][den_j][0]-tmp_proj_x),2) + pow((rawRansac[1][den_j][1]-tmp_proj_y),2));
								if(temp_distance < smallest_distance)
								{
									smallest_distance = temp_distance;
									minimum_ind = den_j;
								}
							}

							temp_unit_proj[0] = tmp_proj_x;
							temp_unit_proj[1] = tmp_proj_y;
							temp_ransacmarkers_proj.push_back(temp_unit_proj);

							sum_of_errors.push_back(smallest_distance);
						}

						sort(sum_of_errors.begin(),sum_of_errors.begin()+rawRansac[0].size());

						float median_value;
						if(sum_of_errors.size()%2 == 0)
						{
							median_value = (sum_of_errors[sum_of_errors.size()/2] + sum_of_errors[sum_of_errors.size()/2 - 1])/2;
						}
						else
						{
							median_value = sum_of_errors[sum_of_errors.size()/2];
						}

						if(median_value < overall_minimum_median)
						{
							overall_minimum_median = median_value;
							ransacmarkers_topo = rawRansac[0];
							ransacmarkers_proj = temp_ransacmarkers_proj;
						}
//					}
					/*else
					{
						vector< vector < float > > temp_ransacmarkers_proj;

						vector< vector < float > > new_proj;

						for(int den_i = 0; den_i < rawRansac[0].size(); den_i++)
						{
							float tmp_proj_x = topo_matrix_v[0]*rawRansac[0][den_i][0] + topo_matrix_v[1]*rawRansac[0][den_i][1] + topo_matrix_v[4];
							float tmp_proj_y = topo_matrix_v[2]*rawRansac[0][den_i][0] + topo_matrix_v[3]*rawRansac[0][den_i][1] + topo_matrix_v[5];
							vector<float> tmp_unit;
							tmp_unit.push_back(tmp_proj_x);
							tmp_unit.push_back(tmp_proj_y);
							new_proj.push_back(tmp_unit);
						}

						for(int den_j = 0; den_j < rawRansac[1].size(); den_j++)
						{
							float smallest_distance = 100000;

							for(int t_i = 0; t_i < new_proj.size(); t_i++)
							{
								float temp_distance = sqrt(pow((rawRansac[1][den_j][0]-new_proj[t_i][0]),2) + pow((rawRansac[1][den_j][1]-new_proj[t_i][1]),2));
								if(temp_distance < smallest_distance)
								{
									smallest_distance = temp_distance;
								}
							}
							sum_of_errors.push_back(smallest_distance);
						}

						sort(sum_of_errors.begin(),sum_of_errors.begin()+rawRansac[1].size());

						float median_value;
						if(sum_of_errors.size()%2 == 0)
						{
							median_value = (sum_of_errors[sum_of_errors.size()/2] + sum_of_errors[sum_of_errors.size()/2 - 1])/2;
						}
						else
						{
							median_value = sum_of_errors[sum_of_errors.size()/2];
						}

						if(median_value < overall_minimum_median)
						{
							overall_minimum_median = median_value;
						}
					}*/

	//				printf("no_of_correspondences for %d %d is %d\n", i, j, no_of_correspondences);
				}


/////////////////////////////////////////

			}
		}
	}

//	vector< vector <float> > wh = me->d_aligner->get_d_local_impl()->get_d_alignerUI()->getWidthHeight();

	fprintf(ransacFile,"%d\n", ransacmarkers_topo.size());

	for (int r_i = 0; r_i < ransacmarkers_topo.size(); r_i++) 
	{
//		fprintf(ransacFile,"%g %g ", ransacmarkers_topo[r_i][0]/(wh[0][0]-1), ransacmarkers_topo[r_i][1]/(wh[0][1]-1));
		fprintf(ransacFile,"%g %g ", ransacmarkers_topo[r_i][0], ransacmarkers_topo[r_i][1]);
	}
	fprintf(ransacFile,"\n");

	for (int r_i = 0; r_i < ransacmarkers_proj.size(); r_i++) 
	{
//		fprintf(ransacFile,"%g %g ", ransacmarkers_proj[r_i][0]/(wh[1][0]-1), ransacmarkers_proj[r_i][1]/(wh[1][1]-1));
		fprintf(ransacFile,"%g %g ", ransacmarkers_proj[r_i][0], ransacmarkers_proj[r_i][1]);
	}
	fprintf(ransacFile,"\n");

	fclose (ransacFile);
	fclose (identityFile);
	printf("\noverall_minimum_median : %f\n", overall_minimum_median);
////////////////////////////////////////////////////end//////////////////////////////////////////////

	//there are many repetitions. optimize this part.
/*	for(int topo_index = 0; topo_index < rawRansac[0].size() && found == false ; topo_index++)
	{
		vector<float> topo_index_close_1(2,100000); 
		vector<float> topo_index_close_2(2,100000);

		float topo_index_close_1_dist = 100000, topo_index_close_2_dist = 100000;

		for(int i = 0; i < rawRansac[0].size(); i++)
		{
			if(topo_index != i)
			{
				float dist = distanceMatrixTopography[i][topo_index];
				if(dist < topo_index_close_1_dist)
				{
					topo_index_close_2 = topo_index_close_1;
					topo_index_close_1 = rawRansac[0][i];

					topo_index_close_2_dist = topo_index_close_1_dist;
					topo_index_close_1_dist = dist;
				}
				else if(dist < topo_index_close_2_dist)
				{
					topo_index_close_2 = rawRansac[0][i];
					topo_index_close_2_dist = dist;
				}
			}
		}
		if(topo_index_close_1[0] == 100000 || topo_index_close_1[1] == 100000 ||  topo_index_close_2[0] == 100000 ||  topo_index_close_2[1] == 100000)
		{
			printf("error at handle_runRansac_change: %d %d\n", topo_index, rawRansac[0].size());
			printf("topo_index_close_1_dist: %f   topo_index_close_2_dist: %f\n", topo_index_close_1_dist, topo_index_close_2_dist);
		}
//		printf("topo_index_close_1_dist: %f   topo_index_close_2_dist: %f\n", topo_index_close_1_dist, topo_index_close_2_dist);

		
		for(int proj_index = 0; proj_index < rawRansac[1].size(); proj_index++)
		{
			vector< vector<float> > proj_index_near_1s; // candidate nearest ones
			vector< vector<float> > proj_index_near_2s; // candidate second nearest ones
			for(int j = 0; j < rawRansac[1].size(); j++)
			{
				if(proj_index != j)
				{
					float dist = distanceMatrixProjection[j][proj_index];
					//if( dist < topo_index_close_1_dist)
					if( dist > topo_index_close_1_dist*0.8 && dist < topo_index_close_1_dist*1.2)
					{
						proj_index_near_1s.push_back(rawRansac[1][j]);
					}
					else if( dist > topo_index_close_2_dist*0.8 && dist < topo_index_close_2_dist*1.2)
					{
						proj_index_near_2s.push_back(rawRansac[1][j]);
					}
				}
			}
			if(proj_index_near_1s.size() > 0 && proj_index_near_2s.size() > 0)
			{
				vector<float> topo_point_a = rawRansac[0][topo_index];
				vector<float> topo_point_b = topo_index_close_1;
				vector<float> topo_point_c = topo_index_close_2;
				
				vector<float> proj_point_a = rawRansac[1][proj_index];
				vector<float> proj_point_b;
				vector<float> proj_point_c;

				for(int ind_proj_point_b = 0; ind_proj_point_b < proj_index_near_1s.size(); ind_proj_point_b++)
				{
					proj_point_b = proj_index_near_1s[ind_proj_point_b];

					for(int ind_proj_point_c = 0; ind_proj_point_c < proj_index_near_2s.size(); ind_proj_point_c++)
					{
						proj_point_c = proj_index_near_2s[ind_proj_point_c];
					}
				}

				printf("topo_point: %f %f %f %f %f %f\n", topo_point_a[0], topo_point_a[1], topo_point_b[0], topo_point_b[1], topo_point_c[0], topo_point_c[1]);
				printf("proj_point :%f %f %f %f %f %f\n\n", proj_point_a[0], proj_point_a[1], proj_point_b[0], proj_point_b[1], proj_point_c[0], proj_point_c[1]);

//				printf("topo_index: %d  proj_index: %d\n", topo_index, proj_index);
//				printf("\n");
//				printf("%f %f\n", rawRansac[0][proj_index][0], rawRansac[0][proj_index][1]);
//				printf("%f %f\n", topo_index_close_1[0], topo_index_close_1[1]);
//				printf("%f %f\n", topo_index_close_2[0], topo_index_close_2[1]);
//				printf("\n");
//				printf("%f %f\n", rawRansac[1][proj_index][0], rawRansac[1][proj_index][1]);
//				printf("%f %f\n", proj_index_near_1s[0][0], proj_index_near_1s[0][1]);
//				printf("%f %f\n", proj_index_near_2s[0][0], proj_index_near_2s[0][1]);

			}

			//try every combination
			//if found sth, do sth
			//if not, do nothing
		}
	}*/
//	printf("c_coun: %d\n",c_coun);

} //new

void nmr_RegistrationUI::handle_runDrawRansac_change(vrpn_int32 value, void *ud)
{
	float x,y;

	FILE * pFile;
	pFile = fopen ("output/ransac_markers.txt","r");

	if (pFile==NULL) 
	{
		perror ("Error opening file");
	}
	else
	{
		rewind (pFile);
		int num;
		fscanf (pFile, "%d", &num);

		if(num <= NMR_MAX_FIDUCIAL)
		{
			vrpn_float32 x_src[NMR_MAX_FIDUCIAL], y_src[NMR_MAX_FIDUCIAL], z_src[NMR_MAX_FIDUCIAL],
				x_tgt[NMR_MAX_FIDUCIAL], y_tgt[NMR_MAX_FIDUCIAL], z_tgt[NMR_MAX_FIDUCIAL];

			vrpn_int32 replace = 1;


			printf ("%d ",num);
			printf ("Topography: ");
			for(int i = 0; i<num; i++)
			{
				fscanf (pFile, "%f %f", &x, &y);
				x_src[i] = x;
				y_src[i] = y;
				z_src[i] = 0;
		//		printf ( "%f %f ", x, y);
				if( i == 0 || i == (num-1))
				{
					printf ("source %d %f %f\n", i, x_src[i], y_src[i]);
				}
			}
			printf ("\n");

			printf ("Projection: ");
			for(int i = 0; i<num; i++)
			{
				fscanf (pFile, "%f %f", &x, &y);
				x_tgt[i] = x;
				y_tgt[i] = y;
				z_tgt[i] = 0;
		//		printf ( "%f %f ", x, y);
				if( i == 0 || i == (num-1))
				{
					printf ("target %d %f %f\n", i, x_tgt[i], y_tgt[i]);
				}
			}
			printf ("\n");

			nmr_RegistrationUI *me = static_cast<nmr_RegistrationUI *>(ud);
			me->d_aligner->get_d_local_impl()->get_d_alignerUI()->setFiducial(replace,num,x_src,y_src,z_src,x_tgt,y_tgt,z_tgt);
		}
		else
		{
			printf ("Error: Number of markers in the file is bigger than NMR_MAX_FIDUCIAL\n");
		}
		fclose (pFile);
	}
}


void nmr_RegistrationUI::sendTransformationParameters()
{
  vrpn_float32 *parameters = new vrpn_float32[nmb_numTransformParameters];
  nmb_Transform_TScShR transform;
  transform.setParameter(NMB_ROTATE_2D_Z, (double)d_rotate2D_Z*M_PI/180.0);
  transform.setParameter(NMB_ROTATE_3D_X, (double)d_rotate3D_X*M_PI/180.0);
  transform.setParameter(NMB_ROTATE_3D_Z, (double)d_rotate3D_Z*M_PI/180.0);
  transform.setParameter(NMB_TRANSLATE_X, (double)d_translateX);
  transform.setParameter(NMB_TRANSLATE_Y, (double)d_translateY);
  transform.setParameter(NMB_TRANSLATE_Z, (double)d_translateZ);
  transform.setParameter(NMB_SCALE_X, (double)d_scaleX);
  transform.setParameter(NMB_SCALE_Y, (double)d_scaleY);
  transform.setParameter(NMB_SHEAR_Z, (double)d_shearZ);
  transform.setCenter(d_translateX, d_translateY, d_translateZ);
  int i;
  for (i = 0; i < nmb_numTransformParameters; i++) {
    parameters[i] = (vrpn_float32)transform.getParameter(
                                 nmb_transformParameterOrder[i]);
  }
  d_aligner->setTransformationParameters(parameters);
}

// static
void nmr_RegistrationUI::handle_registrationEnabled_change(
      vrpn_int32 value, void *ud)
{
    nmr_RegistrationUI *me = (nmr_RegistrationUI *)ud;
    if (value) {
        me->d_aligner->setGUIEnable(vrpn_TRUE, NMR_ALLWINDOWS);
        //Init images
        handle_registrationImage3D_change(
            me->d_registrationImageName3D.string(), me);
        handle_registrationImage2D_change(
            me->d_registrationImageName2D.string(), me);

        // We're popping up our window. Set the 3D image to 
        // height plane if we have any clue what that is. 
/*
        nmb_Image *dataim;
        if (!me->d_dataset) return;

        if (me->d_dataset->heightPlaneName) {
          me->d_registrationImageName3D = 
                        me->d_dataset->heightPlaneName->string();
        } else 
        if (me->d_dataset->dataImages()->numImages() > 0){
          dataim = me->d_dataset->dataImages()->getImage(0);
          me->d_registrationImageName3D = dataim->name()->c_str();
        }
        // Also guess at the 2D image name
        int i;
        for (i = 1; i < me->d_dataset->dataImages()->numImages(); i++) {
            dataim = me->d_dataset->dataImages()->getImage(i);
            //printf("Considering %s\n", dataim->name()->c_str());
            if (strcmp(dataim->name()->c_str(),
                       me->d_dataset->heightPlaneName->string())) {
                me->d_registrationImageName2D = dataim->name()->c_str();
                break;
            }
        }
*/
    } else {
        me->d_aligner->setGUIEnable(vrpn_FALSE, NMR_ALLWINDOWS);
    }
}


void nmr_RegistrationUI::createResampleImage(const char * /*imageName */)
{
    //printf("nmr_RegistrationUI::createResampleImage\n");
    nmb_Image *new_image;
    if (!d_dataset) return;
    nmb_Image *im_3D = d_dataset->dataImages()->getImageByName
                          (d_registrationImageName3D.string());
    nmb_Image *im_2D = d_dataset->dataImages()->getImageByName
                          (d_registrationImageName2D.string());

    int displayedTransformIndex = getDisplayedTransformIndex();

    // d_imageTransformWorldSpace is the transformation that takes points from
    // height image to points in the texture image

    // Make sure we have all the information we need and
    // see if the user has given a name to the resampled plane
    // other than "".  If so, we should create a new plane and set the value
    // back to "".
    if (!im_3D || !im_2D ||
               (strlen(d_newResampleImageName.string()) == 0)){
        return;

    } else if (d_invertWarp) {    // warp the AFM image to the SEM image
        new_image = new nmb_ImageGrid(
                (const char *)(d_newResampleImageName.string()),
                (const char *)(im_3D->unitsValue()),
                d_resampleResolutionX, d_resampleResolutionY);
        d_newResampleImageName = (const char *) "";
        nmb_ImageBounds im2D_bounds;
        im_2D->getBounds(im2D_bounds);
        new_image->setBounds(im2D_bounds);
        nmb_TransformMatrix44 projImageFromTopoImage;
        nmb_TransformMatrix44 topoImageFromProjImage;

        /*
          projImageFromTopoImage = (scaledProjImToProjImage)*
                                   (d_scaledProjImFromScaledTopoIm)*
                                   (topoImageToScaledTopoImage)
        */
        nmr_Util::computeResampleTransformInImageCoordinates(
                  im_2D, im_3D,
                  d_scaledProjImFromScaledTopoIm[displayedTransformIndex],
                  projImageFromTopoImage);

        if (projImageFromTopoImage.hasInverse()) {
          topoImageFromProjImage = projImageFromTopoImage;
          topoImageFromProjImage.invert();
        } else {
          fprintf(stderr,
                "createResampleImage failed: non-invertible transform\n");
        }

		double intensityRange = im_2D->maxNonZeroValue() - im_2D->minNonZeroValue();
		double nonZeroIntensityOffset = -(im_2D->minNonZeroValue()-0.01*intensityRange);
        nmr_Util::createResampledImageWithImageSpaceTransformation((*im_3D), 
                 (*im_2D),
                 topoImageFromProjImage, (*new_image), nonZeroIntensityOffset);

        // an extra step: combine the two datasets in a somewhat arbitrary
        // way so that you can see features from both in a single image
        // it would be nice to create a color image from these two instead:
        nmr_Util::addImage((*im_2D), (*new_image), (double)d_resampleRatio,
                  1.0-(double)d_resampleRatio);

    } else {                      // warp the SEM image to the AFM image
      if (d_constrainToTopography){
      // resample the SEM data onto the AFM grid only
        // it is possible to change resolution or select a subregion
        // of the height field at this point by just changing what
        // resolution we give to the constructor and what image
        // bounds we set and everything should just work correctly:
        new_image = new nmb_ImageGrid(
                (const char *)(d_newResampleImageName.string()),
                (const char *)(im_2D->unitsValue()),
                d_resampleResolutionX, d_resampleResolutionY);
        nmb_ImageBounds im3D_bounds;
        im_3D->getBounds(im3D_bounds);
        new_image->setBounds(im3D_bounds);
        TopoFile tf;
        im_3D->getTopoFileInfo(tf);
        new_image->setTopoFileInfo(tf);
        d_newResampleImageName = (const char *) "";
        /*
          projWorldFromTopoWorld = projWorldFromScaledProjIm*
                                   d_scaledProjImFromScaledTopoIm*
                                   scaledTopoImFromTopoWorld
        */
        nmb_TransformMatrix44 projImageFromTopoImage;
        if (nmr_Util::computeResampleTransformInImageCoordinates(im_2D, im_3D,
                d_scaledProjImFromScaledTopoIm[displayedTransformIndex],
                projImageFromTopoImage)) {
          fprintf(stderr,
              "createResampleImage failed: worldFromProjIm not invertible\n");
          return;
        }
        
		double intensityRange = im_2D->maxNonZeroValue() - im_2D->minNonZeroValue();
		double nonZeroIntensityOffset = -(im_2D->minNonZeroValue()-0.01*intensityRange);
        nmr_Util::createResampledImageWithImageSpaceTransformation(
            (*im_2D),
            projImageFromTopoImage, 
            (*new_image), nonZeroIntensityOffset);
      } else {
        
        // projWorldFromTopoWorld = identity because the transforms have 
	// already been adjusted inside the nmb_Image objects to make them
        // their world coordinate systems identical

        nmb_TransformMatrix44 projWorldFromTopoWorld;       
        nmb_TransformMatrix44 projImageFromTopoImage;
        if (nmr_Util::computeResampleTransformInImageCoordinates(im_2D, im_3D,
                d_scaledProjImFromScaledTopoIm[displayedTransformIndex],
                projImageFromTopoImage)){
          fprintf(stderr, 
              "createResampleImage failed: worldFromProjIm not invertible\n");
          return;
        }

        // here we figure out what resolution is required to preserve
        // the entire 2D projection image while keeping the resolution of
        // height field constant
        int min_i, min_j, max_i, max_j;
        nmr_Util::computeResampleExtents((*im_3D), (*im_2D), 
            projImageFromTopoImage, 
            min_i, min_j, max_i, max_j);
        int res_x = max_i - min_i;
        int res_y = max_j - min_j;
        //printf("got extents: %d x %d\n", res_x, res_y);

        new_image = new nmb_ImageGrid(
                (const char *)(d_newResampleImageName.string()),
                (const char *)(im_2D->unitsValue()),
                res_x, res_y);
        d_newResampleImageName = (const char *) "";
        //printf("allocated image\n");
        TopoFile tf;
        im_3D->getTopoFileInfo(tf);
        new_image->setTopoFileInfo(tf);
        //printf("%d, %d, %d, %d\n", min_i, min_j, max_i, max_j);
        nmr_Util::setRegionRelative((*im_3D), (*new_image),
                 min_i, min_j, max_i, max_j);
		double intensityRange = im_2D->maxNonZeroValue() - im_2D->minNonZeroValue();
		double nonZeroIntensityOffset = -(im_2D->minNonZeroValue()-0.01*intensityRange);
        nmr_Util::createResampledImageWithImageSpaceTransformation((*im_2D), 
                 (*im_3D), projImageFromTopoImage, (*new_image),
				 nonZeroIntensityOffset);

        // HACK:
        // an extra step: combine the two datasets in a somewhat arbitrary
        // way so that you can see features from both in a single image
        // it would be nice to create a color image from these two instead:
        nmr_Util::addImage((*im_3D), (*new_image), (double)d_resampleRatio,
                  1.0-(double)d_resampleRatio);

      }
    }
    //printf("finished resampling image\n");
    // now make it available elsewhere:
    d_dataset->dataImages()->addImage(new_image);
}

void nmr_RegistrationUI::createResamplePlane(const char * /*imageName */)
{
    //printf("nmr_RegistrationUI::createResamplePlane\n");
    int displayedTransformIndex = getDisplayedTransformIndex();
    nmb_ImageGrid *new_image;
    if (!d_dataset) return;
    nmb_Image *im_3D = d_dataset->dataImages()->getImageByName
                          (d_registrationImageName3D.string());
    nmb_Image *im_2D = d_dataset->dataImages()->getImageByName
                          (d_registrationImageName2D.string());

    // d_imageTransformWorldSpace is the transformation that takes points from
    // height image to points in the texture image

    // Make sure we have all the information we need and
    // see if the user has given a name to the resampled plane
    // other than "".  If so, we should create a new plane and set the value
    // back to "".
    if (!im_3D || !im_2D ||
               (strlen(d_newResamplePlaneName.string()) == 0)){
        return;

    } 
    // resample the SEM data onto the AFM grid only 
    //Region is the same as the AFM grid, resolution is the same as the AFM
    //grid
        new_image = new nmb_ImageGrid(
                (const char *)(d_newResamplePlaneName.string()),
                (const char *)(im_2D->unitsValue()),
                im_3D->width(), im_3D->height());
        nmb_ImageBounds im3D_bounds;
        im_3D->getBounds(im3D_bounds);
        new_image->setBounds(im3D_bounds);
        TopoFile tf;
        im_3D->getTopoFileInfo(tf);
        new_image->setTopoFileInfo(tf);
        d_newResamplePlaneName = (const char *) "";
        nmb_TransformMatrix44 projImageFromTopoImage;
        if (nmr_Util::computeResampleTransformInImageCoordinates(im_2D, im_3D,
            d_scaledProjImFromScaledTopoIm[displayedTransformIndex], 
            projImageFromTopoImage)) {
           fprintf(stderr, 
                 "createResamplePlane Error, non-invertible transform\n");
           return;
        }

		double intensityRange = im_2D->maxNonZeroValue() - im_2D->minNonZeroValue();
		double nonZeroIntensityOffset = -(im_2D->minNonZeroValue()-0.01*intensityRange);
        nmr_Util::createResampledImageWithImageSpaceTransformation(
                 (*im_2D),
                 projImageFromTopoImage, (*new_image),
				 nonZeroIntensityOffset);

    //printf("finished resampling image\n");
    // now make it available elsewhere:
    // this is the code from nmb_Dataset::addImageToGrid(new_image) which
    // had to be copied here now that I've removed dependence on nmb_Dataset
    
    d_dataset->addImageToGrid(new_image);
    //d_imageList->addImage(new_image);
    nmb_Image::deleteImage(new_image);
}

