#include "BCRenderGrid.h"
#include "BCPlane.h"

// from BCGrid.C
static const double STANDARD_DEVIATIONS = 3.0;


BCRenderGrid::BCRenderGrid (BCGrid * sourceGrid,
                            short numX, short numY,
                            float scale) :
   BCGrid (sourceGrid),
   d_sourceGrid (sourceGrid),
   d_resampledNumX (numX),
   d_resampledNumY (numY),
   d_scale (scale),
   d_gaussian (NULL),
   d_gaussianNumX (-1),
   d_gaussianNumY (-1)
{
  BCPlane * plane;
  double max, deltaZ;
  double d;
  int x, y;

  if ((numX < sourceGrid->numX()) && (numY < sourceGrid->numY())) {
    decimate(numX, numY);

    // Normalize the values
    for (plane = head(); plane; plane = plane->next()) {
      max = plane->maxValue();  // Expensive to recompute!
      deltaZ = max - plane->minValue();

      for (x = 0; x < numX; x++) {
        for (y = 0; y < numY; y++) {

          d = plane->value(x, y);
          d = (max - d) * scale / deltaZ;
          plane->setValue(x, y, d);

        }
      }
    }

    // Cache a gaussian mask to be used in decimate_value().

    computeMask(numX, numY);

    // register callbacks
    // THESE should be registered on the source grid
    for (plane = sourceGrid->head(); plane; plane = plane->next()) {
      plane->add_callback(decimate_value, this);
    }

  } else {
    fprintf(stderr, "BCRenderGrid::BCRenderGrid:  "
                    "Can only be smaller than source grid.\n"
                    "  (Interpolation should be implemented later).\n");
    fprintf(stderr, "  Called with size (%d, %d) but source grid is "
                           "(%d, %d).\n",
            numX, numY, sourceGrid->numX(), sourceGrid->numY());
    return;
  }

}


BCRenderGrid::~BCRenderGrid (void) {

  // Free the gaussian mask - code copied from BCGrid::decimate()

  if (d_gaussian) {
    int i;
    for (i = -d_gaussianNumX; i <= d_gaussianNumX; i++)
      delete [] (d_gaussian[i] - d_gaussianNumX);
    delete [] (d_gaussian - d_gaussianNumX);
  }

}


// code copied from BCGrid::decimate()

void BCRenderGrid::computeMask (short num_x, short num_y) {

  double dx = ((double) numX() - 1.0) / (double) num_x;
  double dy = ((double) numY() - 1.0) / (double) num_y;

  d_gaussianNumX = (int) (STANDARD_DEVIATIONS * dx / 2.0);
  d_gaussianNumY = (int) (STANDARD_DEVIATIONS * dy / 2.0);
 
  d_gaussian = makeMask(d_gaussianNumX, d_gaussianNumY);

}


// static
void BCRenderGrid::decimate_value (BCPlane * sourcePlane,
                                   int x, int y, void * userdata) {
  BCRenderGrid * it;
  BCPlane * outputPlane;
  short minx, miny;
  short maxx, maxy;

  // Get the plane in the RenderGrid that corresponds to the plane
  // that changed in the Grid.

  it = (BCRenderGrid *) userdata;
  outputPlane = it->getPlaneByName(*sourcePlane->name());

  // HACK
  // Only compute once per scan line (delta-y?)
  if (y != it->numY() - 1) {
    return;
  }
fprintf(stderr, "In BCRenderGrid::decimate_value() at end of scan line %d.\n",
x);

  // Determine the locations that change as a result of this update.
  // rough approximation - use a square (even though we know the
  // mask isn't)

  minx = (x - it->d_gaussianNumX) * ((double) outputPlane->numX() /
                                     it->numX());
  if (minx < 0) {
    minx = 0;
  }
  //miny = (y - it->d_gaussianNumY) * ((double) outputPlane->numY() /
  //                                   it->numY());
  //if (miny < 0) {
    miny = 0;
  //}
  if (x != it->numX() - 1) {
    maxx = minx;
  } else {
    maxx = (x + it->d_gaussianNumX) * ((double) outputPlane->numX() /
                                       it->numX());
    if (maxx >= outputPlane->numX()) {
      maxx = outputPlane->numX();
    }
  }
  //maxy = (y + it->d_gaussianNumY) * ((double) outputPlane->numY() /
  //                                     it->numY());
  //if (maxy >= outputPlane->numY()) {
    maxy = outputPlane->numY();
  //}

  // Determine their values.

  decimateRegion (minx, miny, maxx, maxy,
                  sourcePlane, outputPlane,
                  outputPlane->numX(), outputPlane->numY(),
                  it->d_gaussianNumX, it->d_gaussianNumY, it->d_gaussian);

  // Normalize those values.

  for (x = minx; x <= maxx; x++) {
    for (y = miny; y <= maxy; y++) {

      // TODO

    }
  }

  // If this update changed the plane's min or max, flag that so
  // we can recompute normalization constants after the plane
  // finishes scanning.

  // TODO

  // Recompute normalization constants if the scan is complete.

  if (x == it->numX() - 1) {
 
    // TODO
 
  }

}


