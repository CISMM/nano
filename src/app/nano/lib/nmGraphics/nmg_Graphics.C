#include "nmg_Graphics.h"

#include <string.h> // for strlen, strtok, strcpy...

#include <vrpn_Connection.h>
#ifndef _WIN32
#include <netinet/in.h>  // ntoh/hton conversions
#endif
#include <nmb_Util.h>  // Buffer() & Unbuffer()

#define CHECK(a) if (a == -1) return -1

//static
const unsigned int nmg_Graphics::defaultPort = 4507;

nmg_Graphics::nmg_Graphics (vrpn_Connection * c, const char * id) :
   d_connection (c),
   d_myId (-1)  // TODO
{

fprintf(stderr, "In nmg_Graphics::nmg_Graphics()\n");

  if (!c) return;

  // Initialize for remote operation:

fprintf(stderr, "nmg_Graphics:  registering sender\n");

  d_myId = c->register_sender(id);

fprintf(stderr, "nmg_Graphics:  registering message types\n");

 d_resizeViewport_type =
    c->register_message_type("nmg Graphics resizeViewport");
  d_loadRulergridImage_type =
    c->register_message_type("nmg Graphics loadRulergridImage");
  d_enableChartjunk_type =
    c->register_message_type("nmg Graphics enableChartjunk");
  d_enableFilledPolygons_type =
    c->register_message_type("nmg Graphics enableFilledPolygons");
  d_enableSmoothShading_type =
    c->register_message_type("nmg Graphics enableSmoothShading");
  d_enableTrueTip_type =
    c->register_message_type("nmg Graphics enableTrueTip");
  d_setAdhesionSliderRange_type =
    c->register_message_type("nmg Graphics setAdhesionSliderRange");
  d_setAlphaColor_type =
    c->register_message_type("nmg Graphics setAlphaColor");
  d_setAlphaSliderRange_type =
    c->register_message_type("nmg Graphics setAlphaSliderRange");
  d_setBumpMapName_type =
    c->register_message_type("nmg Graphics setBumpMapName");
  d_setColorMapDirectory_type =
    c->register_message_type("nmg Graphics setColorMapDirectory");
  d_setColorMapName_type =
    c->register_message_type("nmg Graphics setColorMapName");
  d_setColorSliderRange_type =
    c->register_message_type("nmg Graphics setColorSliderRange");
  d_setTextureDirectory_type =
    c->register_message_type("nmg Graphics setTextureDirectory");
  d_setComplianceSliderRange_type =
    c->register_message_type("nmg Graphics setComplianceSliderRange");
  d_setContourColor_type =
    c->register_message_type("nmg Graphics setContourColor");
  d_setContourWidth_type =
    c->register_message_type("nmg Graphics setContourWidth");
  d_setFrictionSliderRange_type =
    c->register_message_type("nmg Graphics setFrictionSliderRange");
  d_setBumpSliderRange_type =
	c->register_message_type("nmg Graphics setBumpSliderRange");
  d_setBuzzSliderRange_type =
	c->register_message_type("nmg Graphics setBuzzSliderRange");
  d_setHandColor_type =
    c->register_message_type("nmg Graphics setHandColor");
  d_setHatchMapName_type =
    c->register_message_type("nmg Graphics setHatchMapName");
  d_setIconScale_type =
    c->register_message_type("nmg Graphics setIconScale");
  d_setCollabHandPos_type =
    c->register_message_type("nmg Graphics setCollabHandPos");
  d_setCollabMode_type =
    c->register_message_type("nmg Graphics setCollabMode");
  d_setMinColor_type =
    c->register_message_type("nmg Graphics setMinColor");
  d_setMaxColor_type =
    c->register_message_type("nmg Graphics setMaxColor");
  d_setMinColor_type =
    c->register_message_type("nmg Graphics setMinColor");
  d_setMaxColor_type =
    c->register_message_type("nmg Graphics setMaxColor");
  d_setPatternMapName_type =
    c->register_message_type("nmg Graphics setPatternMapName");
  d_enableRulergrid_type =
    c->register_message_type("nmg Graphics enableRulergrid");
  d_setRulergridAngle_type =
    c->register_message_type("nmg Graphics setRulergridAngle");
  d_setRulergridColor_type =
    c->register_message_type("nmg Graphics setRulergridColor");
  d_setRulergridOffset_type =
    c->register_message_type("nmg Graphics setRulergridOffset");
  d_setRulergridOpacity_type =
    c->register_message_type("nmg Graphics setRulergridOpacity");
  d_setRulergridScale_type =
    c->register_message_type("nmg Graphics setRulergridScale");
  d_setRulergridWidths_type =
    c->register_message_type("nmg Graphics setRulergridWidths");
  d_setSpecularity_type =
    c->register_message_type("nmg Graphics setSpecularity");
  d_setSpecularity_type =
    c->register_message_type("nmg Graphics setSpecularColor");
  d_setDiffusePercent_type = 
    c->register_message_type("nmg Graphics setDiffusePercent"); 
  d_setSurfaceAlpha_type = 
    c->register_message_type("nmg Graphics setSurfaceAlpha");
  d_setSphereScale_type =
    c->register_message_type("nmg Graphics setSphereScale");
  d_setTesselationStride_type =
    c->register_message_type("nmg Graphics setTesselationStride");
  d_setTextureMode_type =
    c->register_message_type("nmg Graphics setTextureMode");
  d_setTextureScale_type =
    c->register_message_type("nmg Graphics setTextureScale");
  d_setTrueTipScale_type =
    c->register_message_type("nmg Graphics setTrueTipScale");
  d_setUserMode_type =
    c->register_message_type("nmg Graphics setUserMode");
  d_setLightDirection_type =
    c->register_message_type("nmg Graphics setLightDirection");
  d_resetLightDirection_type =
    c->register_message_type("nmg Graphics resetLightDirection");
  d_addPolylinePoint_type =
    c->register_message_type("nmg Graphics addPolylinePoint");
  d_emptyPolyline_type =
    c->register_message_type("nmg Graphics emptyPolyline");
  d_setRubberLineStart_type =
    c->register_message_type("nmg Graphics setRubberLineStart");
  d_setRubberLineEnd_type =
    c->register_message_type("nmg Graphics setRubberLineEnd");

  d_setScanlineEndpoints_type =
    c->register_message_type("nmg Graphics setScanlineEndpoints");
  d_displayScanlinePosition_type =
    c->register_message_type("nmg Graphics displayScanlinePosition");

  d_positionAimLine_type =
    c->register_message_type("nmg Graphics positionAimLine");
  d_positionRubberCorner_type =
    c->register_message_type("nmg Graphics positionRubberCorner");
  d_positionSweepLine_type =
    c->register_message_type("nmg Graphics positionSweepLine");
  d_positionSphere_type =
    c->register_message_type("nmg Graphics positionSphere");

  // Genetic Textures Network Types:
  d_enableGeneticTextures_type =
    c->register_message_type("nmg Graphics enableGeneticTextures");
  d_sendGeneticTexturesData_type =
    c->register_message_type("nmg Graphics sendGeneticTexturesData");
  
  // Realign Textures Network Types:
  d_createRealignTextures_type = 
    c->register_message_type("nmg Graphics createRealignTextures");
  d_setRealignTexturesConversionMap_type = 
    c->register_message_type("nmg Graphics setRealignTexturesConversionMap");
  d_setRealignTextureSliderRange_type = 
    c->register_message_type("nmg Graphics setRealignTextureSliderRange");
  d_computeRealignPlane_type = 
    c->register_message_type("nmg Graphics computeRealignPlane");
  d_enableRealignTextures_type = 
    c->register_message_type("nmg Graphics enableRealignTextures");
  d_translateTextures_type = 
    c->register_message_type("nmg Graphics translateTextures");
  d_scaleTextures_type = 
    c->register_message_type("nmg Graphics scaleTextures");
  d_shearTextures_type = 
    c->register_message_type("nmg Graphics shearTextures");
  d_rotateTextures_type = 
    c->register_message_type("nmg Graphics rotateTextures");
  d_setTextureCenter_type = 
    c->register_message_type("nmg Graphics setTextureCenter");

  d_setTextureTransform_type =
    c->register_message_type("nmg Graphics setTextureTransform");
  d_enableRegistration_type =
    c->register_message_type("nmg Graphics enableRegistration");

   // For screen capture
  d_createScreenImage_type =
    c->register_message_type("nmg Graphics createScreenImage");

}

nmg_Graphics::~nmg_Graphics (void) {

}

void nmg_Graphics::mainloop (void) {

  if (d_connection)
    d_connection->mainloop();

}

char * nmg_Graphics::encode_resizeViewport (int * len, 
					   int width , int height) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = 2 * sizeof(int);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_resizeViewport:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, width);
    nmb_Util::Buffer(&mptr, &mlen, height);
  }

  return msgbuf;
}

int nmg_Graphics::decode_resizeViewport (const char * buf, 
					    int *width , int *height) {
  if (!buf || !width || !height) return -1;
  CHECK(nmb_Util::Unbuffer(&buf, width));
  CHECK(nmb_Util::Unbuffer(&buf, height));
  return 0;
}

char * nmg_Graphics::encode_enableChartjunk (int * len, int value) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = sizeof(int);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_enableChartjunk:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, value);
  }

  return msgbuf;
}

int nmg_Graphics::decode_enableChartjunk (const char * buf,
                                           int * value) {
  if (!buf || !value) return -1;
  CHECK(nmb_Util::Unbuffer(&buf, value));
  return 0;
}


char * nmg_Graphics::encode_enableFilledPolygons
                     (int * len, int value) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = sizeof(int);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_enableFilledPolygons:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, value);
  }

  return msgbuf;
}

int nmg_Graphics::decode_enableFilledPolygons (const char * buf,
                                                int * value) {
  if (!buf || !value) return -1;
  CHECK(nmb_Util::Unbuffer(&buf, value));
  return 0;
}


char * nmg_Graphics::encode_enableSmoothShading
                     (int * len, int value) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = sizeof(int);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_enableSmoothShading:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, value);
  }

  return msgbuf;
}

int nmg_Graphics::decode_enableSmoothShading (const char * buf,
                                               int * value) {
  if (!buf || !value) return -1;
  CHECK(nmb_Util::Unbuffer(&buf, value));
  return 0;
}

char * nmg_Graphics::encode_enableTrueTip
                     (int * len, int value) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = sizeof(int);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_enableTrueTip:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, value);
  }

  return msgbuf;
}

int nmg_Graphics::decode_enableTrueTip (const char * buf,
                                               int * value) {
  if (!buf || !value) return -1;
  CHECK(nmb_Util::Unbuffer(&buf, value));
  return 0;
}

char * nmg_Graphics::encode_setAdhesionSliderRange
                     (int * len, float low, float hi) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = 2 * sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_setAdhesionSliderRange:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, low);
    nmb_Util::Buffer(&mptr, &mlen, hi);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setAdhesionSliderRange
                   (const char * buf, float * low, float * hi) {
  if (!buf || !low || !hi) return -1;
  CHECK(nmb_Util::Unbuffer(&buf, low));
  CHECK(nmb_Util::Unbuffer(&buf, hi));
  return 0;
}


char * nmg_Graphics::encode_setAlphaColor
                     (int * len, float r, float g, float b) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = 3 * sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_setAlphaColor:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, r);
    nmb_Util::Buffer(&mptr, &mlen, g);
    nmb_Util::Buffer(&mptr, &mlen, b);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setAlphaColor
                   (const char * buf, float * r, float * g, float * b) {
  if (!buf || !r || !g || !b) return -1;
  CHECK(nmb_Util::Unbuffer(&buf, r));
  CHECK(nmb_Util::Unbuffer(&buf, g));
  CHECK(nmb_Util::Unbuffer(&buf, b));
  return 0;
}

char * nmg_Graphics::encode_setAlphaSliderRange
                     (int * len, float low, float hi) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = 2 * sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_setAlphaSliderRange:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, low);
    nmb_Util::Buffer(&mptr, &mlen, hi);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setAlphaSliderRange
                   (const char * buf, float * low, float * hi) {
  if (!buf || !low || !hi) return -1;
  CHECK(nmb_Util::Unbuffer(&buf, low));
  CHECK(nmb_Util::Unbuffer(&buf, hi));
  return 0;
}

char * nmg_Graphics::encode_setColorSliderRange
                     (int * len, float low, float hi) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = 2 * sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_setColorSliderRange:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, low);
    nmb_Util::Buffer(&mptr, &mlen, hi);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setColorSliderRange
                   (const char * buf, float * low, float * hi) {
  if (!buf || !low || !hi) return -1;
  CHECK(nmb_Util::Unbuffer(&buf, low));
  CHECK(nmb_Util::Unbuffer(&buf, hi));
  return 0;
}

char * nmg_Graphics::encode_setComplianceSliderRange
                     (int * len, float low, float hi) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = 2 * sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_setComplianceSliderRange:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, low);
    nmb_Util::Buffer(&mptr, &mlen, hi);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setComplianceSliderRange
                   (const char * buf, float * low, float * hi) {
  if (!buf || !low || !hi) return -1;
  CHECK(nmb_Util::Unbuffer(&buf, low));
  CHECK(nmb_Util::Unbuffer(&buf, hi));
  return 0;
}

char * nmg_Graphics::encode_setContourColor
                     (int * len, int r, int g, int b) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = 3 * sizeof(int);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_setContourColor:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, r);
    nmb_Util::Buffer(&mptr, &mlen, g);
    nmb_Util::Buffer(&mptr, &mlen, b);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setContourColor
                   (const char * buf, int * r, int * g, int * b) {
  if (!buf || !r || !g || !b) return -1;
  CHECK(nmb_Util::Unbuffer(&buf, r));
  CHECK(nmb_Util::Unbuffer(&buf, g));
  CHECK(nmb_Util::Unbuffer(&buf, b));
  return 0;
}

char * nmg_Graphics::encode_setContourWidth
                     (int * len, float width) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_setContourWidth:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, width);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setContourWidth
                   (const char * buf, float * width) {
  if (!buf || !width) return -1;
  CHECK(nmb_Util::Unbuffer(&buf, width));
  return 0;
}

char * nmg_Graphics::encode_setFrictionSliderRange
                     (int * len, float low, float hi) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = 2 * sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_setFrictionSliderRange:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, low);
    nmb_Util::Buffer(&mptr, &mlen, hi);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setFrictionSliderRange
                   (const char * buf, float * low, float * hi) {
  if (!buf || !low || !hi) return -1;
  CHECK(nmb_Util::Unbuffer(&buf, low));
  CHECK(nmb_Util::Unbuffer(&buf, hi));
  return 0;
}

char * nmg_Graphics::encode_setBumpSliderRange
                     (int * len, float low, float hi) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = 2 * sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_setBumpSliderRange:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, low);
    nmb_Util::Buffer(&mptr, &mlen, hi);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setBumpSliderRange
                   (const char * buf, float * low, float * hi) {
  if (!buf || !low || !hi) return -1;
  CHECK(nmb_Util::Unbuffer(&buf, low));
  CHECK(nmb_Util::Unbuffer(&buf, hi));
  return 0;
}

char * nmg_Graphics::encode_setBuzzSliderRange
                     (int * len, float low, float hi) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = 2 * sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_setBuzzSliderRange:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, low);
    nmb_Util::Buffer(&mptr, &mlen, hi);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setBuzzSliderRange
                   (const char * buf, float * low, float * hi) {
  if (!buf || !low || !hi) return -1;
  CHECK(nmb_Util::Unbuffer(&buf, low));
  CHECK(nmb_Util::Unbuffer(&buf, hi));
  return 0;
}

char * nmg_Graphics::encode_setHandColor
                     (int * len, int color) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = sizeof(int);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_setHandColor:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, color);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setHandColor
                   (const char * buf, int * color) {
  if (!buf || !color) return -1;
  CHECK(nmb_Util::Unbuffer(&buf, color));
  return 0;
}

char * nmg_Graphics::encode_setIconScale
                     (int * len, float scale) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_setIconScale:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, scale);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setIconScale
                   (const char * buf, float * scale) {
  if (!buf || !scale) return -1;
  CHECK(nmb_Util::Unbuffer(&buf, scale));
  return 0;
}

//set the position and orientation of the icon following the collaborator's
//hand position
char * nmg_Graphics::encode_setCollabHandPos(int * len, double pos[],
					     double quat[])
{
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = 7 * sizeof(double);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_setCollabHandPos:  "
	    "Out of memory.\n");
    *len = 0;
  }
  else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, pos[0]);
    nmb_Util::Buffer(&mptr, &mlen, pos[1]);
    nmb_Util::Buffer(&mptr, &mlen, pos[2]);
    nmb_Util::Buffer(&mptr, &mlen, quat[0]);
    nmb_Util::Buffer(&mptr, &mlen, quat[1]);
    nmb_Util::Buffer(&mptr, &mlen, quat[2]);
    nmb_Util::Buffer(&mptr, &mlen, quat[3]);
  }
  return msgbuf;
}

int nmg_Graphics::decode_setCollabHandPos(const char * buf, double pos[3],
					  double quat[4])
{
  if (!buf || !pos) return -1;
  CHECK(nmb_Util::Unbuffer(&buf, &pos[0]));
  CHECK(nmb_Util::Unbuffer(&buf, &pos[1]));
  CHECK(nmb_Util::Unbuffer(&buf, &pos[2]));
  CHECK(nmb_Util::Unbuffer(&buf, &quat[0]));
  CHECK(nmb_Util::Unbuffer(&buf, &quat[1]));
  CHECK(nmb_Util::Unbuffer(&buf, &quat[2]));
  CHECK(nmb_Util::Unbuffer(&buf, &quat[3]));
  return 0;
}

//set the mode of the icon following the collaborator's hand
char * nmg_Graphics::encode_setCollabMode(int * len, vrpn_int32 mode)
{
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_setCollabMode:  Out of memory.\n");
    *len = 0;
  }
  else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, mode);
  }
  return msgbuf;
}

int nmg_Graphics::decode_setCollabMode(const char * buf, vrpn_int32 *mode)
{
  if (!buf || !mode) return -1;
  CHECK(nmb_Util::Unbuffer(&buf, mode));
  return 0;
}

char * nmg_Graphics::encode_setMinColor
                     (int * len, const double c [3]) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = 3 * sizeof(double);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_setMinColor:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, c[0]);
    nmb_Util::Buffer(&mptr, &mlen, c[1]);
    nmb_Util::Buffer(&mptr, &mlen, c[2]);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setMinColor
                   (const char * buf, double c [3]) {
  if (!buf || !c) return -1;
  CHECK(nmb_Util::Unbuffer(&buf, &c[0]));
  CHECK(nmb_Util::Unbuffer(&buf, &c[1]));
  CHECK(nmb_Util::Unbuffer(&buf, &c[2]));

fprintf(stderr, "Server got min color (%.4f, %.4f, %.4f)\n", c[0], c[1], c[2]);
  return 0;
}


char * nmg_Graphics::encode_setMaxColor
                     (int * len, const double c [3]) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = 3 * sizeof(double);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_setMaxColor:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, c[0]);
    nmb_Util::Buffer(&mptr, &mlen, c[1]);
    nmb_Util::Buffer(&mptr, &mlen, c[2]);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setMaxColor
                   (const char * buf, double c [3]) {
  if (!buf || !c) return -1;
  CHECK(nmb_Util::Unbuffer(&buf, &c[0]));
  CHECK(nmb_Util::Unbuffer(&buf, &c[1]));
  CHECK(nmb_Util::Unbuffer(&buf, &c[2]));

fprintf(stderr, "Server got max color (%.4f, %.4f, %.4f)\n", c[0], c[1], c[2]);
  return 0;
}

  // Translate integer arguments in range [0..255]
  // to doubles in range [0..1]

char * nmg_Graphics::encode_setMinColor
                     (int * len, const int c [3]) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = 3 * sizeof(double);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_setMinColor:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, c[0] / 255.0);
    nmb_Util::Buffer(&mptr, &mlen, c[1] / 255.0);
    nmb_Util::Buffer(&mptr, &mlen, c[2] / 255.0);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setMinColor
                   (const char * buf, int c [3]) {
  double d [3];

  if (!buf || !c) return -1;
  CHECK(nmb_Util::Unbuffer(&buf, &d[0]));
  CHECK(nmb_Util::Unbuffer(&buf, &d[1]));
  CHECK(nmb_Util::Unbuffer(&buf, &d[2]));

  c[0] = (int) (255 * d[0]);
  c[1] = (int) (255 * d[1]);
  c[2] = (int) (255 * d[2]);

fprintf(stderr, "Server got min color (%d, %d, %d)\n", c[0], c[1], c[2]);
  return 0;
}

char * nmg_Graphics::encode_setMaxColor
                     (int * len, const int c [3]) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = 3 * sizeof(double);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_setMaxColor:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, c[0] / 255.0);
    nmb_Util::Buffer(&mptr, &mlen, c[1] / 255.0);
    nmb_Util::Buffer(&mptr, &mlen, c[2] / 255.0);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setMaxColor
                   (const char * buf, int c [3]) {
  double d [3];

  if (!buf || !c) return -1;
  CHECK(nmb_Util::Unbuffer(&buf, &d[0]));
  CHECK(nmb_Util::Unbuffer(&buf, &d[1]));
  CHECK(nmb_Util::Unbuffer(&buf, &d[2]));

  c[0] = (int) (255 * d[0]);
  c[1] = (int) (255 * d[1]);
  c[2] = (int) (255 * d[2]);

fprintf(stderr, "Server got max color (%d, %d, %d)\n", c[0], c[1], c[2]);
  return 0;
}

char * nmg_Graphics::encode_enableRulergrid
                     (int * len, int value) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = sizeof(int);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_enableRulergrid:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, value);
  }

  return msgbuf;
}

int nmg_Graphics::decode_enableRulergrid
                   (const char * buf, int * value) {
  if (!buf || !value) return -1;
  CHECK(nmb_Util::Unbuffer(&buf, value));
  return 0;
}

char * nmg_Graphics::encode_setRulergridAngle
                     (int * len, float angle) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_setRulergridAngle:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, angle);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setRulergridAngle
                   (const char * buf, float * angle) {
  if (!buf || !angle) return -1;
  CHECK(nmb_Util::Unbuffer(&buf, angle));
  return 0;
}

char * nmg_Graphics::encode_setRulergridColor
                     (int * len, int r, int g, int b) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = 3 * sizeof(int);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_setRulergridColor:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, r);
    nmb_Util::Buffer(&mptr, &mlen, g);
    nmb_Util::Buffer(&mptr, &mlen, b);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setRulergridColor
                   (const char * buf, int * r, int * g, int * b) {
  if (!buf || !r || !g || !b) return -1;
  CHECK(nmb_Util::Unbuffer(&buf, r));
  CHECK(nmb_Util::Unbuffer(&buf, g));
  CHECK(nmb_Util::Unbuffer(&buf, b));
  return 0;
}

char * nmg_Graphics::encode_setRulergridOffset
                     (int * len, float x, float y) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = 2 * sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_setRulergridOffset:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, x);
    nmb_Util::Buffer(&mptr, &mlen, y);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setRulergridOffset
                   (const char * buf, float * x, float * y) {
  if (!buf || !x || !y) return -1;
  CHECK(nmb_Util::Unbuffer(&buf, x));
  CHECK(nmb_Util::Unbuffer(&buf, y));
  return 0;
}

char * nmg_Graphics::encode_setRulergridOpacity
                     (int * len, float alpha) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_setRulergridOpacity:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, alpha);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setRulergridOpacity
                   (const char * buf, float * alpha) {
  if (!buf || !alpha) return -1;
  CHECK(nmb_Util::Unbuffer(&buf, alpha));
  return 0;
}

char * nmg_Graphics::encode_setRulergridScale
                     (int * len, float scale) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_setRulergridScale:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, scale);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setRulergridScale
                   (const char * buf, float * scale) {
  if (!buf || !scale) return -1;
  CHECK(nmb_Util::Unbuffer(&buf, scale));
  return 0;
}

char * nmg_Graphics::encode_setRulergridWidths
                     (int * len, float x, float y) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = 2 * sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_setRulergridWidths:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, x);
    nmb_Util::Buffer(&mptr, &mlen, y);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setRulergridWidths
                   (const char * buf, float * x, float * y) {
  if (!buf || !x || !y) return -1;
  CHECK(nmb_Util::Unbuffer(&buf, x));
  CHECK(nmb_Util::Unbuffer(&buf, y));
  return 0;
}

char * nmg_Graphics::encode_setSpecularity
                     (int * len, int shiny) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = sizeof(int);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_setSpecularity:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, shiny);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setSpecularity
                   (const char * buf, int * shiny) {
  if (!buf || !shiny) return -1;
  CHECK(nmb_Util::Unbuffer(&buf, shiny));
  return 0;
}

char * nmg_Graphics::encode_setDiffusePercent
		     (int *len, float diffuse) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_setDiffusePercent:  "
		    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, diffuse);
  }

  return msgbuf;
}



int nmg_Graphics::decode_setDiffusePercent
		   (const char *buf, float * diffuse) {
  if (!buf || !diffuse) return -1;
  CHECK(nmb_Util::Unbuffer(&buf, diffuse));
  return 0;
}

char * nmg_Graphics::encode_setSurfaceAlpha
		     (int *len, float surface_alpha) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_setSurfaceAlpha:  "
		    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, surface_alpha);
  }

  return msgbuf;
}



int nmg_Graphics::decode_setSurfaceAlpha
		   (const char *buf, float * surface_alpha) {
  if (!buf || !surface_alpha) return -1;
  CHECK(nmb_Util::Unbuffer(&buf, surface_alpha));
  return 0;
}


char * nmg_Graphics::encode_setSpecularColor
                     (int * len, float specular_color) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_setSpecularColor:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, specular_color);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setSpecularColor
                   (const char * buf, float * specular_color) {
  if (!buf || !specular_color) return -1;
  CHECK(nmb_Util::Unbuffer(&buf, specular_color));
  return 0;
}

char * nmg_Graphics::encode_setSphereScale
                     (int * len, float scale) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_setSphereScale:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, scale);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setSphereScale
                   (const char * buf, float * scale) {
  if (!buf || !scale) return -1;
  CHECK(nmb_Util::Unbuffer(&buf, scale));
  return 0;
}

char * nmg_Graphics::encode_setTesselationStride
                     (int * len, int stride) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = sizeof(int);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_setTesselationStride:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, stride);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setTesselationStride
                   (const char * buf, int * stride) {
  if (!buf || !stride) return -1;
  CHECK(nmb_Util::Unbuffer(&buf, stride));
  return 0;
}

char * nmg_Graphics::encode_setTextureMode
                     (int * len, TextureMode m) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = sizeof(int);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_setTextureMode:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    switch (m) {
      case CONTOUR:  
        nmb_Util::Buffer(&mptr, &mlen, 1); break;
      case RULERGRID:  
        nmb_Util::Buffer(&mptr, &mlen, 2); break;
      case ALPHA:  
        nmb_Util::Buffer(&mptr, &mlen, 3); break;

      default:
        fprintf(stderr, "nmg_Graphics::encode_setTextureMode:  "
                        "Got illegal texture mode %d.  "
                        "Sending NO_TEXTURES instead.\n", m);

	// fall through

      case NO_TEXTURES:
        nmb_Util::Buffer(&mptr, &mlen, 0); break;
    }
  }

  return msgbuf;
}

int nmg_Graphics::decode_setTextureMode
                   (const char * buf, TextureMode * m) {
  int i;

  if (!buf || !m) return -1;
  CHECK(nmb_Util::Unbuffer(&buf, &i));

  switch (i) {
    case 1:  *m = CONTOUR; break;
    case 2:  *m = RULERGRID; break;
    case 3:  *m = ALPHA; break;

    default:
      fprintf(stderr, "nmg_Graphics::decode_setTextureMode:  "
                      "Got illegal texture mode %d.  "
                      "Sending NO_TEXTURES instead.\n", i);


      // fall through

    case 0:  *m = NO_TEXTURES; break;
  }
  return 0;
}

char * nmg_Graphics::encode_setTextureScale
                     (int * len, float scale) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_setTextureScale:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, scale);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setTextureScale
                   (const char * buf, float * scale) {
  if (!buf || !scale) return -1;
  CHECK(nmb_Util::Unbuffer(&buf, scale));
  return 0;
}

char * nmg_Graphics::encode_setTrueTipScale
                     (int * len, float scale) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_setTrueTipScale:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, scale);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setTrueTipScale
                   (const char * buf, float * scale) {
  if (!buf || !scale) return -1;
  CHECK(nmb_Util::Unbuffer(&buf, scale));
  return 0;
}

char * nmg_Graphics::encode_setUserMode
                     (int * len, int oldMode, int newMode, int style) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = 3 * sizeof(int);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_setUserMode:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, oldMode);
    nmb_Util::Buffer(&mptr, &mlen, newMode);
    nmb_Util::Buffer(&mptr, &mlen, style);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setUserMode
                   (const char * buf, int * oldMode, int * newMode,
                             int * style) {
  if (!buf || !oldMode || !newMode || !style) return -1;
  CHECK(nmb_Util::Unbuffer(&buf, oldMode));
  CHECK(nmb_Util::Unbuffer(&buf, newMode));
  CHECK(nmb_Util::Unbuffer(&buf, style));
  return 0;
}

char * nmg_Graphics::encode_setLightDirection
                     (int * len, const q_vec_type & v) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = 3 * sizeof(double);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_setLightDirection:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, v[0]);
    nmb_Util::Buffer(&mptr, &mlen, v[1]);
    nmb_Util::Buffer(&mptr, &mlen, v[2]);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setLightDirection
                   (const char * buf, q_vec_type & v) {
  if (!buf) return -1;
  CHECK(nmb_Util::Unbuffer(&buf, &v[0]));
  CHECK(nmb_Util::Unbuffer(&buf, &v[1]));
  CHECK(nmb_Util::Unbuffer(&buf, &v[2]));
  return 0;
}

char * nmg_Graphics::encode_addPolylinePoint
                     (int * len, const float point [2][3]) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = 6 * sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_addPolylinePoint:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, point[0][0]);
    nmb_Util::Buffer(&mptr, &mlen, point[0][1]);
    nmb_Util::Buffer(&mptr, &mlen, point[0][2]);
    nmb_Util::Buffer(&mptr, &mlen, point[1][0]);
    nmb_Util::Buffer(&mptr, &mlen, point[1][1]);
    nmb_Util::Buffer(&mptr, &mlen, point[1][2]);
  }

  return msgbuf;
}

int nmg_Graphics::decode_addPolylinePoint
                   (const char * buf, float point [2][3]) {
  CHECK(nmb_Util::Unbuffer(&buf, &point[0][0]));
  CHECK(nmb_Util::Unbuffer(&buf, &point[0][1]));
  CHECK(nmb_Util::Unbuffer(&buf, &point[0][2]));
  CHECK(nmb_Util::Unbuffer(&buf, &point[1][0]));
  CHECK(nmb_Util::Unbuffer(&buf, &point[1][1]));
  CHECK(nmb_Util::Unbuffer(&buf, &point[1][2]));
  return 0;
}

char * nmg_Graphics::encode_setRubberLineStart (int * len, const float p [2]) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = 2 * sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_setRubberLineStart:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, p[0]);
    nmb_Util::Buffer(&mptr, &mlen, p[1]);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setRubberLineStart (const char * buf, float p [2]) {
  CHECK(nmb_Util::Unbuffer(&buf, &p[0]));
  CHECK(nmb_Util::Unbuffer(&buf, &p[1]));
  return 0;
}

char * nmg_Graphics::encode_setRubberLineEnd (int * len, const float p [2]) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = 2 * sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_setRubberLineEnd:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, p[0]);
    nmb_Util::Buffer(&mptr, &mlen, p[1]);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setRubberLineEnd (const char * buf, float p [2]) {
  CHECK(nmb_Util::Unbuffer(&buf, &p[0]));
  CHECK(nmb_Util::Unbuffer(&buf, &p[1]));
  return 0;
}

char * nmg_Graphics::encode_setScanlineEndpoints (int * len, 
	const float p [6]) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = 6 * sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_setScanlineEndpoints:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, p[0]);
    nmb_Util::Buffer(&mptr, &mlen, p[1]);
    nmb_Util::Buffer(&mptr, &mlen, p[2]);
    nmb_Util::Buffer(&mptr, &mlen, p[3]);
    nmb_Util::Buffer(&mptr, &mlen, p[4]);
    nmb_Util::Buffer(&mptr, &mlen, p[5]);

  }

  return msgbuf;
}

int nmg_Graphics::decode_setScanlineEndpoints (const char * buf, float p [6]) {
  CHECK(nmb_Util::Unbuffer(&buf, &p[0]));
  CHECK(nmb_Util::Unbuffer(&buf, &p[1]));
  CHECK(nmb_Util::Unbuffer(&buf, &p[2]));
  CHECK(nmb_Util::Unbuffer(&buf, &p[3]));
  CHECK(nmb_Util::Unbuffer(&buf, &p[4]));
  CHECK(nmb_Util::Unbuffer(&buf, &p[5]));
  return 0;
}

char * nmg_Graphics::encode_displayScanlinePosition (int * len,
        const int enable) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = 1 * sizeof(int);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_displayScanlinePosition:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, enable);

  }

  return msgbuf;
}

int nmg_Graphics::decode_displayScanlinePosition(const char * buf, 
							int *enable){
  CHECK(nmb_Util::Unbuffer(&buf, enable));
  return 0;
}

char * nmg_Graphics::encode_positionAimLine
                     (int * len, const PointType lo,
                                   const PointType hi) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = 6 * sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_positionAimLine:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, lo[0]);
    nmb_Util::Buffer(&mptr, &mlen, lo[1]);
    nmb_Util::Buffer(&mptr, &mlen, lo[2]);
    nmb_Util::Buffer(&mptr, &mlen, hi[0]);
    nmb_Util::Buffer(&mptr, &mlen, hi[1]);
    nmb_Util::Buffer(&mptr, &mlen, hi[2]);
  }

  return msgbuf;
}

int nmg_Graphics::decode_positionAimLine
                   (const char * buf, PointType lo, PointType hi) {
  if (!buf || !lo || !hi) return -1;
  CHECK(nmb_Util::Unbuffer(&buf, &lo[0]));
  CHECK(nmb_Util::Unbuffer(&buf, &lo[1]));
  CHECK(nmb_Util::Unbuffer(&buf, &lo[2]));
  CHECK(nmb_Util::Unbuffer(&buf, &hi[0]));
  CHECK(nmb_Util::Unbuffer(&buf, &hi[1]));
  CHECK(nmb_Util::Unbuffer(&buf, &hi[2]));
  return 0;
}

char * nmg_Graphics::encode_positionRubberCorner
                     (int * len, float x0, float y0, float x1, float y1) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = 4 * sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_positionRubberCorner:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, x0);
    nmb_Util::Buffer(&mptr, &mlen, y0);
    nmb_Util::Buffer(&mptr, &mlen, x1);
    nmb_Util::Buffer(&mptr, &mlen, y1);
  }

  return msgbuf;
}

int nmg_Graphics::decode_positionRubberCorner
                   (const char * buf, float * x0, float * y0, float * x1,
                             float * y1) {
  if (!buf || !x0 || !y0 || !x1 || !y1) return -1;
  CHECK(nmb_Util::Unbuffer(&buf, x0));
  CHECK(nmb_Util::Unbuffer(&buf, y0));
  CHECK(nmb_Util::Unbuffer(&buf, x1));
  CHECK(nmb_Util::Unbuffer(&buf, y1));
  return 0;
}

char * nmg_Graphics::encode_positionSweepLine
                     (int * len, const PointType lo,
                                   const PointType hi) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = 6 * sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_positionSweepLine:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, lo[0]);
    nmb_Util::Buffer(&mptr, &mlen, lo[1]);
    nmb_Util::Buffer(&mptr, &mlen, lo[2]);
    nmb_Util::Buffer(&mptr, &mlen, hi[0]);
    nmb_Util::Buffer(&mptr, &mlen, hi[1]);
    nmb_Util::Buffer(&mptr, &mlen, hi[2]);
  }

  return msgbuf;
}

int nmg_Graphics::decode_positionSweepLine
                   (const char * buf, PointType lo, PointType hi) {
  if (!buf || !lo || !hi) return -1;
  CHECK(nmb_Util::Unbuffer(&buf, &lo[0]));
  CHECK(nmb_Util::Unbuffer(&buf, &lo[1]));
  CHECK(nmb_Util::Unbuffer(&buf, &lo[2]));
  CHECK(nmb_Util::Unbuffer(&buf, &hi[0]));
  CHECK(nmb_Util::Unbuffer(&buf, &hi[1]));
  CHECK(nmb_Util::Unbuffer(&buf, &hi[2]));
  return 0;
}

char * nmg_Graphics::encode_positionSphere
                     (int * len, float x, float y, float z) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = 3 * sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_positionSphere:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, x);
    nmb_Util::Buffer(&mptr, &mlen, y);
    nmb_Util::Buffer(&mptr, &mlen, z);
  }

  return msgbuf;
}

int nmg_Graphics::decode_positionSphere
                   (const char * buf, float * x, float * y, float * z) {
  if (!buf || !x || !y || !z) return -1;
  CHECK(nmb_Util::Unbuffer(&buf, x));
  CHECK(nmb_Util::Unbuffer(&buf, y));
  CHECK(nmb_Util::Unbuffer(&buf, z));
  return 0;
}

//
// Genetic Texture Network Transmission Code:
//

// Encodes the state of the Gentic Texture toggle button, value,
// into a buffer for network transmision... 
char * nmg_Graphics::encode_enableGeneticTextures (int * len, int value) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = sizeof(int);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_enableGeneticTextures:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, value);
  }

  return msgbuf;
}

// Decodes the state of the Gentic Texture toggle button, value,
// from buf into * value. 
int nmg_Graphics::decode_enableGeneticTextures (const char * buf, int *value) {
  if (!buf || !value) return -1;
  CHECK(nmb_Util::Unbuffer(&buf, value));
  return 0;
}

// Encodes the 2D array vl (for variable_list) into a
// 1D array for network transmission. The length of the
// new array is returned in the argument len.
// the array itself is the return value of the function.
char * nmg_Graphics::encode_sendGeneticTexturesData (int * len, int num,
						     char **vl) {

  char *msgbuf = NULL;

  *len = 0;
  int i;
  for ( i = 0; i < num; i++ )
    *len += strlen( vl[i] ) + 1;
  msgbuf = new char[*len];

  *len = 0;
  for ( i = 0; i < num; i++ ) {
    sprintf( msgbuf + *len, "%s ", vl[i] );
    *len += strlen( vl[i] ) + 1;
  }
  return msgbuf;
}

// Horribly ugly code. The idea is that the list of variable names is
// sent over as a string of names with spaces between.
// The first loop counts the number of names, so the array vl
// can be allocated, the second loop copies into vl. Ugh.
int nmg_Graphics::decode_sendGeneticTexturesData (const char *buf, int *num,
						  char ***vl) {

  *num = 0;
  char *save_buf = new char [strlen( buf ) + 1 ];
  strcpy( save_buf, buf );
  char *save_buf1 = new char [strlen( buf ) + 1 ];
  strcpy( save_buf1, buf );
  char *a;
  for ( a = strtok( save_buf1, " " ); a; a = strtok( NULL, " " ) ) {
    *num = *num + 1;
  }

  *vl = new char*[ *num ];
  *num = 0;
  for ( a = strtok( save_buf, " " ); a; a = strtok( NULL, " " ) ) {
    (*vl)[ *num ] = new char[ strlen( a ) + 1];
    strcpy( (*vl)[*num], a );
    *num = *num + 1;
  }

  delete [] save_buf;
  delete [] save_buf1;
  return 0;
}

//
// End Genetic Texture Network Transmission Code.
//

//
// Realign Texture Network Transmission Code:
//
// The following encode routines are used by the Realign Textures
// procedures. They should all be replaced with functions like
// encode_float, encode_char_char, encode_int, ...
// as none do anything unexpected.


// Encodes two floats for network transmission
char *nmg_Graphics::encode_setRealignTextureSliderRange ( int *len,
							  float low,
							  float hi ) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = 2 * sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_setRealignTextureSliderRange:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, low);
    nmb_Util::Buffer(&mptr, &mlen, hi);
  }

  return msgbuf;
}

// Decodes two floats after network transmission
int nmg_Graphics::decode_setRealignTextureSliderRange ( const char *buf,
							float *low,float *hi) {
  if (!buf || !low || !hi) return -1;
  CHECK(nmb_Util::Unbuffer(&buf, low));
  CHECK(nmb_Util::Unbuffer(&buf, hi));
  return 0;
}


// Encodes two char *'s for network transmission
char * nmg_Graphics::encode_two_char_arrays (int *len, const char * name,
                                             const char * newname) {
  *len = strlen( name ) + strlen(newname) + 2;
  char * msgbuf = new char [ *len ];
  sprintf( msgbuf, "%s %s", name, newname );
  return msgbuf;
}

// Decodes two char *'s after network transmission
int nmg_Graphics::decode_two_char_arrays ( const char *buf,
					   char **name,
					   char **newname) {
  if (!buf || !(*name) || !(*newname)) return -1;
  sscanf( buf, "%s %s", *name, *newname );
  return 0;

}

// Encodes one int for network transmission
char *nmg_Graphics::encode_enableRealignTextures ( int *len, int on) {

  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = sizeof(int);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_enableRealignTextures_type_type:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, on);
  }

  return msgbuf;
}

// Decodes one int after network transmission
int nmg_Graphics::decode_enableRealignTextures ( const char *buf, int *on) {
  if (!buf || !on) return -1;
  CHECK(nmb_Util::Unbuffer(&buf, on));
  return 0;
}


// Encodes two floats for network transmission
char *nmg_Graphics::encode_dx_dy ( int *len, float dx, float dy ) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = 2 * sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_dx_dy: Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, dx);
    nmb_Util::Buffer(&mptr, &mlen, dy);
  }

  return msgbuf;
}

// Decodes two floats after network transmission
int nmg_Graphics::decode_dx_dy ( const char *buf, float *dx, float *dy ) {
  if (!buf || !dx || !dy) return -1;
  CHECK(nmb_Util::Unbuffer(&buf, dx));
  CHECK(nmb_Util::Unbuffer(&buf, dy));
  return 0;
}

// Encodes one float for network transmission
char *nmg_Graphics::encode_rotateTextures ( int *len, float theta ) { 
  char * msgbuf = NULL;
  char * mptr;
  int mlen;
  
  if (!len) return NULL;
  
  *len = sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_dx_dy: Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, theta);
  }
  
  return msgbuf;
}

// Decodes one float after network transmission
int nmg_Graphics::decode_rotateTextures ( const char *buf, float * theta) {
  if (!buf || !theta) return -1;
  CHECK(nmb_Util::Unbuffer(&buf, theta));
  return 0;
}

// End Realign Texture Network Transmission Code.

// Encodes one int for network transmission
char *nmg_Graphics::encode_enableRegistration ( int *len, int on) {

  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = sizeof(int);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_enableRegistration_type:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, on);
  }

  return msgbuf;
}

// Decodes one int after network transmission
int nmg_Graphics::decode_enableRegistration ( const char *buf, int *on) {
  if (!buf || !on) return -1;
  CHECK(nmb_Util::Unbuffer(&buf, on));
  return 0;
}

char *nmg_Graphics::encode_textureTransform(int *len, double *xform)
{
  char *msgbuf = NULL;
  char * mptr;
  int mlen;
  int i;

  if (!len) return NULL;

  *len = 16*sizeof(double);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_textureTransform: Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    for (i = 0; i < 16; i++)
    	nmb_Util::Buffer(&mptr, &mlen, xform[i]);
  }

  return msgbuf;
}

int nmg_Graphics::decode_textureTransform(const char *buf, double *xform)
{
  if (!buf || !xform) return -1;
  int i;
  for (i = 0; i < 16; i++)
      CHECK(nmb_Util::Unbuffer(&buf, &(xform[i])));
  return 0;
}

// Start screen capture network code

char *nmg_Graphics::encode_createScreenImage
(
   int             *len,
   const char      *filename,
   const ImageType  type
)
{
   fprintf(stderr,"encode_createScreenImage\n");
   char *msgbuf = NULL;
   char *mptr;
   int mlen;

   if (!len) return NULL;

   *len = 512 + sizeof(int);
   msgbuf = new char[*len];
   if (!msgbuf)
   {
      fprintf(stderr,"nmg_Graphics::encode_createScreenImage: Out of memory!\n");
      *len = 0;
   }
   else
   {
      mptr = msgbuf;
      mlen = *len;
      nmb_Util::Buffer(&mptr, &mlen, filename, 512);
      nmb_Util::Buffer(&mptr, &mlen, (int)(type));
   }

   return msgbuf;
}

int nmg_Graphics::decode_createScreenImage
(
   const char  *buf,
   char       **filename,
   ImageType   *type
)
{
   fprintf(stderr,"decode_createScreenImage\n");
   const char *bptr = buf;
   vrpn_int32 temp;
   if (!buf || !(*filename) || !type) return -1;
   CHECK(nmb_Util::Unbuffer(&bptr, *filename, 512));
   fprintf(stderr, "Filename: %s\n", *filename);
   CHECK(nmb_Util::Unbuffer(&bptr, &temp));
   fprintf(stderr, "Type: %d\n", temp);
   *type = (ImageType)(temp);
   return 0;
}

// End screen capture network code
