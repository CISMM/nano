#ifndef NMUI_COMPONENT_H
#define NMUI_COMPONENT_H

#include <stdlib.h>  // for NULL

#include <vrpn_Connection.h>  // for vrpn_HANDLERPARAM
#include <vrpn_Types.h>  // for vrpn_bool

#define NMUI_COMPONENT_MAX_SIZE 100
  // No more than 100 linkvars of each type in the component.

// name is limited to 30 characters so that we can fit in a VRPN
// sender ID

//class Tcl_Interp;  // from <tcl.h>
class TclNet_int;  // from <Tcl_Netvar.h>
class TclNet_float;
class TclNet_selector;

//class nmui_ComponentSync;

class vrpn_Connection;  // from <vrpn_Connection.h>

// We could factor out the common interface from this and from
// TclNet_foo (bindConnection, addPeer, copyReplica, syncReplica)
// and use it as a base class;  that might let us manage a hierarchy
// easier.

typedef int (* nmui_SyncRequestHandler) (void * userdata);
typedef int (* nmui_SyncCompleteHandler) (void * userdata);
typedef int (* nmui_LockHandler) (void * userdata);

class nmui_Component {

  public:

    // CONSTRUCTORS

    nmui_Component (char name [30],
                    TclNet_int ** = NULL, int = 0,
                    TclNet_float ** = NULL, int = 0,
                    TclNet_selector ** = NULL, int = 0);

    virtual ~nmui_Component (void);

    // ACCESSORS

    const char * name (void) const;

    int numPeers (void) const;

    int synchronizedTo (void) const;

    vrpn_bool isLockedRemotely (void) const;
    vrpn_bool holdsRemoteLocks (void) const;

    // MANIPULATORS

    void add (TclNet_int *);
    void add (TclNet_float *);
    void add (TclNet_selector *);
    void add (nmui_Component *);

    void bindConnection (vrpn_Connection *);
    void addPeer (vrpn_Connection *, vrpn_bool serialize);

    void copyReplica (int whichReplica);
    void syncReplica (int whichReplica);

    void requestSync (void);
    void registerSyncRequestHandler (nmui_SyncRequestHandler, void *);
    void registerSyncCompleteHandler (nmui_SyncCompleteHandler, void *);
      // Must not be called before bindConnection()

      // Most processes write any changes into their TclNet objects,
      // so a remote Component that wants to sync will have a copy of
      // the current state in the replicas in those objects.  However,
      // some of the "simplest" processes that we want to synchronize
      // are not entirely user-controlled but instead change as a function
      // of time.  So, when a user requests synchronization copying the
      // current value of the appropriate replica is the *wrong* thing
      // to do.  Instead we send a syncRequest message, and a
      // syncRequestHandler on the remote machine can trigger updates
      // of replica[0] on the relevant TclNet objects to their proper
      // current values.

    // Machine A calls requestSync()
    // Machine B's SyncRequestHandlers are called()
    // all updates from the SyncRequestHandlers arrive on machine A
    // Machine A's SyncCompleteHandlers are called()

    vrpn_bool d_maintain;

    static int handle_reconnect (void *, vrpn_HANDLERPARAM);
      // re-registers type and sender ids.  You will probably
      // want to have this as a callback for vrpn_got_connection
      // on connections you addPeer().
      // HACK:  assumes there's only been one peer added that
      // could be outstanding.

    void initializeConnection (vrpn_Connection *);
      // to be called on a connection on which we should recieve
      // and respond to synchronization requests

  protected:

  private:

    char d_name [31];

    int d_numInts;
    TclNet_int * d_ints [NMUI_COMPONENT_MAX_SIZE];
    int d_numFloats;
    TclNet_float * d_floats [NMUI_COMPONENT_MAX_SIZE];
    int d_numSelectors;
    TclNet_selector * d_selectors [NMUI_COMPONENT_MAX_SIZE];
    int d_numComponents;
    nmui_Component * d_components [NMUI_COMPONENT_MAX_SIZE];

    //static vrpn_bool s_dataFromSync;

    int d_synchronizedTo;

    int d_numPeers;
    vrpn_bool d_isLockedRemotely;
    vrpn_bool d_holdsRemoteLocks;

    vrpn_Connection * d_connection;
    vrpn_int32 d_myId;
    vrpn_int32 d_syncRequest_type;
    vrpn_int32 d_syncComplete_type;
    vrpn_int32 d_lock_type;
    vrpn_int32 d_unlock_type;

    struct reqHandlerEntry {
      nmui_SyncRequestHandler handler;
      void * userdata; 
      reqHandlerEntry * next;
    };
    reqHandlerEntry * d_reqHandlers;
    struct completeHandlerEntry {
      nmui_SyncCompleteHandler handler;
      void * userdata; 
      completeHandlerEntry * next;
    };
    completeHandlerEntry * d_completeHandlers;
    struct lockHandlerEntry {
      nmui_LockHandler handler;
      void * userdata;
      lockHandlerEntry * next;
    };
    lockHandlerEntry * d_lockHandlers;
    lockHandlerEntry * d_unlockHandlers;

    void do_handle_syncRequest (void);
    static int handle_syncRequest (void *, vrpn_HANDLERPARAM);
    static int handle_syncComplete (void *, vrpn_HANDLERPARAM);


    // HACK
    vrpn_Connection * d_peer;
};

#endif  // NMUI_COMPONENT_H

