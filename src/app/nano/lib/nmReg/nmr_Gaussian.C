#include <stdlib.h>
#include "nmr_Gaussian.h"
#include "math.h"
#include "stdio.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

vrpn_bool nmr_Gaussian::s_initialized = vrpn_FALSE;
double nmr_Gaussian::s_xMin = -38.0;
double nmr_Gaussian::s_xMax = 38.0;
int nmr_Gaussian::s_numSamples = 10239;
double nmr_Gaussian::s_sampleInterval = 0.05;
double nmr_Gaussian::s_invSampleInterval = 20;
double *nmr_Gaussian::s_gaussianValues = NULL;


void nmr_Gaussian::init()
{
  s_sampleInterval = (s_xMax - s_xMin)/(s_numSamples - 1);
  s_invSampleInterval = 1.0/s_sampleInterval;
  s_gaussianValues = new double[s_numSamples];
  int i;
  double x;
  double factor = (1.0/sqrt(2.0*M_PI));
  for (i = 0, x = s_xMin; i < s_numSamples; i++, x+=s_sampleInterval) {
    s_gaussianValues[i] = factor*exp(-0.5*x*x);
  }
  s_initialized = vrpn_TRUE;
}

double nmr_Gaussian::interpolatedStandardValue(double x)
{
  if (!s_initialized) {
    init();
  }
  if (x < s_xMin || x > s_xMax) {
    return 0.0;
  } else {
    double fbin = ((x - s_xMin)*s_invSampleInterval);
    int ibin = (int)floor(fbin);
    double a = fbin - ibin; 
    double val_below = s_gaussianValues[ibin];
    double val_above = s_gaussianValues[ibin+1];
    return val_below*(1.0-a) + val_above*a;
  }
}

double nmr_Gaussian::value(double x, double inv_sigma, double mu)
{
  double x_standard = (x-mu)*inv_sigma;
  double result = interpolatedStandardValue(x_standard);
  result *= inv_sigma;
  return result;
}

void nmr_Gaussian::makeFilter(int numVal, double *val, double inv_sigma)
{
  double mu = 0.0;
  int i;
  double x;
  double sum = 0.0, sum_inv;
  for (i = 0, x = -0.5*(double)(numVal-1); i < numVal; i++, x += 1.0) {
    val[i] = value(x, inv_sigma, mu);
    sum += val[i];
  }
  sum_inv = 1.0/sum;

  // normalization so integral is 1.0
  for (i = 0; i < numVal; i++) {
    val[i] *= sum_inv;
  }
}
