#include <math.h>
#include <stdio.h>
#include "nmb_Transform_TScShR.h"

nmb_Transform_TScShR::nmb_Transform_TScShR():
  d_matrixNeedsUpdate(vrpn_FALSE)
{
  setIdentity();
}

nmb_Transform_TScShR & nmb_Transform_TScShR::operator = (
                              const nmb_Transform_TScShR & t) {
  d_matrixNeedsUpdate = t.d_matrixNeedsUpdate;
  int i;
  if (!d_matrixNeedsUpdate) {
    for (i = 0; i < 16; i++) {
      d_matrix[i] = t.d_matrix[i];
    }
  }
  for (i = 0; i < 3; i++) {
    d_rotation[i] = t.d_rotation[i];
    d_cosAngle[i] = t.d_cosAngle[i];
    d_sinAngle[i] = t.d_sinAngle[i];
    d_scale[i] = t.d_scale[i];
    d_shear[i] = t.d_shear[i];
    d_translation[i] = t.d_translation[i];
    d_pivotPoint[i] = t.d_pivotPoint[i];
  }
  return *this;
}

void nmb_Transform_TScShR::setIdentity()
{
  d_matrixNeedsUpdate = vrpn_FALSE;
  int i;
  for (i = 0; i < 3; i++) {
    d_rotation[i] = 0.0;
    d_cosAngle[i] = 1.0;
    d_sinAngle[i] = 0.0;
    d_scale[i] = 1.0;
    d_shear[i] = 0.0;
    d_translation[i] = 0.0;
    d_pivotPoint[i] = 0.0;
  }
  for (i = 0; i < 16; i++) {
    d_matrix[i] = 0.0;
  }
  d_matrix[0] = 1.0;
  d_matrix[5] = 1.0;
  d_matrix[10] = 1.0;
  d_matrix[15] = 1.0;
}

void nmb_Transform_TScShR::setRotation(nmb_Axis axis, 
                                         double angle_radians)
{
  switch (axis) {
    case NMB_X:
     d_rotation[0] = angle_radians;
     d_cosAngle[0] = cos(angle_radians);
     d_sinAngle[0] = sin(angle_radians);
     break;
    case NMB_Y:
     d_rotation[1] = angle_radians;
     d_cosAngle[1] = cos(angle_radians);
     d_sinAngle[1] = sin(angle_radians);
     break;
    default:
     d_rotation[2] = angle_radians;
     d_cosAngle[2] = cos(angle_radians);
     d_sinAngle[2] = sin(angle_radians);
     break;
  }
  d_matrixNeedsUpdate = vrpn_TRUE;
}

double nmb_Transform_TScShR::getRotation(nmb_Axis axis)
{
  double result = 0;
  switch (axis) {
    case NMB_X:
     result = d_rotation[0];
     break;
    case NMB_Y:
     result = d_rotation[1];
     break;
    default:
     result = d_rotation[2];
     break;
  }
  return result;
}

void nmb_Transform_TScShR::rotate(nmb_Axis axis, 
                                                double delta_radians)
{
  switch (axis) {
    case NMB_X:
     d_rotation[0] += delta_radians;
     d_cosAngle[0] = cos(d_rotation[0]);
     d_sinAngle[0] = sin(d_rotation[0]);
     break;
    case NMB_Y:
     d_rotation[1] += delta_radians;
     d_cosAngle[1] = cos(d_rotation[1]);
     d_sinAngle[1] = sin(d_rotation[1]);
     break;
    default:
     d_rotation[2] += delta_radians;
     d_cosAngle[2] = cos(d_rotation[2]);
     d_sinAngle[2] = sin(d_rotation[2]);
     break;
  }
  d_matrixNeedsUpdate = vrpn_TRUE;
}

void nmb_Transform_TScShR::setTranslation(nmb_Axis axis, 
                                                        double translation)
{
  switch (axis) {
    case NMB_X:
     d_translation[0] = translation;
     break;
    case NMB_Y:
     d_translation[1] = translation;
     break;
    default:
     d_translation[2] = translation;
     break;
  }
  d_matrixNeedsUpdate = vrpn_TRUE;
}

double nmb_Transform_TScShR::getTranslation(nmb_Axis axis)
{
  double result = 0;
  switch (axis) {
    case NMB_X:
     result = d_translation[0];
     break;
    case NMB_Y:
     result = d_translation[1];
     break;
    default:
     result = d_translation[2];
     break;
  }
  return result;
}

void nmb_Transform_TScShR::translate(nmb_Axis axis, double delta)
{
  switch (axis) {
    case NMB_X:
     d_translation[0] += delta;
     break;
    case NMB_Y:
     d_translation[1] += delta;
     break;
    default:
     d_translation[2] += delta;
     break;
  }
  d_matrixNeedsUpdate = vrpn_TRUE;
}

void nmb_Transform_TScShR::translate(double tx, double ty, double tz)
{
  d_translation[0] += tx;
  d_translation[1] += ty;
  d_translation[2] += tz;
  d_matrixNeedsUpdate = vrpn_TRUE;
}

void nmb_Transform_TScShR::setScale(nmb_Axis axis, double scale)
{
  switch (axis) {
    case NMB_X:
     d_scale[0] = scale;
     break;
    case NMB_Y:
     d_scale[1] = scale;
     break;
    default:
     d_scale[2] = scale;
     break;
  }
  d_matrixNeedsUpdate = vrpn_TRUE;
}

double nmb_Transform_TScShR::getScale(nmb_Axis axis)
{
  double result = 0;
  switch (axis) {
    case NMB_X:
     result = d_scale[0];
     break;
    case NMB_Y:
     result = d_scale[1];
     break;
    default:
     result = d_scale[2];
     break;
  }
  return result;
}

void nmb_Transform_TScShR::scale(nmb_Axis axis, double scale)
{
  switch (axis) {
    case NMB_X:
     d_scale[0] *= scale;
     break;
    case NMB_Y:
     d_scale[1] *= scale;
     break;
    default:
     d_scale[2] *= scale;
     break;
  }
  d_matrixNeedsUpdate = vrpn_TRUE;
}

void nmb_Transform_TScShR::scale(double sx, double sy, double sz)
{
  d_scale[0] *= sx;
  d_scale[1] *= sy;
  d_scale[2] *= sz;
  d_matrixNeedsUpdate = vrpn_TRUE;
}

void nmb_Transform_TScShR::setShear(nmb_Axis axis, double shear)
{
  switch (axis) {
    case NMB_X:
     d_shear[0] = shear;
     break;
    case NMB_Y:
     d_shear[1] = shear;
     break;
    default:
     d_shear[2] = shear;
     break;
  }
  d_matrixNeedsUpdate = vrpn_TRUE;
}

double nmb_Transform_TScShR::getShear(nmb_Axis axis)
{
  double result = 0;
  switch (axis) {
    case NMB_X:
     result = d_shear[0];
     break;
    case NMB_Y:
     result = d_shear[1];
     break;
    default:
     result = d_shear[2];
     break;
  }
  return result;
}

void nmb_Transform_TScShR::shear(nmb_Axis axis, double shear)
{
  switch (axis) {
    case NMB_X:
     d_shear[0] += shear;
     break;
    case NMB_Y:
     d_shear[1] += shear;
     break;
    default:
     d_shear[2] += shear;
     break;
  }
  d_matrixNeedsUpdate = vrpn_TRUE;
}

void nmb_Transform_TScShR::setCenter(double x, double y, double z)
{
  d_pivotPoint[0] = x;
  d_pivotPoint[1] = y;
  d_pivotPoint[2] = z;
  d_matrixNeedsUpdate = vrpn_TRUE;
}

void nmb_Transform_TScShR::getCenter(double &x, double &y, double &z)
{
  x = d_pivotPoint[0];
  y = d_pivotPoint[1];
  z = d_pivotPoint[2];
}

void nmb_Transform_TScShR::transform(double *p_src, double *p_dest)
{
  if (d_matrixNeedsUpdate) {
    updateMatrix();
  }
  p_dest[0] = d_matrix[0]*p_src[0] + d_matrix[4]*p_src[1] +
              d_matrix[8]*p_src[2] + d_matrix[12]*p_src[3];
  p_dest[1] = d_matrix[1]*p_src[0] + d_matrix[5]*p_src[1] +
              d_matrix[9]*p_src[2] + d_matrix[13]*p_src[3];
  p_dest[2] = d_matrix[2]*p_src[0] + d_matrix[6]*p_src[1] +
              d_matrix[10]*p_src[2] + d_matrix[14]*p_src[3];
  p_dest[3] = d_matrix[3]*p_src[0] + d_matrix[7]*p_src[1] +
              d_matrix[11]*p_src[2] + d_matrix[15]*p_src[3];
}

void nmb_Transform_TScShR::transform(double *pnt)
{
  double temp[4];
  if (d_matrixNeedsUpdate) {
    updateMatrix();
  }
  temp[0] = d_matrix[0]*pnt[0] + d_matrix[4]*pnt[1] +
              d_matrix[8]*pnt[2] + d_matrix[12]*pnt[3];
  temp[1] = d_matrix[1]*pnt[0] + d_matrix[5]*pnt[1] +
              d_matrix[9]*pnt[2] + d_matrix[13]*pnt[3];
  temp[2] = d_matrix[2]*pnt[0] + d_matrix[6]*pnt[1] +
              d_matrix[10]*pnt[2] + d_matrix[14]*pnt[3];
  temp[3] = d_matrix[3]*pnt[0] + d_matrix[7]*pnt[1] +
              d_matrix[11]*pnt[2] + d_matrix[15]*pnt[3];
  pnt[0] = temp[0];
  pnt[1] = temp[1];
  pnt[2] = temp[2];
  pnt[3] = temp[3];
}

void nmb_Transform_TScShR::getMatrix(double *m)
{
  if (d_matrixNeedsUpdate) {
    updateMatrix();
  }
  int i;
  for (i = 0; i < 16; i++) {
    m[i] = d_matrix[i];
  }
}

/* updateMatrix: computes a 4x4 transformation matrix from the parameters.

Here is some useful mathematica code for computing these formulas:

Rx = {{1, 0, 0, 0}, {0, Cos[psi], -Sin[psi], 0}, {0, Sin[psi], Cos[psi], 0},
     {0, 0, 0, 1}};

Ry = {{Cos[theta], 0, Sin[theta], 0}, {0, 1, 0, 0},
     {-Sin[theta], 0, Cos[theta], 0}, {0, 0, 0, 1}};

Rz = {{Cos[phi], -Sin[phi], 0, 0}, {Sin[phi], Cos[phi], 0, 0}, {0, 0, 1, 0},
     {0, 0, 0, 1}};

R = Rz.Ry.Rx;

R = {{Cos[phi]*Cos[theta], -(Cos[psi]*Sin[phi]) + Cos[phi]*Sin[psi]*
        Sin[theta], Sin[phi]*Sin[psi] + Cos[phi]*Cos[psi]*Sin[theta], 0},
     {Cos[theta]*Sin[phi], Cos[phi]*Cos[psi] + Sin[phi]*Sin[psi]*Sin[theta],
      -(Cos[phi]*Sin[psi]) + Cos[psi]*Sin[phi]*Sin[theta], 0},
     {-Sin[theta], Cos[theta]*Sin[psi], Cos[psi]*Cos[theta], 0}, {0, 0, 0, 1}};

T = {{1, 0, 0, tx}, {0, 1, 0, ty}, {0, 0, 1, tz}, {0, 0, 0, 1}};
Sh = {{1, shz, 0, 0}, {shx*shy, 1 + shx*shy*shz, shx, 0},
     {shy, shy*shz, 1, 0}, {0, 0, 0, 1}};

Sc = {{scx, 0, 0, 0}, {0, scy, 0, 0}, {0, 0, scz, 0}, {0, 0, 0, 1}};

Pivot = {PivotX, PivotY, PivotZ, 1};
Tpivot = {{1, 0, 0, Pivot[[1]]}, {0, 1, 0, Pivot[[2]]},
      {0, 0, 1, Pivot[[3]]}, {0, 0, 0, 1}};
TinvPivot = {{1, 0, 0, -Pivot[[1]]}, {0, 1, 0, -Pivot[[2]]},
      {0, 0, 1, -Pivot[[3]]}, {0, 0, 0, 1}};

M = T.Tpivot.Sc.Sh.R.TinvPivot;

dmdtx = D[M,tx];
dmdty = D[M,ty];
dmdtz = D[M,tz];
dmdscx = D[M,scx];
dmdscy = D[M,scy];
dmdscz = D[M,scz];
dmdshx = D[M,shx];
dmdshy = D[M,shy];
dmdshz = D[M,shz];
dmdpsi = D[M,psi];
dmdtheta = D[M,theta];
dmdphi = D[M,phi];

M = Simplify[M];
dmdtx = Simplify[dmdtx];
dmdty = Simplify[dmdty];
dmdtz = Simplify[dmdtz];
dmdscx = Simplify[dmdscx];
dmdscy = Simplify[dmdscy];
dmdscz = Simplify[dmdscz];
dmdshx = Simplify[dmdshx];
dmdshy = Simplify[dmdshy];
dmdshz = Simplify[dmdshz];
dmdpsi = Simplify[dmdpsi];
dmdtheta = Simplify[dmdtheta];
dmdphi = Simplify[dmdphi];

Save["transform.txt", {T, R, Sh, Sc, M, dmdtx, dmdty, dmdtz, dmdscx,
 dmdscy, dmdscz, dmdshx, dmdshy, dmdshz, dmdpsi, dmdtheta, dmdphi}];

eqns= {m00==M[[1,1]], m01==M[[1,2]], m02==M[[1,3]],
       m03==M[[1,4]],
       m10==M[[2,1]], m11==M[[2,2]], m12==M[[2,3]],
       m13==M[[2,4]],
       m20==M[[3,1]], m21==M[[3,2]], m22==M[[3,3]],
       m23==M[[3,4]]};

Solution = Solve[eqns, {psi, theta, phi, tx, ty, tz, scx, scy, scz, shx, shy, shz}];

// some partial transforms that might be useful for debugging
MSc = PostPivotT.Sc.PrePivotT.T;

MShSc = PostPivotT.Sh.Sc.PrePivotT.T;

Save["partial_transform.txt", {T, R, Sh, Sc, MSc, MShSc}];
*/

void nmb_Transform_TScShR::updateMatrix() 
{
  // convert variable names to those in the Mathematica output
  double Cos_psi = d_cosAngle[0];
  double Cos_theta = d_cosAngle[1];
  double Cos_phi = d_cosAngle[2];

  double Sin_psi = d_sinAngle[0];
  double Sin_theta = d_sinAngle[1];
  double Sin_phi = d_sinAngle[2];

  double scx = d_scale[0], scy = d_scale[1], scz = d_scale[2];
  
  double shx = d_shear[0], shy = d_shear[1], shz = d_shear[2];
  double tx = d_translation[0], ty = d_translation[1], tz = d_translation[2];

  double PivotX = d_pivotPoint[0];
  double PivotY = d_pivotPoint[1];
  double PivotZ = d_pivotPoint[2];

  // the function is written like this to facilitate copy and paste from
  // mathematica, 
  // when this is thoroughly tested, you might want to optimize it - if thats
  // possible

  double temp[4][4] = 
{{scx*Cos_theta*(Cos_phi + shz*Sin_phi),
      scx*(Cos_phi*(shz*Cos_psi + Sin_psi*Sin_theta) +
        Sin_phi*(-Cos_psi + shz*Sin_psi*Sin_theta)),
      scx*(Cos_phi*(-(shz*Sin_psi) + Cos_psi*Sin_theta) +
        Sin_phi*(Sin_psi + shz*Cos_psi*Sin_theta)),
      PivotX + tx - PivotX*scx*shz*Cos_theta*Sin_phi -
       PivotZ*scx*Sin_phi*Sin_psi - PivotY*scx*shz*Sin_phi*Sin_psi*
        Sin_theta + scx*Cos_psi*Sin_phi*(PivotY - PivotZ*shz*Sin_theta) -
       scx*Cos_phi*(PivotX*Cos_theta - PivotZ*shz*Sin_psi +
         PivotY*Sin_psi*Sin_theta + Cos_psi*(PivotY*shz +
           PivotZ*Sin_theta))}, {scy*(shx*shy*Cos_phi*Cos_theta +
        (1 + shx*shy*shz)*Cos_theta*Sin_phi - shx*Sin_theta),
      scy*(shx*Cos_theta*Sin_psi + shx*shy*(-(Cos_psi*Sin_phi) +
          Cos_phi*Sin_psi*Sin_theta) + (1 + shx*shy*shz)*
         (Cos_phi*Cos_psi + Sin_phi*Sin_psi*Sin_theta)),
      scy*(shx*Cos_psi*Cos_theta + shx*shy*(Sin_phi*Sin_psi +
          Cos_phi*Cos_psi*Sin_theta) + (1 + shx*shy*shz)*
         (-(Cos_phi*Sin_psi) + Cos_psi*Sin_phi*Sin_theta)),
      PivotY + ty - PivotX*scy*Cos_theta*Sin_phi - PivotX*scy*shx*shy*shz*
        Cos_theta*Sin_phi - PivotY*scy*shx*Cos_theta*Sin_psi -
       PivotZ*scy*shx*shy*Sin_phi*Sin_psi + PivotX*scy*shx*Sin_theta -
       PivotY*scy*Sin_phi*Sin_psi*Sin_theta - PivotY*scy*shx*shy*shz*
        Sin_phi*Sin_psi*Sin_theta - scy*Cos_phi*
        (PivotX*shx*shy*Cos_theta - Sin_psi*(PivotZ + PivotZ*shx*shy*shz -
           PivotY*shx*shy*Sin_theta) + Cos_psi*(PivotY +
           PivotY*shx*shy*shz + PivotZ*shx*shy*Sin_theta)) -
       scy*Cos_psi*(PivotZ*shx*Cos_theta + Sin_phi*(-(PivotY*shx*shy) +
           PivotZ*(1 + shx*shy*shz)*Sin_theta))},
     {scz*(shy*Cos_phi*Cos_theta + shy*shz*Cos_theta*Sin_phi -
        Sin_theta), scz*(-(shy*Cos_psi*Sin_phi) +
        Sin_psi*(Cos_theta + shy*shz*Sin_phi*Sin_theta) +
        shy*Cos_phi*(shz*Cos_psi + Sin_psi*Sin_theta)),
      scz*(shy*(-(shz*Cos_phi) + Sin_phi)*Sin_psi +
        Cos_psi*(Cos_theta + shy*(Cos_phi + shz*Sin_phi)*Sin_theta)),
      PivotZ + tz - PivotX*scz*shy*shz*Cos_theta*Sin_phi -
       PivotY*scz*Cos_theta*Sin_psi - PivotZ*scz*shy*Sin_phi*Sin_psi +
       PivotX*scz*Sin_theta - PivotY*scz*shy*shz*Sin_phi*Sin_psi*
        Sin_theta - scz*shy*Cos_phi*(PivotX*Cos_theta -
         PivotZ*shz*Sin_psi + PivotY*Sin_psi*Sin_theta +
         Cos_psi*(PivotY*shz + PivotZ*Sin_theta)) -
       scz*Cos_psi*(PivotZ*Cos_theta + shy*Sin_phi*
          (-PivotY + PivotZ*shz*Sin_theta))}, {0, 0, 0, 1}};

  int i,j;
  for (i = 0; i < 4; i++) {
    for (j = 0; j < 4; j++) {
      d_matrix[4*j + i] = temp[i][j];
    }
  }

  d_matrixNeedsUpdate = vrpn_FALSE;
}

void nmb_Transform_TScShR::getMatrixDerivative(double *m, 
                                        nmb_TransformParameter param)
{
  // convert variable names to those in the Mathematica output
  double Cos_psi = d_cosAngle[0];
  double Cos_theta = d_cosAngle[1];
  double Cos_phi = d_cosAngle[2];

  double Sin_psi = d_sinAngle[0];
  double Sin_theta = d_sinAngle[1];
  double Sin_phi = d_sinAngle[2];

  double scx = d_scale[0], scy = d_scale[1], scz = d_scale[2];

  double shx = d_shear[0], shy = d_shear[1], shz = d_shear[2];

  double PivotX = d_pivotPoint[0];
  double PivotY = d_pivotPoint[1];
  double PivotZ = d_pivotPoint[2];

  m[0] = 0; m[4] = 0;  m[8] = 0; m[12] = 0;
  m[1] = 0; m[5] = 0;  m[9] = 0; m[13] = 0;
  m[2] = 0; m[6] = 0; m[10] = 0; m[14] = 0;
  m[3] = 0; m[7] = 0; m[11] = 0; m[15] = 0;

  int i, j;
  switch (param) {
    case NMB_TRANSLATE_X:
      {
      double temp[4][4] = 
             {{0, 0, 0, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};
      for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
          m[4*j + i] = temp[i][j];
        }
      }
      }
      break;
    case NMB_TRANSLATE_Y:
      {
      double temp[4][4] =
             {{0, 0, 0, 0}, {0, 0, 0, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}};
      for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
          m[4*j + i] = temp[i][j];
        }
      }
      }
      break;
    case NMB_TRANSLATE_Z:
      {
      double temp[4][4] =
             {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 1}, {0, 0, 0, 0}};
      for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
          m[4*j + i] = temp[i][j];
        }
      }
      }
      break;
    case NMB_SCALE_X:
      {
      double temp[4][4] =
             {{Cos_theta*(Cos_phi + shz*Sin_phi),
      Cos_phi*(shz*Cos_psi + Sin_psi*Sin_theta) +
       Sin_phi*(-Cos_psi + shz*Sin_psi*Sin_theta),
      Cos_phi*(-(shz*Sin_psi) + Cos_psi*Sin_theta) +
       Sin_phi*(Sin_psi + shz*Cos_psi*Sin_theta),
      -(Cos_phi*(PivotX*Cos_theta + Sin_psi*(-(PivotZ*shz) +
            PivotY*Sin_theta) + Cos_psi*(PivotY*shz +
            PivotZ*Sin_theta))) - Sin_phi*(PivotX*shz*Cos_theta +
         Sin_psi*(PivotZ + PivotY*shz*Sin_theta) +
         Cos_psi*(-PivotY + PivotZ*shz*Sin_theta))}, {0, 0, 0, 0},
     {0, 0, 0, 0}, {0, 0, 0, 0}};
      for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
          m[4*j + i] = temp[i][j];
        }
      }
      }
      break;
    case NMB_SCALE_Y:
      {
      double temp[4][4] =
             {{0, 0, 0, 0}, {shx*shy*Cos_phi*Cos_theta +
       (1 + shx*shy*shz)*Cos_theta*Sin_phi - shx*Sin_theta,
      shx*Cos_theta*Sin_psi + shx*shy*(-(Cos_psi*Sin_phi) +
         Cos_phi*Sin_psi*Sin_theta) + (1 + shx*shy*shz)*
        (Cos_phi*Cos_psi + Sin_phi*Sin_psi*Sin_theta),
      shx*Cos_psi*Cos_theta + shx*shy*(Sin_phi*Sin_psi +
         Cos_phi*Cos_psi*Sin_theta) + (1 + shx*shy*shz)*
        (-(Cos_phi*Sin_psi) + Cos_psi*Sin_phi*Sin_theta),
      -(PivotX*Cos_theta*Sin_phi) - PivotX*shx*shy*shz*Cos_theta*
        Sin_phi - PivotY*shx*Cos_theta*Sin_psi - PivotZ*shx*shy*Sin_phi*
        Sin_psi + PivotX*shx*Sin_theta - PivotY*Sin_phi*Sin_psi*
        Sin_theta - PivotY*shx*shy*shz*Sin_phi*Sin_psi*Sin_theta -
       Cos_phi*(PivotX*shx*shy*Cos_theta - Sin_psi*
          (PivotZ + PivotZ*shx*shy*shz - PivotY*shx*shy*Sin_theta) +
         Cos_psi*(PivotY + PivotY*shx*shy*shz + PivotZ*shx*shy*
            Sin_theta)) - Cos_psi*(PivotZ*shx*Cos_theta +
         Sin_phi*(-(PivotY*shx*shy) + PivotZ*(1 + shx*shy*shz)*
            Sin_theta))}, {0, 0, 0, 0}, {0, 0, 0, 0}};
      for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
          m[4*j + i] = temp[i][j];
        }
      }
      }
      break;
    case NMB_SCALE_Z:
      {
      double temp[4][4] =
             {{0, 0, 0, 0}, {0, 0, 0, 0}, {shy*Cos_phi*Cos_theta +
       shy*shz*Cos_theta*Sin_phi - Sin_theta, -(shy*Cos_psi*Sin_phi) +
       Sin_psi*(Cos_theta + shy*shz*Sin_phi*Sin_theta) +
       shy*Cos_phi*(shz*Cos_psi + Sin_psi*Sin_theta),
      shy*(-(shz*Cos_phi) + Sin_phi)*Sin_psi +
       Cos_psi*(Cos_theta + shy*(Cos_phi + shz*Sin_phi)*Sin_theta),
      -(PivotX*shy*shz*Cos_theta*Sin_phi) - PivotY*Cos_theta*Sin_psi -
       PivotZ*shy*Sin_phi*Sin_psi + PivotX*Sin_theta -
       PivotY*shy*shz*Sin_phi*Sin_psi*Sin_theta -
       shy*Cos_phi*(PivotX*Cos_theta - PivotZ*shz*Sin_psi +
         PivotY*Sin_psi*Sin_theta + Cos_psi*(PivotY*shz +
           PivotZ*Sin_theta)) - Cos_psi*(PivotZ*Cos_theta +
         shy*Sin_phi*(-PivotY + PivotZ*shz*Sin_theta))}, {0, 0, 0, 0}};
      for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
          m[4*j + i] = temp[i][j];
        }
      }
      }
      break;
    case NMB_SHEAR_X:
      {
      double temp[4][4] =
             {{0, 0, 0, 0}, {scy*(shy*Cos_phi*Cos_theta +
        shy*shz*Cos_theta*Sin_phi - Sin_theta),
      scy*(-(shy*Cos_psi*Sin_phi) + Sin_psi*(Cos_theta +
          shy*shz*Sin_phi*Sin_theta) + shy*Cos_phi*(shz*Cos_psi +
          Sin_psi*Sin_theta)), scy*(shy*(-(shz*Cos_phi) + Sin_phi)*
         Sin_psi + Cos_psi*(Cos_theta + shy*(Cos_phi + shz*Sin_phi)*
           Sin_theta)), -(scy*(PivotX*shy*shz*Cos_theta*Sin_phi +
         PivotY*Cos_theta*Sin_psi + PivotZ*shy*Sin_phi*Sin_psi -
         PivotX*Sin_theta + PivotY*shy*shz*Sin_phi*Sin_psi*Sin_theta +
         shy*Cos_phi*(PivotX*Cos_theta - PivotZ*shz*Sin_psi +
           PivotY*Sin_psi*Sin_theta + Cos_psi*(PivotY*shz +
             PivotZ*Sin_theta)) + Cos_psi*(PivotZ*Cos_theta +
           shy*Sin_phi*(-PivotY + PivotZ*shz*Sin_theta))))}, {0, 0, 0, 0},
     {0, 0, 0, 0}};
      for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
          m[4*j + i] = temp[i][j];
        }
      }
      }
      break;
    case NMB_SHEAR_Y:
      {
      double temp[4][4] = 
             {{0, 0, 0, 0}, {scy*shx*Cos_theta*(Cos_phi + shz*Sin_phi),
      scy*shx*(Cos_phi*(shz*Cos_psi + Sin_psi*Sin_theta) +
        Sin_phi*(-Cos_psi + shz*Sin_psi*Sin_theta)),
      scy*shx*(Cos_phi*(-(shz*Sin_psi) + Cos_psi*Sin_theta) +
        Sin_phi*(Sin_psi + shz*Cos_psi*Sin_theta)),
      -(scy*shx*(Cos_phi*(PivotX*Cos_theta + Sin_psi*(-(PivotZ*shz) +
             PivotY*Sin_theta) + Cos_psi*(PivotY*shz +
             PivotZ*Sin_theta)) + Sin_phi*(PivotX*shz*Cos_theta +
           Sin_psi*(PivotZ + PivotY*shz*Sin_theta) +
           Cos_psi*(-PivotY + PivotZ*shz*Sin_theta))))},
     {scz*Cos_theta*(Cos_phi + shz*Sin_phi),
      scz*(Cos_phi*(shz*Cos_psi + Sin_psi*Sin_theta) +
        Sin_phi*(-Cos_psi + shz*Sin_psi*Sin_theta)),
      scz*(Cos_phi*(-(shz*Sin_psi) + Cos_psi*Sin_theta) +
        Sin_phi*(Sin_psi + shz*Cos_psi*Sin_theta)),
      -(scz*(Cos_phi*(PivotX*Cos_theta + Sin_psi*(-(PivotZ*shz) +
             PivotY*Sin_theta) + Cos_psi*(PivotY*shz +
             PivotZ*Sin_theta)) + Sin_phi*(PivotX*shz*Cos_theta +
           Sin_psi*(PivotZ + PivotY*shz*Sin_theta) +
           Cos_psi*(-PivotY + PivotZ*shz*Sin_theta))))}, {0, 0, 0, 0}};
      for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
          m[4*j + i] = temp[i][j];
        }
      }
      }
      break;
    case NMB_SHEAR_Z:
      {
      double temp[4][4] =
           {{scx*Cos_theta*Sin_phi, scx*(Cos_phi*Cos_psi +
        Sin_phi*Sin_psi*Sin_theta), scx*(-(Cos_phi*Sin_psi) +
        Cos_psi*Sin_phi*Sin_theta),
      -(scx*(Cos_phi*(PivotY*Cos_psi - PivotZ*Sin_psi) +
         Sin_phi*(PivotX*Cos_theta + (PivotZ*Cos_psi + PivotY*Sin_psi)*
            Sin_theta)))}, {scy*shx*shy*Cos_theta*Sin_phi,
      scy*shx*shy*(Cos_phi*Cos_psi + Sin_phi*Sin_psi*Sin_theta),
      scy*shx*shy*(-(Cos_phi*Sin_psi) + Cos_psi*Sin_phi*Sin_theta),
      -(scy*shx*shy*(Cos_phi*(PivotY*Cos_psi - PivotZ*Sin_psi) +
         Sin_phi*(PivotX*Cos_theta + (PivotZ*Cos_psi + PivotY*Sin_psi)*
            Sin_theta)))}, {scz*shy*Cos_theta*Sin_phi,
      scz*shy*(Cos_phi*Cos_psi + Sin_phi*Sin_psi*Sin_theta),
      scz*shy*(-(Cos_phi*Sin_psi) + Cos_psi*Sin_phi*Sin_theta),
      -(scz*shy*(Cos_phi*(PivotY*Cos_psi - PivotZ*Sin_psi) +
         Sin_phi*(PivotX*Cos_theta + (PivotZ*Cos_psi + PivotY*Sin_psi)*
            Sin_theta)))}, {0, 0, 0, 0}};
      for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
          m[4*j + i] = temp[i][j];
        }
      }
      }
      break;
    case NMB_ROTATE_X:
      {
      double temp[4][4] =
       {{0, scx*(Cos_phi*(-(shz*Sin_psi) + Cos_psi*Sin_theta) +
        Sin_phi*(Sin_psi + shz*Cos_psi*Sin_theta)),
      -(scx*(Cos_phi*(shz*Cos_psi + Sin_psi*Sin_theta) +
         Sin_phi*(-Cos_psi + shz*Sin_psi*Sin_theta))),
      scx*(Cos_phi*(Cos_psi*(PivotZ*shz - PivotY*Sin_theta) +
          Sin_psi*(PivotY*shz + PivotZ*Sin_theta)) -
        Sin_phi*(Cos_psi*(PivotZ + PivotY*shz*Sin_theta) +
          Sin_psi*(PivotY - PivotZ*shz*Sin_theta)))},
     {0, scy*(shx*Cos_psi*Cos_theta + shx*shy*(Sin_phi*Sin_psi +
          Cos_phi*Cos_psi*Sin_theta) + (1 + shx*shy*shz)*
         (-(Cos_phi*Sin_psi) + Cos_psi*Sin_phi*Sin_theta)),
      scy*(-(shx*Cos_theta*Sin_psi) + shx*shy*(Cos_psi*Sin_phi -
          Cos_phi*Sin_psi*Sin_theta) - (1 + shx*shy*shz)*
         (Cos_phi*Cos_psi + Sin_phi*Sin_psi*Sin_theta)),
      scy*(Cos_phi*(Cos_psi*(PivotZ + PivotZ*shx*shy*shz -
            PivotY*shx*shy*Sin_theta) + Sin_psi*(PivotY +
            PivotY*shx*shy*shz + PivotZ*shx*shy*Sin_theta)) -
        Cos_psi*(PivotY*shx*Cos_theta + Sin_phi*(PivotZ*shx*shy +
            PivotY*(1 + shx*shy*shz)*Sin_theta)) +
        Sin_psi*(PivotZ*shx*Cos_theta + Sin_phi*(-(PivotY*shx*shy) +
            PivotZ*(1 + shx*shy*shz)*Sin_theta)))},
     {0, scz*(shy*(-(shz*Cos_phi) + Sin_phi)*Sin_psi +
        Cos_psi*(Cos_theta + shy*(Cos_phi + shz*Sin_phi)*Sin_theta)),
      -(scz*(-(shy*Cos_psi*Sin_phi) + Sin_psi*(Cos_theta +
           shy*shz*Sin_phi*Sin_theta) + shy*Cos_phi*(shz*Cos_psi +
           Sin_psi*Sin_theta))),
      scz*(shy*Cos_phi*(Cos_psi*(PivotZ*shz - PivotY*Sin_theta) +
          Sin_psi*(PivotY*shz + PivotZ*Sin_theta)) -
        Cos_psi*(PivotY*Cos_theta + shy*Sin_phi*(PivotZ +
            PivotY*shz*Sin_theta)) + Sin_psi*(PivotZ*Cos_theta +
          shy*Sin_phi*(-PivotY + PivotZ*shz*Sin_theta)))}, {0, 0, 0, 0}};
      for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
          m[4*j + i] = temp[i][j];
        }
      }
      }
      break;
    case NMB_ROTATE_Y:
      {
      double temp[4][4] = 
      {{-(scx*(Cos_phi + shz*Sin_phi)*Sin_theta),
      scx*Cos_theta*(Cos_phi + shz*Sin_phi)*Sin_psi,
      scx*Cos_psi*Cos_theta*(Cos_phi + shz*Sin_phi),
      -(scx*(Cos_phi + shz*Sin_phi)*(PivotZ*Cos_psi*Cos_theta +
         PivotY*Cos_theta*Sin_psi - PivotX*Sin_theta))},
     {-(scy*(shx*Cos_theta + (shx*shy*Cos_phi + Sin_phi +
           shx*shy*shz*Sin_phi)*Sin_theta)), scy*Sin_psi*
       (shx*shy*Cos_phi*Cos_theta + (1 + shx*shy*shz)*Cos_theta*Sin_phi -
        shx*Sin_theta), scy*Cos_psi*(shx*shy*Cos_phi*Cos_theta +
        (1 + shx*shy*shz)*Cos_theta*Sin_phi - shx*Sin_theta),
      scy*(Cos_theta*(PivotX*shx - PivotZ*(1 + shx*shy*shz)*Cos_psi*
           Sin_phi - PivotY*Sin_phi*Sin_psi - PivotY*shx*shy*shz*Sin_phi*
           Sin_psi - shx*shy*Cos_phi*(PivotZ*Cos_psi + PivotY*Sin_psi)) +
        (PivotX*shx*shy*Cos_phi + PivotZ*shx*Cos_psi + PivotX*Sin_phi +
          PivotX*shx*shy*shz*Sin_phi + PivotY*shx*Sin_psi)*Sin_theta)},
     {-(scz*(Cos_theta + shy*(Cos_phi + shz*Sin_phi)*Sin_theta)),
      scz*Sin_psi*(shy*Cos_phi*Cos_theta + shy*shz*Cos_theta*Sin_phi -
        Sin_theta), scz*Cos_psi*(shy*Cos_phi*Cos_theta +
        shy*shz*Cos_theta*Sin_phi - Sin_theta),
      scz*(Cos_theta*(PivotX - PivotZ*shy*shz*Cos_psi*Sin_phi -
          PivotY*shy*shz*Sin_phi*Sin_psi - shy*Cos_phi*(PivotZ*Cos_psi +
            PivotY*Sin_psi)) + (PivotX*shy*Cos_phi + PivotZ*Cos_psi +
          PivotX*shy*shz*Sin_phi + PivotY*Sin_psi)*Sin_theta)},
     {0, 0, 0, 0}};
      for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
          m[4*j + i] = temp[i][j];
        }
      }
      }
      break;
    case NMB_ROTATE_Z:
      {
      double temp[4][4] =
       {{scx*Cos_theta*(shz*Cos_phi - Sin_phi),
      -(scx*(Sin_phi*(shz*Cos_psi + Sin_psi*Sin_theta) +
         Cos_phi*(Cos_psi - shz*Sin_psi*Sin_theta))),
      scx*(Sin_phi*(shz*Sin_psi - Cos_psi*Sin_theta) +
        Cos_phi*(Sin_psi + shz*Cos_psi*Sin_theta)),
      scx*(Sin_phi*(PivotX*Cos_theta + Sin_psi*(-(PivotZ*shz) +
            PivotY*Sin_theta) + Cos_psi*(PivotY*shz + PivotZ*Sin_theta)) -
        Cos_phi*(PivotX*shz*Cos_theta + Sin_psi*(PivotZ +
            PivotY*shz*Sin_theta) + Cos_psi*(-PivotY +
            PivotZ*shz*Sin_theta)))},
     {scy*Cos_theta*(Cos_phi + shx*shy*shz*Cos_phi - shx*shy*Sin_phi),
      scy*(1 + shx*shy*shz)*(-(Cos_psi*Sin_phi) + Cos_phi*Sin_psi*
          Sin_theta) - scy*shx*shy*(Cos_phi*Cos_psi +
         Sin_phi*Sin_psi*Sin_theta),
      scy*(1 + shx*shy*shz)*(Sin_phi*Sin_psi + Cos_phi*Cos_psi*
          Sin_theta) + scy*shx*shy*(Cos_phi*Sin_psi -
         Cos_psi*Sin_phi*Sin_theta),
      scy*(Sin_phi*(PivotX*shx*shy*Cos_theta - Sin_psi*
           (PivotZ + PivotZ*shx*shy*shz - PivotY*shx*shy*Sin_theta) +
          Cos_psi*(PivotY + PivotY*shx*shy*shz + PivotZ*shx*shy*
             Sin_theta)) - Cos_phi*(PivotX*(1 + shx*shy*shz)*Cos_theta +
          Sin_psi*(PivotZ*shx*shy + PivotY*(1 + shx*shy*shz)*Sin_theta) +
          Cos_psi*(-(PivotY*shx*shy) + PivotZ*(1 + shx*shy*shz)*
             Sin_theta)))}, {scz*shy*Cos_theta*(shz*Cos_phi - Sin_phi),
      -(scz*shy*(Sin_phi*(shz*Cos_psi + Sin_psi*Sin_theta) +
         Cos_phi*(Cos_psi - shz*Sin_psi*Sin_theta))),
      scz*shy*(Sin_phi*(shz*Sin_psi - Cos_psi*Sin_theta) +
        Cos_phi*(Sin_psi + shz*Cos_psi*Sin_theta)),
      scz*shy*(Sin_phi*(PivotX*Cos_theta + Sin_psi*(-(PivotZ*shz) +
            PivotY*Sin_theta) + Cos_psi*(PivotY*shz + PivotZ*Sin_theta)) -
        Cos_phi*(PivotX*shz*Cos_theta + Sin_psi*(PivotZ +
            PivotY*shz*Sin_theta) + Cos_psi*(-PivotY +
            PivotZ*shz*Sin_theta)))}, {0, 0, 0, 0}};
      for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
          m[4*j + i] = temp[i][j];
        }
      }
      }
      break;
    default:
      break;
  }
}
