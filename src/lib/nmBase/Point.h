#ifndef POINT_H
#define POINT_H

#include <stdio.h>
#include <tcl.h>
#include "BCString.h"

class	Point_value;
class	Point_results;
class	Point_list;

const	int	MAX_POINT_CALLBACKS = 32;
typedef	void	(*Point_Valuecall)(Point_value *newval, void *userdata);

class	Point_value
{
  friend class Point_results; // Point_results want access to _next

  public:

    // Name and units of the data set that this value stores.    
    inline BCString*	name (void) { return &_dataset; }
    inline BCString*	units (void) { return &_units; }
    inline void		rename (BCString new_name) { _dataset = new_name; }

    /// Next value in the list associated with the same Point_results
    inline Point_value*   next (void) const { return _next; }
    /// Point_results stores the x,y location and time information.
    inline Point_results* results (void) const { return _results; }

    /// Read and write the value.  Writing yanks the callbacks.
    inline float value (void) const { return _value; }
    inline void  setValue (const float value) {
        _value = value;
	for (int i = 0; i < _numcallbacks; i++)
		_callbacks[i].callback(this, _callbacks[i].userdata);
     }

    /// Register and remove callbacks to be called when this point value
    /// changes.  Return 0 on success and -1 on failure.
    int	add_callback (Point_Valuecall cb, void * userdata);
    int remove_callback (Point_Valuecall cb, void * userdata);

  protected: // accessible by subclasses of Point_value and their friends

    Point_value (BCString name, BCString units);
    Point_value (Point_value * value);
    ///< Yes, these ARE nonconst

    ~Point_value (void) {};

    int lookup_callback (const Point_Valuecall cb, const void * userdata);

    Point_results* _results; 	///< The Point_results to which this belongs
    BCString _dataset;		///< a name for the values stored in _value
    BCString _units;		///< units of the values stores in _value
    Point_value* _next;		///< the next Point_value _results' list

    struct {
	Point_Valuecall	callback;	///< Callback function to call
	void		*userdata;	///< Value to pass as userdata
    } _callbacks[MAX_POINT_CALLBACKS];
    int	_numcallbacks;		///< How many callbacks are registered?

    float _value;
};

class	Point_results
{
  public:
    friend class Point_list;

    Point_results (void) {
	_num_values = 0; _head = NULL; _sec = _usec = 0; _x = _y = _z = -1.0;
	_is3D = 0;
    }
    Point_results (const Point_results & p);
    ~Point_results (void);

    void findUniqueValueName (BCString base_name, BCString * result_name);
    Point_value * addNewValue (BCString dataset, BCString units);
    Point_value * addValueCopy (Point_value * value);

    inline void setTime(long sec, long usec) { _sec = sec; _usec = usec; }
    inline void setPosition(double x, double y) { _x = x; _y = y; }
    inline void setPosition(double x, double y, double z) 
		{_x = x; _y = y; _z = z; _is3D = 1;}

    inline double x (void) const { return _x; }
    inline double y (void) const { return _y; }
    inline double z (void) const { return _z; }
    inline int is3D (void) const { return _is3D; }
    inline long sec (void) const { return _sec; }
    inline long usec (void) const { return _usec; }

    int deleteHead (void);
    int empty (void) const;
    Point_value * head (void) const { return _head; };
    Point_value * getValueByName (const BCString name) const;
    Point_value * getValueByPlaneName (const BCString name) const;

    void print (const char * prelim = "") const;

  protected:

    void addValue (Point_value * value);

    int _num_values;
    Point_value * _head;

    double _x;
    double _y;
    double _z;
    int _is3D;

    long _sec;
    long _usec;
};

const	int	MAX_POINT_LIST = 10000;
/** This class is a list of Point_results, each of which should have the
 same number and type of Point_values.  This list was created for use
 in writing xgraph-readible output from the modification results.
*/
class	Point_list
{
  public:

    Point_list (void) { _num_entries = 0; };
    ~Point_list (void);


    // ACCESSORS


    int numEntries (void) const;
    const Point_results * entry (int) const;


    // MANIPULATORS


    int addEntry (const Point_results & p);
    void clear (void);

    int writeToAsciiFile (FILE * f);
    int writeToTclWindow (Tcl_Interp * interpreter);

  protected:

    int _num_entries;
    Point_results * _entries [MAX_POINT_LIST];
};


#endif
