/// truePlanes
/// Given a Phantom (Tracker+Button) streamfile and a surface snapshot (PPM),
/// computes the "true, exact" planes that should have been presented
/// to every phantom report that arrived while the button was held down.
///
/// Abandoned this code!
/// It looks like I'd have to reproduce large chunks of nano code and make
/// a bunch of assumptions;  easier is to just take a streamfile of "touch
/// stored" and assert that that is the correct surface.

// Nano PPMs put height in all three channels (RGB) - at least with the
// default settings I'm using (maybe they put colormap there if one is
// specified?).

//#include <PPM.h>
#include <BCGrid.h>

static const char * g_surfaceName;
static char g_connectionName [256];

static BCGrid * g_grid;
//static PPM * g_surface;
static vrpn_File_Connection * g_connection;

static TopoFile g_f;  // junk for BCGrid::readFile (?!)

vrpn_int32 g_buttonStatus = 0;

void usage (const char * argv0) {

  fprintf(stderr, "Usage:  %s <streamfile> <ppm>\n", argv0);

}

void ParseArgs (int argc, const char ** argv) {
  if (argc < 3) {
    usage(argv[0]);
  }

  if (strncmp(argv[1], "file:", 5)) {
    sprintf(g_connectionName, "file:%s", argv[1]);
  } else {
    sprintf(g_connectionName, "%s", argv[1]);
  }
  g_surfaceName = argv[2];

}

void LoadData (void) {

  g_connection = new vrpn_File_Connection (g_connectionName);

  if (!g_connection || !g_connection->doing_okay()) {
    fprintf(stderr, "Couldn't open streamfile named %s.\n", g_connectionName);
    exit(0);
  }

#if 0
  g_surface = new PPM (g_surfaceName);

  if (!g_surface) {
    fprintf(stderr, "Couldn't open surface named %s.\n", g_surfaceName);
    exit(0);
  }
#endif

  // Why does grid make us specify this junk when it could be read out
  // of the file?

  g_grid = new BCGrid (300, 300, 0, 300, 0, 300, READ_FILE,
                       g_surfaceName, g_f);

  if (!g_grid) {
    fprintf(stderr, "Couldn't create grid containing file %s.\n",
            g_surfaceName);
    exit(0);
  }
}

void handle_button (void *, vrpn_BUTTONCB p) {

  // More memos to self:  why doesn't VRPN include structures that keep
  // persistent values around?

  g_buttonStatus = p.state;
}

void handle_tracker (void * userdata, vrpn_TRACKERCB p) {
  if (!g_buttonStatus) {
    return;
  }

  // Now we reproduce most of nmui_HSCanned and interaction.c
  // do stuff with p.pos [3]
  // assume the surface is centered?  this could get very messy...



}

void main (int argc, const char ** argv) {

  vrpn_Button * b;
  vrpn_Tracker * t;

  ParseArgs(argc, argv);

  LoadData();

  // How do we know what names to assign to these objects?
  // Another unfortunate kluge required by VRPN that I don't yet
  // know a way around.  Introspection?

  b = new vrpn_Button_Remote ("Phantom0", g_connection);
  t = new vrpn_Tracker_Remote ("Phantom0", g_connection);

  b->register_change_handler(NULL, handle_button);
  t->register_change_handler(NULL, handle_tracker);

  while (g_connection && !g_connection->eof()) {
    g_connection->mainloop();
  }
}

