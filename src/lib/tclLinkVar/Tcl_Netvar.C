#include "Tcl_Netvar.h"

#include <stdio.h>
#include <string.h>

#include <vrpn_Connection.h>
#include <vrpn_SharedObject.h>  // for vrpn_Shared_int32 and the like


#define STARTING_NUM_REPLICAS 10
  // change this value to control memory usage


TclNet_int::TclNet_int (const char * tcl_varname, vrpn_int32 default_value,
                Linkvar_Intcall c, void * userdata) :
  Tclvar_int (tcl_varname, default_value, c, userdata),
  d_replica (NULL),
  d_replicaSource (NULL),
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

  d_replica[0] = new vrpn_Shared_int32_Server (tcl_varname, default_value,
                                               VRPN_SO_IGNORE_OLD);

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



// MANIPULATORS

// virtual
vrpn_int32 TclNet_int::operator = (vrpn_int32 newValue) {

  setLocally(newValue);

//fprintf(stderr, "TclNet_int (%s)::operator = (%d) at %ld:%ld.\n",
//my_tcl_varname, newValue, d_lastUpdate.tv_sec, d_lastUpdate.tv_usec);

//fprintf(stderr, "Setting d_replica[0] to %d.\n", d_myint);
  d_replica[0]->set(d_myint, d_lastUpdate);
  return d_myint;
}


// virtual
vrpn_int32 TclNet_int::operator ++ (void) {
  int retval;

  retval = Tclvar_int::operator ++ ();
  gettimeofday(&d_lastUpdate, NULL);

//fprintf(stderr, "Setting d_replica[0] to %d.\n", d_myint);
  *d_replica[0] = d_myint;
  return retval;
}


// virtual
vrpn_int32 TclNet_int::operator ++ (int newValue) {
  int retval;

  retval = Tclvar_int::operator ++ (newValue);
  gettimeofday(&d_lastUpdate, NULL);

//fprintf(stderr, "Setting d_replica[0] to %d.\n", d_myint);
  *d_replica[0] = d_myint;
  return retval;
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
  d_replica[0]->bindConnection(c);

//fprintf(stderr, "TclNet_int::bindConnection\n");
}


// Add a new peer connection, add a replica, and start
// tracking changes.
void TclNet_int::addPeer (vrpn_Connection * c) {
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

  d_replicaSource[d_numReplicas] = c;

  d_replica[d_numReplicas] = new vrpn_Shared_int32_Remote
       (d_replica[0]->name(), d_replica[0]->value(),
                                               VRPN_SO_IGNORE_OLD);
        //(vrpn_int32) VRPN_SO_IGNORE_IDEMPOTENT);

  d_replica[d_numReplicas]->bindConnection(c);

  d_numReplicas++;

//fprintf(stderr, "TclNet_int::addPeer\n");
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
// receiving any network updates.
void TclNet_int::syncReplica (int whichReplica) {

//fprintf(stderr, "TclNet_int::syncReplica:  Synchronizing with %d.\n",
//whichReplica);

  if ((whichReplica < 0) || (whichReplica >= d_numReplicas)) {
    fprintf(stderr, "TclNet_int::syncReplica:  illegal id %d.\n",
            whichReplica);
    return;  // error
  }

  if (whichReplica == d_activeReplica) {
    return;  // noop
  }

  if (d_activeReplica > 0) {
    d_replica[d_activeReplica]->unregister_handler
             (propagateReceivedUpdate, this);
  }

  copyReplica(whichReplica);
  d_activeReplica = whichReplica;

  if (whichReplica) {
    d_replica[whichReplica]->register_handler
                 (propagateReceivedUpdate, this);
  }
}



// virtual
vrpn_int32 TclNet_int::conditionalEquals (vrpn_int32 newValue, timeval when) {

  if (vrpn_TimevalGreater(when, d_lastUpdate)) {
//fprintf(stderr, "TclNet_int::conditionalEquals (%d:%d) OK vs (%d:%d)\n",
//when.tv_sec, when.tv_usec, d_lastUpdate.tv_sec, d_lastUpdate.tv_usec);

    if ((newValue != mylastint) ||
        (d_permitIdempotentChanges)) {
      d_ignoreChange = VRPN_TRUE;
//fprintf(stderr, "TclNet_int::conditionalEquals set d_ignoreChange.\n");
    }

    setLocally(newValue);
    d_lastUpdate = when;
      // overrides behavior in setLocally()

    d_replica[0]->set(d_myint, when);
  } else {
//fprintf(stderr, "TclNet_int::conditionalEquals "
//"(%d:%d) too old (was vs %d:%d)\n",
//when.tv_sec, when.tv_usec, d_lastUpdate.tv_sec, d_lastUpdate.tv_usec);
  }

  return d_myint;
}

// virtual
vrpn_int32 TclNet_int::setLocally (vrpn_int32 newValue) { 

  Tclvar_int::operator = (newValue);
  gettimeofday(&d_lastUpdate, NULL);

//fprintf(stderr, "TclNet_int::setLocally (%d) at %ld:%ld.\n",
//newValue, d_lastUpdate.tv_sec, d_lastUpdate.tv_usec);

  return d_myint;
}



// static
// Callback registered on the active Remote replica.
// Executes (*(NetTcl_int *)userdata = newValue).
int TclNet_int::propagateReceivedUpdate (void * userdata, vrpn_int32 newValue,
                                         timeval when) {
  TclNet_int * nti;

  if (!userdata) {
    fprintf(stderr, "TclNet_int::propagateReceivedUpdate:  "
                    "NULL pointer.\n");
    return -1;
  }

  nti = (TclNet_int *) userdata;

//fprintf(stderr, "TclNet_int (%s)::propagateReceivedUpdate (%d) at %ld:%ld.\n",
//nti->my_tcl_varname, newValue, when.tv_sec, when.tv_usec);

  nti->conditionalEquals(newValue, when);

//fprintf(stderr, "In TclNet_int::propagateReceivedUpdate:  "
//"set %s #%d to %d.\n", nti->my_tcl_varname, nti->d_activeReplica,
//nti->d_myint);

  return 0;
}







TclNet_float::TclNet_float
               (const char * tcl_varname,
                vrpn_float64 default_value,
                Linkvar_Floatcall c, void * userdata) :
  Tclvar_float (tcl_varname, default_value, c, userdata),
  d_replica (NULL),
  d_replicaSource (NULL),
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

  d_replica[0] = new vrpn_Shared_float64_Server (tcl_varname, default_value,
                                                 VRPN_SO_IGNORE_OLD);

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



// MANIPULATORS

// virtual
vrpn_float64 TclNet_float::operator = (vrpn_float64 newValue) {

  setLocally(newValue);

//fprintf(stderr, "Setting d_replica[0] to %d.\n", d_myfloat);
  d_replica[0]->set(d_myfloat, d_lastUpdate);
  return d_myfloat;
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
  d_replica[0]->bindConnection(c);

//fprintf(stderr, "TclNet_float::bindConnection\n");
}


// Add a new peer connection, add a replica, and start
// tracking changes.
void TclNet_float::addPeer (vrpn_Connection * c) {
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

  d_replicaSource[d_numReplicas] = c;

  d_replica[d_numReplicas] = new vrpn_Shared_float64_Remote
       (d_replica[0]->name(), d_replica[0]->value(),
                                               VRPN_SO_IGNORE_OLD);
        //(vrpn_int32) VRPN_SO_IGNORE_IDEMPOTENT);

  d_replica[d_numReplicas]->bindConnection(c);

  d_numReplicas++;

//fprintf(stderr, "TclNet_float::addPeer\n");
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

  if (whichReplica == d_activeReplica) {
    return;  // noop
  }

  *this = d_replica[whichReplica]->value();

}


// Copy the state of the which-th replica, and any changes to it.
// 0 is the default, local, built-in replica;  syncing to it stops
// receiving any network updates.
void TclNet_float::syncReplica (int whichReplica) {

//fprintf(stderr, "TclNet_float::syncReplica:  Synchronizing with %d.\n",
//whichReplica);

  if ((whichReplica < 0) || (whichReplica >= d_numReplicas)) {
    fprintf(stderr, "TclNet_float::syncReplica:  illegal id %d.\n",
            whichReplica);
    return;  // error
  }

  if (whichReplica == d_activeReplica) {
    return;  // noop
  }

  if (d_activeReplica) {
    d_replica[d_activeReplica]->unregister_handler
             (propagateReceivedUpdate, this);
  }

  copyReplica(whichReplica);
  d_activeReplica = whichReplica;

  if (whichReplica) {
    d_replica[whichReplica]->register_handler
                 (propagateReceivedUpdate, this);
  }
}



// virtual
vrpn_float64 TclNet_float::conditionalEquals (vrpn_float64 newValue,
                                              timeval when) {

  if (vrpn_TimevalGreater(when, d_lastUpdate)) {
//fprintf(stderr, "TclNet_float::conditionalEquals (%d:%d) OK\n",
//when.tv_sec, when.tv_usec);

    // We want to set d_ignoreChange IFF this change will be sent to
    // Tcl by Tclvar_float::updateTcl().

    if ((newValue != mylastfloat) ||
        (d_permitIdempotentChanges)) {
      d_ignoreChange = VRPN_TRUE;
    }

    setLocally(newValue);
    d_lastUpdate = when;
      // overrides behavior in setLocally()

    d_replica[0]->set(d_myfloat, when);
  } else {
//fprintf(stderr, "TclNet_float::conditionalEquals (%d:%d) too old\n",
//when.tv_sec, when.tv_usec);
  }

  return d_myfloat;
}

// virtual
vrpn_float64 TclNet_float::setLocally (vrpn_float64 newValue) { 

  Tclvar_float::operator = (newValue);
  gettimeofday(&d_lastUpdate, NULL);

  return d_myfloat;
}



// static
// Callback registered on the active Remote replica.
// Executes (*(NetTcl_float *)userdata = newValue).
int TclNet_float::propagateReceivedUpdate (void * userdata,
                                           vrpn_float64 newValue,
                                           timeval when) {
  TclNet_float * ntf;

  if (!userdata) {
    fprintf(stderr, "TclNet_float::propagateReceivedUpdate:  "
                    "NULL pointer.\n");
    return -1;
  }

  ntf = (TclNet_float *) userdata;

//fprintf(stderr, "TclNet_float (%s)::propagateReceivedUpdate (%.5f) "
//"at %ld:%ld.\n", ntf->my_tcl_varname, newValue, when.tv_sec, when.tv_usec);

  ntf->conditionalEquals(newValue, when);

//fprintf(stderr, "In TclNet_float::propagateReceivedUpdate:  "
//"set %s #%d to %d.\n", nti->my_tcl_varname, nti->d_activeReplica,
//nti->d_myfloat);

  return 0;
}










TclNet_selector::TclNet_selector
               (const char * default_value) :
  Tclvar_selector (default_value),
  d_replica (NULL),
  d_replicaSource (NULL),
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



// MANIPULATORS

// virtual
const char * TclNet_selector::operator = (const char * newValue) {

  setLocally(newValue);

  if (d_replica[0]) {
//fprintf(stderr, "Setting d_replica[0] to %s.\n", string());
    d_replica[0]->set(string(), d_lastUpdate);
  }
  return string();
}

// virtual
const char * TclNet_selector::operator = (char * newValue) {

  setLocally(newValue);

  if (d_replica[0]) {
//fprintf(stderr, "Setting d_replica[0] to %s.\n", string());
    d_replica[0]->set(string(), d_lastUpdate);
  }
  return string();
}


// virtual
void TclNet_selector::initializeTcl (const char * tcl_varname,
                                     const char * parent_name) {
  Tclvar_selector::initializeTcl(tcl_varname, parent_name);

  d_replica[0] = new vrpn_Shared_String_Server (tcl_varname, string(),
                                                VRPN_SO_IGNORE_OLD);

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
  d_replica[0]->bindConnection(c);

//fprintf(stderr, "TclNet_selector::bindConnection\n");
}


// Add a new peer connection, add a replica, and start
// tracking changes.
void TclNet_selector::addPeer (vrpn_Connection * c) {
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

  d_replicaSource[d_numReplicas] = c;

  d_replica[d_numReplicas] = new vrpn_Shared_String_Remote
       (d_replica[0]->name(), d_replica[0]->value(),
                                               VRPN_SO_IGNORE_OLD);
        //(vrpn_int32) VRPN_SO_IGNORE_IDEMPOTENT);

  d_replica[d_numReplicas]->bindConnection(c);

  d_numReplicas++;

//fprintf(stderr, "TclNet_selector::addPeer\n");
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

  if (whichReplica == d_activeReplica) {
    return;  // noop
  }

  *this = d_replica[whichReplica]->value();

}


// Copy the state of the which-th replica, and any changes to it.
// 0 is the default, local, built-in replica;  syncing to it stops
// receiving any network updates.
void TclNet_selector::syncReplica (int whichReplica) {

//fprintf(stderr, "TclNet_selector::syncReplica:  Synchronizing with %d.\n",
//whichReplica);

  if ((whichReplica < 0) || (whichReplica >= d_numReplicas)) {
    fprintf(stderr, "TclNet_selector::syncReplica:  illegal id %d.\n",
            whichReplica);
    return;  // error
  }

  if (whichReplica == d_activeReplica) {
    return;  // noop
  }

  if (d_activeReplica) {
    d_replica[d_activeReplica]->unregister_handler
             (propagateReceivedUpdate, this);
  }

  copyReplica(whichReplica);
  d_activeReplica = whichReplica;

  if (whichReplica) {
    d_replica[whichReplica]->register_handler
                 (propagateReceivedUpdate, this);
  }
}



// virtual
const char * TclNet_selector::conditionalEquals (const char * newValue,
                                                 timeval when) {

  if (vrpn_TimevalGreater(when, d_lastUpdate)) {
//fprintf(stderr, "TclNet_selector::conditionalEquals (%d:%d) OK\n",
//when.tv_sec, when.tv_usec);

    // We want to set d_ignoreChange IFF this change will be sent to
    // Tcl by Tclvar_float::updateTcl().

    if ((strcmp(newValue, d_myLastString)) ||
        (d_permitIdempotentChanges)) {
      d_ignoreChange = VRPN_TRUE;
    }

    setLocally(newValue);
    d_lastUpdate = when;
      // overrides behavior in setLocally()

    if (d_replica[0]) {
      d_replica[0]->set(string(), when);
    }
  } else {
//fprintf(stderr, "TclNet_selector::conditionalEquals (%d:%d) too old\n",
//when.tv_sec, when.tv_usec);
  }

  return string();
}

// virtual
const char * TclNet_selector::setLocally (const char * newValue) { 

  Tclvar_selector::operator = (newValue);
  gettimeofday(&d_lastUpdate, NULL);

  return string();
}



// static
// Callback registered on the active Remote replica.
// Executes (*(NetTcl_float *)userdata = newValue).
int TclNet_selector::propagateReceivedUpdate (void * userdata,
                                              const char * newValue,
                                              timeval when) {
  TclNet_selector * ntf;

  if (!userdata) {
    fprintf(stderr, "TclNet_selector::propagateReceivedUpdate:  "
                    "NULL pointer.\n");
    return -1;
  }

  ntf = (TclNet_selector *) userdata;

//fprintf(stderr, "TclNet_selector (%s)::propagateReceivedUpdate (%s) "
//"at %ld:%ld.\n", ntf->d_myTclVarname, newValue, when.tv_sec, when.tv_usec);

  ntf->conditionalEquals(newValue, when);

//fprintf(stderr, "In TclNet_selector::propagateReceivedUpdate:  "
//"set %s #%d to %d.\n", nti->my_tcl_varname, nti->d_activeReplica,
//nti->d_myfloat);

  return 0;
}


nmb_Selector * allocate_TclNet_selector (const char * initialValue) {
  return new TclNet_selector (initialValue);

}



