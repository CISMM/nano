/*$Id$*/

/* Gokul Varadhan
 * coneSphere.h
 * Oct 2000
 */

#ifndef _CONESPHERE_H_
#define _CONESPHERE_H_

/* Description of a ConeSphere
 * Our object as expected is defined by a cone and a sphere. It is the frustum
 * we get when we embed the sphere within the cone 
 */

class ConeSphere {
 public:
  double cr; // cone base radius
  double ch; // cone height
  double r;  // radius of the embedded sphere
  double theta; // atan(cr/ch)
  double topRadius; // radius at the top of the frustum
  double topHeight; // height of the top of the frustum
  double sphereHeight; /* height of the embedded sphere centre measured from 
			* the base of the cone - here we do not enforce 
			* sphereHeight to be positive i.e the sphere centre 
			* could be below the base */
  // theta is half cone angle. it should be in radians
  ConeSphere() {}
  ConeSphere(double _r, double _ch, double _theta);
  void set(double _r, double _ch, double _theta);
  void set_r(double _r);
  void set_theta(double _theta);
  void draw();
  void uncert_draw();
  void print();
};

#endif
