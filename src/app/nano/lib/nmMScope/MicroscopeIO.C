#include "MicroscopeIO.h"

#include <stdlib.h>
#include <stdio.h>   // for fprintf()
// Should be !_WIN32 - WHY?  There doesn't seem to be a comment anywhere
// that explains when we want the one and when the other.
//#if !defined(_WIN32) || defined(__CYGWIN)
#if !defined(_WIN32)
#include <unistd.h>  // for sleep()
#include <sys/types.h> // for htonl() used by RecordResistance() hack
#include <netinet/in.h>// "
#endif

#include <stm_cmd.h>

#include <nmb_Globals.h>
#include <nmb_Decoration.h>
#include <nmb_Time.h>
#include <nmb_Debug.h>

#include "Microscope.h"

// from nmm_Microscope.h
#define FC_MAX_HALFCYCLES (100)


#define CHECK(a) if ((a) == -1) return -1
#define CHECKvoid(a) if ((a) == -1) return

// Some compilers seem to be broken, insisting that
// a char ** cannot be passed as a const char ** argument.

#if defined(FLOW) || defined(linux) || defined(__CYGWIN__)
#define UGLYCAST (const char **)
#else
#define UGLYCAST
#endif  // not FLOW...

// readMode defaults to READ_FILE
// InitDevice and InitStream can reset it as appropriate

MicroscopeIO::MicroscopeIO (Microscope * _scope) :
    microscope (_scope),
    comm (),
    inputStream (NULL),
    outputStream (NULL),
    readMode (READ_FILE),
    bufsize (MAXBUF),
    ready (VRPN_FALSE),
    queueLength (0),
    queueTail (-1),
    queueHead (0) {
       stream_time.tv_sec = 0L;
       stream_time.tv_usec = 0L;
       stream_start_time.tv_sec = 0L;
       stream_start_time.tv_usec = 0L;
       d_stream_runtime.tv_sec = 0L;
       d_stream_runtime.tv_usec = 0L;

//fprintf(stderr, "MicroscopeIO constructor\n");

}

MicroscopeIO::~MicroscopeIO (void) {
  // Shut the server down nicely
  // TODO:  make sure we're talking to a live microscope and not using
  //   canned data.  Previous version checked a global variable.  Does
  //   checking IsMicroscopeOpen() work?

  if (comm.IsMicroscopeOpen()) {
    if (Shutdown() == -1) {
      fprintf(stderr, "MicroscopeIO::~MicroscopeIO():  "
                      "could not send quit command to STM server\n");
    }
    // Wait to give the server a chance to receive the message before it
    // gets a connection-closed exception
    sleep(3);
    // Comm's destructor will do a sdi_disconnect_from_device
  }

  // close input stream nicely
  // IGNORES microscope->state.saveStream

  if (outputStream)
    //if (microscope->state.saveStream) {
      if (stm_close_stream(outputStream))
        fprintf(stderr, "Could not close the output stream!\n");
      else
        printf("Successfully saved streamfile.\n");
    //} else
      //stm_abort_stream(outputStream);
}




int MicroscopeIO::InitDevice (const char * _deviceName) {

  readMode = READ_DEVICE;
  return comm.OpenMicroscope(_deviceName);

}

int MicroscopeIO::InitDevice (const int _socketType, const char * _SPMhost,
                              const int _SPMport, const int _UDPport) {

  readMode = READ_DEVICE;
  return comm.OpenMicroscope(_socketType, _SPMhost, _SPMport, _UDPport);

}

int MicroscopeIO::InitStream (const char * _inputStreamName) {

  readMode = READ_STREAM;
  inputStream = stm_open_datastream_for_read(_inputStreamName);
  if (!inputStream) {
    fprintf(stderr,"MicroscopeIO::InitStream():  "
                   "could not open input stream %c\n",0x08);
    return -1;
  }

  return 0;
}

// rewinds the stream to the beginning, so we can view it again.
int MicroscopeIO::RestartStream () {
  if (!inputStream) {
    fprintf(stderr,"MicroscopeIO::RestartStream(): no input stream.\n");
    return -1;
  }
  if (stm_restart_stream(inputStream) != 0) {
    fprintf(stderr,"MicroscopeIO::RestartStream(): couldn't restart stream.\n");
    return -1;
  }
  // Reset the timer to zero so we will pay attention to the packets
  // coming from the re-wound stream

  microscope->ResetClock();
  // I'm not sure which of these must be reset, but this works :)
  stream_start_time.tv_sec = stream_time.tv_sec = 0;  
  stream_start_time.tv_usec = stream_time.tv_usec = 0;
  sec = 0;
  usec = 0;

  //remove traces of any modifications which might have occured.
  decoration->clearScrapes();
  decoration->clearPulses();


  return 0;
}

void MicroscopeIO::SetStreamToTime (struct timeval time) {

  if (time_compare(stream_time, time)) { //stream_time > time
    RestartStream();
  } //end if
  microscope->d_next_time = time;

} //end SetStreamToTime

// ********************** BEGIN HACK **********************
// this allows us to store vrpn_Ohmmeter messages in the stream file
// as if they came from the old ohmeter
// note: this may be removed when we have vrpn_Streamfile working
int MicroscopeIO::RecordResistance
                       (int which_meter, struct timeval t,
                        float r,
			float voltage, float range, float filter, int status)
{
    char res_buf[300];
    char *bufptrRes = res_buf;
    int buffsizeRes = 0;
    static int resOutputFailureCount = 0;
    float res_d = r;
 struct timeval vRes;
    static long time_offset = 0;

    int msg_typeRes = htonl(OHM_RESISTANCE_WSTATUS);
    // if we use the original time from the ohmmeter server then we
    // may have a problem when we replay the stream file because if
    // the ohmmeter time is shifted forward of the microscope time
    // the stream file will appear to pause on the first ohmmeter
    // message while the difference in time elapses. Then you would
    // get a burst of microscope messages for a while because the time
    // will have gotten much farther ahead of future microscope message
    // times. On the other hand, we can just introduce a negative offset
    // for the ohmmeter message times to set ohmmeter times behind the
    // microscope time - then ohmmeter messages will not interfere with
    // other messages but they will still have their own self-consistent
    // timebase - AAS (after realizing that it was a mistake to throw
    // away the ohmmeter timestamps even though they weren't synchronized)

    // original scheme:
    //vRes.tv_sec = htonl(stream_time.tv_sec); 
    //vRes.tv_usec = htonl(stream_time.tv_usec);

    // new scheme:
    vRes.tv_sec = t.tv_sec + time_offset;
    vRes.tv_usec = t.tv_usec;
    if (time_compare(stream_time, vRes) < 0){
	// This code should only execute once - assumes
	// that there was not more than 1000 seconds of delay between
	// last microscope message and this ohmmeter message
	if (time_offset != 0) 
	    fprintf(stderr, "Error: ohmmeter timestamp offset set twice\n");
	time_offset += stream_time.tv_sec - vRes.tv_sec - 1000;
	vRes.tv_sec = t.tv_sec + time_offset;
    }

    int meter = htonl(which_meter);
    int resistance = htonl(*(long *)&res_d);
	int stat = htonl(status);

    // HACK within a HACK (to go to modfile as well as stream file)
    microscope->RcvResistance3(which_meter, vRes.tv_sec, vRes.tv_usec,
	r, voltage, range, filter);

    vRes.tv_sec = htonl(vRes.tv_sec);
    vRes.tv_usec = htonl(vRes.tv_usec);

    memcpy(bufptrRes, &msg_typeRes, sizeof(msg_typeRes));
    bufptrRes += sizeof(msg_typeRes);
    buffsizeRes += sizeof(msg_typeRes);

    memcpy(bufptrRes, &meter, sizeof(meter));
    bufptrRes += sizeof(meter);
    buffsizeRes += sizeof(meter);
    memcpy(bufptrRes, &vRes.tv_sec, sizeof(vRes.tv_sec));
    bufptrRes += sizeof(vRes.tv_sec);
    buffsizeRes += sizeof(vRes.tv_sec);
    memcpy(bufptrRes, &vRes.tv_usec, sizeof(vRes.tv_usec));
    bufptrRes += sizeof(vRes.tv_usec);
    buffsizeRes += sizeof(vRes.tv_usec);
    memcpy(bufptrRes, &resistance, sizeof(resistance));
    bufptrRes += sizeof(resistance);
    buffsizeRes += sizeof(resistance);

    memcpy(bufptrRes, &voltage, sizeof(voltage));
    bufptrRes += sizeof(voltage);
    buffsizeRes += sizeof(voltage);

    memcpy(bufptrRes, &range, sizeof(range));
    bufptrRes += sizeof(range);
    buffsizeRes += sizeof(range);

    memcpy(bufptrRes, &filter, sizeof(filter));
    bufptrRes += sizeof(filter);
    buffsizeRes += sizeof(filter);

    memcpy(bufptrRes, &stat, sizeof(stat));
	bufptrRes += sizeof(stat);
	buffsizeRes += sizeof(stat);

    if (outputStream){
      if (stm_write_block_to_stream(outputStream, res_buf, buffsizeRes)) {
        if (resOutputFailureCount++ == 15) {
	  fprintf(stderr, "OhmHack: Cannot write to output stream.\n");
	  resOutputFailureCount = 0;
        }
	return -1;
      }
    }
    else return -1;
    return 0;
}
// *********************** END HACK ***********************

// Gets a packet in from the network or streamfile
// Gets one out of queue if necessary
int MicroscopeIO::HandlePacket (void) {

  static int outputFailureCount = 0;
  int bs = 0;
  vrpn_bool gotSomething = VRPN_FALSE;
  vrpn_bool updateStreamTime = VRPN_TRUE;
  struct timeval start_t, now_t;
  //int i;

  switch (readMode) {
    case READ_DEVICE:
      bs = GetPacketFromDevice(buffer, bufsize);
      GetStartTime(&start_t);
      GetCurrTime(&now_t);
      decoration->elapsedTime = now_t.tv_sec - start_t.tv_sec;
      break;
    case READ_STREAM:
      bs = GetPacketFromStream(buffer, bufsize, updateStreamTime);
      decoration->elapsedTime = d_stream_runtime.tv_sec;
      break;
  }
  if (bs > 0) gotSomething = VRPN_TRUE;

  // If we are writing to an output stream, add this to it

  if (outputStream && (bs > 0)) {
    if (stm_write_block_to_stream(outputStream, buffer, bs)) {
      if (outputFailureCount++ == 15) {
        fprintf(stderr, "Cannot write to output stream.\n");
        outputFailureCount = 0;
      }
    }
  }

  // Michele Clark's modifications

  if ((comm.SocketType() == SOCKET_MIX) && !gotSomething &&
      ready && (queueLength > 0)) {
    // we didn't get anything, so grab something from the queue
    DispatchPacket(dataQ[queueHead].data,
                   dataQ[queueHead].len,
                   VRPN_FALSE);
    queueHead++; queueHead %= 20;
    queueLength--;
  }


  // Extract each message from the packet and dispatch it to the
  // appropriate handler function
  if (gotSomething)
    CHECK(DispatchPacket(buffer, bs, VRPN_TRUE));

 	 // not completely grokked, but it seems to work - TCH
  	if (updateStreamTime) {
    	// get the stream time from the packet

    	// Michele Clark's experiment
    	if (microscope->state.useRecvTime) {
      		stream_time.tv_sec = temp_time.tv_sec;
    		stream_time.tv_usec = temp_time.tv_usec;
    	} else {
      		stream_time.tv_sec = sec;
      		stream_time.tv_usec = usec;
    	}

    	if (!stream_start_time.tv_sec && !stream_start_time.tv_usec)
      		stream_start_time = stream_time;

    	// Calculate the stream run time from the stream startup time
    	// and the current stream time
    	time_subtract(stream_time, stream_start_time, &d_stream_runtime);
	}

  return gotSomething;
}


int MicroscopeIO::EnableOutputStream (int (* f) (stm_stream *)) {

  outputStream =
        stm_open_datastream_for_write(microscope->state.outputStreamName);
  if (!outputStream) {
    fprintf(stderr, "Could not open output stream\n");
    return -1;
  }
  if (microscope->state.allowdup)
    if (stm_allow_tcp_duplication_on(outputStream, 5561) == -1)
      fprintf(stderr, "Error setting outstream to duplicate\n");
  if (f && (f)(outputStream)) {
    fprintf(stderr, "Error putting version into output stream\n");
    return -1;
  }

  return 0;
}

int MicroscopeIO::InputStreamPosition (void) const {
  if (!inputStream)
    return 0;

  return inputStream->cur - inputStream->buffer;
}

int MicroscopeIO::OutputStreamPosition (void) const {
  if (!outputStream)
    return 0;

  return outputStream->cur - outputStream->buffer;
}

void MicroscopeIO::GetCurrTime (struct timeval * t) const {
	*t = stream_time;
}

void MicroscopeIO::GetStartTime (struct timeval * t) const {
	*t = stream_start_time;
}

void MicroscopeIO::SkipInputStream (const int _amount) {
  if (!inputStream)
    return;

  // not implemented
  //instream_skip = inputStream->cur - inputStream->buffer + _amount;
  if (_amount < 0) {
    inputStream->cur = inputStream->buffer;
    fprintf(stderr, "Warning:  skipping backwards doesn't do "
                    "what you'd expect.\n");
  } else
    fprintf(stderr, "Warning:  skipping forwards never implemented.\n");
}
  












int MicroscopeIO::GetPacketFromDevice (char * _buf,
                                       const int _bufsize) {
  int retval;
  int socket = SOCKET_TCP;

  switch (comm.SocketType()) {
    case SOCKET_MIX:
    case SOCKET_TCP:
      CHECK(retval = comm.CheckForIncomingPacket(SOCKET_TCP));
      if (comm.SocketType() == SOCKET_TCP) {
        if (!retval) return 0;
        break;
      }
      if (retval) break;  // Read TCP first!
    case SOCKET_UDP:
      CHECK(retval = comm.CheckForIncomingPacket(SOCKET_UDP));
      if (!retval) return 0;
      socket = SOCKET_UDP;
      break;
    default:
      fprintf(stderr, "Illegal value of comm.SocketType() (%d)\n",
              comm.SocketType());
      return -1;
  }

  retval = comm.RecvBuffer(_buf, _bufsize, socket);
  if (!retval)  // end of file, socket closed
    return -1;
  if (retval == -1) {
    perror("MicroscopeIO::GetPacketFromDevice():  "
           "read error on socket");
    return -1;
  }

  return retval;
}

int MicroscopeIO::GetPacketFromStream (char * _buf,
                                       const int /* _bufsize */,
                                       vrpn_bool & _updateStreamTime) {

  int retval;

  VERBOSE(5,"      Checking for packet from stream (before knob check)");

  /* If the read rate is zero, do not read anything */
  if (decoration->rateOfTime == 0) {
    VERBOSE(5,"      Stream rate 0");
    return 0;
  } else {
    VERBOSE(5,"      Comparing time...");
	// if d_next_time comes after d_stream_runtime
    if (time_compare(d_stream_runtime, microscope->d_next_time) < 0) {
      VERBOSE(5,"      Reading block from stream");
      retval = stm_read_block_from_stream(inputStream, _buf);

      _updateStreamTime = VRPN_TRUE;
      // Set stream_time to 0 as a default in case there is
      // no time information in the packet.
      stream_time.tv_sec = stream_time.tv_usec = 0L;
      sec = 0; usec = 0;

      // -1 if end of stream;  otherwise size of block read
      return retval;
    } else {
      VERBOSE(5,"      Not time for new block yet");
      return 0;
    }
  }
}



int MicroscopeIO::Shutdown (void) {
  comm.ClearSendBuffer();
  comm.Buffer(STM_SHUTDOWN);
  return comm.SendBuffer();
}



// microscape.c::handleMouseEvents()
int MicroscopeIO::ResumeWindowScan (void) {
  comm.ClearSendBuffer();
  comm.Buffer(STM_RESUME_WINDOW_SCAN);
  return comm.SendBuffer();
}

int MicroscopeIO::SetRegionNM (const float _minx, const float _miny,
                                      const float _maxx, const float _maxy) {
  char * msgbuf;
  long len;	// Tiger change int to long

  comm.ClearSendBuffer();
  comm.Buffer(STM_SET_REGION_NM);
  msgbuf = microscope->encode_SetRegionNM(&len, _minx, _miny, _maxx, _maxy);
  comm.Buffer(msgbuf, len);
  delete [] msgbuf;
  return comm.SendBuffer();
}



int MicroscopeIO::ScanTo (const float _x, const float _y) {
  char * msgbuf;
  long len;	// Tiger change int to long

  comm.ClearSendBuffer();
  comm.Buffer(STM_SCAN_POINT_NM);
  msgbuf = microscope->encode_ScanTo(&len, _x, _y);
  comm.Buffer(msgbuf, len);
  delete [] msgbuf;
  return comm.SendBuffer();
}

int MicroscopeIO::ScanTo (const float _x, const float _y, const float _z) {
  char * msgbuf;
  long len;	// Tiger change int to long

  comm.ClearSendBuffer();
  comm.Buffer(SPM_XYZ_POINT_NM);
  msgbuf = microscope->encode_ScanTo(&len, _x, _y, _z);
  comm.Buffer(msgbuf, len);
  delete [] msgbuf;
  return comm.SendBuffer();
}

int MicroscopeIO::ZagTo (const float _x, const float _y,
                                const float _yaw,
                                const float _sweepWidth,
                                const float _regionDiag) {
  char * msgbuf;
  long len;	// Tiger change int to long

  comm.ClearSendBuffer();
  comm.Buffer(SPM_ZAG_POINT_NM);
  msgbuf = microscope->encode_ZagTo(&len, _x, _y, _yaw, _sweepWidth, _regionDiag);
  comm.Buffer(msgbuf, len);
  delete [] msgbuf;
  return comm.SendBuffer();
}

int MicroscopeIO::MarkModifyMode (void) {
  comm.ClearSendBuffer();
  comm.Buffer(SPM_ECHO);
  comm.Buffer(1);
  comm.Buffer(AFM_IN_MOD_MODE);
  return comm.SendBuffer();
}

int MicroscopeIO::MarkImageMode (void) {
  comm.ClearSendBuffer();
  comm.Buffer(SPM_ECHO);
  comm.Buffer(1);
  comm.Buffer(AFM_IN_IMG_MODE);
  return comm.SendBuffer();
}

int MicroscopeIO::EnterTappingMode (const float _p, const float _i,
                                           const float _d, const float _set,
                                           const float _amp) {
  char * msgbuf;
  long len;	// Tiger change int to long

  comm.ClearSendBuffer();
  comm.Buffer(AFM_TAPPING_MODE);
  msgbuf = microscope->encode_EnterTappingMode(&len, _p, _i, _d, _set, _amp);
  comm.Buffer(msgbuf, len);
  delete [] msgbuf;
  return comm.SendBuffer();
}

int MicroscopeIO::EnterContactMode (const float _p, const float _i,
                                           const float _d, const float _set) {
  char * msgbuf;
  long len;	// Tiger change int to long

  comm.ClearSendBuffer();
  comm.Buffer(AFM_CONTACT_MODE);
  msgbuf = microscope->encode_EnterContactMode(&len, _p, _i, _d, _set);
  comm.Buffer(msgbuf, len);
  delete [] msgbuf;
  return comm.SendBuffer();
}

int MicroscopeIO::EnterDirectZControl (const float _max_z_step, 
				       const float _max_xy_step, 
				       const float _min_setpoint, 
				       const float _max_setpoint, 
				       const float _max_lateral_force) {
  char * msgbuf;
  long len;	// Tiger change int to long

  comm.ClearSendBuffer();
  comm.Buffer(AFM_DIRECTZ_MODE);
  msgbuf = microscope->encode_EnterDirectZControl(&len, _max_z_step, 
						  _max_xy_step, _min_setpoint, 
						  _max_setpoint, 
						  _max_lateral_force);
  comm.Buffer(msgbuf, len);
  delete [] msgbuf;
  return comm.SendBuffer();
}

int MicroscopeIO::EnterSewingStyle (const float _set, const float _bot,
                                           const float _top, const float _zpull,
                                           const float _punch,
                                           const float _speed,
                                           const float _watchdog) {
  char * msgbuf;
  long len;	// Tiger change int to long

  comm.ClearSendBuffer();
  comm.Buffer(AFM_SEWING_MODE);
  msgbuf = microscope->encode_EnterSewingStyle(&len, _set, _top, _bot,
                       _zpull, _punch, _speed, _watchdog);
  comm.Buffer(msgbuf, len);
  delete [] msgbuf;
  return comm.SendBuffer();
}

int MicroscopeIO::EnterForceCurveStyle (const float _setpoint,
			const float _start_delay, 
			const float _z_start, const float _z_end,
			const float _z_pullback, const float _forcelimit,
			const float _movedist, const int _num_points,
			const int _num_halfcycles,
			const float _sample_speed, const float _pull_speed,
			const float _start_speed, const float _fdback_speed,
			const int _avg_num, const float _sample_delay,
			const float _pull_delay, const float _fdback_delay) {
  char * msgbuf;
  long len;	// Tiger change int to long

  comm.ClearSendBuffer();
  printf("MicroscopeIO::EnterForceCurveStyle\n");
  comm.Buffer(SPM_SPECTROSCOPY_MODE);
  msgbuf = microscope->encode_EnterSpectroscopyMode(&len,_setpoint,
	_start_delay,_z_start,
        _z_end,_z_pullback,_forcelimit,_movedist,_num_points,_num_halfcycles,
	_sample_speed, _pull_speed, _start_speed, _fdback_speed,
	_avg_num, _sample_delay, _pull_delay, _fdback_delay);
  comm.Buffer(msgbuf, len);
  delete [] msgbuf;
  return comm.SendBuffer();

  // _start_delay = delay at z_start (usec)
  // _z_start = distance at which to start acquiring (nm)
  // _z_end = distance at which to stop acquiring (nm)
  // _z_pullback = initial pull back distance (nm)
  // _forcelimit = maximum force at which to stop descent (nA)
  // _movedist = distance between force curves (nm)
  // _num_points = number of different z values to sample at
  // _num_halfcycles = number of down-up curves to acquire per pnt
  // _sample_speed = speed while sampling (um)
  // _pull_speed = speed while pulling back (um)
  // _start_speed = speed going to start pnt (um)
  // _fdback_speed = speed going to feedback pnt (um)
  // _avg_num = # of samples to average
  // _sample_delay = delay before sample (us)
  // _pull_delay = delay after pullback (us)
  // _fdback_delay = delay to establish feedback (us)
}

int MicroscopeIO::EnterScanlineMode(const long _enable) {
  char * msgbuf;
  long len;

  //printf("EnterScanlineMode %ld\n", _enable);
  comm.ClearSendBuffer();
  comm.Buffer(SPM_SCANLINE_MODE);
  msgbuf = microscope->encode_EnterScanlineMode(&len, _enable);
  comm.Buffer(msgbuf, len);
  delete [] msgbuf;
  int res = comm.SendBuffer();
  return res;
}

int MicroscopeIO::RequestScanLine(const float _x, const float _y,const float _z,
                const float _angle, const float _slope,
				const float _width, const long _res,
				const long _enable_feedback, const long _check_forcelimit,
				const float _max_force, const float _max_z_step,
				const float _max_xy_step) {

  char * msgbuf;
  long len;

  //printf("RequestScanLine");
  comm.ClearSendBuffer();
  comm.Buffer(SPM_REQUEST_SCANLINE);
  msgbuf = microscope->encode_RequestScanLine(&len, _x, _y, _z, _angle,
        _slope, _width, _res, _enable_feedback, _check_forcelimit, 
		_max_force, _max_z_step, _max_xy_step);
  comm.Buffer(msgbuf, len);
  delete [] msgbuf;
  return comm.SendBuffer();
}

int MicroscopeIO::SetScanStyle (const int _style) {
  char * msgbuf;
  long len;	// Tiger change int to long

  comm.ClearSendBuffer();
  comm.Buffer(STM_SET_SCAN_STYLE);
  msgbuf = microscope->encode_SetScanStyle(&len, _style);
  comm.Buffer(msgbuf, len);
  delete [] msgbuf;
  return comm.SendBuffer();
}

int MicroscopeIO::SetSlowScan (const int _value) {
  char * msgbuf;
  long len;	// Tiger change int to long

  // If state is identical, don't send message. 
  if (  microscope->state.slowScanEnabled == _value) return 0;

  comm.ClearSendBuffer();
  comm.Buffer(SPM_SET_SLOW_SCAN);
  msgbuf = microscope->encode_SetSlowScan(&len, _value);
  comm.Buffer(msgbuf, len);
  delete [] msgbuf;
  return comm.SendBuffer();
}

int MicroscopeIO::SetStdDelay (const int _delay) {
  char * msgbuf;
  long len;	// Tiger change int to long

  comm.ClearSendBuffer();
  comm.Buffer(AFM_SET_STD_DELAY);
  msgbuf = microscope->encode_SetStdDelay(&len, _delay);
  comm.Buffer(msgbuf, len);
  delete [] msgbuf;
  return comm.SendBuffer();
}

int MicroscopeIO::SetStPtDelay (const int _delay) {
  char * msgbuf;
  long len;	// Tiger change int to long

  comm.ClearSendBuffer();
  comm.Buffer(AFM_SET_STPT_DELAY);
  msgbuf = microscope->encode_SetStPtDelay(&len, _delay);
  comm.Buffer(msgbuf, len);
  delete [] msgbuf;
  return comm.SendBuffer();
}

int MicroscopeIO::SetRelax (const int _min, const int _sep) {
  char * msgbuf;
  long len;	// Tiger change int to long

  comm.ClearSendBuffer();
  comm.Buffer(SPM_SET_RELAX);
  msgbuf = microscope->encode_SetRelax(&len, _min, _sep);
  comm.Buffer(msgbuf, len);
  delete [] msgbuf;
  return comm.SendBuffer();
}


int MicroscopeIO::SetStdDevParams (const int _samples,
                                          const float _freq) {
  char * msgbuf;
  long len;	// Tiger change int to long

  comm.ClearSendBuffer();
  comm.Buffer(STM_SET_STD_DEV_PARAMS);
  msgbuf = microscope->encode_SetStdDevParams(&len, _samples, _freq);
  comm.Buffer(msgbuf, len);
  delete [] msgbuf;
  return comm.SendBuffer();
}


int MicroscopeIO::SetScanWindow (const int _minx, const int _miny,
                                        const int _maxx, const int _maxy) {
  char * msgbuf;
  long len;	// Tiger change int to long

  comm.ClearSendBuffer();
  comm.Buffer(STM_SCAN_WINDOW);
  msgbuf = microscope->encode_SetScanWindow(&len, _minx, _miny, _maxx, _maxy);
  comm.Buffer(msgbuf, len);
  delete [] msgbuf;
  return comm.SendBuffer();
}

int MicroscopeIO::SetGridSize (const int _x, const int _y) {
  char * msgbuf;
  long len;	// Tiger change int to long

  comm.ClearSendBuffer();
  comm.Buffer(STM_SET_GRID_SIZE);
  msgbuf = microscope->encode_SetGridSize(&len, _x, _y);
  comm.Buffer(msgbuf, len);
  delete [] msgbuf;
  return comm.SendBuffer();
}

int MicroscopeIO::SetBias (const float _bias) {
  char * msgbuf;
  long len;	// Tiger change int to long

  comm.ClearSendBuffer();
  comm.Buffer(STM_SET_BIAS);
  msgbuf = microscope->encode_SetBias(&len, _bias);
  comm.Buffer(msgbuf, len);
  delete [] msgbuf;
  return comm.SendBuffer();
}

int MicroscopeIO::SetPulsePeak (const float _height) {
  char * msgbuf;
  long len;	// Tiger change int to long

  comm.ClearSendBuffer();
  comm.Buffer(STM_SET_PULSE_PEAK);
  msgbuf = microscope->encode_SetPulsePeak(&len, _height);
  comm.Buffer(msgbuf, len);
  delete [] msgbuf;
  return comm.SendBuffer();
}

int MicroscopeIO::SetPulseDuration (const float _width) {
  char * msgbuf;
  long len;	// Tiger change int to long

  comm.ClearSendBuffer();
  comm.Buffer(STM_SET_PULSE_DURATION);
  msgbuf = microscope->encode_SetPulseDuration(&len, _width);
  comm.Buffer(msgbuf, len);
  delete [] msgbuf;
  return comm.SendBuffer();
}

int MicroscopeIO::QueryPulseParams (void) {
  comm.ClearSendBuffer();
  comm.Buffer(STM_QUERY_PULSE_PARAMS);
  return comm.SendBuffer();
}

int MicroscopeIO::QueryScanRange (void) {
  comm.ClearSendBuffer();
  comm.Buffer(SPM_QUERY_SCAN_RANGE);
  return comm.SendBuffer();
}

int MicroscopeIO::QueryStdDevParams (void) {
  comm.ClearSendBuffer();
  comm.Buffer(STM_QUERY_STD_DEV_PARAMS);
  return comm.SendBuffer();
}

int MicroscopeIO::SetRateNM (const float _rate) {
  char * msgbuf;
  long len;	// Tiger change int to long

  comm.ClearSendBuffer();
  comm.Buffer(STM_SET_RATE_NMETERS);
  msgbuf = microscope->encode_SetRateNM(&len, _rate);
  comm.Buffer(msgbuf, len);
  delete [] msgbuf;
  return comm.SendBuffer();
}

int MicroscopeIO::SetMaxMove (const float _distance) {
  char * msgbuf;
  long len;	// Tiger change int to long

  comm.ClearSendBuffer();
  comm.Buffer(SPM_SET_MAX_MOVE);
  msgbuf = microscope->encode_SetMaxMove(&len, _distance);
  comm.Buffer(msgbuf, len);
  delete [] msgbuf;
  return comm.SendBuffer();
}

// I found out after I put this in that it DOES NOTHING - 
// the Topometrix AFM's don't respond to this mesage.
// I'm leaving the code in because I think we might need it
// later - now we use "EnterContactMode" and "EnterTappingMode"
// to set the forces - sounds weird to me. ATH 1/22/99
int MicroscopeIO::SetModForce (const float _newforce,
			       const float _min, const float _max) {
  char * msgbuf;
  int len;

  comm.ClearSendBuffer();
  comm.Buffer(AFM_MOD_SET);
  msgbuf = microscope->encode_SetModForce(&len, _min, _max, _newforce);
  comm.Buffer(msgbuf, len);
  delete [] msgbuf;
  return comm.SendBuffer();
}


// Ohmeter code
// (from ohmeter.C and from microscape.c::handleCharacterCommand)
// These 5 function are OBSOLETE - ohmeter has been removed.
// French ohmmeter is still used, though. 
int MicroscopeIO::SetOhmmeterSampleRate (const int _rate) {
  char * msgbuf;
  long len;	// Tiger change int to long

  // hardwired to ohmmeter 0

  comm.ClearSendBuffer();
  comm.Buffer(SPM_MEASURE_RESIST);
  msgbuf = microscope->encode_SetOhmmeterSampleRate(&len, 0, _rate);
  comm.Buffer(msgbuf, len);
  delete [] msgbuf;
  return comm.SendBuffer();
}

int MicroscopeIO::EnableAmp (const int _amp, const float _offset,
                                    const float _uncalOffset,
                                    const int _gain) {
  char * msgbuf;
  long len;	// Tiger change int to long

  comm.ClearSendBuffer();
  comm.Buffer(SPM_ENABLE_AMP);
  msgbuf = microscope->encode_EnableAmp(&len, _amp, _offset, _uncalOffset, _gain);
  comm.Buffer(msgbuf, len);
  delete [] msgbuf;
  return comm.SendBuffer();
}

int MicroscopeIO::DisableAmp (const int _amp) {
  char * msgbuf;
  long len;	// Tiger change int to long

  comm.ClearSendBuffer();
  comm.Buffer(SPM_DISABLE_AMP);
  msgbuf = microscope->encode_DisableAmp(&len, _amp);
  comm.Buffer(msgbuf, len);
  delete [] msgbuf;
  return comm.SendBuffer();
}

int MicroscopeIO::EnableVoltsource (const int _which,
                                           const float _voltage) {
  char * msgbuf;
  long len;	// Tiger change int to long

  comm.ClearSendBuffer();
  comm.Buffer(SPM_ENABLE_VOLTSOURCE);
  msgbuf = microscope->encode_EnableVoltsource(&len, _which, _voltage);
  comm.Buffer(msgbuf, len);
  delete [] msgbuf;
  return comm.SendBuffer();
}

int MicroscopeIO::DisableVoltsource (const int _which) {
  char * msgbuf;
  long len;	// Tiger change int to long

  comm.ClearSendBuffer();
  comm.Buffer(SPM_DISABLE_VOLTSOURCE);
  msgbuf = microscope->encode_DisableVoltsource(&len, _which);
  comm.Buffer(msgbuf, len);
  delete [] msgbuf;
  return comm.SendBuffer();
}



// microscape.c::AFM_Modify_Parameters::do_line()
int MicroscopeIO::DrawSharpLine (const float _startx, const float _starty,
                               const float _endx, const float _endy,
                               const float _stepSize) {
  char * msgbuf;
  long len;	// Tiger change int to long

  comm.ClearSendBuffer();
  comm.Buffer(SPM_SHARP_LINE);
  msgbuf = microscope->encode_DrawSharpLine(& len, _startx, _starty, _endx, _endy, _stepSize);
  comm.Buffer(msgbuf, len);
  delete [] msgbuf;
  return comm.SendBuffer();
}

int MicroscopeIO::DrawSweepLine (const float _startx, const float _starty,
                               const float _endx, const float _endy,
                               const float _yaw, const float _sweepWidth,
                               const float _stepSize) {
  char * msgbuf;
  long len;	// Tiger change int to long

  comm.ClearSendBuffer();
  comm.Buffer(SPM_SWEEP_LINE);
  msgbuf = microscope->encode_DrawSweepLine(&len, _startx, _starty, _yaw, _sweepWidth, _endx, _endy, _yaw, _sweepWidth, _stepSize);
  comm.Buffer(msgbuf, len);
  delete [] msgbuf;
  return comm.SendBuffer();
}

// microscape.c::AFM_Modify_Parameters::do_arc()
int MicroscopeIO::DrawSweepArc (const float _x, const float _y,
                              const float _startAngle, const float _endAngle,
                              const float _sweepWidth,
                              const float _stepSize) {
  char * msgbuf;
  long len;	// Tiger change int to long

  comm.ClearSendBuffer();
  comm.Buffer(SPM_SWEEP_ARC);
  msgbuf = microscope->encode_DrawSweepArc(&len, _x, _y, _startAngle, _sweepWidth, _endAngle, _sweepWidth, _stepSize);
  comm.Buffer(msgbuf, len);
  delete [] msgbuf;
  return comm.SendBuffer();

}


// active_set.C
int MicroscopeIO::GetNewPointDatasets (const Tclvar_checklist_with_entry * _list) {
  char namebuf [STM_NAME_LENGTH];
  int numSets = 0;
  int i;

  comm.ClearSendBuffer();
  comm.Buffer(SPM_REQUEST_POINT_DATASETS);

  for (i = 0; i < _list->Num_checkboxes(); i++)
    if (1 == _list->Is_set(i))
      numSets++;

  comm.Buffer(numSets);

  for (i = 0; i < _list->Num_checkboxes(); i++)
    if (1 == _list->Is_set(i)) {
      strncpy(namebuf, _list->Checkbox_name(i), STM_NAME_LENGTH);
      comm.Buffer(namebuf, STM_NAME_LENGTH);

      // ask for 10 samples of each,
      // except for Topography, for which we want  90
//       if (!strcmp("Topography", namebuf))
//         comm.Buffer(90);
//       else
//         comm.Buffer(10);
      // change so that these values can be set by the user.
      // Look up the value in the entry field of this checklist
      comm.Buffer((int)(_list->Get_entry_val(_list->Checkbox_name(i))));
    }

  return comm.SendBuffer();
}

// active_set.C
int MicroscopeIO::GetNewScanDatasets (const Tclvar_checklist_with_entry * _list) {
  char namebuf [STM_NAME_LENGTH];
  int numSets = 0;
  int i;

  comm.ClearSendBuffer();
  comm.Buffer(SPM_REQUEST_SCAN_DATASETS);

  for (i = 0; i < _list->Num_checkboxes(); i++)
    if (1 == _list->Is_set(i))
      numSets++;

  comm.Buffer(numSets);

  for (i = 0; i < _list->Num_checkboxes(); i++)
    if (1 == _list->Is_set(i)) {
      strncpy(namebuf, _list->Checkbox_name(i), STM_NAME_LENGTH);
      comm.Buffer(namebuf, STM_NAME_LENGTH);
    }

  return comm.SendBuffer();
}

int MicroscopeIO::DispatchPacket (char * buffer, const int bs,
                                  const vrpn_bool _enableQueueing) {

  int dataType;

  // While the buffer has not been used up, read more reports

  VERBOSE(5, "      Parsing the reports");
  bufptr = buffer;
  while ((bufptr - buffer) < bs) {

    microscope->state.dlistchange = VRPN_FALSE;

    comm.Unbuffer(bufptr, dataType);
    //printf("message type: %d\n", dataType);
    switch (dataType) {
      case AFM_IN_TAPPING_MODE:
        RcvInTappingMode(&bufptr);  break;
      case AFM_IN_CONTACT_MODE:
        RcvInContactMode(&bufptr);  break;
      case AFM_IN_SEWING_MODE:
        RcvInSewingStyle(&bufptr);  break;
      case SPM_IN_SPECTROSCOPY_MODE:
	RcvInSpectroscopyMode(&bufptr); break;
      case AFM_FORCE_PARAMETERS:
        RcvForceParameters(&bufptr);  break;
      case AFM_BASE_MOD_PARAMETERS:
        RcvBaseModParameters(&bufptr);  break;
      case AFM_FORCE_SETTINGS:
        RcvForceSettings(&bufptr);  break;
      case AFM_MOD_FORCE_SET:
        RcvModForceSet(&bufptr);  break;
      case AFM_IMG_FORCE_SET:
        RcvImgForceSet(&bufptr);  break;
      case AFM_MOD_SET:
        RcvModSet(&bufptr);  break;
      case AFM_IMG_SET:
        RcvImgSet(&bufptr);  break;
      case AFM_FORCE_SET:
        RcvForceSet(&bufptr);  break;
      case AFM_IMG_FORCE_SET_FAILURE:
      case AFM_MOD_FORCE_SET_FAILURE:
      case AFM_FORCE_SET_FAILURE:
        RcvForceSetFailure(&bufptr);  break;
      case AFM_IN_MOD_MODE_T:  // obsolete
        RcvInModModeT(&bufptr);  break;
      case AFM_IN_MOD_MODE:
        RcvInModMode(&bufptr);  break;
      case AFM_IN_IMG_MODE_T:  // obsolete
        RcvInImgModeT(&bufptr);  break;
      case AFM_IN_IMG_MODE:
        RcvInImgMode(&bufptr);  break;

      case STM_PULSE_PARAMETERS:
        RcvPulseParameters(&bufptr);  break;
      case STM_PULSE_COMPLETED_NM:
        RcvPulseCompletedNM(&bufptr);  break;
      case STM_PULSE_FAILURE_NM:
        RcvPulseFailureNM(&bufptr);  break;

      case SPM_VOLTSOURCE_ENABLED:
        RcvVoltsourceEnabled(&bufptr);  break;
      case SPM_VOLTSOURCE_DISABLED:
        RcvVoltsourceDisabled(&bufptr);  break;
      case SPM_AMP_ENABLED:
        RcvAmpEnabled(&bufptr);  break;
      case SPM_AMP_DISABLED:
        RcvAmpDisabled(&bufptr);  break;
      case SPM_STARTING_TO_RELAX:
        RcvStartingToRelax(&bufptr);  break;
      case SPM_RELAX_SET:
        RcvRelaxSet(&bufptr);  break;
      case STM_STD_DEV_PARAMETERS:
        RcvStdDevParameters(&bufptr);  break;

      case SPM_WINDOW_LINE_DATA:
        // Handle Michele's queueing of window line data
        // ASSUMES that WINDOW_LINE_DATA is the LAST message
        // in a packet;  other messages sent after it may be delayed.
        if (_enableQueueing &&
            EnqueueWindowLineData(dataType, bufptr,
                                  bs - (bufptr - buffer))) {
          // skip to next packet;  don't play anything
          bufptr = buffer + bs;
//fprintf(stderr, "Enqueued packet;  set bufptr to beyond end of packet\n");
          break;
        }
        RcvWindowLineData(&bufptr);  break;

      case STM_WINDOW_SCAN_NM:
        RcvWindowScanNM(&bufptr);  break;
      case STM_WINDOW_BACKSCAN_NM:
        RcvWindowBackscanNM(&bufptr);  break;
      case STM_POINT_RESULT_NM:
        RcvPointResultNM(&bufptr);  break;
      case SPM_POINT_RESULT_DATA:
      case SPM_BOTTOM_PUNCH_RESULT_DATA:
      case SPM_TOP_PUNCH_RESULT_DATA: 
        RcvResultData(dataType, &bufptr);  break;
      case STM_ZIG_RESULT_NM:
      case SPM_BLUNT_RESULT_NM:
        RcvResultNM(&bufptr);  break;
      case SPM_FORCE_CURVE_DATA:
	RcvForceCurveData(&bufptr); break;
      case SPM_SCAN_RANGE:
        RcvScanRange(&bufptr);  break;
      case STM_SET_REGION_COMPLETED:
      case STM_SET_REGION_CLIPPED:
        RcvSetRegionC(dataType, &bufptr);  break;
      case SPM_RESISTANCE_FAILURE:
        RcvResistanceFailure(&bufptr);  break;
      case SPM_RESISTANCE:
        RcvResistance(&bufptr);  break;
      case OHM_RESISTANCE:
	RcvResistance2(&bufptr); break;
	  case OHM_RESISTANCE_WSTATUS:
		RcvResistanceWithStatus(&bufptr); break;
      case SPM_REPORT_SLOW_SCAN:
        RcvReportSlowScan(&bufptr);  break;
      case STM_SCAN_PARAMETERS:
        RcvScanParameters(&bufptr);  break;
      case SPM_HELLO_MESSAGE:
        RcvHelloMessage(&bufptr);  break;
      case SPM_CLIENT_HELLO:
        RcvClientHello(&bufptr);  break;
      case SPM_PID_PARAMETERS:
        RcvPidParameters(&bufptr);  break;
      case SPM_SCANRATE_PARAMETER:
        RcvScanrateParameter(&bufptr);  break;
      case SPM_REPORT_GRID_SIZE:
        RcvReportGridSize(&bufptr);  break;
      case SPM_SERVER_PACKET_TIMESTAMP:
        RcvServerPacketTimestamp(&bufptr);  break;
      case SPM_TOPO_FILE_HEADER:
        RcvTopoFileHeader(&bufptr);  break;
      case SPM_VISIBLE_TRAIL:
        RcvInModMode(&bufptr);  break;
      case SPM_INVISIBLE_TRAIL:
        RcvInImgMode(&bufptr);  break;

        // OBSOLETE
      case SPM_SNAP_SHOT_BEGIN:
        // Prepare to wind the stream back to the point
        // immediately before this snapshot began
        //streamCutPoint = bufptr - sizeof(int);
        fprintf(stderr, "SnapShots were declared obsolete 24 Sept 97.\n");
        break;
      case SPM_SNAP_SHOT_END:
        fprintf(stderr, "SnapShots were declared obsolete 24 Sept 97.\n");
        break;


      case SPM_SCAN_DATASETS:
        RcvScanDatasets(&bufptr);  break;
      case SPM_POINT_DATASETS:
        RcvPointDatasets(&bufptr);  break;
      case SPM_IN_SCANLINE_MODE:
        RcvInScanlineMode(&bufptr); break;
      case SPM_SCANLINE_DATA:
        RcvScanlineData(&bufptr); break;

      case NANO_RECV_TIMESTAMP:
        RcvRecvTimestamp(&bufptr);  break;
      case FAKE_SPM_SEND_TIMESTAMP:
        RcvFakeSendTimestamp(&bufptr);  break;
      case SPM_UDP_SEQ_NUM:
        RcvUdpSeqNum(&bufptr);  break;

      default:
        fprintf(stderr, "MicroscopeIO::HandlePacket():  "
                        "Unknown data type %d\n", dataType);
        return -1;
    }
  }

  return VRPN_TRUE;
}




#if 0
void MicroscopeIO::GetTimestamp (char * & _bufptr, int & _sec, int & _usec) {

  comm.Unbuffer(_bufptr, _sec);
  comm.Unbuffer(_bufptr, _usec);

  sec = _sec;
  usec = _usec;
}

void MicroscopeIO::GetRasterPosition (char * & _bufptr, int & _x, int & _y) {
  comm.Unbuffer(_bufptr, _x);
  comm.Unbuffer(_bufptr, _y);
  microscope->GetRasterPosition(_x, _y);
}
#endif





void MicroscopeIO::RcvInTappingMode (char ** _bufptr) {
  float P, I, D, setPoint, driveAmp;

  microscope->decode_InTappingMode(UGLYCAST _bufptr, &P, &I, &D, &setPoint, &driveAmp);
  microscope->RcvInTappingMode(P, I, D, setPoint, driveAmp);
}

void MicroscopeIO::RcvInContactMode (char ** _bufptr) {
  float P, I, D, setPoint;

  microscope->decode_InContactMode(UGLYCAST _bufptr, &P, &I, &D, &setPoint);
  microscope->RcvInContactMode(P, I, D, setPoint);
}

void MicroscopeIO::RcvInSewingStyle (char ** _bufptr) {
  float setPoint, bottomDelay, topDelay, pbDist, mDist, rate, mSafe;

  microscope->decode_InSewingStyle(UGLYCAST _bufptr, &setPoint, &bottomDelay, &topDelay,
              &pbDist, &mDist, &rate, &mSafe);
  microscope->RcvInSewingStyle(setPoint, bottomDelay, topDelay, pbDist,
                              mDist, rate, mSafe);
}

void MicroscopeIO::RcvInSpectroscopyMode (char ** _bufptr) {
  float setpoint;
  float startDelay, zStart, zEnd, zPullback, forceLimit, distBetweenFC;
  long numPoints, numHalfcycles;	// Tiger changed int to long
  float sampleSpeed, pullbackSpeed, startSpeed, feedbackSpeed;
  long avgNum;	// Tiger changed int to long
  float sampleDelay, pullbackDelay, feedbackDelay;

  microscope->decode_InSpectroscopyMode(UGLYCAST _bufptr, &setpoint, 
	&startDelay, &zStart, &zEnd, &zPullback, &forceLimit, &distBetweenFC,
	&numPoints, &numHalfcycles,
	&sampleSpeed, &pullbackSpeed, &startSpeed, &feedbackSpeed,
	&avgNum, &sampleDelay, &pullbackDelay, &feedbackDelay);
  microscope->RcvInSpectroscopyMode(setpoint, startDelay, zStart, zEnd, 
	zPullback, forceLimit, distBetweenFC, numPoints, numHalfcycles,
	sampleSpeed, pullbackSpeed, startSpeed, feedbackSpeed,
	avgNum, sampleDelay, pullbackDelay, feedbackDelay);
}

void MicroscopeIO::RcvForceParameters (char ** _bufptr) {
  long enableModify;	// Tiger changed int to long
  float fscrap;

  microscope->decode_ForceParameters(UGLYCAST _bufptr, &enableModify, &fscrap);
  microscope->RcvForceParameters(enableModify, fscrap);
}

void MicroscopeIO::RcvBaseModParameters (char ** _bufptr) {
  float min, max;

  microscope->decode_BaseModParameters(UGLYCAST _bufptr, &min, &max);
  microscope->RcvBaseModParameters(min, max);
}

void MicroscopeIO::RcvForceSettings (char ** _bufptr) {
  float min, max, setpoint;

  microscope->decode_ForceSettings(UGLYCAST _bufptr, &min, &max, &setpoint);
  microscope->RcvForceSettings(min, max, setpoint);
}

void MicroscopeIO::RcvVoltsourceEnabled (char ** _bufptr) {
  long vmNum;	// Tiger changed int to long
  float voltage;

  microscope->decode_VoltsourceEnabled(UGLYCAST _bufptr, &vmNum, &voltage);
  microscope->RcvVoltsourceEnabled(vmNum, voltage);
}

void MicroscopeIO::RcvVoltsourceDisabled (char ** _bufptr) {
  long vmNum;	// Tiger changed int to long

  microscope->decode_VoltsourceDisabled(UGLYCAST _bufptr, &vmNum);
  microscope->RcvVoltsourceDisabled(vmNum);
}

void MicroscopeIO::RcvAmpEnabled (char ** _bufptr) {
  long aNum, gainMode;	// Tiger changed int to long
  float offset, percentOffset;

  microscope->decode_AmpEnabled(UGLYCAST _bufptr, &aNum, &offset, &percentOffset, &gainMode);
  microscope->RcvAmpEnabled(aNum, offset, percentOffset, gainMode);
}

void MicroscopeIO::RcvAmpDisabled (char ** _bufptr) {
  long aNum;	// Tiger changed int to long

  microscope->decode_AmpDisabled(UGLYCAST _bufptr, &aNum);
  microscope->RcvAmpDisabled(aNum);
}

void MicroscopeIO::RcvStartingToRelax (char ** _bufptr) {
  long sec, usec;	// Tiger changed int to long

  microscope->decode_StartingToRelax(UGLYCAST _bufptr, &sec, &usec);
  this->sec = sec;
  this->usec = usec;
  microscope->RcvStartingToRelax(sec, usec);
}

void MicroscopeIO::RcvInModModeT (char ** _bufptr) {
  long sec, usec;	// Tiger changed int to long

  microscope->decode_InModModeT(UGLYCAST _bufptr, &sec, &usec);
  this->sec = sec;
  this->usec = usec;
  microscope->RcvInModModeT(sec, usec);
}

// case AFM_IN_MOD_MODE:
// case SPM_VISIBLE_TRAIL:
void MicroscopeIO::RcvInModMode (char **) {
  microscope->RcvInModMode();
}

void MicroscopeIO::RcvInImgModeT (char ** _bufptr) {
  long sec, usec;	// Tiger changed int to long

  microscope->decode_InImgModeT(UGLYCAST _bufptr, &sec, &usec);
  this->sec = sec;
  this->usec = usec;
  microscope->RcvInImgModeT(sec, usec);
}

// case AFM_IN_IMG_MODE:
// case SPM_INVISIBLE_TRAIL:
void MicroscopeIO::RcvInImgMode (char **) {
  microscope->RcvInImgMode();
}

void MicroscopeIO::RcvModForceSet (char ** _bufptr) {
  float setpoint;

  microscope->decode_ModForceSet(UGLYCAST _bufptr, &setpoint);
  microscope->RcvModForceSet(setpoint);
}

void MicroscopeIO::RcvImgForceSet (char ** _bufptr) {
  float setpoint;

  microscope->decode_ImgForceSet(UGLYCAST _bufptr, &setpoint);
  microscope->RcvImgForceSet(setpoint);
}

void MicroscopeIO::RcvModSet (char ** _bufptr) {
  long enableModify;	// Tiger changed int to long
  float min, max, setpoint;

  microscope->decode_ModSet(UGLYCAST _bufptr, &enableModify, &max, &min, &setpoint);
  microscope->RcvModSet(enableModify, max, min, setpoint);
}

void MicroscopeIO::RcvImgSet (char ** _bufptr) {
  long enableModify;	// Tiger changed int to long
  float min, max, setpoint;

  microscope->decode_ImgSet(UGLYCAST _bufptr, &enableModify, &max, &min, &setpoint);
  microscope->RcvImgSet(enableModify, max, min, setpoint);
}

void MicroscopeIO::RcvRelaxSet (char ** _bufptr) {
  long min, sep;	// Tiger changed int to long

  microscope->decode_RelaxSet(UGLYCAST _bufptr, &min, &sep);
  microscope->RcvRelaxSet(min, sep);
}

void MicroscopeIO::RcvForceSet (char ** _bufptr) {
  float fscrap;

  microscope->decode_ForceSet(UGLYCAST _bufptr, &fscrap);
  microscope->RcvForceSet(fscrap);
}

// case AFM_IMG_FORCE_SET_FAILURE:
// case AFM_MOD_FORCE_SET_FAILURE:
// case AFM_FORCE_SET_FAILURE:
void MicroscopeIO::RcvForceSetFailure (char ** _bufptr) {
  float fscrap;

  microscope->decode_ForceSetFailure(UGLYCAST _bufptr, &fscrap);
  microscope->RcvForceSetFailure(fscrap);
}

void MicroscopeIO::RcvPulseParameters (char ** _bufptr) {
  long enablePulse;	// Tiger changed int to long
  float bias, peak, width;

  microscope->decode_PulseParameters(UGLYCAST _bufptr, &enablePulse, &bias, &peak, &width);
  microscope->RcvPulseParameters(enablePulse, bias, peak, width);
}

void MicroscopeIO::RcvStdDevParameters (char ** _bufptr) {
  long samples;	// Tiger changed int to long
  float frequency;

  microscope->decode_StdDevParameters(UGLYCAST _bufptr, &samples, &frequency);
  microscope->RcvStdDevParameters(samples, frequency);
}

void MicroscopeIO::RcvWindowLineData (char ** _bufptr) {
  long x, y, dx, dy, lineCount, fieldCount, sec, usec, i;	// Tiger changed int to long
  float fields [MAX_CHANNELS];

  microscope->decode_WindowLineDataHeader(UGLYCAST _bufptr, &x, &y, &dx, &dy, &lineCount, &fieldCount, &sec, &usec);
  microscope->GetRasterPosition(x, y);
  this->sec = sec;
  this->usec = usec;

  for (i = 0; i < lineCount; i++) {
    microscope->decode_WindowLineDataField(UGLYCAST _bufptr, fieldCount, fields);
    CHECKvoid(microscope->RcvWindowLineData(x + dx * i, y + dy * i, sec, usec,
                                            fieldCount, fields));
  }

  microscope->RcvWindowLineData();
}

void MicroscopeIO::RcvWindowScanNM (char ** _bufptr) {
  long x, y, sec, usec;	// Tiger changed int to long
  float value, deviation;

  microscope->decode_WindowScanNM(UGLYCAST _bufptr, &x, &y, &sec, &usec, &value, &deviation);
  microscope->GetRasterPosition(x, y);
  this->sec = sec;
  this->usec = usec;
  microscope->RcvWindowScanNM(x, y, sec, usec, value, deviation);
}

void MicroscopeIO::RcvWindowBackscanNM (char ** _bufptr) {
  long x, y, sec, usec;	// Tiger changed int to long
  float value, deviation;

  microscope->decode_WindowBackscanNM(UGLYCAST _bufptr, &x, &y, &sec, &usec, &value, &deviation);
  microscope->GetRasterPosition(x, y);
  this->sec = sec;
  this->usec = usec;
  microscope->RcvWindowBackscanNM(x, y, sec, usec, value, deviation);
}

void MicroscopeIO::RcvPointResultNM (char ** _bufptr) {
  float x, y, height, deviation;
  long sec, usec;	// Tiger changed int to long

  microscope->decode_PointResultNM(UGLYCAST _bufptr, &x, &y, &sec, &usec, &height, &deviation);
  this->sec = sec;
  this->usec = usec;
  microscope->RcvPointResultNM(x, y, sec, usec, height, deviation);
}

// case SPM_POINT_RESULT_DATA:
// case SPM_BOTTOM_PUNCH_RESULT_DATA:
// case SPM_TOP_PUNCH_RESULT_DATA: {
void MicroscopeIO::RcvResultData (const int _type, char ** _bufptr) {
  float x, y, fields [MAX_CHANNELS];
  long sec, usec, fieldCount;	// Tiger changed int to long

  microscope->decode_ResultData(UGLYCAST _bufptr, &x, &y, &sec, &usec, &fieldCount, fields);
  this->sec = sec;
  this->usec = usec;
  microscope->RcvResultData(_type, x, y, sec, usec, fieldCount, fields);
}

// case STM_ZIG_RESULT_NM:
// case SPM_BLUNT_RESULT_NM:
void MicroscopeIO::RcvResultNM (char ** _bufptr) {
  float x, y, height, normX, normY, normZ;
  long sec, usec;	// Tiger changed int to long

  microscope->decode_ResultNM(UGLYCAST _bufptr, &x, &y, &sec, &usec, &height, &normX, &normY, &normZ);
  this->sec = sec;
  this->usec = usec;
  microscope->RcvResultNM(x, y, sec, usec, height, normX, normY, normZ);
}

void MicroscopeIO::RcvPulseCompletedNM (char ** _bufptr) {
  float x, y;

  microscope->decode_PulseCompletedNM(UGLYCAST _bufptr, &x, &y);
  microscope->RcvPulseCompletedNM(x, y);
}

void MicroscopeIO::RcvPulseFailureNM (char ** _bufptr) {
  float x, y;

  microscope->decode_PulseFailureNM(UGLYCAST _bufptr, &x, &y);
  microscope->RcvPulseFailureNM(x, y);
}

void MicroscopeIO::RcvScanRange (char ** _bufptr) {
  float minX, minY, maxX, maxY, minZ, maxZ;

  microscope->decode_ScanRange(UGLYCAST _bufptr, &minX, &maxX, &minY, &maxY, &minZ, &maxZ);
  microscope->RcvScanRange(minX, maxX, minY, maxY, minZ, maxZ);
}

void MicroscopeIO::RcvSetRegionC (const int _type, char ** _bufptr) {
  float minX, minY, maxX, maxY;

  microscope->decode_SetRegionC(UGLYCAST _bufptr, &minX, &minY, &maxX, &maxY);
  microscope->RcvSetRegionC(_type, minX, minY, maxX, maxY);
}

void MicroscopeIO::RcvResistanceFailure (char ** _bufptr) {
  long which;	// Tiger changed int to long

  microscope->decode_ResistanceFailure(UGLYCAST _bufptr, &which);
  microscope->RcvResistanceFailure(which);
}

void MicroscopeIO::RcvResistance (char ** _bufptr) {
  long which, sec, usec;	// Tiger changed int to long
  float resistance;

  microscope->decode_Resistance(UGLYCAST _bufptr, &which, &sec, &usec, &resistance);
  this->sec = sec;
  this->usec = usec;
  microscope->RcvResistance(which, sec, usec, resistance);
}

void MicroscopeIO::RcvResistance2 (char ** _bufptr) {
  long which, sec, usec;	// Tiger changed int to long
  float resistance;
  float voltage, range, filter;

  microscope->decode_Resistance2(UGLYCAST _bufptr, &which, &sec, &usec, &resistance, &voltage, &range, &filter);
	if ((this->sec != 0) || (this->usec != 0)){
	this->sec = sec;
	this->usec = usec;
	}
  	microscope->RcvResistance2(which, sec, usec, resistance, 
		voltage, range, filter);
}


void MicroscopeIO::RcvResistanceWithStatus (char ** _bufptr) {
  long which, sec, usec;    // Tiger changed int to long
  float resistance;
  float voltage, range, filter;
  long status;

  microscope->decode_ResistanceWithStatus(UGLYCAST _bufptr, &which, 
	&sec, &usec, &resistance, &voltage, &range, &filter, &status);
  this->sec = sec;
  this->usec = usec;
  microscope->RcvResistanceWithStatus(which, sec, usec, 
		resistance, voltage, range, filter, status);
}

void MicroscopeIO::RcvReportSlowScan (char ** _bufptr) {
  long isEnabled;	// Tiger changed int to long

  microscope->decode_ReportSlowScan(UGLYCAST _bufptr, &isEnabled);
  microscope->RcvReportSlowScan(isEnabled);
}

// Don't understand this but have to implement something
// to make sure _bufptr is advanced correctly

void MicroscopeIO::RcvScanParameters (char ** _bufptr) {
  long length, i;	// Tiger changed int to long
  char * buffer, * bp;

  microscope->decode_ScanParameters(UGLYCAST _bufptr, &length, &buffer);
  printf("Scan Parameters:  length = %d\n", length);

  bp = buffer;

  for (i = 0; bp && (i < length); i++) {
    if (!*bp)
      printf("\n");
    else
      printf("%c", *bp);
    bp += sizeof(char);
  }

  microscope->RcvScanParameters((const char **) bp);

  if (buffer)
    delete [] buffer;
}

void MicroscopeIO::RcvHelloMessage (char ** _bufptr) {
  char magic [4];
  char name [STM_NAME_LENGTH];
  long minorVersion;	// Tiger changed int to long
  long majorVersion;	// Tiger changed int to long

  microscope->decode_HelloMessage(UGLYCAST _bufptr, magic, name,
                                  &minorVersion, &majorVersion);
  microscope->RcvHelloMessage(magic, name, majorVersion, minorVersion);
}

void MicroscopeIO::RcvClientHello (char ** _bufptr) {
  char magic [4];
  char name [STM_NAME_LENGTH];
  long minorVersion;	// Tiger changed int to long
  long majorVersion;	// Tiger changed int to long

  microscope->decode_ClientHello(UGLYCAST _bufptr, magic, name,
                                  &minorVersion, &majorVersion);
  microscope->RcvClientHello(magic, name, majorVersion, minorVersion);
}

void MicroscopeIO::RcvScanDatasets (char ** _bufptr) {
  long count, i;	// Tiger changed int to long
  char name [STM_NAME_LENGTH];
  char units [STM_NAME_LENGTH];
  float offset;
  float scale;

  microscope->decode_ScanDatasetHeader(UGLYCAST _bufptr, &count);

  microscope->RcvClearScanChannels();
  printf("New scan datasets (%d) from microscope:\n", count);
  for (i = 0; i < count; i++) {
    microscope->decode_ScanDataset(UGLYCAST _bufptr, name, units, &offset, &scale);
    microscope->RcvScanDataset(name, units, offset, scale);
  }

  ready = VRPN_TRUE;
//fprintf(stderr, "Set MicroscopeIO::ready to VRPN_TRUE\n");
}

void MicroscopeIO::RcvPointDatasets (char ** _bufptr) {
  long count, i;	// Tiger changed int to long
  char name [STM_NAME_LENGTH];
  char units [STM_NAME_LENGTH];
  long numSamples;	// Tiger changed int to long
  float offset;
  float scale;

  microscope->decode_PointDatasetHeader(UGLYCAST _bufptr, &count);

  microscope->RcvClearPointChannels();
  printf("New point datasets from microscope:\n");
  for (i = 0; i < count; i++) {
    microscope->decode_PointDataset(UGLYCAST _bufptr, name, units, &numSamples,
                                    &offset, &scale);
    microscope->RcvPointDataset(name, units, numSamples, offset, scale);
  }
}

void MicroscopeIO::RcvInScanlineMode (char ** _bufptr) {
  long enable;

  microscope->decode_InScanlineMode(UGLYCAST _bufptr,
        &enable);

  microscope->RcvInScanlineMode(enable);
}

void MicroscopeIO::RcvScanlineData (char ** _bufptr) {
  long i;
  long sec, usec;
  float x, y, z, angle, slope;
  float width;
  long resolution;
  long feedback_enabled, checking_forcelimit;
  float max_force_setting, max_z_step, max_xy_step;
  long num_channels;
  float fields [MAX_CHANNELS];

  microscope->decode_ScanlineDataHeader(UGLYCAST _bufptr, &x, &y, &z, &angle,
        &slope, &width, &resolution, &feedback_enabled, &checking_forcelimit,
		&max_force_setting, &max_z_step, &max_xy_step, &sec, &usec,
		&num_channels);

  this->sec = sec;
  this->usec = usec;

  microscope->RcvScanlineDataHeader(x, y, z, angle, slope, width, resolution,
		feedback_enabled, checking_forcelimit, max_force_setting, 
		max_z_step, max_xy_step, sec, usec, num_channels);

  for (i = 0; i < resolution; i++) {
    microscope->decode_ScanlineDataPoint(UGLYCAST _bufptr, num_channels,fields);
    microscope->RcvScanlineData(i, num_channels, fields);
  }
}


void MicroscopeIO::RcvPidParameters (char ** _bufptr) {
  float p, i, d;

  microscope->decode_PidParameters(UGLYCAST _bufptr, &p, &i, &d);
  microscope->RcvPidParameters(p, i, d);
}

void MicroscopeIO::RcvScanrateParameter (char ** _bufptr) {
  float rate;

  microscope->decode_ScanrateParameter(UGLYCAST _bufptr, &rate);
  microscope->RcvScanrateParameter(rate);
}

void MicroscopeIO::RcvReportGridSize (char ** _bufptr) {
  long x, y;	// Tiger changed int to long

  microscope->decode_ReportGridSize(UGLYCAST _bufptr, &x, &y);
  microscope->RcvReportGridSize(x, y);
}

void MicroscopeIO::RcvServerPacketTimestamp (char ** _bufptr) {
  long sec, usec;	// Tiger changed int to long

  microscope->decode_ServerPacketTimestamp(UGLYCAST _bufptr, &sec, &usec);
  this->sec = sec;
  this->usec = usec;
  microscope->RcvServerPacketTimestamp(sec, usec);
}

void MicroscopeIO::RcvTopoFileHeader (char ** _bufptr) {
  long length;	// Tiger changed int to long
  //static char * header = NULL;
  char * buffer;

  microscope->decode_TopoFileHeader(UGLYCAST _bufptr, &length, &buffer);
  microscope->RcvTopoFileHeader(length, buffer);

  if (buffer)
    delete [] buffer;
}


// Allocates memory for force curves and stores force curves in it.
// If memory allocation fails, the message can still be unbuffered without
// a problem,
void MicroscopeIO::RcvForceCurveData (char ** _bufptr) {
  float x, y;
  long num_points, num_halfcycles, sec, usec, i, j;	// Tiger changed int to long
  float ** curves = NULL;	// one for each halfcycle
  float * z_values;		// one for each point
  float z;
  float force [FC_MAX_HALFCYCLES];
  int mem_err = 0;

//  printf("starting force curve decoding\n");
  microscope->decode_ForceCurveDataHeader(UGLYCAST _bufptr, &x, &y, &num_points,
	&num_halfcycles, &sec, &usec);

//  printf("got header: x=%f,y=%f,layers=%d,halfcyc=%d\n",x,y,num_points,
//		num_halfcycles);
  curves = (float **)malloc(num_halfcycles*sizeof(float *));
  if (!curves) mem_err = 1;
  else {
    for (i = 0; i < num_halfcycles; i++){
      curves[i] = (float *)malloc(num_points*sizeof(float));
      if (!curves[i]) mem_err = 1;
    }
  }
  z_values = new float[num_points];
  if (!z_values) mem_err = 1;


  //microscope->GetRasterPosition(x, y);
  this->sec = sec;
  this->usec = usec;

  for (i = 0; i < num_points; i++) {
    microscope->decode_ForceCurveDataSingleLevel(UGLYCAST _bufptr, 
	num_halfcycles, &z, force);
//    printf("got pnt: %d, z=%f",i, z);
    if (!mem_err){
      z_values[i] = z;
      for (j = 0; j < num_halfcycles; j++){
	  curves[j][i] = force[j];
//	  printf(",f=%f", force[j]);
      }
    }
//    printf("\n");
  }
  if (mem_err){
    fprintf(stderr, "nmm_Microscope::RcvForceCurveData:  "
		    "Out of memory.\n");
  }
  else {
    microscope->RcvForceCurveData(x, y, sec, usec, num_points, num_halfcycles,
				z_values, (const float **) curves);
  }
//  printf("decoded force curve data successfully\n");
}

void MicroscopeIO::RcvRecvTimestamp (char ** _bufptr) {

  // stored on MicroscopeIO so that at the end of packet
  // we can use this instead of the normal value when
  // updateStreamTime is true

  microscope->decode_RecvTimestamp(UGLYCAST _bufptr, &temp_time);
  microscope->RcvRecvTimestamp(temp_time);
}

void MicroscopeIO::RcvFakeSendTimestamp (char ** _bufptr) {
  struct timeval t;

  microscope->decode_FakeSendTimestamp(UGLYCAST _bufptr, &t);
  microscope->RcvFakeSendTimestamp(t);
}

void MicroscopeIO::RcvUdpSeqNum (char ** _bufptr) {
  long sn;	// Tiger changed int to long

  microscope->decode_UdpSeqNum(UGLYCAST _bufptr, &sn);
  microscope->RcvUdpSeqNum(sn);
}






int MicroscopeIO::EnqueueWindowLineData (const int _dataType,
                                         char * & _bufptr,
                                         const int _size) {
  char * tp;

  if (comm.SocketType() != SOCKET_MIX) return 0;

  if ((queueLength > 5) && ready) {
    // play something from the data queue
    DispatchPacket(dataQ[queueHead].data,
                   dataQ[queueHead].len,
                   VRPN_FALSE);
    queueHead++;  queueHead %= 20;
    queueLength--;
  }

  queueTail++;  queueTail %= 20;
  tp = dataQ[queueTail].data;
  memcpy(tp, &_dataType, sizeof(int));
  tp += sizeof(int);
  memcpy(tp, _bufptr, _size);
  dataQ[queueTail].len = _size + sizeof(int);
  queueLength++;

  return 1;
}


