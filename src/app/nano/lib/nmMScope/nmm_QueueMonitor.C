#include "nmm_QueueMonitor.h"

#include <vrpn_RedundantTransmission.h>

#include <nmb_Decoration.h>

#include "nmm_MicroscopeRemote.h"
#include "active_set.h"  // for MAX_CHANNELS

#define CHECK(a) if ((a) == -1) return -1;

nmm_QueueMonitor::nmm_QueueMonitor (nmm_Microscope_Remote * ms,
                                    vrpn_RedundantReceiver * rr) :
    d_microscope (ms),
    d_redReceiver (rr),
    d_isEnabled (VRPN_FALSE),
    d_queueLength (0),
    d_callbacks (NULL),
    d_queueHead (NULL),
    d_queueTail (NULL),
    d_numDispatches (0),
    d_numPolls (0),
    d_numDrops (0),
    d_numGaps (0) {


  int i;

  if (d_redReceiver && d_microscope && d_microscope->getConnection()) {
    // if !getConnection(), pointResultType will be invalid
    d_redReceiver->register_handler(d_microscope->pointResultType(),
                                    handle_resultData, this);
  }

  for (i = 0; i < QM_MAX_QUEUE; i++) {
    d_queueCounter[i] = 0;
  }

  setThreshold(150, 2.0);
}


nmm_QueueMonitor::~nmm_QueueMonitor (void) {
  deleteQueue();
}

void nmm_QueueMonitor::mainloop (void) {
  vrpn_bool discard = VRPN_FALSE;
  int i;

  d_numPolls++;

  if (!d_isEnabled) {
    // if we have n pending messages, (n-1) will essentially be ignored
    if (d_queueLength) {
      d_numDrops += d_queueLength - 1;
    }

    // dispatch all pending messages
    do {
      dispatchQueueHead();
    } while (d_queueHead);

    return;
  }

  // "Thresholding operation"
  for (i = 0; (i < d_queueLength - 2) && (i < QM_MAX_QUEUE); i++) {
    d_queueCounter[i]++;
  }

  for (i = d_queueLength - 2; i < QM_MAX_QUEUE; i++) {
    d_queueCounter[i] = 0;
  }

  // Check d_queueCounter against d_queueThreshold;
  //   possibly discard/deliver the head message. 

  for (i = 0; i < QM_MAX_QUEUE; i++) {
    if (d_queueCounter[i] > d_queueThreshold[i]) {
      discard = VRPN_TRUE;
    }
  }

  if (discard) {
    // For now, we deliver the message instead of throwing it away,
    // since the effects are essentially the same...

    d_numDrops++;
    deleteQueueHead();
    for (i = 0; i < QM_MAX_QUEUE; i++) {
      d_queueCounter[i] = 0;
    }
  }

  // Deliver next message

  dispatchQueueHead();
}

void nmm_QueueMonitor::enable (vrpn_bool on) {
  d_isEnabled = on;
}

void nmm_QueueMonitor::registerResultDataHandler
                            (vrpn_MESSAGEHANDLER handler,
                             void * userdata) {
  vrpnMsgCallbackEntry * ce;

  ce = new vrpnMsgCallbackEntry;
  if (!ce) {
    fprintf(stderr, "nmm_QueueMonitor::registerResultDataHandler:  "
                    "Out of memory.\n");
    return;
  }

  ce->handler = handler;
  ce->userdata = userdata;
  ce->next = d_callbacks;
  d_callbacks = ce;
}

void nmm_QueueMonitor::setThreshold (vrpn_int32 base2value,
                                     vrpn_float64 decay) {
  vrpn_float64 threshold = base2value;
  int i;

  for (i = 0; i < QM_MAX_QUEUE; i++) {
    d_queueThreshold[i] = threshold;
    threshold /= decay;
  }
}

void nmm_QueueMonitor::write (char * filename) {
  FILE * fp;

  fprintf(stderr, "Queue Monitor with threshold %d, decay %.5f.\n",
          d_queueThreshold[0],
          d_queueThreshold[0] / (float) d_queueThreshold[1]);

  fprintf(stderr, "Gaps:  %d\n", d_numGaps);
  fprintf(stderr, "Drops:  %d\n", d_numDrops);

  fp = fopen(filename, "w");

  fprintf(fp, "Queue Monitor with threshold %d, decay %.5f.\n",
          d_queueThreshold[0],
          d_queueThreshold[0] / (float) d_queueThreshold[1]);

  fprintf(fp, "Gaps:  %d\n", d_numGaps);
  fprintf(fp, "Drops:  %d\n", d_numDrops);

  fclose(fp);

}

void nmm_QueueMonitor::clear (void) {

  d_numGaps = 0;
  d_numDrops = 0;
}


void nmm_QueueMonitor::enqueue (vrpn_HANDLERPARAM p) {
  vrpn_LOGLIST * qm;

  qm = new vrpn_LOGLIST;
  if (!qm) {
    fprintf(stderr, "nmm_QueueMonitor::enqueue:  Out of memory.\n");
    return;
  }

  if (d_queueTail) {
    d_queueTail->next = qm;
  }

  qm->data = p;
  qm->next = NULL;

  d_queueTail = qm;

  if (!d_queueHead) {
    d_queueHead = qm;
  }

  d_queueLength++;
}

int nmm_QueueMonitor::dispatchQueueHead (void) {
  vrpnMsgCallbackEntry * ce;

  if (!d_queueHead) {

    // Nothing to dispatch?
    // Count this as a gap only if we're expecting feedback
    // (this will overcount a # of gaps = mean latency / frame time)

    if (d_microscope &&
        ((d_microscope->Decor()->mode == nmb_Decoration::FEEL) ||
         (d_microscope->Decor()->mode == nmb_Decoration::MODIFY))) {
      d_numGaps++;
    }
    return 0;
  }

  for (ce = d_callbacks; ce; ce = ce->next) {
    CHECK((*ce->handler)(ce->userdata, d_queueHead->data));
  }

  d_numDispatches++;

  deleteQueueHead();

  return 0;
}

void nmm_QueueMonitor::deleteQueueHead (void) {
  vrpn_LOGLIST * qm;

  qm = d_queueHead;

  if (!qm) {
    return;
  }

  d_queueHead = qm->next;
  d_queueLength--;

  delete qm;
}

void nmm_QueueMonitor::deleteQueue (void) {
  vrpn_LOGLIST * qm;

  for (qm = d_queueHead; d_queueHead; qm = d_queueHead) {
    d_queueHead = qm->next;
    delete qm;
  }
  d_queueTail = NULL;
  d_queueLength = 0;
}



// static
int nmm_QueueMonitor::handle_resultData (void * ud, vrpn_HANDLERPARAM p) {
  nmm_QueueMonitor * me = (nmm_QueueMonitor *) ud;

  me->enqueue(p);

  return 0;
}

