#include "nmui_Component.h"

#include <string.h>  // for strncpy(), strcat(), strlen()
#include <stdio.h>  // for sprintf()

#include <vrpn_Connection.h>

#include <Tcl_Linkvar.h>
#include <Tcl_Netvar.h>



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
    d_numPeers (0),
    d_connection (NULL),
    d_myId (-1),
    d_syncRequest_type (-1)
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

  if (d_connection) {
    d_connection->unregister_handler(d_syncRequest_type, handle_syncRequest,
                                     this, d_myId);
    d_connection->unregister_handler(d_syncComplete_type, handle_syncComplete,
                                     this, d_myId);
  }
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

}

  // messages are SENT on d_connection and RECEIVED on the connections
  // bound in addPeer()

void nmui_Component::addPeer (vrpn_Connection * c) {
  int i;

  for (i = 0; i < d_numInts; i++) {
    d_ints[i]->addPeer(c);
  }
  for (i = 0; i < d_numFloats; i++) {
    d_floats[i]->addPeer(c);
  }
  for (i = 0; i < d_numSelectors; i++) {
    d_selectors[i]->addPeer(c);
  }
  for (i = 0; i < d_numComponents; i++) {
    d_components[i]->addPeer(c);
  }

  initializeConnection(c);

  d_peer = c;
  d_numPeers++;

fprintf(stderr, "Finished with addPeer() for nmui Component %s.\n", d_name);
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
}


void nmui_Component::requestSync (void) {
  timeval now;

  if (!d_connection) {
    fprintf(stderr, "nmui_Component::requestSync has no connection.\n");
    return;
  }

  gettimeofday(&now, NULL);
  d_connection->pack_message(0, now, d_syncRequest_type, d_myId, NULL,
                             vrpn_CONNECTION_RELIABLE);

fprintf(stderr, "nmui_Component::requestSync sent.\n");
}

void nmui_Component::registerSyncRequestHandler
                                (nmui_SyncRequestHandler h,
                                 void * userdata) {
  reqHandlerEntry * he;

  if (!d_connection) {
    return;
  }

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

  if (!d_connection) {
    return;
  }

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
  c->initializeConnection(c->d_peer);
  return 0;
}

void nmui_Component::initializeConnection (vrpn_Connection * c) {
  char namebuf [60];
  vrpn_int32 myId;
  vrpn_int32 syncRequest_type;
  vrpn_int32 syncComplete_type;

  sprintf(namebuf, "nmui Component %s", d_name);
  myId = c->register_sender(namebuf);
  syncRequest_type =
      c->register_message_type("nmui Component request sync");
  syncComplete_type =
      c->register_message_type("nmui Component sync complete");
  c->register_handler(syncRequest_type, handle_syncRequest,
                      this, myId);
  c->register_handler(syncComplete_type, handle_syncComplete,
                      this, myId);
}



// static
int nmui_Component::handle_syncRequest (void * userdata, vrpn_HANDLERPARAM) {
  nmui_Component * c;
  reqHandlerEntry * he;
  timeval now;

  c = (nmui_Component *) userdata;

fprintf(stderr, "In component %s handle_syncRequest\n", c->name());

  for (he = c->d_reqHandlers; he; he = he->next) {
    (*he->handler)(he->userdata);
  }

  gettimeofday(&now, NULL);
  c->d_connection->pack_message(0, now, c->d_syncComplete_type,
                                c->d_myId, NULL,
                                vrpn_CONNECTION_RELIABLE);
  return 0;
}


// static
int nmui_Component::handle_syncComplete (void * userdata, vrpn_HANDLERPARAM) {
  nmui_Component * c;
  completeHandlerEntry * he;

  c = (nmui_Component *) userdata;

fprintf(stderr, "In component %s handle_syncComplete\n", c->name());

  for (he = c->d_completeHandlers; he; he = he->next) {
    (*he->handler)(he->userdata);
  }

  return 0;
}

