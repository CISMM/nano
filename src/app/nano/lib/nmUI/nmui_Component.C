#include "nmui_Component.h"

#include <string.h>  // for strncpy(), strcat(), strlen()
#include <stdio.h>  // for sprintf()

#include <vrpn_Connection.h>

#include <Tcl_Linkvar.h>
#include <Tcl_Netvar.h>


// sync:  all sync traffic is SENT over d_connection
// (collaboratingPeerServerConnection in microscape.c) and received over
// d_peer (collaboratingPeerRemoteConnection in microscape.c).


nmui_Component::nmui_Component
                   (char name [30],
                    TclNet_int ** ilist, int numI,
                    TclNet_float ** flist, int numF,
                    TclNet_selector ** sellist, int numSel) :
    d_maintain (VRPN_FALSE),
    d_numInts (numI),
    d_numFloats (numF),
    d_numSelectors (numSel),
    d_numComponents (0),
    d_synchronizedTo (0),
    d_numPeers (0),
    d_isLockedRemotely (VRPN_FALSE),
    d_holdsRemoteLocks (VRPN_FALSE),
    d_connection (NULL),
    d_myId (-1),
    d_syncRequest_type (-1),
    d_syncComplete_type (-1),
    d_lock_type (-1),
    d_unlock_type (-1),
    d_reqHandlers (NULL),
    d_completeHandlers (NULL),
    d_lockHandlers (NULL),
    d_peer (NULL)
{
  int i;

  strncpy(d_name, name, 30);

  for (i = 0; i < numI; i++) {
    add(ilist[i]);
  }
  for (i = 0; i < numF; i++) {
    add(flist[i]);
  }
  for (i = 0; i < numSel; i++) {
    add(sellist[i]);
  }

}

nmui_Component::~nmui_Component (void) {

  if (d_peer) {
    d_peer->unregister_handler(d_syncRequest_type, handle_syncRequest,
                                     this, d_myId);
    d_peer->unregister_handler(d_syncComplete_type, handle_syncComplete,
                                     this, d_myId);
  }
}

const char * nmui_Component::name (void) const {
  return d_name;
}

int nmui_Component::numPeers (void) const {
  return d_numPeers;
 }

int nmui_Component::synchronizedTo (void) const {
  return d_synchronizedTo;
}

vrpn_bool nmui_Component::isLockedRemotely (void) const {
  return d_isLockedRemotely;
}

vrpn_bool nmui_Component::holdsRemoteLocks (void) const {
  return d_holdsRemoteLocks;
}



void nmui_Component::add (TclNet_int * newLinkvar) {
  if (d_numInts >= NMUI_COMPONENT_MAX_SIZE) {
    fprintf(stderr, "nmui_Component::add:  "
                    "Too many linkvars in component.\n");
    return;
  }

  d_ints[d_numInts] = newLinkvar;
  d_numInts++;
}

void nmui_Component::add (TclNet_float * newLinkvar) {
  if (d_numFloats >= NMUI_COMPONENT_MAX_SIZE) {
    fprintf(stderr, "nmui_Component::add:  "
                    "Too many linkvars in component.\n");
    return;
  }

  d_floats[d_numFloats] = newLinkvar;
  d_numFloats++;
}


void nmui_Component::add (TclNet_selector * newLinkvar) {
  if (d_numSelectors >= NMUI_COMPONENT_MAX_SIZE) {
    fprintf(stderr, "nmui_Component::add:  "
                    "Too many linkvars in component.\n");
    return;
  }

  d_selectors[d_numSelectors] = newLinkvar;
  d_numSelectors++;
}


void nmui_Component::add (nmui_Component * newComponent) {
  if (d_numComponents >= NMUI_COMPONENT_MAX_SIZE) {
    fprintf(stderr, "nmui_Component::add:  "
                    "Too many (sub-)components in component.\n");
    return;
  }

  d_components[d_numComponents] = newComponent;
  d_numComponents++;
}


void nmui_Component::bindConnection (vrpn_Connection * c) {
  char namebuf [60];
  int i;

  d_connection = c;

  for (i = 0; i < d_numInts; i++) {
    d_ints[i]->bindConnection(c);
  }
  for (i = 0; i < d_numFloats; i++) {
    d_floats[i]->bindConnection(c);
  }
  for (i = 0; i < d_numSelectors; i++) {
    d_selectors[i]->bindConnection(c);
  }
  for (i = 0; i < d_numComponents; i++) {
    d_components[i]->bindConnection(c);
  }

  sprintf(namebuf, "nmui Component %s", d_name);
  d_myId = d_connection->register_sender(namebuf);
  d_syncRequest_type =
      d_connection->register_message_type("nmui Component request sync");
  d_syncComplete_type =
      d_connection->register_message_type("nmui Component sync complete");

//fprintf(stderr, "## Bound connection %ld for nmuiComponent %s.\n",
//c, d_name);
}

void nmui_Component::bindLogConnection (vrpn_Connection * c) {
  char namebuf [60];
  int i;

  for (i = 0; i < d_numInts; i++) {
    d_ints[i]->bindLogConnection(c);
  }
  for (i = 0; i < d_numFloats; i++) {
    d_floats[i]->bindLogConnection(c);
  }
  for (i = 0; i < d_numSelectors; i++) {
    d_selectors[i]->bindLogConnection(c);
  }
  for (i = 0; i < d_numComponents; i++) {
    d_components[i]->bindLogConnection(c);
  }

}

void nmui_Component::addPeer (vrpn_Connection * c, vrpn_bool serialize) {
  int i;

  for (i = 0; i < d_numInts; i++) {
    d_ints[i]->addPeer(c, serialize);
  }
  for (i = 0; i < d_numFloats; i++) {
    d_floats[i]->addPeer(c, serialize);
  }
  for (i = 0; i < d_numSelectors; i++) {
    d_selectors[i]->addPeer(c, serialize);
  }
  for (i = 0; i < d_numComponents; i++) {
    d_components[i]->addPeer(c, serialize);
  }

  d_peer = c;
  d_numPeers++;

//fprintf(stderr, "## Finished with addPeer() for nmui Component %s, \n"
//"peer connection %ld.\n", d_name, d_peer);
}

void nmui_Component::copyReplica (int whichReplica) {
  int i;

  for (i = 0; i < d_numInts; i++) {
    d_ints[i]->copyReplica(whichReplica);
  }
  for (i = 0; i < d_numFloats; i++) {
    d_floats[i]->copyReplica(whichReplica);
  }
  for (i = 0; i < d_numSelectors; i++) {
    d_selectors[i]->copyReplica(whichReplica);
  }
  for (i = 0; i < d_numComponents; i++) {
    d_components[i]->copyReplica(whichReplica);
  }
}

void nmui_Component::syncReplica (int whichReplica) {
  int i;

  for (i = 0; i < d_numInts; i++) {
    d_ints[i]->syncReplica(whichReplica);
  }
  for (i = 0; i < d_numFloats; i++) {
    d_floats[i]->syncReplica(whichReplica);
  }
  for (i = 0; i < d_numSelectors; i++) {
    d_selectors[i]->syncReplica(whichReplica);
  }
  for (i = 0; i < d_numComponents; i++) {
    d_components[i]->syncReplica(whichReplica);
  }

  d_synchronizedTo = whichReplica;
}


void nmui_Component::requestSync (void) {
  timeval now;

  gettimeofday(&now, NULL);
  d_connection->pack_message(0, now, d_syncRequest_type, d_myId, NULL,
                             vrpn_CONNECTION_RELIABLE);

//fprintf(stderr, "++ nmui_Component::requestSync sent.\n");
}

void nmui_Component::registerSyncRequestHandler
                                (nmui_SyncRequestHandler h,
                                 void * userdata) {
  reqHandlerEntry * he;

  he = new reqHandlerEntry;
  if (!he) {
    fprintf(stderr, "nmui_Component::registerSyncRequestHandler:  "
                    "Out of memory.\n");
    return;
  }

  he->handler = h;
  he->userdata = userdata;
  he->next = d_reqHandlers;
  d_reqHandlers = he;
}

void nmui_Component::registerSyncCompleteHandler
                                (nmui_SyncCompleteHandler h,
                                 void * userdata) {
  completeHandlerEntry * he;

  he = new completeHandlerEntry;
  if (!he) {
    fprintf(stderr, "nmui_Component::registerSyncCompleteHandler:  "
                    "Out of memory.\n");
    return;
  }

  he->handler = h;
  he->userdata = userdata;
  he->next = d_completeHandlers;
  d_completeHandlers = he;
}


// static
int nmui_Component::handle_reconnect (void * userdata, vrpn_HANDLERPARAM) {
  nmui_Component * c;
  c = (nmui_Component *) userdata;
  // XXX  Which connection is that, again?  HACK
  //c->initializeConnection(c->d_peer);
  return 0;
}

void nmui_Component::initializeConnection (vrpn_Connection * c) {
  char namebuf [60];
  vrpn_int32 myId;
  vrpn_int32 syncRequest_type;
  vrpn_int32 syncComplete_type;
  vrpn_int32 lock_type;
  vrpn_int32 unlock_type;

  if (!c) {
    fprintf(stderr, "nmui_Component::initializeConnection:  is NULL!\n");
    return;
  }

//fprintf(stderr, "## In nmui_Component::initializeConnection (%ld).\n", c);

  sprintf(namebuf, "nmui Component %s", d_name);
  myId = c->register_sender(namebuf);
  syncRequest_type =
      c->register_message_type("nmui Component request sync");
  syncComplete_type =
      c->register_message_type("nmui Component sync complete");
  // MAKE SURE THERE'S ONLY ONE COPY OF THE HANDLER - HACK
  c->unregister_handler(syncRequest_type, handle_syncRequest,
                      this, myId);
  c->unregister_handler(syncComplete_type, handle_syncComplete,
                      this, myId);
  c->register_handler(syncRequest_type, handle_syncRequest,
                      this, myId);
  c->register_handler(syncComplete_type, handle_syncComplete,
                      this, myId);
}


// need to break this off of handle_syncRequest so that only one syncComplete
// message is sent

void nmui_Component::do_handle_syncRequest (void) {
  nmui_Component * c;
  reqHandlerEntry * he;
  int i;

  for (he = d_reqHandlers; he; he = he->next) {
    (*he->handler)(he->userdata);
  }

  for (i = 0; i < d_numComponents; i++) {
    d_components[i]->do_handle_syncRequest();
  }
}

// static
int nmui_Component::handle_syncRequest (void * userdata, vrpn_HANDLERPARAM p) {
  nmui_Component * c;
  timeval now;

  c = (nmui_Component *) userdata;

//fprintf(stderr, "++ In component %s handle_syncRequest\n", c->name());

  // HACK
  // Call handlers iff we are synchronized to a remote (non-0) replica.
  // This keeps state consistent between replicas using centralized
  // serialization and shared replicas.
  if (c->synchronizedTo()) {
    c->do_handle_syncRequest();
  }

  gettimeofday(&now, NULL);
  c->d_connection->pack_message(0, now, c->d_syncComplete_type,
                                c->d_myId, NULL,
                                vrpn_CONNECTION_RELIABLE);
  return 0;
}


// static
int nmui_Component::handle_syncComplete (void * userdata, vrpn_HANDLERPARAM p) {
  nmui_Component * c;
  nmui_Component * cc;
  completeHandlerEntry * he;
  int i;

  c = (nmui_Component *) userdata;

//fprintf(stderr, "++ In component %s handle_syncComplete\n", c->name());

  for (he = c->d_completeHandlers; he; he = he->next) {
    (*he->handler)(he->userdata);
  }

  // traverse children
  for (i = 0; i < c->d_numComponents; i++) {
    handle_syncComplete(c->d_components[i], p);
  }

  return 0;
}

