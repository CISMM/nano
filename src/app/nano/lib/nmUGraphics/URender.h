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
#include <iostream.h>
#include <fstream.h>
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
	
public:

    
	//constructors
	URender();
	virtual ~URender();

	//management functions
	void SetVisibility(int s);
	void SetRecursion(int r);
	void SetSelect(int s){ if(s>0) selected=1; else selected=0;}

	int GetVisibility(){return visible;}
	int GetRecursion(){return recursion;}
	int GetSelect(){ return selected;}

	void SetTexture(URender *t);
	void SetColor(GLfloat nc[4]){c[0]=nc[0];c[1]=nc[1];c[2]=nc[2];c[3]=nc[3];}
	void SetAlpha(GLfloat nc){c[3]=nc;}

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



