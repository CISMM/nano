#include "nmr_AlignerMI.h"
#include "nmr_Gaussian.h"
#include "nmr_Util.h"

// #define DEBUG_FILE_OUTPUT

nmr_AlignerMI::nmr_AlignerMI():
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
  d_dimensionMode(REF_2D)
{
}

nmr_AlignerMI::~nmr_AlignerMI()
{
  if (d_workspace) {
    delete [] d_workspace;
  }
}

void nmr_AlignerMI::setDimensionMode(nmr_DimensionMode mode)
{
  d_dimensionMode = mode;
}

nmr_DimensionMode nmr_AlignerMI::getDimensionMode()
{
  return d_dimensionMode;
}

int nmr_AlignerMI::setSampleSizes(int sizeA, int sizeB)
{
  // allocate all our big arrays at once
  d_workspace = new double[9*(sizeA+sizeB)];

  if (d_workspace == NULL) {
    fprintf(stderr, "nmr_AlignerMI::setSampleSizes: out of memory\n");
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

void nmr_AlignerMI::getSampleSizes(int &sizeA, int &sizeB)
{
  sizeA = d_sizeA;
  sizeB = d_sizeB;
}

void nmr_AlignerMI::setCovariance(double sigmaRefRef, double sigmaTestTest)
{
  d_sigmaRefRef = sigmaRefRef;
  d_sigmaTestTest = sigmaTestTest;
}

void nmr_AlignerMI::getCovariance(double &sigmaRefRef, double &sigmaTestTest)
{
  sigmaRefRef = d_sigmaRefRef;
  sigmaTestTest = d_sigmaTestTest;
}

void nmr_AlignerMI::setTestVariance(double sigma)
{
  d_sigmaTest = sigma;
}

void nmr_AlignerMI::getTestVariance(double &sigma)
{
  sigma = d_sigmaTest;
}

void nmr_AlignerMI::setRefVariance(double sigma)
{
  d_sigmaRef = sigma;
}

void nmr_AlignerMI::getRefVariance(double &sigma)
{
  sigma = d_sigmaRef;
}

int nmr_AlignerMI::setTransformation2D(double *T)
{
  d_T[0] = T[0]; d_T[1] = T[1]; d_T[2] = 0.0; d_T[3] = T[2];
  d_T[4] = T[3]; d_T[5] = T[4]; d_T[6] = 0.0; d_T[7] = T[5];

  d_sampleA_acquired = VRPN_FALSE;
  d_sampleB_acquired = VRPN_FALSE;

  if (d_dimensionMode != REF_2D) {
    return -1;
  } else {
    return 0;
  }
}

int nmr_AlignerMI::getTransformation2D(double *T)
{
  T[0] = d_T[0]; T[1] = d_T[1]; T[2] = d_T[3]; 
  T[3] = d_T[4]; T[4] = d_T[5]; T[5] = d_T[7];
  if (d_dimensionMode != REF_2D) {
    return -1;
  } else {
    return 0;
  }
}

int nmr_AlignerMI::setTransformationHF(double *T)
{
  d_T[0] = T[0]; d_T[1] = T[1]; d_T[2] = T[2]; d_T[3] = T[3];
  d_T[4] = T[4]; d_T[5] = T[5]; d_T[6] = T[6]; d_T[7] = T[7];

  d_sampleA_acquired = VRPN_FALSE;
  d_sampleB_acquired = VRPN_FALSE;

  if (d_dimensionMode != REF_HEIGHTFIELD) {
    return -1;
  } else {
    return 0;
  }
}

int nmr_AlignerMI::getTransformationHF(double *T)
{
  T[0] = d_T[0]; T[1] = d_T[1]; T[2] = d_T[2]; T[3] = d_T[3];
  T[4] = d_T[4]; T[5] = d_T[5]; T[6] = d_T[6]; T[7] = d_T[7];
  if (d_dimensionMode != REF_HEIGHTFIELD) {
    return -1;
  } else {
    return 0;
  }
}

/* buildSampleHelper: builds a set of points and their corresponding 
   values from the ref, test, grad_x_test, and grad_y_test images 
   o ref, test, grad_x_test, grad_y_test, ref_z: input images
   o randomize: if TRUE then points are selected randomly
	        if FALSE, points are pixel centers in ref selected in
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
int nmr_AlignerMI::buildSampleHelper(nmb_Image *ref, nmb_Image *test,
                     nmb_Image *grad_x_test,
                     nmb_Image *grad_y_test,
                     vrpn_bool randomize, 
                     nmb_Image *ref_z,
       int numPoints, double *x_ref, double *y_ref, double *z_ref,
       double *x_test, double *y_test, double *val_ref, double *val_test,
       double *dtest_dx, double *dtest_dy, double minSqrGradientMagnitude)
{
  // check for incorrect usage (possible programmer error)
  if ((ref_z == NULL && d_dimensionMode == REF_HEIGHTFIELD) ||
     (ref_z != NULL && d_dimensionMode == REF_2D)) {
     return -1;
  }

  // we can actually reference x positions from 0..width but we lose
  // information about intensity and the ability to calculate derivatives
  // at the edges so we stay away from them
  double x, y, z = 0.0, x2, y2;
  double xmin_ref, xmax_ref, ymin_ref, ymax_ref;
  double xmin_test, xmax_test, ymin_test, ymax_test;
  xmin_ref = 0;
  xmax_ref = ref->width();
  ymin_ref = 0;
  ymax_ref = ref->height();
  xmin_test = 0;
  xmax_test = test->width();
  ymin_test = 0;
  ymax_test = test->height();

  int numPointsAcquired = 0;

  // perhaps we should be sampling from integer coordinates in
  // ref and only interpolate in the test image but this is more symmetrical

  int i = xmin_ref, j = ymin_ref;
  while (numPointsAcquired < numPoints) {
    if (i == xmax_ref) {
      j++;
      if (j == ymax_ref) {
        j = ymin_ref;
      }
      i = xmin_ref;
    }
    if (randomize) {
      x = nmr_Util::sampleUniformDistribution(xmin_ref, xmax_ref);
      y = nmr_Util::sampleUniformDistribution(ymin_ref, ymax_ref);
    } else {
      x = (double)i+0.5;
      y = (double)j+0.5;
    }
    if (ref_z) {
      z = ref_z->getValueInterpolated(x, y);
      x2 = transform_x(x, y, z);
      y2 = transform_y(x, y, z);
    } else {
      z = 0.0;
      x2 = transform_x(x, y);
      y2 = transform_y(x, y);
    }
    if (x2 >= xmin_test && x2 < xmax_test &&
        y2 >= ymin_test && y2 < ymax_test) {
      dtest_dx[numPointsAcquired] =
                   grad_x_test->getValueInterpolated(x2,y2);
      dtest_dy[numPointsAcquired] =
                   grad_y_test->getValueInterpolated(x2,y2);
      vrpn_bool passedGradientTest = vrpn_TRUE;

      if (minSqrGradientMagnitude > 0) {
        double mag = dtest_dx[numPointsAcquired]*dtest_dx[numPointsAcquired] +
                     dtest_dy[numPointsAcquired]*dtest_dy[numPointsAcquired];
        if (mag < minSqrGradientMagnitude) {
           passedGradientTest = vrpn_FALSE;
        }
      }

      if (passedGradientTest) {
        x_ref[numPointsAcquired] = x;
        y_ref[numPointsAcquired] = y;
        z_ref[numPointsAcquired] = z;
        x_test[numPointsAcquired] = x2;
        y_test[numPointsAcquired] = y2;
        val_ref[numPointsAcquired] = ref->getValueInterpolated(x,y);
        float pixelVal = 0.0;
        if (!randomize) {
          pixelVal = test->getValue((int)floor(x2), (int)floor(y2));
          // (for debugging), make sure this is approx. equal to 
          // val_ref[numPointsAcquired] (within floating point error)
        }
        val_test[numPointsAcquired] = test->getValueInterpolated(x2,y2);
        numPointsAcquired++;
      }
    } else {
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
    }
    i++;
  }

  return 0;
}

int nmr_AlignerMI::buildSampleA(nmb_Image *ref, nmb_Image *test, 
                     nmb_Image *grad_x_test, 
                     nmb_Image *grad_y_test,
                     vrpn_bool randomize,
                     double minSqrGradientMagnitude,
                     nmb_Image *ref_z)

{
  int result = buildSampleHelper(ref, test, grad_x_test, grad_y_test, 
         randomize, ref_z,
         d_sizeA, d_Ax_ref, d_Ay_ref, d_Az_ref, d_Ax_test, d_Ay_test,
         d_refValuesA, d_testValuesA, d_dTestValueA_dx, d_dTestValueA_dy,
         minSqrGradientMagnitude);
  if (result == 0) {
    d_sampleA_acquired = VRPN_TRUE;
  }
  return result;
}


int nmr_AlignerMI::buildSampleB(nmb_Image *ref, nmb_Image *test, 
                     nmb_Image *grad_x_test,       
                     nmb_Image *grad_y_test,
                     vrpn_bool randomize, 
                     double minSqrGradientMagnitude,
                     nmb_Image *ref_z)
{
  int result = buildSampleHelper(ref, test, grad_x_test, grad_y_test, 
         randomize, ref_z,
         d_sizeB, d_Bx_ref, d_By_ref, d_Bz_ref, d_Bx_test, d_By_test,
         d_refValuesB, d_testValuesB, d_dTestValueB_dx, d_dTestValueB_dy,
         minSqrGradientMagnitude);
  if (result == 0) {
    d_sampleB_acquired = VRPN_TRUE;
  }
  return result;
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

int nmr_AlignerMI::buildJointHistogram(nmb_Image *ref, nmb_Image *test,
                            nmb_Image *histogram, vrpn_bool blur,
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

  for (i = 0, x_ref = 0.5; i < ref->width(); i++, x_ref += 1.0) {
    for (j = 0, y_ref = 0.5; j < ref->height(); j++, y_ref += 1.0) {
      x_test = transform_x(x_ref, y_ref);
      y_test = transform_y(x_ref, y_ref);
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
                             d_sigmaRefRef/deltaRefVal);
    nmr_Gaussian::makeFilter(colFilterLength, colFilter,
                             d_sigmaTestTest/deltaTestVal);

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
            printf("this shouldn't happen\n");
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
            printf("this shouldn't happen\n");
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
int nmr_AlignerMI::computeCrossEntropyGradient(double &dHc_dsigma_ref, 
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
        vrpn_bool printedAlready = VRPN_FALSE;
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

int nmr_AlignerMI::crossEntropy(double &entropy)
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

int nmr_AlignerMI::computeTestEntropyGradient(double &dH_dsigma_test)
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
        vrpn_bool printedAlready = VRPN_FALSE;
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

int nmr_AlignerMI::testEntropy(double &entropy)
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

int nmr_AlignerMI::computeRefEntropyGradient(double &dH_dsigma_ref)
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

int nmr_AlignerMI::refEntropy(double &entropy)
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

int nmr_AlignerMI::computeTransformationGradient(double *dIdT)
{
  // it is possible that we have acquired samples A and B but changed
  // the transformation since then so we could still compute the
  // gradient with respect to variance but it would be for the old
  // transformation - instead we just return an error
  if (!d_sampleA_acquired || !d_sampleB_acquired) {
    return -1;
  }

  if (d_dimensionMode == REF_HEIGHTFIELD) {
    return computeTransformationGradient_HeightfieldRef(dIdT);
  } else {
    double temp[8];
    int result = computeTransformationGradient_HeightfieldRef(temp);
    dIdT[0] = temp[0];
    dIdT[1] = temp[1];
    dIdT[2] = temp[3];
    dIdT[3] = temp[4];
    dIdT[4] = temp[5];
    dIdT[5] = temp[7];
    return result;
  }
}

int nmr_AlignerMI::computeTransformationGradient_HeightfieldRef(double *dIdT)
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
        vrpn_bool printedAlready = VRPN_FALSE;
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
        vrpn_bool printedAlready = VRPN_FALSE;
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

int nmr_AlignerMI::computeMutualInformation(double &mutualInfo)
{
  double test_entropy, ref_entropy, cross_entropy;

  if (testEntropy(test_entropy) || refEntropy(ref_entropy) ||
      crossEntropy(cross_entropy)) {
    return -1;
  }
  mutualInfo = test_entropy + ref_entropy - cross_entropy;
  return 0;
}
