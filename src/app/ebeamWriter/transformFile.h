#ifndef TRANSFORMFILE_H
#define TRANSFORMFILE_H

#include "vrpn_Types.h"
#include "nmb_ImageTransform.h"
#include "list.h"

/*
format for transformation file (the matrix gives the world to image xform):
num_files <n>
file_name <filename_1>
t00 t01 t02 t03
t10 t11 t12 t13
t20 t21 t22 t23
t30 t31 t32 t33
file_name <filename_2>
t00 t01 t02 t03
t10 t11 t12 t13
t20 t21 t22 t23
t30 t31 t32 t33
...
file_name <filename_n)>
t00 t01 t02 t03
t10 t11 t12 t13
t20 t21 t22 t23
t30 t31 t32 t33
*/

class TransformFileEntry {
  public:
    TransformFileEntry():transform(4,4)
    { fileName[0] = '\0'; }

    char fileName[256];
    static const int maxFileName;
    nmb_ImageTransformAffine transform;
};

class TransformFile {
  public:
    TransformFile();
    int load(char *filename);
    int save(char *filename);
    vrpn_bool lookupImageTransformByName(const char *name, double *matrix);

  protected:
    list<TransformFileEntry> data;
};

#endif
