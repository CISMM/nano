#ifndef NMUI_COMPONENT_H
#define NMUI_COMPONENT_H

#include <stdlib.h>  // for NULL

#include <vrpn_Connection.h>  // for vrpn_HANDLERPARAM
#include <vrpn_Types.h>  // for vrpn_bool

#define NMUI_COMPONENT_MAX_SIZE 100
  // No more than 100 linkvars of each type in the component.

// name is limited to 30 characters so that we can fit in a VRPN
// sender ID

class TclNet_int;  // from <Tcl_Netvar.h>
class TclNet_float;
class TclNet_selector;

class vrpn_Connection;  // from <vrpn_Connection.h>

// We could factor out the common interface from this and from
// TclNet_foo (bindConnection, addPeer, copyReplica, syncReplica)
// and use it as a base class;  that might let us manage a hierarchy
// easier.

typedef int (* nmui_SyncRequestHandler) (void * userdata);
typedef int (* nmui_SyncCompleteHandler) (void * userdata);
typedef int (* nmui_LockHandler) (void * userdata);

// BUGS
//   Some of this code was written when we intended to have one
// shared state per user, rather than one private state per user
// and some additional number of shared states.  There are some
// deep misassumptions buried in the code around addPeer() that
// probably have roots down into Tcl_Netvar.

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
      // Returns the number of peers we're sharing state with.

    int synchronizedTo (void) const;
      // Returns 0 if not synchronized to anybody else,
      // nonzero if synchronized to a shared state copy.
      // Current implementation only has 1 shared state, so
      // *should* only return 0 or 1, but may not if addPeer()
      // is called multiple times (by the user typing in multiple
      // values for collab_machine_name in the Tcl interface).

    vrpn_bool isLockedRemotely (void) const;
    vrpn_bool holdsRemoteLocks (void) const;
      // Used for locking, which is not fully implemented.

    // MANIPULATORS

    void add (TclNet_int *);
    void add (TclNet_float *);
    void add (TclNet_selector *);
    void add (nmui_Component *);
      // Adds a netvar or component to the list of objects
      // maintained by this component.  Everything in this
      // list will be copied or synchronized when copyReplica()
      // or syncReplica() is called.
      // HACK:  these are fixed-size arrays.

    void bindConnection (vrpn_Connection *);
      // Specifies the vrpn_Connection which is our SERVER
      // to be connected to by our peer.
    void bindLogConnection (vrpn_Connection *);
      // Specifies a vrpn_Connection to log private events to.
    void addPeer (vrpn_Connection *, vrpn_bool serialize);
      // Specifies the vrpn_Connection which is our REMOTE
      // which is connected to our peer's server.

    void copyReplica (int whichReplica);
      // Copies the state of <whichReplica> over the current state.
    void syncReplica (int whichReplica);
      // Makes <whichReplica> be the current state.

    void requestSync (void);
      // Request that a peer send us all the information we need
      // to correctly sync or copy replicas (see below).
    void registerSyncRequestHandler (nmui_SyncRequestHandler, void *);
    void registerSyncCompleteHandler (nmui_SyncCompleteHandler, void *);
      // Must not be called before bindConnection()

      // Most processes write any changes into their TclNet objects,
      // so a remote Component that wants to sync will have a copy of
      // the current state in its replicas in those objects.  However,
      // some of the "simplest" processes that we want to synchronize
      // are not entirely user-controlled but instead change as a function
      // of time.  So, when a user requests synchronization copying the
      // current value of the appropriate replica is the *wrong* thing
      // to do.  Instead we send a syncRequest message, and a
      // syncRequestHandler on the remote machine can trigger updates
      // of the relevant TclNet objects to their proper current values.

    // Machine A calls requestSync()
    // Machine B's SyncRequestHandlers are called()
    // all updates from the SyncRequestHandlers arrive on machine A
    // Machine A's SyncCompleteHandlers are called()

    vrpn_bool d_maintain;
      // Controls behavior of handle_syncComplete().
      // If 0, assumes the syncComplete message recieved is
      // the completion of a copyReplica() call.  If 1,
      // assume it's the completion of a syncReplica() call.
      // Probably should be rewritten to have two different
      // syncRequest/syncComplete messages.
      // This bool *could* get corrupted by interleaving
      // presses of sync and copy, but that might not hurt anything.

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

    int d_synchronizedTo;
      // state for d_synchronizedTo

    int d_numPeers;

    vrpn_bool d_isLockedRemotely;
    vrpn_bool d_holdsRemoteLocks;

    vrpn_Connection * d_connection;
    vrpn_int32 d_myId;
    vrpn_int32 d_syncRequest_type;
    vrpn_int32 d_syncComplete_type;
    vrpn_int32 d_lock_type;
    vrpn_int32 d_unlock_type;

    // callback structures
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
      // do_handle_syncRequest() is the part of handle_syncRequest()
      // that actually needs to recurse down the tree, calling handlers
      // that may be registered at subnodes.
      // HACK:  there should probably be an equivalent function for
      // handle_syncComplete().
    static int handle_syncRequest (void *, vrpn_HANDLERPARAM);
    static int handle_syncComplete (void *, vrpn_HANDLERPARAM);


    // HACK
    vrpn_Connection * d_peer;
};

#endif  // NMUI_COMPONENT_H

