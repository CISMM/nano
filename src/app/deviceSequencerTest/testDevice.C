#include "testDevice.h"


TestDevice::TestDevice ( const char *name, vrpn_Connection *c)
{
   if (c) {
      d_RequestModify_type =
            c->register_message_type("TestDevice RequestModify");
      d_ModifyResult_type = 
            c->register_message_type("TestDevice ModifyResult");
      d_PeriodicData_type = 
            c->register_message_type("TestDevice PeriodicData");
   } else {
      printf("TestDevice::TestDevice: Warning, connection is NULL\n");
   }
}

TestDevice::~TestDevice()
{

}

vrpn_int32 TestDevice::getPeriodicDataMessageType()
{
    return d_PeriodicData_type;
}

char *TestDevice::encode_Text(long *len, const char *msg)
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  vrpn_int32 msg_length = 0;
  if (msg) {
    msg_length += strlen(msg) + 1;
  }
  mlen = msg_length + sizeof(vrpn_int32);
  *len = mlen;

  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "TestDevice::encode_Text:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, msg_length);
    if (msg_length > 0) {
        vrpn_buffer(&mptr, &mlen, msg, msg_length);
    }
  }

  return msgbuf;
}

vrpn_int32 TestDevice::decode_Text(const char **buf, char **msg)
{
  vrpn_int32 msg_length;
  if (vrpn_unbuffer(buf, &msg_length)) return -1;
  if (msg_length > 0) {
    *msg = new char [msg_length];
    if (*msg) {
      if (vrpn_unbuffer(buf, *msg, msg_length)) return -1;
    } else {
      fprintf(stderr, "TestDevice::decode_Text: "
             "Error, out of memory\n");
      return -1;
    }
  } else {
    *msg = NULL;
  }

  return 0;
}

TestDeviceServer::TestDeviceServer ( const char *name, vrpn_Connection *c) :
   nmb_Device_Server(name, c),
   TestDevice(name, c),
   d_timeInit(vrpn_FALSE)
{
   if (d_connection) {
      d_connection->register_handler(
                               d_RequestModify_type,
                               handle_RequestModify,
                               (void *)this);
   }
}

TestDeviceServer::~TestDeviceServer()
{
   if (d_connection){
      d_connection->unregister_handler(
                               d_RequestModify_type,
                               handle_RequestModify,
                               (void *)this);
   }
}

long TestDeviceServer::mainloop()
{
  struct timeval now;
  gettimeofday(&now, NULL);

  char message[128];

  if (!d_timeInit || vrpn_TimevalGreater(now, d_nextTimeToSend)) {
      if (connected()) {
          sprintf(message, "%s: periodic data", d_myName);
          sendData(message);
      }
      if (!d_timeInit) {
          d_nextTimeToSend = now;
          d_nextTimeToSend.tv_sec++;
          d_timeInit = vrpn_TRUE;
      } else {
          d_nextTimeToSend.tv_sec++;
      }
  }
  return 0;
}

int TestDeviceServer::sendData(const char *msg)
{
  char * msgbuf;
  long len;

  msgbuf = encode_Text(&len, msg);
  if (!msgbuf){
    return -1;
  }
  return dispatchMessage(len, msgbuf, 
                              d_PeriodicData_type);
}

int TestDeviceServer::sendModifyResult(const char *msg)
{
  char * msgbuf;
  long len;

  msgbuf = encode_Text(&len, msg);
  if (!msgbuf){
    return -1;
  }
  return dispatchMessage(len, msgbuf, 
                              d_ModifyResult_type);
}

//static
int TestDeviceServer::handle_RequestModify(void *ud, vrpn_HANDLERPARAM param)
{
  TestDeviceServer *me = (TestDeviceServer *)ud;

  char *msg;

  if (decode_Text(&param.buffer, &msg)){
    return -1;
  }

  printf("got request: %s; sending response\n", msg);
  char message[256];
  sprintf(message, "%s: %s", me->d_myName, msg);
  me->sendModifyResult(message);

  if (msg) delete [] msg;

  return 0;
}

TestDeviceClient::TestDeviceClient ( const char *name, vrpn_Connection *c) :
     nmb_Device_Client(name, c?c:vrpn_get_connection_by_name(name)),
     TestDevice(name, d_connection)
{
  if (d_connection) {
      d_connection->register_handler(
                         d_ModifyResult_type,
                         handle_ModifyResult,
                         (void *)this);
      d_connection->register_handler(
                         d_PeriodicData_type,
                         handle_PeriodicData,
                         (void *)this);
  }
}

TestDeviceClient::~TestDeviceClient()
{
   if (d_connection){
      d_connection->unregister_handler(
                         d_ModifyResult_type,
                         handle_ModifyResult,
                         (void *)this);
      d_connection->unregister_handler(
                         d_PeriodicData_type,
                         handle_PeriodicData,
                         (void *)this);

   }
}

// static
int TestDeviceClient::handle_ModifyResult(void *ud, vrpn_HANDLERPARAM param)
{
  TestDeviceClient *me = (TestDeviceClient *)ud;

  char *msg;

  if (decode_Text(&param.buffer, &msg)){
    return -1;
  }

  printf("got response: %s\n", msg);

  if (msg) delete [] msg;

  return 0;
}

// static
int TestDeviceClient::handle_PeriodicData(void *ud, vrpn_HANDLERPARAM param)
{
  TestDeviceClient *me = (TestDeviceClient *)ud;

  char *msg;

  if (decode_Text(&param.buffer, &msg)){
    return -1;
  }

  //printf("got periodic data: %s\n", msg);

  if (msg) delete [] msg;

  return 0;
}


int TestDeviceClient::sendRequestModify(const char *msg)
{
  char * msgbuf;
  long len;

  msgbuf = encode_Text(&len, msg);
  if (!msgbuf){
    return -1;
  }
  return dispatchMessage(len, msgbuf, 
                          d_RequestModify_type);
}

long TestDeviceClient::mainloop()
{
  if (d_connection){
    d_connection->mainloop();
  }
  return 0;
}
