#include "URPolygon.h"
#include "UTree.h"

#include <Tcl_Linkvar.h>
#include <Tcl_Netvar.h>

#include <string.h>

URPolygon::URPolygon():URender(){

  Dlist_array=NULL; 
  num_objects=0;
  disp_proj_text = true;

  obj_type=URPOLYGON;		//set object type
  c[0]=c[1]=c[2]=1; c[3]=1;	//set color and alpha
}

URPolygon::~URPolygon(){
  int i;
  for(i=0; i < num_objects; i++){
	  if(Dlist_array[i]!=0) glDeleteLists(Dlist_array[i],1);
  }
}

GeometryGenerator* URPolygon::GetGenerator()
{
    return d_generator;
}

int URPolygon::SetVisibilityAll(void* userdata) {
	int setvisibility = *(int*) userdata;

	this->SetVisibility(setvisibility);

	if(recursion) return ITER_CONTINUE;
	else return ITER_STOP;
}

int URPolygon::SetProjTextEnableAll(void* userdata) {
	int enableProjTexture = *(int*) userdata;

	this->SetProjTextEnable(enableProjTexture);

	if(recursion) return ITER_CONTINUE;
	else return ITER_STOP;
}

int URPolygon::SetLockObjectAll(void* userdata) {
	bool setlockobject = (*(int*) userdata) != 0;

	this->SetLockObject(setlockobject);

	if(recursion) return ITER_CONTINUE;
	else return ITER_STOP;
}

int URPolygon::SetLockTextureAll(void* userdata) {
	int setlocktexture = *(int*) userdata;

	this->SetLockTexture(setlocktexture);

	if(recursion) return ITER_CONTINUE;
	else return ITER_STOP;
}

int URPolygon::ChangeStaticFile(void* userdata) {
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

int URPolygon::ChangeHeightPlane(void* userdata) {
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


int URPolygon::ScaleAll(void* userdata) {
	double scale = *(double*) userdata;

	this->GetLocalXform().SetScale(scale);

	if(recursion) return ITER_CONTINUE;
	else return ITER_STOP;
}


int URPolygon::SetTransxAll(void* userdata) {	
	double transx = *(double*) userdata;
	const q_vec_type &trans = this->GetLocalXform().GetTrans();
	this->GetLocalXform().SetTranslate(transx, trans[1], trans[2]);

	if(recursion) return ITER_CONTINUE;	
	else return ITER_STOP;
}

int URPolygon::SetTransyAll(void* userdata) {	
	double transy = *(double*) userdata;
	const q_vec_type &trans = this->GetLocalXform().GetTrans();
	this->GetLocalXform().SetTranslate(trans[0], transy, trans[2]);

	if(recursion) return ITER_CONTINUE;	
	else return ITER_STOP;
}

int URPolygon::SetTranszAll(void* userdata) {	
	double transz = *(double*) userdata;
	const q_vec_type &trans = this->GetLocalXform().GetTrans();
	this->GetLocalXform().SetTranslate(trans[0], trans[1], transz);

	if(recursion) return ITER_CONTINUE;	
	else return ITER_STOP;
}

int URPolygon::SetRotAll(void* userdata) {
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


int URPolygon::SetColorAll(void* userdata) {
	RGB* color = (RGB*) userdata;

	this->SetColor3(color->red, color->green, color->blue);

	if(recursion) return ITER_CONTINUE;
	else return ITER_STOP;
}

int URPolygon::SetAlphaAll(void* userdata) {
	float alpha = *(double*) userdata;

	this->SetAlpha(alpha);

	if(recursion) return ITER_CONTINUE;
	else return ITER_STOP;
}

int URPolygon::SetProjTextureAll(void *userdata) {
	URProjectiveTexture *texture = (URProjectiveTexture *)userdata;

	this->SetProjTexture(texture);

	if (recursion) return ITER_CONTINUE;
	else return ITER_STOP;
}

int URPolygon::SetTextureTransformAll(void* userdata) {
	double *xform = (double*) userdata;

	if (!this->GetLockTexture()) {
		this->SetTextureTransform(xform);
	}

	if(recursion) return ITER_CONTINUE;
	else return ITER_STOP;
}

int URPolygon::Render(void * userdata){
	Xform object_xform;

	if(visible){		//if visible draw things
		if (GetProjTextEnable() && texture) {
			double objectToWorld[16];
			if (GetLockObject()) {
				localXformForLockedObject.GetOpenGLMatrix(objectToWorld);
			} else {
				GetLocalXform().GetOpenGLMatrix(objectToWorld);
				localXformForLockedObject = GetLocalXform();
			}
			texture->enable(textureTransform, objectToWorld, true);
		}

		glPushAttrib(GL_CURRENT_BIT | GL_TRANSFORM_BIT);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		this->GetLocalXform().Push_As_OGL();
		glColor4fv(c);
		// if alpha is not 1 then use alpha blending
		if(c[3]!=1.0){
			glPushAttrib(GL_COLOR_BUFFER_BIT);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
			glAlphaFunc(GL_GREATER,0);
			glEnable(GL_ALPHA_TEST);
		}

		//draw display lists
		for(int i = 0; i < num_objects; i++) { 
			glCallList(Dlist_array[i]);
		}
		if(selected){		//draw bounding box
			DrawBounds();
		}

		if(c[3]!=1.0){		//turn off alpha blending
			glPopAttrib();
		}

		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
		glPopAttrib();
	}

    if(recursion) return ITER_CONTINUE;
	else return ITER_STOP;

}

void URPolygon::LoadGeometry(GeometryGenerator *gen)
{
    d_generator = gen;
    num_objects = d_generator->Load(this, Dlist_array);
}

/**Added by Leila Plummer, based on above code, for importing from tube_foundry**/

void URPolygon::ReloadGeometry(){
  num_objects = d_generator->ReLoad(this, Dlist_array);
}



