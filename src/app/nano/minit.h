#ifndef MINIT_H
#define MINIT_H


int x_init (char * argv []);
int reset_phantom (void);
int peripheral_init (vrpn_Connection *, vrpn_Connection *,
                     const char * handTrackerName,
                     const char * headTrackerName, const char * bdboxName,
                     char * magellanName);
  ///< Calls phantom_init(), sets up head tracker, button box, magellan.
  ///< Takes lots of parameters so we don't have to share globals from
  ///< microscape.c, cutting down on (evil!) use of microscape.h
  ///< which causes circular depenencies which are hard to understand and
  ///< maintain.

int phantom_init (vrpn_Connection *, vrpn_Connection *,
                  const char * handTrackerName);
  ///< Sets up phantom.  Exposed so that the phantom connection can be
  ///< torn down and reestablished if we're replaying user input.
  ///< First connection is for Tracker and Button (mostly data sources),
  ///< Second is for ForceDevice (mostly data sink).
int teardown_phantom
             (vrpn_MousePhantom ** mousePhantomServer,
              vrpn_ForceDevice_Remote ** forceDevice,
              vrpn_Button_Remote ** phantButton,
              vrpn_Tracker_Remote ** vrpnHandTracker);
  ///< Unregisters callbacks, deletes objects, sets pointers to NULL.


#endif  // MINIT_H


