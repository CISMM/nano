#ifndef NMR_REGISTRATION_IMPLUI_H
#define NMR_REGISTRATION_IMPLUI_H
/*===3rdtech===
  Copyright (c) 2001 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/

#include "nmb_Image.h"
#include "nmr_Registration_Interface.h"
#include "nmr_Registration_Impl.h"
#include "correspondenceEditor.h"

class nmb_ColorMap;
class nmr_Registration_Impl;

/// This class provides a graphical user-interface to the
/// nmr_Registration_Impl in order let the user edit correspondence 
/// information 
class nmr_Registration_ImplUI {
  public:
    nmr_Registration_ImplUI();

    ~nmr_Registration_ImplUI();
    void enable(vrpn_bool enable);
    void registerImages();
    void mainloop();

    void newScanline(nmr_ImageType whichImage,
                                vrpn_int32 row, nmb_Image *im);

    void setFiducial(vrpn_float32 x_src, vrpn_float32 y_src,
                     vrpn_float32 z_src, vrpn_float32 x_tgt, 
                     vrpn_float32 y_tgt, vrpn_float32 z_tgt);
    void setColorMap(nmr_ImageType whichImage, nmb_ColorMap * cmap);
    void setColorMinMax(nmr_ImageType whichImage, 
                              vrpn_float64 dmin, vrpn_float64 dmax,
                              vrpn_float64 cmin, vrpn_float64 cmax);
    void setImageOrientation(nmr_ImageType whichImage,
                              vrpn_bool flipX, vrpn_bool flipY);
    void getCorrespondence(Correspondence &c, int &srcIndex, int &tgtIndex);

    void registerCorrespondenceHandler(CorrespondenceCallback handler,void *ud);

  protected:
    static int s_numImages;
    static char *s_imageWinNames[];
    static int s_sourceImageIndex, s_targetImageIndex;
    vrpn_bool d_sourceFlipX, d_sourceFlipY;
    vrpn_bool d_targetFlipX, d_targetFlipY;

    static void handle_registration_start_change(vrpn_int32 val, 
                                                 void *userdata);
    static void handle_ServerMessage( void *ud,
                         const nmr_ServerChangeHandlerData &info);

    CorrespondenceEditor d_ce; // ui to display landmarks and
                              // let user set them

};

#endif
