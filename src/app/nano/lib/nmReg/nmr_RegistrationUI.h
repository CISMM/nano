/*===3rdtech===
  Copyright (c) 2000 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#ifndef NMR_REGISTRATIONUI_H
#define NMR_REGISTRATIONUI_H
#include "nmg_Graphics.h"
#include "nmb_Image.h"
#include "nmb_String.h"
#include "nmr_Registration_Proxy.h"
#include "nmr_ImageTransform.h"
#include "Tcl_Linkvar.h"
#include "Tcl_Netvar.h"

/// This class is in charge of the client-side user interface which 
/// lets the user manipulate and view data from an nmr_Registration_Client 
/// The display of the registration results is done using nmg_Graphics and
/// a projective texture; The control panel also provides utilities for
/// resampling an image using the registration result in order to align
/// one image to another one.
class nmr_RegistrationUI {
  public:
    nmr_RegistrationUI(nmg_Graphics *g, nmb_ImageList *im,
                       nmr_Registration_Proxy *aligner);

    ~nmr_RegistrationUI();

    void setupCallbacks();
    void teardownCallbacks();
    void changeDataset(nmb_ImageList *im);
    void handleRegistrationChange(const nmr_ProxyChangeHandlerData &info);
    static void handle_registrationChange(void *ud,
                  const nmr_ProxyChangeHandlerData &info);
    static void handle_resampleImageName_change(const char *name, void *ud);
    static void handle_registrationImage3D_change(const char *name, void *ud);
    static void handle_registrationImage2D_change(const char *name, void *ud);
    static void handle_textureDisplayEnabled_change(vrpn_int32 value, void *ud);
    static void handle_registrationRequest_change(vrpn_int32 value, void *ud);
    static void handle_registrationEnabled_change(vrpn_int32 value, void *ud);
    void createResampleImage(const char *imageName);
    void setProjectionImage(const char *imageName) {
       d_registrationImageName2D = imageName;
       handle_registrationImage2D_change(imageName, (void *)this);
    }
    void displayTexture(int enable) {d_textureDisplayEnabled = enable;};

  protected:
 
    Tclvar_string d_registrationImageName3D;
    Tclvar_string d_registrationImageName2D;
    Tclvar_string d_newResampleImageName;
    Tclvar_int d_registrationEnabled;
    Tclvar_int d_registrationRequested;
    Tclvar_int d_constrainToTopography;
    Tclvar_int d_invertWarp;
    Tclvar_int d_textureDisplayEnabled;
    Tclvar_int d_resampleResolutionX;
    Tclvar_int d_resampleResolutionY;
    Tclvar_float d_resampleRatio;

    vrpn_bool d_registrationValid;
    nmg_Graphics *d_graphicsDisplay;
    nmb_ImageList *d_imageList;
    nmr_Registration_Proxy *d_aligner;

    nmr_ImageTransformAffine d_imageTransformWorldSpace;
    nmr_ImageTransformAffine d_imageTransformImageSpace;
    nmr_ImageTransformAffine d_imageTransformImageSpaceInv;
};

#endif
