/** \file nM_coord_change.C
	last modified by Tom Hudson on 23 Feb 2000.
	This is the server that takes the position/orientation messages from
	the phantom, transforms these into world-space coordinates, and then
	sends this data on another connection so that it can be read by a
	vrpn_Tracker_Remote in the other copy of microscape.  Adding this
        extra function means we need to create this object whether or not
        there is a local tracker, which means accepting NULL trackers.
*/ 

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <v.h>
#include "nM_coord_change.h"

nM_coord_change::nM_coord_change (const char * name,
				  vrpn_Tracker_Remote * tracker,
				  vrpn_Synchronized_Connection * serverC,
                                  vrpn_Connection * remoteC) :
  vrpn_Tracker (name, serverC),
  d_peerIsSynced (VRPN_FALSE),
  d_callbacks (NULL)
{
  if (tracker) {
    tracker->register_change_handler(this,
				     handle_tracker_pos_change);
  }

  gettimeofday(&timestamp, NULL);

  bindConnection(remoteC);

  d_myId = serverC->register_sender("nM coord change");
  d_changeSyncStatus_type = serverC->register_message_type
             ("change sync status");
}

nM_coord_change::~nM_coord_change (void) {
  // connection doesn't belong to us;  why are we writing into it in
  // the destructor?
    //connection = NULL;
}

vrpn_bool nM_coord_change::peerIsSynchronized (void) const {
  return d_peerIsSynced;
}

void nM_coord_change::mainloop(const struct timeval *curr_time)
{
  if (connection)
    connection->mainloop(curr_time);
}

void nM_coord_change::bindConnection (vrpn_Connection * remoteC) {
  vrpn_int32 type;
  vrpn_int32 id;

  if (!remoteC) {
    return;
  }

  type = remoteC->register_message_type("change sync status");
  id = remoteC->register_sender("nM coord change");
  remoteC->register_handler(type, handle_changeSyncStatus, this, id);

}

void nM_coord_change::startSync (void) {
  timeval now;
  vrpn_Connection * c;
  char buf [50];
  char * bptr;
  vrpn_int32 mlen;

  mlen = 50;
  bptr = buf;
  vrpn_buffer(&bptr, &mlen, (vrpn_int32) VRPN_TRUE);

  c = connectionPtr();  // get the Tracker's copy of server connection
  gettimeofday(&now, NULL);
  c->pack_message(50 - mlen, now, d_changeSyncStatus_type, d_myId,
                  buf, vrpn_CONNECTION_RELIABLE);
}

void nM_coord_change::stopSync (void) {
  timeval now;
  vrpn_Connection * c;
  char buf [50];
  char * bptr;
  vrpn_int32 mlen;

  mlen = 50;
  bptr = buf;
  vrpn_buffer(&bptr, &mlen, (vrpn_int32) VRPN_FALSE);

  c = connectionPtr();  // get the Tracker's copy of server connection
  gettimeofday(&now, NULL);
  c->pack_message(50 - mlen, now, d_changeSyncStatus_type, d_myId,
                  buf, vrpn_CONNECTION_RELIABLE);

}

void nM_coord_change::registerPeerSyncChangeHandler (PSCHANDLER p, void * ud) {
  cbList * li;

  li = new cbList;
  if (!li) {
    fprintf(stderr, "nM_coord_change::registerPeerSyncChangeHandler:  "
                    "Out of memory.\n");
    return;
  }

  li->cb = p;
  li->userdata = ud;
  li->next = d_callbacks;

  d_callbacks = li;
}

//static
void nM_coord_change::handle_tracker_pos_change(void *userdata,
						const vrpn_TRACKERCB)
{

  vrpn_int32 len;
  v_xform_type worldFromHandPtr;
  nM_coord_change *me = (nM_coord_change *)userdata;
  vrpn_float64	floatbuf[128];	// Aligns properly
  char * msgbuf = (char*)(void*)(floatbuf);

  v_get_world_from_hand(0, &worldFromHandPtr);

  me->pos[0] = worldFromHandPtr.xlate[Q_X];
  me->pos[1] = worldFromHandPtr.xlate[Q_Y];
  me->pos[2] = worldFromHandPtr.xlate[Q_Z];
  me->quat[0] = worldFromHandPtr.rotate[Q_X];
  me->quat[1] = worldFromHandPtr.rotate[Q_Y];
  me->quat[2] = worldFromHandPtr.rotate[Q_Z];
  me->quat[3] = worldFromHandPtr.rotate[Q_W];

  gettimeofday(&me->timestamp, NULL);

  if (me->connection) {
    len = me->encode_to(msgbuf);
    if (me->connection->pack_message(len, me->timestamp, me->position_m_id,
				     me->my_id, msgbuf, 
				     vrpn_CONNECTION_LOW_LATENCY)) {
      fprintf(stderr, "nM_coord_change: cannot write message: tossing\n");
    }
  }
}

// static
int nM_coord_change::handle_changeSyncStatus (void * userdata,
                                               vrpn_HANDLERPARAM p) {
  nM_coord_change * it = (nM_coord_change *) userdata;
  const char * bptr;
  cbList * li;
  vrpn_int32 newValue;

  bptr = p.buffer;
  vrpn_unbuffer(&bptr, &newValue);

  it->d_peerIsSynced = newValue;

  for (li = it->d_callbacks; li; li = li->next) {
    (*li->cb)(li->userdata, newValue);
  }

  return 0;
}
