#include "nmg_RenderServer.h"

#include <vrpn_Connection.h>

#include <BCPlane.h>
#include <BCGrid.h>
#include <nmb_Dataset.h>

#include "graphics_globals.h"


class nmg_RenderServer_ViewStrategy {

  public:

    nmg_RenderServer_ViewStrategy (nmg_Graphics_RenderServer *);
    virtual ~nmg_RenderServer_ViewStrategy (void) = 0;

    virtual vrpn_bool alwaysSendEntireScreen (void) const = 0;
    virtual void setViewingTransform (void) = 0;

    virtual void setGraphicsModes (void) = 0;

  protected:

    nmg_Graphics_RenderServer * d_server;

};

/** \class nmg_RSViewS_Ortho
 * Manages an orthographic projection of the surface from directly
 * overhead.
 */

class nmg_RSViewS_Ortho : public nmg_RenderServer_ViewStrategy {

  public:

    nmg_RSViewS_Ortho (nmg_Graphics_RenderServer *);
    virtual ~nmg_RSViewS_Ortho (void);

    virtual vrpn_bool alwaysSendEntireScreen (void) const;
    virtual void setViewingTransform (void);

    virtual void setGraphicsModes (void);
      /**< Turns off chartjunk, measure lines;  sets "planeonly".
       * Ortho wants to capture only the image of the plane.
       */

};

/** \class nmg_RSViewS_Slave
 * Manages a perspective projection of the surface slaved to the
 * user's viewpoint.
 */

class nmg_RSViewS_Slave : public nmg_RenderServer_ViewStrategy {

  public:

    nmg_RSViewS_Slave (nmg_Graphics_RenderServer *);
    virtual ~nmg_RSViewS_Slave (void);

    virtual vrpn_bool alwaysSendEntireScreen (void) const;
    virtual void setViewingTransform (void);

    virtual void setGraphicsModes (void);
      /**< Turns on chartjunk, measure lines;  not "planeonly".
       * Slave wants to capture the entire scene, plane and decorations.
       */
};





class nmg_RenderServer_Strategy {

  public:

    nmg_RenderServer_Strategy (nmg_Graphics_RenderServer *);
    virtual ~nmg_RenderServer_Strategy (void) = 0;

    virtual void captureData (void) = 0;
    virtual void sendData (int minx, int maxx, int miny, int maxy) = 0;

  protected:

    nmg_Graphics_RenderServer * d_server;

};

/** \class nmg_RSStrategy_Texture
 * Sends back a
 * high-resolution capture of the surface colors meant to be used as a
 * texture map.  If d_alwaysSendEntireScreen is set this works as a
 * "video" mode.
 */

class nmg_RSStrategy_Texture : public nmg_RenderServer_Strategy {

  public:

    nmg_RSStrategy_Texture (nmg_Graphics_RenderServer *);
    virtual ~nmg_RSStrategy_Texture (void);

    virtual void captureData (void);
    virtual void sendData (int minx, int maxx, int miny, int maxy);
};

/** \class nmg_RSStrategy_Vertex
 * Sends back a
 * low-resolution capture of surface colors and depth meant to be used as a
 * vertex mesh.  If d_alwaysSendEntireScreen is set this works as a
 * "IBR" mode.
 */

class nmg_RSStrategy_Vertex : public nmg_RenderServer_Strategy {

  public:

    nmg_RSStrategy_Vertex (nmg_Graphics_RenderServer *);
    virtual ~nmg_RSStrategy_Vertex (void);

    virtual void captureData (void);
    virtual void sendData (int minx, int maxx, int miny, int maxy);

};




nmg_RenderServer_ViewStrategy::nmg_RenderServer_ViewStrategy
                      (nmg_Graphics_RenderServer * g) :
    d_server (g) {

}

nmg_RenderServer_ViewStrategy::~nmg_RenderServer_ViewStrategy (void) {

}

nmg_RSViewS_Ortho::nmg_RSViewS_Ortho (nmg_Graphics_RenderServer * g) :
  nmg_RenderServer_ViewStrategy (g) {

}

nmg_RSViewS_Ortho::~nmg_RSViewS_Ortho (void) {

}

// virtual
vrpn_bool nmg_RSViewS_Ortho::alwaysSendEntireScreen (void) const {
  return VRPN_FALSE;
}

// virtual
void nmg_RSViewS_Ortho::setViewingTransform (void) {

  BCPlane * plane;
  double scaleTo;
  double xlateTo;
  double lenY;
  double gridMidpointX, gridMidpointY;

  // Attempt to guarantee a full-screen view from directly overhead.
  // Requires a screen with square pixels to work properly?

  // Taken from find_center_xforms() in microscape.c
  plane = g_inputGrid->getPlaneByName (g_heightPlaneName);
  lenY = fabs(plane->maxY() - plane->minY());
  scaleTo = lenY / ScreenWidthMetersY;

//fprintf(stderr, "lenY is %.5lf;  computed scaleTo is %.5lf\n",
//lenY, scaleTo);

  xlateTo = 10000.0;
  if (plane->maxValue() * 1.5 > xlateTo) {
    xlateTo = 1.5 * plane->maxValue();
  }

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
  //scaleTo *= d_server->screenSizeY() * 0.0085;

  gridMidpointX = (g_inputGrid->minX() +
                   g_inputGrid->maxX()) * 0.5;
  gridMidpointY = (g_inputGrid->minY() +
                   g_inputGrid->maxY()) * 0.5;
  v_world.users.xforms[0].xlate[0] = gridMidpointX;
  v_world.users.xforms[0].xlate[1] = gridMidpointY;
  v_world.users.xforms[0].xlate[2] = scaleTo ;
  v_world.users.xforms[0].rotate[0] = 0.0;
  v_world.users.xforms[0].rotate[1] = 0.0;
  v_world.users.xforms[0].rotate[2] = 0.0;
  v_world.users.xforms[0].rotate[3] = 1.0;
  v_world.users.xforms[0].scale = scaleTo;

fprintf(stderr, "Translation is %.5f, %.5f, %.5f;  scale is %.5f.\n",
gridMidpointX, -scaleTo + gridMidpointY, scaleTo , v_world.users.xforms[0].scale);

}

// virtual
void nmg_RSViewS_Ortho::setGraphicsModes (void) {

  g_config_chartjunk = 0;
  g_config_measurelines = 0;
  g_config_planeonly = 1;  // should make chartjunk and measurelines redundant

}




nmg_RSViewS_Slave::nmg_RSViewS_Slave (nmg_Graphics_RenderServer * g) :
  nmg_RenderServer_ViewStrategy (g) {

}

nmg_RSViewS_Slave::~nmg_RSViewS_Slave (void) {

}

// virtual
vrpn_bool nmg_RSViewS_Slave::alwaysSendEntireScreen (void) const {
  return VRPN_TRUE;
}

// virtual
void nmg_RSViewS_Slave::setViewingTransform (void) {

  // DO NOTHING
  // Default behavior of graphics remote should make this Server
  // properly slaved to the Remote.

}

// virtual
void nmg_RSViewS_Slave::setGraphicsModes (void) {

  g_config_chartjunk = 1;
  g_config_measurelines = 1;
  g_config_planeonly = 0;

}






nmg_RenderServer_Strategy::nmg_RenderServer_Strategy
                     (nmg_Graphics_RenderServer * g) :
    d_server (g) {

}

nmg_RenderServer_Strategy::~nmg_RenderServer_Strategy (void) {

}

nmg_RSStrategy_Texture::nmg_RSStrategy_Texture
                     (nmg_Graphics_RenderServer * g) :
    nmg_RenderServer_Strategy (g) {

}

nmg_RSStrategy_Texture::~nmg_RSStrategy_Texture (void) {

}

// virtual
void nmg_RSStrategy_Texture::captureData (void) {
  d_server->screenCapture();
}

// virtual
void nmg_RSStrategy_Texture::sendData (int minx, int maxx,
                                            int miny, int maxy) {
  d_server->sendPartialPixelData(minx, maxx, miny, maxy);
}

nmg_RSStrategy_Vertex::nmg_RSStrategy_Vertex
                     (nmg_Graphics_RenderServer * g) :
    nmg_RenderServer_Strategy (g) {

}

nmg_RSStrategy_Vertex::~nmg_RSStrategy_Vertex (void) {

}


// virtual
void nmg_RSStrategy_Vertex::captureData (void) {
  d_server->screenCapture();
  d_server->depthCapture();
}

// virtual
void nmg_RSStrategy_Vertex::sendData (int minx, int maxx,
                                            int miny, int maxy) {
  d_server->sendPartialPixelData(minx, maxx, miny, maxy);
  d_server->sendPartialDepthData(minx, maxx, miny, maxy);
}







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
  if (d_viewStrategy) {
    d_viewStrategy->setViewingTransform();
  }

  // Render
  nmg_Graphics_Implementation::mainloop();


  // Compute which portions of the screen changed
  computeScreenChange(&minx, &maxx, &miny, &maxy);

  // Capture the screen
  if (d_strategy) {
    d_strategy->captureData();
  }

  // Output
fprintf(stderr, "Sending data for X [%d - %d] by Y [%d - %d].\n",
minx, maxx, miny, maxy);
  if (d_strategy) {
    d_strategy->sendData(minx, maxx, miny, maxy);
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

/* void nmg_Graphics_RenderServer::sendDepthData (void) {
}
*/

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

