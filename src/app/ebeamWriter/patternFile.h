#ifndef PATTERNFILE_H
#define PATTERNFILE_H

#include <stdio.h>
#include "exposurePattern.h"

class PatternFile {
  public:
    PatternFile();
    void setPattern(list<PatternShape> &pattern);
    list<PatternShape> &getPattern();
    int writeToFile(const char *filename);
    int writeToFile(list<PatternShape> &shapes, FILE *fout);
    int readFromFile(const char *filename);
    int readFromFile(list<PatternShape> &shapes, FILE *fin);

  protected:
    list<PatternShape> d_pattern;
};

#endif
