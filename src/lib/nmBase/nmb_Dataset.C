/*===3rdtech===
  Copyright (c) 2000 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#include "nmb_Dataset.h"

#include "BCGrid.h"
#include "BCPlane.h"
#include "nmb_String.h"

#include "Topo.h"

#include "filter.h"  // for filter_plane

#ifndef min
#define min(a,b) ((a)<(b) ? (a) : (b))
#endif

#ifndef max
#define max(a,b) ((a)>(b) ? (a) : (b))
#endif

//variables to store command line information needed to set variables
bool nmb_Dataset::doInitColorPlane = false;
bool nmb_Dataset::doInitHeight = false;
char nmb_Dataset::initHeight[256];
char nmb_Dataset::initColorPlane[256];

nmb_Dataset::nmb_Dataset
              (vrpn_bool useFileResolution, int xSize, int ySize,
               float xMin, float xMax, float yMin, float yMax,
               int readMode, const char ** gridFileNames, int numGridFiles,
               const char ** imageFileNames, int numImageFiles,
	       nmb_String * hostname,
               nmb_String * (* string_allocator) (const char *),
	       nmb_ListOfStrings * (* list_of_strings_allocator) (),
               TopoFile &topoFile):

  inputGrid (new BCGrid (xSize, ySize, xMin, xMax, yMin, yMax,
                         readMode, gridFileNames, numGridFiles,
                         topoFile)),
  inputPlaneNames (list_of_strings_allocator()),
  imageNames(list_of_strings_allocator()),
  dataImages (new nmb_ImageList(imageNames,
				imageFileNames, numImageFiles,
                                topoFile)),
  range_of_change (inputGrid),   // reference to pointer!

  alphaPlaneName (string_allocator("none")),
  colorPlaneName (string_allocator("none")),
  colorMapName (string_allocator("none")),
  contourPlaneName (string_allocator("none")),
  opacityPlaneName (string_allocator("none")),
  heightPlaneName (string_allocator("Topography-Forward")),
  //Visualization related planes
  transparentPlaneName (string_allocator("none")),
  maskPlaneName (string_allocator("none")),
  vizPlaneName (string_allocator("none")),

  done (0),
  calculatedPlane_head( NULL ),
  d_hostname(hostname)
{

	//variables to save the command line args for heightplane and colorplane
	initHeight[0] = '\0';
	initColorPlane[0] = '\0';



  ensureHeightPlane();

  // Make a friction grid from deflection data (if it exists),
  // or from an auxiliary grid
  if (inputGrid->getPlaneByName("deflection")) {
    printf("Computing friction plane from deflection.\n");
    mapInputToInputNormalized("deflection", "friction");
  } else if (inputGrid->getPlaneByName("auxiliary 0")) {
    printf("Computing friction plane from auxiliary plane.\n");
    mapInputToInputNormalized("auxiliary 0", "friction");
  }

  inputPlaneNames->addEntry("none");
  // Add every plane in the input grid list into the possible ones
  // to map to outputs
  BCPlane * p;
  p = inputGrid->head();
  while (p) {
    inputPlaneNames->addEntry(p->name()->Characters());
    p = p->next();
  }

  // in addition to static images, 
  // dataImages should include data from all microscopes so here we
  // add data images from the spm grid (actually, these are just
  // pointers into inputGrid)
  for (p = inputGrid->head(); p != NULL; p = p->next()) {
    if (dataImages->getImageByName(*(p->name())) == NULL){
    	nmb_Image *im = new nmb_ImageGrid(p);
        im->setTopoFileInfo(topoFile);
    	dataImages->addImage(im);
    }
  }

}

nmb_Dataset::~nmb_Dataset (void) {
  if (dataImages)
    delete dataImages;
  if (imageNames)
    delete imageNames;
  if (inputPlaneNames)
    delete inputPlaneNames;
  if (inputGrid)
    delete inputGrid;

  if (alphaPlaneName)
    delete alphaPlaneName;
  if (colorPlaneName)
    delete colorPlaneName;
  if (colorMapName)
    delete colorMapName;
  if (contourPlaneName)
    delete contourPlaneName;
  if (heightPlaneName)
    delete heightPlaneName;
}


/**
   Loads a list of files, by calling BCGrid::loadFiles, then
   adding any new planes to our dataImages list. 
   @return -1 on invalid data, -2 on conflict with existing grid data, 0 on success
   @author Aron Helser
   @date modified 3-22-00 Aron Helser
*/
int
nmb_Dataset::loadFiles(const char** file_names, int num_files, 
		       TopoFile &topoFile)
{
  // Load the files
    int ret;
    if ((ret = inputGrid->loadFiles(file_names, num_files, topoFile)) != 0) {
        return ret;
    }
  
  // Add any new planes to our lists. 
  for (BCPlane *p = inputGrid->head(); p != NULL; p = p->next()) {
    if (dataImages->getImageByName(*(p->name())) == NULL){
      nmb_Image *im = new nmb_ImageGrid(p);
      im->setTopoFileInfo(topoFile);
      dataImages->addImage(im);
    }
  }
  return 0;
}

int
nmb_Dataset::addImageToGrid(nmb_ImageGrid * new_image) 
{
    BCPlane *newplane;
    BCString name;
    inputGrid->findUniquePlaneName(*(new_image->name()),&name);
    newplane = inputGrid->addPlaneCopy(new_image->plane);
    newplane->rename(name);
    inputPlaneNames->addEntry(name);
    dataImages->addImage(new nmb_ImageGrid(newplane));

    return 0;
}

int nmb_Dataset::setGridSize(int x, int y)
{
    return (inputGrid->setGridSize(x,y));
}

BCPlane * nmb_Dataset::ensureHeightPlane (void) {

  BCPlane * plane;

  // Look for a plane within the grid that is in nanometers.
  // add to the grid a height plane, if there isn't one.
  plane = inputGrid->head();
  while (plane) {
    if (!strcmp(*plane->units(),"nm")) {
      heightPlaneName->Set(plane->name()->Characters());
      // This line is EVIL - TCH 14 Jan 00
      //*heightPlaneName = nmb_Selector(NULL, (plane->name()->Characters()));
      break;  // Found one!
    }
    plane = plane->next();
  }
  if (plane == NULL) {
    plane = inputGrid->getPlaneByName(EMPTY_PLANE_NAME);
    if (!plane) {
        //fprintf(stderr,"Warning! No height plane input, using zero plane\n");
      plane = inputGrid->addNewPlane(EMPTY_PLANE_NAME, "nm", NOT_TIMED);
      heightPlaneName->Set(plane->name()->Characters());
      dataImages->addImage(new nmb_ImageGrid(plane));
    }
  }
  return plane;
}


int nmb_Dataset::computeFilteredPlane
                             (const char * outputPlane,
                              const char * inputPlane,
                              const char * filterPath,
                              const char * filterName,
                              float scale, float angle,
                              const char * filterParameters) {
  char fullPath [256];
  int ret;

  // Construct the path to the filter program
  strncpy(fullPath, filterPath, sizeof(fullPath));
  if ((strlen(fullPath) + strlen(filterName) + 2) > sizeof(fullPath)) {
    fprintf(stderr, "nmb_Dataset::computeFilteredPlane:  "
                    "Path too long.\n");
    return -1;
  }

  strcat(fullPath, "/");
  strcat(fullPath, filterName);

  ret = filter_plane(fullPath,
                     inputGrid->getPlaneByName(inputPlane), outputPlane,
                     scale, angle, filterParameters, inputGrid);

  return ret;
}

int nmb_Dataset::computeAdhesionFromDeflection
                             (const char * outputPlane,
                              const char * firstInputPlane,
                              const char * lastInputPlane,
                              int numberToAverage) {

  BCPlane *def[512];      // List of deflection planes
  BCPlane *adhesion;      // Adhesion plane
  int     first_def, last_def, num_def;   // Numbers of deflection list
  int     i;
  char    num[10];
  BCString newname;
  int     x,y, plane;
  char    *ffl_in_first, *ffl_in_last;
  char    basename[1000];

  //
  // Make sure that the base name for the adhesion mapping is
  // "*.ffl" and figure out the number of the first and last
  // ones we are looking for.
  //
  ffl_in_first = max(strstr(firstInputPlane, ".ffl"),
                     strstr(firstInputPlane, ".FFL"));
  ffl_in_last = max(strstr(lastInputPlane, ".ffl"),
                    strstr(lastInputPlane, ".FFL"));
  if (!ffl_in_first || !ffl_in_last) {
          fprintf(stderr,"computeAdhesionFromDeflection:  "
                         "Can only use deflection planes, not %s.\n",
                  firstInputPlane);
          return -1;
  }
  first_def = max(1, atoi(ffl_in_first + 4));
  last_def = atoi(ffl_in_last + 4);
  num_def = last_def - first_def + 1;

  //
  // Make sure we have a positive number of planes to average
  //
  if (numberToAverage < 1) numberToAverage = 1;

  //
  // Make sure there are more planes than we are going to average.
  //
  if (num_def < (numberToAverage + 2)) {
          fprintf(stderr,"compute_adhesion_from_deflection():  "
                         "Requires at least %d planes, got %d\n",
                         numberToAverage + 2, num_def);
          return -1;
  }
  if (num_def > 512) {
          fprintf(stderr,"compute_adhesion_from_deflection(): Can only do 512 deflection planes, got %d\n",num_def);
          return -1;
  }
  //
  // Look up all of the deflection planes.  Recall that the first
  // plane in a series is "*.ffl", then "*.ffl2" and up.
  //
  if ( (def[0] = inputGrid->getPlaneByName(firstInputPlane)) == NULL ) {
          fprintf(stderr,
              "compute_adhesion_from_deflection(): No %s\n",firstInputPlane);
          return -1;      // No planes, so cant make it
  }
  for (i = first_def+1; i <= last_def; i++) {
    strncpy(basename, firstInputPlane, (ffl_in_first - firstInputPlane)+4);
    basename[(ffl_in_first - firstInputPlane)+4] = '\0';
    sprintf(num,"%d",i);
    newname = basename;
    newname += num;
    def[i-first_def] = inputGrid->getPlaneByName(newname);
    if (def[i-first_def] == NULL) {
            fprintf(stderr, "computeAdhesionFromDeflection:  "
                            "No %s.\n", newname.Characters());
            return -1;
    }
  };

  //
  // Create an adhesion plane, if it doesnt already exist
  //
  adhesion = inputGrid->getPlaneByName(outputPlane);
  if (!adhesion){
    adhesion = inputGrid->addNewPlane(outputPlane, "nanoamps", NOT_TIMED);
    if (adhesion){
        dataImages->addImage(new nmb_ImageGrid(adhesion));
    }
  }
  if (!adhesion) {
    fprintf(stderr,
      "computeAdhesionFromDeflection:  Can't make plane.\n");
    return -1;
  }

  //
  // Fill the adhesion plane with the difference between the minimum
  // value at any point in the planes to the
  // average of the last numberToAverage values.
  //
  for (x = 0; x < inputGrid->numX(); x++) {
   for (y = 0; y < inputGrid->numY(); y++) {
      double minval = def[0]->value(x,y);
      double avgval;
      for (plane = 0; plane < num_def; plane++) {
          minval = min(minval, def[plane]->value(x,y));
      }
      avgval = 0;
      for (plane = num_def - 1; plane > num_def - numberToAverage;
           plane--)
        avgval += def[plane]->value(x, y);
      avgval /= numberToAverage;
      adhesion->setValue(x, y, avgval - minval);
   }
  }

  adhesion->setMinAttainableValue(adhesion->minValue());
  adhesion->setMaxAttainableValue(adhesion->maxValue());

  return 0;
}



// adds a new plane to the list of calculated planes
void nmb_Dataset::
addNewCalculatedPlane( nmb_CalculatedPlane* plane )
{
  if( plane == NULL )
    {
      fprintf( stderr, "nmb_Dataset::addNewCalculatedPlane:  "
	       "null plane.  Sorry.\n" );
      return;
    }

  nmb_CalculatedPlaneNode* node = new nmb_CalculatedPlaneNode;
  if( !node ) 
    {
      fprintf(stderr, "nmb_Dataset::addNewCalculatedPlane:  "
	      "Out of memory.\n");
      return;
    }
  node->data = plane;
  node->next = calculatedPlane_head;
  calculatedPlane_head = node;

  inputPlaneNames->addEntry( plane->getName()->Characters() );

} // end addNewCalculatedPlane( ... )




/** Map a plane in the input grid into a plane in the output grid, normalizing
 * the values in the output plane.  Ensure that the output plane does not
 * already exist.  This is used to map adhesion or auxilliary planes into
 * color parameter or friction. */
int     nmb_Dataset::mapInputToInputNormalized (const char * in_name,
                                                const char * out_name)
{
        BCPlane * comp, * color;
        double  minval, maxval, offset, scale;
        int     x,y;

        //
        // Make sure the mapping works
        //
        if ( (inputGrid->numX() != inputGrid->numX()) ||
             (inputGrid->numY() != inputGrid->numY()) ) {
                fprintf(stderr,
                        "mapInputToInputNormalized(): Unmatched grids\n");
                return -1;
        }

        //
        // Find the input plane
        //
        comp = inputGrid->getPlaneByName(in_name);
        if ( comp == NULL ) {
                fprintf(stderr, "mapInputToInputNormalized:  "
                        "No %s plane in input grid\n", in_name);
                return -1;
        }

        //
        // Create an output plane
        //
        color = inputGrid->getPlaneByName(out_name);
        if (color != NULL) {
                fprintf(stderr,"mapInputToInputNormalized(): %s exists!\n",
                        out_name);
                return -1;
        }
        color = inputGrid->addNewPlane(out_name,"normalized",NOT_TIMED);
        if (color == NULL) {
                fprintf(stderr,
                        "mapInputToInputNormalized(): Can't make %s!\n",
                        out_name);
                return -1;
        }
        nmb_Image *input_im = dataImages->getImageByName(in_name);
        nmb_Image *output_im = new nmb_ImageGrid(color);
        TopoFile tf;
        if (input_im) {
            input_im->getTopoFileInfo(tf);
            output_im->setTopoFileInfo(tf);
        } else {
            fprintf(stderr, "Warning: image not in list\n");
        }
	dataImages->addImage(output_im);

        //
        // Map the input to output, normalizing it
        //
        minval = comp->minValue();
        maxval = comp->maxValue();
        offset = minval;
        scale = 1.0 / (maxval - minval);
        for (x = 0; x < inputGrid->numX(); x++) {
         for (y = 0; y < inputGrid->numY(); y++) {
                color->setValue(x,y, (comp->value(x,y)-offset)*scale);
         }
        }

        return 0;
}


float nmb_Dataset::getFirstLineAvg(BCPlane * plane)
{
    float avgVal = 0;
    if (!plane) return 0;
    for (int i = 0; i < plane->numX(); i++) {
	avgVal += plane->value(i, plane->numY()-1);
    }
    avgVal /= plane->numX();
    //printf("Found line average %g\n", avgVal);
    return avgVal;

}


const char*
nmb_Dataset::getHostname( ) const
{
  return d_hostname->string( );
}
