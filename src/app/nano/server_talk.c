/*	This file contains routines that will be useful to the client
 * and server routines that talk with the STM. */

#include	<stdio.h>
#include	<unistd.h>
#include	<string.h>
#include        <sys/time.h>      // for gettimeofday() and struct timeval
// for recv
#include <sys/types.h>
#include <sys/socket.h>

#include        <sdi.h>
#include	<vrpn_Connection.h> // for vrpn replacements of sdi functions

#include	"stm_cmd.h"
#include 	"server_talk.h"

#include <nmb_Time.h>
#include <nmb_Debug.h>


extern int spm_verbosity;
#define    VERBOSE(level,msg) if (spm_verbosity >= level) printf("%s\n",msg);


/* GLOBAL VARIABLES */

static	char	out_buffer[MAXBUF];
static	char	*out_bufptr;
static	int	out_bufused = 0;
static	int	stm_send_disabled = 1;

/* added by Michele Clark 6/2/97 */
long totalMsgs = 0, totalUDPMsgs = 0;
long lost = 0, outOfOrder = 0, expecting = 1;
long seqNum = 0;

// Added by Michele Clark 7/22/97 for network stuff  (from microscape.h)
#define TCP 0
#define UDP 1
#define MIX 2
extern int socketType;   // defined in microscape.c
extern int UDPport;
extern int SPMport;
extern int stm_descriptor;
extern int stm_descriptor_UDP;


void reportStats ()
/* Michele Clark 6/23/97 */ 
/* Report #/% messages lost, out-of-order, and # received */
{
  float percentLost, percentOrder;
  long totalSent;

  printf ("\n-----------------------------------------------------\n");
  if (socketType == MIX) {
    printf ("TCP stats for microscape --> SPM: (%u msgs)\n", totalMsgs);
    printf ("UDP mix stats for microscape --> SPM: (%u msgs)\n", totalUDPMsgs);
    if (totalUDPMsgs == 0)
      return;
  }
  else {
    if (socketType == TCP) 
      printf ("TCP stats for microscape --> SPM: (%u msgs)\n", totalMsgs);
    else 
      printf ("UDP stats for microscape --> SPM: (%u msgs)\n", totalMsgs);

    if (totalMsgs == 0)
      return;
  }

  if (socketType == UDP)
    totalSent = totalMsgs + lost;
  else if (socketType == MIX)
    totalSent = totalUDPMsgs + lost;

  if (socketType == UDP || socketType == MIX) {
    percentLost = (lost / (float) totalSent) * 100.0;
    percentOrder = (outOfOrder / (float) totalSent) * 100.0;

    printf ("******************************************************\n");
    printf ("   total # msgs received : %u\n", totalSent-lost);
    printf ("   total # msgs sent here: %u\n", totalSent);
    printf ("   # lost        : %u\t%% lost        : %.2g\n", lost,
	    percentLost);
    printf ("   # out-of-order: %u\t%% out-of-order: %.2g\n", outOfOrder,
	    percentOrder);
  }
  printf ("-----------------------------------------------------\n");
  fflush (stdout);
}


/** 	this routine enables the sending of data to outfile via
** stm_send_buffer_to().  it should be called if in READ_DEVICE_MODE,
** ie. if there's a device out there to receive commands.  This
** allows user modes to always function as if there was a microscope
** server out there even if the data is canned.  Any routine sending
** out commands MUST BE PREPARED TO BE IGNORED.  No sending commands
** and waiting for a reply!
**/
int	stm_enable_send()
{
	return (stm_send_disabled = 0);
}

/**	This routine will clear the outgoing buffer and prepare it to
 * be filled with commands or data for the other side.  It reserves
 * space for the length of the buffer to be placed as the first thing
 * in the buffer. */

int	
stm_clear_outgoing_buffer()
{
	out_bufptr = &(out_buffer[0]) + sizeof(out_bufused);
	out_bufused = sizeof(out_bufused);

	return(0);
}


/**	These routines will add the given types into the buffer. */

int stm_buffer_int(int value) 
{
	int	local_value = htonl(value);

	memcpy(out_bufptr, &local_value, sizeof(local_value)); 
	out_bufused += sizeof(local_value); 
	out_bufptr += sizeof(local_value); 
	if (out_bufused > sizeof(out_buffer)) return(-1); 
	return(0); 
}

int stm_buffer_float(double value) 
{
#if (defined(sgi) || defined(hpux) || defined(sparc))
	float	local_value = value;
#else
	float	temp_value = value;
	int	local_value = htonl(*(int*)(&temp_value));
#endif

	memcpy(out_bufptr, &local_value, sizeof(local_value)); 
	out_bufused += sizeof(local_value); 
	out_bufptr += sizeof(local_value); 
	if (out_bufused > sizeof(out_buffer)) return(-1); 
	return(0); 
}

int stm_buffer_chars(char *c, int len) 
{
	memcpy(out_bufptr, c, len); 
	out_bufused += len; 
	out_bufptr += len; 
	if (out_bufused > sizeof(out_buffer)) return(-1); 
	return(0); 
}

/**	This routine will fill in the length of the outgoing buffer and
 *      send the buffer to the client.  It will hang until the entire buffer
 *      has been sent or there is an error in the sending.
 *	If there is not data in the buffer, the buffer will not be sent.
 *	Returns number of bytes written on success, -1 on failure.
 *	The file descriptor to be sent to is passed. 

 
   Changes made by Michele Clark:
     o returns actual number of bytes written on success (including size) (6/22)
     o if using UDP, add sequence number  (6/23)
 */   

int	stm_send_buffer_to(int outfile)
{
  int net_bufused;
  long net_seqNum;
  int newSize = out_bufused;
  char newBuf[MAXBUF];
  char* ptr, *bufptr;
  int tcp;
  int cmd;

  // If there's no output file, do nothing 
  if (stm_send_disabled) {
    return(0);
  }
  
  // If there is no data, do nothing 
  if (out_bufused == sizeof (int)) {
    return(0);
  }

  // If we're using split streams, send everything via TCP for now
  if (socketType == MIX)
    outfile = stm_descriptor;

  // Let's determine if we're sending tcp or udp
  if (socketType == TCP || (socketType == MIX && outfile == stm_descriptor))
    tcp = 1;
  else
    tcp = 0;

  if (!tcp) {
    // Add in size of SPM_UDP_SEQ_NUM and long
    newSize += (sizeof (int) + sizeof (long));
    ptr = newBuf;
  }
  else
    ptr = out_buffer;


  // Make sure we are not asking for too much 
  if (newSize > sizeof(out_buffer)) {
    fprintf(stderr,"stm_send_buffer_to: Can only send %d characters\n", 
	    sizeof(out_buffer));
    return(-1);
  }
  
  // Fill in the length 
  net_bufused = htonl (newSize);   // convert to network format
  memcpy (ptr, &net_bufused, sizeof (int));
  ptr += sizeof (int);

  // If using UDP, add sequence number
  if (!tcp) {
      // Add in SPM_UDP_SEQ_NUM to out_buffer
      cmd = SPM_UDP_SEQ_NUM;
      cmd = htonl (cmd);
      memcpy (ptr, &cmd, sizeof (int));
      ptr += sizeof (int);
      seqNum++;
      net_seqNum = htonl (seqNum);
      memcpy (ptr, &net_seqNum, sizeof (long));
      ptr += sizeof (long);
  }

  if (tcp) { 
    // Try to send buffer
    if (vrpn_noint_block_write (outfile, out_buffer, out_bufused) != out_bufused){
      fprintf (stderr, "stm_send_buffer_to: Write failed\n");
      return (-1);
    }
  }
  else {
    // add in data
    bufptr = &(out_buffer[0]) + sizeof (int);  // skip past old size
    out_bufused -= sizeof (int);
    memcpy (ptr, bufptr, out_bufused);
    
    // Try to send buffer
    if (vrpn_noint_block_write (outfile, newBuf, newSize) != newSize) {
      fprintf (stderr, "stm_send_buffer_to: Write failed\n");
      return (-1);
    }
  }    

  return (newSize);
}


/**	This routine will read a block that was sent from the other
 * side.  It returns the number of characters that were read in the
 * block.  It will hang until it has got all of the next block that
 * is sent or there is an error.
 *	It is up to the caller to guarantee that there is enough
 * space to receive the block (maxlen).
 *	This routine returns the number of characters in the block
 * on success and -1 on failure. 

 
    Changes made by Michele Clark:
     o if using UDP, get sequence number (6/23/97)
     o if using UDP, calculate lost or out-of-order messages
     o add receive time to the buffer that we're passing back to microscape 
       (7/23/97)
 */
int	stm_recv_buffer_from(int infile, char* inbuf, int maxlen)
{
  int net_blocklen;
  long net_seqNum;
  int cmd;
  int block_len;
  int bytesRead;
  struct timeval netRecvTime, recvTime;
  char* bufptr;
  long recvSeqNum;
  char debugBuf[1024];
  char tempBuf[MAXBUF];
  char* tmpptr;

  // Try to read the number of bytes in the next block 
  if (socketType == TCP || (socketType == MIX && infile == stm_descriptor)) {
    bytesRead = vrpn_noint_block_read (infile, (char *) &net_blocklen, 
				      sizeof(int));

    // save time we received this msg
    gettimeofday (&recvTime, NULL);

    if (bytesRead == 0) {
      // we've reached EOF
      fprintf (stderr, "stm_recv_buffer_from: Reached EOF\n");
      return (bytesRead);
    }
    if (bytesRead != sizeof (int)) {
      fprintf(stderr,"stm_recv_buffer_from: Cannot get block len: %d\n", 
	      bytesRead);
      return(-1);
    }

    block_len = ntohl (net_blocklen);    // convert to host format

    // Make sure the block isn't too long.  If so, fail 
    if (block_len > maxlen) {
      fprintf(stderr,"stm_recv_buffer_from(): Block too long!\n");
      return -1;
    }

    // Strip off length taken up by length header
    block_len -= sizeof (int);
    
    bufptr = inbuf;

    if (SPMport != -1) {
      // Add in NANO_RECV_TIMESTAMP message
      cmd = NANO_RECV_TIMESTAMP;
      memcpy (bufptr, &cmd, sizeof (int));
      bufptr += sizeof (int);
      netRecvTime.tv_sec = htonl (recvTime.tv_sec);
      netRecvTime.tv_usec = htonl (recvTime.tv_usec);
      memcpy (bufptr, &netRecvTime, sizeof (struct timeval));
      bufptr += sizeof (struct timeval);
    }

    // Try to get the rest of the block 
    bytesRead = vrpn_noint_block_read (infile, bufptr, block_len);
    if (bytesRead != block_len) {
      fprintf(stderr,"stm_recv_buffer_from(): Cannot read block: %d\n", 
	      bytesRead);
      return(-1);
    }

    sprintf (debugBuf, "---------------------------------------------------");
    VERBOSE (3, debugBuf);
    sprintf (debugBuf, "** Message received: %d bytes at %us %ums %uus", 
	     bytesRead, recvTime.tv_sec, recvTime.tv_usec/1000, 
	     recvTime.tv_usec%1000);
    VERBOSE (3, debugBuf);
  }
  else {
    // Read in entire packet
    bytesRead = recv (infile, tempBuf, MAXBUF, 0);
    
    // Save time we received the message
    gettimeofday (&recvTime, NULL);

    if (bytesRead < 0) {
      fprintf(stderr,"stm_recv_buffer_from(): Cannot read block: %d\n", 
	      bytesRead);
      return(-1);
    }

    // Parse the packet {size|SPM_UDP_SEQ_NUM seqNum|data}
    tmpptr = tempBuf;
    net_blocklen = *(int *) tempBuf;
    block_len = ntohl (net_blocklen);
    tmpptr += sizeof (int);
    block_len -= sizeof (int);

    // Get the sequence number, but don't take it out of what's
    // passed up (we want to save this in the stream file)
    cmd = *(int *) tmpptr;
    cmd = ntohl (cmd);
    tmpptr += sizeof (int);
    net_seqNum = *(long *) tmpptr;
    recvSeqNum = ntohl (net_seqNum);
    tmpptr -= sizeof (int);  // point the bufptr at SPM_UDP_SEQ_NUM

    // Calculate lost or out-of-order
    if (recvSeqNum < expecting) {
      // This is an out-of-order message!
      outOfOrder++;
      lost--;
      printf ("#%d out-of-order", recvSeqNum);
    }
    else if (recvSeqNum > expecting) {
      // There's been a lost message (maybe more)!
      lost += (recvSeqNum - expecting);
      printf ("%d msg(s) lost: #%d - #%d", (recvSeqNum - expecting), 
	       expecting, recvSeqNum - 1);      
      expecting = recvSeqNum + 1;
    }
    else
      // This is what we were expecting!
      expecting++;

    bufptr = inbuf;
    if (SPMport != -1) {
      // Add in NANO_RECV_TIMESTAMP to inbuf
      cmd = NANO_RECV_TIMESTAMP;
      memcpy (bufptr, &cmd, sizeof (int));
      bufptr += sizeof (int);
      netRecvTime.tv_sec = htonl (recvTime.tv_sec);
      netRecvTime.tv_usec = htonl (recvTime.tv_usec);
      memcpy (bufptr, &netRecvTime, sizeof (struct timeval));
      bufptr += sizeof (struct timeval);
    }
    
    // Add data to inbuf
    memcpy (bufptr, tmpptr, block_len);

    sprintf (debugBuf, "** Message #%d received: %d bytes at %us %ums %uus", 
	     recvSeqNum, bytesRead,
	     recvTime.tv_sec, recvTime.tv_usec/1000, 
	     recvTime.tv_usec%1000);    
    VERBOSE (3, debugBuf);
  }

  if (socketType == MIX && infile == stm_descriptor_UDP)
    totalUDPMsgs++;
  else
    totalMsgs++;

  if (SPMport != -1)
    // account for NANO_RECV_TIMESTAMP
    return (block_len + sizeof (int) + sizeof (struct timeval));
  else 
    return (block_len);
}
