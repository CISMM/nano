#ifndef NMR_GAUSSIAN
#define NMR_GAUSSIAN

#include "vrpn_Types.h"

class nmr_Gaussian {
 public:
  static double interpolatedStandardValue(double x);
  static double value(double x, double inv_sigma, double mu);

 protected:
  static void init();
  static vrpn_bool s_initialized;
  
  static double s_xMin, s_xMax;
  static int s_numSamples;
  static double s_sampleInterval;
  static double s_invSampleInterval;
  static double *s_gaussianValues;
};

#endif
