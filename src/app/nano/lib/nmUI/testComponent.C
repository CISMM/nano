#include <tcl.h>

#include <Tcl_Linkvar.h>

#include <nmb_Decoration.h>
#include <nmb_Dataset.h>
#include <MicroscopeFlavors.h>

#include <nmg_Graphics.h>  // for graphics for tcl_tk

#include "nmui_Component.h"
#include "nmui_ComponentSync.h"

nmb_Decoration * decoration;
nmb_Dataset * dataset;

TopoFile GTF;

nmg_Graphics * graphics;

static void handle_synchronization (int, void * userdata) {
  nmui_ComponentSync * sync = (nmui_ComponentSync *) userdata;
  if (sync->currentlySendingRemoteSync()) {
    sync->update();
  }
}


int main (int argc, char ** argv) {

  vrpn_Connection * connection;
  Tcl_Interp * interp;
  nmui_Component * component;
  nmui_ComponentSync * cSync;

  if (argc == 1) {  // "server" startup
    connection = new vrpn_Connection;
  } else {
    connection = vrpn_get_connection_by_name (argv[1]);
  }

  interp = Tcl_CreateInterp();
  Tcl_Init(interp);
  Tclvar_init(interp);

  component = new nmui_Component (interp, "mycomponent");

  Tclvar_int myint ("myint", 0);

  component->add(&myint);

  cSync = new nmui_ComponentSync (connection, component);
  cSync->maintainSync();

  myint.addCallback(handle_synchronization, cSync);

  while (1) {
    connection->mainloop();
   
    Tclvar_mainloop();
 
  }

}
