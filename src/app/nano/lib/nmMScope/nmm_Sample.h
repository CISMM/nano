#ifndef NMM_SAMPLE_H
#define NMM_SAMPLE_H


///< Specifies the set of samples to be taken by a FeelAhead command.
///< Current implementation is only as a grid:  nx by ny samples, spaced
///< dx and dy nm apart respectively, rotated <orientation> radians.

struct nmm_Sample {

  vrpn_int32 numx;
  vrpn_int32 numy;

  vrpn_float32 dx;
  vrpn_float32 dy;

  vrpn_float32 orientation;

};


#endif  // NMM_SAMPLE_H

