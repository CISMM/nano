#include "nmg_RenderClientImpl.h"

#include <nmb_Dataset.h>
#include <nmb_Decoration.h>
#include <BCGrid.h>
#include <BCPlane.h>
#include <nmb_TimerList.h>
#include <nmg_Surface.h>

#include <GL/glu.h>  // for gluErrorString
//#include <GL/glut.h>

#include "graphics_globals.h"  // for g_PRERENDERED_COLORS
#include "spm_gl.h"  // for init_vertexArray()

#include "graphics.h"  // for buildRemoteRenderedTexture

class nmg_RCIStrategy {

  public:

    nmg_RCIStrategy (nmg_Graphics_RenderClient_Implementation *,
                     vrpn_int32 xsize, vrpn_int32 ysize);
    virtual ~nmg_RCIStrategy (void) = 0;

    virtual void mainloop (void);
      /**< Default implementation calls d_inputConnection->mainloop()
       * and nmg_Graphics_Implementation::mainloop
       */

    virtual void initializeTextures (void);
      /**< Default implementation does nothing. */

    virtual void handlePixelData (vrpn_int32 x, vrpn_int32 y,
                                  vrpn_int32 dx, vrpn_int32 dy,
                                  vrpn_int32 pixelCount,
                                  const char * buffer);
      /**< Default implementation does nothing. */

    virtual void handleDepthData (vrpn_int32 x, vrpn_int32 y,
                                  vrpn_int32 dx, vrpn_int32 dy,
                                  vrpn_int32 pixelCount,
                                  const vrpn_float64 * buffer);
      /**< Default implementation does nothing. */

  protected:

    nmg_Graphics_RenderClient_Implementation * d_imp;

    vrpn_int32 d_screenSizeX;
    vrpn_int32 d_screenSizeY;


};



/** \class nmg_RCIS_Mesh
 * Gets pixel and depth data over the network and stores it in a BCGrid,
 * reconstructing the original vertex-colored polygon mesh.
 * \warning Doesn't handle depth properly (really an encoding fault at the
 * sender, or at least one we mean to fix by changing encoding at the sender
 * from Z-buffer-value-read to true Z).
 */

class nmg_RCIS_Mesh : public nmg_RCIStrategy {

  public:

    nmg_RCIS_Mesh (nmg_Graphics_RenderClient_Implementation *,
                     vrpn_int32 xsize, vrpn_int32 ysize);
    virtual ~nmg_RCIS_Mesh (void);

    virtual void handlePixelData (vrpn_int32 x, vrpn_int32 y,
                                  vrpn_int32 dx, vrpn_int32 dy,
                                  vrpn_int32 pixelCount,
                                  const char * buffer);
    virtual void handleDepthData (vrpn_int32 x, vrpn_int32 y,
                                  vrpn_int32 dx, vrpn_int32 dy,
                                  vrpn_int32 pixelCount,
                                  const vrpn_float64 * buffer);


};



/** \class nmg_RCIS_TextureBase
 * Base class for strategies that represent the pixel data received over the
 * network as a flat array of characters in remote_data (global declared
 * in graphics_globals.h).
 */

class nmg_RCIS_TextureBase : public nmg_RCIStrategy {

  public:

    nmg_RCIS_TextureBase (nmg_Graphics_RenderClient_Implementation *,
                     vrpn_int32 xsize, vrpn_int32 ysize);
    virtual ~nmg_RCIS_TextureBase (void) = 0;

    virtual void initializeTextures (void);

    virtual void handlePixelData (vrpn_int32 x, vrpn_int32 y,
                                  vrpn_int32 dx, vrpn_int32 dy,
                                  vrpn_int32 pixelCount,
                                  const char * buffer);

  protected:

    vrpn_bool d_textureChanged;
};



/** \class nmg_RCIS_Texture
 * Gets a texture of the parallel-projected surface over the network
 * and displays it on the underlying mesh.
 */

class nmg_RCIS_Texture : public nmg_RCIS_TextureBase {

  public:

    nmg_RCIS_Texture (nmg_Graphics_RenderClient_Implementation *,
                     vrpn_int32 xsize, vrpn_int32 ysize);
    virtual ~nmg_RCIS_Texture (void);

    virtual void mainloop (void);
      /**< Rebuilds texture if necessary;  sets up texture transform matrix. */
    virtual void initializeTextures (void);

};



/** \class nmg_RCIS_Video
 * Gets an exact screen capture over the network and blits it onto the
 * screen.
 */

class nmg_RCIS_Video : public nmg_RCIS_TextureBase {

  public:

    nmg_RCIS_Video (nmg_Graphics_RenderClient_Implementation *,
                     vrpn_int32 xsize, vrpn_int32 ysize);
    virtual ~nmg_RCIS_Video (void);

    virtual void mainloop (void);
      /**< Writes the texture into the window as a block of pixels;
       * doesn't call nmg_Graphics_Implementation::mainloop().
       */

};







nmg_RCIStrategy::nmg_RCIStrategy
        (nmg_Graphics_RenderClient_Implementation * imp,
                     vrpn_int32 xsize, vrpn_int32 ysize) :
    d_imp (imp),
    d_screenSizeX (xsize),
    d_screenSizeY (ysize) {

}

// virtual
nmg_RCIStrategy::~nmg_RCIStrategy (void) {

}

// virtual
void nmg_RCIStrategy::mainloop (void) {

  d_imp->inputConnection()->mainloop();
  d_imp->nmg_Graphics_Implementation::mainloop();

}

// virtual
void nmg_RCIStrategy::initializeTextures (void) {

  // do nothing

}

// virtual
void nmg_RCIStrategy::handlePixelData (
    vrpn_int32 /*x*/, vrpn_int32 /*y*/,
    vrpn_int32 /*dx*/, vrpn_int32 /*dy*/,
    vrpn_int32 /*pixelCount*/,
    const char * /*buffer*/)
{
      // Default implementation does nothing. 
}

// virtual
void nmg_RCIStrategy::handleDepthData (
    vrpn_int32 /*x*/, vrpn_int32 /*y*/,
    vrpn_int32 /*dx*/, vrpn_int32 /*dy*/,
    vrpn_int32 /*pixelCount*/,
    const vrpn_float64 * /*buffer*/)
{
      // Default implementation does nothing.
}







nmg_RCIS_Mesh::nmg_RCIS_Mesh (nmg_Graphics_RenderClient_Implementation * imp,
                              vrpn_int32 xsize, vrpn_int32 ysize) :
    nmg_RCIStrategy (imp, xsize, ysize) {

}

// virtual
nmg_RCIS_Mesh::~nmg_RCIS_Mesh (void) {

}

// virtual
void nmg_RCIS_Mesh::handlePixelData (vrpn_int32 nx, vrpn_int32 ny,
                                     vrpn_int32 dx, vrpn_int32 dy,
                                     vrpn_int32 pixelCount,
                                     const char * buffer) {

  BCPlane * red;
  BCPlane * green;
  BCPlane * blue;

  vrpn_int32 i;
  vrpn_int32 l;

  red = d_imp->renderingGrid()->getPlaneByName("prerendered red");
  green = d_imp->renderingGrid()->getPlaneByName("prerendered green");
  blue = d_imp->renderingGrid()->getPlaneByName("prerendered blue");

  for (l = 0, i = 0; i < pixelCount; i++) {
    red->setValue(nx, ny, buffer[l++] / 255.0f);
    green->setValue(nx, ny, buffer[l++] / 255.0f);
    blue->setValue(nx, ny, buffer[l++] / 255.0f);

    d_imp->rangeOfChange.AddPoint(nx, ny);

    nx += dx;
    ny += dy;
  }
}

// virtual
void nmg_RCIS_Mesh::handleDepthData (vrpn_int32 nx, vrpn_int32 ny,
                                     vrpn_int32 dx, vrpn_int32 dy,
                                     vrpn_int32 pixelCount,
                                     const vrpn_float64 * buffer) {
  BCPlane * height;
  vrpn_int32 i;

  height = d_imp->renderingGrid()->getPlaneByName("captured depth");

  for (i = 0; i < pixelCount; i++) {
    height->setValue(nx, ny, buffer[i]);

    d_imp->rangeOfChange.AddPoint(nx, ny);

    nx += dx;
    ny += dy;
  }
}



nmg_RCIS_TextureBase::nmg_RCIS_TextureBase
                    (nmg_Graphics_RenderClient_Implementation * imp,
                     vrpn_int32 xsize, vrpn_int32 ysize) :
   nmg_RCIStrategy (imp, xsize, ysize),
   d_textureChanged (VRPN_FALSE) {

}

// virtual
nmg_RCIS_TextureBase::~nmg_RCIS_TextureBase (void) {

  if (remote_data) {
    delete [] remote_data;
  }
}

// virtual
void nmg_RCIS_TextureBase::initializeTextures (void) {

  int i;

  remote_data = new GLubyte [d_screenSizeX * d_screenSizeY * 4];
  if (!remote_data) {
    fprintf(stderr, "nmg_Graphics_RenderClient_Implementation::"
                    "initializeTextures:  Out of memory.\n");
    return;
  }

  for (i = 0; i < d_screenSizeX * d_screenSizeY * 4; i++) {
    remote_data[i] = i * 255 / d_screenSizeX / d_screenSizeY / 4;
  }
}

// virtual
void nmg_RCIS_TextureBase::handlePixelData (
    vrpn_int32 x, vrpn_int32 y,
    vrpn_int32 /*dx*/, vrpn_int32 /*dy*/,
    vrpn_int32 pixelCount,
    const char * buffer)
{

  // BUG - assumes dx 1, dy 0 or vice-versa or something

  vrpn_int32 i;
  vrpn_int32 k;
  vrpn_int32 l;

  k = (y * d_screenSizeX + x) * 4;

//if (!(y % 10)) {
//fprintf(stderr, "in handlePixelData at (%d, %d) for %d - offset %d:  %d, %d, %d.\n",
//x, y, pixelCount, k, buffer[0], buffer[1], buffer[2]);
//}

      
  for (l = 0, i = 0; i < pixelCount; i++) {
    remote_data[k++] = buffer[l++];
    remote_data[k++] = buffer[l++];
    remote_data[k++] = buffer[l++];
    remote_data[k++] = 255;
  }
  
//if (!(y % 10)) {
//fprintf(stderr, "leaving handlePixelData at (%d, %d) for %d - final offset %d:  %d, %d, %d.\n",
//x, y, pixelCount, k, buffer[l-3], buffer[l-2], buffer[l-1]);
//}

  d_textureChanged = VRPN_TRUE;
}



nmg_RCIS_Texture::nmg_RCIS_Texture
        (nmg_Graphics_RenderClient_Implementation * imp,
                              vrpn_int32 xsize, vrpn_int32 ysize) :
    nmg_RCIS_TextureBase (imp, xsize, ysize) {

}

// virtual
nmg_RCIS_Texture::~nmg_RCIS_Texture (void) {

}


// virtual
void nmg_RCIS_Texture::mainloop (void) {

//fprintf(stderr, "Texture mainloop.\n");

// BUG BUG BUG
// Extra frame of latency -
//   inputConnection->mainloop() is only called in RCIStrategy::mainloop,
// AFTER we've updated the texture!


  if (d_textureChanged) {
    buildRemoteRenderedTexture(d_screenSizeX, d_screenSizeY, remote_data);
    d_textureChanged = VRPN_FALSE;
  }
  // hack - overloading genetic texture coordinate handling
  // But now genetic textures have been removed. 

  float dx = d_imp->dataset()->inputGrid->maxX() -
             d_imp->dataset()->inputGrid->minX();
  float dy = d_imp->dataset()->inputGrid->maxY() -
             d_imp->dataset()->inputGrid->minY();
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
  g_texture_transform[12] = -d_imp->dataset()->inputGrid->minX() / dx;
  g_texture_transform[13] = -d_imp->dataset()->inputGrid->minY() / dy;
  g_texture_transform[14] = 0.0;
  g_texture_transform[15] = 1.0;

  // Can't call setTextureMode() every frame because that calls
  // causeGridRedraw(), which regenerates the grid.
  g_texture_mode = GL_TEXTURE_2D;
  g_texture_displayed = nmg_Graphics::REMOTE_DATA;
  g_texture_transform_mode = nmg_Graphics::REGISTRATION_COORD;

  // in the right window
  v_gl_set_context_to_vlib_window();

  glShadeModel(GL_FLAT);

  nmg_RCIStrategy::mainloop();

}


// virtual
void nmg_RCIS_Texture::initializeTextures (void) {

  nmg_RCIS_TextureBase::initializeTextures();

  buildRemoteRenderedTexture(d_screenSizeX, d_screenSizeY,
                             remote_data);

}






nmg_RCIS_Video::nmg_RCIS_Video (nmg_Graphics_RenderClient_Implementation * imp,
                     vrpn_int32 xsize, vrpn_int32 ysize) :
    nmg_RCIS_TextureBase (imp, xsize, ysize) {

}

// virtual
nmg_RCIS_Video::~nmg_RCIS_Video (void) {

}


extern void swapbuffers ();

// virtual
void nmg_RCIS_Video::mainloop (void) {

  GLenum i;

//fprintf(stderr, "Video mainloop.\n");

  // get any new frame data
  d_imp->inputConnection()->mainloop();

  // draw all the frame data we've got:

  // in the right window
  v_gl_set_context_to_vlib_window();

  // let glRasterPos() work in screen coordinates
  glViewport(0, 0, d_screenSizeX, d_screenSizeY);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0.0, d_screenSizeX, 0.0, d_screenSizeY);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glShadeModel(GL_FLAT);
  glDrawBuffer(GL_FRONT);
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

  //glScissor(0, 0, d_screenSizeX, d_screenSizeY);
  glDisable(GL_SCISSOR_TEST);
  glDisable(GL_ALPHA_TEST);
  glDisable(GL_STENCIL_TEST);
  glDisable(GL_DEPTH_TEST);

  // blit remote_data onto the screen
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  //glClear(GL_COLOR_BUFFER_BIT);
  glRasterPos2i(0, 0);
  glDrawPixels(d_screenSizeX, d_screenSizeY, GL_RGBA, GL_UNSIGNED_BYTE,
               remote_data);
  glFlush();

//fprintf(stderr, "Drew %d x %d.\n", d_screenSizeX, d_screenSizeY);

  // check for errors
  while ((i = glGetError()) != GL_NO_ERROR) {
    fprintf(stderr, "  %s\n", gluErrorString(i));
  }

  // hack - from vlib (vogl)
  //swapbuffers();
  //glutSwapBuffers();

}














nmg_Graphics_RenderClient_Implementation::
nmg_Graphics_RenderClient_Implementation
                      (nmb_Dataset * data,
                       const int minColor [3],
                       const int maxColor [3],
                       vrpn_Connection * inputConnection,
                       RemoteColorMode cMode,
                       RemoteDepthMode dMode,
                       RemoteProjectionMode pMode,
                       nmb_TimerList * timer,
                       int xsize, int ysize) :
    nmg_Graphics_Implementation (data, minColor, maxColor, NULL, NULL),
    rangeOfChange (d_renderingGrid),
    //d_pixelBuffer (new unsigned char [xsize * ysize * 3]),
    //d_depthBuffer (new float [xsize * ysize]),
    //d_renderingGrid (xsize, ysize, 0, xsize - 1, 0, ysize - 1),
    d_renderingGrid (new BCGrid (xsize, ysize, data->inputGrid->minX(),
                                   data->inputGrid->maxX(),
                                   data->inputGrid->minY(),
                                   data->inputGrid->maxY())),
    d_inputConnection (inputConnection),
    d_timer (timer),
    d_strategy (NULL) {


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
  // This used to be cMode, but it caused a warning, and I think
  // it should be dMode
  if (dMode == VERTEX_DEPTH) {
    g_PRERENDERED_DEPTH = 1;
  }
  g_prerendered_grid = d_renderingGrid;
  g_prerenderedChange = &rangeOfChange;

  // vertex array is initialized by nmg_Graphics_Implementation, but
  // it gets the wrong size
  // Remove the conditional on g_VERTEX_ARRAY on 12/8/00 by Jason
  // Removed because the vertex arrays are used to store normals so
  // they are always needed, and this conforms with nmg_Graphics_Implementation

  if (!g_surface->init(xsize, ysize)) {
	  fprintf(stderr," initVertexArrays: out of memory.\n");
      exit(0);
  }
  

  d_dataset->inputGrid->registerMinMaxCallback(getNewBounds, this);

  if ((cMode == VERTEX_COLORS) && (dMode == VERTEX_DEPTH)) {
    d_strategy = new nmg_RCIS_Mesh (this, xsize, ysize);
  } else if ((cMode == SUPERSAMPLED_COLORS) && (dMode == NO_DEPTH)) {
    if (pMode == ORTHO_PROJECTION) {
      d_strategy = new nmg_RCIS_Texture (this, xsize, ysize);
    } else if (pMode == PERSPECTIVE_PROJECTION) {
      d_strategy = new nmg_RCIS_Video (this, xsize, ysize);
    }
  }

  if (d_strategy) {
    d_strategy->initializeTextures();
  }

  setTextureMode(REMOTE_DATA, REGISTRATION_COORD);

}



// virtual
nmg_Graphics_RenderClient_Implementation::
~nmg_Graphics_RenderClient_Implementation (void) {

  if (d_strategy) {
    delete d_strategy;
  }
}

// virtual
void nmg_Graphics_RenderClient_Implementation::mainloop (void) {

  if (d_strategy) {
    d_strategy->mainloop();
  }

}

vrpn_Connection * nmg_Graphics_RenderClient_Implementation::inputConnection
        (void) {
  return d_inputConnection;
}

BCGrid * nmg_Graphics_RenderClient_Implementation::renderingGrid (void) {
  return d_renderingGrid;
}

nmb_Dataset * nmg_Graphics_RenderClient_Implementation::dataset (void) {
  return d_dataset;
}


// PROTECTED


// static
int nmg_Graphics_RenderClient_Implementation::handle_pixelData
                                                 (void * userdata,
                                                  vrpn_HANDLERPARAM p) {
  nmg_Graphics_RenderClient_Implementation * gp =
          (nmg_Graphics_RenderClient_Implementation *) userdata;
  const char * bp = p.buffer;
  //char lb [3072];  // HACK - can't handle lines over 1024 pixels long

  vrpn_int32 x, y, dx, dy;
  vrpn_int32 lineCount, fieldCount;
  vrpn_int32 sec, usec;

  vrpn_unbuffer(&bp, &x);
  vrpn_unbuffer(&bp, &y);
  vrpn_unbuffer(&bp, &dx);
  vrpn_unbuffer(&bp, &dy);
  vrpn_unbuffer(&bp, &lineCount);
  vrpn_unbuffer(&bp, &fieldCount);
  vrpn_unbuffer(&bp, &sec);
  vrpn_unbuffer(&bp, &usec);

  //vrpn_unbuffer(&bp, lb, (vrpn_uint32) lineCount * 3);

  if (gp->d_strategy) {
    gp->d_strategy->handlePixelData(x, y, dx, dy, lineCount, bp);
  }

  return 0;
}

// static
int nmg_Graphics_RenderClient_Implementation::handle_depthData
                                                 (void * userdata,
                                                  vrpn_HANDLERPARAM p) {
  nmg_Graphics_RenderClient_Implementation * gp =
           (nmg_Graphics_RenderClient_Implementation *) userdata;
  const char * bp = p.buffer;

  vrpn_int32 x, y, dx, dy;
  vrpn_int32 lineCount, fieldCount;
  vrpn_int32 sec, usec;

  vrpn_int32 i;

  vrpn_float64 z [1024];  // HACK - can't handle lines over 1024 pixels long

  vrpn_unbuffer(&bp, &x);
  vrpn_unbuffer(&bp, &y);
  vrpn_unbuffer(&bp, &dx);
  vrpn_unbuffer(&bp, &dy);
  vrpn_unbuffer(&bp, &lineCount);
  vrpn_unbuffer(&bp, &fieldCount);
  vrpn_unbuffer(&bp, &sec);
  vrpn_unbuffer(&bp, &usec);

  // It would be more efficient to push this unbuffer down into
  // handleDepthData(), so that implementations that didn't care about
  // depth data wouldn't spend any time rerepresenting and copying it,
  // but if the server type properly matches the client type only those
  // clients who care about depth data will get any.

  for (i = 0; i < lineCount; i++) {
    vrpn_unbuffer(&bp, &z[i]);
  }

  if (gp->d_strategy) {
    gp->d_strategy->handleDepthData(x, y, dx, dy, lineCount, z);
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

//fprintf(stderr, "Unblocked timestamp #%d.\n", sn);

  return 0;
}


