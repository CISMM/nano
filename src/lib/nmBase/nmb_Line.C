/*===3rdtech===
  Copyright (c) 2000 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#include "nmb_Line.h"

#include "BCPlane.h"

nmb_Line::nmb_Line (void) :
  d_changed (VRPN_TRUE),
  d_moveCallbacks (NULL) {

}

nmb_Line::~nmb_Line (void) {

}

const PointType & nmb_Line::top (void) const {
  return d_top;
}

const PointType & nmb_Line::bottom (void) const {
  return d_bottom;
}

float nmb_Line::x (void) const {
  return d_top[0];
}

float nmb_Line::y (void) const {
  return d_top[1];
}

/**
 * Computes the point at which this line intercepts the given plane
 * and returns the Z value at that point.
 * Note this is the Z value in real-world units; it is not scaled by
 * the plane's scale value.  
 * Uses BCPlane::valueAt(), which will actually give the Z value
 * of a nearby grid point rather than interpolating to the "exact"
 * plane value at (x, y).
 * Uses moveTo to move the line inside the bounds of \a plane 
 * if necessary.
 
 @return 0.0 on failure (plane->valueAt fails twice)
*/
double nmb_Line::getIntercept (BCPlane * plane) {
    double result = 0.0;
    if ( plane->valueAt(&result, d_top[0], d_top[1])) {
        // We requested an out-of-bound value. Fix it.
        // moveTo will do bounds checking and move the line
        // in-bounds.
        moveTo(d_top[0], d_top[1], plane);
                
        plane->valueAt(&result, d_top[0], d_top[1]);
    }
    // Return the unscaled value - the data value in real-world units
    //result *=plane->scale();

    return result;
}
/**
 * Computes the point at which this line intercepts the given plane
 * and returns it in p.
 @overload double nmb_Line::getIntercept (BCPlane * plane)
 */
void nmb_Line::getIntercept (q_vec_type p, BCPlane * plane) {
    // z first, because call to getIntercept(plane) clips to plane boundaries.
  p[2] = getIntercept(plane);
  p[0] = d_top[0];
  p[1] = d_top[1];
}


vrpn_bool nmb_Line::changed (void) const {
  return d_changed;
}

void nmb_Line::normalize (BCPlane * plane) {
  d_top[2] = plane->maxAttainableValue() * plane->scale();
  d_bottom[2] = plane->minAttainableValue() * plane->scale();
  // Above works fine for stream files, where m**AttainableValues is
  // determined by the scanner range. But for static files, it is the
  // range of the data, which can be very small, resulting in short lines. 
  //So let's make the line height about twice the
  // xy extent of the surface
  // NOTE: this totally ignores units - the plane might not be in 
  // of nm like the xy range is. 
  float range = plane->maxX() - plane->minX();
  if (( d_top[2] - d_bottom[2] ) < range) {
    d_top[2] += range;
    d_bottom[2] -= range;
  }
  d_changed = VRPN_TRUE;
}

void nmb_Line::moveTo (float x, float y, BCPlane * plane) {
  // Bounds check
  if (!plane) {
    fprintf(stderr, "nmb_Line::moveTo:  NULL plane!\n");
    return;
  }
  if (x < plane->minX()) {
    x = plane->minX();
  }
  if (x > plane->maxX()) {
    x = plane->maxX();
  }
  if (y < plane->minY()) {
    y = plane->minY();
  }
  if (y > plane->maxY()) {
    y = plane->maxY();
  }

  d_top[0] = x;
  d_top[1] = y;

  d_bottom[0] = x;
  d_bottom[1] = y;

  normalize(plane);

  d_changed = VRPN_TRUE;
}

void nmb_Line::clearChanged (void) {
  d_changed = VRPN_FALSE;
}

void nmb_Line::registerMoveCallback (nmb_LINE_MOVE_CALLBACK f,
                                     void * userdata) {
  moveCallbackEntry * e;

  e = new moveCallbackEntry;
  if (!e) {
    fprintf(stderr, "nmb_Line::registerMoveCallback:  Out of memory.\n");
    return;
  }

  e->f = f;
  e->userdata = userdata;
  e->next = d_moveCallbacks;
  d_moveCallbacks = e;
}

void nmb_Line::doCallbacks (float x, float y, BCPlane * plane) {
  moveCallbackEntry * e;

  if (!plane) {
    fprintf(stderr, "nmb_Line::doCallbacks:  NULL plane!\n");
    return;
  }
  if (x < plane->minX()) {
    x = plane->minX();
  }
  if (x > plane->maxX()) {
    x = plane->maxX();
  }
  if (y < plane->minY()) {
    y = plane->minY();
  }
  if (y > plane->maxY()) {
    y = plane->maxY();
  }

  for (e = d_moveCallbacks; e; e = e->next) {
    (e->f)(x, y, e->userdata);
  }
}

