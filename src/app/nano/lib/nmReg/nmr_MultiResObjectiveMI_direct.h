#ifndef NMR_MULTIRESOBJECTIVEMI_DIRECT_H
#define NMR_MULTIRESOBJECTIVEMI_DIRECT_H

#include "nmr_ObjectiveMI_direct.h"

/* nmr_MultiResObjectiveMI_direct
  This class is a multiresolution version of nmr_ObjectiveMI_direct
*/
class nmr_MultiResObjectiveMI_direct {
  public:
    nmr_MultiResObjectiveMI_direct();
    nmr_MultiResObjectiveMI_direct(int numLevels, float *stddev);
    ~nmr_MultiResObjectiveMI_direct();

    int numLevels() {return d_numResolutionLevels;}
    void getBlurStdDev(float *stddev);
    int getLevelByScaleOrder(int order);

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
	void getJointHistogramSize(int level, int &numX, int &numY);
	void getJointHistogramImage(int level, nmb_Image *image);

  protected:
    int d_numResolutionLevels;
    float *d_stddev;
    int *d_sortOrder; // gives the order of the scales from largest to smallest
    nmr_ObjectiveMI_direct *d_objectiveMI;
    static int s_defaultNumResolutionLevels;
    static float s_defaultStdDev[];
};

#endif
