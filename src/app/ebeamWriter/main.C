#include "GL/glut.h"
#include <stdio.h>
#include "patternEditor.h"
#include "nmb_ImageTransform.h"
#include "nmr_Util.h"

PatternEditor *pe = NULL;

/* arguments:

 -t <transformation file>
 -f <spmlab file or pgm>

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

const int TransformFileEntry::maxFileName = 256;

static int parseArgs(int argc, char **argv);
static int loadTransformationFile(char *filename,
                                  list<TransformFileEntry> data);
static int saveTransformationFile(char *filename,
                                  list<TransformFileEntry> data);

#define MAX_PLANNING_IMAGES 10
static nmb_ListOfStrings planningImageNameList;
static char **planningImageNames;
static int numPlanningImages = 0;
nmb_ImageList *planningImages = NULL;

static nmb_Image *liveImage;
static list<TransformFileEntry> transformFile;
static char transformFileName[256];
static vrpn_bool timeToQuit = vrpn_FALSE;

int main(int argc, char **argv)
{
    int i;
    planningImageNames = new char *[MAX_PLANNING_IMAGES];
    for (i = 0; i < MAX_PLANNING_IMAGES; i++){
        planningImageNames[i] = NULL;
    }
    sprintf(transformFileName, "default_transforms.txt");
    // fill in the list of planning images, setting their worldToImage
    // transformations from the transformation file if it is available and
    // contains the files loaded
    parseArgs(argc, argv);

    // only load the transformations if the transform file exists
    FILE *test = fopen(transformFileName, "r");
    if (test) {
       fclose(test);
       loadTransformationFile((char *)transformFileName, transformFile);
    }

    TopoFile defaultTopoFileSettings;
    planningImages = new nmb_ImageList(&planningImageNameList,
                          (const char **)planningImageNames, numPlanningImages,
                          defaultTopoFileSettings);

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);

    

    pe = new PatternEditor();

    double matrix[16];
    double default_matrix[16] = {1.0, 0.0, 0.0, 0.0,
                                 0.0, 1.0, 0.0, 0.0,
                                 0.0, 0.0, 1.0, 0.0,
                                 0.0, 0.0, 0.0, 1.0};
    nmb_Image *currImage;
    list<TransformFileEntry>::iterator currTransformEntry;

    for (i = 0; i < planningImages->numImages(); i++){
        currImage = planningImages->getImage(i);
        currImage->normalize();
        currImage->setWorldToImageTransform(default_matrix);
        // search for this image in the list of transformations we loaded
        for (currTransformEntry = transformFile.begin();
             currTransformEntry != transformFile.end();
             currTransformEntry++){
           if (strcmp((*currTransformEntry).fileName, 
                      (currImage->name())->Characters()) == 0) {
               (*currTransformEntry).transform.getMatrix(matrix);
               currImage->setWorldToImageTransform(matrix);
               printf("setting world to image transform for %s\n",
                   currImage->name()->Characters());
           }
        }
        pe->addImage(currImage);
    }

    pe->show();

    while(!timeToQuit)
      glutProcessEvents_UNC();

    return 0;
}

static int usage(char *programName)
{
  fprintf(stderr, "Usage: %s [-t transform_file] [-f image_file]+\n", 
         programName);
  exit(0);
}


static int parseArgs(int argc, char **argv)
{
  int i = 1;
  while (i < argc) {
    printf("%s, ", argv[i]);
    if (strcmp(argv[i], "-f") == 0) {
       if (++i == argc) usage(argv[0]);
       printf("%s, ", argv[i]);
       planningImageNames[numPlanningImages] = new char[strlen(argv[i])+1];
       sprintf(planningImageNames[numPlanningImages], "%s", argv[i]);
       numPlanningImages++;
    } else if (strcmp(argv[i], "-t") == 0) {
       if (++i == argc) usage(argv[0]);
       sprintf(transformFileName, "%s", argv[i]);
    } else {
       usage(argv[0]);
    }
    i++;
  }

  printf("\n");
  return 0;
}

static int loadTransformationFile(char *filename, 
                                  list<TransformFileEntry> data)
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
          sscanf(line, "%g %g %g %g", 
             &matrix[i], &matrix[i+4], &matrix[i+8], &matrix[i+12]) != 4){
         fprintf(stderr, "Error while reading transformation\n");
         entryOkay = vrpn_FALSE;
         break;
      }
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
 
static int saveTransformationFile(char *filename, 
                                  list<TransformFileEntry> data)
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
}

