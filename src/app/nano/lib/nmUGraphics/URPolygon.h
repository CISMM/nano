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
    
    //PARSERS
    void LoadGeometry(GeometryGenerator *gen);	//generic function to scan file names and call
    //the appropriate parser
    
    //Added by Leila Plummer for importing from tube_foundry
    void ReloadGeometry();
};


#endif
