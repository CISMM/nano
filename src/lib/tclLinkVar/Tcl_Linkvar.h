/////////////////////////////////////////////////////////////////////////////
//	Defines classes of variables that will link themselves with Tcl
// variables.  They are meant to make it easy to create new variables and
// link them with Tcl variables.
//	They provide two-way links, with change in either Tcl or C changing
// the other.
//	The Tclvar_init() routine must be called to link all of them.  Each
// time through the main loop, Tclvar_mainloop() must be called to make any
// changes.
/////////////////////////////////////////////////////////////////////////////

#ifndef	TCL_LINKVAR_H
#define	TCL_LINKVAR_H

#include <stdlib.h>  // for NULL
#include <stdio.h>
#include <vrpn_Types.h>

#include <nmb_String.h>
#include <nmb_Types.h>  // for vrpn_bool

//struct Tcl_Interp;  // from tcl.h
#include <tcl.h>  // for Client_Data, Tcl_Interp

extern	int	Tclvar_init(Tcl_Interp *tcl_interp);
extern	int	Tclvar_mainloop(void);

class	Tclvar_int_with_button;
class	Tclvar_int_with_entry;
class	Tclvar_string;
class	Tclvar_checklist;

typedef	void (* Linkvar_Intcall) (vrpn_int32 new_value, void * userdata);
typedef	void (* Linkvar_Floatcall) (vrpn_float64 new_value, void * userdata);
typedef	void (* Linkvar_Stringcall) (const char * new_value, void * userdata);
typedef	void (* Linkvar_ListOfStringscall) (const char * new_value, void * userdata);
typedef	void (* Linkvar_Checkcall) (const char * checkbox,
                                    int new_value, void * userdata);

// Very useful for sending tcl commands from c to tcl. 
#define TCLEVALCHECK(interp, command) if (Tcl_Eval(interp, command)!= TCL_OK) \
                       {fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command, \
                                Tcl_GetStringResult(interp)); return(-1);}
#define TCLEVALCHECK2(interp, command) if (Tcl_Eval(interp, command)!= TCL_OK) \
                       {fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command, \
                                Tcl_GetStringResult(interp)); return;}

// TCH 1 Oct 97
#define TCLVAR_STRING_LENGTH 128

#define NUM_ENTRIES 100
#define NUM_CHECKBOXES 100

struct tclIntCallbackEntry {
  Linkvar_Intcall handler;
  void * userdata;
  tclIntCallbackEntry * next;
};
struct tclFloatCallbackEntry {
  Linkvar_Floatcall handler;
  void * userdata;
  tclFloatCallbackEntry * next;
};
struct tclStringCallbackEntry {
  Linkvar_Stringcall handler;
  void * userdata;
  tclStringCallbackEntry * next;
};
struct tclListOfStringsCallbackEntry {
  Linkvar_ListOfStringscall handler;
  void * userdata;
  tclListOfStringsCallbackEntry * next;
};
struct tclCheckCallbackEntry {
  Linkvar_Checkcall handler;
  void * userdata;
  tclCheckCallbackEntry * next;
};


class	Tclvar_int {

    friend char * handle_int_value_change (ClientData clientData,
        Tcl_Interp *, char *, char *, int);

    public:
	Tclvar_int(const char * tcl_varname, vrpn_int32 default_value = 0,
		   Linkvar_Intcall c = NULL, void * ud = NULL);
	virtual	~Tclvar_int (void);

        void addCallback (Linkvar_Intcall callback, void * userdata);
        void doCallbacks (void);

	inline operator vrpn_int32 (void) {return d_myint;}
	virtual vrpn_int32 operator = (vrpn_int32 v);

	virtual vrpn_int32 operator ++ (void);
	virtual vrpn_int32 operator ++ (int);

	char	*d_myTclVarname;

	vrpn_int32	mylastint;

        vrpn_bool     d_dirty;
          // 1 if tcl variable needs to be update because of an error.
          // 0 otherwise

        vrpn_bool d_ignoreChange;
        vrpn_bool d_permitIdempotentChanges;

        // These two functions are NOT defined;  they should
        // generate compile-time errors.
        Tclvar_int (const Tclvar_int &);
        Tclvar_int & operator = (const Tclvar_int &);

    protected:

      virtual void SetFromTcl (vrpn_int32);

	vrpn_int32	d_myint;

        tclIntCallbackEntry * d_callbacks;

        vrpn_bool d_inCallback;
        vrpn_bool d_updateFromTcl;

        void updateTcl (void);
};

class	Tclvar_float {

    friend char * handle_float_value_change (ClientData clientData,
        Tcl_Interp *, char *, char *, int);

    public:
	Tclvar_float(const char * tcl_varname, 
		     vrpn_float64 default_value = 0.0,
		     Linkvar_Floatcall c = NULL, 
		     void * ud = NULL);
	virtual	~Tclvar_float (void);

        void addCallback (Linkvar_Floatcall callback, void * userdata);
        void doCallbacks (void);

	inline operator vrpn_float64 () {return d_myfloat;}
	virtual vrpn_float64 operator = (vrpn_float64 v);

	char	*d_myTclVarname;

	vrpn_float64	mylastfloat;

        vrpn_bool     d_dirty;
          // 1 if tcl variable needs to be update because of an error.
          // 0 otherwise

        vrpn_bool d_ignoreChange;
        vrpn_bool d_permitIdempotentChanges;

        // These two functions are NOT defined;  they should
        // generate compile-time errors.
        Tclvar_float (const Tclvar_float &);
        Tclvar_float & operator = (const Tclvar_float &);

    protected:

      virtual void SetFromTcl (vrpn_float64);

	vrpn_float64	d_myfloat;

        tclFloatCallbackEntry * d_callbacks;

        vrpn_bool d_inCallback;
        vrpn_bool d_updateFromTcl;

        void updateTcl (void);

};




class	Tclvar_string : public nmb_String {

    friend char * handle_string_value_change (ClientData clientData,
        Tcl_Interp *, char *, char *, int);
 
  public:

    Tclvar_string (const char * initial_value = "");

    Tclvar_string (const char * tcl_varname,
                     const char * initial_value,
                     Linkvar_Stringcall c = NULL,
                     void * userdata = NULL);

    virtual ~Tclvar_string (void);

    void addCallback (Linkvar_Stringcall callback, void * userdata);
    void doCallbacks (void);

    virtual const char * operator = (const char *);
    virtual const char * operator = (char *);

    virtual void Set (const char *);

    virtual void initializeTcl (const char * tcl_varname);

    char * d_myTclVarname;

    vrpn_bool d_dirty;
          // 1 if tcl variable needs to be update because of an error.
          // 0 otherwise

    vrpn_bool d_ignoreChange;
    vrpn_bool d_permitIdempotentChanges;

        // These two functions are NOT defined;  they should
        // generate compile-time errors.
        Tclvar_string (const Tclvar_string &);
        Tclvar_string & operator = (const Tclvar_string &);

  protected:

    virtual void SetFromTcl (const char *);

    // Expose a limited interface to d_myString and d_myLastString
    // without exposing them completely;  together with Set() these
    // are complete for our current needs.

    int compareStrings (void);
    void resetString (void);
      // copies myString over myLastString

    tclStringCallbackEntry * d_callbacks;
    
    void updateTcl (void);

        vrpn_bool d_inCallback;
        vrpn_bool d_updateFromTcl;

  private:
    vrpn_bool d_initialized;
          // 0 until initialize() has been called;  1 thereafter
};

nmb_String * allocate_Tclvar_string (const char * initialValue);

class	Tclvar_list_of_strings : public nmb_ListOfStrings {

    friend char * handle_list_of_strings_value_change (ClientData clientData,
        Tcl_Interp *, char *, char *, int);
   friend int	Tclvar_init(Tcl_Interp *tcl_interp);

  public:

    Tclvar_list_of_strings ();

    Tclvar_list_of_strings (const char * tcl_varname,
                     Linkvar_ListOfStringscall c = NULL,
                     void * userdata = NULL);

    ~Tclvar_list_of_strings (void);

    //    virtual const char * operator = (const char *);
    //    virtual const char * operator = (char *);

    //    virtual void Set (const char *);

    void addCallback (Linkvar_ListOfStringscall callback, void * userdata);
    void doCallbacks (void);

    virtual void initializeTcl (const char * tcl_varname);

    char * d_myTclVarname;

    vrpn_bool d_dirty;
          // 1 if tcl variable needs to be update because of an error.
          // 0 otherwise

    vrpn_bool d_ignoreChange;
    vrpn_bool d_permitIdempotentChanges;

    virtual int clearList ();
    virtual int addEntry (const char *);
    virtual int deleteEntry (const char *);
    virtual int copyList(nmb_ListOfStrings *);

        // These two functions are NOT defined;  they should
        // generate compile-time errors.
        Tclvar_list_of_strings (const Tclvar_list_of_strings &);
        Tclvar_list_of_strings & operator = (const Tclvar_list_of_strings &);

  protected:

    //virtual void SetFromTcl (const char *);

    // Expose a limited interface to d_myString and d_myLastString
    // without exposing them completely;  together with Set() these
    // are complete for our current needs.

    //    int compareStrings (void);
    //    void resetString (void);
      // copies myString over myLastString

    tclListOfStringsCallbackEntry * d_callbacks;

    void updateTcl (void);
    
private:
    // holds the string representation of the list
    // so it can be communicated to the tcl interpreter.
    char * d_myString;

};

nmb_ListOfStrings * allocate_Tclvar_list_of_strings ();


//	This class maintains a checklist of items, each of which has a
// string name and a state.  The state is either on or off (0 or 1).  It
// instantiates checkboxes to display the states.  It uses a array
// as its items to choose from.

typedef	struct {
	char			name[60];	// Name of the checkbox
	Tclvar_int_with_button	*button;	// Button for the checkbox
        Tclvar_int_with_entry *entry;           // Entry for checkbox -see checklist_with_entry
} Tclvar_Checkbox;

class	Tclvar_checklist {
    public:
	Tclvar_checklist (const char * parent_name);
	~Tclvar_checklist (void);

        void addCallback (Linkvar_Checkcall, void * userdata);

	int	Add_checkbox (const char *checkbox_name, int value = 0);
	int	Remove_checkbox (const char *checkbox_name);
	int	Set_checkbox (const char *checkbox_name);
	int	Unset_checkbox (const char *checkbox_name);
	int	Is_set (const char *checkbox_name) const;

	int	Num_checkboxes (void) const {return num_checkboxes;};
	const char * Checkbox_name (const int which) const;
	int	Is_set (const int which) const;

	char			*tcl_parent_name;	// Parent widget

        // These two functions are NOT defined;  they should
        // generate compile-time errors.
        Tclvar_checklist (const Tclvar_checklist &);
        Tclvar_checklist & operator = (const Tclvar_checklist &);

    protected:
	Tclvar_Checkbox	checkboxes [NUM_CHECKBOXES];
	int num_checkboxes;	// How many are in the list

	int	Lookup_checkbox (const char * name) const;

        tclCheckCallbackEntry * d_callbacks;

        vrpn_bool d_inCallback;

        void doCallbacks (const char * buttonName, int buttonValue);
          // Trigger callbacks because the button on this checklist
          // named "buttonName" just changed to value "buttonValue"

        static void checklist_callback (vrpn_int32, void *);
          // Callback that is automatically registered on each
          // button to trigger doCallbacks.
};


class	Tclvar_checklist_with_entry : public Tclvar_checklist {
    public:
	Tclvar_checklist_with_entry (const char * parent_name);
	~Tclvar_checklist_with_entry (void);

	int	Add_checkbox_entry (const char *checkbox_name, int checkval, int entryval);
	int	Remove_checkbox (const char *checkbox_name);
	int	Set_checkbox_entry (const char *checkbox_name, int entryval);
	int	Get_entry_val (const char *checkbox_name) const;

	int	initialize(Tcl_Interp *interpreter);

};

#endif

