#include "controlPanels.h"
#include "imageViewer.h"
#include "nmm_EDAX.h"
#include "nmb_TransformMatrix44.h"

static int expose_point_count = 0;

ControlPanels::ControlPanels(PatternEditor *pe,
                             nmr_Registration_Proxy *rp,
                             nmm_Microscope_SEM_Remote *sem):
   d_imageNames(new Tclvar_list_of_strings()),
   d_bufferImageFormatList(new Tclvar_list_of_strings()),
   d_openImageFileName("open_image_filename", ""),
   d_saveImageFileName("save_image_filename", ""),
   d_saveImageFileType("save_image_filetype", ""),
   d_saveImageName("export_plane", ""),
   d_saveImageFormatList(new Tclvar_list_of_strings()),
   d_bufferImageFileName("bufferImage_filename", ""),
   d_bufferImageFormat("bufferImage_format", ""),
   d_lineWidth_nm("line_width_nm", 0),
   d_exposure_uCoulombs_per_square_cm("exposure_uCoulombs_per_square_cm", 300),
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

//   d_clearAlignPoints("clear_align_points", 0),
   d_alignmentNeeded("alignment_needed", 0),
   d_sourceImageName("source_image_name", "none"),
   d_targetImageName("target_image_name", "none"),
   d_resampleResolutionX("resample_resolution_x", 640),
   d_resampleResolutionY("resample_resolution_y", 480),
   d_resampleImageName("resample_image_name", "none"),
   d_alignWindowOpen("align_window_open", 0),

   d_semWindowOpen("sem_window_open", 0),
   d_semAcquireImagePushed("sem_acquire_image", 0),
   d_semAcquireContinuousChecked("sem_acquire_continuous", 0),
   d_semPixelIntegrationTime_nsec("sem_pixel_integration_time_nsec", 0),
   d_semInterPixelDelayTime_nsec("sem_inter_pixel_delay_time_nsec", 0),
   d_semResolution("sem_resolution", 1),
   d_semAcquisitionMagnification("sem_acquisition_magnification", 10000),
   d_semOverwriteOldData("sem_overwrite_old_data", 1),
   d_semBeamBlankEnable("sem_beam_blank_enable", 0),
   d_semHorizRetraceDelay_nsec("sem_horiz_retrace_delay_nsec", 0),
   d_semVertRetraceDelay_nsec("sem_vert_retrace_delay_nsec", 0),
   d_semXDACGain("sem_x_dac_gain", 16384), 
   d_semXDACOffset("sem_x_dac_offset", 0),
   d_semYDACGain("sem_y_dac_gain", 16384),
   d_semYDACOffset("sem_y_dac_offset", 0),
   d_semZADCGain("sem_z_adc_gain", 16384),
   d_semZADCOffset("sem_z_adc_offset", 0),
   d_semExternalScanControlEnable("sem_external_scan_control_enable", 0),
   d_semDataBuffer("sem_data_buffer", "none"),
   d_semBufferImageNames(new Tclvar_list_of_strings()),

   d_semExposureMagnification("sem_exposure_magnification", 10000),
   d_semBeamWidth_nm("sem_beam_width_nm", 200),
   d_semBeamCurrent_picoAmps("sem_beam_current_picoAmps", 1),
   d_semBeamExposePushed("sem_beam_expose_now", 0),

   d_patternEditor(pe),
   d_aligner(rp),
   d_SEM(sem),
   d_exposureManager(new ExposureManager()),
   d_imageList(NULL)
{
  int i;
  d_imageNames->initializeTcl("imageNames");
  d_bufferImageFormatList->initializeTcl("bufferImage_format_list");
  d_semBufferImageNames->initializeTcl("sem_bufferImageNames");
  for (i = 0; i < ImageType_count; i++)
    d_bufferImageFormatList->addEntry(ImageType_names[i]);

  d_saveImageFormatList->initializeTcl("save_image_format_list");

  setupCallbacks();
  ImageViewer *image_viewer = ImageViewer::getImageViewer();
  d_semWinID = image_viewer->createWindow(NULL, 10, 10, 
               100, 100, "SEM", GL_UNSIGNED_BYTE);
  if (d_semWinID == 0) {
    fprintf(stderr, "Error creating sem window\n");
  }
  image_viewer->setWindowDisplayHandler(d_semWinID,
         handle_semWindowRedraw, this);
  for (i = 0; i < EDAX_NUM_SCAN_MATRICES; i++){
    imageCount[i] = 0;
  }
  handle_semAcquisitionMagnification_change(
          (int)d_semAcquisitionMagnification, (void *)this);
  d_patternEditor->setDrawingParameters((double)(d_lineWidth_nm),
                    (double)(d_exposure_uCoulombs_per_square_cm));
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
  d_aligner->setImage(NMR_SOURCE, src_im);
  d_aligner->setImage(NMR_TARGET, tgt_im);
  updateCurrentImageControls();
}

nmb_ListOfStrings *ControlPanels::imageNameList()
{
   return (nmb_ListOfStrings*)d_imageNames;
}

void ControlPanels::displayDwellTimes()
{
  double beam_width_nm = (double)(d_semBeamWidth_nm);
  double beamCurrent = (double)(d_semBeamCurrent_picoAmps);
  double exposure = (double)(d_exposure_uCoulombs_per_square_cm);

  d_exposureManager->setColumnParameters(100e-6,
										beam_width_nm,
										beamCurrent);
  d_exposureManager->setExposure(exposure);
  double line_dwell_sec, area_dwell_sec;
  d_exposureManager->getDwellTimes(line_dwell_sec, area_dwell_sec);
  printf("line: %g usec, area: %g usec\n", line_dwell_sec*1e6, area_dwell_sec*1e6);

}

void ControlPanels::setupCallbacks()
{
  // control panel variable callbacks
  d_bufferImageFileName.addCallback(handle_bufferImageFileName_change, this);
  d_openImageFileName.addCallback(handle_openImageFileName_change, this);
  d_saveImageFileName.addCallback(handle_saveImageFileName_change, this);
  d_saveImageName.addCallback(handle_saveImageName_change, this);

  d_lineWidth_nm.addCallback(handle_lineWidth_nm_change, this);
  d_exposure_uCoulombs_per_square_cm.addCallback(handle_exposure_change, this);
  d_drawingTool.addCallback(handle_drawingTool_change, this);
  d_clearDrawing.addCallback(handle_clearDrawing_change, this);

  d_imageColorChanged.addCallback(handle_imageColorChanged_change, this);
  d_imageOpacity.addCallback(handle_imageOpacity_change, this);
  d_hideOtherImages.addCallback(handle_hideOtherImages_change, this);
  d_enableImageDisplay.addCallback(handle_enableImageDisplay_change, this);
  d_currentImage.addCallback(handle_currentImage_change, this);

//  d_clearAlignPoints.addCallback(handle_clearAlignPoints_change, this);
  d_alignmentNeeded.addCallback(handle_alignmentNeeded_change, this);
  d_sourceImageName.addCallback(handle_sourceImageName_change, this);
  d_targetImageName.addCallback(handle_targetImageName_change, this);
  d_resampleImageName.addCallback(handle_resampleImageName_change, this);
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
  d_semAcquisitionMagnification.addCallback(
                          handle_semAcquisitionMagnification_change, this);
  d_semBeamBlankEnable.addCallback(
                          handle_semBeamBlankEnable_change, this);
  d_semHorizRetraceDelay_nsec.addCallback(
                          handle_semHorizRetraceDelay_change, this);
  d_semVertRetraceDelay_nsec.addCallback(
                          handle_semVertRetraceDelay_change, this);

  d_semXDACGain.addCallback(handle_semDACParams_change, this);
  d_semXDACOffset.addCallback(handle_semDACParams_change, this);
  d_semYDACGain.addCallback(handle_semDACParams_change, this);
  d_semYDACOffset.addCallback(handle_semDACParams_change, this);
  d_semZADCGain.addCallback(handle_semDACParams_change, this);
  d_semZADCOffset.addCallback(handle_semDACParams_change, this);
  d_semExternalScanControlEnable.addCallback(
                          handle_semExternalScanControlEnable_change, this);

  d_semExposureMagnification.addCallback(
                          handle_semExposureMagnification_change, this);
  d_semBeamWidth_nm.addCallback(handle_semBeamWidth_change, this);
  d_semBeamCurrent_picoAmps.addCallback(handle_semBeamCurrent_change, this);
  d_semBeamExposePushed.addCallback(
         handle_semBeamExposePushed_change, this);

  // other types of callbacks
  d_aligner->registerChangeHandler((void *)this, handle_registration_change);

  if (d_SEM) {
    d_SEM->registerChangeHandler((void *)this, handle_sem_change);
  }
}

//static
void ControlPanels::handle_openImageFileName_change(const char * /*new_value*/,
                                              void *ud)
{
  double default_matrix[16] = {0.001, 0.0, 0.0, 0.0,
                               0.0, 0.001, 0.0, 0.0,
                               0.0, 0.0, 1.0, 0.0,
                               0.0, 0.0, 0.0, 1.0};
  ControlPanels *me = (ControlPanels *)ud;
  double imageRegionWidth, imageRegionHeight;
  me->d_SEM->getScanRegion_nm(imageRegionWidth, imageRegionHeight);
  if (imageRegionWidth != 0 && imageRegionHeight != 0) {
    default_matrix[0] = 2.0/imageRegionWidth;
    default_matrix[5] = 2.0/imageRegionHeight;
  }
  printf("open file %s\n", (const char *)me->d_openImageFileName);
  if (strlen((const char *)me->d_openImageFileName) <= 0) return;

  if (nmb_ImageGrid::openFile((const char *)me->d_openImageFileName)) {
    return;
  }
  nmb_ImageGrid *im = nmb_ImageGrid::getNextImage();
  while (im) {
    // add im to the list
    im->normalize();
    im->setWorldToImageTransform(default_matrix);

    me->d_patternEditor->addImage(im);
    me->d_imageList->addImage(im);
	
    im = nmb_ImageGrid::getNextImage();
  }
}

//static 
void ControlPanels::handle_bufferImageFileName_change(
                                              const char * /*new_value*/,
                                              void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
  printf("save to file %s with format %s\n", 
                              (const char *)me->d_bufferImageFileName,
                              (const char *)me->d_bufferImageFormat);
  ImageType filetype = TIFFImageType;

  if (strcmp(ImageType_names[TIFFImageType], 
      (const char *)me->d_bufferImageFormat) == 0) {
     filetype = TIFFImageType;
  } else if (strcmp(ImageType_names[PNMImageType],
      (const char *)me->d_bufferImageFormat) == 0) {
     filetype = PNMImageType;
  }
  me->d_patternEditor->saveImageBuffer(
                             (const char *)me->d_bufferImageFileName,
                             filetype);
}

//static
void ControlPanels::handle_saveImageName_change(const char * /*new_value*/,
                                                void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
  // figure out what the valid file types are for this image and set 
  // the type list accordingly
  nmb_Image *im = me->d_imageList->getImageByName(
                          (const char *)me->d_saveImageName);
  if (!im) {
    fprintf(stderr, "error, couldn't find image: %s\n",
                          (const char *)me->d_saveImageName);
    return;
  }
  
  me->d_saveImageFormatList->copyList(im->exportFormatNames());
}

//static
void ControlPanels::handle_saveImageFileName_change(const char * /*new_value*/,
                                                    void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
  printf("save plane %s to file %s with format %s\n",
                    (const char *)me->d_saveImageName,
                    (const char *)me->d_saveImageFileName,
                    (const char *)me->d_saveImageFileType);

  if (strlen(me->d_saveImageFileName) > 0) {
      // Find which plane we are going to export.
      nmb_Image *im = me->d_imageList->getImageByName
                                      (me->d_saveImageName.string());
      if (im == NULL) {
        fprintf(stderr, "Couldn't find this data to save: %s\n",
                me->d_saveImageName.string());
        return;
      }

      // find out which type of file we are writing, and write it.

      FILE *file_ptr;
      // "wb" stands for write binary - so it should work on PC platforms, too.
      file_ptr=fopen(me->d_saveImageFileName.string(),"wb");
      if(file_ptr==NULL){
          fprintf(stderr, "Couldn't open this file: %s\n"
                                "Please try another name or directory",
                                me->d_saveImageFileName.string());
          return;
      }

      if (im->exportToFile(file_ptr, me->d_saveImageFileType.string(),
                              me->d_saveImageFileName.string())) {
          fprintf(stderr, "Couldn't write to this file: %s\n"
                                "Please try another name or directory",
                                me->d_saveImageFileName.string());
        fclose(file_ptr);
        return;
      }
      fclose(file_ptr);

  }
  me->d_saveImageFileName = "";
}

// static
void ControlPanels::handle_lineWidth_nm_change(double /*new_value*/, void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
  printf("lineWidth: %g\n", (double)(me->d_lineWidth_nm));
  me->d_patternEditor->setDrawingParameters((double)(me->d_lineWidth_nm),
                    (double)(me->d_exposure_uCoulombs_per_square_cm));
}

// static 
void ControlPanels::handle_exposure_change(double /*new_value*/, void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
//  printf("exposure: %g\n", (double)(me->d_exposure_uCoulombs_per_square_cm));
  me->d_patternEditor->setDrawingParameters((double)(me->d_lineWidth_nm),
                    (double)(me->d_exposure_uCoulombs_per_square_cm));
  me->displayDwellTimes();
}

// static
void ControlPanels::handle_drawingTool_change(int new_value, void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
  printf("tool: %d\n", (int)(me->d_drawingTool));
  PE_DrawTool tool;
  if (new_value == 1) {
    // polyline
    tool = PE_POLYLINE;
  } else if (new_value == 2){
    // polygon
    tool = PE_POLYGON;
  } else if (new_value == 3){
    tool = PE_DUMP_POINT;
  } else {
    tool = PE_SELECT;
  }
  me->d_patternEditor->setDrawingTool(tool);
}

// static
void ControlPanels::handle_clearDrawing_change(int /*new_value*/, void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
  printf("clear drawing: %d\n", (int)(me->d_clearDrawing));
  me->d_patternEditor->clearShape();
}

// static
void ControlPanels::handle_imageColorChanged_change(int /*new_value*/, void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
  printf("color: %d,%d,%d\n", 
         (int)me->d_imageRed, (int)me->d_imageGreen, (int)me->d_imageBlue);

  nmb_Image *im = me->d_imageList->getImageByName(
              BCString((const char *)(me->d_currentImage)));
  if (!im) {
    fprintf(stderr, "handle_imageColor: Error, could not find image\n");
    return;
  }
  double r, g, b;
  r = (double)((int)(me->d_imageRed))/255.0;
  g = (double)((int)(me->d_imageGreen))/255.0;
  b = (double)((int)(me->d_imageBlue))/255.0;
  me->d_patternEditor->setImageColor(im, r,g,b);
}

// static
void ControlPanels::handle_imageOpacity_change(double new_value, void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
  printf("opacity: %g\n", (double)me->d_imageOpacity);
  nmb_Image *im = me->d_imageList->getImageByName(
                BCString((const char *)(me->d_currentImage)));
  if (!im) {
    fprintf(stderr, "handle_imageOpacity: Error, could not find image\n");
    return;
  }
  me->d_patternEditor->setImageOpacity(im, new_value);
}

// static 
void ControlPanels::handle_hideOtherImages_change(int new_value, void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
  printf("hide others: %d\n", (int)me->d_hideOtherImages);
  if (new_value) {
    nmb_Image *im = me->d_imageList->getImageByName(
                BCString((const char *)(me->d_currentImage)));
    if (!im) {
      fprintf(stderr, "handle_hideOtherImages: Error, could not find image\n");
      return;
    }
    me->d_patternEditor->showSingleImage(im);
  } else {
    me->d_patternEditor->showSingleImage(NULL);
  }
}

// static
void ControlPanels::handle_enableImageDisplay_change(int /*new_value*/, 
                                                     void *ud)
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
void ControlPanels::handle_currentImage_change(const char * /*new_value*/, 
                                               void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
  me->updateCurrentImageControls();
}

void ControlPanels::updateCurrentImageControls()
{
  if (!d_imageList) return;

  printf("current image: %s\n", (const char *)d_currentImage);
  nmb_Image *im = d_imageList->getImageByName(
                  BCString((const char *)(d_currentImage)));
  if (!im) {
    fprintf(stderr, "handle_currentImage_change: Error, couldn't find image\n");
  }
  ImageElement *ie = d_patternEditor->getImageParameters(im);
  if (!ie) {
	fprintf(stderr, "Error, couldn't update current image\n");
	return;
  }
  d_imageRed = (int)(255*ie->d_red);
  d_imageGreen = (int)(255*ie->d_green);
  d_imageBlue = (int)(255*ie->d_blue);
  d_imageColorChanged = 1;
  d_imageOpacity = ie->d_opacity;
  d_enableImageDisplay = ie->d_enabled;
  if (d_hideOtherImages) {
    d_patternEditor->showSingleImage(im);
  }
}

/*
// static
void ControlPanels::handle_clearAlignPoints_change(int new_value, void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
  printf("clear align points: %d\n",(int)me->d_clearAlignPoints);
  if (new_value) {
    me->d_aligner->

  me->d_clearAlignPoints = 0;
}
*/

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
      vrpn_float32 sizeX, sizeY;
      vrpn_bool height_field;
      d_aligner->getImageParameters(which_image, res_x, res_y, sizeX, sizeY);
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
      nmb_TransformMatrix44 targetImFromSourceIm;
      d_aligner->getRegistrationResult(targetImFromSourceIm);

      if (d_imageList == NULL) {
         fprintf(stderr, "handleRegistrationChange: Error, image list null\n");
         return;
      }
      nmb_Image *sourceImage = d_imageList->getImageByName
                          (d_sourceImageName.string());
      nmb_Image *targetImage = d_imageList->getImageByName
                          (d_targetImageName.string());

      if (!sourceImage || !targetImage) {
           fprintf(stderr, "handleRegistrationChange: can't find image\n");
           return;
      }

      double srcAcqDistX, srcAcqDistY, tgtAcqDistX, tgtAcqDistY;
      sourceImage->getAcquisitionDimensions(srcAcqDistX, srcAcqDistY);
      targetImage->getAcquisitionDimensions(tgtAcqDistX, tgtAcqDistY);

      // adjust for scaling of images
      double xform_matrix[16];
      targetImFromSourceIm.getMatrix(xform_matrix);
      int i;
      for (i = 0; i < 4; i++) {
        xform_matrix[i] *= srcAcqDistX;
        xform_matrix[i+4] *= srcAcqDistY;
        xform_matrix[4*i] /= tgtAcqDistX;
        xform_matrix[4*i+1] /= tgtAcqDistY;
      }
      targetImFromSourceIm.setMatrix(xform_matrix);

      nmb_TransformMatrix44 sourceImFromWorld;
      sourceImage->getWorldToImageTransform(sourceImFromWorld);
      nmb_TransformMatrix44 targetImFromWorld;

      /* ********************************************************* 
       targetImFromWorld = targetImFromSourceIm * sourceIm
       * ********************************************************* */
      targetImFromWorld = targetImFromSourceIm;
      targetImFromWorld.compose(sourceImFromWorld);

      targetImage->setWorldToImageTransform(targetImFromWorld);
      // now tell pattern editor that the transform for this image changed
      d_patternEditor->newPosition(targetImage);
      break;
  }
}

// static
int ControlPanels::handle_semWindowRedraw(
        const ImageViewerDisplayData & /*data*/,
        void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
  ImageViewer *image_viewer = ImageViewer::getImageViewer();
  image_viewer->drawImage(me->d_semWinID);
  return 0;
}

// static
void ControlPanels::handle_sem_change(void *ud,
                        const nmm_Microscope_SEM_ChangeHandlerData &info)
{
  ControlPanels *me = (ControlPanels *)ud;
  me->handleSEMChange(info);
}

void ControlPanels::handleSEMChange(
                        const nmm_Microscope_SEM_ChangeHandlerData &info)
{
  static vrpn_int32 last_dwell_time = 0;
  static struct timeval first_expose_time;
  vrpn_int32 res_x, res_y;
  vrpn_int32 time_nsec = 0;
  vrpn_int32 enabled;
  //vrpn_int32 x = 0, y = 0;
  vrpn_int32 h_time_nsec, v_time_nsec;
  vrpn_int32 xg, xo, yg, yo, zg, zo;
  void *scanlineData;
  vrpn_int32 start_x, start_y, dx,dy, line_length, num_fields, num_lines;
  nmb_PixelType pix_type;
  int i,j;
  vrpn_float32 mag = 1;

  vrpn_uint8 *uint8_data = NULL;
  vrpn_uint16 *uint16_data = NULL;
  vrpn_float32 *float32_data = NULL;
  nmb_Image *currentImage;
  char currentImageName[256];
  int index = 0;

  ImageViewer *image_viewer = ImageViewer::getImageViewer();

  double imageRegionWidth = 1000;
  double imageRegionHeight = 1000;

  nmb_ImageBounds ib;

  switch(info.msg_type) {
    case nmm_Microscope_SEM::REPORT_RESOLUTION:
        info.sem->getResolution(res_x, res_y);
        fprintf(stderr, "REPORT_RESOLUTION: %d, %d\n", res_x, res_y);
        image_viewer->setWindowImageSize(d_semWinID, res_x, res_y);
        i = nmm_EDAX::resolutionToIndex(res_x, res_y);
        if (i < 0) {
           fprintf(stderr, "Error, resolution unexpected\n");
           break;
        }
        d_semResolution = i;
      break;
    case nmm_Microscope_SEM::REPORT_PIXEL_INTEGRATION_TIME:
        info.sem->getPixelIntegrationTime(time_nsec);
        printf("REPORT_PIXEL_INTEGRATION_TIME: %d\n",time_nsec);
        d_semPixelIntegrationTime_nsec = time_nsec;
      break;
    case nmm_Microscope_SEM::REPORT_INTERPIXEL_DELAY_TIME:
        info.sem->getInterPixelDelayTime(time_nsec);
        printf("REPORT_INTERPIXEL_DELAY_TIME: %d\n", time_nsec);
        d_semInterPixelDelayTime_nsec = time_nsec;
      break;
    case nmm_Microscope_SEM::SCANLINE_DATA:
        info.sem->getScanlineData(start_x, start_y, dx, dy, line_length,
                                  num_fields, num_lines,
                                  pix_type, &scanlineData);

        info.sem->getResolution(res_x, res_y);
       
        if (d_semOverwriteOldData) {
           sprintf(currentImageName, "SEM_DATA%dx%d", res_x, res_y);
        } else {
           index = imageCount[(int)(d_semResolution)];
           sprintf(currentImageName, "SEM_DATA%dx%d_%.2d",
                                         res_x, res_y, index%100);
           if (start_y+num_lines*dy == res_y) {
             imageCount[(int)(d_semResolution)]++;
             if (imageCount[(int)(d_semResolution)] == 100) {
                 printf("Warning, exceeded 100 image limit, overwriting "
                    "old data starting with the oldest\n");
             }
           }
        }
        currentImage = d_imageList->getImageByName(currentImageName);
        if (!currentImage) {
            currentImage = new nmb_ImageArray(currentImageName, "SEM",
                  (short)res_x, (short)res_y, pix_type);
            d_imageList->addImage(currentImage);
            d_patternEditor->addImage(currentImage);
            d_semBufferImageNames->addEntry(currentImageName);
            // in case this was the first image created, the following is
            // necessary
	    updateCurrentImageControls();
        }
        info.sem->getScanRegion_nm(imageRegionWidth, imageRegionHeight);
        currentImage->setAcquisitionDimensions(imageRegionWidth, 
                                               imageRegionHeight);
        ib = nmb_ImageBounds(0.0, 0.0, imageRegionWidth, imageRegionHeight);
        currentImage->setBounds(ib);
        d_patternEditor->newPosition(currentImage);
        d_semDataBuffer.Set(currentImageName);
        if (start_y + num_lines*dy > res_y || line_length*dx != res_x) {
           fprintf(stderr, "SCANLINE_DATA, dimensions unexpected\n");
           fprintf(stderr, "  got (%d,[%d-%d]), expected (%d,%d)\n",
                              line_length*dx, start_y, start_y + dy*(num_lines),
                                res_x, res_y);
           break;
        }
        int x, y;
        x = start_x;
        y = start_y;
        uint8_data = (vrpn_uint8 *)scanlineData;
        uint16_data = (vrpn_uint16 *)scanlineData;
        float32_data = (vrpn_float32 *)scanlineData;
        switch(pix_type) {
          case NMB_UINT8:
            image_viewer->setValueRange(d_semWinID, 0.0, 255.0);
            for (i = 0; i < num_lines; i++) {
              x = start_x;

/*
              if (num_fields == 1) {
                 image_viewer->setScanline(d_semWinID,
                      y, &(uint8_data[i*line_length]));
              } else {
*/
                for (j = 0; j < line_length; j++) {
                  double val = 
                     (double)(uint8_data[(i*line_length+j)*num_fields]);
                  image_viewer->setValue(d_semWinID,
                      x, y, val);
                  currentImage->setValue(x, y, val);
                  x += dx;
                }
/*
              }
*/
              y += dy;
            }
            break;
          case NMB_UINT16:
              image_viewer->setValueRange(d_semWinID,0.0,65535.0);
              for (i = 0; i < num_lines; i++) {
                x = start_x;
                for (j = 0; j < line_length; j++) {
                   image_viewer->setValue(d_semWinID,
                        x, y,
                        (double)(uint16_data[(i*line_length+j)*num_fields]));
                   x += dx;
                }
                y += dy;
              }
              break;
          case NMB_FLOAT32:
              for (i = 0; i < num_lines; i++) {
                x = start_x;
                for (j = 0; j < line_length; j++) {
                   image_viewer->setValue(d_semWinID,
                        x, y,
                        (double)(float32_data[(i*line_length+j)*num_fields]));
                   x += dx;
                }
                y += dy;
              }
              break;
        }

        // when we get the end of an image restart the scan
        // and redraw the window
        if (start_y+num_lines == res_y) {
          image_viewer->dirtyWindow(d_semWinID);
          image_viewer->dirtyWindow(d_patternEditor->mainWinID());
        }

        if (start_y+num_lines == res_y) {
            if (d_semAcquireContinuousChecked){
                info.sem->requestScan(1);
            } else {
                info.sem->requestScan(0);
                info.sem->setExternalScanControlEnable(0);
            }
        }
      break;
    case nmm_Microscope_SEM::POINT_DWELL_TIME:
      info.sem->getPointDwellTime(time_nsec);
      if (last_dwell_time != time_nsec) {
        printf("POINT_DWELL_TIME: %d nsec\n", time_nsec);
        last_dwell_time = time_nsec;
      }
      break;
    case nmm_Microscope_SEM::BEAM_BLANK_ENABLE:
      info.sem->getBeamBlankEnabled(enabled);
      printf("BEAM_BLANK_ENABLE: %d\n", enabled);
      d_semBeamBlankEnable = enabled;
      break;
    case nmm_Microscope_SEM::MAX_SCAN_SPAN:
      info.sem->getMaxScan(x, y);
      printf("MAX_SCAN_SPAN: %d, %d\n", x, y);
      break;
    case nmm_Microscope_SEM::BEAM_LOCATION:
      info.sem->getBeamLocation(x, y);
      if (expose_point_count == 0) {
         first_expose_time = info.msg_time;
      }
      expose_point_count++;
      if (expose_point_count % 10000 == 0) {
         printf("%d points exposed\n", expose_point_count);
         struct timeval total_time = vrpn_TimevalDiff(info.msg_time, 
                                                      first_expose_time);
         double total_time_msec = vrpn_TimevalMsecs(total_time);
         double avg_dwell_time_msec = total_time_msec/
                                      (double)expose_point_count;
         printf("avg dwell time= %g msec\n", avg_dwell_time_msec);
      }
//      printf("BEAM_LOCATION: %d, %d\n", x, y);
      break;
    case nmm_Microscope_SEM::RETRACE_DELAYS:
      info.sem->getRetraceDelays(h_time_nsec, v_time_nsec);
      printf("RETRACE_DELAYS: horiz = %d, vert = %d\n", 
             h_time_nsec, v_time_nsec);
      d_semHorizRetraceDelay_nsec = h_time_nsec;
      d_semVertRetraceDelay_nsec = v_time_nsec;
      break;
    case nmm_Microscope_SEM::DAC_PARAMS:
      info.sem->getDACParams(xg, xo, yg, yo, zg, zo);
      printf("DAC_PARAMS: (gain, offset)\n");
      printf("          x (%d, %d)\n", xg, xo);
      printf("          y (%d, %d)\n", yg, yo);
      printf("          z (%d, %d)\n", zg, zo);
      break;
    case nmm_Microscope_SEM::EXTERNAL_SCAN_CONTROL_ENABLE:
      info.sem->getExternalScanControlEnable(enabled);
      printf("EXTERNAL_SCAN_CONTROL_ENABLE: %d\n", enabled);
      d_semExternalScanControlEnable = enabled;
      break;
    case nmm_Microscope_SEM::REPORT_MAGNIFICATION:
      info.sem->getMagnification(mag);
      printf("REPORT_MAGNIFICATION: %f\n", mag);
      d_semAcquisitionMagnification = mag;
      break;
    default:
      printf("unknown message type: %d\n", info.msg_type);
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
  me->d_aligner->setImage(NMR_SOURCE, im);
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
  me->d_aligner->setImage(NMR_TARGET, im);
}

// static
void ControlPanels::handle_resampleImageName_change(
                              const char * /*new_value*/, void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
  printf("new resampled image: %s\n",(const char *)me->d_resampleImageName);
  printf("resampling %s onto %s at resolution %dx%d\n",
          (const char *)me->d_sourceImageName, 
          (const char *)me->d_targetImageName,
          me->d_resampleResolutionX, me->d_resampleResolutionY);

  printf("this feature is disabled\n");
/*
  if (me->d_imageList == NULL){
      return;
  }
  nmb_Image *srcIm = me->d_imageList->getImageByName(
                              (const char *)me->d_sourceImageName);
  nmb_Image *tgtIm = me->d_imageList->getImageByName(
                              (const char *)me->d_targetImageName);
  if (!srcIm || !tgtIm || (strlen(d_resampleImageName.string()) == 0)){
    return;
  }
  nmb_Image *new_image = new nmb_ImageGrid(
                      (const char *)(d_resampleImageName.string()),
                      (const char *)(tgtIm->unitsValue()),
                      d_resampleResolutionX, d_resampleResolutionY);
  nmb_ImageBounds srcIm_bounds;
  srcIm->getBounds(srcIm_bounds);
  new_image->setBounds(srcIm_bounds);
  TopoFile tf;
  srcIm->getTopoFileInfo(tf);
  new_image->setTopoFileInfo(tf);
  d_resampleImageName = (const char *) "";
  nmr_Util::createResampledImage((*tgtIm), (*srcIm),
        d_TargetFromSourceTransform, (*new_image));
add the image to the list
*/
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
  ImageViewer *image_viewer = ImageViewer::getImageViewer();
  if (new_value) {
    image_viewer->showWindow(me->d_semWinID);
  } else {
    image_viewer->hideWindow(me->d_semWinID);
  }
}

// static
void ControlPanels::handle_semAcquireImagePushed_change(int /*new_value*/, 
                                                        void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
  printf("sem acquire image pushed: %d\n",(int)me->d_semAcquireImagePushed);
  
  if (me->d_SEM) {
    me->d_SEM->setExternalScanControlEnable(1);
    printf("requesting scan\n");
    me->d_SEM->requestScan(1);
  }
}

// static
void ControlPanels::handle_semAcquireContinuousChecked_change(
                         int /*new_value*/, void *ud)
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
  int curr_value;
  if (me->d_SEM) {
    me->d_SEM->getPixelIntegrationTime(curr_value);
    if (curr_value != new_value) {
      me->d_SEM->setPixelIntegrationTime(new_value);
    }
  }
}

// static
void ControlPanels::handle_semInterPixelDelayTime_change(
                    int new_value, void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
  printf("sem interpix delay time: %d\n",
                       (int)me->d_semInterPixelDelayTime_nsec);
  int curr_value;
  if (me->d_SEM) {
    me->d_SEM->getInterPixelDelayTime(curr_value);
    if (curr_value != new_value) {
      me->d_SEM->setInterPixelDelayTime(new_value);
    }
  }
}

// static
void ControlPanels::handle_semResolution_change(int new_value, void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
  printf("sem res: %d\n",(int)me->d_semResolution);
  
  int res_x; 
  int res_y;
  nmm_EDAX::indexToResolution(new_value, res_x, res_y);
  printf("setting resolution to %d,%d\n", res_x, res_y);
  int curr_res_x, curr_res_y;
  if (me->d_SEM) {
    me->d_SEM->getResolution(curr_res_x, curr_res_y);
    if (curr_res_x != res_x || curr_res_y != res_y) {
      me->d_SEM->setResolution(res_x, res_y);
    }
  }
}

// static
void ControlPanels::handle_semAcquisitionMagnification_change(
             int new_value, void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
  // set exposure mag as a convenience since it will generally be the same
  me->d_semExposureMagnification = (int)me->d_semAcquisitionMagnification;
  // scale the display so that images at this magnification fill the window
  double width, height;
  if (me->d_SEM) {
    me->d_SEM->getScanRegion_nm(width, height);
  } else {
    width = 1e8/(double)new_value;
    height = width;
  }
//  printf("acq. mag.-> %d; viewport->(%g, %g)\n", new_value, width, height);
  me->d_patternEditor->setViewport(0, 0, width, height);
}

// static
void ControlPanels::handle_semBeamBlankEnable_change(
             int new_value, void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
  if (me->d_SEM) {
    int curr_value;
    me->d_SEM->getBeamBlankEnabled(curr_value);
    if (curr_value != new_value) {
      me->d_SEM->setBeamBlankEnable(new_value);
    }
  }
}

// static
void ControlPanels::handle_semHorizRetraceDelay_change(
             int new_value, void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
  if (me->d_SEM) {
    int hdelay, vdelay;
    me->d_SEM->getRetraceDelays(hdelay, vdelay);
    if (hdelay != new_value) {
      me->d_SEM->setRetraceDelays((vrpn_int32)(me->d_semHorizRetraceDelay_nsec),
                                (vrpn_int32)(me->d_semVertRetraceDelay_nsec));
    }
  }
}

// static
void ControlPanels::handle_semVertRetraceDelay_change(
             int new_value, void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
  if (me->d_SEM) {
    int hdelay, vdelay;
    me->d_SEM->getRetraceDelays(hdelay, vdelay);
    if (vdelay != new_value) {
      me->d_SEM->setRetraceDelays((vrpn_int32)(me->d_semHorizRetraceDelay_nsec),
                                (vrpn_int32)(me->d_semVertRetraceDelay_nsec));
    }
  }
}


// static
void ControlPanels::handle_semDACParams_change(int /*new_value*/, void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
  if (me->d_SEM) {
    int xg, xo, yg, yo, zg, zo;
    me->d_SEM->getDACParams(xg, xo, yg, yo, zg, zo);
    if ((xg != (int)(me->d_semXDACGain)) || 
        (xo != (int)(me->d_semXDACOffset)) ||
        (yg != (int)(me->d_semYDACGain)) ||
        (yo != (int)(me->d_semYDACOffset)) ||
        (zg != (int)(me->d_semZADCGain)) ||
        (zo != (int)(me->d_semZADCOffset))) {
      me->d_SEM->setDACParams((vrpn_int32)(me->d_semXDACGain),
                              (vrpn_int32)(me->d_semXDACOffset),
                              (vrpn_int32)(me->d_semYDACGain),
                              (vrpn_int32)(me->d_semYDACOffset),
                              (vrpn_int32)(me->d_semZADCGain),
                              (vrpn_int32)(me->d_semZADCOffset));
    }
  }
}

// static
void ControlPanels::handle_semExternalScanControlEnable_change(
             int new_value, void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
  if (me->d_SEM) {
    int curr_value;
    me->d_SEM->getExternalScanControlEnable(curr_value);
    if (curr_value != new_value) {
      me->d_SEM->setExternalScanControlEnable(new_value);
    }
  }
}

// EXPOSURE settings:
// static
void ControlPanels::handle_semExposureMagnification_change(
             int /*new_value*/, void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
  printf("sem exposure mag: %d\n",(int)me->d_semExposureMagnification);
}

// static
void ControlPanels::handle_semBeamWidth_change(double /*new_value*/, void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
//  printf("sem beam width: %g\n",(double)me->d_semBeamWidth_nm);
  me->displayDwellTimes();
}


// static
void ControlPanels::handle_semBeamCurrent_change(double /*new_value*/, void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
//  printf("sem beam current: %g\n",(double)me->d_semBeamCurrent_picoAmps);
  me->displayDwellTimes();
}

// static
void ControlPanels::handle_semBeamExposePushed_change(int /*new_value*/, 
                                                      void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
  printf("sem beam expose: %d\n",(int)me->d_semBeamExposePushed);
  int dwell_time_nsec = 0;
  double dwell_time_sec;
  double beam_width_nm, beamCurrent;
  nmm_EDAX::snapIntegrationTime_nsec(dwell_time_nsec, vrpn_TRUE);
  dwell_time_sec = 1e-9*(double)dwell_time_nsec;
  beam_width_nm = (double)(me->d_semBeamWidth_nm);
  beamCurrent = (double)(me->d_semBeamCurrent_picoAmps);

  int externalControlSave = me->d_semExternalScanControlEnable;
  if (me->d_SEM) {
    me->d_SEM->setExternalScanControlEnable(1);
  }
  me->d_exposureManager->setColumnParameters(dwell_time_sec,
              beam_width_nm, beamCurrent);
  expose_point_count = 0;
  me->d_exposureManager->exposePattern(me->d_patternEditor->shapeList(),
                                       me->d_patternEditor->dumpPointList(),
                                     me->d_SEM, me->d_semExposureMagnification);
  if (me->d_SEM) {
    me->d_SEM->setExternalScanControlEnable(externalControlSave);
  }
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



