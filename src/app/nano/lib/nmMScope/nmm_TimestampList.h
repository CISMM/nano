#ifndef NMM_TIMESTAMP_LIST_H
#define NMM_TIMESTAMP_LIST_H

#include <vrpn_Shared.h>  // for timeval

struct nmm_Timestamp {
  timeval elapsedTime;
  timeval finalTime;
  nmm_Timestamp * next;
};

class nmm_TimestampList {

  public:

    nmm_TimestampList (void);
    ~nmm_TimestampList (void);

    // MANIPULATORS

    void markSend (void);
    void add (timeval elapsed, timeval final);

    void record (vrpn_bool);
      ///< Unless this is set to VRPN_TRUE, calls to add() are ignored
      ///< (to minimize memory & time cost).

    void write (char * filename);
    void clear (void);

  protected:

    nmm_Timestamp * d_tp;
    nmm_Timestamp * d_tpEnd;

    timeval d_start;
    timeval d_end;

    vrpn_int32 d_numSends;
      ///< Number of requests sent;  incremented by markSend().

    vrpn_int32 d_numReceives;
      ///< Number of responses received;  incremented by add().

    vrpn_bool d_record;
};





#endif  // NMM_TIMESTAMP_LIST_H


