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
  SLOW_LINE,
  SLOW_LINE_3D
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

// TODO:  rewrite NetworkedMicroscopeChannel so we don't expose these,
// (or switch to VRPN first)

enum {
  SOCKET_NONE = 0,
  SOCKET_TCP = 1,
  SOCKET_UDP = 2,
  SOCKET_MIX = 3
};



#endif  // NMM_TYPES_H

  
