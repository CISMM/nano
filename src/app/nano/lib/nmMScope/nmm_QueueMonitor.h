#ifndef NMM_QUEUE_MONITOR_H
#define NMM_QUEUE_MONITOR_H

#include <vrpn_Shared.h>
#include <vrpn_Connection.h>

class vrpn_RedundantReceiver;  // from vrpn_RedundantTransmission.h
class nmm_Microscope_Remote;  // from nmm_Microscope.h

/// @class nmm_QueueMonitor
/// A cute little object meant to sit between a vrpn_RedundantReceiver
/// and the nmm_Microscope_Remote.  If activated, it tries to smooth
/// out point result data coming back from topo so that playout doesn't
/// see any jitter.  Uses Don Stone's Queue Monitoring.  This will probably
/// have to be generalized to be time-sensitive when we hook it up to a
/// real microscope, but that requires more complex algorithmic design.

// Original design depended on nmm_Microscope, not _Remote, but since
// the Connection pointer got moved to nmb_Device, which isn't a base
// class of the Microscope, we have to use a circular dependency.  Yuck.
// (Moving the Connection pointer would be more reasonable IF _Remote
// instantiated Microscope instead of inheriting from it.)

#define QM_MAX_QUEUE 10

class nmm_QueueMonitor {

  public:

    nmm_QueueMonitor (nmm_Microscope_Remote *, vrpn_RedundantReceiver *);

    ~nmm_QueueMonitor (void);

    void mainloop (void);

    void enable (vrpn_bool);

    void registerResultDataHandler (vrpn_MESSAGEHANDLER handler,
                                    void * userdata);

    void setThreshold (vrpn_int32 base2value, vrpn_float64 decay);
      ///< Sets the threshold for a queue length of 2 and the
      ///< ratio threshold[n] / threshold[n+1];  defaults to 150 and 2.

    void write (char * filename);
    void clear (void);

  protected:

    nmm_Microscope_Remote * d_microscope;
      ///< Uses microscope to decode PointResultData messages enough
      ///< to extract the timestamp.

    vrpn_RedundantReceiver * d_redReceiver;
      ///< If the microscope is using FEC, the RedundantReceiver
      ///< will screen out duplicates for us.

    vrpn_bool d_isEnabled;
      ///< If false, all received messages are propagated to registered
      ///< handlers no later than the next call to mainloop();
      ///< if true, runs queue monitoring algorithm.
      ///< Defaults to false.

    int d_queueLength;
    vrpn_int32 d_queueCounter [QM_MAX_QUEUE];
    vrpn_int32 d_queueThreshold [QM_MAX_QUEUE];

    vrpnMsgCallbackEntry * d_callbacks;
    vrpn_LOGLIST * d_queueHead;
    vrpn_LOGLIST * d_queueTail;

    void enqueue (vrpn_HANDLERPARAM);
      ///< Adds the given message to the head of the queue.

    int dispatchQueueHead (void);
      ///< Returns -1 if a handler fails.

    void deleteQueueHead (void);

    void deleteQueue (void);

    static int VRPN_CALLBACK handle_resultData (void *, vrpn_HANDLERPARAM);

    vrpn_int32 d_numPolls;
      ///< Number of times mainloop was called.
    vrpn_int32 d_numDispatches;
      ///< Number of messages dispatched.  Under QM, equal to numPolls
      ///< minus numGaps.
    vrpn_int32 d_numDrops;
      ///< Number of messages dropped because of queue monitoring buffer
      ///< maintenance.
    vrpn_int32 d_numGaps;
      ///< Number of times mainloop was called without any message to
      ///< dispatch.
};


#endif  // NMM_QUEUE_MONITOR_H


