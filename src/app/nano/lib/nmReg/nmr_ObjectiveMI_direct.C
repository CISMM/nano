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

  double deltaXRef, deltaYRef;
  double refSizeX, refSizeY;
  d_refValue->getAcquisitionDimensions(refSizeX, refSizeY);
  double testSizeX, testSizeY;
  d_testValue->getAcquisitionDimensions(testSizeX, testSizeY);
  deltaXRef = refSizeX/(d_refValue->width());
  deltaYRef = refSizeY/(d_refValue->width());
  double pixelsPerUnitXTest = (double)(d_testValue->width())/testSizeX;
  double pixelsPerUnitYTest = (double)(d_testValue->height())/testSizeY;
  int i, j;
  double minRefVal, maxRefVal;
  double minTestVal, maxTestVal;
  minRefVal = d_refValue->minValue();
  maxRefVal = d_refValue->maxValue();
  minTestVal = d_testValue->minValue();
  maxTestVal = d_testValue->maxValue();
  double rangeInvRef = (maxRefVal - minRefVal);
  if (rangeInvRef == 0) {
	rangeInvRef = 1.0;
  } else {
	rangeInvRef = 1.0/rangeInvRef;
  }
  double rangeInvTest = (maxTestVal - minTestVal);
  if (rangeInvTest == 0) {
	rangeInvTest = 1.0;
  } else {
	rangeInvTest = 1.0/rangeInvTest;
  }

  double temp;
  double x_pix, y_pix, z = 0.0;
  double x_world, y_world;
  double x_world2, y_world2;
  double x_pix2, y_pix2;
  int intValues[2];
  // XXX - this needs work - there is too much confusion in what the coordinates of a pixel are
  for (i = 0, x_pix = 0.5, x_world=deltaXRef*0.5; i < d_refValue->width(); i++, x_pix+=1.0, x_world+=deltaXRef) {
    for (j = 0, y_pix = 0.5, y_world=deltaYRef*0.5; j < d_refValue->height(); j++, y_pix+=1.0, y_world+=deltaYRef) {
      if (d_dimensionMode == REF_HEIGHTFIELD) {
        z = d_refZ->getValue(i, j);
        x_world2 = transform_x(x_world, y_world, z, testFromReferenceTransform);
        y_world2 = transform_y(x_world, y_world, z, testFromReferenceTransform);
      } else {
        x_world2 = transform_x(x_world, y_world, testFromReferenceTransform);
        y_world2 = transform_y(x_world, y_world, testFromReferenceTransform);
      }
	  x_pix2 = x_world2*pixelsPerUnitXTest;
	  y_pix2 = y_world2*pixelsPerUnitYTest;
      if (x_pix2 >= 0 && x_pix2 <= d_testValue->width() && 
          y_pix2 >= 0 && y_pix2 <= d_testValue->height()) {
		temp = (d_refValue->getValue(i,j) - minRefVal)*rangeInvRef;
        intValues[0] = (int)(temp*d_numRefBins);
        if (intValues[0] > d_numRefBins-1) {
          intValues[0] = d_numRefBins-1;
        }
    
        float testVal = d_testValue->getValueInterpolated(x_pix2, y_pix2);
		temp = (testVal - minTestVal)*rangeInvTest;
        intValues[1] = (int)(temp*d_numTestBins);
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

void nmr_ObjectiveMI_direct::getJointHistogramSize(int &numX, int &numY)
{
  numX = d_numRefBins;
  numY = d_numTestBins;
}

void nmr_ObjectiveMI_direct::getJointHistogramImage(nmb_Image *image)
{
  d_jointHistogram->getImage(image);
}
