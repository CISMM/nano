#ifndef NMG_GRAPHICS_RENDER_SERVER_H
#define NMG_GRAPHICS_RENDER_SERVER_H

#include <vrpn_Connection.h>  // for vrpn_HANDLERPARAM

#include <nmb_TimerList.h>

#include "nmg_GraphicsImpl.h"

// A RenderServer is an implementation of nmg_Graphics that expects to
// render a nanoManipulator image and send it off to a RenderClient.
// To generalize it, a RenderServer is driven by the small objects
// defined in RenderServerStrategies.h.
// The original set of strategies composes to form the following
// meaningful options:

//   Orthogonal view, Texture capture:
//     The server sends a texture to be draped over a polymesh.
//   Orthogonal view, Vertex capture:
//     "IBR":  The server sends a set of rgbz values to be used
//     for IBR.
//   Slave view, Texture capture:
//     "Video":  The server sends a screen capture to be blitted
//     to the client's screen.

//   (Slave view, vertex capture is in most ways inferior to orthogonal
// view, vertex capture;  specularities are correct for the sampled
// viewpoint, but the surface is undersampled because of occlusions.)

// Strategies to be added in the near future include an interface to
// Jason Clark's nmg_CloudTexturer rendering class.



// from nmg_RenderServerStrategies.h
// Circular dependency, so we can't include it.
class nmg_RenderServer_ViewStrategy;
class nmg_RenderServer_Strategy;



class nmg_Graphics_RenderServer : public nmg_Graphics_Implementation {

  public:

    nmg_Graphics_RenderServer (nmb_Dataset * data,
                       const int minColor [3],
                       const int maxColor [3],
                       vrpn_Connection * outputConnection,
                       RemoteColorMode cMode,
                       RemoteDepthMode dMode,
                       RemoteProjectionMode pMode,
                       int xsize = 32, int ysize = 32,
                       const char * rulergridName = NULL,
                       vrpn_Connection * commandsFromUser = NULL);

    virtual ~nmg_Graphics_RenderServer (void);

    virtual void mainloop (void);
      // Does a render, captures the maps, and writes them
      // into the output dataset.

    virtual void resizeViewport (int width, int height);
      // Does nothing - mustn't resize this viewport!

    // All other functions inherited from nmg_Graphics_Implementation.

  // Functions for use by strategies

    vrpn_int32 screenSizeX (void) const;
    vrpn_int32 screenSizeY (void) const;

    void screenCapture (void);
    void depthCapture (void);

    void sendPartialPixelData (int minx, int maxx, int miny, int maxy);
    void sendPartialDepthData (int minx, int maxx, int miny, int maxy);

    void defaultRender (void);

    virtual void setViewTransform (v_xform_type);
      ///< Override default behavior to allow our strategies to
      ///< decide whether or not to accept a view transform from
      ///< the remote.


  // Diagnostic functions

    vrpn_int32 messagesSent (void) const;
      ///< Reports how many VRPN messages of graphics information
      ///< have been sent to a client.  Allows user to compute
      ///< length of VRPN headers and *estimate* size of TCP/UDP
      ///< headers to get a total value for the dataflow.
      ///< Does NOT report VRPN system messages, timing messages,
      ///< or other auxiliary data, just pixel and depth buffer fragments.
    vrpn_int32 bytesSent (void) const;
      ///< Reports how many bytes of graphics information (including
      ///< application-level headers) have been sent to a client.
      ///< Does NOT report VRPN system messages, timing messages,
      ///< or other auxiliary data, just pixel and depth buffer fragments.


  protected:

    void computeScreenChange (int * minx, int * maxx, int * miny, int * maxy);
      ///< Computes the portion of the screen that has changed in the current
      ///< frame, overriding the computation if d_sendEntireScreen or
      ///< d_viewStrategy->alwaysSendEntireScreen() are true.

    void clearWaitingTimestamps (void);
      ///< Send the serial numbers of all timestamps that were waiting for
      ///< this update back to our client so that graphics time is correctly
      ///< tracked.

    unsigned char * d_pixelBuffer;
    float * d_depthBuffer;

    char * d_pixelOutputBuffer;
    char * d_depthOutputBuffer;
    vrpn_int32 d_pixelOutputBufferSize;
    vrpn_int32 d_depthOutputBufferSize;

    vrpn_int32 d_screenSizeX;
    vrpn_int32 d_screenSizeY;

    vrpn_Connection * d_outputConnection;

    vrpn_int32 d_myId;
    vrpn_int32 d_pixelData_type;
    vrpn_int32 d_depthData_type;

    vrpn_int32 d_timerSN_type_in;
    vrpn_int32 d_timerSN_type_out;

    static int handle_newConnection (void *, vrpn_HANDLERPARAM);
    static int handle_timerSN (void *, vrpn_HANDLERPARAM);

    nmb_TimerList d_timerList;

    vrpn_bool d_sendEntireScreen;  // on next rerender

    nmg_RenderServer_Strategy * d_strategy;
    nmg_RenderServer_ViewStrategy * d_viewStrategy;

  private:
    
    //Sometimes the Strategies need access to private data members of
    //nmg_RenderServer.  The cleanest way to handle this I feel, is to set
    //nmg_RenderServer_Strategy (the base class for all the strategies as a
    //friend), then define member functions to access those members in the
    //base strategy class, and have the children call those when necessary.
    //This scopes the internal access of RenderServer to one class
    friend class nmg_RenderServer_Strategy;

    vrpn_int32 d_messagesSent;
    vrpn_int32 d_bytesSent;
    
};

#endif  // NMG_GRAPHICS_RENDER_SERVER_H






