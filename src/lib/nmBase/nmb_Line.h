#ifndef NMB_LINE_H
#define NMB_LINE_H

//#include <vrpn_Types.h>  // for vrpn_bool

#if 0
#include <Tcl_Netvar.h>  // for TclNet_float
#endif

#include "nmb_Types.h"  // for PointType, vrpn_Types

class BCPlane;  // from BCPlane.h


// Encapsulate Measure Lines and anything else that needs a similar
// implementation.

class nmb_Line;

typedef void (* nmb_LINE_MOVE_CALLBACK) (nmb_Line *, void * userdata);

class nmb_Line {

  public:

    // CONSTRUCTORS

#if 0
    nmb_Line (const char * name);
#else
    nmb_Line (void);
#endif
    ~nmb_Line (void);

    // ACCESSORS

    const PointType & top (void) const;
    const PointType & bottom (void) const;
    float x (void) const;
    float y (void) const;

    vrpn_bool changed (void) const;

    // MANIPULATORS

    void moveTo (float x, float y, BCPlane * plane);
      // Move to (x, y) and normalize.
    void normalize (BCPlane * plane);
      // Make sure that Z extents are from the top to the bottom of plane.

    void clearChanged (void);

    void registerMoveCallback (nmb_LINE_MOVE_CALLBACK f, void * userdata);

    void doCallbacks (void);

#if 0
    TclNet_int d_x;
    TclNet_int d_y;
#endif

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

