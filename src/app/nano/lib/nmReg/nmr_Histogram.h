#ifndef NMR_HISTOGRAM_H
#define NMR_HISTOGRAM_H

#include "nmb_Image.h"

class nmr_Histogram {
  public:
    nmr_Histogram(int numDim, int dimSize);
    nmr_Histogram(int numDim, int *dimSizes);
    ~nmr_Histogram();
    void incrBin(int *bin);
    void clear();
    double entropy();
    void getImage(nmb_Image *image);
    void setImage(nmb_Image *image);

  private:
    inline int computeIndex(int *bin);

    int d_numDim;

    int *d_dimSize;
    int *d_stride;

    // this is an array with (d_dimSize[0]*d_dimSize[1]...) elements
    int d_numBins;
    int *d_data;
    int d_total;
};

#endif
