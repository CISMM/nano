#include "URPolygon.h"
#include "UTree.h"

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



