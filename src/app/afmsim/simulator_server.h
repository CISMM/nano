#ifndef SIMULATOR_SERVER_H
#define SIMULATOR_SERVER_H

#include "vrpn_Connection.h"


int handle_wuit (void* , vrpn_HANDLERPARAM);

int handle_any_print (void* , vrpn_HANDLERPARAM);

int initJake (int num_x, int num_y, int port = 4500,
              const char * ipname = NULL);

int jakeMain (float scan_time_diff = .1,
              vrpn_bool isWaiting = vrpn_FALSE,
              float waitTime = 0.25f);
  ///< scan_time_diff : time to scan one scanline
  ///< isWaiting : TRUE if we're simulating latency
  ///< waitTime : amount of latency to simulate (in seconds)
  ///<   BUG - "latency" is badly broken

extern int g_isRude;
#endif
