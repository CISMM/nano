#ifndef NMM_MICROSCOPE_TRANSLATOR_H
#define NMM_MICROSCOPE_TRANSLATOR_H

#include "nmm_Microscope.h"
#include "nmb_SharedDevice.h"
#include "stm_file.h"

/* 
  nmm_Microscope_Translator:

   The purpose of this class is to translate old stream files
   into the latest vrpn format and write them out to a vrpn_Connection
   You can think of this as analogous to a server class
*/
class nmm_Microscope_Translator: public nmb_SharedDevice_Server, public nmm_Microscope
{
  public:
    nmm_Microscope_Translator (const char * name,
                                vrpn_Connection *);
    virtual ~nmm_Microscope_Translator (void);

    int translate_packet(stm_stream *instream);

    timeval getTimeElapsed();

  protected:
    void doTimeCorrection(timeval &msg_time);

    // now:  most recent (corrected) time read in original stream
    // needs to be here because not every stream message
    // has a timestamp and we reuse the value from previous
    // timestamped messages by saving it here
    timeval now;

    long seqNum;
    timeval start_time_for_stream;
    timeval start_time_for_vrpn;
    vrpn_bool start_time_for_stream_initialized;
};

#endif  // NMM_MICROSCOPE_TRANSLATOR_H
