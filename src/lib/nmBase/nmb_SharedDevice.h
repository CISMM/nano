#ifndef NMB_SHARED_DEVICE_H
#define NMB_SHARED_DEVICE_H

#include "nmb_Device.h"

#include <vrpn_Mutex.h>
//class vrpn_Mutex;  // from <vrpn_Mutex.h>

/// nmb_SharedDevice
///
///   A subclass of nmb_Device_Client that uses a vrpn_Mutex to ensure that
/// only one instance of the class is sending commands to the Device_Server
/// at any one time.  Calls to dispatchMessage() at an instance which
/// doesn't have the mutex are ignored.

// This version assumes vrpn_Mutex 1.4 and has a couple of workarounds in
// place:
//   We start up with a mutex and request() it immediately.  Since there
// are no peers, we have it, so it doesn't interfere with standalone work.
// This means we have to call mainloop() on the mutex.
//   Before we call vrpn_Mutex::addPeer(), we release() the mutex.

// At some future time we'll want to extend (or further subclass) to
// make the mutex only restrict a subset of message types sent.

class nmb_SharedDevice : public nmb_Device_Client {

  public:

    nmb_SharedDevice (const char * name, int portForMutexServer,
                      vrpn_Connection * connectionToDeviceServer = NULL,
                      const char * mutexNICaddress = NULL);
    virtual ~nmb_SharedDevice (void);

    virtual int mainloop (void);
      ///< MUST be called by subclasses!

    void requestMutex (void);
    void releaseMutex (void);
    void addPeer (const char * stationName);
      ///< Takes a vrpn station name, of the form "<host>:<port>".

    virtual long dispatchMessage (long len, const char * buf, vrpn_int32 type);
      ///< If we don't have the mutex, returns 0;  otherwise, passes the
      ///< message on to nmb_Device_Client::dispatchMessage() to be buffered
      ///< or sent as appropriate.

    void registerGotMutexCallback (void * userdata,
                                   void (*) (void *, nmb_SharedDevice *));
      ///< Lets the using process take some action to notify the user
      ///< when the lock is secured.

  private:

    vrpn_Mutex d_mutex;

    struct sharedDeviceCallback {
      void (* f) (void *, nmb_SharedDevice *);
      void * userdata;
      sharedDeviceCallback * next;
    };

    sharedDeviceCallback * d_gotMutexCallbacks;

    static int handle_gotMutex (void *);

};

#endif  // NMB_SHARED_DEVICE_H

