#include <Tcl_Linkvar.h>
#include <Tcl_Netvar.h>
#include "guardedscan.h"
#include "splat.h"
#include "nmb_Dataset.h"
#include "nmb_Decoration.h"
#include "error_display.h"
#include "BCPlane.h"
#include "math.h"
#include "microscape.h"
#include "nmm_MicroscopeRemote.h"
#include "point.h"

extern nmm_Microscope_Remote* microscope;
extern nmb_Dataset* dataset;
extern nmb_Decoration* decoration;

const int SAMPLES_PER_MAINLOOP = 30; // specifies how many samples to take during each call to mainloop (performance improvement)

const int APPROACH_STEPS = 25; // specifies how many m_fMaxZSteps should be taken toward the guard plane during an approach.  See CGuardedScan::Approach()

#define q_vec_print(msg, vec) {printf("   %s=(%0.3f, %0.3f, %0.3f)\n", msg, vec[0], vec[1], vec[2]);}

CGuardedScan::CGuardedScan() :
  m_bModeActive(0),
  guarded_scan_start("guarded_scan_start", 0),
  guarded_scan_stop("guarded_scan_stop", 0),
  guarded_plane_acquire("guarded_plane_acquire", 0),
  guarded_plane_depth("guarded_plane_depth", 0.0f),
  guarded_scan_max_step("guarded_scan_max_step", 0.01f),
  m_pPlane(NULL),
  m_pDecoration(NULL)
{
  // Register TCL callbacks...
  guarded_scan_start.addCallback(HandleScanStart, this);
  guarded_scan_stop.addCallback(HandleScanStop, this);
  guarded_plane_acquire.addCallback(HandlePlaneAcquire, this);
  guarded_plane_depth.addCallback(HandleDepth, this);
  guarded_scan_max_step.addCallback(HandleMaxZStep, this);
}

CGuardedScan::~CGuardedScan()
{

}

/// Mode active indicator
int CGuardedScan::IsModeActive()
{
  return m_bModeActive;
}

/// Main guarded scan loop (nano side implementation)
void CGuardedScan::mainloop()
{
  printf("Guarded scan mainloop.\n");
  
  for(int i = 0;i < SAMPLES_PER_MAINLOOP;++i) {
    // Calculate sample position...
    double fX = ((double)m_nLastXSample * m_fXResolution) + m_pPlane->minX();
    double fY = ((double)m_nLastYSample * m_fYResolution) + m_pPlane->minY();
    double fZ = m_fLastZValue;

    //    int TakeDirectZStep (const float x, const float y, const float z,
    //                 Point_value * point = NULL,
    //                 const vrpn_bool awaitResult = VRPN_FALSE);
    printf("Scanning to (%0.3f, %0.3f, %0.3f)\n", fX, fY, fZ);
   
    // Move to next point in region...
    m_nLastXSample++;
    if(m_nLastXSample > m_pPlane->numX()) {
      m_nLastXSample = 0;
      m_nLastYSample++;
    }

    // Check for scan completion
    if(m_nLastYSample > m_pPlane->numY()) {
      m_nLastYSample = 0;
      m_bModeActive = FALSE;
      guarded_scan_stop = 1;
    }
  }

}

/// Handle TCL button for starting the guarded scan mode
void CGuardedScan::HandleScanStart(vrpn_int32 a_nVal, void* a_pObject)
{
  CGuardedScan* pMe = (CGuardedScan*)a_pObject;
  if(pMe == NULL) {
    // No object pointer... bail...
    printf("CGuardedScan::HandleScanStart bailed: no object pointer.\n");
    return;
  }
  
  // Set mode flag true so that microscape::main calls CGuardedScan's mainloop
  pMe->m_bModeActive = TRUE;

  // Approach surface at corner of scan...
  pMe->Approach();
}

/// Handle TCL button for stopping the guarded scan mode
void CGuardedScan::HandleScanStop(vrpn_int32 a_nVal, void* a_pObject)
{
  CGuardedScan* pMe = (CGuardedScan*)a_pObject;
  if(pMe == NULL) {
    // No object pointer... bail...
    printf("CGuardedScan::HandleScanStop bailed: no object pointer.\n");
    return;
  }

  // Set mode flag false so that microscape::main stops calling our mainloop
  pMe->m_bModeActive = FALSE;

  // Withdraw from surface...
  microscope->WithdrawTip();
}

/// Handle TCL button for acquiring the guard plane
void CGuardedScan::HandlePlaneAcquire(vrpn_int32 a_nVal, void* a_pObject)
{
  CGuardedScan* pMe = (CGuardedScan*)a_pObject;
  if(pMe == NULL) {
    // No object pointer... bail...
    printf("CGuardedScan::HandlePlaneAcquire bailed: no object pointer.\n");
    return;
  }

  // Grab the current height plane and decoration objects...
  BCPlane* pPlane = (dataset->inputGrid->
		     getPlaneByName(dataset->heightPlaneName->string()));
  pMe->m_pPlane = pPlane;
  pMe->m_pDecoration = decoration;

  if(pMe->m_pDecoration == NULL) {
    // No decoration pointer... bail...
    printf("CGuardedScan::HandlePlaneAcquire bailed: no decoration pointer.\n");
    return;
  }

  if(pMe->m_pPlane == NULL) {
    // No plane pointer... bail...
    printf("CGuardedScan::HandlePlaneAcquire bailed: no plane pointer.\n");
    return;
  }

  // Grab coords of where measure lines intercept height plane...
  q_vec_type vRed, vGreen, vBlue;
  pMe->m_pDecoration->red.getIntercept(vRed, pMe->m_pPlane);
  pMe->m_pDecoration->green.getIntercept(vGreen, pMe->m_pPlane);
  pMe->m_pDecoration->blue.getIntercept(vBlue, pMe->m_pPlane);

  // Calculate plane normal...
  CalculatePlaneNormal(pMe->m_vPlaneNormal, vRed, vGreen, vBlue);
  
  // Adjust (flip) normal if it points into the plane... i.e. if |vNorm| < 0
  if(CalculateLength(pMe->m_vPlaneNormal) < 0.0f) {
    pMe->m_vPlaneNormal[0] *= -1.0f;
    pMe->m_vPlaneNormal[1] *= -1.0f;
    pMe->m_vPlaneNormal[2] *= -1.0f;
  }

  q_vec_print("Plane normal", pMe->m_vPlaneNormal);

  // Calculate D parameter for plane (distance to origin)
  pMe->m_fPlaneDistance = CalculatePlaneDistanceToOrigin(pMe->m_vPlaneNormal, vRed);

  printf(" D=%0.3f\n", pMe->m_fPlaneDistance);

  // Grab plane information used during a scan...
  pMe->m_fXResolution = (pPlane->maxX() - pPlane->minX()) / (double)pPlane->numX();
  pMe->m_fYResolution = (pPlane->maxY() - pPlane->minY()) / (double)pPlane->numY();
  pMe->m_nLastXSample = 0;
  pMe->m_nLastYSample = 0;

  // Calculate the first sample's Z value from the guard plane...
  q_vec_type vCorner; // upper left corner of scan
  vCorner[0] = pPlane->minX();
  vCorner[1] = pPlane->minY();
  vCorner[2] = 0.0f;
  pMe->m_fLastZValue = -(pMe->CalculateDistanceToPlane(vCorner));
}

void CGuardedScan::HandleDepth(vrpn_float64 a_fDepth, void* a_pObject)
{
  CGuardedScan* pMe = (CGuardedScan*)a_pObject;
  if(pMe == NULL) {
    // No object pointer... bail...
    printf("CGuardedScan::HandleDepth bailed: no object pointer.\n");
    return;
  }

  printf("Guard depth = %f\n", a_fDepth);
  pMe->m_fGuardDepth = a_fDepth;
}

void CGuardedScan::HandleMaxZStep(vrpn_float64 a_fStep, void* a_pObject)
{
  CGuardedScan* pMe = (CGuardedScan*)a_pObject;
  if(pMe == NULL) {
    // No object pointer... bail...
    printf("CGuardedScan::HandleMaxZStep bailed: no object pointer.\n");
    return;
  }

  printf("Max Z step = %f\n", a_fStep);
  pMe->m_fMaxZStep = a_fStep;
}

/// Calculate the normal of a plane defined by three points
void CGuardedScan::CalculatePlaneNormal(q_vec_type& a_ovReturnedNormal, q_vec_type a_vPt1, q_vec_type a_vPt2, q_vec_type a_vPt3)
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
double CGuardedScan::CalculateLength(q_vec_type a_vVector)
{
  return sqrt((a_vVector[0] * a_vVector[0]) + (a_vVector[1] * a_vVector[1]) + (a_vVector[2] * a_vVector[2]));
}

/// Calculate the distance of a point from a plane along a vector
double CGuardedScan::CalculatePlaneDistanceToOrigin(q_vec_type a_vNormal, q_vec_type a_vPointOnPlane)
{
  double fRet;

  fRet = (a_vNormal[0] * a_vPointOnPlane[0]) + (a_vNormal[1] * a_vPointOnPlane[1]) + (a_vNormal[2] * a_vPointOnPlane[2]);

  return fRet;
}

/// Calculates the distance between a point and the most recently acquired plane
double CGuardedScan::CalculateDistanceToPlane(q_vec_type a_vPoint)
{
  double fRet;

  // fRet = (D - Ax - By) / C
  fRet = (m_fPlaneDistance - (m_vPlaneNormal[0] * a_vPoint[0])
	  - (m_vPlaneNormal[1] * a_vPoint[1])) / m_vPlaneNormal[2];

  return fRet;
}

/// Approach the surface from APPROACH_STEPs away from the guard plane in m_fMaxZStep increments
void CGuardedScan::Approach()
{
  printf("Approaching surface...\n");

  // Calculate the Z value from the upper left of the scan
  // (Note: microscope->state.lastZ is at the bottom right of the scan...)
  q_vec_type vCorner;
  vCorner[0] = m_pPlane->minX();
  vCorner[1] = m_pPlane->minY();
  vCorner[2] = 0.0f;
  float fX = vCorner[0];
  float fY = vCorner[1];
  float fZ = -(CalculateDistanceToPlane(vCorner));
  float fPlaneZ = fZ;
  
  // Back away from the plane by APPROACH_STEPS * m_fMaxZStep
  fZ += (float)APPROACH_STEPS * m_fMaxZStep;

  // Approach by looping until the SPM signal exceeds the setpoint
  // OR until fZ will pass through the guard plane on the next step...
  float fSetpoint = microscope->state.image.setpoint;
  Point_value* pPoint = microscope->state.data.inputPoint->getValueByName("height");
  
  if(pPoint == NULL) {
    display_error_dialog("Point value not availabe.");
    return;
  }

  int nBailout = 100;
  while(((pPoint->value() < fSetpoint) &&
	((fZ - m_fMaxZStep) > fPlaneZ)) || (nBailout < 0)) {
    // Take the step...
    microscope->TakeDirectZStep(fX, fY, fZ, pPoint, VRPN_TRUE);    
    m_fLastZValue = fZ;

    // Decrement the Z height (move toward the surface)
    fZ -= m_fMaxZStep;

    // Bailout handling (just in case)
    nBailout--;
  }

  if(nBailout <= 0) {
    // Throw an error and stop the scan...
    display_error_dialog("Guarded scan could not approach surface within 100 Z steps.");
    m_bModeActive = FALSE;
    guarded_scan_stop = 1;
    return;
  }

}
