#ifndef GUARDEDSCAN_H
#define GUARDEDSCAN_H

#include <tcl.h>
#include <Tcl_Linkvar.h>
#include <Tcl_Netvar.h>
#include "splat.h"
#include "nmb_Decoration.h"
#include "BCPlane.h"

class CGuardedScan {
 public:
  CGuardedScan();
  ~CGuardedScan();

  // Methods
 public:
  void mainloop();
  int  IsModeActive();

  // TCL handlers...
  static void HandleScanStart(vrpn_int32 a_nVal, void* a_pObject);
  static void HandleScanStop(vrpn_int32 a_nVal, void* a_pObject);
  static void HandlePlaneAcquire(vrpn_int32 a_nVal, void* a_pObject);
  static void HandleDepth(vrpn_float64 a_fDepth, void* a_pObject);
  static void HandleMaxZStep(vrpn_float64 a_fStep, void* a_pObject);

  // Plane calculations...
  static void   CalculatePlaneNormal(q_vec_type& a_ovReturnedNormal, q_vec_type a_vPt1, q_vec_type a_vPt2, q_vec_type a_vPt3);
  static double CalculatePlaneDistanceToOrigin(q_vec_type a_vNormal, q_vec_type a_vPointOnPlane);
  double CalculateDistanceToPlane(q_vec_type a_vPoint);
  static double CalculateLength(q_vec_type a_vVector);

  // Scanning/approach routines...
  void Approach(); // Approaches the surface from APPROACH_STEPS away from the guard plane

  // Members
 protected:
  int m_bModeActive; // <>0 if guarded scan is active
  
  Tclvar_int   guarded_scan_start; // TCL callback variables
  Tclvar_int   guarded_scan_stop; 
  Tclvar_int   guarded_plane_acquire;
  Tclvar_float guarded_plane_depth;
  Tclvar_float guarded_scan_max_step;

  nmb_Decoration* m_pDecoration; // for plane calculation from measure lines
  BCPlane*        m_pPlane; // Data plane (height)

  q_vec_type   m_vPlaneNormal; // Normal of Most Recently Acquired plane
  double       m_fPlaneDistance; // Distance of the "MRA" plane to origin
  double       m_fGuardDepth; // Depth of guard under acquired plane

  short  m_nLastXSample; // Keeps track of X scanning
  short  m_nLastYSample; // Keeps track of Y scanning
  double m_fXResolution; // X resolution (X units / sample)
  double m_fYResolution; // Y resolution (Y units / sample)
  double m_fLastZValue;  // Last Z value

  double m_fMaxZStep; // Max. Z step during the approach and while scanning
};

#endif
