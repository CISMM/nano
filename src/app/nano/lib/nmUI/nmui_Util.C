#include "nmui_Util.h"

#include <v.h>  // for v_xform_type, v_get_world_from_hand()
#include <stdio.h>  // for stderr

#include <BCPlane.h>

#include <nmb_Globals.h>  // for dataset
#include <nmb_Dataset.h>
#include <nmb_String.h>
#include <nmg_Globals.h>  // for graphics
#include <nmg_Graphics.h>

// static
int nmui_Util::getHandInWorld (int user, q_vec_type & position) {
  v_xform_type worldFromHand;
  v_get_world_from_hand(user, &worldFromHand);
  q_vec_copy(position, worldFromHand.xlate);
  return 0;
}

// static
vrpn_bool nmui_Util::convertPositionToNM (BCPlane * plane,
                                          q_vec_type & position) {

  // We want the position we return to be in real-world units (nanometers),
  // not the world coords of vlib, so we must take into account
  // the arbitrary scale applied to the Z coord by the plane.

  if (plane->scale() > 1e-10) {
    position[2] = position[2] / plane->scale();
    return vrpn_true;
  } else {
    // signal error
    return vrpn_false;
  }
}

/** Takes a position in world space, and makes sure it is inside
 the edges of the plane passed in. This works because the boundaries
of the plane are defined in world space. */

// static
int nmui_Util::clipPosition (BCPlane * plane, q_vec_type & position) {

  if (!plane) {
    fprintf(stderr, "Error in nmui_Util::clipPosition:  "
                    "No input plane.\n");
    return -1;
  }

  if (position[0] < plane->minX()) position[0] = plane->minX();
  if (position[0] > plane->maxX()) position[0] = plane->maxX();
  if (position[1] < plane->minY()) position[1] = plane->minY();
  if (position[1] > plane->maxY()) position[1] = plane->maxY();

  return 0;
}

/** Constrain movement to a line, represented by the first two points in the
    position_list. */

// static
int nmui_Util::clipPositionLineConstraint (BCPlane * plane, 
					   q_vec_type & position, 
					   Position_list & p) {
  if (!plane) {
    fprintf(stderr, "Error in nmui_Util::clipPositionLineConstraint:  "
                    "No input plane.\n");
    return -1;
  }

  if (position[0] < plane->minX()) position[0] = plane->minX();
  if (position[0] > plane->maxX()) position[0] = plane->maxX();
  if (position[1] < plane->minY()) position[1] = plane->minY();
  if (position[1] > plane->maxY()) position[1] = plane->maxY();

  //We want the position we return to be in real-world units (nanometers),
  // not the world coords of vlib. So we must take into account
  // the arbitrary scale applied to the Z coord by the plane.
  position[2] = position[2]/plane->scale();

  // position now holds the user's real hand position. We want to
  // constrain it to a line defined by the two points stored in the
  // position_list p. We need to find the point on the line closest to
  // the hand position. So we take a vector from p0 to hand, and
  // project it onto the vector from p0 to p1. Adding this vector to
  // p0 gives us our point. See Folley and vanDam p. 1100 exercise for
  // more.
  
  // vector v from p0 to p1
  float v0, v1;
  p.goToHead();
  v0 = (p.peekNext())->x() - p.currX();
  v1 = (p.peekNext())->y() - p.currY();

  // if v is zero this won't work - return hand position unchanged.  
  if ((v0 == 0) && (v1 == 0)) return 0;

  // vector w from p0 to hand position.
  float w0, w1;
  w0 = position[0] - p.currX();
  w1 = position[1] - p.currY();

  // parameter t of line from p0 to p1. Solve eqn for t: (w.v)/(v.v)
  float t = (w0*v0 + w1*v1)/(v0*v0 + v1*v1);

  // Our new point is p0 + t*v
  position[0] = p.currX() + t * v0;
  position[1] = p.currY() + t * v1;

  return 0;
}

// static
int nmui_Util::moveAimLine (BCPlane * plane, q_vec_type position, 
			    nmg_Graphics * graphics) {
  PointType Top, Bot;

  Top[0] = Bot[0] = position[0];
  Top[1] = Bot[1] = position[1];
  Top[2] = plane->maxAttainableValue() * plane->scale();
  Bot[2] = plane->minAttainableValue() * plane->scale();

  graphics->positionAimLine(Top, Bot);

  return 0;
}

// static
int nmui_Util::moveSphere (q_vec_type position, nmg_Graphics * graphics) {

  graphics->positionSphere(position[0], position[1], position[2]);

  return 0;
}
