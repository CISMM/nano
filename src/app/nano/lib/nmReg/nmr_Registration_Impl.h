#ifndef NMR_REGISTRATION_IMPL_H
#define NMR_REGISTRATION_IMPL_H

#include "nmr_Registration_Server.h"
#include "nmr_Registration_ImplUI.h"
#include "nmb_Image.h"
#include "correspondence.h"
#include "nmr_CoarseToFineSearch.h"
#include "nmb_Transform_TScShR.h"

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
            vrpn_float32 xSizeWorld, vrpn_float32 ySizeWorld);
    int setRegistrationEnable(vrpn_bool enable);
    int setGUIEnable(vrpn_bool enable);
    int setFiducial(nmr_ImageType whichImage,
                vrpn_float32 x, vrpn_float32 y, vrpn_float32 z);
    int setScanline(nmr_ImageType whichImage,
         vrpn_int32 row, vrpn_int32 length, vrpn_float32 *data);
    int setTransformationOptions(nmr_TransformationType type);
    int registerImagesFromPointCorrespondence(double *xform);
    int registerImagesUsingMutualInformation(nmb_Transform_TScShR &xform,
                              vrpn_bool initialize = vrpn_FALSE);
    void sendResult(double *xform);

  protected:
    void ensureThreePoints(Correspondence &c,  int corrSourceIndex, 
                      vrpn_bool normalized, vrpn_bool lookupZ);
    void convertTo3DSpace(Correspondence &c, int corrSourceIndex);
    int registerImagesFromPointCorrespondence(Correspondence &c,
                 int corrSourceIndex, int corrTargetIndex,
                 double *xform = NULL);
    int adjustTransformFromRotatedCorrespondence(Correspondence &c,
                 int corrSourceIndex, int corrTargetIndex, 
                 nmb_Transform_TScShR &xform);
    void convertRyRxToViewingDirection(double Ry, double Rx, 
                         double &vx, double &vy, double &vz);
    void convertViewingDirectionToRyRx(double vx, double vy, double vz,
                                       double &Ry, double &Rx);


    nmb_Image *d_images[2];
    nmr_Registration_ImplUI *d_alignerUI;
    nmr_Registration_Server *d_server;
    nmr_TransformationType d_transformType;

    nmr_CoarseToFineSearch d_mutInfoAligner;
    vrpn_bool d_imageChangeSinceLastRegistration;
};

#endif
