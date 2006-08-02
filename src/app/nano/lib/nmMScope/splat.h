#ifndef SPLAT_H
#define SPLAT_H

class BCGrid;
class Point_results;
class Point_value;

float ** mkSplat (BCGrid *);
  ///< Initializes static storage for splat kernel.
int ptSplat (int *, BCGrid * grid, Point_results * result);
  ///< Assumes we want to splat "Topography" or "Height" from result into grid;
  ///< ignores first argument.
int ptSplat (BCGrid * grid, Point_value * value);
  ///< Splats an arbitrary dataset into the grid.

#endif  // SPLAT_H
