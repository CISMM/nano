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

#include <nmb_Selector.h>
#include <nmb_Types.h>  // for vrpn_bool

//struct Tcl_Interp;  // from tcl.h
#include <tcl.h>  // for Client_Data, Tcl_Interp

extern	int	Tclvar_init(Tcl_Interp *tcl_interp);
extern	int	Tclvar_mainloop(void);

class	Tclvar_int_with_button;
class	Tclvar_int_with_entry;
class	Tclvar_selector;
class	Tclvar_checklist;

typedef	void (* Linkvar_Intcall) (vrpn_int32 new_value, void * userdata);
typedef	void (* Linkvar_Floatcall) (vrpn_float64 new_value, void * userdata);
typedef	void (* Linkvar_Selectcall) (const char * new_value, void * userdata);
typedef	void (* Linkvar_Checkcall) (const char * checkbox,
                                    int new_value, void * userdata);

// Very useful for sending tcl commands from c to tcl. 
#define TCLEVALCHECK(interp, command) if (Tcl_Eval(interp, command)!= TCL_OK) \
                       {fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command, \
                                interp->result); return(-1);}
#define TCLEVALCHECK2(interp, command) if (Tcl_Eval(interp, command)!= TCL_OK) \
                       {fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command, \
                                interp->result); return;}

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
struct tclSelectCallbackEntry {
  Linkvar_Selectcall handler;
  void * userdata;
  tclSelectCallbackEntry * next;
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

	char	*my_tcl_varname;

	vrpn_int32	mylastint;

        vrpn_bool     d_dirty;
          // 1 if tcl variable needs to be update because of an error.
          // 0 otherwise

        vrpn_bool d_ignoreChange;
        vrpn_bool d_permitIdempotentChanges;

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
	Tclvar_float(const char * tcl_varname, vrpn_float64 default_value = 0.0,
		Linkvar_Floatcall c = NULL, void * ud = NULL);
	virtual	~Tclvar_float (void);

        void addCallback (Linkvar_Floatcall callback, void * userdata);
        void doCallbacks (void);

	inline operator vrpn_float64 () {return d_myfloat;}
	virtual vrpn_float64 operator = (vrpn_float64 v);

	char	*my_tcl_varname;

	vrpn_float64	mylastfloat;

        vrpn_bool     d_dirty;
          // 1 if tcl variable needs to be update because of an error.
          // 0 otherwise

        vrpn_bool d_ignoreChange;
        vrpn_bool d_permitIdempotentChanges;

    protected:

      virtual void SetFromTcl (vrpn_float64);

	vrpn_float64	d_myfloat;

        tclFloatCallbackEntry * d_callbacks;
        vrpn_bool d_inCallback;
        vrpn_bool d_updateFromTcl;

};




class	Tclvar_selector : public nmb_Selector {

    friend char * handle_string_value_change (ClientData clientData,
        Tcl_Interp *, char *, char *, int);

  public:

    Tclvar_selector (const char * initial_value = "");

    Tclvar_selector (const char * tcl_varname,
                     const char * parent_name,
                     nmb_ListOfStrings * list = NULL,
                     const char * initial_value = "",
                     Linkvar_Selectcall c = NULL,
                     void * userdata = NULL);

    virtual ~Tclvar_selector (void);

    virtual const char * operator = (const char *);
    virtual const char * operator = (char *);

    virtual void Set (const char *);

    void addCallback (Linkvar_Selectcall callback, void * userdata);
    void doCallbacks (void);

    virtual void initializeTcl (const char * tcl_varname,
                                const char * parent_name);

    virtual int bindList (nmb_ListOfStrings * list);

    int	initialize (Tcl_Interp * interpreter);
    int	initializeList (void);

    char * d_myTclVarname;
    char * d_tclWidgetName;
    char * d_tclLabelName;

    vrpn_bool d_dirty;
          // 1 if tcl variable needs to be update because of an error.
          // 0 otherwise

    vrpn_bool d_ignoreChange;
    vrpn_bool d_permitIdempotentChanges;

  protected:

    virtual void SetFromTcl (const char *);

    virtual int clearList ();
    virtual int addEntry (const char *);
    virtual int deleteEntry (const char *);

    // Expose a limited interface to d_myString and d_myLastString
    // without exposing them completely;  together with Set() these
    // are complete for our current needs.

    int compareStrings (void);
    void resetString (void);
      // copies myString over myLastString

    tclSelectCallbackEntry * d_callbacks;

    void updateTcl (void);

        vrpn_bool d_inCallback;
        vrpn_bool d_updateFromTcl;

  private:
    vrpn_bool d_initialized;
          // 0 until initialize() has been called;  1 thereafter
};

nmb_Selector * allocate_Tclvar_selector (const char * initialValue);


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
	virtual ~Tclvar_checklist (void);

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
	virtual ~Tclvar_checklist_with_entry (void);

	int	Add_checkbox_entry (const char *checkbox_name, int checkval, int entryval);
	int	Remove_checkbox (const char *checkbox_name);
	int	Set_checkbox_entry (const char *checkbox_name, int entryval);
	int	Get_entry_val (const char *checkbox_name) const;

	int	initialize(Tcl_Interp *interpreter);

};

#endif

