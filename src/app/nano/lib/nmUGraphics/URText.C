#include "URText.h"
#include "UTree.h"

#include <string.h>

URText::URText():URender()
{
  Dlist_array=NULL; 
  num_objects=0;
  disp_proj_text = false;

  obj_type=URTEXT;		//set object type
  c[0]=c[1]=c[2]=1; c[3]=1;	//set color and alpha

  // Load the default font
  d_font = loadFont(NULL);
}

URText::~URText(){
  int i;
  for(i=0; i < num_objects; i++){
    if(Dlist_array[i]!=0) glDeleteLists(Dlist_array[i],1);
  }
}

GeometryGenerator* URText::GetGenerator()
{
    return d_generator;
}

int URText::Render(void * userdata){
    Xform object_xform;

    if(visible){		//if visible draw things
	// We don't do projective texture onto text, so that code is not
        // present here.

	glPushAttrib(GL_CURRENT_BIT | GL_TRANSFORM_BIT | GL_ENABLE_BIT);
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

	// Set the color mode to use no lighting, and only ambient color
	glDisable(GL_LIGHTING);

	// Draw the text at the correct location
	glRasterPos3f(0.0,0.0,0.0);
	drawStringInFont(d_font, d_text_string.c_str());

	// If we're selected, underline the text by drawing an underscore
	// over top of each of the characters in the string.
	if(selected){
	    string underscores;
	    int i;
	    for (i = 0; i < d_text_string.size(); i++) {
	      underscores += '_';
	    }
	    drawStringInFont(d_font, underscores.c_str());
	}

	if(c[3]!=1.0){		//turn off alpha blending
	    glPopAttrib();
	}

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glPopAttrib();
    }

    if(recursion) {
      return ITER_CONTINUE;
    } else {
      return ITER_STOP;
    }
}
