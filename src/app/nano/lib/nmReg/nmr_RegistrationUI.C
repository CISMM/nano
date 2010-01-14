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

	rewind (pFile);
	int num;
	fscanf (pFile, "%d", &num);

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
		printf ( "%f %f ", x, y);
	}
	printf ("\n");

	printf ("Projection: ");
	for(int i = 0; i<num; i++)
	{
		fscanf (pFile, "%f %f", &x, &y);
		x_tgt[i] = x;
		y_tgt[i] = y;
		z_tgt[i] = 0;
		printf ( "%f %f ", x, y);
	}
	printf ("\n");

	nmr_RegistrationUI *me = static_cast<nmr_RegistrationUI *>(ud);
	me->d_aligner->get_d_local_impl()->get_d_alignerUI()->setFiducial(replace,num,x_src,y_src,z_src,x_tgt,y_tgt,z_tgt);
	
	fclose (pFile);
		printf("save here \n");
} //new

void nmr_RegistrationUI::handle_saveReport_change(vrpn_int32 value, void *ud) //(vrpn_float64 /*value*/, void *ud)
{
	nmr_RegistrationUI *me = static_cast<nmr_RegistrationUI *>(ud);

	FILE * pFile;
	pFile = fopen ("output/methods section.txt","w");
 
	fprintf(pFile,"rms error between images: %f\n", me->d_aligner->get_d_local_impl()->get_rmsError());

	fclose (pFile);

} //new

void nmr_RegistrationUI::handle_runRansac_change(vrpn_int32 value, void *ud)
{
	nmr_RegistrationUI *me = static_cast<nmr_RegistrationUI *>(ud);

//	Correspondence	c;
//	int	src, tgt;
//	me->d_aligner->get_d_local_impl()->get_d_alignerUI()->getCorrespondence(c, src, tgt);
	me->d_aligner->get_d_local_impl()->get_d_alignerUI()->readPixels();

} //new

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

