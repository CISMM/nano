/*===3rdtech===
  Copyright (c) 2000 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#include "nmg_Graphics.h"

#include <string.h> // for strlen, strtok, strcpy...

#include <vrpn_Connection.h>
#ifndef _WIN32
#include <netinet/in.h>  // ntoh/hton conversions
#endif

#include <assert.h>

#define CHECK(a) if (a == -1) return -1

nmg_Graphics::nmg_Graphics (vrpn_Connection * c, const char * id) :
   d_textureMode (NO_TEXTURES),
   d_connection (c),
   d_myId (-1)  // TODO
{

//fprintf(stderr, "In nmg_Graphics::nmg_Graphics()\n");

  if (!c) return;

  // Initialize for remote operation:

//fprintf(stderr, "nmg_Graphics:  registering sender\n");

  d_myId = c->register_sender(id);

//fprintf(stderr, "nmg_Graphics:  registering message types\n");

  d_resizeViewport_type =
    c->register_message_type("nmg Graphics resizeViewport");
  d_loadRulergridImage_type =
    c->register_message_type("nmg Graphics loadRulergridImage");
  d_loadVizImage_type =
    c->register_message_type("nmg Graphics loadVizImage");
  d_causeGridRedraw_type =
    c->register_message_type("nmg Graphics causeGridRedraw");
  d_causeGridRebuild_type =
    c->register_message_type("nmg Graphics causeGridRebuild");
  d_enableChartjunk_type =
    c->register_message_type("nmg Graphics enableChartjunk");
  d_enableFilledPolygons_type =
    c->register_message_type("nmg Graphics enableFilledPolygons");
  d_enableSmoothShading_type =
    c->register_message_type("nmg Graphics enableSmoothShading");
  //d_enableUber_type =
    //c->register_message_type("nmg Graphics enableUber");
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
  d_setColorMinMax_type =
    c->register_message_type("nmg Graphics setColorMinMax");
  d_setDataColorMinMax_type =
    c->register_message_type("nmg Graphics setDataColorMinMax");
  d_setOpacitySliderRange_type =
    c->register_message_type("nmg Graphics setOpacitySliderRange");
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
  d_enableCollabHand_type =
    c->register_message_type("nmg Graphics enableCollabHand");
  d_setCollabHandPos_type =
    c->register_message_type("nmg Graphics setCollabHandPos");
  d_setCollabMode_type =
    c->register_message_type("nmg Graphics setCollabMode");
  d_setAlphaPlaneName_type =
    c->register_message_type("nmg Graphics setAlphaPlaneName");
  d_setColorPlaneName_type =
    c->register_message_type("nmg Graphics setColorPlaneName");
  d_setContourPlaneName_type =
    c->register_message_type("nmg Graphics setContourPlaneName");
  d_setOpacityPlaneName_type =
    c->register_message_type("nmg Graphics setOpacityPlaneName");
  d_setHeightPlaneName_type =
    c->register_message_type("nmg Graphics setHeightPlaneName");
  d_setMaskPlaneName_type =
    c->register_message_type("nmg Graphics setMaskPlaneName");
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
  d_setNullDataAlphaToggle_type =
    c->register_message_type("nmg Graphics setNullDataAlphaToggle");
  d_setRulergridScale_type =
    c->register_message_type("nmg Graphics setRulergridScale");
  d_setRulergridWidths_type =
    c->register_message_type("nmg Graphics setRulergridWidths");
  d_setSpecularity_type =
    c->register_message_type("nmg Graphics setSpecularity");
  d_setSpecularColor_type =
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
  d_addPolySweepPoints_type =
    c->register_message_type("nmg Graphics addPolySweepPoints");
  d_emptyPolyline_type =
    c->register_message_type("nmg Graphics emptyPolyline");
  d_setRubberLineStart_type =
    c->register_message_type("nmg Graphics setRubberLineStart");
  d_setRubberLineEnd_type =
    c->register_message_type("nmg Graphics setRubberLineEnd");
  d_setRubberSweepLineStart_type =
    c->register_message_type("nmg Graphics setRubberSweepLineStart");
  d_setRubberSweepLineEnd_type =
    c->register_message_type("nmg Graphics setRubberSweepLineEnd");

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

  d_setViewTransform_type =
    c->register_message_type("nmg Graphics setViewTransform");

   // For screen capture
  d_createScreenImage_type =
    c->register_message_type("nmg Graphics createScreenImage");

  d_updateTexture_type =
    c->register_message_type("nmg Graphics updateTexture");

  //For surface based approach
  d_setRegionMaskHeight_type =
      c->register_message_type("nmg Graphics setRegionMaskHeight");
  d_setRegionControlPlaneName_type =
    c->register_message_type("nmg Graphics setRegionControlPlaneName");
  d_createRegion_type =
    c->register_message_type("nmg Graphics createRegion");
  d_destroyRegion_type =
    c->register_message_type("nmg Graphics destroyRegion");
  d_associateAlpha_type =
    c->register_message_type("nmg Graphics associateAlpha");
  d_associateFilledPolygons_type =
    c->register_message_type("nmg Graphics associateFilledPolygons");
  d_associateStride_type =
    c->register_message_type("nmg Graphics associateStride");
  d_associateTextureDisplayed_type =
    c->register_message_type("nmg Graphics associateTextureDisplayed");
  d_associateTextureMode_type =
    c->register_message_type("nmg Graphics associateTextureMode");
  d_associateTextureTransformMode_type =
    c->register_message_type("nmg Graphics associateTextureTransformMode");
  //For visualization
  d_setViztexScale_type =
    c->register_message_type("nmg Graphics setViztexScale");
}

nmg_Graphics::~nmg_Graphics (void) {

}

void nmg_Graphics::mainloop (void) {

  if (d_connection)
    d_connection->mainloop();

}

nmg_Graphics::TextureMode nmg_Graphics::getTextureMode (void) const {
  return d_textureMode;
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
    vrpn_buffer(&mptr, &mlen, width);
    vrpn_buffer(&mptr, &mlen, height);
  }

  return msgbuf;
}

int nmg_Graphics::decode_resizeViewport (const char * buf, 
					    int *width , int *height) {
  if (!buf || !width || !height) return -1;
  CHECK(vrpn_unbuffer(&buf, width));
  CHECK(vrpn_unbuffer(&buf, height));
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
    vrpn_buffer(&mptr, &mlen, value);
  }

  return msgbuf;
}

int nmg_Graphics::decode_enableChartjunk (const char * buf,
                                           int * value) {
  if (!buf || !value) return -1;
  CHECK(vrpn_unbuffer(&buf, value));
  return 0;
}


char * nmg_Graphics::encode_enableFilledPolygons
                     (int * len, int value, int region) {
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
    vrpn_buffer(&mptr, &mlen, value);
    vrpn_buffer(&mptr, &mlen, region);
  }

  return msgbuf;
}

int nmg_Graphics::decode_enableFilledPolygons (const char * buf,
                                                int * value, int * region) {
  if (!buf || !value) return -1;
  CHECK(vrpn_unbuffer(&buf, value));
  CHECK(vrpn_unbuffer(&buf, region));
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
    vrpn_buffer(&mptr, &mlen, value);
  }

  return msgbuf;
}

int nmg_Graphics::decode_enableSmoothShading (const char * buf,
                                               int * value) {
  if (!buf || !value) return -1;
  CHECK(vrpn_unbuffer(&buf, value));
  return 0;
}

/*
char * nmg_Graphics::encode_enableUber
                     (int * len, int value) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = sizeof(int);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_enableUber:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, value);
  }

  return msgbuf;
}

int nmg_Graphics::decode_enableUber (const char * buf,
                                               int * value) {
  if (!buf || !value) return -1;
  CHECK(vrpn_unbuffer(&buf, value));
  return 0;
}
*/

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
    vrpn_buffer(&mptr, &mlen, value);
  }

  return msgbuf;
}

int nmg_Graphics::decode_enableTrueTip (const char * buf,
                                               int * value) {
  if (!buf || !value) return -1;
  CHECK(vrpn_unbuffer(&buf, value));
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
    vrpn_buffer(&mptr, &mlen, low);
    vrpn_buffer(&mptr, &mlen, hi);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setAdhesionSliderRange
                   (const char * buf, float * low, float * hi) {
  if (!buf || !low || !hi) return -1;
  CHECK(vrpn_unbuffer(&buf, low));
  CHECK(vrpn_unbuffer(&buf, hi));
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
    vrpn_buffer(&mptr, &mlen, r);
    vrpn_buffer(&mptr, &mlen, g);
    vrpn_buffer(&mptr, &mlen, b);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setAlphaColor
                   (const char * buf, float * r, float * g, float * b) {
  if (!buf || !r || !g || !b) return -1;
  CHECK(vrpn_unbuffer(&buf, r));
  CHECK(vrpn_unbuffer(&buf, g));
  CHECK(vrpn_unbuffer(&buf, b));
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
    vrpn_buffer(&mptr, &mlen, low);
    vrpn_buffer(&mptr, &mlen, hi);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setAlphaSliderRange
                   (const char * buf, float * low, float * hi) {
  if (!buf || !low || !hi) return -1;
  CHECK(vrpn_unbuffer(&buf, low));
  CHECK(vrpn_unbuffer(&buf, hi));
  return 0;
}

char * nmg_Graphics::encode_setColorMinMax
                     (int * len, float low, float hi) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = 2 * sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_setColorMinMax:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, low);
    vrpn_buffer(&mptr, &mlen, hi);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setColorMinMax
                   (const char * buf, float * low, float * hi) {
  if (!buf || !low || !hi) return -1;
  CHECK(vrpn_unbuffer(&buf, low));
  CHECK(vrpn_unbuffer(&buf, hi));
  return 0;
}

char * nmg_Graphics::encode_setDataColorMinMax
                     (int * len, float low, float hi) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = 2 * sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_setDataColorMinMax:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, low);
    vrpn_buffer(&mptr, &mlen, hi);
  }

  return msgbuf;  
}

char * nmg_Graphics::encode_setOpacitySliderRange( int * len, float low, float hi ) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = 2 * sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_setOpacitySliderRange:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, low);  
    vrpn_buffer(&mptr, &mlen, hi);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setDataColorMinMax
                   (const char * buf, float * low, float * hi) {
  if (!buf || !low || !hi) return -1;
  CHECK(vrpn_unbuffer(&buf, low));
  CHECK(vrpn_unbuffer(&buf, hi));
  return 0;
}



int nmg_Graphics::decode_setOpacitySliderRange
                         (const char * buf, float * low, float * hi) {
  if (!buf || !low || !hi) return -1;
  CHECK(vrpn_unbuffer(&buf, low));
  CHECK(vrpn_unbuffer(&buf, hi));
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
    vrpn_buffer(&mptr, &mlen, low);
    vrpn_buffer(&mptr, &mlen, hi);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setComplianceSliderRange
                   (const char * buf, float * low, float * hi) {
  if (!buf || !low || !hi) return -1;
  CHECK(vrpn_unbuffer(&buf, low));
  CHECK(vrpn_unbuffer(&buf, hi));
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
    vrpn_buffer(&mptr, &mlen, r);
    vrpn_buffer(&mptr, &mlen, g);
    vrpn_buffer(&mptr, &mlen, b);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setContourColor
                   (const char * buf, int * r, int * g, int * b) {
  if (!buf || !r || !g || !b) return -1;
  CHECK(vrpn_unbuffer(&buf, r));
  CHECK(vrpn_unbuffer(&buf, g));
  CHECK(vrpn_unbuffer(&buf, b));
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
    vrpn_buffer(&mptr, &mlen, width);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setContourWidth
                   (const char * buf, float * width) {
  if (!buf || !width) return -1;
  CHECK(vrpn_unbuffer(&buf, width));
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
    vrpn_buffer(&mptr, &mlen, low);
    vrpn_buffer(&mptr, &mlen, hi);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setFrictionSliderRange
                   (const char * buf, float * low, float * hi) {
  if (!buf || !low || !hi) return -1;
  CHECK(vrpn_unbuffer(&buf, low));
  CHECK(vrpn_unbuffer(&buf, hi));
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
    vrpn_buffer(&mptr, &mlen, low);
    vrpn_buffer(&mptr, &mlen, hi);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setBumpSliderRange
                   (const char * buf, float * low, float * hi) {
  if (!buf || !low || !hi) return -1;
  CHECK(vrpn_unbuffer(&buf, low));
  CHECK(vrpn_unbuffer(&buf, hi));
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
    vrpn_buffer(&mptr, &mlen, low);
    vrpn_buffer(&mptr, &mlen, hi);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setBuzzSliderRange
                   (const char * buf, float * low, float * hi) {
  if (!buf || !low || !hi) return -1;
  CHECK(vrpn_unbuffer(&buf, low));
  CHECK(vrpn_unbuffer(&buf, hi));
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
    vrpn_buffer(&mptr, &mlen, color);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setHandColor
                   (const char * buf, int * color) {
  if (!buf || !color) return -1;
  CHECK(vrpn_unbuffer(&buf, color));
  return 0;
}

#if 0
char * nmg_Graphics::encode_setAlphaPlaneName (int * len, const char * n) {

}

int nmg_Graphics::decode_setAlphaPlaneName (const char * buf, const char * n) {
  
}

char * nmg_Graphics::encode_setColorPlaneName (int * len, const char * n) {

}

int nmg_Graphics::decode_setColorPlaneName (const char * buf, const char * n) {

}

char * nmg_Graphics::encode_setContourPlaneName (int * len, const char * n) {

}

int nmg_Graphics::decode_setContourPlaneName (const char * buf,
                                              const char * n) {
  if (!buf || !color) return -1;
  CHECK(vrpn_unbuffer(&buf, n, 128));
  return 0;

}

char * nmg_Graphics::encode_setMaskPlaneName (int * len, const char * n) {

}

int nmg_Graphics::decode_setMaskPlaneName (const char *buf,
					      const char *n) {
}

char * nmg_Graphics::encode_setOpacityPlaneName (int * len, const char * n) {

}

int nmg_Graphics::decode_setOpacityPlaneName (const char *buf,
					      const char *n) {
}

char * nmg_Graphics::encode_setHeightPlaneName (int * len, const char * n) {

}

int nmg_Graphics::decode_setHeightPlaneName (const char * buf, const char * n) {

}

#endif



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
    vrpn_buffer(&mptr, &mlen, scale);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setIconScale
                   (const char * buf, float * scale) {
  if (!buf || !scale) return -1;
  CHECK(vrpn_unbuffer(&buf, scale));
  return 0;
}

char * nmg_Graphics::encode_enableCollabHand (int * len, vrpn_bool on) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;
 
  if (!len) return NULL;

  *len = sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_enableCollabHand:  "
            "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, on);
  }
  return msgbuf;
}

int nmg_Graphics::decode_enableCollabHand (const char * buf, vrpn_bool * on) {
  if (!buf || !on) return -1;
  CHECK(vrpn_unbuffer(&buf, on));
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
    vrpn_buffer(&mptr, &mlen, pos[0]);
    vrpn_buffer(&mptr, &mlen, pos[1]);
    vrpn_buffer(&mptr, &mlen, pos[2]);
    vrpn_buffer(&mptr, &mlen, quat[0]);
    vrpn_buffer(&mptr, &mlen, quat[1]);
    vrpn_buffer(&mptr, &mlen, quat[2]);
    vrpn_buffer(&mptr, &mlen, quat[3]);
  }
  return msgbuf;
}

int nmg_Graphics::decode_setCollabHandPos(const char * buf, double pos[3],
					  double quat[4])
{
  if (!buf || !pos || !quat) return -1;
  CHECK(vrpn_unbuffer(&buf, &pos[0]));
  CHECK(vrpn_unbuffer(&buf, &pos[1]));
  CHECK(vrpn_unbuffer(&buf, &pos[2]));
  CHECK(vrpn_unbuffer(&buf, &quat[0]));
  CHECK(vrpn_unbuffer(&buf, &quat[1]));
  CHECK(vrpn_unbuffer(&buf, &quat[2]));
  CHECK(vrpn_unbuffer(&buf, &quat[3]));
  return 0;
}

//set the mode of the icon following the collaborator's hand
char * nmg_Graphics::encode_setCollabMode(int * len, int mode)
{
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = sizeof(int);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_setCollabMode:  Out of memory.\n");
    *len = 0;
  }
  else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, mode);
  }
  return msgbuf;
}

int nmg_Graphics::decode_setCollabMode(const char * buf, int *mode)
{
  if (!buf || !mode) return -1;
  CHECK(vrpn_unbuffer(&buf, mode));
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
    vrpn_buffer(&mptr, &mlen, c[0]);
    vrpn_buffer(&mptr, &mlen, c[1]);
    vrpn_buffer(&mptr, &mlen, c[2]);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setMinColor
                   (const char * buf, double c [3]) {
  if (!buf || !c) return -1;
  CHECK(vrpn_unbuffer(&buf, &c[0]));
  CHECK(vrpn_unbuffer(&buf, &c[1]));
  CHECK(vrpn_unbuffer(&buf, &c[2]));

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
    vrpn_buffer(&mptr, &mlen, c[0]);
    vrpn_buffer(&mptr, &mlen, c[1]);
    vrpn_buffer(&mptr, &mlen, c[2]);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setMaxColor
                   (const char * buf, double c [3]) {
  if (!buf || !c) return -1;
  CHECK(vrpn_unbuffer(&buf, &c[0]));
  CHECK(vrpn_unbuffer(&buf, &c[1]));
  CHECK(vrpn_unbuffer(&buf, &c[2]));

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
    vrpn_buffer(&mptr, &mlen, c[0] / 255.0);
    vrpn_buffer(&mptr, &mlen, c[1] / 255.0);
    vrpn_buffer(&mptr, &mlen, c[2] / 255.0);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setMinColor
                   (const char * buf, int c [3]) {
  double d [3];

  if (!buf || !c) return -1;
  CHECK(vrpn_unbuffer(&buf, &d[0]));
  CHECK(vrpn_unbuffer(&buf, &d[1]));
  CHECK(vrpn_unbuffer(&buf, &d[2]));

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
    vrpn_buffer(&mptr, &mlen, c[0] / 255.0);
    vrpn_buffer(&mptr, &mlen, c[1] / 255.0);
    vrpn_buffer(&mptr, &mlen, c[2] / 255.0);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setMaxColor
                   (const char * buf, int c [3]) {
  double d [3];

  if (!buf || !c) return -1;
  CHECK(vrpn_unbuffer(&buf, &d[0]));
  CHECK(vrpn_unbuffer(&buf, &d[1]));
  CHECK(vrpn_unbuffer(&buf, &d[2]));

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
    vrpn_buffer(&mptr, &mlen, value);
  }

  return msgbuf;
}

int nmg_Graphics::decode_enableRulergrid
                   (const char * buf, int * value) {
  if (!buf || !value) return -1;
  CHECK(vrpn_unbuffer(&buf, value));
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
    vrpn_buffer(&mptr, &mlen, angle);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setRulergridAngle
                   (const char * buf, float * angle) {
  if (!buf || !angle) return -1;
  CHECK(vrpn_unbuffer(&buf, angle));
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
    vrpn_buffer(&mptr, &mlen, r);
    vrpn_buffer(&mptr, &mlen, g);
    vrpn_buffer(&mptr, &mlen, b);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setRulergridColor
                   (const char * buf, int * r, int * g, int * b) {
  if (!buf || !r || !g || !b) return -1;
  CHECK(vrpn_unbuffer(&buf, r));
  CHECK(vrpn_unbuffer(&buf, g));
  CHECK(vrpn_unbuffer(&buf, b));
  return 0;
}

char * nmg_Graphics::encode_setNullDataAlphaToggle (int *len, int val) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_setNullDataAlphaToggle:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, val);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setNullDataAlphaToggle (const char *buf, int * val) {
  if (!buf || !val) return -1;
  CHECK(vrpn_unbuffer(&buf, val));
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
    vrpn_buffer(&mptr, &mlen, x);
    vrpn_buffer(&mptr, &mlen, y);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setRulergridOffset
                   (const char * buf, float * x, float * y) {
  if (!buf || !x || !y) return -1;
  CHECK(vrpn_unbuffer(&buf, x));
  CHECK(vrpn_unbuffer(&buf, y));
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
    vrpn_buffer(&mptr, &mlen, alpha);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setRulergridOpacity
                   (const char * buf, float * alpha) {
  if (!buf || !alpha) return -1;
  CHECK(vrpn_unbuffer(&buf, alpha));
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
    vrpn_buffer(&mptr, &mlen, scale);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setRulergridScale
                   (const char * buf, float * scale) {
  if (!buf || !scale) return -1;
  CHECK(vrpn_unbuffer(&buf, scale));
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
    vrpn_buffer(&mptr, &mlen, x);
    vrpn_buffer(&mptr, &mlen, y);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setRulergridWidths
                   (const char * buf, float * x, float * y) {
  if (!buf || !x || !y) return -1;
  CHECK(vrpn_unbuffer(&buf, x));
  CHECK(vrpn_unbuffer(&buf, y));
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
    vrpn_buffer(&mptr, &mlen, shiny);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setSpecularity
                   (const char * buf, int * shiny) {
  if (!buf || !shiny) return -1;
  CHECK(vrpn_unbuffer(&buf, shiny));
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
    vrpn_buffer(&mptr, &mlen, diffuse);
  }

  return msgbuf;
}



int nmg_Graphics::decode_setDiffusePercent
		   (const char *buf, float * diffuse) {
  if (!buf || !diffuse) return -1;
  CHECK(vrpn_unbuffer(&buf, diffuse));
  return 0;
}

char * nmg_Graphics::encode_setSurfaceAlpha
		     (int *len, float surface_alpha, int region) {
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
    vrpn_buffer(&mptr, &mlen, surface_alpha);
    vrpn_buffer(&mptr, &mlen, region);
  }

  return msgbuf;
}



int nmg_Graphics::decode_setSurfaceAlpha
		   (const char *buf, float * surface_alpha, int * region) {
  if (!buf || !surface_alpha) return -1;
  CHECK(vrpn_unbuffer(&buf, surface_alpha));
  CHECK(vrpn_unbuffer(&buf, region));
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
    vrpn_buffer(&mptr, &mlen, specular_color);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setSpecularColor
                   (const char * buf, float * specular_color) {
  if (!buf || !specular_color) return -1;
  CHECK(vrpn_unbuffer(&buf, specular_color));
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
    vrpn_buffer(&mptr, &mlen, scale);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setSphereScale
                   (const char * buf, float * scale) {
  if (!buf || !scale) return -1;
  CHECK(vrpn_unbuffer(&buf, scale));
  return 0;
}

char * nmg_Graphics::encode_setTesselationStride
                     (int * len, int stride, int region) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = 2 * sizeof(int);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_setTesselationStride:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, stride);
    vrpn_buffer(&mptr, &mlen, region);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setTesselationStride
                   (const char * buf, int * stride, int * region) {
  if (!buf || !stride || !region) return -1;
  CHECK(vrpn_unbuffer(&buf, stride));
  CHECK(vrpn_unbuffer(&buf, region));
  return 0;
}

char * nmg_Graphics::encode_setTextureMode
                     (int * len, TextureMode m, TextureTransformMode xm, int region) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = 3*sizeof(vrpn_int32);
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
        vrpn_buffer(&mptr, &mlen, 1); break;
      case RULERGRID:  
        vrpn_buffer(&mptr, &mlen, 2); break;
      case ALPHA:  
        vrpn_buffer(&mptr, &mlen, 3); break;
      case COLORMAP:
	vrpn_buffer(&mptr, &mlen, 4); break;
      case SEM_DATA:
	vrpn_buffer(&mptr, &mlen, 5); break;
      case BUMPMAP:
	vrpn_buffer(&mptr, &mlen, 6); break;
      case HATCHMAP:
	vrpn_buffer(&mptr, &mlen, 7); break;
      case PATTERNMAP:
	vrpn_buffer(&mptr, &mlen, 8); break;
      default:
        fprintf(stderr, "nmg_Graphics::encode_setTextureMode:  "
                        "Got illegal texture mode %d.  "
                        "Sending NO_TEXTURES instead.\n", m);

	// fall through

      case NO_TEXTURES:
        vrpn_buffer(&mptr, &mlen, 0); break;
    }
    switch (xm) {
      case RULERGRID_COORD:
	vrpn_buffer(&mptr, &mlen, 0); break;
      case REGISTRATION_COORD:
	vrpn_buffer(&mptr, &mlen, 1); break;
      case MANUAL_REALIGN_COORD:
	vrpn_buffer(&mptr, &mlen, 2); break;
      default:
	fprintf(stderr, "nmg_Graphics::encode_setTextureMode:  "
			"Got illegal texture transform mode %d.  "
			"Sending RULERGRID_COORD instead.\n", xm);
	vrpn_buffer(&mptr, &mlen, 0); break;    
    }
    vrpn_buffer(&mptr, &mlen, region);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setTextureMode
                   (const char * buf, TextureMode * m,
			TextureTransformMode * xm, int * region) {
  int i;

  if (!buf || !m || !xm || !region) return -1;
  CHECK(vrpn_unbuffer(&buf, &i));

  switch (i) {
    case 0:  *m = NO_TEXTURES; break;
    case 1:  *m = CONTOUR; break;
    case 2:  *m = RULERGRID; break;
    case 3:  *m = ALPHA; break;
    case 4:  *m = COLORMAP; break;
    case 5:  *m = SEM_DATA; break;
    case 6:  *m = BUMPMAP; break;
    case 7:  *m = HATCHMAP; break;
    case 8:  *m = PATTERNMAP; break;

    default:
      fprintf(stderr, "nmg_Graphics::decode_setTextureMode:  "
                      "Got illegal texture mode %d.  "
                      "Sending NO_TEXTURES instead.\n", i);


      // fall through
      *m = NO_TEXTURES; break;
  }

  CHECK(vrpn_unbuffer(&buf, &i));
  
  switch (i) {
    case 0:  *xm = RULERGRID_COORD; break;
    case 1:  *xm = REGISTRATION_COORD; break;
    case 2:  *xm = MANUAL_REALIGN_COORD; break;
    default:
      fprintf(stderr, "nmg_Graphics::decode_setTextureMode:  "
                      "Got illegal texture transform mode %d.  "
                      "Sending RULERGRID_COORD instead.\n", i);


      // fall through
      *xm = RULERGRID_COORD; break;
  }

  CHECK(vrpn_unbuffer(&buf, region));

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
    vrpn_buffer(&mptr, &mlen, scale);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setTextureScale
                   (const char * buf, float * scale) {
  if (!buf || !scale) return -1;
  CHECK(vrpn_unbuffer(&buf, scale));
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
    vrpn_buffer(&mptr, &mlen, scale);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setTrueTipScale
                   (const char * buf, float * scale) {
  if (!buf || !scale) return -1;
  CHECK(vrpn_unbuffer(&buf, scale));
  return 0;
}

char * nmg_Graphics::encode_setUserMode
                     (int * len, int oldMode, int oldStyle, int newMode, int style,
		      int tool) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = 4 * sizeof(int);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_setUserMode:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, oldMode);
    vrpn_buffer(&mptr, &mlen, oldStyle);
    vrpn_buffer(&mptr, &mlen, newMode);
    vrpn_buffer(&mptr, &mlen, style);
    vrpn_buffer(&mptr, &mlen, tool);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setUserMode
                   (const char * buf, int * oldMode, int * oldStyle, int * newMode,
                             int * style, int * tool) {
  if (!buf || !oldMode || !oldStyle || !newMode || !style || !tool) return -1;
  CHECK(vrpn_unbuffer(&buf, oldMode));
  CHECK(vrpn_unbuffer(&buf, oldStyle));
  CHECK(vrpn_unbuffer(&buf, newMode));
  CHECK(vrpn_unbuffer(&buf, style));
  CHECK(vrpn_unbuffer(&buf, tool));
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
    vrpn_buffer(&mptr, &mlen, v[0]);
    vrpn_buffer(&mptr, &mlen, v[1]);
    vrpn_buffer(&mptr, &mlen, v[2]);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setLightDirection
                   (const char * buf, q_vec_type & v) {
  if (!buf) return -1;
  CHECK(vrpn_unbuffer(&buf, &v[0]));
  CHECK(vrpn_unbuffer(&buf, &v[1]));
  CHECK(vrpn_unbuffer(&buf, &v[2]));
  return 0;
}

char * nmg_Graphics::encode_addPolylinePoint
//                     (int * len, const float point [2][3]) {
                       (int *len, const PointType point[2]) {
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
    vrpn_buffer(&mptr, &mlen, point[0][0]);
    vrpn_buffer(&mptr, &mlen, point[0][1]);
    vrpn_buffer(&mptr, &mlen, point[0][2]);
    vrpn_buffer(&mptr, &mlen, point[1][0]);
    vrpn_buffer(&mptr, &mlen, point[1][1]);
    vrpn_buffer(&mptr, &mlen, point[1][2]);
  }

  return msgbuf;
}

int nmg_Graphics::decode_addPolylinePoint(const char * buf, PointType point[2]){
//                   (const char * buf, float point [2][3]) {

  CHECK(vrpn_unbuffer(&buf, &point[0][0]));
  CHECK(vrpn_unbuffer(&buf, &point[0][1]));
  CHECK(vrpn_unbuffer(&buf, &point[0][2]));
  CHECK(vrpn_unbuffer(&buf, &point[1][0]));
  CHECK(vrpn_unbuffer(&buf, &point[1][1]));
  CHECK(vrpn_unbuffer(&buf, &point[1][2]));
  return 0;
}

char * nmg_Graphics::encode_addPolySweepPoints(int * len, 
					       const PointType topL,
					       const PointType botL,
					       const PointType topR,
					       const PointType botR ) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = 12 * sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_addPolySweepPoints:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, topL[0]);
    vrpn_buffer(&mptr, &mlen, topL[1]);
    vrpn_buffer(&mptr, &mlen, topL[2]);

    vrpn_buffer(&mptr, &mlen, botL[0]);
    vrpn_buffer(&mptr, &mlen, botL[1]);
    vrpn_buffer(&mptr, &mlen, botL[2]);

    vrpn_buffer(&mptr, &mlen, topR[0]);
    vrpn_buffer(&mptr, &mlen, topR[1]);
    vrpn_buffer(&mptr, &mlen, topR[2]);

    vrpn_buffer(&mptr, &mlen, botR[0]);
    vrpn_buffer(&mptr, &mlen, botR[1]);
    vrpn_buffer(&mptr, &mlen, botR[2]);
  }

  return msgbuf;
}

int nmg_Graphics::decode_addPolySweepPoints(const char * buf,
					    PointType topL,
					    PointType botL,
					    PointType topR,
					    PointType botR ) {
  CHECK(vrpn_unbuffer(&buf, &topL[0]));
  CHECK(vrpn_unbuffer(&buf, &topL[1]));
  CHECK(vrpn_unbuffer(&buf, &topL[2]));

  CHECK(vrpn_unbuffer(&buf, &botL[0]));
  CHECK(vrpn_unbuffer(&buf, &botL[1]));
  CHECK(vrpn_unbuffer(&buf, &botL[2]));

  CHECK(vrpn_unbuffer(&buf, &topR[0]));
  CHECK(vrpn_unbuffer(&buf, &topR[1]));
  CHECK(vrpn_unbuffer(&buf, &topR[2]));

  CHECK(vrpn_unbuffer(&buf, &botR[0]));
  CHECK(vrpn_unbuffer(&buf, &botR[1]));
  CHECK(vrpn_unbuffer(&buf, &botR[2]));
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
    vrpn_buffer(&mptr, &mlen, p[0]);
    vrpn_buffer(&mptr, &mlen, p[1]);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setRubberLineStart (const char * buf, float p [2]) {
  CHECK(vrpn_unbuffer(&buf, &p[0]));
  CHECK(vrpn_unbuffer(&buf, &p[1]));
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
    vrpn_buffer(&mptr, &mlen, p[0]);
    vrpn_buffer(&mptr, &mlen, p[1]);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setRubberLineEnd (const char * buf, float p [2]) {
  CHECK(vrpn_unbuffer(&buf, &p[0]));
  CHECK(vrpn_unbuffer(&buf, &p[1]));
  return 0;
}

char * nmg_Graphics::encode_setRubberSweepLineStart (int * len,
						     const PointType left,
						     const PointType right) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = 6 * sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_setRubberLineStart:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, left[0]);
    vrpn_buffer(&mptr, &mlen, left[1]);
    vrpn_buffer(&mptr, &mlen, left[2]);

    vrpn_buffer(&mptr, &mlen, right[0]);
    vrpn_buffer(&mptr, &mlen, right[1]);
    vrpn_buffer(&mptr, &mlen, right[2]);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setRubberSweepLineStart (const char * buf,
						  PointType left,
						  PointType right) {
  CHECK(vrpn_unbuffer(&buf, &left[0]));
  CHECK(vrpn_unbuffer(&buf, &left[1]));
  CHECK(vrpn_unbuffer(&buf, &left[2]));

  CHECK(vrpn_unbuffer(&buf, &right[0]));
  CHECK(vrpn_unbuffer(&buf, &right[1]));
  CHECK(vrpn_unbuffer(&buf, &right[2]));
  return 0;
}

char * nmg_Graphics::encode_setRubberSweepLineEnd (int * len,
						   const PointType left,
						   const PointType right) {

  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = 6 * sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_setRubberLineEnd:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, left[0]);
    vrpn_buffer(&mptr, &mlen, left[1]);
    vrpn_buffer(&mptr, &mlen, left[2]);

    vrpn_buffer(&mptr, &mlen, right[0]);
    vrpn_buffer(&mptr, &mlen, right[1]);
    vrpn_buffer(&mptr, &mlen, right[2]);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setRubberSweepLineEnd (const char * buf,
						PointType left,
						PointType right) {
  CHECK(vrpn_unbuffer(&buf, &left[0]));
  CHECK(vrpn_unbuffer(&buf, &left[1]));
  CHECK(vrpn_unbuffer(&buf, &left[2]));

  CHECK(vrpn_unbuffer(&buf, &right[0]));
  CHECK(vrpn_unbuffer(&buf, &right[1]));
  CHECK(vrpn_unbuffer(&buf, &right[2]));
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
    vrpn_buffer(&mptr, &mlen, p[0]);
    vrpn_buffer(&mptr, &mlen, p[1]);
    vrpn_buffer(&mptr, &mlen, p[2]);
    vrpn_buffer(&mptr, &mlen, p[3]);
    vrpn_buffer(&mptr, &mlen, p[4]);
    vrpn_buffer(&mptr, &mlen, p[5]);

  }

  return msgbuf;
}

int nmg_Graphics::decode_setScanlineEndpoints (const char * buf, float p [6]) {
  CHECK(vrpn_unbuffer(&buf, &p[0]));
  CHECK(vrpn_unbuffer(&buf, &p[1]));
  CHECK(vrpn_unbuffer(&buf, &p[2]));
  CHECK(vrpn_unbuffer(&buf, &p[3]));
  CHECK(vrpn_unbuffer(&buf, &p[4]));
  CHECK(vrpn_unbuffer(&buf, &p[5]));
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
    vrpn_buffer(&mptr, &mlen, enable);

  }

  return msgbuf;
}

int nmg_Graphics::decode_displayScanlinePosition(const char * buf, 
							int *enable){
  CHECK(vrpn_unbuffer(&buf, enable));
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
    vrpn_buffer(&mptr, &mlen, lo[0]);
    vrpn_buffer(&mptr, &mlen, lo[1]);
    vrpn_buffer(&mptr, &mlen, lo[2]);
    vrpn_buffer(&mptr, &mlen, hi[0]);
    vrpn_buffer(&mptr, &mlen, hi[1]);
    vrpn_buffer(&mptr, &mlen, hi[2]);
  }

  return msgbuf;
}

int nmg_Graphics::decode_positionAimLine
                   (const char * buf, PointType lo, PointType hi) {
  //  if (!buf || !lo || !hi) return -1;  !lo and !hi aren't valid statements...
  if (!buf) return -1;
  CHECK(vrpn_unbuffer(&buf, &lo[0]));
  CHECK(vrpn_unbuffer(&buf, &lo[1]));
  CHECK(vrpn_unbuffer(&buf, &lo[2]));
  CHECK(vrpn_unbuffer(&buf, &hi[0]));
  CHECK(vrpn_unbuffer(&buf, &hi[1]));
  CHECK(vrpn_unbuffer(&buf, &hi[2]));
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
    vrpn_buffer(&mptr, &mlen, x0);
    vrpn_buffer(&mptr, &mlen, y0);
    vrpn_buffer(&mptr, &mlen, x1);
    vrpn_buffer(&mptr, &mlen, y1);
  }

  return msgbuf;
}

int nmg_Graphics::decode_positionRubberCorner
                   (const char * buf, float * x0, float * y0, float * x1,
                             float * y1) {
  if (!buf || !x0 || !y0 || !x1 || !y1) return -1;
  CHECK(vrpn_unbuffer(&buf, x0));
  CHECK(vrpn_unbuffer(&buf, y0));
  CHECK(vrpn_unbuffer(&buf, x1));
  CHECK(vrpn_unbuffer(&buf, y1));
  return 0;
}

char * nmg_Graphics::encode_positionSweepLine(int * len,
					      const PointType loL,
					      const PointType hiL,
					      const PointType loR,
					      const PointType hiR) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = 12 * sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_positionSweepLine:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, loL[0]);
    vrpn_buffer(&mptr, &mlen, loL[1]);
    vrpn_buffer(&mptr, &mlen, loL[2]);

    vrpn_buffer(&mptr, &mlen, hiL[0]);
    vrpn_buffer(&mptr, &mlen, hiL[1]);
    vrpn_buffer(&mptr, &mlen, hiL[2]);

    vrpn_buffer(&mptr, &mlen, loR[0]);
    vrpn_buffer(&mptr, &mlen, loR[1]);
    vrpn_buffer(&mptr, &mlen, loR[2]);

    vrpn_buffer(&mptr, &mlen, hiR[0]);
    vrpn_buffer(&mptr, &mlen, hiR[1]);
    vrpn_buffer(&mptr, &mlen, hiR[2]);
  }

  return msgbuf;
}

int nmg_Graphics::decode_positionSweepLine
                   (const char * buf, PointType loL, PointType hiL,
		    PointType loR, PointType hiR) {
  //  if (!buf || !loL || !hiL || !loR || !hiR) return -1; // !loL...!hiR aren't valid statements
  if (!buf) return -1;
  CHECK(vrpn_unbuffer(&buf, &loL[0]));
  CHECK(vrpn_unbuffer(&buf, &loL[1]));
  CHECK(vrpn_unbuffer(&buf, &loL[2]));

  CHECK(vrpn_unbuffer(&buf, &hiL[0]));
  CHECK(vrpn_unbuffer(&buf, &hiL[1]));
  CHECK(vrpn_unbuffer(&buf, &hiL[2]));

  CHECK(vrpn_unbuffer(&buf, &loR[0]));
  CHECK(vrpn_unbuffer(&buf, &loR[1]));
  CHECK(vrpn_unbuffer(&buf, &loR[2]));

  CHECK(vrpn_unbuffer(&buf, &hiR[0]));
  CHECK(vrpn_unbuffer(&buf, &hiR[1]));
  CHECK(vrpn_unbuffer(&buf, &hiR[2]));
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
    vrpn_buffer(&mptr, &mlen, x);
    vrpn_buffer(&mptr, &mlen, y);
    vrpn_buffer(&mptr, &mlen, z);
  }

  return msgbuf;
}

int nmg_Graphics::decode_positionSphere
                   (const char * buf, float * x, float * y, float * z) {
  if (!buf || !x || !y || !z) return -1;
  CHECK(vrpn_unbuffer(&buf, x));
  CHECK(vrpn_unbuffer(&buf, y));
  CHECK(vrpn_unbuffer(&buf, z));
  return 0;
}

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
    vrpn_buffer(&mptr, &mlen, low);
    vrpn_buffer(&mptr, &mlen, hi);
  }

  return msgbuf;
}

// Decodes two floats after network transmission
int nmg_Graphics::decode_setRealignTextureSliderRange ( const char *buf,
							float *low,float *hi) {
  if (!buf || !low || !hi) return -1;
  CHECK(vrpn_unbuffer(&buf, low));
  CHECK(vrpn_unbuffer(&buf, hi));
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
    vrpn_buffer(&mptr, &mlen, on);
  }

  return msgbuf;
}

// Decodes one int after network transmission
int nmg_Graphics::decode_enableRealignTextures ( const char *buf, int *on) {
  if (!buf || !on) return -1;
  CHECK(vrpn_unbuffer(&buf, on));
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
    vrpn_buffer(&mptr, &mlen, dx);
    vrpn_buffer(&mptr, &mlen, dy);
  }

  return msgbuf;
}

// Decodes two floats after network transmission
int nmg_Graphics::decode_dx_dy ( const char *buf, float *dx, float *dy ) {
  if (!buf || !dx || !dy) return -1;
  CHECK(vrpn_unbuffer(&buf, dx));
  CHECK(vrpn_unbuffer(&buf, dy));
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
    vrpn_buffer(&mptr, &mlen, theta);
  }
  
  return msgbuf;
}

// Decodes one float after network transmission
int nmg_Graphics::decode_rotateTextures ( const char *buf, float * theta) {
  if (!buf || !theta) return -1;
  CHECK(vrpn_unbuffer(&buf, theta));
  return 0;
}

// End Realign Texture Network Transmission Code.

char *nmg_Graphics::encode_updateTexture ( int *len, int whichTexture,
  const char *name, int start_x, int start_y, int end_x, int end_y) {

  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = 6*sizeof(int) + strlen(name);
  int name_len = strlen(name);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_updateTexture_type:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, whichTexture);
    vrpn_buffer(&mptr, &mlen, name_len);
    vrpn_buffer(&mptr, &mlen, name, strlen(name));
    vrpn_buffer(&mptr, &mlen, start_x);
    vrpn_buffer(&mptr, &mlen, start_y);
    vrpn_buffer(&mptr, &mlen, end_x);
    vrpn_buffer(&mptr, &mlen, end_y);
  }

  return msgbuf;
}

int nmg_Graphics::decode_updateTexture ( const char *buf, int *whichTexture,
   char **name, int *start_x, int *start_y, int *end_x, int *end_y) {
  int name_len;

  if (!buf || !whichTexture || !name || !start_x || !start_y ||
	!end_x || !end_y) return -1;
  CHECK(vrpn_unbuffer(&buf, whichTexture));
  CHECK(vrpn_unbuffer(&buf, &name_len));
  *name = new char [name_len];
  CHECK(vrpn_unbuffer(&buf, *name, name_len));
  CHECK(vrpn_unbuffer(&buf, start_x));
  CHECK(vrpn_unbuffer(&buf, start_y));
  CHECK(vrpn_unbuffer(&buf, end_x));
  CHECK(vrpn_unbuffer(&buf, end_y));
  return 0;
}

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
    vrpn_buffer(&mptr, &mlen, on);
  }

  return msgbuf;
}

// Decodes one int after network transmission
int nmg_Graphics::decode_enableRegistration ( const char *buf, int *on) {
  if (!buf || !on) return -1;
  CHECK(vrpn_unbuffer(&buf, on));
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
    	vrpn_buffer(&mptr, &mlen, xform[i]);
  }

  return msgbuf;
}

int nmg_Graphics::decode_textureTransform(const char *buf, double *xform)
{
  if (!buf || !xform) return -1;
  int i;
  for (i = 0; i < 16; i++)
      CHECK(vrpn_unbuffer(&buf, &(xform[i])));
  return 0;
}

char * nmg_Graphics::encode_setViewTransform (
     int             * len,
     v_xform_type xform) {
//fprintf(stderr,"encode_setViewTransform\n");
  char *msgbuf = NULL;
  char *mptr;
  int mlen;

  if (!len) return NULL;

  *len = 8 * sizeof(double);
  msgbuf = new char[*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_setViewTransform:  "
                    "Out of memory!\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, xform.xlate[0]);
    vrpn_buffer(&mptr, &mlen, xform.xlate[1]);
    vrpn_buffer(&mptr, &mlen, xform.xlate[2]);
    vrpn_buffer(&mptr, &mlen, xform.rotate[0]);
    vrpn_buffer(&mptr, &mlen, xform.rotate[1]);
    vrpn_buffer(&mptr, &mlen, xform.rotate[2]);
    vrpn_buffer(&mptr, &mlen, xform.rotate[3]);
    vrpn_buffer(&mptr, &mlen, xform.scale);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setViewTransform (const char  * buf,
                                           v_xform_type * xform) {
//fprintf(stderr,"decode_setViewTransform\n");
  const char *bptr = buf;
  if (!buf || !xform) return -1;
  CHECK(vrpn_unbuffer(&bptr, &xform->xlate[0]));
  CHECK(vrpn_unbuffer(&bptr, &xform->xlate[1]));
  CHECK(vrpn_unbuffer(&bptr, &xform->xlate[2]));
  CHECK(vrpn_unbuffer(&bptr, &xform->rotate[0]));
  CHECK(vrpn_unbuffer(&bptr, &xform->rotate[1]));
  CHECK(vrpn_unbuffer(&bptr, &xform->rotate[2]));
  CHECK(vrpn_unbuffer(&bptr, &xform->rotate[3]));
  CHECK(vrpn_unbuffer(&bptr, &xform->scale));
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
//fprintf(stderr,"encode_createScreenImage\n");
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
      vrpn_buffer(&mptr, &mlen, filename, 512);
      vrpn_buffer(&mptr, &mlen, (int)(type));
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
//fprintf(stderr,"decode_createScreenImage\n");
   const char *bptr = buf;
   int temp;
   if (!buf || !(*filename) || !type) return -1;
   CHECK(vrpn_unbuffer(&bptr, *filename, 512));
//fprintf(stderr, "Filename: %s\n", *filename);
   CHECK(vrpn_unbuffer(&bptr, &temp));
//fprintf(stderr, "Type: %d\n", temp);
   *type = (ImageType)(temp);
   return 0;
}
// End screen capture network code


char * nmg_Graphics::encode_setViztexScale
                     (int * len, float scale) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_setViztexScale:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, scale);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setViztexScale
                   (const char * buf, float * scale) {
  if (!buf || !scale) return -1;
  CHECK(vrpn_unbuffer(&buf, scale));
  return 0;
}

char * nmg_Graphics::encode_setRegionMaskHeight
                     (int * len, float min_height, 
                      float max_height, int region) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = 2 * sizeof(float) + sizeof(int);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_setRegionMaskHeight:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, min_height);
    vrpn_buffer(&mptr, &mlen, max_height);
    vrpn_buffer(&mptr, &mlen, region);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setRegionMaskHeight
                   (const char * buf, float * min_height,
                    float * max_height, int * region) {
  if (!buf || !min_height || !max_height || !region) return -1;
  CHECK(vrpn_unbuffer(&buf, min_height));
  CHECK(vrpn_unbuffer(&buf, max_height));
  CHECK(vrpn_unbuffer(&buf, region));
  return 0;
}

char * nmg_Graphics::encode_setRegionControlPlaneName (int * len, const char * name, 
                                                       int region) {
  char * msgbuf = NULL;

  if (!len) return NULL;

  *len = sizeof(int) + strlen(name) + 3 + sizeof(region);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_setRegionControlPlaneName:  "
                    "Out of memory.\n");
    *len = 0;
  } else {    
    sprintf( msgbuf, "%d %s %d", strlen(name), name, region);
  }

  return msgbuf;
}

int nmg_Graphics::decode_setRegionControlPlaneName (const char * buf, char ** name, 
                                                    int *region) {
  if (!buf || !name || !region) return -1;
  int len;
  const char *mptr;
  sscanf( buf, "%d ", &len);
  mptr = buf + sizeof(int);
  *name = new char[len+1];
  sscanf( mptr, "%s %d", *name, region );
  return 0;
}

char * nmg_Graphics::encode_createRegion
                     (int * len) {
  return NULL;
}

int nmg_Graphics::decode_createRegion
                   (const char * buf) {
  return 0;
}

char * nmg_Graphics::encode_destroyRegion
                     (int * len, int region) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = sizeof(int);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_destroyRegion:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, region);
  }

  return msgbuf;
}

int nmg_Graphics::decode_destroyRegion
                   (const char * buf, int * region) {
  if (!buf || !region) return -1;
  CHECK(vrpn_unbuffer(&buf, region));
  return 0;
}

char * nmg_Graphics::encode_associate (int * len, vrpn_bool associate, int region)
{
 char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = sizeof(vrpn_bool) + sizeof(int);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmg_Graphics::encode_associate:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, associate);
    vrpn_buffer(&mptr, &mlen, region);
  }

  return msgbuf;
}

int nmg_Graphics::decode_associate (const char * buf, vrpn_bool *associate, int *region)
{
  if (!buf || !associate || !region) return -1;
  CHECK(vrpn_unbuffer(&buf, associate));
  CHECK(vrpn_unbuffer(&buf, region));
  return 0;
}


// TCH Dissertation Dec 2001
//
char * nmg_Graphics::encode_setFeelGrid (int * len, int xside, int yside,
      q_vec_type * vertices) {

   assert(0);
   return NULL;
}

int nmg_Graphics::decode_setFeelGrid (const char * buf, int * xside,
      int * yside, q_vec_type ** vertices) {

   assert(0);
   return 0;
}
char * nmg_Graphics::encode_showFeelGrid (int * len, vrpn_bool on) {

   assert(0);
   return NULL;
}

int nmg_Graphics::decode_showFeelGrid (const char * buf, vrpn_bool * on) {

   assert(0);
   return 0;
}
char * nmg_Graphics::encode_setFeelPlane (int * len, q_vec_type origin,
      q_vec_type normal) {

   assert(0);
   return NULL;
}

int nmg_Graphics::decode_setFeelPlane (const char * buf,
      q_vec_type * origin, q_vec_type * normal) {

   assert(0);
   return 0;
}
char * nmg_Graphics::encode_showFeelPlane (int * len, vrpn_bool on) {

   assert(0);
   return NULL;
}

int nmg_Graphics::decode_showFeelPlane (const char * buf, vrpn_bool * on) {

   assert(0);
   return 0;
}
