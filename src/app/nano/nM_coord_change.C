/* nM_coord_change.C
	last modified by Amy Henderson on 8/6/99
	This is the server that takes the position/orientation messages from
	the phantom, transforms these into world-space coordinates, and then
	sends this data on another connection so that it can be read by a
	vrpn_Tracker_Remote in the other copy of microscape.
*/ 

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <v.h>
#include "nM_coord_change.h"

nM_coord_change::nM_coord_change(const char *name,
				 vrpn_Tracker_Remote *tracker,
				 vrpn_Synchronized_Connection *c) :
  vrpn_Tracker(name, c)
{
  tracker->register_change_handler(this,
				   handle_tracker_pos_change);
  gettimeofday(&timestamp, NULL);
}

void nM_coord_change::mainloop(const struct timeval *curr_time)
{
  if (connection)
    connection->mainloop(curr_time);
}

nM_coord_change::~nM_coord_change()
{
    connection = NULL;
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

