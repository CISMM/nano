#ifndef GUARDEDSCANCLIENT_H
#define GUARDEDSCANCLIENT_H

#include <tcl.h>
#include <Tcl_Linkvar.h>
#include <Tcl_Netvar.h>
#include "nmb_Decoration.h"
#include "BCPlane.h"
#include "Point.h"

#define TRUE 1
#define FALSE 0

class CGuardedScanClient {
 public:
  CGuardedScanClient();
  ~CGuardedScanClient();

  // Methods
  void SetDepth(float a_fDepth) { m_fGuardDepth = a_fDepth; }

  // Plane calculations...
  void AcquirePlane(); // Grabs a plane using the measure lines...
  void GetNormal(float& a_fX, float& a_fY, float& a_fZ) { a_fX = m_vPlaneNormal[0]; a_fY = m_vPlaneNormal[1]; a_fZ = m_vPlaneNormal[2]; }
  float GetPlaneDistance() { return m_fPlaneDistance; }

 private:
  static void   CalculatePlaneNormal(q_vec_type& a_ovReturnedNormal, q_vec_type a_vPt1, q_vec_type a_vPt2, q_vec_type a_vPt3);
  static double CalculatePlaneDistanceToOrigin(q_vec_type a_vNormal, q_vec_type a_vPointOnPlane);
  double        CalculateDistanceToPlane(q_vec_type a_vPoint);
  static double CalculateLength(q_vec_type a_vVector);
  double        CalculatePlaneHeight(double a_fX, double a_fY);

  // Members
 protected:
  nmb_Decoration* m_pDecoration; // for plane calculation from measure lines
  BCPlane*        m_pPlane; // Data plane (internal sensor)
  Point_value*    m_pPoint;

  q_vec_type   m_vPlaneNormal; // Normal of Most Recently Acquired plane
  double       m_fPlaneDistance; // Distance of the "MRA" plane to origin
  double       m_fGuardDepth; // Depth of guard under acquired plane
};

#endif
