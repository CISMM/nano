#include "URPolygon.h"
#include "URTexture.h"
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
};

int URPolygon::SetProjTextAll(void* userdata) {
	int setprojtext = *(int*) userdata;

	this->SetProjText(setprojtext);

	if(recursion) return ITER_CONTINUE;
	else return ITER_STOP;
};

int URPolygon::ChangeStaticFile(void* userdata) {
	// modifies the scale and translation so appears in same place...

	double scale = *(double*) userdata;

	this->GetLocalXform().SetScale(this->GetLocalXform().GetScale() * scale);

	const q_vec_type &q1 = this->GetLocalXform().GetTrans();
	q_vec_type q2;
	
	q2[0] = q1[0]; q2[1] = q1[1]; q2[2] = q1[2];

	q_vec_scale(q2, scale, q2);

	this->GetLocalXform().SetTranslate(q2);

	this->scale_triangles = scale;	// in case this object is a ShapeAnalyze nanotube
									// file that we want to send to the AFM simulator

	if(recursion) return ITER_CONTINUE;
	else return ITER_STOP;
}


int URPolygon::ScaleAll(void* userdata) {
	double scale = *(double*) userdata;

	this->GetLocalXform().SetScale(scale);

	this->scale_triangles = scale;	// in case this object is a ShapeAnalyze nanotube
									// file that we want to send to the AFM simulator

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

int URPolygon::SetRotxAll(void* userdata) {	
	double rotx = *(double*) userdata;
	const q_type &rot = this->GetLocalXform().GetRot();
	this->GetLocalXform().SetRotate(rotx, rot[1], rot[2], rot[3]);

	if(recursion) return ITER_CONTINUE;	
	else return ITER_STOP;
}

int URPolygon::SetRotyAll(void* userdata) {	
	double roty = *(double*) userdata;
	const q_type &rot = this->GetLocalXform().GetRot();
	this->GetLocalXform().SetRotate(rot[0], roty, rot[2], rot[3]);

	if(recursion) return ITER_CONTINUE;	
	else return ITER_STOP;
}

int URPolygon::SetRotzAll(void* userdata) {	
	double rotz = *(double*) userdata;
	const q_type &rot = this->GetLocalXform().GetRot();
	this->GetLocalXform().SetRotate(rot[0], rot[1], rotz, rot[3]);

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

	

int URPolygon::Render(void * userdata){


// Commented code was here before--not sure what, if any, of this is needed... 


/*	if(visible){		//if visible draw things
 		if(texture!=NULL){
			if(texture->GetType()==URTEXTURE){
				texture->Render();	//setup texturing
				glColor3f(1,1,1);
			}
		}

		else{

			glColor4fv(c);

			// if alpha is not 1 then use alpha blending
			if(c[3]!=1.0){


				glPushAttrib(GL_COLOR_BUFFER_BIT);

				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
				glAlphaFunc(GL_GREATER,0);
				glEnable(GL_ALPHA_TEST);

				glPopAttrib();

			}
	  	}

		int i;		//draw display lists
		for(i = 0; i < num_objects; i++) { 
			glCallList(Dlist_array[i]);
		}

		if(selected){		//draw bounding box
			DrawBounds();
		}

		if(c[3]!=1.0){		//turn off alpha blending
       			glDisable(GL_BLEND);
			glEnable(GL_ALPHA_TEST);
		}

		if(texture!=NULL){	//clean up after the texture -- turn texturing off
			if(texture->GetType() == URTEXTURE){
				((URTexture *)texture)->PostRender();
			}
		}

	}
*/

	if(visible){		//if visible draw things
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		this->GetLocalXform().Push_As_OGL();

		if (* (int*)userdata == 1 /* displaying projective textures */ ) {
			glMatrixMode(GL_TEXTURE);
			if (this->ShowProjText()) { 
				glEnable(GL_TEXTURE_2D);
				glEnable(GL_TEXTURE_GEN_S);
				glEnable(GL_TEXTURE_GEN_T);
				glEnable(GL_TEXTURE_GEN_R);
				glEnable(GL_TEXTURE_GEN_Q);

				glPushMatrix();
				this->GetLocalXform().Push_As_OGL();
			}
			else {
				glDisable(GL_TEXTURE_2D);
				glDisable(GL_TEXTURE_GEN_S);
				glDisable(GL_TEXTURE_GEN_T);
				glDisable(GL_TEXTURE_GEN_R);
				glDisable(GL_TEXTURE_GEN_Q);
			}
		}

		glColor4fv(c);
		glPushAttrib(GL_COLOR_BUFFER_BIT);
	
		//draw display lists
		for(int i = 0; i < num_objects; i++) { 
			glCallList(Dlist_array[i]);
		}
		glPopAttrib();

		if (* (int*)userdata == 1) {
			if (this->ShowProjText()) {
				glMatrixMode(GL_TEXTURE);
				glPopMatrix();
			}
		}

		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
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



