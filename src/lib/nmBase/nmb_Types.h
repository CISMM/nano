#ifndef NMB_TYPES_H
#define NMB_TYPES_H

//------------------------------------------------------------------
//   This header contains definitions for architecture-dependent
// types, to make sure size of data type is consistent over a 
// network connection.

#include <vrpn_Types.h>


typedef float PointType [3];


enum UserMode {
  USER_FLY_MODE = (0),
  USER_GRAB_MODE = (1),
  USER_SCALE_UP_MODE = (2),
  USER_SCALE_DOWN_MODE = (3),
  USER_SERVO_MODE = (4),
  USER_PULSE_MODE = (5),
  USER_MEAS_MOVE_MODE = (6),
  USER_LINE_MODE = (7),
  USER_SWEEP_MODE = (8),
  USER_MEASURE_MODE = (9),
  USER_LIGHT_MODE = (10),
  USER_PLANE_MODE = (11),
  USER_PLANEL_MODE = (12),
  USER_BLUNT_TIP_MODE = (13),
  USER_COMB_MODE = (14),
  USER_CENTER_TEXTURE_MODE = (15),
  USER_SCANLINE_MODE = (16),
  USER_REGION_MODE = (17),
  USER_CROSS_SECTION_MODE = (18),
  NUM_USER_MODES = (19)
};  // user_mode
  // values are fixed because they must correspond to literals in Tcl/Tk

enum AFMStyle {
  SHARP,
  BLUNT,
  SWEEP,
  SEWING,
  FORCECURVE
};  // style


#endif  // NMB_TYPES_H
