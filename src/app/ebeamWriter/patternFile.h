#ifndef PATTERNFILE_H
#define PATTERNFILE_H

#include <stdio.h>
#include "exposurePattern.h"

class PatternFile {
  public:
    PatternFile();
    void setPattern(ExposurePattern &pattern);
    ExposurePattern &getPattern();
    int writeToFile(const char *filename);
    int writeToFile(list<PatternShapeListElement> &shapes, FILE *fout);
    int readFromFile(const char *filename);
    int readFromFile(list<PatternShapeListElement> &shapes, FILE *fin);

  protected:
    ExposurePattern d_pattern;
};

#endif
