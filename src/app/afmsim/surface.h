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


int moveTipToXYLoc( float, float, float = 1.0f);

int getImageHeightAtXYLoc( float , float , float* );

#endif



