#ifndef NMR_AFM_SEM_CALIBRATION_H
#define NMR_AFM_SEM_CALIBRATION_H
/*
 This class implements a calibration algorithm for aligning three different
 coordinate systems. The coordinate systems are:

 AFM: defined by the tip position for the AFM (depends on
      any piezo non-linearity compensations and resulting piezo response to
      applied voltage when a position command is given) - this is usually
      a function of time but we will assume that the time dependence is
      insignificant or that the distortions resulting from time dependence
      can be described by a simple two-dimensional shearing of the image.
 SEM: defined by the projection and resulting image for the SEM
 Model: coordinate system used to describe a model of the surface being
      imaged by both the AFM and SEM
*/

#include "nmr_SurfaceModel.h"
#include "correspondence.h"

// homogenous coordinates (4th element is typically 1.0)
typedef vrpn_float32 nmr_CalibrationPoint[4];

class nmr_AFM_SEM_Calibration 
{
 public:
  nmr_AFM_SEM_Calibration(nmr_SurfaceModel *model);

  // >=4 non-coplanar pairs of corresponding points from the 
  // AFM and SEM coordinate systems
  int setFreeTipCorrespondence(nmr_CalibrationPoint *AFM,
                               nmr_CalibrationPoint *SEM,
                               int numPoints);

  // >=3 non-colinear pairs of corresponding points from the
  // Model and SEM coordinate systems
  int setSurfaceFeatureCorrespondence(nmr_CalibrationPoint *model, 
                                      nmr_CalibrationPoint *SEM,
                                      int numPoints);

  // >=3 pairs of corresponding points in the AFM and SEM coordinate systems
  // that also lie on the surface (tip was in contact with sample but
  // cantilever was not deflected when these positions were measured)
  int setContactTipCorrespondence(nmr_CalibrationPoint *AFM,
                                  nmr_CalibrationPoint *SEM,
                                  int numPoints);

  void updateSolution();

  // here are the various things that can be found from the solution:
  void getSEMProjectionDirectionInAFM(double &azimuth_deg,double &altitude_deg);
  void getSEMProjectionDirectionInAFM(double &vx, double &vy, double &vz);

  void getSEMProjectionDirectionInModel(double &azim_deg,double &alt_deg);
  void getSEMProjectionDirectionInModel(double &vx, double &vy, double &vz);

  int numContactPoints();
  void getModelContactPoints(nmr_CalibrationPoint *model);

  void getSEMfromAFMMatrix(double *matrix);
  void getSEMfromModel2DMatrix(double *matrix);
  void getSEMfromModel3DMatrix(double *matrix);
  void getAFMfromModel2DMatrix(double *matrix);
  void getAFMfromModel3DMatrix(double *matrix);

 private:
  // solve for various parameters in 7 steps

  // step 1
  // update d_SEMfromAFMMatrix using freeTip AFM<-->SEM points
  void updateSEMfromAFM_3D();

  // step 2
  // update d_SEMfromModel2DMatrix using surfaceFeature model<-->SEM points
  // and an assumed SEM projection direction perpendicular to the x-y plane
  void updateSEMfromModel_2D();

  // step 3
  // update contactTip model points using model description and 
  // contactTip SEM<-->AFM correspondence by 
  // back-projecting the points in the SEM image onto 
  // the model using d_SEMfromModel2DMatrix
  void updateModelPointsAssumingOverheadProjection();

  // step 4
  // update d_AFMfromModel2DMatrix using contactTip model<-->AFM points
  void updateAFMfromModel_2D();

  // step 5
  // compute direction of SEM projection in the AFM coordinate system using
  // d_SEMfromAFMMatrix
  void updateProjDirInAFM();

  // step 6
  // update d_projDirInModel using d_SEMfromAFMMatrix and 
  // d_AFMfromModel2DMatrix
  void updateProjDirInModel();

  // step 7
  // update d_SEMfromModel3DMatrix using d_projDirInModel and
  // surfaceFeature model<-->SEM points
  void updateSEMfromModel_3D();

  // step 8
  void updateModelPointsFromRotatedProjection();

  // step 9
  // update d_AFMfromModel3DMatrix
  void updateAFMfromModel_3D();

  // helper functions
  void convertProjectionDirectionToRxRz(double vx, double vy, double vz,
                                     double &Rx, double &Rz);
  void convertRxRzToProjectionDirection(double Rx, double Rz,
                                     double &vx, double &vy, double &vz);
  void computeProjectionDirection(double *matrix,
                                  double &vx, double &vy, double &vz);
  double getSurfaceHeight(double x, double y);
  void findSurfaceIntersection(double *rayStart, double *dir, double *point);

  double d_projDirInAFM[4];
  double d_projDirInAFM_Rx, d_projDirInAFM_Rz;

  static int s_maxCorrespondencePoints;

  // Correspondence to constrain the full 3D transformation that takes
  // points in the AFM coordinate system to the corresponding points
  // in the SEM image. Since the surface is typically quite flat it may
  // be important to acquire at least one of these points off the 
  // surface to allow a well-conditioned result for the transformation.
  // This correspondence consists of points acquired using the tip as
  // fiducial
  static int s_freeTip_SEMIndex;
  static int s_freeTip_AFMIndex;
  Correspondence d_freeTipPoints;
  int d_freeTipPointsChangedSinceSolution;

  // Correspondence to constrain the 2D component of the transformation
  // (assuming a particular 3D rotation of the model) that takes points in
  // the model to points in the SEM image. 
  // This correspondence consists of points found by using corresponding
  // features on the surface given by the surface model and as viewed in
  // the SEM image.
  static int s_surfaceFeature_modelIndex;
  static int s_surfaceFeature_SEMIndex;
  Correspondence d_surfaceFeaturePoints;
  int d_surfaceFeaturePointsChangedSinceSolution;

  // Correspondence to constrain the coordinate system relationships not
  // accounted for because of the orthographic nature of the SEM projection.
  // As viewed in the SEM image, the surface has an unknown shearing in z as a 
  // function of x or y and has an unknown overall z offset.
  // This correspondence consists of points found by touching the tip to the
  // surface and then using the tip as a fiducial. Note that the tip may be 
  // anywhere on the surface and not necessarily near matchable features.
  // SEM and AFM points are passed in by the user. Corresponding model points
  // are estimated by various calculations done internally.
  static int s_contactTip_SEMIndex;
  static int s_contactTip_AFMIndex;
  static int s_contactTip_modelIndex;
  Correspondence d_contactTipPoints;
  int d_contactTipPointsChangedSinceSolution;

  // transformation matrices in column major order
  double d_projDirInModel[4];
  double d_projDirInModel_Rx, d_projDirInModel_Rz;
  double d_SEMfromAFMMatrix[16];
  double d_SEMfromModel2DMatrix[16];
  double d_SEMfromModel3DMatrix[16];
  double d_AFMfromModel2DMatrix[16];
  double d_AFMfromModel3DMatrix[16];

  nmr_SurfaceModel *d_model;

};

#endif
