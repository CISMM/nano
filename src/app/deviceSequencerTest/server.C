#include "testDevice.h"

int main(int argc, char **argv)
{
#ifdef _WIN32
        WSADATA wsaData;
        int status;
        if ((status = WSAStartup(MAKEWORD(1,1), &wsaData)) != 0) {
         fprintf(stderr, "WSAStartup failed with %d\n", status);
        }
#endif

    vrpn_Synchronized_Connection *connection0, *connection1;
    connection0 = 
         new vrpn_Synchronized_Connection(vrpn_DEFAULT_LISTEN_PORT_NO);
    connection1 = 
         new vrpn_Synchronized_Connection(vrpn_DEFAULT_LISTEN_PORT_NO + 1);
    TestDeviceServer *device0, *device1;
    device0 = new TestDeviceServer("TestDevice0", connection0);
    device1 = new TestDeviceServer("TestDevice1", connection1);

    while (1){
        device0->mainloop();
        device1->mainloop();
        connection0->mainloop();
        connection1->mainloop();
    }
    return 0;
}
