#ifndef NMR_MULTIRESOBJECTIVEMI_H
#define NMR_MULTIRESOBJECTIVEMI_H

#include "nmr_ObjectiveMI.h"

/* nmr_MultiResObjectiveMI
  This class is a multiresolution version of nmr_ObjectiveMI
*/
class nmr_MultiResObjectiveMI {
  public:
    nmr_MultiResObjectiveMI();
    nmr_MultiResObjectiveMI(int numLevels, float *stddev);
    ~nmr_MultiResObjectiveMI();

    int numLevels() {return d_numResolutionLevels;}
    void getBlurStdDev(float *stddev);

    /* **************
    functions that affect the number and meaning of parameters of the
    objective function
    ***************** */

    /// choose between 2D reference or 2.5D reference image
    void setDimensionMode(nmr_DimensionMode mode);
    nmr_DimensionMode getDimensionMode();

    /* **************
    functions that affect the value of the objective function and its gradient
    ***************** */

    /// set/get how big the samples are - this affects how noisy the
    /// objective function value is
    /// return value is 0 if successful or -1 if out of memory
    int setSampleSizes(int sizeA, int sizeB);
    void getSampleSizes(int &sizeA, int &sizeB);

    /// set whether to choose samples randomly, on a regular grid
    /// or randomly with a gradient magnitude-based sample rejection criterion
    void setSampleMode(nmr_SampleMode mode);
    void setMinSampleSqrGradientMagnitude(double mag);

    /// set images, if a list is supplied then multiple versions of the
    /// image will be added to the list for the various resolutions
    void setReferenceValueImage(nmb_Image *ref, 
                                nmb_ImageList *monitorList = NULL);
    void setTestValueImage(nmb_Image *test,
                           nmb_ImageList *monitorList = NULL);
    void setReferenceZImage(nmb_Image *ref_z,
                            nmb_ImageList *monitorList = NULL);
 
    /* **************
    objective function value and gradient
    ***************** */

    /// these functions compute objective function value and gradient vector
    /// as a function of the transformation (passed in as a 16 element
    /// array of doubles as used by nmb_TransformMatrix44)
    /// that takes points in the reference image (possibly augmented by
    /// a height value if a z value image has been set)
    /// this transformation should be in terms of pixels

    /// objective function value
    double value(int level, double *testFromReferenceTransform);

    /// get gradient vector
    void gradient(int level, double *testFromReferenceTransform, 
                  double *gradMI);

    /// since we can share some of the computation between value and
    /// gradient computations, if you need both you should call this
    /// function
    void valueAndGradient(int level, double *testFromReferenceTransform,
                double &valueMI, double *gradMI);

    int getJointHistogram(int level,
                           nmb_Image *histogram, 
                           double *transform, vrpn_bool blur, 
                           vrpn_bool setRefScale,
                           float min_ref, float max_ref,
                           vrpn_bool setTestScale,
                           float min_test, float max_test);

    void setCovariance(int level, double sigmaRefRef, double sigmaTestTest);
    void getCovariance(int level, double &sigmaRefRef, double &sigmaTestTest);
    void crossEntropyGradient(int level, double &dHc_dsigma_ref,
                                         double &dHc_dsigma_test);
    void crossEntropy(int level, double &entropy);

    void setTestVariance(int level, double sigma);
    void getTestVariance(int level, double &sigma);
    void testEntropyGradient(int level, double &dH_dsigma_test);
    void testEntropy(int level, double &entropy);

    void setRefVariance(int level, double sigma);
    void getRefVariance(int level, double &sigma);
    void refEntropyGradient(int level, double &dH_dsigma_ref);
    void refEntropy(int level, double &entropy);

  protected:
    int d_numResolutionLevels;
    float *d_stddev;
    nmr_ObjectiveMI *d_objectiveMI;
    static int s_defaultNumResolutionLevels;
    static float s_defaultStdDev[];
};

#endif
