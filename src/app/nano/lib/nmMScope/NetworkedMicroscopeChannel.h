#ifndef MICROSCOPE_CHANNEL_H
#define MICROSCOPE_CHANNEL_H

#include <time.h>

#include <nmb_Types.h>  // vrpn_bool

#include <stm_cmd.h>  // included only for definition of MAXBUF

#include "nmm_Types.h"  // for SOCKET_* types


// NetworkedMicroscopeChannel
//
// Tom Hudson, August, 1997
// Taken wholly from previous code in server_talk.[ch]

// This class buffers up data to be sent to a microscope.  The send only
// occurs if EnableSend() has been called;  otherwise, SendBuffer is ignored.
// This allows higher level commands to ignore the difference between a
// live microscope and canned data.  However, this means that any routine
// sending commands MUST BE PREPARED TO BE IGNORED.  Nothing that sends
// a command can then *wait* for a reply!

// This class also contains routines to receive buffers and to unpack
// them into local variables.

// TODO:
//    TCP, UDP ought not be visible outside this class
//    Why it's hard:  We'd need to do twice as many selects(), which
// could really slow down the networking.




class NetworkedMicroscopeChannel {

  public:

    NetworkedMicroscopeChannel (void);
    ~NetworkedMicroscopeChannel (void);
 

    int OpenMicroscope (const char * _deviceName);
      // Standard initialization

    int OpenMicroscope (const int _socketType,
                        const char * _SPMhost,
                        const int _SPMport, const int _UDPport);
      // Special initialization for Michele Clark's experiments

    int CloseMicroscope (void);
      // Tears down the connection


    vrpn_bool IsMicroscopeOpen (void) const;
    int SocketType (void) const;
      // returns one of the SOCKET_ defines
      // (SOCKET_TCP in case of standard initialization)

    int ClearSendBuffer (void);
    int Buffer (const int);
    int Buffer (const float);
    int Buffer (const char *, const unsigned int _len);
    int Buffer (const struct timeval);
      // Add data to the buffer to be sent


    int SendBuffer (int = SOCKET_TCP);

    int CheckForIncomingPacket (const int = SOCKET_TCP) const;
      // returns 1 if there is a packet ready to read, 0 if not,
      // and -1 on error (otherwise it'd return vrpn_bool)


    int Unbuffer (char * & _buffer, int &) const;
    int Unbuffer (char * & _buffer, float &) const;
    int Unbuffer (char * & _buffer, char *, const unsigned int _len) const;
    int Unbuffer (char * & _buffer, struct timeval &) const;
      // Extracts data from the buffer and advances the pointer
      // passed in to point beyond the data extracted

    int RecvBuffer (char * _buffer, const unsigned int _len,
                    const int = SOCKET_TCP);
      // Receives a buffer from the network
      // nonconst because it updates our statistics records

    void ReportStats (void) const;

  private:

    char outbuf [MAXBUF];
    char * outbufPtr;
    unsigned int outbufUsed;

    int microscope_fd;
    int microscope_udp_fd;
    int max_fd;
    int socketType;  // TCP, UDP, MIX

    // when we're connecting to a port rather than a device
    vrpn_bool directConnect;
    int SPMport;
    int UDPport;

    // statistics and loss/ordering tracking for UDP
    long totalMsgs;
    long totalUDPMsgs;
    long lost;
    long outOfOrder;
    long expecting;
    long seqNum;

    int RecvTCPBuffer (char * _buffer, const unsigned int _len);
    int RecvUDPBuffer (char * _buffer, const unsigned int _len);
      // nonconst because they update our statistics records

    int SendTCPBuffer (void);
    int SendUDPBuffer (void);

    int Connectsock (const char *, const int,
                     const vrpn_bool, const int, const int = -1);
      // Utility code from Michele Clark to connect to a socket
};

#endif  // MICROSCOPE_CHANNEL_H

