#include "controlPanels.h"

ControlPanels::ControlPanels(PatternEditor *pe,
                             nmr_Registration_Proxy *rp,
                             nmm_Microscope_SEM_Remote *sem):
   d_imageNames(new Tclvar_list_of_strings()),
   d_lineWidth_nm("line_width_nm", 0),
   d_exposure_uCoulombs_per_square_cm("exposure_uCoulombs_per_square_cm", 0),
   d_drawingTool("drawing_tool", 1),
   d_clearDrawing("clear_drawing", 0),

   d_imageColorChanged("image_color_changed", 0),
   d_imageRed("image_r", 255),
   d_imageGreen("image_g", 255),
   d_imageBlue("image_b", 120),
   d_imageOpacity("image_opacity", 50.0),
   d_hideOtherImages("hide_other_images", 0),
   d_enableImageDisplay("enable_image_display", 0),
   d_currentImage("current_image", "none"),

   d_clearAlignPoints("clear_align_points", 0),
   d_alignmentNeeded("alignment_needed", 0),
   d_sourceImageName("source_image_name", "none"),
   d_targetImageName("target_image_name", "none"),
   d_alignWindowOpen("align_window_open", 0),

   d_semWindowOpen("sem_window_open", 0),
   d_semAcquireImagePushed("sem_acquire_image", 0),
   d_semAcquireContinuousChecked("sem_acquire_continuous", 0),
   d_semPixelIntegrationTime_nsec("sem_pixel_integration_time_nsec", 0),
   d_semInterPixelDelayTime_nsec("sem_inter_pixel_delay_time_nsec", 0),
   d_semResolution("sem_resolution", 1),

   d_semBeamWidth_nm("sem_beam_width_nm", 200),
   d_semBeamCurrent_uA("sem_beam_current_uAmps", 1),
   d_semBeamExposePushed("sem_beam_expose_now", 0),

   d_patternEditor(pe),
   d_aligner(rp),
   d_SEM(sem),
   d_imageList(NULL)
{
  d_imageNames->initializeTcl("imageNames");
  setupCallbacks();
}

ControlPanels::~ControlPanels()
{
   delete d_imageNames;
}

void ControlPanels::setImageList(nmb_ImageList *data)
{
  d_imageList = data;

  printf("setImageList: initializing source to %s\n",
                              (const char *)d_sourceImageName);
  printf("              initializing target to %s\n",
                              (const char *)d_targetImageName);

  nmb_Image *src_im = 
        d_imageList->getImageByName((const char *)d_sourceImageName);
  nmb_Image *tgt_im =
        d_imageList->getImageByName((const char *)d_targetImageName);
  if (!src_im) {
     fprintf(stderr, "source image not found\n");
     return;
  }
  if (!tgt_im) {
     fprintf(stderr, "target image not found\n");
     return;
  }
  // send the images off to the proxy
  d_aligner->setImage(NMR_SOURCE, src_im, vrpn_FALSE);
  d_aligner->setImage(NMR_TARGET, tgt_im, vrpn_FALSE);
}

nmb_ListOfStrings *ControlPanels::imageNameList()
{
   return (nmb_ListOfStrings*)d_imageNames;
}

void ControlPanels::setupCallbacks()
{
  // control panel variable callbacks
  d_lineWidth_nm.addCallback(handle_lineWidth_nm_change, this);
  d_exposure_uCoulombs_per_square_cm.addCallback(handle_exposure_change, this);
  d_drawingTool.addCallback(handle_drawingTool_change, this);
  d_clearDrawing.addCallback(handle_clearDrawing_change, this);

  d_imageColorChanged.addCallback(handle_imageColorChanged_change, this);
  d_imageOpacity.addCallback(handle_imageOpacity_change, this);
  d_hideOtherImages.addCallback(handle_hideOtherImages_change, this);
  d_enableImageDisplay.addCallback(handle_enableImageDisplay_change, this);
  d_currentImage.addCallback(handle_currentImage_change, this);

  d_clearAlignPoints.addCallback(handle_clearAlignPoints_change, this);
  d_alignmentNeeded.addCallback(handle_alignmentNeeded_change, this);
  d_sourceImageName.addCallback(handle_sourceImageName_change, this);
  d_targetImageName.addCallback(handle_targetImageName_change, this);
  d_alignWindowOpen.addCallback(handle_alignWindowOpen_change, this);

  d_semWindowOpen.addCallback(handle_semWindowOpen_change, this);
  d_semAcquireImagePushed.addCallback(handle_semAcquireImagePushed_change, 
                                      this);
  d_semAcquireContinuousChecked.addCallback(
               handle_semAcquireContinuousChecked_change, this);
  d_semPixelIntegrationTime_nsec.addCallback(
         handle_semPixelIntegrationTime_change, this);
  d_semInterPixelDelayTime_nsec.addCallback(
         handle_semInterPixelDelayTime_change, this);
  d_semResolution.addCallback(handle_semResolution_change, this);

  d_semBeamWidth_nm.addCallback(handle_semBeamWidth_change, this);
  d_semBeamCurrent_uA.addCallback(handle_semBeamCurrent_change, this);
  d_semBeamExposePushed.addCallback(
         handle_semBeamExposePushed_change, this);

  // other types of callbacks
  d_aligner->registerChangeHandler((void *)this, handle_registration_change);
}

// static
void ControlPanels::handle_lineWidth_nm_change(double new_value, void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
  printf("lineWidth: %g\n", (double)(me->d_lineWidth_nm));
}

// static 
void ControlPanels::handle_exposure_change(double new_value, void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
  printf("exposure: %g\n", (double)(me->d_exposure_uCoulombs_per_square_cm));
}

// static
void ControlPanels::handle_drawingTool_change(int new_value, void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
  printf("tool: %d\n", (int)(me->d_drawingTool));
}

// static
void ControlPanels::handle_clearDrawing_change(int new_value, void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
  printf("clear drawing: %d\n", (int)(me->d_clearDrawing));
}

// static
void ControlPanels::handle_imageColorChanged_change(int new_value, void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
  printf("color: %d,%d,%d\n", 
         (int)me->d_imageRed, (int)me->d_imageGreen, (int)me->d_imageBlue);
}

// static
void ControlPanels::handle_imageOpacity_change(double new_value, void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
  printf("opacity: %g\n", (double)me->d_imageOpacity);
}

// static 
void ControlPanels::handle_hideOtherImages_change(int new_value, void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
  printf("hide others: %d\n", (int)me->d_hideOtherImages);
}

// static
void ControlPanels::handle_enableImageDisplay_change(int new_value, void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
  printf("enabled: %d\n", (int)me->d_enableImageDisplay);

  // lookup the current image by name in the nmb_ImageList
  if (me->d_imageList == NULL) {
     fprintf(stderr, "handle_enableImageDisplay_change: d_imageList NULL\n");
     return;
  } 
  nmb_Image *currImage = me->d_imageList->getImageByName(
                         BCString((const char *)(me->d_currentImage)));
  if (!currImage) {
     fprintf(stderr, "handle_enableImageDisplay_change: image not found\n");
     return;
  }

  // tell the pattern editor to enable this image
  if ((int)me->d_enableImageDisplay == 0){
    me->d_patternEditor->setImageEnable(currImage, vrpn_FALSE);
  } else {
    me->d_patternEditor->setImageEnable(currImage, vrpn_TRUE);
  }
}

// static
void ControlPanels::handle_currentImage_change(const char *new_value, void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
  printf("current image: %s\n", (const char *)me->d_currentImage);
/*
for when d_currentImage changes:
  set d_imageRed, d_imageGreen, d_imageBlue, d_imageOpacity,
  d_enableImageDisplay from the patternEditor's settings for this
  image - use d_patternEditor->getImageColor(const nmb_Image *im, r,g,b)
                               getImageOpacity(im, opacity)
                               getImageEnable(im, enable)

  if (d_hideOtherImages) then tell d_patternEditor that the current image
  changed using d_patternEditor->showSingleImage(nmb_Image *im);
*/

}

// static
void ControlPanels::handle_clearAlignPoints_change(int new_value, void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
  printf("clear align points: %d\n",(int)me->d_clearAlignPoints);

  me->d_clearAlignPoints = 0;
}

// static
void ControlPanels::handle_alignmentNeeded_change(int new_value, void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
  printf("alignment request: %d\n",(int)me->d_alignmentNeeded);
  if (new_value) {
    me->d_aligner->registerImages();
  }
  me->d_alignmentNeeded = 0;
}

// static
void ControlPanels::handle_registration_change(void *ud, 
                        const nmr_ProxyChangeHandlerData &info)
{
  ControlPanels *me = (ControlPanels *)ud;
  me->handleRegistrationChange(info);
}

void ControlPanels::handleRegistrationChange
                        (const nmr_ProxyChangeHandlerData &info)
{
  switch(info.msg_type) {
    case NMR_IMAGE_PARAM:
      nmr_ImageType which_image;
      vrpn_int32 res_x, res_y;
      vrpn_bool height_field;
      d_aligner->getImageParameters(which_image, res_x, res_y, height_field);
      // this is just a confirmation of settings so we probably don't need 
      // to do anything
      break;
    case NMR_TRANSFORM_OPTION:
      nmr_TransformationType xform_type;
      d_aligner->getTransformationOptions(xform_type);
      // this is just a confirmation of settings so we probably don't need 
      // to do anything
      break;
    case NMR_REG_RESULT:
      printf("got transformation\n");

      /* *******************************************************************
         Store the raw result from the registration code and compute inverse
       * *******************************************************************/
      double targetImFromSourceIm_matrix[16];
      d_aligner->getRegistrationResult(targetImFromSourceIm_matrix);

      if (d_imageList == NULL) {
         fprintf(stderr, "handleRegistrationChange: Error, image list null\n");
         return;
      }
      nmb_Image *targetImage = d_imageList->getImageByName
                          (d_targetImageName.string());
      if (!targetImage) {
           fprintf(stderr, "handleRegistrationChange: can't find image\n");
           return;
      }

      targetImage->setWorldToImageTransform(targetImFromSourceIm_matrix);
      // now tell pattern editor that the transform for this image changed
      d_patternEditor->newPosition(targetImage);
      break;
  }
}

// static
void ControlPanels::handle_sourceImageName_change(
                            const char *new_value, void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
  printf("source image: %s\n",(const char *)me->d_sourceImageName);
  if (me->d_imageList == NULL) {
      return;
  }
  nmb_Image *im = me->d_imageList->getImageByName(new_value);
  if (!im) {
     fprintf(stderr, "image not found: %s\n", new_value);
     return;
  }
  // send the image off to the proxy
  me->d_aligner->setImage(NMR_SOURCE, im, vrpn_FALSE);
}

// static
void ControlPanels::handle_targetImageName_change(const char *new_value, void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
  printf("target image: %s\n",(const char *)me->d_targetImageName);
  if (me->d_imageList == NULL){
      return;
  }
  nmb_Image *im = me->d_imageList->getImageByName(new_value);
  if (!im) {
     fprintf(stderr, "image not found: %s\n", new_value);
     return;
  }
  // send image off to the proxy
  me->d_aligner->setImage(NMR_TARGET, im, vrpn_FALSE);
}

// static
void ControlPanels::handle_alignWindowOpen_change(int new_value, void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
  printf("align window open: %d\n",(int)me->d_alignWindowOpen);

  if (new_value) {
    me->d_aligner->setGUIEnable(vrpn_TRUE);
  } else {
    me->d_aligner->setGUIEnable(vrpn_FALSE);
  }

}

// static
void ControlPanels::handle_semWindowOpen_change(int new_value, void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
  printf("sem window open: %d\n",(int)me->d_semWindowOpen);
}

// static
void ControlPanels::handle_semAcquireImagePushed_change(int new_value, void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
  printf("sem acquire image pushed: %d\n",(int)me->d_semAcquireImagePushed);
}

// static
void ControlPanels::handle_semAcquireContinuousChecked_change(
                         int new_value, void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
  printf("sem acquire continuous: %d\n",
                      (int)me->d_semAcquireContinuousChecked);
}

// static
void ControlPanels::handle_semPixelIntegrationTime_change(
                    int new_value, void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
  printf("sem pixel integrate time: %d\n",
                        (int)me->d_semPixelIntegrationTime_nsec);
}

// static
void ControlPanels::handle_semInterPixelDelayTime_change(
                    int new_value, void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
  printf("sem interpix delay time: %d\n",
                       (int)me->d_semInterPixelDelayTime_nsec);
}

// static
void ControlPanels::handle_semResolution_change(int new_value, void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
  printf("sem res: %d\n",(int)me->d_semResolution);
}

// static
void ControlPanels::handle_semBeamWidth_change(double new_value, void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
  printf("sem beam width: %g\n",(double)me->d_semBeamWidth_nm);
}


// static
void ControlPanels::handle_semBeamCurrent_change(double new_value, void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
  printf("sem beam current: %g\n",(double)me->d_semBeamCurrent_uA);
}

// static
void ControlPanels::handle_semBeamExposePushed_change(int new_value, void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
  printf("sem beam expose: %d\n",(int)me->d_semBeamExposePushed);
}

/*

some notes on actions in response to changes in control variables:

d_currentImage:
  set d_imageRed, d_imageGreen, d_imageBlue, d_imageOpacity,
  d_enableImageDisplay from the patternEditor's settings for this
  image - use d_patternEditor->getImageColor(const nmb_Image *im, r,g,b)
                               getImageOpacity(im, opacity)
                               getImageEnable(im, enable)

  if (d_hideOtherImages) then tell d_patternEditor that the current image
  changed using d_patternEditor->showSingleImage(nmb_Image *im);

d_hideOtherImages:
  do d_patternEditor->showSingleImage(nmb_Image *im);

d_imageColorChanged:
  d_patternEditor->setImageColor(const nmb_Image *im, r,g,b)

d_imageOpacity:
  d_patternEditor->setImageOpacity(im, opacity);

d_enableImageDisplay:
  d_patternEditor->setImageEnable(im, enabled);

*/

/*
drawing controls:

d_lineWidth_nm:
  d_patternEditor->setLineWidth(width)


d_exposure...:
  d_patternEditor->setExposure(exposure)

d_drawMode: (for now, either polyline or polygon)
  d_patternEditor->setDrawMode(mode)
*/



