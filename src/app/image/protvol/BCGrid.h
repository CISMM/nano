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

#ifndef _BCGRID_
#define _BCGRID_

const double STM_MIN_X = -10.0;
const double STM_MAX_X = 10.0;
const double STM_MIN_Y = STM_MIN_X;
const double STM_MAX_Y = STM_MAX_X;

const int READ_DEVICE = 0;
const int READ_FILE = 1;
const int READ_STREAM = 2;

#include <ctype.h> 
#include <stdio.h> // for FILE
#include "BCDebug.h"

class BCPlane;

class BCGrid
{
  friend class TopoFile;
  public:

    BCGrid(short num_x, short num_y, 
	   double min_x, double max_x, 
	   double min_y, double max_y,
	   int read_mode,
	   const char* file_name);

    BCGrid(short num_x, short num_y, 
	   double min_x, double max_x, 
	   double min_y, double max_y,
	   int read_mode,
	   const char** file_names, int num_files);

    BCGrid(short num_x, short num_y, 
	   double min_x, double max_x, 
	   double min_y, double max_y);

    ~BCGrid();

    void     findUniquePlaneName(BCString base_name, BCString *result_name);
    BCPlane* addNewPlane(BCString dataset, BCString units, int timed);
    BCPlane* addPlaneCopy(BCPlane* grid);

    int deleteHead();
    int empty();
    int empty_list();
    BCPlane* head() {return _head; };
    BCPlane* getPlaneByName(BCString name);
    inline int numPlanes() {return _num_planes;};

    void decimate(short num_x, short num_y);

    inline short numX() const { return _num_x; }
    inline short numY() const { return _num_y; }
    double minX() const { return _min_x; }
    double maxX() const { return _max_x; }
    double minY() const { return _min_y; }
    double maxY() const { return _max_y; }

    BCGrid * _next;  // used in the snapshot grid list

    int readMode() const { return _read_mode; }
    
    inline double derangeX()
    {	
	if (_modified)
	    _derange_x = (_num_x-1) / (_max_x - _min_x);
	return _derange_x;
    }

    inline double derangeY()
    {
	if (_modified)
	    _derange_y = (_num_y-1) / (_max_y-_min_y);
	return _derange_y;
    }
    
    void setMinX(double min_x) { _min_x = min_x; _modified = 1; }
    void setMaxX(double max_x) { _max_x = max_x; _modified = 1; }
    void setMinY(double min_y) { _min_y = min_y; _modified = 1; }
    void setMaxY(double max_y) { _max_y = max_y; _modified = 1; }

    double transform(short* datum, int image_mode, double scale); // defined in readNanoscopeFile.C

    int writeTextFile(FILE* file, BCPlane* grid);
    int writeBinaryFile(FILE* file, BCPlane* grid);
    int writeUNCAFile(FILE* file, BCPlane* grid);
    int writeSPIPFile(FILE* file, BCPlane* grid);
    int writePPMFile(FILE* file, BCPlane* grid);
    int writeRawVolFile(const char* file_name);

    friend ostream& operator << (ostream& os, BCGrid* grid);

  private:

    void BCGridFill(short num_x, short num_y, 
	   double min_x, double max_x, 
	   double min_y, double max_y,
	   int read_mode,
	   const char** file_names, int num_files);

    void addPlane(BCPlane* grid);

    double** makeMask(short num_x, short num_y);

    int readFile(FILE* file, const char *name);
    int readTextFile(FILE* file, const char *name);
    int readBinaryFile(FILE* file, const char *name);
    int readUNCAFile(FILE* file, const char *name);
    int readUNCBFile(FILE* file, const char *name);
    int readSPIPFile(FILE* file, const char *name);
    int readNanoscopeFileWithoutHeader(FILE* file, const char *name);
      // readNanoscopeFile.C
    int readBinaryNanoscopeFile(FILE* file, const char *name);
      // readNanoscopeFile.C 
    int readAsciiNanoscopeFile(FILE *file, const char *name);
      // readNanoscopeFile.C
    int parseNanoscopeFileHeader(FILE* file);
      // defined in readNanoscopeFile.C
    int convertTopoData(const char *name);
      // defined in readTopometrixFile.C
    int readTopometrixFile(FILE* file, const char *name);
      // defined in BCGrid.C
    int readTopometrixFile(TopoFile& TF, const char *name);
      // in readTopometrixFile.C
    int readComment(FILE *file, char *buffer, double* max_value);
    int readPPMorPGMFile(FILE* file, const char *name);

    short _num_x, _num_y;
    double _min_x, _max_x;
    double _min_y, _max_y;
    double _derange_x, _derange_y;
    int _num_planes;
    BCPlane* _head;


    // the following are used by the methods that read Nanoscope files
    double _detection_sensitivity;
    double _attenuation_in_z;
    double _z_scale;
    double _z_scale_auxc;
    double _z_max; 
    double _input_sensitivity;
    double _z_sensitivity;
    double _input_1_max;
    double _input_2_max;
   
    int _modified; // true if _min_x, _max_x, _min_y, or _max_y have been altered

    static int _read_mode;

    static int _times_invoked;
};

#endif // _BCGRID_

