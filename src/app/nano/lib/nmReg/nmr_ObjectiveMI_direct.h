#ifndef NMR_OBJECTIVEMI_DIRECT_H
#define NMR_OBJECTIVEMI_DIRECT_H

#include "nmb_Image.h"
#include "nmr_Objective.h"
#include "nmr_Histogram.h"

class nmr_ObjectiveMI_direct : public nmr_Objective {
  public:
    nmr_ObjectiveMI_direct();
    ~nmr_ObjectiveMI_direct();

    /* ************** 
    functions that affect the number and meaning of parameters of the
    objective function
    ***************** */

    /// choose between 2D reference or 2.5D reference image
    virtual void setDimensionMode(nmr_DimensionMode mode);
    virtual nmr_DimensionMode getDimensionMode();

    /* **************
    functions that affect the value of the objective function and its gradient
    ***************** */

    /// set images 
    virtual void setReferenceValueImage(nmb_Image *ref);
    virtual void setTestValueImage(nmb_Image *test);
    virtual void setReferenceZImage(nmb_Image *ref_z);

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
    virtual double value(double *testFromReferenceTransform);

    /// get gradient vector
    virtual void gradient(double *testFromReferenceTransform, double *gradMI);

    /// since we can share some of the computation between value and
    /// gradient computations, if you need both you should call this 
    /// function
    virtual void valueAndGradient(double *testFromReferenceTransform, 
                double &valueMI, double *gradMI);

	void getJointHistogramSize(int &numX, int &numY);
	void getJointHistogramImage(nmb_Image *image);

  protected:

    // inline helper functions
    double transform_x(double x, double y, double *T) 
      {  return T[0]*x + T[4]*y + T[12];  }
    double transform_y(double x, double y, double *T)
      {  return T[1]*x + T[5]*y + T[13];  }
    double transform_x(double x, double y, double z, double *T)
      {  return T[0]*x + T[4]*y + T[8]*z + T[12];  }
    double transform_y(double x, double y, double z, double *T)
      {  return T[1]*x + T[5]*y + T[9]*z + T[13];  }

    // does reference image correspond to a simple 2D projection or a 
    // height field?
    nmr_DimensionMode d_dimensionMode;

    // images
    nmb_Image *d_testValue;
    nmb_Image *d_gradX_test, *d_gradY_test;
    nmb_Image *d_refValue;
    nmb_Image *d_refZ;

    nmr_Histogram *d_testHistogram;
    nmr_Histogram *d_refHistogram;
    nmr_Histogram *d_jointHistogram;
    int d_numRefBins;
    int d_numTestBins;
};

#endif
