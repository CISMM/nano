#include <vrpn_ForceDevice.h>
#include <vrpn_Phantom.h>

vrpn_ForceDevice_Remote * fdr = NULL;

void oscillatePlane (void) {
  const int period = 5;

  static int count = 0;

  count = (count + 1) % period;

  // Segfaults if called frequently.
  // Sometimes segfaults as soon as the program begins
  // to run;  other times only when the Phantom is jiggled.
  
  fdr->clearTrimesh();

  fdr->setVertex(0, -.5, -.05 - .005 * count, -.5);
  fdr->setVertex(1,  .5, 0 - .005 * count, -.5);
  fdr->setVertex(2, -.5, 0 - .005 * count,  .5);
  fdr->setVertex(3,  .5, -.05 - .005 * count,  .5);

  fdr->setTriangle(0, 0, 2, 1);
  fdr->setTriangle(1, 1, 2, 3);

  fdr->updateTrimeshChanges();

  // This code will run for "a while", but then segfaults,
  // generates runtime "pure virtual function call" errors, 
  // and other random bugs.
}

int main (int argc, char ** argv) {

  const vrpn_int32 updateRate = 60;
  const int period = 5000;

  vrpn_Connection * localDeviceConnection;
  vrpn_Phantom * ph;

  int count = 0;

  localDeviceConnection = new vrpn_Synchronized_Connection();

  ph = new vrpn_Phantom ("Phantom0", localDeviceConnection, updateRate);

  fdr = new vrpn_ForceDevice_Remote ("Phantom0", localDeviceConnection);



  // 0 - 1
  // | / |
  // 2 - 3
  
  fdr->clearTrimesh();

  fdr->setVertex(0, -.5, -.05, -.5);
  fdr->setVertex(1,  .5, 0, -.5);
  fdr->setVertex(2, -.5, 0,  .5);
  fdr->setVertex(3,  .5, -.05,  .5);

/* 
  // left-handed
  // creates a surface that can only be felt from below
  fdr->setTriangle(0, 0, 1, 2);
  fdr->setTriangle(1, 1, 3, 2);
 */

/* */
  // right-handed
  // creates a surface that can only be felt from above
  fdr->setTriangle(0, 0, 2, 1);
  fdr->setTriangle(1, 1, 2, 3);
/* */

  fdr->updateTrimeshChanges();

  while (1) {

     fdr->mainloop();
     ph->mainloop();

     fdr->mainloop();
     ph->mainloop();

     //count = (count + 1) % period;
//fprintf(stderr, "%d", count);
     //if (!count) {
     sleep(1);
       oscillatePlane();
     //}
  }

}
