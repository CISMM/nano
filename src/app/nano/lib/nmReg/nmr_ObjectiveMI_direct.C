#include "nmr_ObjectiveMI_direct.h"
#include "nmr_Util.h"

nmr_ObjectiveMI_direct::nmr_ObjectiveMI_direct():
  d_dimensionMode(REF_2D),
  d_testValue(NULL),
  d_gradX_test(NULL), d_gradY_test(NULL),
  d_refValue(NULL),
  d_refZ(NULL),
  d_testHistogram(NULL),
  d_refHistogram(NULL),
  d_jointHistogram(NULL),
  d_numRefBins(64),
  d_numTestBins(64)
{
  int dimSizes[2];
  dimSizes[0] = d_numRefBins;
  dimSizes[1] = d_numTestBins;
  d_refHistogram = new nmr_Histogram(1, d_numRefBins);
  d_testHistogram = new nmr_Histogram(1, d_numTestBins);
  d_jointHistogram = new nmr_Histogram(2, dimSizes);
}

nmr_ObjectiveMI_direct::~nmr_ObjectiveMI_direct()
{
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
    fprintf(stderr, "nmr_ObjectiveMI_direct: Warning: possible memory leak\n");
  }
}

void nmr_ObjectiveMI_direct::setDimensionMode(nmr_DimensionMode mode)
{
  d_dimensionMode = mode;
}

nmr_DimensionMode nmr_ObjectiveMI_direct::getDimensionMode()
{
  return d_dimensionMode;
}

void nmr_ObjectiveMI_direct::setReferenceValueImage(nmb_Image *ref) 
{
  d_refValue = ref;
  d_refValue->normalize();
}

void nmr_ObjectiveMI_direct::setTestValueImage(nmb_Image *test)
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
    const char *testImageName, *testUnitsName;
    testImageName = *(d_testValue->name());
    testUnitsName = *(d_testValue->unitsValue());
    int test_width = d_testValue->width();
    int test_height = d_testValue->height();
    char gradXTestImageName[256], gradXTestUnitsName[256];
    char gradYTestImageName[256], gradYTestUnitsName[256];
    sprintf(gradXTestImageName, "%s_gradx", testImageName);
    sprintf(gradYTestImageName, "%s_grady", testImageName);
    sprintf(gradXTestUnitsName, "d%s_dx", testUnitsName);
    sprintf(gradYTestUnitsName, "d%s_dy", testUnitsName);
    d_gradX_test = new nmb_ImageGrid(gradXTestImageName, gradXTestUnitsName,
                                     test_width, test_height);
    d_gradY_test = new nmb_ImageGrid(gradYTestImageName, gradYTestUnitsName,
                                     test_width, test_height);
    nmr_Util::createGradientImages(*d_testValue, *d_gradX_test, *d_gradY_test);
  }
}

void nmr_ObjectiveMI_direct::setReferenceZImage(nmb_Image *ref_z)
{
  d_refZ = ref_z;
}

double nmr_ObjectiveMI_direct::value(double *testFromReferenceTransform)
{
  d_testHistogram->clear();
  d_refHistogram->clear();
  d_jointHistogram->clear();

  int i, j;
  double x, y, z = 0.0, x2, y2;
  int intValues[2];
  for (i = 0, x = 0.5; i < d_refValue->width(); i++, x+=1.0) {
    for (j = 0, y = 0.5; j < d_refValue->height(); j++, y+=1.0) {
      if (d_dimensionMode == REF_HEIGHTFIELD) {
        z = d_refZ->getValueInterpolated(x,y);
        x2 = transform_x(x, y, z, testFromReferenceTransform);
        y2 = transform_y(x, y, z, testFromReferenceTransform);
      } else {
        x2 = transform_x(x, y, testFromReferenceTransform);
        y2 = transform_y(x, y, testFromReferenceTransform);
      }
      if (x2 >= 0 && x2 <= d_testValue->width() && 
          y2 >= 0 && y2 <= d_testValue->height()) {
        intValues[0] = (int)((d_refValue->getValue(i,j))*d_numRefBins);
        if (intValues[0] > d_numRefBins-1) {
          intValues[0] = d_numRefBins-1;
        }
        
        float testVal = d_testValue->getValueInterpolated(x2, y2);
        intValues[1] = (int)(testVal*d_numTestBins);
        if (intValues[1] > d_numTestBins-1) {
          intValues[1] = d_numTestBins-1;
        }

        d_refHistogram->incrBin(&(intValues[0]));
        d_testHistogram->incrBin(&(intValues[1]));
        d_jointHistogram->incrBin(intValues);
      }
    }
  }
  double refEntropy = d_refHistogram->entropy();
  double testEntropy = d_testHistogram->entropy();
  double jointEntropy = d_jointHistogram->entropy();
  double result = refEntropy + testEntropy - jointEntropy;
  return result;
}

void nmr_ObjectiveMI_direct::gradient(double *testFromReferenceTransform, 
                               double *gradMI)
{
}


void nmr_ObjectiveMI_direct::valueAndGradient(
                double *testFromReferenceTransform,
                double &valueMI, double *gradMI)
{
  valueMI = value(testFromReferenceTransform);
}

