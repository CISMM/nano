//MSIFile.C

#include "URender.h"
#include "MSIFile.h"

#ifdef __CYGWIN__
// XXX juliano 9/19/99
//       this was implicitly declared.  Needed to add decl.
//       getpid comes from unistd.h
#include <sys/types.h>  // for pid_t
extern "C" {
pid_t getpid();
}
#endif

MSISphere::MSISphere(){
  dl = 0; //indicates that no display list was previously created for this sphere
  sphere_depth = 2;
} /*MSISphere::MSISphere*/

void MSISphere::SetSphereDepth(int new_sphere_depth){
/*PURPOSE: Set the tesselation of the sphere*/
  sphere_depth = new_sphere_depth;
}/*MSISphere::SetSphereDepth*/

void MSISphere::Subdivide(float *v1, float *v2, float *v3, int depth){
/*PURPOSE: Subdivides the sphere (makes it look more like a sphere)*/ 
  GLfloat v12[3], v23[3], v31[3];
  int i;
  float d1,d2,d3;

  if (depth==0) 
    {
         glBegin(GL_POLYGON);
         glNormal3fv(v1);
         glVertex3fv(v1);
         glNormal3fv(v2);
         glVertex3fv(v2);
         glNormal3fv(v3);
         glVertex3fv(v3);
      glEnd();
      return;
    }

  for (i=0; i<3; i++)
    {
      v12[i] = v1[i] + v2[i];
      v23[i] = v2[i] + v3[i];
      v31[i] = v3[i] + v1[i];
    }

  d1 = sqrt(v12[0]*v12[0]+v12[1]*v12[1]+v12[2]*v12[2]);
  d2 = sqrt(v23[0]*v23[0]+v23[1]*v23[1]+v23[2]*v23[2]);
  d3 = sqrt(v31[0]*v31[0]+v31[1]*v31[1]+v31[2]*v31[2]);

  if (d1 !=0 && d2 != 0 && d3 != 0)
    {
      for (i=0; i<3; i++)
	{
	  v12[i] /= d1;
	  v23[i] /= d2;
	  v31[i] /= d3;
	}
    }
  else
    {
      printf("division by zero while creating sphere icon\n");
    }

  Subdivide(v1,v12,v31, depth-1);
  Subdivide(v2,v23,v12, depth-1);
  Subdivide(v3,v31,v23, depth-1);
  Subdivide(v12,v23,v31, depth-1);
}/*MSISphere::Subdivide*/

GLuint MSISphere::DisplayList() {
/*PURPOSE: Creates display list*/

  int i;
  //create subdivided sphere display list
  static VertexType vdata[] = {
    {-xx,0,zz},   {xx,0,zz}, {-xx,0,-zz}, {xx,0,-zz},   
    {0,zz,xx},   {0,zz,-xx}, {0,-zz,xx}, {0,-zz,-xx},   
    {zz,xx,0}, {-zz,xx,0},  {zz,-xx,0}, {-zz,-xx,0} };

  static GLint trindex[20][3] = {
    {0,4,1},   {0,9,4},   {9,5,4},  {4,5,8},  {4,8,1},
    {8,10,1}, {8,3,10},   {5,3,8},  {5,2,3},  {2,7,3},
    {7,10,3}, {7,6,10},  {7,11,6}, {11,0,6},  {0,1,6},
    {6,1,10}, {9,0,11}, {9,11,2},  {9,2,5}, {7,2,11} };
  if ((dl != 0) && (glIsList(dl))) { //display list was previously created for this sphere --
               //we want to delete and replace it
    glDeleteLists(dl,1);
    //this does not delete if dl is 0, since it may be that some other display
    //list is using this index.  We had to ititialize dl to something, though,
    //and I chose 0.  This is not very elegant.
  }
  dl=glGenLists(1);
  glNewList(dl,GL_COMPILE);
  for (i=0;i<20;i++){
    //needed to make the vertices get traversed the other way, otherwise we
    //get a backfacing sphere
    Subdivide(vdata[trindex[i][2]],vdata[trindex[i][1]],vdata[trindex[i][0]],sphere_depth);
  }
  glEndList();
  return(dl);
}/*MSISphere::DisplayLists*/

MSISphere::~MSISphere() { //destructor
  if (glIsList(dl))
    glDeleteLists(dl,1);
}

//Default constructor
MSIFile::MSIFile(){
  filename = NULL;
  Pobject = NULL;
  atom_ptr = NULL;
  dl = 0;

  bond_width = 1.0;
  sphere_radius = 1.0;
  //bonds start out blue
  bond_colorR = 0.0;
  bond_colorG = 0.0;
  bond_colorB = 1.0;
  //spheres start out red
  sphere_colorR = 0.0;
  sphere_colorG = 0.0;
  sphere_colorB = 1.0;
  import_mode = 0; //assume we start out in bond mode
  visibility_mode = 1; //assume we start out in show mode
} /*MSIFile::MSIFile*/

MSIFile::MSIFile(URender* pobj,char* fname){
  /*PURPOSE: Constructs an instance of MSIFile*/
  filename = fname;
  Pobject = pobj;
  atom_ptr = NULL;
  dl = 0; //no display lists created yet

  bond_width = 1.0;
  sphere_radius = 1.0;
  //bonds start out blue
  bond_colorR = 0.0;
  bond_colorG = 0.0;
  bond_colorB = 1.0;
  //spheres start out red
  sphere_colorR = 0.0;
  sphere_colorG = 0.0;
  sphere_colorB = 1.0;
  import_mode = 0; //assume we start out in bond mode
  visibility_mode = 1; //assume we start out in show mode
}/*MSIFile::MSIFile*/

void MSIFile::BuildListMSI(GLuint dl){
/*PURPOSE: Build the MSI file's display list*/
/**Note: We should never be here in Hide mode**/
  int i, j, atom_i;
  if (import_mode == 0) { //if we are in bond mode
    glNewList(dl,GL_COMPILE); //init display list
    glColor3f(bond_colorR,bond_colorG,bond_colorB);
    glLineWidth(bond_width);
    for(i=0;i<bond_count;i++){
      glBegin(GL_LINES);
      atom_i = bond1[i]-2;
      glVertex3d(atomx[atom_i],atomy[atom_i],atomz[atom_i]);
      atom_i = bond2[i]-2;
      glVertex3d(atomx[atom_i],atomy[atom_i],atomz[atom_i]);
      glEnd();
    }
    glEndList();
  }
  else if (atom_count >= 1) {  //we are in sphere mode
    if (atom_ptr != NULL)
      delete atom_ptr; //if there was already a sphere display list, delete it
    atom_ptr = new MSISphere;
    GLuint atom_dl;
    atom_dl = atom_ptr->DisplayList(); //make a new sphere display list
    glNewList(dl,GL_COMPILE);
    glColor3f(sphere_colorR,sphere_colorG,sphere_colorB);
    for (j=0;j<atom_count;j++){
      glPushMatrix();
      glPushAttrib(GL_CURRENT_BIT);
      glTranslatef(atomx[j],atomy[j],atomz[j]);
      glScalef(sphere_radius,sphere_radius,sphere_radius);
      glCallList(atom_dl); //Call the sphere's display list
      glPopAttrib();
      glPopMatrix();
    }
   glEndList();
  }
} /*MSIFile::BuildListMSI*/

void MSIFile::SetImportMode(int new_import_mode){
  import_mode = new_import_mode;
}/*MSIFile::SetImportMode*/

void MSIFile::SetVisibilityMode(int new_visibility_mode){
  visibility_mode = new_visibility_mode;
}/*MSIFile::SetVisibility*/

void MSIFile::SetBondWidth(float new_bond_width){
  bond_width = new_bond_width;
}/*MSIFile::SetBondWidth*/

void MSIFile::SetBondColorR(float new_color){
  bond_colorR = new_color;
}/*MSIFile::SetBondColorR*/

void MSIFile::SetBondColorG(float new_color){
  bond_colorG = new_color;
}/*MSIFile::SetBondColorG*/

void MSIFile::SetBondColorB(float new_color){
  bond_colorB = new_color;
}/*MSIFile::SetBondColorB*/

void MSIFile::SetSphereRadius(float new_sphere_radius){
  sphere_radius = new_sphere_radius;
}/*MSIFile::SetSphereRadius*/

void MSIFile::SetSphereDepth(int new_sphere_depth){
  sphere_depth = new_sphere_depth;
}/*MSIFile::SetSphereDepth*/

void MSIFile::SetSphereColorR(float new_color){
  sphere_colorR = new_color;
}/*MSIFile::SetSphereColorR*/

void MSIFile::SetSphereColorG(float new_color){
  sphere_colorG = new_color;
}/*MSIFile::SetSphereColorG*/

void MSIFile::SetSphereColorB(float new_color){
  sphere_colorB = new_color;
}/*MSIFile::SetSphereColorB*/

int MSIFile::LoadMSIFile(GLuint *&Dlist_array){
/*PURPOSE: Loads the geometry contained in a .msi file*/

  ifstream readfile;
  char buf[500];
  int cnt, i, j, k, l;
  int more_objects, more_models; //bool variables -- 0=false,1=true
  char obj_type,mod_type;

  atom_count = 0;
  bond_count = 0;
  model_count = 0;

  //open data file
  readfile.open(filename);
  //assert(readfile);

  if (readfile.bad()){
    cerr<<"Unable to open input file "<<buf<<"\n";
    return 0;
  }

  //PHASE 1: Scan file to get model, atom and bond counts
  if (!readfile.eof()){
    //skip first line
    readfile.getline(buf,500);
    //read in first model
    if (!readfile.eof()){
       readfile.getline(buf,500);
      sscanf(buf,"(%d %c",&cnt,&mod_type);
      if (mod_type == 'M')
        more_models = 1;
      else{
        more_models = 0;
        cerr<<"Unexpected token -- "<<buf<<"\n";
      }
    }
    else{
      cerr<<"Unexpected eof -- "<<filename<<"\n";
      more_models = 0;
    }
  }
  else{
    more_models = 0;
    cerr<<"Unexpected eof -- "<<filename<<"\n";
  }
  while (more_models){
    model_count++;
    //skip first ID line
    readfile.getline(buf,500);
    if (!readfile.eof()){
      //read in first object
      readfile.getline(buf,500);
      sscanf(buf," (%d %c", &cnt, &obj_type);
      more_objects = 1;
    }
    else
      more_objects = 0;
    while (more_objects){
      //count atoms
      if (obj_type == 'A'){
        atom_count++;
        //skip 4 lines
        readfile.getline(buf,500);
        readfile.getline(buf,500); 
        readfile.getline(buf,500);
        readfile.getline(buf,500);
      }
      else if (obj_type == 'B'){
        bond_count++; 
        //skip 3 lines
        readfile.getline(buf,500);
        readfile.getline(buf,500);
        readfile.getline(buf,500); 
      }
      else{
        cerr<<"Unexpected token -- "<<buf<<"\n";
        more_objects = 0;
      }
      //skip closing parentheses for object
      readfile.getline(buf,500);
      readfile.getline(buf,500);
      //if more objects for model, then read in next line
      if (buf[0]!=')'){
        sscanf(buf," (%d %c", &cnt, &obj_type);
      }
      //no more objects for model
      else{
        more_objects = 0;
        if (!readfile.eof()){
          //more models, so read in next line
          sscanf(buf,"(%d %c",&cnt,&mod_type);
          if (mod_type != 'M'){
            more_models = 0;
            cerr<<"Unexpected token -- "<<buf<<"\n";
          }
        }
        else
	  //we have reached eof, so no more models
          more_models = 0;
      }
    }  
  }
  //reset to beginning of file
  readfile.seekg(0,ios::beg);
  readfile.clear();

  //allocate data structures
  atomx = new float[atom_count];
  atomy = new float[atom_count];
  atomz = new float[atom_count];
  bond1 = new int[bond_count];  
  bond2 = new int[bond_count];
  if(!(atomx && atomy && atomz && bond1 && bond2)){
    cerr << "Unable to allocate sufficient memory store for MSI file\n";
    //kill(getpid(),SIGINT);
      	return 0;
  }
  //skip first line 
  readfile.getline(buf,500);   
  //PHASE 2: Store the atom and bond information
  for (i=0;i<model_count;i++){
    //skip model and ID line
    readfile.getline(buf,500);
    readfile.getline(buf,500);
    //store atoms
    for (j=0;j<atom_count;j++){
      //skip first 2 lines
      readfile.getline(buf,500);
      readfile.getline(buf,500);
      //store atom
      readfile.getline(buf,500);
      sscanf(buf,"  (A D XYZ (%f %f %f", atomx+j,atomy+j,atomz+j);
      //skip next 3 lines
      readfile.getline(buf,500);
      readfile.getline(buf,500); 
      readfile.getline(buf,500);
    }
    //store bonds
    for (k=0;k<bond_count;k++){
      //skip first 2 lines
      readfile.getline(buf,500);
      readfile.getline(buf,500);
      //store bond
      readfile.getline(buf,500);
      sscanf(buf,"  (A O Atom1 %d", bond1+k);
      readfile.getline(buf,500);
      sscanf(buf,"  (A O Atom2 %d", bond2+k);
      //skip next line
      readfile.getline(buf,500);
    }
  } 
  //reset file ptr
  readfile.seekg(0,ios::beg);
  readfile.clear();
  Dlist_array=new GLuint[model_count];
  dl=glGenLists(model_count);
  if(dl==0 || Dlist_array==NULL){ cerr << "Bad Display List generation\n"; 
     //kill(getpid(),SIGINT); 
     return 0;
  }  
  for(l=0; l<model_count; l++){
    //BuildListMSI  actually builds the geometry from
    //the data structures previously built
    BuildListMSI(dl+l);
    Dlist_array[l]=dl+l;
  }
  /**Unlike in LoadWavefrontFile, we DO NOT clean up after ourselves and 
    delete the arrays, since we may have to use the reload function later
    and it will be nice to have the data already stored.  THEREFORE we
    have to call a delete function when an imported_obj is deleted.
    (LoadMSIFile is called whenever an imported_obj is created)**/
  readfile.close();
  return (model_count);
} /*MSIFile::LoadMSIFile*/

int MSIFile::ReloadMSIFile(GLuint *&Dlist_array){
/*PURPOSE: Loads the geometry contained in a .msi file*/

  int l;
  if ((glIsList(dl)) && (dl != 0)) //lists were previously created for this MSIFile--we want to
                    //deleted them
    glDeleteLists(dl,model_count);
  //we do not delete dl in the case that dl=0, since some other display list may be using this index.  We had to initialize dl to something, though, and I chose 0
  if (visibility_mode == 1) //we are in Show mode
  {
    Dlist_array=new GLuint[model_count];
    dl=glGenLists(model_count);
    if(dl==0 || Dlist_array==NULL){ cerr << "Bad Display List generation\n"; 
       //kill(getpid(),SIGINT);
       return 0;
    }  
    for(l=0; l<model_count; l++){
      //BuildListMSI  actually builds the geometry from
      //the data structures previously built
      BuildListMSI(dl+l);
      Dlist_array[l]=dl+l;
    }
  }
  /**Unlike in LoadWavefrontFile, we DO NOT clean up after ourselves and 
     delete the arrays, since we may have to use the reload function later
     and it will be nice to have the data already stored.  THEREFORE we
     have to call a delete function when an imported_obj is deleted.
     (LoadMSIFile, which creates the arrays, is called whenever an imported_obj is created)**/
  return (model_count);
} /*MSIFile::ReloadMSIFile*/

MSIFile::~MSIFile(){
  //clean up after myself
  delete atomx; delete atomy; delete atomz;
  delete bond1; delete bond2;
  if (glIsList(dl))
    glDeleteLists(dl,model_count);
}/*MSIFile::~MSIFile*/
