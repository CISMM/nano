
#include "edax_defs.h"

int EDAX_SCAN_MATRIX_X[EDAX_NUM_SCAN_MATRICES] ={64, 128, 256, 512, 1024, 2048, 4096};
int EDAX_SCAN_MATRIX_Y[EDAX_NUM_SCAN_MATRICES] ={50, 100, 200, 400, 800,  1600, 3200};

int resolutionToIndex(const int res_x, const int res_y)
{
    int i;
    for (i = 0; i < EDAX_NUM_SCAN_MATRICES; i++){
        if (EDAX_SCAN_MATRIX_X[i] == res_x &&
            EDAX_SCAN_MATRIX_Y[i] == res_y) {
            return i;
        }
    }
    return -1;
}



int indexToResolution(const int id, int &res_x, int &res_y)
{
  if (id < 0 || id >= EDAX_NUM_SCAN_MATRICES)
     return -1;
  res_x = EDAX_SCAN_MATRIX_X[id];
  res_y = EDAX_SCAN_MATRIX_Y[id];
  return 0;
}

