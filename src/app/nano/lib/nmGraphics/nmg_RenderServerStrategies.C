#include "nmg_RenderServerStrategies.h"

#include <BCPlane.h>
#include <BCGrid.h>
#include <nmb_Dataset.h>

#include "nmg_RenderServer.h"

#include "graphics_globals.h"
#include <nmb_Globals.h>
#include "nmg_CloudTexturer.h"



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

// virtual
void nmg_RenderServer_Strategy::render (void) {
  d_server->defaultRender();
}


void nmg_RenderServer_Strategy::
updatePixelBuffer (int minx, int maxx, int miny, int maxy, 
                   vrpn_uint8 *new_buf) 
{
    if (d_server->d_pixelBuffer == (vrpn_uint8*)NULL) {
        d_server->d_pixelBuffer = new vrpn_uint8[d_server->d_screenSizeX * d_server->d_screenSizeY * 3];
    }
    
    //Copy the contents of new_buf into d_pixelBuffer, as we don't want
    //someone deleting d_pixelBuffer if we set it to new_buf
    int index = 3 * (miny * d_server->d_screenSizeX + minx);
    for(int i = miny; i < maxy; i++) {
        for(int j = minx; j < maxx; j++) {
            d_server->d_pixelBuffer[index] = new_buf[index++];
            d_server->d_pixelBuffer[index] = new_buf[index++];
            d_server->d_pixelBuffer[index] = new_buf[index++];
        }
    }
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

nmg_RSStrategy_CloudTexture::
nmg_RSStrategy_CloudTexture(nmg_Graphics_RenderServer *g) 
  :  nmg_RenderServer_Strategy (g)
{
    d_cloud_render = new nmg_CloudTexturer(d_server->screenSizeX(), d_server->screenSizeY());
}

nmg_RSStrategy_CloudTexture::
~nmg_RSStrategy_CloudTexture (void) {
}

// virtual
void nmg_RSStrategy_CloudTexture::
render (void)
{
    nmg_RenderServer_Strategy::render();
    q_vec_type lightdir;
    d_server->getLightDirection(&lightdir);
    //Don't know how to get ahold of an "intensity" value.  So use 1 for now
    d_cloud_render->set_light(lightdir, 1);

    if (d_frame != NULL) {
        delete [] d_frame;
    }
    
    d_frame = d_cloud_render->render_detail();
}

// virtual
void nmg_RSStrategy_CloudTexture::
sendData(int minx, int maxx, int miny, int maxy) 
{
    d_server->sendPartialPixelData(minx, maxx, miny, maxy);
}

// virtual
void nmg_RSStrategy_CloudTexture::
captureData(void) 
{
    nmg_RenderServer_Strategy::updatePixelBuffer(g_minChangedX, g_maxChangedX,
                                             g_minChangedY, g_maxChangedY, d_frame);    
}

