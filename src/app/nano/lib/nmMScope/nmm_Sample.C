#include "nmm_Sample.h"

#include <nmb_DeviceSequencer.h>

#include "nmm_MicroscopeRemote.h"


nmm_Sample::nmm_Sample (nmm_Microscope_Remote * m) :
    d_sequencer (new nmb_DeviceSequencer ()),
    d_iteration (0) {

  setMicroscope(m);

}

// virtual
nmm_Sample::~nmm_Sample (void) {

}



void nmm_Sample::setMicroscope (nmm_Microscope_Remote * m) {
  d_scope = m;
}

void nmm_Sample::sampleAt (float x, float y) {

  d_sampleX = x;
  d_sampleY = y;

  // The microscope should send all pending messages,
  // and receive (xSize * ySize) messages.

  d_sequencer->begin();
  d_sequencer->addDeviceMessageSend(d_scope);
  d_sequencer->addDataMessage(d_scope,
                              d_scope->pointResultType(),
                              numSamples());
  d_sequencer->end();

  d_sequencer->registerIterationHandler(handleSequencerIteration, this);

  // Get ready for a new set of points received from the microscope.

  // Swap on AFMState the receivedPointList and incomingPointList structures
  // Clean up old data on should-now-be-blank incomingPointList

  d_sequencer->takeControl();

}

void nmm_Sample::addPostSampleCallback (PostSampleCallback f,
                                        void * userdata) {
  pscE * n = new pscE;

  if (!n) {
    fprintf(stderr, "nmm_Sample::addPostSampleCallback():  "
                    "Out of memory!\n");
    return;
  }

  n->f = f;
  n->userdata = userdata;
  n->next = d_callbacks;
  d_callbacks = n;
}






void nmm_Sample::processSampleRequests (void) {

  // Clean-up:  swap incomingPointList with receivedPointList so
  // that the user sees the new data.

  Point_list * temp;
  temp = d_scope->state.data.receivedPointList;
  d_scope->state.data.receivedPointList = d_scope->state.data.incomingPointList;
  d_scope->state.data.incomingPointList = temp;

  // Get rid of stale data on the (new) incomingPointList.
  // TODO

}

// static
int nmm_Sample::handleSequencerIteration (void * userdata) {
  nmm_Sample * sample = (nmm_Sample *) userdata;

  switch (sample->d_iteration) {
    case 0:
      sample->sendSampleRequests();
      break;
    case 1:
      sample->processSampleRequests();
      break;
  }
  sample->d_iteration = (sample->d_iteration + 1) % 2;

  return 0;
}

void nmm_Sample::triggerPostSampleCallbacks (void) {
  pscE * n;

  for (n = d_callbacks; n; n = n->next) {
    (*n->f)(n->userdata);
  }

}





nmm_SampleGrid::nmm_SampleGrid (nmm_Microscope_Remote * m) :
    nmm_Sample (m),
    d_xSize (1),
    d_ySize (1),
    d_xDistance (0.0f),
    d_yDistance (0.0f) {

  recomputeMeasure();
}

// virtual
nmm_SampleGrid::~nmm_SampleGrid (void) {

}

// virtual
int nmm_SampleGrid::numSamples (void) const {
  return d_xSize * d_ySize;
}

void nmm_SampleGrid::setGridSize (int x, int y) {

  d_xSize = x;
  d_ySize = y;

  recomputeMeasure();
}

void nmm_SampleGrid::setGridSpacing (float d) {

  d_xDistance = d;
  d_yDistance = d;

  recomputeMeasure();
}

void nmm_SampleGrid::recomputeMeasure (void) {

  // Side length of a grid = (# units - 1) x unit size

  d_xMeasure = (d_xSize - 1) * d_xDistance;
  d_yMeasure = (d_ySize - 1) * d_yDistance;

}

void nmm_SampleGrid::sendSampleRequests (void) {

  int i, j;
  float x0, y0;

  // TODO:  put some sort of marker in the stream so that
  // d_scope saves these up as a set rather than reporting them back
  // one-by-one.

  for (i = 0, x0 = d_sampleX - (d_xMeasure / 2.0);
       i < d_xSize;
       i++, x0 += d_xDistance) {
    for (j = 0, y0 = d_sampleY - (d_yMeasure / 2.0);
         j < d_ySize;
         j++, y0 += d_yDistance) {

      d_scope->ScanTo(x0, y0);

    }
  }

  // TODO:  put an end-of-sample-set marker in the stream


}

void nmm_SampleGrid::processSampleRequests (void) {

  nmm_Sample::processSampleRequests();
}

