/*===3rdtech===
  Copyright (c) 2000 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#include <stdlib.h> // for exit()
#include <malloc.h> // for calloc() and free()
#include <errno.h> // for perror()
#ifdef	_WIN32
#include <io.h>
#ifdef __CYGWIN__
// [juliano 9/19/99]
//   added these decl so can compile BCPlane.C on my PC in cygwin.
extern "C" {
int write( int fildes, const void *buf, size_t nbyte );
}
#endif
#else
#include <unistd.h>   // for write()
#endif

#include "BCPlane.h"
#include "BCGrid.h"

#ifndef	min
#define min(x,y) ( (x) < (y) ? (x) : (y) )
#endif
#ifndef	max
#define max(x,y) ( (x) > (y) ? (x) : (y) )
#endif

/**
setValue
    
        @author Kimberly Passarella Jones
 @date modified 7/2/96 by Russ Taylor
*/
void
BCPlane::setValue(int x, int y, float value)
{
    _value[x * _num_y + y] = value;
    _modified = 1;
    _modified_nz = 1;

    for (int i = 0; i < _numcallbacks; i++)
	_callbacks[i].callback(this, x,y, _callbacks[i].userdata);
}

/**
setTime
    Does nothing.  Allows this routine to be called for all
		planes, even untimed ones.
        @author Russ Taylor
 @date modified 5/3/96 by Russ Taylor
*/
//void
//BCPlane::setTime(int x, int y, long sec, long usec)
 //{ x = x; y = y; sec = sec; usec = usec; }


// virtual
void BCPlane::computeMinMax (void) {
  int x, y;

  _max_value = -1.0e33;
  _min_value = 1.0e33;

  for (x = 0; x < numX(); x++) {
    for (y = 0; y < numY(); y++) {
      double value =  this->value(x, y);

      // Original algorithm
      // if ((x == 0) && (y == 0)) {
      //   _min_value = value;
      //   _min_value = value;
      // }

      if (value < _min_value) { _min_value = value; }
      if (value > _max_value) { _max_value = value; }
    }
  }
  //  printf("_min_value = %f, _max_value = %f\n", _min_value, _max_value);
}
	

/**
minValue
    
        @author Kimberly Passarella Jones
 @date modified 9-10-95 by Kimberly Passarella Jones
*/
double
BCPlane::minValue()
{
  if (!_modified) {
	return minValueComputedLast();
  }
  else {
    BCPlane::computeMinMax();  //have to tell it which computeMinMax to use
    _modified = 0;
    //    printf("_min_value = %f\n", _min_value);
    return _min_value;
  }
} // minValue


/**
maxValue
    
        @author Kimberly Passarella Jones
 @date modified 9-10-95 by Kimberly Passarella Jones
*/
double
BCPlane::maxValue() {

  if (!_modified)
    return maxValueComputedLast();
  else
    {
      BCPlane::computeMinMax();  //have to tell it which computeMinMax to use
      _modified = 0;
      return _max_value;
    }
} // maxValue

/**
minNonZeroValue
    Minimum nonzero value in plane. Useful if not all data 
                has been filled in yet. 
        @author Aron Helser
 @date modified 8-19-98 Aron Helser
*/
double
BCPlane::minNonZeroValue()
{
    if (!_modified_nz)
	return minNonZeroValueComputedLast();
    else
    {
      int x, y;
      _min_nonzero_value = 0;
      _max_nonzero_value = 0;
	for (x = 0; x < numX(); x++) 
	{
	    for (y = 0; y < numY(); y++) 
	    {
		double value =  this->value(x, y);
	    
		if (((x == 0) && (y == 0)) || (_min_nonzero_value == 0))
		{
		    _min_nonzero_value = value;
		    _max_nonzero_value = value;
		}
		if (value != 0.0) {
		  if (value < _min_nonzero_value) { _min_nonzero_value = value; }
		  if (value > _max_nonzero_value) { _max_nonzero_value = value; }
		}
	    }
	}
	
	if (_max_nonzero_value == _min_nonzero_value) // no real data has been stored in the grid
	{
	    _max_nonzero_value = -1.0e33;
	    _min_nonzero_value = 1.0e33;
	}
	
	_modified_nz = 0;
	return _min_nonzero_value;
    }
} // minNonZeroValue


/**
maxNonZeroValue
    Maximum nonzero value in plane. Useful if not all data 
                has been filled in yet. 
        @author Aron Helser
 @date modified 8-19-98 Aron Helser
*/
double
BCPlane::maxNonZeroValue()
{
    if (!_modified_nz)
	return maxNonZeroValueComputedLast();
    else
    {
	int x, y;
      _min_nonzero_value = 0;
      _max_nonzero_value = 0;

    	for (x = 0; x < numX(); x++) 
	{
	    for (y = 0; y < numY(); y++) 
	    {
		double value = this->value(x, y);
		
		if (((x == 0) && (y == 0)) || (_min_nonzero_value == 0))
		{
		    _min_nonzero_value = value;
		    _max_nonzero_value = value;
		}
		if (value != 0.0) {
		  if (value < _min_nonzero_value) { _min_nonzero_value = value; }
		  if (value > _max_nonzero_value) { _max_nonzero_value = value; }
		}
	    }
	}

	if (_max_nonzero_value == _min_nonzero_value) // no real data has been stored in the grid
	{
	    _max_nonzero_value = -1.0e33;
	    _min_nonzero_value = 1.0e33;
	}
	    
	_modified_nz = 0;
	return _max_nonzero_value;
    }
} // maxNonZeroValue

double BCPlane::minAttainableValue (void) const {
  return _min_attainable_value;
}

double BCPlane::maxAttainableValue (void) const {
  return _max_attainable_value;
}


/**
 findValidDataRange
    Assuming the initial plane was filled with all zeros, finds
   		the rectangle of data which is no longer zero. Used to write 
		Topo files, versions 4 and 5. If you tell Topo that the whole
		grid is valid, and part of the grid is filled with zeros, Topo
		chokes. 
		Returns -1 if the whole grid is still zero, otherwise 
		returns 0 (success), and fills in the argument pointers with
		the region it found. 
        @author Aron Helser
 @date modified 10-4-99 Aron Helser
*/
int
BCPlane::findValidDataRange(short* o_top, short* o_left, short* o_bottom, short* o_right)
{
    // top edge is y= numY -1, left is x = 0, etc. 
    short top = -1, left = -1, bottom =-1, right =-1;
    short x, y;

    	for (x = 0; x < numX(); x++) 
	{
	    for (y = 0; y < numY(); y++) 
	    {
		double value = this->value(x, y);
		if (value != 0.0) {
		    // Is this first non-zero value?
		    if (top == -1) {
			// set region to be equal to this point.
			top = bottom = y;
			left = right = x;
		    } else {
			// expand the region to include new points. 
			if (y > top) top = y;
			if (y < bottom) bottom = y;
			if (x < left) left = x;
			if (x > right) right = x;
		    }
		}
	    }
	}

	if ((top == -1) || (left == -1) || (bottom == -1) || (right == -1)) {
	    // We didn't find any valid data in this grid. 
	    return -1;
	}
	*o_top = top;
	*o_left = left;
	*o_right = right;
	*o_bottom = bottom;
	return 0;
} // findValidDataRange

/**
   Gives the data value stored in the plane at a location specified
   in nanometers. If the location is not inside the min/max bounds
   of the plane, the function returns -1 and does not modify the \a result
   pointer.
   @param result Set to the value at the \a x \a y location specified
   @param x desired x location, in nm.
   @param y desired y location, in nm.
   @return 0 on success, -1 on failure (value out of bounds)
*/
int BCPlane::valueAt (double * result, double x, double y) {
//int BCPlane::valueAt (double x, double y) {
  int ix;
  int iy;

  ix = (int) (derangeX() * (x - minX()));
  iy = (int) (derangeY() * (y - minY()));

  if ((ix < 0) || (iy < 0) ||
      (ix >= _num_x) || (iy >= _num_y)) {
      //fprintf(stderr, "BCPlane::valueAt %d %d:  Out of bounds!.\n", ix, iy);
    return -1;
  }
  *result = _value [ix * _num_y + iy];
  return 0;
}


float BCPlane::interpolatedValue(double x, double y) {
    int ix = (int)x;
    int iy = (int)y;
    float a = (float)x - (float)ix;
    float b = (float)y - (float)iy;
    if (ix >= numX()-1) {ix = numX()-1; a = 0;}
    if (iy >= numY()-1) {iy = numY()-1; b = 0;}
    if (ix < 0) {ix = 0; a = 0;}
    if (iy < 0) {iy = 0; b = 0;}

    float z;

    // interpolation to get z
    z = value(ix, iy)*(1-a)*(1-b);
    if (a != 0)
      z += value(ix+1, iy  )*(  a)*(1-b);
    if (b != 0){
      z += value(ix,   iy+1)*(1-a)*(  b);
      if (a != 0)
        z += value(ix+1, iy+1)*(  a)*(  b);
    }

    return z;
}

float BCPlane::interpolatedValueAt(double x, double y) {
        double x2 = (derangeX()*(x-minX()));
        double y2 = (derangeY()*(y-minY()));
        return interpolatedValue(x2, y2);
}


/**
level
    This method "levels out" the grid by subtracting from each
                of its values the average plane.
        @author Kimberly Passarella Jones
 @date modified 9-10-95 by Kimberly Passarella Jones
*/
void
BCPlane::level()
{	

    int x, y;

    double dx = 0.0;
    double dy = 0.0;
    double mean = 0.0;

    // compute the mean and the average slopes in each direction
    for (x = 1; x <  numX(); x++)
	for (y = 1; y < numY(); y++)
	{
	    dx += (this->value(x, y) - this->value(x-1, y));
	    dy += (this->value(x, y) - this->value(x, y-1));
	    mean += this->value(x, y);
	}

    double temp = (numX() - 1) * (numY() - 1);
    dx /= temp;
    dy /= temp;
    mean /= temp;

    // compute the base (the height of the point (0,0))
    double base = mean - dx * (numX() - 1)/2.0 - dy * (numY() - 1)/2.0;

    // subtract the plane
    for (x = 0; x < numX(); x++)
	for (y= 0; y < numY(); y++)
	    setValue(x,y,  this->value(x,y) - (base + dx*x + dy*y) );

} // level


// Look up the callback with the given values and return its index in the
// list.  If there is no such callback, return -1.

int     BCPlane::lookup_callback(Plane_Valuecall cb, void *userdata)
{
        int     i = 0;

        // Return its index if we find it in the list.
        for (i = 0; i < _numcallbacks; i++) {
          if ( (_callbacks[i].callback == cb) &&
               (_callbacks[i].userdata == userdata) ) {
            return i;
          }
        }

        // Return -1 if we didn't find it in the list.
        return -1;
}


int     BCPlane::add_callback(Plane_Valuecall cb, void *userdata)
{
        // If the callback is to a NULL function, fail.
        if (cb == NULL) { return -1; }

        // See if there is already one in the list.  If so, fail.
        if (lookup_callback(cb, userdata) != -1) { return -1; }

        // If we have a full list, fail.
        if (_numcallbacks >= MAX_PLANE_CALLBACKS) { return -1; }

        // Put it at the end of the list
        _callbacks[_numcallbacks].callback = cb;
        _callbacks[_numcallbacks].userdata = userdata;
        _numcallbacks++;
        return 0;
}


int     BCPlane::remove_callback(Plane_Valuecall cb, void *userdata)
{
        int     which;

        // Find the callback, if it is in the list.  If not, fail.
        if ( (which = lookup_callback(cb, userdata)) == -1) { return -1; }

        // Move the last one on the list to this location and decrement count
        _callbacks[which] = _callbacks[_numcallbacks-1];
        _numcallbacks--;
        return 0;
}


/******************************************************************************\
  
  The following methods of BCPlane are protected!
  
*/


/**
BCPlane --> constructor
    
        @author Kimberly Passarella Jones
 @date modified 9-10-95 by Kimberly Passarella Jones
*/
BCPlane::BCPlane(BCString name, BCString units, int nx, int ny):
         tm_scale(1),
         tm_offset(0)
{
    //int x;

    _dataset = name;
    _units = units;
    _num_x = nx;
    _num_y = ny;
    _grid = (BCGrid*) NULL;

    _min_value = 0.0;
    _max_value = 0.0;
    _min_attainable_value = -1.0e+6;
    _max_attainable_value = +1.0e+6;
    _scale = 1.0;
    _modified = 1;
    _modified_nz = 1;

    _numcallbacks = 0;

    _next = NULL;

    // Allocate space for the values array
    //if ( (_value = new float*[_num_x]) == NULL) {
	//fprintf(stderr,"BCPlane::BCPlane(): new failed!\n");
	//exit(-1);
    //}
    //for (x = 0; x < _num_x; x++) {
	//if ( (_value[x] = new float[_num_y]) == NULL) {
		//fprintf(stderr,"BCPlane::BCPlane(): new failed!\n");
		//exit(-1);
	//}
    //}
    _value = new float [_num_x * _num_y];
    if (!_value) {
      fprintf(stderr,"BCPlane::BCPlane(): new failed!\n");
      exit(-1);
    }

}


/**
BCPlane --> constructor
    
        @author Kimberly Passarella Jones
 @date modified 9-10-95 by Kimberly Passarella Jones
*/
BCPlane::BCPlane(BCPlane* plane):
         tm_scale(1),
         tm_offset(0)
{
    //int x;

    _dataset = plane->_dataset;
    _units = plane->_units;
    _num_x = plane->_num_x;
    _num_y = plane->_num_y;
    _grid = plane->_grid;

    _min_value = plane->_min_value;
    _max_value = plane->_max_value;
    _min_attainable_value = plane->_min_attainable_value;
    _max_attainable_value = plane->_max_attainable_value;
    _scale = plane->_scale;
    _modified = 1;
    _modified_nz = 1;

    _numcallbacks = 0;

    _next = NULL;

    _value = new float [_num_x * _num_y];
    if (!_value) {
      fprintf(stderr,"BCPlane::BCPlane(): new failed!\n");
      exit(-1);
    }
}

BCPlane::BCPlane(BCPlane* plane, int newX, int newY):
         tm_scale(1),
         tm_offset(0)
{
    //int x;

    _dataset = plane->_dataset;
    _units = plane->_units;
    _num_x = newX;
    _num_y = newY;
    _grid = plane->_grid;

    _min_value = plane->_min_value;
    _max_value = plane->_max_value;
    _min_attainable_value = plane->_min_attainable_value;
    _max_attainable_value = plane->_max_attainable_value;
    _scale = plane->_scale;
    _modified = 1;
    _modified_nz = 1;

    _numcallbacks = 0;

    _next = NULL;

    _value = new float [newX * newY];
    if (!_value) {
      fprintf(stderr,"BCPlane::BCPlane(): new failed!\n");
      exit(-1);
    }
}
   

/**
~BCPlane --> destructor
    
        @author Kimberly Passarella Jones
 @date modified 9-10-95 by Kimberly Passarella Jones
*/
BCPlane::~BCPlane()
{
//    //int x;

    // Free the space taken by the values array
    // in the reverse order it was allocated
    //for (x = _num_x-1; x >= 0; x--) {
	//delete [] _value[x];
    //}
    delete [] _value;
}


/**
   Check the grid size to see if it's changed. If so, 
   Changes the grid resolution, and erases the data in this plane
Protected so no one except the grid can change our resolution.

   @param x new grid x dimension
   @param y new grid y dimension
   @return 0 if successful, -1 on failure.
 */
int BCPlane::setGridSize(int x, int y)
{
  if ((x==_num_x) && (y==_num_y)) {
    return 0;
  }
  // Grid resolution has changed. Re-allocate and initialize plane to zero.
    delete [] _value;
    _value = NULL;
    _value = new float [x * y];
    if (!_value) {
	fprintf(stderr, "BCPlane::setGridSize, out of memory.\n");
	_num_x = _num_y = 0;
	return -1;
    }
    // Set plane values to zero - indicates that no data has arrived in the plane. 
    memset(_value, 0, x*y*sizeof(float));

    if (_timed == TIMED) {
	int i;

	for (i = _num_x; i > 0; --i) {
	    delete [] _sec[i-1];
	    delete [] _usec[i-1];	
	}
	
	delete [] _sec;
	delete [] _usec;
	
	_sec = new long*[x];
	_usec = new long*[x];
	if ((!_sec) || (!_usec)){
	    fprintf(stderr, "BCPlane::setGridSize, out of memory.\n");
	    _num_x = _num_y = 0;
	    return -1;
	}
	
	for (i = 0; i < x; i++) {
	    _sec[i] = new long[y];
	    _usec[i] = new long[y];
	    if ((!_sec[i]) || (!_usec[i])){
		fprintf(stderr, "BCPlane::setGridSize, out of memory.\n");
		_num_x = _num_y = 0;
		return -1;
	    }
	    // Set timed values to zero - indicates that no data has arrived in the plane. 
	    memset(_sec[i], 0, y*sizeof(long));
	    memset(_usec[i], 0, y*sizeof(long));
	}
    }

    _num_x = x;
    _num_y = y;

    return 0;

}

/**
readTextFile
    
        @author ?
 @date modified 9-10-95 by Kimberly Passarella Jones
*/
int BCPlane::readTextFile(FILE* file)
{
    if (fscanf(file,"%lf %lf", &_min_value, &_max_value) != 2) 
    {
	perror("CPlane::readTextFile: Could not get min/max value!");
	return -1;
    }
	
    int	first_value_read = 1;	
    int x, y;
	
    for (y = 0; y < numY(); y++) {
	for (x = 0; x < numX(); x++) {
	    double value;

	    if (fscanf(file,"%lf", &value) != 1) {
		perror("CPlane::readTextFile: Could not read value!");
		fprintf(stderr,"   x: %d  y: %d\n", x, y);
		return -1;
	    }

	    if (first_value_read) {
		_min_value = value;
		_max_value = value;
		first_value_read = 0;
	    }

	    if (value < _min_value) { _min_value = value; }
	    if (value > _max_value) { _max_value = value; }

	    setValue(x,y, value);

	    if (_timed) {
		_sec[x][y] = 0;
		_usec[x][y] = 0;
	    }
	}
    }

    return 0;
} // readTextFile


/**
writeTextFile
    
        @author ?
 @date modified 9-10-95 by Kimberly Passarella Jones
*/
int
BCPlane::writeTextFile(FILE* file)
{ 
    if (fprintf(file,"%f %f\n\n", _min_value, _max_value) == EOF) 
    {
	perror("BCPlane::writeTextFile: Could not write min/max value!");
	return -1;
    }

    int x, y;

    for (y = 0; y < numY(); y++) 
    {
	for (x = 0; x < numX(); x++) 
	{
	    double  value = this->value(x, y);
	    
	    if (fprintf(file,"%f\n",value) == EOF) 
	    {
		perror("BCPlane::writeTextFile: Could not write value!");
		fprintf(stderr,"   x: %d  y: %d\n", x, y);
		return -1;
	    }

	}
    }
    
    return 0;

} // writeTextFile


/**
readBinaryFile
    
        @author ?
 @date modified 9-10-95 by Kimberly Passarella Jones
*/
int
BCPlane::readBinaryFile(FILE* file)
{    
    if (fread(&_min_value, sizeof(_min_value), 1, file) != 1) 
    {
	perror("BCPlane::readBinaryFile: Could not get min_value!");
	return -1;
    }
    if (fread(&_max_value, sizeof(_max_value), 1, file) != 1) 
    {
	perror("BCPlane::readBinaryFile: Could not get max_value!");
	return -1;
    }
	
    int	first_value_read = 1;	
    int x, y;
	
    for (y = 0; y < numY(); y++) 
    {
	for (x = 0; x < numX(); x++) 
	{
	    double value;

	    if (fread(&value, sizeof(value), 1, file) != 1) 
	    {
		perror("BCPlane::readBinaryFile: Could not read value!");
		fprintf(stderr,"   x: %d  y: %d\n",x,y);
		return -1;
	    }

	    if (first_value_read) 
	    {
		_min_value = value;
		_max_value = value;
		first_value_read = 0;
	    }

	    if (value < _min_value) { _min_value = value; }

	    if (value > _max_value) { _max_value = value; }

	    setValue(x,y, value);

	    if (_timed)
	    {
		_sec[x][y] = 0;
		_usec[x][y] = 0;
	    }
	}
    }

    return 0;
}  // readBinaryFile


/**
writeBinaryFile
    
        @author ?
 @date modified 9-10-95 by Kimberly Passarella Jones
*/
int
BCPlane::writeBinaryFile(FILE* file)
{ 

    if (fwrite(&_min_value, sizeof(_min_value),1,file) != 1) 
    {
	perror("BCPlane::writeBinaryFile: Could not write min_value!");
	return -1;
    }
    if (fwrite(&_max_value, sizeof(_max_value), 1, file) != 1) 
    {
	perror("BCPlane::writeBinaryFile: Could not write max_value!");
	return -1;
    }

    int x, y;

    for (y = 0; y < numY(); y++) 
    {
	for (x = 0; x < numX(); x++) 
	{
	    double value = this->value(x, y);
	    
	    if (fwrite(&value,sizeof(value),1,file) != 1) 
	    {
		perror("BCPlane::writeBinaryFile: Could not write value!");
		fprintf(stderr,"   x: %d  y: %d\n", x, y);
		return -1;
	    }
	}
    }
    
    return 0;

} // writeBinaryFile


/**
readUNCAFile
    
        @author ?
 @date modified 9-10-95 by Kimberly Passarella Jones
*/
int
BCPlane::readUNCAFile(FILE* file)
{
    if (fscanf(file,"%lf %lf\n\n", &_min_value, &_max_value) != 2) 
    {
	perror("BCPlane::readUNCAFile: Could not get min/max value!");
	return -1;
    }
	
    int	first_value_read = 1;	
    int x, y;
	
    for (y = 0; y < numY(); y++) 
    {
	for (x = 0; x < numX(); x++) 
	{
	    long sec, usec;
	    double value;

	    if(( fscanf(file, "%ld %ld %lf\n", &sec, &usec, &value)) != 3)  
	    {
		perror("BCPlane::readUNCAFile: Could not read value!");
		fprintf(stderr,"   x: %d  y: %d\n", x, y);
		return -1;
	    }
	    if (first_value_read) 
	    {
		_min_value = value;
		_max_value = value;
		first_value_read = 0;
	    }

	    if (value < _min_value) { _min_value = value; }

	    if (value > _max_value) { _max_value = value; }

	    setValue(x,y, value);

	    if (_timed)
	    {
		_sec[x][y] = 0;
		_usec[x][y] = 0;
	    }
	}
    }

    return 0;
} // readUNCAFile


/**
writeUNCAFile
    
        @author ?
 @date modified 9-10-95 by Kimberly Passarella Jones
*/
int
BCPlane::writeUNCAFile(FILE* file)
{ 
    if (fprintf(file,"%f %f\n\n", _min_value, _max_value) == EOF) 
    {
	perror("BCPlane::writeUNCAFile: Could not write min/max value!");
	return -1;
    }

    int x, y;

    for (y = 0; y < numY(); y++) 
    {
	for (x = 0; x < numX(); x++) 
	{
	    long sec = 0;
	    long usec = 0;

	    if (_timed)
	    {
		sec = _sec[x][y];
		usec = _usec[x][y];		
	    }

	    double value = this->value(x, y);
	    //fprintf(stderr,"value = %lf,value[%d][d]= %f",value,this->value(x, y));  
	    if (fprintf(file,"%ld %ld %f\n", sec, usec, value) == EOF) 
	    {
		perror("BCPlane::writeUNCAFile: Could not write value!");
		fprintf(stderr,"   x: %d  y: %d\n", x, y);
		return -1;
	    }
	}
    }
    
    return 0;

} // writeUNCAFile


/**
readUNCBFile
    
        @author Kimberly Passarella Jones
 @date modified 9-10-95 by Kimberly Passarella Jones
*/
int     
BCPlane::readUNCBFile(FILE* file)
{
    if (fread(&_min_value, sizeof(_min_value), 1, file) != 1) 
    {
	perror("BCPlane::readUNCBFile: Could not get min_value!");
	return -1;
    }
    if (fread(&_max_value, sizeof(_max_value), 1, file) != 1) 
    {
	perror("BCPlane::readUNCBFile: Could not get max_value!");
	return -1;
    }
	
    int	first_value_read = 1;	
    int x, y;
	
    for (y = 0; y < numY(); y++) 
    {
	for (x = 0; x < numX(); x++) 
	{
	    double value;

	    if (fread(&value, sizeof(value), 1, file) != 1) 
	    {
		perror("BCPlane::readUNCBFile: Could not read value!");
		fprintf(stderr,"   x: %d  y: %d\n",x, y);
		return -1;
	    }

	    if (first_value_read) 
	    {
		_min_value = value;
		_max_value = value;
		first_value_read = 0;
	    }

	    if (value < _min_value) { _min_value = value; }

	    if (value > _max_value) { _max_value = value; }

	    setValue(x,y, value);

	    long sec;

	    if (fread(&sec, sizeof(sec), 1, file) != 1) 
	    {
		perror("BCPlane::readUNCBFile: Could not read sec!");
		fprintf(stderr,"   x: %d  y: %d\n", x, y);
		return -1;
	    }

	    if (_timed)
		_sec[x][y] = 0;

	    long usec;

	    if (fread(&usec, sizeof(usec), 1, file) != 1) 
	    {
		perror("BCPlane::readUNCBFile: Could not read usec!");
		fprintf(stderr,"   x: %d  y: %d\n", x, y);
		return -1;
	    }

	    if (_timed)
	       	_usec[x][y] = 0;

	    // Note! colorparam was a member of the old (pre-C++) grid structure. It is
	    // not a member of BCPlane. Hence, we read it, but do nothing with it.
	    double colorparam;
	    
	    if (fread(&colorparam,sizeof(colorparam),1,file) != 1) 
	    {
		perror("BCPlane::readUNCBFile: Could not read colorparam!");
		fprintf(stderr,"   x: %d  y: %d\n", x, y);
		return -1;
	    }
	}
    }

    return 0;

} // readUNCBFile


/**
readPPMorPGMFile
    XXX This sure doesn't look like it will read a PGM file,
		but it should read a PPM file.
        @author ?
 @date modified 9-10-95 by Kimberly Passarella Jones
*/
int 
BCPlane::readPPMorPGMFile(FILE *file, double scale)
{
   unsigned char *value = (unsigned char *)calloc(numX(), 3 * sizeof(unsigned char));

   int x,y;

   // Reverse Y traversal so image is not flipped vertically.
   for(y = numY() -1; y >=0; y-- ) 
   {
       fread(value, 3*sizeof(char), numX(), file );
       for (x = 0; x < numX(); x++ )
	   setValue(x,y, value[3*x] * scale);
   }
  
   free(value);

   return 0;

}  // readPPMorPGMFile


/**
writePPMFile
    Writes a greyscale ppm file (just data, no header). Data is 
                scaled from range minValue() - maxValue() to range 0 - 255
        @author ?
 @date modified 8/13/98 Aron Helser
*/
int 
BCPlane::writePPMFile(int file_descriptor)
{  
    char *value = (char *) calloc(numX(), 3 * sizeof(char));

    double scale = 254.0 / (maxValue() - minNonZeroValue());

    unsigned int val;

    int x, y;

    //printf("%f %f %f\n", minValue(), maxValue(), scale);
    // Reverse Y traversal so image is not flipped vertically.
    for(y = numY() -1; y >=0; y-- ) 
    {
	for(x = 0; x < numX(); x++ ) 
	{
            if (this->value(x,y) < minNonZeroValue()) {
               val = 0;
            } else {
               val = 1 + 
                     (unsigned)((this->value(x, y) - minNonZeroValue()) * scale);
            }
	    value[x*3] = 
		value[x*3+1] = 
                    value[x*3+2] = val;
	    //printf("%d ", value[x*3]);
	}
	//printf("*****\n");
	if (write(file_descriptor, value, 3 * numX()) == -1 ) 
	{
	    perror("BCPlane::writePPMFile: Could not write values!");
	    return -1;
	}
    }

    free(value);

    return 0;

}  // writePPMFile

/**
writeRawByteFile
    Writes out raw bytes, where the range giving by minval and maxval 
                is scaled to the range 0 to 255
        @author Aron Helser
 @date modified 8/13/98 Aron helser
*/
int 
BCPlane::writeRawByteFile(int file_descriptor, double minval, double maxval)
{  
    char *value = (char *) calloc(numY(), sizeof(char));

    double scale = 255.0 / (maxval - minval);

    int x, y;

    //printf("%f %f %f\n", minval, maxval, scale);
    for(x = 0; x < numX(); x++ ) 
    {
	for(y = 0; y < numY(); y++ ) 
	{
	    value[y] = (unsigned)((this->value(x, y) - minval) * scale);
	    //printf("%d ", value[y]);
	}
	//printf("*****\n");
	if (write(file_descriptor, value, numY()) == -1 ) 
	{
	    perror("BCPlane::writeRawByteFile: Could not write values!");
	    return -1;
	}
    }

    free(value);

    return 0;

}  // writeRawByteFile


int BCPlane::writeSPIPFile(FILE *file)
{  
    double scale =  2 / (maxValue() - minValue());
    short value;

    int x, y;

    if( fseek(file, 2048, 0) ) {
       fprintf(stderr," fseek in writeSPIPFile failed.\n");
       return -1;
    }
    fprintf(stderr,"current position is %ld\n",ftell(file));   
    for(x = 0; x < numX(); x++ ) {
      for(y = 0; y < numY(); y++ ) {
           value=(short) ( (( (this->value(x, y) - minValue())*scale)-1 ) * 32767);
	   if( fwrite(&value, 2, 1,file)!=1) {
              fprintf(stderr,"writeSPIPFile failed\n");
           }
      }
    }
    return 0;

}  // writeSPIPFile


int BCPlane::readSPIPFile(FILE *file, double max_value) 
{  short value;  // short is two bytes
   double real_value, scale; 
   int x, y;
    
   if( fseek(file,2048,0) ) {
     fprintf(stderr," fseek in readSPIPFile failed.\n");
     return -1;
    }
   scale =   max_value / 2.0;
//fprintf(stderr," numx= %d, numy= %d\n", numX(),numY()); 
//fprintf(stderr," max_value= %e,scale = %e\n",max_value,scale ); 

   for (x = 0; x < numX(); x++) {
        for (y = 0; y < numY(); y++) {
            if (fread(&value, sizeof(value), 1, file) != 1) { 
                perror("BCPlane::readSPIPFile: Could not read value!");
                fprintf(stderr,"   x: %d  y: %d\n",x,y);
                return -1;
            }
            real_value= (value/32767.0 + 1 ) * scale; 
            setValue(x,y, real_value);
        }
   }
   setMinAttainableValue(0);
   setMaxAttainableValue(max_value);
   return 0;
}

/**
readNanoscopeFileWithoutHeader
    
        @author ?
 @date modified 9-10-95 by Kimberly Passarella Jones
*/
int BCPlane::readNanoscopeFileWithoutHeader(FILE* file)
{
    int	first_value_read = 1;
    int x, y;

    for (x = 0; x < numX(); x++) {
	for (y = 0; y < numY(); y++) {
	    unsigned char bytes[2];
	    int	int_value;
	    double value;
	    
	    if (fread(bytes, sizeof(char), 2, file) != 2) {
		perror("BCPlane::readNanoscopeFileWithoutHeader: Could not read value!");
		fprintf(stderr,"       (x = %d, y = %d)\n",x, y);
		return -1;
	    }
	    
	    if (bytes[1] < 128) { // just read a non-negative number 
		int_value = bytes[0] + 256 * bytes[1];
	    } else {  // just read a negative 2's complement number 
		int_value = (bytes[0] + 256 * bytes[1]) - 65536;
	    }
	    
	    value = 10 * (double)(int_value) / (32768);
	    
	    if (first_value_read) {
		_min_value = value;
		_max_value = value;
		first_value_read = 0;
	    }

	    if (value < _min_value) { _min_value = value; }
	    if (value > _max_value) { _max_value = value; }

	    setValue(x,y, value);

	    if (_timed) {
		_sec[x][y] = 0;
		_usec[x][y] = 0;
	    }
	}
    }
    setMinAttainableValue(_min_value);
    setMaxAttainableValue(_max_value);
    return 0;
}

/**
readAsciiRHKFile
   
        @author seeger
 @date modified 6/18/99 by seeger
*/
int BCPlane::readAsciiRHKFile(FILE* file, double z_offset_nm, double z_scale_pm)
{
    int first_value_read = 1;
    int x, y;
    float fval;

    for (y = 0; y < numY(); y++) {
        for (x = 0; x < numX(); x++) {
            short value;

            if (fscanf( file, "%hd", &value) != 1) {
                // skip ahead to next number
                fscanf(file, "%*[^-+0-9]");

                if (fscanf( file, "%hd", &value) != 1) {
                    perror("BCPlane::readAsciiRHKFile: could not read value!");
                    fprintf(stderr,"   x: %d  y: %d\n",x, y);
                    return -1;
                }
            }

	    fval = value*z_scale_pm*0.001 + z_offset_nm;
            setValue(x,y, fval);

            if (first_value_read) {
                _min_value = fval;
                _max_value = fval;
                first_value_read = 0;
            }

            if (fval < _min_value) { _min_value = fval; }
            if (fval > _max_value) { _max_value = fval; }


            if (_timed) {
                _sec[x][y] = 0;
                _usec[x][y] = 0;
            }
        }
    }
    setMinAttainableValue(_min_value);
    setMaxAttainableValue(_max_value);

    printf("Zscale=%g Zoffset=%g\n", z_scale_pm*0.001, z_offset_nm);
    printf("min_z %g max_z %g\n", _min_value, _max_value);

    return 0;

} // readAsciiRHKFile


/**
readAsciiNanoscopeFile
    
        @author ?
 @date modified 9-10-95 by Kimberly Passarella Jones
*/
int BCPlane::readAsciiNanoscopeFile(FILE* file)
{
    int	first_value_read = 1;
    int x, y;

    for (x = 0; x < numX(); x++) {
	for (y = 0; y < numY(); y++) {
	    short value;
	    
	    if (fscanf( file, "%hd", &value) != 1) {
		// skip ahead to next number
		fscanf(file, "%*[^-+0-9]");
		
		if (fscanf( file, "%hd", &value) != 1) {
		    perror("BCPlane::readAsciiNanoscopeFile: could not read value!");
		    fprintf(stderr,"   x: %d  y: %d\n",x, y);
		    return -1;
		}
	    } 

	    setValue(x,y, _grid->transform(&value, _image_mode, _scale) );

	    if (first_value_read) {
		_min_value = this->value(x, y);
		_max_value = this->value(x, y);
		first_value_read = 0;
	    }
	        
	    if (this->value(x, y) < _min_value) { _min_value = value; }
	    if (this->value(x, y) > _max_value) { _max_value = value; }


	    if (_timed) {
		_sec[x][y] = 0;
		_usec[x][y] = 0;
	    }
	}		
    }
    setMinAttainableValue(_min_value);
    setMaxAttainableValue(_max_value);

    return 0;
    
} // readAsciiNanoscopeFile


/**
readBinaryNanoscopeFile
    
        @author ?
 @date modified 9-10-95 by Kimberly Passarella Jones
*/
int
BCPlane::readBinaryNanoscopeFile(FILE* file)
{
    int	first_value_read = 1;
    int x, y;
	int ret;
    
    for (y = 0; y < numY(); y++) {
	short* value = (short*) calloc(numX(), sizeof(short));

	ret = fread(value, sizeof(short), numX(), file);
	if (numX() != ret) {
	    perror("BCPlane::readBinaryNanoscopeFile: could not read value!");
	    fprintf(stderr,"   x: 0-%d  y: %d,  ret=%d\n", numX(), y, ret);
	    return -1;
	}

	for (x = 0; x < numX(); x++) {
	    setValue(x,y, _grid->transform(value + x, _image_mode, _scale) );

	    if (first_value_read) {
		_min_value = this->value(x, y);
		_max_value = this->value(x, y);
		first_value_read = 0;
	    }
	        
	    if (this->value(x, y) < _min_value) { _min_value = this->value(x, y); }
	    if (this->value(x, y) > _max_value) { _max_value = this->value(x, y); }

	    if (_timed) {
		_sec[x][y] = 0;
		_usec[x][y] = 0;
	    }
	}

	free (value);
    }
    setMinAttainableValue(_min_value);
    setMaxAttainableValue(_max_value);

    return 0;
    
} // readBinaryNanoscopeFile


/**
writeNCFile
    Writes a Numerically-controlled machine tool file that will
	mill out the surface.  It assumes that the grid has the header
	and footer to place the tool above the surface, start and stop
	the spindle, and set the milling rate.
    This routine will scale the surface so that it fits within the
	size for all dimensions (x,y and z).  The surface is uniformly
	scaled to match.  The user specifies an offset (zoff) that
	this code uses to drop the entire design down into the material.
	The highest surface point will map to Z=-zoff.
	X and Y are assumed to be centered, with the Z=0 touching the
	material, when the program starts.

        @author Russell Taylor
 @date modified 1/3/98 Russell Taylor
         notes: This routine should be modified to set the number of sweeps
			that the tool will make in the slow-scan direction.
	    *** Some provision should be made for tool clearance at the edges
			of the milled area.
	    It is possible that some sort of spline interpolation will be
			preferable to piecewise linear for future versions.
*/

int 
BCPlane::writeNCFile(FILE* file, double sizex, double sizey, double sizez,
	double maxCut, double zoff, int roughskip)
{
    double scalex = sizex / (maxX() - minX());
    double scaley = sizey / (maxY() - minY());
    double scalez = (sizez-zoff) / (maxValue() - minValue());

    double bestscale = min( scalex, min(scaley,scalez) );

    double centerx = (maxX() + minX()) / 2;
    double centery = (maxY() + minY()) / 2;

    double Depth;	// Current maximum depth of cut

    // Make cuts until we are sure that we are below the maximum depth
    // for the entire surface.  Only do maxcut depth at each step.
    // DO check for the ability to skip each line as we go to decrease
    // the cutting time.
    for (Depth = -maxCut; Depth > (-zoff-sizez-maxCut); Depth -=maxCut) {
	writeNCPass(file, bestscale, centerx, centery, Depth, zoff, maxCut,
		roughskip, 1);
    }

    // Make the final "polishing" pass over the surface.  DO NOT skip
    // rows that are completely above the cutting distance, since we are
    // doing more lines in this pass than in the previous rough cuts.
    // Only do the polishing pass if we were skipping lines in the rough
    // cuts.
    if (roughskip > 1) {
	writeNCPass(file, bestscale, centerx, centery, Depth, zoff, maxCut,
		1, 0);
    }

    return 0;

}  // writeNCFile


/**
writeNCPass
	Writes one pass of an NC miling program into the file.  It is told
	the current depth.  It skips 'increment' lines from the plane each
	time back and forth; this allows rough cutting of higher levels.
	It checks to see if it can skip a line if it is told to; this can
	decrease milling time, but should not be done on the final
	polishing pass.

        @author Russell Taylor
 @date modified 1/3/98 Russell Taylor
	    *** Some provision should be made for tool clearance at the edges
			of the milled area.
*/
int
BCPlane::writeNCPass(FILE *file, double bestscale,
	double centerx, double centery,
	double Depth, double zoff, double maxCut, int increment, int checkskip)
{
    int x, y;
    int ystart, ystop, yinc;
    int direction = 1;		// Which direction to sweep?

    for(x = 0; x < numX(); x+= increment ) {

	// See if we need to cut this line at all.  If all of the
	// values in the line are at least a maxCut above the
	// current Depth, then we will have already cut them so
	// we can skip the line entirely.

	int	numToDo = 0;
	for (y = 0; y < numY(); y++) {
		double z = ((this->value(x,y) - maxValue()) * bestscale) - zoff;
		if ( z < (Depth + maxCut) ) {
			numToDo++;
		}
	}
	if ( checkskip && (numToDo == 0) ) {
		continue;	// Skip this row
	}

	// Trace out the line, being sure not to penetrate more than the
	// specified maximum cut distance at any point
	// The scan is boustrophedonic (up one line and back the next)
	// to minimize the cutting time.

	if ( direction == 1 ) {
		ystart = 0;
		ystop = numY();	// Stop when y == this
		yinc = 1;
	} else {
		ystart = numY() - 1;
		ystop = -1;
		yinc = -1;
	}
	direction = -direction;	// Scan the other way the next time

	// Go to the beginning of the line, above the surface
	// (assumed above to start)
	if (fprintf(file, "G00X%gY%g\n",
		(xInWorld(x) - centerx) * bestscale,
		(yInWorld(ystart) - centery) * bestscale) == EOF) {
	}

	for(y = ystart; y != ystop; y+= yinc) {
		double z = ((this->value(x,y) - maxValue()) * bestscale) - zoff;
		if (fprintf(file, "G01Y%gZ%g\n",
			(yInWorld(y) - centery) * bestscale,
			max(Depth, z)) == EOF) {
		    perror("BCPlane::writeNCFile: Error writing value to file");
		    return -1;
		}
	}

	// Pop back above the surface after each line, even if
	// we are going right back down, in case we need to skip
	// the next line.
	if (fprintf(file, "G01Z0.1\n") == EOF) {
	    perror("BCPlane::writeNCFile: Error writing line end to file");
	    return -1;
	}

    }
	return 0;
}


/**
setTime
    
        @author Kimberly Passarella Jones
 @date modified 9-10-95 by Kimberly Passarella Jones
*/
void CPlane::setTime(int /*x*/, int /*y*/, long /*sec*/, long /*usec*/)
{
    // Does nothing: this plane does not store time
} // setTime


/**
CPlane --> constructor
    
        @author Kimberly Passarella Jones
 @date modified 9-10-95 by Kimberly Passarella Jones
*/
CPlane::CPlane(BCString name, BCString units, int nx, int ny) :
	BCPlane(name, units, nx, ny)
{
    _timed = NOT_TIMED;

    _sec = NULL;
    _usec = NULL;

    int x, y;

    for (x = 0; x < _num_x; x++) 
	for (y = 0; y < _num_y; y++)
	    _value[x * _num_y + y] = 0.0;
}


/**
CPlane --> constructor
    
        @author Kimberly Passarella Jones
 @date modified 9-10-95 by Kimberly Passarella Jones
*/
CPlane::CPlane(CPlane* plane) : BCPlane(plane)
{
    _timed = NOT_TIMED;

    _sec = NULL;
    _usec = NULL;    

    int x, y;
    
    for (x = 0; x < _num_x; x++) 
	for (y = 0; y < _num_y; y++)
	    _value[x * _num_y + y] = 0.0;

    for (x = 0; x < plane->numX(); x++) 
	for (y = 0; y < plane->numY(); y++) 
	    _value[x * _num_y + y] =  plane->value(x, y);
}

CPlane::CPlane (CPlane * plane, int newX, int newY) :
    BCPlane (plane, newX, newY) {

    _timed = NOT_TIMED;

    _sec = NULL;
    _usec = NULL;    

    int x, y;
    
    for (x = 0; x < newX; x++) 
	for (y = 0; y < newY; y++)
	    _value[x * newY + y] = 0.0;

    for (x = 0; x < newX; x++) 
	for (y = 0; y < newY; y++) 
	    _value[x * newY + y] =  plane->value(x, y);
}


/**
~CPlane --> destructor
    
        @author Kimberly Passarella Jones
 @date modified 9-10-95 by Kimberly Passarella Jones
*/
CPlane::~CPlane() 
{
}


/**
setTime
    
        @author Kimberly Passarella Jones
 @date modified 9-10-95 by Kimberly Passarella Jones
*/
void
CTimedPlane::setTime(int x, int y, long sec, long usec)
{
    _sec[x][y] = sec;
    _usec[x][y] = usec;
} // setTime


//virtual
void CTimedPlane::computeMinMax (void) {
  int x, y;

  _max_value = -1.0e33;
  _min_value = 1.0e33;

  for (x = 0; x < numX(); x++) {
    for (y = 0; y < numY(); y++) {
      double value =  this->value(x, y);

      // Original algorithm
      // if ((x == 0) && (y == 0)) {
      //   _min_value = value;
      //   _max_value = value;
      // }

      if (_sec[x][y] && _usec[x][y]) {
        if (value < _min_value) { _min_value = value; }
        if (value > _max_value) { _max_value = value; }
      }
    }
  }

}


/**
CTimedPlane --> constructor
    
        @author Kimberly Passarella Jones
 @date modified 9-10-95 by Kimberly Passarella Jones
*/
CTimedPlane::CTimedPlane(BCString name, BCString units, int nx, int ny) :
	BCPlane(name, units, nx, ny)
{
    _timed = TIMED;

    _sec = new long*[_num_x];
    _usec = new long*[_num_x];

    int x, y;

    for (x = 0; x < _num_x; x++)
    {
	_sec[x] = new long[_num_y];
	_usec[x] = new long[_num_y];

	for (y = 0; y < _num_y; y++) 
	{
	    _value[x * _num_y + y] =  0.0;
	    _sec[x][y] = 0;
	    _usec[x][y] = 0;
	}
    }
}


/**
CTimedPlane --> constructor
    
        @author Kimberly Passarella Jones
 @date modified 9-10-95 by Kimberly Passarella Jones
*/
CTimedPlane::CTimedPlane(CTimedPlane* plane) : BCPlane(plane)
{
    _timed = TIMED;

    _sec = new long*[_num_x];
    _usec = new long*[_num_x];

    int x, y;

    for (x = 0; x < _num_x; x++) 
    {
	_sec[x] = new long[_num_y];
	_usec[x] = new long[_num_y];	

	for (y = 0; y < _num_y; y++)
	{
	    _value[x * _num_y + y] =  0.0;
	    _sec[x][y] = 0;
	    _usec[x][y] = 0; 
	}
    }

    for (x = 0; x < plane->numX(); x++) 
    {
	for (y = 0; y < plane->numY(); y++) 
	{
	    _value[x * _num_y + y] = plane->value(x, y);
	    _sec[x][y] = plane->_sec[x][y];
	    _usec[x][y] = plane->_usec[x][y];
	}
    }
}


CTimedPlane::CTimedPlane(CTimedPlane* plane, int newX, int newY) :
   BCPlane(plane, newX, newY)
{
    _timed = TIMED;

    _sec = new long*[newX];
    _usec = new long*[newX];

    int x, y;

    for (x = 0; x < newX; x++) 
    {
	_sec[x] = new long[newY];
	_usec[x] = new long[newY];	

	for (y = 0; y < newY; y++)
	{
	    _value[x * newY + y] =  0.0;
	    _sec[x][y] = 0;
	    _usec[x][y] = 0; 
	}
    }

    for (x = 0; x < newX; x++) 
    {
	for (y = 0; y < newY; y++) 
	{
	    _value[x * newY + y] = plane->value(x, y);
	    _sec[x][y] = plane->_sec[x][y];
	    _usec[x][y] = plane->_usec[x][y];
	}
    }
}


/**
~CTimedPlane --> destructor
    
        @author Kimberly Passarella Jones
 @date modified 9-10-95 by Kimberly Passarella Jones
*/
CTimedPlane::~CTimedPlane()
{
    int x;
    
    for (x = _num_x; x > 0; --x) {
	delete [] _sec[x-1];
	delete [] _usec[x-1];	
    }
    
    delete [] _sec;
    delete [] _usec;
}


ostream& operator << (ostream& os, BCPlane* plane)
{
    os << "***************************************" << endl;
    os << "name = " << plane->_dataset << endl;
    os << "num_x = " << plane->numX() << endl;
    os << "num_y = " << plane->numY() << endl;
    os << "min_x = " << plane->minX() << endl;
    os << "max_x = " << plane->maxX() << endl;
    os << "min_y = " << plane->minY() << endl;
    os << "max_y = " << plane->maxY() << endl;
    os << "min_value = " << plane->_min_value << endl;
    os << "max_value = " << plane->_max_value << endl;
    os << "scale = " << plane->_scale << endl;
    os << "***************************************" << endl;
    
    return os;
}

