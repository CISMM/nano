#ifndef NMR_ALIGNERMI_H
#define NMR_ALIGNERMI_H

#include "nmb_Image.h"
#include "nmr_MultiResObjectiveMI.h"
#include "nmb_Transform_TScShR.h"

class nmr_AlignerMI {
  public:
    nmr_AlignerMI();
    ~nmr_AlignerMI();

    void initImages(nmb_Image *ref, nmb_Image *test, nmb_Image *ref_z = NULL,
                    nmb_ImageList *monitorList = NULL);
    void initImages(nmb_Image *ref, nmb_Image *test, 
                    int numLevels, float *stddev, 
                    nmb_Image *ref_z = NULL, nmb_ImageList *monitorList = NULL);
    void optimizeVarianceParameters();
    void optimizeTransform();
    void takeGradientSteps(int resolutionIndex, int numSteps, float stepSize);
    // transform is assumed to be in pixel units
    void setTransform(nmb_Transform_TScShR &xform);
    void getTransform(nmb_Transform_TScShR &xform);
    void plotObjective(FILE *outf);
    int getNumLevels();
    void getBlurStdDev(float *stddev);

  protected:
    nmr_MultiResObjectiveMI *d_objective;
    nmb_Transform_TScShR d_transform;
    int d_refWidth, d_refHeight;
    int d_testWidth, d_testHeight;
};

#endif
