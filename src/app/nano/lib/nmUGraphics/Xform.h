#ifndef XFORM_H 
#define XFORM_H

// make the SGI compile without tons of warnings
#ifdef sgi
#pragma set woff 1110,1424,3201
#endif

#include <iostream>
using namespace std;

// and reset the warnings
#ifdef sgi
#pragma reset woff 1110,1424,3201
#endif

#include <quat.h>

#include "Xform4x4.h"		//adds additional xform functionality for a generalized 4x4 matrix

#define XP_4BY4 0		// CONSTANTS FOR PRINT
#define XP_RAW 1
#define XP_STATIC 2


//THIS TRANSFORMATION CLASS IS A LIMITED XFORMATION -- IT ONLY DEALS WITH A ROTATION/TRANSLATION/SCALE
//TRANSFORMATION.  GENERALIZED PROJECTIONS AND SUCH CANNOT BE REPRESENTED HERE.  SHOULD PROBABLY 
//UPGRADE THIS DEFINITION TO USE A GENERALIZED 4X4 MATRIX OR DERIVE A CO-CLASS TO WORK WITH THIS
//KEEPING THIS OPTIMIZED CASE AND PROMOTING THIS TYPE TO A GENERALIZED 4X4 ONLY WHEN NECESSARY

 class Xform{
  friend class Xform4x4;  
  q_type rot;
  q_vec_type trans;
  q_vec_type gettrans;	// used for returning the translation vector
						// need this so what we return is not a temporary variable
  double scale;

  double Xoffset;		// These values correspond to the current height-plane's origin
  double Yoffset;
  double Zoffset;	 

  bool transformMatrixNeedsUpdate;
  double transformMatrix[16];

  int lock_trans;		//lock translations
  int lock_rot;			//lock rotations
  int lock_scale;		//lock scale
  friend ostream& operator<< (ostream&,const Xform& x); //by default uses RAW

public:
  // Xform is stored in quat, vector, scalar format and applied as Translate*Rotate*Scale*Point
  // where the trasnlate is the vector, the quat is the rotation, and the scalar is the scale
  // this implies that you cannot have non-uniform scaling in this representation
  Xform();								//identitiy
  Xform(q_vec_type t, q_type r, double s);				//q,v,s
  Xform(q_vec_type t, q_type r, double s, int lt, int lr, int ls);	//q,v,x with locks
  void print(int val=0);				//print the matrix it uses the defines
							//above to pick the format to print in
							//RAW is for the raw q,v,s data
							//4BY4 is for the matrix format
							//XP_STATIC is unused at this time

  //TO OPTIMIZE:  WE SHOULD PROBABLY CACHE THE INVERSE ONCE ITS BEEN CALCULATED
  //AND SWAP THE UNINVERTED AND INVERTED VERSION AN INVERSE FUNCTION CALL
  void invert();					//invert the matrix


  //these cause matrix multiplications to knock out portions of the (A*B) B matrix
  //to prevent the scale, translation, or rotation respectively from being changed
  // should probably upgrade this so so you can lock each rotation or translation
  //sepearately rather than all at once
  void SetRotLock(int i);				//Set the rotation lock
  void SetTransLock(int i);				//Set the translation lock
  void SetScaleLock(int i);				//Set the scale lock

  
  // the offset of the current height plane...add to translations
  void SetXOffset(double x); 
  void SetYOffset(double y);
  void SetZOffset(double z); 


  //the set commands ignore the locks -- only the multiplication currently
  //respects the locks

  void SetRotate(q_type q);				//Set the rotation
  void SetRotate(double rx, double ry, double rz, double rw);	//Set the rotation

  void SetTranslate(q_vec_type t);			//Set the translation
  void SetTranslate(double tx, double ty, double tz);	//Set the translation

  void SetScale(double s);				//set scale

  const q_vec_type & GetTrans(){	q_vec_set(gettrans,	trans[0] - Xoffset, 
														trans[1] - Yoffset,
														trans[2] - Zoffset);
									return gettrans;}		//get translation
  const q_type & GetRot(){return rot;}			//get rotation
  const double & GetScale(){return scale;}		//get scale

  //These functions incrementally add amounts to the current rotation
  //you should be sparing with repeated calls to these functions because
  //numerical error can creep in though the quat form of a rotation is more
  //robust as regards error than the generalized 4x4
  void AddTranslate(q_vec_type t);			//Add a translation to the current one
  void AddRotate(q_type q);				//Add a rotation to the current one

  //TO OPTIMIZE: SHOULD CACHE THE MATRIX RATHER THAN REBUILD IT EVERY TIME
  //THIS IS PROBABLY BAD TO HAVE OPENGL STUFF IN MY XFORM CLASS -- MAYBE MOVE IT ELSEWHERE
  void GetOpenGLMatrix(double *matrix);
  void Push_As_OGL();					//builds the matrix
							//and pushes it as an OpenGL stack

  //don't need compose... operator* will work
  Xform compose(Xform& src);

  void apply(q_vec_type src, q_vec_type result);	//apply matrix to a point
  Xform& operator=(const Xform&);			//copy constructor
  Xform operator*(const Xform&);			//multiply two matrices
  Xform4x4 operator*(const Xform4x4& x);

};

//helper function to test if my stuff was working right
extern void PrintGLStack();

#endif











