#include "nmb_Subgrid.h"

#include <BCGrid.h>
#include <BCPlane.h>

#include <thread.h>  // for class Semaphore

#include "nmb_Debug.h"

#define min(a,b) ((a) < (b) ? (a) : (b))
#define max(a,b) ((a) < (b) ? (b) : (a))



// nmb_Subgrid
//
// Tom Hudson, September 1997.
// Data structure from microscape.h;  code from animate.c
// (now MicroscopeRcv.C), active_set.C, microscape.c
// (now microscopeHandlers.C), openGL.c, tcl_tk.c

// Tracks the portion of a grid that has changed (since the
// last rendering iteration, so that those display lists can
// be regenerated).

Semaphore * range_ps = NULL;

nmb_Subgrid::nmb_Subgrid (BCGrid * & _grid) :
  //ps (NULL),
  min_x (0), min_y (0),
  max_x (-1), max_y (-1),
  grid (_grid) {

}



/** Determine whether anything in the subgrid has changed
 (since the last Clear())
*/
vrpn_bool nmb_Subgrid::Changed (void) const {
  return ((min_x <= max_x) && (min_y <= max_y));
}

float nmb_Subgrid::RatioOfChange (void) const {
  float x_change, y_change;

  if (!Changed()) return 1.0f;

  x_change = 1 + max_x - min_x;
  y_change = 1 + max_y - min_y;

  return y_change / x_change;
}



/// Mark a single point of the subgrid as changed
void nmb_Subgrid::AddPoint (const int x, const int y) {

  int retval;
  if (range_ps) {
    retval = range_ps->p();
    if (retval == 1) {
      VERBOSE(20, "Writer got semaphore.\n");
    } else
      fprintf(stderr, "Writer had error in getting semaphore.\n");
  }

  min_x = min(min_x, x);
  min_y = min(min_y, y);
  max_x = max(max_x, x);
  max_y = max(max_y, y);

  if (range_ps) {
    retval = range_ps->v();
    if (!retval) {
      VERBOSE(20, "Writer released semaphore.\n");
    } else
      fprintf(stderr, "Writer had error in releasing semaphore.\n");
  }
}



void nmb_Subgrid::Clear (void) {

  if (!grid) return;

  int retval;
  if (range_ps) {
    retval = range_ps->p();
    if (retval == 1) {
      VERBOSE(20, "Clearer got semaphore.\n");
    } else
      fprintf(stderr, "Clearer had error in getting semaphore.\n");
  }

  min_x = grid->numX();
  max_x = 0;
  min_y = grid->numY();
  max_y = 0;

  if (range_ps) {
    retval = range_ps->v();
    if (!retval) {
      VERBOSE(20, "Clearer released semaphore.\n");
    } else
      fprintf(stderr, "Clearer had error in releasing semaphore.\n");
  }

}

void nmb_Subgrid::GetBoundsAndClear (int * minX, int * maxX,
                                     int * minY, int * maxY) {

  if (!grid) return;

  int retval;
  if (range_ps) {
    retval = range_ps->p();
    if (retval == 1) {
      VERBOSE(20, "Clearer got semaphore.\n");
    } else
      fprintf(stderr, "Clearer had error in getting semaphore.\n");
  }

  if (minX) *minX = min_x;
  if (maxX) *maxX = max_x;
  if (minY) *minY = min_y;
  if (maxY) *maxY = max_y;

  min_x = grid->numX();
  max_x = 0;
  min_y = grid->numY();
  max_y = 0;

  if (range_ps) {
    retval = range_ps->v();
    if (!retval) {
      VERBOSE(20, "Clearer released semaphore.\n");
    } else
      fprintf(stderr, "Clearer had error in releasing semaphore.\n");
  }

}


void nmb_Subgrid::ChangeAll (void) {

  if (!grid) return;

  int retval;
  if (range_ps) {
    retval = range_ps->p();
    if (retval == 1) {
      VERBOSE(20, "Writer got semaphore.\n");
    } else
      fprintf(stderr, "Writer had error in getting semaphore.\n");
  }

  min_x = 0;
  max_x = grid->numX() - 1;
  min_y = 0;
  max_y = grid->numY() - 1;

  if (range_ps) {
    retval = range_ps->v();
    if (!retval) {
      VERBOSE(20, "Writer released semaphore.\n");
    } else
      fprintf(stderr, "Writer had error in releasing semaphore.\n");
  }
}



