#ifndef NMR_REGISTRATION_IMPL_H
#define NMR_REGISTRATION_IMPL_H

#include "nmr_Registration_Server.h"
#include "nmr_Registration_ImplUI.h"
#include "nmb_Image.h"
#include "correspondence.h"

enum {SOURCE_IMAGE_INDEX = 0, TARGET_IMAGE_INDEX = 1};

class nmr_Registration_ImplUI;

class nmr_Registration_Impl {
  public:
    nmr_Registration_Impl(nmr_Registration_Server *server = NULL);
    ~nmr_Registration_Impl();

    void mainloop();

    static void serverMessageHandler(void *ud,
                            const nmr_ServerChangeHandlerData &info);

    nmb_Image *getImage(nmr_ImageType type);
    int updateImage();

    int setImageParameters(nmr_ImageType whichImage,
                           vrpn_int32 res_x, vrpn_int32 res_y,
                           vrpn_bool treat_as_height_field);
    int setRegistrationEnable(vrpn_bool enable);
    int setGUIEnable(vrpn_bool enable);
    int setFiducial(nmr_ImageType whichImage,
                vrpn_float32 x, vrpn_float32 y, vrpn_float32 z);
    int setScanline(nmr_ImageType whichImage,
         vrpn_int32 row, vrpn_int32 length, vrpn_float32 *data);
    int setTransformationOptions(nmr_TransformationType type);
    int registerImages(Correspondence &c,
                 int corrSourceIndex, int corrTargetIndex, 
                 double *xform = NULL);
    int registerImages(double *xform);

  protected:
    nmb_Image *d_images[2];
    nmr_Registration_ImplUI *d_alignerUI;
    nmr_Registration_Server *d_server;
    nmr_TransformationType d_transformType;
};

#endif
