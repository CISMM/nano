#include "nmr_MultiResObjectiveMI_EMMA.h"
#include "nmr_Util.h"
#include <math.h>

// #define DEBUG_FILE_OUTPUT

// static data members:
int nmr_MultiResObjectiveMI_EMMA::s_defaultNumResolutionLevels = 7;
float nmr_MultiResObjectiveMI_EMMA::s_defaultStdDev[] =
              {0.0, 0.5, 1.0, 2.0, 3.0, 4.0, 8.0};

const int defaultSampleSize = 64;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

nmr_MultiResObjectiveMI_EMMA::nmr_MultiResObjectiveMI_EMMA()
{
  d_numResolutionLevels = s_defaultNumResolutionLevels;
  d_stddev = new float[d_numResolutionLevels];
  d_objectiveMI = new nmr_ObjectiveMI_EMMA[d_numResolutionLevels];
  int i;
  for (i = 0; i < d_numResolutionLevels; i++) {
    d_stddev[i] = s_defaultStdDev[i];
    d_objectiveMI[i].setDimensionMode(REF_2D);
    if (d_objectiveMI[i].setSampleSizes(defaultSampleSize, defaultSampleSize)) {
      printf("setSampleSizes error\n");
    } else {
      //printf("setSampleSizes succeeded\n");
    }
  }
}

nmr_MultiResObjectiveMI_EMMA::nmr_MultiResObjectiveMI_EMMA(int numLevels, float *stddev):
  d_numResolutionLevels(numLevels)
{
  d_stddev = new float[d_numResolutionLevels];
  d_objectiveMI = new nmr_ObjectiveMI_EMMA[d_numResolutionLevels];
  int i;
  for (i = 0; i < d_numResolutionLevels; i++) {
    d_stddev[i] = stddev[i];
    d_objectiveMI[i].setDimensionMode(REF_2D);
    if (d_objectiveMI[i].setSampleSizes(defaultSampleSize, defaultSampleSize)) {
      printf("setSampleSizes error\n");
    } else {
      //printf("setSampleSizes succeeded\n");
    }
  }
}

nmr_MultiResObjectiveMI_EMMA::~nmr_MultiResObjectiveMI_EMMA()
{
  delete [] d_stddev;
  delete [] d_objectiveMI;
}

void nmr_MultiResObjectiveMI_EMMA::getBlurStdDev(float *stddev)
{
  int i; 
  for (i = 0; i < d_numResolutionLevels; i++) {
    stddev[i] = d_stddev[i];
  }
}

void nmr_MultiResObjectiveMI_EMMA::setDimensionMode(nmr_DimensionMode mode)
{
  int i;
  for (i = 0; i < d_numResolutionLevels; i++) {
    d_objectiveMI[i].setDimensionMode(mode);
  }
}

nmr_DimensionMode nmr_MultiResObjectiveMI_EMMA::getDimensionMode()
{
  return d_objectiveMI[0].getDimensionMode();
}

int nmr_MultiResObjectiveMI_EMMA::setSampleSizes(int sizeA, int sizeB)
{
  int i;
  int result = 0;
  for (i = 0; i < d_numResolutionLevels; i++) {
    if (d_objectiveMI[i].setSampleSizes(sizeA, sizeB)) {
       result = -1;
       break;
    }
  }
  return result;
}

void nmr_MultiResObjectiveMI_EMMA::getSampleSizes(int &sizeA, int &sizeB)
{
  d_objectiveMI[0].getSampleSizes(sizeA, sizeB);
}

void nmr_MultiResObjectiveMI_EMMA::setSamplePositionMode(nmr_SamplePositionMode mode)
{
  int i;
  for (i = 0; i < d_numResolutionLevels; i++) {
    d_objectiveMI[i].setSamplePositionMode(mode);
  }
}

void nmr_MultiResObjectiveMI_EMMA::setSampleRejectionCriterion
         (nmr_SampleRejectionCriterion crit)
{
  int i;
  for (i = 0; i < d_numResolutionLevels; i++) {
    d_objectiveMI[i].setSampleRejectionCriterion(crit);
  }
}

void nmr_MultiResObjectiveMI_EMMA::setMinSampleSqrGradientMagnitude(double mag)
{
  int i;
  for (i = 0; i < d_numResolutionLevels; i++) {
    d_objectiveMI[i].setMinSampleSqrGradientMagnitude(mag);
  }
}

void nmr_MultiResObjectiveMI_EMMA::setRefFeaturePoints(int numPnts, 
                                          double *x, double *y)
{
  int i;
  for (i = 0; i < d_numResolutionLevels; i++) {
    d_objectiveMI[i].setRefFeaturePoints(numPnts, x, y);
  }
}

void nmr_MultiResObjectiveMI_EMMA::setReferenceValueImage(nmb_Image *ref,
                                               nmb_ImageList *monitorList)
{
  int i;
  if (ref == NULL) {
    for (i = 0; i < d_numResolutionLevels; i++) {
      d_objectiveMI[i].setReferenceValueImage(NULL);
    }
    return;
  }

  nmb_Image **pyramid = new nmb_Image * [d_numResolutionLevels];
  printf("building gaussian pyramid for reference image...");
  nmr_Util::buildGaussianPyramid(*ref, d_numResolutionLevels, d_stddev, 
                                 pyramid);
  printf("done\n");

  for (i = 0; i < d_numResolutionLevels; i++) {
    d_objectiveMI[i].setReferenceValueImage(pyramid[i]);
    if (monitorList) {
      monitorList->addImage(pyramid[i]);
    }
  }
  delete [] pyramid;
}

void nmr_MultiResObjectiveMI_EMMA::setTestValueImage(nmb_Image *test,
                                               nmb_ImageList *monitorList)
{
  int i;
  if (test == NULL) {
    for (i = 0; i < d_numResolutionLevels; i++) {
      d_objectiveMI[i].setTestValueImage(NULL);
    }
    return;
  }

  nmb_Image **pyramid = new nmb_Image * [d_numResolutionLevels];
  printf("building gaussian pyramid for test image...");
  nmr_Util::buildGaussianPyramid(*test, d_numResolutionLevels, d_stddev, 
                                 pyramid);
  printf("done\n");

  for (i = 0; i < d_numResolutionLevels; i++) {
    d_objectiveMI[i].setTestValueImage(pyramid[i]);
    if (monitorList) {
      monitorList->addImage(pyramid[i]);
    }
  }
  delete [] pyramid;
}

void nmr_MultiResObjectiveMI_EMMA::setReferenceZImage(nmb_Image *ref_z,
                                               nmb_ImageList *monitorList)
{
  int i;
  if (ref_z == NULL) {
    for (i = 0; i < d_numResolutionLevels; i++) {
      d_objectiveMI[i].setReferenceZImage(NULL);
    }
    return;
  }

  for (i = 0; i < d_numResolutionLevels; i++) {
    d_objectiveMI[i].setReferenceZImage(ref_z);
  }
}

double nmr_MultiResObjectiveMI_EMMA::value(int level, 
                                      double *testFromReferenceTransform)
{
  return d_objectiveMI[level].value(testFromReferenceTransform);
}

void nmr_MultiResObjectiveMI_EMMA::gradient(int level, 
                  double *testFromReferenceTransform,
                  double *gradMI)
{
  d_objectiveMI[level].gradient(testFromReferenceTransform, gradMI);
}

void nmr_MultiResObjectiveMI_EMMA::valueAndGradient(int level, 
                double *testFromReferenceTransform,
                double &valueMI, double *gradMI)
{
  d_objectiveMI[level].valueAndGradient(testFromReferenceTransform,
                                   valueMI, gradMI);
}

int nmr_MultiResObjectiveMI_EMMA::getJointHistogram(int level,
                           nmb_Image *histogram, 
                           double *transform, vrpn_bool blur,
                           vrpn_bool setRefScale,
                           float min_ref, float max_ref,
                           vrpn_bool setTestScale,
                           float min_test, float max_test)
{

  if (level < 0 || level >= d_numResolutionLevels) return -1;

  return d_objectiveMI[level].getJointHistogram(histogram, transform, blur,
             setRefScale, min_ref, max_ref, setTestScale, min_test, max_test);
}

void nmr_MultiResObjectiveMI_EMMA::setCovariance(int level, double sigmaRefRef, 
                                                       double sigmaTestTest)
{
  d_objectiveMI[level].setCovariance(sigmaRefRef, sigmaTestTest);
}

void nmr_MultiResObjectiveMI_EMMA::getCovariance(int level, 
                              double &sigmaRefRef, double &sigmaTestTest)
{
  d_objectiveMI[level].getCovariance(sigmaRefRef, sigmaTestTest);
}

void nmr_MultiResObjectiveMI_EMMA::crossEntropyGradient(int level, 
                                         double &dHc_dsigma_ref,
                                         double &dHc_dsigma_test)
{
  if (d_objectiveMI[level].computeCrossEntropyGradient(dHc_dsigma_ref,
                                                       dHc_dsigma_test)) {
    fprintf(stderr, "nmr_MultiResObjectiveMI_EMMA::crossEntropyGradient: "
            "Warning, samples not yet acquired, will do this automatically\n");
    d_objectiveMI[level].buildSampleSets();
    d_objectiveMI[level].computeCrossEntropyGradient(dHc_dsigma_ref,
                                                     dHc_dsigma_test);
  }
}

void nmr_MultiResObjectiveMI_EMMA::crossEntropy(int level, double &entropy)
{
  if (d_objectiveMI[level].crossEntropy(entropy)) {
    fprintf(stderr, "nmr_MultiResObjectiveMI_EMMA::crossEntropy: "
            "Warning, samples not yet acquired, will do this automatically\n");
    d_objectiveMI[level].buildSampleSets();
    d_objectiveMI[level].crossEntropy(entropy);
  }
}

void nmr_MultiResObjectiveMI_EMMA::setTestVariance(int level, double sigma)
{
  d_objectiveMI[level].setTestVariance(sigma);
}

void nmr_MultiResObjectiveMI_EMMA::getTestVariance(int level, double &sigma)
{
  d_objectiveMI[level].getTestVariance(sigma);
}

void nmr_MultiResObjectiveMI_EMMA::testEntropyGradient(int level, 
                                                  double &dH_dsigma_test)
{
  if (d_objectiveMI[level].computeTestEntropyGradient(dH_dsigma_test)) {
    fprintf(stderr, "nmr_MultiResObjectiveMI_EMMA::testEntropyGradient: "
            "Warning, samples not yet acquired, will do this automatically\n");
    d_objectiveMI[level].buildSampleSets();
    d_objectiveMI[level].computeTestEntropyGradient(dH_dsigma_test);
  }
}

void nmr_MultiResObjectiveMI_EMMA::testEntropy(int level, double &entropy)
{
  if (d_objectiveMI[level].testEntropy(entropy)) {
    fprintf(stderr, "nmr_MultiResObjectiveMI_EMMA::testEntropy: "
            "Warning, samples not yet acquired, will do this automatically\n");
    d_objectiveMI[level].buildSampleSets();
    d_objectiveMI[level].testEntropy(entropy);
  }
}

void nmr_MultiResObjectiveMI_EMMA::setRefVariance(int level, double sigma)
{
  d_objectiveMI[level].setRefVariance(sigma);
}

void nmr_MultiResObjectiveMI_EMMA::getRefVariance(int level, double &sigma)
{
  d_objectiveMI[level].getRefVariance(sigma);
}

void nmr_MultiResObjectiveMI_EMMA::refEntropyGradient(int level, 
                                             double &dH_dsigma_ref)
{
  if (d_objectiveMI[level].computeRefEntropyGradient(dH_dsigma_ref)) {
    fprintf(stderr, "nmr_MultiResObjectiveMI_EMMA::refEntropyGradient: "
            "Warning, samples not yet acquired, will do this automatically\n");
    d_objectiveMI[level].buildSampleSets();
    d_objectiveMI[level].computeRefEntropyGradient(dH_dsigma_ref);
  }
}

void nmr_MultiResObjectiveMI_EMMA::refEntropy(int level, double &entropy)
{
  if (d_objectiveMI[level].refEntropy(entropy)) {
    fprintf(stderr, "nmr_MultiResObjectiveMI_EMMA::refEntropy: "
            "Warning, samples not yet acquired, will do this automatically\n");
    d_objectiveMI[level].buildSampleSets();
    d_objectiveMI[level].refEntropy(entropy);
  }
}
