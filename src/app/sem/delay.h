#ifndef DELAY_H
#define DELAY_H

#include "vrpn_Shared.h"

/*
  This is a utility for creating a high-resolution delay in execution of
  some user code in Windows
  It was originally created for the purpose of timing the dwell time while
  exposing a sample with the SEM electron beam 
  The problem with gettimeofday is that its resolution may be too low although
  it is used with the assumption that it changes value with high precision

  (see nmm_Microscope_SEM_EDAX::goToPoint() for example)
*/

class Delay {
  public:
    static void init();
    static void beginRealTimeSection();
    static void endRealTimeSection();
    static void busyWaitSleep(double time_msec);

  protected:
    static int s_inRealTimeSection;
    static double s_iterationsPerMSec;
    static DWORD s_dwPriorityClass;
    static int s_iThreadPriority;
};

#endif
