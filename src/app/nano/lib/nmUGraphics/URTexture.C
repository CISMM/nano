#include "URTexture.h"
#include <stdlib.h>
#include <stdio.h>
#include <fstream.h>
#include <assert.h>

#ifdef __CYGWIN__
// XXX juliano 9/19/99
//       this was implicitly declared.  Needed to add decl.
//       getpid comes from unistd.h
#include <sys/types.h>  // for pid_t
extern "C" {
pid_t getpid();
}
#endif

int UR_TEXTURE_MIPMAP=1;
int UR_TEXTURE_FILTER=1;
int UR_TEXTURE_REPEAT_S=1;
int UR_TEXTURE_REPEAT_T=1;

URTexture::URTexture():URender()
{
	texture_data=NULL;
	xsize=ysize=maxval=0;
	textureid=0;
	obj_type=URTEXTURE;
	alpha=0;
  
	return;
  
}

URTexture::~URTexture()
{
	if(texture_data!=NULL){
		delete []texture_data;
	}
  
	return;
}

  
int URTexture::LoadPPMFile(char *filename, int *filter)
{
	ifstream in;
	char buf[200];
	long isize,rsize;
	long i;
  
	//open texture file
	if(filename==NULL){
		cerr << "Unexpectedly given a null ptr for a filename\n";
		return -1;
	}
 	if(filter) alpha=1; 
	in.open(filename);
	assert(in);
  
	if(in==NULL){
		cerr << "Couldn't open texture file " << filename << "\n";
		return -1;	
	}

  
	buf[0]='\0';
	//check for magic number (is it a PPM file?)
	in.getline(buf,10);
	if(buf[0]!='P' || buf[1]!='6'){
		cerr << "Looking for magic number \'P6\'";
		return -1;
    	}
	in.getline(buf,200);	//get the #creator line from the file
	in.getline(buf,200);
	sscanf(buf,"%ld %ld",&xsize,&ysize);
	in.getline(buf,200);
	sscanf(buf,"%ld",&maxval);
	cout << "Loading Texture: " << filename << " " << xsize << "x" << ysize;
   

	if(texture_data!=NULL) delete []texture_data;
   

    //get texture from file
    
    if(filter) isize = xsize * ysize * 4;
    else isize = xsize * ysize * 3;
    rsize = xsize * ysize * 3;

    texture_data = new GLubyte[isize];
    if(texture_data==NULL){
	cerr << "Could not allocate texture data array -- memory fault\n";
	kill(getpid(),SIGINT);
    }
    in.read(texture_data, rsize);

    if(filter){
	for(i=(xsize*ysize-1); i>=0; i--){
		texture_data[4*i]=texture_data[3*i];	
		texture_data[4*i+1]=texture_data[3*i+1];	
		texture_data[4*i+2]=texture_data[3*i+2];	

		if( texture_data[3*i] == filter[0] /*&&
		    texture_data[3*i+1] == filter[1] &&
		    texture_data[3*i+2] == filter[2]*/ ){
			 texture_data[4*i+3]=0;
		}
		else{
		   texture_data[4*i+3]=255; 
		}

	}
    }

    in.close();
    Compile2DTexture();
    cout << " ID=" << textureid << endl;
	return 1;
	
}

void URTexture::Compile2DTexture()
{


#ifdef sgi
	glGenTexturesEXT(1, &textureid);
                         //create a texture id 
	glBindTextureEXT(GL_TEXTURE_2D, textureid);
                         //bind and make that id current
#endif

	//set parameters -- there are a few globals at the top of this file to set
	//behavior with
	if(UR_TEXTURE_REPEAT_S) glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	else glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
	if(UR_TEXTURE_REPEAT_S) glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	else glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
  
	if(UR_TEXTURE_MIPMAP){
		if(UR_TEXTURE_FILTER){
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
		}
		else{
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST_MIPMAP_NEAREST);
		}
#ifdef	sgi
		//XXX I don't know if something else should be done if this isn't an SGI
		if(alpha){
#ifdef sgi
			gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA /*# components*/ ,
				  xsize /*width */, ysize /*height*/,
				  GL_RGBA /*format*/,GL_UNSIGNED_BYTE,/*type*/
				  (void*)texture_data /*ptr to data*/);
#endif
		}
		else{
#ifdef sgi
			gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB/*# components*/ ,
				  xsize /*width */, ysize /*height*/,
				  GL_RGB /*format*/,GL_UNSIGNED_BYTE,/*type*/
				  (void*)texture_data /*ptr to data*/);
#endif
			cerr << xsize <<  " " << ysize << "\n";
		}
	
#endif
	}
	else{
		if(UR_TEXTURE_FILTER){
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		}
		else{
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
		}
		if(alpha){
			glTexImage2D( GL_TEXTURE_2D, 0, /*used for multires textures -- 0 for me here */
					  GL_RGBA /*internal format */, xsize, ysize, 
					  0 /* border width -- 0 for me  */,
					  GL_RGBA /*format*/, GL_UNSIGNED_BYTE /*type*/, 
					  texture_data /*ptr to data */);
		}
		else{
			glTexImage2D( GL_TEXTURE_2D, 0, /*used for multires textures -- 0 for me here */
					  GL_RGB /*internal format */, xsize, ysize, 
					  0 /* border width -- 0 for me  */,
					  GL_RGB /*format*/, GL_UNSIGNED_BYTE /*type*/, 
					  texture_data /*ptr to data */);
		}
	
	}

	return;
	
}


int URTexture::Render(void * /*userdata*/ )
{  
  
	//enable texture
	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	txform.Push_As_OGL();

//	glPushAttrib(GL_TEXTURE_BIT | GL_ENABLE_BIT);
	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
  
#ifdef sgi
	glBindTextureEXT(GL_TEXTURE_2D,textureid);
#endif
	if(alpha){
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glAlphaFunc(GL_GREATER,0);
		glEnable(GL_ALPHA_TEST);
	}
	glMatrixMode(GL_MODELVIEW);


	return 1;
  
}
void URTexture::PostRender()
{
	if(alpha){ 
		glDisable(GL_BLEND);
		glEnable(GL_ALPHA_TEST);
	}
	glDisable(GL_TEXTURE_2D);
	glMatrixMode(GL_TEXTURE);
		glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
//	glPopAttrib();
}
