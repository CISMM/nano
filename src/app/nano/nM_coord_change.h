/** \class nM_coord_change
This is a server to take the tracker position and orientation coming in from 
the phantom and pass it to a collaborator in a different coordinate system 
so we can see the hand positions of both users.
Extended to support telling the peer when we're collaborating and when we're
not, since the only place we're using that information right now is to control
display of the partner's hand icon.

\author Amy Henderson  7/13/99
\author Tom Hudson 23 Feb 2000

*/

#ifndef NM_COORD_CHANGE_H
#define NM_COORD_CHANGE_H

#include <vrpn_Tracker.h>
#include <vrpn_Connection.h>

typedef void (* PSCHANDLER) (void * userdata, vrpn_bool state);

class nM_coord_change : public vrpn_Tracker {

  public:

    nM_coord_change (const char * name, 
                     vrpn_Tracker_Remote * tracker,
                     vrpn_Synchronized_Connection * serverC = NULL,
                     vrpn_Connection * remoteC = NULL);
    virtual ~nM_coord_change (void);

    vrpn_bool peerIsSynchronized (void) const;

    void mainloop (const struct timeval * timeout = NULL);

    void bindConnection (vrpn_Connection * remoteC);
      /**< Late binding of remote connection (connection to listen
       * on for changeSyncStatus messages).
       */

    void startSync (void);
      /**< Send a message to the peer to let them know we're in sync. */

    void stopSync (void);
      /**< Send a message to the peer to let them know we're out of sync. */

    void registerPeerSyncChangeHandler (PSCHANDLER, void *);

  protected:

    static void handle_tracker_pos_change (void * userdata,
						const vrpn_TRACKERCB info);

    static int handle_changeSyncStatus (void * userdata, vrpn_HANDLERPARAM p);

    vrpn_int32 d_myId;
    vrpn_int32 d_changeSyncStatus_type;

    vrpn_bool d_peerIsSynced;

    struct cbList {
      PSCHANDLER cb;
      void * userdata;
      cbList * next;
    };

    cbList * d_callbacks;
};

#endif
