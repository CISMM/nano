/* Gokul Varadhan
 * coneSphere.h
 * Oct 2000
 */

#ifndef _TIPS_H_
#define _TIPS_H_

#include "ConeSphere.h"

class SphereTip {
public:
  double r;
  SphereTip(double _r);
  void print();
};

/* This is basically a cone with radius cr and height ch its tip facing down 
 * containing a sphere of radius r. We call it a InvConeSphereTip since it is 
 * basically an inverted ConeSphere (see ConeSphere.{h,c++}.
 *
 */
class InvConeSphereTip : public ConeSphere{
public:
  // angle is half cone angle. it should be in radians
  InvConeSphereTip(double _r, double _ch, double _angle);
  void draw();
};

#endif
