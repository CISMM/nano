#ifndef PROJ_TEXT_OBJ_GENERATOR_H
#define PROJ_TEXT_OBJ_GENERATOR_H

#include "FileGenerator.h"

class ProjTextObjGenerator : public FileGenerator {
public:
	ProjTextObjGenerator(const char *filename = NULL);

	virtual int Load(URender *node, GLuint *&dlist_array);
	virtual int ReLoad(URender *node, GLuint *&dlist_array);
};

#endif