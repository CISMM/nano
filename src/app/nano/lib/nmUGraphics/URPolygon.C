#include "URPolygon.h"
#include "URTexture.h"
#include <string.h>

URPolygon::URPolygon():URender(){

  Dlist_array=NULL; 
  num_objects=0;

  obj_type=URPOLYGON;		//set object type
  c[0]=c[1]=c[2]=1; c[3]=1;	//set color and alpha
  filename=NULL;
}

MSIFile* URPolygon::GetMSIFile(){
  return MyMSIFile;
}

void URPolygon::StoreFilename(char* fname){
  if(filename) delete []filename;
  filename=new char[strlen(fname)+1];
  strcpy(filename,fname);
  return;
}

URPolygon::~URPolygon(){
  int i;
  for(i=0; i < num_objects; i++){
	  if(Dlist_array[i]!=0) glDeleteLists(Dlist_array[i],1);
  }
}

int URPolygon::Render(void * /*userdata*/ ){
  
	if(visible){		//if visible draw things
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
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
				glAlphaFunc(GL_GREATER,0);
				glEnable(GL_ALPHA_TEST);
			}
	  	}

		int i;		//draw display lists
		for(i=0; i < num_objects; i++){ glCallList(Dlist_array[i]);}

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

        if(recursion) return ITER_CONTINUE;
	else return ITER_STOP;
}

void URPolygon::LoadGeometry(char *fname){
	int index,i;
	
	index=strlen(fname);		//save filename
	StoreFilename(fname);

	for(i=index-1; i>=0; i--){
		if(fname[i]=='.') break;
	}
	if(strncmp(fname+i+1,"obj",3)==0){
		num_objects=LoadWaveFrontFile(this,fname,Dlist_array);
	}
        //added by Leila Plummer for loading objects from tube_foundry
      	else if (strncmp(fname+i+1,"msi",3)==0){
	  //It seemed best to create the structure for loading .msi files
          //as a class, since it is time-efficient for much of the .msi
          //file's data to be stored for later use.  It would be logical
          //to change the WaveFrontFiles to a similar format
           MyMSIFile = new MSIFile(this,fname);
           //num_objects=MyMSIFile->LoadMSIFile();
           num_objects=MyMSIFile->LoadMSIFile(Dlist_array);
	}
	else{
		cerr << "File type not recognized for " << fname << "\n";
	}	
	return;
}

/**Added by Leila Plummer, based on above code, for importing from tube_foundry**/

void URPolygon::ReloadGeometry(){
  num_objects=MyMSIFile->ReloadMSIFile(Dlist_array);
}



