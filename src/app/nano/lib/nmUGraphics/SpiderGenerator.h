#ifndef SPIDER_GENERATOR_H
#define SPIDER_GENERATOR_H

#include "FileGenerator.h"

#include "URSpider.h"

class SpiderGenerator : public FileGenerator {
public:
	SpiderGenerator(const char *filename = NULL);

	virtual int Load(URender *node, GLuint *&dlist_array);
	virtual int ReLoad(URender *node, GLuint *&dlist_array);
};

#endif