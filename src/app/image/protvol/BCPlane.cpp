#ifdef	_WIN32
#include <io.h>
#endif
#include <stdlib.h> // for exit()
#include <malloc.h> // for calloc() and free()
#include <errno.h> // for perror()

#include "BCPlane.h"
#include "BCGrid.h"
#include "BCDebug.h"


/******************************************************************************\
@setValue
--------------------------------------------------------------------------------
   description: 
        author: Kimberly Passarella Jones
 last modified: 7/2/96 by Russ Taylor
\******************************************************************************/
void
BCPlane::setValue(int x, int y, float value)
{
    _value[x][y] = value;
    _modified = 1;
    _modified_nz = 1;

    for (int i = 0; i < _numcallbacks; i++)
	_callbacks[i].callback(this, x,y, _callbacks[i].userdata);
}

/******************************************************************************\
@setTime
--------------------------------------------------------------------------------
   description: Does nothing.  Allows this routine to be called for all
		planes, even untimed ones.
        author: Russ Taylor
 last modified: 5/3/96 by Russ Taylor
\******************************************************************************/
void
BCPlane::setTime(int x, int y, long sec, long usec)
 { x = x; y = y; sec = sec; usec = usec; }


/******************************************************************************\
@minValue
--------------------------------------------------------------------------------
   description: 
        author: Kimberly Passarella Jones
 last modified: 9-10-95 by Kimberly Passarella Jones
\******************************************************************************/
double
BCPlane::minValue()
{
    if (!_modified)
	return minValueComputedLast();
    else
    {
	int x, y;

	for (x = 0; x < numX(); x++) 
	{
	    for (y = 0; y < numY(); y++) 
	    {
		double value =  _value[x][y];
	    
		if ((x == 0) && (y == 0))
		{
		    _min_value = value;
		    _max_value = value;
		}
	    
		if (value < _min_value) { _min_value = value; }
		if (value > _max_value) { _max_value = value; }
	    }
	}
	
	if (_max_value == _min_value) // no real data has been stored in the grid
	{
	    _max_value = -1.0e33;
	    _min_value = 1.0e33;
	}
	
	_modified = 0;
	return _min_value;
    }
} // minValue


/******************************************************************************\
@maxValue
--------------------------------------------------------------------------------
   description: 
        author: Kimberly Passarella Jones
 last modified: 9-10-95 by Kimberly Passarella Jones
\******************************************************************************/
double
BCPlane::maxValue()
{
    if (!_modified)
	return maxValueComputedLast();
    else
    {
	int x, y;

    	for (x = 0; x < numX(); x++) 
	{
	    for (y = 0; y < numY(); y++) 
	    {
		double value = _value[x][y];
		
		if ((x == 0) && (y == 0))
		{
		    _min_value = value;
		    _max_value = value;
		}
	    
		if (value < _min_value) { _min_value = value; }
		if (value > _max_value) { _max_value = value; }
	    }
	}

	if (_max_value == _min_value) // no real data has been stored in the grid
	{
	    _max_value = -1.0e33;
	    _min_value = 1.0e33;
	}
	    
	_modified = 0;
	return _max_value;
    }
} // maxValue

/******************************************************************************\
@minNonZeroValue
--------------------------------------------------------------------------------
   description: Minimum nonzero value in plane. Useful if not all data 
                has been filled in yet. 
        author: Aron Helser
 last modified: 8-19-98 Aron Helser
\******************************************************************************/
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
		double value =  _value[x][y];
	    
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


/******************************************************************************\
@maxNonZeroValue
--------------------------------------------------------------------------------
   description: Maximum nonzero value in plane. Useful if not all data 
                has been filled in yet. 
        author: Aron Helser
 last modified: 8-19-98 Aron Helser
\******************************************************************************/
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
		double value = _value[x][y];
		
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


/******************************************************************************\
@level
--------------------------------------------------------------------------------
   description: This method "levels out" the grid by subtracting from each
                of its values the average plane.
        author: Kimberly Passarella Jones
 last modified: 9-10-95 by Kimberly Passarella Jones
\******************************************************************************/
void
BCPlane::level()
{	
    BCDebug debug("BCPlane::level", PLANE_CODE);

    debug.warn("collecting statistics on grid" );

    int x, y;

    double dx = 0.0;
    double dy = 0.0;
    double mean = 0.0;

    // compute the mean and the average slopes in each direction
    for (x = 1; x <  numX(); x++)
	for (y = 1; y < numY(); y++)
	{
	    dx += (value(x, y) - value(x-1, y));
	    dy += (value(x, y) - value(x, y-1));
	    mean += value(x, y);
	}

    double temp = (numX() - 1) * (numY() - 1);
    dx /= temp;
    dy /= temp;
    mean /= temp;

    debug.watch("dx", dx);
    debug.watch("dy", dy);
    debug.watch("mean", mean);

    // compute the base (the height of the point (0,0))
    double base = mean - dx * (numX() - 1)/2.0 - dy * (numY() - 1)/2.0;

    // subtract the plane
    debug.warn("correcting grid");
    for (x = 0; x < numX(); x++)
	for (y= 0; y < numY(); y++)
	    setValue(x,y,  value(x,y) - (base + dx*x + dy*y) );

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
  
\******************************************************************************/


/******************************************************************************\
@BCPlane --> constructor
--------------------------------------------------------------------------------
   description: 
        author: Kimberly Passarella Jones
 last modified: 9-10-95 by Kimberly Passarella Jones
\******************************************************************************/
BCPlane::BCPlane(BCString name, BCString units, int nx, int ny)
{
    int x;

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
    if ( (_value = new float*[_num_x]) == NULL) {
	fprintf(stderr,"BCPlane::BCPlane(): new failed!\n");
	exit(-1);
    }
    for (x = 0; x < _num_x; x++) {
	if ( (_value[x] = new float[_num_y]) == NULL) {
		fprintf(stderr,"BCPlane::BCPlane(): new failed!\n");
		exit(-1);
	}
    }
}


/******************************************************************************\
@BCPlane --> constructor
--------------------------------------------------------------------------------
   description: 
        author: Kimberly Passarella Jones
 last modified: 9-10-95 by Kimberly Passarella Jones
\******************************************************************************/
BCPlane::BCPlane(BCPlane* plane)
{
    int x;

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

    // Allocate space for the values array
    if ( (_value = new float*[_num_x]) == NULL) {
	fprintf(stderr,"BCPlane::BCPlane(): new failed!\n");
	exit(-1);
    }
    for (x = 0; x < _num_x; x++) {
	if ( (_value[x] = new float[_num_y]) == NULL) {
		fprintf(stderr,"BCPlane::BCPlane(): new failed!\n");
		exit(-1);
	}
    }
}
    

/******************************************************************************\
@~BCPlane --> destructor
--------------------------------------------------------------------------------
   description: 
        author: Kimberly Passarella Jones
 last modified: 9-10-95 by Kimberly Passarella Jones
\******************************************************************************/
BCPlane::~BCPlane()
{
    int x;

    // Free the space taken by the values array
    // in the reverse order it was allocated
    for (x = _num_x-1; x >= 0; x--) {
	delete [] _value[x];
    }
    delete [] _value;
}


/******************************************************************************\
@readTextFile
--------------------------------------------------------------------------------
   description: 
        author: ?
 last modified: 9-10-95 by Kimberly Passarella Jones
\******************************************************************************/
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


/******************************************************************************\
@writeTextFile
--------------------------------------------------------------------------------
   description: 
        author: ?
 last modified: 9-10-95 by Kimberly Passarella Jones
\******************************************************************************/
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
	    double  value = _value[x][y];
	    
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


/******************************************************************************\
@readBinaryFile
--------------------------------------------------------------------------------
   description: 
        author: ?
 last modified: 9-10-95 by Kimberly Passarella Jones
\******************************************************************************/
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


/******************************************************************************\
@writeBinaryFile
--------------------------------------------------------------------------------
   description: 
        author: ?
 last modified: 9-10-95 by Kimberly Passarella Jones
\******************************************************************************/
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
	    double value = _value[x][y];
	    
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


/******************************************************************************\
@readUNCAFile
--------------------------------------------------------------------------------
   description: 
        author: ?
 last modified: 9-10-95 by Kimberly Passarella Jones
\******************************************************************************/
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


/******************************************************************************\
@writeUNCAFile
--------------------------------------------------------------------------------
   description: 
        author: ?
 last modified: 9-10-95 by Kimberly Passarella Jones
\******************************************************************************/
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

	    double value = _value[x][y];
	    //fprintf(stderr,"value = %lf,value[%d][d]= %f",value,_value[x][y]);  
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


/******************************************************************************\
@readUNCBFile
--------------------------------------------------------------------------------
   description: 
        author: Kimberly Passarella Jones
 last modified: 9-10-95 by Kimberly Passarella Jones
\******************************************************************************/
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


/******************************************************************************\
@readPPMorPGMFile
--------------------------------------------------------------------------------
   description: XXX This sure doesn't look like it will read a PGM file,
		but it should read a PPM file.
        author: ?
 last modified: 9-10-95 by Kimberly Passarella Jones
\******************************************************************************/
int 
BCPlane::readPPMorPGMFile(FILE *file, double scale)
{  
   unsigned char *value = (unsigned char *)calloc(numY(), 3 * sizeof(unsigned char));

   int x,y;

   for (x = 0; x < numX(); x++) 
   {
       fread(value, 3*sizeof(char), numY(), file );
       for (y = 0; y < numY(); y++ )
	   setValue(x,y, value[3*y] * scale);
   }
  
   free(value);

   return 0;

}  // readPPMorPGMFile


/******************************************************************************\
@writePPMFile
--------------------------------------------------------------------------------
   description: Writes a greyscale ppm file (just data, no header). Data is 
                scaled from range minValue() - maxValue() to range 0 - 255
        author: ?
 last modified: 8/13/98 Aron Helser
\******************************************************************************/
int 
BCPlane::writePPMFile(int file_descriptor)
{  
    char *value = (char *) calloc(numY(), 3 * sizeof(char));

    double scale = 255.0 / (maxValue() - minValue());

    int x, y;

    //printf("%f %f %f\n", minValue(), maxValue(), scale);
    for(x = 0; x < numX(); x++ ) 
    {
	for(y = 0; y < numY(); y++ ) 
	{
	    value[y*3] = 
		value[y*3+1] = 
		    value[y*3+2] = 
			(unsigned)((_value[x][y] - minValue()) * scale);
	    //printf("%d ", value[y*3]);
	}
	//printf("*****\n");
	if (write(file_descriptor, value, 3 * numY()) == -1 ) 
	{
	    perror("BCPlane::writePPMFile: Could not write values!");
	    return -1;
	}
    }

    free(value);

    return 0;

}  // writePPMFile

/******************************************************************************\
@writeRawByteFile
--------------------------------------------------------------------------------
   description: Writes out raw bytes, where the range giving by minval and maxval 
                is scaled to the range 0 to 255
        author: Aron Helser
 last modified: 8/13/98 Aron helser
\******************************************************************************/
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
	    value[y] = (unsigned)((_value[x][y] - minval) * scale);
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
           value=(short) ( (( (_value[x][y] -minValue())*scale)-1 ) * 32767);
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
/******************************************************************************\
@readNanoscopeFileWithoutHeader
--------------------------------------------------------------------------------
   description: 
        author: ?
 last modified: 9-10-95 by Kimberly Passarella Jones
\******************************************************************************/
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


/******************************************************************************\
@readAsciiNanoscopeFile
--------------------------------------------------------------------------------
   description: 
        author: ?
 last modified: 9-10-95 by Kimberly Passarella Jones
\******************************************************************************/
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
		_min_value = _value[x][y];
		_max_value = _value[x][y];
		first_value_read = 0;
	    }
	        
	    if (_value[x][y] < _min_value) { _min_value = value; }
	    if (_value[x][y] > _max_value) { _max_value = value; }


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


/******************************************************************************\
@readBinaryNanoscopeFile
--------------------------------------------------------------------------------
   description: 
        author: ?
 last modified: 9-10-95 by Kimberly Passarella Jones
\******************************************************************************/
int
BCPlane::readBinaryNanoscopeFile(FILE* file)
{
    BCDebug debug("BCPlane::readBinaryNanoscopeFile", PLANE_CODE);

    int	first_value_read = 1;
    int x, y;
    
    for (y = 0; y < numY(); y++) {
	short* value = (short*) calloc(numX(), sizeof(short));

	if (numX() != fread(value, sizeof(short), numX(), file)) {
	    perror("BCPlane::readBinaryNanoscopeFile: could not read value!");
	    fprintf(stderr,"   x: %d  y: %d\n", x, y);
	    return -1;
	}

	for (x = 0; x < numX(); x++) {
	    setValue(x,y, _grid->transform(value + x, _image_mode, _scale) );

	    if (first_value_read) {
		_min_value = _value[x][y];
		_max_value = _value[x][y];
		first_value_read = 0;
	    }
	        
	    if (_value[x][y] < _min_value) { _min_value = _value[x][y]; }
	    if (_value[x][y] > _max_value) { _max_value = _value[x][y]; }

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


/******************************************************************************\
@setTime
--------------------------------------------------------------------------------
   description: 
        author: Kimberly Passarella Jones
 last modified: 9-10-95 by Kimberly Passarella Jones
\******************************************************************************/
void CPlane::setTime(int x, int y, long sec, long usec)
{
    x = x; y = y; sec = sec; usec = usec;
    // Does nothing: this plane does not store time
} // setTime


/******************************************************************************\
@CPlane --> constructor
--------------------------------------------------------------------------------
   description: 
        author: Kimberly Passarella Jones
 last modified: 9-10-95 by Kimberly Passarella Jones
\******************************************************************************/
CPlane::CPlane(BCString name, BCString units, int nx, int ny) :
	BCPlane(name, units, nx, ny)
{
    BCDebug debug("CPlane::CPlane", PLANE_CODE);

    _timed = NOT_TIMED;

    _sec = NULL;
    _usec = NULL;

    int x, y;

    for (x = 0; x < _num_x; x++) 
	for (y = 0; y < _num_y; y++)
	    _value[x][y] = 0.0;
}


/******************************************************************************\
@CPlane --> constructor
--------------------------------------------------------------------------------
   description: 
        author: Kimberly Passarella Jones
 last modified: 9-10-95 by Kimberly Passarella Jones
\******************************************************************************/
CPlane::CPlane(CPlane* plane) : BCPlane(plane)
{
    BCDebug debug("CPlane::CPlane", PLANE_CODE);

    _timed = NOT_TIMED;

    _sec = NULL;
    _usec = NULL;    

    int x, y;
    
    for (x = 0; x < _num_x; x++) 
	for (y = 0; y < _num_y; y++)
	    _value[x][y] = 0.0;

    for (x = 0; x < plane->numX(); x++) 
	for (y = 0; y < plane->numY(); y++) 
	    _value[x][y] =  plane->_value[x][y];
}


/******************************************************************************\
@~CPlane --> destructor
--------------------------------------------------------------------------------
   description: 
        author: Kimberly Passarella Jones
 last modified: 9-10-95 by Kimberly Passarella Jones
\******************************************************************************/
CPlane::~CPlane() 
{
    BCDebug debug("CPlane::~CPlane", PLANE_CODE);
}


/******************************************************************************\
@setTime
--------------------------------------------------------------------------------
   description: 
        author: Kimberly Passarella Jones
 last modified: 9-10-95 by Kimberly Passarella Jones
\******************************************************************************/
void
CTimedPlane::setTime(int x, int y, long sec, long usec)
{
    _sec[x][y] = sec;
    _usec[x][y] = usec;
} // setTime


/******************************************************************************\
@CTimedPlane --> constructor
--------------------------------------------------------------------------------
   description: 
        author: Kimberly Passarella Jones
 last modified: 9-10-95 by Kimberly Passarella Jones
\******************************************************************************/
CTimedPlane::CTimedPlane(BCString name, BCString units, int nx, int ny) :
	BCPlane(name, units, nx, ny)
{
    BCDebug debug("CTimedPlane::CTimedPlane", PLANE_CODE);

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
	    _value[x][y] =  0.0;
	    _sec[x][y] = 0;
	    _usec[x][y] = 0;
	}
    }
}


/******************************************************************************\
@CTimedPlane --> constructor
--------------------------------------------------------------------------------
   description: 
        author: Kimberly Passarella Jones
 last modified: 9-10-95 by Kimberly Passarella Jones
\******************************************************************************/
CTimedPlane::CTimedPlane(CTimedPlane* plane) : BCPlane(plane)
{
    BCDebug debug("CTimedPlane::CTimedPlane", PLANE_CODE);
    
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
	    _value[x][y] =  0.0;
	    _sec[x][y] = 0;
	    _usec[x][y] = 0; 
	}
    }

    for (x = 0; x < plane->numX(); x++) 
    {
	for (y = 0; y < plane->numY(); y++) 
	{
	    _value[x][y] =  plane->_value[x][y];
	    _sec[x][y] = plane->_sec[x][y];
	    _usec[x][y] = plane->_usec[x][y];
	}
    }
}


/******************************************************************************\
@~CTimedPlane --> destructor
--------------------------------------------------------------------------------
   description: 
        author: Kimberly Passarella Jones
 last modified: 9-10-95 by Kimberly Passarella Jones
\******************************************************************************/
CTimedPlane::~CTimedPlane()
{
    BCDebug debug("CTimedPlane::~CTimedPlane", PLANE_CODE);

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

