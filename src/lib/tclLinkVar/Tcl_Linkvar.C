#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<ctype.h>

#include "nmb_Selector.h"
#include "Tcl_Linkvar.h"
#include "Tcl_Netvar.h"


#include <tcl.h>
#include <tk.h>

// Moved a lot of code here that was formerly inlined in the header
// file because we had to change it to virtual and that generated
// warnings on the HP compiler.  - T. Hudson, Sept 99

static	const int MAX_INTS =	1000;	// Max number of int variables
static	const int MAX_IB =	1000;	// Max number of int/buttons
static	const int MAX_FLOATS =	1000;	// Max number of float variables
static	const int MAX_FS =	1000;	// Max number of floatscale vars
static	const int MAX_IS =	1000;	// Max number of intscale vars
static	const int MAX_INTENTRY= 1000;	// Max number of intentry vars
static	const int MAX_CHECKENTRY= 1000;	// Max number of intentry vars
static	const int MAX_SELS =	1000;	// Max number of selectors

static	int	num_ints = 0;			// Number of int variables
static	Tclvar_int	*int_list[MAX_INTS];	// Int variables

static	int	num_ib = 0;			// Number of ints with buttons
static	Tclvar_int_with_button	*ib_list[MAX_IB];// IB variables

static	int	num_floats = 0;			// Number of float variables
static	Tclvar_float	*float_list[MAX_FLOATS];// Float variables

static	int	num_fs = 0;			// Number of floats with scales
static	Tclvar_float_with_scale	*fs_list[MAX_FS];// Floatscale variables

static	int	num_is = 0;			// Number of ints with scales
static	Tclvar_int_with_scale	*is_list[MAX_IS];// Intscale variables

static	int	num_intentry = 0;		// Number of ints with scales
static	Tclvar_int_with_entry	*intentry_list[MAX_INTENTRY];// Intscale variables

static	int	num_checkentry = 0;		// Number of ints with scales
static	Tclvar_checklist_with_entry	*checkentry_list[MAX_CHECKENTRY];// Intscale variab

static	int	num_sels = 0;			// Number of selectors
static	Tclvar_selector		*sel_list[MAX_SELS];

static	Tcl_Interp	*interpreter = NULL;	// Tcl interpreter used

// DO NOT USE commands such as Tcl_GetVar2 (with the 2 at the end)
// if we use Tcl_GetVar instead, we wil be able to link with array elements 
// in TCL. 

//	Update the integer variable in the handler that is pointed to
// when the variables changes.

static	char	*handle_int_value_change(ClientData clientData,
	Tcl_Interp *interp, char */*name1*/, char */*name2*/, int /*flags*/)
{
        char    *cvalue;
	int	value;
          // As long as Tcl_GetInt is machine-dependent we want to
          // pass in an int here, although we could lose precision on
          // 16-bit int architectures.  BUG BUG BUG
	Tclvar_int	*intvar = (Tclvar_int*)(clientData);

	// Look up the new value of the variable
	cvalue = Tcl_GetVar(interp, intvar->my_tcl_varname,
		 TCL_GLOBAL_ONLY);
	if (cvalue == NULL) {
		fprintf(stderr,"Warning!  Can't read %s from Tcl\n",
			intvar->my_tcl_varname);
		intvar->d_dirty = VRPN_TRUE;
	} else if (Tcl_GetInt(interp, cvalue, &value) != TCL_OK) {
		fprintf(stderr,"Warning!  %s not int from Tcl\n",
			intvar->my_tcl_varname);
		intvar->d_dirty = VRPN_TRUE;
	} else if (intvar->d_ignoreChange) {
          intvar->d_ignoreChange = VRPN_FALSE;

//fprintf(stderr, "handle_int_value_change (%s, %d):  ignoring the change.\n",
//intvar->my_tcl_varname, value);

        } else { //no errors: update the variable
          // Need to invoke operator = since it might be redefined
          // by derived class.  (?)

//fprintf(stderr, "handle_int_value_change (%s, %d).\n",
//intvar->my_tcl_varname, value);

          //*intvar = value;
          intvar->d_updateFromTcl = VRPN_TRUE;
          intvar->SetFromTcl(value);
          intvar->mylastint = value;
	} 

	return NULL;
};

//	Update the float variable in the handler that is pointed to
// when the variables changes.

static	char	*handle_float_value_change(ClientData clientData,
	Tcl_Interp *interp, char */*name1*/, char */*name2*/, int /*flags*/)
{
        char    *cvalue;
	vrpn_float64	value;
	Tclvar_float	*floatvar = (Tclvar_float*)(clientData);

// 	printf("floatvalchange: %s %s %s\n", name1, name2, floatvar->my_tcl_varname);
	// Look up the new value of the variable
	cvalue = Tcl_GetVar(interp, floatvar->my_tcl_varname, 
		 TCL_GLOBAL_ONLY);
	if (cvalue == NULL) {
		fprintf(stderr,"Warning!  Can't read %s from Tcl\n",
			floatvar->my_tcl_varname);
		floatvar->d_dirty = VRPN_TRUE;
	} else if (Tcl_GetDouble(interp, cvalue, &value) != TCL_OK) {
		fprintf(stderr,"Warning!  %s not double from Tcl\n",
			floatvar->my_tcl_varname);
		floatvar->d_dirty = VRPN_TRUE;
        } else if (floatvar->d_ignoreChange) {
          floatvar->d_ignoreChange = VRPN_FALSE;
	} else { // no errors: update the variable
//  fprintf(stderr, "handle_float_value_change: %s %s old %f new %f\n", 
//         floatvar->my_tcl_varname, cvalue, floatvar->mylastfloat, value);
           // Need to invoke operator = since it might be redefined
           // by derived class.  (?)
	   //*floatvar = value;
          floatvar->d_updateFromTcl = VRPN_TRUE;
           floatvar->SetFromTcl(value);
	   floatvar->mylastfloat = value;
	}
	return NULL;
};

//	Update the string variable in the handler that is pointed to
// when the variables changes.

static	char	*handle_string_value_change(ClientData clientData,
	Tcl_Interp *interp, char */*name1*/, char */*name2*/, int /*flags*/)
{
        char    *cvalue;
	Tclvar_selector	*selvar = (Tclvar_selector*)(clientData);

	// Look up the new value of the variable
	cvalue = Tcl_GetVar(interp, selvar->d_myTclVarname, 
		 TCL_GLOBAL_ONLY);
	if (cvalue == NULL) {
		fprintf(stderr,"Warning!  Can't read %s from Tcl\n",
			selvar->d_myTclVarname);
		selvar->d_dirty = VRPN_TRUE;
	} else if (selvar->d_ignoreChange) {
//fprintf(stderr, "handle_string_value_change:  ignoring.\n");
          selvar->d_ignoreChange = VRPN_FALSE;
        } else { // no errors: update the variable
//fprintf(stderr, "handle_string_value_change:  changing %s to %s.\n",
//selvar->d_myTclVarname, cvalue);
           // BUG BUG BUG - doesn't use operator =?
          selvar->d_updateFromTcl = VRPN_TRUE;
           selvar->SetFromTcl(cvalue);
	}
	return NULL;
};

//	Set the interpreter to be used by the integers and floats.
//	Put together links for all of the variables that have been created
// before the interpreter was set.
//	Return 0 on success, -1 on failure.

int	Tclvar_init(Tcl_Interp *tcl_interp)
{
	int	i;
	char	cvalue[100];

//fprintf(stderr, "Tclvar_init() called\n");

	// Set the interpreter
	interpreter = tcl_interp;

	// Insert callbacks for the integer variables
	for (i = 0; i < num_ints; i++) {
		// initialize the tcl variable with the value from the
		// C variable
		sprintf(cvalue, "%d", (vrpn_int32) *int_list[i]);
		Tcl_SetVar(interpreter, int_list[i]->my_tcl_varname,
			   cvalue, TCL_GLOBAL_ONLY);
		if (Tcl_TraceVar(interpreter, int_list[i]->my_tcl_varname,
			TCL_TRACE_WRITES | TCL_GLOBAL_ONLY,
			handle_int_value_change,
			(ClientData)((void*)(int_list[i]))) != TCL_OK) {
			fprintf(stderr, "Tcl_TraceVar(%s) failed: %s\n",
				int_list[i]->my_tcl_varname,
				interpreter->result);
			return(-1);
		}
	}

	// Insert callbacks for the float variables
	for (i = 0; i < num_floats; i++) {
		// initialize the tcl variable with the value from the
		// C variable
		sprintf(cvalue, "%g", (vrpn_float64) *float_list[i]);
		Tcl_SetVar(interpreter, float_list[i]->my_tcl_varname,
			   cvalue, TCL_GLOBAL_ONLY);
		if (Tcl_TraceVar(interpreter, float_list[i]->my_tcl_varname,
			TCL_TRACE_WRITES | TCL_GLOBAL_ONLY,
			handle_float_value_change,
			(ClientData)((void*)(float_list[i]))) != TCL_OK) {
			fprintf(stderr, "Tcl_TraceVar(%s) failed: %s\n",
				float_list[i]->my_tcl_varname,
				interpreter->result);
			return(-1);
		}

	}


	// Set up the checkentries.
	// this must be done before the integer with buttons
	// and intentries
	for (i = 0; i < num_checkentry; i++) {
	    if (checkentry_list[i]->tcl_parent_name != NULL) {
		if (checkentry_list[i]->initialize(interpreter)) return(-1);
	    }
	}

        // NANOX
        //for (i = 0; i < num_ints; i++) {
          //if (int_list[i]->initialize(interpreter)) return(-1);
        //}

	// Set up the integer with buttons.
	for (i = 0; i < num_ib; i++) {
	    if (ib_list[i]->tcl_widget_name != NULL) {
		if (ib_list[i]->initialize(interpreter)) return(-1);
	    }
	}

	// Set up the floatscales.
	for (i = 0; i < num_fs; i++) {
	    if (fs_list[i]->tcl_widget_name != NULL) {
		if (fs_list[i]->initialize(interpreter)) return(-1);
	    }
	}

	// Set up the intscales.
	for (i = 0; i < num_is; i++) {
	    if (is_list[i]->tcl_widget_name != NULL) {
		if (is_list[i]->initialize(interpreter)) return(-1);
	    }
	}

	// Set up the intentries.
	for (i = 0; i < num_intentry; i++) {
	    if (intentry_list[i]->tcl_widget_name != NULL) {
		if (intentry_list[i]->initialize(interpreter)) return(-1);
	    }
	}

	// Insert callbacks for the selector variables
	// Set up the selectors.
	for (i = 0; i < num_sels; i++) {
          if (sel_list[i]->d_myTclVarname) {  // TCH  6 April 98
//fprintf(stderr, "Tracing selector %d.\n", i);
            if (Tcl_TraceVar(interpreter, sel_list[i]->d_myTclVarname,
                             TCL_TRACE_WRITES | TCL_GLOBAL_ONLY,
                             handle_string_value_change,
                             (ClientData)((void*)(sel_list[i]))) != TCL_OK) {
              fprintf(stderr, "Tcl_TraceVar(%s) failed: %s\n",
                      sel_list[i]->d_myTclVarname,
                      interpreter->result);
              return(-1);
            }
          }
          if (sel_list[i]->d_tclWidgetName) {
//fprintf(stderr, "Initializing selector %d (%s).\n",
//i, sel_list[i]->d_myTclVarname);
            if (sel_list[i]->initialize(interpreter)) return(-1);
          } else {
            //fprintf(stderr, "Selector %d has no widget.\n", i);
          }
	}

	return	0;

}

int	Tclvar_mainloop (void)
{
	int	i;

	// Make sure there is a valid interpeter
	if (interpreter == NULL) {
		return -1;
	}

#if 0
	// Check for changes in the C selectors and update the Tcl variables
	// when they occur.
	for (i = 0; i < num_sels; i++) {
		if (sel_list[i]->compareStrings() ||
		    (sel_list[i]->d_dirty)) {
                        sel_list[i]->resetString();
			sel_list[i]->d_dirty = VRPN_FALSE;
			Tcl_SetVar(interpreter, sel_list[i]->d_myTclVarname,
                            (char *) sel_list[i]->string(), TCL_GLOBAL_ONLY);
		}
	}
#endif

	return 0;
}

//	Add an entry into the list of active integer Tcl variables, and
// point it to this variable.
//	Add a Tcl callback, if an interpreter has been declared.

Tclvar_int::Tclvar_int(const char *tcl_varname, vrpn_int32 default_value,
	Linkvar_Intcall c, void * ud) :
  my_tcl_varname (NULL),
  d_myint (default_value),
  mylastint (default_value),
  d_dirty (VRPN_FALSE),
  d_ignoreChange (VRPN_FALSE),
  d_permitIdempotentChanges (VRPN_FALSE),
  d_callbacks (NULL),
  d_inCallback (VRPN_FALSE),
  d_updateFromTcl (VRPN_FALSE)
{
//fprintf(stderr, "Tclvar_int constructor\n");

	if (num_ints >= (MAX_INTS-1)) {
		fprintf(stderr,"Tclvar_int::Tclvar_int(): Can't link to %s\n",
			tcl_varname);
		fprintf(stderr,"                          (Out of storage)\n");
	} else {
		int_list[num_ints] = this;
		my_tcl_varname = new char [strlen(tcl_varname) + 1];
		if (my_tcl_varname == NULL) {
			fprintf(stderr,"Tclvar_int::Tclvar_int(): Out of memory\n");
		} else {
			strcpy(my_tcl_varname, tcl_varname);
			num_ints++;
		}
	}

	// Add a callback for change if there is an interpreter
	if (interpreter != NULL) {
		if (Tcl_TraceVar(interpreter, my_tcl_varname,
			TCL_TRACE_WRITES | TCL_GLOBAL_ONLY,
			handle_int_value_change,
			(ClientData)((void*)(this))) != TCL_OK) {
			fprintf(stderr, "Tclvar_int::Tclvar_int(): Tcl_TraceVar(%s) failed: %s\n",
				my_tcl_varname, interpreter->result);
		}
	}

  addCallback(c, ud);
};

//	Remove the entry from the list of active integer Tcl variables.

Tclvar_int::~Tclvar_int (void)
{
	register int	i = 0;
//fprintf(stderr, "~Tclvar_int() %s", my_tcl_varname);
	// Remove the trace callback, if there is an interpreter
	if (interpreter != NULL) {
		Tcl_UntraceVar(interpreter, my_tcl_varname,
			TCL_TRACE_WRITES | TCL_GLOBAL_ONLY,
			handle_int_value_change,
			(ClientData)((void*)(this)));
	}

	// Free the space for the name
	if (my_tcl_varname != NULL) {delete [] my_tcl_varname; }

	// Find this entry in the list of variables
	while ( (i < num_ints) && (int_list[i] != this) ) { i++; };
	if (i >= num_ints) {
		fprintf(stderr,"Tclvar_int::~Tclvar_int(): Internal error -- not in list\n");
		return;
	}
//fprintf(stderr, " %d of %d\n", i, num_ints);

	// Move the last entry to this one
	int_list[i] = int_list[num_ints-1];

	// Reduce the number in the list
	num_ints--;
};

#if 0
int Tclvar_int::initialize (Tcl_Interp * interpreter) {
  if (!d_interpreter) {
    d_interpreter = interpreter;
  }
  return 0;
}
#endif

void Tclvar_int::addCallback (Linkvar_Intcall cb, void * userdata) {
  tclIntCallbackEntry * newEntry;

  if (!cb) {
    //fprintf(stderr, "Tclvar_int::addCallback:  NULL handler.\n");
    return;
  }

  // Allocate and initialize new entry
  newEntry = new tclIntCallbackEntry;
  if (!newEntry) {
    fprintf(stderr, "Tclvar_int::addCallback:  Out of memory.\n");
    return;
  }

  // Add this handler to the chain at the beginning (don't check to see
  // if it is already there, since duplication is okay).

  newEntry->handler = cb;
  newEntry->userdata = userdata;
  newEntry->next = d_callbacks;

  d_callbacks = newEntry;
}

void Tclvar_int::doCallbacks (void) {
  tclIntCallbackEntry * e;

  if (d_inCallback) {
    return;
  }
  d_inCallback = VRPN_TRUE;
  for (e = d_callbacks; e; e = e->next) {
    (*e->handler)(d_myint, e->userdata);
  }
  d_inCallback = VRPN_FALSE;
}

// virtual 
vrpn_int32 Tclvar_int::operator = (vrpn_int32 v) {
  vrpn_int32 retval;
  retval = (d_myint = v);
  updateTcl();
  return retval;
}

// virtual
vrpn_int32 Tclvar_int::operator ++ (void) {
  vrpn_int32 retval;
  retval = ++d_myint;
  updateTcl();
  return retval;
}

// virtual
vrpn_int32 Tclvar_int::operator ++ (int) {
  vrpn_int32 retval;
  retval = d_myint++;
  updateTcl();
  return retval;
}

// virtual
void Tclvar_int::SetFromTcl (vrpn_int32 v) {
  vrpn_int32 retval;
  retval = (d_myint = v);
  doCallbacks();
}

void Tclvar_int::updateTcl (void) {
  char cvalue [100];

  if (d_updateFromTcl) {
    d_updateFromTcl = VRPN_FALSE;
    return;
  }
  // Idempotent check. Only notify tcl if the new value is
  // different from the old value. 
  if (!interpreter) {
    return;
  }
  if ((d_myint != mylastint) ||
      (d_permitIdempotentChanges)) {
  d_ignoreChange = VRPN_TRUE;
    mylastint = d_myint;
    d_dirty = VRPN_FALSE;
    sprintf(cvalue, "%d", d_myint);
    Tcl_SetVar(interpreter, my_tcl_varname, cvalue, TCL_GLOBAL_ONLY);
//fprintf(stderr, "Tclvar_int::updateTcl(%d) - was %d.\n",
//d_myint, mylastint);
  }
}



//	Add an entry into the list of active float Tcl variables, and
// point it to this variable.
//	Add a trace callback if there is an interpreter.

Tclvar_float::Tclvar_float(const char * tcl_varname, vrpn_float64 default_value,
	Linkvar_Floatcall c, void * ud) :
  my_tcl_varname (NULL),
  mylastfloat (default_value),
  d_dirty (VRPN_FALSE),
  d_ignoreChange (VRPN_FALSE),
  d_permitIdempotentChanges (VRPN_FALSE),
  d_myfloat (default_value),
  d_callbacks (NULL),
  d_inCallback (VRPN_FALSE),
  d_updateFromTcl (VRPN_FALSE)
{
//fprintf(stderr, "Tclvar_float constructor\n");

	if (num_floats >= (MAX_FLOATS-1)) {
		fprintf(stderr,"Tclvar_float::Tclvar_float(): Can't link to %s\n",
			tcl_varname);
		fprintf(stderr,"                          (Out of storage)\n");
	} else {
		float_list[num_floats] = this;
		my_tcl_varname = new char[strlen(tcl_varname)+1];
//fprintf(stderr, "  Allocated for tcl_varname %s.\n", tcl_varname);
		if (my_tcl_varname == NULL) {
			fprintf(stderr,"Tclvar_float::Tclvar_float(): Out of memory\n");
		} else {
			strcpy(my_tcl_varname, tcl_varname);
			num_floats++;
		}
	}

        // Add a callback for change if there is an interpreter
        if (interpreter != NULL) {
                if (Tcl_TraceVar(interpreter, my_tcl_varname,
                        TCL_TRACE_WRITES | TCL_GLOBAL_ONLY,
                        handle_float_value_change,
			(ClientData)((void*)this)) != TCL_OK) {
                        fprintf(stderr, "Tclvar_float::Tclvar_float(): Tcl_TraceVar(%s) failed: %s\n",
                                my_tcl_varname, interpreter->result);
                }
        }
  addCallback(c, ud);
//fprintf(stderr, "Done with Tclvar_float #%d (%s = %.5f)\n",
//num_floats, tcl_varname, default_value);
};

//	Remove the entry from the list of active float Tcl variables.

Tclvar_float::~Tclvar_float (void)
{
//fprintf(stderr, "~Tclvar_float() %s", my_tcl_varname);
	register int	i = 0;

	// Remove the trace callback, if there is an interpreter
	if (interpreter != NULL) {
		Tcl_UntraceVar(interpreter, my_tcl_varname, 
			TCL_TRACE_WRITES | TCL_GLOBAL_ONLY,
			handle_float_value_change,
			(ClientData)((void*)this));
	}

	// Free the space for the name
	if (my_tcl_varname != NULL) { delete [] my_tcl_varname; }

	// Find this entry in the list of variables
	while ( (i < num_floats) && (float_list[i] != this) ) { i++; };
	if (i >= num_floats) {
		fprintf(stderr,"Tclvar_float::~Tclvar_float(): Internal error -- not in list\n");
		return;
	}
//fprintf(stderr, " %d of %d.\n", i, num_floats);

	// Move the last entry to this one
	float_list[i] = float_list[num_floats-1];

	// Reduce the number in the list
	num_floats--;
};

void Tclvar_float::addCallback (Linkvar_Floatcall cb, void * userdata) {
  tclFloatCallbackEntry * newEntry;

  if (!cb) {
    //fprintf(stderr, "Tclvar_float::addCallback:  NULL handler.\n");
    return;
  }

  // Allocate and initialize new entry
  newEntry = new tclFloatCallbackEntry;
  if (!newEntry) {
    fprintf(stderr, "Tclvar_float::addCallback:  Out of memory.\n");
    return;
  }

  // Add this handler to the chain at the beginning (don't check to see
  // if it is already there, since duplication is okay).

  newEntry->handler = cb;
  newEntry->userdata = userdata;
  newEntry->next = d_callbacks;

  d_callbacks = newEntry;
}

void Tclvar_float::doCallbacks (void) {
  tclFloatCallbackEntry * e;

//fprintf(stderr, "Tclvar_float::doCallbacks (%s).\n", my_tcl_varname);

  if (d_inCallback) {
    return;
  }
  d_inCallback = VRPN_TRUE;
  for (e = d_callbacks; e; e = e->next) {
    (*e->handler)(d_myfloat, e->userdata);
  }
  d_inCallback = VRPN_FALSE;
}

// virtual
vrpn_float64 Tclvar_float::operator = (vrpn_float64 v) {
  vrpn_float64 retval;
  char cvalue [100];

//fprintf(stderr, "Tclvar_float: %s = %.5f\n", my_tcl_varname, v);

  retval = (d_myfloat = v);

  if (d_updateFromTcl) {
    d_updateFromTcl = VRPN_FALSE;
    return retval;
  }
  if (!interpreter) {
    return retval;
  }
  if ((d_myfloat != mylastfloat) ||
      (d_permitIdempotentChanges)) {
    d_ignoreChange = VRPN_TRUE;
    mylastfloat = d_myfloat;
    d_dirty = VRPN_FALSE;
    sprintf(cvalue, "%f", d_myfloat);
    Tcl_SetVar(interpreter, my_tcl_varname, cvalue, TCL_GLOBAL_ONLY);
  }

  return retval;
}

// virtual
void Tclvar_float::SetFromTcl (vrpn_float64 v) {
  vrpn_float64 retval;

  retval = (d_myfloat = v);

  doCallbacks();
}




//	Add a trace callback and handler widget if there is an interpreter.

Tclvar_int_with_button::Tclvar_int_with_button
       (const char * tcl_varname, const char * parent_name,
        vrpn_int32 default_value,
        Linkvar_Intcall c, void *ud) :
	TclNet_int (tcl_varname, default_value, c, ud)
{
//fprintf(stderr, "Tclvar_int_with_button constructor\n");

	// Add this to the list of integers with buttons
	if (num_ib >= (MAX_IB-1)) {
		fprintf(stderr,"Tclvar_int_with_button::Tclvar_int_with_button(): Can't link to %s\n",
			tcl_varname);
		fprintf(stderr,"                          (Out of storage)\n");
	} else {
		ib_list[num_ib] = this;
		my_tcl_varname = new char[strlen(tcl_varname)+1];
		if (my_tcl_varname == NULL) {
			fprintf(stderr,"Tclvar_int_with_button::Tclvar_int_with_button(): Out of memory\n");
		} else {
			strcpy(my_tcl_varname, tcl_varname);
			d_myint = mylastint = default_value;
			num_ib++;
		}
	}

	// Set the widget name, if there is a parent specified
	// The widget name matches the tcl_varname, unless there are '.'s
	// in the name.  If there are '.'s, the name is just the part after
	// the last '.' (this strips away the prepended widget part).
	if (parent_name != NULL) {
		const char	*last_part;

		last_part = strrchr(tcl_varname, '.');
		if (last_part == NULL) {
			last_part = tcl_varname;
		} else {
			last_part++;	// Skip the .
		}
		tcl_widget_name =
			new char[strlen(parent_name) + strlen(last_part) + 2];
		if (tcl_widget_name == NULL) {
			fprintf(stderr,"Tclvar_int_with_button::Tclvar_int_with_button(): Out of memory\n");
		}
		sprintf(tcl_widget_name,"%s.%s",parent_name,last_part);

		// Make sure widget name starts with a lower-case character.
		tcl_widget_name[strlen(parent_name)+1] =
		  tolower(tcl_widget_name[strlen(parent_name)+1]);
	} else {
		tcl_widget_name = NULL;
	}

	// Add and pack a checkbutton if there is an interpreter and a widget
	if ( (interpreter != NULL) && (tcl_widget_name != NULL) ) {
		initialize(interpreter);
	}
};

Tclvar_int_with_button::~Tclvar_int_with_button (void)
{
	register int	i = 0;

	// Unpack and destroy the checkbutton, if there is an
	// interpreter
	if (interpreter != NULL) {
		char	command[1000];
		sprintf(command,"destroy %s",tcl_widget_name);
		if (Tcl_Eval(interpreter, command) != TCL_OK) {
			fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command,
				interpreter->result);
		}
	}

	// Free the space for the widget name
	if (tcl_widget_name != NULL) { delete [] tcl_widget_name; }

	// Find this entry in the list of variables
	while ( (i < num_ib) && (ib_list[i] != this) ) { i++; };
	if (i >= num_ib) {
		fprintf(stderr,"Tclvar_int_with_button::~Tclvar_int_with_button(): Internal error -- not in list\n");
		return;
	}

	// Move the last entry to this one
	ib_list[i] = ib_list[num_ib-1];

	// Reduce the number in the list
	num_ib--;
};


// Add and pack a checkbutton if there is an interpreter and a widget
// for each intwithbutton variable.  Note that the change callback will
// be handled by the fact that it is derived from an int var.
// Set the value of the variable so it shows up on the display.

int	Tclvar_int_with_button::initialize(Tcl_Interp *interpreter)
{
	char	command[1000];
	char	cvalue[100];
	char	*last_part;

        //Tclvar_int::initialize(interpreter);

	// Set the variable to its default value
	sprintf(cvalue,"%d", d_myint);
	Tcl_SetVar(interpreter, my_tcl_varname, cvalue, TCL_GLOBAL_ONLY);

	// Create the checkbutton.  Only use the truncated name for the
	// text in the widget (the part of the variable name after the
	// last '.').
	last_part = strrchr(my_tcl_varname, '.');
	if (last_part == NULL) {
		last_part = my_tcl_varname;
	} else {
		last_part++;	// Skip the .
	}
	sprintf(command,"checkbutton {%s} -text {%s} -variable {%s} -anchor w",
		tcl_widget_name, last_part, my_tcl_varname);
	if (Tcl_Eval(interpreter, command) != TCL_OK) {
		fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command,
			interpreter->result);
		return(-1);
	}

	// Pack the checkbutton
	sprintf(command,"pack {%s} -fill x\n", tcl_widget_name);
	if (Tcl_Eval(interpreter, command) != TCL_OK) {
		fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command,
			interpreter->result);
		return(-1);
	}

	return 0;
}

// virtual
vrpn_int32 Tclvar_int_with_button::operator = (vrpn_int32 v) {
  return TclNet_int::operator = (v);
}

// virtual
vrpn_int32 Tclvar_int_with_button::operator ++ (void) {
  return Tclvar_int::operator ++ ();
}

// virtual
vrpn_int32 Tclvar_int_with_button::operator ++ (int v) {
  return Tclvar_int::operator ++ (v);
}



//	Add an entry into the list of active floatscale Tcl variables, and
// point it to this variable.  Set up the min and max floats.
//	Add a trace callback and handler widget if there is an interpreter.

Tclvar_float_with_scale::Tclvar_float_with_scale
       (const char * tcl_varname, const char * parent_name,
        vrpn_float64 minval, vrpn_float64 maxval, vrpn_float64 default_value,
	Linkvar_Floatcall c, void * ud) :
	TclNet_float (tcl_varname, default_value, c, ud)
{
//fprintf(stderr, "Tclvar_float_with_scale constructor\n");

	minvalue = minval;
	maxvalue = maxval;

	if (num_fs >= (MAX_FS-1)) {
		fprintf(stderr,"Tclvar_float_with_scale::Tclvar_float_with_scale(): Can't link to %s\n",
			tcl_varname);
		fprintf(stderr,"                          (Out of storage)\n");
	} else {
		fs_list[num_fs] = this;
		my_tcl_varname = new char[strlen(tcl_varname)+1];
		if (my_tcl_varname == NULL) {
			fprintf(stderr,"Tclvar_float_with_scale::Tclvar_float_with_scale(): Out of memory\n");
		} else {
			strcpy(my_tcl_varname, tcl_varname);
			d_myfloat = mylastfloat = default_value;
			num_fs++;
		}
	}

	// Set the widget and label names, if there is a parent specified
	if (parent_name != NULL) {
		tcl_widget_name =
			new char[strlen(parent_name) + strlen(tcl_varname) + 2];
		if (tcl_widget_name == NULL) {
			fprintf(stderr,"Tclvar_float_with_scale::Tclvar_float_with_scale(): Out of memory\n");
		}
		sprintf(tcl_widget_name,"%s.%s",parent_name,tcl_varname);

		tcl_label_name =
			new char[strlen(parent_name) + strlen(tcl_varname) +12];
		if (tcl_label_name == NULL) {
			fprintf(stderr,"Tclvar_float_with_scale::Tclvar_float_with_scale(): Out of memory\n");
		}
		sprintf(tcl_label_name,"%s.%slabel",parent_name,tcl_varname);
	} else {
		tcl_widget_name = NULL;
	}

	// Add and pack a floatscale if there is an interpreter and a widget
	if ( (interpreter != NULL) && (tcl_widget_name != NULL) ) {
		initialize(interpreter);
	}
//fprintf(stderr, "Done with float_with_scale %d (%s = %.5f)\n",
//num_fs, tcl_varname, default_value);
};

//	Remove the entry from the list of active float Tcl variables.
//	Base class destructor handles the rest.

Tclvar_float_with_scale::~Tclvar_float_with_scale (void)
{
	register int	i = 0;

	// Unpack and destroy the floatscale and label, if there is an
	// interpreter
	if (interpreter != NULL) {
		char	command[1000];
		sprintf(command,"destroy %s",tcl_widget_name);
		if (Tcl_Eval(interpreter, command) != TCL_OK) {
			fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command,
				interpreter->result);
		}
		sprintf(command,"destroy %s",tcl_label_name);
		if (Tcl_Eval(interpreter, command) != TCL_OK) {
			fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command,
				interpreter->result);
		}
	}

	// Free the space for the widget name
	if (tcl_widget_name != NULL) { delete [] tcl_widget_name; }
	if (tcl_label_name != NULL) { delete [] tcl_label_name; }

	// Find this entry in the list of variables
	while ( (i < num_fs) && (fs_list[i] != this) ) { i++; };
	if (i >= num_fs) {
		fprintf(stderr,"Tclvar_float_with_scale::~Tclvar_float_with_scale(): Internal error -- not in list\n");
		return;
	}

	// Move the last entry to this one
	fs_list[i] = fs_list[num_fs-1];

	// Reduce the number in the list
	num_fs--;
};

// Add and pack a floatscale if there is an interpreter and a widget
// for each floatscale variable.  Note that the change callback will
// be handled by the fact that it is derived from a float var.
// Set the value of the variable so it shows up on the display.
// Put in a label with the variable's name

int	Tclvar_float_with_scale::initialize(Tcl_Interp *interpreter)
{
	char	command[1000];
	char	cvalue[100];

	// Set the variable to its default value
	sprintf(cvalue,"%g", d_myfloat);
	Tcl_SetVar(interpreter, my_tcl_varname, cvalue, TCL_GLOBAL_ONLY);

	// Create the floatscale
	sprintf(command,"floatscale %s %g %g 300 0 1 %s",
		tcl_widget_name, minvalue, maxvalue, my_tcl_varname);
	if (Tcl_Eval(interpreter, command) != TCL_OK) {
		fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command,
			interpreter->result);
		return(-1);
	}

	// Make the label for the floatscale
	sprintf(tcl_label_name,"%slabel",tcl_widget_name);
	sprintf(command,"label %s -text %s", tcl_label_name, my_tcl_varname);
	if (Tcl_Eval(interpreter, command) != TCL_OK) {
		fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command,
			interpreter->result);
		return(-1);
	}

	// Pack the label and floatscale
	sprintf(command,"pack %s %s\n", tcl_label_name, tcl_widget_name);
	if (Tcl_Eval(interpreter, command) != TCL_OK) {
		fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command,
			interpreter->result);
		return(-1);
	}

	return 0;
}

// virtual
vrpn_float64 Tclvar_float_with_scale::operator = (vrpn_float64 v) {
  return TclNet_float::operator = (v);
}


//	Add an entry into the list of active intscale Tcl variables, and
// point it to this variable.  Set up the min and max ints.
//	Add a trace callback and handler widget if there is an interpreter.

Tclvar_int_with_scale::Tclvar_int_with_scale
       (const char * tcl_varname, const char * parent_name,
        vrpn_int32 minval, vrpn_int32 maxval, vrpn_int32 default_value,
	Linkvar_Intcall c, void * ud) :
	TclNet_int(tcl_varname, default_value, c, ud)
{
//fprintf(stderr, "Tclvar_int_with_scale constructor\n");

	minvalue = minval;
	maxvalue = maxval;

	if (num_is >= (MAX_IS-1)) {
		fprintf(stderr,"Tclvar_int_with_scale::"
                             "Tclvar_int_with_scale(): Can't link to %s\n",
			tcl_varname);
		fprintf(stderr,"                          (Out of storage)\n");
	} else {
		is_list[num_is] = this;
		my_tcl_varname = new char[strlen(tcl_varname)+1];
		if (my_tcl_varname == NULL) {
			fprintf(stderr,"Tclvar_int_with_scale::"
                             "Tclvar_int_with_scale(): Out of memory\n");
		} else {
			strcpy(my_tcl_varname, tcl_varname);
			d_myint = mylastint = default_value;
			num_is++;
		}
	}

	// Set the widget and label names, if there is a parent specified
	if (parent_name != NULL) {
		tcl_widget_name =
			new char[strlen(parent_name) + strlen(tcl_varname) + 2];
		if (tcl_widget_name == NULL) {
			fprintf(stderr,"Tclvar_int_with_scale::"
                             "Tclvar_int_with_scale():  Out of memory\n");
		}
		sprintf(tcl_widget_name,"%s.%s",parent_name,tcl_varname);

		tcl_label_name =
			new char[strlen(parent_name) + strlen(tcl_varname) +12];
		if (tcl_label_name == NULL) {
			fprintf(stderr, "Tclvar_int_with_scale::"
                             "Tclvar_int_with_scale():  Out of memory\n");
		}
		sprintf(tcl_label_name,"%s.%slabel",parent_name,tcl_varname);
	} else {
		tcl_widget_name = NULL;
	}

	// Add and pack a intscale if there is an interpreter and a widget
	if ( (interpreter != NULL) && (tcl_widget_name != NULL) ) {
		initialize(interpreter);
	}
//fprintf(stderr, "Done with int_with_scale %d (%s = %.5f)\n",
//num_is, tcl_varname, default_value);
};

//	Remove the entry from the list of active int Tcl variables.
//	Base class destructor handles the rest.

Tclvar_int_with_scale::~Tclvar_int_with_scale (void)
{
	register int	i = 0;

	// Unpack and destroy the intscale and label, if there is an
	// interpreter
	if (interpreter != NULL) {
		char	command[1000];
		sprintf(command,"destroy %s",tcl_widget_name);
		if (Tcl_Eval(interpreter, command) != TCL_OK) {
			fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command,
				interpreter->result);
		}
		sprintf(command,"destroy %s",tcl_label_name);
		if (Tcl_Eval(interpreter, command) != TCL_OK) {
			fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command,
				interpreter->result);
		}
	}

	// Free the space for the widget name
	if (tcl_widget_name != NULL) { delete [] tcl_widget_name; }
	if (tcl_label_name != NULL) { delete [] tcl_label_name; }

	// Find this entry in the list of variables
	while ( (i < num_is) && (is_list[i] != this) ) { i++; };
	if (i >= num_is) {
		fprintf(stderr,"Tclvar_int_with_scale::~Tclvar_int_with_scale(): Internal error -- not in list\n");
		return;
	}

	// Move the last entry to this one
	is_list[i] = is_list[num_is-1];

	// Reduce the number in the list
	num_is--;
};

// Add and pack a intscale if there is an interpreter and a widget
// for each int_entry variable.  Note that the change callback will
// be handled by the fact that it is derived from a int var.
// Set the value of the variable so it shows up on the display.
// Put in a label with the variable's name

int	Tclvar_int_with_scale::initialize(Tcl_Interp *interpreter)
{
	char	command[1000];
	char	cvalue[100];

        //Tclvar_int::initialize(interpreter);

	// Set the variable to its default value
	sprintf(cvalue,"%d", d_myint);
	Tcl_SetVar(interpreter, my_tcl_varname, cvalue, TCL_GLOBAL_ONLY);

	// Create the int_entry
	sprintf(command,"intscale %s %d %d 300 0 1 %s",
		tcl_widget_name, minvalue, maxvalue, my_tcl_varname);
	if (Tcl_Eval(interpreter, command) != TCL_OK) {
		fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command,
			interpreter->result);
		return(-1);
	}

	// Make the label for the intscale
	sprintf(tcl_label_name,"%slabel",tcl_widget_name);
	sprintf(command,"label %s -text %s", tcl_label_name, my_tcl_varname);
	if (Tcl_Eval(interpreter, command) != TCL_OK) {
		fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command,
			interpreter->result);
		return(-1);
	}

	// Pack the label and intscale
	sprintf(command,"pack %s %s\n", tcl_label_name, tcl_widget_name);
	if (Tcl_Eval(interpreter, command) != TCL_OK) {
		fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command,
			interpreter->result);
		return(-1);
	}

	return 0;
}

// virtual
vrpn_int32 Tclvar_int_with_scale::operator = (vrpn_int32 v) {
  return TclNet_int::operator = (v);
}





//	Add an entry into the list of active intentry Tcl variables, and
// point it to this variable.  
//	Add a trace callback and handler widget if there is an interpreter.

Tclvar_int_with_entry::Tclvar_int_with_entry(const char *tcl_varname,
	const char *parent_name, vrpn_int32 default_value,
	Linkvar_Intcall c, void *ud) :
	TclNet_int(tcl_varname, default_value, c, ud)
{
//fprintf(stderr, "Tclvar_int_with_entry constructor\n");


	if (num_intentry >= (MAX_INTENTRY-1)) {
		fprintf(stderr,"Tclvar_int_with_entry::Tclvar_int_with_entry(): Can't link to %s\n",
			tcl_varname);
		fprintf(stderr,"                          (Out of storage)\n");
	} else {
		intentry_list[num_intentry] = this;
		my_tcl_varname = new char[strlen(tcl_varname)+1];
		if (my_tcl_varname == NULL) {
			fprintf(stderr,"Tclvar_int_with_entry::Tclvar_int_with_entry(): Out of memory\n");
		} else {
			strcpy(my_tcl_varname, tcl_varname);
			d_myint = mylastint = default_value;
			num_intentry++;
		}
	}

	// Set the widget name and label names, if there is a parent specified
	// The widget name matches the tcl_varname, unless there are '.'s
	// in the name.  If there are '.'s, the name is just the part after
	// the last '.' (this strips away the prepended widget part).
	if (parent_name != NULL) {
		const char	*last_part;

		last_part = strrchr(tcl_varname, '.');
		if (last_part == NULL) {
			last_part = tcl_varname;
		} else {
			last_part++;	// Skip the .
		}
		tcl_widget_name =
			new char[strlen(parent_name) + strlen(last_part) + 2];
		if (tcl_widget_name == NULL) {
			fprintf(stderr,"Tclvar_int_with_entry::Tclvar_int_with_entry(): Out of memory\n");
		}
		sprintf(tcl_widget_name,"%s.%s",parent_name,last_part);
		// Make sure widget name starts with a lower-case character.
		tcl_widget_name[strlen(parent_name)+1] =
		  tolower(tcl_widget_name[strlen(parent_name)+1]);
		// remove spaces - they are bad for widget names.
		int i;
		for (i = 0; i < strlen(tcl_widget_name); i++)
		   if (tcl_widget_name[i] == ' ')
		      tcl_widget_name[i] = '_';

		tcl_label_name =
			new char[strlen(parent_name) + strlen(last_part) +8];
		if (tcl_label_name == NULL) {
			fprintf(stderr,"Tclvar_int_with_entry::Tclvar_int_with_entry(): Out of memory\n");
		}
		sprintf(tcl_label_name,"%s.%slabel",parent_name,last_part);

		// Make sure label name starts with a lower-case character.
		tcl_label_name[strlen(parent_name)+1] =
		  tolower(tcl_label_name[strlen(parent_name)+1]);
		// remove spaces - they are bad for widget names.
		for (i = 0; i < strlen(tcl_label_name); i++)
		   if (tcl_label_name[i] == ' ')
		      tcl_label_name[i] = '_';

	} else {
		tcl_widget_name = NULL;
	}
	
	//printf("int_w_entry contsr: %s %s %s\n", my_tcl_varname, parent_name,tcl_widget_name); 
	
	// Add and pack a intentry if there is an interpreter and a widget
	if ( (interpreter != NULL) && (tcl_widget_name != NULL) ) {
		initialize(interpreter);
	}
};

//	Remove the entry from the list of active int Tcl variables.
//	Base class destructor handles the rest.

Tclvar_int_with_entry::~Tclvar_int_with_entry (void)
{
	register int	i = 0;

	// Unpack and destroy the intentry and label, if there is an
	// interpreter
	if ( (interpreter != NULL) && (tcl_widget_name != NULL) ){
		char	command[1000];
		sprintf(command,"destroy %s",tcl_widget_name);
		if (Tcl_Eval(interpreter, command) != TCL_OK) {
			fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command,
				interpreter->result);
		}
		sprintf(command,"destroy %s",tcl_label_name);
		if (Tcl_Eval(interpreter, command) != TCL_OK) {
			fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command,
				interpreter->result);
		}
	}

	// Free the space for the widget name
	if (tcl_widget_name != NULL) { delete [] tcl_widget_name; }
	if (tcl_label_name != NULL) { delete [] tcl_label_name; }

	// Find this entry in the list of variables
	while ( (i < num_intentry) && (intentry_list[i] != this) ) { i++; };
	if (i >= num_intentry) {
		fprintf(stderr,"Tclvar_int_with_entry::~Tclvar_int_with_entry(): Internal error -- not in list\n");
		return;
	}

	// Move the last entry to this one
	intentry_list[i] = intentry_list[num_intentry-1];

	// Reduce the number in the list
	num_intentry--;
};

// Add and pack a intentry if there is an interpreter and a widget
// for each intentry variable.  Note that the change callback will
// be handled by the fact that it is derived from a int var.
// Set the value of the variable so it shows up on the display.
// Put in a label with the variable's name

int	Tclvar_int_with_entry::initialize(Tcl_Interp *interpreter)
{
	char	command[1000];
	char	cvalue[100];


	// Create the intentry
	sprintf(command,"intentry {%s} {%s}",
		tcl_widget_name, my_tcl_varname);
	TCLEVALCHECK(interpreter, command);

	//set the initial value of the entry field
	// Set the variable to its default value
	sprintf(cvalue,"%d", d_myint);
	Tcl_SetVar(interpreter, my_tcl_varname, cvalue, TCL_GLOBAL_ONLY);

	// Make the label for the intentry
	sprintf(tcl_label_name,"%slabel",tcl_widget_name);

	return 0;
}

// virtual
vrpn_int32 Tclvar_int_with_entry::operator = (vrpn_int32 v) {
  return TclNet_int::operator = (v);
}






// Too dangerous - interferes with nmb_Selector::operator = (const char *).
// Perhaps we could just write Tclvar_selector::operator =, but I don't
// think that's the right thing to do.

Tclvar_selector::Tclvar_selector (const char * initial_value) :
  nmb_Selector (NULL, initial_value),
  d_myTclVarname (NULL),
  d_tclWidgetName (NULL),
  d_tclLabelName (NULL),
  d_dirty (VRPN_FALSE),
  d_ignoreChange (VRPN_FALSE),
  d_permitIdempotentChanges (VRPN_FALSE),
  d_callbacks (NULL),
  d_initialized (VRPN_FALSE),
  d_inCallback (VRPN_FALSE),
  d_updateFromTcl (VRPN_FALSE)
{
  if (num_sels >= (MAX_SELS - 1)) {
    fprintf(stderr, "Tclvar_selector:  "
                    "Can't link to nameless variable.\n"
                    "(Out of storage)\n");
    return;
  }
  sel_list[num_sels] = this;
  num_sels++;

//fprintf(stderr, "Constructed Tclvar_selector # %d ( = %s).\n", num_sels,
//initial_value);
}



Tclvar_selector::Tclvar_selector
                    (const char * tcl_varname,
                     const char * parent_name,
                     nmb_ListOfStrings * list,
                     const char * initial_value,
                     Linkvar_Selectcall c,
                     void * userdata) :
  nmb_Selector (list, initial_value),
  d_myTclVarname (NULL),
  d_tclWidgetName (NULL),
  d_tclLabelName (NULL),
  d_dirty (VRPN_FALSE),
  d_callbacks (NULL), 
  d_initialized (VRPN_FALSE)
{
  if (num_sels >= (MAX_SELS - 1)) {
    fprintf(stderr, "Tclvar_selector:  "
                    "Can't link to nameless variable.\n"
                    "(Out of storage)\n");
    return;
  }
  sel_list[num_sels] = this;
  num_sels++;
  initializeTcl(tcl_varname, parent_name);

  addCallback(c, userdata);

//fprintf(stderr, "Constructed & Tcl-initialized Tclvar_selector "
//"# %d (%s = %s).\n",
//num_sels, tcl_varname, initial_value);
}

//virtual
Tclvar_selector::~Tclvar_selector (void) {
  char command [1000];
  int i;

  // Remove the trace callback, if there is an interpreter.
  if (interpreter && d_myTclVarname) {
    Tcl_UntraceVar(interpreter, d_myTclVarname,
                   TCL_TRACE_WRITES | TCL_GLOBAL_ONLY,
                   handle_string_value_change,
                   (ClientData) (void *) this);
  }

  if (interpreter && d_tclWidgetName) {
    sprintf(command, "destroy %s", d_tclWidgetName);
    if (Tcl_Eval(interpreter, command) != TCL_OK) {
      fprintf(stderr, "Tcl_Eval(%s) failed:  %s\n", command,
              interpreter->result);
    }
  }
  if (interpreter && d_tclLabelName) {
    sprintf(command, "destroy %s", d_tclLabelName);
    if (Tcl_Eval(interpreter, command) != TCL_OK) {
      fprintf(stderr, "Tcl_Eval(%s) failed:  %s\n", command,
              interpreter->result);
    }
  }

  if (d_myTclVarname)
    delete [] d_myTclVarname;
  if (d_tclWidgetName)
    delete [] d_tclWidgetName;
  if (d_tclLabelName)
    delete [] d_tclLabelName;

  for (i = 0; (i < num_sels) && (sel_list[i] != this); i++) ;
  if (i >= num_sels) {
    fprintf(stderr, "~Tclvar_selector:  Internal error.\n");
    return;
  }

  sel_list[i] = sel_list[num_sels - 1];
  num_sels--;

}

void Tclvar_selector::addCallback (Linkvar_Selectcall cb, void * userdata) {
  tclSelectCallbackEntry * newEntry;

  if (!cb) {
    //fprintf(stderr, "Tclvar_selector::addCallback:  NULL handler.\n");
    return;
  }

  // Allocate and initialize new entry
  newEntry = new tclSelectCallbackEntry;
  if (!newEntry) {
    fprintf(stderr, "Tclvar_selector::addCallback:  Out of memory.\n");
    return;
  }

  // Add this handler to the chain at the beginning (don't check to see
  // if it is already there, since duplication is okay).

  newEntry->handler = cb;
  newEntry->userdata = userdata;
  newEntry->next = d_callbacks;

  d_callbacks = newEntry;
}

void Tclvar_selector::doCallbacks (void) {
  tclSelectCallbackEntry * e;

  if (d_inCallback) {
    return;
  }
  d_inCallback = VRPN_TRUE;
  for (e = d_callbacks; e; e = e->next) {
    (*e->handler)(string(), e->userdata);
  }
  d_inCallback = VRPN_FALSE;
}




// virtual
const char * Tclvar_selector::operator = (const char * v) {
  nmb_Selector::operator = (v);
  updateTcl();
  return string();
}

// virtual
const char * Tclvar_selector::operator = (char * v) {
  nmb_Selector::operator = (v);
  updateTcl();
  return string();
}

//virtual
void Tclvar_selector::Set (const char * v) {
  nmb_Selector::Set(v);
  // nmb_Selector::Set() will call Tclvar_selector::operator =(),
  // which will call updateTcl(), which will trigger SetFromTcl(),
  // which will call doCallbacks().
}

//virtual
void Tclvar_selector::SetFromTcl (const char * v) {
  nmb_Selector::operator = (v);
  doCallbacks();
}

//
void Tclvar_selector::updateTcl (void) {

  if (d_updateFromTcl) {
    d_updateFromTcl = VRPN_FALSE;
    return;
  }

  if (!interpreter) {
    return;
  }

  // Idempotent check.
  if (compareStrings() ||
      d_permitIdempotentChanges) {
    d_ignoreChange = VRPN_TRUE;
    resetString();
    d_dirty = VRPN_FALSE;
    Tcl_SetVar(interpreter, d_myTclVarname, (char *) string(), TCL_GLOBAL_ONLY);
//fprintf(stderr, "Tclvar_int::updateTcl(%s) - was %s.\n",
//string(), d_myLastString);
  }
}

void Tclvar_selector::initializeTcl (const char * tcl_varname,
                                     const char * parent_name) {
  if (!tcl_varname) {
    //fprintf(stderr, "Tclvar_selector::initializeTcl:  "
                    //"NULL variable name!\n");
    return;
  }

  d_myTclVarname = new char [strlen(tcl_varname) + 1];
  if (!d_myTclVarname) {
    fprintf(stderr, "Tclvar_selector::initializeTcl:  Out of memory.\n");
    return;
  }
  strcpy(d_myTclVarname, tcl_varname);
//fprintf(stderr, "  Set varname %s.\n", tcl_varname);

  if (parent_name) {
    d_tclWidgetName = new char [strlen(parent_name) +
                                strlen(tcl_varname) + 2];
    if (!d_tclWidgetName) {
      fprintf(stderr, "Tclvar_selector::initializeTcl:  Out of memory.\n");
      return;
    }
    sprintf(d_tclWidgetName, "%s.%s", parent_name, tcl_varname);
//fprintf(stderr, "  Set widget name %s.\n", d_tclWidgetName);

    d_tclLabelName = new char [strlen(parent_name) +
                                strlen(tcl_varname) + 12];  // +7?
    if (!d_tclLabelName) {
      fprintf(stderr, "Tclvar_selector::initializeTcl:  Out of memory.\n");
      return;
    }
    sprintf(d_tclLabelName, "%s.%slabel", parent_name, tcl_varname);
//fprintf(stderr, "  Set label name %s.\n", d_tclLabelName);
  }

  // Add a callback for changes if there is an interpreter.
  if (interpreter) {
    if (Tcl_TraceVar(interpreter, d_myTclVarname,
                     TCL_TRACE_WRITES | TCL_GLOBAL_ONLY,
                     handle_string_value_change,
                     (ClientData) (void *) this) != TCL_OK) {
      fprintf(stderr, "Tclvar_selector::initializeTcl:  "
                      "Tcl_TraceVar(%s) failed:  %s\n",
              d_myTclVarname, interpreter->result);
    }
  }

  // Build the menu for this selector, if there is an interpreter
  // and a widget name.
  if (interpreter && d_tclWidgetName) {
    initialize(interpreter);
  } else {
    //fprintf(stderr, "  No interpreter yet.\n");
  }
}

//virtual
int Tclvar_selector::bindList (nmb_ListOfStrings * list) {
  int ret = 0;

  clearList(); // we need to do this before we replace d_myList
  ret = nmb_Selector::bindList(list);

  if (d_initialized && list && !ret) {
    ret = initializeList();
  } else if (d_initialized && !list && !ret) {
    ret = clearList();
  }

  return ret;
}

int Tclvar_selector::initialize (Tcl_Interp * interpreter) {
  char command [1000];
  int ret;

//fprintf(stderr, "In Tclvar_selector::initialize() for widget %s\n",
//d_tclWidgetName);

  // Set default value
  Tcl_SetVar(interpreter, d_myTclVarname, d_myString, TCL_GLOBAL_ONLY);

  // Create button to invoke menu
  sprintf(command, "menubutton %s -textvariable %s -bd 2 -relief raised "
                   "-menu %s.menu",
          d_tclWidgetName, d_myTclVarname, d_tclWidgetName);
  if (Tcl_Eval(interpreter, command) != TCL_OK) {
    fprintf(stderr, "Tcl_Eval(%s) failed:  %s\n", command,
            interpreter->result);
    return -1;
  }

  // Cause button 3 to bring up a dialog box
  sprintf(command, "bind %s <Button-3> { newvalue_dialogue %s }",
          d_tclWidgetName, d_myTclVarname);
  if (Tcl_Eval(interpreter, command) != TCL_OK) {
    fprintf(stderr, "Tcl_Eval(%s) failed:  %s\n", command,
            interpreter->result);
    return -1;
  }

  // Create the menu
  sprintf(command, "menu %s.menu", d_tclWidgetName);
  if (Tcl_Eval(interpreter, command) != TCL_OK) {
    fprintf(stderr, "Tcl_Eval(%s) failed:  %s\n", command,
            interpreter->result);
    return -1;
  }

  if (d_myList) {
    ret = initializeList();
    if (ret < 0)
      return -1;
  }

  // Make the label for the menu
  sprintf(d_tclLabelName, "%slabel", d_tclWidgetName);
  sprintf(command, "label %s -text %s", d_tclLabelName, d_myTclVarname);
  if (Tcl_Eval(interpreter, command) != TCL_OK) {
    fprintf(stderr, "Tcl_Eval(%s) failed:  %s\n", command,
            interpreter->result);
    return -1;
  }

  // Pack the label and the menu
  sprintf(command, "pack %s %s", d_tclLabelName, d_tclWidgetName);
  if (Tcl_Eval(interpreter, command) != TCL_OK) {
    fprintf(stderr, "Tcl_Eval(%s) failed:  %s\n", command,
            interpreter->result);
    return -1;
  }

  d_initialized = VRPN_TRUE;

  return 0;
}

int Tclvar_selector::clearList (void) {
  int retval; 
  if (!d_myList) return 0;
  for (int i = 0; i < d_myList->numEntries(); i++) {
    retval = deleteEntry(d_myList->entry(i));
    if (retval) {
      return -1;
    }
  }
  return 0;
}  

int Tclvar_selector::initializeList (void) {
  int i;
  int retval;

  if (!d_myList) return 0;

  for (i = 0; i < d_myList->numEntries(); i++) {
    retval = addEntry(d_myList->entry(i));
    if (retval) {
      return -1;
    }
  }

  return 0;
}







// virtual
int Tclvar_selector::addEntry (const char * entry) {
  char command [1000];

  if (!interpreter) return 0;
  if (!d_tclWidgetName) return 0;
//fprintf(stderr, "Adding %s to menu %s.\n", entry,
//d_tclWidgetName);
  int i;
  if (d_tclWidgetName[0] == '$') {
    printf("Tclvar_selector::addEntry: Warning: assuming global var in path and declaring it as such\n");
    sprintf(command, "global %s", &(d_tclWidgetName[1]));
    for (i = 7; i < strlen(d_tclWidgetName) + 7; i++){
      if (command[i] == '(' || command[i] == '.'){
        command[i] = '\0';
        break;
      }
    }
    if (Tcl_Eval(interpreter, command) != TCL_OK) {
      fprintf(stderr, "Tcl_Eval(%s) failed:  %s\n", command,
            interpreter->result);
      return -1;
    }
  }

  sprintf(command, "%s.menu add command -label {%s} -underline 0 "
                   "-command \"set %s {%s}\"",
          d_tclWidgetName, entry, d_myTclVarname, entry);
  if (Tcl_Eval(interpreter, command) != TCL_OK) {
    fprintf(stderr, "Tcl_Eval(%s) failed:  %s\n", command,
            interpreter->result);
    return -1;
  }

  return 0;
}


// virtual
int Tclvar_selector::deleteEntry (const char * entry) {
  char command [1000];

  if (!interpreter) return 0;
  if (!d_tclWidgetName) return 0;

//fprintf(stderr, "In deleteEntry for %s, %s.\n",
//d_tclWidgetName, entry);
  int i;
  if (d_tclWidgetName[0] == '$') {
    printf("Tclvar_selector::addEntry: Warning: assuming global var in path and"
		"declaring it as such\n");
    sprintf(command, "global %s", &(d_tclWidgetName[1]));
    for (i = 7; i < strlen(d_tclWidgetName) + 7; i++){
      if (command[i] == '(' || command[i] == '.'){
        command[i] = '\0';
        break;
      }
    }
    if (Tcl_Eval(interpreter, command) != TCL_OK) {
      fprintf(stderr, "Tcl_Eval(%s) failed:  %s\n", command,
            interpreter->result);
      return -1;
    }
  }

  sprintf(command, "%s.menu delete {%s}",
          d_tclWidgetName, entry);
  if (Tcl_Eval(interpreter, command) != TCL_OK) {
    fprintf(stderr, "Tcl_Eval(%s) failed:  %s\n", command,
            interpreter->result);
    return -1;
  }

  return 0;
}

int Tclvar_selector::compareStrings (void) {
  return strcmp(d_myLastString, d_myString);
}

void Tclvar_selector::resetString (void) {
  strcpy(d_myLastString, d_myString);
}



nmb_Selector * allocate_Tclvar_selector (const char * initialValue) {
  return new Tclvar_selector (initialValue);
}


//-----------------------------------------------------------------------
// Checklist class.  First is listed the type of the data sent to the callback
// and the callback handler for the checkboxes in each list.

typedef	struct {
	Tclvar_checklist	*whos;
	Tclvar_Checkbox	        *which;
} Check_Parm;

//each button added to a checklist calls this callback. 
// This function activates the callback for the whole checklist,
// passing the name of the checkbox which caused the callback.
// static
void Tclvar_checklist::checklist_callback(vrpn_int32 /*value*/, void *userdata)
{
  Check_Parm * parm = (Check_Parm *) (userdata);

  parm->whos->doCallbacks(parm->which->name, (int) *parm->which->button);

}

Tclvar_checklist::Tclvar_checklist (const char * parent_name) :
  tcl_parent_name (NULL),
  num_checkboxes (0),
  d_callbacks (NULL),
  d_inCallback (VRPN_FALSE)
{

//fprintf(stderr, "Tclvar_checklist constructor\n");

	// Set the parent name.
	if (parent_name) {
		tcl_parent_name = new char[strlen(parent_name) + 1];
		if (tcl_parent_name == NULL) {
		  fprintf(stderr,
		    "Tclvar_checklist::Tclvar_checklist(): Out of memory\n");
		  return;
		}
		sprintf(tcl_parent_name,"%s",parent_name);
	} else {
		tcl_parent_name = NULL;
	}
};

Tclvar_checklist::~Tclvar_checklist (void) {
	int	i = 0;
//fprintf(stderr, "~Tclvar_checklist()\n");

	// Free the space for the name
	if (tcl_parent_name != NULL) { delete [] tcl_parent_name; }

	// Destroy all of the checkboxes 
	for (i = 0; i < num_checkboxes; i++) {
		delete checkboxes[i].button;
	}
}




void Tclvar_checklist::addCallback (Linkvar_Checkcall cb, void * userdata) {
  tclCheckCallbackEntry * newEntry;

  if (!cb) {
    //fprintf(stderr, "Tclvar_checklist::addCallback:  NULL handler.\n");
    return;
  }

  // Allocate and initialize new entry
  newEntry = new tclCheckCallbackEntry;
  if (!newEntry) {
    fprintf(stderr, "Tclvar_checklist::addCallback:  Out of memory.\n");
    return;
  }

  // Add this handler to the chain at the beginning (don't check to see
  // if it is already there, since duplication is okay).

  newEntry->handler = cb;
  newEntry->userdata = userdata;
  newEntry->next = d_callbacks;

  d_callbacks = newEntry;
}

void Tclvar_checklist::doCallbacks (const char * name, int value) {
  tclCheckCallbackEntry * e;

  if (d_inCallback) {
    return;
  }
  d_inCallback = VRPN_TRUE;
  for (e = d_callbacks; e; e = e->next) {
    (*e->handler)(name, value, e->userdata);
  }
  d_inCallback = VRPN_FALSE;
}








int	Tclvar_checklist::Lookup_checkbox (const char * checkbox_name) const
{
	int	i = 0;

	// Are there any checkboxes?
	if (num_checkboxes == 0) return -1;

	// Find this checkbox in the list
	while ( (i < num_checkboxes) &&
		strcmp(checkbox_name,checkboxes[i].name) ) {
		i++;
	};
	if (i >= num_checkboxes) {
		return -1;	// No such checkbox
	}

	return i;
}

int	Tclvar_checklist::Add_checkbox (const char * checkbox_name, int value)
{
	Check_Parm	*parm = new Check_Parm;
	char		button_name[1000];

	if (parm == NULL) {
	    fprintf(stderr,"Tclvar_checklist::Add_checkbox(): Can't do parm\n");
	    return -1;
	}

	if ( (checkbox_name == NULL) || (strlen(checkbox_name) == 0) ) {
	    fprintf(stderr,"Tclvar_checklist::Add_checkbox(): Empty name\n");
	    return -1;
	}

	// Make sure it's not already in the list
	if (Lookup_checkbox(checkbox_name) != -1) {
	    fprintf(stderr,"Tclvar_checklist::Add_checkbox(): Already there\n");
	    return -1;
	}

	// Make sure there is enough room for the new checkbox
	if (num_checkboxes >= NUM_CHECKBOXES) {
		fprintf(stderr,"Tclvar_checklist::Add_checkbox(): Too many\n");
		return -1;
	}

	// Create the new checkbox.
	// The name is that given as a parameter (truncated to fit within the
	// name length).  The name of the button has the parent name prepended,
	// to distinguish the checkboxes of different checklists.
	strncpy(checkboxes[num_checkboxes].name, checkbox_name,
		sizeof(checkboxes[num_checkboxes].name) - 1);
	checkboxes[num_checkboxes].name[sizeof(checkboxes[num_checkboxes].name) - 1] = '\0';

	if (tcl_parent_name) {
		strncpy(button_name, tcl_parent_name, sizeof(button_name));
	} else {
		button_name[0] = '\0';
	}
	if ( (strlen(button_name)+1) < sizeof(button_name)) {
		strcat(button_name,".");
	}
	strncat(button_name, checkbox_name,
		sizeof(button_name)-strlen(button_name) - 2);
	checkboxes[num_checkboxes].button =
		new Tclvar_int_with_button(button_name,tcl_parent_name, value);
	if (checkboxes[num_checkboxes].button == NULL) {
		fprintf(stderr,"Tclvar_checklist::Add_checkbox(): Out of mem\n");
		return -1;
	}
	num_checkboxes++;

	// Set up the callback for this variable.  This tells which checklist
	// set the callback and for which checkbox it was set.  This lets the
	// callback route the result correctly.
	parm->whos = this;
	parm->which = &checkboxes[num_checkboxes - 1];
	checkboxes[num_checkboxes - 1].button->
                         addCallback(checklist_callback, parm);

	return 0;
};

int	Tclvar_checklist::Remove_checkbox (const char * checkbox_name)
{
	int	i = Lookup_checkbox(checkbox_name);

	// Ensure that it is really in the list
	if (i == -1) { return -1; }

	// Delete the checkbox, move the last checkbox to this one
	// and decrement counter
	delete checkboxes[i].button;
	checkboxes[i] = checkboxes[num_checkboxes-1];
	num_checkboxes--;

	return 0;
};

int	Tclvar_checklist::Set_checkbox (const char * checkbox_name)
{
	int i = Lookup_checkbox(checkbox_name);

	if (i == -1) { return -1; }

	*(checkboxes[i].button) = 1;

	return 0;
}

int	Tclvar_checklist::Unset_checkbox (const char * checkbox_name)
{
	int i = Lookup_checkbox(checkbox_name);

	if (i == -1) { return -1; }

	*(checkboxes[i].button) = 0;

	return 0;
}

int	Tclvar_checklist::Is_set (const char *checkbox_name) const
{
	int i = Lookup_checkbox(checkbox_name);

	if (i == -1) { return -1; }

	return *(checkboxes[i].button);
}

const char * Tclvar_checklist::Checkbox_name (const int which) const
{
	if ( (which < 0) || (which >= num_checkboxes) ) { return NULL; }

	return checkboxes[which].name;
}

int	Tclvar_checklist::Is_set (const int which) const
{
	if ( (which < 0) || (which >= num_checkboxes) ) { return -1; }

	return *(checkboxes[which].button);
}

 









Tclvar_checklist_with_entry::Tclvar_checklist_with_entry
                      (const char * parent_name) :
   Tclvar_checklist (parent_name)
{
//fprintf(stderr, "Tclvar_checklist_with_entry constructor\n");

	if (num_checkentry >= (MAX_CHECKENTRY-1)) {
		fprintf(stderr,"Tclvar_checklist_with_entry::Tclvar_checklist_with_entry: (Out of storage)\n");
	} else {
		checkentry_list[num_checkentry] = this;
		num_checkentry++;
	}


	// Set the parent name.
	if (parent_name) {
		tcl_parent_name = new char[strlen(parent_name) + 1];
		if (tcl_parent_name == NULL) {
		  fprintf(stderr,
		    "Tclvar_checklist::Tclvar_checklist(): Out of memory\n");
		  return;
		}
		sprintf(tcl_parent_name,"%s",parent_name);
	} else {
		tcl_parent_name = NULL;
	}
};

Tclvar_checklist_with_entry::~Tclvar_checklist_with_entry (void)
{
	int	i = 0;
//fprintf(stderr, "~Tclvar_checklist_with_entry()\n");

	// Free the space for the name
	if (tcl_parent_name != NULL) { delete [] tcl_parent_name; }

	// Destroy all of the checkboxes and entries
	for (i = 0; i < num_checkboxes; i++) {
		delete checkboxes[i].button;
		if ( checkboxes[i].entry != NULL) delete checkboxes[i].entry;
	}
}

// Make the checkbox name into a widget name.
// Make sure the first letter is lower case, 
// and replace all ' ' with '_'
static void fix_checkbox_name (char * my_checkbox_name) {
	my_checkbox_name[0] = tolower(my_checkbox_name[0]);
	for (int i = 0; i < strlen(my_checkbox_name); i++)
	   if (my_checkbox_name[i] == ' ')
	      my_checkbox_name[i] = '_';
}

// pack the frames necessary for the checkbutton and entry
int	Tclvar_checklist_with_entry::initialize(Tcl_Interp *interpreter)
{
	char	command[1000];
	char * my_checkbox_name;

	int i;
	for (i = 0; i < num_checkboxes; i++) {
	   // create the frames needed. 
	   // This must be done before the checkbox and entry are initialized. 
	   my_checkbox_name = new char[strlen(checkboxes[i].name) + 1];
	   strcpy(my_checkbox_name, checkboxes[i].name);	
	   // make a suitable widget name
	   fix_checkbox_name(my_checkbox_name);

	   sprintf(command, "pack [frame %s.%sf] -expand yes -fill x", 
		   tcl_parent_name, my_checkbox_name);
	   TCLEVALCHECK(interpreter, command);
	   sprintf(command, "pack [frame %s.%sf.b] -side left", 
		   tcl_parent_name, my_checkbox_name);
	   TCLEVALCHECK(interpreter, command);
	   sprintf(command, "pack [frame %s.%sf.e] -side right", 
		   tcl_parent_name, my_checkbox_name);
	   TCLEVALCHECK(interpreter, command);
	   delete my_checkbox_name;
	}
	return 0;
}

// Adds one checkbox/entry combo to the list. Sets default values for both
// depends on the static variable "interpreter" to pack frames
int	Tclvar_checklist_with_entry::Add_checkbox_entry (const char * checkbox_name, int checkval, int entryval)
{
	Check_Parm	*parm = new Check_Parm;
	char		button_name[1000];
	char            entry_name[1000];
	char            command[500];
	char * my_checkbox_name;

	if (parm == NULL) {
	    fprintf(stderr,"Tclvar_checklist::add_checkbox_entry(): Can't do parm\n");
	    return -1;
	}

	if ( (checkbox_name == NULL) || (strlen(checkbox_name) == 0) ) {
	    fprintf(stderr,"Tclvar_checklist::add_checkbox_entry(): Empty name\n");
	    return -1;
	}

	// Make sure it's not already in the list
	if (Lookup_checkbox(checkbox_name) != -1) {
	    fprintf(stderr,"Tclvar_checklist::add_checkbox_entry(): Already there\n");
	    return -1;
	}

	// Make sure there is enough room for the new checkbox
	if (num_checkboxes >= NUM_CHECKBOXES) {
		fprintf(stderr,"Tclvar_checklist::add_checkbox_entry(): Too many\n");
		return -1;
	}

	// Create the new checkbox.  The name is that given as a
	// parameter (truncated to fit within the name length).  The
	// name of the button has the parent name prepended, to
	// distinguish the checkboxes of different checklists. It has
	// "button" postpended to distinguish it from the entry
	strncpy(checkboxes[num_checkboxes].name, checkbox_name,
		sizeof(checkboxes[num_checkboxes].name) - 1);
	checkboxes[num_checkboxes].name[sizeof(checkboxes[num_checkboxes].name) - 1] = '\0';

	my_checkbox_name = new char[strlen(checkboxes[num_checkboxes].name) + 1];
	strcpy(my_checkbox_name, checkboxes[num_checkboxes].name);	
	// make a suitable widget name
	fix_checkbox_name(my_checkbox_name);
	
	// create spots for the new checkbutton and entry
	// only if the interpreter exists. Otherwise, this will
	// be done when the interpreter is created. 
	if (tcl_parent_name && interpreter ) {
	   
	   sprintf(command, "pack [frame %s.%sf] -expand yes -fill x", 
		   tcl_parent_name, my_checkbox_name);
	   TCLEVALCHECK(interpreter, command);
	   sprintf(command, "pack [frame %s.%sf.b] -side left", 
		   tcl_parent_name, my_checkbox_name);
	   TCLEVALCHECK(interpreter, command);
	   sprintf(command, "pack [frame %s.%sf.e] -side right", 
		   tcl_parent_name, my_checkbox_name);
	   TCLEVALCHECK(interpreter, command);
	}

	if (tcl_parent_name) {
		strncpy(button_name, tcl_parent_name, sizeof(button_name));
		strncpy(entry_name, tcl_parent_name, sizeof(entry_name));
	} else {
		button_name[0] = '\0';
		entry_name[0] = '\0';
	}
	strncat(button_name, ".",
		sizeof(button_name)-strlen(button_name) - 2);
	strncat(button_name, my_checkbox_name,
		sizeof(button_name)-strlen(button_name) - 2);
	strncat(button_name, "f.b.",
		sizeof(button_name)-strlen(button_name) - 2);
	strncat(button_name, my_checkbox_name,
		sizeof(button_name)-strlen(button_name) - 2);
	// no, I'm not using it as a command, just a convenient string
	sprintf(command, "%s.%sf.b", tcl_parent_name, my_checkbox_name);
	checkboxes[num_checkboxes].button =
		new Tclvar_int_with_button(button_name,command, checkval);
	if (checkboxes[num_checkboxes].button == NULL) {
		fprintf(stderr,"Tclvar_checklist::add_checkbox_entry(): Out of mem\n");
		return -1;
	}

	// make the entry field, similarly. It has "entry" postpended
	// to distinguish it from the button.
	strncat(entry_name, ".",
		sizeof(entry_name)-strlen(entry_name) - 2);
	strncat(entry_name, my_checkbox_name,
		sizeof(entry_name)-strlen(entry_name) - 2);
	strncat(entry_name, "f.e.",
		sizeof(entry_name)-strlen(entry_name) - 2);
	strncat(entry_name, my_checkbox_name,
		sizeof(entry_name)-strlen(entry_name) - 2);
	// no, I'm not using it as a command, just a convenient string
	sprintf(command, "%s.%sf.e", tcl_parent_name, my_checkbox_name);
	checkboxes[num_checkboxes].entry =
		new Tclvar_int_with_entry(entry_name,command, entryval);
	if (checkboxes[num_checkboxes].entry == NULL) {
		fprintf(stderr,"Tclvar_checklist::add_checkbox_entry(): Out of mem\n");
		return -1;
	}



	num_checkboxes++;

	// Set up the callback for this variable.  This tells which checklist
	// set the callback and for which checkbox it was set.  This lets the
	// callback route the result correctly.
	parm->whos = this;
	parm->which = &checkboxes[num_checkboxes - 1];
	checkboxes[num_checkboxes - 1].button->
                        addCallback(checklist_callback, parm);
	checkboxes[num_checkboxes - 1].entry->
                        addCallback(checklist_callback, parm);
	delete my_checkbox_name;
	return 0;
};

int	Tclvar_checklist_with_entry::Remove_checkbox (const char * checkbox_name)
{
	int	i = Lookup_checkbox(checkbox_name);

	// Ensure that it is really in the list
	if (i == -1) { return -1; }

	// Delete the checkbox, move the last checkbox to this one
	// and decrement counter
	delete checkboxes[i].button;
	if ( checkboxes[i].entry != NULL) delete checkboxes[i].entry;
	checkboxes[i] = checkboxes[num_checkboxes-1];
	num_checkboxes--;

	return 0;
};

int	Tclvar_checklist_with_entry::Set_checkbox_entry (const char * checkbox_name, int entryval)
{
	int i = Lookup_checkbox(checkbox_name);

	if (i == -1) { return -1; }

	*(checkboxes[i].entry) = entryval;

	return 0;
}


int	Tclvar_checklist_with_entry::Get_entry_val (const char *checkbox_name) const
{
	int i = Lookup_checkbox(checkbox_name);

	if (i == -1) { return -1; }

	return *(checkboxes[i].entry);
}


