#include "nmm_Microscope_SEM_EDAX.h"

void handleQuit();

nmm_Microscope_SEM_EDAX *sem = NULL;

int main(int argc, char **argv)
{
    int     port = vrpn_DEFAULT_LISTEN_PORT_NO;

#ifdef _WIN32
    WSADATA wsaData; 
    int status;
    if ((status = WSAStartup(MAKEWORD(1,1), &wsaData)) != 0) {
        fprintf(stderr, "WSAStartup failed with %d\n", status);
    }
#endif
  
//    atexit(handleQuit);

    if (argc > 1) {
		port = atoi(argv[1]);
	}

    vrpn_Synchronized_Connection connection(port);

#ifdef VIRTUAL_SEM
    sem = new nmm_Microscope_SEM_EDAX("SEM", &connection, vrpn_TRUE);
#else
    sem = new nmm_Microscope_SEM_EDAX("SEM", &connection, vrpn_FALSE);
#endif

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 10000;
 
    while (1){
        sem->mainloop();
        connection.mainloop(&timeout);
    }
    return 0;
}

void handleQuit()
{
    if (sem) {
      delete sem;
    }
}
