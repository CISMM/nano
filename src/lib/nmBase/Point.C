#include <math.h>
#include <malloc.h>	// NULL is defined here?
#include <stdio.h>
#include "Point.h"

//----------------------------------------------------------------------
// Point_value methods
//----------------------------------------------------------------------

Point_value::Point_value (BCString name, BCString units)
{
    _dataset = name;
    _units = units;
    _results = (Point_results*) NULL;

    _next = NULL;	
    _value = 0;
    _numcallbacks = 0;
}

Point_value::Point_value (Point_value * value)
{
    _dataset = value->_dataset;
    _units = value->_units;
    _results = value->_results;

    _next = NULL;
    _value = 0;
    _numcallbacks = 0;
}

/** Look up the callback with the given values and return its index in the
 list.  If there is no such callback, return -1.
*/
int	Point_value::lookup_callback (const Point_Valuecall cb,
                                      const void * userdata)
{
	int	i = 0;

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

int	Point_value::add_callback(Point_Valuecall cb, void *userdata)
{
	// If the callback is to a NULL function, fail.
	if (cb == NULL) { return -1; }

	// See if there is already one in the list.  If so, fail.
	if (lookup_callback(cb, userdata) != -1) { return -1; }

	// If we have a full list, fail.
	if (_numcallbacks >= MAX_POINT_CALLBACKS) { return -1; }

	// Put it at the end of the list
	_callbacks[_numcallbacks].callback = cb;
	_callbacks[_numcallbacks].userdata = userdata;
	_numcallbacks++;
	return 0;
}

int	Point_value::remove_callback(Point_Valuecall cb, void *userdata)
{
	int	which;

	// Find the callback, if it is in the list.  If not, fail.
	if ( (which = lookup_callback(cb, userdata)) == -1) { return -1; }

	// Move the last one on the list to this location and decrement count
	_callbacks[which] = _callbacks[_numcallbacks-1];
	_numcallbacks--;
	return 0;
}

//----------------------------------------------------------------------
// Point_results methods
//----------------------------------------------------------------------

/** Create a new point that is a copy of another point, including all of
 its value sets.
*/
Point_results::Point_results (const Point_results &p)
{
	Point_value	*value;

	// No points yet (will copy them in later)
	_num_values = 0; _head = NULL;

	// Copy the information from the point record
	_x = p._x; _y = p._y; _z = p._z;
	_sec = p._sec; _usec = p._usec;
	_is3D = p._is3D;

	// Create by copying the values from the other point
	for (value = p._head; value != NULL; value = value->_next) {
		Point_value *newvalue = addValueCopy(value);
		newvalue->setValue(value->value());
	}
}

Point_results::~Point_results (void)
{   
    Point_value* current;
    Point_value* next;

    current = _head;

    while (current != NULL)
    {
	next = current->_next;
	delete(current);
	current = next;
    } 
}

void Point_results::findUniqueValueName(BCString base_name,
	BCString *result_name)
{
	int	next_number_to_try = 2;
	char	appendix[10];
	*result_name = base_name;

	while (getValueByName(*result_name) != NULL) {
		sprintf(appendix,"%d",next_number_to_try);
		next_number_to_try++;
		*result_name = base_name;
		*result_name += appendix;
	}
}

Point_value* Point_results::addNewValue(BCString dataset, BCString units)
{
	Point_value* value = new Point_value(dataset, units);
        addValue(value);
	return value;
}


Point_value* Point_results::addValueCopy(Point_value* value)
{
	Point_value* copy = new Point_value((Point_value*) value);
        addValue(copy);
	return copy;
} 


int Point_results::deleteHead (void)
{
    if (_head == NULL)
	return -1;

    Point_value* temp = _head->_next;
    delete(_head);
    _head = temp;

    _num_values--;

    return 0;
}


int Point_results::empty (void) const
{
    if (_head == NULL)
	return 1;
    else
	return 0;
}


Point_value* Point_results::getValueByName (const BCString name) const
{
    Point_value * current = _head;

    while (current != NULL) {
	if (current->_dataset == name)
	    return current;
	current = current->_next;
    }

    return NULL;
}


Point_value* Point_results::getValueByPlaneName (const BCString planeName) const
{
    char	fullname[100];
    BCString	name;
    Point_value* current = _head;

    // Find the part of the name that comes before -Forward or -Backward and
    // use that to form the name we're searching for.
    fullname[sizeof(fullname)-1] = '\0';
    strncpy(fullname, planeName.Characters(), sizeof(fullname)-1);
    if (strrchr(fullname,'-') == NULL) {
	return NULL;
    }
    *strrchr(fullname,'-') = '\0';
    name = fullname;

    while (current != NULL) {
	if (current->_dataset == name)
	    return current;
	current = current->_next;
    }

    return NULL;
}


void Point_results::addValue(Point_value* value)
{
    _num_values++;

    Point_value* last = NULL;
    Point_value* current = _head;
    
    while (current!= NULL) {
	last = current;
	current = current->_next;
    }
    
    if (last == NULL)
	_head = value; 
    else 
	last->_next = value;

    value->_results = this;
}


void Point_results::print (const char *prelim) const
{
	Point_value	*next;

	// Print out the preliminary string
	printf("%s",prelim);

	// Print out the information local to this structure
	if (_is3D)
	    printf("Point_result at (%g,%g,%g), time %ld:%ld, %d values:",
		_x,_y,_z, _sec,_usec, _num_values);
	else
	    printf("Point_result at (%g,%g), time %ld:%ld, %d values:",
		_x,_y, _sec,_usec, _num_values);

	// Print out the values
	next = _head;
	while (next != NULL) {
		printf(" %s:%g(%s)",
			next->name()->Characters(),
			next->value(),
			next->units()->Characters() );
		next = next->_next;
	}

	// Done with the message
	printf("\n");
}


//----------------------------------------------------------------------
// Point_list methods
//----------------------------------------------------------------------

Point_list::~Point_list()
{
	int i;

	for (i = 0; i < _num_entries; i++) {
		delete _entries[i];
	}
}


int Point_list::numEntries (void) const {
  return _num_entries;
}

const Point_results * Point_list::entry (int which) const {
  if ((which < 0) || (which >= _num_entries)) {
    return NULL;
  }
  return _entries[which];
}



int Point_list::addEntry(const Point_results &p)
{
	// Make sure there is enough room
	if (_num_entries >= MAX_POINT_LIST) {
		return -1;
	}

	// Add a new point, copying from the one passed in
	if ( (_entries[_num_entries] = new Point_results(p)) == NULL) {
		return -1;
	};
	_num_entries++;

	return 0;
}

void Point_list::clear(void)
{
	int i;
	for (i = 0; i < _num_entries; i++) {
		delete _entries[i];
	}
	_num_entries = 0;
}


int Point_list::writeToTclWindow(Tcl_Interp *interpreter)
{
	int	i, numValues;
	double	last_x, last_y /*, last_z*/;
	long	first_sec,first_usec;
	double	s, time;
	Point_value	*value;
	int is3D;

	// Don't do anything if there aren't any points
	if (_num_entries == 0) {
		return 0;
	}

	// Find out how many values are in the first point.  Used for
	// comparison later.  Also find the starting location and time
	// of the first point.
	numValues = _entries[0]->_num_values;
	last_x = _entries[0]->x();
	last_y = _entries[0]->y();
	//last_z = _entries[0]->z();
	is3D = _entries[0]->is3D();
	first_sec = _entries[0]->_sec;
	first_usec = _entries[0]->_usec;
	s = 0;	// No distance yet.
        
	static  char    *command1;
	char    command2[1000];
	char    str[1000];

	command1=(char *)".mod.text delete 1.0 end";

	if (Tcl_Eval(interpreter, command1) != TCL_OK) {
                fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command1,
                        interpreter->result);
                return(-1);
        }

	// Put a tab-separated header line into the window with the names of
	// the data sets.  Put s, time, x,y, first.
	if (is3D)
	    sprintf(str, "\"s(nm)\tT(sec)\tX(nm)\tY(nm)\tZ(nm) \"");
	else
	    sprintf(str, "\"s(nm)\tTime(sec)\tX(nm)\tY(nm) \"");
        sprintf(command2,".mod.text insert end %s",str);
                 
        if (Tcl_Eval(interpreter, command2) != TCL_OK) {
                fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command2,
                        interpreter->result);
                return(-1);
        }
        

       for (value = _entries[0]->_head; value != NULL; value = value->next()) {
		sprintf(str, "\"\t%s(%s)\"",
			value->name()->Characters(),
			value->units()->Characters() );
	        sprintf(command2,".mod.text insert end %s",str);
	        if (Tcl_Eval(interpreter, command2) != TCL_OK) {
                   fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command2,
                           interpreter->result);
                   return(-1);
               }	
       }

        sprintf(command2,".mod.text insert end \"\n\"");
	if (Tcl_Eval(interpreter, command2) != TCL_OK) {
                fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command2,
                        interpreter->result);
                return(-1);
        }
   
	// Write the values to the Tcl window.Each value within each point will
	// be tab-separated.  Compute s from x and y as difference from start.
	// Compute time as floating-point difference in seconds from the
	// time of the first point.  There is one point per line.
	for (i = 0; i < _num_entries; i++) {
	  Point_results	*p = _entries[i];

	  // Verify that each Point has the same number of values.  If not,
	  // put a warning into the file.
	  if (p->_num_values != numValues) {
		sprintf(str,"\"WARNING -- Different number of values!!!\n\"");
	        sprintf(command2,".mod.text insert end %s",str);
		if (Tcl_Eval(interpreter, command2) != TCL_OK) {
                     fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command2,
                            interpreter->result);
                    return(-1);
                }
	  }	
	  

	  // Calculate s by accumulating from the start
	  s += sqrt( (p->x() - last_x) * (p->x() - last_x) +
		     (p->y() - last_y) * (p->y() - last_y) );
	  last_x = p->x();
	  last_y = p->y();

	  // Calculate time by difference from the start.  Time is in
	  // seconds, but is stored in a double
	  time = (p->_sec - first_sec) + (p->_usec - first_usec)*0.000001;

	  // Put s, time, x,y, then the data sets from the values.
	  if (is3D)
	      sprintf(str,"\"%g\t%g\t%g\t%g\t%g\"", s, time, p->x(), p->y(),
							p->z());
	  else
	      sprintf(str,"\"%g\t%g\t%g\t%g\"", s, time, p->x(), p->y());
	  sprintf(command2,".mod.text insert end %s",str);
	  if (Tcl_Eval(interpreter, command2) != TCL_OK) {
                fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command2,
                        interpreter->result);
                return(-1);
          }	
	  
	  for (value = p->head(); value != NULL; value = value->next()) {
		sprintf(str,"\"\t%12.9g\"", value->value());
                sprintf(command2,".mod.text insert end %s",str);
	        if (Tcl_Eval(interpreter, command2) != TCL_OK) {
                     fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command2,
                            interpreter->result);
                return(-1);
		}	
          }

		
	  sprintf(command2,".mod.text insert end \"\n\"");
	  if (Tcl_Eval(interpreter, command2) != TCL_OK) {
                     fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command2,
                            interpreter->result);
                return(-1);
          }
	  
	}


	return 0;
}

