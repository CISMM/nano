#include "delay.h"

int Delay::s_inRealTimeSection = 0;
double Delay::s_iterationsPerMSec = 0.0;
DWORD Delay::s_dwPriorityClass = 0;
int Delay::s_iThreadPriority = 0;

#pragma optimize("",off)
void Delay::init()
{
  beginRealTimeSection();
  struct timeval t_start, t_end;
  gettimeofday(&t_start, NULL);
  gettimeofday(&t_end, NULL);
  double delta_t = vrpn_TimevalMsecs(t_end) - vrpn_TimevalMsecs(t_start);
  int numIterations = 0;
  // wait for one increment to occur
  while (delta_t < 1.0) {
    gettimeofday(&t_end, NULL);
    delta_t = vrpn_TimevalMsecs(t_end) - vrpn_TimevalMsecs(t_start);
  }
  t_start = t_end;
  delta_t = 0;
  // now wait for one complete cycle
  while (delta_t < 1.0) {
    gettimeofday(&t_end, NULL);
    delta_t = vrpn_TimevalMsecs(t_end) - vrpn_TimevalMsecs(t_start);
    numIterations++;
  }

  s_iterationsPerMSec = (double)numIterations/delta_t;
  endRealTimeSection();
}
#pragma optimize("",on)

void Delay::beginRealTimeSection()
{
  if (!s_inRealTimeSection) {
    s_dwPriorityClass = GetPriorityClass(GetCurrentProcess());
    s_iThreadPriority = GetThreadPriority(GetCurrentThread());
    SetPriorityClass( GetCurrentProcess() , REALTIME_PRIORITY_CLASS );
    SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL );
  }
  s_inRealTimeSection++;
}

void Delay::endRealTimeSection()
{
  if (!s_inRealTimeSection) return;
  s_inRealTimeSection--;
  if (!s_inRealTimeSection) {
    SetPriorityClass( GetCurrentProcess() , s_dwPriorityClass );
    SetThreadPriority( GetCurrentThread(), s_iThreadPriority );
  }
}

#pragma optimize("",off)
void Delay::busyWaitSleep(double time_msec)
{
  int numIterationsToDo = time_msec*s_iterationsPerMSec;
  int i;
  struct timeval t_start, t_end;
  int numIterations;

  double delta_t;
  gettimeofday(&t_start, NULL);
  for (i = 0; i < numIterationsToDo; i++) {
    gettimeofday(&t_end, NULL);
    delta_t = vrpn_TimevalMsecs(t_end) - vrpn_TimevalMsecs(t_start);
    numIterations++;
  }
}
#pragma optimize("",on)
