#include "nmg_RenderClientImpl.h"

#include <nmb_Dataset.h>
#include <nmb_Decoration.h>
#include <BCGrid.h>
#include <BCPlane.h>
#include <nmb_TimerList.h>

#include "graphics_globals.h"  // for g_PRERENDERED_COLORS
#include "spm_gl.h"  // for init_vertexArray()

#include "graphics.h"  // for buildRemoteRenderedTexture


nmg_Graphics_RenderClient_Implementation::
nmg_Graphics_RenderClient_Implementation
                      (nmb_Dataset * data,
                       const int minColor [3],
                       const int maxColor [3],
                       vrpn_Connection * inputConnection,
                       RemoteColorMode cMode,
                       RemoteDepthMode dMode,
                       nmb_TimerList * timer,
                       int xsize, int ysize) :
    nmg_Graphics_Implementation (data, minColor, maxColor, NULL, NULL),
    d_colorMode (cMode),
    d_depthMode (dMode),
    rangeOfChange (d_renderingGrid),
    d_pixelBuffer (new unsigned char [xsize * ysize * 3]),
    d_depthBuffer (new float [xsize * ysize]),
    d_screenSizeX (xsize),
    d_screenSizeY (ysize),
    //d_renderingGrid (xsize, ysize, 0, xsize - 1, 0, ysize - 1),
    d_renderingGrid (new BCGrid (xsize, ysize, data->inputGrid->minX(),
                                   data->inputGrid->maxX(),
                                   data->inputGrid->minY(),
                                   data->inputGrid->maxY())),
    d_inputConnection (inputConnection),
    d_remoteTextureChanged (VRPN_FALSE),
    d_timer (timer) {


  d_myId = inputConnection->register_sender("nmg Graphics Renderer");
  d_pixelData_type = inputConnection->register_message_type("nmg pixel data");
  d_depthData_type = inputConnection->register_message_type("nmg depth data");
  d_timerSN_type = inputConnection->register_message_type
        ("nmg Graphics RemoteRender Timer SN");  // 34 char

  inputConnection->register_handler(d_pixelData_type, handle_pixelData,
                                    this, d_myId);
  inputConnection->register_handler(d_depthData_type, handle_depthData,
                                    this, d_myId);

  //inputConnection->register_handler(d_timerSN_type, handle_timerSN,
   //                                 this, nmg_Graphics::d_myId);
  inputConnection->register_handler(d_timerSN_type, handle_timerSN,
                                    this, vrpn_ANY_SENDER);

  d_renderingGrid->addNewPlane("captured depth", "", 0);
  d_renderingGrid->addNewPlane("prerendered red", "", 0);
  d_renderingGrid->addNewPlane("prerendered green", "", 0);
  d_renderingGrid->addNewPlane("prerendered blue", "", 0);

  if (cMode == VERTEX_COLORS) {
    g_PRERENDERED_COLORS = 1;
  }
  if (cMode == SUPERSAMPLED_COLORS) {
    g_PRERENDERED_TEXTURE = 1;
  }
  if (cMode == VERTEX_DEPTH) {
    g_PRERENDERED_DEPTH = 1;
  }
  g_prerendered_grid = d_renderingGrid;
  g_prerenderedChange = &rangeOfChange;

  // vertex array is initialized by nmg_Graphics_Implementation, but
  // it gets the wrong size

  if (g_VERTEX_ARRAY) {
    if (!init_vertexArray(xsize, ysize)) {
          fprintf(stderr," init_vertexArray: out of memory.\n");
          exit(0);
     }
  }

  d_dataset->inputGrid->registerMinMaxCallback(getNewBounds, this);

  initializeTextures();

  setTextureMode(REMOTE_DATA, REGISTRATION_COORD);

}



// virtual
nmg_Graphics_RenderClient_Implementation::
~nmg_Graphics_RenderClient_Implementation (void) {

  if (remote_data) {
    delete [] remote_data;
  }

}

// virtual
void nmg_Graphics_RenderClient_Implementation::mainloop (void) {

  if (d_colorMode == SUPERSAMPLED_COLORS) {
    if (d_remoteTextureChanged) {
      buildRemoteRenderedTexture(d_screenSizeX, d_screenSizeY, remote_data);
      d_remoteTextureChanged = VRPN_FALSE;
    }
    // hack - overloading genetic texture coordinate handling
    float dx = d_dataset->inputGrid->maxX() - d_dataset->inputGrid->minX();
    float dy = d_dataset->inputGrid->maxY() - d_dataset->inputGrid->minY();
    g_texture_transform[0] = 1.0 / dx;
    g_texture_transform[1] = 0.0;
    g_texture_transform[2] = 0.0;
    g_texture_transform[3] = 0.0;
    g_texture_transform[4] = 0.0;
    g_texture_transform[5] = 1.0 / dy;
    g_texture_transform[6] = 0.0;
    g_texture_transform[7] = 0.0;
    g_texture_transform[8] = 0.0;
    g_texture_transform[9] = 0.0;
    g_texture_transform[10] = 1.0;
    g_texture_transform[11] = 0.0;
    g_texture_transform[12] = -d_dataset->inputGrid->minX() / dx;
    g_texture_transform[13] = -d_dataset->inputGrid->minY() / dy;
    g_texture_transform[14] = 0.0;
    g_texture_transform[15] = 1.0;
    // can't call setTextureMode() every frame because that calls
    // causeGridRedraw(), which regenerates the grid.
    //setTextureMode(REMOTE_DATA, REGISTRATION_COORD);
//fprintf(stderr, "In RCI mainloop.\n");
    g_texture_mode = GL_TEXTURE_2D;
    g_texture_displayed = REMOTE_DATA;
    g_texture_transform_mode = REGISTRATION_COORD;
  }

  nmg_Graphics_Implementation::mainloop();

  d_inputConnection->mainloop();

}


// PROTECTED


// initializeTextures() is called in the nmg_Graphics_Implementation
// constructor, before our constructor has executed, so the original
// version is called instead of this one since the vtable hasn't been
// set up.  We call this one explicitly in our constructor.

// virtual
void nmg_Graphics_RenderClient_Implementation::initializeTextures (void) {
  long i;

  //nmg_Graphics_Implementation::initializeTextures();

#if defined(sgi) || defined(FLOW) || defined (__CYGWIN__)

  if (d_colorMode == SUPERSAMPLED_COLORS) {
    remote_data = new GLubyte [d_screenSizeX * d_screenSizeY * 4];
    if (!remote_data) {
      fprintf(stderr, "nmg_Graphics_RenderClient_Implementation::"
                      "initializeTextures:  Out of memory.\n");
      return;
    }

    for (i = 0; i < d_screenSizeX * d_screenSizeY * 4; i++) {
      remote_data[i] = i * 255 / d_screenSizeX / d_screenSizeY / 4;
    }

    buildRemoteRenderedTexture(d_screenSizeX, d_screenSizeY,
                               remote_data);
  }

#endif
}

// static
int nmg_Graphics_RenderClient_Implementation::handle_pixelData
                                                 (void * userdata,
                                                  vrpn_HANDLERPARAM p) {
  nmg_Graphics_RenderClient_Implementation * gp =
          (nmg_Graphics_RenderClient_Implementation *) userdata;
  BCPlane * red;
  BCPlane * green;
  BCPlane * blue;
  const char * bp = p.buffer;
  char lb [3072];  // HACK - can't handle lines over 1024 pixels long

  vrpn_int32 x, y, dx, dy;
  vrpn_int32 lineCount, fieldCount;
  vrpn_int32 sec, usec;

  vrpn_int32 i;
  vrpn_int32 k;
  vrpn_int32 l;
  int nx, ny;
  char r, g, b;

  vrpn_unbuffer(&bp, &x);
  vrpn_unbuffer(&bp, &y);
  vrpn_unbuffer(&bp, &dx);
  vrpn_unbuffer(&bp, &dy);
  vrpn_unbuffer(&bp, &lineCount);
  vrpn_unbuffer(&bp, &fieldCount);
  vrpn_unbuffer(&bp, &sec);
  vrpn_unbuffer(&bp, &usec);

  red = gp->d_renderingGrid->getPlaneByName("prerendered red");
  green = gp->d_renderingGrid->getPlaneByName("prerendered green");
  blue = gp->d_renderingGrid->getPlaneByName("prerendered blue");

  vrpn_unbuffer(&bp, lb, (vrpn_uint32) lineCount * 3);

  if (gp->d_colorMode == VERTEX_COLORS) {
    nx = x;
    ny = y;
    for (l = 0, i = 0; i < lineCount; i++) {
      red->setValue(nx, ny, lb[l++] / 255.0f);
      green->setValue(nx, ny, lb[l++] / 255.0f);
      blue->setValue(nx, ny, lb[l++] / 255.0f);

      gp->rangeOfChange.AddPoint(nx, ny);

      nx += dx;
      ny += dy;
    }
  } else if (gp->d_colorMode == SUPERSAMPLED_COLORS) {
    k = (y * gp->d_screenSizeX + x) * 4;
    for (l = 0, i = 0; i < lineCount; i++) {
      remote_data[k++] = lb[l++];
      remote_data[k++] = lb[l++];
      remote_data[k++] = lb[l++];
      remote_data[k++] = 255;
    }

    gp->d_remoteTextureChanged = VRPN_TRUE;
  }

//fprintf(stderr, "Got pixel data at %d, %d.\n", x, y);
//if (!(y % 10)) {
//fprintf(stderr, "handle_pixelData() from %d, %d:\n", x, y);
//fprintf(stderr, "into planes %ld, %ld, %ld:\n", red, green, blue);
//for (i = 0; i < lineCount; i += 10) {
//fprintf(stderr, " (%.2f %.2f %.2f)",
//red->value(x + dx * i, y + dy * i),
//green->value(x + dx * i, y + dy * i),
//blue->value(x + dx * i, y + dy * i));
//}
//fprintf(stderr, "\n");
//}

  return 0;
}

// static
int nmg_Graphics_RenderClient_Implementation::handle_depthData
                                                 (void * userdata,
                                                  vrpn_HANDLERPARAM p) {
  nmg_Graphics_RenderClient_Implementation * gp =
           (nmg_Graphics_RenderClient_Implementation *) userdata;
  BCPlane * height;
  const char * bp = p.buffer;

  vrpn_int32 x, y, dx, dy;
  vrpn_int32 lineCount, fieldCount;
  vrpn_int32 sec, usec;

  vrpn_int32 i;
  //vrpn_int32 l;

  vrpn_float64 z;
  int nx, ny;

  vrpn_unbuffer(&bp, &x);
  vrpn_unbuffer(&bp, &y);
  vrpn_unbuffer(&bp, &dx);
  vrpn_unbuffer(&bp, &dy);
  vrpn_unbuffer(&bp, &lineCount);
  vrpn_unbuffer(&bp, &fieldCount);
  vrpn_unbuffer(&bp, &sec);
  vrpn_unbuffer(&bp, &usec);

  height = gp->d_renderingGrid->getPlaneByName("captured depth");

  nx = x;
  ny = y;
  for (i = 0; i < lineCount; i++) {
    vrpn_unbuffer(&bp, &z);  // Z
    height->setValue(nx, ny, z);

    gp->rangeOfChange.AddPoint(nx, ny);

    nx += dx;
    ny += dy;
  }

  return 0;
}


// static
void nmg_Graphics_RenderClient_Implementation::getNewBounds
                                             (void * userdata,
                                              double minX, double maxX,
                                              double minY, double maxY) {
  nmg_Graphics_RenderClient_Implementation * gp =
           (nmg_Graphics_RenderClient_Implementation *) userdata;

  gp->d_renderingGrid->setMinX(minX);
  gp->d_renderingGrid->setMaxX(maxX);
  gp->d_renderingGrid->setMinY(minY);
  gp->d_renderingGrid->setMaxY(maxY);
}

// static
int nmg_Graphics_RenderClient_Implementation::handle_timerSN
                                             (void * userdata,
                                              vrpn_HANDLERPARAM p) {
  nmg_Graphics_RenderClient_Implementation * gp =
    (nmg_Graphics_RenderClient_Implementation *) userdata;
  vrpn_int32 sn;
  const char * bp = p.buffer;

  vrpn_unbuffer(&bp, &sn);
  gp->d_timer->unblock(sn);

fprintf(stderr, "Unblocked timestamp #%d.\n", sn);

  return 0;
}


