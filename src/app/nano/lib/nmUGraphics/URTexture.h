#ifndef URTEXTURE_H
#define URTEXTURE_H

#include <iostream.h>
#include "URender.h"
#include <GL/gl.h>
#include <GL/glu.h>

extern int UR_TEXTURE_MIPMAP;		//these are used to set behaviors properties for all 
extern int UR_TEXTURE_FILTER;		//textures ... should probably not be compile time	
extern int UR_TEXTURE_REPEAT_S;		//constants but user definable flags on a per instance
extern int UR_TEXTURE_REPEAT_T;		//basis -- CHANGE THIS!!!!!
					//for now if you use MIPMAP (values are actually set in the .C )
					//then you must have a square nxn image or it gets upset

class URTexture:public URender{
private:
	Xform txform;			//texture xform
	long xsize,ysize,maxval;
	int alpha;  			//flag indicating if this is a 
					//texture with transparency
	GLubyte *texture_data;		//the texture data from the PPM
	GLuint textureid;		//texture ID for glBindTextures
	
public:

	//filter is a ptr to an array of int filter[3] that is used to mark
	//a color in the incoming PPM file as transparent.  It will
	//ad an alpha channel to the underlying PPM data before passing it
	//into glTexture2D -- if NULL it will read it as a standard texture 
	int LoadPPMFile(char*filename, int *filter=NULL);

	URTexture();
	~URTexture();
	void SetTexID(GLuint id){	textureid=id;}
	GLuint GetTexID(){	return textureid;}
	void SetAlpha(int a){ alpha=a;}

	//GL_TEXTURE IS A SPECIAL KIND OF RENDER HERE IT SHOULD !!NEVER!! 
	//RENDER ITS CHILDREN BECAUSE THEN IT WILL TEXTURE EVERYTHING IN 
	//THE SUBTREE WHICH IS GENERALLY NOT DESIRED... INSTEAD
	//URPOLYGON HAS A POINTER TO A URTEXTURE AND CALLS URTEX::RENDER THEN
	//URPOLYGON RENDERS ITSELF THEN CALLS URTEX::POSTRENDER TO CLEAN UP
	//AFTER THE TEXTURE CALL -- 
	//SIMILARLY URTEXTURE OBJECTS SHOULD NOT BE FOUND IN THE GENERAL 
	//RENDERING HIERARCHY BUT INSTEAD SHOULD BE IN A SEPARATE LIST
	//THAT DOESN'T CALL RENDER ON THE PARENT NODE  -- MAYBE MAKE THIS A 
	//UDATA OBJECT INSTEAD SINCE IT TECHNICALLY CONTAINS DATA FOR A 
	//RENDERABLE BUT IT CALLS GL STUFF SO ITS KIND OF BOTH?
	int Render(void *userdata=NULL);
	void PostRender();
	void Compile2DTexture();
  
};

#endif
