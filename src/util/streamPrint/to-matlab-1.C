// computes verterr; outputs ideal-Z coordinate, ideal-Z plane normal Z
// component, and verterr to a file to be read into matlab to be plotted

#ifdef sgi
#include <unistd.h>
#endif

#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>  // bzero

#include <math.h>

#include <vrpn_Shared.h>
#include <vrpn_Connection.h>
#include <vrpn_FileConnection.h>



vrpn_Connection * g_pointC;
vrpn_Connection * g_planeC;
vrpn_Connection * g_iPlaneC;
vrpn_File_Connection * g_pointFC;
vrpn_File_Connection * g_planeFC;
vrpn_File_Connection * g_iPlaneFC;

// Must unpack these things as float32, since that's how they're packed.
// Might want to copy into float64 if we're paranoid about precision.
vrpn_float32 g_x, g_y, g_z, g_v;
vrpn_float32 g_ix, g_iy, g_iz, g_iv;

//int g_pass = 0;

vrpn_int32 g_numPlanes = 0;
vrpn_float64 g_totalVerterr = 0.0;
vrpn_float64 g_totalSquaredV = 0.0;
vrpn_float64 g_meanVerterr = 0.0;
//vrpn_float64 g_totalDeviation = 0.0;
vrpn_float64 g_standardDeviation = 0.0;

int handle_plane (void * userdata, vrpn_HANDLERPARAM p) {
  const char * mptr;

// Plane coordinates appear to be in meters;  this is inferred from
// lots of other things in the ForceDevice header files being documented
// as being in units of meters.

//fprintf(stderr, "plane at %ld.%ld.\n", p.msg_time.tv_sec, p.msg_time.tv_usec);
//fprintf(stderr, "l");

  mptr = p.buffer;
  vrpn_unbuffer(&mptr, &g_x);
  vrpn_unbuffer(&mptr, &g_y);
  vrpn_unbuffer(&mptr, &g_z);
  vrpn_unbuffer(&mptr, &g_v);

//fprintf(stderr, "plane at %.5f %.5f %.5f %.5f\n", g_x, g_y, g_z, g_v);

  //g_x *= 1000.0;
  //g_y *= 1000.0;
  //g_z *= 1000.0;
  //g_v *= 1000.0;

  return 0;
}


int handle_iPlane (void * userdata, vrpn_HANDLERPARAM p) {
  const char * mptr;

//fprintf(stderr, "\t\t\tplane at %ld.%ld.\n", p.msg_time.tv_sec, p.msg_time.tv_usec);
//fprintf(stderr, "i");

  mptr = p.buffer;
  vrpn_unbuffer(&mptr, &g_ix);
  vrpn_unbuffer(&mptr, &g_iy);
  vrpn_unbuffer(&mptr, &g_iz);
  vrpn_unbuffer(&mptr, &g_iv);

//fprintf(stderr, "\t\t\t\t\tplane at %.5f %.5f %.5f %.5f\n",
//g_ix, g_iy, g_iz, g_iv);

  //g_ix *= 1000.0;
  //g_iy *= 1000.0;
  //g_iz *= 1000.0;
  //g_iv *= 1000.0;

  return 0;
}



int handle_point (void * userdata, vrpn_HANDLERPARAM p) {
  const char * mptr;

  vrpn_int32 scrap;
  vrpn_float64 x, y, z;
  vrpn_float64 e = 0.0, d;
  vrpn_float64 f, v;
  timeval ept;

// Tracker position callbacks APPEAR TO be in meters
// This is nowhere stated in the header file or source code (?),
// but is taken from an example on the web page where the units
// of a transform matrix in the config file are commented as
// meters.
//    TCH 19 June 2002

//fprintf(stderr, "Point at %ld.%ld.\n", p.msg_time.tv_sec, p.msg_time.tv_usec);

  // Find the plane in effect at the time of this point
  if (!g_planeFC->eof()) {
    g_planeFC->play_to_filetime(p.msg_time);
  }
  // iPlaneFC is in a different timeframe, so we need to convert
  // to elapsed time - it's the same phantom path being played,
  // so this ought to be appropriate.
  if (!g_iPlaneFC->eof()) {
    //ept = g_planeFC->get_length();
    g_planeFC->time_since_connection_open(&ept);
//fprintf(stderr, "g_iPlaneFC will play to %d.%d\n", ept.tv_sec, ept.tv_usec);
    g_iPlaneFC->play_to_time(ept);
  }
  if (g_planeFC->eof() || g_iPlaneFC->eof()) {
    // If we're off the end of that streamfile, there wasn't any FFB,
    // so we ignore this point.
    return 0;
  }
//fprintf(stderr, "o");

  // unpack x, y coordinates
  mptr = p.buffer;
  vrpn_unbuffer(&mptr, &scrap);
  vrpn_unbuffer(&mptr, &scrap);
  vrpn_unbuffer(&mptr, &x);
  vrpn_unbuffer(&mptr, &y);
  vrpn_unbuffer(&mptr, &z);

  // Compute this Z measurement
  if ((g_z != 0.0) && (g_iz != 0.0)) {

//fprintf(stderr, "g_iPlaneFC played to %d.%d\n", ept.tv_sec, ept.tv_usec);

    //x *= 1000.0;
    //y *= 1000.0;
    //z *= 1000.0;

    e = (x*g_x + y*g_y + g_v) / g_z;  // projection onto felt plane
    f = (x*g_ix + y*g_iy + g_iv) / g_iz;  // projection onto ideal
    v = fabs(e - f);  // difference = vertical error
//fprintf(stderr, "p1 %.5f %.5f %.5f %.5f p2 %.5f %.5f %.5f %.5f\n",
//g_x, g_y, g_z, g_v, g_ix, g_iy, g_iz, g_iv);
//fprintf(stderr, "Felt distance %.5f, ideal %.5f => %.5f error.\n",
//e, f, v);

//printf("point %.5f %.5f %.5f plane %.5f %.5f %.5f %.5f e %.5f\n",
//x, y, z, g_x, g_y, g_z, g_v, e);


/* Apparent error found 2 Jun 2002
 * f wasn't the same for different runs (e.g. 0 ms and 270 ms delay);
 *   shouldn't it be?!  
 * Added output of x, y for debugging.
 */

    // This is the line that outputs for matlab
    //printf("%.5f %.5f %.5f\n", f, g_iz, v);
    printf("%.5f %.5f %.5f %.5f %.5f\n", f, x, y, g_iz, v);

    // Some problems 31 Mar 02 -
    //   tried accumulating fabs(verterr) instead of verterr

    g_numPlanes++;
    g_totalVerterr += v;
    g_totalSquaredV += (v * v);
  } else {
    // If we haven't yet received a plane from the microscope, there
    // wasn't any FFB, so we ignore this point.
    //printf("0z\n");
  }

  return 0;
}


void main (int argc, char ** argv) {
  char point_name [512];
  char plane_name [512];
  char iPlane_name [512];
  vrpn_int32 point_id;
  vrpn_int32 plane_id;
  vrpn_int32 iPlane_id;
  int arg;
  vrpn_bool done = VRPN_FALSE;

  sprintf(iPlane_name, "file:%s", argv[1]);

  // run mainloop

  for (arg = 2; arg < argc; arg++) {

    g_iPlaneC = vrpn_get_connection_by_name(iPlane_name);
    if (g_iPlaneC) {
      g_iPlaneFC = g_iPlaneC->get_File_Connection();
      if (g_iPlaneFC) {
        g_iPlaneFC->set_replay_rate(1000.0);
      }
    }

    g_pointC = NULL;
    g_planeC = NULL;
    if (argv[arg][0] != '-') {
      sprintf(point_name, "file:%s", argv[arg]);
      g_pointC = vrpn_get_connection_by_name(point_name);
fprintf(stderr, "Opening %s", point_name);
      bzero(plane_name, 512);
      strncpy(plane_name, point_name,
              strstr(point_name, ".phantom") - point_name);
      strcat(plane_name, ".surface");
      g_planeC = vrpn_get_connection_by_name(plane_name);
fprintf(stderr, " and %s.\n", plane_name);

      if (g_pointC) {
        g_pointFC = g_pointC->get_File_Connection();
        if (g_pointFC) {
          g_pointFC->set_replay_rate(1000.0);
        }
      }
      if (g_planeC) {
        g_planeFC = g_planeC->get_File_Connection();
        if (g_planeFC) {
          g_planeFC->set_replay_rate(1000.0);
        }
      }
    }

    if (!g_pointFC || !g_planeFC || !g_iPlaneFC) {
      return;
    }

    point_id = g_pointC->register_message_type("vrpn_Tracker Pos_Quat");
    g_pointC->register_handler(point_id, handle_point, NULL);
    plane_id = g_planeC->register_message_type("vrpn_ForceDevice Plane");
    g_planeC->register_handler(plane_id, handle_plane, NULL);
    iPlane_id = g_iPlaneC->register_message_type("vrpn_ForceDevice Plane");
    g_iPlaneC->register_handler(iPlane_id, handle_iPlane, NULL);


    printf("%% This file is meant to be read by matlab.\n");
    printf("%% It was generated from:\n");
    printf("%%   %s\n", iPlane_name);
    printf("%%   %s\n", point_name);
    printf("%%   %s\n", plane_name);
    printf("%% First column is Z coordinate of \"ideal\" plane.\n");
    printf("%% Second column is Z component of \"ideal\" plane normal.\n");
    printf("%% Third column is vertical error (difference in Z coordinates).\n");

    while (!done) {
      //sleep(0.01);
      vrpn_SleepMsecs(10.0);
      
      //------------------------------------------------------------
      // Send/receive message from our vrpn connections.
      g_pointC->mainloop();

      if (g_pointFC && g_pointFC->eof()) {
        done = VRPN_TRUE;
      }
    }
  
    done = VRPN_FALSE;
  }

  if (g_numPlanes > 0) {
    g_meanVerterr = g_totalVerterr / g_numPlanes;
  }
  if (g_numPlanes > 1) {
    g_standardDeviation =
      sqrt((g_totalSquaredV -
            g_numPlanes * g_meanVerterr * g_meanVerterr) /
           (g_numPlanes - 1));
  }

  fprintf(stderr, "%d points with mean %f, sigma %f.\n",
          g_numPlanes, g_meanVerterr, g_standardDeviation);
}

