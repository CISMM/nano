/*$Id$*/

/* Gokul Varadhan
 * Sept 2000
 * varadhan@cs.unc.edu
 *
 * 3D objects
 *
 */


#ifndef _3DOBJECT_H_
#define _3DOBJECT_H_

#include "Vec3d.h"
#include "Tips.h"


#define NULLOB (-1)
// object types
#define UNUSED 0
#define NTUBE 1
/* Although sphere is a special case, we make the distinction at some places
 * for sake of optimizations
 */
#define SPHERE 2 
#define TRIANGLE 3
#define DNA 4
#define CYLINDER 5

// whether the object was grabbed (by the mouse) along XY or YZ
#define XY_GRAB 0
#define XZ_GRAB 1

/* The most general object - this is an abstract class */
class OB {
public :
  int type;
  Vec3d pos;
  Vec3d vGrabOffset;
  int obj_group;

  void setPos_z(double);
  virtual void setPos(Vec3d)=0; // pure virtual function
  virtual void translate(Vec3d)=0;
  virtual void scale(double)=0;
  virtual void print()=0;
  virtual void draw()=0;
  virtual void afm_sphere_tip(SphereTip)=0;
  virtual void uncert_afm_sphere_tip(SphereTip sp)=0;
  virtual void afm_inv_cone_sphere_tip(InvConeSphereTip)=0;
  virtual void uncert_afm_inv_cone_sphere_tip(InvConeSphereTip)=0;
  /* Each object manages its keyboard. This might seem funny, but it allows
   * lot of programming convenience. The reason for doing this is that I want
   * everything dealing with an object to be encapsulated in it. Otherwise, if
   * I were to have one common keyboard funcn in the main file, I would need 
   * to access the fields of different objects (these fields are different for
   * different objects)
   * 
   * This might not be efficient, but I chose ease over efficiency.
   */
  virtual void keyboardFunc(unsigned char key, int x, int y)=0;
  // mouse routines
  virtual double xy_distance(Vec3d vMouseWorld)=0;
  virtual double xz_distance(Vec3d vMouseWorld)=0;
  virtual void grabOb(Vec3d vMouseWorld, int xy_or_xz)=0;
  virtual void moveGrabbedOb(Vec3d vMouseWorld)=0;
  double norm_xy( Vec3d v );
  double norm_xz( Vec3d v );
};

int addToGroup(OB* obj,int* group_number);
int changeGroup(OB* obj,int* new_group_number);
bool inGroup(OB* obj,int* group_number);
void removeFromGroup(OB* obj,int* group_number);
void saveAllGroups();
void retrieveAllGroups();

// subclass of OB
class Ntube : public OB {
private:
  void recalc_all();
public:
  Vec3d axis; // unit axis vector of the nano tube
  
  // these are in radians
  double yaw;	// for in-XY-plane rotations
  double roll;		// for tube rotating around its axis
  double pitch; 
  
  double leng;		// length of the tube
  double diam;		// diameter of the tube
  Ntube( void );
  Ntube(Vec3d leftendpt, Vec3d rightendpt, double diam);
  Ntube(int _type, Vec3d _pos, double _yaw, double _roll, double _pitch, double _length, double _diameter);
  void set( int _type, Vec3d _pos, double _angle, double _roll, double _pitch, double _length, double _diameter);
  void set(Vec3d leftendpt, Vec3d rightendpt, double diam);
  void setPos(Vec3d);
  void translate(Vec3d);
  void scale(double);
  void setDiam(double _diameter);
  void setLength(double _length);
  void setYaw(double);
  void setRoll(double);
  void setPitch(double);
  Vec3d getLeftEndPt(); // the one away from the direction of the axis
  Vec3d getRightEndPt(); // the one in the direction of the axis
  void print();
  void draw();
  void uncert_draw();
  void afm_sphere_tip(SphereTip);
  void uncert_afm_sphere_tip(SphereTip sp);
  void afm_inv_cone_sphere_tip(InvConeSphereTip);
  void uncert_afm_inv_cone_sphere_tip(InvConeSphereTip);
  void keyboardFunc(unsigned char key, int x, int y);
  double xy_distance(Vec3d vMouseWorld);
  double xz_distance(Vec3d vMouseWorld);
  void grabOb(Vec3d vMouseWorld, int xy_or_xz);
  void moveGrabbedOb(Vec3d vMouseWorld);
};

void addNtube(int type, Vec3d pos, double yaw, double roll, double pitch, double leng, double diam,
			  int *group_number = NULL);


// subclass of OB
/* pos here is the location of the centroid  = (a+b+c)/3 */
class Triangle : public OB {
public:
  // the three sides
  Vec3d a;
  Vec3d b;
  Vec3d c;
  Vec3d normal;
  
  /* We treat the sides ab, bc, ca as Ntubes */
  Ntube ab;
  Ntube bc;
  Ntube ca;

  Triangle(void);
  Triangle(Vec3d _a, Vec3d _b, Vec3d _c);
  void set(Vec3d _a, Vec3d _b, Vec3d _c);
  void setPos(Vec3d _pos);
  void translate(Vec3d);
  void scale(double);
  void print();
  void draw();
  void afm_sphere_tip(SphereTip);
  void uncert_afm_sphere_tip(SphereTip sp);
  void afm_inv_cone_sphere_tip(InvConeSphereTip);
  void uncert_afm_inv_cone_sphere_tip(InvConeSphereTip);
  void keyboardFunc(unsigned char key, int x, int y);
  double xy_distance(Vec3d vMouseWorld);
  double xz_distance(Vec3d vMouseWorld);
  void grabOb(Vec3d vMouseWorld, int xy_or_xz);
  void moveGrabbedOb(Vec3d vMouseWorld);
};

void addTriangle(Vec3d a, Vec3d b, Vec3d c,int *group_number = NULL);


/* DNA :
 *
 * We model the DNA as a piece-wise linear set of segments where each segment 
 * is further modeled as a Ntube. Initially, we have the following information
 *  about DNA the position and orientations at the end pts a, b, adir and bdir
 *  resply. the length of the DNA strand between a and b and we are given the 
 * number of segments.
 *
 *
 * Initially, we place the segment end pts on a Hermite curve. This achieves 
 * the end pt position and orientation constraints but not the length 
 * constraint. To capture the length constraint, we do a physically based 
 * spring simulation as follows.
 *
 * Physically based simulation :
 *
 * We model the DNA strand to have two kinds of springs :
 *    1. a spring on each segment that tries to keep the segment end pts
 *       at a certain distance (= rest length of the spring)
 *       Given a segment (p, q) with end pts p and q, the force on the spring
 *       is given by
 *
 *       F = K * (|q-p| - rest_length)
 * 
 *       We apply an "appropriate" part of the force F to each end point. 
 *       
 *    
 *    2. a spring between adjacent segments that tries to keep the two 
 *       segments aligned. 
 *
 *       If theta is the angle between the adjacent segments, the angular 
 *       force is given by
 *
 *       F = Ktheta * (PI - theta)
 *
 * We compute forces on each end point and then move these pts by a 
 * displacement
 *
 *      c * Force at that pt
 *
 * The simulation stops when the forces on each of the pts become negligible.
 *
 * We put the initial pts on the Hermite curve to achieve the end pt position
 * and orientation constraints. But the above simulation can undo this. To 
 * prevent this, we add additional constraints.
 *
 * We classify certain points as "constrained" (as immovable).
 * 
 */
class Dna : public OB {
#define SEG_DIAM 1
#define MASS 1
#define SPRING_CONSTANT 1
#define ROTATION_SPRING_CONSTANT 0.1
#define FORCE_THRESHOLD 0.2
#define TIME_INTERVAL 0.2
private :
  Vec3d *F;
  Vec3d *pos;
  bool *constraint_list;
  double *rest_lengths;
  void init_state();
  void get_coeff(float *coeff, float *M, double P1v, double P2v, double dP1v, double dP2v);
  double cubic(float *coeff, double t);
  Vec3d calc_hermite(Vec3d P1, Vec3d P2, Vec3d dP1, Vec3d dP2, double t);
  // initializes the DNA segments end pts to lie on a Hermite
  void init_dna_hermite();
  Vec3d stretched_spring_force(int i);
  Vec3d calc_reaction_force(Vec3d a, Vec3d b, Vec3d c, Vec3d f);
  void bending_spring_force(int i);
  void set_segment_stretched_forces(int i);
  void calc_force_at_all_pts();
  void compute_new_pos();
  void print_angles();
  void print_vectors();
  double maxF();
  double calc_dna_length();
  // internal storage vbls
  int closestSeg;
  int sim_done;
public :
  Vec3d P1;
  Vec3d P2;
  Vec3d dP1;
  Vec3d dP2;
  int numSegments;
  double length;
  Ntube **segs;
  Dna(void);
  /* DNA whose length and end pts and end pt orientations are given by P1, P2, 
   * P1dir and P2dir resply. numSegs is the number of segments the DNA model is 
   * made up of .
   */
  Dna(Vec3d _P1, Vec3d _P2, Vec3d _dP1, Vec3d _dP2, double _length, int numSegs);
  void set(Vec3d _P1, Vec3d _P2, Vec3d _dP1, Vec3d _dP2, double _length);
  void draw();
  // this runs the DNA physically based simulation step
  void run();
  int done_run();

  void setPos(Vec3d) {};
  void translate(Vec3d) {};
  void scale(double) {};
  void print();
  void printF();
  void afm_sphere_tip(SphereTip);
  void uncert_afm_sphere_tip(SphereTip sp);
  void afm_inv_cone_sphere_tip(InvConeSphereTip);
  void uncert_afm_inv_cone_sphere_tip(InvConeSphereTip);
  void keyboardFunc(unsigned char key, int x, int y) {};
  double xy_distance(Vec3d vMouseWorld);
  double xz_distance(Vec3d vMouseWorld);
  void grabOb(Vec3d vMouseWorld, int xy_or_xz);
  void moveGrabbedOb(Vec3d vMouseWorld);
};

extern Dna *dna;

Dna *addDna(Vec3d P1, Vec3d P2, Vec3d dP1, Vec3d dP2, double length, int numSegs,int *group_number = NULL);

void init_dna(char *filename);


/**************************************************************************************/
// FUNCTIONS
void	error( char* errMsg );

#endif // ROBOT_H_GUARD
