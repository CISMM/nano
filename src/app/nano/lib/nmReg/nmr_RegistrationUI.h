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
#include "nmb_TransformMatrix44.h"
#include "Tcl_Linkvar.h"
#include "Tcl_Netvar.h"

class nmb_Dataset;
/// This class is in charge of the client-side user interface which 
/// lets the user manipulate and view data from an nmr_Registration_Client 
/// The display of the registration results is done using nmg_Graphics and
/// a projective texture; The control panel also provides utilities for
/// resampling an image using the registration result in order to align
/// one image to another one.

/***************************************************************
     there are 4 coordinate systems we need to worry about 

        We have two images: 
           1) Topography/3D/Source 
                (unless using a 2D->2D transform this is the source)
           2) Projection/2D/Target 
                (unless using a 2D->2D transform this is the target)

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
    nmr_RegistrationUI(nmg_Graphics *g, nmb_Dataset *d,
                       nmr_Registration_Proxy *aligner);

    ~nmr_RegistrationUI();

    void setupCallbacks();
    void teardownCallbacks();
    void changeDataset(nmb_Dataset *d);
    void handleRegistrationChange(const nmr_ProxyChangeHandlerData &info);
    static void handle_registrationChange(void *ud,
                  const nmr_ProxyChangeHandlerData &info);
    static void handle_resampleImageName_change(const char *name, void *ud);
    static void handle_resamplePlaneName_change(const char *name, void *ud);
    static void handle_registrationImage3D_change(const char *name, void *ud);
    static void handle_registrationImage2D_change(const char *name, void *ud);
    static void handle_textureDisplayEnabled_change(vrpn_int32 value, void *ud);
    static void handle_registrationRequest_change(vrpn_int32 value, void *ud);
    static void handle_registrationEnabled_change(vrpn_int32 value, void *ud);
    void createResampleImage(const char *imageName);
    void createResamplePlane(const char *imageName);
    void setProjectionImage(const char *imageName) {
       d_registrationImageName2D = imageName;
       handle_registrationImage2D_change(imageName, (void *)this);
    }
    void displayTexture(int enable) {d_textureDisplayEnabled = enable;};

  protected:
 
    Tclvar_string d_registrationImageName3D;
    Tclvar_string d_registrationImageName2D;
    Tclvar_string d_newResampleImageName;
    Tclvar_string d_newResamplePlaneName;
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
    nmb_Dataset *d_dataset;
    nmr_Registration_Proxy *d_aligner;

    nmb_TransformMatrix44 d_ProjWorldFromTopoWorldTransform;
    nmb_TransformMatrix44 d_ProjImageFromTopoImageTransform;
    nmb_TransformMatrix44 d_ProjImageFromTopoWorldTransform;
};

#endif
