#ifndef _BCPLANE_
#define _BCPLANE_

const int TIMED = 1;
const int NOT_TIMED = 0;

const double DEFAULT_MIN_ATTAINABLE_VALUE = -10;
const double DEFAULT_MAX_ATTAINABLE_VALUE = 10;


#include "BCDebug.h"
#include "BCGrid.h"


// Callback to say that one or more of the values in the plane have changed.
// If x and y are >=0, then it is a single point.  If they are both -1, then
// all points may have changed (or the scale changed or something).
const	int	MAX_PLANE_CALLBACKS = 32;
typedef	void	(*Plane_Valuecall)(BCPlane *plane, int x,int y, void *userdata);

class BCPlane
{      
  friend class BCGrid; // BCGrids want access to _next
  friend class TopoFile;
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
	return (int) (254 * (_value[x][y] - minAttainableValue()) /
		      (maxAttainableValue() - minAttainableValue()));
    }

    double minValue();
    double maxValue();

    double minNonZeroValue();
    double maxNonZeroValue();

    inline double minValueComputedLast() const { return _min_value; }
    inline double maxValueComputedLast() const { return _max_value; }

    inline double minNonZeroValueComputedLast() const { return _min_nonzero_value; }
    inline double maxNonZeroValueComputedLast() const { return _max_nonzero_value; }

    inline double minAttainableValue() const { return _min_attainable_value; }
    inline double maxAttainableValue() const { return _max_attainable_value; }

    inline void setMinAttainableValue(double v) { _min_attainable_value = v; }
    inline void setMaxAttainableValue(double v) { _max_attainable_value = v; }

    inline double scaledMinValue() { return minValue()* _scale; }
    inline double scaledMaxValue() { return maxValue()*_scale; }

    double derangeX() { return _grid->derangeX(); }
    double derangeY() { return _grid->derangeY(); }     

    double scale() const { return _scale; }
    void setScale(double scale) { _scale = scale; }

    void level();

    void rename(BCString new_name) { _dataset = new_name; }

    virtual	void setTime(int x, int y, long sec, long usec);

    inline float value(int x, int y) const { return _value[x][y]; }
    inline float valueAt(double x, double y) {
	int ix = (int) (derangeX() * (x - minX()));
	int iy = (int) (derangeY() * (y - minY()));
	return _value[ix][iy];
    };

    inline double scaledValue(int x, int y) const {
	return _value[x][y] * (double) _scale;
    }

    void setValue(int x, int y, float value);

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

    // Register and remove callbacks to be called when a value in the Plane
    // changes.  Return 0 on success and -1 on failure.
    int add_callback(Plane_Valuecall cb, void *userdata);
    int remove_callback(Plane_Valuecall cb, void *userdata);

    friend ostream& operator << (ostream& os, BCPlane* plane);
    friend ostream& operator << (ostream& os, BCGrid* grid); // in BCGrid.C
    BCPlane(BCPlane* plane);
    BCPlane(BCString name, BCString units, int nx, int ny);
    virtual ~BCPlane (void);
	
  protected: // accessible by subclasses of BCPlane and their friends


    int readTextFile(FILE* file);
    int writeTextFile(FILE* file);

    int readBinaryFile(FILE* file);
    int writeBinaryFile(FILE* file);

    int readSPIPFile(FILE * file, double max_value);
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
    int readAsciiNanoscopeFile(FILE* file);
    int readBinaryNanoscopeFile(FILE* file);

    BCGrid* _grid; 	// the instance of BCGrid to which this belongs
    int _timed;		// true if space for secs and usecs has been allocated
    BCString _dataset;	// a name for the values stored in _value
    BCString _units;	// units of the values stores in _value
    double _min_value, _max_value;	
    double _min_nonzero_value, _max_nonzero_value;	
    double _min_attainable_value;
    double _max_attainable_value;
    double _scale;		
    BCPlane* _next;	// the next BCPlane in the list maintained by _grid

    float** _value;
    int _num_x, _num_y;

    long** _sec;
    long** _usec;

    int _modified;	// true if _value has been changed
    int _modified_nz;	// true if _value has been changed
    int _image_mode;

    struct {
	Plane_Valuecall callback;	// Callback function to call
	void		*userdata;	// Value to pass as userdata
    } _callbacks[MAX_PLANE_CALLBACKS];
    int _numcallbacks;          // How many callbacks are registered?
    int lookup_callback(Plane_Valuecall cb, void *userdata);
};


class CTimedPlane : public BCPlane
{
  friend class BCGrid;
    
  public:

    virtual void setTime(int x, int y, long sec, long usec);

  private:

    CTimedPlane(BCString name, BCString units, int nx, int ny);
    CTimedPlane(CTimedPlane* grid);

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

    ~CPlane();
};

#endif // _BCPLANE_
