#ifndef _WIN32
#include <signal.h>
#endif

#include "nmr_Registration_Server.h"
#include "nmr_Registration_Impl.h"

void Usage (const char * s);
void sighandler (int);
void shutDown (void);
int handle_dlc (void *, vrpn_HANDLERPARAM p);
void idlefunc();

static int verbose = 1;
vrpn_Connection *connection = NULL;
nmr_Registration_Server *alignerServer = NULL;
nmr_Registration_Impl *alignerImpl = NULL;

int main(int argc, char **argv)
{
    char    * client_name = NULL;
    int     client_port;
    int     realparams = 0;
    int     auto_quit = 0;
    int     port = vrpn_DEFAULT_LISTEN_PORT_NO;
    int     milli_sleep_time = 0;  // How long to sleep each iteration?

#ifdef _WIN32
    WSADATA wsaData; 
    int status;
    if ((status = WSAStartup(MAKEWORD(1,1), &wsaData)) != 0) {
        fprintf(stderr, "WSAStartup failed with %d\n", status);
    }
#else
#ifdef sgi
    sigset( SIGINT, sighandler );
    sigset( SIGKILL, sighandler );
    sigset( SIGTERM, sighandler );
    sigset( SIGPIPE, sighandler );
#else
    signal( SIGINT, sighandler );
    signal( SIGKILL, sighandler );
    signal( SIGTERM, sighandler );
    signal( SIGPIPE, sighandler );
#endif // sgi
#endif

    // Parse the command line
    int i = 1;
    while (i < argc) {
      if (!strcmp(argv[i], "-millisleep")) { // msec to sleep each loop?
            if (++i > argc) { Usage(argv[0]); }
            milli_sleep_time = atoi(argv[i]);
      } else if (!strcmp(argv[i], "-v")) {  // Verbose
            verbose = 1;
      } else if (!strcmp(argv[i], "-q")) {  // quit on dropped last con
            auto_quit = 1;
      } else if (!strcmp(argv[i], "-client")) { // specify a waiting client
            if (++i > argc) { Usage(argv[0]); }
            client_name = argv[i];
            if (++i > argc) { Usage(argv[0]); }
            client_port = atoi(argv[i]);
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

    connection = new vrpn_Synchronized_Connection (port);
    alignerServer = new nmr_Registration_Server("Aligner", connection);

    /* you need to initialize glut to use the user interface which is
       in nmr_Registration_Impl */
#ifdef V_GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutIdleFunc(idlefunc);
#endif

    alignerImpl = new nmr_Registration_Impl(alignerServer);

    if (auto_quit) {
        int dlc_m_id = connection->register_message_type(
                                        vrpn_dropped_last_connection);
        connection->register_handler(dlc_m_id, handle_dlc, NULL);
    }

    if (client_name) {
        fprintf(stderr, "vrpn_serv: connecting to client: %s:%d\n",
            client_name, client_port);
        if (connection->connect_to_client(client_name, client_port)){
            fprintf(stderr, "server: could not connect to client %s:%d\n",
                    client_name, client_port);
            shutDown();
        }
    }

#ifdef V_GLUT
    glutMainLoop();
#else
    while (connection->doing_okay()){
        idlefunc();
        // Sleep so we don't eat the CPU
        if (milli_sleep_time > 0) {
            vrpn_SleepMsecs(milli_sleep_time);
        }
    }
    fprintf(stderr, "reg_server: quitting because of connection failure\n");
#endif
    return 0;
}

void idlefunc() {
  alignerImpl->mainloop();
  connection->mainloop();
  if (!connection->doing_okay()){
     fprintf(stderr, "reg_server: quitting because of connection failure\n");
     exit(0);
  }
}

void Usage (const char * s)
{
  fprintf(stderr,"Usage: %s [-v] [port] [-q]\n",s);
  fprintf(stderr,"       [-client machinename port] [-millisleep n]\n");
  fprintf(stderr,"       -v: Verbose\n");
  fprintf(stderr,"       -q: quit when last connection is dropped\n");
  fprintf(stderr,"       -client: where server connects when it starts up\n");
  fprintf(stderr,"       -millisleep: sleep n milliseconds each loop cycle\n");
  exit(-1);
}

int handle_dlc (void *, vrpn_HANDLERPARAM)
{
    fprintf(stderr, "reg_server: dropped last connection, quitting\n");
    shutDown();
    return 0;
}

void sighandler (int)
{
    fprintf(stderr, "reg_server: got signal to quit\n");
    shutDown();
}

void shutDown (void)
{
    delete alignerServer;
    delete alignerImpl;
    delete connection;
    exit(0);
    return;
}
