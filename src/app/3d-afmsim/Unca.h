#ifndef _UNCA_H_
#define _UNCA_H_

#include <iostream.h>

class Unca {
public:
  int Xres;
  int Yres;
  double minx;
  double maxx;
  double miny;
  double maxy;
  double minz;
  double maxz;
  double *zHeight; /* a 2D static arrays is basically a single array stored in
		      row major order */
  //  Unca(void) {}
  // will point to a height field.
  Unca();
  Unca(int _Xres, int _Yres, double _minx, double _maxx, double _miny, double _maxy, double _minz, double _maxz, double *_zHeight);
  void set(int _Xres, int _Yres, double _minx, double _maxx, double _miny, double _maxy, double _minz, double _maxz, double *_zHeight);
  void writeUnca(char *filename);
};


#endif
