#include <Tcl_Linkvar.h>
#include <Tcl_Netvar.h>
#include "nmm_GuardedscanClient.h"
#include "nmb_Dataset.h"
#include "nmb_Decoration.h"
#include "error_display.h"
#include "BCPlane.h"
#include "math.h"
#include "nmm_MicroscopeRemote.h"
#include "Point.h"
#include "tcl_tk.h"

extern nmm_Microscope_Remote* microscope;
extern nmb_Dataset* dataset;
extern nmb_Decoration* decoration;

#define q_vec_print(msg, vec) {printf("   %s=(%0.3f, %0.3f, %0.3f)\n", msg, vec[0], vec[1], vec[2]);}

CGuardedScanClient::CGuardedScanClient() :
  m_pPlane(NULL),
  m_pDecoration(NULL),
  m_fGuardDepth(0.0f)
{
  m_vPlaneNormal[0] = m_vPlaneNormal[1] = m_vPlaneNormal[2] = 0.0f;
}

CGuardedScanClient::~CGuardedScanClient()
{

}

/// Acquire a plane from the measure lines...
void CGuardedScanClient::AcquirePlane()
{
  // Grab the current height plane and decoration objects...
  BCPlane* pPlane = dataset->inputGrid->getPlaneByName(dataset->heightPlaneName->string());
  if(pPlane == NULL) {
    // No height plane defined...
    display_error_dialog("Guard plane could not be acquired because there is currently no height plane.");
    return;
  }

  m_pPlane = pPlane;
  m_pDecoration = decoration;
 
  // Grab coords of where measure lines intercept height plane...
  q_vec_type vRed, vGreen, vBlue;
  m_pDecoration->red.getIntercept(vRed, m_pPlane);
  m_pDecoration->green.getIntercept(vGreen, m_pPlane);
  m_pDecoration->blue.getIntercept(vBlue, m_pPlane);

  // Calculate plane normal...
  CalculatePlaneNormal(m_vPlaneNormal, vRed, vGreen, vBlue);
  q_vec_print("Plane normal", m_vPlaneNormal);

  // Calculate D parameter for plane (distance to origin)
  m_fPlaneDistance = CalculatePlaneDistanceToOrigin(m_vPlaneNormal, vRed);

  printf(" D=%0.3f\n", m_fPlaneDistance);
}

/// Calculate the normal of a plane defined by three points
void CGuardedScanClient::CalculatePlaneNormal(q_vec_type& a_ovReturnedNormal, q_vec_type a_vPt1, q_vec_type a_vPt2, q_vec_type a_vPt3)
{
  q_vec_type vRet, vV, vU;
  
  // vU = a_vPt2 - a_vPt1;
  vU[0] = a_vPt2[0] - a_vPt1[0];
  vU[1] = a_vPt2[1] - a_vPt1[1];
  vU[2] = a_vPt2[2] - a_vPt1[2];

  // vV = a_vPt3 - a_vPt2;
  vV[0] = a_vPt3[0] - a_vPt2[0];
  vV[1] = a_vPt3[1] - a_vPt2[1];
  vV[2] = a_vPt3[2] - a_vPt2[2];

  // vRet = vU X vV;
  vRet[0] = (vU[1] * vV[2]) + (vU[2] * vV[1]);
  vRet[1] = (vU[2] * vV[0]) + (vU[0] * vV[2]);
  vRet[2] = (vU[0] * vV[1]) - (vU[1] * vV[0]);

  // Now normalize vRet so that |vRet| == 1.0
  float fLen = CalculateLength(vRet);
  if((fLen > 0.0f) || (fLen < 0.0f)) {
    vRet[0] /= fLen;
    vRet[1] /= fLen;
    vRet[2] /= fLen;
  }

  a_ovReturnedNormal[0]  = vRet[0];
  a_ovReturnedNormal[1]  = vRet[1];
  a_ovReturnedNormal[2]  = vRet[2];
}

/// Calculate the length of a vector
double CGuardedScanClient::CalculateLength(q_vec_type a_vVector)
{
  return sqrt((a_vVector[0] * a_vVector[0]) + (a_vVector[1] * a_vVector[1]) + (a_vVector[2] * a_vVector[2]));
}

/// Calculate the distance of a point from a plane along a vector
double CGuardedScanClient::CalculatePlaneDistanceToOrigin(q_vec_type a_vNormal, q_vec_type a_vPointOnPlane)
{
  double fRet;

  fRet = (a_vNormal[0] * a_vPointOnPlane[0]) + (a_vNormal[1] * a_vPointOnPlane[1]) + (a_vNormal[2] * a_vPointOnPlane[2]);

  return fRet;
}

/// Calculates the distance between a point and the most recently acquired plane
double CGuardedScanClient::CalculateDistanceToPlane(q_vec_type a_vPoint)
{
  double fRet;
  
  // Ax + By + Cz = D
  // z = (D - Ax - By) / C
  fRet = (m_fPlaneDistance - (m_vPlaneNormal[0] * a_vPoint[0])
	  - (m_vPlaneNormal[1] * a_vPoint[1])) / m_vPlaneNormal[2];

  return fRet;
}

/// Calculates the hieght of the guard plane in world coordinates (real world)
double CGuardedScanClient::CalculatePlaneHeight(double a_fX, double a_fY)
{
  // Find distance of the point to the plane along the plane normal
  // Ax + By + Cz = D
  // z = (D - Ax - By) / C
  // (NOTE: this is the same calc. as CalculateDistanceToPlane)
  q_vec_type vPoint;
  vPoint[0] = a_fX;
  vPoint[1] = a_fY;
  vPoint[2] = 0.0f; // unused
  double fDis = CalculateDistanceToPlane(vPoint);

  // Now decrease that value by the guard depth... the value is that much lower
  fDis -= m_fGuardDepth;

  return fDis;
}
