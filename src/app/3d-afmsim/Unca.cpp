/* Gokul Varadhan
 * varadhan@cs.unc.edu
 * Dec 2000
 */
#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include "Unca.h"

// Unit is nm

Unca :: Unca(){}

Unca :: Unca (int _Xres, int _Yres, double _minx, double _maxx, double _miny, double _maxy, double _minz, double _maxz, double *_zHeight) {
  set(_Xres, _Yres, _minx, _maxx, _miny, _maxy, _minz, _maxz, _zHeight);
}


void Unca :: set (int _Xres, int _Yres, double _minx, double _maxx, double _miny, double _maxy, double _minz, double _maxz, double *_zHeight) {
  Xres = _Xres;
  Yres = _Yres;
  minx = _minx;
  maxx = _maxx;
  miny = _miny;
  maxy = _maxy;
  minz = _minz;
  maxz = _maxz;
  zHeight = _zHeight;
}

void Unca :: writeUnca(char *filename) {
  FILE *f = fopen(filename, "w");

  fprintf (f,"UNCA\n");
  fprintf (f,"%d %d\n",Xres, Yres);//** changed last two params from Xres, Yres
  fprintf (f,"%lf %lf\n",minx, maxx);
  fprintf (f,"%lf %lf\n",miny, maxy);
  fprintf (f,"%lf %lf\n",minz, maxz);

//*** test of enlarging image
  /*for (int j=0;j<Yres;j++) {
	for(int k = 1;k <=4;++k){//4 times 128 = 512
		for (int i=0;i<Xres;i++) {
			for(int u = 1;u <=4;++u){//4 times 128 = 512
				fprintf(f,"0 0 %lf\n",zHeight[Xres*j+i]);
			}
		}
	}
  }*/


  for (int j=0;j<Yres;j++) {
    for (int i=0;i<Xres;i++) {
      fprintf(f,"0 0 %lf\n",zHeight[Xres*j+i]);
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
