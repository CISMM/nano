#ifndef NMB_LINE_H
#define NMB_LINE_H

#include <quat.h>  // for q_vec_type

#include "nmb_Types.h"  // for PointType, vrpn_Types

class BCPlane;  // from BCPlane.h

typedef void (* nmb_LINE_MOVE_CALLBACK) (float x, float y, void * userdata);

/// Encapsulate Measure Lines and anything else that needs a similar
/// implementation.
class nmb_Line {

  public:

    // CONSTRUCTORS

    nmb_Line (void);
    ~nmb_Line (void);

    // ACCESSORS

    const PointType & top (void) const;
    const PointType & bottom (void) const;
    float x (void) const;
    float y (void) const;

    vrpn_bool changed (void) const;

    // MANIPULATORS
    double getIntercept (BCPlane *);
    ///< Computes intercept of line with plane. Side-effect: moves
    ///< line inside bounds of plane if necessary.

    void getIntercept (q_vec_type p, BCPlane *) ;
    ///< Computes intercept of line with plane. Side-effect: moves
    ///< line inside bounds of plane if necessary.

    void moveTo (float x, float y, BCPlane * plane);
      ///< Move to (x, y) and normalize.
    void normalize (BCPlane * plane);
      ///< Make sure that Z extents are from the top to the bottom of plane.
    static void normalize (PointType & top, PointType & bottom, 
                           BCPlane * plane);
      ///< Utility to normalize PointTypes consistently throughout nano 

    void clearChanged (void);

    void registerMoveCallback (nmb_LINE_MOVE_CALLBACK f, void * userdata);

    void doCallbacks (float x, float y, BCPlane * plane);

  private:

    PointType d_top;
    PointType d_bottom;

    vrpn_bool d_changed;

    struct moveCallbackEntry {
      nmb_LINE_MOVE_CALLBACK f;
      void * userdata;
      moveCallbackEntry * next;
    };
    moveCallbackEntry * d_moveCallbacks;

};

#endif  // NMB_LINE_H

