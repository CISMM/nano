#include "simulator_server.h"

#include <stdio.h>
// #include "vrpn_Connection.h"

#ifndef NMM_MICROSCOPE_SIMULATOR_H
#include "nmm_Microscope_Simulator.h"
#endif

#include "BCGrid.h"
#include "BCPlane.h"
#include "Topo.h" // added 4/19/99
#include <iostream.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <stdlib.h>

TopoFile GTF; // added 4/19/99

// Yucky that this is global, and scope doesn't regulate scanning within
// itself.
int currentline;

static int num_x, num_y;
static int g_isWaiting = 0;
static float g_waitTime = 0.0f;
  // How long to pause before each call to connection->mainloop(),
  // in seconds.  Gives us a cheap approximation of network latency.
static int g_isRude = 0;

struct timeval t_start, t_now;
struct timeval latency;
struct timeval mainloopDelay;
float point;
float diff_time;
int x, y;
long quitType;





int handle_quit (void * userdata, vrpn_HANDLERPARAM) 
{
  int * quitNow = (int *) userdata;
  printf("Quitting now...\n");
  *quitNow = 1;

  return 0;  // non-error completion
}

int handle_any_print (void * userdata, vrpn_HANDLERPARAM p) 
{
  vrpn_Connection * c = (vrpn_Connection *) userdata;

  //fprintf(stderr, "Got message \"%s\" from \"%s\".\n",
          //c->message_type_name(p.type), c->sender_name(p.sender));

  return 0;  // non-error completion
}




int initJake (int x, int y, int port) {  
  if (!x || !y) {
    fprintf(stderr, "initJake:  a 0-size grid makes microscopes crash!\n");
    return -1;
  }
  num_x = x;
  num_y = y;
  int quitNow = 0;
  latency.tv_sec = g_waitTime;
  latency.tv_usec = (g_waitTime - latency.tv_sec) * 1000000;
  currentline = 0;
  StartServer((num_x), (num_y), port);
  quitType = connection->register_message_type("Server Quit Type");
  connection->register_handler(quitType, handle_quit, &quitNow);
  connection->register_handler(vrpn_ANY_TYPE, handle_any_print, connection);
  gettimeofday(&t_start, NULL);
  return 0;
}



int jakeMain () 
{
  float scan_time_diff = .0050000;  	// Time between scanning new line

  if (g_isWaiting) {
    //fprintf(stderr, "Waiting...\n");
    latency.tv_sec = g_waitTime;
    latency.tv_usec = (g_waitTime - latency.tv_sec) * 1000000;
    select(0, NULL, NULL, NULL, &latency);
  }

  if (g_isRude) {
    mainloopDelay.tv_sec = 0L;
    mainloopDelay.tv_usec = 0L;
  } else {
    mainloopDelay.tv_sec = 0L;
    mainloopDelay.tv_usec = 1000L;
  }
  connection->mainloop(&mainloopDelay);

  gettimeofday(&t_now, NULL);
  diff_time = (t_now.tv_sec - t_start.tv_sec)
            + (t_now.tv_usec - t_start.tv_usec) / 1000000.0;
  if ((diff_time >= scan_time_diff) && (mic_Started() == 1)) {
    AFMSimulator->spm_report_window_line_data(currentline);
    gettimeofday(&t_start, NULL);
    currentline = ((currentline + 1) % (num_y - 1));
  }
  return 0; 
}

