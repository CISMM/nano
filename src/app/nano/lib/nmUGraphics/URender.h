#ifndef URENDER_H
#define URENDER_H

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <sys/types.h>	//used for sending signals in critical conditions
#include <signal.h>
#include <GL/gl.h>


#include <assert.h>

// make the SGI compile without tons of warnings
#ifdef sgi
#pragma set woff 1110,1424,3201
#endif

#include <iostream>
#include <fstream>
#include <vector>
using namespace std;

// and reset the warnings
#ifdef sgi
#pragma reset woff 1110,1424,3201
#endif

#include <quat.h>


class UTree;
extern UTree World;
#include "Xform.h"
#include "URenderAux.h"

#if defined(_WIN32) && !defined(__CYGWIN__)
// bogus double to float conversion warning.
#pragma warning(disable:4244)
#pragma warning(disable:4305)
#endif

//RUN TIME TYPE INFORMATION THAT I'M KEEPING FOR EACH OBJECT
enum URender_Type {URENDER, URAXIS, URTEXTURE, URPOLYGON};


typedef struct {
	float red;
	float green;
	float blue;
} RGB;

typedef struct {
	double x1, y1, z1;
	double x2, y2, z2;
	double radius;
	double length;
	double az;
	double alt;
} cylinder;

class URender{
friend class UTree;
protected:
	//no runtime type checking so keep my own
	URender_Type obj_type;

	//state toggles
	int visible;
	int recursion;
	int selected;  
	int show_bounds;	//not used yet
	int wireframe;		//not used yet
	int disp_proj_text;
	int CCW;			// true if load as counter clockwise, false if load as clockwise
	int tess;			// controls the number of faces along the nano-tube
	int axis_step;		// controls the number of nano-tube sections 
	int lock_object;	// controls whether or not to lock the object to the projective texture
	int lock_texture;	// controls whether or not to lock the projective texture to the object
	int update_AFM;		// controls whether position, orientation, and scale are sent to the
						// AFM simulator, when a connection is present
	int grab_object;	// when true, translations and rotations from using the mouse in the
						// surface window are applied to the current object

	// locks for translations and rotations--helps when using the phantom to align objects with
	// projective textures
	int lock_transx;
	int lock_transy;
	int lock_transz;
	int lock_rotx;
	int lock_roty;
	int lock_rotz;

	// fine tune booleans for translations and rotations -- only applied when using the phantom/mouse phantom
	// used for fine-tuning transformations
	int tune_trans;
	int tune_rot;

	// stuff for spiders -- changed to store per leg
	double spider_length[8];
	double spider_width[8];
	double spider_thick[8];
	int spider_tess[8];
	double spider_curve[8];
	int spider_legs;

	//bounding box
	BBOX bounds;	
	int bounds_init;
	//BBOX allbounds;	<---------- ADD THIS
	//
	//right now I only track local bounding box position for 
	//the local geometry -- (this is calculated by URPolygon when 
	// you load geometry, prototype other types that need BBOX from that)
	// it should be augmented with a local bounding box as well as 
	// a combined bounding box that takes into account its children
	// so you can select higher level groups of objects
	// additional thought??? Perhaps have the UTree wrapper contain
	// the union of the bounding information for its contents+children
	// instead of the contents having to know about children which 
	// was the reason for abstracting the tree out??

	//Texture stuff and color
	URender *texture;
	GLfloat c[4];

	char *name;
        
	//estimated fps for timing
	static double fps_est;

	//xform made public for access
	Xform lxform;

	//saved xform for locking the object to the projective texture
	Xform saved_object_xform;

	//saved matrix for locking the projective texture to the object
	qogl_matrix_type saved_texture_xform;
	
public:

    
	//constructors
	URender();
	virtual ~URender();

	//management functions
	void SetVisibility(int s);
	void SetRecursion(int r);
	void SetSelect(int s){ if(s>0) selected=1; else selected=0;}
	void SetProjText(int b) { disp_proj_text = b; }
	void SetCCW(int b) { CCW = b; }
	void SetTess(int t) { tess = t; }
	void SetAxisStep(int s) { axis_step = s; }
	void SetLockObject(int l) { lock_object = l; }
	void SetLockTexture(int l) { lock_texture = l; }
	void SetUpdateAFM(int u) { update_AFM = u; }
	void SetGrabObject(int g) { grab_object = g; }
	void SetLockTransx(int l) { lock_transx = l; }
	void SetLockTransy(int l) { lock_transy = l; }
	void SetLockTransz(int l) { lock_transz = l; }
	void SetLockRotx(int l) { lock_rotx = l; }
	void SetLockRoty(int l) { lock_roty = l; }
	void SetLockRotz(int l) { lock_rotz = l; }
	void SetTuneTrans(int t) { tune_trans = t; }
	void SetTuneRot(int t) { tune_rot = t; }

	void SetSpiderLength(int i, double l) { spider_length[i] = l; }
	void SetSpiderWidth(int i, double w) { spider_width[i] = w; }
	void SetSpiderThick(int i, double t) { spider_thick[i] = t; }
	void SetSpiderTess(int i, int t) { spider_tess[i] = t; }
	void SetSpiderCurve(int i, double c) { spider_curve[i] = c; }
	void SetSpiderLegs(int l) { spider_legs = l; }

	int GetVisibility(){return visible;}
	int GetRecursion(){return recursion;}
	int GetSelect(){ return selected;}
	int ShowProjText() { return disp_proj_text; }
	int GetCCW() { return CCW; }
	int GetTess() { return tess; }
	int GetAxisStep() { return axis_step; }
	int GetLockObject() { return lock_object; }
	int GetLockTexture() { return lock_texture; }
	int GetUpdateAFM() { return update_AFM; }
	int GetGrabObject() { return grab_object; }
	int GetLockTransx() { return lock_transx; }
	int GetLockTransy() { return lock_transy; }
	int GetLockTransz() { return lock_transz; }
	int GetLockRotx() { return lock_rotx; }
	int GetLockRoty() { return lock_roty; }
	int GetLockRotz() { return lock_rotz; }
	int GetTuneTrans() { return tune_trans; }
	int GetTuneRot() { return tune_rot; }

	double GetSpiderLength(int i) { return spider_length[i]; }
	double GetSpiderWidth(int i) { return spider_width[i]; }
	double GetSpiderThick(int i) { return spider_thick[i]; }
	int GetSpiderTess(int i) { return spider_tess[i]; }
	double GetSpiderCurve(int i) { return spider_curve[i]; }
	int GetSpiderLegs() { return spider_legs; }
	void SaveSpider(const char*);

	void SetTexture(URender *t);
	void SetColor(GLfloat nc[4]){c[0]=nc[0];c[1]=nc[1];c[2]=nc[2];c[3]=nc[3];}
    void SetColor3(GLfloat r, GLfloat g, GLfloat b){c[0]=r;c[1]=g;c[2]=b;}
    void SetAlpha(GLfloat a){c[3] = a;}
	
	//picking operations and basic bounding box operations
	void UpdateBoundsWithPoint(double,double,double);
	void UpdateBoundsWithPoint(double c[3]);

	
	//io functions
	friend ostream& operator<< (ostream& co,const URender& r);
        void Print(int level=0);  	//level is the indention level
					//currently only prints type information
					//for the base class

	//info functions
	URender_Type GetType(){return obj_type;}
	Xform &GetLocalXform(){return lxform;}
	Xform &GetSavedObjectXform(){return saved_object_xform;}
	qogl_matrix_type &GetSavedTextureMatrix(){return saved_texture_xform;}
	double maxX(){ return bounds.xmax;}
	double maxY(){ return bounds.ymax;}
	double maxZ(){ return bounds.zmax;}
	double minX(){ return bounds.xmin;}
	double minY(){ return bounds.ymin;}
	double minZ(){ return bounds.zmin;}

	//standard methods
	void DrawBounds();

	//UTree::Do iterative functions

	virtual int Render(void *userdata=NULL);

	// these functions are used to change the properties of all objects to the 
	// appropriate value -- called from tcl callbacks
	virtual int SetVisibilityAll(void *userdata=NULL);
	virtual int SetProjTextAll(void *userdata=NULL);
	virtual int SetLockObjectAll(void *userdata=NULL);
	virtual int SetLockTextureAll(void *userdata=NULL);
	virtual int ScaleAll(void *userdata=NULL);
	virtual int SetTransxAll(void *userdata=NULL);
	virtual int SetTransyAll(void *userdata=NULL);
	virtual int SetTranszAll(void *userdata=NULL);
	virtual int SetRotAll(void *userdata=NULL);
	virtual int SetColorAll(void *userdata=NULL);
	virtual int SetAlphaAll(void *userdata=NULL);

	virtual int ChangeStaticFile(void *userdata=NULL);
	virtual int ChangeHeightPlane(void *userdata=NULL);

	virtual void ReloadGeometry()=0;

	int IntersectLine(void *userdata=NULL);

	//SelectionSet* IntersectPoint(double p[3]);
	//virtual URender* Select(URender& rootnode);


	//standard functions should be implemented for saving the 
	//configuration state -- prototype it like Render
	//	virtual int ReadParam(void *userdata=NULL);
	//	virtual int WriteParam(void *userdata=NULL);


	
	// holds geometry for sending to afm simulator
	float** triangles;
	long num_triangles;

	cylinder* cylinders;
	long num_cylinders;
};

//defines for virtual functions that will be used by the UTREE iterator
//function Do.  These are used to cancel recursion in the generic UTree::Do()
#define ITER_ERROR 	-1
#define ITER_STOP   	0
#define ITER_CONTINUE 	1

#endif



