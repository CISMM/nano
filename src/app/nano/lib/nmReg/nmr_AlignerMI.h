#ifndef NMR_ALIGNERMI_H
#define NMR_ALIGNERMI_H

#include "nmb_Image.h"

typedef enum {REF_2D,        // treat reference image as a simple 2D image
              REF_HEIGHTFIELD // treat reference image as a 2.5D image
} nmr_DimensionMode;

class nmr_AlignerMI {
  public:
    nmr_AlignerMI();
    ~nmr_AlignerMI();

    // choose between 2D reference or 2.5D reference image
    void setDimensionMode(nmr_DimensionMode mode);
    nmr_DimensionMode getDimensionMode();

    // set/get how big the samples are
    // return value is 0 if successful or -1 if out of memory 
    int setSampleSizes(int sizeA, int sizeB);
    void getSampleSizes(int &sizeA, int &sizeB);

    // set/get gaussian standard deviations for constructing Parzen 
    // window estimates of density
    void setCovariance(double sigmaRefRef, double sigmaTestTest);
    void getCovariance(double &sigmaRefRef, double &sigmaTestTest);

    void setTestVariance(double sigma);
    void getTestVariance(double &sigma);
    void setRefVariance(double sigma);
    void getRefVariance(double &sigma);

    // we only represent a single transformation but we provide different
    // ways to set it to make transformation parameter semantics explicit

    // set/get transformation
    // T should have 6 elements
    // returns -1 if the dimension mode is not set to REF_2D
    int setTransformation2D(double *T);
    int getTransformation2D(double *T);

    // set/get transformation for height field reference, 
    // T should have 8 elements
    // returns -1 if the dimension mode is not set to REF_HEIGHTFIELD
    int setTransformationHF(double *T);
    int getTransformationHF(double *T);

    // if the dimension mode is set for a 2.5D reference image then
    // a z value for each corresponding (x,y) position should be supplied in
    // the optional ref_z argument
    // returns  0 if successful and 
    //         -1 if ref_z value does not agree with dimension mode
    // [EXPENSIVE]
    int buildSampleA(nmb_Image *ref, nmb_Image *test, 
                     nmb_Image *grad_x_test, 
                     nmb_Image *grad_y_test, 
                     vrpn_bool randomize = vrpn_TRUE,
                     double minSqrGradientMagnitude = 0.0,
                     nmb_Image *ref_z = NULL);
    int buildSampleB(nmb_Image *ref, nmb_Image *test, 
                     nmb_Image *grad_x_test, 
                     nmb_Image *grad_y_test,
                     vrpn_bool randomize = vrpn_TRUE,
                     double minSqrGradientMagnitude = 0.0,
                     nmb_Image *ref_z = NULL);

    // fills in the joint histogram and blurs it with the current
    // covariance matrix - the resolution is determined by the
    // resolution of the image passed in and min and max values are
    // optional
    int buildJointHistogram(nmb_Image *ref, nmb_Image *test,
                            nmb_Image *histogram, vrpn_bool blur = vrpn_FALSE,
                            vrpn_bool setRefScale = VRPN_FALSE,
                            float min_ref = 0.0, float max_ref = 0.0,
                            vrpn_bool setTestScale = VRPN_FALSE,
                            float min_test = 0.0, float max_test = 0.0);

    // compute gradient of cross-entropy with respect to sigma
    // return values:
    //  0 = successful
    // -1 = error, need to call buildSampleA and buildSampleB 
    //      after transformation is set
    // [EXPENSIVE]
    int computeCrossEntropyGradient(double &dHc_dsigma_ref, 
                                    double &dHc_dsigma_test);

    // cross-entropy computed using the new values for sigmaRef and sigmaTest
    // (cross-entropy should tend to minimized as we step along - or at least
    //  this is the goal)
    // return values:
    //   0 = successful
    //  -1 = error, need to call buildSampleA and buildSampleB 
    //       after transformation is set
    // [EXPENSIVE]
    int crossEntropy(double &entropy);

    // compute gradient of test image entropy with respect to sigma
    // return values:
    //  0 = successful
    // -1 = error, need to call buildSampleA and buildSampleB
    //      after transformation is set
    // [EXPENSIVE]
    int computeTestEntropyGradient(double &dH_dsigma_test);

    // compute entropy of test image
    //  0 = successful
    // -1 = error, need to call buildSampleA and buildSampleB
    //      after transformation is set
    // [EXPENSIVE]
    int testEntropy(double &entropy);

    // compute gradient of ref image entropy with respect to sigma
    // return values:
    //  0 = successful
    // -1 = error, need to call buildSampleA and buildSampleB
    //      after transformation is set
    // [EXPENSIVE]
    int computeRefEntropyGradient(double &dH_dsigma_ref);

    // compute entropy of reference image
    //  0 = successful
    // -1 = error, need to call buildSampleA and buildSampleB
    //      after transformation is set
    // [EXPENSIVE]
    int refEntropy(double &entropy);

    // compute gradient of mutual information from last sample to optimize T
    // return values:
    //   0 = successful
    //  -1 = error, need to call buildSampleA and buildSampleB 
    //       after transformation is set
    // [EXPENSIVE]
    int computeTransformationGradient(double *dIdT);

    // compute the mutual information from last sample
    // return values:
    //   0 = successful
    //  -1 = error, need to call buildSampleA and buildSampleB 
    //       after transformation is set
    // [EXPENSIVE]
    int computeMutualInformation(double &mutualInfo);

  protected:
    // helper for computeTransformationGradient()
    int computeTransformationGradient_HeightfieldRef(double *dIdT);

    // helper for buildSampleA and buildSampleB
    int buildSampleHelper(nmb_Image *ref, nmb_Image *test,
                     nmb_Image *grad_x_test,
                     nmb_Image *grad_y_test,
                     vrpn_bool randomize,
                     nmb_Image *ref_z,
       int numPoints, double *x_ref, double *y_ref, double *z_ref,
       double *x_test, double *y_test, double *val_ref, double *val_test,
       double *dtest_dx, double *dtest_dy, double minSqrGradientMagnitude);

    // inline helper functions
    double transform_x(double x, double y) 
      {  return d_T[0]*x + d_T[1]*y + d_T[3];  }
    double transform_y(double x, double y)
      {  return d_T[4]*x + d_T[5]*y + d_T[7];  }
    double transform_x(double x, double y, double z)
      {  return d_T[0]*x + d_T[1]*y + d_T[2]*z + d_T[3];  }
    double transform_y(double x, double y, double z)
      {  return d_T[4]*x + d_T[5]*y + d_T[6]*z + d_T[7];  }

    // allocation/deallocation reference for the x,y,z, and intensity arrays
    // for the A and B samples
    double *d_workspace;

    // has buildSampleA been called since transformation matrix changed?
    vrpn_bool d_sampleA_acquired; 
    
    int d_sizeA; // size of sample A
    // x,y,z and intensity values for sample A:
    double *d_Ax_ref, *d_Ay_ref, *d_Az_ref, *d_Ax_test, *d_Ay_test;
    double *d_refValuesA, *d_testValuesA;
    double *d_dTestValueA_dx, *d_dTestValueA_dy;

    // has buildSampleB been called since transformation matrix changed?
    vrpn_bool d_sampleB_acquired;
    
    int d_sizeB; // size of sample B
    // x,y,z and intensity values for sample B:
    double *d_Bx_ref, *d_By_ref, *d_Bz_ref, *d_Bx_test, *d_By_test;
    double *d_refValuesB, *d_testValuesB;
    double *d_dTestValueB_dx, *d_dTestValueB_dy;

    // transformation matrix for both 2D and 2.5D cases
    double d_T[8];

    // representation for sqrt of covariance matrix (assumed to be diagonal)
    double d_sigmaRefRef, d_sigmaTestTest;

    // standard deviation for Parzen windowing of test image intensity 
    // distribution
    double d_sigmaTest;

    // standard deviation for Parzen windowing of ref image intensity
    // distribution
    double d_sigmaRef;

    // does reference image correspond to a simple 2D projection or a 
    // height field?
    nmr_DimensionMode d_dimensionMode;
};

#endif
