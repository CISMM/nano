// renderer.C
// Tom Hudson, September 1998
//
// This is the core of a process to render frames for the nanoManipulator.
// Controlled by an nmm_Graphics_Remote.  Has an nmm_Microscope_Remote
// to get the dataset and microscope state to be rendered from the
// microscope.

#include <stdio.h>
#include <signal.h>

#include <nmb_Decoration.h>
#include <nmb_Dataset.h>
#include <nmm_MicroscopeRemote.h>
#include <nmg_GraphicsImpl.h>

// HACK
// Copied from microscape.c
#define DATA_SIZE 128
#define MAXFILES 30
#include <Topo.h>
TopoFile GTF;

// These variables are global because
//   1)  they have to be global to use scope/ and graphics/
//   (maybe someday some brave soul will fix that, but it'll take
//   an awful lot of work)
//   2)  need to be seen by the signal handler so they can be shut
//   down nicely.

nmb_Dataset * dataset = NULL;
nmb_Decoration * decoration = NULL;
nmm_Microscope_Remote * microscope = NULL;
nmg_Graphics_Implementation * graphics = NULL;

// These variables are global so we can shut them down gracefully
// by the signal handler

vrpn_Connection * microscope_connection = NULL;
vrpn_Connection * graphics_connection = NULL;

// RendererInitializationState carries everything that can be set
// by command line arguments.
// Default values are initialized in constructor.

static char tcl_default_dir [] = "/afs/cs.unc.edu/project/stm/bin/";
static char defaultColormapDirectory [] = "/afs/unc/proj/stm/etc/colormaps";

struct RendererInitializationState {
  AFMInitializationState afm;
  char * microscopeName;

  int num_x, num_y;
  float x_min, x_max;
  float y_min, y_max;
  int read_mode;

  int num_stm_files;
  char * stm_file_names [MAXFILES];

//  int instream_rate;

  char * tcl_script_dir;
  char * colorMapDir;

  float alpha_red, alpha_green, alpha_blue;
  int minColor [3];
  int maxColor [3];

  RendererInitializationState (void);
};

RendererInitializationState::RendererInitializationState (void) :

  microscopeName (NULL),
  num_x (DATA_SIZE),
  num_y (DATA_SIZE),
  x_min (afm.xMin),
  x_max (afm.xMax),
  y_min (afm.yMin),
  y_max (afm.yMax),
  read_mode (READ_DEVICE),

  num_stm_files (0),

//  instream_rate (1),

  tcl_script_dir (tcl_default_dir),
  colorMapDir (defaultColormapDirectory),

  alpha_red (0.0f),
  alpha_green (1.0f),
  alpha_blue (0.0f)

{
  int i;

  for (i = 0; i < 3; i++) {
    minColor[i] = 255;  // ?
      // minColor = 255 is copied out of microscape code, but I don't
      // believe it.
    maxColor[i] = 255;
  }
}

// Die, gracefully.

void handle_interrupt (int) {

  fprintf(stderr, "Got an interrupt...\n");

  if (graphics) {
    delete graphics;
    graphics = NULL;
  }
  if (microscope){
    delete microscope;
    microscope = NULL;
  }
  if (dataset){
    delete dataset;
    dataset = NULL;
  }
  if (decoration){
    delete decoration;
    decoration = NULL;
  }

  if (microscope_connection){
    delete microscope_connection;
    microscope_connection = NULL;
  }
  if (graphics_connection){
    delete graphics_connection;
    graphics_connection = NULL;
  }

  fprintf(stderr, "Shut down.\n");

  exit(0);
}

// Quit, gracefully.
// TODO:  add more command-line options from microscape.
//        crib the text directly from there for familiarity's sake.

void usage (const char * argv0) {
  fprintf(stderr, "Usage:  %s -d device [-grid <x> <y>]\n", argv0);
  fprintf(stderr, "        [-region lowx lowy highx highy]\n");
  fprintf(stderr, "        [-color hr hg hb lr lg lb]\n");
  fprintf(stderr, "        [-alphacolor r g b]\n");
  fprintf(stderr,
          "    -d:  Use given device (no default;  mandatory).\n");
  fprintf(stderr,
          "    -grid:  Take x by y samples for the grid.\n");
  fprintf(stderr,
          "    -region:  Scan from low to high in x and y (units are nm).\n");
  fprintf(stderr,
          "    -color:  high to low color range (Red to Blue default).\n");
  fprintf(stderr,
          "    -alphacolor:  color for alpha texture (default: 0 1 0).\n");

}

// Parse argv and write the values into s.
// Return nonzero on failure.
// TODO:  add more command-line options from microscape.

int parse (RendererInitializationState * s, int argc, char ** argv) {

  int ret = 0;
  int i;

  i = 1;
  while (i < argc) {
    fprintf(stderr, "parse:  arg %d %s\n", i, argv[i]);

    if (!strcmp(argv[i], "-d")) {
      if (++i >= argc) { ret = 1; continue; }
      s->microscopeName = argv[i];
    } else if (strcmp(argv[i], "-grid") == 0) {
      if (++i >= argc) usage(argv[0]);
      s->num_x = atoi(argv[i]);
      if (++i >= argc) usage(argv[0]);
      s->num_y = atoi(argv[i]);
    } else if (strcmp(argv[i], "-region") == 0) {
      if (++i >= argc) usage(argv[0]);
      s->x_min = atof(argv[i]);
      if (++i >= argc) usage(argv[0]);
      s->y_min = atof(argv[i]);
      if (++i >= argc) usage(argv[0]);
      s->x_max = atof(argv[i]);
      if (++i >= argc) usage(argv[0]);
      s->y_max = atof(argv[i]);
      printf("Will set scan region (%g,%g) to (%g,%g)\n",
             s->x_min, s->y_min, s->x_max, s->y_max);
    } else if (strcmp(argv[i], "-color") == 0) {
      if (++i >= argc) usage(argv[0]);
      s->maxColor[0] = atoi(argv[i]);
      if (++i >= argc) usage(argv[0]);
      s->maxColor[1] = atoi(argv[i]);
      if (++i >= argc) usage(argv[0]);
      s->maxColor[2] = atoi(argv[i]);
      if (++i >= argc) usage(argv[0]);
      s->minColor[0] = atoi(argv[i]);
      if (++i >= argc) usage(argv[0]);
      s->minColor[1] = atoi(argv[i]);
      if (++i >= argc) usage(argv[0]);
      s->minColor[2] = atoi(argv[i]);
    } else if (strcmp(argv[i], "-alphacolor") == 0) {
      if (++i >= argc) Usage(argv[0]);
      s->alpha_red = atof(argv[i]);
      if (++i >= argc) Usage(argv[0]);
      s->alpha_green = atof(argv[i]);
      if (++i >= argc) Usage(argv[0]);
      s->alpha_blue = atof(argv[i]);
    } else
      ret = 1;

    i++;
  }

  // check for mandatory arguments
  if (!s->microscopeName) ret = 1;

  return ret;
}

int main (int argc, char ** argv) {

  RendererInitializationState s;

  int retval;

  retval = parse(&s, argc, argv);
  if (retval) {
    usage(argv[0]);
    exit(0);
  }

  // Some bright day we might not need this!
  s.tcl_script_dir = getenv("NM_TCL_DIR");
  if (!s.tcl_script_dir)
    s.tcl_script_dir = tcl_default_dir;

  s.colorMapDir = getenv("NM_COLORMAP_DIR");
  if (!s.colorMapDir)
    s.colorMapDir = defaultColormapDirectory;

  dataset = new nmb_Dataset (s.num_x, s.num_y,
                             s.x_min, s.x_max,
                             s.y_min, s.y_max, s.read_mode,
                             (const char **) s.stm_file_names,
                             s.num_stm_files);
  if (!dataset) {
    fprintf(stderr, "%s:  out of memory.  Exiting.\n", argv[0]);
    exit(0);
  }

  decoration = new nmb_Decoration;
  if (!decoration) {
    fprintf(stderr, "%s:  out of memory.  Exiting.\n", argv[0]);
    exit(0);
  }
//  decoration->rateOfTime = s.instream_rate;

  microscope_connection = vrpn_get_connection_by_name (s.microscopeName);
  if (!microscope_connection) {
    fprintf(stderr, "%s:  could not connect to microscope.  Exiting.\n",
            argv[0]);
    //exit(0);
  }

  microscope = new nmm_Microscope_Remote (s.afm, microscope_connection);
  if (!microscope) {
    fprintf(stderr, "%s:  out of memory.  Exiting.\n", argv[0]);
    exit(0);
  }

  microscope->InitializeDataset(dataset);
  microscope->InitializeDecoration(decoration);
  microscope->InitializeTcl(s.tcl_script_dir);
  //setupStateCallbacks(microscope);
    // TODO:  generalize or rewrite or determine we don't need this



  graphics_connection = new vrpn_Synchronized_Connection;
    // TODO:  read port # from command line
    // TODO:  accept files to replay
  if (!graphics_connection) {
    fprintf(stderr, "%s:  could not open up port for controller.  Exiting.\n",
            argv[0]);
    exit(0);
  }

  graphics = new nmg_Graphics_Implementation
    (dataset, s.minColor, s.maxColor, NULL, graphics_connection);
  if (!graphics) {
    fprintf(stderr, "%s:  out of memory.  Exiting.\n", argv[0]);
    exit(0);
  }

  graphics->setColorMapDirectory(s.colorMapDir);
  graphics->setAlphaColor(s.alpha_red, s.alpha_green, s.alpha_blue);

  signal(SIGINT, handle_interrupt);


  // STEADY STATE


  fprintf(stderr, "%s:  entering steady state.\n", argv[0]);

  while (1) {

    if (microscope_connection)  // if not, should have exited
      microscope_connection->mainloop();

    if (graphics_connection)  // if not, should have exited
      graphics_connection->mainloop();

    if (microscope)
      microscope->mainloop();

    if (graphics)  // if not, should have exited
      graphics->mainloop();

  }

}

