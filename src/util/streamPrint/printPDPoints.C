/// Print out data from a nmm_Microscope streamfile.
/// Lines are of the form "<timestamp> <x> <y> <z>", where <x>-<z>
/// are the positions of the point data reported at <timestamp>.
/// Timestamps are all normalized so that streamfiles start at 0.
/// Data to stdout, diagnostics to stderr.

#ifdef sgi
#include <unistd.h>
#endif

#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>

#include <vrpn_Shared.h>
#include <vrpn_Connection.h>
#include <vrpn_FileConnection.h>
 
// The "reuseful" thing to do would be to have a nmm_Microscope_Remote
// and just use its point data callback facility, but nMR is VERY heavyweight
// and drags a lot of other stuff along with it
//#include <nmm_MicroscopeRemote.h>

static void parseArguments (int argc, char ** argv);
static void handle_cntl_c (int);


static vrpn_Connection * connection = NULL;
static vrpn_File_Connection * fcon = NULL;

static int isReadingStreamFile = 0;
static char * device_name = NULL;

static vrpn_bool g_offsetInitialized = 0;
static timeval g_offset;

static int g_elapsed = 0;
static int g_normalize = 0;

//--------------------------------------------------------------------------
// Handles VRPN messages

// Since VRPN used inheritance when it should have used composition,
// we have to reproduce the code from vrpn_ForceDevice::decode_plane
// here (or create a class that inherits from it).

int handle_point (void *, vrpn_HANDLERPARAM p) {
  const char ** buf;
  vrpn_float32 x, y, z;
  vrpn_int32 sec, usec, fields;
  timeval elapsed;

  // Elapsed time from streamfiles is actually elapsed time since
  // the first user data.  So we hack here uglyly to display the
  // elapsed time since the first data we output.

  buf = &p.buffer;
  vrpn_unbuffer(buf, &x);
  vrpn_unbuffer(buf, &y);
  vrpn_unbuffer(buf, &sec);
  vrpn_unbuffer(buf, &usec);
  vrpn_unbuffer(buf, &fields);
  vrpn_unbuffer(buf, &z);

  if (g_elapsed) {
    connection->time_since_connection_open(&elapsed);

    if (g_normalize) {
      if (!g_offsetInitialized) {
        g_offsetInitialized = 1;
        g_offset = elapsed;
      }

      elapsed = vrpn_TimevalDiff(elapsed, g_offset);
    }

    printf("%d.%06d %.6f %.6f %.6f\n",
           elapsed.tv_sec, elapsed.tv_usec, x, y, z);
  } else {
    printf("%d.%06d %.6f %.6f %.6f\n",
           p.msg_time.tv_sec, p.msg_time.tv_usec, x, y, z);
  }

  return 0;
}


// Argument handling
void usage (char * program_name) {
  fprintf(stderr, "Usage: %s [-i streamfile]", program_name);
  fprintf(stderr, " [-d device] [-elapsed] [-normalize]\n");
  exit(-1);
}

void parseArguments(int argc, char ** argv) {
  int i;
  for (i = 1; i < argc; i++){
    if (!strcmp(argv[i], "-d")){
      if (++i >= argc) usage(argv[0]);
      device_name = strdup(argv[i]);
    } else if (!strcmp(argv[i], "-i")){
      if (++i >= argc) usage(argv[0]);
      isReadingStreamFile = 1;
      device_name = new char [5 + strlen(argv[i]) + 1];
      sprintf(device_name,"file:%s", argv[i]);
    } else if (!strcmp(argv[i], "-elapsed")){
      g_elapsed = 1;
    } else if (!strcmp(argv[i], "-normalize")){
      g_normalize = 1;
    } else {
      usage(argv[0]);
    }
  }
  fprintf(stderr, "Reading from %s\n", device_name);
}

int main (unsigned argc, char ** argv) {

  vrpn_int32 resultData_type;
  vrpn_bool done;
    
  done = VRPN_FALSE;
  parseArguments(argc, argv);
    
  // Initialize our connections to the things we are going to control.
  if (!device_name) {
    return -1;
  } 
  connection = vrpn_get_connection_by_name(device_name);
  if (!connection) {
    // connection failed. VRPN prints error message.
    return -1;
  }

  if (isReadingStreamFile) {
    fcon = connection->get_File_Connection();
    if (!fcon) {
      fprintf(stderr, "Error: expected but didn't get file connection\n");
      exit(0);
    }
    fcon->set_replay_rate(1000.0);
  }

  signal(SIGINT, handle_cntl_c);

  resultData_type = connection->register_message_type
                           ("nmm Microscope PointResultData");
  connection->register_handler(resultData_type, handle_point, NULL);

  while (!done) {
    //sleep(0.01);
    vrpn_SleepMsecs(10.0);
      
    //------------------------------------------------------------
    // Send/receive message from our vrpn connections.
    connection->mainloop();

    if (fcon && fcon->eof()) {
      done = VRPN_TRUE;
    }
  }

  if (connection)  {
    delete connection; // needed to make stream file write out
  }
     
  return 0;
}


void handle_cntl_c (int) {
  fprintf(stderr, "Received ^C signal, shutting down and saving log file.\n");
  if (connection) {
    delete connection; // needed to make stream file write out
  }

  exit(-1);
}
