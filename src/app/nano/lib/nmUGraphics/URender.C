#include "UTree.h"
#include "URender.h"
#include <string.h>
#include <stdlib.h>

#include <Tcl_Linkvar.h>
#include <Tcl_Netvar.h>

#ifdef __CYGWIN__
// XXX juliano 9/19/99
//       this was implicitly declared.  Needed to add decl.
//       getpid comes from unistd.h
#include <sys/types.h>  // for pid_t
extern "C" {
pid_t getpid();
}
#endif

void SelectionSet::AddSelection(URender *r){
	if(numselected>MAXSELECT){ 
		cerr << "Selection capacity exceeded... modify MAXSELECT and recompile\n";
		return;
	}
	sset[numselected]=r;
	numselected++;
}

double URender::fps_est=20;	//sets expected fps rate for timing

void URender::SetVisibility(int s){
	if(s>0) visible=1;
	else visible=0;
}

void URender::SetRecursion(int r){
	if(r>0) recursion=1;
	else recursion=0;
}

URender::URender(){

	visible=1; selected=0; recursion=1;
	bounds.xmin=bounds.ymin=bounds.zmin=0;
	bounds.xmax=bounds.ymax=bounds.zmax=0;
	bounds_init=0;
	obj_type=URENDER;
	texture=NULL;
	name=NULL;
}

void URender::UpdateBoundsWithPoint(double vx, double vy, double vz)
{
  if(bounds_init==0){
	bounds.xmin=bounds.xmax=vx;
	bounds.ymin=bounds.ymax=vy;
	bounds.zmin=bounds.zmax=vz;
	bounds_init=1;

  }
  else{
	if(vx < bounds.xmin) bounds.xmin=vx;
	else if(vx > bounds.xmax) bounds.xmax=vx;
	if(vy < bounds.ymin) bounds.ymin=vy;
	if(vy > bounds.ymax) bounds.ymax=vy;	
	if(vz < bounds.zmin) bounds.zmin=vz;
	if(vz > bounds.zmax) bounds.zmax=vz;
  }
  return;
}

  
URender::~URender(){
	if(name) delete []name; 
	name=NULL;
}

int URender::SetVisibilityAll(void* userdata) {
	int setvisibility = *(int*) userdata;

	this->SetVisibility(setvisibility);

	if(recursion) return ITER_CONTINUE;
	else return ITER_STOP;
}

int URender::SetProjTextEnableAll(void* userdata) {
	int enableProjTexture = *(int*) userdata;

	this->SetProjTextEnable(enableProjTexture);

	if(recursion) return ITER_CONTINUE;
	else return ITER_STOP;
}

int URender::SetLockObjectAll(void* userdata) {
	bool setlockobject = (*(int*) userdata) != 0;

	this->SetLockObject(setlockobject);

	if(recursion) return ITER_CONTINUE;
	else return ITER_STOP;
}

int URender::SetLockTextureAll(void* userdata) {
	int setlocktexture = *(int*) userdata;

	this->SetLockTexture(setlocktexture);

	if(recursion) return ITER_CONTINUE;
	else return ITER_STOP;
}

int URender::ChangeStaticFile(void* userdata) {
    // modifies the scale and translation so appears in same place...
	extern Tclvar_float	import_scale;
	extern Tclvar_float import_transx;
	extern Tclvar_float import_transy;
	extern Tclvar_float import_transz;
	extern Tclvar_string current_object;

	change_static_file csf = *(change_static_file*) userdata;

	this->GetLocalXform().SetXOffset(csf.xoffset);
	this->GetLocalXform().SetYOffset(csf.yoffset);
	this->GetLocalXform().SetZOffset(csf.zoffset);

	this->GetLocalXform().SetScale(this->GetLocalXform().GetScale() * csf.scale);

	// if current object, update the tcl stuff
	if (strcmp(this->name, current_object.string()) == 0) {
		import_scale = this->GetLocalXform().GetScale();
	}


	const q_vec_type &q1 = this->GetLocalXform().GetTrans();

	q_vec_type q2, q3;

	q_vec_copy(q2, q1);
	q_vec_copy(q3, q1);

	q_vec_scale(q2, csf.scale, q2);

	this->GetLocalXform().SetTranslate(q2);

	// if current object, update the tcl stuff
	if (strcmp(this->name, current_object.string()) == 0) {
		import_transx = q2[0];
		import_transy = q2[1];
		import_transz = q2[2];
	}

	if(recursion) return ITER_CONTINUE;
	else return ITER_STOP;
}

int URender::ChangeHeightPlane(void* userdata) {
	// modifies the scale and translation so appears in same place...

	double z = *(double*) userdata;

	const q_vec_type &q1 = this->GetLocalXform().GetTrans();
	q_vec_type q2;

	q_vec_copy(q2, q1);

	this->GetLocalXform().SetZOffset(z);

	this->GetLocalXform().SetTranslate(q2);

	if(recursion) return ITER_CONTINUE;
	else return ITER_STOP;
}


int URender::ScaleAll(void* userdata) {
	double scale = *(double*) userdata;

	this->GetLocalXform().SetScale(scale);

	if(recursion) return ITER_CONTINUE;
	else return ITER_STOP;
}


int URender::SetTransxAll(void* userdata) {	
	double transx = *(double*) userdata;
	const q_vec_type &trans = this->GetLocalXform().GetTrans();
	this->GetLocalXform().SetTranslate(transx, trans[1], trans[2]);

	if(recursion) return ITER_CONTINUE;	
	else return ITER_STOP;
}

int URender::SetTransyAll(void* userdata) {	
	double transy = *(double*) userdata;
	const q_vec_type &trans = this->GetLocalXform().GetTrans();
	this->GetLocalXform().SetTranslate(trans[0], transy, trans[2]);

	if(recursion) return ITER_CONTINUE;	
	else return ITER_STOP;
}

int URender::SetTranszAll(void* userdata) {	
	double transz = *(double*) userdata;
	const q_vec_type &trans = this->GetLocalXform().GetTrans();
	this->GetLocalXform().SetTranslate(trans[0], trans[1], transz);

	if(recursion) return ITER_CONTINUE;	
	else return ITER_STOP;
}

int URender::SetRotAll(void* userdata) {
	double* array = (double*)userdata;
	double rotx = array[0];
	double roty = array[1];
	double rotz = array[2];

	q_vec_type euler;

	euler[2] = rotx;
	euler[1] = roty;
	euler[0] = rotz;

	q_type rot;
	q_from_euler(rot, euler[0], euler[1], euler[2]);

    this->GetLocalXform().SetRotate(rot[0], rot[1], rot[2], rot[3]);

	if(recursion) return ITER_CONTINUE;	
	else return ITER_STOP;
}


int URender::SetColorAll(void* userdata) {
	RGB* color = (RGB*) userdata;

	this->SetColor3(color->red, color->green, color->blue);

	if(recursion) return ITER_CONTINUE;
	else return ITER_STOP;
}

int URender::SetAlphaAll(void* userdata) {
	float alpha = *(double*) userdata;

	this->SetAlpha(alpha);

	if(recursion) return ITER_CONTINUE;
	else return ITER_STOP;
}

int URender::SetProjTextureAll(void *userdata) {
	URProjectiveTexture *texture = (URProjectiveTexture *)userdata;

	this->SetProjTexture(texture);

	if (recursion) return ITER_CONTINUE;
	else return ITER_STOP;
}

int URender::SetTextureTransformAll(void* userdata) {
	double *xform = (double*) userdata;

	if (!this->GetLockTexture()) {
		this->SetTextureTransform(xform);
	}

	if(recursion) return ITER_CONTINUE;
	else return ITER_STOP;
}

int URender::Render(void * /*userdata*/ ){
  //base class does nothing
  cerr << "Base class Rendering\n";
  if(recursion) return ITER_CONTINUE; 
  else return ITER_STOP;
}

int URender::ChangeDataset(void* userdata) {
	nmb_Dataset *dataset = (nmb_Dataset *)userdata;
	if (texture) {
		texture->changeDataset(dataset);
	}
	if(recursion) return  ITER_CONTINUE;
	else return ITER_STOP;
}



void URender::ReloadGeometry() {
        // base class does nothing
        cerr << "Base class Reloading Geometry\n";
}


void URender::DrawBounds(){

       	glColor3f(1,0,0);
       	glBegin(GL_LINE_LOOP);
        glVertex3d(bounds.xmin,bounds.ymin,bounds.zmin);
        glVertex3d(bounds.xmax,bounds.ymin,bounds.zmin);
        glVertex3d(bounds.xmax,bounds.ymax,bounds.zmin);
        glVertex3d(bounds.xmin,bounds.ymax,bounds.zmin);
        glEnd();
        glBegin(GL_LINE_LOOP);
        glVertex3d(bounds.xmin,bounds.ymin,bounds.zmax);
        glVertex3d(bounds.xmax,bounds.ymin,bounds.zmax);
        glVertex3d(bounds.xmax,bounds.ymax,bounds.zmax);
        glVertex3d(bounds.xmin,bounds.ymax,bounds.zmax);
        glEnd();
        glBegin(GL_LINES);
        glVertex3d(bounds.xmin,bounds.ymin,bounds.zmin);
        glVertex3d(bounds.xmin,bounds.ymin,bounds.zmax);
        glVertex3d(bounds.xmin,bounds.ymax,bounds.zmin);
        glVertex3d(bounds.xmin,bounds.ymax,bounds.zmax);
        glVertex3d(bounds.xmax,bounds.ymin,bounds.zmin);
        glVertex3d(bounds.xmax,bounds.ymin,bounds.zmax);
        glVertex3d(bounds.xmax,bounds.ymax,bounds.zmin);
        glVertex3d(bounds.xmax,bounds.ymax,bounds.zmax);
        glEnd();

}


ostream& operator<< (ostream& co,const URender& r){

	switch(r.obj_type){
	case URAXIS:
	  co << " (URAxis)\n";
	  break;
	case URENDER:
	  co << " (UR)\n";
	  break;
	case URPOLYGON:
	  co <<" (URPoly)\n";
	  break;
	default:	
	  co << " (?)\n";
	  break;
	}
  
	return co;
}

void GetNormal(double p1[3], double p2[3], double p3[3], q_vec_type n);
double TestIntersection(double planepoint[3], double p1[3], double p2[3], q_vec_type n);

int URender::IntersectLine(void *userdata){

//THIS FUNCTION NEEDS TO BE WORKED ON -- IT WILL STOP RECURSION WHEN IT
//TESTS A NODE AND FINDS THAT IT DOESN'T INTERSECT ANYTHING -- THIS MAKES MORE SENSE IN CONTEXT
//OF THE NESTED BOUNDING BOXES WHICH AREN'T YET IMPLEMENTED -- SEE NOTE IN URENDER.H
//FOR NOW IT WILL WORK BUT MAYBE NOT AS YOU EXPECTED.  IF ALL THE FIRST LEVEL NODES IN 
//THE TREE MISS THEN YOU WILL NEVER MAKE IT DOWN TO THE CHILDREN BUT THEN IF YOU ARE *ACTUALLY*
//TRYING TO PICK SOMETHING THEN YOU'VE PROBABLY HIT AT LEAST ONE THING SO IT MIGHT BE OK

	SelectionSet *s= (SelectionSet *)userdata;
	if(s==NULL) s=new SelectionSet;
	if(s==NULL){
		cerr << "Unable to create selection set .. Memory fault -- aborting\n"; 
		//kill(getpid(),SIGINT);
                return -1;
	}

	if(obj_type==URPOLYGON){
		q_vec_type p1,p2;
		Xform x;
	//	x=s->rootnode->TGetXformByName("World",name);
                x.apply(s->p1,p1);	//map s.p1 & s.p2 in world to local object space
                x.apply(s->p2,p2);

		double f1[3],u;
		q_vec_type n, pintersect;

		//FACE 1 in Z
		f1[0]=bounds.xmin; f1[1]=bounds.ymin; f1[2]=bounds.zmin;
		n[0]=n[1]=0; n[2]=-1;
		u=TestIntersection(f1,p1,p2,n);
		if(u>=0 && u <= 1){
			pintersect[0]=p1[0]*(1-u)+p2[0]*u;
			pintersect[1]=p1[1]*(1-u)+p2[1]*u;
			pintersect[2]=p1[2]*(1-u)+p2[2]*u;	
			if(pintersect[0] <= bounds.xmax && pintersect[0] >= bounds.xmin &&
			   pintersect[1] <= bounds.ymax && pintersect[1] >= bounds.ymin){
				s->AddSelection(this);
				return ITER_CONTINUE;	
			}
		}

		//FACE 2 in Z
		f1[0]=bounds.xmin; f1[1]=bounds.ymin; f1[2]=bounds.zmax;
		n[0]=n[1]=0; n[2]=1;
		u=TestIntersection(f1,p1,p2,n);
		if(u>=0 && u <= 1){
			pintersect[0]=p1[0]*(1-u)+p2[0]*u;
			pintersect[1]=p1[1]*(1-u)+p2[1]*u;
			pintersect[2]=p1[2]*(1-u)+p2[2]*u;	
			if(pintersect[0] <= bounds.xmax && pintersect[0] >= bounds.xmin &&
			   pintersect[1] <= bounds.ymax && pintersect[1] >= bounds.ymin){
				s->AddSelection(this);
				return ITER_CONTINUE;	
			}
		}



		//FACE 3 in Y
		f1[0]=bounds.xmin; f1[1]=bounds.ymin; f1[2]=bounds.zmin;
		n[0]=n[2]=0; n[1]=-1;
		u=TestIntersection(f1,p1,p2,n);
		if(u>=0 && u <= 1){
			pintersect[0]=p1[0]*(1-u)+p2[0]*u;
			pintersect[1]=p1[1]*(1-u)+p2[1]*u;
			pintersect[2]=p1[2]*(1-u)+p2[2]*u;	
			if(pintersect[2] <= bounds.zmax && pintersect[2] >= bounds.zmin &&
			   pintersect[0] <= bounds.xmax && pintersect[0] >= bounds.xmin){
				s->AddSelection(this);
				return ITER_CONTINUE;
			}
		}

		//FACE 4 in Y
		f1[0]=bounds.xmin; f1[1]=bounds.ymax; f1[2]=bounds.zmin;
		n[0]=n[2]=0; n[1]=1;
		u=TestIntersection(f1,p1,p2,n);
		if(u>=0 && u <= 1){
			pintersect[0]=p1[0]*(1-u)+p2[0]*u;
			pintersect[1]=p1[1]*(1-u)+p2[1]*u;
			pintersect[2]=p1[2]*(1-u)+p2[2]*u;
			if(pintersect[2] <= bounds.zmax && pintersect[2] >= bounds.zmin &&
			   pintersect[0] <= bounds.xmax && pintersect[0] >= bounds.xmin){
				s->AddSelection(this);
				return ITER_CONTINUE;
			}
		}

		//FACE 5 in X
		f1[0]=bounds.xmin; f1[1]=bounds.ymin; f1[2]=bounds.zmin;
		n[1]=n[2]=0; n[0]=-1;
		u=TestIntersection(f1,p1,p2,n);
		if(u>=0 && u <= 1){
			pintersect[0]=p1[0]*(1-u)+p2[0]*u;
			pintersect[1]=p1[1]*(1-u)+p2[1]*u;
			pintersect[2]=p1[2]*(1-u)+p2[2]*u;
			if(pintersect[2] <= bounds.zmax && pintersect[2] >= bounds.zmin &&
			   pintersect[1] <= bounds.ymax && pintersect[1] >= bounds.ymin){
				s->AddSelection(this);
				return ITER_CONTINUE;
			}
		}

		//FACE 6 in X
		f1[0]=bounds.xmax; f1[1]=bounds.ymin; f1[2]=bounds.zmin;
		n[1]=n[2]=0; n[0]=1;
		u=TestIntersection(f1,p1,p2,n);

		if(u>=0 && u <= 1){
			pintersect[0]=p1[0]*(1-u)+p2[0]*u;
			pintersect[1]=p1[1]*(1-u)+p2[1]*u;
			pintersect[2]=p1[2]*(1-u)+p2[2]*u;
			if(pintersect[2] <= bounds.zmax && pintersect[2] >= bounds.zmin &&
			   pintersect[1] <= bounds.ymax && pintersect[1] >= bounds.ymin){
				s->AddSelection(this);
				return ITER_CONTINUE;
			}
		}
	}
	return ITER_STOP;
}

double TestIntersection(double planepoint[3], double p1[3], double p2[3], q_vec_type n){
	double denom,u;
	q_vec_type temp;

	q_vec_subtract(temp,p2,p1);
	denom=q_vec_dot_product(n,temp);
	if(denom==0){return -1;}
	q_vec_subtract(temp,planepoint,p1);
	u=q_vec_dot_product(n,temp)/denom;
	if(u>=0 && u <=1) return u;
	else return -1;

}



//helper function
void GetNormal(double p1[3], double p2[3], double p3[3], q_vec_type n){
	q_vec_subtract(p1,p1,p2);
	q_vec_subtract(p3,p3,p2);
	q_vec_cross_product(p1,p1,p3);
	q_vec_normalize(p1,p1);
	q_vec_copy(n,p1);
}








