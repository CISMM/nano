#include "testDevice.h"
#include "nmb_DeviceSequencer.h"

#define SERVER "lead-cs.cs.unc.edu"
#define NUM_DEVICES (2)

int sequenceIterationHandler(void *ud);

int main(int argc, char **argv)
{
  nmb_DeviceSequencer *sequencer;
  TestDeviceClient *device[NUM_DEVICES];
  char device_name[100];
  
  int i;
  for (i = 0; i < NUM_DEVICES; i++) {
    sprintf(device_name, "TestDevice%1d@%s:%d",
             i, SERVER, vrpn_DEFAULT_LISTEN_PORT_NO+i);
    printf("device %d: %s\n", i, device_name);
    device[i] = new TestDeviceClient(device_name);
  }

  vrpn_bool connected = vrpn_FALSE;
  while (!connected) {
      for (i = 0; i < NUM_DEVICES; i++){
          device[i]->mainloop();
      }
      connected = vrpn_TRUE;
      for (i = 0; i < NUM_DEVICES; i++){
          connected = connected && device[i]->connected();
      }
  }

  sequencer = new nmb_DeviceSequencer(10);
  sequencer->begin();
  sequencer->addDeviceMessageSend(device[0]);
  sequencer->addCheckPoint();
  sequencer->addDataMessage(device[1], 
                device[1]->getPeriodicDataMessageType(),
                10);
  sequencer->addCheckPoint();
  sequencer->end();

  sequencer->registerIterationHandler(sequenceIterationHandler, NULL);

  sequencer->takeControl();

  char message[128];
  struct timeval now, next_time_to_mod, mod_interval;
  int mod_count = 0;

  gettimeofday(&now, NULL);
  mod_interval.tv_sec = 0;
  mod_interval.tv_usec = 700000;

  next_time_to_mod = now;

  while (1){
      gettimeofday(&now, NULL);
      if (vrpn_TimevalGreater(now, next_time_to_mod)) {
          sprintf(message, "mod: %d", mod_count);
          device[0]->sendRequestModify(message);
          next_time_to_mod = vrpn_TimevalSum(next_time_to_mod, mod_interval);
          mod_count++;
      }
      for (i = 0; i < NUM_DEVICES; i++){
          device[i]->mainloop();
      }
  }
  return 0;
}

int sequenceIterationHandler(void *ud)
{
    // might want to send modify commands exclusively from here in order to tie
    // the rate of modification to the rate of acquisition -
    // this would not be practical for point-by-point 
    // interactive manipulation in which the human user controls the rate
    // of modification but would work for batch operations such as lines
    printf("#### beginning sequence iteration\n");
    return 0;
}
