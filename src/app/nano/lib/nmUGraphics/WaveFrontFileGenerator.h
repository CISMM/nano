#ifndef WAVE_FRONT_FILE_GENERATOR_H
#define WAVE_FRONT_FILE_GENERATOR_H

#include "FileGenerator.h"

class WaveFrontFileGenerator : public FileGenerator
{
public:
    WaveFrontFileGenerator(const char *filename = NULL);

    virtual int Load(URender *node, GLuint *&dlist_array);
    virtual int ReLoad(URender *node, GLuint *&dlist_array);
};

#endif

