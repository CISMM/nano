/*$Id$*/

/* Gokul Varadhan
 * Sept 2000
 *
 * 3D objects
 *
 */


#ifndef _3DOBJECT_H_
#define _3DOBJECT_H_

#include "vec3d.h"
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

/* The most general object - this is an abstract class */
class OB {
public :
  int type;
  Vec3d pos;
  virtual void setPos(Vec3d)=0; // pure virtual function
  void setPos_z(double);
  virtual void translate(Vec3d)=0;
  virtual void scale(double)=0;
  virtual void print()=0;
  virtual void draw()=0;
  virtual void afm_sphere_tip(SphereTip)=0;
  virtual void afm_inv_cone_sphere_tip(InvConeSphereTip)=0;
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
};

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
  int   nextSeg;	// link to next segment of bendable tube (can be null)
  int   prevSeg;	// link to previous segment of bendable tube  (can be null)
  int   moved;	// 1=already moved this sim step; 0=free to be moved.

  Ntube( void );
  Ntube(Vec3d leftendpt, Vec3d rightendpt, double diam);
  Ntube(int _type, Vec3d _pos, double _yaw, double _roll, double _pitch, double _length, double _diameter, int _nextSeg, int _prevSeg);
  void set( int _type, Vec3d _pos, double _angle, double _roll, double _pitch, double _length, double _diameter, int _nextSeg, int _prevSeg );
  void set(Vec3d leftendpt, Vec3d rightendpt, double diam);
  void setPos(Vec3d);
  void translate(Vec3d);
  void scale(double);
  void setDiam(double _diameter);
  void setLength(double _length);
  void setYaw(double);
  void setRoll(double);
  void setPitch(double);
  void print();
  void draw();
  void afm_sphere_tip(SphereTip);
  void afm_inv_cone_sphere_tip(InvConeSphereTip);
  void keyboardFunc(unsigned char key, int x, int y);
};

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
  void afm_inv_cone_sphere_tip(InvConeSphereTip);
  void keyboardFunc(unsigned char key, int x, int y);
};



/**************************************************************************************/
// FUNCTIONS
void	error( char* errMsg );

#endif // ROBOT_H_GUARD
