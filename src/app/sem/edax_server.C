#include "nmm_Microscope_SEM_EDAX.h"

void handleQuit();

nmm_Microscope_SEM_EDAX *sem = NULL;

int main(int argc, char **argv)
{
#ifdef _WIN32
    WSADATA wsaData; 
    int status;
    if ((status = WSAStartup(MAKEWORD(1,1), &wsaData)) != 0) {
        fprintf(stderr, "WSAStartup failed with %d\n", status);
    }
#endif
  
    atexit(handleQuit);

    vrpn_Synchronized_Connection connection;
    sem = new nmm_Microscope_SEM_EDAX("SEM", &connection);

    while (1){
        sem->mainloop();
        connection.mainloop();
    }
    return 0;
}

void handleQuit()
{
    if (sem) {
      delete sem;
    }
}
