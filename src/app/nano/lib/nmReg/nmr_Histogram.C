#include "nmr_Histogram.h"

nmr_Histogram::nmr_Histogram(int numDim, int dimSize):
  d_numDim(numDim),
  d_dimSize(new int[numDim]),
  d_stride(new int[numDim]),
  d_total(0)
{
  int i;
  d_stride[0] = 1;
  d_numBins = 1;
  for (i = 0; i < d_numDim; i++) {
    d_dimSize[i] = dimSize;
    if (i < d_numDim-1) {
      d_stride[i+1] = d_stride[i]*d_dimSize[i];
    }
    d_numBins *= d_dimSize[i];
  }

  d_data = new int[d_numBins];
  clear();
}

nmr_Histogram::nmr_Histogram(int numDim, int *dimSizes):
  d_numDim(numDim),
  d_dimSize(new int[numDim]),
  d_stride(new int[numDim]),
  d_total(0)
{
  int i;
  d_stride[0] = 1;
  d_numBins = 1;
  for (i = 0; i < d_numDim; i++) {
    d_dimSize[i] = dimSizes[i];
    if (i < d_numDim-1) {
      d_stride[i+1] = d_stride[i]*d_dimSize[i];
    }
    d_numBins *= d_dimSize[i];
  }

  d_data = new int[d_numBins];
  clear();
}

nmr_Histogram::~nmr_Histogram()
{
  delete [] d_dimSize;
  delete [] d_stride;
  delete [] d_data;
}

void nmr_Histogram::incrBin(int *bin)
{
  int index = computeIndex(bin);
  d_data[index]++;
  d_total++;
}

void nmr_Histogram::clear()
{
  int i;
  for (i = 0; i < d_numBins; i++) {
    d_data[i] = 0;
  }
  d_total = 0;
}

double nmr_Histogram::entropy()
{
  int i;
  double p;
  double normFactor = 1.0/(double)d_total;
  double result = 0;
  for (i = 0; i < d_numBins; i++) {
    if (d_data[i] != 0) {
      p = (double)d_data[i]*normFactor;
      result -= p*log(p);
    }
  }
  return result;
}

void nmr_Histogram::getImage(nmb_Image *image)
{
  if (!image) return;
  int w = image->width();
  int h = image->height();
  if (d_numDim != 2 || w != d_dimSize[0] || h != d_dimSize[1]) return;
  int index[2];
  for (index[0] = 0; index[0] < w; index[0]++) {
    for (index[1] = 0; index[1] < h; index[1]++) {
      image->setValue(index[0], index[1], d_data[computeIndex(index)]);
    }
  }
}

void nmr_Histogram::setImage(nmb_Image *image)
{
  if (!image) return;
  int w = image->width();
  int h = image->height();
  if (d_numDim != 2 || w != d_dimSize[0] || h != d_dimSize[1]) return;
  int index[2];
  for (index[0] = 0; index[0] < w; index[0]++) {
    for (index[1] = 0; index[1] < h; index[1]++) {
      float val = image->getValue(index[0], index[1]);
      d_data[computeIndex(index)] = (int)val;
    }
  }
}

int nmr_Histogram::computeIndex(int *bin)
{
  int result = 0;
  int i;
  for (i = 0; i < d_numDim; i++) {
    result += d_stride[i]*bin[i];
  }
  return result;
}
