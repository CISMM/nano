/*===3rdtech===
  Copyright (c) 2000 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#ifndef _BCGRID_
#define _BCGRID_

const double STM_MIN_X = -10.0;
const double STM_MAX_X = 10.0;
const double STM_MIN_Y = STM_MIN_X;
const double STM_MAX_Y = STM_MAX_X;

const int READ_DEVICE = 0;
const int READ_FILE = 1;
const int READ_STREAM = 2;

extern const char * EMPTY_PLANE_NAME;

#include <ctype.h> 
#include <stdio.h> // for FILE
#include "BCString.h"

#if defined(_WIN32) && !defined(__CYGWIN__)
// bogus double to float conversion warning.
#pragma warning(disable:4244)
#pragma warning(disable:4305)
#endif

class BCPlane;

typedef void (* BCGrid_MinMaxCallback) (void * userdata,
                                        double minX, double maxX,
                                        double minY, double maxY);

class BCGrid
{
  friend class TopoFile;
  public:

    BCGrid();

    BCGrid(short num_x, short num_y, 
	   double min_x, double max_x, 
	   double min_y, double max_y,
	   int read_mode,
	   const char* file_name,
           TopoFile &topoFile);

    BCGrid(short num_x, short num_y, 
	   double min_x, double max_x, 
	   double min_y, double max_y,
	   int read_mode,
	   const char** file_names, int num_files,
           TopoFile &topoFile);

    BCGrid(short num_x, short num_y, 
	   double min_x, double max_x, 
	   double min_y, double max_y);

    BCGrid (const BCGrid *);  // copy constructor

    virtual ~BCGrid (void);

    /// Makes a name unique.
    void     findUniquePlaneName (
	  BCString base_name, ///< Desired name
	  BCString *result_name ///< Uniqueified name
	  );
    BCPlane* addNewPlane(BCString dataset, BCString units, int timed);
    BCPlane* addPlaneCopy(BCPlane* grid);

    int loadFiles(const char** file_names, int num_files, TopoFile &topoFile);
      ///< Load files with the same grid size into this grid.
      ///< Load files with any grid size if no meaningful data has 
      ///< yet been loaded.
    int setGridSize(int x, int y);
      ///< Changes the grid size, erases all data.

    int deleteHead();
    int empty();
    int empty_list();
    BCPlane* head() {return _head; };
    BCPlane* getPlaneByName(BCString name);
    inline int numPlanes() {return _num_planes;};

    void decimate(short num_x, short num_y);

    /// Return the number of samples along the X-axis
    inline short numX() const { return _num_x; }

    /// Return the number of samples along the Y-axis
    inline short numY() const { return _num_y; }

    // These tell you where the grid is located in the microscope space.
    // (minX(), minY()) is the location of the grid origin.
    // (maxX(), maxY()) is the location of the opposite corner of the grid.
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

    inline void gridToWorld(const double i, const double j,
                  double &x, double &y)
    {
        x = i/derangeX() + minX();
        y = j/derangeY() + minY();
    }

    inline void worldToGrid(const double x, const double y,
                  double &i, double &j)
    {
        i = (x-minX())*derangeX();
        j = (y-minY())*derangeY();
    }
    
    void setMinX (double min_x);
    void setMaxX (double max_x);
    void setMinY (double min_y);
    void setMaxY (double max_y);

    void registerMinMaxCallback (BCGrid_MinMaxCallback cb, void * userdata);

    double transform(short* datum, int image_mode, double scale); // defined in readNanoscopeFile.C

    int writeTextFile(FILE* file, BCPlane* grid);
    int writeBinaryFile(FILE* file, BCPlane* grid);
    int writeUNCAFile(FILE* file, BCPlane* grid);
    int writeSPIPFile(FILE* file, BCPlane* grid);
    int writePPMFile(FILE* file, BCPlane* grid);
    int writeTIFFile(FILE* file, BCPlane* grid, const char * filename);
    int writeRawVolFile(const char* file_name);
    int writeNCFile(FILE* file, BCPlane* grid,
	double sizex = 1, double sizey = 1, double sizez = 1,
	double maxcut = 0.1, double zoff = 0.1, int roughskip = 1);

    friend ostream& operator << (ostream& os, BCGrid* grid);

  protected:

    double ** makeMask (short num_x, short num_y);

    static void decimateRegion
                  (short minx, short miny, short maxx, short maxy,
                   BCPlane * source, BCPlane * dest,
                   int destNumX, int destNumY,
                   int maskNumX, int maskNumY, double ** mask);
    /**< decimates a subset of <source> into the region [minx, maxx] x
       [miny, maxy] of <dest>, which is <destNumX> x <destNumY> in size,
       using <mask> */

    void doMinMaxCallbacks (void);

    struct MinMaxCB {
      BCGrid_MinMaxCallback cb;
      void * userdata;
      MinMaxCB * next;
    };
    MinMaxCB * d_minMaxCB;

  private:

    void BCGridFill(short num_x, short num_y, 
	   double min_x, double max_x, 
	   double min_y, double max_y,
	   int read_mode,
	   const char** file_names, int num_files,
           TopoFile &topoFile);

    void addPlane(BCPlane* grid);

    int readFile(FILE* file, const char *name, TopoFile &topoFile);
    int readTextFile(FILE* file, const char *name);
    int readBinaryFile(FILE* file, const char *name);
    int readUNCAFile(FILE* file, const char *name);
    int readUNCBFile(FILE* file, const char *name);
    int readSPIPFile(FILE* file, const char *name);
    int readAsciiRHKFile(TopoFile &TGF, FILE* file, const char *name);

    int readNanoscopeFileWithoutHeader(FILE* file, const char *name);
    ///< readNanoscopeFile.C
    int readBinaryNanoscopeFile(FILE* file, const char *name);
      ///< readNanoscopeFile.C 
    int readAsciiNanoscopeFile(FILE *file, const char *name);
      ///< readNanoscopeFile.C
    int parseNanoscopeFileHeader(FILE* file);
      ///< defined in readNanoscopeFile.C

    int readTopometrixFile(TopoFile& TF, FILE* file, const char *name);
      ///< in Topo.C

    int readComment(FILE *file, char *buffer, double* max_value);
    int readPPMorPGMFile(FILE* file, const char *name);
    int readPPMorPGMFileNew(FILE *file, const char *filename);

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
   
    int _modified; ///< true if _min_x, _max_x, _min_y, or _max_y have been altered

    int _read_mode;

    static int _times_invoked;
};

#endif // _BCGRID_

