#ifndef SURFACE_H
#define SURFACE_H

/*****************************************************************************************
//
// This program is used in the simulation of surfaces, which are stored in a BCGrid
// The code here is adapted from the original code from the COMP145 simulator_server
// 	but is being adapted so that the simulator no longer relies on where the data
//	is coming from but uses a "surface" class to handle the data
//
*****************************************************************************************/


#ifndef SIMULATOR_SERVER_H
#include "simulator_server.h"
#endif

#include <stdio.h>
#include <iostream.h>
#include <stdlib.h>
#include <math.h>

#include <BCGrid.h>
#include <BCPlane.h>
#include <Topo.h>

void usage( const char * );

void open_image ( char * );

void get_grid_info( int , int );

int parse ( int , char ** );

int moveTipToXYLoc( float, float, float );

int getImageHeightAtXYLoc( float , float , float* );

int main ( int , char ** );

#endif



