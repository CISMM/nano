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
    vrpn_bool isReplayingInterface (void) const;


    // MANIPULATORS


    void mainloop (void);

    void setNIC (char * NICaddress);
      ///< Specify either DNS name or IP number of NIC to use.

    void setHandServerName (char * serverName);
    void setModeServerName (char * serverName);

    void setLogging (char * path, int timestamp);
    void enableLogging (vrpn_bool);

    void setPeerPort (int);
      ///< What port is our peer listening on?
    void setServerPort (int);
      ///< What port should we listen on?

    void setTimer (nmb_TimerList *);

    void initialize (vrpn_Tracker_Remote * handTracker,
                     void * syncChangeData,
                     void (* syncChangeCH) (void *, vrpn_bool));
      ///< Open a port to listen on (or the replay file), log the interface,
      ///< and then open handServer and modeServer.

    void setUI (nmui_Component *);
      ///< Must be called before setPeerName(), but need not be called
      ///< before initialize().
    void setPlaneSync (nmui_PlaneSync *);

    void setUserMode (int);

    void setPeerName (const char * newName,
                      void * handChangeData,
                      void (* handChangeCB) (void *, const vrpn_TRACKERCB),
                      void * modeChangeData,
                      void (* modeChangeCB) (void *, const vrpn_ANALOGCB));
      ///< Connect to the named peer, calling the given callbacks in case
      ///< of change of peer's hand position or interface mode.


    vrpn_Connection * peerServerConnection (void);
    vrpn_Connection * interfaceLogConnection (void);
    vrpn_Connection * peerRemoteConnection (void);
    nM_coord_change * handServer (void);
    nmui_Component * uiRoot (void);
    nmui_PlaneSync * planeSync (void);

  protected:

    vrpn_Connection * d_peerServer;
      ///< Synchronizes tclvars - was collaboratingPeerServerConnection.
    vrpn_Connection * d_peerRemote;
      ///< Synchronizes tclvars - was collaboratingPeerRemoteConnection.

    vrpn_Connection * d_interfaceLog;
      ///< Logs tclvars - was interfaceLogConnection.

    vrpn_Tracker_Remote * d_peerHand;
      ///< Receives tracker updates from peer - was vrpnHandTracker_collab[0].
    nM_coord_change * d_handServer;
      ///< Sends tracker updates to peer - was nM_coord_change_server.

    vrpn_Analog_Remote * d_peerMode;
      ///< Receives interface mode updates from peer - was vrpnMode_collab.
    vrpn_Analog_Server * d_modeServer;
      ///< Sends interface mode updates to peer - was vrpnMode_Local.

    nmui_PlaneSync * d_planeSync;


    int d_peerPort;
    int d_serverPort;
    char * d_logPath;
    int d_logTime;
    vrpn_bool d_log;
    char * d_NIC_IP;
    vrpn_bool d_replay;
    int d_userMode;

    char * d_handServerName;
    char * d_modeServerName;

    nmui_Component * d_uiController;

    // for timing collaboration

    nmb_TimerList * d_timer;
    nmb_TimerList d_peerTimer;

    vrpn_int32 d_myId_svr;
    vrpn_int32 d_myId_rem;
    vrpn_int32 d_timerSN_type;
    vrpn_int32 d_timerSNreply_type;

    void sendOurTimer (void);
    void respondToPeerTimer (void);
    static int handle_peerTimer (void *, vrpn_HANDLERPARAM);
    static int handle_timerResponse (void *, vrpn_HANDLERPARAM);

};


#endif  // COLLABORATION_H



