#include <string.h>
#include "nmg_RenderClient.h"

#include <vrpn_Shared.h>
#include <vrpn_Connection.h>

nmg_Graphics_RenderClient::nmg_Graphics_RenderClient
          (nmb_Dataset * data,
           const int minColor [3],
           const int maxColor [3],
           vrpn_Connection * inputConnection,
           RemoteColorMode cMode,
           RemoteDepthMode dMode,
           int xsize, int ysize,
           vrpn_Connection * controlConnection,
           nmb_TimerList * timer) :
    nmg_Graphics_Remote (controlConnection),
    d_implementation (data, minColor, maxColor, inputConnection,
                      cMode, dMode, timer, xsize, ysize),
    d_timer (timer) {

  // HACK
  // Need to call *some* of the initiaization functions -
  // aren't sure which of these are necessary.
  d_implementation.setColorMapDirectory("");
  d_implementation.setTextureDirectory("");
  d_implementation.setAlphaColor(0.0f, 0.0f, 0.0f);
  d_implementation.setTextureScale(1.0f);
  d_implementation.setBumpMapName("");
  d_implementation.setHatchMapName("");
  d_implementation.setColorMapName("");
  d_implementation.setTextureMode(NO_TEXTURES);
  d_implementation.resetLightDirection();


  d_timerSN_type = controlConnection->register_message_type
        ("nmg Graphics RemoteRender Timer SN");  // 34 char

}



nmg_Graphics_RenderClient::~nmg_Graphics_RenderClient (void) {


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
  int msglen;

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
void nmg_Graphics_RenderClient::setColorSliderRange (float a, float b) {
  blockTimer();
  nmg_Graphics_Remote::setColorSliderRange (a, b);
}

// virtual
void nmg_Graphics_RenderClient::setComplianceSliderRange (float a, float b) {
  blockTimer();
  nmg_Graphics_Remote::setComplianceSliderRange (a, b);
}

// virtual
void nmg_Graphics_RenderClient::setContourColor (int a, int b, int c) {
  blockTimer();
  nmg_Graphics_Remote::setContourColor (a, b, c);
}

// virtual
void nmg_Graphics_RenderClient::setFrictionSliderRange (float a, float b) {
  blockTimer();
  nmg_Graphics_Remote::setFrictionSliderRange (a, b);
}

// virtual
void nmg_Graphics_RenderClient::setBumpSliderRange (float a, float b) {
  blockTimer();
  nmg_Graphics_Remote::setBumpSliderRange (a, b);
}

// virtual
void nmg_Graphics_RenderClient::setBuzzSliderRange (float a, float b) {
  blockTimer();
  nmg_Graphics_Remote::setBuzzSliderRange (a, b);
}

// virtual
void nmg_Graphics_RenderClient::setContourWidth (float a) {
  blockTimer();
  nmg_Graphics_Remote::setContourWidth (a);
}

// virtual
void nmg_Graphics_RenderClient::setPatternMapName (const char * a) {
  blockTimer();
  nmg_Graphics_Remote::setPatternMapName (a);
}

// virtual
void nmg_Graphics_RenderClient::translateTextures (int a, float b, float c) {
  blockTimer();
  nmg_Graphics_Remote::translateTextures (a, b, c);
}

// virtual
void nmg_Graphics_RenderClient::scaleTextures (int a, float b, float c) {
  blockTimer();
  nmg_Graphics_Remote::scaleTextures (a, b, c);
}

// virtual
void nmg_Graphics_RenderClient::shearTextures (int a, float b, float c) {
  blockTimer();
  nmg_Graphics_Remote::shearTextures (a, b, c);
}

// virtual
void nmg_Graphics_RenderClient::rotateTextures (int a, float b) {
  blockTimer();
  nmg_Graphics_Remote::rotateTextures (a, b);
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
void nmg_Graphics_RenderClient::setSurfaceAlpha (float a) {
  blockTimer();
  nmg_Graphics_Remote::setSurfaceAlpha (a);
}

// virtual
void nmg_Graphics_RenderClient::setSpecularColor (float a) {
  blockTimer();
  nmg_Graphics_Remote::setSpecularColor (a);
}

// virtual
void nmg_Graphics_RenderClient::setTesselationStride (int a) {
  blockTimer();
  nmg_Graphics_Remote::setTesselationStride (a);
}

// virtual
void nmg_Graphics_RenderClient::setTextureMode (TextureMode a,
                                                TextureTransformMode b) {
  blockTimer();
  nmg_Graphics_Remote::setTextureMode (a, b);
}

// virtual
void nmg_Graphics_RenderClient::setTextureScale (float a) {
  blockTimer();
  nmg_Graphics_Remote::setTextureScale (a);
}

// virtual
void nmg_Graphics_RenderClient::sendGeneticTexturesData (int a, char ** b) {
  blockTimer();
  nmg_Graphics_Remote::sendGeneticTexturesData (a, b);
}


