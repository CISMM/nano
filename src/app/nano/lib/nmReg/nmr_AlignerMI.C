#include "nmr_AlignerMI.h"
#include "nmr_Util.h"

nmr_AlignerMI::nmr_AlignerMI():
  d_objective(NULL),
  d_refWidth(0), d_refHeight(0), d_testWidth(0), d_testHeight(0)
{
}

nmr_AlignerMI::~nmr_AlignerMI()
{
  if (d_objective) {
    delete d_objective;
  }
}

void nmr_AlignerMI::initImages(nmb_Image *ref, nmb_Image *test, 
                               nmb_Image *ref_z, nmb_ImageList *monitorList)
{
  if (d_objective) {
    delete d_objective;
  }
  d_objective = new nmr_MultiResObjectiveMI_direct();

  d_objective->setReferenceValueImage(ref, monitorList);
  d_objective->setTestValueImage(test, monitorList);
  d_objective->setReferenceZImage(ref_z, monitorList);
  if (ref) {
    d_refWidth = ref->width();
    d_refHeight = ref->height();
  }
  if (test) {
    d_testWidth = test->width();
    d_testHeight = test->height();
  }
}

void nmr_AlignerMI::initImages(nmb_Image *ref, nmb_Image *test,
                               int numLevels, float *stddev,
                               nmb_Image *ref_z, nmb_ImageList *monitorList)
{
  if (d_objective) {
    delete d_objective;
  }
  d_objective = new nmr_MultiResObjectiveMI_direct(numLevels, stddev);

  d_objective->setReferenceValueImage(ref, monitorList);
  d_objective->setTestValueImage(test, monitorList);
  d_objective->setReferenceZImage(ref_z, monitorList);
  if (ref) {
    d_refWidth = ref->width();
    d_refHeight = ref->height();
  }
  if (test) {
    d_testWidth = test->width();
    d_testHeight = test->height();
  }
}

/*
void nmr_AlignerMI::optimizeVarianceParameters()
{
 int level;
 double default_rate = 1;
 double rate;

 for (level = 0; level < d_objective->numLevels(); level++) {
  double dH_dsigma_ref, dH_dsigma_test, dHc_dsigma_ref, dHc_dsigma_test;
  double refEntropy, testEntropy, crossEntropy;
  double sigmaRef, sigmaTest, sigmaRefRef, sigmaTestTest;

  d_objective->getRefVariance(level, sigmaRef);
  d_objective->getTestVariance(level, sigmaTest);
  d_objective->getCovariance(level, sigmaRefRef, sigmaTestTest);

  int i;
  printf("optimizing variance for reference image:\n");
  rate = default_rate;
  for (i = 0; i < 100; i++) {
    d_objective->refEntropyGradient(level, dH_dsigma_ref);
    d_objective->refEntropy(level, refEntropy);
    printf("%g->%g, %g\n", sigmaRef, refEntropy, dH_dsigma_ref);
    if (dH_dsigma_ref < 0 && rate > -0.1*sigmaRef/dH_dsigma_ref) {
      rate = -0.1*sigmaRef/dH_dsigma_ref;
    }
    sigmaRef += rate*dH_dsigma_ref;
    d_objective->setRefVariance(level, sigmaRef);
  }

  printf("optimizing variance for test image:\n");
  rate = default_rate;
  for (i = 0; i < 100; i++) {
    d_objective->testEntropyGradient(level, dH_dsigma_test);
    d_objective->testEntropy(level, testEntropy);
    printf("%g->%g, %g\n", sigmaTest, testEntropy, dH_dsigma_test);
    if (dH_dsigma_test < 0 && rate > -0.1*sigmaTest/dH_dsigma_test) {
      rate = -0.1*sigmaTest/dH_dsigma_test;
    }
    sigmaTest += rate*dH_dsigma_test;
    d_objective->setTestVariance(level, sigmaTest);
  }

  printf("optimizing covariance for joint histogram:\n");
  rate = default_rate;
  for (i = 0; i < 100; i++) {
    d_objective->crossEntropyGradient(level, dHc_dsigma_ref, dHc_dsigma_test);
    d_objective->crossEntropy(level, crossEntropy);
    printf("(%g,%g)->%g, (%g,%g)\n", sigmaRefRef, sigmaTestTest, 
                           crossEntropy, dHc_dsigma_ref, dHc_dsigma_test);
    if (dHc_dsigma_test < 0 && rate > -0.1*sigmaTestTest/dHc_dsigma_test) {
      rate = -0.1*sigmaTestTest/dHc_dsigma_test;
    }
    if (dHc_dsigma_ref < 0 && rate > -0.1*sigmaRefRef/dHc_dsigma_ref) {
      rate = -0.1*sigmaRefRef/dHc_dsigma_ref;
    }
    sigmaRefRef += rate*dHc_dsigma_ref;
    sigmaTestTest += rate*dHc_dsigma_test;
    d_objective->setCovariance(level, sigmaRefRef, sigmaTestTest);
  }
 }
}
*/

void nmr_AlignerMI::takeGradientSteps(int resolutionIndex, int numSteps,
                                      float stepSize)
{
  double imageSizeFactor = 0.5*(d_testWidth + d_testHeight);
  double translationRate = stepSize; // used to be hardcoded as 0.0001
  // now we set the other rates to be approximately commensurate with
  // translation by taking into consideration the sizes of translations that
  // occur in the image as a result of changing each parameter
  // the rates are such that the maximum offset that occurs due to any
  // rotation, scaling, or shearing changes is approximately equal to the
  // offset introduced by a translation
  double rotationRate = translationRate/imageSizeFactor;
  double scalingRate = translationRate/imageSizeFactor;
  double shearingRate = translationRate/imageSizeFactor;

  // the MI code computes the mutual information and its gradient in 
  // terms of 4x4 matrix elements
  double valueMI, gradMI[16];

  // we want to work with the gradient in the coordinates of translation,
  // rotation, scale, and shear
  double dMI_dRotZ, dMI_dTransX, dMI_dTransY;
  double dMI_dScaleX, dMI_dScaleY, dMI_dShearZ;

  // derivatives for transforming the gradient from being in terms of matrix
  // elements to being in terms of rotation, translation, scale, and shear
  double dTransform_dRotZ[16];
  double dTransform_dTransX[16], dTransform_dTransY[16];
  double dTransform_dScaleX[16], dTransform_dScaleY[16];
  double dTransform_dShearZ[16];

  // current transformation matrix
  double xform_matrix[16];
  double tx, ty, phi, shz, scx, scy;
  double cx, cy, cz;
  d_transform.getCenter(cx, cy, cz);
  printf("transform center: %g, %g, %g\n", cx, cy, cz);

  int i, j;
  for (i = 0; i < numSteps; i++) {
    d_transform.getMatrixDerivative(dTransform_dRotZ, NMB_ROTATE_Z);
    d_transform.getMatrixDerivative(dTransform_dTransX, NMB_TRANSLATE_X);
    d_transform.getMatrixDerivative(dTransform_dTransY, NMB_TRANSLATE_Y);
    d_transform.getMatrixDerivative(dTransform_dScaleX, NMB_SCALE_X);
    d_transform.getMatrixDerivative(dTransform_dScaleY, NMB_SCALE_Y);
    d_transform.getMatrixDerivative(dTransform_dShearZ, NMB_SHEAR_Z);
    d_transform.getMatrix(xform_matrix);
    d_objective->valueAndGradient(resolutionIndex,xform_matrix,valueMI,gradMI);
    phi = d_transform.getRotation(NMB_Z);
    tx = d_transform.getTranslation(NMB_X);
    ty = d_transform.getTranslation(NMB_Y);
    scx = d_transform.getScale(NMB_X);
    scy = d_transform.getScale(NMB_Y);
    shz = d_transform.getShear(NMB_Z);
    printf("(tx,ty,phi,shz,scx,scy,MI)= %g,%g,%g,%g,%g,%g, %g\n",
             tx,ty,phi,shz,scx,scy,valueMI);

    // convert the gradient using the chain rule
    dMI_dRotZ = 0; dMI_dTransX = 0; dMI_dTransY = 0;
    dMI_dScaleX = 0; dMI_dScaleY = 0; dMI_dShearZ = 0;
    for (j = 0; j < 16; j++) {
      dMI_dRotZ += dTransform_dRotZ[j]*gradMI[j];
      dMI_dTransX += dTransform_dTransX[j]*gradMI[j];
      dMI_dTransY += dTransform_dTransY[j]*gradMI[j];
      dMI_dScaleX += dTransform_dScaleX[j]*gradMI[j];
      dMI_dScaleY += dTransform_dScaleY[j]*gradMI[j];
      dMI_dShearZ += dTransform_dShearZ[j]*gradMI[j];
    }

    // take a step using the computed gradient and the rate factors
    d_transform.rotate(NMB_Z, dMI_dRotZ*rotationRate);
    d_transform.translate(dMI_dTransX*translationRate,
                          dMI_dTransY*translationRate, 0);
    d_transform.addScale(dMI_dScaleX*scalingRate,
                         dMI_dScaleY*scalingRate, 0);
    d_transform.shear(NMB_Z, dMI_dShearZ*shearingRate);
  }

  d_transform.getMatrix(xform_matrix);
  d_objective->valueAndGradient(resolutionIndex,xform_matrix,valueMI,gradMI);
  phi = d_transform.getRotation(NMB_Z);
  tx = d_transform.getTranslation(NMB_X);
  ty = d_transform.getTranslation(NMB_Y);
  scx = d_transform.getScale(NMB_X);
  scy = d_transform.getScale(NMB_Y);
  shz = d_transform.getShear(NMB_Z);
  printf("(tx,ty,phi,shz,scx,scy,MI)= %g,%g,%g,%g,%g,%g, %g\n",
           tx,ty,phi,shz,scx,scy,valueMI);

}

void nmr_AlignerMI::optimizeTransform()
{
  // start out at the lowest resolution
  int level = d_objective->numLevels()-1;
  double valueMI, gradMI[16];
  double xform_matrix[16];
  double dTransform_dRotZ[16];
  double dTransform_dTransX[16], dTransform_dTransY[16];
  double dTransform_dScaleX[16], dTransform_dScaleY[16];
  double dTransform_dShearZ[16];
  double dMI_dRotZ, dMI_dTransX, dMI_dTransY;
  double dMI_dScaleX, dMI_dScaleY, dMI_dShearZ;
  double imageSizeFactor = 0.5*(d_testWidth + d_testHeight);
  double translationRate = 0.0001;
  // now we set the other rates to be approximately commensurate with
  // translation by taking into consideration the sizes of translations that
  // occur in the image as a result of changing each parameter
  double rotationRate = translationRate/imageSizeFactor;
  double scalingRate = translationRate/imageSizeFactor;
  double shearingRate = translationRate/imageSizeFactor;
  int i,j;
  double rot, tx, ty, sx, sy, sh;

  FILE *outf = fopen("transformOptimizerOutput.txt", "w");

  for (level = d_objective->numLevels()-1; level >= 0; level--) {
    fprintf(outf, "level %d:\n", level);
    printf("level %d:", level);
    fprintf(outf, "rot, tx, ty, sx, sy, sh, MI, dMI_drot, dMI_dtx, dMI_dty, "
                  "dMI_dsx, dMI_dsy, dMI_dsh\n");
    for (i = 0; i < 100; i++) {
      d_transform.getMatrixDerivative(dTransform_dRotZ, NMB_ROTATE_Z);
      d_transform.getMatrixDerivative(dTransform_dTransX, NMB_TRANSLATE_X);
      d_transform.getMatrixDerivative(dTransform_dTransY, NMB_TRANSLATE_Y);
      d_transform.getMatrixDerivative(dTransform_dScaleX, NMB_SCALE_X);
      d_transform.getMatrixDerivative(dTransform_dScaleY, NMB_SCALE_Y);
      d_transform.getMatrixDerivative(dTransform_dShearZ, NMB_SHEAR_Z);
      d_transform.getMatrix(xform_matrix);
      d_objective->valueAndGradient(level, xform_matrix, valueMI, gradMI);
      dMI_dRotZ = 0; dMI_dTransX = 0; dMI_dTransY = 0;
      dMI_dScaleX = 0; dMI_dScaleY = 0; dMI_dShearZ = 0;
      for (j = 0; j < 16; j++) {
        dMI_dRotZ += dTransform_dRotZ[j]*gradMI[j];
        dMI_dTransX += dTransform_dTransX[j]*gradMI[j];
        dMI_dTransY += dTransform_dTransY[j]*gradMI[j];
        dMI_dScaleX += dTransform_dScaleX[j]*gradMI[j];
        dMI_dScaleY += dTransform_dScaleY[j]*gradMI[j];
        dMI_dShearZ += dTransform_dShearZ[j]*gradMI[j];
      }
      rot = d_transform.getRotation(NMB_Z);
      tx = d_transform.getTranslation(NMB_X);
      ty = d_transform.getTranslation(NMB_Y);
      sx = d_transform.getScale(NMB_X);
      sy = d_transform.getScale(NMB_Y);
      sh = d_transform.getShear(NMB_Z);
      fprintf(outf, "%g,%g,%g,%g,%g,%g, %g, %g,%g,%g,%g,%g,%g\n", 
         rot, tx, ty, sx, sy, sh, valueMI,
         dMI_dRotZ, dMI_dTransX, dMI_dTransY, 
         dMI_dScaleX, dMI_dScaleY, dMI_dShearZ);
      d_transform.rotate(NMB_Z, dMI_dRotZ*rotationRate);
      d_transform.translate(dMI_dTransX*translationRate, 
                          dMI_dTransY*translationRate, 0);
      d_transform.addScale(dMI_dScaleX*scalingRate, 
                         dMI_dScaleY*scalingRate, 0);
      d_transform.shear(NMB_Z, dMI_dShearZ*shearingRate);
      if (i%20 == 0) printf(".");
    }
    printf("\n");
    // reduce step size (this is a form of annealing)
    rotationRate *= 0.5;
    translationRate *= 0.5;
    scalingRate *= 0.5;
    shearingRate *= 0.5;
  }
  fclose(outf);
}

void nmr_AlignerMI::setTransform(nmb_Transform_TScShR &xform)
{
  d_transform = xform;
}

void nmr_AlignerMI::getTransform(nmb_Transform_TScShR &xform)
{
  xform = d_transform;
}

/* multiResPatternSearch:
   maintain an array of flags that indicates at which levels we are
   stuck in a local minimum; before calling the patternSearch() routine
   at any one level, make sure that we are stuck at all coarser levels 
*/
void nmr_AlignerMI::multiResPatternSearch(
         int maxIterations, float translationStepSize)
{
  int i;
  int numLevels = d_objective->numLevels();
  vrpn_bool *patternSearchMinimumReached = new vrpn_bool[numLevels];
  float *std_dev = new float[numLevels];
  d_objective->getBlurStdDev(std_dev);
  vrpn_bool exhaustedAllLevels = vrpn_FALSE;
  int currLevel = 0; //getLevelByScaleOrder(currLevel);

  for (i = 0; i < numLevels; i++) {
    patternSearchMinimumReached[i] = vrpn_FALSE;
  }
  
  while (!exhaustedAllLevels) {
    //currLevel = 0; starting over like this can get us into infinite loops
    while (currLevel < numLevels &&
           patternSearchMinimumReached[currLevel]) {
      currLevel++;
    }
    if (currLevel == numLevels) {
      exhaustedAllLevels = vrpn_TRUE;
    } else {
      int levelIndex = d_objective->getLevelByScaleOrder(currLevel);
      int numIterations = patternSearch(
                          levelIndex,
                          maxIterations, translationStepSize,
                          0.1*translationStepSize);
      printf("performed %d iterations at scale %g\n",
             numIterations, std_dev[levelIndex]);
      if (numIterations == 0) {
        patternSearchMinimumReached[currLevel] = vrpn_TRUE;
      } else {
        for (i = 0; i < numLevels; i++) {
          patternSearchMinimumReached[i] = vrpn_FALSE;
        }
        if (numIterations < maxIterations) {
           patternSearchMinimumReached[currLevel] = vrpn_TRUE;
        }
      }
    }
  }
  delete [] patternSearchMinimumReached;
  delete [] std_dev;
}

int nmr_AlignerMI::patternSearch(int resolutionIndex, int maxIterations,
                                  float translationStepSize,
                                  float minTranslationStepSize)
{
  double imageSizeFactor = 0.5*(d_testWidth + d_testHeight);
  double translationStep = translationStepSize;
  // we set the other rates to be approximately commensurate with
  // translation by taking into consideration the sizes of translations that
  // occur in the image as a result of changing each parameter
  // the rates are such that the maximum offset that occurs due to any
  // rotation, scaling, or shearing changes is approximately equal to the
  // offset introduced by a translation
  double rotationStep = translationStep/imageSizeFactor;
  double scalingStep = translationStep/imageSizeFactor;
  double shearingStep = translationStep/imageSizeFactor;
  double stepReductionFactor = 0.5;

  const int numParameters = 6;
  nmb_TransformParameter parameterTypes[numParameters] = 
        {NMB_ROTATE_Z, NMB_TRANSLATE_X,
         NMB_TRANSLATE_Y, NMB_SCALE_X, NMB_SCALE_Y, NMB_SHEAR_Z};
  double patternVector[numParameters] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  double exploreVector[numParameters] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  double stepSize[numParameters] = 
             {rotationStep, translationStep, translationStep,
              scalingStep, scalingStep, shearingStep};

  nmb_Transform_TScShR testTransform;
  double xform_matrix[16];
  d_transform.getMatrix(xform_matrix);

  double maxObjective = d_objective->value(resolutionIndex, xform_matrix);
  double testObjective;

  int i, j, k;
  double paramVal;
  testTransform = d_transform;
 
  int numIterations = 0; 

  while (numIterations < maxIterations && 
         translationStep > minTranslationStepSize) {
    for (j = 0; j < numParameters; j++) {
      paramVal = testTransform.getParameter(parameterTypes[j]);
      paramVal += stepSize[j];
      testTransform.setParameter(parameterTypes[j], paramVal);
      testTransform.getMatrix(xform_matrix);
      testObjective = d_objective->value(resolutionIndex, xform_matrix);     
      if (testObjective > maxObjective) {
        maxObjective = testObjective;
        d_transform = testTransform;
        exploreVector[j] = 1.0;
      } else {
        exploreVector[j] = 0.0;
      }
      paramVal -= 2*stepSize[j];
      testTransform.setParameter(parameterTypes[j], paramVal);
      testTransform.getMatrix(xform_matrix);
      testObjective = d_objective->value(resolutionIndex, xform_matrix);
      if (testObjective > maxObjective) {
        maxObjective = testObjective;
        d_transform = testTransform;
        exploreVector[j] = -1.0;
      } else {
        testTransform.setParameter(parameterTypes[j], 
                                   d_transform.getParameter(parameterTypes[j]));
      }
    }
    // update pattern vector
    printf("exploreVector: (");
    vrpn_bool zeroExploreVector = vrpn_TRUE;
    for (j = 0; j < numParameters; j++) {
      if (exploreVector[j] != 0.0) {
        patternVector[j] += exploreVector[j];
        zeroExploreVector = vrpn_FALSE;
      }
      printf("%g,", exploreVector[j]);
    }
    printf(")\n");

    if (zeroExploreVector) {
      printf("zero explore vector found, reducing step size\n");
      for (j = 0; j < numParameters; j++) {
        stepSize[j] *= stepReductionFactor;
        patternVector[j] = 0.0;
      }
      translationStep *= stepReductionFactor;
    } else {
      // pattern move
      for (j = 0; j < numParameters; j++) {
        paramVal = testTransform.getParameter(parameterTypes[j]);
        paramVal += patternVector[j]*stepSize[j];
        testTransform.setParameter(parameterTypes[j], paramVal);
      }
      testTransform.getMatrix(xform_matrix);
      testObjective = d_objective->value(resolutionIndex, xform_matrix);
      if (testObjective > maxObjective) {
        printf("pattern move succeeded\n");
        maxObjective = testObjective;
        d_transform = testTransform;
      } else {
        printf("pattern move failed, resetting to 0\n");
        for (j = 0; j < numParameters; j++) {
          patternVector[j] = 0.0;
        }
      }
      printf("patternVector: (");
      for (j = 0; j < numParameters; j++) {
        printf("%g,", patternVector[j]);
      }
      printf(")\n");
      numIterations++;
    }
  }

  return numIterations;
}

void nmr_AlignerMI::plotObjective(FILE *outf)
{
  int level;
  int paramIndex;
  int startOffset = -10;
  int numSteps = 21;
  double paramVal;

  int numParameters = 6;
  nmb_TransformParameter parameterTypes[6] = {NMB_ROTATE_Z, NMB_TRANSLATE_X,
         NMB_TRANSLATE_Y, NMB_SCALE_X, NMB_SCALE_Y, NMB_SHEAR_Z};
  char *paramNames[6] = {"rotate_z", "translate_x", "translate_y",
                         "scale_x", "scale_y", "shear_z"};
  char *paramDerivNames[6] = {"dMI_drotZ", "dMI_dtranX", "dMI_dtranY",
                         "dMI_dscX", "dMI_dscY", "dMI_dshZ"};
  double deltaParamZeroScale[6] = {0.001, 0.1, 0.1, 1.002, 1.002, 0.003};
  double deltaParam[6];
  double paramGradient[6];

  double valueMI;
  double gradient[16];
  double dTransform_dParam[16];
  double transformMatrix[16];

  vrpn_bool outputToDisk = (outf != stdout && outf != stderr);

  int i,j,k;
  nmb_Transform_TScShR test = d_transform;
  vrpn_bool repeatNeg = vrpn_FALSE;

  for (level = d_objective->numLevels()-1; level >= 0; level--) {
    fprintf(outf, "level %d:\n", level);
    if (outputToDisk) {
      printf("level %d:\n", level);
    }
    for (paramIndex = 0; paramIndex < numParameters; paramIndex++) {
      fprintf(outf, "%s, MI", paramNames[paramIndex]);
      for (j = 0; j < numParameters; j++) {
        fprintf(outf, ", %s", paramDerivNames[j]);
      }
      fprintf(outf, "\n");

      if (outputToDisk) {
        printf("%s, MI", paramNames[paramIndex]);
        for (j = 0; j < numParameters; j++) {
          printf(", %s", paramDerivNames[j]);
        }
        printf("\n");
      }

      test = d_transform;
      paramVal = test.getParameter(parameterTypes[paramIndex]);
      if (parameterTypes[paramIndex] == NMB_SCALE_X ||
          parameterTypes[paramIndex] == NMB_SCALE_Y ||
          parameterTypes[paramIndex] == NMB_SCALE_Z) {
        deltaParam[paramIndex] = deltaParamZeroScale[paramIndex];
        for (i = 0; i < level; i++) deltaParam[paramIndex] *= 1.0+
                2.0/((double)d_refWidth);
        paramVal *= pow(deltaParam[paramIndex], startOffset);
      } else {
        deltaParam[paramIndex] = deltaParamZeroScale[paramIndex];
        for (i = 0; i < level; i++) deltaParam[paramIndex] *= 2.0;
        paramVal += startOffset*deltaParam[paramIndex];
      }
      for (i = 0; i < numSteps; i++) {
        if (parameterTypes[paramIndex] == NMB_SCALE_X ||
          parameterTypes[paramIndex] == NMB_SCALE_Y ||
          parameterTypes[paramIndex] == NMB_SCALE_Z) {
          paramVal *= deltaParam[paramIndex];
        } else {
          paramVal += deltaParam[paramIndex];
        }
        test.setParameter(parameterTypes[paramIndex], paramVal);
        test.getMatrix(transformMatrix);
        do {
          d_objective->valueAndGradient(level,transformMatrix,valueMI,gradient);
          // transform the gradient so its in terms of the parameters we vary
          for (j = 0; j < numParameters; j++) {
            test.getMatrixDerivative(dTransform_dParam, parameterTypes[j]);
            paramGradient[j] = 0.0;
            for (k = 0; k < 16; k++) {
              paramGradient[j] += dTransform_dParam[k]*gradient[k];
            }
          }

          fprintf(outf, "%g, %g", paramVal, valueMI);
          for (j = 0; j < numParameters; j++) {
            fprintf(outf, ", %g", paramGradient[j]);
          }
          fprintf(outf, "\n");

          if (outputToDisk) {
            printf("%g, %g", paramVal, valueMI);
            for (j = 0; j < numParameters; j++) {
              printf(", %g", paramGradient[j]);
            } 
            printf("\n");
          }
        } while (repeatNeg && valueMI < 0); 
                 // repeat this to make sure its only random
      }
    }
  }
}

int nmr_AlignerMI::getNumLevels()
{
  if (d_objective) {
    return d_objective->numLevels();
  }
  return 0;
}

void nmr_AlignerMI::getBlurStdDev(float *stddev)
{
  if (d_objective) {
    d_objective->getBlurStdDev(stddev);
  }
}

/*
void nmr_AlignerMI::setRefFeaturePoints(int numPnts, double *x, double *y)
{
  if (d_objective) {
    d_objective->setRefFeaturePoints(numPnts, x, y);
  }
}
*/
