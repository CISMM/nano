#ifndef NMUI_COMPONENT_H
#define NMUI_COMPONENT_H

#include <stdlib.h>  // for NULL

#include <vrpn_Connection.h>  // for vrpn_HANDLERPARAM
#include <vrpn_Types.h>  // for vrpn_bool

#define NMUI_COMPONENT_MAX_SIZE 100
  ///< No more than 100 linkvars of each type in the component.

// name is limited to 30 characters so that we can fit in a VRPN
// sender ID

class Tcl_Netvar;
//class TclNet_int;  // from <Tcl_Netvar.h>
//class TclNet_float;
//class TclNet_string;

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

/**
 * Synchronization hierarchy of Tcl_Netvars and support functions.
 * nmui_Component builds a hierarchy of Tcl_Netvars that can be
 * synchronized en masse or at finer granularity.  Its syncRequest()
 * and registerSync*Handler() functions provide support for updating
 * variables with semantics not correctly synchronized by standard
 * Tcl_Netvar behavior (example:  clocks).  Has some support for
 * vrpn's deferred connection and automatic reconnection, although
 * this is messy (because it depends on Netvar's addPeer(), which
 * is quite messy).
 *
 * Most processes write any changes into their TclNet objects,
 * so a remote Component that wants to sync will have a copy of
 * the current state in its replicas in those objects.  However,
 * some of the "simplest" processes that we want to synchronize
 * are not entirely user-controlled but instead change as a function
 * of time.  So, when a user requests synchronization copying the
 * current value of the appropriate replica is the *wrong* thing
 * to do.  Instead we send a syncRequest message, and a
 * syncRequestHandler on the remote machine can trigger updates
 * of the relevant TclNet objects to their proper current values.
 *
 *   Machine A calls requestSync()
 *   Machine B's SyncRequestHandlers are called()
 *   all updates from the SyncRequestHandlers arrive on machine A
 *   Machine A's SyncCompleteHandlers are called()
 */

class nmui_Component {


  public:


    // CONSTRUCTORS


    nmui_Component (char name [30]);

    virtual ~nmui_Component (void);


    // ACCESSORS


    const char * name (void) const;

    int numPeers (void) const;
      /**< Returns the number of peers we're sharing state with. */

    int synchronizedTo (void) const;
      /**<
       * Returns 0 if not synchronized to anybody else,
       * nonzero if synchronized to a shared state copy.
       * Current implementation only has 1 shared state, so
       * *should* only return 0 or 1, but may not if addPeer()
       * is called multiple times (by the user typing in multiple
       * values for collab_machine_name in the Tcl interface), or
       * the peer going down and coming back up multiple times.
       */

    vrpn_bool isLockedRemotely (void) const;
      /**<
       * Returns VRPN_TRUE if the hierarchy of Netvars rooted at
       * this component is locked by some other process.
       * Used for locking, which is not fully implemented.
       */

    vrpn_bool holdsRemoteLocks (void) const;
      /**<
       * Returns VRPN_TRUE if the hierarchy of Netvars rooted at
       * this component is locked by this process so that other
       * processes can't write into any of the Netvars involved.
       * Used for locking, which is not fully implemented.
       */


    // MANIPULATORS


    void add (Tcl_Netvar *);
      /**< Adds a netvar to the hierarchy under this component. */

    void add (nmui_Component *);
      /**< Adds a component to the hierarchy under this component.  */

    void remove(Tcl_Netvar *);
      /**< Removes a netvar from the heirarchy under this component.  */

    nmui_Component * find(char *);
      /**< Finds a component by the name specified under this component. */

    void bindConnection (vrpn_Connection *);
      /**<
       * Specifies the vrpn_Connection which is our SERVER
       * to be connected to by our peer.
       */

    void bindLogConnection (vrpn_Connection *);
      /**<
       * Specifies a vrpn_Connection to log private events to.
       */

    void addPeer (vrpn_Connection *, vrpn_bool serialize);
      /**<
       * Specifies the vrpn_Connection which is our REMOTE,
       * connected to our peer's server.
       * Messy interface to a messy function (Tcl_Netvar::addPeer());
       * this could be profitably redesigned.
       */

    void copyReplica (int whichReplica);
      /**<
       * Copies the state of <whichReplica> over the current state
       * of all netvars below this one in the hierarchy.
       */

    void copyFromToReplica (int sourceReplica, int destReplica);
      /**< 
       * Copy the state of the source replica to the destination replica.
       */

    void syncReplica (int whichReplica);
      /**<
       * Makes all netvars below this one in the hierarchy use
       * <whichReplica> as their current state.
       */

    void requestSync (void);
      /**<
       * Request that a peer send us all the information we need
       * to correctly sync or copy replicas (see below).
       */

    void registerSyncRequestHandler (nmui_SyncRequestHandler, void *);
    void registerSyncCompleteHandler (nmui_SyncCompleteHandler, void *);

    vrpn_bool d_maintain;
      /**<
	 Controls behavior of handle_syncComplete().
	 If 0, assumes the syncComplete message recieved is
	 the completion of a copyReplica() call.  If 1,
	 assume it's the completion of a syncReplica() call.
	 Probably should be rewritten to have two different
	 syncRequest/syncComplete messages.
	 This bool *could* get corrupted by interleaving
	 presses of sync and copy, but that might not hurt anything.
      */

    static int handle_reconnect (void *, vrpn_HANDLERPARAM);
      /**<
       * Re-registers type and sender ids.
       * You will probably want to have this as a callback for
       * vrpn_got_connection on connections you addPeer().
       * HACK:  assumes there's only been one peer added that
       * could be outstanding.
       */

    void initializeConnection (vrpn_Connection *);
      /**<
       * (OBSOLETE) Function that must be called on a connection on which
       * we should recieve and respond to synchronization requests.
       */


  protected:


  private:


    char d_name [31];

    int d_numVars;
    Tcl_Netvar * d_vars [3 * NMUI_COMPONENT_MAX_SIZE];
    int d_numComponents;
    nmui_Component * d_components [NMUI_COMPONENT_MAX_SIZE];

    int d_synchronizedTo;
      ///< state for d_synchronizedTo

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
    /**< do_handle_syncRequest() is the part of handle_syncRequest()
      that actually needs to recurse down the tree, calling handlers
      that may be registered at subnodes.
      HACK:  there should probably be an equivalent function for
      handle_syncComplete(). */
    static int handle_syncRequest (void *, vrpn_HANDLERPARAM);
    static int handle_syncComplete (void *, vrpn_HANDLERPARAM);


    // HACK
    vrpn_Connection * d_peer;
};

#endif  // NMUI_COMPONENT_H

