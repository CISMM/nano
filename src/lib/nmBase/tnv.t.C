#include <stdio.h>  // for printf()
#include <unistd.h>  // for sleep()

#include <vrpn_Connection.h>
#include <Tcl_Netvar.h>

// externs for nM
#include <nmb_Decoration.h>
#include <nmb_Dataset.h>
#include <nmg_Graphics.h>
nmb_Decoration * decoration;
nmb_Dataset * dataset;
nmg_Graphics * graphics;

// tnv.t.C
// Test code for Tcl_Netvar.
//
// Current version instantiates a TclNet_int and increments it,
// verifying that the increments are received properly.  Preincrement,
// postincrement, and assignment *should* all be satisfied by the
// same test case.

void Usage (char * argv0) {
  printf("%s <port> <peer-address> <mode>\n"
         "    <port> is the port to use as a server.\n"
         "    <peer-address> is a VRPN location specifier, "
              "usually <host>:<port>\n"
         "    <mode> is 's' or 'r'\n",
         argv0);
}

void serverCB (int new_value, void * ) {

  printf("Server is now %d", new_value);
}

void clientCB (int new_value, void * ) {

  printf("Client is now %d", new_value);
}

int main (int argc, char ** argv) {

  vrpn_Connection * c;
  vrpn_Connection * pc;
  char mode;
  timeval delay;

  if (argc != 4) {
    Usage(argv[0]);
    exit(0);
  }

  mode = argv[3][0];

fprintf(stderr, "Opening server-c on %s.\n", argv[1]);
  c = new vrpn_Synchronized_Connection (atoi(argv[1]));
  c->mainloop();
  if (mode == 's') {
fprintf(stderr, "Server delaying...  \n");
    delay.tv_sec = 30L;
    delay.tv_usec = 0L;
    c->mainloop(&delay);
fprintf(stderr, "Free.\n");
  }
fprintf(stderr, "Opening peer-c to %s.\n", argv[2]);
  pc = vrpn_get_connection_by_name (argv[2]);
  pc->mainloop();
  c->mainloop();

  TclNet_int ti ("testInt", 32);
  ti.bindConnection(c);
  ti.addPeer(pc);

  switch (mode) {
    case 's':
      ti.addCallback(serverCB, &ti);
      break;
    case 'r':
      ti.addCallback(clientCB, &ti);
      ti.syncReplica(1);
      break;
  }

  while (c->doing_okay()) {
    switch (mode) {
      case 's':
        sleep(3);
        ti++;
fprintf(stderr, "Incremented server-value to %d.\n", ti.myint);
        break;
      case 'r':
        sleep(1);
        // DO NOTHING
fprintf(stderr, "Remote-value is %d.\n", ti.myint);
        break;
    }
    c->mainloop();
    pc->mainloop();
  }

  delete pc;
  delete c;

}

