#ifndef NMG_RENDERSERVER_STRATEGIES_H
#define NMG_RENDERSERVER_STRATEGIES_H

// These small objects parameterize class nmg_Graphics_RenderServer.

#include <vrpn_Types.h>

class nmg_Graphics_RenderServer;
class nmg_CloudTexturer;
class nmg_State;

/** \class nmg_RenderServer_ViewStrategy
 * Specifies to a render server how to manage the viewing transform
 * and some other details of the rendering process.
 */

class nmg_RenderServer_ViewStrategy {

  public:

    nmg_RenderServer_ViewStrategy (nmg_Graphics_RenderServer *);
    virtual ~nmg_RenderServer_ViewStrategy (void) = 0;

    virtual vrpn_bool alwaysSendEntireScreen (void) const = 0;
    virtual void setViewingTransform (nmg_State *) = 0;

    virtual void setGraphicsModes (nmg_State *) = 0;

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
    virtual void setViewingTransform (nmg_State *);

    virtual void setGraphicsModes (nmg_State *);
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
    virtual void setViewingTransform (nmg_State *);

    virtual void setGraphicsModes (nmg_State *);
      /**< Turns on chartjunk, measure lines;  not "planeonly".
       * Slave wants to capture the entire scene, plane and decorations.
       */
};

class nmg_RenderServer_Strategy {

  public:

    nmg_RenderServer_Strategy (nmg_Graphics_RenderServer *);
    virtual ~nmg_RenderServer_Strategy (void) = 0;

    virtual void render (nmg_State *);
      ///< Standard implementation calls nmg_RenderServer::defaultRender().
    virtual void captureData (nmg_State *) = 0;
    virtual void sendData (int minx, int maxx, int miny, int maxy) = 0;
    void updatePixelBuffer(int minx, int maxx, int miny, int maxy, vrpn_uint8 *new_buf);
    

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

    virtual void captureData (nmg_State *);
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

    virtual void captureData (nmg_State * );
    virtual void sendData (int minx, int maxx, int miny, int maxy);

};

/** \class nmg_RSStrategy_CloudTexture
 * Sends back a
 * high resolution texture that is generated according to empirical
 * observations of clouds.
 */

class nmg_RSStrategy_CloudTexture : public nmg_RenderServer_Strategy {

  public:

    nmg_RSStrategy_CloudTexture (nmg_Graphics_RenderServer *);
    virtual ~nmg_RSStrategy_CloudTexture (void);

    virtual void render (nmg_State *);
    virtual void captureData (nmg_State *);
    virtual void sendData (int minx, int maxx, int miny, int maxy);

  private:
    vrpn_uint8 *d_frame;
    nmg_CloudTexturer *d_cloud_render;
};

#endif  // NMG_RENDERSERVER_STRATEGIES_H
