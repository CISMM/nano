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
    
public:
    //constructor destructor
    URPolygon();
    ~URPolygon();
    
    //standard functions
    int Render(void *userdata=NULL);

	int SetVisibilityAll(void *userdata=NULL);
	int SetProjTextAll(void *userdata=NULL);
	int ScaleAll(void *userdata=NULL);
	int SetTransxAll(void *userdata=NULL);
	int SetTransyAll(void *userdata=NULL);
	int SetTranszAll(void *userdata=NULL);
	int SetRotxAll(void *userdata=NULL);
	int SetRotyAll(void *userdata=NULL);
	int SetRotzAll(void *userdata=NULL);
	int SetColorAll(void *userdata=NULL);
	int SetAlphaAll(void *userdata=NULL);

	int ChangeStaticFile(void *userdata=NULL);

    GeometryGenerator* GetGenerator();
    
    //Geometry functions
    void LoadGeometry(GeometryGenerator *gen);	    
    void ReloadGeometry();
};


#endif
