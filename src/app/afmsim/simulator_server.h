#ifndef SIMULATOR_SERVER_H
#define SIMULATOR_SERVER_H

#include "vrpn_Connection.h"


int handle_wuit (void* , vrpn_HANDLERPARAM);

int handle_any_print (void* , vrpn_HANDLERPARAM);

int initJake (int num_x, int num_y, int port = 4500);

int jakeMain();

#endif
