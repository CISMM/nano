#include "UTree.h"
#include "URender.h"
#include <string.h>
#include <stdlib.h>

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
	disp_proj_text = 1;
	CCW = 0;
	tess = 10;
	num_triangles = 0;
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

int URender::Render(void * /*userdata*/ ){
  //base class does nothing
  cerr << "Base class Rendering\n";
  if(recursion) return ITER_CONTINUE; 
  else return ITER_STOP;
}

int URender::SetVisibilityAll(void * /*userdata*/) {	
	//base class does nothing
	cerr << "Base class Changing Visibility\n";
	if(recursion) return ITER_CONTINUE; 
	else return ITER_STOP;
}

int URender::SetProjTextAll(void * /*userdata*/) {	
	//base class does nothing
	cerr << "Base class Setting Projective Texture\n";
	if(recursion) return ITER_CONTINUE; 
	else return ITER_STOP;
}

int URender::ScaleAll(void * /*userdata*/) {	
	//base class does nothing
	cerr << "Base class Scaling\n";
	if(recursion) return ITER_CONTINUE; 
	else return ITER_STOP;
}

int URender::SetTransxAll(void * /*userdata*/) {	
	//base class does nothing
	cerr << "Base class Translating\n";
	if(recursion) return ITER_CONTINUE; 
	else return ITER_STOP;
}

int URender::SetTransyAll(void * /*userdata*/) {	
	//base class does nothing
	cerr << "Base class Translating\n";
	if(recursion) return ITER_CONTINUE; 
	else return ITER_STOP;
}

int URender::SetTranszAll(void * /*userdata*/) {	
	//base class does nothing
	cerr << "Base class Translating\n";
	if(recursion) return ITER_CONTINUE; 
	else return ITER_STOP;
}

int URender::SetRotxAll(void * /*userdata*/) {	
	//base class does nothing
	cerr << "Base class Translating\n";
	if(recursion) return ITER_CONTINUE; 
	else return ITER_STOP;
}

int URender::SetRotyAll(void * /*userdata*/) {	
	//base class does nothing
	cerr << "Base class Translating\n";
	if(recursion) return ITER_CONTINUE; 
	else return ITER_STOP;
}

int URender::SetRotzAll(void * /*userdata*/) {	
	//base class does nothing
	cerr << "Base class Translating\n";
	if(recursion) return ITER_CONTINUE; 
	else return ITER_STOP;
}

int URender::SetColorAll(void * /*userdata*/) {	
	//base class does nothing
	cerr << "Base class Translating\n";
	if(recursion) return ITER_CONTINUE; 
	else return ITER_STOP;
}

int URender::SetAlphaAll(void * /*userdata*/) {	
	//base class does nothing
	cerr << "Base class Translating\n";
	if(recursion) return ITER_CONTINUE; 
	else return ITER_STOP;
}



int URender::ChangeStaticFile(void* /*userdata*/) {
	// bas class does nothing
	cerr << "Base class Changing Static File\n";
	if(recursion) return  ITER_CONTINUE;
	else return ITER_STOP;
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
	case URTEXTURE:
	  co << " (URTex)\n";
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

void URender::SetTexture(URender *tex)
{
	if(tex->obj_type!=URTEXTURE){
		cerr << "Tried to bind an object as texture which wasn't of type URTexture\n";
		return;
	}
	texture=tex;
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








