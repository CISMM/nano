#ifndef NMR_REGISTRATION_IMPL_H
#define NMR_REGISTRATION_IMPL_H

#include "nmr_Registration_Server.h"
#include "nmr_Registration_ImplUI.h"
#include "nmb_Image.h"
#include "correspondence.h"
#include "nmb_Transform_TScShR.h"
#include "nmr_AlignerMI.h"

enum {SOURCE_IMAGE_INDEX = 0, TARGET_IMAGE_INDEX = 1};

class nmr_Registration_ImplUI;
class nmb_ColorMap;

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
            vrpn_float32 xSizeWorld, vrpn_float32 ySizeWorld,
            vrpn_bool flipX, vrpn_bool flipY);
    int setAutoAlignEnable(vrpn_bool enable);
    int setGUIEnable(vrpn_bool enable);
    int setColorMap(nmr_ImageType whichImage, nmb_ColorMap * cmap);
    int setColorMinMax(nmr_ImageType whichImage, 
                              vrpn_float64 dmin, vrpn_float64 dmax,
                              vrpn_float64 cmin, vrpn_float64 cmax);
    int setFiducial(vrpn_float32 x_src, vrpn_float32 y_src, vrpn_float32 z_src,
                    vrpn_float32 x_tgt, vrpn_float32 y_tgt, vrpn_float32 z_tgt);
    int setScanline(nmr_ImageType whichImage,
         vrpn_int32 row, vrpn_int32 length, vrpn_float32 *data);
    int setTransformationOptions(nmr_TransformationType type);
    int registerImagesFromPointCorrespondence(double *xform);
    int registerImagesUsingMutualInformation(nmb_Transform_TScShR &xform);
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

    vrpn_bool d_imageChangeSinceLastRegistration;
    nmr_AlignerMI d_mutInfoAligner;
    // for auto-alignment
    vrpn_int32 d_numLevels;
    vrpn_float32 d_stddev[NMR_MAX_RESOLUTION_LEVELS];
    vrpn_int32 d_resolutionIndex;
    vrpn_int32 d_maxIterations;
    vrpn_float32 d_stepSize;
};

#endif
