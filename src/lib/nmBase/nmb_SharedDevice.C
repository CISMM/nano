#include "nmb_SharedDevice.h"

nmb_SharedDevice::nmb_SharedDevice (const char * name, int portForMutexServer,
                      vrpn_Connection * connectionToDeviceServer,
                      const char * NICaddress) :
    nmb_Device_Client (name, connectionToDeviceServer),
    d_mutex (name, portForMutexServer, NICaddress),
    d_gotMutexCallbacks (NULL),
    d_deniedMutexCallbacks (NULL),
    d_mutexTakenCallbacks (NULL),
    d_mutexReleasedCallbacks (NULL)
{
  int i;

  for (i = 0; i < vrpn_CONNECTION_MAX_TYPES; i++) {
    d_typeSafe[i] = VRPN_FALSE;
  }

  d_mutex.addRequestGrantedCallback(this, handle_gotMutex);
  d_mutex.addRequestDeniedCallback(this, handle_deniedMutex);
  d_mutex.addTakeCallback(this, handle_mutexTaken);
  d_mutex.addReleaseCallback(this, handle_mutexReleased);

fprintf(stderr, "nmb_SharedDevice:  requesting the lock on %s.\n", d_myName);

  // We're currently running standalone;  grab the mutex.
  d_mutex.request();

  // XXX HACK
  // Call mainloop now to make sure we have the mutex before we return;
  // some of the code we're driving tries to send initialization commands/
  // queries before it ever calls mainloop, and we need to not quash those
  // when running standalone - they're being changed from commands to
  // queries by Aron/Alexandra so that they don't hurt when we get a
  // late joiner while running multiuser.  We could instead attach those
  // commands to a gotMutex callback...
  //d_mutex.mainloop();

}




// virtual
nmb_SharedDevice::~nmb_SharedDevice (void) {


}




vrpn_bool nmb_SharedDevice::typeIsSafe (vrpn_int32 type) const {
  if ((type < 0) || (type >= vrpn_CONNECTION_MAX_TYPES)) {
    return VRPN_FALSE;
  }
  return d_typeSafe[type];
}






int nmb_SharedDevice::mainloop (void) {
  d_mutex.mainloop();
  return 0;
}




void nmb_SharedDevice::requestMutex (void) {

fprintf(stderr, "nmb_SharedDevice:  requesting the lock on %s.\n", d_myName);

  d_mutex.request();

}

void nmb_SharedDevice::releaseMutex (void) {

fprintf(stderr, "nmb_SharedDevice:  releasing the lock on %s.\n", d_myName);

  d_mutex.release();

}




void nmb_SharedDevice::addPeer (const char * stationName) {

fprintf(stderr, "nmb_SharedDevice:  releasing the lock on %s "
"to add a peer named %s.\n", d_myName, stationName);

  d_mutex.release();
  d_mutex.addPeer(stationName);

}





// virtual
long nmb_SharedDevice::dispatchMessage (long len, const char * buf,
                                        vrpn_int32 type) {

  if (d_mutex.isHeldLocally() || typeIsSafe(type)) {
    return nmb_Device_Client::dispatchMessage(len, buf, type);
  }

fprintf(stderr, "nmb_SharedDevice:  throwing out a message\n"
"  (type %d, to %s) since we don't have the lock.\n", type, d_myName);

  return 0;
}





void nmb_SharedDevice::markTypeAsSafe (vrpn_int32 type) {
  if ((type < 0) || (type >= vrpn_CONNECTION_MAX_TYPES)) {
    fprintf(stderr, "nmb_SharedDevice::markTypeAsSafe:  illegal type %d.\n",
            type);
    return;
  }
  d_typeSafe[type] = VRPN_TRUE;
}




void nmb_SharedDevice::registerGotMutexCallback
            (void * userdata,
             void (* f) (void *, nmb_SharedDevice *)) {
  sharedDeviceCallback * cb;

  cb = new sharedDeviceCallback;
  if (!cb) {
    fprintf(stderr, "nmb_SharedDevice::registerGotMutexCallback:  "
                    "Out of memory.\n");
    return;
  }

  cb->f = f;
  cb->userdata = userdata;
  cb->next = d_gotMutexCallbacks;
  d_gotMutexCallbacks = cb;

}

// static
void nmb_SharedDevice::registerDeniedMutexCallback
            (void * userdata,
             void (* f) (void *, nmb_SharedDevice *)) {
  sharedDeviceCallback * cb;

  cb = new sharedDeviceCallback;
  if (!cb) {
    fprintf(stderr, "nmb_SharedDevice::registerDeniedMutexCallback:  "
                    "Out of memory.\n");
    return;
  }

  cb->f = f;
  cb->userdata = userdata;
  cb->next = d_deniedMutexCallbacks;
  d_deniedMutexCallbacks = cb;

}

// static
void nmb_SharedDevice::registerMutexTakenCallback
            (void * userdata,
             void (* f) (void *, nmb_SharedDevice *)) {
  sharedDeviceCallback * cb;

  cb = new sharedDeviceCallback;
  if (!cb) {
    fprintf(stderr, "nmb_SharedDevice::registerMutexTakenCallback:  "
                    "Out of memory.\n");
    return;
  }

  cb->f = f;
  cb->userdata = userdata;
  cb->next = d_mutexTakenCallbacks;
  d_mutexTakenCallbacks = cb;

}

// static
void nmb_SharedDevice::registerMutexReleasedCallback
            (void * userdata,
             void (* f) (void *, nmb_SharedDevice *)) {
  sharedDeviceCallback * cb;

  cb = new sharedDeviceCallback;
  if (!cb) {
    fprintf(stderr, "nmb_SharedDevice::registerMutexReleasedCallback:  "
                    "Out of memory.\n");
    return;
  }

  cb->f = f;
  cb->userdata = userdata;
  cb->next = d_mutexReleasedCallbacks;
  d_mutexReleasedCallbacks = cb;

}

// static
int nmb_SharedDevice::handle_gotMutex (void * userdata) {
  nmb_SharedDevice * me = (nmb_SharedDevice *) userdata;
  sharedDeviceCallback * cb;

fprintf(stderr, "nmb_SharedDevice:  got the lock on %s.\n", me->d_myName);

  for (cb = me->d_gotMutexCallbacks;  cb;  cb = cb->next) {
    (cb->f)(cb->userdata, me);
  }

  return 0;
}


// static
int nmb_SharedDevice::handle_deniedMutex (void * userdata) {
  nmb_SharedDevice * me = (nmb_SharedDevice *) userdata;
  sharedDeviceCallback * cb;

fprintf(stderr, "nmb_SharedDevice:  denied the lock on %s.\n", me->d_myName);

  for (cb = me->d_deniedMutexCallbacks;  cb;  cb = cb->next) {
    (cb->f)(cb->userdata, me);
  }

  return 0;
}


// static
int nmb_SharedDevice::handle_mutexTaken (void * userdata) {
  nmb_SharedDevice * me = (nmb_SharedDevice *) userdata;
  sharedDeviceCallback * cb;

fprintf(stderr, "nmb_SharedDevice:  somebody got %s.\n", me->d_myName);

  for (cb = me->d_mutexTakenCallbacks;  cb;  cb = cb->next) {
    (cb->f)(cb->userdata, me);
  }

  return 0;
}


// static
int nmb_SharedDevice::handle_mutexReleased (void * userdata) {
  nmb_SharedDevice * me = (nmb_SharedDevice *) userdata;
  sharedDeviceCallback * cb;

fprintf(stderr, "nmb_SharedDevice:  somebody released %s.\n", me->d_myName);

  for (cb = me->d_mutexReleasedCallbacks;  cb;  cb = cb->next) {
    (cb->f)(cb->userdata, me);
  }

  return 0;
}


