#ifndef DS_AXIS_GENERATOR_H
#define DS_AXIS_GENERATOR_H

#include "FileGenerator.h"

class DsAxisGenerator : public FileGenerator {
public:
	DsAxisGenerator(const char *filename = NULL);

	virtual int Load(URender *node, GLuint *&dlist_array);
	virtual int ReLoad(URender *node, GLuint *&dlist_array);
	void BuildListDsAxis(URender *Pobject, GLuint dl);
};

#endif
