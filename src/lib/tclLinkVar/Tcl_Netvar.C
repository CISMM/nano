 #include "Tcl_Netvar.h"

#include <stdio.h>

#include <tcl.h>  // for Tcl_Interp

#include <vrpn_Connection.h>
#include <vrpn_SharedObject.h>  // for vrpn_Shared_int32 and the like

#include <nmb_TimerList.h>
#include <nmb_Debug.h>

/** \file Tcl_Netvar.C
   Implementation for Tcl variables replicated and shared across the network.
   Definitions:
     A shared variable is one that multiple processes can write to.
     A replicated variable is one that has several independant copies
   of its state which readers and writers can choose among.
  
   This package can run in two modes, chosen between by the optional
   second argument to Tclnet_init().
  
   If we're using centralized serialization (useOptimism == VRPN_FALSE),
  
    * start up all vrpn_SharedObjects as VRPN_SO_DEFER_UPDATES
    * always write into d_replica[d_writeReplica],
   which is the same as d_activeReplica, which is whatever end-user
   we are synchronizing this Tclvar with.
    * accept updates from any replica.
  
   If we're using distributed serialization (useOptimism == VRPN_TRUE)
  
    * we rely on the clocks of all processors involved being acceptably
   serialized to reduce the latency hit of a round-trip to the central
   serializer.
    * start up all vrpn_SharedObjects as VRPN_SO_IGNORE_OLD
  
   Verbosity scheme
   ----------------
   0 - no output
   2 - integer netvar initialization
   4 - integer linkvar initialization
   6 - integer netvar assignment
   8 - integer linkvar assignment
   10 - netvar timing data

 */


#define STARTING_NUM_REPLICAS 2
/**<
 * Change this value to control memory usage vs. initialization
 * speed tradeoffs.  Shouldn't be an issue until we hit serious
 * multiuser or start caching states.
 */


static vrpn_bool quashSideEffectsLater = VRPN_FALSE;
static vrpn_bool quashSideEffects = VRPN_FALSE;
/**<
 * When quashSideEffects is true, all calls to "operator =" or
 * equivalent functions should return without effect, IFF this is
 * a Netvar that is properly bound to a connection (NOT just bound
 * to a logging connection)!  This can be determined by checking
 * d_replicaSource[0], which is NULL when we're logging but valid
 * if we're actually replicated.
 */

static Tcl_Interp * interpreter;
static vrpn_int32 sharedObjectMode = VRPN_SO_DEFER_UPDATES;
static vrpn_bool migrateSerializer = VRPN_FALSE;
static nmb_TimerList * tclnetTimer = NULL;

void Tclnet_init (Tcl_Interp * i, int collabMode,
                  nmb_TimerList * timer) {
  interpreter = i;

  switch (collabMode) {
    
    case TCLNET_SERIALIZE_CENTRALIZED:
      break;
    case TCLNET_SERIALIZE_MIGRATING:
      migrateSerializer = VRPN_TRUE;
      break;
    case TCLNET_SERIALIZE_WALLCLOCK:
      sharedObjectMode = VRPN_SO_IGNORE_OLD;
      break;
    case TCLNET_SERIALIZE_LAMPORTCLOCK:
      fprintf(stderr, "TCLNET_SERIALIZE_LAMPORTCLOCK not implemented!\n");
      break;
  }

  tclnetTimer = timer;
}

static void activateTimer (void) {
  vrpn_int32 sn;

  if (!tclnetTimer) {
    return;
  }
  sn = tclnetTimer->getListHead();
  if (sn != -1) {
    tclnetTimer->activate(sn);
    collabVerbose(10, "Activated timer %d.\n", sn);
  }
}

static int blockTimer (void *) {
  vrpn_int32 sn;

  if (!tclnetTimer) {
    return 0;
  }
  sn = tclnetTimer->getListHead();
  if (sn != -1) {
    tclnetTimer->block(sn);
    collabVerbose(10, "Blocked timer %d.\n", sn);
  }
  return 0;
}






Tcl_Netvar::Tcl_Netvar (void) :
    d_replica (NULL),
    d_replicaSource (NULL),
    d_writeReplica (0),
    d_activeReplica (0),
    d_numReplicas (0),
    d_numReplicasAllocated (0),
    d_isLocked (VRPN_FALSE) {

  int i;

  d_replica = new vrpn_SharedObject * [STARTING_NUM_REPLICAS];
  d_replicaSource = new vrpn_Connection * [STARTING_NUM_REPLICAS];
  if (!d_replica || !d_replicaSource) {
    fprintf(stderr, "Tcl_Netvar::TclNet_var:  Out of memory.\n");
    return;
  }

  d_numReplicasAllocated = STARTING_NUM_REPLICAS;
  for (i = 0; i < d_numReplicasAllocated; i++) {
      d_replica[i] = NULL;
      d_replicaSource[i] = NULL;
  }
}

// virtual
Tcl_Netvar::~Tcl_Netvar (void) {
  int i;

  if (d_replica) {
    for (i = 0; i < d_numReplicas; i++) {
      if (d_replica[i]) {
        delete d_replica[i];
	d_replica[i] = NULL;
      }
    }
    delete [] d_replica;
    d_replica = NULL;
  }
  if (d_replicaSource) {
    // We don't allocate the connections, so we shouldn't delete them.
    delete [] d_replicaSource;
    d_replicaSource = NULL;
  }

}

vrpn_bool Tcl_Netvar::isLocked (void) const {
  return d_isLocked;
}

vrpn_bool Tcl_Netvar::isSerializer (void) const {
  if (d_replica[d_writeReplica]) {
    return d_replica[d_writeReplica]->isSerializer();
  } else {
    return VRPN_TRUE;
  }
}


// Don't insist that the vrpn_Connection be available in
// the constructor, or we can't have globals.
void Tcl_Netvar::bindConnection (vrpn_Connection * c) {
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

//fprintf(stderr, "## Tcl_Netvar::bindConnection\n");
}

void Tcl_Netvar::bindLogConnection (vrpn_Connection * c) {
  if (!d_replica[0]) {
    fprintf(stderr, "Tcl_Netvar::bindLogConnection:  "
                    "Must initialize Tcl first!\n");
  }

  d_replica[0]->bindConnection(c);
}




// Add a new peer connection, add a replica, and start
// tracking changes.
void Tcl_Netvar::addPeer (vrpn_Connection * c, vrpn_bool serialize) {
  int newReplicaIndex;

  if (!c) {
    fprintf(stderr, "Tcl_Netvar::addPeer:  NULL pointer.\n");
    return;
  }

  // Do the actual work.

//fprintf(stderr, "## Tcl_Netvar::addPeer -");

  switch (serialize) {
    case vrpn_TRUE:
//fprintf(stderr, "Serializer.\n");
      newReplicaIndex = addServerReplica();
      d_replicaSource[newReplicaIndex] = d_replicaSource[0];
        // HACK:  d_replicaSource[0] is the connection that was passed
        // in to bindConnection();  it's the SERVER connection
      break;
    case vrpn_FALSE:
//fprintf(stderr, "Remote.\n");
      newReplicaIndex = addRemoteReplica();
      d_replicaSource[newReplicaIndex] = c;
      break;
  }
  d_replica[newReplicaIndex]->bindConnection
            (d_replicaSource[newReplicaIndex]);
}



void Tcl_Netvar::lock (void) {
  d_isLocked = VRPN_TRUE;
}

void Tcl_Netvar::unlock (void) {
  d_isLocked = VRPN_FALSE;
}


void Tcl_Netvar::reallocateReplicaArrays (void) {
  vrpn_SharedObject ** newReplica;
  vrpn_Connection ** newReplicaSource;
  int newNumReplicas;
  int i;

  if (d_numReplicas + 1 >= d_numReplicasAllocated) {
    newNumReplicas = 2 * d_numReplicasAllocated;
    newReplica = new vrpn_SharedObject * [newNumReplicas];
    newReplicaSource = new vrpn_Connection * [newNumReplicas];
    if (!newReplica || !newReplicaSource) {
      fprintf(stderr,
              "TclNet_int::reallocateReplicaArrays:  Out of memory.\n");
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

}

void Tcl_Netvar::setupReplica (vrpn_SharedObject * r) {
  r->registerDeferredUpdateCallback(blockTimer, NULL);
}







TclNet_int::TclNet_int (const char * tcl_varname, vrpn_int32 default_value,
                Linkvar_Intcall c, void * userdata,
                vrpn_bool readDefaultFromTcl) :
  Tclvar_int (tcl_varname, default_value, c, userdata, readDefaultFromTcl),
  Tcl_Netvar ()
{

  int newReplicaIndex;

  newReplicaIndex = addServerReplica();

  ((vrpn_Shared_int32 *) d_replica[newReplicaIndex])->register_handler
                 (propagateReceivedUpdate, this);

}

// virtual 
TclNet_int::~TclNet_int (void) {

}



// ACCESSORS





// MANIPULATORS

// If no replica has been allocated,
//   assume we're running disconnected or not collaborative or
//   in some other mode where we should act just like a Linkvar.


// virtual
vrpn_int32 TclNet_int::operator = (vrpn_int32 newValue) {
  timeval now;

  if (quashSideEffects && (d_numReplicas > 1)) {
    collabVerbose(6, "Quashing set of %s.\n", d_myTclVarname);
    return d_myint;
  }

  activateTimer();

  if (!isLocked()) {

    gettimeofday(&now, NULL);

    collabVerbose(6, "TclNet_int (%s)::operator = (%d) "
                  "into d_replica[%d] at %ld:%ld.\n",
                  d_myTclVarname, newValue, d_writeReplica,
                  now.tv_sec, now.tv_usec);

    if (d_replica[d_writeReplica]) {
      ((vrpn_Shared_int32 *) d_replica[d_writeReplica])->set(newValue, now);
      if (migrateSerializer) {
        d_replica[d_writeReplica]->becomeSerializer();
      }
    } else {
      // Only triggered if we weren't properly initialized;  this
      // short-circuits around vrpn_SharedObject as if we were a Tclvar_int.
      Tclvar_int::operator = (newValue);
    }
  }

  return d_myint;
}

// virtual
int TclNet_int::addServerReplica (void) {

  reallocateReplicaArrays();

  d_replica[d_numReplicas] = new vrpn_Shared_int32_Server
           (d_myTclVarname,
            d_myint,
            sharedObjectMode);
  if (!d_replica[d_numReplicas]) {
    fprintf(stderr, "TclNet_int::addServerReplica:  Out of memory.\n");
    return -1;
  }

  setupReplica(d_replica[d_numReplicas]);

  return d_numReplicas++;
}

// virtual
int TclNet_int::addRemoteReplica (void) {

  reallocateReplicaArrays();

  d_replica[d_numReplicas] = new vrpn_Shared_int32_Remote
           (d_myTclVarname,
            d_myint,
            sharedObjectMode);
  if (!d_replica[d_numReplicas]) {
    fprintf(stderr, "TclNet_int::addRemoteReplica:  Out of memory.\n");
    return -1;
  }

  setupReplica(d_replica[d_numReplicas]);

  return d_numReplicas++;
}





// virtual
void TclNet_int::SetFromTcl (vrpn_int32 newValue) {
  operator = (newValue);
}





// Copy the state of the which-th replica.
// ISSUE:  Need a better way of specifying which one (?)
void TclNet_int::copyReplica (int whichReplica) {

  collabVerbose(6, "TclNet_int::copyReplica:  Copying %d.\n",
                whichReplica);

  if ((whichReplica < 0) || (whichReplica >= d_numReplicas)) {
    fprintf(stderr, "TclNet_int::copyReplica:  illegal id %d.\n",
            whichReplica);
    return;  // error
  }

  quashSideEffectsLater = VRPN_TRUE;
  *this = ((vrpn_Shared_int32 *) d_replica[whichReplica])->value();
  quashSideEffectsLater = VRPN_FALSE;

}

// Copy the state of the source replica to the destination replica.
void TclNet_int::copyFromToReplica (int sourceReplica, int destReplica)
{

  collabVerbose(6, "++ TclNet_int::copyFromToReplica:  "
                "Copying from %d to %d.\n",
                sourceReplica, destReplica);

  if ((sourceReplica < 0) || (sourceReplica >= d_numReplicas)) {
    fprintf(stderr, "TclNet_int::copyFromToReplica:  illegal id %d.\n",
            sourceReplica);
    return;  // error
  }
  if ((destReplica < 0) || (destReplica >= d_numReplicas)) {
    fprintf(stderr, "TclNet_int::copyFromToReplica:  illegal id %d.\n",
            destReplica);
    return;  // error
  }

  quashSideEffectsLater = VRPN_TRUE;
  if ( d_activeReplica == destReplica) {
      *this = ((vrpn_Shared_int32 *) d_replica[sourceReplica])->value();
  } else {
      timeval now;
      gettimeofday(&now, NULL);

      ((vrpn_Shared_int32 *) d_replica[destReplica])->set
         (((vrpn_Shared_int32 *) d_replica[sourceReplica])->value(), now);
  }
  quashSideEffectsLater = VRPN_FALSE;
}


// Copy the state of the which-th replica, and any changes to it.
// 0 is the default, local, built-in replica;  syncing to it stops
// receiving any network updates (under OPTIMISTIC_CONNECTIONS).
void TclNet_int::syncReplica (int whichReplica) {

  collabVerbose(6, "++ TclNet_int::syncReplica:  Synchronizing with %d.\n",
                whichReplica);

  if ((whichReplica < 0) || (whichReplica >= d_numReplicas)) {
    fprintf(stderr, "TclNet_int::syncReplica:  illegal id %d.\n",
            whichReplica);
    return;  // error
  }

  if (whichReplica == d_activeReplica) {
    return;  // noop
  }

  ((vrpn_Shared_int32 *) d_replica[d_activeReplica])->unregister_handler
             (propagateReceivedUpdate, this);

  d_activeReplica = whichReplica;

  d_writeReplica = whichReplica;

  ((vrpn_Shared_int32 *) d_replica[whichReplica])->register_handler
                 (propagateReceivedUpdate, this);

  copyReplica(whichReplica);
}

void TclNet_int::setReplica (int whichReplica, vrpn_int32 value) {

  timeval now;

  if ((whichReplica < 0) || (whichReplica >= d_numReplicas)) {
    fprintf(stderr, "TclNet_int::setReplica:  illegal id %d.\n",
            whichReplica);
    return;  // error
  }

  gettimeofday(&now, NULL);

  ((vrpn_Shared_int32 *) d_replica[whichReplica])->set(value, now);
}









// static
// Callback registered on the active Remote replica.
// Executes (*(NetTcl_int *)userdata = newValue).
int TclNet_int::propagateReceivedUpdate (
    void * userdata,
    vrpn_int32 newValue,
    timeval when,
    vrpn_bool isLocal)
{
  TclNet_int * nti;

  if (!userdata) {
    fprintf(stderr, "TclNet_int::propagateReceivedUpdate:  "
                    "NULL pointer.\n");
    return -1;
  }

  nti = (TclNet_int *) userdata;

  collabVerbose(6, "TclNet_int (%s)::propagateReceivedUpdate (%d) - %s - "
                "at %ld:%ld.\n",
                nti->d_myTclVarname, newValue, isLocal ? "local" : "remote",
                when.tv_sec, when.tv_usec);

  // Ignore the next change to this variable generated by Tcl
  // if and only if updateTcl() is going to send an update
  // and Tcl is going to pay attention.
  // (updateTcl() does not send idempotent changes.  Tcl ignores
  // changes generated in the middle of a Trace.)

  collabVerbose(6, "          with ignore %d and fromTcl %d.\n",
                nti->d_ignoreChange, nti->d_updateFromTcl);


  if (!nti->d_updateFromTcl && interpreter) {

    // Since we're not inside a Tcl callback, we're about to generate one (if
    // an interpreter exists), which we want to ignore to avoid endless loops
    // or hitting the callback twice.

    collabVerbose(8, "TclNet_int::updateTcl(%s):  setting d_ignoreChange.\n",
                  nti->d_myTclVarname);
    nti->d_ignoreChange = VRPN_TRUE;
  }

  nti->Tclvar_int::operator = (newValue);
  nti->d_lastUpdate = when;
  if (interpreter) {
    // SetFromTcl() MUST come after operator = ().

    // This is where side effects (&callbacks) happen
    if (!isLocal || quashSideEffectsLater) {
      quashSideEffects = VRPN_TRUE;
    }
    nti->Tclvar_int::SetFromTcl(newValue);
    quashSideEffects = VRPN_FALSE;
  }

  collabVerbose(6, "Leaving TclNet_int::pre with ignore %d and fromTcl %d.\n",
                nti->d_ignoreChange, nti->d_updateFromTcl);

  return 0;
}







TclNet_float::TclNet_float
               (const char * tcl_varname,
                vrpn_float64 default_value,
                Linkvar_Floatcall c, void * userdata) :
  Tclvar_float (tcl_varname, default_value, c, userdata),
  Tcl_Netvar () {

  int newReplicaIndex;

  newReplicaIndex = addServerReplica();

  ((vrpn_Shared_float64 *) d_replica[newReplicaIndex])->register_handler
                 (propagateReceivedUpdate, this);

}

// virtual 
TclNet_float::~TclNet_float (void) {

}


// ACCESSORS



// MANIPULATORS

// If no replica has been allocated,
//   assume we're running disconnected or not collaborative or
//   in some other mode where we should act just like a Linkvar.


// virtual
vrpn_float64 TclNet_float::operator = (vrpn_float64 newValue) {
  timeval now;

  if (quashSideEffects && (d_numReplicas > 1)) {
    collabVerbose(6, "Quashing set of %s.\n", d_myTclVarname);
    return d_myfloat;
  }

  activateTimer();

  if (!isLocked()) {

    gettimeofday(&now, NULL);
    collabVerbose(6, "TclNet_float (%s)::operator = (%.5f) "
                  "into d_replica[%d] at %ld:%ld.\n",
                  d_myTclVarname, newValue, d_writeReplica,
                  now.tv_sec, now.tv_usec);

//fprintf(stderr, "TclNet_float (%s)::operator = (%.5f) "
//"into d_replica[%d] at %ld:%ld.\n",
//my_tcl_varname, newValue, d_writeReplica, now.tv_sec, now.tv_usec);
    if (d_replica[d_writeReplica]) {
      ((vrpn_Shared_float64 *) d_replica[d_writeReplica])->set(newValue, now);
      if (migrateSerializer) {
        d_replica[d_writeReplica]->becomeSerializer();
      }
    } else {
      Tclvar_float::operator = (newValue);
    }
  }

  return d_myfloat;
}

// virtual
int TclNet_float::addServerReplica (void) {

  reallocateReplicaArrays();

  d_replica[d_numReplicas] = new vrpn_Shared_float64_Server
           (d_myTclVarname,
            d_myfloat,
            sharedObjectMode);
  if (!d_replica[d_numReplicas]) {
    fprintf(stderr, "TclNet_float::addServerReplica:  Out of memory.\n");
    return -1;
  }

  setupReplica(d_replica[d_numReplicas]);

  return d_numReplicas++;
}

// virtual
int TclNet_float::addRemoteReplica (void) {

  reallocateReplicaArrays();

  d_replica[d_numReplicas] = new vrpn_Shared_float64_Remote
           (d_myTclVarname,
            d_myfloat,
            sharedObjectMode);
  if (!d_replica[d_numReplicas]) {
    fprintf(stderr, "TclNet_float::addRemoteReplica:  Out of memory.\n");
    return -1;
  }

  setupReplica(d_replica[d_numReplicas]);

  return d_numReplicas++;
}


// virtual
void TclNet_float::SetFromTcl (vrpn_float64 newValue) {
  operator = (newValue);
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

  quashSideEffectsLater = VRPN_TRUE;
  *this = ((vrpn_Shared_float64 *) d_replica[whichReplica])->value();
  quashSideEffectsLater = VRPN_FALSE;

}

// Copy the state of the source replica to the destination replica.
void TclNet_float::copyFromToReplica (int sourceReplica, int destReplica)
{
  if ((sourceReplica < 0) || (sourceReplica >= d_numReplicas)) {
    fprintf(stderr, "TclNet_int::copyFromToReplica:  illegal id %d.\n",
            sourceReplica);
    return;  // error
  }
  if ((destReplica < 0) || (destReplica >= d_numReplicas)) {
    fprintf(stderr, "TclNet_int::copyFromToReplica:  illegal id %d.\n",
            destReplica);
    return;  // error
  }

  quashSideEffectsLater = VRPN_TRUE;
  if ( d_activeReplica == destReplica) {
      *this = ((vrpn_Shared_float64 *) d_replica[sourceReplica])->value();
  } else {
      timeval now;
      gettimeofday(&now, NULL);

      ((vrpn_Shared_float64 *) d_replica[destReplica])->set
         (((vrpn_Shared_float64 *) d_replica[sourceReplica])->value(), now);
  }
  quashSideEffectsLater = VRPN_FALSE;
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

    ((vrpn_Shared_float64 *) d_replica[d_activeReplica])->unregister_handler
             (propagateReceivedUpdate, this);

  d_activeReplica = whichReplica;

  d_writeReplica = whichReplica;

    ((vrpn_Shared_float64 *) d_replica[whichReplica])->register_handler
                 (propagateReceivedUpdate, this);

  copyReplica(whichReplica);
}

void TclNet_float::setReplica (int whichReplica, vrpn_float64 value) {

  timeval now;

  if ((whichReplica < 0) || (whichReplica >= d_numReplicas)) {
    fprintf(stderr, "TclNet_float::setReplica:  illegal id %d.\n",
            whichReplica);
    return;  // error
  }

  gettimeofday(&now, NULL);

  ((vrpn_Shared_float64 *) d_replica[whichReplica])->set(value, now);
}






// static
// Callback registered on the active Remote replica.
// Executes (*(NetTcl_float *)userdata = newValue).
int TclNet_float::propagateReceivedUpdate (
    void * userdata,
    vrpn_float64 newValue,
    timeval when,
    vrpn_bool isLocal)
{
  TclNet_float * ntf;

  if (!userdata) {
    fprintf(stderr, "TclNet_float::propagateReceivedUpdate:  "
                    "NULL pointer.\n");
    return -1;
  }

  ntf = (TclNet_float *) userdata;

  collabVerbose(6, "TclNet_float (%s)::propagateReceivedUpdate (%.5f) - %s - "
                "at %ld:%ld.\n",
                ntf->d_myTclVarname, newValue, isLocal ? "local" : "remote",
                when.tv_sec, when.tv_usec);

  // Ignore the next change to this variable generated by Tcl
  // if and only if updateTcl() is going to send an update
  // and Tcl is going to pay attention.
  // (updateTcl() does not send idempotent changes.  Tcl ignores
  // changes generated in the middle of a Trace.)

  if (!ntf->d_updateFromTcl && interpreter) {

    // Since we're not inside a Tcl callback, we're about to generate one (if
    // an interpreter exists), which we want to ignore to avoid endless loops
    // or hitting the callback twice.

    collabVerbose(8,
                  "TclNet_float::updateTcl(%s):  setting d_ignoreChange.\n",
                  ntf->d_myTclVarname);
    ntf->d_ignoreChange = VRPN_TRUE;
  }

  ntf->Tclvar_float::operator = (newValue);
  ntf->d_lastUpdate = when;
  if (interpreter) {
    // SetFromTcl() MUST come after operator = ().

    // This is where side effects (&callbacks) happen
    if (!isLocal || quashSideEffectsLater) {
      quashSideEffects = VRPN_TRUE;
    }
    ntf->Tclvar_float::SetFromTcl(newValue);
    quashSideEffects = VRPN_FALSE;
  }
  return 0;
}










TclNet_string::TclNet_string
               (const char * default_value) :
  Tclvar_string (default_value),
  Tcl_Netvar () {

  d_replica[0] = NULL;
}

TclNet_string::TclNet_string
               (const char * tcl_varname,
                const char * default_value,
                Linkvar_Stringcall c, void * userdata) :
  Tclvar_string (tcl_varname, default_value, c, userdata),
  Tcl_Netvar () {

  initializeTcl(tcl_varname);
}

// virtual 
TclNet_string::~TclNet_string (void) {


}


// ACCESSORS


// MANIPULATORS

// If no replica has been allocated,
//   assume we're running disconnected or not collaborative or
//   in some other mode where we should act just like a Linkvar.


// virtual
const char * TclNet_string::operator = (const char * newValue) {
  timeval now;

  if (quashSideEffects && (d_numReplicas > 1)) {
    collabVerbose(6, "Quashing set of %s.\n", d_myTclVarname);
    return string();
  }

  activateTimer();

  if (!isLocked()) {
    gettimeofday(&now, NULL);
    if (d_replica[d_writeReplica]) {
      ((vrpn_Shared_String *) d_replica[d_writeReplica])->set(newValue, now);
      if (migrateSerializer) {
        d_replica[d_writeReplica]->becomeSerializer();
      }
    } else {
      Tclvar_string::operator = (newValue);
    }
  }

  return string();
}

// virtual
const char * TclNet_string::operator = (char * newValue) {
  timeval now;

  if (quashSideEffects && (d_numReplicas > 1)) {
    collabVerbose(6, "Quashing set of %s.\n", d_myTclVarname);
    return string();
  }

  activateTimer();

  if (!isLocked()) {
    gettimeofday(&now, NULL);
    if (d_replica[d_writeReplica]) {
      ((vrpn_Shared_String *) d_replica[d_writeReplica])->set(newValue, now);
      if (migrateSerializer) {
        d_replica[d_writeReplica]->becomeSerializer();
      }
    } else {
      Tclvar_string::operator = (newValue);
    }
  }

  return string();
}

// virtual
void TclNet_string::Set (const char * newValue) {
  timeval now;

  if (quashSideEffects && (d_numReplicas > 1)) {
    collabVerbose(6, "Quashing set of %s.\n", d_myTclVarname);
    return;
  }

  activateTimer();

  if (!isLocked()) {
    gettimeofday(&now, NULL);
    if (d_replica[d_writeReplica]) {
      ((vrpn_Shared_String *) d_replica[d_writeReplica])->set(newValue, now);
      if (migrateSerializer) {
        d_replica[d_writeReplica]->becomeSerializer();
      }
    } else {
      Tclvar_string::Set(newValue);
    }
  }
}

// virtual
int TclNet_string::addServerReplica (void) {

  reallocateReplicaArrays();

  d_replica[d_numReplicas] = new vrpn_Shared_String_Server
           (d_myTclVarname,
            string(),
            sharedObjectMode);
  if (!d_replica[d_numReplicas]) {
    fprintf(stderr, "TclNet_string::addServerReplica:  Out of memory.\n");
    return -1;
  }

  setupReplica(d_replica[d_numReplicas]);

  return d_numReplicas++;
}

// virtual
int TclNet_string::addRemoteReplica (void) {

  reallocateReplicaArrays();

  d_replica[d_numReplicas] = new vrpn_Shared_String_Remote
           (d_myTclVarname,
            string(),
            sharedObjectMode);
  if (!d_replica[d_numReplicas]) {
    fprintf(stderr, "TclNet_string::addRemoteReplica:  Out of memory.\n");
    return -1;
  }

  setupReplica(d_replica[d_numReplicas]);

  return d_numReplicas++;
}



// virtual
void TclNet_string::SetFromTcl (const char * newValue) {
  operator = (newValue);
}



// virtual
void TclNet_string::initializeTcl (const char * tcl_varname) {
  int newReplicaIndex;

  Tclvar_string::initializeTcl(tcl_varname);

  newReplicaIndex = addServerReplica();

  ((vrpn_Shared_String *) d_replica[newReplicaIndex])->register_handler
                 (propagateReceivedUpdate, this);

}




// Copy the state of the which-th replica.
// ISSUE:  Need a better way of specifying which one (?)
void TclNet_string::copyReplica (int whichReplica) {

//fprintf(stderr, "TclNet_string::copyReplica:  Copying %d.\n",
//whichReplica);

  if ((whichReplica < 0) || (whichReplica >= d_numReplicas)) {
    fprintf(stderr, "TclNet_string::copyReplica:  illegal id %d.\n",
            whichReplica);
    return;  // error
  }

  quashSideEffectsLater = VRPN_TRUE;
  *this = ((vrpn_Shared_String *) d_replica[whichReplica])->value();
  quashSideEffectsLater = VRPN_FALSE;

}

// Copy the state of the source replica to the destination replica.
void TclNet_string::copyFromToReplica (int sourceReplica, int destReplica)
{
  if ((sourceReplica < 0) || (sourceReplica >= d_numReplicas)) {
    fprintf(stderr, "TclNet_int::copyFromToReplica:  illegal id %d.\n",
            sourceReplica);
    return;  // error
  }
  if ((destReplica < 0) || (destReplica >= d_numReplicas)) {
    fprintf(stderr, "TclNet_int::copyFromToReplica:  illegal id %d.\n",
            destReplica);
    return;  // error
  }

  quashSideEffectsLater = VRPN_TRUE;
  if ( d_activeReplica == destReplica) {
      *this = ((vrpn_Shared_String *) d_replica[sourceReplica])->value();
  } else {
      timeval now;
      gettimeofday(&now, NULL);

      ((vrpn_Shared_String *) d_replica[destReplica])->set
      (((vrpn_Shared_String *) d_replica[sourceReplica])->value(), now);
  }
  quashSideEffectsLater = VRPN_FALSE;
}


// Copy the state of the which-th replica, and any changes to it.
// 0 is the default, local, built-in replica;  syncing to it stops
// receiving any network updates.
void TclNet_string::syncReplica (int whichReplica) {

//fprintf(stderr, "++ TclNet_string::syncReplica:  Synchronizing with %d.\n",
//whichReplica);

  if ((whichReplica < 0) || (whichReplica >= d_numReplicas)) {
    fprintf(stderr, "TclNet_string::syncReplica:  illegal id %d.\n",
            whichReplica);
    return;  // error
  }

  if (whichReplica == d_activeReplica) {
    return;  // noop
  }

  ((vrpn_Shared_String *) d_replica[d_activeReplica])->unregister_handler
             (propagateReceivedUpdate, this);

  d_activeReplica = whichReplica;

  d_writeReplica = whichReplica;

  ((vrpn_Shared_String *) d_replica[whichReplica])->register_handler
                 (propagateReceivedUpdate, this);

  copyReplica(whichReplica);
}

void TclNet_string::setReplica (int whichReplica, const char * value) {

  timeval now;

  if ((whichReplica < 0) || (whichReplica >= d_numReplicas)) {
    fprintf(stderr, "TclNet_string::setReplica:  illegal id %d.\n",
            whichReplica);
    return;  // error
  }

  gettimeofday(&now, NULL);

  ((vrpn_Shared_String *) d_replica[whichReplica])->set(value, now);
}





// static
// Callback registered on the active Remote replica.
// Executes (*(NetTcl_float *)userdata = newValue).
int TclNet_string::propagateReceivedUpdate (
    void * userdata,
    const char * newValue,
    timeval when,
    vrpn_bool isLocal)
{
  TclNet_string * nts;

  if (!userdata) {
    fprintf(stderr, "TclNet_string::propagateReceivedUpdate:  "
                    "NULL pointer.\n");
    return -1;
  }

  nts = (TclNet_string *) userdata;

//fprintf(stderr, "TclNet_string (%s)::propagateReceivedUpdate (%s) "
//"at %ld:%ld.\n", nts->d_myTclVarname, newValue, when.tv_sec, when.tv_usec);

//fprintf(stderr, "In TclNet_string::propagateReceivedUpdate:  "
//"set %s #%d to %d.\n", nti->my_tcl_varname, nti->d_activeReplica,
//nti->d_myfloat);


  // Ignore the next change to this variable generated by Tcl
  // if and only if updateTcl() is going to send an update
  // and Tcl is going to pay attention.
  // (updateTcl() does not send idempotent changes.  Tcl ignores
  // changes generated in the middle of a Trace.)

  if (!nts->d_updateFromTcl && interpreter) {

    // Since we're not inside a Tcl callback, we're about to generate one (if
    // an interpreter exists), which we want to ignore to avoid endless loops
    // or hitting the callback twice.

    collabVerbose(8, "TclNet_string::updateTcl(%s):  setting d_ignoreChange.\n",
                  nts->d_myTclVarname);
    nts->d_ignoreChange = VRPN_TRUE;
  }

  nts->Tclvar_string::operator = (newValue);
  nts->d_lastUpdate = when;
  if (interpreter) {
    // SetFromTcl() MUST come after operator = ().
    // This is where side effects (&callbacks) happen
    if (!isLocal || quashSideEffectsLater) {
      quashSideEffects = VRPN_TRUE;
    }
    nts->Tclvar_string::SetFromTcl(newValue);
    quashSideEffects = VRPN_FALSE;
  }


  return 0;
}


nmb_String * allocate_TclNet_string (const char * initialValue) {
  return new TclNet_string (initialValue);

}



