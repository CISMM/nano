/* Gokul Varadhan
 * varadhan@cs.unc.edu
 * May 2001
 */
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "Uncertw.h"

// Unit is nm

Uncertw :: Uncertw (int _Xres, int _Yres, float _minx, float _maxx, float _miny, float _maxy, float _mincol, float _maxcol, float *_color) {
  set(_Xres, _Yres, _minx, _maxx, _miny, _maxy, _mincol, _maxcol, _color);
}


void Uncertw :: set (int _Xres, int _Yres, float _minx, float _maxx, float _miny, float _maxy, float _mincol, float _maxcol, float *_color) {
  Xres = _Xres;
  Yres = _Yres;
  minx = _minx;
  maxx = _maxx;
  miny = _miny;
  maxy = _maxy;
  mincol = _mincol;
  maxcol = _maxcol;
  color = _color;
}

void Uncertw :: writeUncertw(char *filename) {
  FILE *f = fopen(filename, "w");

  fprintf (f,"UNCA\n");
  fprintf (f,"%d %d\n",Xres, Yres);
  fprintf (f,"%f %f\n",minx, maxx);
  fprintf (f,"%f %f\n",miny, maxy);
  fprintf (f,"%f %f\n",mincol, maxcol);

  for (int j=0;j<Yres;j++) {
    for (int i=0;i<Xres;i++) {
      fprintf(f,"0 0 %f\n",color[Xres*j+i]);
    }
  }
  fclose(f);
}

