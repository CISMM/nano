/* Gokul Varadhan
 * varadhan@cs.unc.edu
 * May 2001
 */
#include <stdlib.h>		//stdlib.h vs cstdlib
#include <stdio.h>		//stdio.h vs cstdio
#include <iostream>
#include <fstream>
#include "3Dobject.h"
#include <math.h>		//math.h vs cmath
#include <GL/glut_UNC.h>
#include "defns.h"
#include "Tips.h"
#include "sim.h"

Dna *dna = NULL;

int timeSteps=0;
Dna :: Dna(void) {
  numSegments = 0;
  segs = NULL; 
}

Dna :: Dna(Vec3d _P1, Vec3d _P2, Vec3d _dP1, Vec3d _dP2, double _length, int numSegs) {
  numSegments = numSegs;
  init_state();
  sim_done=0;
  set(_P1,_P2,_dP1,_dP2,_length);
}

void Dna :: init_state() {
  segs = (Ntube **) malloc (numSegments*sizeof(Ntube *));  
  F = (Vec3d *) malloc ((numSegments+1)*sizeof(Vec3d));
  //pos = (Vec3d *) malloc ((numSegments+1)*sizeof(Vec3d));
  pos = new Vec3d[numSegments+1];
  rest_lengths = (double *) malloc (numSegments*sizeof(double));
  constraint_list = (bool *) malloc ((numSegments+1)*sizeof(bool));  
}

/* returns M * [P1v P2v dP1v dP2v]'
 *
 * (' means transpose as in MATLAB)
 */
void Dna :: get_coeff(float *coeff, float *M, double P1v, double P2v, double dP1v, double dP2v) {
  coeff[0] = M[0]*P1v + M[1]*P2v + M[2]*dP1v + M[3]*dP2v;
  coeff[1] = M[4]*P1v + M[5]*P2v + M[6]*dP1v + M[7]*dP2v;
  coeff[2] = M[8]*P1v + M[9]*P2v + M[10]*dP1v + M[11]*dP2v;
  coeff[3] = M[12]*P1v + M[13]*P2v + M[14]*dP1v + M[15]*dP2v;
}

/* return at^3 + bt^2 + ct + d */
double Dna :: cubic(float *coeff, double t) {
  return pow(t,3)*coeff[0] + pow(t,2)*coeff[1] + t*coeff[2] + coeff[3]; 
}


/* The curve can be expresses as a cubic polynomial in Hermite form as
 *
 * [ x(t) y(t) z(t) ] = [t^3 t^2 t 1] * M * [P1 P2 dP1 dP2]'
 *
 * where M = [ 2 -2  1   1
 *            -3  3  -2 -1
 *             0  0   1  0
 *             1  0   0  0 ]       
 *
 */
Vec3d Dna :: calc_hermite(Vec3d P1, Vec3d P2, Vec3d dP1, Vec3d dP2, double t) {
  float M[] = {2, -2, 1, 1,
	       -3, 3, -2, -1,
	       0, 0, 1, 0,
	       1, 0, 0, 0};
  float coeffx[4], coeffy[4], coeffz[4];

  get_coeff(coeffx, M, P1.x, P2.x, dP1.x, dP2.x);
  get_coeff(coeffy, M, P1.y, P2.y, dP1.y, dP2.y);
  get_coeff(coeffz, M, P1.z, P2.z, dP1.z, dP2.z);
  return Vec3d(cubic(coeffx, t), cubic(coeffy, t), cubic(coeffz, t));

}

void Dna :: init_dna_hermite() {
  int i;


  // use the info
  pos[0] = P1;
  // put the remaining vertices on the hermite curve whose endpts are 
  // P1 and P2
  for (i=1;i<(numSegments+1);i++) {
    double t = (double)i/numSegments;
    pos[i] = calc_hermite(P1,P2,dP1,dP2,t);
    segs[i-1] = new Ntube(pos[i-1],pos[i],SEG_DIAM);
  }

  // init 
  for (i=0;i<(numSegments+1);i++) {
    constraint_list[i]=0;
  }
  // constrain the end four pts
  constraint_list[0] = 1;
  constraint_list[1] = 1;
  constraint_list[numSegments-1] = 1;
  constraint_list[numSegments] = 1;

  // account for the length of the constrained segments
  Vec3d t1 = pos[1] - pos[0];
  Vec3d t2 = pos[numSegments] - pos[numSegments-1];
  double rest_length = (length-t1.magnitude()-t2.magnitude())/(numSegments-2);

  for (i=0;i<numSegments;i++) {
    rest_lengths[i] = rest_length;
  }
  rest_lengths[0] = t1.magnitude();
  rest_lengths[numSegments-1] = t2.magnitude();
}


void Dna :: set(Vec3d _P1, Vec3d _P2, Vec3d _dP1, Vec3d _dP2, double _length) {
  P1 = _P1;
  P2 = _P2;
  dP1 = _dP1;
  dP2 = _dP2;
  length = _length;
  init_dna_hermite();
}

extern void	setColor( int colorIndex );
void Dna :: draw() {
  for (int i=0;i<numSegments;i++) {


    // we want different colors for our objects
    if ((i % 3) == 0)
      setColor( YELLOW );
    else if ((i % 3) == 1)
      setColor( GREEN );
    else 
      setColor( BLUE );
    segs[i]->draw();
  }
}


/* Given a segment with endpoints a and b, 
 * this gives the force in the direction from a to b 
 *
 * This force corresponds to the force due to the stretching of the spring
 * along the segment.
 *
 * force = K * (rest_length - |b-a|) * unit(b-a)
 *
 * where unit(b-a) is the unit vector from a to b
 *
 */
Vec3d Dna :: stretched_spring_force(int i) {
  Vec3d v = pos[i+1]-pos[i];
  double magni = SPRING_CONSTANT*(rest_lengths[i] - v.magnitude());
  // now the direction
  Vec3d dir = v;
  dir.normalize();
  return (dir * magni);
}


/* We are given three points a, b, and c such that a is constrained, (b and c 
 * aren't) and a force f that is being applied to point c perpendicular to bc.
 *
 * This calculates the reaction force on point b.
 *
 * We basically balance the torque
 *
 * Fb * |ab| = |f|*cos(m) * |ac|
 *
 * m = angle between bc and ac
 * 
 *
 * This gives us the required force Fb
 *
 *
 * The force acts in a direction perpendicular to ab in the direction of the
 * interior of triangle abc
 *
 *  i.e       (ba X bc) X ba
 */
Vec3d Dna :: calc_reaction_force(Vec3d a, Vec3d b, Vec3d c, Vec3d f) {
  Vec3d ba = a-b;
  Vec3d bc = c-b;
  Vec3d ac = c-a;
  
  double m = Vec3d :: angleBetween(bc,ac);
  double Fb = f.magnitude() * cos(m) * (ac.magnitude())/(ba.magnitude());
  // let us get the direcn
  Vec3d uba = ba.normalize();
  Vec3d ubc = bc.normalize();
  Vec3d uac = ac.normalize();
  Vec3d dir = Vec3d :: crossProd (Vec3d :: crossProd(uba,ubc), uba);
  return dir * Fb;
}

/* 
 * We also have springs between adjacent segments which tries to reduce the 
 * angle between successive segments and thus reduce curvature of the DNA 
 * strand.
 *
 * force = ROTATION_SPRING_CONSTANT * (180 - theta) where theta is the angle between the 
 * adjacent segments.
 *
 *
 */
void Dna :: bending_spring_force(int i) {

  // the two segments first
  Vec3d a = pos[i-1] - pos[i];
  a = a.normalize();

  Vec3d b = pos[i+1] - pos[i];
  b = b.normalize();

  // the angle between the adjacent segments
  double dot = Vec3d :: dotProd(a,b);
  if (dot < -1)
    dot = -1;
  else if (dot > 1) 
    dot = 1;
  double theta = acos(dot);

  // magnitude of the force
  double Fmagni = ROTATION_SPRING_CONSTANT *fabs(PI-theta);

  // the vector a x b
  Vec3d temp = Vec3d :: crossProd(a,b);
  temp = temp.normalize();

  // force on the left segment
  Vec3d dir1 = Vec3d :: crossProd(a,temp);
  dir1 = dir1.normalize();
  Vec3d f1 = dir1 * Fmagni;

  // force on the right segment
  Vec3d dir2 = Vec3d :: crossProd(temp,b);
  dir2 = dir2.normalize();
  Vec3d f2 = dir2 * Fmagni;

  // case analysis (this assume no three consec pts are constrained)

  if (constraint_list[i-1]) {// i-1 is constr
    F[i-1] = Vec3d(0.,0.,0.);
    if (!constraint_list[i]) {// case 1 : i-1 constr, i unconstr
      Vec3d a = pos[i-1];
      Vec3d b = pos[i];
      Vec3d c = pos[i+1];
      F[i] += calc_reaction_force(a,b,c,f2);
      F[i+1] += f2; 
    }
    else {// case 2 : i-1 and i are constr
      F[i] = Vec3d(0.,0.,0.);
      F[i+1] += f2;       
    }
  }
  else if (constraint_list[i+1]) {// i-1 is unconstr, i+1 is constrained
    F[i+1] = Vec3d(0.,0.,0.);
    if (!constraint_list[i]) {// case 3 : i unconstr, i+1 constr
      F[i-1] += f1; 
      Vec3d a = pos[i+1];
      Vec3d b = pos[i];
      Vec3d c = pos[i-1];
      F[i] += calc_reaction_force(a,b,c,f1);
    }
    else {// case 4: i, i+1 constr
      F[i-1] += f1;       
      F[i] = Vec3d(0.,0.,0.);
    }
  }
  else {// i-1, i+1 are unconstrained
    F[i-1] += f1;
    F[i+1] += f2;
  }

  if (constraint_list[i]) {
    F[i] = Vec3d(0.,0.,0.);
  }

}

/* This calculates/increments the stretched forces for pts i and i+1 of 
 * segment i. 
 * 
 */
void Dna :: set_segment_stretched_forces(int i) {
  // calculate the force in this segment
  Vec3d f = stretched_spring_force(i);
  
  /* The assignments and increments in the following piece of code has
   * been worked out carefully.
   *
   * Change them at your own risk !!!!
   */
  // case analysis
  if (constraint_list[i] && constraint_list[i+1]) {
    F[i] = Vec3d(0.,0.,0.);
    F[i+1] = Vec3d(0.,0.,0.);
  }
  else if (!constraint_list[i] && constraint_list[i+1]) {
    F[i] += f*(-1);
    F[i+1] = Vec3d(0.,0.,0.);
  }
  else if (constraint_list[i] && !constraint_list[i+1]) {
    F[i] = Vec3d(0.,0.,0.);
    F[i+1] = f;
  }
  else {//if (!constraint_list[i] && !constraint_list[i+1])
    F[i] += -f;
    F[i+1] = f;
  }
}

/* calculate force at each of the segments ends 
 *
 * for each pt,
 *         f1 = stretched force on left segment / 2.
 *         f2 = stretched force on right segment / 2.
 *         f for this pt =  f1 + f2 (add with appropriate signs)
 * 
 * Note that we have a division factor of 2 above. 
 *
 * In addition to the stretched spring forces, also calculate the bending
 * spring forces.
 * 
 *
 */
void Dna :: calc_force_at_all_pts() {

  F[0] = Vec3d(0.0,0.,0.); // a weird initialization step
  for (int i=0;i<numSegments; i++) {
    /* 
     * this figures out the forces on points i and i+1 due to segment i
     */
    set_segment_stretched_forces(i);
  }

  // calc the bending spring forces.
  for (i=1;i<numSegments; i++) {
    bending_spring_force(i);
  }
}

/* Given current posns and the forces, calculate new positions over a time
 * interval
 *
 * Assuming a small time interval dt,
 *
 * newpos = curpos + (F/m)*dt*dt
 *
 */
///check on this:  think the equation might by r = F/m * (t^2)/2
void Dna :: compute_new_pos() {
  int i;
  for (i=0;i<(numSegments+1);i++) {
    pos[i] = pos[i] + F[i]*(TIME_INTERVAL*TIME_INTERVAL/MASS);
  }
  for (i=0;i<numSegments;i++) {
    segs[i]->set(pos[i],pos[i+1],SEG_DIAM);
  }
}

/* 
 * print the angles between adjacent segments 
 *
 */
void Dna :: print_angles() {
  for (int i=1;i<numSegments;i++) {
    Vec3d a = pos[i-1] - pos[i];
    a = a.normalize();
    Vec3d b = pos[i+1] - pos[i];
    b = b.normalize();
    double theta = acos(Vec3d :: dotProd(a,b));
    cout << " Ang at i = " << 180*theta/PI;
  }
  cout << "\n";
}



// print an array of Vec3d
void Dna :: print() {
  for (int i=0;i<(numSegments+1);i++) {
    pos[i].print();
  }
}

void Dna :: printF() {
  for (int i=0;i<(numSegments+1);i++) {
    F[i].print();
  }
}

double Dna :: maxF() {
  double m;
  
  m = pos[0].magnitude();

  for (int i=1; i<(numSegments+1); i++) {
    double magni = pos[i].magnitude();
    if (magni > m) {
      m = magni;
    }
  }
  return m;
}

double Dna :: calc_dna_length() {
  double len = 0.;
  for (int i=0;i<numSegments;i++) {
    Vec3d t = pos[i+1] - pos[i];
    len +=  t.magnitude();
  }
  return len;
}

int start_sim=0;
void Dna :: run() {
  if (maxF() > FORCE_THRESHOLD) {
    calc_force_at_all_pts(); 
    compute_new_pos();
    timeSteps++;

    if ((timeSteps % 50) == 0) {
      double dna_len = calc_dna_length();
      printf("Total length = %lf\n",dna_len);
    }

  }
  else {// if forces small enough, simulation done !
    sim_done = 1;
    // calc total length, output it
    double dna_len = calc_dna_length();
    cout << "Total length = " << dna_len << endl;
    //    exit(0);
  }
}

int Dna :: done_run() {
  return sim_done;
}

void Dna :: afm_sphere_tip(SphereTip sp) {
  for (int i=0;i<numSegments;i++) {
    segs[i]->afm_sphere_tip(sp);
  }
}

void Dna :: uncert_afm_sphere_tip(SphereTip sp) {
  for (int i=0;i<numSegments;i++) {
    segs[i]->uncert_afm_sphere_tip(sp);
  }
}

void Dna :: afm_inv_cone_sphere_tip(InvConeSphereTip ics) {
  for (int i=0;i<numSegments;i++) {
    segs[i]->afm_inv_cone_sphere_tip(ics);
  }
}

void Dna :: uncert_afm_inv_cone_sphere_tip(InvConeSphereTip ics) {
  for (int i=0;i<numSegments;i++) {
    segs[i]->uncert_afm_inv_cone_sphere_tip(ics);
  }
}

double Dna :: xy_distance(Vec3d vMouseWorld) {
  double dist=1000000.;
  for (int i=0;i<numSegments;i++) {
    double t = segs[i]->xy_distance(vMouseWorld);
    dist = (t < dist ? t : dist); 
  }
  
  return dist;
}

double Dna :: xz_distance(Vec3d vMouseWorld) {
  double dist=1000000.;
  for (int i=0;i<numSegments;i++) {
    double t = segs[i]->xz_distance(vMouseWorld);
    dist = (t < dist ? t : dist); 
  }

  return dist;
}

void Dna :: grabOb(Vec3d vMouseWorld, int xy_or_xz) {
  double dist=1000000.;
  for (int i=0;i<numSegments;i++) {
    double t;
    if (xy_or_xz == XY_GRAB) {
      t = segs[i]->xy_distance(vMouseWorld);
    }
    else {
      t = segs[i]->xz_distance(vMouseWorld);
    }
    if (t < dist) {
      dist = t;
      closestSeg = i;
    }
  }
  printf("close seg %d\n",closestSeg);
  //  exit(0);
  vGrabOffset = segs[closestSeg]->pos - vMouseWorld;
}

int __cnt=0;
void Dna :: moveGrabbedOb(Vec3d vMouseWorld) {

  //  segs[0]->print();

  // first by how much are we moving the above segment
  Vec3d offset = Vec3d(vMouseWorld.x + vGrabOffset.x, vMouseWorld.y + vGrabOffset.y, vMouseWorld.z + vGrabOffset.z) 
    - segs[closestSeg]->pos;
  cout << "offset before = " << offset << endl;
  /*int count = 0;
  while(offset.magnitude() > 1 && (count < 100)){//cut it down to a reasonable size
    Vec3d tempMouseWorld = vMouseWorld/10;
    offset = tempMouseWorld + vGrabOffset - segs[closestSeg]->pos;
    count++;
  }
  */
  cout << "offset after = " << offset << endl;
  
  //  offset.print();

  // As vMouseWorld changes, move the object
  // update segment
  segs[closestSeg]->setPos( segs[closestSeg]->pos + offset );
  
  // update pos
  pos[closestSeg] = pos[closestSeg] + offset;
  pos[closestSeg+1] = pos[closestSeg] + offset;

  // need to change the orientation of the adjacent two segments
  // first need to figure out couple of things

  if (closestSeg > 0) {
    // left segment
    Vec3d leftendpt = segs[closestSeg-1]->pos - segs[closestSeg-1]->axis * ((segs[closestSeg-1]->leng)/2.);
    Vec3d rightendpt = segs[closestSeg-1]->pos + segs[closestSeg-1]->axis * ((segs[closestSeg-1]->leng)/2.);
    Vec3d newrightendpt = rightendpt + offset;

    segs[closestSeg-1]->set(leftendpt, newrightendpt, segs[closestSeg-1]->diam);
  }

  if (closestSeg < (numSegments-1)) {

    Vec3d t1 = segs[closestSeg+1]->getLeftEndPt();
    Vec3d t2 = segs[closestSeg+1]->getRightEndPt();
    t1.print();
    t2.print();

    // now for the right segment
    Vec3d leftendpt = segs[closestSeg+1]->pos - segs[closestSeg+1]->axis * ((segs[closestSeg+1]->leng)/2.);
    Vec3d rightendpt = segs[closestSeg+1]->pos + segs[closestSeg+1]->axis * ((segs[closestSeg+1]->leng)/2.);
    
    Vec3d newleftendpt = leftendpt + offset;
    segs[closestSeg+1]->set(newleftendpt, rightendpt, segs[closestSeg+1]->diam);
  }
  sim_done=0;

  //  segs[0]->print();
  //  exit(0);

  
}
  

Dna* addDna(Vec3d P1, Vec3d P2, Vec3d dP1, Vec3d dP2, double length, int numSegs) {
  Dna *d = new Dna(P1, P2, dP1, dP2, length, numSegs);
  ob[numObs] = d;
  ob[numObs]->type = DNA;
  selectedOb = numObs;
  numObs++;
  return d;
}


void init_dna(char *filename) {
  double dna_length;
  int numSegments;
  Vec3d P1, P2, dP1, dP2;

  ifstream fin;
  fin.open(filename);
  cout << "filename=" << filename << endl;
  if(!fin.fail()){
	fin >> numSegments >> dna_length >> P1.x >> P1.y >> P1.z >> P2.x >> P2.y >> P2.z
		>> dP1.x >> dP1.y >> dP1.z >> dP2.x >> dP2.y >> dP2.z;
	dna = addDna(P1, P2, dP1, dP2, dna_length, numSegments);
  }
  else{
	  cout << "Could not open " << filename << "." << endl;
  }
}




