#ifndef _UNCERTW_H_
#define _UNCERTW_H_

/* Write out uncertainty map */

#include <iostream>
using namespace std;

class Uncertw {
public:
  int Xres;
  int Yres;
  float minx;
  float maxx;
  float miny;
  float maxy;
  float mincol;
  float maxcol;
  float *color; /* a 2D static arrays is basically a single array stored in
		      row major order */

  Uncertw(int _Xres, int _Yres, float _minx, float _maxx, float _miny, float _maxy, float _mincol, float _maxcol, float *_color);
  void set(int _Xres, int _Yres, float _minx, float _maxx, float _miny, float _maxy, float _mincol, float _maxcol, float *_color);
  void writeUncertw(char *filename);
};

#endif
