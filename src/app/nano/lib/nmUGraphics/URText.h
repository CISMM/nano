#ifndef URTEXT_H
#define URTEXT_H

// make the SGI compile without tons of warnings
#ifdef sgi
#pragma set woff 1110,1424,3201
#endif

#include <string>
#include <iostream>
#include "font.h"
using namespace std;

// and reset the warnings
#ifdef sgi
#pragma reset woff 1110,1424,3201
#endif

#include "URender.h"
#include "GeomGenerator.h"

#define DLIST_CHUNK	5

class URText:public URender {
private:
    GLuint *Dlist_array;
    int num_objects;
    
    GeometryGenerator *d_generator;
    Xform localXformForLockedObject;
    
    int	    d_font;
    string  d_text_string;

public:
    //constructor destructor
    URText();
    ~URText();

    //standard functions
    int Render(void *userdata=NULL);
    GeometryGenerator* GetGenerator();

    //Text-specific functions
    void SetText(const string &s) { d_text_string = s; }
    const string &GetText(void) const { return d_text_string; }
};


#endif
