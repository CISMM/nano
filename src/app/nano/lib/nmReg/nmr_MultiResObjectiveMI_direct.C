#include "nmr_MultiResObjectiveMI_direct.h"
#include "nmr_Util.h"
#include <math.h>

// #define DEBUG_FILE_OUTPUT

// static data members:
int nmr_MultiResObjectiveMI_direct::s_defaultNumResolutionLevels = 7;
float nmr_MultiResObjectiveMI_direct::s_defaultStdDev[] =
              {0.0, 0.5, 1.0, 2.0, 3.0, 4.0, 8.0};

#ifndef M_PI
static double M_PI = 3.14159265358979323846;
#endif

nmr_MultiResObjectiveMI_direct::nmr_MultiResObjectiveMI_direct()
{
  d_numResolutionLevels = s_defaultNumResolutionLevels;
  d_stddev = new float[d_numResolutionLevels];
  d_sortOrder = new int[d_numResolutionLevels];
  d_objectiveMI = new nmr_ObjectiveMI_direct[d_numResolutionLevels];
  int i;
  for (i = 0; i < d_numResolutionLevels; i++) {
    d_stddev[i] = s_defaultStdDev[i];
    d_objectiveMI[i].setDimensionMode(REF_2D);
    d_sortOrder[i] = i;
  }
  // do a little selection sort:
  int j;
  int maxIndex = 0;
  int temp;
  for (i = 0; i < d_numResolutionLevels; i++) {
    maxIndex = i;
    for (j = i; j < d_numResolutionLevels; j++) {
      if (d_stddev[d_sortOrder[j]] > d_stddev[d_sortOrder[maxIndex]]) {
        maxIndex = j;
      }
    }
    temp = d_sortOrder[i];
    d_sortOrder[i] = d_sortOrder[maxIndex];
    d_sortOrder[maxIndex] = temp;
  }
}

nmr_MultiResObjectiveMI_direct::nmr_MultiResObjectiveMI_direct(int numLevels, 
  float *stddev):
  d_numResolutionLevels(numLevels)
{
  d_stddev = new float[d_numResolutionLevels];
  d_sortOrder = new int[d_numResolutionLevels];
  d_objectiveMI = new nmr_ObjectiveMI_direct[d_numResolutionLevels];
  int i;
  for (i = 0; i < d_numResolutionLevels; i++) {
    d_stddev[i] = stddev[i];
    d_objectiveMI[i].setDimensionMode(REF_2D);
    d_sortOrder[i] = i;
  }
  // do a little selection sort:
  int j;
  int maxIndex = 0;
  int temp;
  for (i = 0; i < d_numResolutionLevels; i++) {
    maxIndex = i;
    for (j = i; j < d_numResolutionLevels; j++) {
      if (d_stddev[d_sortOrder[j]] > d_stddev[d_sortOrder[maxIndex]]) {
        maxIndex = j;
      }
    }
    temp = d_sortOrder[i];
    d_sortOrder[i] = d_sortOrder[maxIndex];
    d_sortOrder[maxIndex] = temp;
  }
}

nmr_MultiResObjectiveMI_direct::~nmr_MultiResObjectiveMI_direct()
{
  delete [] d_stddev;
  delete [] d_objectiveMI;
}

void nmr_MultiResObjectiveMI_direct::getBlurStdDev(float *stddev)
{
  int i; 
  for (i = 0; i < d_numResolutionLevels; i++) {
    stddev[i] = d_stddev[i];
  }
}

int nmr_MultiResObjectiveMI_direct::getLevelByScaleOrder(int order) 
{
  return d_sortOrder[order];
}

void nmr_MultiResObjectiveMI_direct::setDimensionMode(nmr_DimensionMode mode)
{
  int i;
  for (i = 0; i < d_numResolutionLevels; i++) {
    d_objectiveMI[i].setDimensionMode(mode);
  }
}

nmr_DimensionMode nmr_MultiResObjectiveMI_direct::getDimensionMode()
{
  return d_objectiveMI[0].getDimensionMode();
}

void nmr_MultiResObjectiveMI_direct::setReferenceValueImage(nmb_Image *ref,
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

void nmr_MultiResObjectiveMI_direct::setTestValueImage(nmb_Image *test,
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

void nmr_MultiResObjectiveMI_direct::setReferenceZImage(nmb_Image *ref_z,
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

double nmr_MultiResObjectiveMI_direct::value(int level, 
                                      double *testFromReferenceTransform)
{
  return d_objectiveMI[level].value(testFromReferenceTransform);
}

void nmr_MultiResObjectiveMI_direct::gradient(int level, 
                  double *testFromReferenceTransform,
                  double *gradMI)
{
  d_objectiveMI[level].gradient(testFromReferenceTransform, gradMI);
}

void nmr_MultiResObjectiveMI_direct::valueAndGradient(int level, 
                double *testFromReferenceTransform,
                double &valueMI, double *gradMI)
{
  d_objectiveMI[level].valueAndGradient(testFromReferenceTransform,
                                   valueMI, gradMI);
}
