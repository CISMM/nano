#ifndef NMR_OBJECTIVE_H
#define NMR_OBJECTIVE_H

#include "nmb_Image.h"

typedef enum {REF_2D,        // treat reference image as a simple 2D image
              REF_HEIGHTFIELD // treat reference image as a 2.5D image
} nmr_DimensionMode;

class nmr_Objective {
  public:
    /// choose between 2D reference or 2.5D reference image
    virtual void setDimensionMode(nmr_DimensionMode mode) = 0;
    virtual nmr_DimensionMode getDimensionMode() = 0;

    /// set images
    virtual void setReferenceValueImage(nmb_Image *ref) = 0;
    virtual void setTestValueImage(nmb_Image *test) = 0;
    virtual void setReferenceZImage(nmb_Image *ref_z) = 0;

    /// objective function value
    virtual double value(double *testFromReferenceTransform) = 0;

    /// get gradient vector
    virtual void gradient(double *testFromReferenceTransform, double *grad) = 0;

    /// since we can share some of the computation between value and
    /// gradient computations, if you need both you should call this
    /// function
    virtual void valueAndGradient(double *testFromReferenceTransform,
                double &value, double *grad) = 0;

};

#endif
