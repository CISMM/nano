#ifndef NMB_SUBGRID_H
#define NMB_SUBGRID_H

#include "nmb_Types.h"  // for NM_BOOLEAN

class BCGrid;
class BCPlane;

class Semaphore;  // from multip/thread.h

// Synchronization.
// This object may be written by multiple processes when
// doing shared-memory multiprocessing.
// TCL blows up if the Semaphore is part of the object?

extern Semaphore * range_ps;

// nmb_Subgrid
//
// Tom Hudson, September 1997.
// Data structure from microscape.h;  code from animate.c
// (now MicroscopeRcv.C), active_set.C, microscape.c
// (now microscopeHandlers.C), openGL.c, tcl_tk.c

// Tracks the portion of a grid that has changed (since the
// last rendering iteration, so that those display lists can
// be regenerated).

class nmb_Subgrid {

  public:

    nmb_Subgrid (BCGrid * &);

    vrpn_bool Changed (void) const;
      // Returns V_TRUE if any point in the subgrid has changed
      // since the last call to Clear()
    float RatioOfChange (void) const;
      // Returns the ratio of the size of the subgrid in Y to
      // the size of the sugrid in X

    void AddPoint (const int, const int);
      // Marks a single point as changed.

    void ChangeAll (void);
    //void ChangeAll (const BCGrid *);
    //void ChangeAll (const BCPlane *);
      // Marks the entire subgrid as changed.

    void Clear (void);
    //void Clear (const BCGrid *);
    //void Clear (const BCPlane *);
      // Clears the subgrid.
    void GetBoundsAndClear (int * minX, int * maxX, int * minY, int * maxY);
      // Looks up min/max values and clears the subgrid atomically.
      // Bounds are returned in the non-NULL arguments.


    int MinX (void) const { return min_x; }
    int MaxX (void) const { return max_x; }
    int MinY (void) const { return min_y; }
    int MaxY (void) const { return max_y; }

    //Semaphore * ps;

  private:

    int   min_x, min_y;   /* Lower-left corner of grid */
    int   max_x, max_y;   /* Upper-right corner of grid */

    BCGrid * & grid;

};


#endif  // NMB_SUBGRID_H

