#ifndef NMUI_UTIL_H
#define NMUI_UTIL_H

#include <quat.h>
#include <vrpn_Types.h>

#include "Position.h"

class BCPlane;  // from <BCPlane.h>
class nmg_Graphics;  // from <nmg_Graphics.h>

// utility functions for nmui (nanmManipulator User Interface)

struct nmui_Util {

  static int getHandInWorld (int user, q_vec_type & position);
  static vrpn_bool convertPositionToNM (BCPlane * plane, q_vec_type & position);
    ///< Divides position[2] by plane->scale() and returns vrpn_true if scale
    ///< is not near zero; returns vrpn_false (error) otherwise.
  static int clipPosition (BCPlane * plane, q_vec_type & clippedPosition);
  static int clipPositionLineConstraint (BCPlane * plane, 
					 q_vec_type & position, 
					 Position_list & p,
					 int user_mode = -1,
					 int xyz_param = -1);
  static int moveAimLine (BCPlane * plane, q_vec_type position, 
			    nmg_Graphics * graphics);  // OBSOLETE
  static int moveSphere (q_vec_type position, nmg_Graphics * graphics);
};


#endif  // NMUI_UTIL_H
