#include "FileGenerator.h"
#include "WaveFrontFileGenerator.h"
#include "MSIFileGenerator.h"
#include <stdio.h>
#include <string.h>

FileGenerator::FileGenerator(const char *fname, const char * exten)
 : GeometryGenerator(), extension(exten), filename(NULL)
{
    StoreFilename(fname);
}

void FileGenerator::StoreFilename(const char* fname){
    if (fname == NULL) {
        return;
    }

    int len=strlen(fname);
    if(strncmp(fname+(len-3),extension,3)!=0){
        fprintf(stderr, "FileGenerator of extension %s called on incorrect file type\n", extension);
        filename = NULL;
        return;
    }
    if(filename) delete []filename;
    filename=new char[strlen(fname)+1];
    strcpy(filename,fname);
    return;
}

FileGenerator* FileGenerator::CreateFileGenerator(const char *fname)
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
    return (FileGenerator*)NULL;
}

