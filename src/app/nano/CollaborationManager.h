#ifndef COLLABORATION_H
#define COLLABORATION_H

#include <vrpn_Types.h>  // for vrpn_bool
#include <vrpn_Tracker.h>  // for vrpn_TRACKERCB, vrpn_Tracker_Remote
#include <vrpn_Analog.h>  // for vrpn_ANALOGCB, vrpn_Analog_{Remote,Server}

class vrpn_Connection;  // from <vrpn_Connection.h>

#include <nmb_TimerList.h>  // for nmb_TimerList

class nmui_Component;  // from <nmui_Component.h>
class nmui_PlaneSync;  // from <nmui_PlaneSync.h>

class nM_coord_change;  // from "nM_coord_change.h"


/// Class CollaborationManager encapsulates the state and actions required
/// to manage collaboration with ONE peer;  keeping the UI synchronized
/// in the steady-state is handled at a lower level by Tcl_Netvar and
/// nmui_Component.


class CollaborationManager {

  public:

    CollaborationManager (vrpn_bool replay);
    ~CollaborationManager (void);


    // ACCESSORS


    vrpn_bool isLoggingInterface (void) const;
      ///< True if all UI I/O is being logged.

    vrpn_bool isReplayingInterface (void) const;
      ///< True if all UI I/O is being replayed from a logfile.

    vrpn_bool isCollaborationOn (void) const;
      ///< True if collaborative session is going on.


    // MANIPULATORS


    // STEADY-STATE MANIPULATORS


    void mainloop (void);

    void setUserMode (int);
      ///< Specifies user mode to be passed by ModeServer;  this will
      ///< be obsolete soon.


    // INITIALIZATION MANIPULATORS


    void setNIC (char * NICaddress);
      ///< Specify either DNS name or IP number of NIC to use for
      ///< all collaboration traffic at this host.


    void setHandServerName (char * serverName);
      ///< Name of the VRPN servers that will handle the coordinates
      ///< of PHANTOMs.

    void setModeServerName (char * serverName);
      ///< Name of the VRPN srevers that will handle the current
      ///< user mode;  this will be obsolete soon, when the
      ///< special-purpose user_0_mode Tcl variable is replaced
      ///< by a TclNet_int for user_mode.


    void setLogging (char * path, int timestamp);
      ///< Specifies the directory in which to write the logs and
      ///< the timestamp with which to label them;  several logs
      ///< will be created for the different I/O flows (incoming
      ///< collaborative, outgoing collaborative, private interface).

    void enableLogging (vrpn_bool);
      ///< Turns logging on or off.


    void setPeerPort (int);
      ///< What port is our peer listening on?

    void setServerPort (int);
      ///< What port should we listen on?


    void setTimer (nmb_TimerList *);
      ///< Specifies the object to use to time performance of
      ///< collaborative code (specifically serialization/consistiency
      ///< control), or NULL if no timing is to be done.


    void setPlaneSync (nmui_PlaneSync *);
      ///< Specifies the object to be used to make sure that creation
      ///< of new derived planes is properly synchronized between
      ///< collaborating peers.


    void initialize (vrpn_Tracker_Remote * handTracker,
                     void * syncChangeData,
                     void (* syncChangeCH) (void *, vrpn_bool));
      ///< Open a port to listen on (or the replay file), log the interface,
      ///< and then open handServer and modeServer.


    void setUI (nmui_Component *);
      ///< Specifies the root node of the user interface; must be called
      ///< before setPeerName(), but need not be called before initialize().

    void setPeerName (const char * newName,
                      void * handChangeData,
                      void (* handChangeCB) (void *, const vrpn_TRACKERCB),
                      void * modeChangeData,
                      void (* modeChangeCB) (void *, const vrpn_ANALOGCB));
      ///< Connect to the named peer, calling the given callbacks in case
      ///< of change of peer's hand position or user mode; naming
      ///< must be by DNS name to guarantee correct collaboration.


    // NON-CONST ACCESSORS


    vrpn_Connection * peerServerConnection (void);
      ///< Connection on which we transmit state changes (?).
    vrpn_Connection * peerRemoteConnection (void);
      ///< Connection on which we receive state changes (?).

    vrpn_Connection * interfaceLogConnection (void);
      ///< Connection on which we log the private user interface.

    nmui_Component * uiRoot (void);
      ///< Returns the node passed in by setUI().

    nM_coord_change * handServer (void);
      ///< Exposes our vrpn_Tracker subclass that sends our PHANTOM position
      ///< to collaborator AND lets them know when we're collaborating and
      ///< when we aren't.

    nmui_PlaneSync * planeSync (void);
      ///< Exposes the object that makes sure creation of new derived planes
      ///< is properly synchronized between collaborators.


  protected:


    vrpn_Connection * d_peerServer;
      ///< Synchronizes tclvars.
    vrpn_Connection * d_peerRemote;
      ///< Synchronizes tclvars.

    vrpn_Connection * d_interfaceLog;
      ///< Logs tclvars.

    vrpn_Tracker_Remote * d_peerHand;
      ///< Receives tracker updates from peer.
    nM_coord_change * d_handServer;
      ///< Sends tracker updates to peer.

    vrpn_Analog_Remote * d_peerMode;
      ///< Receives interface mode updates from peer.
    vrpn_Analog_Server * d_modeServer;
      ///< Sends interface mode updates to peer.

    nmui_PlaneSync * d_planeSync;
      ///< Creates derived planes when necessary.


    int d_peerPort;
    int d_serverPort;
    char * d_logPath;
    int d_logTime;
    vrpn_bool d_log;
    char * d_NIC_IP;
    vrpn_bool d_replay;
    int d_userMode;
    char * d_peerName;

    vrpn_bool d_isCollaborationOn;
       ///< Flag to indicate whether a collaborative session is on or not.

    char * d_handServerName;
      ///< VRPN name of d_peerHand and d_handServer.
    char * d_modeServerName;
      ///< VRPN name of d_peerMode and d_modeServer.

    nmui_Component * d_uiController;


    // Support for timing collaboration - Tom Hudson, 2000

    // Timing algorithm:
    //   When we've done something that has to wait until we hear
    // a response from our peer, we block the current frame of d_timer
    // and send a timing request to our peer.  After the peer has responded
    // to the event that triggered our request, it responds to the timing
    // request.  On receiving that, we unblock the blocked frame of d_timer,
    // which is then marked as complete and added to the log and statistics.

    nmb_TimerList * d_timer;
    nmb_TimerList d_peerTimer;

    vrpn_int32 d_myId_svr;
    vrpn_int32 d_myId_rem;
    vrpn_int32 d_timerSN_type;
    vrpn_int32 d_timerSNreply_type;

    void sendOurTimer (void);
      ///< Send a timing request to our peer (blocking the current
      ///< frame of d_timer is done externally, where some complex
      ///< logic determines whether or not it needs to be blocked;
      ///< if it hasn't been blocked externally this function doesn't
      ///< send the timing request).

    void respondToPeerTimer (void);
      ///< A timing request from our peer has come out of queue;
      ///< send a response to it.

    static int handle_peerTimer (void *, vrpn_HANDLERPARAM);
      ///< Our peer has sent us a timing request;  move that into the
      ///< local queue.

    static int handle_timerResponse (void *, vrpn_HANDLERPARAM);
      ///< React to the response from our peer by unblocking the
      ///< specified frame of d_timer.

    // Tom Hudson, 2001

    // Tell the user when the collaborative connection is set up;
    // let them copy & change to shared state then.

    vrpn_bool d_gotPeerServer;
    vrpn_bool d_gotPeerRemote;
    static int handle_gotPeerServerConnection (void *, vrpn_HANDLERPARAM);
    static int handle_gotPeerRemoteConnection (void *, vrpn_HANDLERPARAM);

    int fullyConnected (void);

};


#endif  // COLLABORATION_H



