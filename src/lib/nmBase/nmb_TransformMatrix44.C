#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "nmb_TransformMatrix44.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

nmb_TransformMatrix44::nmb_TransformMatrix44()
{
    buildIdentity(xform);
    buildIdentity(inverse_xform);
    inverse_needs_to_be_computed = vrpn_FALSE;
}

nmb_TransformMatrix44 & nmb_TransformMatrix44::operator = (
                              const nmb_TransformMatrix44 & t) {
  int i,j;
  for (i = 0; i < 4; i++) {
    for (j = 0; j < 4; j++) {
      xform[i][j] = t.xform[i][j];
      inverse_xform[i][j] = t.xform[i][j];
    }
  }
  inverse_needs_to_be_computed = t.inverse_needs_to_be_computed;
  inverse_valid = t.inverse_valid;

  return *this;
}

void nmb_TransformMatrix44::print()
{
    int i,j;
    printf("xform:\n");
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++)
            printf("%g ", xform[i][j]);
        printf("\n");
    }

    printf("inv_xform:\n");
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++)
            printf("%g ", inverse_xform[i][j]);
        printf("\n");
    }
}

void nmb_TransformMatrix44::set(int i_dest, int i_src, double value) {
    assert(i_dest >= 0 && i_dest < 4 && i_src >= 0 && i_src < 4);
    xform[i_dest][i_src] = value;
    if (!inverse_needs_to_be_computed){
//        printf("nmb_TransformMatrix44::set: inverse dirty\n");
        inverse_needs_to_be_computed = vrpn_TRUE;
    }
}

void nmb_TransformMatrix44::setMatrix(double *matrix)
{
    int i,j,k = 0;
    for (j = 0; j < 4; j++) {
        for (i = 0; i < 4; i++) {
            xform[i][j] = matrix[k];
            k++;
        }
    }
    inverse_needs_to_be_computed = vrpn_TRUE;
}

void nmb_TransformMatrix44::getMatrix(double *matrix)
{
/*
    matrix[0,1,2,3] = whatever we transform (1,0,0,0) into which is
    xform[0][0], xform[1][0], xform[2][0], xform[3][0]
    matrix[4,5,6,7] = whatever we transform (0,1,0,0) into which is
    xform[0][1], xform[1][1], xform[2][1], xform[3][1]
    ...
*/
    int i,j,k = 0;
    for (j = 0; j < 4; j++) {
        for (i = 0; i < 4; i++) {
            matrix[k] = xform[i][j];
            k++;
        }
    }
}

void nmb_TransformMatrix44::compose(nmb_TransformMatrix44 &m)
{
    int i,j,k;
    double p[4][4];
    for (i = 0; i < 4; i++){
        for (j = 0; j < 4; j++){
            p[i][j] = 0.0;
            for (k = 0; k < 4; k++){
                p[i][j] += xform[i][k]*m.xform[k][j];
//                 p[i][j] += xform[k][j]*m.xform[i][k];
            }
        }
    }
    for (i = 0; i < 4; i++){
        for (j = 0; j < 4; j++){
            xform[i][j] = p[i][j];
        }
    }
    inverse_needs_to_be_computed = vrpn_TRUE;
}

void nmb_TransformMatrix44::transform(double *p_src, double *p_dest) const {

    for (int i = 0; i < 4; i++){
        p_dest[i] = 0;
        for (int j = 0; j < 4; j++){
            p_dest[i] += xform[i][j]*p_src[j];
        }
    }
}

void nmb_TransformMatrix44::invTransform(double *p_src, double *p_dest) {
    if (!hasInverse()) {
        fprintf(stderr, "nmb_TransformMatrix44::invTransform: Warning,"
               " failed use of inverse (non-invertible transform)\n");
        return;
    }
//    printf("invTransform\n");
//    print();

    double result[4] = {0,0,0,0};
    int i,j;
    for (i = 0; i < 4; i++){
        for (j = 0; j < 4; j++){
            result[i] += inverse_xform[i][j]*p_src[j];
        }
    }
    for (i = 0; i < 4; i++){
        p_dest[i] = result[i];
    }
}

void nmb_TransformMatrix44::invert() {
    if (!hasInverse()) {
        fprintf(stderr, "nmb_TransformMatrix44::invert: Warning,"
               " failed use of invert (non-invertible transform)\n");
        return;
    }
    double swap;
    int i,j;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            swap = xform[i][j];
            xform[i][j] = inverse_xform[i][j];
            inverse_xform[i][j] = swap;
        }
    }

}

nmb_TransformMatrix44 *nmb_TransformMatrix44::duplicate() const {
    nmb_TransformMatrix44 *ita = new nmb_TransformMatrix44();
    for (int i = 0; i < 4; i++){
        for (int j = 0; j < 4; j++){
            ita->xform[i][j] = xform[i][j];
        }
    }
    return (nmb_TransformMatrix44 *)ita;
}

void nmb_TransformMatrix44::buildIdentity(double m[4][4])
{
    int i,j;
    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
            m[i][j] = (double)(i==j);
}


/* computeInverse() - adapted from code in Matrix44.cpp by Dave McAllister */
vrpn_bool nmb_TransformMatrix44::computeInverse() {
//    printf("before computeInverse\n");
//    print();
    inverse_needs_to_be_computed = vrpn_FALSE;
    int i,j;
    double p[4][4];
    buildIdentity(p);
    // copy xform into inverse_xform
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++){
            inverse_xform[i][j] = xform[i][j];
        }
    }

    // Make it upper triangular using Gauss-Jordan with partial pivoting.
    for (i=0; i<4; i++) 
    {
        // Find largest row.
        double max=fabs(inverse_xform[i][i]);
        int row = i;
        int j;
        for (j=i+1; j<4; j++)
        {
            if (fabs(inverse_xform[j][i]) > max)
            {
                max = fabs(inverse_xform[j][i]);
                row = j;
            }
        }
        
        // Pivot around largest row.
        if (max <= 0) {
            return vrpn_FALSE;
        }
        if (row != i) {
            switch_rows(inverse_xform, i, row);
            switch_rows(p, i, row);
        }

        // Subtract scaled rows to eliminate column i.
        if (inverse_xform[i][i] == 0){
            // we're in trouble here
            return vrpn_FALSE;
        }
        double denom = 1./inverse_xform[i][i];
        for (j=i+1; j<4; j++)
        {
            double factor = inverse_xform[j][i] * denom;
            sub_rows(inverse_xform, j, i, factor);
            sub_rows(p, j, i, factor);
        }
    }

    // Diagonalize inverse_xform using Jordan.
    for (i=1; i<4; i++)
    {
        if (inverse_xform[i][i] == 0){
            // we're in trouble here
            return vrpn_FALSE;
        }
        double denom = 1./inverse_xform[i][i];
        for (int j=0; j<i; j++)
        {
            double factor = inverse_xform[j][i] * denom;
            sub_rows(inverse_xform, j, i, factor);
            sub_rows(p, j, i, factor);
        }
    }

    // Normalize inverse_xform to the identity and copy p over inverse_xform.
    for(i=0; i<4; i++)
    {
        if (inverse_xform[i][i] == 0){
            // we're in trouble here
            return vrpn_FALSE;
        }
        double factor = 1./inverse_xform[i][i];
        for (int j=0; j<4; j++)
        {
            // As if we were doing inverse_xform[i][j] *= factor
            p[i][j] *= factor;
            inverse_xform[i][j] = p[i][j];
        }
    }
//    printf("end of computeInverse\n");
//    print();

    return vrpn_TRUE;
}

vrpn_bool nmb_TransformMatrix44::hasInverse() {
    if (inverse_needs_to_be_computed) {
//        printf("computing inverse\n");
        inverse_valid = computeInverse();
    } else {
//        printf("not computing inverse\n");
    }
    return inverse_valid;
}

// does x and y output depend only on the x and y input?
vrpn_bool nmb_TransformMatrix44::is2D() {
  vrpn_bool result = (xform[0][2] == 0 && xform[1][2] == 0);
  return result; 
}

vrpn_bool nmb_TransformMatrix44::getTScShR_2DParameters(
                        double centerX, double centerY,
                        double &tx, double &ty,
                        double &phi, double &shz,
                        double &scx, double &scy)
{
  if (!is2D()) return vrpn_FALSE;
/* The assumed form of the xform matrix for which we extract these parameters 
   is as follows:

(this is mathematica notation since I used it to help me calculate some of this)
R = {{Cos[phi], -Sin[phi], 0, 0},
      {Sin[phi], Cos[phi], 0, 0},
      {0, 0, 1, 0},
      {0, 0, 0, 1}};

T = {{1, 0, 0, tx}, {0, 1, 0, ty}, {0, 0, 1, 0}, {0,0,0,1}};

Sh = {{1, shz, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0,0,0,1}};

Sc = {{scx, 0, 0, 0}, {0, scy, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}};

Pivot = {centerX, centerY, 0, 1};
TinvPivot = {{1, 0, 0, -Pivot[[1]]}, {0, 1, 0, -Pivot[[2]]},
      {0, 0, 1, -Pivot[[3]]}, {0, 0, 0, 1}};
Tpivot = {{1, 0, 0, Pivot[[1]]}, {0, 1, 0, Pivot[[2]]},
      {0, 0, 1, Pivot[[3]]}, {0, 0, 0, 1}};
// Warning, this doesn't produce a general affine transformation: 
//            M = PostPivotT.Sc.R.Sh.PrePivotT.T;

M = T.Tpivot.Sc.Sh.R.TinvPivot;

M = {{scx*Cos[phi] + scx*shz*Sin[phi], scx*shz*Cos[phi] - scx*Sin[phi], 0,
      centerX + tx + (-centerY - ty)*(scx*shz*Cos[phi] - scx*Sin[phi]) +
       ty*(scx*shz*Cos[phi] - scx*Sin[phi]) + (-centerX - tx)*
        (scx*Cos[phi] + scx*shz*Sin[phi]) +
       tx*(scx*Cos[phi] + scx*shz*Sin[phi])}, {scy*Sin[phi], scy*Cos[phi], 0,
      centerY + ty + scy*(-centerY - ty)*Cos[phi] + scy*ty*Cos[phi] +
       scy*(-centerX - tx)*Sin[phi] + scy*tx*Sin[phi]}, {0, 0, 1, 0},
     {0, 0, 0, 1}}

eqns={xform00==M[[1,1]], xform01==M[[1,2]], xform03==M[[1,4]],
      xform10==M[[2,1]], xform11==M[[2,2]], xform13==M[[2,4]]};

Solution = Solve[eqns, {scx, scy, phi, shz, tx, ty}];
Save["transform2D.txt", {R, Sh, Sc, T, M, Solution}];

*/

  double y_mag = sqrt(xform[1][0]*xform[1][0] + xform[1][1]*xform[1][1]);
  double y_mag_inv = 1.0/y_mag;
  double det = xform[0][0]*xform[1][1] - xform[0][1]*xform[1][0]; 
  double det_inv = 1.0/det;

  tx = -centerX + centerX*xform[0][0] + centerY*xform[0][1] + xform[0][3];
  ty = -centerY + centerX*xform[1][0] + centerY*xform[1][1] + xform[1][3];

  scx = det*y_mag_inv;
  scy = y_mag;
  shz = (xform[0][0]*xform[1][0] + xform[0][1]*xform[1][1])*det_inv;
  double cos_phi = xform[1][1]*y_mag_inv;
  double sin_phi = xform[1][0]*y_mag_inv;
  if (sin_phi > 0) {
    phi = acos(cos_phi);
  } else {
    phi = 2.0*M_PI-acos(cos_phi);
  }

  return vrpn_TRUE;
}
