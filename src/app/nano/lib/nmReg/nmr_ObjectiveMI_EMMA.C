#include "nmr_ObjectiveMI_EMMA.h"
#include "nmr_Gaussian.h"
#include "nmr_Util.h"

// #define DEBUG_FILE_OUTPUT
#define PARZEN_VARIANCE_ESTIMATION_FACTOR (0.10)

nmr_ObjectiveMI_EMMA::nmr_ObjectiveMI_EMMA():
  d_workspace(NULL),
  d_sampleA_acquired(VRPN_FALSE),
  d_sizeA(0),
  d_Ax_ref(NULL), d_Ay_ref(NULL), d_Az_ref(NULL), 
  d_Ax_test(NULL), d_Ay_test(NULL),
  d_refValuesA(NULL), d_testValuesA(NULL),
  d_dTestValueA_dx(NULL), d_dTestValueA_dy(NULL),
  d_sampleB_acquired(VRPN_FALSE),
  d_sizeB(0),
  d_Bx_ref(NULL), d_By_ref(NULL), d_Bz_ref(NULL),
  d_Bx_test(NULL), d_By_test(NULL),
  d_refValuesB(NULL), d_testValuesB(NULL),
  d_dTestValueB_dx(NULL), d_dTestValueB_dy(NULL),
  d_sigmaRefRef(1.0), d_sigmaTestTest(1.0),
  d_sigmaTest(1.0), d_sigmaRef(1.0),
  d_dimensionMode(REF_2D),
  d_samplePositionMode(NMR_JITTERED),
  d_sampleRejectionCriterion(NMR_NO_SELECT),
  d_minSampleSqrGradientMagnitude(0.0),
  d_numRefFeaturePoints(0),
  d_xFeature(NULL),
  d_yFeature(NULL),
  d_maxDistanceToFeaturePoint(10),
  d_testValue(NULL),
  d_gradX_test(NULL), d_gradY_test(NULL),
  d_refValue(NULL),
  d_refZ(NULL)
{
}

nmr_ObjectiveMI_EMMA::~nmr_ObjectiveMI_EMMA()
{
  if (d_workspace) {
    delete [] d_workspace;
  }
  vrpn_bool deleteFailed = vrpn_FALSE;
  if (d_testValue) {
    deleteFailed = deleteFailed && nmb_Image::deleteImage(d_testValue);
  }
  if (d_gradX_test) {
    deleteFailed = deleteFailed && nmb_Image::deleteImage(d_gradX_test);
  }
  if (d_gradY_test) {
    deleteFailed = deleteFailed && nmb_Image::deleteImage(d_gradY_test);
  }
  if (d_refValue) {
    deleteFailed = deleteFailed && nmb_Image::deleteImage(d_refValue);
  }
  if (d_refZ) {
    deleteFailed = deleteFailed && nmb_Image::deleteImage(d_refZ);
  }
  if (deleteFailed) {
    fprintf(stderr, "nmr_ObjectiveMI_EMMA: Warning: possible memory leak\n");
  }
  if (d_xFeature) delete [] d_xFeature;
  if (d_yFeature) delete [] d_yFeature;
}

void nmr_ObjectiveMI_EMMA::setDimensionMode(nmr_DimensionMode mode)
{
  d_dimensionMode = mode;
}

nmr_DimensionMode nmr_ObjectiveMI_EMMA::getDimensionMode()
{
  return d_dimensionMode;
}

int nmr_ObjectiveMI_EMMA::setSampleSizes(int sizeA, int sizeB)
{
  if (d_workspace) {
    delete [] d_workspace;
  }

  // allocate all our big arrays at once
  d_workspace = new double[9*(sizeA+sizeB)];

  if (d_workspace == NULL) {
    fprintf(stderr, "nmr_ObjectiveMI_EMMA::setSampleSizes: out of memory\n");
    return -1;
  }

  // assign pointers into this array
  d_Ax_ref = d_workspace;
  d_Ay_ref = d_Ax_ref + sizeA;
  d_Az_ref = d_Ay_ref + sizeA;
  d_Ax_test = d_Az_ref + sizeA;
  d_Ay_test = d_Ax_test + sizeA;
  d_refValuesA = d_Ay_test + sizeA;
  d_testValuesA = d_refValuesA + sizeA;
  d_dTestValueA_dx = d_testValuesA + sizeA;
  d_dTestValueA_dy = d_dTestValueA_dx + sizeA; 

  d_Bx_ref = d_dTestValueA_dy + sizeA;
  d_By_ref = d_Bx_ref + sizeB;
  d_Bz_ref = d_By_ref + sizeB;
  d_Bx_test = d_Bz_ref + sizeB;
  d_By_test = d_Bx_test + sizeB;
  d_refValuesB = d_By_test + sizeB;
  d_testValuesB = d_refValuesB + sizeB;
  d_dTestValueB_dx = d_testValuesB + sizeB;
  d_dTestValueB_dy = d_dTestValueB_dx + sizeB;

  d_sizeA = sizeA;
  d_sizeB = sizeB;

  return 0;
}

void nmr_ObjectiveMI_EMMA::getSampleSizes(int &sizeA, int &sizeB)
{
  sizeA = d_sizeA;
  sizeB = d_sizeB;
}

void nmr_ObjectiveMI_EMMA::setSamplePositionMode(nmr_SamplePositionMode mode)
{
  d_samplePositionMode = mode;
}

void nmr_ObjectiveMI_EMMA::setSampleRejectionCriterion(
                         nmr_SampleRejectionCriterion crit)
{
  d_sampleRejectionCriterion = crit;
}

void nmr_ObjectiveMI_EMMA::setMinSampleSqrGradientMagnitude(double mag)
{
  d_minSampleSqrGradientMagnitude = mag;
}

void nmr_ObjectiveMI_EMMA::setRefFeaturePoints(int numPnts, double *x, double *y)
{
  int i;
  d_numRefFeaturePoints = numPnts;
  if (d_xFeature) delete [] d_xFeature;
  if (d_yFeature) delete [] d_yFeature;
  d_xFeature = new double[numPnts];
  d_yFeature = new double[numPnts];
  for (i = 0; i < numPnts; i++) {
    d_xFeature[i] = x[i];
    d_yFeature[i] = y[i];
  }
}

void nmr_ObjectiveMI_EMMA::setCovariance(double sigmaRefRef, double sigmaTestTest)
{
  d_sigmaRefRef = sigmaRefRef;
  d_sigmaTestTest = sigmaTestTest;
}

void nmr_ObjectiveMI_EMMA::getCovariance(double &sigmaRefRef, double &sigmaTestTest)
{
  sigmaRefRef = d_sigmaRefRef;
  sigmaTestTest = d_sigmaTestTest;
}

void nmr_ObjectiveMI_EMMA::setTestVariance(double sigma)
{
  d_sigmaTest = sigma;
}

void nmr_ObjectiveMI_EMMA::getTestVariance(double &sigma)
{
  sigma = d_sigmaTest;
}

void nmr_ObjectiveMI_EMMA::setRefVariance(double sigma)
{
  d_sigmaRef = sigma;
}

void nmr_ObjectiveMI_EMMA::getRefVariance(double &sigma)
{
  sigma = d_sigmaRef;
}

void nmr_ObjectiveMI_EMMA::setReferenceValueImage(nmb_Image *ref) 
{
  d_refValue = ref;
  d_refValue->normalize();
  if (d_refValue) {
    // make a reasonable guess at what the initial variance should be
    // for the Parzen windowing
    double mean = nmr_Util::computeMean(*d_refValue);
    double variance = nmr_Util::computeVariance(*d_refValue, mean);
    double sigma = PARZEN_VARIANCE_ESTIMATION_FACTOR*sqrt(variance);
    d_sigmaRef = sigma;
    d_sigmaRefRef = sigma;
  }
}

void nmr_ObjectiveMI_EMMA::setTestValueImage(nmb_Image *test)
{
  if (d_gradX_test) {
    nmb_Image::deleteImage(d_gradX_test);
    d_gradX_test = NULL;
  } 
  if (d_gradY_test) {
    nmb_Image::deleteImage(d_gradY_test);
    d_gradY_test = NULL;
  }
  d_testValue = test;
  d_testValue->normalize();
  if (d_testValue) {
    // make a reasonable guess at what the initial variance should be
    // for the Parzen windowing
    double mean = nmr_Util::computeMean(*d_testValue);
    double variance = nmr_Util::computeVariance(*d_testValue, mean);
    double sigma = PARZEN_VARIANCE_ESTIMATION_FACTOR*sqrt(variance);
    d_sigmaTest = sigma;
    d_sigmaTestTest = sigma;

    string testImageName, testUnitsName;
    testImageName = *d_testValue->name();
    testUnitsName = *d_testValue->unitsValue();
    int test_width = d_testValue->width();
    int test_height = d_testValue->height();
    string gradXTestImageName, gradXTestUnitsName;
    string gradYTestImageName, gradYTestUnitsName;
    gradXTestImageName = testImageName + "_gradx";
    gradYTestImageName = testImageName + "_grady";
    gradXTestUnitsName = "d" + testUnitsName + "_dx";
    gradYTestUnitsName = "d" + testUnitsName + "_dy";
    d_gradX_test = new nmb_ImageGrid(gradXTestImageName.c_str(), gradXTestUnitsName.c_str(),
                                     test_width, test_height);
    d_gradY_test = new nmb_ImageGrid(gradYTestImageName.c_str(), gradYTestUnitsName.c_str(),
                                     test_width, test_height);
    nmr_Util::createGradientImages(*d_testValue, *d_gradX_test, *d_gradY_test);
    int i,j;
    double gradMagSqrSum = 0;
    double gradX, gradY;
    for (i = 0; i < test_width; i++) {
      for (j = 0; j < test_height; j++) {
        gradX = d_gradX_test->getValue(i,j);
        gradY = d_gradY_test->getValue(i,j);
        gradMagSqrSum += gradX*gradX + gradY*gradY;
      }
    }
    d_minSampleSqrGradientMagnitude = 
                        gradMagSqrSum/(double)(test_width*test_height);
  }
}

void nmr_ObjectiveMI_EMMA::setReferenceZImage(nmb_Image *ref_z)
{
  d_refZ = ref_z;
}

double nmr_ObjectiveMI_EMMA::value(double *testFromReferenceTransform)
{
  double transformPixelUnits[16];
  
  convertTransformToPixelUnits(testFromReferenceTransform,
                               transformPixelUnits);
  int error = 0;
  error = buildSampleA(d_refValue, d_testValue, d_gradX_test, d_gradY_test,
               transformPixelUnits, d_samplePositionMode, 
               d_sampleRejectionCriterion,
               d_minSampleSqrGradientMagnitude,
               d_refZ);
  if (error) {
    fprintf(stderr, "nmr_ObjectiveMI_EMMA::value: error: cannot acquire sample A\n");
  }
  error = buildSampleB(d_refValue, d_testValue, d_gradX_test, d_gradY_test,
               transformPixelUnits, d_samplePositionMode, 
               d_sampleRejectionCriterion,
               d_minSampleSqrGradientMagnitude,
               d_refZ);
  if (error) {
    fprintf(stderr, "nmr_ObjectiveMI_EMMA::value: error: cannot acquire sample B\n");
  }
  double result = 0;
  if (!error) {
    computeMutualInformation(result);
  }
  return result;
}

void nmr_ObjectiveMI_EMMA::gradient(double *testFromReferenceTransform, 
                               double *gradMI)
{
  double transformPixelUnits[16];

  convertTransformToPixelUnits(testFromReferenceTransform,
                               transformPixelUnits);
  int error = 0;
  error = buildSampleA(d_refValue, d_testValue, d_gradX_test, d_gradY_test,
               transformPixelUnits, d_samplePositionMode,
               d_sampleRejectionCriterion,
               d_minSampleSqrGradientMagnitude, d_refZ);
  if (error) {
    fprintf(stderr, "nmr_ObjectiveMI_EMMA::gradient:"
                    " error: cannot acquire sample A\n");
  }
  error = buildSampleB(d_refValue, d_testValue, d_gradX_test, d_gradY_test,
               transformPixelUnits, d_samplePositionMode, 
               d_sampleRejectionCriterion,
               d_minSampleSqrGradientMagnitude, d_refZ);
  if (error) {
    fprintf(stderr, "nmr_ObjectiveMI_EMMA::gradient:" 
                    " error: cannot acquire sample B\n");
  }
  // we need to convert between formats for the transformation matrices
  // old format (used by computeTransformationGradient) and the new
  // format (assumed by this function)
  // [m[0] m[1] m[2] m[3]]        [m[0] m[4] m[8]  m[12]]
  // [m[4] m[5] m[6] m[7]]   -->  [m[1] m[5] m[9]  m[13]]
  // [ 0    0    1    0  ]        [m[2] m[6] m[10] m[14]]
  // [ 0    0    0    1  ]        [m[3] m[7] m[11] m[15]]

  double tmp[8] = {0,0,0,0,0,0,0,0};
  if (!error) {
    computeTransformationGradient(tmp);
  }
  gradMI[0]=tmp[0]; gradMI[4]=tmp[1]; gradMI[8]=tmp[2]; gradMI[12]=tmp[3];
  gradMI[1]=tmp[4]; gradMI[5]=tmp[5]; gradMI[9]=tmp[6]; gradMI[13]=tmp[7];
  gradMI[2] = 0;    gradMI[6] = 0;    gradMI[10] = 1.0; gradMI[14] = 0;
  gradMI[3] = 0;    gradMI[7] = 0;    gradMI[11] = 0;   gradMI[15] = 1.0;
  convertTransformToImageUnits(gradMI, gradMI);
}


void nmr_ObjectiveMI_EMMA::valueAndGradient(double *testFromReferenceTransform,
                double &valueMI, double *gradMI)
{
  double transformPixelUnits[16];

  convertTransformToPixelUnits(testFromReferenceTransform,
                               transformPixelUnits);
  int error = 0;
  buildSampleA(d_refValue, d_testValue, d_gradX_test, d_gradY_test,
               transformPixelUnits, d_samplePositionMode, 
               d_sampleRejectionCriterion,
               d_minSampleSqrGradientMagnitude, d_refZ);
  if (error) {
    fprintf(stderr, "nmr_ObjectiveMI_EMMA::gradient:" 
                    " error: cannot acquire sample A\n");
  }

  buildSampleB(d_refValue, d_testValue, d_gradX_test, d_gradY_test,
               transformPixelUnits, d_samplePositionMode, 
               d_sampleRejectionCriterion,
               d_minSampleSqrGradientMagnitude, d_refZ);
  if (error) {
    fprintf(stderr, "nmr_ObjectiveMI_EMMA::gradient:"
                    " error: cannot acquire sample B\n");
  }

  // we need to convert between formats for the transformation matrices
  // old format (used by computeTransformationGradient) and the new
  // format (assumed by this function)
  // [m[0] m[1] m[2] m[3]]        [m[0] m[4] m[8]  m[12]]
  // [m[4] m[5] m[6] m[7]]   -->  [m[1] m[5] m[9]  m[13]]
  // [ 0    0    1    0  ]        [m[2] m[6] m[10] m[14]]
  // [ 0    0    0    1  ]        [m[3] m[7] m[11] m[15]]

  double tmp[8] = {0,0,0,0,0,0,0,0};
  valueMI = 0.0;
  if (!error) {
    computeTransformationGradient(tmp);
    computeMutualInformation(valueMI);
  }

  gradMI[0]=tmp[0]; gradMI[4]=tmp[1]; gradMI[8]=tmp[2]; gradMI[12]=tmp[3];
  gradMI[1]=tmp[4]; gradMI[5]=tmp[5]; gradMI[9]=tmp[6]; gradMI[13]=tmp[7];
  gradMI[2] = 0;    gradMI[6] = 0;    gradMI[10] = 1.0; gradMI[14] = 0;
  gradMI[3] = 0;    gradMI[7] = 0;    gradMI[11] = 0;   gradMI[15] = 1.0;
  convertTransformToImageUnits(gradMI, gradMI);

}

int nmr_ObjectiveMI_EMMA::buildSampleSets(double *user_transform)
{
  if (!d_refValue || !d_testValue) return -1;
  double default_transform[16] = {1.0, 0.0, 0.0, 0.0,
                                  0.0, 1.0, 0.0, 0.0,
                                  0.0, 0.0, 1.0, 0.0,
                                  0.0, 0.0, 0.0, 1.0};
  double transform[16];
  if (!user_transform) {
    convertTransformToPixelUnits(default_transform, transform);
  } else {
    convertTransformToPixelUnits(user_transform, transform);
  }
  int error = 0;
  error = buildSampleA(d_refValue, d_testValue, d_gradX_test, d_gradY_test,
               transform, d_samplePositionMode,
               d_sampleRejectionCriterion,
               d_minSampleSqrGradientMagnitude, d_refZ);
  if (error) {
    return -1;
  }
  error = buildSampleB(d_refValue, d_testValue, d_gradX_test, d_gradY_test,
               transform, d_samplePositionMode,
               d_sampleRejectionCriterion,
               d_minSampleSqrGradientMagnitude, d_refZ);
  if (error) {
    return -1;
  }
  return 0;
}

/* buildSampleHelper: builds a set of points and their corresponding 
   values from the ref, test, grad_x_test, and grad_y_test images 
   o ref, test, grad_x_test, grad_y_test, ref_z: input images
   o sampleMode: if NMR_RANDOM then points are selected randomly
	        otherwise, points are pixel centers in ref selected in
                row major order (y = slow direction) and this order is
                repeated if necessary to get the specified number of points
   o numPoints: number of points to collect (size of sample)
   o x_ref, y_ref, z_ref, x_test, y_test, val_ref, val_test, dtest_dx,
     dtest_dy: arrays for output - should be allocated with numPoints elements
   o minSqrGradientMagnitude: provides a test to reject points from being
     included in the sample - if the square of the gradient magnitude in the
     test image at a point is less than this value then that point will be
     rejected; setting this to 0 causes all points to be acceptable; this may
     be used to extract samples more from those parts of the image where
     there is more useful information

   If a point is selected which falls outside of the test image (the
   location in the test image is determined by the current transformation
   setting) then the test value and its gradient are assumed to be 0
*/
int nmr_ObjectiveMI_EMMA::buildSampleHelper(nmb_Image *ref, nmb_Image *test,
                     nmb_Image *grad_x_test,
                     nmb_Image *grad_y_test,
                     double *transform,
                     nmr_SamplePositionMode samplePositionMode,
                     nmr_SampleRejectionCriterion sampleRejectionCriterion,
                     nmb_Image *ref_z,
       int numPoints, double *x_ref, double *y_ref, double *z_ref,
       double *x_test, double *y_test, double *val_ref, double *val_test,
       double *dtest_dx, double *dtest_dy, double minSqrGradientMagnitude)
{
  // check for incorrect usage (possible programmer error)
  if (ref_z == NULL && d_dimensionMode == REF_HEIGHTFIELD) {
     return -1;
  }

  // we can actually reference x positions from 0..width but we lose
  // information about intensity and the ability to calculate derivatives
  // at the edges so we stay away from them
  double x, y, z = 0.0, x2, y2;
  double xmin_ref = 0, xmax_ref = 0, ymin_ref = 0, ymax_ref = 0;
  double xmin_test = 0, xmax_test = 0, ymin_test = 0, ymax_test = 0;

  double border = 0.001;
  xmin_ref = border;
  xmax_ref = ref->width() - border;
  ymin_ref = border;
  ymax_ref = ref->height() - border;
  xmin_test = border;
  xmax_test = test->width() - border;
  ymin_test = border;
  ymax_test = test->height() - border;

  int numPointsAcquired = 0;
  int numConsecutiveDiscardedPoints = 0;

  // perhaps we should be sampling from integer coordinates in
  // ref and only interpolate in the test image but this is more symmetrical

  // find the smallest grid with dimensions nxn or nx(n-1) that contains
  // more points than the number of points we want to sample
  int gridWidth = (int)ceil(sqrt( (float) numPoints) );
  // we know gridWidth*gridWidth >= numPoints but its possible that
  // gridWidth*(gridWidth-1) >= numPoints so assume this is the case and
  // if it turns out that it isn't then switch to use the square grid
  int gridHeight = gridWidth-1;
  if (gridWidth*gridHeight < numPoints) {
    gridHeight++;
  }
  // keep things reasonable so the rest of the code stays happy
  if (gridWidth < 1) gridWidth = 1;
  if (gridHeight < 1) gridHeight = 1;

  // compute grid step sizes
  double x_incr = (xmax_ref - xmin_ref)/(gridWidth);
  double y_incr = (ymax_ref - ymin_ref)/(gridHeight);

  int i = 0,j = 0;

  while (numPointsAcquired < numPoints && 
         numConsecutiveDiscardedPoints < numPoints) {
    if (i == gridWidth) {
      j++;
      if (j == gridHeight) {
        j = 0;
      }
      i = 0;
    }
    if (samplePositionMode == NMR_RANDOM) {
      x = nmr_Util::sampleUniformDistribution(xmin_ref, xmax_ref);
      y = nmr_Util::sampleUniformDistribution(ymin_ref, ymax_ref);
    } else if (samplePositionMode == NMR_REGULAR){
      x = xmin_ref + x_incr*(i+0.5);
      y = ymin_ref + y_incr*(j+0.5);
    } else if (samplePositionMode == NMR_JITTERED) {
      double min_x = xmin_ref + x_incr*i;
      double max_x = min_x + x_incr;
      double min_y = ymin_ref + y_incr*j;
      double max_y = min_y + y_incr;
      x = nmr_Util::sampleUniformDistribution(min_x, max_x);
      y = nmr_Util::sampleUniformDistribution(min_y, max_y);
    }
    if (d_dimensionMode == REF_HEIGHTFIELD) {
      z = ref_z->getValueInterpolated(x, y);
      x2 = transform_x(x, y, z, transform);
      y2 = transform_y(x, y, z, transform);
    } else {
      z = 0.0;
      x2 = transform_x(x, y, transform);
      y2 = transform_y(x, y, transform);
    }
    if (x2 >= xmin_test && x2 < xmax_test &&
        y2 >= ymin_test && y2 < ymax_test) {
      dtest_dx[numPointsAcquired] =
                   grad_x_test->getValueInterpolated(x2,y2);
      dtest_dy[numPointsAcquired] =
                   grad_y_test->getValueInterpolated(x2,y2);
      vrpn_bool passedGradientTest = vrpn_TRUE;
      vrpn_bool passedFeatureDistanceTest = vrpn_TRUE;

      if (sampleRejectionCriterion == NMR_GRADIENT_SELECT) {
        if (minSqrGradientMagnitude > 0) {
          double mag = dtest_dx[numPointsAcquired]*dtest_dx[numPointsAcquired] +
                     dtest_dy[numPointsAcquired]*dtest_dy[numPointsAcquired];
          if (mag < minSqrGradientMagnitude) {
             passedGradientTest = vrpn_FALSE;
          }
        }
      } else if (sampleRejectionCriterion == NMR_REF_FEATURE_DISTANCE_SELECT) {
        double dist = distanceToNearestFeaturePoint(x, y);
        if (dist > d_maxDistanceToFeaturePoint) {
          passedFeatureDistanceTest = vrpn_FALSE;
        }
      }

      if (passedGradientTest && passedFeatureDistanceTest) {
        x_ref[numPointsAcquired] = x;
        y_ref[numPointsAcquired] = y;
        z_ref[numPointsAcquired] = z;
        x_test[numPointsAcquired] = x2;
        y_test[numPointsAcquired] = y2;
        val_ref[numPointsAcquired] = ref->getValueInterpolated(x,y);
        val_test[numPointsAcquired] = test->getValueInterpolated(x2,y2);
        numConsecutiveDiscardedPoints = 0;
        numPointsAcquired++;
      } else {
        numConsecutiveDiscardedPoints++;
      }
    } else {
      // the point falls outside the test image so we can either pretend we
      // know what the image looks like or we can choose to not use this point
      // The following is commented out so we don't use points that fall
      // outside the intersection of the two images
      numConsecutiveDiscardedPoints++;
/*
      dtest_dx[numPointsAcquired] = 0.0;
      dtest_dy[numPointsAcquired] = 0.0;
      x_ref[numPointsAcquired] = x;
      y_ref[numPointsAcquired] = y;
      z_ref[numPointsAcquired] = z;
      x_test[numPointsAcquired] = x2;
      y_test[numPointsAcquired] = y2;
      val_ref[numPointsAcquired] = ref->getValueInterpolated(x,y);
      val_test[numPointsAcquired] = 0.0;
      numPointsAcquired++;
*/
    }
    i++;
  }
  if (numPointsAcquired == numPoints) {
    return 0;
  } else {
    return -1;
  }
}

int nmr_ObjectiveMI_EMMA::buildSampleA(nmb_Image *ref, nmb_Image *test, 
                     nmb_Image *grad_x_test, 
                     nmb_Image *grad_y_test,
                     double *transform,
                     nmr_SamplePositionMode samplePositionMode,
                     nmr_SampleRejectionCriterion sampleRejCrit,
                     double minSqrGradientMagnitude,
                     nmb_Image *ref_z)

{
  int result = buildSampleHelper(ref, test, grad_x_test, grad_y_test, 
         transform, samplePositionMode, sampleRejCrit, ref_z,
         d_sizeA, d_Ax_ref, d_Ay_ref, d_Az_ref, d_Ax_test, d_Ay_test,
         d_refValuesA, d_testValuesA, d_dTestValueA_dx, d_dTestValueA_dy,
         minSqrGradientMagnitude);
  if (result == 0) {
    d_sampleA_acquired = VRPN_TRUE;
  }
  return result;
}


int nmr_ObjectiveMI_EMMA::buildSampleB(nmb_Image *ref, nmb_Image *test, 
                     nmb_Image *grad_x_test,       
                     nmb_Image *grad_y_test,
                     double *transform,
                     nmr_SamplePositionMode samplePositionMode,
                     nmr_SampleRejectionCriterion sampleRejCrit,
                     double minSqrGradientMagnitude,
                     nmb_Image *ref_z)
{
  int result = buildSampleHelper(ref, test, grad_x_test, grad_y_test, 
         transform, samplePositionMode, sampleRejCrit, ref_z,
         d_sizeB, d_Bx_ref, d_By_ref, d_Bz_ref, d_Bx_test, d_By_test,
         d_refValuesB, d_testValuesB, d_dTestValueB_dx, d_dTestValueB_dy,
         minSqrGradientMagnitude);
  if (result == 0) {
    d_sampleB_acquired = VRPN_TRUE;
  }
  return result;
}

int nmr_ObjectiveMI_EMMA::getJointHistogram(nmb_Image *histogram,
                           double *transform, vrpn_bool blur,
                           vrpn_bool setRefScale,
                           float min_ref, float max_ref,
                           vrpn_bool setTestScale,
                           float min_test, float max_test)
{
  double transformPixelUnits[16];

  convertTransformToPixelUnits(transform, transformPixelUnits);

  return buildJointHistogram(d_refValue, d_testValue, d_refZ, histogram,
             transformPixelUnits, blur, setRefScale, min_ref, max_ref, 
             setTestScale, min_test, max_test);
}

/*
 buildJointHistogram: this function is intended for visualization and 
 debugging purposes

 ref: input reference image
 test: input test image
 histogram: output with arbitrary resolution (whatever user supplies), 
            x->ref val, y->test val, scaling of reference and test values to
            indices is either the full scale by default if
            setRefScale==FALSE and setTestScale==FALSE
            or is specified in the arguments as
            a range: (min_ref, max_ref) and (min_test, max_test) - this lets
            you zoom in to get a more detailed picture of some part of the
            histogram
 blur: if TRUE then the histogram is blurred with the gaussian that has
       variance in ref and test dimensions equal to the variance used for 
       the Parzen windowing.
       

*/

int nmr_ObjectiveMI_EMMA::buildJointHistogram(nmb_Image *ref, nmb_Image *test,
                            nmb_Image *ref_z,
                            nmb_Image *histogram, double *transform,
                            vrpn_bool blur,
                            vrpn_bool setRefScale,
                            float min_ref, float max_ref,
                            vrpn_bool setTestScale,
                            float min_test, float max_test)
{
  int i,j,k;
  double x_ref, y_ref, x_test, y_test;
  double min_x_test = 0.5, max_x_test = test->width()-0.5;
  double min_y_test = 0.5, max_y_test = test->height()-0.5;
  float val_ref, val_test;
  float val_hist;
  int ref_val_index, test_val_index;
  double dref_val_index, dtest_val_index;
  int hist_width = histogram->width();
  int hist_height = histogram->height();

  if (!setRefScale) {
    min_ref = ref->minValue();
    max_ref = ref->maxValue();
  }
  if (!setTestScale) {
    min_test = test->minValue();
    max_test = test->maxValue();
  }
  float ref_span_inv, test_span_inv;
  ref_span_inv = 1.0/(max_ref - min_ref);
  test_span_inv = 1.0/(max_test - min_test);

  for (i = 0; i < histogram->width(); i++) {
    for (j = 0; j < histogram->height(); j++) {
      histogram->setValue(i,j, 0.0);
    }
  }

  double z_ref;
  for (i = 0, x_ref = 0.5; i < ref->width(); i++, x_ref += 1.0) {
    for (j = 0, y_ref = 0.5; j < ref->height(); j++, y_ref += 1.0) {
      if (ref_z) {
        z_ref = ref_z->getValue(i,j);
        x_test = transform_x(x_ref, y_ref, z_ref, transform);
        y_test = transform_y(x_ref, y_ref, z_ref, transform);
      } else {
        x_test = transform_x(x_ref, y_ref, transform);
        y_test = transform_y(x_ref, y_ref, transform);
      }
      if (x_test > min_x_test && x_test < max_x_test &&
          y_test > min_y_test && y_test < max_y_test) {
        val_ref = ref->getValue(i,j);
        val_test = test->getValueInterpolated(x_test, y_test);
        dref_val_index = (val_ref - min_ref)*ref_span_inv*hist_width;
        dtest_val_index = (val_test - min_test)*test_span_inv*hist_height;
        ref_val_index = (int)floor(dref_val_index);
        test_val_index = (int)floor(dtest_val_index);
        // there is a rare possibility that the following decrement will
        // be necessary (when val_ref == max_ref or val_test == max_test)
        if (ref_val_index == hist_width) ref_val_index--;
        if (test_val_index == hist_width) test_val_index--;
        val_hist = histogram->getValue(ref_val_index, test_val_index);
        val_hist += 1.0;
        histogram->setValue(ref_val_index, test_val_index, val_hist);
      }
    }
  }

  if (blur) {
    double deltaRefVal = hist_width*ref_span_inv;
    double deltaTestVal = hist_height*test_span_inv;
    double numStdDev = 3.0;
    double refFilterExtent = numStdDev*d_sigmaRefRef/deltaRefVal;
    double testFilterExtent = numStdDev*d_sigmaTestTest/deltaTestVal;
    double total_weight;
    
    float *lineCopy = NULL;
    if (hist_width > hist_height) {
      lineCopy = new float[hist_width];
    } else {
      lineCopy = new float[hist_height];
    }
    int rowFilterLength = 2*(int)ceil(refFilterExtent)+1;
    double *rowFilter = new double[rowFilterLength];
    int colFilterLength = 2*(int)ceil(testFilterExtent)+1;
    double *colFilter = new double[colFilterLength];
    nmr_Gaussian::makeFilter(rowFilterLength, rowFilter, 
                             deltaRefVal/d_sigmaRefRef);
    nmr_Gaussian::makeFilter(colFilterLength, colFilter,
                             deltaTestVal/d_sigmaTestTest);

    double blurredVal;
    // blur rows first
    for (j = 0; j < hist_height; j++) {
      // make temporary copy of the row
      for (i = 0; i < hist_width; i++) {
        lineCopy[i] = histogram->getValue(i,j);
      }
      // convolve the row with a gaussian filter with std. dev. equal to
      // d_sigmaRefRef/deltaRefVal with filter extent of refFilterExtent
      for (i = 0; i < hist_width; i++) {
        int i_conv;
        int i_conv_min;
        i_conv_min = i - rowFilterLength/2;
        total_weight = 1.0;
        blurredVal = 0.0;
        vrpn_bool renormalize = vrpn_FALSE;
        for (k = 0, i_conv = i_conv_min; k < rowFilterLength; k++, i_conv++) {
          if (i_conv >= 0 && i_conv < hist_width) {
            blurredVal += rowFilter[k]*lineCopy[i_conv];
          } else {
            renormalize = vrpn_TRUE;
            total_weight -= rowFilter[k];
          }
        }
        if (renormalize) {
          if (total_weight == 0) {
            printf("buildJointHistogram row blur: this shouldn't happen\n");
          }
          blurredVal /= total_weight;
        }
        histogram->setValue(i, j, blurredVal);
      }
    }
    // now blur columns
    for (i = 0; i < hist_width; i++) {
      // make temporary copy of the column 
      for (j = 0; j < hist_height; j++) {
        lineCopy[j] = histogram->getValue(i,j);
      }
      // convolve the row with a gaussian filter with std. dev. equal to
      // d_sigmaTestTest/deltaTestVal with filter extent of testFilterExtent
      for (j = 0; j < hist_height; j++) {
        int j_conv;
        int j_conv_min;
        j_conv_min = j - colFilterLength/2;
        total_weight = 1.0;
        blurredVal = 0.0;
        vrpn_bool renormalize = vrpn_FALSE;
        for (k = 0, j_conv = j_conv_min; k < colFilterLength; k++, j_conv++) {
          if (j_conv >= 0 && j_conv < hist_height) {
            blurredVal += colFilter[k]*lineCopy[j_conv];
          } else {
            total_weight -= colFilter[k];
            renormalize = vrpn_TRUE;
          }
        }
        if (renormalize) {
          if (total_weight == 0) {
            printf("buildJointHistogram column blur: this shouldn't happen\n");
          }
          blurredVal /= total_weight;
        }
        histogram->setValue(i, j, blurredVal);
      }
    }

    delete [] lineCopy;
    delete [] rowFilter;
    delete [] colFilter;
  } // end if blur

  float val;
  float minVal = histogram->minNonZeroValue();
  float invLogMinVal = 1.0/log(2.0);
  for (i = 0; i < hist_width; i++) {
    for (j = 0; j < hist_height; j++) {
      val = histogram->getValue(i, j);
      if (val > 5.0) val = 5.0;
      if (val > 0) {
          val = invLogMinVal*log(val+1.0);
          histogram->setValue(i, j, val);
      }
    }
  }

  return 0;
}

// the best choice for sigma is the one that minimizes h* so we
// will use this derivative in a gradient descent algorithm
int nmr_ObjectiveMI_EMMA::computeCrossEntropyGradient(double &dHc_dsigma_ref, 
                                             double &dHc_dsigma_test)
{
  // it is possible that we have acquired samples A and B but changed
  // the transformation since then so we could still compute the
  // gradient with respect to variance but it would be for the old
  // transformation - instead we just return an error 
  if (!d_sampleA_acquired || !d_sampleB_acquired) {
    return -1;
  }

  // compute derivative of entropy estimator h*(z) with respect to the
  // Parzen window standard deviation sigma
  //
  // dh*(z)/ds = (1/Nb)*  Sum     Sum    (W(zb, za)*(1/s)*((zb-za)^2/s^2 - 1))
  //                    zb in B za in A

  dHc_dsigma_ref = 0.0;
  dHc_dsigma_test = 0.0;

  // parameters for evaluating gaussian
  double z[2];
  double inv_sigma[2] = {1.0/d_sigmaRefRef, 1.0/d_sigmaTestTest};
  double mu[2] = {0.0, 0.0};
  
  double invRefVariance = 1.0/(d_sigmaRefRef*d_sigmaRefRef);
  double invTestVariance = 1.0/(d_sigmaTestTest*d_sigmaTestTest);
  int i,j;
  for (i = 0; i < d_sizeB; i++) {
    for (j = 0; j < d_sizeA; j++) {
      // code to compute W(z_i, z_j)
      //
      //                     G_sigma(z_i - z_j)
      // W(z_i, z_j) = ------------------------------
      //                   Sum   (G_sigma(z_i - z_k))
      //                z_k in A
      int k;
      double W_den = 0.0;
      for (k = 0; k < d_sizeA; k++) {
        // z = z_i - z_k
        z[0] = d_refValuesB[i] - d_refValuesA[k];
        z[1] = d_testValuesB[i] - d_testValuesA[k];
        W_den += nmr_Gaussian::value(z[0], inv_sigma[0], mu[0]) *
                 nmr_Gaussian::value(z[1], inv_sigma[1], mu[1]);
      }
      // z = z_i - z_j
      z[0] = d_refValuesB[i] - d_refValuesA[j];
      z[1] = d_testValuesB[i] - d_testValuesA[j];
      double W_num = nmr_Gaussian::value(z[0], inv_sigma[0], mu[0]) *
                     nmr_Gaussian::value(z[1], inv_sigma[1], mu[1]);
      double W = 0;
      if (W_den == 0) {
        static vrpn_bool printedAlready = VRPN_FALSE;
        if (printedAlready) 
        printf("computeCrossEntropyGradient: this shouldn't happen\n");
        //printedAlready = VRPN_TRUE;
      } else {
        W = W_num/W_den;
      }
      dHc_dsigma_ref += W*(z[0]*z[0]*invRefVariance - 1.0);
      dHc_dsigma_test += W*(z[1]*z[1]*invTestVariance - 1.0);
    }
  }
  dHc_dsigma_ref *= 1.0/(d_sigmaRefRef*(double)d_sizeB);
  dHc_dsigma_test *= 1.0/(d_sigmaTestTest*(double)d_sizeB);
  return 0;
}

int nmr_ObjectiveMI_EMMA::crossEntropy(double &entropy)
{
  // it is possible that we have acquired samples A and B but changed
  // the transformation since then so we could still compute the
  // gradient with respect to variance but it would be for the old
  // transformation - instead we just return an error
  if (!d_sampleA_acquired || !d_sampleB_acquired) {
    return -1;
  }

  // compute the cross entropy:
  //             
  // h = (-1/Nb)*  Sum  ( ln[(1/Na)*(  Sum  ( G_sigma(zb-za))))
  //             zb in B             za in A
  //

  // parameters for evaluating gaussian
  double z[2];
  double inv_sigma[2] = {1.0/d_sigmaRefRef, 1.0/d_sigmaTestTest};
  double mu[2] = {0.0, 0.0};

  double sum = 0;
  int i,j;
  for (i = 0; i < d_sizeB; i++) {
    double density = 0.0;
    for (j = 0; j < d_sizeA; j++) {
      z[0] = d_refValuesB[i] - d_refValuesA[j];
      z[1] = d_testValuesB[i] - d_testValuesA[j];
      density += nmr_Gaussian::value(z[0], inv_sigma[0], mu[0]) *
                 nmr_Gaussian::value(z[1], inv_sigma[1], mu[1]);
    }
    density /= (double)d_sizeA;
    if (density != 0) {
      sum += log(density);
    } else {
      sum += 0.0;
    }
  }
  sum /= -1.0*(double)d_sizeB;
  entropy = sum;
  return 0;
}

int nmr_ObjectiveMI_EMMA::computeTestEntropyGradient(double &dH_dsigma_test)
{
  // it is possible that we have acquired samples A and B but changed
  // the transformation since then so we could still compute the
  // gradient with respect to variance but it would be for the old
  // transformation - instead we just return an error
  if (!d_sampleA_acquired || !d_sampleB_acquired) {
    return -1;
  }

  // compute derivative of entropy estimator h*(z) with respect to the
  // Parzen window standard deviation sigma
  //
  // dh*(z)/ds = (1/Nb)*  Sum     Sum    (W(zb, za)*(1/s)*((zb-za)^2/s^2 - 1))
  //                    zb in B za in A

  dH_dsigma_test = 0.0;

  // parameters for evaluating gaussian
  double v;
  double mu = 0.0;

  double invSigmaTest = 1.0/d_sigmaTest;

  double invTestVariance = invSigmaTest*invSigmaTest;
  int i,j;
  for (i = 0; i < d_sizeB; i++) {
    for (j = 0; j < d_sizeA; j++) {
      // code to compute W(v_i, v_j)
      //
      //                     G_sigma(v_i - v_j)
      // W(z_i, z_j) = ------------------------------
      //                   Sum   (G_sigma(v_i - v_k))
      //                z_k in A
      int k;
      double W_den = 0.0;
      for (k = 0; k < d_sizeA; k++) {
        // v = v_i - v_k
        v = d_testValuesB[i] - d_testValuesA[k];
        W_den += nmr_Gaussian::value(v, invSigmaTest, mu);
      }

      // v = v_i - v_j
      v = d_testValuesB[i] - d_testValuesA[j];
      double W_num = nmr_Gaussian::value(v, invSigmaTest, mu);

      double W = 0;
      if (W_den == 0) {
        static vrpn_bool printedAlready = VRPN_FALSE;
        if (printedAlready) 
        printf("computeTestEntropyGradient: this shouldn't happen\n");
        //printedAlready = VRPN_TRUE;
      } else {
        W = W_num/W_den;
      }

      dH_dsigma_test += W*(v*v*invTestVariance - 1.0);
    }
  }
  dH_dsigma_test *= 1.0/(d_sigmaTest*(double)d_sizeB);
  return 0;
}

int nmr_ObjectiveMI_EMMA::testEntropy(double &entropy)
{
  // it is possible that we have acquired samples A and B but changed
  // the transformation since then so we could still compute the
  // gradient with respect to variance but it would be for the old
  // transformation - instead we just return an error
  if (!d_sampleA_acquired || !d_sampleB_acquired) {
    return -1;
  }

  // parameters for evaluating gaussian
  double v;
  double mu = 0.0;
  double invSigmaTest = 1.0/d_sigmaTest;

  double sum = 0;
  int i,j;
  for (i = 0; i < d_sizeB; i++) {
    double density = 0.0;
    for (j = 0; j < d_sizeA; j++) {
      v = d_testValuesB[i] - d_testValuesA[j];
      density += nmr_Gaussian::value(v, invSigmaTest, mu);
    }
    density /= (double)d_sizeA;
    if (density != 0) {
      sum += log(density);
    } else {
      sum += 0.0;
    }
  }
  sum /= -1.0*(double)d_sizeB;
  entropy = sum;
  return 0;
}

int nmr_ObjectiveMI_EMMA::computeRefEntropyGradient(double &dH_dsigma_ref)
{
  // it is possible that we have acquired samples A and B but changed
  // the transformation since then so we could still compute the
  // gradient with respect to variance but it would be for the old
  // transformation - instead we just return an error
  if (!d_sampleA_acquired || !d_sampleB_acquired) {
    return -1;
  }

  // compute derivative of entropy estimator h*(z) with respect to the
  // Parzen window standard deviation sigma
  //
  // dh*(z)/ds = (1/Nb)*  Sum     Sum    (W(zb, za)*(1/s)*((zb-za)^2/s^2 - 1))
  //                    zb in B za in A

  dH_dsigma_ref = 0.0;

  // parameters for evaluating gaussian
  double v;
  double mu = 0.0;

  double invSigmaRef = 1.0/d_sigmaRef;

  double invRefVariance = invSigmaRef*invSigmaRef;
  int i,j;
  for (i = 0; i < d_sizeB; i++) {
    for (j = 0; j < d_sizeA; j++) {
      // code to compute W(v_i, v_j)
      //
      //                     G_sigma(v_i - v_j)
      // W(z_i, z_j) = ------------------------------
      //                   Sum   (G_sigma(v_i - v_k))
      //                z_k in A
      int k;
      double W_den = 0.0;
      for (k = 0; k < d_sizeA; k++) {
        // v = v_i - v_k
        v = d_refValuesB[i] - d_refValuesA[k];
        W_den += nmr_Gaussian::value(v, invSigmaRef, mu);
      }
      // v = v_i - v_j
      v = d_refValuesB[i] - d_refValuesA[j];
      double W_num = nmr_Gaussian::value(v, invSigmaRef, mu);
      double W = 0;
      if (W_num == 0) {
        static vrpn_bool printedAlready = vrpn_FALSE;
        if (!printedAlready)
        printf("computeRefEntropyGradient: this shouldn't happen\n");
        //printedAlready = vrpn_TRUE;
      } else {
        W = W_num/W_den;
      }
      dH_dsigma_ref += W*(v*v*invRefVariance - 1.0);
    }
  }
  dH_dsigma_ref *= 1.0/(d_sigmaRef*(double)d_sizeB);
  return 0;
}

int nmr_ObjectiveMI_EMMA::refEntropy(double &entropy)
{
  // it is possible that we have acquired samples A and B but changed
  // the transformation since then so we could still compute the
  // gradient with respect to variance but it would be for the old
  // transformation - instead we just return an error
  if (!d_sampleA_acquired || !d_sampleB_acquired) {
    return -1;
  }

  // parameters for evaluating gaussian
  double v;
  double mu = 0.0;
  double invSigmaRef = 1.0/d_sigmaRef;

  double sum = 0;
  int i,j;
  for (i = 0; i < d_sizeB; i++) {
    double density = 0.0;
    for (j = 0; j < d_sizeA; j++) {
      v = d_refValuesB[i] - d_refValuesA[j];
      density += nmr_Gaussian::value(v, invSigmaRef, mu);
    }
    density /= (double)d_sizeA;
    if (density != 0) {
      sum += log(density);
    } else {
      sum += 0.0;
    }
  }
  sum /= -1.0*(double)d_sizeB;
  entropy = sum;
  return 0;
}

int nmr_ObjectiveMI_EMMA::computeTransformationGradient(double *dIdT)
{
  // it is possible that we have acquired samples A and B but changed
  // the transformation since then so we could still compute the
  // gradient with respect to variance but it would be for the old
  // transformation - instead we just return an error
  if (!d_sampleA_acquired || !d_sampleB_acquired) {
    return -1;
  }

//  if (d_dimensionMode == REF_HEIGHTFIELD) {
    return computeTransformationGradient_HeightfieldRef(dIdT);
/*  } else {
    double temp[8];
    int result = computeTransformationGradient_HeightfieldRef(temp);
    dIdT[0] = temp[0];
    dIdT[1] = temp[1];
    dIdT[2] = temp[2];
    dIdT[3] = temp[3];
    dIdT[4] = temp[4];
    dIdT[5] = temp[5];
    dIdT[6] = 0.0;
    dIdT[7] = 0.0;
    return result;
  }
*/
}

int nmr_ObjectiveMI_EMMA::computeTransformationGradient_HeightfieldRef(double *dIdT)
{
  // it is possible that we have acquired samples A and B but changed
  // the transformation since then so we could still compute the
  // gradient with respect to variance but it would be for the old
  // transformation - instead we just return an error
  if (!d_sampleA_acquired || !d_sampleB_acquired) {
    return -1;
  }

  // derivative of the difference of intensities (v_i - v_j)
  // with respect to each component of the transformation T
  double dv_dT[8];
  // parameters for evaluating gaussian
  double w[2]; // (u, v)
  double inv_sigma[2] = {1.0/d_sigmaRefRef, 1.0/d_sigmaTestTest};
  double invSigmaTest = 1.0/d_sigmaTest;
  double mu[2] = {0.0, 0.0};
  double v;
  int i,j,k;
  double dv_dx_A, dv_dy_A, dv_dx_B, dv_dy_B;

  for (i = 0; i < 8; i++) {
    dIdT[i] = 0.0;
  }
  for (i = 0; i < d_sizeB; i++) {
    double Wv, Ww, Wv_den, Wv_num, Ww_den, Ww_num;
    double diff;

    Wv_den = 0.0;
    for (k = 0; k < d_sizeA; k++) {
      // diff = v_i - v_k
      diff = d_testValuesB[i] - d_testValuesA[k];
      Wv_den += nmr_Gaussian::value(diff, invSigmaTest, 0.0);
    }
    
    Ww_den = 0.0;
    for (k = 0; k < d_sizeA; k++) {
        // w[0] = u_i - u_k
        // w[1] = v_i - v_k
        w[0] = d_refValuesB[i] - d_refValuesA[k];
        w[1] = d_testValuesB[i] - d_testValuesA[k];
        Ww_den += nmr_Gaussian::value(w[0], inv_sigma[0], mu[0]) *
                  nmr_Gaussian::value(w[1], inv_sigma[1], mu[1]);
    }


    for (j = 0; j < d_sizeA; j++) {
      // dTestIntensity_dx at (d_Ax_test, d_Ay_test)
      dv_dx_A = d_dTestValueA_dx[j];
      // dTestIntensity_dy at (d_Ax_test, d_Ay_test)
      dv_dy_A = d_dTestValueA_dy[j];
      // dTestIntensity_dx at (d_Bx_test, d_By_test)
      dv_dx_B = d_dTestValueB_dx[i];
      // dTestIntensity_dy at (d_Bx_test, d_By_test)
      dv_dy_B = d_dTestValueB_dy[i];

      dv_dT[0] = d_Bx_ref[i]*dv_dx_B - d_Ax_ref[j]*dv_dx_A;
      dv_dT[1] = d_By_ref[i]*dv_dx_B - d_Ay_ref[j]*dv_dx_A;
      dv_dT[2] = d_Bz_ref[i]*dv_dx_B - d_Az_ref[j]*dv_dx_A;
      dv_dT[3] = dv_dx_B - dv_dx_A;
      dv_dT[4] = d_Bx_ref[i]*dv_dy_B - d_Ax_ref[j]*dv_dy_A;
      dv_dT[5] = d_By_ref[i]*dv_dy_B - d_Ay_ref[j]*dv_dy_A;
      dv_dT[6] = d_Bz_ref[i]*dv_dy_B - d_Az_ref[j]*dv_dy_A;
      dv_dT[7] = dv_dy_B - dv_dy_A;

      v = d_testValuesB[i] - d_testValuesA[j];
      // now compute (Wv(v_i, v_j)/d_sigmaTest - Ww(w_i, w_j)/d_sigmaTestTest)
      // and multiply it by v*dv_dT to get the result

      // code to compute Wv(v_i, v_j)
      //
      //                     G_sigma(v_i - v_j)
      // W(z_i, z_j) = ------------------------------
      //                   Sum   (G_sigma(v_i - v_k))
      //                z_k in A
      // diff = v_i - v_j
      diff = d_testValuesB[i] - d_testValuesA[j];
      
      Wv_num = nmr_Gaussian::value(diff, invSigmaTest, 0.0);

      if (Wv_den == 0) {
        static vrpn_bool printedAlready = VRPN_FALSE;
        if (!printedAlready)
        printf("computeTransformationGradient: this shouldn't happen (1)\n");
        //printedAlready = VRPN_TRUE;
        Wv = 0.0;
      } else {
        Wv = Wv_num/Wv_den;
      }

      // code to compute Ww(w_i, w_j)
      // w[0] = u_i - u_j
      // w[1] = v_i - v_j
      w[0] = d_refValuesB[i] - d_refValuesA[j];
      w[1] = d_testValuesB[i] - d_testValuesA[j];
      
      Ww_num = nmr_Gaussian::value(w[0], inv_sigma[0], mu[0]) *
               nmr_Gaussian::value(w[1], inv_sigma[1], mu[1]);

      if (Ww_den == 0) {
        static vrpn_bool printedAlready = VRPN_FALSE;
        if (!printedAlready)
        printf("computeTransformationGradient: this shouldn't happen (2)\n");
        //printedAlready = VRPN_TRUE;
        Ww = 0.0;
      } else {
        Ww = Ww_num/Ww_den;
      }

      double commonFactor = v*(Wv/d_sigmaTest - Ww/d_sigmaTestTest);

      for (k = 0; k < 8; k++) {
        dIdT[k] += dv_dT[k]*commonFactor;
      }
    }
  }
  for (i = 0; i < 8; i++) {
    dIdT[i] *= (1.0/(double)d_sizeB);
  }

  return 0;
}

int nmr_ObjectiveMI_EMMA::computeMutualInformation(double &mutualInfo)
{
  double test_entropy, ref_entropy, cross_entropy;

  if (testEntropy(test_entropy) || refEntropy(ref_entropy) ||
      crossEntropy(cross_entropy)) {
    return -1;
  }
  mutualInfo = test_entropy + ref_entropy - cross_entropy;
  return 0;
}

void nmr_ObjectiveMI_EMMA::convertTransformToPixelUnits(double *T1, double *T2)
{
  // initialize transformation to the transform in image units
  int i;
  if (T1 != T2) {
    for (i = 0; i < 16; i++) {
      T2[i] = T1[i];
    }
  }
  return;
  if (!d_refValue || !d_testValue) {
    return;
  }

  nmb_Image *ref = d_refValue;
  nmb_Image *test = d_testValue;
  double dref_width_inv = 1.0/(ref->width());
  double dref_height_inv = 1.0/(ref->height());

  // scale input down from 0..ref.width, 0..ref.height to 0..1, 0..1
  T2[0] *= dref_width_inv;
  T2[1] *= dref_width_inv;
  T2[2] *= dref_width_inv;
  T2[3] *= dref_width_inv;

  T2[4] *= dref_height_inv;
  T2[5] *= dref_height_inv;
  T2[6] *= dref_height_inv;
  T2[7] *= dref_height_inv;

  // scale output from 0..1, 0..1 to 0..test.width, 0..test.height
  T2[0] *= test->width();
  T2[4] *= test->width();
  T2[8] *= test->width();
  T2[12] *= test->width();
 
  T2[1] *= test->height();
  T2[5] *= test->height();
  T2[9] *= test->height();
  T2[13] *= test->height();

  return;
}

void nmr_ObjectiveMI_EMMA::convertTransformToImageUnits(double *T1, double *T2)
{
  // initialize transformation to the transform in pixel units
  int i;
  if (T1 != T2) {
    for (i = 0; i < 16; i++) {
      T2[i] = T1[i];
    }
  }
  return;
  if (!d_refValue || !d_testValue) {
    return;
  }

  nmb_Image *ref = d_refValue;
  nmb_Image *test = d_testValue;
  // scale up input values from 0..1, 0..1 to 0..ref.width, 0..ref.height
  double dref_width = ref->width();
  double dref_height = ref->height();
  T2[0] *= dref_width;
  T2[1] *= dref_width;
  T2[2] *= dref_width;
  T2[3] *= dref_width;

  T2[4] *= dref_height;
  T2[5] *= dref_height;
  T2[6] *= dref_height;
  T2[7] *= dref_height;

  // scale output from 0..test.width, 0..test.height to 0..1, 0..1 
  double test_width_inv = 1.0/(test->width());
  double test_height_inv = 1.0/(test->height());
  T2[0] *= test_width_inv;
  T2[4] *= test_width_inv;
  T2[8] *= test_width_inv;
  T2[12] *= test_width_inv;

  T2[1] *= test_height_inv;
  T2[5] *= test_height_inv;
  T2[9] *= test_height_inv;
  T2[13] *= test_height_inv;

  return;
}

double nmr_ObjectiveMI_EMMA::distanceToNearestFeaturePoint(double x, double y)
{
  double minDist;
  double testDist;
  int i;
  double dx, dy;
  if (d_numRefFeaturePoints == 0) return 0.0;
  dx = x - d_xFeature[0];
  dy = y - d_yFeature[0];
  minDist = dx*dx + dy*dy;
  for (i = 1; i < d_numRefFeaturePoints; i++) {
    dx = x - d_xFeature[i];
    dy = y - d_yFeature[i];
    testDist = dx*dx+dy*dy;
    if (testDist < minDist) {
      minDist = testDist;
    }
  }
  minDist = sqrt(minDist);
  return minDist;
}
