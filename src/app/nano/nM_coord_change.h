/*
Amy Henderson  7/13/99
This is a server to take the tracker position and orientation coming in from 
the phantom and pass it to a collaborator in a different coordinate system 
so we can see the hand positions of both users.
*/
#ifndef NM_COORD_CHANGE_H
#define NM_COORD_CHANGE_H
#include <vrpn_Tracker.h>

class nM_coord_change : public vrpn_Tracker {
	public:
		nM_coord_change(const char *name, 
				vrpn_Tracker_Remote *tracker,
				vrpn_Synchronized_Connection * c = NULL);
		void mainloop(const struct timeval *timeout=NULL);
		~nM_coord_change();

	protected:
		static void handle_tracker_pos_change(void *userdata,
						const vrpn_TRACKERCB info);
};

#endif
