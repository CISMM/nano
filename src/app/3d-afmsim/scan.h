#ifndef _SCAN_H_
#define _SCAN_H_

#include <stdlib.h>
#include "defns.h"


// raw values (normalized) from Z-buffer
extern float zBuffer[ DEPTHSIZE*DEPTHSIZE ];//***			
// array of heights: image scan data
extern double zHeight [MAX_GRID][MAX_GRID];	
extern float colorBuffer[ DEPTHSIZE*DEPTHSIZE ];//***			
extern int    scanResolution;
extern double scanStep;
extern double scanXMin;
extern double scanYMin;
extern double scanNear;
extern double scanFar;


double ** doImageScanApprox( int& row_col_length );
void  imageScanDepthRender();
void showGrid( void );
double find_volume();

#endif
