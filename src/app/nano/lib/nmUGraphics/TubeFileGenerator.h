// Header file for loading tubes output by the Shape Analyze Code
// This code is *not* related to the TubeGenerator files


#ifndef TUBE_FILE_GENERATOR_H
#define TUBE_FILE_GENERATOR_H

#include "FileGenerator.h"

class TubeFileGenerator : public FileGenerator {
public:
	TubeFileGenerator(const char *filename = NULL);

	virtual int Load(URender *node, GLuint *&dlist_array);
	virtual int ReLoad(URender *node, GLuint *&dlist_array);
};

#endif
