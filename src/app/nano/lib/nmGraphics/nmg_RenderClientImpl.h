#ifndef NMG_GRAPHICS_RENDER_CLIENT_IMPL_H
#define NMG_GRAPHICS_RENDER_CLIENT_IMPL_H

#include "nmg_GraphicsImpl.h"

#include <vrpn_Connection.h>  // for vrpn_HANDLERPARAM

class nmb_TimerList;  // from nmb_TimerList.h
#include <BCGrid.h>
#include <nmb_Subgrid.h>


class nmg_RCIStrategy;


/** \class nmg_Graphics_RenderClient_Implementation
 * A Graphics_Implementation is used by a nmg_Graphics_RenderClient
 * to render meshes, textures, or bitmaps sent by a nmg_Graphics_RenderServer.
 */

class nmg_Graphics_RenderClient_Implementation :
                               public nmg_Graphics_Implementation {

  public:

    nmg_Graphics_RenderClient_Implementation (nmb_Dataset * data,
                       const int surfaceColor [3],
                       vrpn_Connection * inputConnection,
                       RemoteColorMode cMode,
                       RemoteDepthMode dMode,
                       RemoteProjectionMode pMode,
                       nmb_TimerList * timer,
                       int xsize = 32, int ysize = 32);

    virtual ~nmg_Graphics_RenderClient_Implementation (void);

    virtual void mainloop (void);

    nmb_Subgrid rangeOfChange;

  // functions for use by strategies

    vrpn_Connection * inputConnection (void);
    BCGrid * renderingGrid  (void);

    nmb_Dataset * dataset (void);

  protected:

    static int handle_pixelData (void *, vrpn_HANDLERPARAM);
      /**< Receives colors from the RenderServer over d_inputConnection */

    static int handle_depthData (void *, vrpn_HANDLERPARAM);
      /**< Receives depths from the RenderServer over d_inputConnection */

    static int handle_timerSN (void *, vrpn_HANDLERPARAM);
      /**<
       * Support for timing (nmb_TimerList).
       * When a graphics command is queued up by the nmg_Graphics_RenderClient,
       * a timer serial number is sent to the nmg_Graphics_RenderServer;
       * after the results of that graphics command have been sent to
       * this Implementation the timer serial number is also sent.
       * This function then marks the timer as complete.
       */

    static void getNewBounds (void * userdata, double minX, double maxX,
                                               double minY, double maxY);
      /**<
       * Makes sure the grid we render from tracks movement of the grid
       * the microscope is writing into.
       */

    //unsigned char * d_pixelBuffer;
    //float * d_depthBuffer;

    BCGrid * d_renderingGrid;
      /**<
       * The grid we display from, which contains colors or depth or textures
       * or ... rather than actual microscope data.
       * This is only a pointer because of limitations of the
       * rangeOfChange constructor - aargh.
       */

    vrpn_Connection * d_inputConnection;

    vrpn_int32 d_myId;
    vrpn_int32 d_pixelData_type;
    vrpn_int32 d_depthData_type;

    vrpn_int32 d_timerSN_type;

    nmb_TimerList * d_timer;
      /**<
       * The timer list whose elements we mark as complete after receiving
       * their serial numbers back from the nmg_Graphics_RenderServer.
       */

    nmg_RCIStrategy * d_strategy;
};

#endif  // NMG_GRAPHICS_RENDER_CLIENT_IMPL_H

