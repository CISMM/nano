/* Gokul Varadhan
 * coneSphere.h
 * Oct 2000
 */

#include <iostream.h>
#include <math.h>               //math.h vs cmath
#include <GL/glut.h>
#include "Tips.h"

SphereTip :: SphereTip(double _r) {
  r = _r;
}

void SphereTip :: print() {
  cout << "r = " << r << endl;
}

// angle is half cone angle. it should be in radians
InvConeSphereTip :: InvConeSphereTip(double _r, double _ch, double _angle)
  : ConeSphere(_r,_ch,_angle) {}

void InvConeSphereTip :: draw() {
 static int firstTime = 1;
  static GLUquadricObj* qobj;
  if( firstTime ) { qobj = gluNewQuadric(); }
  
  glPushMatrix();
  gluQuadricDrawStyle( qobj, GLU_LINE );
  gluQuadricNormals( qobj, GLU_FLAT );

  glRotatef(180, 1.0, 0.0, 0.0 ); 
  double h = this->sphereHeight+this->r;
  glTranslatef(0,0,-h);

  // draw 
  gluCylinder( qobj, cr, topRadius, topHeight, 30, 10);
  // now go to the centre of the embedded sphere
  glTranslatef(0, 0, sphereHeight);
  gluSphere( qobj, r, 30, 30);
  glPopMatrix();
}

#if 0 // a simple test 
void main(int argc, char *argv[]) {
  SphereTip sTip = SphereTip(24.99);
  InvConeSphereTip icsTip = InvConeSphereTip(5,20,30);
  sTip.print();
  icsTip.print();
}
#endif

