#ifndef FILE_GENERATOR_H
#define FILE_GENERATOR_H

#include "GeomGenerator.h"

class FileGenerator : public GeometryGenerator
{
public:
    FileGenerator(const char *filename, const char *exten);

    void StoreFilename(const char* fname);
    static FileGenerator* CreateFileGenerator(const char *fname);
    const char* const GetExtension() {return extension;}

protected:
    char *filename;
    //Each subclass needs to set this appropriately
    const char* const extension;
};

#endif