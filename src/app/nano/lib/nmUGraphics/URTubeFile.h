#ifndef URTUBEFILE_H
#define URTUBEFILE_H

#include "URPolygon.h"

typedef struct {
	double x1, y1, z1;
	double x2, y2, z2;
	double radius;
	double length;
	double az;
	double alt;
} cylinder;

class URTubeFile : public URPolygon {
private:
	int tess;			// controls the number of faces along the nano-tube
	int axis_step;		// controls the number of nano-tube sections 

	int update_AFM;		// controls whether or not to update the AFM when manipulating
						// the tube in nano
public:
	// constructor destructor
	URTubeFile();
	~URTubeFile();

	// cylinders for sending to afm simulator
	cylinder* cylinders;
	long num_cylinders;

	// management functions
	void SetTess(int t) { tess = t; }
	void SetAxisStep(int s) { axis_step = s; }
	void SetUpdateAFM(int u) { update_AFM = u; }

	int GetTess() { return tess; }
	int GetAxisStep() { return axis_step; }
	int GetUpdateAFM() { return update_AFM; }

	// overload the URPolygon versions, as we need to update the cylinders
	int ChangeStaticFile(void* userdata=NULL);	
	int ScaleAll(void* userdate=NULL);			
	int SetTransxAll(void *userdata=NULL);
	int SetTransyAll(void *userdata=NULL);
	int SetTranszAll(void *userdata=NULL);
	int SetRotAll(void *userdata=NULL);
};

#endif