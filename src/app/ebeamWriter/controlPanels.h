#ifndef CONTROLPANELS_H
#define CONTROLPANELS_H

#include <Tcl_Linkvar.h>
#include "patternEditor.h"
#include "nmm_Microscope_SEM_Remote.h"
#include "nmb_Image.h"
#include "nmr_Registration_Proxy.h"
#include "nmm_EDAX.h"
#include "exposureManager.h"

// This class manages variables linked to the control panel components of
// the GUI
class ControlPanels {
 public:
   ControlPanels(PatternEditor *pe,
                 nmr_Registration_Proxy *rp,
                 nmm_Microscope_SEM_Remote *sem);
   ~ControlPanels();

   void setImageList(nmb_ImageList *data);
   nmb_ListOfStrings *imageNameList();

 protected:
   // callback stuff
   void setupCallbacks();

   static void handle_openImageFileName_change(const char *new_value, void *ud);
   static void handle_bufferImageFileName_change(const char *new_value, 
                                                 void *ud);

   static void handle_lineWidth_nm_change(double new_value, void *ud);
   static void handle_exposure_change(double new_value, void *ud);
   static void handle_drawingTool_change(int new_value, void *ud);
   static void handle_clearDrawing_change(int new_value, void *ud);

   static void handle_imageColorChanged_change(int new_value, void *ud);
   static void handle_imageOpacity_change(double new_value, void *ud);
   static void handle_hideOtherImages_change(int new_value, void *ud);
   static void handle_enableImageDisplay_change(int new_value, void *ud);
   static void handle_currentImage_change(const char *new_value, void *ud);
   void updateCurrentImageControls();

//   static void handle_clearAlignPoints_change(int new_value, void *ud);
   static void handle_alignmentNeeded_change(int new_value, void *ud);
   static void handle_sourceImageName_change(const char *new_value, void *ud);
   static void handle_targetImageName_change(const char *new_value, void *ud);
   static void handle_alignWindowOpen_change(int new_value, void *ud);
   static void handle_semWindowOpen_change(int new_value, void *ud);
   static void handle_semAcquireImagePushed_change(int new_value, void *ud);
   static void handle_semAcquireContinuousChecked_change(
               int new_value, void *ud);
   static void handle_semPixelIntegrationTime_change(int new_value, void *ud);
   static void handle_semInterPixelDelayTime_change(int new_value, void *ud);
   static void handle_semResolution_change(int new_value, void *ud);
   static void handle_semAcquisitionMagnification_change(int new_value, 
                    void *ud);
   static void handle_semBeamBlankEnable_change(int new_value, void *ud);
   static void handle_semHorizRetraceDelay_change(int new_value, void *ud);
   static void handle_semVertRetraceDelay_change(int new_value, void *ud);
   static void handle_semDACParams_change(int new_value, void *ud);
   static void handle_semExternalScanControlEnable_change(
                                                 int new_value, void *ud);

   static void handle_semExposureMagnification_change(int new_value, void *ud);
   static void handle_semBeamWidth_change(double new_value, void *ud);
   static void handle_semBeamCurrent_change(double new_value, void *ud);
   static void handle_semBeamExposePushed_change(int new_value, void *ud);

   static void handle_registration_change(void *ud,
                              const nmr_ProxyChangeHandlerData &info);
   void handleRegistrationChange
                              (const nmr_ProxyChangeHandlerData &info);
   static void handle_sem_change(void *ud,
                        const nmm_Microscope_SEM_ChangeHandlerData &info);
   void handleSEMChange(const nmm_Microscope_SEM_ChangeHandlerData &info);

   static int handle_semWindowRedraw(const ImageViewerDisplayData &data,
        void *ud);

   // list of all images available for display
   Tclvar_list_of_strings *d_imageNames;

   // file menu
   Tclvar_list_of_strings *d_bufferImageFormatList;
   Tclvar_string d_openImageFileName;
   Tclvar_string d_bufferImageFileName;
   Tclvar_string d_bufferImageFormat;

   // Tcl variables linked to control panels
   // drawing parameters:
   Tclvar_float d_lineWidth_nm;
   Tclvar_float d_exposure_uCoulombs_per_square_cm;
   Tclvar_int d_drawingTool;
   Tclvar_int d_clearDrawing;

   // display parameters:
   Tclvar_int d_imageColorChanged;
   Tclvar_int d_imageRed;
   Tclvar_int d_imageGreen;
   Tclvar_int d_imageBlue;
   Tclvar_float d_imageOpacity;
   Tclvar_int d_hideOtherImages;
   Tclvar_int d_enableImageDisplay;
   Tclvar_string d_currentImage;

   // alignment
//   Tclvar_int d_clearAlignPoints;
   Tclvar_int d_alignmentNeeded;
   Tclvar_string d_sourceImageName;
   Tclvar_string d_targetImageName;
   Tclvar_int d_alignWindowOpen;

   // SEM
   Tclvar_int d_semWindowOpen;
   Tclvar_int d_semAcquireImagePushed;
   Tclvar_int d_semAcquireContinuousChecked;
   Tclvar_int d_semPixelIntegrationTime_nsec;
   Tclvar_int d_semInterPixelDelayTime_nsec;
   Tclvar_int d_semResolution;
   Tclvar_int d_semAcquisitionMagnification; // for a 12.8 cm wide display
   Tclvar_int d_semOverwriteOldData;
   Tclvar_int d_semBeamBlankEnable;
   Tclvar_int d_semHorizRetraceDelay_nsec;
   Tclvar_int d_semVertRetraceDelay_nsec;
   Tclvar_int d_semXDACGain, d_semXDACOffset;
   Tclvar_int d_semYDACGain, d_semYDACOffset;
   Tclvar_int d_semZADCGain, d_semZADCOffset;
   Tclvar_int d_semExternalScanControlEnable;
   Tclvar_string d_semDataBuffer;
   Tclvar_list_of_strings *d_semBufferImageNames;
   int d_semWinID;

   // Beam Control
   Tclvar_int d_semExposureMagnification; // for a 12.8 cm wide display
   Tclvar_float d_semBeamWidth_nm;
   Tclvar_float d_semBeamCurrent_picoAmps;
   Tclvar_int d_semBeamExposePushed;
 

   PatternEditor *d_patternEditor;
   nmr_Registration_Proxy *d_aligner;
   nmm_Microscope_SEM_Remote *d_SEM;
   ExposureManager *d_exposureManager;
   nmb_ImageList *d_imageList;
   int imageCount[EDAX_NUM_SCAN_MATRICES]; // a count of the number 
                    // of images being stored at each resolution

};

#endif
