#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include "Unca.h"

Unca :: Unca (int _Xres, int _Yres, double _minx, double _maxx, double _miny, double _maxy, double _minz, double _maxz, double *_zHeight) {
  set(_Xres, _Yres, _minx, _maxx, _miny, _maxy, _minz, _maxz, _zHeight);
}

void Unca :: set (int _Xres, int _Yres, double _minx, double _maxx, double _miny, double _maxy, double _minz, double _maxz, double *_zHeight) {
  Xres = _Xres;
  Yres = _Yres;
  // input is in Angstroms. need to convert to nm.
  minx = 10*_minx;
  maxx = 10*_maxx;
  miny = 10*_miny;
  maxy = 10*_maxy;
  minz = 10*_minz;
  maxz = 10*_maxz;
  zHeight = _zHeight;
}

void Unca :: writeUnca(char *filename) {
  FILE *f = fopen(filename, "w");

  fprintf (f,"UNCA\n");
  fprintf (f,"%d %d\n",Xres, Yres);
  fprintf (f,"%lf %lf\n",minx, maxx);
  fprintf (f,"%lf %lf\n",miny, maxy);
  fprintf (f,"%lf %lf\n",minz, maxz);

  fprintf(f,"\n");

  for (int j=0;j<Yres;j++) {
    for (int i=0;i<Xres;i++) {
      // input is in Angstroms. need to convert to nm.
      //cout << 0 << " " << 0 << " " << 10*zHeight[Xres*j+i] << endl;
      fprintf(f,"0 0 %lf\n",10*zHeight[Xres*j+i]);
    }
  }
  fclose(f);
}

#if 0
void main() {
  double zh[30][30];
  zh[0][0] = 11;
  //  Unca u = Unca(6, 10,0.,7.,0.,9.,0.,11.,(double **)zh);
  //  cout << "zHeight =" << (int) zh << endl;
  //  cout << "zHeight =" << **zh << endl;
  
  cout << "zHeight =" << (int)zh << endl;
  trial((double *)zh);

  //  Unca u = Unca();
  //  u.writeUnca("try.out");
}
#endif
