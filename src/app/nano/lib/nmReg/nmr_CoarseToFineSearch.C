#include "nmr_CoarseToFineSearch.h"
#include "nmr_Util.h"
#include <math.h>

// #define DEBUG_FILE_OUTPUT

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

nmr_CoarseToFineSearch::nmr_CoarseToFineSearch():
  d_numResolutionLevels(0),
  d_resolutionScaleFactor(0.5),
  d_refImages(NULL),
  d_testImages(NULL),
  d_testGradXImages(NULL),
  d_testGradYImages(NULL),
  d_minSqrTestGradMagnitude(NULL),
  d_currentLevel(0),
  d_sigmaRef(1/sqrt(2.0*M_PI)), d_sigmaTest(1/sqrt(2.0*M_PI)), 
  d_sigmaRefRef(1/sqrt(2.0*M_PI)), d_sigmaTestTest(1/sqrt(2.0*M_PI)),
  d_transformUpdateRate(0.01),
  d_sigmaUpdateRate(0.01),
  d_levelSwitchThreshold(0),
  d_firstStepAtLevel(vrpn_TRUE),
  d_lastDirection(NMR_TRANSLATE_X),
  d_lastDirectionSign(+1),
  d_lastObjFuncValue(0),
  d_currObjFuncValue(1),
  d_lastStepSize(0.1),
  d_startStepSize(0.1),
  d_stepsAtLevel(0),
  d_direction(NMR_TRANSLATE_X),
  d_takeRandomSamples(VRPN_TRUE)
{
  d_transformPixelUnits[0] = 1.0;
  d_transformPixelUnits[1] = 0.0;
  d_transformPixelUnits[2] = 0.0;
  d_transformPixelUnits[3] = 0.0;
  d_transformPixelUnits[4] = 1.0;
  d_transformPixelUnits[5] = 0.0;

  d_transformImageUnits[0] = 1.0;
  d_transformImageUnits[1] = 0.0;
  d_transformImageUnits[2] = 0.0;
  d_transformImageUnits[3] = 0.0;
  d_transformImageUnits[4] = 1.0;
  d_transformImageUnits[5] = 0.0;

  d_alignerMI.setDimensionMode(REF_2D);
  if (d_alignerMI.setSampleSizes(200, 200)) {
    printf("setSampleSizes error\n");
  } else {
      //printf("setSampleSizes succeeded\n");
  }
}

nmr_CoarseToFineSearch::~nmr_CoarseToFineSearch()
{
  int i;
  for (i = 0; i < d_numResolutionLevels; i++) {
    nmb_Image::deleteImage(d_refImages[i]);
    nmb_Image::deleteImage(d_testImages[i]);
    nmb_Image::deleteImage(d_testGradXImages[i]);
    nmb_Image::deleteImage(d_testGradYImages[i]);
  }
  if (d_refImages) {
    delete [] d_refImages;
  }
  if (d_testImages) {
    delete [] d_testImages;
  }
  if (d_testGradXImages) {
    delete [] d_testGradXImages;
  }
  if (d_testGradYImages) {
    delete [] d_testGradYImages;
  }
  if (d_minSqrTestGradMagnitude) {
    delete [] d_minSqrTestGradMagnitude;
  }
}

int nmr_CoarseToFineSearch::initImages(nmb_Image *reference, nmb_Image *test,
                                       nmb_ImageList *monitorList)
{
  if (!reference || !test) {
    printf("nmr_CoarseToFineSearch::initImages: error, NULL input image\n");
    return -1;
  }
  int i;
  
  if (!d_takeRandomSamples) {
    int numPixelsInReference = reference->width()*reference->height();
    d_alignerMI.setSampleSizes(numPixelsInReference, numPixelsInReference);
  }

  for (i = 0; i < d_numResolutionLevels; i++) {
    nmb_Image::deleteImage(d_refImages[i]);
    nmb_Image::deleteImage(d_testImages[i]);
    nmb_Image::deleteImage(d_testGradXImages[i]);
    nmb_Image::deleteImage(d_testGradYImages[i]);
  }

  if (d_refImages) {
    delete [] d_refImages;
  }
  if (d_testImages) {
    delete [] d_testImages;
  }
  if (d_testGradXImages) {
    delete [] d_testGradXImages;
  }
  if (d_testGradYImages) {
    delete [] d_testGradYImages;
  }
  if (d_minSqrTestGradMagnitude) {
    delete [] d_minSqrTestGradMagnitude;
  }
  int ref_width = reference->width();
  int ref_height = reference->height();
  int test_width = test->width();
  int test_height = test->height();

  int min_resolution = ref_width;
  if (test_width < min_resolution) 
     min_resolution = test_width;
  if (ref_height < min_resolution) 
     min_resolution = ref_height;
  if (test_height < min_resolution) 
     min_resolution = test_height;
  if (d_resolutionScaleFactor > 1.0) {
    printf("initImages: programmer error, scale factor > 1\n");
    printf("  proceeding using 1/scale instead\n");
    d_resolutionScaleFactor = 1.0/d_resolutionScaleFactor;
  }
  d_numResolutionLevels = 1; 
           // -log((double)min_resolution)/log(d_resolutionScaleFactor) - 3;
  if (d_numResolutionLevels > 4)
      d_numResolutionLevels = 4;

  d_refImages = new nmb_Image *[d_numResolutionLevels];
  d_testImages = new nmb_Image *[d_numResolutionLevels];
  d_testGradXImages = new nmb_Image *[d_numResolutionLevels];
  d_testGradYImages = new nmb_Image *[d_numResolutionLevels];
  d_minSqrTestGradMagnitude = new double[d_numResolutionLevels];

  const char *refImageName, *refUnitsName;
  const char *testImageName, *testUnitsName;
  refImageName = *(reference->name());
  refUnitsName = *(reference->unitsValue());
  testImageName = *(test->name());
  testUnitsName = *(test->unitsValue());

  d_refImages[d_numResolutionLevels-1] = new nmb_ImageGrid(reference);
  d_testImages[d_numResolutionLevels-1] = new nmb_ImageGrid(test);

  char gradXTestImageName[256];
  char gradYTestImageName[256];
  sprintf(gradXTestImageName, "%s_gradx", testImageName);
  sprintf(gradYTestImageName, "%s_grady", testImageName);

  d_testGradXImages[d_numResolutionLevels-1] =
         new nmb_ImageGrid(gradXTestImageName, testUnitsName, 
                           test_width, test_height);
  d_testGradYImages[d_numResolutionLevels-1] =
         new nmb_ImageGrid(gradYTestImageName, testUnitsName, 
                           test_width, test_height);

  nmr_Util::createGradientImages(*test,
       *d_testGradXImages[d_numResolutionLevels-1],
       *d_testGradYImages[d_numResolutionLevels-1]);

  float maxGradX, minGradX, maxGradY, minGradY;
  maxGradX = d_testGradXImages[d_numResolutionLevels-1]->maxValue();
  minGradX = d_testGradXImages[d_numResolutionLevels-1]->minValue();
  maxGradY = d_testGradYImages[d_numResolutionLevels-1]->maxValue();
  minGradY = d_testGradYImages[d_numResolutionLevels-1]->minValue();
  if (fabs(maxGradX) < fabs(minGradX)) {
     maxGradX = fabs(minGradX);
  }
  if (fabs(maxGradY) < fabs(minGradY)) {
     maxGradY = fabs(minGradY);
  }
  d_minSqrTestGradMagnitude[d_numResolutionLevels-1] = 0.0;
    //   0.1*(maxGradX*maxGradX + maxGradY*maxGradY);

  printf("at level %d, min gradient is %g\n", d_numResolutionLevels-1,
           sqrt(d_minSqrTestGradMagnitude[d_numResolutionLevels-1]));


  ref_width = (int)((double)ref_width*d_resolutionScaleFactor);
  ref_height = (int)((double)ref_height*d_resolutionScaleFactor);
  test_width = (int)((double)test_width*d_resolutionScaleFactor);
  test_height = (int)((double)test_height*d_resolutionScaleFactor);

  char subsampled_image_name[256];
  printf("starting resampling\n");
  for (i = d_numResolutionLevels-2; i >= 0; i--) {
    sprintf(subsampled_image_name, "%s_%d", refImageName,
                                   d_numResolutionLevels-1-i);
    d_refImages[i] = new nmb_ImageGrid(subsampled_image_name, 
                                       refUnitsName, 
                                       ref_width, ref_height);
    sprintf(subsampled_image_name, "%s_%d", testImageName,
                                   d_numResolutionLevels-1-i);
    d_testImages[i] = new nmb_ImageGrid(subsampled_image_name, 
                                       testUnitsName,
                                       test_width, test_height);

    nmr_Util::resample(*d_refImages[i+1], *d_refImages[i]);
    nmr_Util::resample(*d_testImages[i+1], *d_testImages[i]);

    sprintf(subsampled_image_name, "%s_%d", gradXTestImageName,
                                   d_numResolutionLevels-1-i);
    d_testGradXImages[i] = new nmb_ImageGrid(subsampled_image_name,
                                             testUnitsName,
                                             test_width, test_height);

    sprintf(subsampled_image_name, "%s_%d", gradYTestImageName,
                                   d_numResolutionLevels-1-i);
    d_testGradYImages[i] = new nmb_ImageGrid(subsampled_image_name,
                                             testUnitsName,
                                             test_width, test_height);

    nmr_Util::createGradientImages(*d_testImages[i],
           *d_testGradXImages[i], *d_testGradYImages[i]);

    maxGradX = d_testGradXImages[i]->maxValue();
    minGradX = d_testGradXImages[i]->minValue();
    maxGradY = d_testGradYImages[i]->maxValue();
    minGradY = d_testGradYImages[i]->minValue();
    if (fabs(maxGradX) < fabs(minGradX)) {
       maxGradX = fabs(minGradX);
    }
    if (fabs(maxGradY) < fabs(minGradY)) {
       maxGradY = fabs(minGradY);
    }
    d_minSqrTestGradMagnitude[i] =  0.0;
//	0.1*(maxGradX*maxGradX + maxGradY*maxGradY);

    printf("at level %d, min gradient is %g\n", i, 
           sqrt(d_minSqrTestGradMagnitude[i]));

    ref_width = (int)((double)ref_width*d_resolutionScaleFactor);
    ref_height = (int)((double)ref_height*d_resolutionScaleFactor);
    test_width = (int)((double)test_width*d_resolutionScaleFactor);
    test_height = (int)((double)test_height*d_resolutionScaleFactor);
    ref_width += (ref_width % 2);
    ref_height += (ref_height % 2);
    test_width += (test_width % 2);
    test_height += (test_height % 2);
  }
  printf("done resampling\n");
  // add references to an external list so the images can be used outside
  // this class
  if (monitorList) {
    for (i = 0; i < d_numResolutionLevels-1; i++) {
      monitorList->addImage(d_refImages[i]);
      monitorList->addImage(d_testImages[i]);
      monitorList->addImage(d_testGradXImages[i]);
      monitorList->addImage(d_testGradYImages[i]);
    }
    monitorList->addImage(d_testGradXImages[d_numResolutionLevels-1]);
    monitorList->addImage(d_testGradYImages[d_numResolutionLevels-1]);
  }

  /* the Parzen window variance needs to be large enough to allow the
     gaussian window function to bridge the largest intensity
     differences between adjacent pixels. If it isn't big enough then
     our gradient estimates will be too small or 0 instead of something
     useful */


  printf("max gradient x = %g, max gradient y = %g\n", maxGradX, maxGradY);


  // compute a quick and dirty estimate of what the
  // Parzen window variance should be
  d_sigmaTest = 0.1*(test->maxValue() - test->minValue());
  d_sigmaRef = 0.1*(reference->maxValue() - reference->minValue());
  d_sigmaRefRef = d_sigmaRef;
  d_sigmaTestTest = d_sigmaTest;

  convertTransformToPixelUnits(d_transformImageUnits, d_transformPixelUnits);

  // try optimizing the Parzen window parameters
  d_alignerMI.setTransformation2D(d_transformPixelUnits);
  d_alignerMI.setRefVariance(d_sigmaRef);
  d_alignerMI.setTestVariance(d_sigmaTest);

  d_currentLevel = 0;
  d_stepsAtLevel = 0;

  d_alignerMI.setCovariance(d_sigmaRefRef, d_sigmaTestTest);

  return 0;
}

// we use this when the user sets the transformation or just after the
// current image resolution changes so that
// d_transformPixelUnits stays consistent 
void nmr_CoarseToFineSearch::convertTransformToPixelUnits(double *T1, 
                                                          double *T2) 
{
  // initialize transformation to the transform in image units
  int i;
  for (i = 0; i < 6; i++) {
    T2[i] = T1[i];
  }

  if (!d_refImages || !d_testImages) {
    return;
  }

  nmb_Image *ref = d_refImages[d_currentLevel];
  nmb_Image *test = d_testImages[d_currentLevel];
  // scale down from 0..ref.width, 0..ref.height to 0..1, 0..1
  double dref_width = ref->width();
  double dref_height = ref->height();
  T2[0] /= dref_width;
  T2[1] /= dref_height;
  T2[3] /= dref_width;
  T2[4] /= dref_height;

  // result is 0..1, 0..1 but we want 0..test.width, 0..test.height so scale
  // the result appropriately
  T2[0] *= test->width();
  T2[1] *= test->width();
  T2[2] *= test->width();
  T2[3] *= test->height();
  T2[4] *= test->height();
  T2[5] *= test->height();
 
  return;
}

// we use this when the user request the current transformation or
// just before changing the current image resolution so that
// d_transformImageUnits stays consistent with whatever we got from
// the nmr_AlignerMI
void nmr_CoarseToFineSearch::convertTransformToImageUnits(double *T1,
                                                          double *T2) 
{
  // initialize transformation to the transform in pixel units
  int i;
  for (i = 0; i < 6; i++) {
    T2[i] = T1[i];
  }

  if (!d_refImages || !d_testImages) {
    return;
  }

  nmb_Image *ref = d_refImages[d_currentLevel];
  nmb_Image *test = d_testImages[d_currentLevel];
  // scale up from 0..1, 0..1 to 0..ref.width, 0..ref.height
  double dref_width = ref->width();
  double dref_height = ref->height();
  T2[0] *= dref_width;
  T2[1] *= dref_height;
  T2[3] *= dref_width;
  T2[4] *= dref_height;

  // result is 0..test.width, 0..test.height but we want 0..1, 0..1 so scale
  // the result appropriately
  T2[0] /= test->width();
  T2[1] /= test->width();
  T2[2] /= test->width();
  T2[3] /= test->height();
  T2[4] /= test->height();
  T2[5] /= test->height();

  return;
}

void nmr_CoarseToFineSearch::finishSearchAtCurrentLevel()
{
  convertTransformToImageUnits(d_transformPixelUnits, d_transformImageUnits);
}

int nmr_CoarseToFineSearch::initializeSearchAtLevel(int level, double *T) {
  if (level < 0 || level > d_numResolutionLevels-1) {
    return -1;
  }

  int i;
  for (i = 0; i < 6; i++) {
    d_transformImageUnits[i] = T[i];
  }
  d_currentLevel = level;
  convertTransformToPixelUnits(d_transformImageUnits, d_transformPixelUnits);

  d_firstStepAtLevel = VRPN_TRUE;

  return 0;
}

void nmr_CoarseToFineSearch::setTransformation(double *T)
{
  int i;
  for (i = 0; i < 6; i++) {
    d_transformImageUnits[i] = T[i];
  }
  convertTransformToPixelUnits(d_transformImageUnits, d_transformPixelUnits);
}

void nmr_CoarseToFineSearch::getTransformation(double *T)
{
  int i;
  convertTransformToImageUnits(d_transformPixelUnits, d_transformImageUnits);
  for (i = 0; i < 6; i++) {
    T[i] = d_transformImageUnits[i];
  }
}

int nmr_CoarseToFineSearch::takeGradientStep()
{
  int i;
  // give nmr_AlignerMI the data it needs to compute mutual information-related
  // stuff
  
  // set current transformation from d_transformPixelUnits
  if (d_alignerMI.setTransformation2D(d_transformPixelUnits)) {
    printf("setTransformation2D error\n");
    return NMR_SEARCH_ERROR;
  }
  d_alignerMI.setRefVariance(d_sigmaRef);
  d_alignerMI.setTestVariance(d_sigmaTest);
  d_alignerMI.setCovariance(d_sigmaRefRef, d_sigmaTestTest);

  // take one random sample
  if (d_alignerMI.buildSampleA(d_refImages[d_currentLevel],
                               d_testImages[d_currentLevel],
                               d_testGradXImages[d_currentLevel],
                               d_testGradYImages[d_currentLevel],
                               d_takeRandomSamples,
                               d_minSqrTestGradMagnitude[d_currentLevel]) ||
  // take another random sample
      d_alignerMI.buildSampleB(d_refImages[d_currentLevel],
                               d_testImages[d_currentLevel],
                               d_testGradXImages[d_currentLevel],
                               d_testGradYImages[d_currentLevel],
                               d_takeRandomSamples,
                               d_minSqrTestGradMagnitude[d_currentLevel])) {
    printf("buildSample error\n");
    return NMR_SEARCH_ERROR;
  }

  double dIdT[6];
  double dHtest_dSigmaTest;
  double dHcross_dSigmaTestTest;
  double dHcross_dSigmaRefRef;

  // compute the derivative of MI with respect to d_transformPixelUnits
  if (d_alignerMI.computeTransformationGradient(dIdT)) {
    printf("computeTransformationGradient error\n");
    return NMR_SEARCH_ERROR;
  }
 
  // compute the derivative of test image entropy with respect to
  // d_sigmaTest
  if (d_alignerMI.computeTestEntropyGradient(dHtest_dSigmaTest)) {
    printf("computeTestEntropyGradient error\n");
    return NMR_SEARCH_ERROR;
  }

  // compute the derivative of cross-entropy with respect to 
  // (d_sigmaRefRef, d_sigmaTestTest)
  if (d_alignerMI.computeCrossEntropyGradient(dHcross_dSigmaRefRef,
                                              dHcross_dSigmaTestTest)) {
    printf("computeCrossEntropyGradient error\n");
    return NMR_SEARCH_ERROR;
  }

  /* increment d_transformPixelUnits, d_sigmaTest, 
     d_sigmaRefRef and d_sigmaTestTest
     note: here we are interleaving updates to the transformation
     and the Parzen window parameters one to one although at different rates
     but it might work better to update these with both different rates and
     different frequencies (I have no idea what is better so I'll try it
     this way first)
  */

  // maximize mutual information -> gradient ascent -> addition
  for (i = 0; i < 6; i++) {
    d_transformPixelUnits[i] += dIdT[i]*d_transformUpdateRate;
  }

  // minimize estimated entropy -> gradient descent -> subtraction
  d_sigmaTest -= dHtest_dSigmaTest*d_sigmaUpdateRate;
  d_sigmaTestTest -= dHcross_dSigmaTestTest*d_sigmaUpdateRate;
  d_sigmaRefRef -= dHcross_dSigmaRefRef*d_sigmaUpdateRate;

  // set the levelSwitchThreshold to 1% of the first derivative calculated
  // at this level
  if (d_firstStepAtLevel) {
    double maxDeriv = dIdT[0];
    for (i = 1; i < 6; i++) {
      if (dIdT[i] > maxDeriv) maxDeriv = dIdT[i];
    }
    d_levelSwitchThreshold = 0.01*maxDeriv;
    d_stepsAtLevel = 1;
    d_firstStepAtLevel = VRPN_FALSE;
  } else {
    d_stepsAtLevel++;
  }

  // if all components of the derivative of MI with respect to 
  // d_transformPixelUnits are below some threshold then we
  // switch to the next higher resolution
  vrpn_bool timeToSwitch = VRPN_TRUE;
  for (i = 0; i < 6; i++) {
    if (dIdT[i] > d_levelSwitchThreshold) {
      timeToSwitch = VRPN_FALSE;
      break;
    }
  }
  
  if (timeToSwitch || d_stepsAtLevel > 100) {
    if (d_currentLevel == d_numResolutionLevels-1) {
      printf("done with coarse to fine search\n");
      return NMR_SEARCH_FINISHED;
    } else {
      printf("done with level %d\n", d_currentLevel);
      finishSearchAtCurrentLevel();
      initializeSearchAtLevel(d_currentLevel+1, d_transformImageUnits);
      return NMR_SEARCH_LEVEL_FINISHED;
    }
  }
  return NMR_SEARCH_LEVEL_IN_PROGRESS;
}


void nmr_CoarseToFineSearch::setParameterValue(double value)
{

  int i;
  double factor;
  double ca = 0, sa = 0, angleChange = 0, currValue = 0;
  double newT[6];
  double shearChange = 0;
  double halfWidth = 0.5*(double)(d_testImages[d_currentLevel]->width());
  double halfHeight = 0.5*(double)(d_testImages[d_currentLevel]->height());
  double halfDiagonal = sqrt(halfWidth*halfWidth + halfHeight*halfHeight);
  double centerX = d_transformPixelUnits[2] - halfWidth;
  double centerY = d_transformPixelUnits[5] - halfHeight;
  double adjustedValue = 1.0;
  switch(d_direction) {
    case NMR_TRANSLATE_X:
      // units for value are pixels
/*
      if (fabs(d_transformPixelUnits[0]) > fabs(d_transformPixelUnits[3])) {
        factor = (value*d_transformPixelUnits[0] - d_transformPixelUnits[2])/
                 d_transformPixelUnits[0];
      } else {
        factor = (value*d_transformPixelUnits[3] - d_transformPixelUnits[5])/
                 d_transformPixelUnits[3];
      }
      d_transformPixelUnits[2] += factor*d_transformPixelUnits[0];
      d_transformPixelUnits[5] += factor*d_transformPixelUnits[3];
*/
      d_transformPixelUnits[2] = value;
      break;
    case NMR_TRANSLATE_Y:
      // units for value are pixels
/*
      if (fabs(d_transformPixelUnits[1]) > fabs(d_transformPixelUnits[4])) {
        factor = (value*d_transformPixelUnits[1] - d_transformPixelUnits[2])/
                 d_transformPixelUnits[1];
      } else {
        factor = (value*d_transformPixelUnits[4] - d_transformPixelUnits[5])/
                 d_transformPixelUnits[4];
      }
      d_transformPixelUnits[2] += factor*d_transformPixelUnits[1];
      d_transformPixelUnits[5] += factor*d_transformPixelUnits[4];
*/
      d_transformPixelUnits[5] = value;
      break;
    case NMR_SCALE_X:
      adjustedValue = value;
      factor = 1.0/(adjustedValue*
                    sqrt(d_transformPixelUnits[0]*d_transformPixelUnits[0]
                       + d_transformPixelUnits[1]*d_transformPixelUnits[1]));
/*
      if (fabs(d_transformPixelUnits[0]) > fabs(d_transformPixelUnits[1])) {
         factor = adjustedValue/d_transformPixelUnits[0];
      } else {
         factor = adjustedValue/d_transformPixelUnits[1];
      }
*/
      d_transformPixelUnits[0] *= factor;
      d_transformPixelUnits[1] *= factor;
      d_transformPixelUnits[2] = halfWidth + centerX*factor;
      getParameterValue();
      break;
    case NMR_SCALE_Y:
      adjustedValue = value;
      factor = 1.0/(adjustedValue*
                    sqrt(d_transformPixelUnits[3]*d_transformPixelUnits[3]
                       + d_transformPixelUnits[4]*d_transformPixelUnits[4]));
/*
      if (fabs(d_transformPixelUnits[3]) > fabs(d_transformPixelUnits[4])) {
         factor = adjustedValue/d_transformPixelUnits[3];
      } else {
         factor = adjustedValue/d_transformPixelUnits[4];
      }
*/
      d_transformPixelUnits[3] *= factor;
      d_transformPixelUnits[4] *= factor;
      d_transformPixelUnits[5] = halfHeight + centerY*factor;
      break;
    case NMR_ROTATE_Z:
      // determine what z-rotation we need to multiply by in order to make
      // the extracted parameter value equal to the specified one and then
      // apply it
      currValue = getParameterValue();

      angleChange = value - currValue;

      // we scale down value by halfDiagonal in order to preserve the
      // maximum displacement per change in value (this occurs at the corners
      // which are halfDiagonal away from the center of rotation)
      angleChange /= halfDiagonal;

      // now multiply by 
      //  cos(angleChange) sin(angleChange)
      // -sin(angleChange) cos(angleChange)
      ca = cos(angleChange);
      sa = sin(angleChange);
      newT[0] = ca*d_transformPixelUnits[0] + -sa*d_transformPixelUnits[3];
      newT[1] = ca*d_transformPixelUnits[1] + -sa*d_transformPixelUnits[4];
      newT[2] = ca*centerX + -sa*centerY + halfWidth;
      newT[3] = sa*d_transformPixelUnits[0] + ca*d_transformPixelUnits[3];
      newT[4] = sa*d_transformPixelUnits[1] + ca*d_transformPixelUnits[4];
      newT[5] = sa*centerX + ca*centerY + halfHeight;
      for (i = 0; i < 6; i++)
        d_transformPixelUnits[i] = newT[i];
      break;
    case NMR_SHEAR:
      // determine what shear we need to multiply by 
      // shearing matrix is of the form [1 s 0]
      //                                [0 1 0]
      // to make the extracted parameter equal to the specified one and then
      // apply it
      currValue = getParameterValue();
      // we scale down value by halfHeight in order to preserve the
      // maximum displacement per change in value (this occurs at the top
      // and bottom of the image which are halfHeight away from the 
      // center of shearing)
      shearChange = (value - currValue)/halfHeight;
      newT[0] = d_transformPixelUnits[0] + shearChange*d_transformPixelUnits[3];
      newT[1] = d_transformPixelUnits[1] + shearChange*d_transformPixelUnits[4];
      newT[2] = d_transformPixelUnits[2] + shearChange*(centerY);
      newT[3] = d_transformPixelUnits[3];
      newT[4] = d_transformPixelUnits[4];
      newT[5] = d_transformPixelUnits[5];
      for (i = 0; i < 6; i++)
        d_transformPixelUnits[i] = newT[i];
      break;
    default:
      printf("setParameterValue: Error, unhandled direction\n");
  }
}

double nmr_CoarseToFineSearch::getParameterValue(vrpn_bool resInd)
{
  double result = 0;
  double halfWidth = 0.5*(double)(d_testImages[d_currentLevel]->width());
  double halfHeight = 0.5*(double)(d_testImages[d_currentLevel]->height());
  double halfDiagonal = sqrt(halfWidth*halfWidth + halfHeight*halfHeight);

  if (resInd) {
    convertTransformToImageUnits(d_transformPixelUnits, d_transformImageUnits);
    double *xform = d_transformImageUnits;

    switch(d_direction) {
     case NMR_TRANSLATE_X:
/*
      if (fabs(xform[0]) > fabs(xform[3])) {
         result = xform[2]/xform[0];
      } else {
         result = xform[5]/xform[3];
      }
*/
      result = xform[2];
      break;
     case NMR_TRANSLATE_Y:
/*
      if (fabs(xform[1]) > fabs(xform[4])) {
         result = xform[2]/xform[1];
      } else {
         result = xform[5]/xform[4];
      }
*/
      result = xform[5];
      break;
     case NMR_SCALE_X:
      result = 1.0/sqrt(xform[0]*xform[0] + xform[1]*xform[1]);
/*
      if (fabs(xform[0]) > fabs(xform[1])) {
         result = xform[0];
      } else {
         result = xform[1];
      }
*/
      break;
     case NMR_SCALE_Y:
      result = 1.0/sqrt(xform[3]*xform[3] + xform[4]*xform[4]);
/*
      if (fabs(xform[3]) > fabs(xform[4])) {
         result = xform[3];
      } else {
         result = xform[4];
      }
*/
      break;
     case NMR_ROTATE_Z:
      double x_rot, y_rot, tan_angle, angle;
      // how do you extract one angle from 4 matrix entries
      // we do this by transforming the vector (1, 0) and seeing what
      // we get out - this defines our meaning of rotation
      x_rot = xform[0];
      y_rot = xform[3];
      tan_angle = y_rot/x_rot;
      angle = atan(tan_angle);
      if (x_rot < 0) {
        angle += M_PI;
      }
      result = angle;
      break;
     case NMR_SHEAR:
      if (fabs(xform[3]) > fabs(xform[4])) {
         result = xform[0]/xform[3];
      } else {
         result = xform[1]/xform[4];
      }
      break;
     default:
      printf("getParameterValue: Error, unhandled direction\n");
   }
  } else {

   switch(d_direction) {
    case NMR_TRANSLATE_X:
/*
      if (fabs(d_transformPixelUnits[0]) > fabs(d_transformPixelUnits[3])) {
         result = d_transformPixelUnits[2]/d_transformPixelUnits[0];
      } else {
         result = d_transformPixelUnits[5]/d_transformPixelUnits[3];
      }
*/
      result = d_transformPixelUnits[2];
      break;
    case NMR_TRANSLATE_Y:
/*
      if (fabs(d_transformPixelUnits[1]) > fabs(d_transformPixelUnits[4])) {
         result = d_transformPixelUnits[2]/d_transformPixelUnits[1];
      } else {
         result = d_transformPixelUnits[5]/d_transformPixelUnits[4];
      }
*/
      result = d_transformPixelUnits[5];
      break;
    case NMR_SCALE_X:
/*
      if (fabs(d_transformPixelUnits[0]) > fabs(d_transformPixelUnits[1])) {
         result = d_transformPixelUnits[0];
      } else {
         result = d_transformPixelUnits[1];
      }
*/
      result = 1.0/sqrt(d_transformPixelUnits[0]*d_transformPixelUnits[0] +
                        d_transformPixelUnits[1]*d_transformPixelUnits[1]);
      break;
    case NMR_SCALE_Y:
/*
      if (fabs(d_transformPixelUnits[3]) > fabs(d_transformPixelUnits[4])) {
         result = d_transformPixelUnits[3];
      } else {
         result = d_transformPixelUnits[4];
      }
*/
      result = 1.0/sqrt(d_transformPixelUnits[3]*d_transformPixelUnits[3] +
                        d_transformPixelUnits[4]*d_transformPixelUnits[4]);

      break;
    case NMR_ROTATE_Z:
      double x_rot, y_rot, tan_angle, angle;
      // how do you extract one angle from 4 matrix entries
      // we do this by transforming the vector (1, 0) and seeing what
      // we get out - this defines our meaning of rotation
      x_rot = d_transformPixelUnits[0];
      y_rot = d_transformPixelUnits[3];
      tan_angle = y_rot/x_rot;
      angle = atan(tan_angle);
      if (x_rot < 0) {
        angle += M_PI;
      }
      result = angle;
      // we need to scale this up by half the diagonal in order to
      // be consistent with setParameterValue - see comment there
      result *= halfDiagonal;
      break;
    case NMR_SHEAR:
      if (fabs(d_transformPixelUnits[3]) > fabs(d_transformPixelUnits[4])) {
         result = d_transformPixelUnits[0]/d_transformPixelUnits[3];
      } else {
         result = d_transformPixelUnits[1]/d_transformPixelUnits[4];
      }

      // we need to scale this up by half the height in order to
      // be consistent with setParameterValue - see comment there
      result *= halfHeight;
      break;
    default:
      printf("getParameterValue: Error, unhandled direction\n");
   }
  }
  return result;
}

vrpn_bool nmr_CoarseToFineSearch::directionValid(int direction)
{
  switch (direction) {
    case NMR_TRANSLATE_X:
    case NMR_TRANSLATE_Y:
    case NMR_SCALE_X:
    case NMR_SCALE_Y:
    case NMR_ROTATE_Z:
    case NMR_SHEAR:
      return vrpn_TRUE;
    default:
      return vrpn_FALSE;
  }
}

int nmr_CoarseToFineSearch::optimizeInDirection(int direction,
                                                double &change,
                                                int numStepsUp,
                                                int numStepsDown,
                                                double stepSizeFactor)
{
  if (!directionValid(direction)) return -1;
  double startParameterValue;
  vrpn_bool geometricSequence = VRPN_FALSE;

  d_direction = direction;
  startParameterValue = getParameterValue();

  int numValues = numStepsUp + numStepsDown + 1;
  double *paramValues = new double[numValues];
  double *objValues = new double[numValues];

  double avgObjValue = 0;
  int i;

  double stepSize;

  switch(direction) {
    case NMR_TRANSLATE_X:
    case NMR_TRANSLATE_Y:
      if (direction == NMR_TRANSLATE_X) {
        printf("translate_x:\n");
      } else {
        printf("translate_y:\n");
      }
      stepSize = 0.05*stepSizeFactor; // 0.2 pixels
      break;
    case NMR_SCALE_X:
    case NMR_SCALE_Y:
      if (direction == NMR_SCALE_X) {
        printf("scale_x:\n");
      } else {
        printf("scale_y:\n");
      }
      geometricSequence = VRPN_TRUE;
      stepSize = 1.03*stepSizeFactor;
      break;
    case NMR_ROTATE_Z:
      printf("rotate_z:\n");
      stepSize = stepSizeFactor*2.0*M_PI/(360.0);
      break;
    case NMR_SHEAR:
      printf("shear:\n");
      stepSize = 0.02*stepSizeFactor;
      break;
    default:
      stepSize = 1.0;
      printf("optimizeInDirection: Error, this shouldn't happen\n");
      break;
  }

  paramValues[0] = startParameterValue;
  if (geometricSequence) {
    // create geometric sequence
    for (i = 0; i < numStepsDown; i++)
      paramValues[0] /= stepSize;
    for (i = 1; i < numValues; i++) {
      paramValues[i] = paramValues[i-1]*stepSize;
    }
  } else {
    // create arithmetic sequence
    paramValues[0] = startParameterValue - numStepsDown*stepSize;
    for (i = 1; i < numValues; i++) {
      paramValues[i] = paramValues[i-1] + stepSize;
    }
  }

  for (i = 0; i < numValues; i++) {
    setParameterValue(paramValues[i]);
    d_alignerMI.setTransformation2D(d_transformPixelUnits);
    d_alignerMI.buildSampleA(d_refImages[d_currentLevel],
                             d_testImages[d_currentLevel],
                             d_testGradXImages[d_currentLevel],
                             d_testGradYImages[d_currentLevel],
                             d_takeRandomSamples,
                             d_minSqrTestGradMagnitude[d_currentLevel]);
    d_alignerMI.buildSampleB(d_refImages[d_currentLevel],
                             d_testImages[d_currentLevel],
                             d_testGradXImages[d_currentLevel],
                             d_testGradYImages[d_currentLevel],
                             d_takeRandomSamples,
                             d_minSqrTestGradMagnitude[d_currentLevel]);
    d_alignerMI.computeMutualInformation(objValues[i]);
    avgObjValue += objValues[i];
    printf("param=%g, MI=%g\n",
          paramValues[i], objValues[i]);
  }
  avgObjValue /= (double)numValues;

  // only use above-average values because background value for objective is
  // expected to be flat and we want to find the center of mass of
  // those that are near the peak
  double weightSum = 1.0;
  double weightedValueSum = startParameterValue;
  for (i = 0; i < numValues; i++) {
    if (objValues[i] > avgObjValue) {
       weightSum += (objValues[i] - avgObjValue);
       weightedValueSum += (objValues[i] - avgObjValue)*paramValues[i];
    }
  }

  double estimatedOptimumValue = weightedValueSum/weightSum;
  
  setParameterValue(estimatedOptimumValue);
  printf("estimated max at %g\n", estimatedOptimumValue);

  convertTransformToImageUnits(d_transformPixelUnits, d_transformImageUnits);

  // compute change in number of steps from where we started
  if (geometricSequence) {
    change = log(estimatedOptimumValue/startParameterValue)/
             log(stepSize);
  } else {
    change = (estimatedOptimumValue - startParameterValue)/stepSize;
  }


  delete [] paramValues;
  delete [] objValues;
  return 0;
}

// for analysis of the objective
void nmr_CoarseToFineSearch::plotObjective(int direction, 
                                          int numValues, FILE *fout)
{
  if (fout == NULL) return;
  int i,j;
  d_direction = direction;
  double stepSize;
  double stepSizeFactor = 2.0;
  double *paramValues = new double[numValues];
  double *paramValuesToPrint = new double[numValues];
  vrpn_bool geometricSequence = VRPN_FALSE;
  double startParameterValue = getParameterValue();

  switch(direction) {
    case NMR_TRANSLATE_X:
    case NMR_TRANSLATE_Y:
      if (direction == NMR_TRANSLATE_X) {
        fprintf(fout, "translate_x:\n");
        printf("translate_x:\n");
      } else {
        fprintf(fout, "translate_y:\n");
        printf("translate_y:\n");
      }
      stepSize = stepSizeFactor*0.05; // 0.2 pixels
      break;
    case NMR_SCALE_X:
    case NMR_SCALE_Y:
      if (direction == NMR_SCALE_X) {
        fprintf(fout, "scale_x:\n");
        printf("scale_x:\n");
      } else {
        fprintf(fout, "scale_y:\n");
        printf("scale_y:\n");
      }
      stepSize = stepSizeFactor*0.005;
      break;
    case NMR_ROTATE_Z:
      fprintf(fout, "rotate_z:\n");
      printf("rotate_z:\n");
      stepSize = stepSizeFactor*2.0*M_PI/(36.0);
      break;
    case NMR_SHEAR:
      fprintf(fout, "shear:\n");
      printf("shear:\n");
      stepSize = 0.2*stepSizeFactor;
      break;
    default:
      stepSize = 1.0;
      printf("optimizeInDirection: Error, this shouldn't happen\n");
      break;
  }

  double mutInfo;
  double dIdT[6];
  double dTdP[6];
  double dIdP;
  double T1[6], T2[6];

  for (d_currentLevel = 0; d_currentLevel < d_numResolutionLevels; 
                           d_currentLevel++) {
    convertTransformToPixelUnits(d_transformImageUnits, d_transformPixelUnits);
    startParameterValue = getParameterValue();

    paramValues[0] = startParameterValue;

    if (geometricSequence) {
      // create geometric sequence
      for (i = 0; i < numValues/2; i++)
        paramValues[0] /= stepSize;
      for (i = 1; i < numValues; i++) {
        paramValues[i] = paramValues[i-1]*stepSize;
      }
    } else {
      // create arithmetic sequence
      paramValues[0] = startParameterValue - floor(0.5*numValues)*stepSize;
      for (i = 1; i < numValues; i++) {
        paramValues[i] = paramValues[i-1] + stepSize;
      }
    }

    fprintf(fout, "level %d, ref:(%d,%d), test:(%d,%d)\n",
      d_currentLevel, 
      d_refImages[d_currentLevel]->width(), 
      d_refImages[d_currentLevel]->height(),
      d_testImages[d_currentLevel]->width(),
      d_testImages[d_currentLevel]->height());
    printf("level %d, ref:(%d,%d), test:(%d,%d)\n",
      d_currentLevel,
      d_refImages[d_currentLevel]->width(),
      d_refImages[d_currentLevel]->height(),
      d_testImages[d_currentLevel]->width(),
      d_testImages[d_currentLevel]->height());

    fprintf(fout, "value\tI\tdI_dvalue\n");
    printf("value\tI\tdI_dvalue\n");
    for (i = 0; i < numValues; i++) {
      setParameterValue(paramValues[i]);
      paramValuesToPrint[i] = getParameterValue(VRPN_TRUE);
      d_alignerMI.setTransformation2D(d_transformPixelUnits);
      d_alignerMI.buildSampleA(d_refImages[d_currentLevel],
                               d_testImages[d_currentLevel],
                               d_testGradXImages[d_currentLevel],
                               d_testGradYImages[d_currentLevel],
                               d_takeRandomSamples,
                               d_minSqrTestGradMagnitude[d_currentLevel]);
      d_alignerMI.buildSampleB(d_refImages[d_currentLevel],
                               d_testImages[d_currentLevel],
                               d_testGradXImages[d_currentLevel],
                               d_testGradYImages[d_currentLevel],
                               d_takeRandomSamples,
                               d_minSqrTestGradMagnitude[d_currentLevel]);
      d_alignerMI.computeMutualInformation(mutInfo);
      d_alignerMI.computeTransformationGradient(dIdT);
      setParameterValue(paramValues[i]-0.0005);
      getTransformation(T1);
      setParameterValue(paramValues[i]+0.0005);
      getTransformation(T2);
      dIdP = 0.0;
      for (j = 0; j < 6; j++) {
        dTdP[j] = 1000.0*(T2[j] - T1[j]);
        dIdP += dTdP[j]*dIdT[j];
      }
#ifdef DEBUG_FILE_OUTPUT
      fprintf(fout, "%g\t%g\t%g"
                    "\t%g\t%g\t%g\t%g\t%g\t%g"
                    "\t%g\t%g\t%g\t%g\t%g\t%g",
                    paramValuesToPrint[i], mutInfo, dIdP,
                    dIdT[0], dIdT[1], dIdT[2], dIdT[3], dIdT[4], dIdT[5],
                    dTdP[0], dTdP[1], dTdP[2], dTdP[3], dTdP[4], dTdP[5]);

      fprintf(fout, "\t%g\t%g\t%g\t%g\t%g\t%g\n",
                    d_transformPixelUnits[0], d_transformPixelUnits[1],
                    d_transformPixelUnits[2], d_transformPixelUnits[3],
                    d_transformPixelUnits[4], d_transformPixelUnits[5]);
#else
      fprintf(fout, "%g\t%g\t%g\n", paramValuesToPrint[i], mutInfo, dIdP);
#endif
      printf("%g\t%g\t%g\n", paramValuesToPrint[i], mutInfo, dIdP);
    }

    setParameterValue(startParameterValue);
    convertTransformToImageUnits(d_transformPixelUnits, d_transformImageUnits);
  }
  d_currentLevel = 0;
  delete [] paramValues;
  delete [] paramValuesToPrint;
}

int nmr_CoarseToFineSearch::getJointHistogram(int level,
                           nmb_Image *histogram, vrpn_bool blur,
                           vrpn_bool setRefScale,
                           float min_ref, float max_ref,
                           vrpn_bool setTestScale,
                           float min_test, float max_test)
{

  if (level < 0 || level >= d_numResolutionLevels) return -1;

  convertTransformToImageUnits(d_transformPixelUnits, d_transformImageUnits);

  int currLevelSave = d_currentLevel;
  d_currentLevel = level;

  convertTransformToPixelUnits(d_transformImageUnits, d_transformPixelUnits);
  d_alignerMI.setTransformation2D(d_transformPixelUnits);
  d_alignerMI.buildJointHistogram(d_refImages[d_currentLevel],
                                  d_testImages[d_currentLevel],
                                  histogram, blur,
                                  setRefScale,
                                  min_ref, max_ref,
                                  setTestScale,
                                  min_test, max_test);

  d_currentLevel = currLevelSave;
  return 0;
}
