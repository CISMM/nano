 #include "Tcl_Netvar.h"

#include <stdio.h>

#include <tcl.h>  // for Tcl_Interp

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



#define STARTING_NUM_REPLICAS 2
  // Change this value to control memory usage vs. initialization
  // speed tradeoffs.  Shouldn't be an issue until we hit serious
  // multiuser or start caching states.


static Tcl_Interp * interpreter;

void Tclnet_init (Tcl_Interp * i) {
  interpreter = i;
}

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
  d_replica[0] = new vrpn_Shared_int32_Server (tcl_varname, default_value);
#else
  d_replica[0] = new vrpn_Shared_int32_Server (tcl_varname, default_value,
                                               VRPN_SO_DEFER_UPDATES);
#endif

  if (!d_replica[0]) {
    fprintf(stderr, "TclNet_int::TclNet_int:  Out of memory.\n");
    return;
  }

  d_replica[0]->register_handler
                 (propagateReceivedUpdate, this);

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

vrpn_bool TclNet_int::isSerializer (void) const {
  if (d_replica[d_writeReplica]) {
    return d_replica[d_writeReplica]->isSerializer();
  } else {
    return VRPN_TRUE;
  }
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
  if (!isSerializer()) {
    d_updateFromTcl = VRPN_FALSE;
  }
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

    d_replica[d_activeReplica]->unregister_handler
             (propagateReceivedUpdate, this);

  d_activeReplica = whichReplica;

  d_writeReplica = whichReplica;

    d_replica[whichReplica]->register_handler
                 (propagateReceivedUpdate, this);

  copyReplica(whichReplica);
}





void TclNet_int::lock (void) {
  d_isLocked = VRPN_TRUE;
}
void TclNet_int::unlock (void) {
  d_isLocked = VRPN_FALSE;
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

  // Ignore the next change to this variable generated by Tcl
  // if and only if updateTcl() is going to send an update
  // and Tcl is going to pay attention.
  // (updateTcl() does not send idempotent changes.  Tcl ignores
  // changes generated in the middle of a Trace.)

  //if (!isLocal) {
    //if ((newValue != nti->mylastint) ||
        //(nti->d_permitIdempotentChanges)) {
      //nti->d_ignoreChange = VRPN_TRUE;
//fprintf(stderr, "TclNet_int::conditionalEquals set d_ignoreChange.\n");
    //}
  //}

  nti->Tclvar_int::operator = (newValue);
  nti->d_lastUpdate = when;
  if (interpreter) {
    nti->Tclvar_int::SetFromTcl(newValue);
  }

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
  d_replica[0] = new vrpn_Shared_float64_Server (tcl_varname, default_value);
#else
  d_replica[0] = new vrpn_Shared_float64_Server (tcl_varname, default_value,
                                                 VRPN_SO_DEFER_UPDATES);
#endif

  if (!d_replica[0]) {
    fprintf(stderr, "TclNet_float::TclNet_float:  Out of memory.\n");
    return;
  }

//#ifndef USE_OPTIMISTIC_CONNECTIONS
  d_replica[0]->register_handler
                 (propagateReceivedUpdate, this);
//#endif

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

vrpn_bool TclNet_float::isSerializer (void) const {
  if (d_replica[d_writeReplica]) {
    return d_replica[d_writeReplica]->isSerializer();
  } else {
    return VRPN_TRUE;
  }
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
  if (!isSerializer()) {
    d_updateFromTcl = VRPN_FALSE;
  }
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

    d_replica[d_activeReplica]->unregister_handler
             (propagateReceivedUpdate, this);

  d_activeReplica = whichReplica;

  d_writeReplica = whichReplica;

    d_replica[whichReplica]->register_handler
                 (propagateReceivedUpdate, this);

  copyReplica(whichReplica);
}

void TclNet_float::lock (void) {
  d_isLocked = VRPN_TRUE;
}
void TclNet_float::unlock (void) {
  d_isLocked = VRPN_FALSE;
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

  // Ignore the next change to this variable generated by Tcl
  // if and only if updateTcl() is going to send an update
  // and Tcl is going to pay attention.
  // (updateTcl() does not send idempotent changes.  Tcl ignores
  // changes generated in the middle of a Trace.)

  //if (!isLocal) {
    //if ((newValue != ntf->mylastfloat) ||
        //(ntf->d_permitIdempotentChanges)) {
      //ntf->d_ignoreChange = VRPN_TRUE;
//fprintf(stderr, "TclNet_float::conditionalEquals set d_ignoreChange.\n");
    //}
  //}
  ntf->Tclvar_float::operator = (newValue);
  ntf->d_lastUpdate = when;
  if (interpreter) {
    ntf->Tclvar_float::SetFromTcl(newValue);
  }


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

vrpn_bool TclNet_selector::isSerializer (void) const {
  if (d_replica[d_writeReplica]) {
    return d_replica[d_writeReplica]->isSerializer();
  } else {
    return VRPN_TRUE;
  }
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
  if (!isSerializer()) {
    d_updateFromTcl = VRPN_FALSE;
  }
  /*Tclvar_selector::*/operator = (newValue);
}



// virtual
void TclNet_selector::initializeTcl (const char * tcl_varname,
                                     const char * parent_name) {
  Tclvar_selector::initializeTcl(tcl_varname, parent_name);

#ifdef USE_OPTIMISTIC_CONNECTIONS
  d_replica[0] = new vrpn_Shared_String_Server (tcl_varname, string());
#else
  d_replica[0] = new vrpn_Shared_String_Server (tcl_varname, string(),
                                                VRPN_SO_DEFER_UPDATES);
#endif

  if (!d_replica[0]) {
    fprintf(stderr, "TclNet_selector::TclNet_selector:  Out of memory.\n");
    return;
  }

//#ifndef USE_OPTIMISTIC_CONNECTIONS
  d_replica[0]->register_handler
                 (propagateReceivedUpdate, this);
//#endif

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

    d_replica[d_activeReplica]->unregister_handler
             (propagateReceivedUpdate, this);

  d_activeReplica = whichReplica;

  d_writeReplica = whichReplica;

    d_replica[whichReplica]->register_handler
                 (propagateReceivedUpdate, this);

  copyReplica(whichReplica);
}

void TclNet_selector::lock (void) {
  d_isLocked = VRPN_TRUE;
}
void TclNet_selector::unlock (void) {
  d_isLocked = VRPN_FALSE;
}





// static
// Callback registered on the active Remote replica.
// Executes (*(NetTcl_float *)userdata = newValue).
int TclNet_selector::propagateReceivedUpdate (void * userdata,
                                              const char * newValue,
                                              timeval when,
                                              vrpn_bool isLocal) {
  TclNet_selector * nts;

  if (!userdata) {
    fprintf(stderr, "TclNet_selector::propagateReceivedUpdate:  "
                    "NULL pointer.\n");
    return -1;
  }

  nts = (TclNet_selector *) userdata;

//fprintf(stderr, "TclNet_selector (%s)::propagateReceivedUpdate (%s) "
//"at %ld:%ld.\n", nts->d_myTclVarname, newValue, when.tv_sec, when.tv_usec);

//fprintf(stderr, "In TclNet_selector::propagateReceivedUpdate:  "
//"set %s #%d to %d.\n", nti->my_tcl_varname, nti->d_activeReplica,
//nti->d_myfloat);


  // Ignore the next change to this variable generated by Tcl
  // if and only if updateTcl() is going to send an update
  // and Tcl is going to pay attention.
  // (updateTcl() does not send idempotent changes.  Tcl ignores
  // changes generated in the middle of a Trace.)

  //if (!isLocal) {
    //if ((strcmp(newValue, nts->d_myLastString)) ||
        //(nts->d_permitIdempotentChanges)) {
      //nts->d_ignoreChange = VRPN_TRUE;
//fprintf(stderr, "TclNet_selector::conditionalEquals set d_ignoreChange.\n");
    //}
  //}

  nts->Tclvar_selector::operator = (newValue);
  nts->d_lastUpdate = when;
  if (interpreter) {
    nts->Tclvar_selector::SetFromTcl(newValue);
  }


  return 0;
}


nmb_Selector * allocate_TclNet_selector (const char * initialValue) {
  return new TclNet_selector (initialValue);

}



