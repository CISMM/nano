#ifndef URAXIS_H
#define URAXIS_H

#include "URender.h"

class URAxis:public URender{
private:
public:
  URAxis();
  ~URAxis();
  int Render(void *userdata=NULL);
};

#endif
