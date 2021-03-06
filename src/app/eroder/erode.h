/* The nanoManipulator and its source code have been released under the
 * Boost software license when nanoManipulator, Inc. ceased operations on
 * January 1, 2014.  At this point, the message below from 3rdTech (who
 * sublicensed from nanoManipulator, Inc.) was superceded.
 * Since that time, the code can be used according to the following
 * license.  Support for this system is now through the NIH/NIBIB
 * National Research Resource at cismm.org.

Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

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


double ** doErosion( int& row_length,double zrange,nmm_SimulatedMicroscope* nano_eroder_connection,
					double xworldratio );
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
