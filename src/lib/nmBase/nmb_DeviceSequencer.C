#include "nmb_DeviceSequencer.h"

SequenceEntry::SequenceEntry():
   numSenders(0),
   numDevices(0),
   numMessages(0),
   next(NULL)
{

}

void SequenceEntry::print()
{
    int i;
    printf("%d devices: ", numDevices);
    for (i = 0; i < numDevices; i++){
        printf("%d ", deviceIndex[i]);
    }
    printf("\n");
    printf("%d senders: ", numSenders);
    for (i = 0; i < numSenders; i++){
        printf("%d ", senderIndex[i]);
    }
    printf("\n");
    printf("%d messages (index,repeat): ", numMessages);
    for (i = 0; i < numMessages; i++){
        printf("(%d, %d) ", messageIndex[i], numRepeat[i]);
    }
    printf("\n");
}

nmb_DeviceSequencer::nmb_DeviceSequencer(int verbosity):
   d_iterationHandlers(NULL),
   d_numDevices(0),
   d_numSenders(0),
   d_numMessageTypes(0),
   d_sequenceHead(NULL),
   d_sequenceTail(NULL),
   d_verbosity(verbosity),
   d_needNewSequenceEntry(vrpn_FALSE),
   d_running(vrpn_FALSE),
   d_specified(vrpn_FALSE),
   d_currentSequenceEntry(NULL)
{

}

nmb_DeviceSequencer::~nmb_DeviceSequencer()
{

}

int nmb_DeviceSequencer::begin()
{
    if (d_running) {
        fprintf(stderr, "nmb_DeviceSequencer::begin():" 
              "Error, can't change sequence while running\n");
        return -1;
    }
    d_specified = vrpn_FALSE;

    SequenceEntry *temp;
    while (d_sequenceHead) {
       temp = d_sequenceHead;
       d_sequenceHead = d_sequenceHead->next;
       delete temp;
    }
    d_needNewSequenceEntry = vrpn_FALSE;
    d_sequenceHead = new SequenceEntry();
    d_sequenceTail = d_sequenceHead;
    d_numMessageTypes = 0;
    return 0;
}

int nmb_DeviceSequencer::end()
{
    if (d_specified) {
        fprintf(stderr, "nmb_DeviceSequencer::end(): "
                   "Error: called end() twice without calling begin()\n");
        return -1;
    }

    if (d_verbosity > 0) {
        printf("nmb_DeviceSequencer::end()\n");
        SequenceEntry *sequence_element = d_sequenceHead;
        while (sequence_element) {
            printf("element: \n");
            sequence_element->print();
            sequence_element = sequence_element->next;
        }
        printf("\n");
    }
    d_needNewSequenceEntry = vrpn_FALSE;
    d_specified = vrpn_TRUE;
    d_currentSequenceEntry = d_sequenceHead;
    return 0;
}

int nmb_DeviceSequencer::unregisterSynchHandlers()
{
    int i;
    for (i = 0; i < d_numDevices; i++){
        d_devices[i]->unregisterSynchHandler(deviceSynchHandler,
                          (void *)&(d_synchHandlerInfo[i]));
    }
    return 0;
}

int nmb_DeviceSequencer::registerSynchHandlers()
{
    int i;
    for (i = 0; i < d_numDevices; i++){
        d_devices[i]->registerSynchHandler(deviceSynchHandler,
                          (void *)&(d_synchHandlerInfo[i]));
    }
    return 0;
}

int nmb_DeviceSequencer::unregisterMessageHandlers()
{
    int i;
    vrpn_Connection *c;
    for (i = 0; i < d_numMessageTypes; i++){
        c = d_messageTypes[i].device->getConnection();
        if (c) {
            if (c->unregister_handler(d_messageTypes[i].message_type,
                        dataMessageHandler,
                        (void *)&(d_dataHandlerInfo[i]))) {
                fprintf(stderr, "nmb_DeviceSequencer::begin(): "
                      "Error: couldn't unregister handler for device\n");
                return -1;
            }
        } else {
            fprintf(stderr, "nmb_DeviceSequencer::begin(): "
                   "Error: couldn't get connection for device\n");
            return -1;
        }
    }
    return 0;
}

int nmb_DeviceSequencer::registerMessageHandlers()
{
    int i;
    vrpn_Connection *c;
    for (i = 0; i < d_numMessageTypes; i++){
        c = d_messageTypes[i].device->getConnection();
        if (c) {
            if (c->register_handler(d_messageTypes[i].message_type,
                        dataMessageHandler,
                        (void *)&(d_dataHandlerInfo[i]))) {
                fprintf(stderr, "nmb_DeviceSequencer::begin(): "
                      "Error: couldn't unregister handler for device\n");
                return -1;
            }
        } else {
            fprintf(stderr, "nmb_DeviceSequencer::begin(): "
                   "Error: couldn't get connection for device\n");
            return -1;
        }
    }
    return 0;
}

int nmb_DeviceSequencer::addCheckPoint() {
    d_needNewSequenceEntry = vrpn_TRUE;
    return 0;
}

int nmb_DeviceSequencer::addEntry()
{
    d_sequenceTail->next = new SequenceEntry();
    if (d_sequenceTail->next == NULL) {
         fprintf(stderr, "nmb_DeviceSequencer::addEntry(): Error, "
          "out of memory\n");
         return -1;
    }
    d_sequenceTail = d_sequenceTail->next;
    return 0;
}

int nmb_DeviceSequencer::addDevice(int deviceIndex)
{
    if (d_needNewSequenceEntry) {
        addEntry();
        d_needNewSequenceEntry = vrpn_FALSE;
    }
    if (d_sequenceTail->numDevices == NMB_DEVICESEQUENCER_MAX_DEVICES) {
        fprintf(stderr, "nmb_DeviceSequencer::addDevice: Error, "
                "maxed-out sequence element numDevices\n");
        return -1;
    }

    d_sequenceTail->deviceIndex[d_sequenceTail->numDevices] = deviceIndex;
    d_sequenceTail->numDevices++;
    return 0;
}

/**
adds device to global device list (if it wasn't there already) 
and to last sequence entry
*/
int nmb_DeviceSequencer::addDevice(nmb_Device_Client *device)
{
    int i;
    for (i = 0; i < d_numDevices; i++){
        if (d_devices[i] == device) {
            return addDevice(i);
        }
    }
    if (d_numDevices == NMB_DEVICESEQUENCER_MAX_DEVICES) {
        fprintf(stderr, "nmb_DeviceSequencer::addDevice: Error, "
                "maxed-out global numDevices\n");
        return -1;
    }
    d_devices[d_numDevices] = device;
    d_synchHandlerInfo[d_numDevices].sequencer = this;
    d_synchHandlerInfo[d_numDevices].deviceIndex = d_numDevices;
    d_numDevices++;
    return addDevice(d_numDevices-1);
}

/**
adds device to global sender list (if it wasn't there already)
*/
int nmb_DeviceSequencer::addSender(nmb_Device_Client *device)
{
    int i;
    for (i = 0; i < d_numSenders; i++){
        if (d_senders[i] == device) {
            return 0;
        }
    }
    if (d_numSenders == NMB_DEVICESEQUENCER_MAX_DEVICES) {
        fprintf(stderr, "nmb_DeviceSequencer::addSender: Error, "
                "maxed-out global numSenders\n");
        return -1;
    }
    d_senders[d_numSenders] = device;
    d_numSenders++;
    return 0;
}

int nmb_DeviceSequencer::addDeviceMessageSend(int senderIndex)
{
    if (d_needNewSequenceEntry) {
        addEntry();
        d_needNewSequenceEntry = vrpn_FALSE;
    }
    if (d_sequenceTail->numSenders == NMB_DEVICESEQUENCER_MAX_SENDING_DEVICES) {
        fprintf(stderr, "nmb_DeviceSequencer::addDeviceMessageSend: Error, "
                "maxed-out sequence element numSenders\n");
        return -1;
    }
    d_sequenceTail->senderIndex[d_sequenceTail->numSenders] = senderIndex;
    d_sequenceTail->numSenders++;
    return 0;
}

int nmb_DeviceSequencer::addDeviceMessageSend(nmb_Device_Client *device)
{
    int i;
    if (addDevice(device)) {
        return -1;
    }

    if (addSender(device)) {
        return -1;
    } 

    for (i = 0; i < d_numDevices; i++){
        if (d_devices[i] == device) {
            return addDeviceMessageSend(i);
        }
    }

    fprintf(stderr, "nmb_DeviceSequencer::addDeviceMessageSend: Error, "
       "couldn't find device in list\n");
    return -1;
}

int nmb_DeviceSequencer::addDataMessage(int messageIndex,
                                     int n_repeat)
{
    if (d_needNewSequenceEntry) {
        addEntry();
        d_needNewSequenceEntry = vrpn_FALSE;
    }
    if (d_sequenceTail->numMessages == NMB_DEVICESEQUENCER_MAX_DATA_MESSAGES) {
        fprintf(stderr, "nmb_DeviceSequencer::addDataMessage: Error, "
                "maxed-out sequence element numMessages\n");
        return -1;
    }
    d_sequenceTail->messageIndex[d_sequenceTail->numMessages] = messageIndex;
    d_sequenceTail->numRepeat[d_sequenceTail->numMessages] = n_repeat;
    d_sequenceTail->numMessages++;
    return 0;
}

int nmb_DeviceSequencer::addDataMessage(nmb_Device_Client *device, 
                                     vrpn_int32 message_type,
                                     int n_repeat)
{
    int i;
    if (addDevice(device)) {
        return -1;
    }

    for (i = 0; i < d_numMessageTypes; i++){
        if (d_messageTypes[i].device == device &&
            d_messageTypes[i].message_type == message_type) {
            return addDataMessage(i, n_repeat);
        }
    }
    if (d_numMessageTypes == NMB_DEVICESEQUENCER_MAX_DATA_MESSAGES) {
        fprintf(stderr, "nmb_DeviceSequencer::addDataMessage: Error, "
                "maxed-out global numMessages\n");
        return -1;
    }
    d_messageTypes[d_numMessageTypes].device = device;
    d_messageTypes[d_numMessageTypes].message_type = message_type;
    d_dataHandlerInfo[d_numMessageTypes].sequencer = this;
    d_dataHandlerInfo[d_numMessageTypes].messageTypeIndex = d_numMessageTypes;

    d_numMessageTypes++;
    return addDataMessage(d_numMessageTypes-1, n_repeat);
}

// *******************************************************************


// static
int nmb_DeviceSequencer::dataMessageHandler(void *ud,
                                            vrpn_HANDLERPARAM /*p*/)
{
    nmb_DeviceSequencer::dataHandlerInfo *info = 
          (nmb_DeviceSequencer::dataHandlerInfo *)ud;

    if (info->sequencer->d_synchInProgress) {
        // if we get data while we are trying to synchronize then it is 
        // simply extra data for the last interval and we are not depending
        // on it in order to transition to the next interval
        return 0;
    }

    (info->sequencer->d_numMessagesReceived)[(info->messageTypeIndex)]++;

    if (info->sequencer->d_verbosity > 1) {
        printf("nmb_DeviceSequencer: got data message %d; count = %d\n",
           info->messageTypeIndex,
           (info->sequencer->d_numMessagesReceived)[(info->messageTypeIndex)]);
    }
    
    if (info->sequencer->dataCompleted()) {
        info->sequencer->increment();
        info->sequencer->sendCommands();
        info->sequencer->requestSynch();
    }

    return 0;
}

// static
int nmb_DeviceSequencer::deviceSynchHandler(
    void *ud,
    const nmb_SynchMessage * /*sm*/)
{
    nmb_DeviceSequencer::synchHandlerInfo *info = 
          (nmb_DeviceSequencer::synchHandlerInfo *)ud;

    if (info->sequencer->d_verbosity > 1) {
        printf("nmb_DeviceSequencer: got synch from device %d\n",
             info->deviceIndex);
    }

    if (!(info->sequencer->d_deviceSynch)[(info->deviceIndex)]) {
        (info->sequencer->d_deviceSynch)[(info->deviceIndex)] = vrpn_TRUE;
        info->sequencer->d_numSynchReceived++;
        // reset counter for all messages coming from this device
        info->sequencer->resetDataCounters(info->deviceIndex);
    } else {
        fprintf(stderr, "nmb_DeviceSequencer::deviceSynchHandler: Warning, "
                "unexpected synch message received\n");
        if (info->sequencer->d_verbosity > 2) {
          printf("(%d: %d)\n",
              info->deviceIndex, 
              (info->sequencer->d_deviceSynch)[(info->deviceIndex)]);
          printf("0 indicates expected: ");
          int i;
          for (i = 0; 
            i < info->sequencer->d_currentSequenceEntry->numDevices; i++){
              printf("(%d: %d) ", 
              info->sequencer->d_currentSequenceEntry->deviceIndex[i],
              info->sequencer->
                d_deviceSynch[info->sequencer->
                              d_currentSequenceEntry->deviceIndex[i]]);
          }
          printf("\n");
        }
        return 0;
    }

    if (info->sequencer->synchCompleted()) {
       info->sequencer->d_synchInProgress = vrpn_FALSE;
       // check for this condition here in case there is no data that
       // we need to receive in this interval
       if (info->sequencer->dataCompleted()) {
           info->sequencer->increment();
           info->sequencer->sendCommands();
           info->sequencer->requestSynch();
       }
    }
    return 0;
}

vrpn_bool nmb_DeviceSequencer::synchCompleted()
{
    return (d_numSynchExpected == d_numSynchReceived);
}

vrpn_bool nmb_DeviceSequencer::dataCompleted()
{
    int i;
    for (i = 0; i < d_currentSequenceEntry->numMessages; i++) {
        // have we not received at least as many messages of this type
        // as we want to receive?
        if (d_currentSequenceEntry->numRepeat[i] > 
            d_numMessagesReceived[d_currentSequenceEntry->messageIndex[i]]) {
            if (d_verbosity > 2) {
                printf("nmb_DeviceSequencer::dataCompleted false\n");
                printf("   messages needed: %d, messages received: %d\n",
                  d_currentSequenceEntry->numRepeat[i],
                  d_numMessagesReceived[d_currentSequenceEntry->messageIndex[i]]);
            }
            return vrpn_FALSE;
        }
    }

    if (d_verbosity > 2) {
        printf("nmb_DeviceSequencer::dataCompleted true\n");
    }
    return vrpn_TRUE;
}

void nmb_DeviceSequencer::increment()
{
    if (d_verbosity > 0) {
        printf("nmb_DeviceSequencer: incrementing\n");
    }

    if (!d_currentSequenceEntry) return;
    d_currentSequenceEntry = d_currentSequenceEntry->next;
    if (d_currentSequenceEntry == NULL) { // we've reached the end of the cycle
        doIterationCallbacks();
        d_currentSequenceEntry = d_sequenceHead;
    }
}

void nmb_DeviceSequencer::requestSynch()
{
    int i;

    // flag devices for which we are expecting synchronization messages
    // by initializing to false the synch flags for those devices
    // exclusively
    for (i = 0; i < d_numDevices; i++){
        d_deviceSynch[i] = vrpn_TRUE;
    }
    for (i = 0; i < d_currentSequenceEntry->numDevices; i++){
        d_deviceSynch[d_currentSequenceEntry->deviceIndex[i]] = vrpn_FALSE;
    }

    d_numSynchExpected = d_currentSequenceEntry->numDevices;
    d_numSynchReceived = 0;

    if (d_verbosity > 1) {
        printf("  requesting synch for %d devices\n",
           d_numSynchExpected);
        printf("0 indicates expected: ");
        for (i = 0; i < d_numDevices; i++){
            printf("(%d: %d) ", i, d_deviceSynch[i]);
        }
        printf("\n");
    }
    d_synchInProgress = vrpn_TRUE;

    for (i = 0; i < d_currentSequenceEntry->numDevices; i++){
        nmb_Device_Client *d = d_devices[d_currentSequenceEntry->deviceIndex[i]];
        d->setBufferEnable(vrpn_FALSE);
        d->requestSynchronization(0, 0, NULL);
        d->setBufferEnable(vrpn_TRUE);
    }
}

void nmb_DeviceSequencer::enableCommandBuffers()
{
    int i;
    for (i = 0; i < d_numSenders; i++){
        d_senders[i]->setBufferEnable(vrpn_TRUE);
    }
}

void nmb_DeviceSequencer::disableCommandBuffers()
{
    int i;
    for (i = 0; i < d_numSenders; i++){
        d_senders[i]->setBufferEnable(vrpn_FALSE);
        d_senders[i]->sendBuffer();
    }
}

void nmb_DeviceSequencer::sendCommands()
{
    int i;
    for (i = 0; i < d_currentSequenceEntry->numSenders; i++){
        d_devices[d_currentSequenceEntry->senderIndex[i]]->sendBuffer();
    }
}

// reset counters for all messages coming from this device (specified
// as index into d_devices
void nmb_DeviceSequencer::resetDataCounters(int deviceIndex)
{
    int i;
    for (i = 0; i < d_numMessageTypes; i++){
         if (d_messageTypes[i].device == d_devices[deviceIndex]){
             d_numMessagesReceived[i] = 0;
         }
    }
}

int nmb_DeviceSequencer::takeControl()
{
    if (!d_specified) {
        fprintf(stderr, "nmb_DeviceSequencer::takeControl(): Error, "
               "incomplete specification\n");
        return -1;
    }
    if (d_running) {
        fprintf(stderr, "nmb_DeviceSequencer::takeControl(): Error, "
               "already running\n");
        return -1;
    }

    d_currentSequenceEntry = d_sequenceHead;
    enableCommandBuffers();
    d_running = vrpn_TRUE;
    registerSynchHandlers();
    registerMessageHandlers();

    doIterationCallbacks();
    sendCommands();
    requestSynch();

    return 0;
}

int nmb_DeviceSequencer::releaseControl()
{
    if (!d_running) {
        fprintf(stderr, "nmb_DeviceSequencer::releaseControl(): Error, "
               "not running\n");
        return -1;
    }
    unregisterSynchHandlers();
    unregisterMessageHandlers();
    disableCommandBuffers();
    d_running = vrpn_FALSE;
    return 0;
}

int nmb_DeviceSequencer::registerIterationHandler (int (* handler) (void *), 
                                                 void *userdata)
{
  iterationHandlerEntry * newEntry;

  newEntry = new iterationHandlerEntry;
  if (!newEntry) {
    fprintf(stderr, "nmb_DeviceSequencer::registerIterationHandler:  "
                    "Out of memory.\n");
    return -1;
  }

  newEntry->handler = handler;
  newEntry->userdata = userdata;
  newEntry->next = d_iterationHandlers;

  d_iterationHandlers = newEntry;

  return 0;
}

int nmb_DeviceSequencer::unregisterIterationHandler (int (* handler) (void *), 
                                                   void *userdata)
{
  iterationHandlerEntry * victim, ** snitch;

  snitch = &d_iterationHandlers;
  victim = *snitch;
  while (victim &&
         (victim->handler != handler) &&
         (victim->userdata != userdata)) {
    snitch = &((*snitch)->next);
    victim = *snitch;
  }

  if (!victim) {
    fprintf(stderr, "nmb_DeviceSequencer::unregisterIterationHandler:  "
                    "No such handler.\n");
    return -1;
  }

  *snitch = victim->next;
  delete victim;

  return 0;
}

void nmb_DeviceSequencer::doIterationCallbacks()
{
  iterationHandlerEntry * l;

  l = d_iterationHandlers;
  while (l) {
    if ((l->handler)(l->userdata)) {
      fprintf(stderr, "nmb_DeviceSequencer::doIterationCallbacks:  "
                      "Nonzero return value.\n");
      return;
    }
    l = l->next;
  }
}
