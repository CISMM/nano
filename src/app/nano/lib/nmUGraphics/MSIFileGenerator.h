#ifndef MSIFILE_GENERATOR_H
#define MSIFILE_GENERATOR_H

#include "FileGenerator.h"

/*For MSISphere*/
typedef GLfloat VertexType[3];
#define xx .525731112119133606
#define zz .850650808352039932

class MSIFileGenerator; //forward declaration

//MSISphere is basically mysphere of graphics/globjects.c, but modified
//and in class form.  It is used to draw atoms.
class MSISphere{
  //private:
    GLuint dl;
    int sphere_depth;
  public:
    MSISphere();
    void SetSphereDepth(int);
    void Subdivide(float*,float*,float*,int);
    GLuint DisplayList();
    ~MSISphere();
    friend class MSIFileGenerator;
};

//This class handles all loading of a geometry contained in a .msi file into a display list, which is stored in an uberGraphics tree
class MSIFileGenerator : public FileGenerator {
  private:
    GLuint dl; //index of display list
    MSISphere* atom_ptr; //pointer to the atom we want to use in our display list

    float *atomx; //an array of atom x-coordinates
    float *atomy; //an array of atom y-coordinates
    float *atomz; //an array of atom z-coordinates
    int *bond1;   //an array of the atoms from which bonds are drawn
    int *bond2;   //an array of the atoms to which bonds are drawn
    int bond_count,model_count,atom_count;
    float bond_width,sphere_radius,sphere_depth;
    int import_mode; //import_mode is 0 for bond mode, 1 for sphere mode
    int visibility_mode; //visibility_mode is 0 for hide mode, 1 for show mode
  public:
    MSIFileGenerator(const char* fname = NULL);
    
    virtual int Load(URender *node, GLuint *&Dlist_array);
    virtual int ReLoad(URender *node, GLuint *&Dlist_array);

    void BuildListMSI(GLuint);
    void SetImportMode(int);
    void SetVisibilityMode(int);
    void SetBondWidth(float);
    void SetSphereRadius(float);
    void SetSphereDepth(int);
    ~MSIFileGenerator();
};

#endif

