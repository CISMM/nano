#ifndef FILE_GENERATOR_H
#define FILE_GENERATOR_H

#include "GeomGenerator.h"

//Abstract class for any Generator that generates the information
//from a pre-defined file.  
class FileGenerator : public GeometryGenerator
{
public:
    FileGenerator(const char *filename, const char *exten);

    void StoreFilename(const char* fname);
    static FileGenerator* CreateFileGenerator(const char *fname);
    //In order to allow for an easy interface to grabbing the 
    //correct generator for whatever file you want to load
    //this class has this static method that automatically chooses
    //which Generator is appropriate for the passed in file name
    const char* GetExtension() {return extension;}

protected:
    char *filename;
    //Each subclass needs to set this appropriately
    const char* const extension;
};

#endif
