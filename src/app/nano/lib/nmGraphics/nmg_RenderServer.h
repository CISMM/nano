#ifndef NMG_GRAPHICS_RENDER_SERVER_H
#define NMG_GRAPHICS_RENDER_SERVER_H

#include <vrpn_Connection.h>  // for vrpn_HANDLERPARAM

#include <nmb_TimerList.h>

#include "nmg_GraphicsImpl.h"

// The comment below is incorrect;  this class has been greatly generalized.

// A RenderServer is an implementation of nmg_Graphics that renders
// a microscope heightPlane in parallel projection at a user-defined
// resolution, then captures the image and depth map of that
// surface and writes it into an nmb_Dataset where it can be used
// for remote rendering (IBR or other approaches).

// To use:
//   setenv V_DISPLAY "ortho crt"
//   setenv V_SCREEN_DIM_PXFL "32 32"
//     (or whatever size you pass in to the RenderServer constructor -
//      to avoid interpolation artifacts this should be the grid size
//      or an even multiple thereof.)

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

};

#endif  // NMG_GRAPHICS_IMPL_H






