#include "controlPanels.h"
#include "imageViewer.h"
#include "nmm_EDAX.h"
#include "nmb_TransformMatrix44.h"
#include "exposureUtil.h"
#include "nmb_ImgMagick.h"

static int expose_point_count = 0;

int ControlPanels::s_numImageFileFormats = 5;
char *ControlPanels::s_imageFileFormatNames[] = 
{"TIFF", "JPG", "BMP", "PGM", "PPM"};

ControlPanels::ControlPanels(PatternEditor *pe,
                             nmm_Microscope_SEM_Remote *sem):
   d_imageNames(new Tclvar_list_of_strings()),
   d_bufferImageFormatList(new Tclvar_list_of_strings()),
   d_openImageFileName("open_image_filename", ""),
   d_saveImageFileName("save_image_filename", ""),
   d_saveImageFileType("save_image_filetype", ""),
   d_saveImageName("export_plane", ""),
   d_openPatternFileName("open_pattern_filename", ""),
   d_savePatternFileName("save_pattern_filename", ""),

   d_saveImageFormatList(new Tclvar_list_of_strings()),
   d_bufferImageFileName("bufferImage_filename", ""),
   d_bufferImageFormat("bufferImage_format", ""),
   d_lineWidth_nm("line_width_nm", 0),
   d_line_exposure_pCoulombs_per_cm(
                 "line_exposure_pCoulombs_per_cm", 1000),
   d_area_exposure_uCoulombs_per_square_cm(
                 "area_exposure_uCoulombs_per_square_cm", 1000),
   d_drawingTool("drawing_tool", 1),
   d_clearDrawing("clear_drawing", 0),
   d_addTestGrid("add_test_grid", 0),
   d_canvasImage("canvas_image", "none"),

   d_imageColorChanged("image_color_changed", 0),
   d_imageRed("image_r", 255),
   d_imageGreen("image_g", 255),
   d_imageBlue("image_b", 120),
   d_imageOpacity("image_opacity", 50.0),
   d_imageMagnification("image_magnification", 1000),
   d_hideOtherImages("hide_other_images", 0),
   d_enableImageDisplay("enable_image_display", 0),
   d_currentImage("current_image", "none"),

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
   d_semDotSpacing_nm("sem_dot_spacing_nm", 10),
   d_semLineSpacing_nm("sem_line_spacing_nm", 10),
   d_semBeamCurrent_picoAmps("sem_beam_current_picoAmps", 10),
   d_semBeamExposePushed("sem_beam_expose_now", 0),
   d_semBeamExposeEnabled("sem_beam_expose_enabled", 0),
   d_semDoTimingTest("sem_do_timing_test", 0),
   d_semPointReportEnable("sem_point_report_enable", 0),
   d_semExposureStatus("sem_exposure_status", "N/A"),
   d_semMinLinExposure("sem_min_lin_exposure", "0"),
   d_semMinAreaExposure("sem_min_area_exposure", "0"),
   d_semMinLinExposure_pCoul_per_cm(0),
   d_semMinAreaExposure_uCoul_per_sq_cm(0),
   d_patternEditor(pe),
   d_SEM(sem),
   d_imageList(NULL),
   d_autoEnabledImageName(NULL)
{
  int i;
  d_imageNames->initializeTcl("imageNames");
  d_bufferImageFormatList->initializeTcl("bufferImage_format_list");
  d_semBufferImageNames->initializeTcl("sem_bufferImageNames");
  for (i = 0; i < s_numImageFileFormats; i++)
    d_bufferImageFormatList->addEntry(s_imageFileFormatNames[i]);

  d_saveImageFormatList->initializeTcl("save_image_format_list");

  setupCallbacks();
  for (i = 0; i < EDAX_NUM_SCAN_MATRICES; i++){
    imageCount[i] = 0;
  }
  handle_semAcquisitionMagnification_change(
          (int)d_semAcquisitionMagnification, (void *)this);
  d_patternEditor->setDrawingParameters((double)(d_lineWidth_nm),
                    (double)(d_area_exposure_uCoulombs_per_square_cm),
                    (double)(d_line_exposure_pCoulombs_per_cm));

  // for some reason the callbacks don't get called for these at the start
  if (d_SEM) {
    d_SEM->setDotSpacing((vrpn_float32)d_semDotSpacing_nm);
    d_SEM->setLineSpacing((vrpn_float32)d_semLineSpacing_nm);
    d_SEM->setBeamCurrent((vrpn_float32)d_semBeamCurrent_picoAmps);
  }
  updateMinimumDoses();
}

ControlPanels::~ControlPanels()
{
   delete d_imageNames;
}

void ControlPanels::setImageList(nmb_ImageList *data)
{
  d_imageList = data;

  updateCurrentImageControls();
}

nmb_ListOfStrings *ControlPanels::imageNameList()
{
   return (nmb_ListOfStrings*)d_imageNames;
}

void ControlPanels::updateMinimumDoses()
{
// printf("should be doing some kind of constraint management somewhere in"
//       " this file instead of displaying when there is a problem\n");
  
  double minLinExp = 
             ExposureUtil::computeMinLinearExposure(EDAX_MIN_POINT_DWELL_SEC,
                       (double)(d_semDotSpacing_nm),
                       (double)(d_semBeamCurrent_picoAmps));
  double minAreaExp = 
             ExposureUtil::computeMinAreaExposure(EDAX_MIN_POINT_DWELL_SEC,
                       (double)(d_semDotSpacing_nm),
                       (double)(d_semLineSpacing_nm),
                       (double)(d_semBeamCurrent_picoAmps));

  d_semMinLinExposure_pCoul_per_cm = minLinExp;
  d_semMinAreaExposure_uCoul_per_sq_cm = minAreaExp;
  char str[128];
  sprintf(str, "%d",
         (int)d_semMinLinExposure_pCoul_per_cm);
  d_semMinLinExposure = str;
  sprintf(str, "%d",
         (int)d_semMinAreaExposure_uCoul_per_sq_cm);
  d_semMinAreaExposure = str;


/*
  double beam_width_nm = (double)(d_semBeamWidth_nm);
  double beamCurrent = (double)(d_semBeamCurrent_picoAmps);
  double exposure = (double)(d_exposure_uCoulombs_per_square_cm);

  d_exposureManager->setColumnParameters(EDAX_MIN_POINT_DWELL_SEC, 
                                         beam_width_nm, beamCurrent);
  list<PatternShape> shapes = d_patternEditor->getShapeList();
  should check the whole list of shapes and not just the current setting
  if (!d_exposureManager->checkExposure(exposure)) {
    fprintf(stderr, "WARNING, WARNING, WARNING:"
      " dwell time is below EDAX limit; exposure may be inconsistent\n");
  }
*/
}

void ControlPanels::setupCallbacks()
{
  // control panel variable callbacks
  d_bufferImageFileName.addCallback(handle_bufferImageFileName_change, this);
  d_openImageFileName.addCallback(handle_openImageFileName_change, this);
  d_saveImageFileName.addCallback(handle_saveImageFileName_change, this);
  d_saveImageName.addCallback(handle_saveImageName_change, this);
  d_openPatternFileName.addCallback(handle_openPatternFileName_change, this);
  d_savePatternFileName.addCallback(handle_savePatternFileName_change, this);

  d_lineWidth_nm.addCallback(handle_lineWidth_nm_change, this);
  d_line_exposure_pCoulombs_per_cm.addCallback(
        handle_line_exposure_change, this);
  d_area_exposure_uCoulombs_per_square_cm.addCallback(
        handle_area_exposure_change, this);
  d_drawingTool.addCallback(handle_drawingTool_change, this);
  d_clearDrawing.addCallback(handle_clearDrawing_change, this);
  d_addTestGrid.addCallback(handle_addTestGrid_change, this);
  d_canvasImage.addCallback(handle_canvasImage_change, this);

  d_imageColorChanged.addCallback(handle_imageColorChanged_change, this);
  d_imageOpacity.addCallback(handle_imageOpacity_change, this);
  d_imageMagnification.addCallback(handle_imageMagnification_change, this);
  d_hideOtherImages.addCallback(handle_hideOtherImages_change, this);
  d_enableImageDisplay.addCallback(handle_enableImageDisplay_change, this);
  d_currentImage.addCallback(handle_currentImage_change, this);

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
  d_semDotSpacing_nm.addCallback(handle_semDotSpacing_change, this);
  d_semLineSpacing_nm.addCallback(handle_semLineSpacing_change, this);
  d_semBeamCurrent_picoAmps.addCallback(handle_semBeamCurrent_change, this);
  d_semBeamExposePushed.addCallback(
         handle_semBeamExposePushed_change, this);
  d_semDoTimingTest.addCallback(
         handle_semDoTimingTest_change, this);
  d_semPointReportEnable.addCallback(
         handle_semPointReportEnable_change, this);

  if (d_SEM) {
    d_SEM->registerChangeHandler((void *)this, handle_sem_change);
  }
}

//static
void ControlPanels::handle_openImageFileName_change(const char * /*new_value*/,
                                              void *ud)
{
  nmb_TransformMatrix44 defaultWorldToImage;
  ControlPanels *me = (ControlPanels *)ud;
  double imageRegionWidth, imageRegionHeight;
  me->d_SEM->getScanRegion_nm(imageRegionWidth, imageRegionHeight);
  if (imageRegionWidth != 0 && imageRegionHeight != 0) {
	defaultWorldToImage.scale(2.0/imageRegionWidth, 
		2.0/imageRegionHeight, 1.0);
  } else {
	defaultWorldToImage.scale(0.001, 0.001, 1.0);
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
    im->setWorldToImageTransform(defaultWorldToImage);

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
  /*
  ImageType filetype = TIFFImageType;

  if (strcmp(ImageType_names[TIFFImageType], 
      (const char *)me->d_bufferImageFormat) == 0) {
     filetype = TIFFImageType;
  } else if (strcmp(ImageType_names[PNMImageType],
      (const char *)me->d_bufferImageFormat) == 0) {
     filetype = PNMImageType;
  }
  */
  me->d_patternEditor->saveImageBuffer(
                             (const char *)me->d_bufferImageFileName,
                             (const char *)me->d_bufferImageFormat);
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
    fprintf(stderr, "handle_saveImageName_change:"
                    "error, couldn't find image: %s\n",
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
      fclose(file_ptr);
      file_ptr = NULL;
	  if (nmb_ImgMagick::writeFileMagick(me->d_saveImageFileName.string(),
		  me->d_saveImageFileType.string(), im)) {
//      if (im->exportToFile(file_ptr, me->d_saveImageFileType.string(),
//                              me->d_saveImageFileName.string())) {
          fprintf(stderr, "Couldn't write to this file: %s\n"
                                "Please try another name or directory",
                                me->d_saveImageFileName.string());
        //fclose(file_ptr);
        return;
      }
      //fclose(file_ptr);

  }
  me->d_saveImageFileName = "";
}

//static 
void ControlPanels::handle_openPatternFileName_change(
                                      const char * /*new_value*/, void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
  printf("open pattern file %s\n", (const char *)me->d_openPatternFileName);
  if (strlen((const char *)me->d_openPatternFileName) <= 0) return;

  if ((me->d_patternFile).readFromFile(
                                  (const char *)me->d_openPatternFileName)){
    return;
  }
  me->d_patternEditor->setPattern((me->d_patternFile).getPattern());
  me->d_openPatternFileName = "";
}

//static 
void ControlPanels::handle_savePatternFileName_change(
                           const char * /*new_value*/, void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
  printf("save pattern file %s\n", (const char *)me->d_savePatternFileName);
  if (strlen((const char *)me->d_savePatternFileName) <= 0) return;

  (me->d_patternFile).setPattern(me->d_patternEditor->getPattern());
  (me->d_patternFile).writeToFile((const char *)me->d_savePatternFileName);
  me->d_savePatternFileName = "";
}

// static
void ControlPanels::handle_lineWidth_nm_change(double /*new_value*/, void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
  printf("lineWidth: %g\n", (double)(me->d_lineWidth_nm));
  me->d_patternEditor->setDrawingParameters((double)(me->d_lineWidth_nm),
                    (double)(me->d_area_exposure_uCoulombs_per_square_cm),
                    (double)(me->d_line_exposure_pCoulombs_per_cm));
}

// static
void ControlPanels::handle_line_exposure_change(double /*new_value*/, void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
//  printf("exposure: %g\n",
//        (double)(me->d_line_exposure_pCoulombs_per_cm));
  me->d_patternEditor->setDrawingParameters((double)(me->d_lineWidth_nm),
                    (double)(me->d_area_exposure_uCoulombs_per_square_cm),
                    (double)(me->d_line_exposure_pCoulombs_per_cm));
  if ((double)(me->d_line_exposure_pCoulombs_per_cm) <
      (double)(me->d_semMinLinExposure_pCoul_per_cm))  {
    printf("WARNING: can't do that\n");
  }
}

// static 
void ControlPanels::handle_area_exposure_change(double /*new_value*/, void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
//  printf("exposure: %g\n", 
//        (double)(me->d_area_exposure_uCoulombs_per_square_cm));
  me->d_patternEditor->setDrawingParameters((double)(me->d_lineWidth_nm),
                    (double)(me->d_area_exposure_uCoulombs_per_square_cm),
                    (double)(me->d_line_exposure_pCoulombs_per_cm));

  if ((double)(me->d_area_exposure_uCoulombs_per_square_cm) <
      (double)(me->d_semMinAreaExposure_uCoul_per_sq_cm)) {
    printf("WARNING: can't do that\n");
  }
}

// static
void ControlPanels::handle_drawingTool_change(int new_value, void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
  printf("tool: %d\n", (int)(me->d_drawingTool));
  PE_DrawTool tool;
  if (new_value == 1) {
    // polyline
    tool = PE_THINPOLYLINE;
  } else if (new_value == 2){
    // thick polyline
    tool = PE_THICKPOLYLINE;
  } else if (new_value == 3){
    // polygon
    tool = PE_POLYGON;
  } else if (new_value == 4){
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
  me->d_patternEditor->clearExposurePoints();
}

// static
void ControlPanels::handle_addTestGrid_change(int /*new_value*/, void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
  //printf("add test grid: %d\n", (int)(me->d_addTestGrid));
  double minX_nm, minY_nm;
  double maxX_nm, maxY_nm;
  double xSpan_nm, ySpan_nm;
  int numHorizontal, numVertical;

//  me->d_patternEditor->getViewport(minX_nm, minY_nm, maxX_nm, maxY_nm);
  me->d_SEM->getScanRegion_nm(xSpan_nm, ySpan_nm);

  // shrink the scan region by 1% on each side to avoid going outside the scan
  // region in later calculations
  double inset = 0.01;
  minX_nm = inset*xSpan_nm;
  minY_nm = inset*ySpan_nm;
  maxX_nm = xSpan_nm - minX_nm;
  maxY_nm = ySpan_nm - minY_nm;

  numVertical = 10;
  double x_incr = (maxX_nm - minX_nm)/(double)(numVertical-1);
  double y_incr = x_incr;
  double y_span = (maxY_nm - minY_nm);
  numHorizontal = floor(y_span/y_incr) + 1;
  y_span = (numHorizontal-1)*y_incr;
  maxY_nm = minY_nm + y_span;

  me->d_patternEditor->addTestGrid(minX_nm, minY_nm, maxX_nm, maxY_nm,
             numHorizontal, numVertical);
}

// static
void ControlPanels::handle_canvasImage_change(const char * /*new_value*/, 
                                               void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
  nmb_Image *im = me->d_imageList->getImageByName(
              string((const char *)(me->d_canvasImage)));

  me->d_patternEditor->setCanvasImage(im);
}

// static
void ControlPanels::handle_imageColorChanged_change(int /*new_value*/, void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
  printf("color: %d,%d,%d\n", 
         (int)me->d_imageRed, (int)me->d_imageGreen, (int)me->d_imageBlue);

  nmb_Image *im = me->d_imageList->getImageByName(
              string((const char *)(me->d_currentImage)));
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
                string((const char *)(me->d_currentImage)));
  if (!im) {
    fprintf(stderr, "handle_imageOpacity: Error, could not find image\n");
    return;
  }
  me->d_patternEditor->setImageOpacity(im, new_value);
}

// static
void ControlPanels::handle_imageMagnification_change(double new_value, void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
  printf("magnification: %g\n", (double)me->d_imageMagnification);
  nmb_Image *im = me->d_imageList->getImageByName(
                string((const char *)(me->d_currentImage)));
  if (!im) {
    fprintf(stderr, "handle_imageMagnification: Error, could not find image\n");
    return;
  }
  nmb_TransformMatrix44 worldToImage;

  double width_nm = 10.0e7/new_value;
  double height_nm = width_nm*(double)(im->height())/
	  (double)(im->width());
  worldToImage.scale(1.0/width_nm, 1.0/height_nm, 1.0);
  im->setWorldToImageTransform(worldToImage);
  me->d_patternEditor->updateDisplayTransform(im, NULL);
}

// static 
void ControlPanels::handle_hideOtherImages_change(int new_value, void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
  printf("hide others: %d\n", (int)me->d_hideOtherImages);
  if (new_value) {
    nmb_Image *im = me->d_imageList->getImageByName(
                string((const char *)(me->d_currentImage)));
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
                         string((const char *)(me->d_currentImage)));
  if (!currImage) {
     fprintf(stderr, "handle_enableImageDisplay_change: image not found\n");
     return;
  }

  // tell the pattern editor to enable this image
  if ((int)me->d_enableImageDisplay == 0){
    if (me->d_autoEnabledImageName &&
        strcmp(me->d_autoEnabledImageName, 
               (const char *)(me->d_currentImage)) == 0) {
       delete [] me->d_autoEnabledImageName;
       me->d_autoEnabledImageName = NULL;
    }
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
                  string((const char *)(d_currentImage)));
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
  d_imageMagnification = 10.0e7/(ie->d_image->widthWorld());
  d_enableImageDisplay = ie->d_enabled;
  if (d_hideOtherImages) {
    d_patternEditor->showSingleImage(im);
  }
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
  vrpn_int32 x = 0, y = 0;
  double x_nm = 0.0, y_nm = 0.0;
  vrpn_int32 h_time_nsec, v_time_nsec;
  vrpn_int32 xg, xo, yg, yo, zg, zo;
  void *scanlineData;
  vrpn_int32 start_x, start_y, dx,dy, line_length, num_fields, num_lines;
  nmb_PixelType pix_type;
  int i,j;
  vrpn_float32 mag = 1;
  vrpn_int32 numPointsTotal, numPointsDone;
  vrpn_float32 timeTotal_sec, timeDone_sec;
  char status[128];
  vrpn_uint8 *uint8_data = NULL;
  vrpn_uint16 *uint16_data = NULL;
  vrpn_float32 *float32_data = NULL;
  nmb_Image *currentImage;
  char currentImageName[256];
  int index = 0;
  double percentDone = 0.0;
  ImageViewer *image_viewer = ImageViewer::getImageViewer();

  nmb_TransformMatrix44 worldToImage;
  double imageRegionWidth = 1000;
  double imageRegionHeight = 1000;

  switch(info.msg_type) {
    case nmm_Microscope_SEM::REPORT_RESOLUTION:
        info.sem->getResolution(res_x, res_y);
        fprintf(stderr, "REPORT_RESOLUTION: %d, %d\n", res_x, res_y);
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
        }


        info.sem->getScanRegion_nm(imageRegionWidth, imageRegionHeight);
        currentImage->setAcquisitionDimensions(imageRegionWidth, 
                                               imageRegionHeight);

		worldToImage.scale(1.0/imageRegionWidth, 1.0/imageRegionHeight, 1.0);
        currentImage->setWorldToImageTransform(worldToImage);
        d_patternEditor->updateDisplayTransform(currentImage, NULL);
        d_semDataBuffer.Set(currentImageName);
        if (start_y + num_lines*dy > res_y || line_length*dx != res_x) {
           fprintf(stderr, "SCANLINE_DATA, dimensions unexpected\n");
           fprintf(stderr, "  got (%d,[%d-%d]), expected (%d,%d)\n",
                              line_length*dx, start_y, start_y + dy*(num_lines),
                                res_x, res_y);
           break;
        }
		// make sure the image is visible in the pattern editor window
        if (!(d_patternEditor->getImageEnable(currentImage))) {
          if (d_autoEnabledImageName) {
			  nmb_Image *prevAEImage = 
				   d_imageList->getImageByName(d_autoEnabledImageName);
			  if (prevAEImage) {
				d_patternEditor->setImageEnable(prevAEImage, vrpn_FALSE);
			  } else {
				fprintf(stderr, "autoenable image not found\n");
			  }
			  delete [] d_autoEnabledImageName;
			  d_autoEnabledImageName = NULL;
		  }
          d_autoEnabledImageName = new char[strlen(currentImageName)+1];
          strcpy(d_autoEnabledImageName, currentImageName);
          d_patternEditor->setImageEnable(currentImage, vrpn_TRUE);
		  d_currentImage.Set(currentImageName);
        }
        x = start_x;
        y = start_y;
        uint8_data = (vrpn_uint8 *)scanlineData;
        uint16_data = (vrpn_uint16 *)scanlineData;
        float32_data = (vrpn_float32 *)scanlineData;
        double val;
        switch(pix_type) {
          case NMB_UINT8:
            for (i = 0; i < num_lines; i++) {
              x = start_x;
              for (j = 0; j < line_length; j++) {
                 val = (double)(uint8_data[(i*line_length+j)*num_fields]);
                 currentImage->setValue(x, y, val);
                 x += dx;
              }
              y += dy;
            }
            break;
          case NMB_UINT16:
            for (i = 0; i < num_lines; i++) {
              x = start_x;
              for (j = 0; j < line_length; j++) {
                 val = (double)(uint16_data[(i*line_length+j)*num_fields]);
                 currentImage->setValue(x, y, val);
                 x += dx;
              }
              y += dy;
            }
            break;
          case NMB_FLOAT32:
            for (i = 0; i < num_lines; i++) {
              x = start_x;
              for (j = 0; j < line_length; j++) {
                 val = (double)(float32_data[(i*line_length+j)*num_fields]);
                 currentImage->setValue(x, y, val);
                 x += dx;
              }
              y += dy;
            }
            break;
        }

        // when we get the end of an image restart the scan
        // and redraw the window
        if (start_y+num_lines == res_y) {
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
      info.sem->convert_DAC_to_nm(x, y, x_nm, y_nm);
      d_patternEditor->addExposurePoint(x_nm, y_nm);
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
    case nmm_Microscope_SEM::REPORT_EXPOSURE_STATUS:
      info.sem->getExposureStatus(numPointsTotal, numPointsDone,
                                  timeTotal_sec, timeDone_sec);
//      printf("REPORT_EXPOSURE_STATUS: (%d, %d, %g, %g)\n",
//              numPointsTotal, numPointsDone, timeTotal_sec, timeDone_sec);
	  if (numPointsTotal != 0) {
		  percentDone = 100.0*(double)numPointsDone/(double)numPointsTotal;
	     sprintf(status, "(%3.1lf sec) %3.2lf%% done", timeTotal_sec,percentDone);
	  } else {
		 sprintf(status, "0 exposure points");
	  }
	  d_semExposureStatus = status;
      break;
    default:
      printf("unknown message type: %d\n", info.msg_type);
      break;
  }
}

// static
void ControlPanels::handle_semWindowOpen_change(int /*new_value */, void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
  printf("sem window open: %d\n",(int)me->d_semWindowOpen);
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
    width = SEM_STANDARD_DISPLAY_WIDTH_NM/(double)new_value;
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
  if (me->d_SEM) {
    vrpn_float32 mag;
    me->d_SEM->getMagnification(mag);
    if ((int)mag != (int)me->d_semExposureMagnification) {
      me->d_SEM->setMagnification((vrpn_float32)me->d_semExposureMagnification);
    }
  }
}

// static
void ControlPanels::handle_semDotSpacing_change(double new_value, void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
  me->updateMinimumDoses();
  if (me->d_SEM) {
    me->d_SEM->setDotSpacing((vrpn_float32)new_value);
  }
}

// static
void ControlPanels::handle_semLineSpacing_change(double new_value, void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
  me->updateMinimumDoses();
  if (me->d_SEM) {
    me->d_SEM->setLineSpacing((vrpn_float32)new_value);
  }
}

// static
void ControlPanels::handle_semBeamCurrent_change(double new_value, void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
//  printf("sem beam current: %g\n",(double)me->d_semBeamCurrent_picoAmps);
  me->updateMinimumDoses();
  if (me->d_SEM) {
    me->d_SEM->setBeamCurrent((vrpn_float32)new_value);
  }
}

// static
void ControlPanels::handle_semBeamExposePushed_change(int /*new_value*/, 
                                                      void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
  printf("sem beam expose: %d\n",(int)me->d_semBeamExposePushed);

  me->d_patternEditor->clearExposurePoints();

  if (me->d_SEM) {
    int externalControlSave = me->d_semExternalScanControlEnable;
    me->d_SEM->setExternalScanControlEnable(1);
    me->d_SEM->clearExposePattern();
    (me->d_patternEditor->getPattern()).drawToSEM(me->d_SEM);
    me->d_SEM->exposePattern();
    me->d_SEM->setExternalScanControlEnable(externalControlSave);
    me->d_semBeamExposeEnabled = 0;
  }
}

// static
void ControlPanels::handle_semDoTimingTest_change(int /*new_value*/, void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
  printf("sem timing test: %d\n", (int)me->d_semDoTimingTest);

  if (me->d_SEM) {
    int externalControlSave = me->d_semExternalScanControlEnable;
    me->d_SEM->setExternalScanControlEnable(0);
    me->d_SEM->clearExposePattern();
    (me->d_patternEditor->getPattern()).drawToSEM(me->d_SEM);
    me->d_SEM->exposureTimingTest();
    me->d_SEM->setExternalScanControlEnable(externalControlSave);
    me->d_semBeamExposeEnabled = 1;
  }
}

// static
void ControlPanels::handle_semPointReportEnable_change(int /*new_value*/, 
                                                       void *ud)
{
  ControlPanels *me = (ControlPanels *)ud;
  if (me->d_SEM) {
    me->d_SEM->setPointReportEnable((vrpn_int32)me->d_semPointReportEnable);
  }
  me->d_patternEditor->setExposurePointDisplayEnable(
                                     (vrpn_int32)me->d_semPointReportEnable);
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



