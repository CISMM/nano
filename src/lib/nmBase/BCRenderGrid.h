#ifndef BC_RENDER_GRID_H
#define BC_RENDER_GRID_H

#include "BCGrid.h"

// BCRenderGrid
//
// Given a grid, uses BCGrid::decimate() or interpolate() to expand
// to a given size, attempting to normalize to the range [0, 1].
// Registers callbacks on all planes in the BCGrid so that as new
// data comes in to the BCGrid it is automatically
// filtered into the BCRenderGrid.

class BCRenderGrid : public BCGrid {

  public:

    // CONSTRUCTORS

    BCRenderGrid (BCGrid * sourceGrid,
                  short numX, short numY,
                  float scale = 1.0f);
      // "scale" determines the range of the data.
      // By default, data is normalized to [0, 1].
      // If scale is specified, data will be normalized to
      // [0, scale].

    ~BCRenderGrid (void);

    // ACCESSORS

    // MANIPULATORS

    // void setScale (float scale);

  protected:

    void computeMask (short numX, short numY);
      // Fill in d_gaussian, d_GaussianNum[XY] for a given extent.

  private:

    static void decimate_value (BCPlane * sourcePlane,
                                int x, int y, void * userdata);
      // Callback to be placed on planes in sourceGrid so that
      // our copies are updated when they change.

    BCGrid * d_sourceGrid;
    short d_resampledNumX;
    short d_resampledNumY;

    float d_scale;
      // Scaling to perform on source grid.
      // Currently fixed at compile time, but needs (?) to be made
      // variable;  maybe we have to resample everything from
      // d_sourceGrid?

    double ** d_gaussian;
    short d_gaussianNumX;
    short d_gaussianNumY;

};

#endif  // BC_RENDER_GRID_H

