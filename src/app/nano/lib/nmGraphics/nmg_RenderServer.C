#include "nmg_RenderServer.h"

#include <vrpn_Connection.h>

#include <BCPlane.h>
#include <BCGrid.h>
#include <nmb_Dataset.h>

#include "graphics_globals.h"


nmg_Graphics_RenderServer::nmg_Graphics_RenderServer
                                (nmb_Dataset * data,
                                 const int minColor [3],
                                 const int maxColor [3],
                                 vrpn_Connection * outputConnection,
                                 RemoteColorMode cMode,
                                 RemoteDepthMode dMode,
                                 int xsize, int ysize,
                                 const char * rulergridName,
                                 vrpn_Connection * controlConnection) :
    nmg_Graphics_Implementation (data, minColor, maxColor, rulergridName,
                                 controlConnection),
    d_colorMode (cMode),
    d_depthMode (dMode),
    d_pixelBuffer (NULL),
    d_depthBuffer (NULL),
    d_pixelOutputBuffer (NULL),
    d_pixelOutputBufferSize (-1),
    d_depthOutputBuffer (NULL),
    d_depthOutputBufferSize (-1),
    d_screenSizeX (xsize),
    d_screenSizeY (ysize),
    d_outputConnection (outputConnection),
    d_sendEntireScreen (VRPN_FALSE),
    d_alwaysSendEntireScreen (VRPN_FALSE) {

  vrpn_int32 control_id;
  vrpn_int32 newConn_type;

fprintf(stderr,
        "In nmg_Graphics_RenderServer::nmg_Graphics_RenderServer()\n");

  // Pixel and depth buffers will be allocated the first time screenCapture()/
  // depthCapture() are called, and reused thereafter.

  d_myId = outputConnection->register_sender("nmg Graphics Renderer");

  d_pixelData_type = outputConnection->register_message_type("nmg pixel data");
  d_depthData_type = outputConnection->register_message_type("nmg depth data");

  d_timerSN_type_in = controlConnection->register_message_type
              ("nmg Graphics RemoteRender Timer SN");  // 34 char
  d_timerSN_type_out = outputConnection->register_message_type
              ("nmg Graphics RemoteRender Timer SN");  // 34 char

  d_pixelOutputBufferSize = 8 * sizeof(vrpn_int32)
                          + xsize * 3 * sizeof(char);
  d_depthOutputBufferSize = 8 * sizeof(vrpn_int32)
                          + xsize * sizeof(vrpn_float32);
  d_pixelOutputBuffer = new char [d_pixelOutputBufferSize];
  d_depthOutputBuffer = new char [d_depthOutputBufferSize];

  // set defaults
  g_config_chartjunk = 0;
  g_config_measurelines = 0;
  g_config_planeonly = 1;  // should make chartjunk and measurelines redundant

  control_id = outputConnection->register_sender (vrpn_CONTROL);
  newConn_type = outputConnection->register_message_type
                        (vrpn_got_connection);
  outputConnection->register_handler(newConn_type, handle_newConnection,
                                     this, control_id);

  //controlConnection->register_handler(d_timerSN_type_in, handle_timerSN,
                                      //this, nmg_Graphics::d_myId);
  controlConnection->register_handler(d_timerSN_type_in, handle_timerSN,
                                      this, vrpn_ANY_SENDER);
}

nmg_Graphics_RenderServer::~nmg_Graphics_RenderServer (void) {

}



void nmg_Graphics_RenderServer::mainloop (void) {

  char msgbuf [24];
  BCPlane * plane;
  char * mptr;
  timeval now;
  double scaleTo;
  double lenY;
  double gridToScreenX, gridToScreenY;
  double gridMidpointX, gridMidpointY;
  vrpn_int32 sN;
  int msglen;
  int w, h;
  int minx, miny, maxx, maxy;

  d_connection->mainloop();

  // HACK
  // Make sure it looks right
  // Can't just enableChartjunk(0) because it does too much else.
  g_config_chartjunk = 0;



  // Set up viewing transform to guarantee full-screen.

  // Taken from find_center_xforms() in microscape.c
  plane = g_inputGrid->getPlaneByName (g_heightPlaneName);
  lenY = fabs(plane->maxY() - plane->minY());
  scaleTo = lenY / ScreenWidthMetersY;

//fprintf(stderr, "lenY is %.5lf;  computed scaleTo is %.5lf\n",
//lenY, scaleTo);

  // AD HOC - correction factor probably due to error in ScreenWidthMetersY,
  // since that's an assumption that varies from display to display.
  // 100x100 render buffer on an O2 thinks it's 5cm on a side but looking at
  // the screen is more like 28 mm x 29 mm
  // BUG - why does the screen seem to scale inconsistiently?
  // We get an edge of background showing through along the vertical
  // sides while clipping the ends of the horizontal sides...
  // This might be explained by the 1mm mismatch in window sizes when
  // we tell it to open a "square" window - pixels aren't square!  Do
  // we need to distort to account for this?  Use non-square rendering
  // grids at the client that depend on physical characteristics of the
  // server?  AARGH!
  scaleTo *= d_screenSizeY * 0.0085;

  gridMidpointX = (g_inputGrid->minX() +
                   g_inputGrid->maxX()) * 0.5;
  gridMidpointY = (g_inputGrid->minY() +
                   g_inputGrid->maxY()) * 0.5;
  v_world.users.xforms[0].xlate[0] = gridMidpointX;
  v_world.users.xforms[0].xlate[1] = gridMidpointY;
  v_world.users.xforms[0].rotate[0] = 0.0;
  v_world.users.xforms[0].rotate[1] = 0.0;
  v_world.users.xforms[0].rotate[2] = 0.0;
  v_world.users.xforms[0].rotate[3] = 1.0;
  v_world.users.xforms[0].scale = scaleTo;


  // Render

  nmg_Graphics_Implementation::mainloop();


  // Compute which portions of the screen changed

  if (d_sendEntireScreen || d_alwaysSendEntireScreen) {
    d_sendEntireScreen = VRPN_FALSE;
    minx = 0;
    maxx = d_screenSizeX - 1;
    miny = 0;
    maxy = d_screenSizeY - 1;
  } else {
    getLatestChange(&minx, &maxx, &miny, &maxy);

    // getLatestChange() reports the portion of the *grid* redrawn in the last
    // frame;  we need to convert that into screen coordinates.
    // BUG:  sometimes after the entire screen is redrawn getLatestChange()
    // doesn't report it.

    gridToScreenX = ((double) d_screenSizeX) / g_inputGrid->numX();
    gridToScreenY = ((double) d_screenSizeY) / g_inputGrid->numY();
//fprintf(stderr, "Input conversion factors are %.5f and %.5f.\n",
//gridToScreenX, gridToScreenY);
//fprintf(stderr, "Original range of change is X [%d - %d] by Y [%d - %d].\n",
//minx, maxx, miny, maxy);
    minx = MAX(0, minx * gridToScreenX);
    maxx = MIN(d_screenSizeX - 1, maxx * gridToScreenX + 1);
    miny = MAX(0, miny * gridToScreenY);
    maxy = MIN(d_screenSizeY - 1, maxy * gridToScreenY + 1);
  }

  // Capture the screen

  // ASSUMPTION:  it doesn't cost us much more to capture the entire
  // screen than it does to capture a small portion.  If that's untrue,
  // rewrite screenCapture() and depthCapture() to get the computed
  // part of the screen that changed.

  if ((d_colorMode == VERTEX_COLORS) ||
      (d_colorMode == SUPERSAMPLED_COLORS)) {
    screenCapture(&w, &h, &d_pixelBuffer, VRPN_FALSE);
  }

  if (d_depthMode == VERTEX_DEPTH) {
    depthCapture(&w, &h, &d_depthBuffer, VRPN_FALSE);
    // w and h should be equal after each call and equal to d_screenSize[XY]
    // Send VRPN_FALSE to capture out of the front buffer rather than the
    // back - we want to be sure we get the current frame rather than the
    // previous.
  }


  // Output

fprintf(stderr, "Sending data for X [%d - %d] by Y [%d - %d].\n",
minx, maxx, miny, maxy);

  if (d_colorMode != NO_COLORS) {
    sendPartialPixelData(minx, maxx, miny, maxy);
  }

  if (d_depthMode != NO_DEPTH) {
    sendPartialDepthData(minx, maxx, miny, maxy);
  }


  // Timer maintenance
  // Send SN of all timestamps that wre waiting for this update.

  gettimeofday(&now, NULL);
  while ((sN = d_timerList.getListHead()) != -1) {
    d_timerList.remove();
    mptr = msgbuf;
    msglen = 24;
    vrpn_buffer(&mptr, &msglen, sN);
    d_outputConnection->pack_message(24 - msglen, now, d_timerSN_type_out,
                                     nmg_Graphics::d_myId, msgbuf,
                                     vrpn_CONNECTION_RELIABLE);
fprintf(stderr, "Resent timer message for SN %d.\n", sN);
  }

  d_outputConnection->mainloop();
}


// virtual
void nmg_Graphics_RenderServer::resizeViewport (int width, int height) {

  // do nothing
  // need to override nmg_Graphics_Implementation behavior
  // which resets frame buffer sizes to fullscreen
}

void nmg_Graphics_RenderServer::sendPixelData (void) {

  sendPartialPixelData(0, d_screenSizeX - 1, 0, d_screenSizeY - 1);

}

void nmg_Graphics_RenderServer::sendDepthData (void) {

  sendPartialDepthData(0, d_screenSizeX - 1, 0, d_screenSizeY - 1);

}

void nmg_Graphics_RenderServer::sendPartialPixelData (int minx, int maxx,
                                                      int miny, int maxy) {

  timeval now;
  char * bp;
  vrpn_int32 buflen;
  vrpn_int32 y;
  vrpn_int32 x;

  for (y = miny; y <= maxy; y++) {

    bp = (char *) d_pixelOutputBuffer;
    buflen = d_pixelOutputBufferSize;
    vrpn_buffer(&bp, &buflen, (vrpn_int32) minx);  // x
    vrpn_buffer(&bp, &buflen, y);  // y
    vrpn_buffer(&bp, &buflen, (vrpn_int32) 1);  // dx
    vrpn_buffer(&bp, &buflen, (vrpn_int32) 0);  // dy
    vrpn_buffer(&bp, &buflen, (vrpn_int32) maxx - minx + 1);  // lineCount
    vrpn_buffer(&bp, &buflen, (vrpn_int32) 3);  // fieldCount
    vrpn_buffer(&bp, &buflen, (vrpn_int32) 0);  // sec
    vrpn_buffer(&bp, &buflen, (vrpn_int32) 0);  // usec

    // Buffer RGB for (maxx - minx +1) pixels
    vrpn_buffer(&bp, &buflen,
                (char *) &d_pixelBuffer[(y * d_screenSizeX + minx) * 3],
                (maxx - minx + 1) * 3);

    gettimeofday(&now, NULL);
    d_outputConnection->pack_message(d_pixelOutputBufferSize - buflen, now,
                                     d_pixelData_type, d_myId,
                                     d_pixelOutputBuffer,
                                     vrpn_CONNECTION_RELIABLE);

//if (!(y % 10)) {
//fprintf(stderr, "Sending pixel data from %d, %d:\n", minx, y);
//for (x = minx; x < maxx; x += 10) {
//fprintf(stderr, " (%d %d %d)",
//d_pixelBuffer[d_screenSizeX * y * 3 + x * 3],
//d_pixelBuffer[d_screenSizeX * y * 3 + x * 3 + 1],
//d_pixelBuffer[d_screenSizeX * y * 3 + x * 3 + 2]);
//}
//fprintf(stderr, "\n");
//}
  }
}

void nmg_Graphics_RenderServer::sendPartialDepthData (int minx, int maxx,
                                                      int miny, int maxy) {

  timeval now;
  char * bp;
  vrpn_int32 buflen;
  vrpn_int32 y;
  vrpn_int32 x;

  for (y = miny; y <= maxy; y++) {

    bp = (char *) d_depthOutputBuffer;
    buflen = d_depthOutputBufferSize;
    vrpn_buffer(&bp, &buflen, (vrpn_int32) minx);  // x
    vrpn_buffer(&bp, &buflen, y);  // y
    vrpn_buffer(&bp, &buflen, (vrpn_int32) 1);  // dx
    vrpn_buffer(&bp, &buflen, (vrpn_int32) 0);  // dy
    vrpn_buffer(&bp, &buflen, (vrpn_int32) maxx - minx + 1);  // lineCount
    vrpn_buffer(&bp, &buflen, (vrpn_int32) 3);  // fieldCount
    vrpn_buffer(&bp, &buflen, (vrpn_int32) 0);  // sec
    vrpn_buffer(&bp, &buflen, (vrpn_int32) 0);  // usec

    for (x = minx; x <= maxx; x++) {
      vrpn_buffer(&bp, &buflen, d_depthBuffer[y * d_screenSizeX + x]);  // Z
    }

    gettimeofday(&now, NULL);
    d_outputConnection->pack_message(d_depthOutputBufferSize - buflen, now,
                                     d_depthData_type, d_myId,
                                     d_depthOutputBuffer,
                                     vrpn_CONNECTION_RELIABLE);
  }
}


// static
int nmg_Graphics_RenderServer::handle_newConnection (void * userdata,
                                                     vrpn_HANDLERPARAM) {
  nmg_Graphics_RenderServer * g = (nmg_Graphics_RenderServer *) userdata;

  g->d_sendEntireScreen = VRPN_TRUE;

  return 0;
}

// static
int nmg_Graphics_RenderServer::handle_timerSN (void * userdata,
                                               vrpn_HANDLERPARAM p) {

  // Unpack the SN we received on the control connection and buffer it up
  // to be sent on the output connection.

  nmg_Graphics_RenderServer * gs = (nmg_Graphics_RenderServer *) userdata;
  const char * bp;
  vrpn_int32 sn;

  bp = p.buffer;
  vrpn_unbuffer(&bp, &sn);

  gs->d_timerList.insert(sn);

fprintf(stderr, "Got timer SN message for %d.\n", sn);

  return 0;
}

