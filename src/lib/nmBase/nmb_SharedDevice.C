#include "nmb_SharedDevice.h"

nmb_SharedDevice::nmb_SharedDevice (const char * name, int portForMutexServer,
                      vrpn_Connection * connectionToDeviceServer,
                      const char * NICaddress) :
    nmb_Device_Client (name, connectionToDeviceServer),
    d_mutex (name, portForMutexServer, NICaddress) {

  d_mutex.addRequestGrantedCallback(this, handle_gotMutex);

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
  d_mutex.mainloop();

}

// virtual
nmb_SharedDevice::~nmb_SharedDevice (void) {


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

  if (d_mutex.isHeldLocally()) {
    return nmb_Device_Client::dispatchMessage(len, buf, type);
  }

fprintf(stderr, "nmb_SharedDevice:  throwing out a message\n"
"  (type %d, to %s) since we don't have the lock.\n", type, d_myName);

  return 0;
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
int nmb_SharedDevice::handle_gotMutex (void * userdata) {
  nmb_SharedDevice * me = (nmb_SharedDevice *) userdata;
  sharedDeviceCallback * cb;

fprintf(stderr, "nmb_SharedDevice:  got the lock on %s.\n", me->d_myName);

  for (cb = me->d_gotMutexCallbacks;  cb;  cb = cb->next) {
    (cb->f)(cb->userdata, me);
  }

  return 0;
}


