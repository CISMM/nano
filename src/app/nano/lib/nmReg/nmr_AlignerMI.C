#include "nmr_AlignerMI.h"
#include "nmr_Gaussian.h"
#include "nmr_Util.h"

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

int nmr_AlignerMI::buildSampleA(nmb_Image *ref, nmb_Image *test, 
                               nmb_Image *ref_z)
{
  // check for incorrect usage (programmer error)
  if ((ref_z == NULL && d_dimensionMode == REF_HEIGHTFIELD) ||
     (ref_z != NULL && d_dimensionMode == REF_2D)) {
     return -1;
  }

  // we can actually reference x positions from 0..width but we lose
  // information about intensity and the ability to calculate derivatives
  // at the edges so we stay away from them
  double x, y, z = 0.0, x2, y2;
  int ix2, iy2;
  double xmin_ref, xmax_ref, ymin_ref, ymax_ref;
  double xmin_test, xmax_test, ymin_test, ymax_test;
  xmin_ref = 1;
  xmax_ref = ref->width() - 1;
  ymin_ref = 1;
  ymax_ref = ref->height() - 1;
  xmin_test = 1;
  xmax_test = test->width() - 2;
  ymin_test = 1;
  ymax_test = test->height() - 2;

  int numPointsAcquired = 0;

  // perhaps we should be sampling from integer coordinates in 
  // ref and only interpolate in the test image but this is more symmetrical
  while (numPointsAcquired < d_sizeA) {
    x = nmr_Util::sampleUniformDistribution(xmin_ref, xmax_ref);
    y = nmr_Util::sampleUniformDistribution(ymin_ref, ymax_ref);
    if (ref_z) {
      z = ref_z->getValueInterpolated(x, y);
      x2 = transform_x(x, y, z);
      y2 = transform_y(x, y, z);
    } else {
      z = 0.0;
      x2 = transform_x(x, y);
      y2 = transform_y(x, y);
    }
    if (x2 > xmin_test && x2 <= xmax_test &&
        y2 >= ymin_test && y2 <= ymax_test) {
      d_Ax_ref[numPointsAcquired] = x;
      d_Ay_ref[numPointsAcquired] = y;
      d_Az_ref[numPointsAcquired] = z;
      d_Ax_test[numPointsAcquired] = x2;
      d_Ay_test[numPointsAcquired] = y2;
      d_refValuesA[numPointsAcquired] = ref->getValueInterpolated(x,y);
      d_testValuesA[numPointsAcquired] = test->getValueInterpolated(x2,y2);
      ix2 = (int)floor(x2+0.5);
      iy2 = (int)floor(y2+0.5);
      // compute a cheap and small-aperture estimate of the gradient
      d_dTestValueA_dx[numPointsAcquired] = (0.166666)*
           (test->getValue(ix2+1,iy2+1) - test->getValue(ix2-1,iy2+1) +
            test->getValue(ix2+1,iy2)   - test->getValue(ix2-1,iy2) +
            test->getValue(ix2+1,iy2-1) - test->getValue(ix2-1,iy2-1));
      d_dTestValueA_dy[numPointsAcquired] = (0.166666)*
           (test->getValue(ix2+1,iy2+1) - test->getValue(ix2+1,iy2-1) +
            test->getValue(ix2,iy2+1)   - test->getValue(ix2,iy2-1) +
            test->getValue(ix2-1,iy2+1) - test->getValue(ix2-1,iy2-1));
      numPointsAcquired++;
    }
  }

  d_sampleA_acquired = VRPN_TRUE;
  return 0;
}


int nmr_AlignerMI::buildSampleB(nmb_Image *ref, nmb_Image *test, 
                               nmb_Image *ref_z)
{
  // check for incorrect usage (programmer error)
  if ((ref_z == NULL && d_dimensionMode == REF_HEIGHTFIELD) || 
     (ref_z != NULL && d_dimensionMode == REF_2D)) {
     return -1;
  }

  // we can actually reference x positions from 0..width but we lose
  // information about intensity at the edges so we stay away from them
  double x, y, z = 0.0, x2, y2;
  int ix2, iy2;
  double xmin_ref, xmax_ref, ymin_ref, ymax_ref;
  double xmin_test, xmax_test, ymin_test, ymax_test;
  xmin_ref = 1;
  xmax_ref = ref->width() - 1;
  ymin_ref = 1;
  ymax_ref = ref->width() - 1;
  xmin_test = 1;
  xmax_test = test->height() - 2;
  ymin_test = 1;
  ymax_test = test->height() - 2;

  int numPointsAcquired = 0;

  // perhaps we should be sampling from integer coordinates in
  // ref and only interpolate in the test image but this is more symmetrical
  while (numPointsAcquired < d_sizeB) {
    x = nmr_Util::sampleUniformDistribution(xmin_ref, xmax_ref);
    y = nmr_Util::sampleUniformDistribution(ymin_ref, ymax_ref);
    if (ref_z) {
      z = ref_z->getValueInterpolated(x, y);
      x2 = transform_x(x, y, z);
      y2 = transform_y(x, y, z);
    } else {
      z = 0.0;
      x2 = transform_x(x, y);
      y2 = transform_y(x, y);
    }
    if (x2 > xmin_test && x2 <= xmax_test &&
        y2 >= ymin_test && y2 <= ymax_test) {
      d_Bx_ref[numPointsAcquired] = x;
      d_By_ref[numPointsAcquired] = y;
      d_Bz_ref[numPointsAcquired] = z;
      d_Bx_test[numPointsAcquired] = x2;
      d_By_test[numPointsAcquired] = y2;
      d_refValuesB[numPointsAcquired] = ref->getValueInterpolated(x,y);
      d_testValuesB[numPointsAcquired] = test->getValueInterpolated(x2,y2);
      ix2 = (int)floor(x2+0.5);
      iy2 = (int)floor(y2+0.5);
      // compute a cheap and small-aperture estimate of the gradient
      d_dTestValueB_dx[numPointsAcquired] = (0.166666)*
           (test->getValue(ix2+1,iy2+1) - test->getValue(ix2-1,iy2+1) +
            test->getValue(ix2+1,iy2)   - test->getValue(ix2-1,iy2) +
            test->getValue(ix2+1,iy2-1) - test->getValue(ix2-1,iy2-1));
      d_dTestValueB_dy[numPointsAcquired] = (0.166666)*
           (test->getValue(ix2+1,iy2+1) - test->getValue(ix2+1,iy2-1) +
            test->getValue(ix2,iy2+1)   - test->getValue(ix2,iy2-1) +
            test->getValue(ix2-1,iy2+1) - test->getValue(ix2-1,iy2-1));
      numPointsAcquired++;
    }
  }
  d_sampleB_acquired = VRPN_TRUE;
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
      double W = W_num/W_den;
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
    sum += log(density);
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

  double invTestVariance = 1.0/(d_sigmaTest*d_sigmaTest);
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
      double W = W_num/W_den;
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
    sum += log(density);
  }
  sum /= -1.0*(double)d_sizeB;
  entropy = sum;
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
    sum += log(density);
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

  double dv_dx_A, dv_dy_A, dv_dx_B, dv_dy_B;

  int i,j;
  for (i = 0; i < 8; i++) {
    dIdT[i] = 0.0;
  }
  for (i = 0; i < d_sizeB; i++) {
    for (j = 0; j < d_sizeA; j++) {
      // dTestIntensity_dx at (d_Ax_test, d_Ay_test)
      dv_dx_A = d_dTestValueA_dx[j];
      // dTestIntensity_dy at (d_Ax_test, d_Ay_test)
      dv_dy_A = d_dTestValueA_dy[j];
      // dTestIntensity_dx at (d_Bx_test, d_By_test)
      dv_dx_B = d_dTestValueB_dx[i];
      // dTestIntensity_dy at (d_Bx_test, d_By_test)
      dv_dy_B = d_dTestValueB_dy[i];

      dv_dT[0] = d_Bx_ref[j]*dv_dx_B - d_Ax_ref[i]*dv_dx_A;
      dv_dT[1] = d_By_ref[j]*dv_dx_B - d_Ay_ref[i]*dv_dx_A;
      dv_dT[2] = d_Bz_ref[j]*dv_dx_B - d_Az_ref[i]*dv_dx_A;
      dv_dT[3] = dv_dx_B - dv_dx_A;
      dv_dT[4] = d_Bx_ref[j]*dv_dy_B - d_Ax_ref[i]*dv_dy_A;
      dv_dT[5] = d_By_ref[j]*dv_dy_B - d_Ay_ref[i]*dv_dy_A;
      dv_dT[6] = d_Bz_ref[j]*dv_dy_B - d_Az_ref[i]*dv_dy_A;
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
      int k;
      double W_den = 0.0;
      double diff;
      for (k = 0; k < d_sizeA; k++) {
        // diff = v_i - v_k
        diff = d_testValuesB[i] - d_testValuesA[k];
        W_den += nmr_Gaussian::value(diff, invSigmaTest, 0.0);
      }
      // diff = v_i - v_j
      diff = d_testValuesB[i] - d_testValuesA[j];
      double W_num = nmr_Gaussian::value(diff, invSigmaTest, 0.0);
      double Wv = W_num/W_den;
 
      // code to compute Ww(w_i, w_j)
      W_den = 0.0;
      for (k = 0; k < d_sizeA; k++) {
        // w[0] = u_i - u_k
        // w[1] = v_i - v_k
        w[0] = d_refValuesB[i] - d_refValuesA[k];
        w[1] = d_testValuesB[i] - d_testValuesA[k];
        W_den += nmr_Gaussian::value(w[0], inv_sigma[0], mu[0]) *
                 nmr_Gaussian::value(w[1], inv_sigma[1], mu[1]);
      }
      // w[0] = u_i - u_j
      // w[1] = v_i - v_j
      w[0] = d_refValuesB[i] - d_refValuesA[j];
      w[1] = d_testValuesB[i] - d_testValuesA[j];
      W_num = nmr_Gaussian::value(w[0], inv_sigma[0], mu[0]) *
              nmr_Gaussian::value(w[1], inv_sigma[1], mu[1]);
      double Ww = W_num/W_den;
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
