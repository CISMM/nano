/*
optimize now code and callback function

 */


#ifndef OPTIMIZE_NOW_H
#define OPTIMIZE_NOW_H

extern void computeOptimizeMinMax (int type, int x0, int y0, int x1, int y1,
			   double *x_max_coord, double *y_max_coord);
int optimize_now_ReceiveNewPoint (void * _mptr, const Point_results * p);

#endif // OPTIMIZE_NOW_H
