#ifndef TCL_NET_VAR_H
#define TCL_NET_VAR_H

//#include <sys/time.h>  // for struct timeval - get from vrpn_Shared
#include <stdio.h>

#include <vrpn_Shared.h>  // for architecture-independent sizes
#include <vrpn_Types.h>

#include <Tcl_Linkvar.h>

class vrpn_Connection;  // from vrpn_Connection.h
class vrpn_Shared_int32;  // from vrpn_SharedObject.h
class vrpn_Shared_float64;
class vrpn_Shared_String;

class TclNet_int : public Tclvar_int {

  public:

    TclNet_int (const char * tcl_varname, vrpn_int32 default_value = 0,
                Linkvar_Intcall c = NULL, void * userdata = NULL);
    virtual ~TclNet_int (void);

    // MANIPULATORS

    virtual vrpn_int32 operator = (vrpn_int32);
    virtual vrpn_int32 operator ++ (void);
    virtual vrpn_int32 operator ++ (int);
      // Require changing base class;  should also check the
      // return type of these against reference books.
      // All of these set the value of d_replica[0],
      // which will transmit the new value to the appropriate
      // vrpn_Shared_int32_Remotes.

    void bindConnection (vrpn_Connection *);
      // Don't insist that the vrpn_Connection be available in
      // the constructor, or we can't have globals.
    void addPeer (vrpn_Connection *);
      // Add a new peer connection, add a replica, and start
      // tracking changes.

    void copyReplica (int whichReplica);
      // Copy the state of the which-th replica.
      // ISSUE:  Need a better way of specifying which one (?)
      // ISSUE:  Probably should syncReplica(0) first?
    void syncReplica (int whichReplica);
      // Copy the state of the which-th replica, and any changes to it.
      // To "unsync", issue syncReplica(0)

  protected:

    timeval d_lastUpdate;
      // Time at which the last update was made, or the timestamp
      // propagated with the last conditional update.

    vrpn_Shared_int32 ** d_replica;
    vrpn_Connection ** d_replicaSource;

    int d_activeReplica;
    int d_numReplicas;
    int d_numReplicasAllocated;

    virtual vrpn_int32 conditionalEquals (vrpn_int32 value, timeval when);
      // Operator = overwrites myint and sets d_lastUpdate.
      // conditionalEquals() overwrites myint IFF when > d_lastUpdate;
      // otherwise it is ignored.  The timestamp is written into
      // d_lastUpdate if myint is overwritten.
      // Should ONLY be used as a helper function for propagateReceivedUpdate
      // because it plays with d_ignoreChange.

    virtual vrpn_int32 setLocally (vrpn_int32 value);

    static int propagateReceivedUpdate (void * userdata, vrpn_int32 newValue,
                                        timeval when);
      // Callback registered on the active Remote replica.  
      // Used to execute (*(NetTcl_int *)userdata = newValue).
      // Now executes (((NetTcl_int *) userdata)->conditionalEquals
      //   (newValue, when).

};

class TclNet_float : public Tclvar_float {

  public:

    TclNet_float (const char * tcl_varname, vrpn_float64 default_value = 0,
                Linkvar_Floatcall c = NULL, void * userdata = NULL);
    virtual ~TclNet_float (void);

    // MANIPULATORS

    virtual vrpn_float64 operator = (vrpn_float64);
      // Require changing base class;  should also check the
      // return type of these against reference books.
      // All of these set the value of d_replica[0],
      // which will transmit the new value to the appropriate
      // vrpn_Shared_float64_Remotes.

    void bindConnection (vrpn_Connection *);
      // Don't insist that the vrpn_Connection be available in
      // the constructor, or we can't have globals.
    void addPeer (vrpn_Connection *);
      // Add a new peer connection, add a replica, and start
      // tracking changes.

    void copyReplica (int whichReplica);
      // Copy the state of the which-th replica.
      // ISSUE:  Need a better way of specifying which one (?)
      // ISSUE:  Probably should syncReplica(0) first?
    void syncReplica (int whichReplica);
      // Copy the state of the which-th replica, and any changes to it.
      // To "unsync", issue syncReplica(0)

  protected:

    timeval d_lastUpdate;
      // Time at which the last update was made, or the timestamp
      // propagated with the last conditional update.

    vrpn_Shared_float64 ** d_replica;
    vrpn_Connection ** d_replicaSource;

    int d_activeReplica;
    int d_numReplicas;
    int d_numReplicasAllocated;

    virtual vrpn_float64 conditionalEquals (vrpn_float64 value, timeval when);
      // Operator = overwrites myint and sets d_lastUpdate.
      // conditionalEquals() overwrites myint IFF when > d_lastUpdate;
      // otherwise it is ignored.  The timestamp is written into
      // d_lastUpdate if myint is overwritten.
      // Should ONLY be used as a helper function for propagateReceivedUpdate
      // because it plays with d_ignoreChange.

    virtual vrpn_float64 setLocally (vrpn_float64 value);

    static int propagateReceivedUpdate (void * userdata, vrpn_float64 newValue,
                                        timeval when);
      // Callback registered on the active Remote replica.  
      // Used to execute (*(NetTcl_float *)userdata = newValue).
      // Now executes (((NetTcl_float *) userdata)->conditionalEquals
      //   (newValue, when).

};

class TclNet_selector : public Tclvar_selector {

  public:

    TclNet_selector (const char * initial_value = NULL);
    TclNet_selector (const char * tcl_varname,
                     const char * parent_name,
                     const char * default_value = NULL,
                     Linkvar_Selectcall c = NULL, void * userdata = NULL);
    virtual ~TclNet_selector (void);

    // MANIPULATORS

    virtual const char * operator = (const char *);
    virtual const char * operator = (char *);
      // For some unknown reason the const char * arg'd version
      // sometimes doesn't resolve correctly?

    virtual void initializeTcl (const char * tcl_varname,
                                const char * parent_name);


    void bindConnection (vrpn_Connection *);
      // Don't insist that the vrpn_Connection be available in
      // the constructor, or we can't have globals.
    void addPeer (vrpn_Connection *);
      // Add a new peer connection, add a replica, and start
      // tracking changes.

    void copyReplica (int whichReplica);
      // Copy the state of the which-th replica.
      // ISSUE:  Need a better way of specifying which one (?)
      // ISSUE:  Probably should syncReplica(0) first?
    void syncReplica (int whichReplica);
      // Copy the state of the which-th replica, and any changes to it.
      // To "unsync", issue syncReplica(0)

  protected:

    timeval d_lastUpdate;
      // Time at which the last update was made, or the timestamp
      // propagated with the last conditional update.

    vrpn_Shared_String ** d_replica;
    vrpn_Connection ** d_replicaSource;

    int d_activeReplica;
    int d_numReplicas;
    int d_numReplicasAllocated;

    virtual const char * conditionalEquals (const char * value, timeval when);
      // Operator = overwrites myint and sets d_lastUpdate.
      // conditionalEquals() overwrites myint IFF when > d_lastUpdate;
      // otherwise it is ignored.  The timestamp is written into
      // d_lastUpdate if myint is overwritten.
      // Should ONLY be used as a helper function for propagateReceivedUpdate
      // because it plays with d_ignoreChange.

    virtual const char * setLocally (const char * value);

    static int propagateReceivedUpdate (void * userdata, const char * newValue,
                                        timeval when);
      // Callback registered on the active Remote replica.
      // Used to execute (*(NetTcl_float *)userdata = newValue).
      // Now executes (((NetTcl_float *) userdata)->conditionalEquals
      //   (newValue, when).

};

nmb_Selector * allocate_TclNet_selector (const char * initialValue);


class	Tclvar_int_with_button: public TclNet_int {
    friend class Tclvar_checklist;
    public:
	Tclvar_int_with_button
               (const char * tcl_varname, const char * parent_name,
		vrpn_int32 default_value = 0,
                Linkvar_Intcall c=NULL, void *ud = NULL);
	virtual ~Tclvar_int_with_button (void);

	int	initialize(Tcl_Interp *interpreter);  // NOT VIRTUAL

	inline operator vrpn_int32() {return d_myint;}
	virtual vrpn_int32 operator = (vrpn_int32 v);

	virtual vrpn_int32 operator ++ (void);
	virtual vrpn_int32 operator ++ (int);

	char            *tcl_widget_name;
};


//	When variables of this class are declared, the calling program must
// have had Tcl load the file "russ_widgets.tcl" in order to include the
// floatscale function definition.
//	This variable type will automatically display a floatscale inside
// the parent widget (unless the parent has NULL name) to track the value and
// allow its control.

class	Tclvar_float_with_scale: public TclNet_float {
    public:
	Tclvar_float_with_scale
               (const char *tcl_varname,
                const char *parent_name,
		vrpn_float64 minval, vrpn_float64 maxval,
                vrpn_float64 default_value = 0,
		Linkvar_Floatcall c = NULL, void *ud = NULL);
	virtual	~Tclvar_float_with_scale (void);

	int	initialize(Tcl_Interp *interpreter);

	inline operator vrpn_float64() {return d_myfloat;}
	virtual vrpn_float64 operator = (vrpn_float64 v);

	char		*tcl_widget_name;
	char		*tcl_label_name;
	vrpn_float64		minvalue, maxvalue;
};

//	When variables of this class are declared, the calling program must
// have had Tcl load the file "russ_widgets.tcl" in order to include the
// intscale function definition.
//	This variable type will automatically display an intscale inside
// the parent widget (unless the parent has NULL name) to track the value and
// allow its control.

class	Tclvar_int_with_scale: public TclNet_int {
    public:
	Tclvar_int_with_scale
               (const char * tcl_varname, const char * parent_name,
		vrpn_int32 minval, vrpn_int32 maxval,
                vrpn_int32 default_value = 0,
		Linkvar_Intcall c = NULL, void *ud = NULL);
	virtual	~Tclvar_int_with_scale (void);

	int	initialize(Tcl_Interp *interpreter);  // NOT VIRTUAL

	inline operator vrpn_int32() {return d_myint;}
	virtual vrpn_int32 operator = (vrpn_int32 v);

	char		*tcl_widget_name;
	char		*tcl_label_name;
	vrpn_int32		minvalue, maxvalue;
};

//	When variables of this class are declared, the calling program must
// have had Tcl load the package [incr tcl] in order to include the 
// entryfield function definition. 
//	This variable type will automatically display an intentry inside
// the parent widget (unless the parent has NULL name) to track the value and
// allow its control.

class	Tclvar_int_with_entry: public TclNet_int {
    public:
	Tclvar_int_with_entry
               (const char *tcl_varname, const char *parent_name,
		vrpn_int32 default_value = 0,
		Linkvar_Intcall c = NULL, void *ud = NULL);
	virtual	~Tclvar_int_with_entry (void);

	int	initialize(Tcl_Interp *interpreter);  // NOT VIRTUAL

	inline operator vrpn_int32() {return d_myint;}
	virtual vrpn_int32 operator = (vrpn_int32 v);

	char		*tcl_widget_name;
	char		*tcl_label_name;
};


#endif  // TCL_NET_VAR_H
