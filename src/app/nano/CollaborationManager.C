/*===3rdtech===
  Copyright (c) 2000 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#include "CollaborationManager.h"

#if !defined(_WIN32) || defined(__CYGWIN__)
#include <unistd.h>  // for gethostname()
#endif

#include <vrpn_Connection.h>
#include <vrpn_Tracker.h>
#include <vrpn_Analog.h>

#include <nmb_Debug.h>

#include <nmui_Component.h>
#include <nmui_PlaneSync.h>

#include "nM_coord_change.h"

// Verbosity scheme
// 1 - initialization & major changes
// 10 - timing


static
vrpn_Connection * getPeer (const char * hostname, int port,
                           const char * loggingPath, int timestamp,
                           vrpn_bool loggingInterface,
                           const char * NIC_IP) {
  char buf [256];
  char sfbuf [1024];

  sprintf(buf, "%s:%d", hostname, port);
  sprintf(sfbuf, "%s/SharedIFRemLog-%ld.stream", loggingPath, timestamp);

  collabVerbose(1, "Collaboration Manager:  "
                "Connecting to peer %s on NIC %s;  any logging is to %s.\n",
                buf, NIC_IP, sfbuf);

  return vrpn_get_connection_by_name (buf,
             loggingInterface ? sfbuf : NULL,
             loggingInterface ? vrpn_LOG_INCOMING | vrpn_LOG_OUTGOING :
                                vrpn_LOG_NONE,
             NULL, vrpn_LOG_NONE, 1.0, 3, NIC_IP);
}

static
vrpn_Connection * getPeerReplay (const char * loggingPath, int timestamp) {
  char sfbuf [1024];

  sprintf(sfbuf, "file:%s/SharedIFRemLog-%ld.stream", loggingPath,
          timestamp); 

  collabVerbose(1, "Collaboration Manager:  "
                "Replaying peer from file %s.\n", sfbuf);

  return vrpn_get_connection_by_name (sfbuf);
}

static
vrpn_Connection * getServer (int port,
                             const char * loggingPath, int timestamp,
                             vrpn_bool loggingInterface,
                             const char * NIC_IP) {
  char sfbuf [1024];

  sprintf(sfbuf, "%s/SharedIFSvrLog-%ld.stream", loggingPath, timestamp); 

  collabVerbose(1, "Collaboration Manager:  "
                "Opening peer server on port %d, NIC %s.\n",
                port, NIC_IP);

  return new vrpn_Synchronized_Connection
        (port,
         loggingInterface ? sfbuf : NULL,
         loggingInterface ? vrpn_LOG_INCOMING | vrpn_LOG_OUTGOING :
                            vrpn_LOG_NONE,
         NIC_IP);
}


static
vrpn_Connection * getServerReplay (const char * loggingPath, int timestamp) {
  char sfbuf [1024];

  sprintf(sfbuf, "file:%s/SharedIFSvrLog-%ld.stream", loggingPath, timestamp);
  return vrpn_get_connection_by_name (sfbuf);
}


void setString (char ** dest, char * newValue) {
  if (*dest) {
    delete [] *dest;
    *dest = NULL;
  }
  if (!newValue) {
    return;
  }

  *dest = new char [1 + strlen(newValue)];
  if (!*dest) {
    fprintf(stderr, "CollaborationManager::setString:  Out of memory!\n");
    return;
  }
  strcpy(*dest, newValue);
}


CollaborationManager::CollaborationManager (vrpn_bool replay) :
    d_peerServer (NULL),
    d_peerRemote (NULL),
    d_interfaceLog (NULL),
    d_peerHand (NULL),
    d_handServer (NULL),
    d_peerMode (NULL),
    d_modeServer (NULL),
    d_planeSync (NULL),
    d_peerPort (0),
    d_serverPort (0),
    d_logPath (NULL),
    d_logTime (0),
    d_log (VRPN_FALSE),
    d_NIC_IP (NULL),
    d_replay (replay),
    d_userMode (1),
    d_handServerName (NULL),
    d_modeServerName (NULL),
    d_uiController (NULL),
    d_timer (NULL),
    d_myId_svr (-1),
    d_myId_rem (-1),
    d_timerSN_type (-1),
    d_timerSNreply_type (-1)
{

}



CollaborationManager::~CollaborationManager (void) {


  if (d_peerHand)
    delete d_peerHand;
  if (d_handServer)
    delete d_handServer;
  if (d_peerMode)
    delete d_peerMode;
  if (d_modeServer)
    delete d_modeServer;
  if (d_planeSync)
    delete d_planeSync;

  // Need to delete connections AFTER we delete the devices
  // that have callbacks registered on them.

  if (d_peerServer)
    delete d_peerServer;
  if (d_peerRemote)
    delete d_peerRemote;
  if (d_interfaceLog)
    delete d_interfaceLog;

  if (d_logPath)
    delete [] d_logPath;
  if (d_NIC_IP)
    delete [] d_NIC_IP;
  if (d_handServerName)
    delete [] d_handServerName;
  if (d_modeServerName)
    delete [] d_modeServerName;

}




vrpn_bool CollaborationManager::isLoggingInterface (void) const {
  return d_log;
}

vrpn_bool CollaborationManager::isReplayingInterface (void) const {
  return d_replay;
}




void CollaborationManager::mainloop (void) {

  static unsigned long last = 0L;

  struct timeval now;

  gettimeofday(&now, NULL);

  // Tell them where our Phantom is.
  if (d_handServer) {
    d_handServer->mainloop();
  }

  // Find out where their Phantom is.
  if (d_peerHand) {
    d_peerHand->mainloop();
  }

  // Tell them what our mode is.
  if (d_modeServer) {

    d_modeServer->channels()[0] = d_userMode;
    d_modeServer->report_changes(vrpn_CONNECTION_RELIABLE);

    // Send an update every couple of seconds, even if it hasn't changed -
    // soft state.
    if (now.tv_sec - last >= 2) {
      last = now.tv_sec;
      d_modeServer->report();
    }
    d_modeServer->mainloop();
  }

  // Find out what their mode is.
  if (d_peerMode) {
    d_peerMode->mainloop();
  }

  sendOurTimer();

  // Tell them the state of our TCL controls.
  if (d_peerServer) {
    d_peerServer->mainloop();
  }

  // Log the state of our TCL controls.
  if (d_interfaceLog) {
    d_interfaceLog->mainloop();
  }

  // Find out the state of their TCL controls.
  if (d_peerRemote) {
    d_peerRemote->mainloop();
  }

  respondToPeerTimer();
}



void CollaborationManager::setNIC (char * NIC_IP) {
  setString(&d_NIC_IP, NIC_IP);
}

void CollaborationManager::setHandServerName (char * serverName) {
  setString(&d_handServerName, serverName);
}

void CollaborationManager::setModeServerName (char * serverName) {
  setString(&d_modeServerName, serverName);
}

void CollaborationManager::setPeerPort (int port) {
  d_peerPort = port;
}

void CollaborationManager::setServerPort (int port) {
  d_serverPort = port;
}

void CollaborationManager::setLogging (char * path, int timestamp) {
  setString(&d_logPath, path);
  collabVerbose(1, "Collaboration Manager:  "
                "Logging to path \"%s\".\n", d_logPath);
  d_logTime = timestamp;
}

void CollaborationManager::enableLogging (vrpn_bool on) {
  d_log = on;
}

void CollaborationManager::setTimer (nmb_TimerList * timer) {
  d_timer = timer;
}


void CollaborationManager::initialize
     (vrpn_Tracker_Remote * handTracker,
      void * syncChangeData,
      void (* syncChangeCB) (void *, vrpn_bool)) {
  char sfbuf [1024];
  timeval now;
  vrpn_int32 timerSN;

  if (d_replay) {

    sprintf(sfbuf, "file:%s/PrivateIFLog-%ld.stream", d_logPath, d_logTime);

    d_peerServer = getServerReplay (d_logPath, d_logTime);

  } else {

    gettimeofday(&now, NULL);
    d_logTime = now.tv_sec;

    sprintf(sfbuf, "%s/PrivateIFLog-%ld.stream", d_logPath, d_logTime);

    d_peerServer = getServer (d_serverPort, d_logPath, d_logTime, d_log,
                              d_NIC_IP);

  }

  if (d_log || d_replay) {
    d_interfaceLog = vrpn_get_connection_by_name (sfbuf);
  }

  if (!d_peerServer) {
    fprintf(stderr, "CollaborationManager::initialize:  "
                    "Couldn't create server for collaboration.\n");
    return;
  }

  d_handServer = new nM_coord_change (d_handServerName, handTracker,
                       (vrpn_Synchronized_Connection *) d_peerServer);
  d_handServer->registerPeerSyncChangeHandler(syncChangeCB, syncChangeData);
  if (!d_handServer) {
    fprintf(stderr, "CollaborationManager::initialize:  "
                    "Couldn't create hand server for collaboration.\n");
    return;
  }
  collabVerbose(2, "Created handServer named %s.\n", d_handServerName);

  d_modeServer = new vrpn_Analog_Server (d_modeServerName, d_peerServer);
  d_modeServer->setNumChannels(1);
  if (!d_modeServer) {
    fprintf(stderr, "CollaborationManager::initialize:  "
                    "Couldn't create mode server for collaboration.\n");
    return;
  }

  d_myId_svr = d_peerServer->register_sender("nM Collaboration Manager");
  d_timerSNreply_type =
     d_peerServer->register_message_type("nM collab timer sN reply");

  timerSN =
     d_peerServer->register_message_type("nM collab timer sN");
  d_peerServer->register_handler(timerSN, handle_peerTimer, this);
}



void CollaborationManager::setUI (nmui_Component * ui) {
  d_uiController = ui;
}

void CollaborationManager::setPlaneSync (nmui_PlaneSync * ps) {
  d_planeSync = ps;
}

void CollaborationManager::setUserMode (int mode) {
  d_userMode = mode;
}



void CollaborationManager::setPeerName
                     (const char * newName,
                      void * handChangeData,
                      void (* handChangeCB) (void *, const vrpn_TRACKERCB),
                      void * modeChangeData,
                      void (* modeChangeCB) (void *, const vrpn_ANALOGCB)) {
  char peerHandName [1024];
  char peerModeName [1024];
  char sfbuf [1024];
  char buf [256];
  char hnbuf [256];
  vrpn_bool shouldSynchronize;
  vrpn_int32 newConnection_type;
  vrpn_int32 timerSNreply;

  collabVerbose(1, "CollaborationManager::setPeerName:  Peer named %s.\n",
                newName);

  // Open peerRemote FIRST so we're sure logging happens

  if (d_replay) {
    d_peerRemote = getPeerReplay(d_logPath, d_logTime);
  } else {
    d_peerRemote = getPeer(newName, d_peerPort, d_logPath, d_logTime,
                           d_log, d_NIC_IP);
  }

  collabVerbose(1, "CollaborationManager::setPeerName:  Remote is %ld.\n",
                d_peerRemote);

  // Open peerHand and peerMode

  sprintf(sfbuf, "%s/SharedIFRemLog-%ld.stream", d_logPath, d_logTime);
  if (d_replay) {
    sprintf(peerHandName, "handCoordinateServer0@file:%s", sfbuf);
    sprintf(peerModeName, "userModeServer0@file:%s", sfbuf);
  } else {
    sprintf(peerHandName, "handCoordinateServer0@%s:%d", newName, d_peerPort);
    sprintf(peerModeName, "userModeServer0@%s:%d", newName, d_peerPort);
  }

  collabVerbose(1, "CollaborationManager::setPeerName:  "
                   "seeking %s and %s.\n", peerHandName, peerModeName);

  d_peerHand = new vrpn_Tracker_Remote (peerHandName);
  if (d_peerHand) {
    d_peerHand->register_change_handler (handChangeData, handChangeCB);
    collabVerbose(1, "CollaborationManager::setPeerName:  "
                     "Tracker is using %ld.\n", d_peerHand->connectionPtr());
  }

  d_peerMode = new vrpn_Analog_Remote (peerModeName);
  if (d_peerMode) {
    d_peerMode->register_change_handler (modeChangeData, modeChangeCB);
    collabVerbose(1, "CollaborationManager::setPeerName:  "
                     "Analog is using %ld.\n", d_peerMode->connectionPtr());
  }

  // Figure out who does the synchronizing

  if (d_peerRemote && d_peerRemote->doing_okay()) {
    sprintf(buf, "%s", newName);

    gethostname(hnbuf, 256);
    if (strcmp(hnbuf, buf) < 0) {
      shouldSynchronize = VRPN_TRUE;
    } else {
      shouldSynchronize = VRPN_FALSE;
    }

    if (shouldSynchronize) {
      d_uiController->addPeer(d_peerServer, shouldSynchronize);
    } else {
      d_uiController->addPeer(d_peerRemote, shouldSynchronize);
    }
  }

  // Let everybody else know who the remote is and do other misc.
  // initialization tasks.

  if (d_peerRemote && d_peerRemote->doing_okay()) {
    d_uiController->initializeConnection(d_peerRemote);

    newConnection_type = d_peerRemote->register_message_type
                 (vrpn_got_connection);
    d_peerRemote->register_handler (newConnection_type,
                                    nmui_Component::handle_reconnect,
                                    d_uiController);

    if (d_handServer) {
      d_handServer->bindConnection(d_peerRemote);
      collabVerbose(2, "Bound connection to handServer.\n");
    } else {
      collabVerbose(2, "handServer didn't exist for connection to be bound.\n");
    }

    // Plane sync object also needs to know the name of collaborator
    // to uniquely identify planes created.

    if (d_planeSync) {
      d_planeSync->addPeer(d_peerRemote, newName);
    }
  }

  d_myId_rem = d_peerRemote->register_sender("nM Collaboration Manager");
  d_timerSN_type =
     d_peerRemote->register_message_type("nM collab timer sN");

  timerSNreply =
     d_peerRemote->register_message_type("nM collab timer sN reply");
  d_peerRemote->register_handler(timerSNreply, handle_timerResponse, this);
}



vrpn_Connection * CollaborationManager::peerServerConnection (void) {
  return d_peerServer;
}

vrpn_Connection * CollaborationManager::interfaceLogConnection (void) {
  return d_interfaceLog;
}

vrpn_Connection * CollaborationManager::peerRemoteConnection (void) {
  return d_peerRemote;
}

nM_coord_change * CollaborationManager::handServer (void) {
  return d_handServer;
}

nmui_Component * CollaborationManager::uiRoot (void) {
  return d_uiController;
}

nmui_PlaneSync * CollaborationManager::planeSync (void) {
  return d_planeSync;
}



void CollaborationManager::sendOurTimer (void) {

  char msgbuf [24];
  char * mptr;
  timeval now;
  vrpn_int32 sn;
  vrpn_int32 msglen;

  if (!d_timer) {
    return;
  }

  sn = d_timer->getListHead();
  if ((sn < 0) || (!d_timer->isBlocked(sn))) {
    return;
  }

  msglen = 24;
  mptr = msgbuf;
  vrpn_buffer(&mptr, &msglen, sn);

  gettimeofday(&now, NULL);
  d_peerRemote->pack_message(24 - msglen, now, d_timerSN_type,
                             d_myId_rem, msgbuf, vrpn_CONNECTION_RELIABLE);

  collabVerbose(10, "CollaborationManager::sendOurTimer:  "
                "Sent message for sn %d.\n", sn);
}


void CollaborationManager::respondToPeerTimer (void) {

  char msgbuf [24];
  char * mptr;
  timeval now;
  vrpn_int32 sn;
  vrpn_int32 msglen;

  while ((sn = d_peerTimer.getListHead()) != -1) {
    d_peerTimer.remove();

    msglen = 24;
    mptr = msgbuf;
    vrpn_buffer(&mptr, &msglen, sn);

    gettimeofday(&now, NULL);
    d_peerServer->pack_message(msglen, now, d_timerSNreply_type,
                               d_myId_svr, msgbuf, vrpn_CONNECTION_RELIABLE);

  collabVerbose(10, "CollaborationManager::respondToPeerTimer:  "
                "Done with sn %d.\n", sn);
  }

}


// static
int CollaborationManager::handle_peerTimer (void * userdata,
                                            vrpn_HANDLERPARAM p) {
  CollaborationManager * cm = (CollaborationManager *) userdata;
  const char * bp;
  vrpn_int32 sn;

  bp = p.buffer;
  vrpn_unbuffer(&bp, &sn);

  cm->d_peerTimer.insert(sn);

  collabVerbose(10, "CollaborationManager::handle_peerTimer:  "
                "Queueing up sn %d.\n", sn);

  return 0;
}


// static
int CollaborationManager::handle_timerResponse (void * userdata,
                                                vrpn_HANDLERPARAM p) {
  CollaborationManager * cm = (CollaborationManager *) userdata;
  const char * bp;
  vrpn_int32 sn;

  bp = p.buffer;
  vrpn_unbuffer(&bp, &sn);

  if (cm->d_timer) {
    cm->d_timer->unblock(sn);

  collabVerbose(10, "CollaborationManager::handle_timerResponse:  "
                "Unblocking sn %d.\n", sn);
  }

  return 0;
}




