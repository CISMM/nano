#ifndef NMUI_UTIL_H
#define NMUI_UTIL_H

#include <quat.h>
#include "Position.h"

// utility functions for nmui (nanmManipulator User Interface)

struct nmui_Util {

  static int clipPosition (int user, q_vec_type & clippedPosition);
  static int clipPositionLineConstraint (int user, 
					 q_vec_type & clippedPosition, 
					 Position_list & p);
  static int moveAimLine (q_vec_type position);  // OBSOLETE
  static int moveSphere (q_vec_type position);
};


#endif  // NMUI_UTIL_H
