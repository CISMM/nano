#ifndef NMUI_PLANE_SYNC_H
#define NMUI_PLANE_SYNC_H

#include <vrpn_Connection.h>  // for vrpn_HANDLERPARAM
#include <nmb_Dataset.h>  // for flatten_data, etc.
#include <nmb_CalculatedPlane.h>

// NANOX FLAT

/// Class handles synchronization of derived planes between nM replicas.
class nmui_PlaneSync {
public:
  
  nmui_PlaneSync( vrpn_Connection * );
  ~nmui_PlaneSync( void );
  
  /// Start collaborating with someone, and sharing plane creations.
  void changePeer( vrpn_Connection * );
  
  /// When the user is synced to a collaborator, call these.
  /// acceptUpdates will send any flat planes we have created to our peer,
  /// and create any flat planes our peer has sent us.
  void acceptUpdates( void );
  
  /// queueUpdates will set a flag so plane creations are queued again.
  void queueUpdates( void );
  
protected:
  
  static const char vrpnSenderType[];
  static const char vrpnMessageType[];
  
  vrpn_Connection * d_server;
  vrpn_Connection * d_peer;
  
  vrpn_int32 d_senderID;
  vrpn_int32 d_synchCalculatedPlaneMessageType;
  
  /// Are we accepting new updates right now?
  vrpn_bool d_accepting;
  
  static int handleCalculatedPlaneSync( void *, vrpn_HANDLERPARAM );
  
  static NewCalculatedPlaneCallback queueCalculatedPlaneForSync;
  // see nmb_CalculatedPlane for definiton of this (function) type.
  
  // struct and data member for keeping incoming plane messages
  // when we're not accepting
  struct vrpn_HANDLERPARAM_node
  {
    vrpn_HANDLERPARAM* data;
    vrpn_HANDLERPARAM_node* next;
  };
  vrpn_HANDLERPARAM_node* incomingCalcdPlaneList;

  nmb_CalculatedPlaneNode* outgoingCalcdPlaneList;
};

#endif  // NMUI_PLANE_SYNC_H

