#ifndef _SCAN_H_
#define _SCAN_H_

#include <stdlib.h>
#include "defns.h"

// raw values (normalized) from Z-buffer
extern float zBuffer[ 128*128 ];			
// array of heights: image scan data
extern double zHeight [MAX_GRID][MAX_GRID];	
extern int    scanResolution;
extern double scanStep;
extern double scanXMin;
extern double scanYMin;
extern double scanNear;
extern double scanFar;

void  doImageScanApprox( void );
void  imageScanDepthRender();
void showGrid( void );

#endif