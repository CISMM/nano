#ifndef NMB_SHARED_DEVICE_H
#define NMB_SHARED_DEVICE_H

#include "nmb_Device.h"

#include <vrpn_Mutex.h>
#include <vrpn_Connection.h>

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


    //  ACCESSORS


    vrpn_bool haveMutex (void) const;
      ///< Returns VRPN_TRUE if we have exclusive control of
      ///< the shared device.

    vrpn_bool typeIsSafe (vrpn_int32 type) const;
      ///< See markTypeAsSafe() below.  Returns VRPN_FALSE on
      ///< illegal input.


    // MANIPULATORS


    virtual int mainloop (void);
      ///< MUST be called by subclasses!


    void requestMutex (void);
      ///< Request exclusive access to the shared device.

    void releaseMutex (void);
      ///< Release our exclusive access to the shared device.


    void addPeer (const char * stationName);
      ///< Takes a vrpn station name, of the form "<host>:<port>".


    virtual long dispatchMessage (long len, const char * buf, vrpn_int32 type);
      ///< If we don't have the mutex, returns 0;  otherwise, passes the
      ///< message on to nmb_Device_Client::dispatchMessage() to be buffered
      ///< or sent as appropriate.


    void markTypeAsSafe (vrpn_int32 type);
      ///< Messages of the given type will be dispatched even when we don't
      ///< have the mutex.  They must be queries without side-effects.

    void registerGotMutexCallback (void * userdata,
                                   void (*) (void *, nmb_SharedDevice *));
      ///< Lets the using process take some action to notify the user
      ///< when the device is secured.
    void registerDeniedMutexCallback (void * userdata,
                                   void (*) (void *, nmb_SharedDevice *));
      ///< Lets the using process take some action to notify the user
      ///< when a request for the device has been denied.
    void registerMutexTakenCallback (void * userdata,
                                   void (*) (void *, nmb_SharedDevice *));
      ///< Lets the using process take some action to notify the user
      ///< when any user obtains access to the device.
    void registerMutexReleasedCallback (void * userdata,
                                   void (*) (void *, nmb_SharedDevice *));
      ///< Lets the using process take some action to notify the user
      ///< when any user releases the device.

  private:

    vrpn_Mutex d_mutex;

    // HACK - we use a constant-sized array here and assume the implementation
    // in vrpn_Connection is still a constant-sized array.
    vrpn_bool d_typeSafe [vrpn_CONNECTION_MAX_TYPES];

    struct sharedDeviceCallback {
      void (* f) (void *, nmb_SharedDevice *);
      void * userdata;
      sharedDeviceCallback * next;
    };

    sharedDeviceCallback * d_gotMutexCallbacks;
    sharedDeviceCallback * d_deniedMutexCallbacks;
    sharedDeviceCallback * d_mutexTakenCallbacks;
    sharedDeviceCallback * d_mutexReleasedCallbacks;

    static int handle_gotMutex (void *);
    static int handle_deniedMutex (void *);
    static int handle_mutexTaken (void *);
    static int handle_mutexReleased (void *);
};

#endif  // NMB_SHARED_DEVICE_H

