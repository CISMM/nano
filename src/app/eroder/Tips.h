/* The nanoManipulator and its source code have been released under the
 * Boost software license when nanoManipulator, Inc. ceased operations on
 * January 1, 2014.  At this point, the message below from 3rdTech (who
 * sublicensed from nanoManipulator, Inc.) was superceded.
 * Since that time, the code can be used according to the following
 * license.  Support for this system is now through the NIH/NIBIB
 * National Research Resource at cismm.org.

Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

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
