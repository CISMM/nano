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


vrpn_bool nmb_Line::changed (void) const {
  return d_changed;
}

void nmb_Line::normalize (BCPlane * plane) {
  d_top[2] = plane->maxAttainableValue() * plane->scale();
  d_bottom[2] = plane->minAttainableValue() * plane->scale();

  d_changed = VRPN_TRUE;
}

void nmb_Line::moveTo (float x, float y, BCPlane * plane) {
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

void nmb_Line::doCallbacks (void) {
  moveCallbackEntry * e;

  for (e = d_moveCallbacks; e; e = e->next) {
    (e->f)(this, e->userdata);
  }
}

