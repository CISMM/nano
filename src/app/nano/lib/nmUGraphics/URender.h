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
#include "URProjectiveTexture.h"

#if defined(_WIN32) && !defined(__CYGWIN__)
// bogus double to float conversion warning.
#pragma warning(disable:4244)
#pragma warning(disable:4305)
#endif

//RUN TIME TYPE INFORMATION THAT I'M KEEPING FOR EACH OBJECT
enum URender_Type {URENDER, URAXIS, URPOLYGON, URSPIDER, URTUBEFILE, URWAVEFRONT, URHEIGHTFIELD, NMTIP};


typedef struct {
	float red;
	float green;
	float blue;
} RGB;

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
	bool textureInWorldCoordinates;// true==> textureTransform gives worldToTexture
	                               // false==> textureTransform gives objectToTexture
	bool lockTextureTransform;	// true/false==> textureTransform is/isn't set when a new transform
	                            //          is broadcast using SetTextureTransformAll
	int grab_object;	// when true, translations and rotations from using the mouse in the
						// surface window are applied to the current object

	bool lockLocalTransform;
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
	URProjectiveTexture *texture;
	double textureTransform[16];
	GLfloat c[4];

	char *name;
        
	//estimated fps for timing
	static double fps_est;

	//xform made public for access
	Xform lxform;
	
public:

    
	//constructors
	URender();
	virtual ~URender();

	//management functions
	void SetVisibility(int s);
	void SetRecursion(int r);
	void SetSelect(int s){ if(s>0) selected=1; else selected=0;}
	void SetProjTextEnable(int b) { disp_proj_text = b; }
	void SetLockObject(bool l) { lockLocalTransform = l; }
	void SetLockTexture(int l) { lockTextureTransform = (l!=0); }
	void SetGrabObject(int g) { grab_object = g; }
	void SetLockTransx(int l) { lock_transx = l; }
	void SetLockTransy(int l) { lock_transy = l; }
	void SetLockTransz(int l) { lock_transz = l; }
	void SetLockRotx(int l) { lock_rotx = l; }
	void SetLockRoty(int l) { lock_roty = l; }
	void SetLockRotz(int l) { lock_rotz = l; }
	void SetTuneTrans(int t) { tune_trans = t; }
	void SetTuneRot(int t) { tune_rot = t; }
	void SetProjTexture(URProjectiveTexture *t) {texture = t;}
	void SetTextureTransform(double *xform)
	{if (xform) {int i; for (i = 0; i < 16; i++) textureTransform[i] = xform[i];}}
	void SetTextureCoordinatesInWorld(bool textureInWorld) 
	{textureInWorldCoordinates = textureInWorld;}

	int GetVisibility(){return visible;}
	int GetRecursion(){return recursion;}
	int GetSelect(){ return selected;}
	int GetProjTextEnable() { return disp_proj_text; }
	bool GetLockObject() { return lockLocalTransform; }
	int GetLockTexture() { return lockTextureTransform; }
	int GetGrabObject() { return grab_object; }
	int GetLockTransx() { return lock_transx; }
	int GetLockTransy() { return lock_transy; }
	int GetLockTransz() { return lock_transz; }
	int GetLockRotx() { return lock_rotx; }
	int GetLockRoty() { return lock_roty; }
	int GetLockRotz() { return lock_rotz; }
	int GetTuneTrans() { return tune_trans; }
	int GetTuneRot() { return tune_rot; }
	URProjectiveTexture *GetProjTexture() {return texture;}
	void GetTextureTransform(double *xform)
	{if (xform) {int i; for (i = 0; i < 16; i++) xform[i] = textureTransform[i];}}
	bool GetTextureCoordinatesInWorld() {return textureInWorldCoordinates;}

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
	virtual int SetProjTextEnableAll(void *userdata=NULL);
	virtual int SetLockObjectAll(void *userdata=NULL);
	virtual int SetLockTextureAll(void *userdata=NULL);
	virtual int ScaleAll(void *userdata=NULL);
	virtual int SetTransxAll(void *userdata=NULL);
	virtual int SetTransyAll(void *userdata=NULL);
	virtual int SetTranszAll(void *userdata=NULL);
	virtual int SetRotAll(void *userdata=NULL);
	virtual int SetColorAll(void *userdata=NULL);
	virtual int SetAlphaAll(void *userdata=NULL);
	virtual int SetProjTextureAll(void *userdata=NULL);
	virtual int SetTextureTransformAll(void *userdata=NULL);

	virtual int ChangeStaticFile(void *userdata=NULL);
	virtual int ChangeHeightPlane(void *userdata=NULL);
	virtual int ChangeDataset(void *userdata=NULL);

	virtual void ReloadGeometry();

	int IntersectLine(void *userdata=NULL);

	//SelectionSet* IntersectPoint(double p[3]);
	//virtual URender* Select(URender& rootnode);


	//standard functions should be implemented for saving the 
	//configuration state -- prototype it like Render
	//	virtual int ReadParam(void *userdata=NULL);
	//	virtual int WriteParam(void *userdata=NULL);
};

//defines for virtual functions that will be used by the UTREE iterator
//function Do.  These are used to cancel recursion in the generic UTree::Do()
#define ITER_ERROR 	-1
#define ITER_STOP   	0
#define ITER_CONTINUE 	1

#endif



