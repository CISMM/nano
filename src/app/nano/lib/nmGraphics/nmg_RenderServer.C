#include "nmg_RenderServer.h"

#include <vrpn_Connection.h>

#include <BCPlane.h>
#include <BCGrid.h>
#include <nmb_Dataset.h>

#include "nmg_RenderServerStrategies.h"
#include "graphics_globals.h"






nmg_Graphics_RenderServer::nmg_Graphics_RenderServer
                                (nmb_Dataset * data,
                                 const int minColor [3],
                                 const int maxColor [3],
                                 vrpn_Connection * outputConnection,
                                 RemoteColorMode cMode,
                                 RemoteDepthMode dMode,
                                 RemoteProjectionMode pMode,
                                 int xsize, int ysize,
                                 const char * rulergridName,
                                 vrpn_Connection * controlConnection) :
    nmg_Graphics_Implementation (data, minColor, maxColor, rulergridName,
                                 controlConnection),
    //d_colorMode (cMode),
    //d_depthMode (dMode),
    d_pixelBuffer (NULL),
    d_depthBuffer (NULL),
    d_pixelOutputBuffer (NULL),
    d_depthOutputBuffer (NULL),
    d_pixelOutputBufferSize (-1),
    d_depthOutputBufferSize (-1),
    d_screenSizeX (xsize),
    d_screenSizeY (ysize),
    d_outputConnection (outputConnection),
    d_sendEntireScreen (VRPN_FALSE),
    d_strategy (NULL) {

  vrpn_int32 control_id;
  vrpn_int32 newConn_type;

fprintf(stderr,
        "In nmg_Graphics_RenderServer::nmg_Graphics_RenderServer()\n");
fprintf(stderr, "  screen is %d x %d.\n", xsize, ysize);

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

  control_id = outputConnection->register_sender (vrpn_CONTROL);
  newConn_type = outputConnection->register_message_type
                        (vrpn_got_connection);
  outputConnection->register_handler(newConn_type, handle_newConnection,
                                     this, control_id);

  //controlConnection->register_handler(d_timerSN_type_in, handle_timerSN,
                                      //this, nmg_Graphics::d_myId);
  controlConnection->register_handler(d_timerSN_type_in, handle_timerSN,
                                      this, vrpn_ANY_SENDER);



  if ((cMode == VERTEX_COLORS) && (dMode == VERTEX_DEPTH)) {
    d_strategy = new nmg_RSStrategy_Vertex (this);
  }

  if ((cMode == SUPERSAMPLED_COLORS) && (dMode == NO_DEPTH)) {
    d_strategy = new nmg_RSStrategy_Texture (this);
  }

  if ((cMode == CLOUDMODEL_COLORS) && (dMode == NO_DEPTH)) {
      d_strategy = new nmg_RSStrategy_CloudTexture(this);
  }
  

  if (!d_strategy) {
    fprintf(stderr, "nmg_Graphics_RenderServer:  "
                    "Sampling strategy not supported.\n");
  }

  switch (pMode) {

    case ORTHO_PROJECTION:
      d_viewStrategy = new nmg_RSViewS_Ortho (this);
      break;

    case PERSPECTIVE_PROJECTION:
      d_viewStrategy = new nmg_RSViewS_Slave (this);
      break;

    default:
      fprintf(stderr, "nmg_Graphics_RenderServer:  "
                      "Viewpoint control strategy not supported.\n");
      break;
  }

  if (d_viewStrategy) {
    d_viewStrategy->setGraphicsModes();
  }
}

nmg_Graphics_RenderServer::~nmg_Graphics_RenderServer (void) {

}



void nmg_Graphics_RenderServer::mainloop (void) {

  int minx, miny, maxx, maxy;

  // Get input.
  if (d_connection) {
    d_connection->mainloop();
  }

  // HACK
  // Make sure it looks right
  // Can't just enableChartjunk(0) because it does too much else.
  g_config_chartjunk = 0;

  // Set up viewing transform.
  // For some screwy race condition reason this doesn't always
  // override nmg_Graphics_Implementation::setViewTransform
  // so we have to intercept it on nmg_Graphics_RenderServer
  // and pass it down to strategies.

  if (d_viewStrategy) {
    d_viewStrategy->setViewingTransform();
  }

  // Render
  if (d_strategy) {
    d_strategy->render();
  }


  // Compute which portions of the screen changed
  computeScreenChange(&minx, &maxx, &miny, &maxy);

  // Capture the screen
  if (d_strategy) {
    d_strategy->captureData();
  }

  // Output
if (minx <= maxx) {
//fprintf(stderr, "Sending data for X [%d - %d] by Y [%d - %d].\n",
//minx, maxx, miny, maxy);
  if (d_strategy) {
    d_strategy->sendData(minx, maxx, miny, maxy);
  }
}

  clearWaitingTimestamps();

  if (d_outputConnection) {
    d_outputConnection->mainloop();
  }
}


vrpn_int32 nmg_Graphics_RenderServer::screenSizeX (void) const {
  return d_screenSizeX;
}

vrpn_int32 nmg_Graphics_RenderServer::screenSizeY (void) const {
  return d_screenSizeY;
}

// virtual
void nmg_Graphics_RenderServer::resizeViewport (int /*width*/, int /*height*/) {

  // do nothing
  // need to override nmg_Graphics_Implementation behavior
  // which resets frame buffer sizes to fullscreen
}


// ASSUMPTION:  it doesn't cost us much more to capture the entire
// screen than it does to capture a small portion.  If that's untrue,
// rewrite screenCapture() and depthCapture() to get the computed
// part of the screen that changed.

// Send VRPN_FALSE to capture out of the front buffer rather than the
// back - we want to be sure we get the current frame rather than the
// previous.

void nmg_Graphics_RenderServer::screenCapture (void) {
  int w, h;
  nmg_Graphics_Implementation::screenCapture(&w, &h, &d_pixelBuffer, VRPN_FALSE);
}


void nmg_Graphics_RenderServer::depthCapture (void) {
  int w, h;
  nmg_Graphics_Implementation::depthCapture(&w, &h, &d_depthBuffer, VRPN_FALSE);
}

void nmg_Graphics_RenderServer::sendPartialPixelData (int minx, int maxx,
                                                      int miny, int maxy) {

  timeval now;
  char * bp;
  vrpn_int32 buflen;
  vrpn_int32 y;

  if (!d_pixelBuffer || !d_outputConnection) {
    return;
  }

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

    vrpn_buffer(&bp, &buflen,
                (char *) &d_pixelBuffer[(y * d_screenSizeX + minx) * 3],
                (maxx - minx + 1) * 3);

    gettimeofday(&now, NULL);
    d_outputConnection->pack_message(d_pixelOutputBufferSize - buflen, now,
                                     d_pixelData_type, d_myId,
                                     d_pixelOutputBuffer,
                                     vrpn_CONNECTION_RELIABLE);

//if (!(y % 50)) {
//vrpn_int32 x;
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

  if (!d_depthBuffer || !d_outputConnection) {
    return;
  }

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
//if (!(y % 50)) {
//vrpn_int32 x;
//fprintf(stderr, "Sending depth data from %d, %d:\n", minx, y);
//for (x = minx; x < maxx; x += 10) {
//fprintf(stderr, " %.5f", d_depthBuffer[d_screenSizeX * y + x]);
//}
//fprintf(stderr, "\n");
//}
  }
}

void nmg_Graphics_RenderServer::defaultRender (void) {

  nmg_Graphics_Implementation::mainloop();

}

// virtual 
void nmg_Graphics_RenderServer::setViewTransform (v_xform_type t) {
  if (d_viewStrategy) {
    d_viewStrategy->setViewTransform(t);
  }
}






void nmg_Graphics_RenderServer::computeScreenChange
      (int * minx, int * maxx, int * miny, int * maxy) {

  double gridToScreenY;
  double gridToScreenX;

  if (!minx || !maxx || !miny || !maxy) {
    fprintf(stderr, "nmg_Graphics_RenderServer::computeScreenChange:  "
                    "NULL pointer.\n");
  }

  if (d_sendEntireScreen || d_viewStrategy->alwaysSendEntireScreen()) {

//fprintf(stderr, "  -- sending entire screen:  0, %d x 0, %d.\n",
//d_screenSizeX - 1, d_screenSizeY - 1);

    d_sendEntireScreen = VRPN_FALSE;
    *minx = 0;
    *maxx = d_screenSizeX - 1;
    *miny = 0;
    *maxy = d_screenSizeY - 1;
    return;
  }

  getLatestGridChange(minx, maxx, miny, maxy);

  // getLatestGridChange() reports the portion of the *grid* redrawn
  // in the last
  // frame;  we need to convert that into screen coordinates.
  // BUG:  sometimes after the entire screen is redrawn
  // getLatestGridChange() doesn't report it.

  gridToScreenX = ((double) d_screenSizeX) / g_inputGrid->numX();
  gridToScreenY = ((double) d_screenSizeY) / g_inputGrid->numY();
//fprintf(stderr, "Input conversion factors are %.5f and %.5f.\n",
//gridToScreenX, gridToScreenY);
//fprintf(stderr, "Original range of change is X [%d - %d] by Y [%d - %d].\n",
//minx, maxx, miny, maxy);

//fprintf(stderr, " -- changed grid is %d, %d x %d, %d.\n",
//*minx, *maxx, *miny, *maxy);

  *minx = (int)(MAX(0, *minx * gridToScreenX));
  *maxx = (int)(MIN(d_screenSizeX - 1, (*maxx + 1) * gridToScreenX));
  *miny = (int)(MAX(0, *miny * gridToScreenY));
  *maxy = (int)(MIN(d_screenSizeY - 1, (*maxy + 1) * gridToScreenY));

//fprintf(stderr, " -- changed screen is %d, %d x %d, %d.\n",
//*minx, *maxx, *miny, *maxy);

}


void nmg_Graphics_RenderServer::clearWaitingTimestamps (void) {
  vrpn_int32 sN;
  timeval now;

  vrpn_int32 msglen;
  char * mptr;
  char msgbuf [24];

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

