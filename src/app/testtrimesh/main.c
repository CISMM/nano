#include <vrpn_ForceDevice.h>
#include <vrpn_Phantom.h>

vrpn_Tracker_Remote * tr = NULL;
vrpn_ForceDevice_Remote * fdr = NULL;
vrpn_Phantom * ph = NULL;

void printlocation (void * userdata, const vrpn_TRACKERCB info) {
  fprintf(stderr, "ph %.5f %.5f %.5f\n",
	info.pos[0], info.pos[1], info.pos[2]);
}

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

void testOscillation (void) {


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

void testRealData (void) {

   const int xside = 5;
   const int yside = 5;
   int i, j, k, start;


   fdr->clearTrimesh();

   fdr->setVertex(0, 0.5557, 0.2035, 0.7967);
   fdr->setVertex(1, 0.5557, 0.2482, 0.8008);
   fdr->setVertex(2, 0.5557, 0.2929, 0.8048);
   fdr->setVertex(3, 0.5557, 0.3376, 0.8089);
   fdr->setVertex(4, 0.5557, 0.2823, 0.8130);
   fdr->setVertex(5, 0.5844, 0.2279, 0.8211);
   fdr->setVertex(6, 0.5844, 0.2726, 0.8252);
   fdr->setVertex(7, 0.5844, 0.3173, 0.8292);
   fdr->setVertex(8, 0.5844, 0.3620, 0.8333);
   fdr->setVertex(9, 0.5844, 0.4066, 0.8373);
   fdr->setVertex(10, 0.6131, 0.2523, 0.8455);
   fdr->setVertex(11, 0.6131, 0.2970, 0.8495);
   fdr->setVertex(12, 0.6131, 0.3416, 0.8536);
   fdr->setVertex(13, 0.6131, 0.3863, 0.8577);
   fdr->setVertex(14, 0.6131, 0.4310, 0.8617);
   fdr->setVertex(15, 0.6419, 0.2766, 0.8698);
   fdr->setVertex(16, 0.6419, 0.3213, 0.8739);
   fdr->setVertex(17, 0.6419, 0.3660, 0.8780);
   fdr->setVertex(18, 0.6419, 0.4107, 0.8820);
   fdr->setVertex(19, 0.6419, 0.4554, 0.8861);
   fdr->setVertex(20, 0.6706, 0.3010, 0.8942);
   fdr->setVertex(21, 0.6706, 0.3457, 0.8983);
   fdr->setVertex(22, 0.6706, 0.3904, 0.9023);
   fdr->setVertex(23, 0.6706, 0.4351, 0.9064);
   fdr->setVertex(24, 0.6706, 0.4798, 0.9105);

   k = 0;
   for (i = 0; i < xside - 1; i++) {
     start = i * xside;
       for (j = 0; j < yside - 1; j++) {
	 fdr->setTriangle(k++, start + j, start + j + xside,
	                       start + j + 1);
	 fdr->setTriangle(k++, start + j + 1, start + j + xside,
	                       start + j + xside + 1);
	 fprintf(stderr, "tri %d %d %d, ", start + j, start + j + xside,
	       start + j + 1);
	 fprintf(stderr, "tri %d %d %d\n", start + j + 1, start + j + xside,
	       start + j + xside + 1);
       }
   }
   

   fdr->updateTrimeshChanges();

   tr->register_change_handler(NULL, printlocation);

   while (1) {
      ph->mainloop();
      fdr->mainloop();
      tr->mainloop();
   }
}

int main (int argc, char ** argv) {

  const vrpn_int32 updateRate = 60;
  const int period = 5000;

  vrpn_Connection * localDeviceConnection;

  int count = 0;

  localDeviceConnection = new vrpn_Synchronized_Connection();

  ph = new vrpn_Phantom ("Phantom0", localDeviceConnection, updateRate);

  fdr = new vrpn_ForceDevice_Remote ("Phantom0", localDeviceConnection);
  tr = new vrpn_Tracker_Remote ("Phantom0", localDeviceConnection);

  //testOscillation();
  testRealData();

}
