#ifndef URPOLYGON_H
#define URPOLYGON_H

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

#include "URender.h"
#include "GeomGenerator.h"

#define DLIST_CHUNK	5

class URPolygon:public URender{
private:
    GLuint *Dlist_array;
    int num_objects;
    
    GeometryGenerator *d_generator;
	Xform localXformForLockedObject;
    
public:
    //constructor destructor
    URPolygon();
    ~URPolygon();

    //standard functions
    int Render(void *userdata=NULL);

    GeometryGenerator* GetGenerator();
    
    //Geometry functions
    void LoadGeometry(GeometryGenerator *gen);	    
    void ReloadGeometry();

};


#endif
