/* Gokul Varadhan
 * varadhan@cs.unc.edu
 * May 2001
 */
/*$Id$*/
/* Object definitions */

#include <stdlib.h>		//stdlib.h vs cstdlib
#include <stdio.h>		//stdio.h vs cstdio
#include <iostream>
#include "3Dobject.h"
#include <math.h>		//math.h vs cmath
#include <GL/glut_UNC.h>
#include "defns.h"
#include "Tips.h"
#include "uncert.h"
#include "sim.h"

GLenum drawStyle = GL_FILL;       

extern void drawSphere(double diamter);
extern void drawCylinder(double diamter, double height);



/* Class : OB */
void OB :: setPos_z(double _z) {

  Vec3d tpos = pos;
  if (_z > 0) {
    tpos.z = _z;
  }
  else
    tpos.z = 0;

  setPos(tpos);
}

// norm in x and y only
double OB :: norm_xy( Vec3d v )
{
  return sqrt( (v.x * v.x)  +  (v.y * v.y) );
}

double OB :: norm_xz( Vec3d v )
{
  return sqrt( (v.x * v.x)  +  (v.z * v.z) );
}

/* Class : Ntube  */

// default constructor for OB
Ntube::Ntube( void ) {

  type = NTUBE;
  pos = Vec3d(0.,0.,0.);
  yaw = 0.;
  roll = 0.;
  pitch = 0.;
  leng = 0.;
  diam = 0.;
}


// constructor for OB
// we have a type field here because later I may distinguish ntubes from spheres
Ntube::Ntube(int type, Vec3d pos, double yaw, double roll, double pitch, double length, double diameter) {
  set(type,pos,yaw,roll,pitch,length,diameter);
}

void Ntube::set(int _type, Vec3d _pos, double _yaw, double _roll, double _pitch, double _length, double _diameter) {

  Vec3d vec, t1, t2, t3;

  type = _type;
  pos = _pos;
  yaw = _yaw;
  roll = _roll;
  pitch = _pitch;

  if (type == SPHERE) {// not allowed to set this for spheres
    leng = 0;
  }
  else {
    leng = _length;
  }
  diam = _diameter;

  // set the axis
  vec = Vec3d(1,0,0);
  t1 = vec.rotate3(Vec3d(0,0,1),DEG_TO_RAD*yaw);
  t2 = Vec3d(-t1.y,t1.x,0);
  t3 = t1.rotate3(t2,DEG_TO_RAD*pitch);
  axis = t3.normalize();
}

Ntube :: Ntube( Vec3d a, Vec3d b, double diam) {
  set(a,b,diam);
}

void Ntube :: set( Vec3d a, Vec3d b, double diam) {
  Vec3d t = b-a;
  Vec3d txy = Vec3d(t.x,t.y,0);

  //  double pitch = RAD_TO_DEG*acos(Vec3d :: dotProd(t,txy)/(t.magnitude()*txy.magnitude()));
  /* argument for acos might exceed 1 if t=txy due to floating pt errors. 
   * So we do this 
   */
  double temp = Vec3d :: dotProd(t,txy)/(t.magnitude()*txy.magnitude());
  if (temp >= 1)
    temp = 1;
  else
    if (temp < -1)
      temp =-1;
    
  double pitch = RAD_TO_DEG*acos(temp);

  if (t.z > 0) // the correct sign of the angle
    pitch = -pitch;
  
  Vec3d vec = Vec3d(1,0,0);

  temp = Vec3d :: dotProd(vec,txy)/((vec.magnitude())*txy.magnitude());

  if (temp >= 1)
    temp = 1;
  else
    if (temp < -1)
      temp =-1;

  //  double yaw = RAD_TO_DEG*acos(Vec3d :: dotProd(vec,txy)/((vec.magnitude())*txy.magnitude()));;

  double yaw = RAD_TO_DEG*acos(temp);
  if (t.y < 0) {
    yaw = -yaw;
  }


  set(type,(a+b)/2.,yaw,0.,pitch,t.magnitude(),diam);
}

void Ntube :: translate(Vec3d t) {
  setPos(pos+t);
}

void Ntube :: scale(double s) {
  setLength(leng*s);
  setDiam(diam*s);
}

/* whenever any parameter such as length etc changes, recalculate all others 
 * again
 */
void Ntube::recalc_all() {
  Vec3d vec, t1, t2, t3;
  // set the axis
  vec = Vec3d(1,0,0);
  t1 = vec.rotate3(Vec3d(0,0,1),DEG_TO_RAD*yaw);
  t2 = Vec3d(-t1.y,t1.x,0);
  t3 = t1.rotate3(t2,DEG_TO_RAD*pitch);
  axis = t3.normalize();
}


void Ntube :: setPos(Vec3d _pos) {
  pos = _pos;
  recalc_all();
}

void Ntube :: setDiam(double _diameter) {
  if (_diameter > 0) 
    diam = _diameter;
  else
    diam = 0;
  recalc_all();
}


void Ntube :: setLength(double _length) {
  if (type != SPHERE) {
    if (_length > 0)
      leng = _length;
    else
      leng = 0;
    recalc_all();
  }
}

void Ntube :: setYaw(double _yaw) {
  yaw = _yaw;
  recalc_all();
}

void Ntube :: setRoll(double _roll) {
  roll = _roll;
  recalc_all();
}

void Ntube :: setPitch(double _pitch) {
  pitch  = _pitch;
  recalc_all();
}

Vec3d Ntube :: getLeftEndPt() {// the one away from the direction of the axis
  return (pos - axis * (leng/2.));
}

Vec3d Ntube :: getRightEndPt() { // the one in the direction of the axis
  return (pos + axis * (leng/2.));
}


float zBuffer2[ DEPTHSIZE*DEPTHSIZE ];//***			
extern int mousepress;

//ANDREA
#if 1
void Ntube :: draw() {
  glPushMatrix();
  glTranslatef(pos.x, pos.y, pos.z );

  if (type == SPHERE) {// optimize if a sphere
    drawSphere(diam);
  }
  /*else if (type == CYLINDER)
  {
    // set tube yaw angle (in-plane rotation angle)
    glRotatef(yaw, 0.0, 0.0, 1.0 ); 
    
    // set roll angle around tube axis
    glRotatef(pitch,  0.0, 1.0, 0.0 );
    
    // set roll angle around tube axis
    glRotatef(roll,  1.0, 0.0, 0.0 );
    
    // draw cylinder with its axis parallel to X-axis
    glPushMatrix();
    //* we now have to align the cylinder with the Z-axis
    //* if the tube did not have any translation and rotation
    //* we want, the tube to be along the X-axis. GL on the other
    //* hand draws a cylinder along the Z-axis by default. And so
    //* we need to do the following rotate 
    //*
    glRotatef(90, 0.0, 1.0, 0.0 );
    //* The position of the tube is given by its centre on the
    //* axis. We want to draw the cylinder starting from its base
    //* Get to its base
    //*
    glTranslatef( 0., 0., - leng/2. );  
    drawCylinder( diam, leng );      // tube axis starts parallel to Z
    glTranslatef( -leng/2., 0., 0 );  
    glPopMatrix();
    
  }*/
  else{
    // set tube yaw angle (in-plane rotation angle)
    glRotatef(yaw, 0.0, 0.0, 1.0 ); 
    
    // set roll angle around tube axis
    glRotatef(pitch,  0.0, 1.0, 0.0 );
    
    // set roll angle around tube axis
    glRotatef(roll,  1.0, 0.0, 0.0 );
    
    // draw cylinder with its axis parallel to X-axis
    glPushMatrix();
    /* we now have to align the cylinder with the Z-axis
     * if the tube did not have any translation and rotation
     * we want, the tube to be along the X-axis. GL on the other
     * hand draws a cylinder along the Z-axis by default. And so
     * we need to do the following rotate 
     */
    glRotatef(90, 0.0, 1.0, 0.0 );
    /* The position of the tube is given by its centre on the
     * axis. We want to draw the cylinder starting from its base
     * Get to its base
     */
    glTranslatef( 0., 0., - leng/2. );  
    drawCylinder( diam, leng );      // tube axis starts parallel to Z
    glTranslatef( -leng/2., 0., 0 );  
    glPopMatrix();
    
#if 1
    // draw spherical endcap on tube
    glPushMatrix();
    glTranslatef( leng/2., 0., 0. );
    drawSphere(diam); 
    glPopMatrix();
    
    // draw spherical endcap on tube's other end
    glPushMatrix();
    glTranslatef( - leng/2., 0., 0. );
    drawSphere(diam); 
    glPopMatrix();
#endif
  }

  glPopMatrix();
}
#endif

//ANDREA
void Ntube :: uncert_draw() {
  glPushMatrix();
  glTranslatef(pos.x, pos.y, pos.z );

  if (type == SPHERE) {// optimize if a sphere
    draw_uncert_sphere(diam);
  }
  /*else if (type == CYLINDER){
    // set tube yaw angle (in-plane rotation angle)
    glRotatef(yaw, 0.0, 0.0, 1.0 ); 
    
    // set roll angle around tube axis
    glRotatef(pitch,  0.0, 1.0, 0.0 );
    
    // ____No roll_____ when we draw the uncertainty map 

    // set roll angle around tube axis
    //    glRotatef(roll,  1.0, 0.0, 0.0 );
    
    // draw cylinder with its axis parallel to X-axis
    glPushMatrix();
    // we now have to align the cylinder with the Z-axis
    //* if the tube did not have any translation and rotation
    //* we want, the tube to be along the X-axis. GL on the other
    //* hand draws a cylinder along the Z-axis by default. And so
    //* we need to do the following rotate 
    //*
    glRotatef(90, 0.0, 1.0, 0.0 );
    // The position of the tube is given by its centre on the
    //* axis. We want to draw the cylinder starting from its base
    //* Get to its base
    //
    glTranslatef( 0., 0., - leng/2. );  
    draw_uncert_cylinder( diam, leng );      // tube axis starts parallel to Z
    glTranslatef( -leng/2., 0., 0 );  
    glPopMatrix();

  }*/
  else {
    // set tube yaw angle (in-plane rotation angle)
    glRotatef(yaw, 0.0, 0.0, 1.0 ); 
    
    // set roll angle around tube axis
    glRotatef(pitch,  0.0, 1.0, 0.0 );
    
    /* ____No roll_____ when we draw the uncertainty map */

    // set roll angle around tube axis
    //    glRotatef(roll,  1.0, 0.0, 0.0 );
    
    // draw cylinder with its axis parallel to X-axis
    glPushMatrix();
    /* we now have to align the cylinder with the Z-axis
     * if the tube did not have any translation and rotation
     * we want, the tube to be along the X-axis. GL on the other
     * hand draws a cylinder along the Z-axis by default. And so
     * we need to do the following rotate 
     */
    glRotatef(90, 0.0, 1.0, 0.0 );
    /* The position of the tube is given by its centre on the
     * axis. We want to draw the cylinder starting from its base
     * Get to its base
     */
    glTranslatef( 0., 0., - leng/2. );  
    draw_uncert_cylinder( diam, leng );      // tube axis starts parallel to Z
    glTranslatef( -leng/2., 0., 0 );  
    glPopMatrix();

    // draw spherical endcap on tube
    glPushMatrix();
    glTranslatef( leng/2., 0., 0. );
    draw_uncert_sphere(diam); 
    glPopMatrix();
    
    // draw spherical endcap on tube's other end
    glPushMatrix();
    glTranslatef( - leng/2., 0., 0. );
    draw_uncert_sphere(diam); 
    glPopMatrix();
  }

  glPopMatrix();
}

//ANDREA
void Ntube :: afm_sphere_tip(SphereTip sp) {
  glPushMatrix();
  if (type == SPHERE) {
    glTranslatef(pos.x,pos.y,pos.z);
    drawSphere(diam +2*sp.r);
  }
  else {
    Ntube bigtube = *this;
    double tipRadius = sp.r;
    double newradius = diam/2. + tipRadius;
    bigtube.setDiam(2*newradius);
    bigtube.draw();
  }
  glPopMatrix();
}


//ANDREA
/* AFM with the uncertainty map on */
void Ntube :: uncert_afm_sphere_tip(SphereTip sp) {
  glPushMatrix();
  if (type == SPHERE) {
    glTranslatef(pos.x,pos.y,pos.z);
    draw_uncert_sphere(diam+2*sp.r);
  }
  else {
    Ntube bigtube = *this;
    double tipRadius = sp.r;
    double newradius = diam/2. + tipRadius;
    bigtube.setDiam(2*newradius);
    bigtube.uncert_draw();
  }
  glPopMatrix();
}

//ANDREA
void Ntube :: afm_inv_cone_sphere_tip(InvConeSphereTip icsTip) {

  glPushMatrix();
  if (type == SPHERE) {
    // for spheres only
    // Lower surface to match real surface height for ridges and plains.
#if 0
    double tipRadius = icsTip.r; // the radius for the tip
    double theta = icsTip.theta; // theta for the tip
    double radius=diam/2.;
    double fullradius = radius + tipRadius;
    static int created_list = 0;
    static GLUquadricObj* qobj;
    static int epoch = -1;
    int tesselation = icsTip.tesselation;
    ConeSphere c = ConeSphere(tipRadius, 200 + tipRadius/theta, theta);
    if ((created_list == 0) || (icsTip.epoch != epoch)) {
	created_list = 1;
	epoch = icsTip.epoch;

	// Create a ConeSphere display list that has a unit-radius sphere
	// and cone of specified angle.  This will be scaled and translated
	// later to make the appropriate shape.
	printf("Creating ConeSphere list for sphere object (tess %d)\n",
		tesselation);
	printf("  Radius %g, angle %g\n", tipRadius, RAD_TO_DEG*theta);
	qobj = gluNewQuadric();
	gluQuadricDrawStyle( qobj, GLU_FILL);
	gluQuadricNormals( qobj, GLU_FLAT );

	glNewList(TIP_CONE_SPHERE_LIST, GL_COMPILE);
	gluSphere( qobj, tipRadius, tesselation, tesselation );
	glPushMatrix();
	glTranslatef(0,0,-200);
	gluCylinder( qobj, c.cr, c.topRadius, c.topHeight, tesselation,
		tesselation );
	glPopMatrix();
	glEndList();
    }

    // Scale and translate the created display list, then call it.
    glPushMatrix();
    glTranslatef(pos.x,pos.y, pos.z);
    glScalef( fullradius/tipRadius, fullradius/tipRadius, fullradius/fRadius);
    glCallList(TIP_CONE_SPHERE_LIST);
    glPopMatrix();
#else
    double tipRadius = icsTip.r; // the radius for the tip
    double theta = icsTip.theta; // theta for the tip
    double radius=diam/2.;
    double cone_height = pos.z + (radius+tipRadius)/sin(theta);
    ConeSphere c = ConeSphere(radius+tipRadius, cone_height, theta);
    glTranslatef(pos.x,pos.y,pos.z);
    c.draw();
#endif
  }
  /*else if (type == CYLINDER){// for a general ntube
#if 1
    Vec3d A, B, C, D, P, Q, Axy, Bxy, N, N2, Nxy, N2xy, temp;
    Vec3d A2, B2, C2, D2, A2xy, B2xy;
    
    double R = icsTip.r;
    double theta = icsTip.theta;
    
    double r = diam/2.;

    // first draw the two quads
    N = Vec3d(axis.y,-axis.x,0);
    temp = N.rotate3(axis,theta);
    if (temp.z < 0) {
      N = N.rotate3(axis,-theta);
    }
    else
      N=temp;
    N = N.normalize();
    Nxy = Vec3d(N.x,N.y,0);
    Nxy = Nxy.normalize();
    
    // endpts of the tube
    P = pos - axis*leng/2.;
    Q = pos + axis*leng/2.;
    
    A = P + N*(r+R);
    B = Q + N*(r+R);
    
    Axy = Vec3d(A.x,A.y,0);
    Bxy = Vec3d(B.x,B.y,0);
    D = Axy + Nxy*A.z*tan(theta);
    C = Bxy + Nxy*B.z*tan(theta);
    
    // now let us calculate quad on the other side.
    N2 = Vec3d(-axis.y,axis.x,0);
    temp = N2.rotate3(axis,theta);
    if (temp.z < 0) {
      N2 = N2.rotate3(axis,-theta);
    }
    else
      N2=temp;
    N2 = N2.normalize();
    N2xy = Vec3d(N2.x,N2.y,0);
    N2xy = N2xy.normalize();
    
    A2 = P + N2*(r+R);
    B2 = Q + N2*(r+R);
    
    A2xy = Vec3d(A2.x,A2.y,0);
    B2xy = Vec3d(B2.x,B2.y,0);
    D2 = A2xy + N2xy*A2.z*tan(theta);
    C2 = B2xy + N2xy*B2.z*tan(theta);

#if 1
    Vec3d xyz = Vec3d :: crossProd(A-B,A-D);
    xyz.normalize();
    glBegin(GL_POLYGON);
    glNormal3f( xyz.x, xyz.y, xyz.z );
    glVertex3f( A.x, A.y, A.z );
    glVertex3f( B.x, B.y, B.z );
    glVertex3f( C.x, C.y, C.z );
    glVertex3f( D.x, D.y, D.z );
    glEnd();
    
    Vec3d xyz2 = Vec3d :: crossProd(A2-B2,A2-D2);
    xyz2.normalize();
    glBegin(GL_POLYGON);
    glNormal3f( xyz2.x, xyz2.y, xyz2.z );
    glVertex3f( A2.x, A2.y, A2.z );
    glVertex3f( B2.x, B2.y, B2.z );
    glVertex3f( C2.x, C2.y, C2.z );
    glVertex3f( D2.x, D2.y, D2.z );
    glEnd();
#endif
    
#if 1
    double newradius = diam/2. + R;
    Ntube bigtube = *this;
    bigtube.setDiam(2*newradius); 
    bigtube.draw();
#endif
#else 
#endif
  }*/
  else {// for a general ntube
#if 1
    Vec3d A, B, C, D, P, Q, Axy, Bxy, N, N2, Nxy, N2xy, temp;
    Vec3d A2, B2, C2, D2, A2xy, B2xy;
    
    double R = icsTip.r;
    double theta = icsTip.theta;
    
    double r = diam/2.;

    // first draw the two quads
    N = Vec3d(axis.y,-axis.x,0);
    temp = N.rotate3(axis,theta);
    if (temp.z < 0) {
      N = N.rotate3(axis,-theta);
    }
    else
      N=temp;
    N = N.normalize();
    Nxy = Vec3d(N.x,N.y,0);
    Nxy = Nxy.normalize();
    
    // endpts of the tube
    P = pos - axis*leng/2.;
    Q = pos + axis*leng/2.;
    
    A = P + N*(r+R);
    B = Q + N*(r+R);
    
    Axy = Vec3d(A.x,A.y,0);
    Bxy = Vec3d(B.x,B.y,0);
    D = Axy + Nxy*A.z*tan(theta);
    C = Bxy + Nxy*B.z*tan(theta);
    
    // now let us calculate quad on the other side.
    N2 = Vec3d(-axis.y,axis.x,0);
    temp = N2.rotate3(axis,theta);
    if (temp.z < 0) {
      N2 = N2.rotate3(axis,-theta);
    }
    else
      N2=temp;
    N2 = N2.normalize();
    N2xy = Vec3d(N2.x,N2.y,0);
    N2xy = N2xy.normalize();
    
    A2 = P + N2*(r+R);
    B2 = Q + N2*(r+R);
    
    A2xy = Vec3d(A2.x,A2.y,0);
    B2xy = Vec3d(B2.x,B2.y,0);
    D2 = A2xy + N2xy*A2.z*tan(theta);
    C2 = B2xy + N2xy*B2.z*tan(theta);

#if 1
    Vec3d xyz = Vec3d :: crossProd(A-B,A-D);
    xyz.normalize();
    glBegin(GL_POLYGON);
    glNormal3f( xyz.x, xyz.y, xyz.z );
    glVertex3f( A.x, A.y, A.z );
    glVertex3f( B.x, B.y, B.z );
    glVertex3f( C.x, C.y, C.z );
    glVertex3f( D.x, D.y, D.z );
    glEnd();
    
    Vec3d xyz2 = Vec3d :: crossProd(A2-B2,A2-D2);
    xyz2.normalize();
    glBegin(GL_POLYGON);
    glNormal3f( xyz2.x, xyz2.y, xyz2.z );
    glVertex3f( A2.x, A2.y, A2.z );
    glVertex3f( B2.x, B2.y, B2.z );
    glVertex3f( C2.x, C2.y, C2.z );
    glVertex3f( D2.x, D2.y, D2.z );
    glEnd();
#endif


#if 1
    // now draw the two frustums
    Vec3d one_end = pos - axis*leng/2.;
    Vec3d other_end = pos + axis*leng/2.;
    
    double cone_height = one_end.z + (r+R)/sin(theta);
    ConeSphere c = ConeSphere(r+R, cone_height, theta);
    glPushMatrix();
    glTranslatef(one_end.x,one_end.y,one_end.z);
    c.draw();
    glPopMatrix();
  
    // now other end
    cone_height = other_end.z + (r+R)/sin(theta);
    c = ConeSphere(r+R, cone_height, theta);
    glPushMatrix();
    glTranslatef(other_end.x,other_end.y,other_end.z);
    c.draw();
    glPopMatrix();
    
#endif
    
#if 1
    double newradius = diam/2. + R;
    Ntube bigtube = *this;
    bigtube.setDiam(2*newradius); 
    bigtube.draw();
#endif
#else 
#endif
  }
  glPopMatrix();
}

//ANDREA
/* AFM with the uncertainty map on */
void Ntube :: uncert_afm_inv_cone_sphere_tip(InvConeSphereTip icsTip) {

  glPushMatrix();
  if (type == SPHERE) {
    // for spheres only
    // Lower surface to match real surface height for ridges and plains.
    double tipRadius = icsTip.r; // the radius for the tip
    double theta = icsTip.theta; // theta for the tip
    double radius=diam/2.;
    double cone_height = pos.z + (radius+tipRadius)/sin(theta);
    ConeSphere c = ConeSphere(radius+tipRadius, cone_height, theta);
    glTranslatef(pos.x,pos.y,pos.z);
    c.uncert_draw();
  }
  /*else if (type == CYLINDER){
#if 1
    Vec3d A, B, C, D, P, Q, Axy, Bxy, N, N2, Nxy, N2xy, temp;
    Vec3d A2, B2, C2, D2, A2xy, B2xy;
    
    double R = icsTip.r;
    double theta = icsTip.theta;
    
    double r = diam/2.;

    // first draw the two quads
    N = Vec3d(axis.y,-axis.x,0);
    temp = N.rotate3(axis,theta);
    if (temp.z < 0) {
      N = N.rotate3(axis,-theta);
    }
    else
      N=temp;
    N = N.normalize();
    Nxy = Vec3d(N.x,N.y,0);
    Nxy = Nxy.normalize();
    
    // endpts of the tube
    P = pos - axis*leng/2.;
    Q = pos + axis*leng/2.;
    
    A = P + N*(r+R);
    B = Q + N*(r+R);
    
    Axy = Vec3d(A.x,A.y,0);
    Bxy = Vec3d(B.x,B.y,0);
    D = Axy + Nxy*A.z*tan(theta);
    C = Bxy + Nxy*B.z*tan(theta);
    
    // now let us calculate quad on the other side.
    N2 = Vec3d(-axis.y,axis.x,0);
    temp = N2.rotate3(axis,theta);
    if (temp.z < 0) {
      N2 = N2.rotate3(axis,-theta);
    }
    else
      N2=temp;
    N2 = N2.normalize();
    N2xy = Vec3d(N2.x,N2.y,0);
    N2xy = N2xy.normalize();
    
    A2 = P + N2*(r+R);
    B2 = Q + N2*(r+R);
    
    A2xy = Vec3d(A2.x,A2.y,0);
    B2xy = Vec3d(B2.x,B2.y,0);
    D2 = A2xy + N2xy*A2.z*tan(theta);
    C2 = B2xy + N2xy*B2.z*tan(theta);


#if 1
    // get color for the quad
    GLfloat gcol = get_sphere_color_rho(PI/2.-theta);

    Vec3d xyz = Vec3d :: crossProd(A-B,A-D);
    xyz.normalize();
    glBegin(GL_POLYGON);
    glNormal3f( xyz.x, xyz.y, xyz.z );

    glColor3f(gcol,gcol,gcol);
    glVertex3f( A.x, A.y, A.z );
    glVertex3f( B.x, B.y, B.z );

    glVertex3f( C.x, C.y, C.z );
    glVertex3f( D.x, D.y, D.z );
    glEnd();
    
    Vec3d xyz2 = Vec3d :: crossProd(A2-B2,A2-D2);
    xyz2.normalize();
    glBegin(GL_POLYGON);
    glNormal3f( xyz2.x, xyz2.y, xyz2.z );

    glColor3f(gcol,gcol,gcol);
    glVertex3f( A2.x, A2.y, A2.z );
    glVertex3f( B2.x, B2.y, B2.z );

    glVertex3f( C2.x, C2.y, C2.z );
    glVertex3f( D2.x, D2.y, D2.z );
    glEnd();

#endif
    
#if 1
    double newradius = diam/2. + R;
    Ntube bigtube = *this;
    bigtube.setDiam(2*newradius); 
    //    bigtube.draw();
    bigtube.uncert_draw();
#endif
#else 
#endif
  }*/
  else {// for a general ntube
#if 1
    Vec3d A, B, C, D, P, Q, Axy, Bxy, N, N2, Nxy, N2xy, temp;
    Vec3d A2, B2, C2, D2, A2xy, B2xy;
    
    double R = icsTip.r;
    double theta = icsTip.theta;
    
    double r = diam/2.;

    // first draw the two quads
    N = Vec3d(axis.y,-axis.x,0);
    temp = N.rotate3(axis,theta);
    if (temp.z < 0) {
      N = N.rotate3(axis,-theta);
    }
    else
      N=temp;
    N = N.normalize();
    Nxy = Vec3d(N.x,N.y,0);
    Nxy = Nxy.normalize();
    
    // endpts of the tube
    P = pos - axis*leng/2.;
    Q = pos + axis*leng/2.;
    
    A = P + N*(r+R);
    B = Q + N*(r+R);
    
    Axy = Vec3d(A.x,A.y,0);
    Bxy = Vec3d(B.x,B.y,0);
    D = Axy + Nxy*A.z*tan(theta);
    C = Bxy + Nxy*B.z*tan(theta);
    
    // now let us calculate quad on the other side.
    N2 = Vec3d(-axis.y,axis.x,0);
    temp = N2.rotate3(axis,theta);
    if (temp.z < 0) {
      N2 = N2.rotate3(axis,-theta);
    }
    else
      N2=temp;
    N2 = N2.normalize();
    N2xy = Vec3d(N2.x,N2.y,0);
    N2xy = N2xy.normalize();
    
    A2 = P + N2*(r+R);
    B2 = Q + N2*(r+R);
    
    A2xy = Vec3d(A2.x,A2.y,0);
    B2xy = Vec3d(B2.x,B2.y,0);
    D2 = A2xy + N2xy*A2.z*tan(theta);
    C2 = B2xy + N2xy*B2.z*tan(theta);


#if 1
    // get color for the quad
    GLfloat gcol = get_sphere_color_rho(PI/2.-theta);

    Vec3d xyz = Vec3d :: crossProd(A-B,A-D);
    xyz.normalize();
    glBegin(GL_POLYGON);
    glNormal3f( xyz.x, xyz.y, xyz.z );

    glColor3f(gcol,gcol,gcol);
    glVertex3f( A.x, A.y, A.z );
    glVertex3f( B.x, B.y, B.z );

    glVertex3f( C.x, C.y, C.z );
    glVertex3f( D.x, D.y, D.z );
    glEnd();
    
    Vec3d xyz2 = Vec3d :: crossProd(A2-B2,A2-D2);
    xyz2.normalize();
    glBegin(GL_POLYGON);
    glNormal3f( xyz2.x, xyz2.y, xyz2.z );

    glColor3f(gcol,gcol,gcol);
    glVertex3f( A2.x, A2.y, A2.z );
    glVertex3f( B2.x, B2.y, B2.z );

    glVertex3f( C2.x, C2.y, C2.z );
    glVertex3f( D2.x, D2.y, D2.z );
    glEnd();

#endif


#if 1
    // now draw the two frustums
    Vec3d one_end = pos - axis*leng/2.;
    Vec3d other_end = pos + axis*leng/2.;
    
    double cone_height = one_end.z + (r+R)/sin(theta);
    ConeSphere c = ConeSphere(r+R, cone_height, theta);
    glPushMatrix();
    glTranslatef(one_end.x,one_end.y,one_end.z);
    c.uncert_draw();
    glPopMatrix();
  
    // now other end
    cone_height = other_end.z + (r+R)/sin(theta);
    c = ConeSphere(r+R, cone_height, theta);
    glPushMatrix();
    glTranslatef(other_end.x,other_end.y,other_end.z);
    c.uncert_draw();
    glPopMatrix();
    
#endif
    
#if 1
    double newradius = diam/2. + R;
    Ntube bigtube = *this;
    bigtube.setDiam(2*newradius); 
    //    bigtube.draw();
    bigtube.uncert_draw();
#endif
#else 
#endif
  }
  glPopMatrix();
}


void Ntube :: keyboardFunc(unsigned char key, int x, int y) {
  Vec3d left, right;
  int i;

  switch (key) {
  case '+' : 
      for(i = 0; i < number_in_group[obj_group];i++){
			OB * this_obj = group_of_obs[obj_group][i];
			if(this_obj->type == NTUBE){
				
				Ntube * n = (Ntube *)this_obj;
				n->setPos_z(n->pos.z+DIST_UNIT);
			}
	  }
    break;
  case '-' : 
    for(i = 0; i < number_in_group[obj_group];i++){
			OB * this_obj = group_of_obs[obj_group][i];
			if(this_obj->type == NTUBE){
				
				Ntube * n = (Ntube *)this_obj;
				n->setPos_z(n->pos.z-DIST_UNIT);
			}
	  }
    break;
    /* Now we consider various rotations */
  case 'x' :
    if (leng) {
		//find the group center
		Vec3d group_center;
		for(i = 0; i < number_in_group[obj_group];i++){
			OB * this_obj = group_of_obs[obj_group][i];
			group_center += this_obj->pos;
		}
		group_center /= number_in_group[obj_group];

		//translate objects to position ANGLE_UNIT rel. to group_center
		for(i = 0; i < number_in_group[obj_group];i++){
			OB * this_obj = group_of_obs[obj_group][i];
			Vec3d rel_pos = this_obj->pos - group_center;
			Vec3d new_pos = rel_pos.rotate3(Vec3d(1,0,0),DEG_TO_RAD*ANGLE_UNIT) + group_center;
			this_obj->setPos(new_pos);
		}
		
		//rotate objects about themselves
		for(i = 0; i < number_in_group[obj_group];i++){
			OB * this_obj = group_of_obs[obj_group][i];
			if(this_obj->type == NTUBE){
				
				Ntube * n = (Ntube *)this_obj;
				Vec3d left;
				Vec3d right;
				left += n->pos;
				left -= n->axis*n->leng/2;
				right += n->pos;
				right += n->axis*n->leng/2;

				left = n->pos + Vec3d(left - n->pos).rotate3(Vec3d(1,0,0),DEG_TO_RAD*ANGLE_UNIT);
				right = n->pos + Vec3d(right - n->pos).rotate3(Vec3d(1,0,0),DEG_TO_RAD*ANGLE_UNIT);
				n->set(left,right,n->diam);
			}
			else if(this_obj->type == TRIANGLE){
				Triangle * t = (Triangle *)this_obj;
				Vec3d a,b,c;

				a = t->pos+Vec3d(t->a - t->pos).rotate3(Vec3d(1,0,0),DEG_TO_RAD*ANGLE_UNIT);
				b = t->pos+Vec3d(t->b - t->pos).rotate3(Vec3d(1,0,0),DEG_TO_RAD*ANGLE_UNIT);
				c = t->pos+Vec3d(t->c - t->pos).rotate3(Vec3d(1,0,0),DEG_TO_RAD*ANGLE_UNIT);
				t->set(a,b,c);				
			}
		}
    }
    break;
  case 'X' :
    if (leng) {
		//find the group center
		Vec3d group_center;
		for(i = 0; i < number_in_group[obj_group];i++){
			OB * this_obj = group_of_obs[obj_group][i];
			group_center += this_obj->pos;
		}
		group_center /= number_in_group[obj_group];

		//translate objects to position ANGLE_UNIT rel. to group_center
		for(i = 0; i < number_in_group[obj_group];i++){
			OB * this_obj = group_of_obs[obj_group][i];
			Vec3d rel_pos = this_obj->pos - group_center;
			Vec3d new_pos = rel_pos.rotate3(Vec3d(1,0,0),-DEG_TO_RAD*ANGLE_UNIT) + group_center;
			this_obj->setPos(new_pos);
		}

		for(i = 0; i < number_in_group[obj_group];i++){
			OB * this_obj = group_of_obs[obj_group][i];
			if(this_obj->type == NTUBE){
				
				Ntube * n = (Ntube *)this_obj;
				Vec3d left;
				Vec3d right;
				left += n->pos;
				left -= n->axis*n->leng/2;
				right += n->pos;
				right += n->axis*n->leng/2;

				left = n->pos + Vec3d(left - n->pos).rotate3(Vec3d(1,0,0),-DEG_TO_RAD*ANGLE_UNIT);
				right = n->pos + Vec3d(right - n->pos).rotate3(Vec3d(1,0,0),-DEG_TO_RAD*ANGLE_UNIT);
				n->set(left,right,n->diam);
			}
			else if(this_obj->type == TRIANGLE){
				Triangle * t = (Triangle *)this_obj;
				Vec3d a,b,c;

				a = t->pos+Vec3d(t->a - t->pos).rotate3(Vec3d(1,0,0),-DEG_TO_RAD*ANGLE_UNIT);
				b = t->pos+Vec3d(t->b - t->pos).rotate3(Vec3d(1,0,0),-DEG_TO_RAD*ANGLE_UNIT);
				c = t->pos+Vec3d(t->c - t->pos).rotate3(Vec3d(1,0,0),-DEG_TO_RAD*ANGLE_UNIT);
				t->set(a,b,c);				
			}
		}
    }
    break;
  case 'y' :
    if (leng) {
		//find the group center
		Vec3d group_center;
		for(i = 0; i < number_in_group[obj_group];i++){
			OB * this_obj = group_of_obs[obj_group][i];
			group_center += this_obj->pos;
		}
		group_center /= number_in_group[obj_group];

		//translate objects to position ANGLE_UNIT rel. to group_center
		for(i = 0; i < number_in_group[obj_group];i++){
			OB * this_obj = group_of_obs[obj_group][i];
			Vec3d rel_pos = this_obj->pos - group_center;
			Vec3d new_pos = rel_pos.rotate3(Vec3d(0,1,0),DEG_TO_RAD*ANGLE_UNIT) + group_center;
			this_obj->setPos(new_pos);
		}

		for(i = 0; i < number_in_group[obj_group];i++){
			OB * this_obj = group_of_obs[obj_group][i];
			if(this_obj->type == NTUBE){
				
				Ntube * n = (Ntube *)this_obj;
				Vec3d left;
				Vec3d right;
				left += n->pos;
				left -= n->axis*n->leng/2;
				right += n->pos;
				right += n->axis*n->leng/2;

				left = n->pos + Vec3d(left - n->pos).rotate3(Vec3d(0,1,0),DEG_TO_RAD*ANGLE_UNIT);
				right = n->pos + Vec3d(right - n->pos).rotate3(Vec3d(0,1,0),DEG_TO_RAD*ANGLE_UNIT);
				n->set(left,right,n->diam);
			}
			else if(this_obj->type == TRIANGLE){
				Triangle * t = (Triangle *)this_obj;
				Vec3d a,b,c;

				a = t->pos+Vec3d(t->a - t->pos).rotate3(Vec3d(0,1,0),DEG_TO_RAD*ANGLE_UNIT);
				b = t->pos+Vec3d(t->b - t->pos).rotate3(Vec3d(0,1,0),DEG_TO_RAD*ANGLE_UNIT);
				c = t->pos+Vec3d(t->c - t->pos).rotate3(Vec3d(0,1,0),DEG_TO_RAD*ANGLE_UNIT);
				t->set(a,b,c);				
			}
		}
    }
    break;
  case 'Y' :
    if (leng) {
		//find the group center
		Vec3d group_center;
		for(i = 0; i < number_in_group[obj_group];i++){
			OB * this_obj = group_of_obs[obj_group][i];
			group_center += this_obj->pos;
		}
		group_center /= number_in_group[obj_group];

		//translate objects to position ANGLE_UNIT rel. to group_center
		for(i = 0; i < number_in_group[obj_group];i++){
			OB * this_obj = group_of_obs[obj_group][i];
			Vec3d rel_pos = this_obj->pos - group_center;
			Vec3d new_pos = rel_pos.rotate3(Vec3d(0,1,0),-DEG_TO_RAD*ANGLE_UNIT) + group_center;
			this_obj->setPos(new_pos);
		}

		for(i = 0; i < number_in_group[obj_group];i++){
			OB * this_obj = group_of_obs[obj_group][i];
			if(this_obj->type == NTUBE){
				
				Ntube * n = (Ntube *)this_obj;
				Vec3d left;
				Vec3d right;
				left += n->pos;
				left -= n->axis*n->leng/2;
				right += n->pos;
				right += n->axis*n->leng/2;

				left = n->pos + Vec3d(left - n->pos).rotate3(Vec3d(0,1,0),-DEG_TO_RAD*ANGLE_UNIT);
				right = n->pos + Vec3d(right - n->pos).rotate3(Vec3d(0,1,0),-DEG_TO_RAD*ANGLE_UNIT);
				n->set(left,right,n->diam);
			}
			else if(this_obj->type == TRIANGLE){
				Triangle * t = (Triangle *)this_obj;
				Vec3d a,b,c;

				a = t->pos+Vec3d(t->a - t->pos).rotate3(Vec3d(0,1,0),-DEG_TO_RAD*ANGLE_UNIT);
				b = t->pos+Vec3d(t->b - t->pos).rotate3(Vec3d(0,1,0),-DEG_TO_RAD*ANGLE_UNIT);
				c = t->pos+Vec3d(t->c - t->pos).rotate3(Vec3d(0,1,0),-DEG_TO_RAD*ANGLE_UNIT);
				t->set(a,b,c);				
			}
		}
    }
    break;
  case 'z' :
    if (leng) {
		//find the group center
		Vec3d group_center;
		for(i = 0; i < number_in_group[obj_group];i++){
			OB * this_obj = group_of_obs[obj_group][i];
			group_center += this_obj->pos;
		}
		group_center /= number_in_group[obj_group];

		//translate objects to position ANGLE_UNIT rel. to group_center
		for(i = 0; i < number_in_group[obj_group];i++){
			OB * this_obj = group_of_obs[obj_group][i];
			Vec3d rel_pos = this_obj->pos - group_center;
			Vec3d new_pos = rel_pos.rotate3(Vec3d(0,0,1),DEG_TO_RAD*ANGLE_UNIT) + group_center;
			this_obj->setPos(new_pos);
		}

		for(i = 0; i < number_in_group[obj_group];i++){
			OB * this_obj = group_of_obs[obj_group][i];
			if(this_obj->type == NTUBE){
				
				Ntube * n = (Ntube *)this_obj;
				Vec3d left;
				Vec3d right;
				left += n->pos;
				left -= n->axis*n->leng/2;
				right += n->pos;
				right += n->axis*n->leng/2;

				left = n->pos + Vec3d(left - n->pos).rotate3(Vec3d(0,0,1),DEG_TO_RAD*ANGLE_UNIT);
				right = n->pos + Vec3d(right - n->pos).rotate3(Vec3d(0,0,1),DEG_TO_RAD*ANGLE_UNIT);
				n->set(left,right,n->diam);
			}
			else if(this_obj->type == TRIANGLE){
				Triangle * t = (Triangle *)this_obj;
				Vec3d a,b,c;

				a = t->pos+Vec3d(t->a - t->pos).rotate3(Vec3d(0,0,1),DEG_TO_RAD*ANGLE_UNIT);
				b = t->pos+Vec3d(t->b - t->pos).rotate3(Vec3d(0,0,1),DEG_TO_RAD*ANGLE_UNIT);
				c = t->pos+Vec3d(t->c - t->pos).rotate3(Vec3d(0,0,1),DEG_TO_RAD*ANGLE_UNIT);
				t->set(a,b,c);				
			}
		}
    }
    break;
  case 'Z' :
    if (leng) {  
		//find the group center
		Vec3d group_center;
		for(i = 0; i < number_in_group[obj_group];i++){
			OB * this_obj = group_of_obs[obj_group][i];
			group_center += this_obj->pos;
		}
		group_center /= number_in_group[obj_group];

		//translate objects to position ANGLE_UNIT rel. to group_center
		for(i = 0; i < number_in_group[obj_group];i++){
			OB * this_obj = group_of_obs[obj_group][i];
			Vec3d rel_pos = this_obj->pos - group_center;
			Vec3d new_pos = rel_pos.rotate3(Vec3d(0,0,1),-DEG_TO_RAD*ANGLE_UNIT) + group_center;
			this_obj->setPos(new_pos);
		}

		for(i = 0; i < number_in_group[obj_group];i++){
			OB * this_obj = group_of_obs[obj_group][i];
			if(this_obj->type == NTUBE){
				
				Ntube * n = (Ntube *)this_obj;
				Vec3d left;
				Vec3d right;
				left += n->pos;
				left -= n->axis*n->leng/2;
				right += n->pos;
				right += n->axis*n->leng/2;

				left = n->pos + Vec3d(left - n->pos).rotate3(Vec3d(0,0,1),-DEG_TO_RAD*ANGLE_UNIT);
				right = n->pos + Vec3d(right - n->pos).rotate3(Vec3d(0,0,1),-DEG_TO_RAD*ANGLE_UNIT);
				n->set(left,right,n->diam);
			}
			else if(this_obj->type == TRIANGLE){
				Triangle * t = (Triangle *)this_obj;
				Vec3d a,b,c;

				a = t->pos+Vec3d(t->a - t->pos).rotate3(Vec3d(0,0,1),-DEG_TO_RAD*ANGLE_UNIT);
				b = t->pos+Vec3d(t->b - t->pos).rotate3(Vec3d(0,0,1),-DEG_TO_RAD*ANGLE_UNIT);
				c = t->pos+Vec3d(t->c - t->pos).rotate3(Vec3d(0,0,1),-DEG_TO_RAD*ANGLE_UNIT);
				t->set(a,b,c);				
			}
		}
    }
    break;
  case 'e' :
    setRoll(roll + ANGLE_UNIT);
    break;
  case 'E' :
    setRoll(roll - ANGLE_UNIT);
    break;
  case 'f' :
    setYaw(yaw + ANGLE_UNIT);
    break;
  case 'F' :
    setYaw(yaw - ANGLE_UNIT);
    break;
  case 'h' :
    setPitch(pitch + ANGLE_UNIT);
    break;
  case 'H' :
    setPitch(pitch - ANGLE_UNIT);
    break;
    /* Object manipulation */
  case 'l' :
    setLength(leng + LENGTH_UNIT);
    break;
  case 'L' :
    setLength(leng - LENGTH_UNIT);
    break;
  case 'd' :
    setDiam(diam + DIAM_UNIT);
    break;
  case 'D' :
    setDiam(diam - DIAM_UNIT);
    break;
  default :
    break;
  }
}

void Ntube::print() {
  cout << "pos x " << pos.x << " y " << pos.y << " z " << pos.z << " yaw " << yaw << " pitch " << pitch << " roll " << roll << " length " << leng << " diam " << diam << endl;
}

double Ntube :: xy_distance(Vec3d vMouseWorld) {
  return norm_xy(vMouseWorld - pos);
}

double Ntube :: xz_distance(Vec3d vMouseWorld) {
  return norm_xz(vMouseWorld - pos);
}

void Ntube :: grabOb(Vec3d vMouseWorld, int xy_or_xz) {
  // store the grab offset
  vGrabOffset = pos   - vMouseWorld;
}

void Ntube :: moveGrabbedOb(Vec3d vMouseWorld) {
  // As vMouseWorld changes, move the object
	Vec3d new_pos(vMouseWorld.x + vGrabOffset.x, vMouseWorld.y + vGrabOffset.y, vMouseWorld.z + vGrabOffset.z);
    Vec3d diff(new_pos.x,new_pos.y,new_pos.z);
	diff -= pos;
	for(int i = 0; i < number_in_group[obj_group];i++){//obj_group local to 'this'
		OB * this_obj = group_of_obs[obj_group][i];
		Vec3d this_pos;
		this_pos += this_obj->pos;
		this_pos += diff;
		this_obj->setPos(this_pos);
	}
}

/* We have a type field. Later, I plan to distingush spheres from ntubes */
void
addNtube(int type, Vec3d pos, double yaw, double roll, double pitch, double leng, double diam,
		 int *group_number) {

	//testing
	/*group_number = new int();
	*group_number = 0;*/

	Ntube *n = new Ntube(type,pos,yaw,roll,pitch,leng,diam);
	ob[numObs] = n;
	// select this ob
	selectedOb = numObs;
	numObs++;
	int num = addToGroup(n,group_number);
	n->obj_group = num;
}

int addToGroup(OB* obj,int* group_number){
	if(group_number == NULL){//no group specified, create one
		group_of_obs[numGroups] = new OB*[MAXOBS];
		group_number = new int();
		*group_number = numGroups;
		group_of_obs[*group_number][number_in_group[*group_number]++] = obj;
		numGroups++;//one more group now
		obj->obj_group = *group_number;
		cout << "group number: " << *group_number << endl;
	}
	else if(group_of_obs[*group_number] == NULL){//group specified is empty, start it
		group_of_obs[*group_number] = new OB*[MAXOBS];
		group_of_obs[*group_number][number_in_group[*group_number]++] = obj;
		obj->obj_group = *group_number;
		cout << "group number: " << *group_number << endl << "number in group: "
			 << number_in_group[*group_number] << endl;
	}
	else{//group specified is nonempty, add to existing group
		group_of_obs[*group_number][number_in_group[*group_number]++] = obj;
		obj->obj_group = *group_number;
		cout << "group number: " << *group_number << endl << "number in group: "
			 << number_in_group[*group_number] << endl;
	}
	return *group_number;
}

bool inGroup(OB* obj,int* group_number){
	int i = *group_number;
	for(int j = 0;j < number_in_group[i];j++){
		if(group_of_obs[i][j] == obj){
			return true;	
		}
	}
	return false;
}

void removeFromGroup(OB* obj,int* group_number){
	int i = *group_number;
	for(int j = 0;j < number_in_group[i];j++){
		if(group_of_obs[i][j] == obj){
			for(int k = j;k+1 < number_in_group[i];k++){//shift everything back
				group_of_obs[i][k] = group_of_obs[i][k+1];
			}
			group_of_obs[i][number_in_group[i]-1] = NULL;
			number_in_group[i]--;
			cout << "object removed from group number " << *group_number << endl;
			break;			
		}
	}
	int* new_group = NULL;//put it in its own group now
	addToGroup(obj,new_group);

}

int changeGroup(OB* obj,int* new_group_number){
	bool removed = false;

	if(new_group_number == NULL){//no group specified, create one
		group_of_obs[numGroups] = new OB*[MAXOBS];
		new_group_number = new int();
		*new_group_number = numGroups;
		group_of_obs[*new_group_number][number_in_group[*new_group_number]++] = obj;

		//remove from old group
		for(int i = 0;i < numGroups; ++i){
			for(int j = 0;j < number_in_group[i];j++){
				if(group_of_obs[i][j] == obj){
					for(int k = j;k+1 < number_in_group[i];k++){//shift everything back
						group_of_obs[i][k] = group_of_obs[i][k+1];
					}
					group_of_obs[i][number_in_group[i]-1] = NULL;
					number_in_group[i]--;
					removed = true;
					obj->obj_group = *new_group_number;
					cout << "object removed from group number " << i << " and added to group number "
						 << *new_group_number << endl;
					break;
				}
			}
			if(removed)	break;
		}

		numGroups++;//one more group now
		cout << "group number: " << *new_group_number << endl << "number in group: "
			 << number_in_group[*new_group_number] << endl;
	}
	else if(group_of_obs[*new_group_number] == NULL){//group specified is empty, start it
		group_of_obs[*new_group_number] = new OB*[MAXOBS];
		group_of_obs[*new_group_number][number_in_group[*new_group_number]++] = obj;

		//remove from old group
		for(int i = 0;i < numGroups; ++i){
			for(int j = 0;j < number_in_group[i];j++){
				if(group_of_obs[i][j] == obj){
					for(int k = j;k+1 < number_in_group[i];k++){//shift everything back
						group_of_obs[i][k] = group_of_obs[i][k+1];
					}
					group_of_obs[i][number_in_group[i]-1] = NULL;
					number_in_group[i]--;
					removed = true;
					obj->obj_group = *new_group_number;
					cout << "object removed from group number " << i << " and added to group number "
						 << *new_group_number << endl;
					break;
				}
			}
			if(removed)	break;
		}

		cout << "group number: " << *new_group_number << endl << "number in group: "
			 << number_in_group[*new_group_number] << endl;
	}
	else{//group specified is nonempty, add to existing group
		group_of_obs[*new_group_number][number_in_group[*new_group_number]++] = obj;

		//remove from old group
		for(int i = 0;i < numGroups; ++i){
			for(int j = 0;j < number_in_group[i];j++){
				if(group_of_obs[i][j] == obj){
					for(int k = j;k+1 < number_in_group[i];k++){//shift everything back
						group_of_obs[i][k] = group_of_obs[i][k+1];
					}
					group_of_obs[i][number_in_group[i]-1] = NULL;
					number_in_group[i]--;
					removed = true;
					obj->obj_group = *new_group_number;
					cout << "object removed from group number " << i << " and added to group number "
						 << *new_group_number << endl;
					break;
				}
			}
			if(removed)	break;
		}

		cout << "group number: " << *new_group_number << endl << "number in group: "
			 << number_in_group[*new_group_number] << endl;
	}
	return *new_group_number;
}

/* Class : Triangle  */

Triangle :: Triangle(void) {
  type = TRIANGLE;
  a=Vec3d(0,0,0);
  b=Vec3d(0,0,0);
  c=Vec3d(0,0,0);
  pos=Vec3d(0,0,0);
  normal = Vec3d(1,1,1); // irrelevant
  normal = normal.normalize();
}


Triangle :: Triangle(Vec3d _a, Vec3d _b, Vec3d _c) {
    /*glBegin(GL_TRIANGLES);
		glVertex3f(_a.x,_a.y,_a.z);
		glVertex3f(_b.x,_b.y,_b.z);
		glVertex3f(_c.x,_c.y,_c.z);
	glEnd();*/
	set(_a, _b, _c);
}

void Triangle :: set(Vec3d _a, Vec3d _b, Vec3d _c) {
  Vec3d t1, t2;

  type = TRIANGLE;
  a=_a;
  b=_b;
  c=_c;
  pos = (a+b+c)/3.;
  t1=b-a;
  t2=c-a;

  normal = Vec3d :: crossProd(t1,t2);
  normal = normal.normalize();

  // set the ntubes, ab, bc and ca;
  ab = Ntube(a, b, 0.);
  bc = Ntube(b, c, 0.);
  ca = Ntube(c, a, 0.);
}

void Triangle :: setPos(Vec3d _pos) {
  Vec3d offset = _pos - pos;
  a = a + offset;
  b = b + offset;
  c = c + offset;

  set(a,b,c);
}

void Triangle :: translate(Vec3d t) {
  setPos(pos+t);
}

void Triangle :: scale(double s) {
  set(a*s,b*s,c*s);
}

extern int buttonpress;
extern int selected_triangle_side;
extern void setColor(int);
extern int mainWindowID;

/* Note : This draws a triangle with vertices at a,b, c RELATIVE to the 
 * CURRENT ORIGIN. So if you do a translate before calling this funcn
 * the triangle will be drawn relative to the translated origin
 */
void Triangle :: draw() {
  glPushMatrix();
  glBegin(GL_TRIANGLES);
  glNormal3f( normal.x, normal.y, normal.z);
  glVertex3f( a.x, a.y, a.z );
  glVertex3f( b.x, b.y, b.z );
  glVertex3f( c.x, c.y, c.z );
  glEnd();
  glPopMatrix();
}


void Triangle :: afm_sphere_tip(SphereTip sp) {
  double R = sp.r;
  Vec3d offset = normal*R;
  Triangle tr = Triangle(a+offset, b+offset, c+offset); 
  tr.draw();
  
  ab.afm_sphere_tip(sp);
  bc.afm_sphere_tip(sp);
  ca.afm_sphere_tip(sp);
}

/* now get triangle color depending on orientation of the triangle
 * use the color of the pt where the spherical part of the tip touches
 * the triangle.
 */
static void set_tri_color(Triangle tr) {
  float cosrho = Vec3d :: dotProd(tr.normal,Vec3d(0,0,1));
  // avoid round off errors
  cosrho = (cosrho > 1. ? 1. : cosrho);
  cosrho = (cosrho < -1. ? -1. : cosrho);
  float rho = acos(cosrho);
  float c = get_sphere_color_rho(rho);
  glColor3f(c,c,c);
}

void Triangle :: uncert_afm_sphere_tip(SphereTip sp) {

  double R = sp.r;
  Vec3d offset = normal*R;
  Triangle tr = Triangle(a+offset, b+offset, c+offset); 

  /* now get triangle color depending on orientation of the triangle
   * use the color of the pt where the spherical part of the tip touches
   * the triangle.
   */
  set_tri_color(tr);

  tr.draw();
  
  ab.uncert_afm_sphere_tip(sp);
  bc.uncert_afm_sphere_tip(sp);
  ca.uncert_afm_sphere_tip(sp);

}

void Triangle :: afm_inv_cone_sphere_tip(InvConeSphereTip icsTip) {
  double R = icsTip.r;
  Vec3d offset = normal*R;
  Triangle tr = Triangle(a+offset, b+offset, c+offset); 

  tr.draw();
  
  ab.afm_inv_cone_sphere_tip(icsTip);
  bc.afm_inv_cone_sphere_tip(icsTip);
  ca.afm_inv_cone_sphere_tip(icsTip);
}

void Triangle :: uncert_afm_inv_cone_sphere_tip(InvConeSphereTip icsTip) {
  double R = icsTip.r;
  Vec3d offset = normal*R;
  Triangle tr = Triangle(a+offset, b+offset, c+offset); 
  set_tri_color(tr);
  tr.draw();
  
  ab.uncert_afm_inv_cone_sphere_tip(icsTip);
  bc.uncert_afm_inv_cone_sphere_tip(icsTip);
  ca.uncert_afm_inv_cone_sphere_tip(icsTip);
}

int count=0;

void Triangle :: keyboardFunc(unsigned char key, int x, int y) {

  if (buttonpress==RIGHT_BUTTON) {
    /* This means, we are performing operations on a side of the triangle */
    switch (selected_triangle_side) {
    case 1:
      ab.keyboardFunc(key,x,y);
      // recalculate params for ab
      a = ab.pos - ab.axis*ab.leng/2.;
      b = ab.pos + ab.axis*ab.leng/2.;
      set(a,b,c);
      break;
    case 2:
      bc.keyboardFunc(key,x,y);
      // recalculate params for bc
      b = bc.pos - bc.axis*bc.leng/2.;
      c = bc.pos + bc.axis*bc.leng/2.;
      set(a,b,c);
      break;
    case 3:
      ca.keyboardFunc(key,x,y);
      // recalculate params for ca
      c = ca.pos - ca.axis*ca.leng/2.;
      a = ca.pos + ca.axis*ca.leng/2.;
      set(a,b,c);
      break;
    default :
      break;
    }
  }
  else {
    switch (key) {
    case '+' :       
      setPos_z(pos.z+DIST_UNIT);
      break;
    case '-' : 
      setPos_z(pos.z-DIST_UNIT);
      break;
    case 'x':
      a = pos+Vec3d(a-pos).rotate3(Vec3d(1,0,0),DEG_TO_RAD*ANGLE_UNIT);
      b = pos+Vec3d(b-pos).rotate3(Vec3d(1,0,0),DEG_TO_RAD*ANGLE_UNIT);
      c = pos+Vec3d(c-pos).rotate3(Vec3d(1,0,0),DEG_TO_RAD*ANGLE_UNIT);
      set(a,b,c);
      break;
    case 'X':
      a = pos+Vec3d(a-pos).rotate3(Vec3d(1,0,0),-DEG_TO_RAD*ANGLE_UNIT);
      b = pos+Vec3d(b-pos).rotate3(Vec3d(1,0,0),-DEG_TO_RAD*ANGLE_UNIT);
      c = pos+Vec3d(c-pos).rotate3(Vec3d(1,0,0),-DEG_TO_RAD*ANGLE_UNIT);
      set(a,b,c);
      break;
    case 'y':
      a = pos+Vec3d(a-pos).rotate3(Vec3d(0,1,0),DEG_TO_RAD*ANGLE_UNIT);
      b = pos+Vec3d(b-pos).rotate3(Vec3d(0,1,0),DEG_TO_RAD*ANGLE_UNIT);
      c = pos+Vec3d(c-pos).rotate3(Vec3d(0,1,0),DEG_TO_RAD*ANGLE_UNIT);
      set(a,b,c);
      break;
    case 'Y':
      a = pos+Vec3d(a-pos).rotate3(Vec3d(0,1,0),-DEG_TO_RAD*ANGLE_UNIT);
      b = pos+Vec3d(b-pos).rotate3(Vec3d(0,1,0),-DEG_TO_RAD*ANGLE_UNIT);
      c = pos+Vec3d(c-pos).rotate3(Vec3d(0,1,0),-DEG_TO_RAD*ANGLE_UNIT);
      set(a,b,c);
      break;
    case 'z':
      a = pos+Vec3d(a-pos).rotate3(Vec3d(0,0,1),DEG_TO_RAD*ANGLE_UNIT);
      b = pos+Vec3d(b-pos).rotate3(Vec3d(0,0,1),DEG_TO_RAD*ANGLE_UNIT);
      c = pos+Vec3d(c-pos).rotate3(Vec3d(0,0,1),DEG_TO_RAD*ANGLE_UNIT);
      set(a,b,c);
      break;
    case 'Z':
      a = pos+Vec3d(a-pos).rotate3(Vec3d(0,0,1),-DEG_TO_RAD*ANGLE_UNIT);
      b = pos+Vec3d(b-pos).rotate3(Vec3d(0,0,1),-DEG_TO_RAD*ANGLE_UNIT);
      c = pos+Vec3d(c-pos).rotate3(Vec3d(0,0,1),-DEG_TO_RAD*ANGLE_UNIT);
      set(a,b,c);
      break;
    default :
      break;
    }
  }
}

void Triangle :: print() {
  cout << "triang : [";
  a.print();  
  b.print();
  c.print();
  cout << "]\n";
#if 0
  cout << "Ntubes ab\n";
  ab.print();
  bc.print();
  ca.print();
  exit(0);
#endif
}

double Triangle :: xy_distance(Vec3d vMouseWorld) {
  return norm_xy(vMouseWorld - pos);
}

double Triangle :: xz_distance(Vec3d vMouseWorld) {
  return norm_xz(vMouseWorld - pos);
}

void Triangle :: grabOb(Vec3d vMouseWorld, int xy_or_xz) {
  // store the grab offset
  vGrabOffset = pos   - vMouseWorld;
}

void Triangle :: moveGrabbedOb(Vec3d vMouseWorld) {
  // As vMouseWorld changes, move the object
  	Vec3d new_pos(vMouseWorld.x + vGrabOffset.x, vMouseWorld.y + vGrabOffset.y, vMouseWorld.z + vGrabOffset.z);
    Vec3d diff(new_pos.x,new_pos.y,new_pos.z);
	diff -= pos;
	for(int i = 0; i < number_in_group[obj_group];i++){//obj_group local to 'this'
		OB * this_obj = group_of_obs[obj_group][i];
		Vec3d this_pos;
		this_pos += this_obj->pos;
		this_pos += diff;
		this_obj->setPos(this_pos);
	}
}

void addTriangle(Vec3d a, Vec3d b, Vec3d c,int* group_number) {
  ob[numObs] = new Triangle(a,b,c);
  selectedOb = numObs; 
  //testing
  /*group_number = new int();
  *group_number = 0;*/
  int num = addToGroup(ob[numObs],group_number);
  ob[numObs]->obj_group = num;
  numObs++;
}

