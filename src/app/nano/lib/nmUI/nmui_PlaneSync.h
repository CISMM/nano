#ifndef NMUI_PLANE_SYNC_H
#define NMUI_PLANE_SYNC_H

#include <vrpn_Connection.h>  // for vrpn_HANDLERPARAM
#include <nmb_Dataset.h>  // for flatten_data, etc.
#include <AFMState.h>  // for list of input planes.

// NANOX FLAT

/// Class handles synchronization of derived planes between nM replicas.
class nmui_PlaneSync {

  public:

    nmui_PlaneSync (nmb_Dataset *, AFMDataset *, vrpn_Connection *);
    ~nmui_PlaneSync (void);

    /// Start collaborating with someone, and sharing plane creations.
    void addPeer (vrpn_Connection *, const char * new_sync_hostname = NULL);

    /// When the user is synced to a collaborator, call these.
    /// acceptUpdates will send any flat planes we have created to our peer,
    /// and create any flat planes our peer has sent us.
    void acceptUpdates (void);
    /// queueUpdates will set a flag so plane creations are queued again.
    void queueUpdates (void);
  protected:


  private:

    vrpn_Connection * d_server;
    vrpn_Connection * d_peer;

    vrpn_int32 d_myId;
    vrpn_int32 d_flatPlaneSync_type;

    nmb_Dataset * d_dataset;

    AFMDataset * d_afmdata;

    /// if we get planes from a collaborator, we identify them
    /// by appending this hostname.
    char * d_sync_hostname;

    /// Are we accepting new updates right now?
    vrpn_bool d_accepting;
    
    /// If we aren't accepting, we need to queue up flat plane 
    /// creations, both ours to tell the collaborator, and incoming
    /// flat planes from our collaborator.
    flatten_node * d_outgoing_flat_list;
    flatten_node * d_incoming_flat_list;

    static int handle_flatPlaneSync (void *, vrpn_HANDLERPARAM);

    static void queueFlatPlaneForSync (void *, const flatten_data *);
    
    int sendFlatMessage(const flatten_data *);
};

#endif  // NMUI_PLANE_SYNC_H

