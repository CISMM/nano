/*===3rdtech===
  Copyright (c) 2000 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#ifndef NMR_REGISTRATIONUI_H
#define NMR_REGISTRATIONUI_H
#include "nmb_ImageDisplay.h"
#include "nmb_Image.h"
#include "nmb_ImageManager.h"
#include "nmb_String.h"
#include "nmr_Registration_Proxy.h"
#include "nmb_TransformMatrix44.h"
#include "Tcl_Linkvar.h"
#include "Tcl_Netvar.h"

class nmui_ColorMap;

/** This class is in charge of the client-side user interface which 
 lets the user manipulate and view data from an nmr_Registration_Client 
 The display of the registration results is done through an nmb_ImageDisplay  
 interface which may be implemented as a projective texture on a 3D surface
 or a simpler 2D display;
 The control panel also provides utilities for
 resampling an image using the registration result in order to align
 one image to another one.
*/
/***************************************************************
     there are 4 coordinate systems

        We have two images: 
           1) Topography/3D/Source/Reference 
                (unless using a 2D->2D transform this is always the source for
                 points that get transformed)
           2) Projection/2D/Target/Test 
                (unless using a 2D->2D transform this is always the 
                 target space into which points get transformed)

        For each image there is a world space (x,y,z) and an image space (u,v)
        The world space is unique for each device since different devices
        have different world coordinates which are usually aligned 
        with the u,v axes (this gives us information on things like the 
        conversion from pixels to nm). (XXX - I'm not sure how drift
        should fit into this organization)

        Points in the image space are specified with two numbers (u,v)
        which go from (0,0) at the corner for the first pixel to
        (1.0, 1.0) at the corner for the last pixel. Note that these
        coordinates are independent of resolution (this makes it easier
        to change resolution without having to change a bunch of other
        state)

        The 4 coordinate systems are named something like this:
          TopoWorld, TopoImage, ProjWorld, ProjImage
 ****************************************************************/

class nmr_RegistrationUI {
  public:
    nmr_RegistrationUI(nmr_Registration_Proxy *aligner,
                       nmb_ImageDisplay *display);

    ~nmr_RegistrationUI();

    void setupCallbacks();
    void teardownCallbacks();
    void changeDataset(nmb_ImageManager *dataset);
    void handleRegistrationChange(const nmr_ProxyChangeHandlerData &info);
    static void handle_registrationChange(void *ud,
                  const nmr_ProxyChangeHandlerData &info);
    static void handle_resampleImageName_change(const char *name, void *ud);
    static void handle_resamplePlaneName_change(const char *name, void *ud);
    static void handle_registrationImage3D_change(const char *name, void *ud);
	static void handle_refresh3D_change(vrpn_int32 value, void *ud);
	static void handle_fiducialSpotTracker3D_change(const char *name, void *ud);
	static void handle_optimizeSpotTrackerRadius3D_change(vrpn_int32 enable, void *ud);
	static void handle_spotTrackerRadius3D_change(vrpn_float64 radius, void *ud);
	static void handle_spotTrackerRadiusAccuracy3D_change(vrpn_float64 accuracy, void *ud);
	static void handle_spotTrackerPixelAccuracy3D_change(vrpn_float64 accuracy, void *ud);
    static void handle_registrationImage2D_change(const char *name, void *ud);
	static void handle_refresh2D_change(vrpn_int32 value, void *ud);
	static void handle_fiducialSpotTracker2D_change(const char *name, void *ud);
	static void handle_optimizeSpotTrackerRadius2D_change(vrpn_int32 enable, void *ud);
	static void handle_spotTrackerRadius2D_change(vrpn_float64, void *ud);
	static void handle_spotTrackerRadiusAccuracy2D_change(vrpn_float64, void *ud);
	static void handle_spotTrackerPixelAccuracy2D_change(vrpn_float64 accuracy, void *ud);
    static void handle_flipProjectionImageInX_change(vrpn_int32 value, void *ud);
    static void handle_registrationColorMap3D_change(const char *name, void *ud);
    static void handle_registrationColorMap2D_change(const char *name, void *ud);
    static void handle_registrationMinMax3D_change(vrpn_float64, void *ud);
    static void handle_registrationMinMax2D_change(vrpn_float64, void *ud);
    static void handle_textureDisplayEnabled_change(vrpn_int32 value, void *ud);
    static void handle_textureAlpha_change(vrpn_float64, void *ud);
    static void handle_textureTransformMode_change(const char *name, void *ud);
    static void handle_autoAlignRequested_change(vrpn_int32 value, void *ud);
    static void handle_registrationEnabled_change(vrpn_int32 value, void *ud);

    static void handle_transformationParameter_change(vrpn_float64, void *ud);
	static void handle_saveRegistrationMarkers_change(vrpn_int32 value, void *ud); //new
	static void handle_loadRegistrationMarkers_change(vrpn_int32 value, void *ud); //new

    void sendTransformationParameters();

    void createResampleImage(const char *imageName);
    void createResamplePlane(const char *imageName);
    void setProjectionImage(const char *imageName) {
       d_registrationImageName2D = imageName;
       handle_registrationImage2D_change(imageName, (void *)this);
    }
	void setTopographyImage(const char *imageName) {
		d_registrationImageName3D = imageName;
		handle_registrationImage3D_change(imageName, (void *)this);
	}
    void displayTexture(int enable) {d_textureDisplayEnabled = enable;};
    void autoAlignImages();
    // set a function to be called whenever the worldToImage transformation
    // changes for an image in the image list
    void setTransformationCallback(void (*handler)(nmb_Image *));

  //protected:
 
    int getDisplayedTransformIndex();
    int convertTransformSourceTypeToTransformIndex(int type);
    void updateTextureTransform();
    void setTransformationSource(nmr_RegistrationType source);
    void setAutoAlignMode(nmr_AutoAlignMode mode);
	nmr_FiducialSpotTracker getFiducialSpotTrackerByName(const char *trackerName);

    TclNet_string d_registrationImageName3D;
	TclNet_int d_refresh3D;
	TclNet_string d_fiducialSpotTracker3D;
	TclNet_int d_optimizeSpotTrackerRadius3D;
	TclNet_float d_spotTrackerRadius3D;
	TclNet_float d_spotTrackerRadiusAccuracy3D;
	TclNet_float d_spotTrackerPixelAccuracy3D;
    TclNet_string d_registrationImageName2D;
	TclNet_int d_refresh2D;
	TclNet_string d_fiducialSpotTracker2D;
	TclNet_int d_optimizeSpotTrackerRadius2D;
	TclNet_float d_spotTrackerRadius2D;
	TclNet_float d_spotTrackerRadiusAccuracy2D;
	TclNet_float d_spotTrackerPixelAccuracy2D;
    TclNet_string d_newResampleImageName;
    TclNet_string d_newResamplePlaneName;

    TclNet_int d_registrationEnabled;
    TclNet_int d_constrainToTopography;
    TclNet_int d_invertWarp;
    TclNet_int d_textureDisplayEnabled;
    TclNet_float d_textureAlpha;
    TclNet_int d_resampleResolutionX;
    TclNet_int d_resampleResolutionY;
    TclNet_float d_resampleRatio;
    TclNet_string d_registrationColorMap3D;
    TclNet_string d_registrationColorMap2D;

    // for automatic alignment
    TclNet_int d_autoAlignRequested;
    TclNet_int d_numIterations;
    TclNet_float d_stepSize;
    TclNet_string d_resolutionLevel;
    // this array is set by the C-code and is then copied into
    //  d_resolutionLevelList in a string representation
    vrpn_int32 d_numResolutionLevels;
    vrpn_float32 d_stddev[NMR_MAX_RESOLUTION_LEVELS];
    static vrpn_int32 s_defaultNumResolutionLevels;
    static vrpn_float32 s_defaultStdDev[];
    Tclvar_list_of_strings d_resolutionLevelList;
	TclNet_int d_saveRegistrationMarkers; //new
	TclNet_int d_loadRegistrationMarkers; //new

    TclNet_float d_scaleX, d_scaleY;
    TclNet_float d_translateX, d_translateY, d_translateZ;
    TclNet_float d_rotate2D_Z, d_rotate3D_X, d_rotate3D_Z;
    TclNet_float d_shearZ;

    static vrpn_int32 s_numAutoAlignModes;
    static nmr_AutoAlignMode s_autoAlignModes[];
    static char * s_autoAlignModeNames[];
    TclNet_string d_autoAlignMode;
    Tclvar_list_of_strings d_autoAlignModeList;

    static vrpn_int32 s_numTransformationSources;
    static nmr_RegistrationType s_transformationSources[];
    static char * s_transformationSourceNames[];
    TclNet_string d_transformationSource;
    Tclvar_list_of_strings d_transformationSourceList;

    nmb_ImageDisplay *d_imageDisplay;

    nmb_ImageManager *d_dataset;

    nmr_Registration_Proxy *d_aligner;
    nmui_ColorMap * d_3DImageCMap;
    nmui_ColorMap * d_2DImageCMap;

    nmb_Image * d_last2DImage;
    nmb_Image * d_last3DImage;

    // determines whether images appear flipped or not in the two image windows
    vrpn_bool d_flipXreference, d_flipYreference;
	TclNet_int d_flipProjectionImageInX;
	vrpn_bool d_flipYadjustable;

    // transformations created by various means - manual, entry widgets, auto
    // the order of elements corresponds to that for s_transformationSources
    nmb_TransformMatrix44 *d_scaledProjImFromScaledTopoIm;

	nmr_RegistrationType d_lastTransformTypeSent;

	bool d_colormap2DCallbackDisabled;
};

#endif
