#include "nmb_TimerList.h"

#include <stdio.h>

nmb_TimerList::nmb_TimerList (void) :

    d_nextSN (0),
    d_totalTimestampsComplete (0),
    d_totalActiveTimestampsComplete (0),
    d_totalBlockedTimestampsComplete (0),
    d_list (NULL),
    d_freePool (NULL)
{

  d_totalTimeComplete.tv_sec = 0;
  d_totalTimeComplete.tv_usec = 0;
  d_totalActiveTimeComplete.tv_sec = 0;
  d_totalActiveTimeComplete.tv_usec = 0;
  d_totalBlockedTimeComplete.tv_sec = 0;
  d_totalBlockedTimeComplete.tv_usec = 0;

}


nmb_TimerList::~nmb_TimerList (void) {

  nmb_Timestamp * tp;

  while (d_list) {
    tp = d_list;
    d_list = d_list->next;
    delete tp;
  }
  while (d_freePool) {
    tp = d_freePool;
    d_freePool = d_freePool->next;
    delete tp;
  }

}


vrpn_int32 nmb_TimerList::getListHead (void) {

  if (d_list) {
    return d_list->serialNumber;
  } else {
    return -1;
  }

}

timeval nmb_TimerList::getListHeadTime (void) {

  if (d_list) {
    return d_list->timestamp;
  } else {
    timeval errret;
    errret.tv_sec = 0;
    errret.tv_usec = 0;
    return errret;
  }

}

vrpn_bool nmb_TimerList::isBlocked (vrpn_int32 sn) {
  nmb_Timestamp * ts;

  for (ts = d_list; ts; ts = ts->next) {
    if (ts->serialNumber == sn) {
      if (ts->pending) {
        return VRPN_TRUE;
      }
      return VRPN_FALSE;
    }
  }

  return VRPN_FALSE;
}



void nmb_TimerList::start (void) {

  gettimeofday(&d_startTime, NULL);

}

vrpn_int32 nmb_TimerList::newTimestep (void) {

  nmb_Timestamp * ts;
  nmb_Timestamp ** ps;
  nmb_Timestamp * v;
  timeval deltaT;

  ts = newTS();
  if (!ts) {
    fprintf(stderr, "nmb_TimerList::newTimestep:  Out of memory!\n");
    return 0;
  }

  for (ps = &d_list, v = *ps; v; /* nothing */) {
    
    if (!v->pending) {
      deltaT = vrpn_TimevalDiff(ts->timestamp, v->timestamp);
      d_totalTimeComplete = vrpn_TimevalSum(d_totalTimeComplete, deltaT);
      d_totalTimestampsComplete++;
      if (v->active) {
        deltaT = vrpn_TimevalDiff(ts->timestamp, v->timestamp);
        d_totalActiveTimeComplete = vrpn_TimevalSum(d_totalActiveTimeComplete,
                                                    deltaT);
        d_totalActiveTimestampsComplete++;
      }
      if (v->wasPending) {
        deltaT = vrpn_TimevalDiff(ts->timestamp, v->timestamp);
        d_totalBlockedTimeComplete = vrpn_TimevalSum(d_totalBlockedTimeComplete,
                                                     deltaT);
        d_totalBlockedTimestampsComplete++;
      }
      *ps = v->next;
      v->next = d_freePool;
      d_freePool = v;
      v = *ps;
      //freeTS(ps, v);
    } else {
      ps = &v->next;
      v = *ps;
    }
  }

  ts->next = d_list;
  d_list = ts;

  return ts->serialNumber;
}


void nmb_TimerList::block (vrpn_int32 sN) {
  nmb_Timestamp * ts;

  for (ts = d_list; ts; ts = ts->next) {
    if (ts->serialNumber == sN) {
      ts->pending = VRPN_TRUE;
      return;
    }
  }

  if (sN != -1) {
    fprintf(stderr, "nmb_TimerList::block:  %d not in list.\n", sN);
  }
}

void nmb_TimerList::unblock (vrpn_int32 sN) {
  nmb_Timestamp * ts;

  for (ts = d_list; ts; ts = ts->next) {
    if (ts->serialNumber == sN) {
      ts->pending = VRPN_FALSE;
      ts->wasPending = VRPN_TRUE;
      return;
    }
  }

  if (sN != -1) {
    fprintf(stderr, "nmb_TimerList::unblock:  %d not in list.\n", sN);
  }
}

void nmb_TimerList::activate (vrpn_int32 sN) {
  nmb_Timestamp * ts;

  for (ts = d_list; ts; ts = ts->next) {
    if (ts->serialNumber == sN) {
      ts->active = VRPN_TRUE;
//fprintf(stderr, "Activated timestamp %d.\n", sN);
      return;
    }
  }

  if (sN != -1) {
    fprintf(stderr, "nmb_TimerList::activate:  %d not in list.\n", sN);
  }
}



void nmb_TimerList::insert (vrpn_int32 sN) {
  nmb_Timestamp * ts;

  ts = newTS();

  ts->serialNumber = sN;
  d_nextSN--;  // Don't consume a SN from our local count.

  ts->next = d_list;
  d_list = ts;
}
  
void nmb_TimerList::remove (void) {
  if (d_list) {
    freeTS(&d_list, d_list);
  }
}


void nmb_TimerList::report (void) {

  nmb_Timestamp * ts;
  timeval meantime;
  timeval now;
  timeval elapsedWallclock;
  vrpn_int32 bcount;

  gettimeofday(&now, NULL);
  elapsedWallclock = vrpn_TimevalDiff(now, d_startTime);

  printf("Elapsed wallclock time:  %ld sec %ld usec.\n",
          elapsedWallclock.tv_sec, elapsedWallclock.tv_usec);

  printf("Total elapsed time:  %ld sec %ld usec.\n",
          d_totalTimeComplete.tv_sec, d_totalTimeComplete.tv_usec);
  printf("Total timestamps taken:  %d.\n",
          d_totalTimestampsComplete);

  meantime = vrpn_TimevalScale (d_totalTimeComplete,
                                1.0 / d_totalTimestampsComplete);
  printf("Mean elapsed time per timestamp:  %ld sec %ld usec.\n",
          meantime.tv_sec, meantime.tv_usec);

  printf("Total elapsed active time:  %ld sec %ld usec.\n",
          d_totalActiveTimeComplete.tv_sec, d_totalActiveTimeComplete.tv_usec);
  printf("Total active timestamps taken:  %d.\n",
          d_totalActiveTimestampsComplete);

  if (d_totalActiveTimestampsComplete > 0) {
    meantime = vrpn_TimevalScale (d_totalActiveTimeComplete,
                                  1.0 / d_totalActiveTimestampsComplete);
    printf("Mean elapsed time per active timestamp:  %ld sec %ld usec.\n",
           meantime.tv_sec, meantime.tv_usec);
  }

  printf("Total elapsed blocked time:  %ld sec %ld usec.\n",
          d_totalBlockedTimeComplete.tv_sec,
          d_totalBlockedTimeComplete.tv_usec);
  printf("Total blocked timestamps taken:  %d.\n",
          d_totalBlockedTimestampsComplete);

  if (d_totalBlockedTimestampsComplete > 0) {
    meantime = vrpn_TimevalScale (d_totalBlockedTimeComplete,
                                  1.0 / d_totalBlockedTimestampsComplete);
    printf("Mean elapsed time per blocked timestamp:  %ld sec %ld usec.\n",
           meantime.tv_sec, meantime.tv_usec);
  }


  bcount = 0;
  for (ts = d_list; ts; ts = ts->next) {
    if (ts->pending) {
      bcount++;
    }
  }
  if (bcount) {
    printf("Blocked timestamps never finished:  %d.\n", bcount);
  }
}



//  PROTECTED



nmb_Timestamp * nmb_TimerList::newTS (void) {

  nmb_Timestamp * ts;

  if (d_freePool) {
    ts = d_freePool;
    d_freePool = d_freePool->next;
  } else {
    ts = new nmb_Timestamp;
  }

  ts->serialNumber = d_nextSN++;
  gettimeofday(&ts->timestamp, NULL);
  ts->pending = VRPN_FALSE;
  ts->wasPending = VRPN_FALSE;
  ts->active = VRPN_FALSE;
  ts->next = NULL;

  return ts;
}

void nmb_TimerList::freeTS (nmb_Timestamp ** snitch, nmb_Timestamp * victim) {

  *snitch = victim->next;
  victim->next = d_freePool;
  d_freePool = victim;

}

