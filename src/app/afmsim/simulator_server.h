#ifndef SIMULATOR_SERVER_H
#define SIMULATOR_SERVER_H

#include "vrpn_Connection.h"


int handle_wuit (void* , vrpn_HANDLERPARAM);

int handle_any_print (void* , vrpn_HANDLERPARAM);

// Called by nmm_Microscope Simulator in order to change the scan rate
void change_scan_rate ( float );

// Called by the simulation module to start simulator and initialize grid size
int initJake( int, int);

// Called in the main loop of the simulation code to make sure that lines
//  are scanned and the mainloop of the microscope is called
int jakeMain();

#endif
