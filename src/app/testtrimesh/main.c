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

// This can be felt!  But it's tiny!
//
static const double var [][3] = {
   { 0.05557, 0.02035, 0.07967 },
   { 0.05557, 0.02482, 0.08008 },
   { 0.05557, 0.02929, 0.08048 },
   { 0.05557, 0.03376, 0.08089 },
   { 0.05557, 0.02823, 0.08130 },
   { 0.05844, 0.02279, 0.08211 },
   { 0.05844, 0.02726, 0.08252 },
   { 0.05844, 0.03173, 0.08292 },
   { 0.05844, 0.03620, 0.08333 },
   { 0.05844, 0.04066, 0.08373 },
   { 0.06131, 0.02523, 0.08455 },
   { 0.06131, 0.02970, 0.08495 },
   { 0.06131, 0.03416, 0.08536 },
   { 0.06131, 0.03863, 0.08577 },
   { 0.06131, 0.04310, 0.08617 },
   { 0.06419, 0.02766, 0.08698 },
   { 0.06419, 0.03213, 0.08739 },
   { 0.06419, 0.03660, 0.08780 },
   { 0.06419, 0.04107, 0.08820 },
   { 0.06419, 0.04554, 0.08861 },
   { 0.06706, 0.03010, 0.08942 },
   { 0.06706, 0.03457, 0.08983 },
   { 0.06706, 0.03904, 0.09023 },
   { 0.06706, 0.04351, 0.09064 },
   { 0.06706, 0.04798, 0.09105 },
};

void testRealData (void) {

   const int xside = 5;
   const int yside = 5;
   int i, j, k, start;

   double nu [25][3];
   double mean [3];

   mean[0] = 0.0;
   mean[1] = 0.0;
   mean[2] = 0.0;

   for (i = 0; i < 25; i++) {
      mean[0] += var[i][0];
      mean[1] += var[i][1];
      mean[2] += var[i][2];
   }

   mean[0] /= 25;
   mean[1] /= 25;
   mean[2] /= 25;

   for (i = 0; i < 25; i++) {
      nu[i][0] = (var[i][0] - mean[0]) * 40;
      nu[i][1] = (var[i][1] - mean[1]) * 40;
      nu[i][2] = (var[i][2] - mean[2]) * 40;
      printf("%.5f %.5f %.5f\n", nu[i][0], nu[i][1], nu[i][2]);
   }


   fdr->clearTrimesh();

   for (i = 0; i < xside * yside; i++) {
      //fdr->setVertex(i, var[i][0], var[i][1], var[i][2]);
      fdr->setVertex(i, nu[i][0], nu[i][1], nu[i][2]);
   }

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
