#ifndef URPOLYGON_H
#define URPOLYGON_H

#include <iostream.h>
#include "URender.h"
#include "MSIFile.h"
#include "WaveFrontFile.h"

#define DLIST_CHUNK	5

class URPolygon:public URender{
private:
  GLuint *Dlist_array;
  int num_objects;
  int size;
  char *filename;
  //added by Leila Plummer for importing from tube_foundry
  MSIFile *MyMSIFile;

public:

  //constructor destructor
  URPolygon();
  ~URPolygon();

  MSIFile* GetMSIFile();
  //management functions
  void StoreFilename(char*);   //used to store the filename geometry was dervied from
			       //will be used later for reading and writing config files

  //standard functions
  int Render(void *userdata=NULL);


  //PARSERS
  void LoadGeometry(char *filename);	//generic function to scan file names and call
					//the appropriate parser

//Added by Leila Plummer for importing from tube_foundry
  void ReloadGeometry();
};


#endif
