#ifndef NMB_TRANSFORM_TSCSHR_H
#define NMB_TRANSFORM_TSCSHR_H

#include <vrpn_Types.h>

/* 
  This class represents a 3D linear transformation including
  scale, shear, rotation with a translation (translation isn't a linear
  transformation). If M is the
  total transformation matrix where M transforms from the left
  (p' = M*p, for a 4 element column vector p)
  then

  M = T*Sc*Sh*R

  T = translation
  R = rotation
  Sh = shear
  Sc = scale

           1 0 0 tx
       T = 0 1 0 ty
           0 0 1 tz
           0 0 0 1

           cx, sx = cos and sin of angle about x axis
           cy, sy = cos and sin of angle about y axis
           cz, sz = cos and sin of angle about z axis

                    cy*cz    sx*sy*cz-cx*sz   cx*sy*cz+sx*sz  0
       R = RzRyRx = cy*sz    sx*sy*sz-cx*cz   cx*sy*sz-sx*cz  0
                    -sy      sx*cy            cx*cy           0
                    0        0                0               1

            1        shz           0   0
       Sh = shx*shy  1+shx*shy*shz shx 0
            shy      shy*shz       1   0
            0        0             0   1

            scx 0   0   0
       Sc = 0   scy 0   0
            0   0   scz 0
            0   0   0   1

  So there are 12 parameters total (3 translation offsets, 
  3 rotation angles, 3 shear factors, and 3 scale factors)

  This class provides functions to get the 4x4 matrix M and its
  derivatives with respect to each of the 12 intuitive parameters
*/

typedef enum {NMB_ROTATE_X, NMB_ROTATE_Y, NMB_ROTATE_Z,
      NMB_TRANSLATE_X, NMB_TRANSLATE_Y, NMB_TRANSLATE_Z,
      NMB_SCALE_X, NMB_SCALE_Y, NMB_SCALE_Z,
      NMB_SHEAR_X, NMB_SHEAR_Y, NMB_SHEAR_Z} nmb_TransformParameter;

typedef enum {NMB_X, NMB_Y, NMB_Z} nmb_Axis;

class nmb_Transform_TScShR {
  public:
    nmb_Transform_TScShR();
    nmb_Transform_TScShR & operator = (const nmb_Transform_TScShR &);

    void setIdentity();

    void setRotation(nmb_Axis axis, double angle_radians);
    double getRotation(nmb_Axis axis);
    void rotate(nmb_Axis axis, double delta_radians);

    void setTranslation(nmb_Axis axis, double translation);
    double getTranslation(nmb_Axis axis);
    void translate(nmb_Axis axis, double delta);
    void translate(double tx, double ty, double tz);

    void setScale(nmb_Axis axis, double scale);
    double getScale(nmb_Axis axis);
    void scale(nmb_Axis, double scale);
    void scale(double sx, double sy, double sz);
    
    void setShear(nmb_Axis axis, double shear);
    double getShear(nmb_Axis axis);
    void shear(nmb_Axis axis, double shear);

    void setCenter(double x, double y, double z);
    void getCenter(double &x, double &y, double &z);

    void transform(double *p_src, double *p_dest);
    void transform(double *pnt);

    // matrices are in column-major order
    void getMatrix(double *matrix);
    void getMatrixDerivative(double *matrix, nmb_TransformParameter param);
   
  protected:
    void updateMatrix();

    // angles in radians as well as their cosines and sines
    double d_rotation[3];
    double d_cosAngle[3];
    double d_sinAngle[3];
    // scale factors
    double d_scale[3];
    // shear factors
    double d_shear[3];
    // translation
    double d_translation[3];

    // origin for the transformation
    double d_pivotPoint[3];

    vrpn_bool d_matrixNeedsUpdate;
    double d_matrix[16];
};

#endif
