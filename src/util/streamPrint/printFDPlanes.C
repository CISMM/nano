/// Print out data from a vrpn_ForceDevice_Remote outgoing streamfile.
/// Lines are of the form "<timestamp> <a> <b> <c> <d>", where <a>-<d>
/// are the parameters of the plane specified at <timestamp>.
/// Data to stdout, diagnostics to stderr.
/// We could inherit from vrpn_ForceDevice to get access to encode,
/// decode, and message type IDs, but instead reimplement those tiny
/// pieces from scratch.  Memo to self:  inheritance != composition.

#include <vrpn_Shared.h>
#include <vrpn_Connection.h>
#include <vrpn_FileConnection.h>
 
//#include <vrpn_ForceDevice.h>  // Not strictly necessary!

#ifdef sgi
#include <unistd.h>
#endif

#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>


static void parseArguments (int argc, char ** argv);
static void handle_cntl_c (int);


static 	vrpn_Connection * connection = NULL;
static   vrpn_File_Connection * fcon = NULL;

static int isReadingStreamFile = 0;
static char * device_name = NULL;

//--------------------------------------------------------------------------
// Handles VRPN messages

// Since VRPN used inheritance when it should have used composition,
// we have to reproduce the code from vrpn_ForceDevice::decode_plane
// here (or create a class that inherits from it).

int handle_plane (void * userdata, vrpn_HANDLERPARAM p) {
  const char * mptr;
  vrpn_float32 a, b, c, d;
  //vrpn_float32 kspring, kdamp, fdyn, fstat;
  //vrpn_int32 index, cycles;
  int i;

  mptr = p.buffer;
  vrpn_unbuffer(&mptr, &a);
  vrpn_unbuffer(&mptr, &b);
  vrpn_unbuffer(&mptr, &c);
  vrpn_unbuffer(&mptr, &d);
  //vrpn_unbuffer(&mptr, &kspring);
  //vrpn_unbuffer(&mptr, &kdamp);
  //vrpn_unbuffer(&mptr, &fdyn);
  //vrpn_unbuffer(&mptr, &fstat);
  //vrpn_unbuffer(&mptr, &index);
  //vrpn_unbuffer(&mptr, &cycles);

  printf("%d.06%d %.6f %.6f %.6f %.6f\n",
         p.msg_time.tv_sec, p.msg_time.tv_usec, a, b, c, d);

  return 0;
}




// Argument handling
void usage (char * program_name) {
  fprintf(stderr, "Usage: %s [-i streamfile]", program_name);
  fprintf(stderr, " [-d device]\n");
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
    } else {
      usage(argv[0]);
    }
  }
  fprintf(stderr, "Reading from %s\n", device_name);
}

int main (unsigned argc, char ** argv) {
  vrpn_int32 plane_id;
  vrpn_bool done = VRPN_FALSE;
    
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

  plane_id = connection->register_message_type("vrpn_ForceDevice Plane");
  connection->register_handler(plane_id, handle_plane, NULL);

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
