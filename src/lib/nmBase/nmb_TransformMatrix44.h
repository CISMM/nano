#ifndef NMB_TRANSFORMMATRIX44_H
#define NMB_TRANSFORMMATRIX44_H

#include <vrpn_Types.h>

class nmb_TransformMatrix44 {
  public:
    nmb_TransformMatrix44();
    nmb_TransformMatrix44 & operator = (const nmb_TransformMatrix44 &);

    void set(int i_dest, int i_src, double value);
    void setMatrix(double *matrix);
    void getMatrix(double *matrix);
    void compose(nmb_TransformMatrix44 &m);
    virtual void transform(double *p_src, double *p_dest) const;
    virtual void transform(double *pnt) const;
    virtual void invTransform(double *p_src, double *p_dest);
    virtual void invTransform(double *pnt);
    virtual void invert();
    virtual nmb_TransformMatrix44 *duplicate() const;
    virtual vrpn_bool hasInverse();
    virtual vrpn_bool is2D();
    // assuming the transformation is only in the x-y plane and
    // the order is Translate.Rotate.Shear.Scale and the rotation,
    // shear and scaling part of the transformation is about the point given
    // by (centerX, centerY), what are the 6 parameters 
    // of the transformation (tx, ty, phi, shz, scx, scy)
    virtual vrpn_bool getTScShR_2DParameters(double centerX, double centerY,
                        double &tx, double &ty,
                        double &phi, double &shz,
                        double &scx, double &scy);
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

#endif
