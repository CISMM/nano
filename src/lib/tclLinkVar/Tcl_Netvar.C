#include "Tcl_Netvar.h"

#include <stdio.h>

#include <vrpn_Connection.h>
#include <vrpn_SharedObject.h>  // for vrpn_Shared_int32 and the like

//Uncomment this to use a distributed serialization protocol
//#define USE_OPTIMISTIC_CONNECTIONS

// If we're using centralized serialization,
//  * start up all vrpn_SharedObjects as VRPN_SO_DEFER_UPDATES
//  * always write into d_replica[d_writeReplica],
// which is the same as d_activeReplica, which is whatever end-user
// we are synchronizing this Tclvar with.
//  * accept updates from any replica.

// If we're using distributed serialization, we rely on the clocks of
// all processors involved being acceptably serialized to reduce the latency
// hit of a round-trip to the central serializer.
//  * start up all vrpn_SharedObjects as VRPN_SO_IGNORE_OLD
//  * always write into d_replica[0] (achieved by keeping d_writeReplica
//    always equal to 0)
//  * only accept updates from replicas other than 0.


// We always need to call set on d_replica[d_writeReplica],
// and then call setLocally(d_replica[d_writeReplica]->value()),
// to allow the vrpn_SharedObject to decide the semantics of the set().


#define STARTING_NUM_REPLICAS 2
  // Change this value to control memory usage vs. initialization
  // speed tradeoffs.  Shouldn't be an issue until we hit serious
  // multiuser or start caching states.


TclNet_int::TclNet_int (const char * tcl_varname, vrpn_int32 default_value,
                Linkvar_Intcall c, void * userdata) :
  Tclvar_int (tcl_varname, default_value, c, userdata),
  d_isLocked (VRPN_FALSE),
  d_replica (NULL),
  d_replicaSource (NULL),
  d_writeReplica (0),
  d_activeReplica (0),
  d_numReplicas (0),
  d_numReplicasAllocated (0)
{

  d_replica = new vrpn_Shared_int32 * [STARTING_NUM_REPLICAS];
  d_replicaSource = new vrpn_Connection * [STARTING_NUM_REPLICAS];
  if (!d_replica || !d_replicaSource) {
    fprintf(stderr, "TclNet_int::TclNet_int:  Out of memory.\n");
    return;
  }

  d_numReplicasAllocated = STARTING_NUM_REPLICAS;

#ifdef USE_OPTIMISTIC_CONNECTIONS
  d_replica[0] = new vrpn_Shared_int32_Server (tcl_varname, default_value,
                                               VRPN_SO_IGNORE_OLD);
#else
  d_replica[0] = new vrpn_Shared_int32_Server (tcl_varname, default_value,
                                               VRPN_SO_DEFER_UPDATES);
#endif

  if (!d_replica[0]) {
    fprintf(stderr, "TclNet_int::TclNet_int:  Out of memory.\n");
    return;
  }

  d_numReplicas = 1;


}

// virtual 
TclNet_int::~TclNet_int (void) {
  int i;

  if (d_replica) {
    for (i = 0; i < d_numReplicas; i++) {
      if (d_replica[i]) {
        delete d_replica[i];
      }
    }
    delete [] d_replica;
  }
  if (d_replicaSource) {
    // We don't allocate the connections, so we shouldn't delete them.
    delete [] d_replicaSource;
  }

}



// ACCESSORS

vrpn_bool TclNet_int::isLocked (void) const {
  return d_isLocked;
}



// MANIPULATORS

// If no replica has been allocated,
//   assume we're running disconnected or not collaborative or
//   in some other mode where we should act just like a Linkvar.


// virtual
vrpn_int32 TclNet_int::operator = (vrpn_int32 newValue) {
  timeval now;

  if (!d_isLocked) {

    gettimeofday(&now, NULL);

//fprintf(stderr, "TclNet_int (%s)::operator = (%d) "
//"into d_replica[%d] at %ld:%ld.\n",
//my_tcl_varname, newValue, d_writeReplica,
//now.tv_sec, now.tv_usec);

    if (d_replica[d_writeReplica]) {
      d_replica[d_writeReplica]->set(newValue, now);
    } else {
      Tclvar_int::operator = (newValue);
    }
  }

  return d_myint;
}

// virtual
void TclNet_int::SetFromTcl (vrpn_int32 newValue) {
  /*Tclvar_int::*/operator = (newValue);
}




// Don't insist that the vrpn_Connection be available in
// the constructor, or we can't have globals.
void TclNet_int::bindConnection (vrpn_Connection * c) {
  int i;

  if (!c) {
    // unbind everything - this is a HACK for shutdown
    for (i = 0; i < d_numReplicas; i++) {
      if (d_replica[i]) {
        d_replica[i]->bindConnection(NULL);
      }
    }
  }

  d_replicaSource[0] = c;

#ifndef USE_OPTIMISTIC_CONNECTIONS
  d_replica[0]->register_handler
                 (propagateReceivedUpdate, this);
#endif

//fprintf(stderr, "## TclNet_int::bindConnection\n");
}

void TclNet_int::bindLogConnection (vrpn_Connection * c) {
  d_replica[0]->bindConnection(c);
}


// Add a new peer connection, add a replica, and start
// tracking changes.
void TclNet_int::addPeer (vrpn_Connection * c, vrpn_bool serialize) {
  vrpn_Shared_int32 ** newReplica;
  vrpn_Connection ** newReplicaSource;
  int newNumReplicas;
  int i;

  if (!c) {
    fprintf(stderr, "TclNet_int::addPeer:  NULL pointer.\n");
    return;
  }

  // Reallocate array if necessary - annoying overhead.
  // Could probably get rid of the annoyance if we used STL.

  if (d_numReplicas + 1 >= d_numReplicasAllocated) {
    newNumReplicas = 2 * d_numReplicasAllocated;
    newReplica = new vrpn_Shared_int32 * [newNumReplicas];
    newReplicaSource = new vrpn_Connection * [newNumReplicas];
    if (!newReplica || !newReplicaSource) {
      fprintf(stderr, "TclNet_int::addPeer:  Out of memory.\n");
      return;
    }
    for (i = 0; i < d_numReplicas; i++) {
      newReplica[i] = d_replica[i];
      newReplicaSource[i] = d_replicaSource[i];
    }
    delete [] d_replica;
    delete [] d_replicaSource;
    d_replica = newReplica;
    d_replicaSource = newReplicaSource;
    d_numReplicasAllocated = newNumReplicas;
  }


  // Do the actual work.

//fprintf(stderr, "## TclNet_int::addPeer - replica %d named %s -",
//d_numReplicas - 1, d_replica[0]->name());

  switch (serialize) {
    case vrpn_TRUE:
//fprintf(stderr, "Serializer.\n");
      d_replica[d_numReplicas] = new vrpn_Shared_int32_Server
           (d_replica[0]->name(), d_replica[0]->value(),
                                               VRPN_SO_DEFER_UPDATES);
      d_replicaSource[d_numReplicas] = d_replicaSource[0];
        // HACK:  d_replicaSource[0] is the connection that was passed
        // in to bindConnection();  it's the SERVER connection
      break;
    case vrpn_FALSE:
//fprintf(stderr, "Remote.\n");
      d_replica[d_numReplicas] = new vrpn_Shared_int32_Remote
           (d_replica[0]->name(), d_replica[0]->value(),
                                               VRPN_SO_DEFER_UPDATES);
      d_replicaSource[d_numReplicas] = c;
      break;
  }
  d_replica[d_numReplicas]->bindConnection(d_replicaSource[d_numReplicas]);

  d_numReplicas++;

}



// Copy the state of the which-th replica.
// ISSUE:  Need a better way of specifying which one (?)
void TclNet_int::copyReplica (int whichReplica) {

//fprintf(stderr, "TclNet_int::copyReplica:  Copying %d.\n",
//whichReplica);

  if ((whichReplica < 0) || (whichReplica >= d_numReplicas)) {
    fprintf(stderr, "TclNet_int::copyReplica:  illegal id %d.\n",
            whichReplica);
    return;  // error
  }

  *this = d_replica[whichReplica]->value();

}


// Copy the state of the which-th replica, and any changes to it.
// 0 is the default, local, built-in replica;  syncing to it stops
// receiving any network updates (under OPTIMISTIC_CONNECTIONS).
void TclNet_int::syncReplica (int whichReplica) {

//fprintf(stderr, "++ TclNet_int::syncReplica:  Synchronizing with %d.\n",
//whichReplica);

  if ((whichReplica < 0) || (whichReplica >= d_numReplicas)) {
    fprintf(stderr, "TclNet_int::syncReplica:  illegal id %d.\n",
            whichReplica);
    return;  // error
  }

  if (whichReplica == d_activeReplica) {
    return;  // noop
  }

#ifdef USE_OPTIMISTIC_CONNECTIONS
  if (d_activeReplica > 0)
#endif
  {
    d_replica[d_activeReplica]->unregister_handler
             (propagateReceivedUpdate, this);
  }

  d_activeReplica = whichReplica;

#ifndef USE_OPTIMISTIC_CONNECTIONS
  d_writeReplica = whichReplica;
#endif

#ifdef USE_OPTIMISTIC_CONNECTIONS
  if (whichReplica > 0)
#endif
  {
    d_replica[whichReplica]->register_handler
                 (propagateReceivedUpdate, this);
  }

  copyReplica(whichReplica);
}





void TclNet_int::lock (void) {
  d_isLocked = VRPN_TRUE;
}
void TclNet_int::unlock (void) {
  d_isLocked = VRPN_FALSE;
}



// virtual
vrpn_int32 TclNet_int::conditionalEquals (vrpn_int32 newValue, timeval when,
                                          vrpn_bool isLocal) {

//fprintf(stderr, "TclNet_int::conditionalEquals (%d:%d) OK vs (%d:%d)\n",
//when.tv_sec, when.tv_usec, d_lastUpdate.tv_sec, d_lastUpdate.tv_usec);

  if (!isLocal) {
    if ((newValue != mylastint) ||
        (d_permitIdempotentChanges)) {
      d_ignoreChange = VRPN_TRUE;
//fprintf(stderr, "TclNet_int::conditionalEquals set d_ignoreChange.\n");
    }
  }

  setLocally(newValue, when);
  //doCallbacks();
  Tclvar_int::SetFromTcl(newValue);

  return d_myint;
}

// virtual
vrpn_int32 TclNet_int::setLocally (vrpn_int32 newValue, timeval when) { 

  Tclvar_int::operator = (newValue);
  d_lastUpdate = when;

//fprintf(stderr, "TclNet_int::setLocally (%d) at %ld:%ld.\n",
//newValue, when.tv_sec, when.tv_usec);

  return d_myint;
}



// static
// Callback registered on the active Remote replica.
// Executes (*(NetTcl_int *)userdata = newValue).
int TclNet_int::propagateReceivedUpdate (void * userdata, vrpn_int32 newValue,
                                         timeval when, vrpn_bool isLocal) {
  TclNet_int * nti;

  if (!userdata) {
    fprintf(stderr, "TclNet_int::propagateReceivedUpdate:  "
                    "NULL pointer.\n");
    return -1;
  }

  nti = (TclNet_int *) userdata;

//fprintf(stderr, "TclNet_int (%s)::propagateReceivedUpdate (%d) - %s - "
//"at %ld:%ld.\n",
//nti->my_tcl_varname, newValue, isLocal ? "local" : "remote",
//when.tv_sec, when.tv_usec);

  nti->conditionalEquals(newValue, when, isLocal);

  return 0;
}







TclNet_float::TclNet_float
               (const char * tcl_varname,
                vrpn_float64 default_value,
                Linkvar_Floatcall c, void * userdata) :
  Tclvar_float (tcl_varname, default_value, c, userdata),
  d_isLocked (VRPN_FALSE),
  d_replica (NULL),
  d_replicaSource (NULL),
  d_writeReplica (0),
  d_activeReplica (0),
  d_numReplicas (0),
  d_numReplicasAllocated (0)
{

  d_replica = new vrpn_Shared_float64 * [STARTING_NUM_REPLICAS];
  d_replicaSource = new vrpn_Connection * [STARTING_NUM_REPLICAS];
  if (!d_replica || !d_replicaSource) {
    fprintf(stderr, "TclNet_float::TclNet_float:  Out of memory.\n");
    return;
  }

  d_numReplicasAllocated = STARTING_NUM_REPLICAS;

#ifdef USE_OPTIMISTIC_CONNECTIONS
  d_replica[0] = new vrpn_Shared_float64_Server (tcl_varname, default_value,
                                                 VRPN_SO_IGNORE_OLD);
#else
  d_replica[0] = new vrpn_Shared_float64_Server (tcl_varname, default_value,
                                                 VRPN_SO_DEFER_UPDATES);
#endif

  if (!d_replica[0]) {
    fprintf(stderr, "TclNet_float::TclNet_float:  Out of memory.\n");
    return;
  }

  d_numReplicas = 1;


}

// virtual 
TclNet_float::~TclNet_float (void) {
  int i;

  if (d_replica) {
    for (i = 0; i < d_numReplicas; i++) {
      if (d_replica[i]) {
        delete d_replica[i];
      }
    }
    delete [] d_replica;
  }
  if (d_replicaSource) {
    // We don't allocate the connections, so we shouldn't delete them.
    delete [] d_replicaSource;
  }

}


// ACCESSORS

vrpn_bool TclNet_float::isLocked (void) const {
  return d_isLocked;
}




// MANIPULATORS

// If no replica has been allocated,
//   assume we're running disconnected or not collaborative or
//   in some other mode where we should act just like a Linkvar.


// virtual
vrpn_float64 TclNet_float::operator = (vrpn_float64 newValue) {
  timeval now;

  if (!d_isLocked) {

    gettimeofday(&now, NULL);
//fprintf(stderr, "TclNet_float (%s)::operator = (%.5f) "
//"into d_replica[%d] at %ld:%ld.\n",
//my_tcl_varname, newValue, d_writeReplica, now.tv_sec, now.tv_usec);
    if (d_replica[d_writeReplica]) {
      d_replica[d_writeReplica]->set(newValue, now);
    } else {
      Tclvar_float::operator = (newValue);
    }
  }

  return d_myfloat;
}

// virtual
void TclNet_float::SetFromTcl (vrpn_float64 newValue) {
  /*Tclvar_float::*/operator = (newValue);
}



// Don't insist that the vrpn_Connection be available in
// the constructor, or we can't have globals.
void TclNet_float::bindConnection (vrpn_Connection * c) {
  int i;

  if (!c) {
    // unbind everything - this is a HACK for shutdown
    for (i = 0; i < d_numReplicas; i++) {
      if (d_replica[i]) {
        d_replica[i]->bindConnection(NULL);
      }
    }
  }


  d_replicaSource[0] = c;
  //d_replica[0]->bindConnection(c);

#ifndef USE_OPTIMISTIC_CONNECTIONS
  d_replica[0]->register_handler
                 (propagateReceivedUpdate, this);
#endif

//fprintf(stderr, "## TclNet_float::bindConnection\n");
}

void TclNet_float::bindLogConnection (vrpn_Connection * c) {
  d_replica[0]->bindConnection(c);
}


// Add a new peer connection, add a replica, and start
// tracking changes.
void TclNet_float::addPeer (vrpn_Connection * c, vrpn_bool serialize) {
  vrpn_Shared_float64 ** newReplica;
  vrpn_Connection ** newReplicaSource;
  int newNumReplicas;
  int i;

  if (!c) {
    fprintf(stderr, "TclNet_float::addPeer:  NULL pointer.\n");
    return;
  }

  // Reallocate array if necessary - annoying overhead.
  // Could probably get rid of the annoyance if we used STL.

  if (d_numReplicas + 1 >= d_numReplicasAllocated) {
    newNumReplicas = 2 * d_numReplicasAllocated;
    newReplica = new vrpn_Shared_float64 * [newNumReplicas];
    newReplicaSource = new vrpn_Connection * [newNumReplicas];
    if (!newReplica || !newReplicaSource) {
      fprintf(stderr, "TclNet_float::addPeer:  Out of memory.\n");
      return;
    }
    for (i = 0; i < d_numReplicas; i++) {
      newReplica[i] = d_replica[i];
      newReplicaSource[i] = d_replicaSource[i];
    }
    delete [] d_replica;
    delete [] d_replicaSource;
    d_replica = newReplica;
    d_replicaSource = newReplicaSource;
    d_numReplicasAllocated = newNumReplicas;
  }


  // Do the actual work.
//fprintf(stderr, "## TclNet_float::addPeer - ");

  switch (serialize) {
    case vrpn_TRUE:
//fprintf(stderr, "Serializer.\n");
      d_replica[d_numReplicas] = new vrpn_Shared_float64_Server
           (d_replica[0]->name(), d_replica[0]->value(),
                                               VRPN_SO_DEFER_UPDATES);
      d_replicaSource[d_numReplicas] = d_replicaSource[0];
        // HACK:  d_replicaSource[0] is the connection that was passed
        // in to bindConnection();  it's the SERVER connection
      break;
    case vrpn_FALSE:
//fprintf(stderr, "Remote.\n");
      d_replica[d_numReplicas] = new vrpn_Shared_float64_Remote
           (d_replica[0]->name(), d_replica[0]->value(),
                                               VRPN_SO_DEFER_UPDATES);
      d_replicaSource[d_numReplicas] = c;
      break;
  }
  d_replica[d_numReplicas]->bindConnection(d_replicaSource[d_numReplicas]);

  d_numReplicas++;
}




// Copy the state of the which-th replica.
// ISSUE:  Need a better way of specifying which one (?)
void TclNet_float::copyReplica (int whichReplica) {

//fprintf(stderr, "TclNet_float::copyReplica:  Copying %d.\n",
//whichReplica);

  if ((whichReplica < 0) || (whichReplica >= d_numReplicas)) {
    fprintf(stderr, "TclNet_float::copyReplica:  illegal id %d.\n",
            whichReplica);
    return;  // error
  }

  *this = d_replica[whichReplica]->value();

}


// Copy the state of the which-th replica, and any changes to it.
// 0 is the default, local, built-in replica;  syncing to it stops
// receiving any network updates.
void TclNet_float::syncReplica (int whichReplica) {

//fprintf(stderr, "++ TclNet_float::syncReplica:  Synchronizing with %d.\n",
//whichReplica);

  if ((whichReplica < 0) || (whichReplica >= d_numReplicas)) {
    fprintf(stderr, "TclNet_float::syncReplica:  illegal id %d.\n",
            whichReplica);
    return;  // error
  }

  if (whichReplica == d_activeReplica) {
    return;  // noop
  }

#ifdef USE_OPTIMISTIC_CONNECTIONS
  if (d_activeReplica)
#endif
  {
    d_replica[d_activeReplica]->unregister_handler
             (propagateReceivedUpdate, this);
  }

  d_activeReplica = whichReplica;

#ifndef USE_OPTIMISTIC_CONNECTIONS
  d_writeReplica = whichReplica;
#endif

#ifdef USE_OPTIMISTIC_CONNECTIONS
  if (whichReplica)
#endif
  {
    d_replica[whichReplica]->register_handler
                 (propagateReceivedUpdate, this);
  }

  copyReplica(whichReplica);
}

void TclNet_float::lock (void) {
  d_isLocked = VRPN_TRUE;
}
void TclNet_float::unlock (void) {
  d_isLocked = VRPN_FALSE;
}




// virtual
vrpn_float64 TclNet_float::conditionalEquals (vrpn_float64 newValue,
                                              timeval when,
                                              vrpn_bool isLocal) {

//fprintf(stderr, "TclNet_float::conditionalEquals (%s, %.5f).\n",
//my_tcl_varname, newValue);

  // We want to set d_ignoreChange IFF this change will be sent to
  // Tcl by Tclvar_float::updateTcl().

  if (!isLocal) {
    if ((newValue != mylastfloat) ||
        (d_permitIdempotentChanges)) {
      d_ignoreChange = VRPN_TRUE;
//fprintf(stderr, "TclNet_float::conditionalEquals set d_ignoreChange.\n");
    }
  }

  //setLocally(newValue, when);
  Tclvar_float::SetFromTcl(newValue);

  return d_myfloat;
}

// virtual
vrpn_float64 TclNet_float::setLocally (vrpn_float64 newValue, timeval when) { 

  Tclvar_float::operator = (newValue);
  d_lastUpdate = when;

  return d_myfloat;
}



// static
// Callback registered on the active Remote replica.
// Executes (*(NetTcl_float *)userdata = newValue).
int TclNet_float::propagateReceivedUpdate (void * userdata,
                                           vrpn_float64 newValue,
                                           timeval when,
                                           vrpn_bool isLocal) {
  TclNet_float * ntf;

  if (!userdata) {
    fprintf(stderr, "TclNet_float::propagateReceivedUpdate:  "
                    "NULL pointer.\n");
    return -1;
  }

  ntf = (TclNet_float *) userdata;

//fprintf(stderr, "TclNet_float (%s)::propagateReceivedUpdate (%.5f) "
//"at %ld:%ld.\n", ntf->my_tcl_varname, newValue, when.tv_sec, when.tv_usec);

  ntf->conditionalEquals(newValue, when, isLocal);

  return 0;
}










TclNet_selector::TclNet_selector
               (const char * default_value) :
  Tclvar_selector (default_value),
  d_isLocked (VRPN_FALSE),
  d_replica (NULL),
  d_replicaSource (NULL),
  d_writeReplica (0),
  d_activeReplica (0),
  d_numReplicas (0),
  d_numReplicasAllocated (0)
{

  d_replica = new vrpn_Shared_String * [STARTING_NUM_REPLICAS];
  d_replicaSource = new vrpn_Connection * [STARTING_NUM_REPLICAS];
  if (!d_replica || !d_replicaSource) {
    fprintf(stderr, "TclNet_selector::TclNet_selector:  Out of memory.\n");
    return;
  }

  d_numReplicasAllocated = STARTING_NUM_REPLICAS;

  d_replica[0] = NULL;
}

TclNet_selector::TclNet_selector
               (const char * tcl_varname,
                const char * parent_name,
                const char * default_value,
                Linkvar_Selectcall c, void * userdata) :
  Tclvar_selector (tcl_varname, parent_name, NULL, default_value, c, userdata),
  d_replica (NULL),
  d_replicaSource (NULL),
  d_writeReplica (0),
  d_activeReplica (0),
  d_numReplicas (0),
  d_numReplicasAllocated (0)
{

  d_replica = new vrpn_Shared_String * [STARTING_NUM_REPLICAS];
  d_replicaSource = new vrpn_Connection * [STARTING_NUM_REPLICAS];
  if (!d_replica || !d_replicaSource) {
    fprintf(stderr, "TclNet_selector::TclNet_selector:  Out of memory.\n");
    return;
  }

  d_numReplicasAllocated = STARTING_NUM_REPLICAS;

  initializeTcl(tcl_varname, parent_name);
}

// virtual 
TclNet_selector::~TclNet_selector (void) {
  int i;

  if (d_replica) {
    for (i = 0; i < d_numReplicas; i++) {
      if (d_replica[i]) {
        delete d_replica[i];
      }
    }
    delete [] d_replica;
  }
  if (d_replicaSource) {
    // We don't allocate the connections, so we shouldn't delete them.
    delete [] d_replicaSource;
  }

}


// ACCESSORS

vrpn_bool TclNet_selector::isLocked (void) const {
  return d_isLocked;
}


// MANIPULATORS

// If no replica has been allocated,
//   assume we're running disconnected or not collaborative or
//   in some other mode where we should act just like a Linkvar.


// virtual
const char * TclNet_selector::operator = (const char * newValue) {
  timeval now;

  gettimeofday(&now, NULL);
  if (d_replica[d_writeReplica]) {
    d_replica[d_writeReplica]->set(newValue, now);
  } else {
    Tclvar_selector::operator = (newValue);
  }
  return string();
}

// virtual
const char * TclNet_selector::operator = (char * newValue) {
  timeval now;

  gettimeofday(&now, NULL);
  if (d_replica[d_writeReplica]) {
    d_replica[d_writeReplica]->set(newValue, now);
  } else {
    Tclvar_selector::operator = (newValue);
  }
  return string();
}

// virtual
void TclNet_selector::Set (const char * newValue) {
  timeval now;

  gettimeofday(&now, NULL);
  if (d_replica[d_writeReplica]) {
    d_replica[d_writeReplica]->set(newValue, now);
  } else {
    Tclvar_selector::Set(newValue);
  }
}

// virtual
void TclNet_selector::SetFromTcl (const char * newValue) {
  /*Tclvar_selector::*/operator = (newValue);
}



// virtual
void TclNet_selector::initializeTcl (const char * tcl_varname,
                                     const char * parent_name) {
  Tclvar_selector::initializeTcl(tcl_varname, parent_name);

#ifdef USE_OPTIMISTIC_CONNECTIONS
  d_replica[0] = new vrpn_Shared_String_Server (tcl_varname, string(),
                                                VRPN_SO_IGNORE_OLD);
#else
  d_replica[0] = new vrpn_Shared_String_Server (tcl_varname, string(),
                                                VRPN_SO_DEFER_UPDATES);
#endif

  if (!d_replica[0]) {
    fprintf(stderr, "TclNet_selector::TclNet_selector:  Out of memory.\n");
    return;
  }

  d_numReplicas = 1;
}



// Don't insist that the vrpn_Connection be available in
// the constructor, or we can't have globals.
void TclNet_selector::bindConnection (vrpn_Connection * c) {
  int i;

  if (!c) {
    // unbind everything - this is a HACK for shutdown
    for (i = 0; i < d_numReplicas; i++) {
      if (d_replica[i]) {
        d_replica[i]->bindConnection(NULL);
      }
    }
  }


  if (!d_replica[0]) {
    fprintf(stderr, "TclNet_selector::bindConnection:  Must initializeTcl() "
                    "first!\n");
    return;
  }

  d_replicaSource[0] = c;
  //d_replica[0]->bindConnection(c);

#ifndef USE_OPTIMISTIC_CONNECTIONS
  d_replica[0]->register_handler
                 (propagateReceivedUpdate, this);
#endif

//fprintf(stderr, "## TclNet_selector::bindConnection\n");
}

void TclNet_selector::bindLogConnection (vrpn_Connection * c) {
  d_replica[0]->bindConnection(c);
}

// Add a new peer connection, add a replica, and start
// tracking changes.
void TclNet_selector::addPeer (vrpn_Connection * c, vrpn_bool serialize) {
  vrpn_Shared_String ** newReplica;
  vrpn_Connection ** newReplicaSource;
  int newNumReplicas;
  int i;

  if (!c) {
    fprintf(stderr, "TclNet_selector::addPeer:  NULL pointer.\n");
    return;
  }

  // Reallocate array if necessary - annoying overhead.
  // Could probably get rid of the annoyance if we used STL.

  if (d_numReplicas + 1 >= d_numReplicasAllocated) {
    newNumReplicas = 2 * d_numReplicasAllocated;
    newReplica = new vrpn_Shared_String * [newNumReplicas];
    newReplicaSource = new vrpn_Connection * [newNumReplicas];
    if (!newReplica || !newReplicaSource) {
      fprintf(stderr, "TclNet_selector::addPeer:  Out of memory.\n");
      return;
    }
    for (i = 0; i < d_numReplicas; i++) {
      newReplica[i] = d_replica[i];
      newReplicaSource[i] = d_replicaSource[i];
    }
    delete [] d_replica;
    delete [] d_replicaSource;
    d_replica = newReplica;
    d_replicaSource = newReplicaSource;
    d_numReplicasAllocated = newNumReplicas;
  }


  // Do the actual work.

//fprintf(stderr, "## TclNet_selector::addPeer -");

  switch (serialize) {
    case vrpn_TRUE:
//fprintf(stderr, "Serializer.\n");
      d_replica[d_numReplicas] = new vrpn_Shared_String_Server
           (d_replica[0]->name(), d_replica[0]->value(),
                                               VRPN_SO_DEFER_UPDATES);
      d_replicaSource[d_numReplicas] = d_replicaSource[0];
        // HACK:  d_replicaSource[0] is the connection that was passed
        // in to bindConnection();  it's the SERVER connection
      break;
    case vrpn_FALSE:
//fprintf(stderr, "Remote.\n");
      d_replica[d_numReplicas] = new vrpn_Shared_String_Remote
           (d_replica[0]->name(), d_replica[0]->value(),
                                               VRPN_SO_DEFER_UPDATES);
      d_replicaSource[d_numReplicas] = c;
      break;
  }
  d_replica[d_numReplicas]->bindConnection(d_replicaSource[d_numReplicas]);

  d_numReplicas++;

}



// Copy the state of the which-th replica.
// ISSUE:  Need a better way of specifying which one (?)
void TclNet_selector::copyReplica (int whichReplica) {

//fprintf(stderr, "TclNet_selector::copyReplica:  Copying %d.\n",
//whichReplica);

  if ((whichReplica < 0) || (whichReplica >= d_numReplicas)) {
    fprintf(stderr, "TclNet_selector::copyReplica:  illegal id %d.\n",
            whichReplica);
    return;  // error
  }

  *this = d_replica[whichReplica]->value();

}


// Copy the state of the which-th replica, and any changes to it.
// 0 is the default, local, built-in replica;  syncing to it stops
// receiving any network updates.
void TclNet_selector::syncReplica (int whichReplica) {

//fprintf(stderr, "++ TclNet_selector::syncReplica:  Synchronizing with %d.\n",
//whichReplica);

  if ((whichReplica < 0) || (whichReplica >= d_numReplicas)) {
    fprintf(stderr, "TclNet_selector::syncReplica:  illegal id %d.\n",
            whichReplica);
    return;  // error
  }

  if (whichReplica == d_activeReplica) {
    return;  // noop
  }

#ifdef USE_OPTIMISTIC_CONNECTIONS
  if (d_activeReplica)
#endif
  {
    d_replica[d_activeReplica]->unregister_handler
             (propagateReceivedUpdate, this);
  }

  d_activeReplica = whichReplica;

#ifndef USE_OPTIMISTIC_CONNECTIONS
  d_writeReplica = whichReplica;
#endif

#ifdef USE_OPTIMISTIC_CONNECTIONS
  if (whichReplica)
#endif
  {
    d_replica[whichReplica]->register_handler
                 (propagateReceivedUpdate, this);
  }

  copyReplica(whichReplica);
}

void TclNet_selector::lock (void) {
  d_isLocked = VRPN_TRUE;
}
void TclNet_selector::unlock (void) {
  d_isLocked = VRPN_FALSE;
}




// virtual
const char * TclNet_selector::conditionalEquals (const char * newValue,
                                                 timeval when,
                                                 vrpn_bool isLocal) {

  // We want to set d_ignoreChange IFF this change will be sent to
  // Tcl by Tclvar_float::updateTcl().

  if (!isLocal) {
    if ((strcmp(newValue, d_myLastString)) ||
        (d_permitIdempotentChanges)) {
      d_ignoreChange = VRPN_TRUE;
//fprintf(stderr, "TclNet_selector::conditionalEquals set d_ignoreChange.\n");
    }
  }

  setLocally(newValue, when);
  Tclvar_selector::SetFromTcl(newValue);

  return string();
}

// virtual
const char * TclNet_selector::setLocally (const char * newValue,
                                          timeval when) { 

  Tclvar_selector::operator = (newValue);
  d_lastUpdate = when;

  return string();
}



// static
// Callback registered on the active Remote replica.
// Executes (*(NetTcl_float *)userdata = newValue).
int TclNet_selector::propagateReceivedUpdate (void * userdata,
                                              const char * newValue,
                                              timeval when,
                                              vrpn_bool isLocal) {
  TclNet_selector * ntf;

  if (!userdata) {
    fprintf(stderr, "TclNet_selector::propagateReceivedUpdate:  "
                    "NULL pointer.\n");
    return -1;
  }

  ntf = (TclNet_selector *) userdata;

//fprintf(stderr, "TclNet_selector (%s)::propagateReceivedUpdate (%s) "
//"at %ld:%ld.\n", ntf->d_myTclVarname, newValue, when.tv_sec, when.tv_usec);

  ntf->conditionalEquals(newValue, when, isLocal);

//fprintf(stderr, "In TclNet_selector::propagateReceivedUpdate:  "
//"set %s #%d to %d.\n", nti->my_tcl_varname, nti->d_activeReplica,
//nti->d_myfloat);

  return 0;
}


nmb_Selector * allocate_TclNet_selector (const char * initialValue) {
  return new TclNet_selector (initialValue);

}



