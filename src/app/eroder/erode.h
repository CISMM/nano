#ifndef _ERODE_H_
#define _ERODE_H_

#include <stdlib.h>
#include "defns.h"
#include <nmm_SimulatedMicroscope.h>

// raw values (normalized) from Z-buffer
/*extern float zBuffer[ DEPTHSIZE*DEPTHSIZE ];//***			
// array of heights: image scan data
extern double zHeight [MAX_GRID][MAX_GRID];	
extern float colorBuffer[ DEPTHSIZE*DEPTHSIZE ];//***
*/
extern float* zBuffer;
extern float* colorBuffer;
extern double** zHeight;
			
extern int    xResolution, yResolution;
extern double Step;
extern double XMin;
extern double YMin;
extern double Near;
extern double Far;


double ** doErosion( int& row_length,double zrange,nmm_SimulatedMicroscope* nano_eroder_connection );
void  imageErosionRender();
void changeBufferSize();
void showGrid( void );
double find_volume(double & avgHeight, double & maxHeight,double& area);
bool load_simulator_data(char* file);//returns true if incoming data has different
									 //size than default
void invert_zHeight_values(double xworldratio);
bool fillArray(nmm_SimulatedMicroscope* nano_eroder_connection);
	//fills in MicroscopeHeightArray[][] with data from scan in nano
void print_to_file(ostream& outstream, float value, char* spacing_type);

#endif
