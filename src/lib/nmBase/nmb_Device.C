#include <assert.h>
#include <string.h>
#if (!defined(_WIN32) || defined(__CYGWIN__))
#include <strings.h>
#endif

#include "nmb_Device.h"

char * nmb_Device::encode_RequestSynchronization (long *len,
          vrpn_int32 operation_id, vrpn_int32 synch_id,
          const char *comment)
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  vrpn_int32 comment_length = 0;
  if (comment) {
    comment_length = strlen(comment) + 1;
  }
  *len = 3l*(long)sizeof(vrpn_int32) + comment_length;

  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmb_Device::encode_Synchronization:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, operation_id);
    vrpn_buffer(&mptr, &mlen, synch_id);
    vrpn_buffer(&mptr, &mlen, comment_length);
    if (comment_length > 0) {
        vrpn_buffer(&mptr, &mlen, comment, comment_length);
    }
  }

  return msgbuf;
}

long nmb_Device::decode_RequestSynchronization (
                                const char ** buf,
                                vrpn_int32 *operation_id,
                                vrpn_int32 *synch_id,
                                char **comment)
{
  vrpn_int32 comment_length;
  if (vrpn_unbuffer(buf, operation_id) ||
      vrpn_unbuffer(buf, synch_id) ||
      vrpn_unbuffer(buf, &comment_length)) return -1;
  if (comment_length > 0) {
    *comment = new char [comment_length];
    if (*comment) {
      if (vrpn_unbuffer(buf, *comment, comment_length)) return -1;
    } else {
      fprintf(stderr,
             "nmb_Device::decode_Synchronization: "
             "Error, out of memory\n");
      return -1;
    }
  } else {
    *comment = NULL;
  }

  return 0;
}

char * nmb_Device::encode_Synchronization (long *len,
           vrpn_int32 operation_id, vrpn_int32 synch_id, const char *comment)
{
    return encode_RequestSynchronization (len, operation_id, synch_id, comment);
}

long nmb_Device::decode_Synchronization (const char ** buf,
           vrpn_int32 *operation_id,
           vrpn_int32 *synch_id, char **comment)
{
    return decode_RequestSynchronization (buf, operation_id, synch_id, comment);
}

nmb_Device::nmb_Device(const char * name,
                       vrpn_Connection * connection) :
                       d_connection (connection),
                       d_fileController (new vrpn_File_Controller (connection)),
                       d_connected(VRPN_FALSE)
{
  char * servicename = NULL;
  servicename = vrpn_copy_service_name(name);

  d_myName = new char[strlen(name)+1];
  strcpy(d_myName, name);

  if (d_connection) {
    d_connected = d_connection->connected();
    d_GotFirstConnection_type = d_connection->register_message_type
        (vrpn_got_first_connection);
    d_GotConnection_type = d_connection->register_message_type
        (vrpn_got_connection);
    d_DroppedConnection_type = d_connection->register_message_type
        (vrpn_dropped_connection);
    d_DroppedLastConnection_type = d_connection->register_message_type
        (vrpn_dropped_last_connection);
    d_myId = connection->register_sender(servicename);

    d_RequestSynchronization_type = connection->register_message_type
        ("nmb Device RequestSynchronization");
    d_Synchronization_type = connection->register_message_type
        ("nmb Device Synchronization");

    d_connection->register_handler(d_GotFirstConnection_type,
                                   handle_GotFirstConnection,
                                   this);
    d_connection->register_handler(d_GotConnection_type,
                                   handle_GotConnection,
                                   this);
    d_connection->register_handler(d_DroppedConnection_type,
                                   handle_DroppedConnection,
                                   this);
    d_connection->register_handler(d_DroppedLastConnection_type,
                                   handle_DroppedLastConnection,
                                   this);
    // XXX Bug in VRPN. If we're already connected before we register the
    // handle_*GotConnection handlers, they never get executed. So we'll call
    // them explicitly
    if (d_connected) {
        vrpn_HANDLERPARAM p;
        p.buffer=NULL;
        handle_GotFirstConnection(this, p);
        handle_GotConnection(this, p);
    }

  }
  if (servicename) {
    delete [] servicename;
  }
}

nmb_Device::~nmb_Device (void) {
  if (d_connection) {
    d_connection->unregister_handler(d_GotFirstConnection_type,
                                   handle_GotFirstConnection,
                                   this);
    d_connection->unregister_handler(d_GotConnection_type,
                                   handle_GotConnection,
                                   this);
    d_connection->unregister_handler(d_DroppedConnection_type,
                                   handle_DroppedConnection,
                                   this);
    d_connection->unregister_handler(d_DroppedLastConnection_type,
                                   handle_DroppedLastConnection,
                                   this);
  }
}

vrpn_Connection *nmb_Device::getConnection()
{
    return d_connection;
}

long nmb_Device::dispatchMessage (long len, const char * buf,
                                      vrpn_int32 type)
{
  struct timeval now;
  long retval = 0;

  gettimeofday(&now, NULL);
  // If we aren't connected to anything, just pretend we sent the message
  // Useful if we are viewing a file
  if (d_connection) {
      retval = d_connection->pack_message(len, now, type, d_myId,
                                      (char *) buf, vrpn_CONNECTION_RELIABLE);
  } else {
      retval = 0;
  }
  if (len > 0) {
      delete [] ((char *) buf);
  }
  return retval;
}

vrpn_bool nmb_Device::connected()
{
    if (d_connected && !d_connection->connected()){
        printf("warning: connection not connected but server thinks so\n");
    }
    return d_connected;
}

// static
int nmb_Device::handle_GotFirstConnection(void *userdata,
                                          vrpn_HANDLERPARAM /*param*/)
{
  nmb_Device *me = (nmb_Device *) userdata;
  me->d_connected = vrpn_TRUE;
  return 0;
}

// static
int nmb_Device::handle_GotConnection(void *userdata,
                                     vrpn_HANDLERPARAM /*param*/)
{
  nmb_Device *me = (nmb_Device *) userdata;
  me->d_connected = vrpn_TRUE;
  return 0;
}

// static
int nmb_Device::handle_DroppedConnection(void *userdata,
                                         vrpn_HANDLERPARAM /*param*/)
{
  nmb_Device *me = (nmb_Device *) userdata;
  me->d_connected = vrpn_FALSE;
  return 0;
}


// static
int nmb_Device::handle_DroppedLastConnection(void *userdata,
                                             vrpn_HANDLERPARAM /*param*/)
{
  nmb_Device *me = (nmb_Device *) userdata;
  me->d_connected = vrpn_FALSE;
  return 0;
}

nmb_Device_Client::nmb_Device_Client(const char * name,
                       vrpn_Connection * connection) :
                       nmb_Device(name, connection),
                       d_useBuffer(VRPN_FALSE),
                       d_synchHandlers(NULL),
                       d_messageBufferHead(NULL),
                       d_messageBufferTail(NULL)
{
  if (d_connection) {
    d_connection->register_handler(d_Synchronization_type,
                                   handle_Synchronization,
                                   this);
  }
}

nmb_Device_Client::~nmb_Device_Client (void) {
  if (d_connection) {
    d_connection->unregister_handler(d_Synchronization_type,
                                     handle_Synchronization,
                                     this);
  }
}


long nmb_Device_Client::requestSynchronization(
             vrpn_int32 operation_id, vrpn_int32 synch_id, const char *comment)
{
  char * msgbuf;
  long len;

  msgbuf = encode_RequestSynchronization(&len, operation_id,
          synch_id, comment);
  if (!msgbuf){
    return -1;
  }
  return dispatchMessage(len, msgbuf, d_RequestSynchronization_type);
}

long nmb_Device_Client::registerSynchHandler (int (* handler) (void *,
                                        const nmb_SynchMessage *),
                                  void *userdata)
{
  synchHandlerEntry * newEntry;

  newEntry = new synchHandlerEntry;
  if (!newEntry) {
    fprintf(stderr, "nmb_Device_Client::registerSynchHandler:  "
                    "Out of memory.\n");
    return -1;
  }

  newEntry->handler = handler;
  newEntry->userdata = userdata;
  newEntry->next = d_synchHandlers;

  d_synchHandlers = newEntry;

  return 0;
}

long nmb_Device_Client::unregisterSynchHandler (int (* handler) (void *,
                                        const nmb_SynchMessage *),
                                  void *userdata)
{
  synchHandlerEntry * victim, ** snitch;

  snitch = &d_synchHandlers;
  victim = *snitch;
  while (victim &&
         (victim->handler != handler) &&
         (victim->userdata != userdata)) {
    snitch = &((*snitch)->next);
    victim = *snitch;
  }

  if (!victim) {
    fprintf(stderr, "nmb_Device_Client::unregisterSynchHandler:  "
                    "No such handler.\n");
    return -1;
  }

  *snitch = victim->next;
  delete victim;

  return 0;
}

long nmb_Device_Client::sendBuffer()
{
  long retval = 0;

  messageBufferEntry *m = d_messageBufferHead;
  while (m) {
    if (!retval) {
      retval = nmb_Device::dispatchMessage(m->len, m->buf, m->type);
    }
    else {
      fprintf(stderr, "Warning: sendBuffer failure: throwing away messages\n");
    }
    d_messageBufferHead = m->next;
    delete m;
    m = d_messageBufferHead;
  }
  // we've deleted the whole buffer now so tail should be NULL
  d_messageBufferTail = NULL;
  return retval;
}

void nmb_Device_Client::setBufferEnable(vrpn_bool useBuffer)
{
  d_useBuffer = useBuffer;
}

vrpn_bool nmb_Device_Client::getBufferEnable()
{
  return d_useBuffer;
}

long nmb_Device_Client::dispatchMessage (long len, const char * buf,
                                      vrpn_int32 type)
{
  long retval = 0;

  if (!d_useBuffer) {
    return nmb_Device::dispatchMessage(len, buf, type);
  } else {
    if (!d_messageBufferHead) {
      d_messageBufferHead = new messageBufferEntry;
      d_messageBufferTail = d_messageBufferHead;
    } else {
      assert(d_messageBufferTail != NULL);
      d_messageBufferTail->next = new messageBufferEntry;
      if (!(d_messageBufferTail->next)) {
          fprintf(stderr, "nmb_Device_Client::dispatchMessage: Error, "
            "out of memory\n");
          return -1;
      }
      d_messageBufferTail = d_messageBufferTail->next;
    }
    d_messageBufferTail->len = len;
    d_messageBufferTail->buf = (char *) buf;
    d_messageBufferTail->type = type;
    d_messageBufferTail->next = NULL;
  }
  return retval;
}


// static
int nmb_Device_Client::handle_Synchronization(void *userdata,
                                           vrpn_HANDLERPARAM param)
{
  nmb_Device_Client *me = (nmb_Device_Client *) userdata;

  //printf("in nmb_Device_Client::handle_Synchronization\n");

  vrpn_int32 operation_id;
  vrpn_int32 synch_id;
  char *comment;

  if (decode_Synchronization (&param.buffer, &operation_id,
                                                  &synch_id, &comment)){
    return -1;
  }
  nmb_SynchMessage sm;
  sm.operation_id = operation_id;
  sm.synch_id = synch_id;
  sm.comment = comment;

  me->doSynchCallbacks(&sm);

  // comment may be allocated in decode_Synchronization()
  if (comment) delete [] comment;

  return 0;
}

void nmb_Device_Client::doSynchCallbacks (const nmb_SynchMessage *sm)
{
  synchHandlerEntry * l;

  l = d_synchHandlers;
  while (l) {
    if ((l->handler)(l->userdata, sm)) {
      fprintf(stderr, "nmb_Device_Client::doSynchCallbacks:  "
                      "Nonzero return value.\n");
      return;
    }
    l = l->next;
  }
}

nmb_Device_Server::nmb_Device_Server(const char * name,
                       vrpn_Connection * connection) :
                       nmb_Device(name, connection)
{
  if (d_connection) {
    d_connection->register_handler(d_RequestSynchronization_type,
                                   handle_RequestSynchronization,
                                   this);
  }
}

nmb_Device_Server::~nmb_Device_Server (void) {
  if (d_connection) {
    d_connection->unregister_handler(d_RequestSynchronization_type,
                                     handle_RequestSynchronization,
                                     this);
  }
}

// static
int nmb_Device_Server::handle_RequestSynchronization(void *userdata,
                                       vrpn_HANDLERPARAM param)
{
  nmb_Device_Server *me = (nmb_Device_Server *) userdata;

//  printf("in nmb_Device_Server::handle_RequestSynchronization\n");
  // just echo the message back to the client
  char *msgbuf;
  long len;

  vrpn_int32 synch_id;
  vrpn_int32 operation_id;
  char *comment;

  if (decode_RequestSynchronization (&param.buffer, &operation_id,
                                        &synch_id, &comment)){
    return -1;
  }

  msgbuf = encode_Synchronization(&len, operation_id, synch_id, comment);
  // comment may be allocated in decode_RequestSynchronization
  if (comment) delete [] comment;

  return me->dispatchMessage(len, msgbuf, me->d_Synchronization_type);
}

