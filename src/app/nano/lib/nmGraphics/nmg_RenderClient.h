/*===3rdtech===
  Copyright (c) 2000 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#ifndef NMG_GRAPHICS_RENDER_CLIENT_H
#define NMG_GRAPHICS_RENDER_CLIENT_H

#include <nmb_TimerList.h>

#include "nmg_GraphicsRemote.h"
#include "nmg_RenderClientImpl.h"

// Acts as a nmg_Graphics_Remote sending commands to a nmg_RenderServer.
// Also (mostly) encapsulates an nmg_Graphics_RenderClient_Implementation
// to display data recieved from the RenderServer.

class nmg_Graphics_RenderClient : public nmg_Graphics_Remote {

  public:

    nmg_Graphics_RenderClient (nmb_Dataset * data,
                         const int minColor [3],  // get rid of this
                         const int maxColor [3],  // get rid of this
                         vrpn_Connection * inputConnection,
                         RemoteColorMode cMode,
                         RemoteDepthMode dMode,
                         RemoteProjectionMode pMode,
                         int xsize, int ysize,
                         vrpn_Connection * controlConnection,
                         nmb_TimerList * timer = NULL);


    virtual ~nmg_Graphics_RenderClient (void);


    // ACCESSORS


    virtual void setGraphicsTiming (vrpn_bool);
      ///< Only if VRPN_TRUE is passed into this function will graphics
      ///< timing be done.  (Graphics timing doubles network traffic on
      ///< one particular link, although it isn't significant traffic in
      ///< an absolute sense.)

    virtual void getViewportSize(int *width, int * height);
    virtual void getDisplayPosition (q_vec_type &ll, q_vec_type &ul,
                                     q_vec_type &ur);
    virtual void getLightDirection (q_vec_type *) const;
    virtual int getHandColor (void) const;
    virtual int getSpecularity (void) const;
    virtual const double * getMinColor (void) const;
    virtual const double * getMaxColor (void) const;

    nmb_Subgrid & rangeOfChange (void);


    // MANIPULATORS


    virtual void mainloop (void);


    // For timing, we need to override any function that issues
    // a causeGridRedraw() so that it also suspends d_timer and
    // sends a timer SN to the server.  Our implementation will
    // get that timer SN back and unblock the corresponding timer.

    virtual void setAlphaSliderRange (float, float);
    virtual void setColorMapName (const char *);
    virtual void setColorMinMax (float, float);
    virtual void setDataColorMinMax (float, float);
    virtual void setOpacitySliderRange (float, float);
    virtual void setComplianceSliderRange (float, float);
    virtual void setContourColor (int, int, int);
    virtual void setFrictionSliderRange (float, float);
    virtual void setBumpSliderRange (float, float);
    virtual void setBuzzSliderRange (float, float);
    virtual void setContourWidth (float);
    virtual void setPatternMapName (const char *);
    virtual void translateTextures (int, float, float);
    virtual void scaleTextures (int, float, float);
    virtual void shearTextures (int, float, float);
    virtual void rotateTextures (int, float);
    virtual void setTextureTransform (double *);
    virtual void setRulergridAngle (float);
    virtual void setRulergridOffset (float, float);
    virtual void setNullDataAlphaToggle (int);
    virtual void setRulergridScale (float);
    virtual void setRulergridWidths (float, float);
    virtual void setSpecularity (int);
    virtual void setDiffusePercent (float);
    virtual void setSurfaceAlpha (float);
    virtual void setSpecularColor (float);
    virtual void setTesselationStride (int);
    virtual void setTextureMode (TextureMode, TextureTransformMode);
    virtual void setTextureScale (float);

    virtual void setViewTransform (v_xform_type);


  protected:

    nmg_Graphics_RenderClient_Implementation d_implementation;

    void blockTimer (void);
    nmb_TimerList * d_timer;

    vrpn_bool d_timeGraphics;

    vrpn_int32 d_timerSN_type;
};

#endif  // NMG_GRAPHICS_RENDER_CLIENT_H

