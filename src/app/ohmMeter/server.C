#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include "vrpn_Connection.h"
#include "vrpn_Ohmmeter.h"

void Usage (const char * s);

void main (unsigned argc, char *argv[])
{
    int     bail_on_error = 1;
    int     verbose = 1;
    int     realparams = 0;
    int     port = vrpn_DEFAULT_LISTEN_PORT_NO;
    int     milli_sleep_time = 0;  // How long to sleep each iteration?
    float   update_rate = 2;

    // Parse the command line
    int i = 1;
    while (i < argc) {
      if (!strcmp(argv[i], "-millisleep")) { // msec to sleep each loop?
            if (++i > argc) { Usage(argv[0]); }
            milli_sleep_time = atoi(argv[i]);
      } else if (!strcmp(argv[i], "-v")) {  // Verbose
            verbose = 1;
      } else if (!strcmp(argv[i], "-warn")) {// Don't bail on errors
                bail_on_error = 0;
      } else if (!strcmp(argv[i], "-f")) {// update rate
          if (++i > argc) {Usage(argv[0]);}
          update_rate = atof(argv[i]);
      } else if (argv[i][0] == '-') {       // Unknown flag
            Usage(argv[0]);
      } else switch (realparams) {
        case 0:
            port = atoi(argv[i]);
            realparams++;
            break;
        default:
            Usage(argv[0]);
      }
      i++;
    }

#ifdef _WIN32
    WSADATA wsaData; 
    int status;
    if ((status = WSAStartup(MAKEWORD(1,1), &wsaData)) != 0) {
        fprintf(stderr, "WSAStartup failed with %d\n", status);
    } else if(verbose) {
        fprintf(stderr, "WSAStartup success\n");
    }
#endif

    vrpn_Synchronized_Connection	connection(port);
    vrpn_Ohmmeter	*ohmmeter;

    if (verbose) {
        printf("Opening vrpn_Ohmmeter_ORPX2: %s on port %d\n", "Ohmmeter",port);
    }
#if defined(_WIN32) && !defined(FAKE_SERVER)
    ohmmeter = new vrpn_Ohmmeter_ORPX2("Ohmmeter@wherever", 
                                        &connection, update_rate);
#else
    printf("Warning: This is the FAKE ohmmeter server (for testing)\n");
    ohmmeter = new vrpn_Ohmmeter_Server("Ohmmeter@wherever",
                                         &connection, update_rate);
#endif
    if (!ohmmeter) {
         fprintf(stderr,"Error: Can't allocate ohmmeter\n");
         exit(-1);
    }

    // Loop forever calling the mainloop()s for all devices
    while (connection.doing_okay()) {
	ohmmeter->mainloop();

	// Send and receive all messages
	connection.mainloop();
        if (milli_sleep_time > 0) {
            vrpn_SleepMsecs(milli_sleep_time);
        }
    }

}

void    Usage(const char *s)
{
  fprintf(stderr,"Usage: %s [-f rate] [-warn] [-v] [port]\n",s);
  fprintf(stderr,"       [-millisleep n]\n");
  fprintf(stderr,"       -f rate: update rate in Hz\n");
  fprintf(stderr,"       -warn: Only warn on errors (default is to bail)\n");
  fprintf(stderr,"       -v: Verbose\n");
  fprintf(stderr,"       port: which port to use (dddd)\n");
  fprintf(stderr,"       -millisleep: sleep n milliseconds each loop cycle\n");
  exit(-1);
}
