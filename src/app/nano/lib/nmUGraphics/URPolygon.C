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


int URPolygon::Scale(void* userdata) {
		double scale = *(double*) userdata;
cout << "current scale" << this->GetLocalXform().GetScale() << endl;
cout << "surface scale" << *(double*) userdata << endl;
cout << "new scale" << this->GetLocalXform().GetScale() * (* (double*) userdata) << endl;
	this->GetLocalXform().SetScale(this->GetLocalXform().GetScale() * scale);
 
	const q_vec_type & q1 = this->GetLocalXform().GetTrans();
	q_vec_type q2;
	q2[0] = q1[0]; q2[1] = q1[1]; q2[2] = q1[2];

	q_vec_scale(q2, scale, q2);
	this->GetLocalXform().SetTranslate(q2);

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



