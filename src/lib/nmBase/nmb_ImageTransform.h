#ifndef NMB_IMAGETRANSFORM_H
#define NMB_IMAGETRANSFORM_H

#include "nmb_Image.h"
#include "vrpn_Types.h"
#include <assert.h>

/*
	This class is for representing an arbitrary transformation
that takes points in one image to points in another image. In some cases
we may need something more general than a 4x4 transformation matrix.

*/

class nmb_ImageTransform {
  public:
    nmb_ImageTransform(int d_src, int d_dest):dim_src(d_src), dim_dest(d_dest){}
    virtual void transform(double *p_src, double *p_dest) const = 0;
    virtual void invTransform(double *p_src, double *p_dest) = 0;
    virtual void invert() = 0;
    virtual nmb_ImageTransform *duplicate() const = 0;
    virtual int dimSrc() const {return dim_src;}
    virtual int dimDest() const {return dim_dest;}
    virtual vrpn_bool hasInverse() = 0;

  protected:
    int dim_src, dim_dest;	// image dimensions
};

typedef nmb_ImageTransform *nmb_ImageTransformPtr;

// this transformation represents as much as a 4x4 transformation matrix
class nmb_ImageTransformAffine : public nmb_ImageTransform {
  public:
    // initialize to identity transform
    nmb_ImageTransformAffine(int d_src, int d_dest);
    void set(int i_dest, int i_src, double value);
    void setMatrix(vrpn_float64 *matrix);
    void getMatrix(vrpn_float64 *matrix);
    void compose(nmb_ImageTransformAffine &m);
    virtual void transform(double *p_src, double *p_dest) const;
    virtual void invTransform(double *p_src, double *p_dest);
    virtual void invert();
    virtual nmb_ImageTransform *duplicate() const;
    virtual vrpn_bool hasInverse();
    virtual void print();

  protected:
    void buildIdentity(double m[4][4]);
    vrpn_bool computeInverse();
    inline void switch_rows(double m[4][4], int r1, int r2) const
    {
        for(int i=0;i<4;i++){
            double tmp=m[r1][i];
            m[r1][i]=m[r2][i];
            m[r2][i]=tmp;
        }
    }
    inline void sub_rows(double m[4][4], int r1, int r2, double mul) const
    {
        for(int i=0;i<4;i++)
            m[r1][i] -= m[r2][i]*mul;
    }


    double xform[4][4];
    double inverse_xform[4][4];
    vrpn_bool inverse_needs_to_be_computed;
    vrpn_bool inverse_valid;
};



// this transformation represents a mapping from each pixel in the source
// image to a point in the destination image
/*
class nmb_ImageTransformPerPixel : public nmb_ImageTransform {
};
*/

#endif
