#include "transformFile.h"

const int TransformFileEntry::maxFileName = 256;

TransformFile::TransformFile() {}

vrpn_bool TransformFile::lookupImageTransformByName(const char *imageName, 
                                              double *matrix)
{
    list<TransformFileEntry>::iterator currTransformEntry;
    for (currTransformEntry = data.begin();
         currTransformEntry != data.end();
         currTransformEntry++){
       if (strcmp((*currTransformEntry).fileName,
                  imageName) == 0) {
           (*currTransformEntry).transform.getMatrix(matrix);
           return VRPN_TRUE;
       }
    }
    return VRPN_FALSE;
}

int TransformFile::load(char *filename)
{
  FILE *inFile = fopen(filename, "r");
  if (!inFile) {
    fprintf(stderr, "Error opening file: %s\n", filename);
    return -1;
  }
  vrpn_bool entryOkay;
  int numEntries = data.size();
  int numFileEntries;
  char *temp, *name;
  const int maxLineLength = 512;
  char line[maxLineLength];
  double matrix[16];
  float f0, f1, f2, f3;
  fgets(line, maxLineLength, inFile);
  if (!line) {
    fprintf(stderr, "Error reading first line of transformation file\n");
    return -1;
  }
  strtok(line, " ");
  temp = strtok(NULL, " ");
  if (temp == NULL ||
    sscanf(temp, "%d", &numFileEntries) != 1) {
    fprintf(stderr, "Error, couldn't read number from first line\n");
    fclose(inFile);
    return -1;
  }
  numEntries += numFileEntries;
  fgets(line, maxLineLength, inFile);
  int num = 0;
  int i;
  while (line) {
    entryOkay = vrpn_TRUE;
    strtok(line, " ");
    name = strtok(NULL, " ");
    if (!name || (strlen(name) == 0)) {
      fprintf(stderr, "Error at entry %d loading transformation file\n",
                       num);
      entryOkay = vrpn_FALSE;
      break;
    }
    for (i = 0; i < 4; i++) {
      fgets(line, maxLineLength, inFile);
      if (!line ||
          sscanf(line, "%g %g %g %g", &f0, &f1, &f2, &f3) != 4){
         fprintf(stderr, "Error while reading transformation\n");
         entryOkay = vrpn_FALSE;
         break;
      }
      matrix[i] = f0; matrix[i+4] = f1; matrix[i+8] = f2; matrix[i+12] = f3;
    }
    if (entryOkay) {
      // add it to the list
      TransformFileEntry newEntry;
#ifdef _WIN32
      _snprintf(newEntry.fileName, newEntry.maxFileName, "%s", name);
#else
      snprintf(newEntry.fileName, newEntry.maxFileName, "%s", name);
#endif
      newEntry.transform.setMatrix(matrix);
      data.push_back(newEntry);
      num++;
    } else {
      break;
    }
    fgets(line, maxLineLength, inFile);
  }

  if (data.size() != numEntries) {
    fprintf(stderr, "Error, did not read the specified number of entries"
                    " from file %s\n", filename);
  }
  fclose(inFile);
  return 0;
}

int TransformFile::save(char *filename)
{
  FILE *outFile = fopen(filename, "w");
  list<TransformFileEntry>::iterator currEntry;
  double matrix[16];

  int num = 0, i;

  num = data.size();
  fprintf(outFile, "num_files %d\n", num);
  for (currEntry = data.begin();
       currEntry != data.end(); currEntry++)
  {
    fprintf(outFile, "file_name %s\n", (*currEntry).fileName);
    (*currEntry).transform.getMatrix(matrix);
    for (i = 0; i < 4; i++){
        fprintf(outFile, "%g %g %g %g\n",
              matrix[i], matrix[i+4], matrix[i+8], matrix[i+12]);
    }
  }
  fclose(outFile);
  return 0;
}
