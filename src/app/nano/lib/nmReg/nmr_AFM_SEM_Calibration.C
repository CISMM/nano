#include "nmr_AFM_SEM_Calibration.h"
#include "transformSolve.h"
#include "linLeastSqr.h"
#include "nmb_Transform_TScShR.h"

#ifndef M_PI
static double M_PI=3.14159265358979323846;
#endif

int nmr_AFM_SEM_Calibration::s_freeTip_SEMIndex = 0;
int nmr_AFM_SEM_Calibration::s_freeTip_AFMIndex = 1;
int nmr_AFM_SEM_Calibration::s_surfaceFeature_modelIndex = 0;
int nmr_AFM_SEM_Calibration::s_surfaceFeature_SEMIndex = 1;
int nmr_AFM_SEM_Calibration::s_contactTip_SEMIndex = 0;
int nmr_AFM_SEM_Calibration::s_contactTip_AFMIndex = 1;
int nmr_AFM_SEM_Calibration::s_contactTip_modelIndex = 2;

int nmr_AFM_SEM_Calibration::s_maxCorrespondencePoints = 32;

nmr_AFM_SEM_Calibration::nmr_AFM_SEM_Calibration(nmr_SurfaceModel *model):
  d_projDirInAFM_Rx(0.0), d_projDirInAFM_Rz(0.0),
  d_freeTipPoints(2, s_maxCorrespondencePoints),
  d_freeTipPointsChangedSinceSolution(0),
  d_surfaceFeaturePoints(2, s_maxCorrespondencePoints),
  d_surfaceFeaturePointsChangedSinceSolution(0),
  d_contactTipPoints(3, s_maxCorrespondencePoints), 
  d_contactTipPointsChangedSinceSolution(0),
  d_projDirInModel_Rx(0.0), d_projDirInModel_Rz(0.0),
  d_model(model)
{
  d_projDirInAFM[0] = 0.0; d_projDirInAFM[1] = 0.0; 
  d_projDirInAFM[2] = 1.0; d_projDirInAFM[3] = 1.0;
  d_projDirInModel[0] = 0.0; d_projDirInModel[1] = 0.0; 
  d_projDirInModel[2] = 1.0; d_projDirInModel[3] = 1.0;
  int i;
  for (i = 0; i < 16; i++) {
    d_SEMfromAFMMatrix[i] = 0.0;
    d_SEMfromModel2DMatrix[i] = 0.0;
    d_SEMfromModel3DMatrix[i] = 0.0;
    d_AFMfromModel2DMatrix[i] = 0.0;
    d_AFMfromModel3DMatrix[i] = 0.0;
  }
}

// at least 4 non-coplanar pairs of corresponding points from the
// AFM and SEM coordinate systems
int nmr_AFM_SEM_Calibration::setFreeTipCorrespondence(
                             nmr_CalibrationPoint *AFM,
                             nmr_CalibrationPoint *SEM, int numPoints)
{
  if (numPoints < 4 || numPoints > s_maxCorrespondencePoints) return -1;
  corr_point_t point[2];
  d_freeTipPoints.clear();
  int i;
  for (i = 0; i < numPoints; i++) {
    point[s_freeTip_SEMIndex] = corr_point_t(SEM[i][0], SEM[i][1], SEM[i][2]);
    point[s_freeTip_AFMIndex] = corr_point_t(AFM[i][0], AFM[i][1], AFM[i][2]);
    d_freeTipPoints.addPoint(point);
  }
  d_freeTipPointsChangedSinceSolution = 1;
  return 0;

/* volume is calculated as
              ( AFM[0][0] AFM[0][1] AFM[0][2] 1 )
     (1/6)*det( AFM[1][0] AFM[1][1] AFM[1][2] 1 )
              ( AFM[2][0] AFM[2][1] AFM[2][2] 1 )
              ( AFM[3][0] AFM[3][1] AFM[3][2] 1 )

  double volume = (1.0/6.0)*(
    AFM[0][0]*(AFM[1][1]*(AFM[2][2]-AFM[3][2]) - 
               AFM[1][2]*(AFM[2][1]-AFM[3][1]) +
               AFM[2][1]*AFM[3][2]-AFM[2][2]*AFM[3][1]) -
    AFM[0][1]*(AFM[1][0]*(AFM[2][2]-AFM[3][2]) -
               AFM[1][2]*(AFM[2][0]-AFM[3][0]) +
               AFM[2][0]*AFM[3][2]-AFM[2][2]*AFM[3][0]) +
    AFM[0][2]*(AFM[1][0]*(AFM[2][1]-AFM[3][1]) -
               AFM[1][1]*(AFM[2][0]-AFM[3][0]) +
               AFM[2][0]*AFM[3][1]-AFM[2][1]*AFM[3][0]) -
    1*(AFM[1][0]*(AFM[2][1]*AFM[3][2]-AFM[2][2]*AFM[3][1]) -
       AFM[1][1]*(AFM[2][0]*AFM[3][2]-AFM[2][2]*AFM[3][0]) +
       AFM[1][2]*(AFM[2][0]*AFM[3][1]-AFM[2][1]*AFM[3][0])));
  double ex = AFM[1][0] - AFM[0][0];
  double ey = AFM[1][1] - AFM[0][1];
  double ez = AFM[1][2] - AFM[0][2];
  double edgeLength = sqrt(ex*ex + ey*ey + ez*ez);
  if (fabs(volume) < 0.0001*edgeLength*edgeLength*edgeLength) {
    printf("nmr_AFM_SEM_Calibration::setFreeTipCorrespondence: Warning: "
           "tetrahedral volume is near 0\n");
    return -1;
  }
*/
}

// 3 non-colinear pairs of corresponding points from the
// Model and SEM coordinate systems
int nmr_AFM_SEM_Calibration::setSurfaceFeatureCorrespondence(
                             nmr_CalibrationPoint *model, 
                             nmr_CalibrationPoint *SEM, int numPoints)
{
  if (numPoints < 3 || numPoints > s_maxCorrespondencePoints) return -1;
  corr_point_t point[2];
  d_surfaceFeaturePoints.clear();
  int i;
  for (i = 0; i < numPoints; i++) {
    point[s_surfaceFeature_SEMIndex] = 
                         corr_point_t(SEM[i][0], SEM[i][1], SEM[i][2]);
    point[s_surfaceFeature_modelIndex] = 
                         corr_point_t(model[i][0], model[i][1], model[i][2]);
    d_surfaceFeaturePoints.addPoint(point);
  }
  d_surfaceFeaturePointsChangedSinceSolution = 1;
  return 0;
}

// 3 pairs of corresponding points in the AFM and SEM coordinate systems
// that also lie on the surface (tip was in contact with sample but
// cantilever was not deflected when these positions were measured)
int nmr_AFM_SEM_Calibration::setContactTipCorrespondence(
                             nmr_CalibrationPoint *AFM,
                             nmr_CalibrationPoint *SEM, int numPoints)
{
  if (numPoints < 3 || numPoints > s_maxCorrespondencePoints) return -1;
  corr_point_t point[3];
  d_contactTipPoints.clear();
  int i;
  for (i = 0; i < numPoints; i++) {
    point[s_contactTip_SEMIndex] = corr_point_t(SEM[i][0],SEM[i][1],SEM[i][2]);
    point[s_contactTip_AFMIndex] = corr_point_t(AFM[i][0],AFM[i][1],AFM[i][2]);
    point[s_contactTip_modelIndex] = point[s_contactTip_AFMIndex];
    d_contactTipPoints.addPoint(point);
  }
  d_contactTipPointsChangedSinceSolution = 1;
  return 0;
}

// step 1
// update d_SEMfromAFMMatrix using freeTip AFM<-->SEM points
void nmr_AFM_SEM_Calibration::updateSEMfromAFM_3D()
{
  double error;
  transformSolver(d_SEMfromAFMMatrix, &error, d_freeTipPoints,
                  s_freeTip_AFMIndex, s_freeTip_SEMIndex, NMR_3D3D_AFFINE);
}

// step 2
// update d_SEMfromModel2DMatrix using surfaceFeature model<-->SEM points
// and an assumed SEM projection direction perpendicular to the x-y plane
void nmr_AFM_SEM_Calibration::updateSEMfromModel_2D()
{
  double error;
  transformSolver(d_SEMfromModel2DMatrix, &error, d_surfaceFeaturePoints,
    s_surfaceFeature_modelIndex, s_surfaceFeature_SEMIndex, NMR_2D2D_AFFINE);
}

// step 3
// update contactTip model points using model description and
// contactTip SEM<-->AFM correspondence by
// back-projecting the points in the SEM image onto
// the model using d_SEMfromModel2DMatrix
void nmr_AFM_SEM_Calibration::updateModelPointsAssumingOverheadProjection()
{
  corr_point_t temp;
  nmr_CalibrationPoint semPoint = {0,0,0,1};
  nmr_CalibrationPoint modelPoint = {0,0,0,1};
  nmb_TransformMatrix44 semFromModel;
  semFromModel.setMatrix(d_SEMfromModel2DMatrix);
  int i;
  for (i = 0; i < d_contactTipPoints.numPoints(); i++) {
    d_contactTipPoints.getPoint(s_contactTip_SEMIndex, i, &temp);
    semPoint[0] = temp.x; 
    semPoint[1] = temp.y; 
    semPoint[2] = temp.z;
    semFromModel.invTransform(semPoint, modelPoint);
    modelPoint[2] = getSurfaceHeight(modelPoint[0], modelPoint[1]);
    temp.x = modelPoint[0];
    temp.y = modelPoint[1];
    temp.z = modelPoint[2];
    d_contactTipPoints.setPoint(s_contactTip_modelIndex, i, temp);
  }
}

// step 4
// update d_AFMfromModel2DMatrix using contactTip model<-->AFM points
void nmr_AFM_SEM_Calibration::updateAFMfromModel_2D()
{
  double error;
  transformSolver(d_AFMfromModel2DMatrix, &error, d_contactTipPoints,
       s_contactTip_modelIndex, s_contactTip_AFMIndex, 
       NMR_2D2D_AFFINE_Z_UNIFORMSCALING_Z_TRANSLATE);
}

// step 5
// compute direction of SEM projection in the AFM coordinate system
void nmr_AFM_SEM_Calibration::updateProjDirInAFM()
{
  computeProjectionDirection(d_SEMfromAFMMatrix,
               d_projDirInAFM[0], d_projDirInAFM[1], d_projDirInAFM[2]);
  convertProjectionDirectionToRxRz(d_projDirInAFM[0], d_projDirInAFM[1],
          d_projDirInAFM[2], d_projDirInAFM_Rx, d_projDirInAFM_Rz);
}

// step 6
// update d_projDirInModel using d_SEMfromAFMMatrix and
// d_AFMfromModel2DMatrix
void nmr_AFM_SEM_Calibration::updateProjDirInModel()
{
  nmb_TransformMatrix44 AFMfromModel;
  AFMfromModel.setMatrix(d_AFMfromModel2DMatrix);
  AFMfromModel.invTransformVector(d_projDirInAFM, d_projDirInModel);
  double invLength = 1.0/sqrt(d_projDirInModel[0]*d_projDirInModel[0] +
                              d_projDirInModel[1]*d_projDirInModel[1] +
                              d_projDirInModel[2]*d_projDirInModel[2]);
  d_projDirInModel[0] *= invLength;
  d_projDirInModel[1] *= invLength;
  d_projDirInModel[2] *= invLength;
  convertProjectionDirectionToRxRz(d_projDirInModel[0], d_projDirInModel[1], 
          d_projDirInModel[2], d_projDirInModel_Rx, d_projDirInModel_Rz);
}

// step 7
// update d_SEMfromModel3DMatrix using d_projDirInModel and
// surfaceFeature model<-->SEM points
void nmr_AFM_SEM_Calibration::updateSEMfromModel_3D()
{
  // now that we have a better estimate of the SEM projection direction for
  // the model we can use that to better estimate the transformation that
  // takes model points to corresponding SEM points
  Correspondence rotatedSurfaceFeaturePoints = d_surfaceFeaturePoints;
  nmr_CalibrationPoint modelPoint = {0,0,0,1};
  corr_point_t temp;
  
  nmb_Transform_TScShR preRotation;
  preRotation.setRotation(NMB_THETA, d_projDirInModel_Rx);
  preRotation.setRotation(NMB_PHI, d_projDirInModel_Rz);
  int i;
  for (i = 0; i < d_surfaceFeaturePoints.numPoints(); i++) {
    d_surfaceFeaturePoints.getPoint(s_surfaceFeature_modelIndex, i, &temp);
    modelPoint[0] = temp.x;
    modelPoint[1] = temp.y;
    modelPoint[2] = temp.z;
    preRotation.transform(modelPoint);
    temp.x = modelPoint[0];
    temp.y = modelPoint[1];
    temp.z = modelPoint[2];
    rotatedSurfaceFeaturePoints.setPoint(s_surfaceFeature_modelIndex, i, temp);
  }
  double SEMfromRotatedModel2DMatrix[16];
  double error;
  /* note: we might want to add some constraints here by assuming for
     practical purposes that the 2D part has uniform scaling in x,y and no shear
  */
  transformSolver(SEMfromRotatedModel2DMatrix, &error, 
            rotatedSurfaceFeaturePoints, s_surfaceFeature_modelIndex,
            s_surfaceFeature_SEMIndex, 
            NMR_2D2D_AFFINE_Z_UNIFORMSCALING_Z_TRANSLATE);
  // compose the 2D affine part with the pre-rotation part
  double tempMatrix[16];
  nmb_TransformMatrix44 preRotation44;
  preRotation.getMatrix(tempMatrix);
  preRotation44.setMatrix(tempMatrix);
  nmb_TransformMatrix44 SEMfromModel44;
  SEMfromModel44.setMatrix(SEMfromRotatedModel2DMatrix);
  SEMfromModel44.compose(preRotation44);
  SEMfromModel44.getMatrix(d_SEMfromModel3DMatrix);

}

/*
I'm not really sure if this will tell us anything especially useful given the
assumption of a nearly flat sample
void nmr_AFM_SEM_Calibration::updateModelPointsFromRotatedProjection()
{
  // now that we have an SEMfromModel transformation that includes an
  // estimated rotation component, we can better estimate the
  // contact points on the model corresponding to points in the SEM image
  corr_point_t temp;
  nmr_CalibrationPoint semPoint = {0,0,0,1};
  nmr_CalibrationPoint rayStart = {0,0,0,1};
  nmr_CalibrationPoint modelPoint = {0,0,0,1};
  nmb_TransformMatrix44 semFromModel;
  semFromModel.setMatrix(d_SEMfromModel3DMatrix);
  int i;
  for (i = 0; i < d_contactTipPoints.numPoints(); i++) {
    d_contactTipPoints.getPoint(s_contactTip_SEMIndex, i, &temp);
    semPoint[0] = temp.x;
    semPoint[1] = temp.y;
    semPoint[2] = temp.z;
    semFromModel.invTransform(semPoint, rayStart);
    // find nearest intersection of ray (rayStart, d_projDirInModel) with the
    // surface - or something like that, and put the resulting intersection
    // point in modelPoint
    findSurfaceIntersection(rayStart, d_projDirInModel, modelPoint);
    temp.x = modelPoint[0];
    temp.y = modelPoint[1];
    temp.z = modelPoint[2];
    d_contactTipPoints.setPoint(s_contactTip_modelIndex, i, temp);
  }
}
*/

// step 8
// update d_AFMfromModel3DMatrix using d_SEMfromModel3DMatrix and
// d_SEMfromAFMMatrix
// We are basically solving for d_AFMfromModel3DMatrix in the following
// equation. Note that even though d_SEMfromAFMMatrix and 
// d_SEMfromModel3DMatrix do not really need to transform z (since the
// SEM image is independent of z they include the z transformation
// information because of how they were constructed.
// d_SEMfromAFMMatrix*d_AFMfromModel3DMatrix = d_SEMfromModel3DMatrix
void nmr_AFM_SEM_Calibration::updateAFMfromModel_3D()
{
  double A[9];
  double B[3];

  int i;
  for (i = 0; i < 4; i++) {
    A[0] = d_SEMfromAFMMatrix[0];
    A[1] = d_SEMfromAFMMatrix[1];
    A[2] = d_SEMfromAFMMatrix[2];
    A[3] = d_SEMfromAFMMatrix[4];
    A[4] = d_SEMfromAFMMatrix[5];
    A[5] = d_SEMfromAFMMatrix[6];
    A[6] = d_SEMfromAFMMatrix[8];
    A[7] = d_SEMfromAFMMatrix[9];
    A[8] = d_SEMfromAFMMatrix[10];
    B[0] = d_SEMfromModel3DMatrix[i*4+0];
    B[1] = d_SEMfromModel3DMatrix[i*4+1];
    B[2] = d_SEMfromModel3DMatrix[i*4+2];
    linearLeastSquaresSolve(3, 3, A, B); // use as linear solver
    d_AFMfromModel3DMatrix[i*4+0] = B[0];
    d_AFMfromModel3DMatrix[i*4+1] = B[1];
    d_AFMfromModel3DMatrix[i*4+2] = B[2];
  }
  d_AFMfromModel3DMatrix[3] = 0;
  d_AFMfromModel3DMatrix[7] = 0;
  d_AFMfromModel3DMatrix[11] = 0;
  d_AFMfromModel3DMatrix[15] = 1;
}

void nmr_AFM_SEM_Calibration::updateSolution()
{
  updateSEMfromAFM_3D();
  updateSEMfromModel_2D();
  updateModelPointsAssumingOverheadProjection(); 
  updateAFMfromModel_2D();
  updateProjDirInAFM();
  updateProjDirInModel();
  updateSEMfromModel_3D();
  updateAFMfromModel_3D();

  d_freeTipPointsChangedSinceSolution = 0;
  d_surfaceFeaturePointsChangedSinceSolution = 0;
  d_contactTipPointsChangedSinceSolution = 0;
}

void nmr_AFM_SEM_Calibration::getSEMProjectionDirectionInAFM(
                        double &azimuth_deg,double &altitude_deg)
{
  azimuth_deg = d_projDirInAFM_Rz*180.0/M_PI;
  altitude_deg = d_projDirInAFM_Rx*180.0/M_PI;
}

void nmr_AFM_SEM_Calibration::getSEMProjectionDirectionInAFM(
                        double &vx, double &vy, double &vz)
{
  vx = d_projDirInAFM[0];
  vy = d_projDirInAFM[1];
  vz = d_projDirInAFM[2];
}

void nmr_AFM_SEM_Calibration::getSEMProjectionDirectionInModel(
                          double &azim_deg,double &alt_deg)
{
  azim_deg = d_projDirInModel_Rz*180.0/M_PI;
  alt_deg = d_projDirInModel_Rx*180.0/M_PI;
}

void nmr_AFM_SEM_Calibration::getSEMProjectionDirectionInModel(
                        double &vx, double &vy, double &vz)
{
  vx = d_projDirInModel[0];
  vy = d_projDirInModel[1];
  vz = d_projDirInModel[2];
}

int nmr_AFM_SEM_Calibration::numContactPoints()
{
  return d_contactTipPoints.numPoints();
}

void nmr_AFM_SEM_Calibration::getModelContactPoints(
                                      nmr_CalibrationPoint *model)
{
  corr_point_t modelPnt;
  int i;
  for (i = 0; i < d_contactTipPoints.numPoints(); i++) {
    d_contactTipPoints.getPoint(s_contactTip_modelIndex, i, &modelPnt);
    model[i][0] = modelPnt.x;
    model[i][1] = modelPnt.y;
    model[i][2] = modelPnt.z;
    model[i][3] = 1.0;
  }
}

void nmr_AFM_SEM_Calibration::getSEMfromAFMMatrix(double *matrix)
{
  int i;
  for (i = 0; i < 16; i++) {
    matrix[i] = d_SEMfromAFMMatrix[i];
  }
}

void nmr_AFM_SEM_Calibration::getSEMfromModel2DMatrix(double *matrix)
{
  int i;
  for (i = 0; i < 16; i++) {
    matrix[i] = d_SEMfromModel2DMatrix[i];
  }
}

void nmr_AFM_SEM_Calibration::getSEMfromModel3DMatrix(double *matrix)
{
  int i;
  for (i = 0; i < 16; i++) {
    matrix[i] = d_SEMfromModel3DMatrix[i];
  }
}

void nmr_AFM_SEM_Calibration::getAFMfromModel2DMatrix(double *matrix)
{
  int i;
  for (i = 0; i < 16; i++) {
    matrix[i] = d_AFMfromModel2DMatrix[i];
  }
}

void nmr_AFM_SEM_Calibration::getAFMfromModel3DMatrix(double *matrix)
{
  int i;
  for (i = 0; i < 16; i++) {
    matrix[i] = d_AFMfromModel3DMatrix[i];
  }
}

// This function returns the rotation transformation consisting of the 
// rotation about the z axis by Rz followed by a rotation about the
// x axis by Rx that would be required to make the unit vector in the
// +z direction match the direction of the vector (vx, vy, vz)
void nmr_AFM_SEM_Calibration::convertProjectionDirectionToRxRz(
                                   double vx, double vy, double vz,
                                   double &Rx, double &Rz)
{
  double length = sqrt(vx*vx + vy*vy + vz*vz);
  double cos_Rx = vz/length;
  Rx = acos(cos_Rx);
  double sin_Rx = sin(Rx);
  if (sin_Rx == 0) {
    Rz = 0.0;
  } else {
    double sin_Rz = vx/(length*sin_Rx);
    double cos_Rz = vy/(length*sin_Rx);
    if (sin_Rz > 1.0) sin_Rz = 1.0;
    if (sin_Rz < -1.0) sin_Rz = -1.0;
    if (cos_Rz > 1.0) cos_Rz = 1.0;
    if (cos_Rz < -1.0) cos_Rz = -1.0;
    Rz = asin(sin_Rz);
    if (cos_Rz < 0) {
      Rz = M_PI - Rz;
    }
  }
}

// This function returns the vector that would result from applying the 
// rotation about the z-axis by Rz followed by the rotation about the x-axis
// Rx to the unit vector in the +z direction.
void nmr_AFM_SEM_Calibration::convertRxRzToProjectionDirection(
                                   double Rx, double Rz,
                                   double &vx, double &vy, double &vz)
{
  vx = sin(Rz)*sin(Rx);
  vy = cos(Rz)*sin(Rx);
  vz = cos(Rx);
}

// given an array of 16 doubles that represents the 4x4 column-major
// order transformation matrix for a transformation that takes points in
// coordinate system A to the corresponding points in coordinate system B
// returns the vector with length 1 in A that when transformed into
// B has no component in the x or y directions.
// By convention, the returned vector will have vz >= 0
void nmr_AFM_SEM_Calibration::computeProjectionDirection(double *matrix,
                                   double &vx, double &vy, double &vz)
{
 // what change in (x,y,z) in A coordinates will preserve (x,y) in
 // B coordinates?
 // matrix[0]*x + matrix[4]*y + matrix[8]*z = 0
 // matrix[1]*x + matrix[5]*y + matrix[9]*z = 0
 // z = 1.0
 double A[9] = {matrix[0], matrix[1], 0.0,
                matrix[4], matrix[5], 0.0,
                matrix[8], matrix[9], 1.0};
 double B[3] = {0.0,       0.0,       1.0};
 linearLeastSquaresSolve(3, 3, A, B); // using this as a plain linear solver

 double inv_length = 1.0/sqrt(B[0]*B[0] + B[1]*B[1] + B[2]*B[2]);
 vx = B[0]*inv_length;
 vy = B[1]*inv_length;
 vz = B[2]*inv_length;
 return;
}

double nmr_AFM_SEM_Calibration::getSurfaceHeight(double x, double y)
{
  double t;
  d_model->surfaceRayIntersection(x, y, 0, 0, 0, -1, t);
  double height = -t;
  return height;
}
