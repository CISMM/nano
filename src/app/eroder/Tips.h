/*$Id$*/

/* Gokul Varadhan
 * coneSphere.h
 * Oct 2000
 */

#ifndef _TIPS_H_
#define _TIPS_H_

#include "ConeSphere.h"

#define SPHERE_TIP 0
#define INV_CONE_SPHERE_TIP 1

class SphereTip {
public:
  double r;
  SphereTip() {}
  SphereTip(double _r);
  void set(double _r);
  void set_r(double _r);
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
  InvConeSphereTip() {}
  InvConeSphereTip(double _r, double _ch, double _angle, int _tesselation);
  void set(double _r, double _ch, double _angle, int _tesselation);
  void set_r(double _r);
  void draw();
  int tesselation;
  int epoch;
};

class Tip {
public:
  int type;
  SphereTip *spTip;
  InvConeSphereTip *icsTip;
  Tip() {};
  Tip(SphereTip *, InvConeSphereTip *, int tesselation, int default_type);
  void set(SphereTip *, InvConeSphereTip *, int tesselation, int default_type);
  void change_tip_model();
  void inc_r();
  void dec_r();
  void inc_theta();
  void dec_theta();
};

#endif
