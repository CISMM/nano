#include "MSIFileGenerator.h"
#include "WaveFrontFileGenerator.h"
#include <stdio.h>

GeometryGenerator* CreateFileGenerator(char *fname)
{
    int index,i;
    
    index=strlen(fname);		//save filename
    
    for(i=index-1; i>=0; i--){
        if(fname[i]=='.') break;
    }
    if(strncmp(fname+i+1,"obj",3)==0){
        return new WaveFrontFileGenerator(fname);
    }
    //added by Leila Plummer for loading objects from tube_foundry
    else if (strncmp(fname+i+1,"msi",3)==0){
        //It seemed best to create the structure for loading .msi files
        //as a class, since it is time-efficient for much of the .msi
        //file's data to be stored for later use.  It would be logical
        //to change the WaveFrontFiles to a similar format
        return new MSIFileGenerator(fname);
    }
    else{
        fprintf(stderr, "File type not recognized for %s\n", fname);
    }	
    return (GeometryGenerator*)NULL;
}
