#ifndef NMM_TYPES_H
#define NMM_TYPES_H

struct Blunt_result {
  float x, y;
  float height, std_dev;
  double      normal[3];
};

enum AcquisitionMode {
  IMAGE,
  MODIFY,
  SCANLINE
}; 

enum AFMMode {
  TAPPING,
  CONTACT
};  // mode

enum Tool {
  FREEHAND,
  LINE,
  CONSTR_FREEHAND,
  CONSTR_FREEHAND_XYZ,
  SLOW_LINE,
  SLOW_LINE_3D,
  FEELAHEAD,
  OPTIMIZE_NOW
};  // tool

// style is defined in nmb_Types.h

enum Control {
   FEEDBACK,
   DIRECTZ
};  // control

// Used by the Slow Line tool - step and play direction specification.
enum Direction {
  FORWARD,
  REVERSE
}; //direction

// Used by Constrained Freehand XYZ mode
enum ConstrXYZMode {
  CONSTR_XYZ_LINE,
  CONSTR_XYZ_PLANE
};

enum OptimizeNowMode {
  OPTIMIZE_NOW_LINE,
  OPTIMIZE_NOW_AREA
};
// TODO:  rewrite NetworkedMicroscopeChannel so we don't expose these,
// (or switch to VRPN first)

enum {
  SOCKET_NONE = 0,
  SOCKET_TCP = 1,
  SOCKET_UDP = 2,
  SOCKET_MIX = 3
};

#endif  // NMM_TYPES_H

