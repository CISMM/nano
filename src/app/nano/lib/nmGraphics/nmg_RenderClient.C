/*===3rdtech===
  Copyright (c) 2000 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#include <string.h>
#include "nmg_RenderClient.h"

#include <vrpn_Shared.h>
#include <vrpn_Connection.h>

nmg_Graphics_RenderClient::nmg_Graphics_RenderClient
          (nmb_Dataset * data,
           const int surfaceColor [3],
           vrpn_Connection * inputConnection,
           RemoteColorMode cMode,
           RemoteDepthMode dMode,
           RemoteProjectionMode pMode,
           int xsize, int ysize,
           vrpn_Connection * controlConnection,
           nmb_TimerList * timer) :
    nmg_Graphics_Remote (controlConnection),
    d_implementation (data, surfaceColor, inputConnection,
                      cMode, dMode, pMode, timer, xsize, ysize),
    d_timer (timer),
    d_timeGraphics (VRPN_FALSE) {

  // HACK
  // Need to call *some* of the initiaization functions -
  // aren't sure which of these are necessary.
  d_implementation.setColorMapDirectory("");
  d_implementation.setTextureDirectory("");
  d_implementation.setAlphaColor(0.0f, 0.0f, 0.0f);
  d_implementation.setTextureScale(1.0f);
  d_implementation.setColorMapName("");
  d_implementation.setTextureMode(NO_TEXTURES);
  d_implementation.resetLightDirection();


  d_timerSN_type = controlConnection->register_message_type
        ("nmg Graphics RemoteRender Timer SN");  // 34 char

}



nmg_Graphics_RenderClient::~nmg_Graphics_RenderClient (void) {


}


void nmg_Graphics_RenderClient::setGraphicsTiming (vrpn_bool on) {
  d_timeGraphics = on;
}

void nmg_Graphics_RenderClient::mainloop (void) {

  nmg_Graphics_Remote::mainloop();
  d_implementation.mainloop();

}

nmb_Subgrid & nmg_Graphics_RenderClient::rangeOfChange (void) {
  return d_implementation.rangeOfChange;
}




void nmg_Graphics_RenderClient::blockTimer (void) {

  char msgbuf [24];  // hack - static buf, but more than we need
  char * mptr;
  timeval now;
  vrpn_int32 sn;
  vrpn_int32 msglen;

  if (!d_timeGraphics) {
    return;
  }
  if (!d_timer) {
    fprintf(stderr, "nmg_Graphics_RenderClient::blockTimer:  no timer!\n");
    return;
  }

  // Block the current timer
  sn = d_timer->getListHead();
  if (sn < 0) {
    // No timer in the queue.
    // Should only happen on startup.
    return;
  }
  d_timer->block(sn);

  // Pack the message
  msglen = 24;
  mptr = msgbuf;
  vrpn_buffer(&mptr, &msglen, sn);

  // Send message
  gettimeofday(&now, NULL);
  d_connection->pack_message(24 - msglen, now, d_timerSN_type,
                             nmg_Graphics::d_myId, msgbuf,
                             vrpn_CONNECTION_RELIABLE);

  fprintf(stderr, "Blocked timer %d.\n", sn);
}

// virtual
void nmg_Graphics_RenderClient::setAlphaSliderRange (float a, float b) {
  blockTimer();
  nmg_Graphics_Remote::setAlphaSliderRange (a, b);
}

// virtual
void nmg_Graphics_RenderClient::setColorMapName (const char * a) {
  blockTimer();
  nmg_Graphics_Remote::setColorMapName (a);
}

// virtual
void nmg_Graphics_RenderClient::setColorMinMax (float a, float b) {
  blockTimer();
  nmg_Graphics_Remote::setColorMinMax (a, b);
}

// virtual
void nmg_Graphics_RenderClient::setDataColorMinMax (float a, float b) {
  blockTimer();
  nmg_Graphics_Remote::setDataColorMinMax (a, b);
}

// virtual
void nmg_Graphics_RenderClient::setOpacitySliderRange (float a, float b) {
  blockTimer();
  nmg_Graphics_Remote::setOpacitySliderRange (a, b);
}

// virtual
void nmg_Graphics_RenderClient::setContourColor (int a, int b, int c) {
  blockTimer();
  nmg_Graphics_Remote::setContourColor (a, b, c);
}

// virtual
void nmg_Graphics_RenderClient::setContourWidth (float a) {
  blockTimer();
  nmg_Graphics_Remote::setContourWidth (a);
}

// virtual
void nmg_Graphics_RenderClient::setTextureTransform (double * a) {
  blockTimer();
  nmg_Graphics_Remote::setTextureTransform (a);
}

// virtual
void nmg_Graphics_RenderClient::setRulergridAngle (float a) {
  blockTimer();
  nmg_Graphics_Remote::setRulergridAngle (a);
}

// virtual
void nmg_Graphics_RenderClient::setRulergridOffset (float a, float b) {
  blockTimer();
  nmg_Graphics_Remote::setRulergridOffset (a, b);
}

// virtual
void nmg_Graphics_RenderClient::setNullDataAlphaToggle (int val) {
  blockTimer();
  nmg_Graphics_Remote::setNullDataAlphaToggle (val);
}

// virtual
void nmg_Graphics_RenderClient::setRulergridScale (float a) {
  blockTimer();
  nmg_Graphics_Remote::setRulergridScale (a);
}

// virtual
void nmg_Graphics_RenderClient::setRulergridWidths (float a, float b) {
  blockTimer();
  nmg_Graphics_Remote::setRulergridWidths (a, b);
}

// virtual
void nmg_Graphics_RenderClient::setSpecularity (int a) {
  blockTimer();
  nmg_Graphics_Remote::setSpecularity (a);
}

// virtual
void nmg_Graphics_RenderClient::setDiffusePercent (float a) {
  blockTimer();
  nmg_Graphics_Remote::setDiffusePercent (a);
}

// virtual
void nmg_Graphics_RenderClient::setSurfaceAlpha (float a, int region) {
  blockTimer();
  nmg_Graphics_Remote::setSurfaceAlpha (a, region);
}

// virtual
void nmg_Graphics_RenderClient::setSpecularColor (float a) {
  blockTimer();
  nmg_Graphics_Remote::setSpecularColor (a);
}

// virtual
void nmg_Graphics_RenderClient::setTesselationStride (int a, int region) {
  blockTimer();
  nmg_Graphics_Remote::setTesselationStride (a, region);
}

// virtual
void nmg_Graphics_RenderClient::setTextureMode (TextureMode a,
                                                TextureTransformMode b,
                                                int region) {
  blockTimer();
  nmg_Graphics_Remote::setTextureMode (a, b, region);
}

// virtual
void nmg_Graphics_RenderClient::setTextureScale (float a) {
  blockTimer();
  nmg_Graphics_Remote::setTextureScale (a);
}

// virtual
void nmg_Graphics_RenderClient::setViewTransform (v_xform_type x) {
  blockTimer();
  nmg_Graphics_Remote::setViewTransform (x);
  d_implementation.setViewTransform(x);
}


// virtual
void nmg_Graphics_RenderClient::getViewportSize(int *width, int *height) 
{
    d_implementation.getViewportSize(width, height);
    
}

// virtual
void nmg_Graphics_RenderClient::getDisplayPosition (q_vec_type &ll, q_vec_type &ul,
                                                    q_vec_type &ur)
{
    d_implementation.getDisplayPosition(ll, ul, ur);
}

// virtual
void nmg_Graphics_RenderClient::getLightDirection (q_vec_type *l) const
{
    d_implementation.getLightDirection(l);
}

// virtual
int nmg_Graphics_RenderClient::getHandColor (void) const
{
    return d_implementation.getHandColor();
}

// virtual
int nmg_Graphics_RenderClient::getSpecularity (void) const
{
    return d_implementation.getSpecularity();
}

// virtual
const double * nmg_Graphics_RenderClient::getSurfaceColor (void) const
{
    return d_implementation.getSurfaceColor();
}

