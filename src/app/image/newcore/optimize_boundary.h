// optimize_boundary.h

#ifndef OPTIMIZE_BOUNDARY_H_UNC98_HWRAP 
#define OPTIMIZE_BOUNDARY_H_UNC98_HWRAP 0

#include "core_ops.h"
#include "cimage_filter.h"
#include <math.h>

void 
optimize_boundary_sites(CorePoint C, int polarity, cimage im,  
			CorePoint *B1, CorePoint *B2, float ratio);

#endif // OPTIMIZE_BOUNDARY_H_UNC98_HWRAP
