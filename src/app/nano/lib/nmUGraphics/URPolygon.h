#ifndef URPOLYGON_H
#define URPOLYGON_H

#include <iostream.h>
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
    GeometryGenerator* GetGenerator();
    
    //Geometry functions
    void LoadGeometry(GeometryGenerator *gen);	    
    void ReloadGeometry();
};


#endif
