#ifndef NMB_DEVICE_H
#define NMB_DEVICE_H

class vrpn_RedundantTransmission;  // from vrpn_RedundantTransmission.h

#include <vrpn_FileController.h>
#include <vrpn_Connection.h>
#include <vrpn_Shared.h>
#include <thread.h>

// auxiliary class for callback info
class nmb_SynchMessage {
  public:
    vrpn_int32 operation_id;
    vrpn_int32 synch_id;
    char *comment;
};

/** nmb_Device
    This is the base class for nmb_Device_Client and nmb_Device_Server.
    It defines the interface which is used to detect completion of 
    commands for any derived classes. This interface makes it possible to
    script a well-defined order of commands execution and data acquisition
    among multiple nmb_Device_Client-derived devices.

    Any device which can affect recorded data indirectly by executing commands
    or which produces recorded data would be an appropriate derived class from
    nmb_Device_Client/nmb_Device_Server.

    Some other ideas for things we want in a device base class:
        listing available acquisition channels
        getting the latest values for each available channel
        ability to register a callback to be called when new data arrives for
           each acquisition channel

    Added 2000:  manages a vrpn_RedundantTransmission in addition to the
      vrpn_Connection to handle forward error correction (FEC).

    8/15/2001 (AAS):
    Added mutex for client buffer so that messages can be
      enqueued and dequeued in different threads
      Why might that be useful?: We normally assume in our programs that each
      command received by the server is executed in the order it is received
      but with a server and client running in different threads within the
      same program and communicating through the same connection object
      it is a little more difficult to enforce this. An easy
      solution is to use this synchronized queue inside the client and then
      to call the client's sendBuffer() command inside the mainloop for the 
      server thread. In this case client messages will be dispatched
      immediately and executed when we use the connection's dispatch function
      so instead we can just buffer them inside the client thread and then
      deliver them inside the server thread.
      Then I moved the buffer code into the base class. 
*/

class nmb_Device {
  public:
    nmb_Device (const char *name, vrpn_Connection * = NULL);
    virtual ~nmb_Device (void);
 
    vrpn_Connection *getConnection();

    vrpn_bool connected();

    static char * encode_RequestSynchronization (long *len,
           vrpn_int32 operation_id, vrpn_int32 synch_id, 
           const char *comment);
    static long decode_RequestSynchronization (const char ** buf,
           vrpn_int32 *operation_id, vrpn_int32 *synch_id,
           char **comment);

    static char * encode_Synchronization (long *len,
           vrpn_int32 operation_id, vrpn_int32 synch_id,
           const char *comment);
    static long decode_Synchronization (const char ** buf,
           vrpn_int32 *operation_id, vrpn_int32 *synch_id,
           char **comment);

    // (this allows us to control precisely when
    // we can send messages) - this depends on the device
    // using dispatchMessage() for all messages for which control is desired
    virtual long sendBuffer(); // send all messages accumulated since the
                        // last call to sendBuffer() if buffer is enabled
    virtual void setBufferEnable(vrpn_bool useBuffer); // if set to false then
                              // dispatchMessage() will send immediately
                              // otherwise messages will be buffered until
                              // sendBuffer() is called
    virtual vrpn_bool getBufferEnable();
    virtual long numBufferedMessages(); // get how many messages are in buffer
    virtual long clearBuffer(); // delete messages without sending them

    virtual long dispatchMessage (long len, const char * buf, vrpn_int32 type);
    virtual long dispatchRedundantMessage (long len, const char * buf,
                                           vrpn_int32 type);

  protected:
    // message ids
    vrpn_int32 d_RequestSynchronization_type;
    vrpn_int32 d_Synchronization_type;

    vrpn_int32 d_GotFirstConnection_type;
    vrpn_int32 d_GotConnection_type;
    vrpn_int32 d_DroppedConnection_type;
    vrpn_int32 d_DroppedLastConnection_type;

    vrpn_int32 d_myId;
    char * d_myName;
    vrpn_Connection * d_connection;
    vrpn_File_Controller * d_fileController;   

    vrpn_bool d_connected;

    // TCH network adaptations Nov 2000
    vrpn_RedundantTransmission * d_redundancy;

    static int handle_GotFirstConnection(void *ud, vrpn_HANDLERPARAM p);
    static int handle_GotConnection(void *ud, vrpn_HANDLERPARAM p);
    static int handle_DroppedConnection(void *ud, vrpn_HANDLERPARAM p);
    static int handle_DroppedLastConnection(void *ud, vrpn_HANDLERPARAM p);

    virtual long dispatchMessageNoBuffer (long len, const char * buf, 
                                          vrpn_int32 type);
    virtual long dispatchRedundantMessageNoBuffer (long len, const char * buf,
                                           vrpn_int32 type);

    vrpn_bool d_useBuffer;

    struct messageBufferEntry {
      long len;
      char *buf;
      vrpn_int32 type;
      messageBufferEntry * next;
      vrpn_bool isRedundant;
    };

    Semaphore d_bufferSemaphore;
    messageBufferEntry * d_messageBufferHead;
    messageBufferEntry * d_messageBufferTail;
    int d_numBufferedMessages;

    int bufferMessage (long len, const char * buf, vrpn_int32 type);
      ///< Returns nonzero on failure.
};

class nmb_Device_Client : public nmb_Device {
  public:
    nmb_Device_Client(const char *name, vrpn_Connection * = NULL);
    virtual ~nmb_Device_Client (void);

    // have device tell us when it is done with all previous commands and
    // echo the id and comment
    long requestSynchronization(vrpn_int32 operation_id, 
                                vrpn_int32 synch_id,
                                const char *comment);

    long registerSynchHandler (int (* handler) (void *,
                                        const nmb_SynchMessage *),
                                  void *userdata);
    long unregisterSynchHandler (int (* handler) (void *,
                                        const nmb_SynchMessage *),
                                  void *userdata);

  protected:
    static int handle_Synchronization(void *ud, vrpn_HANDLERPARAM p);

    // callback structures
    struct synchHandlerEntry {
      int (* handler) (void *, const nmb_SynchMessage *);
      void *userdata;
      synchHandlerEntry * next;
    };

    synchHandlerEntry * d_synchHandlers;

    void doSynchCallbacks (const nmb_SynchMessage *);
};

class nmb_Device_Server : public nmb_Device {
  public:
    nmb_Device_Server(const char *name, vrpn_Connection * = NULL);
    virtual ~nmb_Device_Server (void);

  protected:
    // callback to echo synch request messages
    static int handle_RequestSynchronization(void *ud,
                                                    vrpn_HANDLERPARAM p);
};

#endif
