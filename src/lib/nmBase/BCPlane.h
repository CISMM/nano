/*===3rdtech===
  Copyright (c) 2000 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#ifndef _BCPLANE_
#define _BCPLANE_

const int TIMED = 1;
const int NOT_TIMED = 0;

const double DEFAULT_MIN_ATTAINABLE_VALUE = -10;
const double DEFAULT_MAX_ATTAINABLE_VALUE = 10;

const int BCPLANE_OPTIMIZE_LINE = 0;
const int BCPLANE_OPTIMIZE_AREA = 1;

#include "BCGrid.h"


const	int	MAX_PLANE_CALLBACKS = 32;
/** Callback to say that one or more of the values in the plane have changed.
 If x and y are >=0, then it is a single point.  If they are both -1, then
 all points may have changed (or the scale changed or something).
*/
typedef	void	(*Plane_Valuecall)(BCPlane *plane, int x,int y, void *userdata);

class BCPlane
{      
  friend class BCGrid; // BCGrids want access to _next
  friend class TopoFile;
  friend class nmb_ImageGrid; // needs to access _grid
  //  friend class nma_ShapeAnalyze;//needs _grid
  //  friend class nma_ShapeIdentifiedPlane;//needs _grid
  public:

    inline BCString* name() { return &_dataset; }
    inline BCString* units() { return &_units; }
    inline BCPlane* next() { return _next; }

    inline short numX() const { return _grid->numX(); }
    inline short numY() const { return _grid->numY(); }
    inline double minX() const { return _grid->minX(); }
    inline double maxX() const { return _grid->maxX(); }
    inline double minY() const { return _grid->minY(); }
    inline double maxY() const { return _grid->maxY(); }

    inline int convertToColor(int x, int y) {
	return (int) (254 * (value(x,y) - minAttainableValue()) /
		      (maxAttainableValue() - minAttainableValue()));
    }
      ///< Only valid for files, not streams or live devices

    BCGrid * GetGrid(){return _grid;}

    double minValue();
    double maxValue();

    double minNonZeroValue();
    double maxNonZeroValue();

    inline double minValueComputedLast() const { return _min_value; }
    inline double maxValueComputedLast() const { return _max_value; }

    inline double minNonZeroValueComputedLast() const { return _min_nonzero_value; }
    inline double maxNonZeroValueComputedLast() const { return _max_nonzero_value; }

    double minAttainableValue (void) const;
    ///< Set to min value reachable by the scanner used in the AFM. 
    ///< For files, set to the min data value. 
    double maxAttainableValue (void) const;
    ///< Set to max value reachable by the scanner used in the AFM. 
    ///< For files, set to the max data value. 

    inline void setMinAttainableValue(double v) { _min_attainable_value = v; }
    inline void setMaxAttainableValue(double v) { _max_attainable_value = v; }

    inline double scaledMinValue() { return minValue()* _scale; }
    inline double scaledMaxValue() { return maxValue()*_scale; }

    double derangeX() { return _grid->derangeX(); }
    double derangeY() { return _grid->derangeY(); }     

    double scale() const { return _scale; }
    void setScale(double scale) { _scale = scale; }
    void tweakScale(); ///< Set scale to reasonable value based
    ///< on scan size and data range. 

    int findValidDataRange(short* o_top, short* o_left, short* o_bottom, short* o_right);

    void level();

    void rename(BCString new_name) { _dataset = new_name; }

    virtual	void setTime(int x, int y, long sec, long usec) = 0;

    inline float value(int x, int y) const 
       { return _value[x + _borderXMin + (y + _borderYMin)*
                                         (_borderXMin+_num_x+_borderXMax)]; }
    int valueAt (double * result, double x, double y);
    int valueAt (float * result, double x, double y);
    
    float interpolatedValue(double x, double y);
    float interpolatedValueAt(double x, double y);

    inline double scaledValue(int x, int y) const {
	return value(x,y) * (double) _scale;
    }

    virtual void setValue(int x, int y, float value);

    inline double xInWorld(int x, double scale = 1.0) const {
	return scale*( minX() + (maxX()-minX()) * ((double)x/(numX()-1)) );
    }
    inline double yInWorld(int y, double scale = 1.0) const {
	return scale*( minY() + (maxY()-minY()) * ((double)y/(numY()-1)) );
    }

    inline double xInGrid(double x ) const {
        return (x - minX()) / (maxX()-minX()) * (numX()-1);
    }

    inline double yInGrid(double y) const {
        return (y - minY()) / (maxY()-minY()) * (numY()-1);
    }



    inline double valueInWorld(int x, int y, double scale = 1.0) const {
	return scale * scaledValue(x,y);
    }

    /** Register and remove callbacks to be called when a value in the Plane
	changes.  Return 0 on success and -1 on failure. */
    int add_callback(Plane_Valuecall cb, void *userdata);
    int remove_callback(Plane_Valuecall cb, void *userdata);

    friend ostream& operator << (ostream& os, BCPlane* plane);
    friend ostream& operator << (ostream& os, BCGrid* grid); // in BCGrid.C
    BCPlane(BCPlane* plane);
    /**< NOT a copy constructor!  Creates a new plane of the same size,
       but does not fill in data values. */
    BCPlane(BCPlane* plane, int newX, int newY);
    /**< NOT a copy constructor!  Creates a new, smaller plane,
       but does not fill in data values.*/
    BCPlane(BCString name, BCString units, int nx, int ny);
    virtual ~BCPlane (void);
	

    /// We need to expose this to an IBR object.  Shouldn't be needed for
    /// any dissimilar purpose. Order of pixels is row-major order:
    /// row0, row1, row2 ... row<_num_y-1>
    const float * flatValueArray (void) const
      { return _value; }

    float tm_scale;     ///< Scale used by ThermoMicroscopes to acquire data
    float tm_offset;    ///< Offset used by ThermoMicroscopes to acquire data

  protected: 

    int setGridSize(int x, int y);
    ///< Changes to a new grid size, deletes the data. 

    virtual void computeMinMax (void);
    /** Puts the identical portions of minValue() and maxValue()
       in one place where they can be overridden by smarter subclasses
       (like CTimedPlane) */

    int readTextFile(FILE* file);
    int writeTextFile(FILE* file);

    int readBinaryFile(FILE* file);
    int writeBinaryFile(FILE* file);

    int readSPIPFile(FILE * file, double scale, int intel_mode);
    int writeSPIPFile(FILE *file);

    int readUNCAFile(FILE* file);
    int writeUNCAFile(FILE* file);

    int readUNCBFile(FILE* file);

    int readTopometrixFile(FILE* file);

    int readComment(FILE *file, char *buffer, double* max_value);
    int readPPMorPGMFile(FILE* file, double scale);
    int writePPMFile(int file_descriptor);

    int writeRawByteFile(int file_descriptor, double min, double max);

    int readNanoscopeFileWithoutHeader(FILE* file);
    int readAsciiNanoscopeFile(FILE* file, nmb_diImageInfo * file_info);
    int readBinaryNanoscopeFile(FILE* file, nmb_diImageInfo * file_info);

    int readAsciiRHKFile(FILE* file, double z_offset_nm, double z_scale_pm);

    int writeNCFile(FILE* file, double sizex, double sizey, double sizez,
		double maxCut = 0.1, double zoff = 0.1, int roughskip = 1);

    int writeNCPass(FILE* file, double scale, double centerx, double centery,
		double Depth, double zoff, double maxCut, int increment,
		int checkskip);

    BCGrid * _grid; 	///< the instance of BCGrid to which this belongs
    int _timed;		///< true if space for secs and usecs has been allocated
    BCString _dataset;	///< a name for the values stored in _value
    BCString _units;	///< units of the values stores in _value
    double _min_value, _max_value;	
    double _min_nonzero_value, _max_nonzero_value;	
    double _min_attainable_value;
    double _max_attainable_value;
    double _scale;		
    BCPlane * _next;	///< the next BCPlane in the list maintained by _grid

    float * _value;     ///< The data
    int _num_x, _num_y; ///< The size of the 2D _value array
    int _max_value_x_coord, _max_value_y_coord;
    int _borderXMin, _borderXMax, _borderYMin, _borderYMax;
    int _validXMin, _validXMax, _validYMin, _validYMax;

    long** _sec;        ///< Times for each data point
    long** _usec;

    int _modified;	///< true if _value has been changed
    int _modified_nz;	///< true if _value has been changed
    int _image_mode;

    struct {
	Plane_Valuecall callback;	///< Callback function to call
	void		*userdata;	///< Value to pass as userdata
    } _callbacks[MAX_PLANE_CALLBACKS];
    int _numcallbacks;          ///< How many callbacks are registered?
    int lookup_callback(Plane_Valuecall cb, void *userdata);
};


class CTimedPlane : public BCPlane
{
  friend class BCGrid;
    
  public:

    virtual void setTime(int x, int y, long sec, long usec);

  protected:

    virtual void computeMinMax (void);

  private:

    CTimedPlane(BCString name, BCString units, int nx, int ny);
    CTimedPlane(CTimedPlane* grid);
    CTimedPlane(CTimedPlane* grid, int newX, int newY);

    ~CTimedPlane();
};


class CPlane : public BCPlane
{
  friend class BCGrid;

  public:

    void setTime(int x, int y, long sec, long usec);

  private:

    CPlane(BCString name, BCString units, int nx, int ny);
    CPlane(CPlane* grid);
    CPlane(CPlane* grid, int newX, int newY);

    ~CPlane();
};

#endif // _BCPLANE_
