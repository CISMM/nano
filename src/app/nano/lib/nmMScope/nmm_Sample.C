#include "nmm_Sample.h"

#include "nmm_MicroscopeRemote.h"


nmm_Sample::nmm_Sample (nmm_Microscope_Remote * m) :
    d_scope (m) {

}

// virtual
nmm_Sample::~nmm_Sample (void) {

}

nmm_SampleGrid::nmm_SampleGrid (nmm_Microscope_Remote * m) :
    nmm_Sample (m),
    d_xSize (1),
    d_ySize (1),
    d_xDistance (0.0f),
    d_yDistance (0.0f) {

  recomputeMeasure();
}

// virtual
nmm_SampleGrid::~nmm_SampleGrid (void) {

}

// virtual
void nmm_SampleGrid::sampleAt (float x, float y) {

  int i, j;
  float x0, y0;

  // TODO:  put some sort of marker in the stream so that
  // d_scope saves these up as a set rather than reporting them back
  // one-by-one.

  for (i = 0, x0 = x - (d_xMeasure / 2.0);
       i < d_xSize;
       i++, x0 += d_xDistance) {
    for (j = 0, y0 = y - (d_yMeasure / 2.0);
         j < d_ySize;
         j++, y0 += d_yDistance) {

      d_scope->ScanTo(x0, y0);

    }
  }

  // TODO:  put an end-of-sample-set marker in the stream

}

void nmm_SampleGrid::setGridSize (int x, int y) {

  d_xSize = x;
  d_ySize = y;

  recomputeMeasure();
}

void nmm_SampleGrid::setGridSpacing (float d) {

  d_xDistance = d;
  d_yDistance = d;

  recomputeMeasure();
}

void nmm_SampleGrid::recomputeMeasure (void) {

  // Side length of a grid = (# units - 1) x unit size

  d_xMeasure = (d_xSize - 1) * d_xDistance;
  d_yMeasure = (d_ySize - 1) * d_yDistance;

}
