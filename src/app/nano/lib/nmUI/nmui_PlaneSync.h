#ifndef NMUI_PLANE_SYNC_H
#define NMUI_PLANE_SYNC_H

#include <vrpn_Connection.h>  // for vrpn_HANDLERPARAM
#include <nmb_Dataset.h>  // for flatten_data, etc.

// Class handles synchronization of derived planes between nM replicas.

// NANOX FLAT

class nmui_PlaneSync {

  public:

    nmui_PlaneSync (nmb_Dataset *, vrpn_Connection *);
    ~nmui_PlaneSync (void);

    void addPeer (vrpn_Connection *);

  protected:


  private:

    vrpn_Connection * d_server;
    vrpn_Connection * d_peer;

    nmb_Dataset * d_dataset;

    vrpn_int32 d_myId;
    vrpn_int32 d_flatPlaneSync_type;

    static int handle_flatPlaneSync (void *, vrpn_HANDLERPARAM);

    static void queueFlatPlaneForSync (void *, const flatten_data *);
    


};

#endif  // NMUI_PLANE_SYNC_H

