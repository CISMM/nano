#include "nmm_EDAX.h"

int EDAX_SCAN_MATRIX_X[EDAX_NUM_SCAN_MATRICES] ={64, 128, 256, 512, 1024, 2048, 4096};
int EDAX_SCAN_MATRIX_Y[EDAX_NUM_SCAN_MATRICES] ={50, 100, 200, 400, 800,  1600, 3200};

//static 
int nmm_EDAX::resolutionToIndex(const int res_x, const int res_y)
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

//static 
int nmm_EDAX::indexToResolution(const int id, int &res_x, int &res_y)
{
  if (id < 0 || id >= EDAX_NUM_SCAN_MATRICES)
     return -1;
  res_x = EDAX_SCAN_MATRIX_X[id];
  res_y = EDAX_SCAN_MATRIX_Y[id];
  return 0;
}

//static 
void nmm_EDAX::snapIntegrationTime_nsec(int &time_nsec, vrpn_bool preferLarger)
{
  int numReads;
  numReads = time_nsec/100;
  if (numReads == 0) numReads = 1;
  if (preferLarger) {
     if (numReads*100 < time_nsec) numReads++;
  }
  time_nsec = numReads*100;
}

//static 
void nmm_EDAX::snapInterPixelDelayTime_nsec(int &time_nsec,
                                           vrpn_bool preferLarger)
{
  int num_usec;
  num_usec = time_nsec/1000;
  if (num_usec == 0) num_usec = 1;
  if (preferLarger) {
     if (num_usec*1000 < time_nsec) num_usec++;
  }
  time_nsec = num_usec*1000;
}

//static 
void nmm_EDAX::snapResolution(int &res_x, int &res_y, vrpn_bool preferLarger)
{
  int i;
  for (i = 0; i < EDAX_NUM_SCAN_MATRICES; i++){
      if (preferLarger) {
        if (EDAX_SCAN_MATRIX_X[i] > res_x && 
            EDAX_SCAN_MATRIX_Y[i] > res_y)
           break;
      } else {
        if (EDAX_SCAN_MATRIX_X[i] > res_x || 
            EDAX_SCAN_MATRIX_Y[i] > res_y)
           break;
      }
  }
  if (preferLarger) {
    if (i < EDAX_NUM_SCAN_MATRICES) {
      res_x = EDAX_SCAN_MATRIX_X[i];
      res_y = EDAX_SCAN_MATRIX_Y[i];
    } else {
      res_x = EDAX_SCAN_MATRIX_X[EDAX_NUM_SCAN_MATRICES-1];
      res_y = EDAX_SCAN_MATRIX_Y[EDAX_NUM_SCAN_MATRICES-1];
    }
  } else {
    if (i == 0) {
      res_x = EDAX_SCAN_MATRIX_X[i];
      res_y = EDAX_SCAN_MATRIX_Y[i];
    } else {
      res_x = EDAX_SCAN_MATRIX_X[i-1];
      res_y = EDAX_SCAN_MATRIX_Y[i-1];
    }
  }
}
