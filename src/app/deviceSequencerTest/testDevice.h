#ifndef TESTDEVICE_H
#define TESTDEVICE_H

#include "nmb_Device.h"

class TestDevice
{
  public:
    TestDevice(const char *name, vrpn_Connection *c = NULL);
    ~TestDevice();
    virtual long mainloop() = 0;
    vrpn_int32 getPeriodicDataMessageType();

  protected:
    vrpn_int32 d_RequestModify_type; // sent from client to server
    vrpn_int32 d_ModifyResult_type;  // sent in response by server
    vrpn_int32 d_PeriodicData_type;  // server sends this periodically
    
    static char *encode_Text(long *len, const char *msg);
    static vrpn_int32 decode_Text(const char **buf, char **msg);
};

class TestDeviceServer: public nmb_Device_Server, public TestDevice {
  public:
    TestDeviceServer ( const char *name, vrpn_Connection *c = NULL);
    virtual ~TestDeviceServer ();
    virtual long mainloop(void);

    int sendData(const char *message);
    int sendModifyResult(const char *message);

  protected:

    static int handle_RequestModify(void *ud, vrpn_HANDLERPARAM param);
    vrpn_bool d_timeInit;
    struct timeval d_nextTimeToSend;
};

class TestDeviceClient : public nmb_Device_Client, public TestDevice{
  public:
    TestDeviceClient ( const char *name, vrpn_Connection *c = NULL);
    virtual ~TestDeviceClient ();
    virtual long mainloop(void);

    int sendRequestModify(const char *msg);

  protected:

    static int handle_ModifyResult(void *ud, vrpn_HANDLERPARAM param);
    static int handle_PeriodicData(void *ud, vrpn_HANDLERPARAM param);
};

#endif
