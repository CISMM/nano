#ifndef TCL_NET_VAR_H
#define TCL_NET_VAR_H

#include <vrpn_Shared.h>  // for architecture-independent sizes
#include <vrpn_Types.h>

//#include <sys/time.h>  // for struct timeval - get from vrpn_Shared
#include <stdio.h>

#include <Tcl_Linkvar.h>

class vrpn_Connection;  // from vrpn_Connection.h
class vrpn_SharedObject;  // from vrpn_SharedObject.h
class vrpn_Shared_int32;
class vrpn_Shared_float64;
class vrpn_Shared_String;

class Tcl_Interp;  // from tcl.h

void Tclnet_init (Tcl_Interp *, vrpn_bool useOptimisim = VRPN_FALSE);
/**<
 * Initializes Network-synchronized, replicated Tcl Variables,
 * specifying Tcl interpreter to use and choosing between locking
 * and optimistic consistiency control.
 */


/** \class Tcl_Netvar
 * Common behavior for all Network-synchronized, replicated Tcl Variables.
 * Once we have template support all the derived classes
 * should collapse into here.
 * \see TclNet_int
 * \see TclNet_float
 * \see TclNet_string
 */

class Tcl_Netvar {

  public:

    Tcl_Netvar (void);
    virtual ~Tcl_Netvar (void);

    // ACCESSORS

    vrpn_bool isSerializer (void) const;
      /**<
       * Returns true if this process is serializing updates to the
       * shared variable.
       */
    vrpn_bool isLocked (void) const;
      /**<
       * Returns true if this shared variable is locked by this
       * process.
       * @see lock()
       * @see unlock()
       */


    // MANIPULATORS

    void bindConnection (vrpn_Connection *);
      /**<
       * Specifies the server connection for the Netvar.
       * If we required passing the server connection to the constructor,
       * we wouldn't be able to have globals.
       */
    void bindLogConnection (vrpn_Connection *);
      /**<
       * Specifies a connection on which to write out a log.
       */

    void addPeer (vrpn_Connection *, vrpn_bool serialize);
      /**<
       * Adds a new replica for this variable.
       * If serialize is true, we're a "master copy" and this replica
       * communicates over the connection passed in bindConnection().
       * If serialize is false, we're a "slave copy" and this replica
       * communicates over the connection passed as its first argument.
       * This is the ugliest function in the class, and should be redesigned.
       */


    void lock (void);
      /**<
       * UNIMPLEMENTED support for remotely locking and unlocking variables.
       */
    void unlock (void);
      /**<
       * UNIMPLEMENTED support for remotely locking and unlocking variables.
       */

  protected:

    virtual int addServerReplica (void) = 0;
      /**<
       * Adds a new vrpn_Shared_*_Server to an empty slot in d_replica[],
       * sets the corresponding entry in d_replicaSource[], and returns
       * the index used (which should be d_numReplicas), or -1 on error.
       * \warning All implementations in derived classes must first call
       * reallocateReplicaArrays() to make sure there's room for the new
       * replica.
       */

    virtual int addRemoteReplica (void) = 0;
      /**<
       * Adds a new vrpn_Shared_*_Remote to an empty slot in d_replica[],
       * sets the corresponding entry in d_replicaSource[], and returns
       * the index used (which should be d_numReplicas), or -1 on error.
       * \warning All implementations in derived classes must first call
       * reallocateReplicaArrays() to make sure there's room for the new
       * replica.
       */

    void reallocateReplicaArrays (void);
      /**<
       * Makes sure there's room for a new entry in d_replica[] and
       * d_replicaSource[].
       * \warning Must be called by any implementation of add*Replica().
       */

    timeval d_lastUpdate;
      /**<
       * Time at which the last update was made, or the timestamp
       * propagated with the last conditional update.
       * Mostly used for optimistic updates.
       */

    vrpn_SharedObject ** d_replica;
    vrpn_Connection ** d_replicaSource;

    int d_writeReplica;
    int d_activeReplica;  // for reads
    int d_numReplicas;
    int d_numReplicasAllocated;

  private:

    vrpn_bool d_isLocked;

};





class TclNet_int : public Tclvar_int, public Tcl_Netvar {

  public:

    TclNet_int (const char * tcl_varname, vrpn_int32 default_value = 0,
                Linkvar_Intcall c = NULL, void * userdata = NULL);
    virtual ~TclNet_int (void);

    // ACCESSORS


    // MANIPULATORS

    virtual vrpn_int32 operator = (vrpn_int32);

    void copyReplica (int whichReplica);
      // Copy the state of the which-th replica.
      // ISSUE:  Need a better way of specifying which one (?)
      // ISSUE:  Probably should syncReplica(0) first?
    void copyFromToReplica (int sourceReplica, int destReplica);
      // Copy the state of the source replica to the destination replica.
    void syncReplica (int whichReplica);
      // Copy the state of the which-th replica, and any changes to it.
      // To "unsync", issue syncReplica(0)

    // These two functions are NOT defined;  they should
    // generate compile-time errors.
    TclNet_int (const TclNet_int &);
    TclNet_int & operator = (const TclNet_int &);

  protected:

    virtual int addServerReplica (void);
    virtual int addRemoteReplica (void);

    virtual void SetFromTcl (vrpn_int32);

    static int propagateReceivedUpdate (void * userdata, vrpn_int32 newValue,
                                        timeval when, vrpn_bool isLocal);
      // Callback registered on the active Remote replica.  

};

class TclNet_float : public Tclvar_float, public Tcl_Netvar {

  public:

    TclNet_float (const char * tcl_varname, vrpn_float64 default_value = 0,
                Linkvar_Floatcall c = NULL, void * userdata = NULL);
    virtual ~TclNet_float (void);

    // ACCESSORS


    // MANIPULATORS

    virtual vrpn_float64 operator = (vrpn_float64);

    void copyReplica (int whichReplica);
      // Copy the state of the which-th replica.
      // ISSUE:  Need a better way of specifying which one (?)
      // ISSUE:  Probably should syncReplica(0) first?
    void copyFromToReplica (int sourceReplica, int destReplica);
      // Copy the state of the source replica to the destination replica.
    void syncReplica (int whichReplica);
      // Copy the state of the which-th replica, and any changes to it.
      // To "unsync", issue syncReplica(0)

    // These two functions are NOT defined;  they should
    // generate compile-time errors.
    TclNet_float (const TclNet_float &);
    TclNet_float & operator = (const TclNet_float &);

  protected:

    virtual int addServerReplica (void);
    virtual int addRemoteReplica (void);

    virtual void SetFromTcl (vrpn_float64);

    static int propagateReceivedUpdate (void * userdata, vrpn_float64 newValue,
                                        timeval when, vrpn_bool isLocal);
      // Callback registered on the active Remote replica.  

};

class TclNet_string : public Tclvar_string, public Tcl_Netvar {

  public:

    TclNet_string (const char * initial_value = "");
    TclNet_string (const char * tcl_varname,
                     const char * default_value,
                     Linkvar_Stringcall c = NULL, void * userdata = NULL);
    virtual ~TclNet_string (void);

    // ACCESSORS


    // MANIPULATORS

    virtual const char * operator = (const char *);
    virtual const char * operator = (char *);
      // For some unknown reason the const char * arg'd version
      // sometimes doesn't resolve correctly?
    virtual void Set (const char *);

    virtual void initializeTcl (const char * tcl_varname);


    void copyReplica (int whichReplica);
      // Copy the state of the which-th replica.
      // ISSUE:  Need a better way of specifying which one (?)
      // ISSUE:  Probably should syncReplica(0) first?
    void copyFromToReplica (int sourceReplica, int destReplica);
      // Copy the state of the source replica to the destination replica.
    void syncReplica (int whichReplica);
      // Copy the state of the which-th replica, and any changes to it.
      // To "unsync", issue syncReplica(0)

    // These two functions are NOT defined;  they should
    // generate compile-time errors.
    TclNet_string (const TclNet_string &);
    TclNet_string & operator = (const TclNet_string &);

  protected:

    virtual int addServerReplica (void);
    virtual int addRemoteReplica (void);

    virtual void SetFromTcl (const char *);

    static int propagateReceivedUpdate (void * userdata, const char * newValue,
                                        timeval when, vrpn_bool isLocal);
      // Callback registered on the active Remote replica.

};

nmb_String * allocate_TclNet_string (const char * initialValue);


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

    // These two functions are NOT defined;  they should
    // generate compile-time errors.
    Tclvar_int_with_button (const Tclvar_int_with_button &);
    Tclvar_int_with_button & operator = (const Tclvar_int_with_button &);

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

    // These two functions are NOT defined;  they should
    // generate compile-time errors.
    Tclvar_int_with_entry (const Tclvar_int_with_entry &);
    Tclvar_int_with_entry & operator = (const Tclvar_int_with_entry &);
};




#endif  // TCL_NET_VAR_H
