#include <stdlib.h>		//stdlib.h vs cstdlib
#include <stdio.h>		//stdio.h vs cstdio
#include <iostream.h>
#include "3Dobject.h"

// default constructor for OB
OB::OB( void )
  : type(TUBE), 
  pos(Vec3d(0.,0.,0.)), yaw(0.), roll(0.), pitch(0.), leng(1.), diam(1.), 
  nextSeg(NULLOB), prevSeg(NULLOB), moved(0)
{}


// constructor for OB
OB::OB( Vec3d pos, double yaw, double roll, double pitch, double length, double diameter )
  : type(TUBE), 
  pos(pos), yaw(yaw), roll(roll), pitch(pitch), leng(length), diam(diameter), 
  nextSeg(NULLOB), prevSeg(NULLOB), moved(0)
{}

void OB::set(int _type, Vec3d _pos, double _yaw, double _roll, double _pitch, double _length, double _diameter, int _nextSeg, int _prevSeg ) {
  type = _type;
  pos = _pos;
  yaw = _yaw;
  roll = _roll;
  pitch = _pitch;
  leng = _length;
  diam = _diameter;
  nextSeg = _nextSeg;
  prevSeg = _prevSeg;
}

void OB::print() {
  cout << "pos x " << pos.x << " y " << pos.y << " z " << pos.z << " yaw " << yaw << " pitch " << pitch << " roll " << roll << " length " << leng << " diam " << diam << endl;
}
