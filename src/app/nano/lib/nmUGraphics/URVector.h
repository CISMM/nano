#ifndef URVECTOR_H
#define URVECTOR_H

#include "URender.h"
#include <GL/glut_UNC.h>

class URVector:public URender{
 public:
  URVector(double start_x=0, double start_y=0, double start_z=0,
	  double dir_x=0, double dir_y=0, double dir_z=1);
  ~URVector();
  int Render(void *userdata=NULL);
  void set(double start_x, double start_y, double start_z,
	  double dir_x, double dir_y, double dir_z);
 private:
  double d_start[3];
  double d_end[3];
  GLUquadricObj *d_quadric;
};

#endif
