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


/**
Stores data needed to compute a new plane from two others.  The
formula used is:  output_plane = first_plane + (second_plane * scale);
To do the subtraction of two planes (make this the default), the scale
should be set to -1.
*/
struct sum_data {
    BCPlane * sum_plane;
    BCPlane * first_plane;
    BCPlane * second_plane;
    double  scale;
};

struct sum_node{
    sum_data * data;
    sum_node * next;
};

struct lblflatten_data {
    BCPlane * lblflat_plane;
    BCPlane * from_plane;
    float firstLineAvgVal;
};

struct lblflatten_node {
    lblflatten_data * data;
    lblflatten_node * next;
};


nmb_Dataset::nmb_Dataset
              (vrpn_bool useFileResolution, int xSize, int ySize,
               float xMin, float xMax, float yMin, float yMax,
               int readMode, const char ** gridFileNames, int numGridFiles,
               const char ** imageFileNames, int numImageFiles,
	       const char * hostname,
               nmb_String * (* string_allocator) (const char *),
	       nmb_ListOfStrings * (* list_of_strings_allocator) (),
               TopoFile &topoFile):

  inputGrid (new BCGrid (xSize, ySize, xMin, xMax, yMin, yMax,
                         readMode, gridFileNames, numGridFiles,
                         topoFile)),
  imageNames(list_of_strings_allocator()),
  dataImages (new nmb_ImageList(imageNames,
				imageFileNames, numImageFiles,
                                topoFile)),
  range_of_change (inputGrid),   // reference to pointer!

  transparentPlaneName (string_allocator("none")),
  maskPlaneName (string_allocator("none")),
  alphaPlaneName (string_allocator("none")),
  colorPlaneName (string_allocator("none")),
  colorMapName (string_allocator("none")),
  contourPlaneName (string_allocator("none")),
  opacityPlaneName (string_allocator("none")),
  heightPlaneName (string_allocator("Topography-Forward")),

  done (0),

  d_flat_list_head(NULL),
  d_lblflat_list_head(NULL),
  d_sum_list_head(NULL),
  d_flatPlaneCB (NULL),
  d_hostname(NULL)

{
  //BCPlane * std_dev_plane;

  int i;
  // files not loaded as grid files should not be height fields by default
  for (i = 0; i < dataImages->numImages(); i++) {
      dataImages->getImage(i)->setHeightField(vrpn_FALSE);
  }
//    if (inputGrid->empty())
//      fprintf(stderr, "nmb_Dataset:  Cannot scan grid.\n");

//    if (readMode == READ_FILE) {
//        if ((!useFileResolution) &&
//            ((inputGrid->numX() > xSize) || (inputGrid->numY() > ySize))) {
//                printf("Decimating input grid to %d x %d...\n", xSize, ySize);
//                inputGrid->decimate(xSize, ySize);
//        } else {
//            printf("Not decimating input grid, resolution is %d x %d\n",
//                inputGrid->numX(), inputGrid->numY());
//        }
//    }

  ensureHeightPlane();

  //if ((readMode == READ_DEVICE) || (readMode == READ_STREAM))
    //std_dev_plane = inputGrid->addNewPlane("std_dev", "nm", TIMED);

  // Make a friction grid from deflection data (if it exists),
  // or from an auxiliary grid
  if (inputGrid->getPlaneByName("deflection")) {
    printf("Computing friction plane from deflection.\n");
    mapInputToInputNormalized("deflection", "friction");
  } else if (inputGrid->getPlaneByName("auxiliary 0")) {
    printf("Computing friction plane from auxiliary plane.\n");
    mapInputToInputNormalized("auxiliary 0", "friction");
  }

  // in addition to static images, 
  // dataImages should include data from all microscopes so here we
  // add data images from the spm grid (actually, these are just
  // pointers into inputGrid)
  for (BCPlane *p = inputGrid->head(); p != NULL; p = p->next()) {
    if (dataImages->getImageByName(*(p->name())) == NULL){
    	nmb_Image *im = new nmb_ImageGrid(p);
        im->setTopoFileInfo(topoFile);
    	dataImages->addImage(im);
    }
  }

  if (hostname) {
      d_hostname = new char [strlen(hostname) +1];
      strcpy(d_hostname, hostname);
  }
}

nmb_Dataset::~nmb_Dataset (void) {
  if (dataImages)
    delete dataImages;
  if (imageNames)
    delete imageNames;
  if (inputGrid)
    delete inputGrid;
  if (d_hostname)
      delete [] d_hostname;

  // XXX Clean up computed planes lists
  // Clean up flat plane callback list.

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


int nmb_Dataset::computeSumPlane (const char * outputPlane,
                                  const char * firstInputPlane,
                                  const char * secondInputPlane,
                                  double scale)
{
        BCPlane * inplanes [2];   // Two input planes
        BCPlane * outplane;      // Output plane
        int     x,y;
        sum_node *sum_ptr, *pre;
        static  sum_node * d_sum_list_head=NULL;


        //Check if the outplane has same name with one of the
        //planes. This should be avoided because of the callbacks.

        if(!strcmp(outputPlane, firstInputPlane) ||
           !strcmp(outputPlane, secondInputPlane)){
           fprintf(stderr,
                  "compute_sum_plane(): can not add from itself\n");
           return -1;
        }

        //
        // Look up the input planes.
        //
        inplanes[0] = inputGrid->getPlaneByName(firstInputPlane);
        if (!inplanes[0]) {
                fprintf(stderr, "compute_sum_plane(): No %s\n",
                        firstInputPlane);
                return -1;      // No plane, so cant make it
        }
        inplanes[1] = inputGrid->getPlaneByName(secondInputPlane);
        if (!inplanes[1]) {
                fprintf(stderr, "compute_sum_plane(): No %s\n",
                        secondInputPlane);
                return -1;      // No plane, so cant make it
        }

        //
        // Ensure that the units match on the input planes.
        // Actually, only warn if they are not the same
        //
        if ( strcmp(inplanes[0]->units()->Characters(),
                    inplanes[1]->units()->Characters()) != 0) {
                fprintf(stderr, "compute_sum_plane(): Unit mismatch\n");
                fprintf(stderr, "    (%s vs. %s)\n",
                        inplanes[0]->units()->Characters(),
                        inplanes[1]->units()->Characters());
        }

        //
        // Create sum plane, if it does not yet exist.  Use the units for
        // the first plane.  It is assumed that the second will have been
        // scaled appropriately.
        //
        if ( (outplane = inputGrid->getPlaneByName(outputPlane)) == NULL) {
            outplane = inputGrid->addNewPlane(outputPlane,
                       inplanes[0]->units()->Characters(), NOT_TIMED);
            if (outplane == NULL) {
                fprintf(stderr,
                  "compute_sum_plane(): Can not make plane %s\n",outputPlane);
                return -1;
            }
            TopoFile tf;
            nmb_Image *im = dataImages->getImageByName(firstInputPlane);
            nmb_Image *output_im = new nmb_ImageGrid(outplane);
            if (im) {
                im->getTopoFileInfo(tf);
                output_im->setTopoFileInfo(tf);
            }
            dataImages->addImage(output_im);
        }
        else {  //the output plane already exist
            pre=sum_ptr=d_sum_list_head;
            while(sum_ptr) {
                if(sum_ptr->data->sum_plane == outplane) {
                     sum_ptr->data->first_plane->remove_callback
                            (updateSumOnPlaneChange,(void *)sum_ptr);
                     sum_ptr->data->second_plane->remove_callback
                            (updateSumOnPlaneChange, (void *)sum_ptr);
                     pre->next = sum_ptr->next;
                     free(sum_ptr->data);
                     free(sum_ptr);
                     break;
                 }
                 else {
                     pre = sum_ptr;
                     sum_ptr=sum_ptr->next;
                 }
             }
        }

        //
        // Fill the sum plane with the weighted sum between the two planes
        // at each point.
        //
        for (x = 0; x < inputGrid->numX(); x++) {
         for (y = 0; y < inputGrid->numY(); y++) {
            outplane->setValue(x,y,
                      inplanes[0]->value(x,y) + scale*inplanes[1]->value(x,y));
         }
        }

        outplane->setMinAttainableValue(outplane->minValue());
        outplane->setMaxAttainableValue(outplane->maxValue());

        sum_data * sum_struct = new sum_data;
        if (sum_struct == NULL) {
                fprintf(stderr,"compute_sum_plane(): Out of memory!\n");
                return -1;
        }

        // Fill in the structure that will be used by the callback
        // when new data arrives for one of the planes.
        sum_struct->first_plane = inplanes[0];
        sum_struct->second_plane = inplanes[1];
        sum_struct->sum_plane = outplane;
        sum_struct->scale = scale;

        sum_node *ptr = new sum_node;
        if (ptr == NULL) {
                fprintf(stderr,"compute_sum_plane(): Out of memory!\n");
                return -1;
        }

        ptr->data = sum_struct;

        //insert the new node as the head of the list
        ptr->next=d_sum_list_head;
        d_sum_list_head=ptr;

        inplanes[0]->add_callback(updateSumOnPlaneChange, sum_struct);

        inplanes[1]->add_callback(updateSumOnPlaneChange, sum_struct);
        return 0;
}

//---------------------------------------------------------------------------
/**
Compute a new plane that is a flattening of an height plane according to
the positions of three measure lines.
After the flattening, the intersections of three measure lines to the
surface have the same z value.
@warning XXX When we implement dynamic updates based on scan data, we need
    hook up callbacks for the two planes feeding into this.
@return NULL on failure.
*/
BCPlane* nmb_Dataset::computeFlattenedPlane
                        (const char * outputPlane,
                         const char * inputPlane,
                         float redX, float greenX, float blueX,
                         float redY, float greenY, float blueY) {

  BCPlane * outplane;      // Output plane
  double  offset, dx, dy;
  double  z1, z2, z3;
  double  x1, x2, x3, y1, y2, y3;
  flatten_data flatten_struct;

  if(strcmp(outputPlane, inputPlane)==0) {
     fprintf(stderr,
            "compute_flattened_plane(): can not flatten from itself\n");
     return NULL;
  }

  BCPlane * plane = inputGrid->getPlaneByName(inputPlane);
  if (plane == NULL)
  {
      fprintf(stderr,
             "compute_flattened_plane(): could not get input plane!\n");
      return NULL;
  }

  x1 = plane->xInGrid(redX);
  x2 = plane->xInGrid(greenX);
  x3 = plane->xInGrid(blueX);

  y1 = plane->yInGrid(redY);
  y2 = plane->yInGrid(greenY);
  y3 = plane->yInGrid(blueY);

  if (( plane->valueAt(&z1, redX, redY)) ||
      ( plane->valueAt(&z2, greenX, greenY))||
      ( plane->valueAt(&z3, blueX, blueY))) {
      fprintf(stderr,"compute_flattened_plane(): "
              "measure lines out of bounds.\n");
      return NULL;
  }
  if (x3 == x1) {
      // These two points are co-linear in a bad way - swap point 2 and point 3
      x3 = plane->xInGrid(greenX);
      x2 = plane->xInGrid(blueX);

      y3 = plane->yInGrid(greenY);
      y2 = plane->yInGrid(blueY);
      plane->valueAt(&z3, greenX, greenY);
      plane->valueAt(&z2, blueX, blueY);
  }
  
  //solve dx,dy for
  // z3-z1= dx(x3-x1) + dy(y3-y1)
  // z2-z1= dx(x2-x1) + dy(y2-y1)

  if (x3 == x1) {
      // These points are also co-linear in a bad way - abort.
      fprintf(stderr,"compute_flattened_plane(): overlapping points.\n");
      return NULL;
  }
  double k;
  k = (x2 - x1) / (x3 - x1);

  //test if those points are collinear
  if( ( (y2-y1)*(x3-x1)+ (y1-y3)*(x2-x1) )== 0) {
       fprintf(stderr,"compute_flattened_plane(): collinear points.\n");
       return NULL;
  }

  dy = (z2 - z1 + (z1 - z3) * k) / (y2 - y1 + (y1 - y3) * k);
  dx = (z3 - z1 - dy * (y3 - y1)) / (x3 - x1);
  offset = dx * inputGrid->numX() / 2 + dy * inputGrid->numY() / 2;

  // Add the host name to the plane name so we can distinguish
  // where the plane came from
  char new_outputPlane[256];
#if 0
  if (d_hostname) {
      sprintf(new_outputPlane, "%s from %s", outputPlane, d_hostname);
  } else {
      sprintf(new_outputPlane, "%s from local", outputPlane);
  }
#else
  // XXX 3rdTech only - no weird plane names.
  sprintf(new_outputPlane, "%s", outputPlane);
#endif

  computeFlattenedPlane(new_outputPlane, inputPlane, dx, dy, offset);

  outplane = inputGrid->getPlaneByName(new_outputPlane);

  flatten_struct.dx = dx;
  flatten_struct.dy = dy;
  flatten_struct.offset = offset;
  flatten_struct.from_plane = plane;
  flatten_struct.flat_plane = outplane;

  newFlatPlaneCB * st;

  for (st = d_flatPlaneCB; st; st = st->next) {
    (*st->cb)(st->userdata, &flatten_struct);
  }

  return outplane;
}

int nmb_Dataset::computeFlattenedPlane
                        (const char * outputPlane,
                         const char * inputPlane,
                         double dx, double dy, double offset) {

  BCPlane * outplane;      // Output plane
  int     x, y;
  flatten_node * flat_ptr, * pre;

  if(strcmp(outputPlane, inputPlane)==0) {
     fprintf(stderr,
            "compute_flattened_plane(): can not flatten from itself\n");
     return -1;
  }

  BCPlane * plane = inputGrid->getPlaneByName(inputPlane);
  if (plane == NULL)
  {
      fprintf(stderr,
             "compute_flattened_plane(): could not get height plane!\n");
      return -1;
  }


  // Create output plane, if it does not yet exist
  outplane = inputGrid->getPlaneByName(outputPlane);
  if (outplane == NULL) {
      char        newunits [1000];
      sprintf(newunits, "%s_flat", plane->units()->Characters());
      outplane = inputGrid->addNewPlane(outputPlane, newunits, NOT_TIMED);
      if (outplane == NULL) {
          fprintf(stderr,
            "compute_flattened_plane(): Can't make plane %s\n",outputPlane);
          return -1;
      }
      TopoFile tf;
      nmb_Image *im = dataImages->getImageByName(inputPlane);
      nmb_Image *output_im = new nmb_ImageGrid(outplane);
      if (im) {
          im->getTopoFileInfo(tf);
          output_im->setTopoFileInfo(tf);
      } else {
          fprintf(stderr, "nmb_Dataset: Warning, input image not in list\n");
      }
      dataImages->addImage(output_im);
  } else {         //the output plane already exist
      pre = flat_ptr = d_flat_list_head;
      while (flat_ptr) {
         if ( flat_ptr->data->flat_plane == outplane ) {
	     // userdata must be the same as when we added the callback!
            flat_ptr->data->from_plane->remove_callback
              (updateFlattenOnPlaneChange, (void *) (flat_ptr->data));
            pre->next = flat_ptr->next;
            free(flat_ptr->data);
            free(flat_ptr);
            break;
         }
         else {
            pre = flat_ptr;
            flat_ptr = flat_ptr->next;
         }
     }
  }

  //
  // Fill the output plane with the flattened plane (shear applied
  // at each point).
  //
  for (x = 0; x < inputGrid->numX(); x++) {
    for (y = 0; y < inputGrid->numY(); y++) {
      outplane->setValue(x, y,
                         plane->value(x, y) + offset - dx * x - dy * y);
    }
  }

  fprintf(stderr, "Flattening: dx=%g, dy=%g, offset=%g\n", dx, dy, offset);
  outplane->setMinAttainableValue(plane->minAttainableValue());
  outplane->setMaxAttainableValue(plane->maxAttainableValue());
  // I think this OK. Except for static file, mxxAttainableValue is from
  // the range of the scanner, not the data range itself. 

  flatten_data  *flatten_struct = new flatten_data;
  if (flatten_struct == NULL) {
    fprintf(stderr,"compute_flattened_plane(): Out of memory!\n");
    return -1;
  }

  flatten_struct->dx = dx;
  flatten_struct->dy = dy;
  flatten_struct->offset = offset;
  flatten_struct->from_plane = plane;
  flatten_struct->flat_plane = outplane;

  flatten_node * ptr = new flatten_node;
  if (ptr == NULL) {
    fprintf(stderr,"compute_flattened_plane(): Out of memory!\n");
    return -1;
  }

  ptr->data = flatten_struct;

  //insert the new node as the head of the list
  ptr->next = d_flat_list_head;
  d_flat_list_head = ptr;

  plane->add_callback(updateFlattenOnPlaneChange, flatten_struct);

  return 0;
}

void nmb_Dataset::registerFlatPlaneCallback (void * userdata,
                  void (* cb) (void *, const flatten_data *)) {
  newFlatPlaneCB * st = new newFlatPlaneCB;

  if (!st) {
    fprintf(stderr, "nmb_Dataset::registerFlatPlaneCallback:  "
                    "Out of memory.\n");
    return;
  }
  st->userdata = userdata;
  st->cb = cb;
  st->next = d_flatPlaneCB;
  d_flatPlaneCB = st;
}


int nmb_Dataset::computeLBLFlattenedPlane (const char * outputPlane,
					   const char * inputPlane)
{

  BCPlane * outplane;      // Output plane
  int     x, y;
  lblflatten_node * lblflat_ptr, * pre;  
  float avgVal = 0;  //average height value of the current scan line
  float firstAvgVal = 0;  //average height value of the 1st scan line
  float diff = 0;  //difference between firstAvgVal and avgVal

//This is necessary because of the callbacks.
  if(strcmp(outputPlane, inputPlane)==0) {
	fprintf(stderr, "computeLBLFlattenedPlane(): can not line-by-line flatten from itself\n");
	return -1;
  } //end if

  BCPlane * plane = inputGrid->getPlaneByName(inputPlane);
  if (plane == NULL) {
	fprintf(stderr, "computeLBLFlattenedPlane(): could not get height plane!\n");
	return -1;
  } //end if

  //Create output plane, if it does not yet exist
  outplane = inputGrid->getPlaneByName(outputPlane);
  if (outplane == NULL) {
	char        newunits [1000];
	sprintf(newunits, "%s_flat", plane->units()->Characters());
	outplane = inputGrid->addNewPlane(outputPlane, newunits, NOT_TIMED);
	if (outplane == NULL) {
	    fprintf(stderr, "computeLBLFlattenedPlane():"
                  " Can't make plane %s\n", outputPlane);
	    return -1;
	} //end if
        nmb_Image *im = dataImages->getImageByName(inputPlane);
        nmb_Image *output_im = new nmb_ImageGrid(outplane);
        TopoFile tf;
        if (im) {
            im->getTopoFileInfo(tf);
            output_im->setTopoFileInfo(tf);
        } else {
            fprintf(stderr, "Warning: image not in list\n");
        }
        dataImages->addImage(output_im);
  } //end if
  else {    //the output plane already exists (I don't know why you'd
	    //output to a plane that already exists.)
	pre = lblflat_ptr = d_lblflat_list_head;
	while (lblflat_ptr) {
		if (lblflat_ptr->data->lblflat_plane == outplane) {
			lblflat_ptr->data->from_plane->remove_callback
			  (updateLBLFlattenOnPlaneChange, (void *)lblflat_ptr);
			pre->next = lblflat_ptr->next;
			free(lblflat_ptr->data);
			free(lblflat_ptr);
			break;
		} //end if
		else {
			pre = lblflat_ptr;
			lblflat_ptr = lblflat_ptr->next;
		} //end else
	} //end while
  } //end else

  //Fill the output plane with the line-by-line flattened plane

  //First, find the average height value of the 1st 2 scan lines and copy the
  //first scan line (unchanged) to the output plane
  for (x = 0; x < inputGrid->numX(); x++) {
	outplane->setValue(x, 0, plane->value(x, 0));
	firstAvgVal += plane->value(x, 0);
	avgVal += plane->value(x, 1);
  } //end for

  firstAvgVal = firstAvgVal / inputGrid->numX();
  avgVal = avgVal / inputGrid->numX();
  diff = firstAvgVal - avgVal;

  //Compute average height value of current line, and flatten the previous
  //scan line before writing it (prev. line) to the output plane
  for (y = 2; y < inputGrid->numY(); y++) {
	avgVal = 0;
	for (x = 0; x < inputGrid->numX(); x++) {
		avgVal += plane->value(x, y);
		outplane->setValue(x, y - 1, plane->value(x, y - 1) + diff); 
	} //end for
	avgVal = avgVal / inputGrid->numX();
	diff = firstAvgVal - avgVal;
  } //end for

  //flatten and output the last scan line
  for (x = 0; x < inputGrid->numX(); x++) {
  	outplane->setValue(x, inputGrid->numY() - 1,
			   plane->value(x, inputGrid->numY() - 1) + diff);
  } //end for

  outplane->setMinAttainableValue(plane->minAttainableValue());
  outplane->setMaxAttainableValue(plane->maxAttainableValue());

  lblflatten_data *lblflatten_struct = new lblflatten_data;
  if (lblflatten_struct == NULL) {
	fprintf(stderr, "computeLBLFlattenedPlane(): Out of memory!\n");
	return -1;
  } //end if

  lblflatten_struct->lblflat_plane = outplane;
  lblflatten_struct->from_plane = plane;
  lblflatten_struct->firstLineAvgVal = firstAvgVal;

  lblflatten_node *ptr = new lblflatten_node;
  if (ptr == NULL) {
	fprintf(stderr, "computeLBLFlattenedPlane(): Out of memory!\n");
	return -1;
  } //end if

  ptr->data = lblflatten_struct;

  //insert the new node as the head of the list
  ptr->next = d_lblflat_list_head;
  d_lblflat_list_head = ptr;

  plane->add_callback(updateLBLFlattenOnPlaneChange, lblflatten_struct);
  return 0;

} //end computeLBLFlattenedPlane


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


// static
void nmb_Dataset::updateFlattenOnPlaneChange (BCPlane *, int x, int y,
                                              void * userdata) {
  flatten_data * data = (flatten_data *) userdata;

  data->flat_plane->setValue(x, y,
                             data->from_plane->value(x, y) +
                             data->offset - data->dx * x - data->dy * y);
}

// static
void nmb_Dataset::updateSumOnPlaneChange (BCPlane *, int x, int y,
                                          void * userdata) {
  sum_data * data = (sum_data *) userdata;

  data->sum_plane->setValue(x, y,
                            data->first_plane->value(x, y) +
                            data->second_plane->value(x, y) * data->scale);
}

void nmb_Dataset::updateLBLFlattenOnPlaneChange (BCPlane *, int /*x*/, int y,
						 void * userdata) {
  lblflatten_data * data = (lblflatten_data *) userdata;
  float avgVal = 0;
  float diff;
  int i;

  for (i = 0; i < data->from_plane->numX(); i++) {
	avgVal += data->from_plane->value(i, y);
  }
  avgVal = avgVal / data->from_plane->numX();
  diff = data->firstLineAvgVal - avgVal; 
  for (i = 0; i < data->from_plane->numX(); i++) {
	data->lblflat_plane->setValue(i, y, data->from_plane->value(i, y) + diff);
  } //end for
} //end updateLBLFlattenOnPlaneChange


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
