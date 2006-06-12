

#ifndef __EDAX_DEFS_H
#define __EDAX_DEFS_H


#include <nmm_EDAX.h>

// for compatibility with the SEM part of nano
//  the EDAX is a particular hardware controller for the Hitachi
//  SEM and has a few fixed resolutions.  We must adhere to these
//  in order to fake being an SEM.  Some of these may be invalid
//  depending on the model of SPOT camera.
/*
const int EDAX_NUM_SCAN_MATRICES =(7);
extern int EDAX_SCAN_MATRIX_X[EDAX_NUM_SCAN_MATRICES];
extern int EDAX_SCAN_MATRIX_Y[EDAX_NUM_SCAN_MATRICES];
const int EDAX_DEFAULT_SCAN_MATRIX =(3);	// (512 x 400)
*/

const int OPTICAL_SERVER_DEFAULT_SCAN_MATRIX=(3);  // (512 X 400)

int resolutionToIndex(const int res_x, const int res_y);
int indexToResolution(const int id, int &res_x, int &res_y);

#define VIRTUAL_ACQ_RES_INDEX (3)

#endif //__EDAX_DEFS_H