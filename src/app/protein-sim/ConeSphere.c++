/* Gokul Varadhan
 * ConeSphere.c++
 * Oct 2000
 */

#include <iostream.h>
#include <math.h>		//math.h vs cmath
#include <GL/glut.h>
#include "ConeSphere.h"

// _theta is in degrees
ConeSphere :: ConeSphere(double _r, double _ch, double _theta) {
  double t;
  r = _r;
  ch = _ch;
  theta = _theta;
  cr = ch * tan(theta);
  // now calculate topRadius and topHeight;
  topRadius = r*cos(theta);
  t = cr - r*cos(theta);
  topHeight = (ch*t)/cr;
  sphereHeight = ch - r/sin(theta);
}

extern GLuint list_sphere, list_cylinder;
extern void drawSphere(double);


/* Draws the ConeSphere with the axis along Z axis with the base of the 
 * cylinder in X-Y plane. The centre of the base of the cylinder is at (0,0)
 */
void ConeSphere :: draw() {
  static int firstTime = 1;
  static GLUquadricObj* qobj, *qobj2;
  if( firstTime ) { qobj = gluNewQuadric(); firstTime=0;}
  
  glPushMatrix();
#define PROFILE 0
  /* to profile adv due to display lists, we dont draw cylinder */
#if ~PROFILE 
  gluQuadricDrawStyle( qobj, GLU_FILL );
  gluQuadricNormals( qobj, GLU_FLAT );
  // draw  (can't put this on a display list because, topRadius is computed at runtime. Therefore can't do a simple scaling of a precompile time frustum.
  gluCylinder( qobj, cr, topRadius, topHeight, 30, 10);
#endif
  
  // now go to the centre of the embedded sphere
  glTranslatef(0, 0, sphereHeight);
#if PROFILE
  glScalef(r,r,r);
  glCallList(list_sphere);
#else
  gluSphere( qobj, r, 30, 30);
#endif
  glPopMatrix();
}

#define PI            3.14159265358979323846
#define DEG_TO_RAD (PI/180.)
#define RAD_TO_DEG	(180. / PI)
void ConeSphere :: print() {
  cout << "cr= " << cr << " ch= " << ch << " r= " << r << " theta = " << (RAD_TO_DEG*theta) << " deg" << " topRadius= " << topRadius << " topHeight= " << topHeight << " sphereHeight= " << sphereHeight << endl;
}

#if 0 // simple test for the constructor

void main(int argc, char *argv[]) {
  /* special case - a slice of cone through centre is an equiv triangle of
   * side 10. That means the base radius is 5 and height is sqrt(75).
   * Also radius of in-sphere is sqrt(75)/3 
   * In this special case, the in-sphere sits just inside the cone on the
   * base.
   * In this case, we should obtain 
   * topRadius = [sqrt(75)/3]*cos(60) = 2.5 , topHeight = 5 * sin(60) = 4.330
   * sphereHeight = radius of in-sphere = sqrt(75)/3 = 2.88675 (note in-sphere
   * is sitting on the base
   */
  //  ConeSphere c = ConeSphere(5.,sqrt(75.),sqrt(75.)/3);
  ConeSphere c = ConeSphere(sqrt(75.)/3,sqrt(75.),DEG_TO_RAD*30);
  c.print();
  return;
};

#endif
