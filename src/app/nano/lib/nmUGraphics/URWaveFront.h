#ifndef URWAVEFRONT_H
#define URWAVEFRONT_H

#include "URPolygon.h"

class URWaveFront : public URPolygon {
private:
	int CCW;	// true if load as counter clockwise, false if load as clockwise
				// used for loading .obj files

public:
	// constructor destructor
	URWaveFront();
	~URWaveFront();

	// management functions
	void SetCCW(int b) { CCW = b; }
	int GetCCW() { return CCW; }
};

#endif