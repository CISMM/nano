#include "nmm_TimestampList.h"

#include <vrpn_Shared.h>  // for timeval, gettimeofday()

#include <stdio.h>  // for FILE
#include <stdlib.h>  // for gettimeofday()


nmm_TimestampList::nmm_TimestampList (void) :
    d_tp (NULL),
    d_tpEnd (NULL),
    d_numSends (0),
    d_numReceives (0),
    d_record (VRPN_FALSE) {

  d_start.tv_sec = 0L;
  d_start.tv_usec = 0L;
  d_end.tv_sec = 0L;
  d_end.tv_usec = 0L;

}

nmm_TimestampList::~nmm_TimestampList (void) {

}

void nmm_TimestampList::markSend (void) {
  d_numSends++;
}

void nmm_TimestampList::add (timeval elapsed, timeval final) {
  nmm_Timestamp * tp;

  if (!d_record) {
    return;
  }

  tp = new nmm_Timestamp;
  if (!tp) {
    fprintf(stderr, "nmm_TimestampList::add:  out of memory.\n");
    return;
  }
  tp->elapsedTime = elapsed;
  tp->finalTime = final;
  tp->next = NULL;

  if (d_tpEnd) {
    d_tpEnd->next = tp;
  } else {
    d_tp = tp;
  }
  d_tpEnd = tp;

  d_numReceives++;
}

void nmm_TimestampList::record (vrpn_bool on) {
  d_record = on;

  if (on) {
    gettimeofday(&d_start, NULL);
  } else {
    gettimeofday(&d_end, NULL);
  }
}


void nmm_TimestampList::write (char * filename) {
  FILE * fp;
  nmm_Timestamp * tp;
  timeval elapsed;

  if (d_record) {
    gettimeofday(&d_end, NULL);
  }

  elapsed = vrpn_TimevalDiff(d_end, d_start);
  fprintf(stderr, "Start at %d.%06d\n", d_start.tv_sec, d_start.tv_usec);
  fprintf(stderr, "End at %d.%06d\n", d_end.tv_sec, d_end.tv_usec);
  fprintf(stderr, "Elapsed:  %d.%06d\n", elapsed.tv_sec, elapsed.tv_usec);
  fprintf(stderr, "Sends:  %d.\n", d_numSends);
  fprintf(stderr, "Receives:  %d.\n", d_numReceives);
  fprintf(stderr, "Application-level loss:  %d.\n", d_numSends - d_numReceives);
  if (elapsed.tv_sec) {
    fprintf(stderr, "  rate:  %.5f/sec.\n", (d_numSends - d_numReceives) /
                                            (float) elapsed.tv_sec);
  }

  fp = fopen(filename, "w");
  if (!fp) {
    fprintf(stderr, "nmm_TimestampList::write:  Couldn't open file %s.\n",
            filename);
    return;
  }

  for (tp = d_tp; tp; tp = tp->next) {
    fprintf(fp, "%d.%06d %d.%06d\n",
            tp->finalTime.tv_sec, tp->finalTime.tv_usec,
            tp->elapsedTime.tv_sec, tp->elapsedTime.tv_usec);
  }

  fprintf(fp, "Start at %d.%06d\n", d_start.tv_sec, d_start.tv_usec);
  fprintf(fp, "End at %d.%06d\n", d_end.tv_sec, d_end.tv_usec);
  fprintf(fp, "Elapsed:  %d.%06d\n", elapsed.tv_sec, elapsed.tv_usec);
  fprintf(fp, "Sends:  %d.\n", d_numSends);
  fprintf(fp, "Receives:  %d.\n", d_numReceives);
  fprintf(fp, "Application-level loss:  %d.\n", d_numSends - d_numReceives);
  if (elapsed.tv_sec) {
    fprintf(fp, "  rate:  %.5f/sec.\n", (d_numSends - d_numReceives) /
                                        (float) elapsed.tv_sec);
  }

  fclose(fp);

  if (d_record) {
    gettimeofday(&d_start, NULL);
  }

}


void nmm_TimestampList::clear (void) {
  nmm_Timestamp * tp;

  for (tp = d_tp; d_tp; tp = d_tp) {
    d_tp = tp->next;
    delete tp;
  }

  d_tp = NULL;
  d_tpEnd = NULL;

  gettimeofday(&d_start, NULL);
}

