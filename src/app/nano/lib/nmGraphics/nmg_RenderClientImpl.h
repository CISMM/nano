#ifndef NMG_GRAPHICS_RENDER_CLIENT_IMPL_H
#define NMG_GRAPHICS_RENDER_CLIENT_IMPL_H

#include "nmg_GraphicsImpl.h"

#include <vrpn_Connection.h>  // for vrpn_HANDLERPARAM

class nmb_TimerList;  // from nmb_TimerList.h
#include <BCGrid.h>
#include <nmb_Subgrid.h>

class nmg_Graphics_RenderClient_Implementation :
                               public nmg_Graphics_Implementation {

  public:

    nmg_Graphics_RenderClient_Implementation (nmb_Dataset * data,
                       const int minColor [3],
                       const int maxColor [3],
                       vrpn_Connection * inputConnection,
                       RemoteColorMode cMode,
                       RemoteDepthMode dMode,
                       nmb_TimerList * timer,
                       int xsize = 32, int ysize = 32);

    virtual ~nmg_Graphics_RenderClient_Implementation (void);

    virtual void mainloop (void);

    nmb_Subgrid rangeOfChange;

  protected:

    virtual void initializeTextures (void);

    static int handle_pixelData (void *, vrpn_HANDLERPARAM);
    static int handle_depthData (void *, vrpn_HANDLERPARAM);
      // Recieve data from VRPN.

    static int handle_timerSN (void *, vrpn_HANDLERPARAM);

    static void getNewBounds (void * userdata, double minX, double maxX,
                                               double minY, double maxY);
      // Make sure the grid we render from tracks movement of the grid
      // the microscope is writing into.

    RemoteColorMode d_colorMode;
    RemoteDepthMode d_depthMode;

    unsigned char * d_pixelBuffer;
    float * d_depthBuffer;

    vrpn_int32 d_screenSizeX;
    vrpn_int32 d_screenSizeY;

    BCGrid * d_renderingGrid;
      // Need to make this a pointer because of limitations of
      // rangeOfChange constructor - aargh.

    vrpn_Connection * d_inputConnection;

    vrpn_int32 d_myId;
    vrpn_int32 d_pixelData_type;
    vrpn_int32 d_depthData_type;

    vrpn_int32 d_timerSN_type;

    vrpn_bool d_remoteTextureChanged;

    nmb_TimerList * d_timer;
};

#endif  // NMG_GRAPHICS_RENDER_CLIENT_IMPL_H

