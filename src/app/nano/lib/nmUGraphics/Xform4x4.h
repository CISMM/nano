#ifndef XFORM4x4_H
#define XFORM4x4_H

//these are utility functions for a generalized 4x4 transformation which should probably be
//treated better than it is here... running out of time for my coding.  I needed a x4 minimal
//class in order to do the mouse based picking operation

#include <quat.h>
#include <iostream>
using namespace std;
#include "Xform.h"

class Xform;
class Xform4x4{
protected:
	double m[4][4];
public:
	Xform4x4();
	Xform4x4(double d[4][4]);
	~Xform4x4();

	Xform4x4  operator*(const Xform4x4&);
	Xform4x4  operator*(const Xform&);		//should implement

	Xform4x4& operator=(const Xform4x4&);     //copy constructors
	//Xform4x4& operator=(const Xform&); 

	void invert();
	void transpose();
	void apply(q_vec_type src, q_vec_type result);

	double operator()(int i, int j);		//get operator
	void operator()(int i, int j, double val);	//set operator

	//add this one later for completeness should probably
	//keep this one around as the 'cached' type in Xform class
	//and set a bit there if the Xform class has created a matrix
	//that can no longer be represented by qvs format and then just
	//call these functions instead of the standard qvs?? I'm sure there
	//is a good ObjOriented method for doing this -- LATER
	//Xform operator*(const Xform&);         //multiply two matrices

	friend ostream& operator<< (ostream&,const Xform4x4& x); //by default uses RAW

};

#endif

