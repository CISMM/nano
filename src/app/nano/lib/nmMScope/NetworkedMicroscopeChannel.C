#include "NetworkedMicroscopeChannel.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#if defined(sgi) || defined(hpux)
#include <strings.h>  // bcopy(), bzero() ON SGI and HP
#endif
#include <stdlib.h>  // bcopy(), bzero() ON SOLARIS - unsupported

#if !defined(_WIN32) || defined(__CYGWIN__)
#include <sys/time.h>
#else
#include <vrpn_Shared.h>
#endif
#include <sys/types.h>

#if !defined(_WIN32) || defined(__CYGWIN__)
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

// This is a horrible hack.  It should be fixed.  (WTL-20/Apr/1999)
#ifdef __CYGWIN__
#ifndef _GNU_H_WINDOWS32_SOCKETS
typedef u_int SOCKET;
/* XXX juliano 9/19/99 -- Standard C++ does not allow implicit
   declaration of functions.  You MUST declare them.  We need to devise
   a better solution to this problem 
// XXX forward declares of otherwise implicitly declared functions
*/
extern "C" {
int close( int filedes );
int sdi_connect_to_device( char *device );
int sdi_start_server( char *machine, char *server_name, char *args );
int sdi_disconnect_from_device( int des );
int sdi_noint_block_read( SOCKET insock, char *buffer, int length );
int sdi_noint_block_write( SOCKET outsock, char buffer[], int length );
}
#endif
#else
/* Including this file causes lots of errors becuase
   it includes winsock.h.  Unfortunately, it needs to
   include winsock.h so because of that we can't include
   it here.  This gives us several warnings, but it compiles. */
/* XXX juliano 9/19/99 -- not true.  see above. */
#include <sdi.h> 
#endif
// We really should use strerror instead of sys_errlist
// because sys_errlist is *NOT* portable whereas strerror
// is.  Until then, however, we'll just use strerror on
// WIN32 since it doesn't have sys_errlist at all.
//
// Tanner Lovelace <lovelace@cs.unc.edu> - 5-Feb-1999
#ifdef _WIN32
#define STRERROR(e) strerror(e)
#else
#define STRERROR(e) sys_errlist[e]
#endif

static char defaultMachine [] = "tyrosine";
static char defaultProgram [] =
            "/glycine/grip6/microscape/server/sdi_stm_server";
static char defaultArgs [] = "";

#define max(a,b) ((a) < (b) ? (b) : (a))


// NetworkedMicroscopeChannel
//
// Tom Hudson, September 1997
// Taken wholly from server_talk.c

// Encapsulates the connection to a PC running nM.
// Includes Michele Clark's modifications for directly connecting to
// specified ports.



NetworkedMicroscopeChannel::NetworkedMicroscopeChannel (void) :
    outbufPtr (outbuf + sizeof(outbufUsed)),
    outbufUsed (sizeof(outbufUsed)),
    microscope_fd (-1),
    microscope_udp_fd (-1),
    max_fd (-1),
    socketType (-1),
    directConnect (VRPN_FALSE),
    totalMsgs (0),
    totalUDPMsgs (0),
    lost (0),
    outOfOrder (0),
    expecting (1),
    seqNum (0)
{

//fprintf(stderr, "CTMS constructor\n");
}



NetworkedMicroscopeChannel::~NetworkedMicroscopeChannel (void) {
  if (directConnect)
    ReportStats();
  CloseMicroscope();

  if (microscope_fd != -1)
    close(microscope_fd);
  if (microscope_udp_fd != -1)
    close(microscope_udp_fd);
}
 


vrpn_bool NetworkedMicroscopeChannel::IsMicroscopeOpen (void) const {
  return ((microscope_fd != -1) ||
          (microscope_udp_fd != -1));
}



// A very few things seem to need to know what kind of socket
// we're using.

int NetworkedMicroscopeChannel::SocketType (void) const {
  return socketType;
}



int NetworkedMicroscopeChannel::ClearSendBuffer (void) {
  outbufPtr = outbuf + sizeof(outbufUsed);
  outbufUsed = sizeof(outbufUsed);

  return 0;
}



int NetworkedMicroscopeChannel::Buffer (const int _i) {
  int value = htonl(_i);

  memcpy(outbufPtr, &value, sizeof(value));
  outbufUsed += sizeof(value);
  outbufPtr += sizeof(value);

  if (outbufUsed > sizeof(outbuf)) return -1;
  return 0;
}



int NetworkedMicroscopeChannel::Buffer (const float _f) {

#if (defined(sgi) || defined(hpux) || defined(sparc))
  float value = _f;
#else
  float temp = _f;
  int value = htonl(*(int *) &temp);
#endif

  memcpy(outbufPtr, &value, sizeof(value));
  outbufUsed += sizeof(value);
  outbufPtr += sizeof(value);

  if (outbufUsed > sizeof(outbuf)) return -1;
  return 0;
}



int NetworkedMicroscopeChannel::Buffer (const struct timeval _t) {

  struct timeval v;
  v.tv_sec = htonl(_t.tv_sec);
  v.tv_usec = htonl(_t.tv_usec);

  memcpy(outbufPtr, &v, sizeof(v));
  outbufUsed += sizeof(v);
  outbufPtr += sizeof(v);

  if (outbufUsed > sizeof(outbuf)) return -1;
  return 0;
}



int NetworkedMicroscopeChannel::Buffer (const char * _c,
                                       const unsigned int _len) {
  memcpy(outbufPtr, _c, _len);
  outbufUsed += _len;
  outbufPtr += _len;

  if (outbufUsed > sizeof(outbuf)) return -1;
  return 0;
}



int NetworkedMicroscopeChannel::Unbuffer (char * & _buf, int & _i) const {
  _i = ntohl(*(int *) _buf);
  _buf += sizeof(int);
  return 0;
}



int NetworkedMicroscopeChannel::Unbuffer (char * & _buf, float & _f) const {

#if (defined(sgi) || defined(hpux) || defined(sparc))
  _f = *(float *) _buf;
#else
  int value = ntohl(*(int *) _buf);
  _f = *(float *) (&value);
#endif

  _buf += sizeof(float);
  return 0;
}



int NetworkedMicroscopeChannel::Unbuffer (char * & _buf,
                                         struct timeval & _tv) const {
  _tv.tv_sec = ntohl(((struct timeval *) _buf)->tv_sec);
  _tv.tv_usec = ntohl(((struct timeval *) _buf)->tv_usec);

  _buf += sizeof(struct timeval);
  return 0;
}



int NetworkedMicroscopeChannel::Unbuffer (char * & _buf, char * _c,
                                         const unsigned int _len) const {
  memcpy(_c, _buf, _len);
  _buf += _len;
  return 0;
}




int NetworkedMicroscopeChannel::OpenMicroscope (const char * _deviceName) {

  socketType = SOCKET_TCP;
  if (microscope_fd != -1) {
    fprintf(stderr, "NetworkedMicroscopeChannel::OpenMicroscope():  "
                    "microscope already open\n");
    return -1;
  }

  microscope_fd = sdi_connect_to_device((char *) _deviceName);
  if (microscope_fd == -1) {
    // See if we can open it directly
    microscope_fd = sdi_start_server(defaultMachine, defaultProgram,
                                     defaultArgs);
    if (microscope_fd == -1) {
      fprintf(stderr, "NetworkedMicroscopeChannel::OpenMicroscope():  "
                      "can't open microscope\n");
      return -1;
    }
  }

  max_fd = max(max_fd, microscope_fd);

  return 0;
}



int NetworkedMicroscopeChannel::OpenMicroscope (const int _socketType,
                                               const char * _SPMhost,
                                               const int _SPMport,
                                               const int _UDPport) {
  if (microscope_fd != -1) {
    fprintf(stderr, "NetworkedMicroscopeChannel::OpenMicroscope():  "
                    "microscope already open\n");
    return -1;
  }

  directConnect = VRPN_TRUE;
  socketType = _socketType;

  // Open TCP or UDP connection using connectsock()

  switch (_socketType) {

    case SOCKET_MIX:
      microscope_fd = Connectsock(_SPMhost, _SPMport, VRPN_TRUE, SOCKET_TCP);
      microscope_udp_fd = Connectsock(_SPMhost, _SPMport + 1,
                                      VRPN_TRUE, SOCKET_UDP, _UDPport);
      if (microscope_fd == -1) {
        fprintf(stderr, "NetworkedMicroscopeChannel::OpenMicroscope():  "
                        "error connecting to TCP socket\n");
        return -1;
      }
      if (microscope_udp_fd == -1) {
        fprintf(stderr, "NetworkedMicroscopeChannel::OpenMicroscope():  "
                        "error connecting to UDP socket\n");
        return -1;
      }
      printf("Connected to STM via TCP/UDP (port %d)!\n", _UDPport);
      break;

    case SOCKET_UDP:
      microscope_udp_fd = Connectsock(_SPMhost, _SPMport, VRPN_TRUE,
                                      SOCKET_UDP, _UDPport);
      if (microscope_udp_fd == -1) {
        fprintf(stderr, "NetworkedMicroscopeChannel::OpenMicroscope():  "
                        "error connecting to UDP socket\n");
        return -1;
      }
      printf("Connected to STM via UDP!\n");
      break;

    case SOCKET_TCP:
      microscope_fd = Connectsock(_SPMhost, _SPMport, VRPN_TRUE, SOCKET_TCP);
      if (microscope_fd == -1) {
        fprintf(stderr, "NetworkedMicroscopeChannel::OpenMicroscope():  "
                        "error connecting to TCP socket\n");
        return -1;
      }
      break;
  }

  max_fd = max(max_fd, microscope_fd);
  max_fd = max(max_fd, microscope_udp_fd);

  return 0;
}



int NetworkedMicroscopeChannel::CloseMicroscope (void) {
  if (directConnect) {
    if (microscope_fd != -1) {
      close(microscope_fd);
    }
    if (microscope_udp_fd != -1) {
      close(microscope_udp_fd);
    }
  } else {
    if (microscope_fd != -1) {
      sdi_disconnect_from_device(microscope_fd);
    }
    if (microscope_udp_fd != -1) {
      sdi_disconnect_from_device(microscope_udp_fd);
    }
  }

  microscope_fd = -1;
  microscope_udp_fd = -1;
  return 0;
}



// Write the length of the buffer into the first (sizeof int) bytes
// Send the buffer into the client
// Returns after the entire buffer is sent,
//         after an error occurs,
//         or (without sending) if the buffer is empty.
// Returns TOTAL number of bytes written, or -1 on failure
// Argument is the file descriptor to write to.

// If using UDP, prepends sequence number

int NetworkedMicroscopeChannel::SendBuffer (int _socketType) {
  // no output file
  if (microscope_fd == -1) return 0;

  // empty buffer
  if (outbufUsed == sizeof(outbufUsed)) return 0;

  switch (_socketType) {
    case SOCKET_MIX:
      // for now, send everything TCP
    case SOCKET_TCP:
      return SendTCPBuffer();
    case SOCKET_UDP:
      return SendUDPBuffer();
    default:
      fprintf(stderr, "NetworkedMicroscopeChannel::SendBuffer():  "
                      "must specify either TCP or UDP socket to send over.\n");
      return -1;
  }
}





  // TODO

int NetworkedMicroscopeChannel::CheckForIncomingPacket
     (const int _socketType) const {
  fd_set readfds, exceptfds;
  struct timeval timeout;
  int retval;
  int usefd;

  FD_ZERO(&readfds);
  FD_ZERO(&exceptfds);
  switch (_socketType) {
    case SOCKET_TCP:
      usefd = microscope_fd;
      break;
    case SOCKET_UDP:
      usefd = microscope_udp_fd;
      break;
  }
  FD_SET(usefd, &readfds);
  FD_SET(usefd, &exceptfds);
  timeout.tv_sec = 0;
  timeout.tv_usec = 0;

  retval = select(max_fd + 1, &readfds, NULL, &exceptfds, &timeout);
  if (retval == -1)
    if (errno == EINTR)  // ignore interrupt
      return 0;
    else {
      perror("NetworkedMicroscopeChannel::CheckForIncomingPacket():  "
             "select failed!");
      return -1;
    }

  // Check for exceptions
  if (FD_ISSET(usefd, &exceptfds)) {
    fprintf(stderr, "NetworkedMicroscopeChannel::CheckForIncomingPacket():  "
                    "exception on socket\n");
    return -1;
  }

  if (FD_ISSET(usefd, &readfds))
    return 1;
  return 0;
}





// Read a block sent from the other side of the connection.
// Returns the number of characters read in the block, or -1 on failure.
// May prepend receive time to the buffer sent up to MicroscopeIO.
// *Blocks* until it receives all of the next block that has been sent
// or there is an error.

// It is up to the caller to guarantee that there is enough space to
// receive the block

// If using UDP, gets sequence number and tracks lost or out-of-order
// messages.


int NetworkedMicroscopeChannel::RecvBuffer (char * _buf,
                                           const unsigned int _bufSize,
                                           const int _socketType) {
  int blockLen;

  switch (_socketType) {
    case SOCKET_TCP:
      blockLen = RecvTCPBuffer(_buf, _bufSize);
      break;
    case SOCKET_UDP:
      blockLen = RecvUDPBuffer(_buf, _bufSize);
      break;
    default:
      fprintf(stderr, "NetworkedMicroscopeChannel::RecvBuffer():  "
                      "must specify either TCP or UDP to receive.\n");
      return -1;
  }

  if ((socketType == SOCKET_MIX) && (_socketType == SOCKET_UDP))
    totalUDPMsgs++;
  else
    totalMsgs++;

  return blockLen;
}





int NetworkedMicroscopeChannel::RecvTCPBuffer
                               (char * _buf,
                                const unsigned int _bufSize) {
  struct timeval recvTime,
                 netRecvTime;
  int blockLen,
      netBlockLen;
  int bytesRead;
  int cmd;

  bytesRead = sdi_noint_block_read(microscope_fd, (char *) &netBlockLen,
                                   sizeof(netBlockLen));

  gettimeofday(&recvTime, NULL);

  if (!bytesRead) {
    fprintf(stderr, "NetworkedMicroscopeChannel::RecvTCPBuffer():  "
                    "at EOF.\n");
    return 0;
  }

  if (bytesRead != sizeof(netBlockLen)) {
    fprintf(stderr, "NetworkedMicroscopeChannel::RecvTCPBuffer():  "
                    "cannot get block len, only %d bytes.\n", bytesRead);
    return -1;
  }
  blockLen = htonl(netBlockLen);

  // BUG BUG BUG
  // Original code reversed the order of the next two commands.
  // Is that correct?

  blockLen -= sizeof(blockLen);
  if (blockLen > (int)_bufSize) {
    fprintf(stderr, "NetworkedMicroscopeChannel::RecvBuffer():  "
                    "buffer too small for incoming block.\n");
    return -1;
  }

  // Add in a NANO_RECV_TIMESTAMP message if appropriate
  if (directConnect) {
    cmd = NANO_RECV_TIMESTAMP;
    memcpy(_buf, &cmd, sizeof(cmd));
    _buf += sizeof(cmd);
    netRecvTime.tv_sec = htonl(recvTime.tv_sec);
    netRecvTime.tv_usec = htonl(recvTime.tv_usec);
    memcpy(_buf, &netRecvTime, sizeof(struct timeval));
    _buf += sizeof(struct timeval);
  }

  // Get the actual data
  if (sdi_noint_block_read(microscope_fd, (char *) _buf, blockLen)
            != blockLen) {
    fprintf(stderr, "NetworkedMicroscopeChannel::RecvBuffer():  "
                    "can't read incoming block.\n");
    return -1;
  }

  if (directConnect)
    blockLen += sizeof(int) + sizeof(struct timeval);

  return blockLen;
}




int NetworkedMicroscopeChannel::RecvUDPBuffer
                               (char * _buf,
                                const unsigned int /* _bufSize */) {
  struct timeval recvTime,
                 netRecvTime;
  char tempBuf [MAXBUF];
  char * tp,
       * ip;
  long recvSeqNum,
       netRecvSeqNum;
  int blockLen,
      netBlockLen;
  int bytesRead;
  int cmd;

  bytesRead = recv(microscope_udp_fd, tempBuf, MAXBUF, 0);

  gettimeofday(&recvTime, NULL);

  if (bytesRead < 0) {
    fprintf(stderr, "NetworkedMicroscopeChannel::RecvUDPBuffer():  "
                    "cannot read block.\n");
    return -1;
  }

  // Parse the packet { size | SPM_UDP_SEQ_NUM seqNum | data }
  tp = tempBuf;
  memcpy(&netBlockLen, tp, sizeof(int));
  blockLen = ntohl(netBlockLen);
  tp += sizeof(int);
  blockLen -= sizeof(int);

  // Get the sequence number;  leave it in what's to be passed up,
  // so it gets saved in the stream file.
  // HACK:  assumes packet begins with a sequence number
  memcpy(&cmd, tp, sizeof(int));
  cmd = ntohl(cmd);
  tp += sizeof(int);
  memcpy(&netRecvSeqNum, tp, sizeof(long));
  recvSeqNum = htonl(netRecvSeqNum);
  tp -= sizeof(int);  // point back at the sequence number indicator

  // Calculate lost or out-of-order
  if (recvSeqNum < expecting) {
    outOfOrder++;
    lost--;
    printf("#%ld out-of-order\n", recvSeqNum);
  } else if (recvSeqNum > expecting) {
    lost += (recvSeqNum - expecting);
    printf("%ld msg(s) lost: #%ld - #%ld\n", (recvSeqNum - expecting),
           expecting, recvSeqNum - 1);
    expecting = recvSeqNum + 1;
  } else
    expecting++;  // got what we expected

  ip = _buf;

  // Add NANO_RECV_TIMESTAMP to inbuf if necessary
  cmd = NANO_RECV_TIMESTAMP;
  memcpy (ip, &cmd, sizeof (int));
  ip += sizeof (int);
  netRecvTime.tv_sec = htonl (recvTime.tv_sec);
  netRecvTime.tv_usec = htonl (recvTime.tv_usec);
  memcpy (ip, &netRecvTime, sizeof (struct timeval));
  ip += sizeof (struct timeval);

  memcpy(ip, tp, blockLen);

  blockLen += sizeof(int) + sizeof(struct timeval);

  return blockLen;
}




// From Michele Clark's experiments

void NetworkedMicroscopeChannel::ReportStats (void) const {

  float percentLost, percentOrder;
  long totalSent;

  printf ("\n-----------------------------------------------------\n");
  if (socketType == SOCKET_MIX) {
    printf ("TCP stats for microscape --> SPM: (%lu msgs)\n", totalMsgs);
    printf ("UDP mix stats for microscape --> SPM: (%lu msgs)\n", totalUDPMsgs);
    if (totalUDPMsgs == 0)
      return;
  }
  else {
    if (socketType == SOCKET_TCP) 
      printf ("TCP stats for microscape --> SPM: (%lu msgs)\n", totalMsgs);
    else 
      printf ("UDP stats for microscape --> SPM: (%lu msgs)\n", totalMsgs);

    if (totalMsgs == 0)
      return;
  }

  if (socketType == SOCKET_UDP)
    totalSent = totalMsgs + lost;
  else if (socketType == SOCKET_MIX)
    totalSent = totalUDPMsgs + lost;

  if (socketType == SOCKET_UDP || socketType == SOCKET_MIX) {
    percentLost = (lost / (float) totalSent) * 100.0;
    percentOrder = (outOfOrder / (float) totalSent) * 100.0;

    printf ("******************************************************\n");
    printf ("   total # msgs received : %lu\n", totalSent-lost);
    printf ("   total # msgs sent here: %lu\n", totalSent);
    printf ("   # lost        : %lu\t%% lost        : %.2g\n", lost,
            percentLost);
    printf ("   # out-of-order: %lu\t%% out-of-order: %.2g\n", outOfOrder,
            percentOrder);
  }
  printf ("-----------------------------------------------------\n");
  fflush (stdout);
}



// If client == VRPN_TRUE, connect to a socket at host::port
// If client == VRPN_FALSE, get a socket and bind to host::port

int NetworkedMicroscopeChannel::Connectsock (const char * _host,
                                            const int _port,
                                            const vrpn_bool _client,
                                            const int _protocol,
                                            const int _UDPport) {
  struct hostent  *phe;          // pointer to host information entry
  struct sockaddr_in serv_addr;  // an Internet endpoint address
  struct sockaddr_in cli_addr;
  int    sd;                     // socket descriptor


  printf ("host: %s  port: %d  client: %d  protocol: %d  (TCP = 1, UDP = 2)\n",
          _host, _port, _client, _protocol);

  bzero ((char *) &serv_addr, sizeof(serv_addr));

  /* Map service name to port number       */
  serv_addr.sin_port = htons (_port);
  serv_addr.sin_family = AF_INET;

  if (_client) {
  /* Map host name to IP address, allowing for dotted decimal   */
    if ((phe = gethostbyname(_host)))
      bcopy(phe->h_addr, (char*) &serv_addr.sin_addr, phe->h_length);
    else if ((serv_addr.sin_addr.s_addr = inet_addr(_host)) == INADDR_NONE) {
      fprintf(stderr, "can't get %s host entry\n", _host);
      return -1;
    }
  }
  else
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  /* Allocate a socket  */
  if (_protocol == SOCKET_TCP) {
    // We're using TCP
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      fprintf(stderr, "can't create socket: %s\n", STRERROR(errno));
      return -1;
    }
  } else if (_protocol == SOCKET_UDP) {
    // We're using UDP
    if ((sd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
      fprintf(stderr, "can't create socket: %s\n", STRERROR(errno));
      return -1;
    }
    /* Bind any local address for us */
    bzero ((char *) &cli_addr, sizeof(cli_addr));    /* zero out */
    cli_addr.sin_family = AF_INET;
    cli_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    cli_addr.sin_port = htons(_UDPport);
    if (bind (sd, (struct sockaddr *) &cli_addr, sizeof(cli_addr)) < 0) {
      fprintf (stderr, "client: can't bind local address\n");
      return -1;
    }
  } else {
    fprintf(stderr, "Illegal protocol (%d) in Connectsock()\n", _protocol);
    return -1;
  }

  if (_client) {
    /* Connect the socket */
    if (connect(sd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        fprintf(stderr, "can't connect to %s:  %s\n", _host,
                STRERROR(errno));
        perror("bind");
        return -1;
    }
  }
  else {
    /* Bind the socket */
    if (bind (sd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
      perror ("can't bind local address");
      return -1;
    }
  }
  return sd;
}

int NetworkedMicroscopeChannel::SendUDPBuffer (void) {

  char newBuf [MAXBUF];
  char * ptr,
       * bufptr;
  long netSeqNum;
  int netBufUsed;
  unsigned int newSize;
  int cmd;

  ptr = newBuf;
  newSize = outbufUsed + sizeof(int) + sizeof(long);

  // illegal buffer
  if (newSize > sizeof(outbuf)) {
    fprintf(stderr, "NetworkedMicroscopeChannel::SendBuffer():  "
                    "can only send %d characters.\n",  sizeof(outbuf));
    return -1;
  }

  // Write the length of the buffer into the initial bytes
  netBufUsed = htonl(newSize);
  memcpy(ptr, &netBufUsed, sizeof(netBufUsed));

  // add sequence number
  ptr += sizeof(int);
  cmd = SPM_UDP_SEQ_NUM;
  cmd = htonl(cmd);
  memcpy(ptr, &cmd, sizeof(int));
  ptr += sizeof(int);
  seqNum++;
  netSeqNum = htonl(seqNum);
  memcpy(ptr, &netSeqNum, sizeof(long));
  ptr += sizeof(long);

  // Try to send
  bufptr = &(outbuf[0]) + sizeof(int);
  //outbufUsed -= sizeof(int);
  memcpy(ptr, bufptr, outbufUsed - sizeof(int));

  if (sdi_noint_block_write(microscope_udp_fd, newBuf, newSize) != (signed)newSize) {
    fprintf(stderr, "NetworkedMicroscopeChannel::SendBuffer():  "
                    "UDP write failed.\n");
    return -1;
  }

  // Changed by Michele Clark from (outbufUsed - sizeof(outbufUsed))
  return newSize;
}

int NetworkedMicroscopeChannel::SendTCPBuffer (void) {

  int netBufUsed;

  // illegal buffer
  if (outbufUsed > sizeof(outbuf)) {
    fprintf(stderr, "NetworkedMicroscopeChannel::SendBuffer():  "
                    "can only send %d characters.\n",  sizeof(outbuf));
    return -1;
  }

  // Write the length of the buffer into the initial bytes
  netBufUsed = htonl(outbufUsed);
  memcpy(outbuf, &netBufUsed, sizeof(netBufUsed));

  // Try to send
  if (sdi_noint_block_write(microscope_fd, outbuf, outbufUsed)
                           != (signed)outbufUsed) {
    fprintf(stderr, "NetworkedMicroscopeChannel::SendBuffer():  "
                    "TCP write failed.\n");
    return -1;
  }

  // Changed by Michele Clark from (outbufUsed - sizeof(outbufUsed))
  return outbufUsed;
}
