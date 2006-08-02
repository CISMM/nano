#ifndef _MICROSCOPE_FLAVORS_
#define _MICROSCOPE_FLAVORS_

// Include the definitions and code needed for each flavor of microscope.
#include "Topo.h"
#include "Asylum.h"

// Create a global variable that stores the currently-active type of microscope.
// It will have a default value of Topometrix for now, but can be set by a
// program from the command-line arguments as well.
typedef enum { Topometrix, Asylum } nmb_MICROSCOPE_FLAVORS;
extern nmb_MICROSCOPE_FLAVORS nmb_MicroscopeFlavor;

#endif
