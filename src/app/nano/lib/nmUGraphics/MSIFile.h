//#include "URender.h"

/*For MSISphere*/
typedef GLfloat VertexType[3];
#define xx .525731112119133606
#define zz .850650808352039932

class MSIFile; //forward declaration

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
    friend class MSIFile;
};

//This class handles all loading of a geometry contained in a .msi file into a display list, which is stored in an uberGraphics tree
class MSIFile{
  //private:
    char *filename; //MSI filename
    URender *Pobject;
    GLuint dl; //index of display list
    MSISphere* atom_ptr; //pointer to the atom we want to use in our display list

    float *atomx; //an array of atom x-coordinates
    float *atomy; //an array of atom y-coordinates
    float *atomz; //an array of atom z-coordinates
    int *bond1;   //an array of the atoms from which bonds are drawn
    int *bond2;   //an array of the atoms to which bonds are drawn
    int bond_count,model_count,atom_count;
    float bond_width,sphere_radius,sphere_depth;
    float bond_colorR, bond_colorG, bond_colorB, sphere_colorR,sphere_colorG,sphere_colorB;
    int import_mode; //import_mode is 0 for bond mode, 1 for sphere mode
    int visibility_mode; //visibility_mode is 0 for hide mode, 1 for show mode
  public:
    MSIFile();
    MSIFile(URender*,char*);
    void BuildListMSI(GLuint);
    int LoadMSIFile(GLuint *&Dlist_array);
    int ReloadMSIFile(GLuint *&Dlist_array);
    void SetImportMode(int);
    void SetVisibilityMode(int);
    void SetBondWidth(float);
    void SetBondColorR(float);
    void SetBondColorG(float);
    void SetBondColorB(float);
    void SetSphereRadius(float);
    void SetSphereDepth(int);
    void SetSphereColorR(float);
    void SetSphereColorG(float);
    void SetSphereColorB(float);
    ~MSIFile();
};
