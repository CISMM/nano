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
	int clamp;			// controls whether or not to clamp projected textures
	int update_AFM;		// controls whether position, orientation, and scale are sent to the
						// AFM simulator, when a connection is present
	int grab_object;	// when true, translations and rotations from using the mouse in the
						// surface window are applied to the current object

	// stuff for spiders
	double spider_length;
	double spider_width;
	double spider_thick;
	int spider_tess;
	double spider_curve;

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

	//saved xform for clamping projective textures
	Xform saved_xform;

	
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
	void SetClamp(int c) { clamp = c; }
	void SetUpdateAFM(int u) { update_AFM = u; }
	void SetGrabObject(int g) { grab_object = g; }

	void SetSpiderLength(double l) { spider_length = l; }
	void SetSpiderWidth(double w) { spider_width = w; }
	void SetSpiderThick(double t) { spider_thick = t; }
	void SetSpiderTess(int t) { spider_tess = t; }
	void SetSpiderCurve(double c) { spider_curve = c; }

	int GetVisibility(){return visible;}
	int GetRecursion(){return recursion;}
	int GetSelect(){ return selected;}
	int ShowProjText() { return disp_proj_text; }
	int GetCCW() { return CCW; }
	int GetTess() { return tess; }
	int GetAxisStep() { return axis_step; }
	int GetClamp() { return clamp; }
	int GetUpdateAFM() { return update_AFM; }
	int GetGrabObject() { return grab_object; }

	double GetSpiderLength() { return spider_length; }
	double GetSpiderWidth() { return spider_width; }
	double GetSpiderThick() { return spider_thick; }
	int GetSpiderTess() { return spider_tess; }
	double GetSpiderCurve() { return spider_curve; }

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
	Xform &GetSavedXform(){return saved_xform;}
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
	virtual int SetClampAll(void *userdata=NULL);
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



