#ifndef NMUI_UTIL_H
#define NMUI_UTIL_H

#include <quat.h>
#include "Position.h"
class BCPlane;
class nmg_Graphics;

// utility functions for nmui (nanmManipulator User Interface)

struct nmui_Util {

  static int getHandInWorld (int user, q_vec_type & position);
  static int clipPosition (BCPlane * plane, q_vec_type & clippedPosition);
  static int clipPositionLineConstraint (BCPlane * plane, 
					   q_vec_type & position, 
					 Position_list & p);
  static int moveAimLine (BCPlane * plane, q_vec_type position, 
			    nmg_Graphics * graphics);  // OBSOLETE
  static int moveSphere (q_vec_type position, nmg_Graphics * graphics);
};


#endif  // NMUI_UTIL_H
