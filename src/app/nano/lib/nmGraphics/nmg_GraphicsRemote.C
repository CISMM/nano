/*===3rdtech===
  Copyright (c) 2000 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#include <string.h>
#include "nmg_GraphicsRemote.h"

#include <vrpn_Shared.h>
#include <vrpn_Connection.h>

nmg_Graphics_Remote::nmg_Graphics_Remote (vrpn_Connection * c) :
    nmg_Graphics (c, "nmg Graphics Remote"),
    d_colorMapDir (NULL),
    d_textureDir (NULL) {


}

nmg_Graphics_Remote::~nmg_Graphics_Remote (void) {


}

void nmg_Graphics_Remote::mainloop (void) {

  nmg_Graphics::mainloop();

}

void nmg_Graphics_Remote::changeDataset( nmb_Dataset * /*data*/) {
  fprintf(stderr, "WARNING: nmg_Graphics_Remote::changeDataset \n"
	  "      Attempting to send pointer over the network DOESN'T WORK.\n");
}

void nmg_Graphics_Remote::resizeViewport (int width , int height ) {
   //	printf("nmg_Graphics_Remote::resizeViewport not implemented\n");
  struct timeval now;
  char * msgbuf;
  int len;
  int retval;

  msgbuf = encode_resizeViewport(&len, width, height);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now, d_resizeViewport_type,
                           d_myId, msgbuf, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::resizeViewport:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;

}
void nmg_Graphics_Remote::getViewportSize(int *width, int * height) {
    fprintf(stderr, "WARNING: nmg_Graphics_Remote::getViewportSize unimplemented!\n");
    *width = 1024;
    *height = 768;
}

void nmg_Graphics_Remote::getDisplayPosition (q_vec_type &  ll ,
                                              q_vec_type &  ul ,
                                              q_vec_type &  ur ){
   printf("WARNING: nmg_Graphics_Remote::getDisplayPosition \
all three vectors hard-coded to zero\n\
    This implies non-headtracked mode!!\n ");

   ll[0] = 0;
   ll[1] = 0;
   ll[2] = 0;
   ul[0] = 0;
   ul[1] = 0;
   ul[2] = 0;
   ur[0] = 0;
   ur[1] = 0;
   ur[2] = 0;
}

void nmg_Graphics_Remote::loadRulergridImage (const char * name) {
  struct timeval now;
  int len = 0;
  int retval;

  if (name)
    len = 1 + strlen(name);
  gettimeofday(&now, NULL);
  if (d_connection) {
    retval = d_connection->pack_message(len, now, d_loadRulergridImage_type,
                           d_myId, (char *) name, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::loadRulergridImage:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
}

void nmg_Graphics_Remote::loadVizImage (const char * name) {
  struct timeval now;
  int len = 0;
  int retval;

  if (name)
    len = 1 + strlen(name);
  gettimeofday(&now, NULL);
  if (d_connection) {
    retval = d_connection->pack_message(len, now, d_loadVizImage_type,
                           d_myId, (char *) name, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::loadVizImage:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
}

void nmg_Graphics_Remote::causeGridReColor (void) {
}

void nmg_Graphics_Remote::causeGridRedraw (void) {
  struct timeval now;
  int retval;

  gettimeofday(&now, NULL);
  if (d_connection) {
    retval = d_connection->pack_message(0, now, d_causeGridRedraw_type,
                           d_myId, NULL, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::causeGridRedraw:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
}

void nmg_Graphics_Remote::causeGridRebuild (void) {
  struct timeval now;
  int retval;

  gettimeofday(&now, NULL);
  if (d_connection) {
    retval = d_connection->pack_message(0, now, d_causeGridRebuild_type,
                           d_myId, NULL, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::causeGridRebuild:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
}

void nmg_Graphics_Remote::enableChartjunk (int value) {
  struct timeval now;
  char * msgbuf;
  int len;
  int retval;

  msgbuf = encode_enableChartjunk(&len, value);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now, d_enableChartjunk_type,
                           d_myId, msgbuf, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::enableChartjunk:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;
}

void nmg_Graphics_Remote::enableFilledPolygons (int value) {
  struct timeval now;
  char * msgbuf;
  int len;
  int retval;

  msgbuf = encode_enableFilledPolygons(&len, value);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now, d_enableFilledPolygons_type,
                           d_myId, msgbuf, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::enableFilledPolygons:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;
}

void nmg_Graphics_Remote::enableSmoothShading (int value) {
  struct timeval now;
  char * msgbuf;
  int len;
  int retval;

  msgbuf = encode_enableSmoothShading(&len, value);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now, d_enableSmoothShading_type,
                           d_myId, msgbuf, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::enableSmoothShading:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;
}

void nmg_Graphics_Remote::enableTrueTip (int value) {
  struct timeval now;
  char * msgbuf;
  int len;
  int retval;

  msgbuf = encode_enableTrueTip(&len, value);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now, d_enableTrueTip_type,
                           d_myId, msgbuf, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::enableTrueTip:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;
}

void nmg_Graphics_Remote::setAdhesionSliderRange (float low, float hi) {
  struct timeval now;
  char * msgbuf;
  int len;
  int retval;

  msgbuf = encode_setAdhesionSliderRange(&len, low, hi);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now, d_setAdhesionSliderRange_type,
                           d_myId, msgbuf, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::setAdhesionSliderRange:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;
}

void nmg_Graphics_Remote::setAlphaColor (float r, float g, float b) {
  struct timeval now;
  char * msgbuf;
  int len;
  int retval;

  msgbuf = encode_setAlphaColor(&len, r, g, b);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now, d_setAlphaColor_type,
                           d_myId, msgbuf, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::setAlphaColor:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;
}

void nmg_Graphics_Remote::setAlphaSliderRange (float low, float hi) {
  struct timeval now;
  char * msgbuf;
  int len;
  int retval;

  d_minAlpha = low;
  d_maxAlpha = hi;

  msgbuf = encode_setAlphaSliderRange(&len, low, hi);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now, d_setAlphaSliderRange_type,
                           d_myId, msgbuf, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::setAlphaSliderRange:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;
}

void nmg_Graphics_Remote::setBumpMapName (const char * name) {
  struct timeval now;
  int len = 0;
  int retval;

  if (name)
    len = 1 + strlen(name);
  gettimeofday(&now, NULL);
  if (d_connection) {
    retval = d_connection->pack_message(len, now, d_setBumpMapName_type,
                           d_myId, (char *) name, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::setBumpMapName:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
}

void nmg_Graphics_Remote::setColorMapDirectory (const char * name) {
  struct timeval now;
  int len = 0;
  int retval;

  if (d_colorMapDir)
    delete [] d_colorMapDir;
  if (!name) {
    d_colorMapDir = NULL;
    return;
  }
  d_colorMapDir = new char [1 + strlen(name)];
  strcpy(d_colorMapDir, name);

  if (name)
    len = 1 + strlen(name);
  gettimeofday(&now, NULL);
  if (d_connection) {
    retval = d_connection->pack_message(len, now, d_setColorMapDirectory_type,
                           d_myId, (char *) name, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::setColorMapDirectory:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
}

void nmg_Graphics_Remote::setTextureDirectory (const char * name) {
   // blank for now.
  struct timeval now;
  int len = 0;
  int retval;

  if (d_textureDir)
    delete [] d_textureDir;
  if (!name) {
    d_textureDir = NULL;
    return;
  }
  d_textureDir = new char [1 + strlen(name)];
  strcpy(d_textureDir, name);

  if (name)
    len = 1 + strlen(name);
  gettimeofday(&now, NULL);
  if (d_connection) {
    retval = d_connection->pack_message(len, now, d_setTextureDirectory_type,
                           d_myId, (char *) name, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::setTextureDirectory:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
}

void nmg_Graphics_Remote::setColorMapName (const char * name) {
  struct timeval now;
  int len = 0;
  int retval;

  // implementation
  if (strcmp(name, "none") == 0) {
          d_curColorMap = NULL;
  } else {
          d_colorMap.load_from_file(name, d_colorMapDir);
          d_curColorMap = &d_colorMap;
  }

  if (name)
    len = 1 + strlen(name);
  gettimeofday(&now, NULL);
  if (d_connection) {
    retval = d_connection->pack_message(len, now, d_setColorMapName_type,
                           d_myId, (char *) name, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::setColorMapName:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
}

void nmg_Graphics_Remote::setColorMinMax (float low, float hi) {
  struct timeval now;
  char * msgbuf;
  int len;
  int retval;

  d_color_min = low;
  d_color_max = hi;

  msgbuf = encode_setColorMinMax(&len, low, hi);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now, d_setColorMinMax_type,
                           d_myId, msgbuf, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::setColorMinMax:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;
}

void nmg_Graphics_Remote::setDataColorMinMax (float low, float hi) {
  struct timeval now;
  char * msgbuf;
  int len;
  int retval;

  d_data_min = low;
  d_data_max = hi;

  msgbuf = encode_setDataColorMinMax(&len, low, hi);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now, d_setDataColorMinMax_type,
                           d_myId, msgbuf, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::setDataColorMinMax:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;
}

void nmg_Graphics_Remote::setOpacitySliderRange (float low, float hi) {
  struct timeval now;
  char * msgbuf;
  int len;
  int retval;

  d_opacity_slider_min = low;
  d_opacity_slider_max = hi;

  msgbuf = encode_setOpacitySliderRange(&len, low, hi);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now, d_setOpacitySliderRange_type,
                           d_myId, msgbuf, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::setOpacitySliderRange:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;
}

void nmg_Graphics_Remote::setComplianceSliderRange (float low, float hi) {
  struct timeval now;
  char * msgbuf;
  int len;
  int retval;

  msgbuf = encode_setComplianceSliderRange(&len, low, hi);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now,
                           d_setComplianceSliderRange_type,
                           d_myId, msgbuf, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::setComplianceSliderRange:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;
}

void nmg_Graphics_Remote::setContourColor (int r, int g, int b) {
  struct timeval now;
  char * msgbuf;
  int len;
  int retval;

  msgbuf = encode_setContourColor(&len, r, g, b);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now, d_setContourColor_type,
                           d_myId, msgbuf, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::setContourColor:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;
}

void nmg_Graphics_Remote::setContourWidth (float width) {
  struct timeval now;
  char * msgbuf;
  int len;
  int retval;

  msgbuf = encode_setContourWidth(&len, width);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now, d_setContourWidth_type,
                           d_myId, msgbuf, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::setContourWidth:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;
}

void nmg_Graphics_Remote::setFrictionSliderRange (float low, float hi) {
  struct timeval now;
  char * msgbuf;
  int len;
  int retval;

  msgbuf = encode_setFrictionSliderRange(&len, low, hi);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now, d_setFrictionSliderRange_type,
                           d_myId, msgbuf, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::setFrictionSliderRange:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;
}

void nmg_Graphics_Remote::setBumpSliderRange (float low, float hi) {
  struct timeval now;
  char * msgbuf;
  int len;
  int retval;

  msgbuf = encode_setBumpSliderRange(&len, low, hi);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now,
							d_setBumpSliderRange_type,
                           d_myId, msgbuf, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::setBumpSliderRange:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;
}

void nmg_Graphics_Remote::setBuzzSliderRange (float low, float hi) {
  struct timeval now;
  char * msgbuf;
  int len;
  int retval;

  msgbuf = encode_setBuzzSliderRange(&len, low, hi);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now,
						d_setBuzzSliderRange_type,
                           d_myId, msgbuf, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::setBuzzSliderRange:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;
}


void nmg_Graphics_Remote::setHandColor (int color) {
  struct timeval now;
  char * msgbuf;
  int len;
  int retval;

  msgbuf = encode_setHandColor(&len, color);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now, d_setHandColor_type,
                           d_myId, msgbuf, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::setHandColor:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;
}

void nmg_Graphics_Remote::setHatchMapName (const char * name) {
  struct timeval now;
  int len = 0;
  int retval;

  if (name)
    len = 1 + strlen(name);
  gettimeofday(&now, NULL);
  if (d_connection) {
    retval = d_connection->pack_message(len, now, d_setHatchMapName_type,
                           d_myId, (char *) name, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::setHatchMapName:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
}

// virtual
void nmg_Graphics_Remote::setAlphaPlaneName (const char * name) {
  int len = 0;
  struct timeval now;
  int retval;

  if (name)
    len = 1 + strlen(name);
  gettimeofday(&now, NULL);
  if (d_connection) {
    retval = d_connection->pack_message(len, now, d_setAlphaPlaneName_type,
                           d_myId, (char *) name, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::setAlphaPlaneName:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
}

// virtual
void nmg_Graphics_Remote::setColorPlaneName (const char * name) {
  int len = 0;
  struct timeval now;
  int retval;

  if (name)
    len = 1 + strlen(name);
  gettimeofday(&now, NULL);
  if (d_connection) {
    retval = d_connection->pack_message(len, now, d_setColorPlaneName_type,
                           d_myId, (char *) name, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::setColorPlaneName:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
}

// virtual
void nmg_Graphics_Remote::setContourPlaneName (const char * name) {
  int len = 0;
  struct timeval now;
  int retval;

  if (name)
    len = 1 + strlen(name);
  gettimeofday(&now, NULL);
  if (d_connection) {
    retval = d_connection->pack_message(len, now, d_setContourPlaneName_type,
                           d_myId, (char *) name, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::setContourPlaneName:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
}

// virtual
void nmg_Graphics_Remote::setOpacityPlaneName (const char * name) {
  int len = 0;
  struct timeval now;
  int retval;

  if (name)
    len = 1 + strlen(name);
  gettimeofday(&now, NULL);
  if (d_connection) {
    retval = d_connection->pack_message(len, now, d_setOpacityPlaneName_type,
                           d_myId, (char *) name, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::setOpacityPlaneName:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
}

// virtual
void nmg_Graphics_Remote::setTransparentPlaneName (const char * name) {
  int len = 0;
  struct timeval now;
  int retval;

  if (name)
    len = 1 + strlen(name);
  gettimeofday(&now, NULL);
  if (d_connection) {
    retval = d_connection->pack_message(len, now, d_setTransparentPlaneName_type,
                           d_myId, (char *) name, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::setTransparentPlaneName:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
}

// virtual
void nmg_Graphics_Remote::setMaskPlaneName (const char * name) {
  int len = 0;
  struct timeval now;
  int retval;

  if (name)
    len = 1 + strlen(name);
  gettimeofday(&now, NULL);
  if (d_connection) {
    retval = d_connection->pack_message(len, now, d_setMaskPlaneName_type,
                           d_myId, (char *) name, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::setMaskPlaneName:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
}

// virtual
void nmg_Graphics_Remote::setHeightPlaneName (const char * name) {
  int len = 0;
  struct timeval now;
  int retval;

  if (name)
    len = 1 + strlen(name);
  gettimeofday(&now, NULL);
  if (d_connection) {
    retval = d_connection->pack_message(len, now, d_setHeightPlaneName_type,
                           d_myId, (char *) name, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::setHeightPlaneName:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
}

// virtual
void nmg_Graphics_Remote::setVizPlaneName (const char * name) {
  int len = 0;
  struct timeval now;
  int retval;

  if (name)
    len = 1 + strlen(name);
  gettimeofday(&now, NULL);
  if (d_connection) {
    retval = d_connection->pack_message(len, now, d_setVizPlaneName_type,
                           d_myId, (char *) name, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::setVizPlaneName:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
}

void nmg_Graphics_Remote::setIconScale (float scale) {
  struct timeval now;
  char * msgbuf;
  int len;
  int retval;

  msgbuf = encode_setIconScale(&len, scale);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now, d_setIconScale_type,
                           d_myId, msgbuf, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::setIconScale:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;
}

void nmg_Graphics_Remote::enableCollabHand (vrpn_bool on) {
  struct timeval now;
  char *msgbuf;
  int len;
  int retval;

  msgbuf = encode_enableCollabHand(&len, on);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now, d_enableCollabHand_type,
                                       d_myId, msgbuf,
                                        vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::enableCollabHand:  "
              "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;
}

void nmg_Graphics_Remote::setCollabHandPos (double pos[], double quat[])
{
  struct timeval now;
  char *msgbuf;
  int len;
  int retval;

  msgbuf = encode_setCollabHandPos(&len, pos, quat);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now, d_setCollabHandPos_type,
					d_myId, msgbuf,
					vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::setCollabHandPos:  "
	      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;
}

void nmg_Graphics_Remote::setCollabMode (int mode)
{
  struct timeval now;
  char *msgbuf;
  int len;
  int retval;

  msgbuf = encode_setCollabMode(&len, mode);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now, d_setCollabMode_type,
					d_myId, msgbuf,
					vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::setCollabMode:  "
	      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;
}

    // arguments in range [0..1]
void nmg_Graphics_Remote::setMinColor (const double c [3]) {
  struct timeval now;
  char * msgbuf;
  int len;
  int retval;

  d_minColor[0] = c[0];
  d_minColor[1] = c[1];
  d_minColor[2] = c[2];

  msgbuf = encode_setMinColor(&len, c);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now, d_setMinColor_type,
                           d_myId, msgbuf, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::setMinColor:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;
}

void nmg_Graphics_Remote::setMaxColor (const double c [3]) {
  struct timeval now;
  char * msgbuf;
  int len;
  int retval;

  d_maxColor[0] = c[0];
  d_maxColor[1] = c[1];
  d_maxColor[2] = c[2];

  msgbuf = encode_setMaxColor(&len, c);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now, d_setMaxColor_type,
                           d_myId, msgbuf, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::setMaxColor:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;
}

    // arguments in range [0..255]
void nmg_Graphics_Remote::setMinColor (const int c [3]) {
  struct timeval now;
  char * msgbuf;
  int len;
  int retval;

  d_minColor[0] = c[0] / 255.0;
  d_minColor[1] = c[1] / 255.0;
  d_minColor[2] = c[2] / 255.0;

  msgbuf = encode_setMinColor(&len, c);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now, d_setMinColor_type,
                           d_myId, msgbuf, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::setMinColor:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;
}

void nmg_Graphics_Remote::setMaxColor (const int c [3]) {
  struct timeval now;
  char * msgbuf;
  int len;
  int retval;

  d_maxColor[0] = c[0] / 255.0;
  d_maxColor[1] = c[1] / 255.0;
  d_maxColor[2] = c[2] / 255.0;

  msgbuf = encode_setMaxColor(&len, c);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now, d_setMaxColor_type,
                           d_myId, msgbuf, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::setMaxColor:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;
}

void nmg_Graphics_Remote::setPatternMapName (const char * name) {
  struct timeval now;
  int len;
  int retval;

  if (name)
    len = 1 + strlen(name);
  gettimeofday(&now, NULL);
  if (d_connection) {
    retval = d_connection->pack_message(len, now, d_setPatternMapName_type,
                           d_myId, (char *) name, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::setPatternMapName:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
}

//
// Realign Textures Remote Code:
//

// 
void nmg_Graphics_Remote::createRealignTextures( const char *name ) {
  struct timeval now;
  int len;
  int retval;

  len = strlen( name );
  gettimeofday(&now, NULL);
  if ( d_connection ) {
    retval = d_connection->pack_message(len, now, d_createRealignTextures_type,
					d_myId, name,
					vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::createRealignTextures:  "
	      "Couldn't pack message to send to server.\n");
    }
  }
}

void nmg_Graphics_Remote::setRealignTextureSliderRange (float low, float high){
  struct timeval now;
  char * msgbuf;
  int len;
  int retval;

  msgbuf = encode_setRealignTextureSliderRange(&len, low, high);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now,
					d_setRealignTextureSliderRange_type,
					d_myId, msgbuf,
					vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::setRealignTextureSliderRange:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;
}

void nmg_Graphics_Remote::setRealignTexturesConversionMap( const char *map,
							   const char *mapdir ) {

  struct timeval now;
  char * msgbuf;
  int len;
  int retval;

  msgbuf = encode_two_char_arrays(&len, map, mapdir);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now,
					d_setRealignTexturesConversionMap_type,
					d_myId, msgbuf,
					vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::setRealignTexturesConversionMap:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;
}

void nmg_Graphics_Remote::computeRealignPlane( const char *name,
                                               const char *newname ) {
  struct timeval now;
  char * msgbuf;
  int len;
  int retval;

  msgbuf = encode_two_char_arrays(&len, name, newname);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now,
					d_computeRealignPlane_type,
					d_myId, msgbuf,
					vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::computeRealignPlane:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;
}

/*
void nmg_Graphics_Remote::enableRealignTextures (int on) {
  struct timeval now;
  char * msgbuf;
  int len;
  int retval;

  msgbuf = encode_enableRealignTextures(&len, on);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now,
					d_enableRealignTextures_type,
					d_myId, msgbuf,
					vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::enableRealignTextures:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;
}
*/

void nmg_Graphics_Remote::translateTextures ( int , float dx, float dy ) {
  struct timeval now;
  char * msgbuf;
  int len;
  int retval;

  msgbuf = encode_dx_dy(&len, dx, dy);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now,
					d_translateTextures_type,
					d_myId, msgbuf,
					vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::translateTextures:  "
	      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;
}

void nmg_Graphics_Remote::scaleTextures ( int , float dx, float dy ) {
  struct timeval now;
  char * msgbuf;
  int len;
  int retval;

  msgbuf = encode_dx_dy(&len, dx, dy);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now,
					d_scaleTextures_type,
					d_myId, msgbuf,
					vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::scaleTextures:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;
}

void nmg_Graphics_Remote::shearTextures ( int , float dx, float dy ) {
  struct timeval now;
  char * msgbuf;
  int len;
  int retval;

  msgbuf = encode_dx_dy(&len, dx, dy);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now,
					d_shearTextures_type,
					d_myId, msgbuf,
					vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::shearTextures:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;
}

void nmg_Graphics_Remote::rotateTextures ( int , float theta ) {
  struct timeval now;
  char * msgbuf;
  int len;
  int retval;

  msgbuf = encode_rotateTextures(&len, theta);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now,
					d_rotateTextures_type,
					d_myId, msgbuf,
					vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::rotateTextures:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;
}

void nmg_Graphics_Remote::setTextureCenter( float dx, float dy ) {
  struct timeval now;
  char * msgbuf;
  int len;
  int retval;

  msgbuf = encode_dx_dy(&len, dx, dy);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now,
					d_setTextureCenter_type,
					d_myId, msgbuf,
					vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::setTextureCenter:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;
}
//
// End Realign Textures Remote Code:
//

void nmg_Graphics_Remote::loadRawDataTexture
       (const int /*which*/, const char * /*image_name*/,
        const int /*start_x*/, const int /*start_y*/) {
  // TODO BUG BUG BUG
}

void nmg_Graphics_Remote::updateTexture(int which,
       const char *image_name,
       int start_x, int start_y,
       int end_x, int end_y)
{
  struct timeval now;
  char * msgbuf;
  int len;
  int retval;

  msgbuf = encode_updateTexture(&len, which, image_name, start_x, start_y,
	end_x, end_y);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now,
                                        d_updateTexture_type,
                                        d_myId, msgbuf,
                                        vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::updateTexture:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;
}

/*
void nmg_Graphics_Remote::enableRegistration (int on) {
  struct timeval now;
  char * msgbuf;
  int len;
  int retval;

  msgbuf = encode_enableRegistration(&len, on);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now,
                                        d_enableRegistration_type,
                                        d_myId, msgbuf,
                                        vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::enableRegistration:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;
}
*/

void nmg_Graphics_Remote::setTextureTransform(double *xform) {
  struct timeval now;
  char * msgbuf;
  int len;
  int retval;

  msgbuf = encode_textureTransform(&len, xform);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now,
                                        d_setTextureTransform_type,
                                        d_myId, msgbuf,
                                        vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::setTextureTransform:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;
}

/*
void nmg_Graphics_Remote::enableRulergrid (int value) {
  struct timeval now;
  char * msgbuf;
  int len;
  int retval;

  msgbuf = encode_enableRulergrid(&len, value);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now, d_enableRulergrid_type,
                           d_myId, msgbuf, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::enableRulergrid:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;
}
*/

void nmg_Graphics_Remote::setRulergridAngle (float angle) {
  struct timeval now;
  char * msgbuf;
  int len;
  int retval;

  msgbuf = encode_setRulergridAngle(&len, angle);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now, d_setRulergridAngle_type,
                           d_myId, msgbuf, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::setRulergridAngle:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;
}

void nmg_Graphics_Remote::setRulergridColor (int r, int g, int b) {
  struct timeval now;
  char * msgbuf;
  int len;
  int retval;

  msgbuf = encode_setRulergridColor(&len, r, g, b);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now, d_setRulergridColor_type,
                           d_myId, msgbuf, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::setRulergridColor:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;
}

void nmg_Graphics_Remote::setRulergridOpacity (float alpha) {
  struct timeval now;
  char * msgbuf;
  int len;
  int retval;

  msgbuf = encode_setRulergridOpacity(&len, alpha);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now, d_setRulergridOpacity_type,
                           d_myId, msgbuf, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::setRulergridOpacity:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;
}

void nmg_Graphics_Remote::setRulergridOffset (float x, float y) {
  struct timeval now;
  char * msgbuf;
  int len;
  int retval;

  msgbuf = encode_setRulergridOffset(&len, x, y);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now, d_setRulergridOffset_type,
                           d_myId, msgbuf, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::setRulergridOffset:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;
}

void nmg_Graphics_Remote::setRulergridScale (float scale) {
  struct timeval now;
  char * msgbuf;
  int len;
  int retval;

  msgbuf = encode_setRulergridScale(&len, scale);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now, d_setRulergridScale_type,
                           d_myId, msgbuf, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::setRulergridScale:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;
}

void nmg_Graphics_Remote::setNullDataAlphaToggle (int val) {
  struct timeval now;
  char * msgbuf;
  int len;
  int retval;

  msgbuf = encode_setNullDataAlphaToggle(&len, val);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now, 
			   d_setNullDataAlphaToggle_type,
                           d_myId, msgbuf, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::setNullDataAlphaToggle:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;
}

void nmg_Graphics_Remote::setRulergridWidths (float x, float y) {
  struct timeval now;
  char * msgbuf;
  int len;
  int retval;

  msgbuf = encode_setRulergridWidths(&len, x, y);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now, d_setRulergridWidths_type,
                           d_myId, msgbuf, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::setRulergridWidths:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;
}

void nmg_Graphics_Remote::setSpecularity (int shiny) {
  struct timeval now;
  char * msgbuf;
  int len;
  int retval;

  msgbuf = encode_setSpecularity(&len, shiny);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now, d_setSpecularity_type,
                           d_myId, msgbuf, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::setSpecularity:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;

  // HACK HACK HACK
  // Ought to get a confirming response from the remote server.

  d_specularity = shiny;
}


void nmg_Graphics_Remote::setDiffusePercent (float diffuse) {
  struct timeval now;
  char * msgbuf;
  int len;
  int retval;

  msgbuf = encode_setDiffusePercent(&len, diffuse);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now, d_setDiffusePercent_type,
                           d_myId, msgbuf, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::setDiffusePercent:  "
		      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;

}

void nmg_Graphics_Remote::setSurfaceAlpha (float surface_alpha) {
  struct timeval now;
  char * msgbuf;
  int len;
  int retval;

  msgbuf = encode_setSurfaceAlpha(&len, surface_alpha);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now, d_setSurfaceAlpha_type,
                           d_myId, msgbuf, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::setSurfaceAlpha:  "
		      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;
}

void nmg_Graphics_Remote::setSpecularColor (float specular_color) {
  struct timeval now;
  char * msgbuf;
  int len;
  int retval;

  msgbuf = encode_setSpecularColor(&len, specular_color);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now, d_setSpecularColor_type,
                           d_myId, msgbuf, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::setSpecularColor:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;
}

void nmg_Graphics_Remote::setSphereScale (float scale) {
  struct timeval now;
  char * msgbuf;
  int len;
  int retval;

  msgbuf = encode_setSphereScale(&len, scale);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now, d_setSphereScale_type,
                           d_myId, msgbuf, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::setSphereScale:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;
}

void nmg_Graphics_Remote::setTesselationStride (int stride) {
  struct timeval now;
  char * msgbuf;
  int len;
  int retval;

  msgbuf = encode_setTesselationStride(&len, stride);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now, d_setTesselationStride_type,
                           d_myId, msgbuf, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::setTesselationStride:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;
}

void nmg_Graphics_Remote::setTextureMode (TextureMode m,
	TextureTransformMode xm) {
  struct timeval now;
  char * msgbuf;
  int len;
  int retval;

  d_textureMode = m;

  msgbuf = encode_setTextureMode(&len, m, xm);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now, d_setTextureMode_type,
                           d_myId, msgbuf, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::setTextureMode:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;
}

void nmg_Graphics_Remote::setTextureScale (float scale) {
  struct timeval now;
  char * msgbuf;
  int len;
  int retval;

  msgbuf = encode_setTextureScale(&len, scale);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now, d_setTextureScale_type,
                           d_myId, msgbuf, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::setTextureScale:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;
}

void nmg_Graphics_Remote::setTrueTipScale (float scale) {
  struct timeval now;
  char * msgbuf;
  int len;
  int retval;

  msgbuf = encode_setTrueTipScale(&len, scale);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now, d_setTrueTipScale_type,
                           d_myId, msgbuf, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::setTrueTipScale:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;
}

void nmg_Graphics_Remote::setUserMode (int oldMode, int oldStyle, int newMode, int style,
				       int tool) {
  struct timeval now;
  char * msgbuf;
  int len;
  int retval;

  msgbuf = encode_setUserMode(&len, oldMode, oldStyle, newMode, style, tool);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now, d_setUserMode_type,
                           d_myId, msgbuf, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::setUserMode:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;
}

void nmg_Graphics_Remote::setLightDirection (q_vec_type & v) {
  struct timeval now;
  char * msgbuf;
  int len;
  int retval;

  msgbuf = encode_setLightDirection(&len, v);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now, d_setLightDirection_type,
                           d_myId, msgbuf, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::setLightDirection:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;

  // HACK HACK HACK
  // Ought to get a confirming response from the remote server.

  q_vec_copy(d_lightDirection, (double *) v);
}

void nmg_Graphics_Remote::resetLightDirection (void) {
  struct timeval now;
  int retval;

  gettimeofday(&now, NULL);
  if (d_connection) {
    retval = d_connection->pack_message(0, now, d_resetLightDirection_type,
                           d_myId, NULL, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::resetLightDirection:  "
                      "Couldn't pack message to send to server.\n");
    }
  }

  // HACK HACK HACK
  // Ought to get a confirming response from the remote server?

  q_set_vec(d_lightDirection, 0.0, 0.0, 1.0);
}

int nmg_Graphics_Remote::addPolylinePoint(const PointType point[2]) {
//int nmg_Graphics_Remote::addPolylinePoint (const float point [2][3]) {
  struct timeval now;
  char * msgbuf;
  int len;
  int retval;

  msgbuf = encode_addPolylinePoint(&len, point);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now, d_addPolylinePoint_type,
                           d_myId, msgbuf, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::addPolylinePoint:  "
                      "Couldn't pack message to send to server.\n");
      return -1;
    }
  }

  return 0;
}

void nmg_Graphics_Remote::emptyPolyline (void) {
  struct timeval now;
  int retval;

  gettimeofday(&now, NULL);
  if (d_connection) {
    retval = d_connection->pack_message(0, now, d_emptyPolyline_type,
                           d_myId, NULL, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::emptyPolyline:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
}

// virtual
void nmg_Graphics_Remote::setRubberLineStart (float p0, float p1) {
  float p [2];
  p[0] = p0;
  p[1] = p1;
  setRubberLineStart(p);
}

// virtual
void nmg_Graphics_Remote::setRubberLineEnd (float p0, float p1) {
  float p [2];
  p[0] = p0;
  p[1] = p1;
  setRubberLineEnd(p);
}

// virtual
void nmg_Graphics_Remote::setRubberLineStart (const float p [2]) {
  struct timeval now;
  char * msgbuf;
  int len;
  int retval;

  msgbuf = encode_setRubberLineStart(&len, p);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now, d_setRubberLineStart_type,
                           d_myId, msgbuf, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::setRubberLineStart:  "
                      "Couldn't pack message to send to server.\n");
    }
  }

  if (msgbuf)
    delete [] msgbuf;
}

// virtual
void nmg_Graphics_Remote::setRubberLineEnd (const float p [2]) {
  struct timeval now;
  char * msgbuf;
  int len;
  int retval;

  msgbuf = encode_setRubberLineEnd(&len, p);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now, d_setRubberLineEnd_type,
                           d_myId, msgbuf, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::setRubberLineEnd:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;
}

// virtual
void nmg_Graphics_Remote::setScanlineEndpoints (const float p0 [3],
		const float p1[3]) {
  struct timeval now;
  char * msgbuf;
  int len;
  int retval;
  float p[6];
  for (int i = 0; i < 3; i++)
  {
	p[i] = p0[i];
	p[i+3] = p1[i];
  }

  msgbuf = encode_setScanlineEndpoints(&len, p);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now, d_setScanlineEndpoints_type,
                           d_myId, msgbuf, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::setScanlineEndpoints:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;
}

void nmg_Graphics_Remote::displayScanlinePosition (const int enable) {
  struct timeval now;
  char * msgbuf;
  int len;
  int retval;

  msgbuf = encode_displayScanlinePosition(&len, enable);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now, 
				d_displayScanlinePosition_type,
                           d_myId, msgbuf, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::displayScanlinePosition:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;

}

void nmg_Graphics_Remote::positionAimLine (const PointType lo,
                                           const PointType hi) {
  struct timeval now;
  char * msgbuf;
  int len;
  int retval;

  msgbuf = encode_positionAimLine(&len, lo, hi);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now, d_positionAimLine_type,
                           d_myId, msgbuf, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::positionAimLine:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;
}

void nmg_Graphics_Remote::positionRubberCorner (float x0, float y0,
                                                float x1, float y1, int) {
  struct timeval now;
  char * msgbuf;
  int len;
  int retval;
    fprintf(stderr,"Warning, nmg_Graphics_Remote::positionRubberCorner ignores last param.\n");
  msgbuf = encode_positionRubberCorner(&len, x0, y0, x1, y1);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now, d_positionRubberCorner_type,
                           d_myId, msgbuf, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::positionRubberCorner:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;
}
void nmg_Graphics_Remote::positionRegionBox (float, float, float, float, float, int) {
    fprintf(stderr,"Warning, nmg_Graphics_Remote::positionRegionBox not implemented.\n");
}

void nmg_Graphics_Remote::setRubberSweepLineStart (const PointType left,
						   const PointType right) {

    struct timeval now;
    char * msgbuf;
    int len;
    int retval;
    
    msgbuf = encode_setRubberSweepLineStart(&len, left, right);
    gettimeofday(&now, NULL);
    if (d_connection && msgbuf) {
	retval = d_connection->pack_message(len, now, d_setRubberSweepLineStart_type,
				    d_myId, msgbuf, vrpn_CONNECTION_RELIABLE);
	if (retval) {
	    fprintf(stderr, "nmg_Graphics_Remote::setRubberSweepLineStart:  "
		    "Couldn't pack message to send to server.\n");
	}
    }
    if (msgbuf)
	delete [] msgbuf;
}
void nmg_Graphics_Remote::setRubberSweepLineEnd (const PointType left,
						 const PointType right) {

    struct timeval now;
    char * msgbuf;
    int len;
    int retval;
    
    msgbuf = encode_setRubberSweepLineEnd(&len, left, right);
    gettimeofday(&now, NULL);
    if (d_connection && msgbuf) {
	retval = d_connection->pack_message(len, now, d_setRubberSweepLineEnd_type,
				    d_myId, msgbuf, vrpn_CONNECTION_RELIABLE);
	if (retval) {
	    fprintf(stderr, "nmg_Graphics_Remote::setRubberSweepLineEnd:  "
		    "Couldn't pack message to send to server.\n");
	}
    }
    if (msgbuf)
	delete [] msgbuf;
}

int nmg_Graphics_Remote::addPolySweepPoints (const PointType topL,
					     const PointType botL,
					     const PointType topR,
					     const PointType botR) {
    struct timeval now;
    char * msgbuf;
    int len;
    int retval;
    
    msgbuf = encode_addPolySweepPoints(&len, topL, botL, topR, botR);
    gettimeofday(&now, NULL);
    if (d_connection && msgbuf) {
	retval = d_connection->pack_message(len, now, d_addPolySweepPoints_type,
				    d_myId, msgbuf, vrpn_CONNECTION_RELIABLE);
	if (retval) {
	    fprintf(stderr, "nmg_Graphics_Remote::addPolySweepPoints:  "
		    "Couldn't pack message to send to server.\n");
	}
    }
    if (msgbuf)
	delete [] msgbuf;
    return 0;
}


void nmg_Graphics_Remote::positionSweepLine (const PointType topL,
                                             const PointType botL,
					     const PointType topR,
                                             const PointType botR     ) {
  struct timeval now;
  char * msgbuf;
  int len;
  int retval;

  msgbuf = encode_positionSweepLine(&len, topL, botL, topR, botR);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now, d_positionSweepLine_type,
                           d_myId, msgbuf, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::positionSweepLine:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;
}

void nmg_Graphics_Remote::positionSphere (float x, float y, float z) {
  struct timeval now;
  char * msgbuf;
  int len;
  int retval;

  msgbuf = encode_positionSphere(&len, x, y, z);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now, d_positionSphere_type,
                           d_myId, msgbuf, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::positionSphere:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;
}

void nmg_Graphics_Remote::setViewTransform (v_xform_type xform) {
  struct timeval now;
  char * msgbuf;
  int len;
  int retval;

  msgbuf = encode_setViewTransform(&len, xform);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now,
                           d_setViewTransform_type,
                           d_myId, msgbuf, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::setViewTransform:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;
}

void nmg_Graphics_Remote::createScreenImage
(
   const char      *filename,
   const ImageType  type
)
{
   struct timeval now;
   char *msgbuf;
   int len;
   int retval;


   msgbuf = encode_createScreenImage(&len, filename, type);
   gettimeofday(&now, NULL);
   if (d_connection && msgbuf)
   {
      retval = d_connection->pack_message(len, now, d_createScreenImage_type,
                                          d_myId, msgbuf,
                                          vrpn_CONNECTION_RELIABLE);
      if (retval)
         fprintf(stderr, "nmg_Graphics_Remote::createScreenImage:  "
                         "Couldn't pack message to send to server.\n");
   }

   if (msgbuf)
      delete [] msgbuf;
}

void nmg_Graphics_Remote::chooseVisualization(int viz_type)
{
   struct timeval now;
   char *msgbuf;
   int len;
   int retval;

   msgbuf = encode_chooseVisualization(&len, viz_type);
   gettimeofday(&now, NULL);
   if (d_connection && msgbuf)
   {
      retval = d_connection->pack_message(len, now, d_createScreenImage_type,
                                          d_myId, msgbuf,
                                          vrpn_CONNECTION_RELIABLE);
      if (retval)
         fprintf(stderr, "nmg_Graphics_Remote::chooseVisualization:  "
                         "Couldn't pack message to send to server.\n");
   }

   if (msgbuf)
      delete [] msgbuf;
}

void nmg_Graphics_Remote::setVisualizationMinHeight(float viz_min)
{
   struct timeval now;
   char *msgbuf;
   int len;
   int retval;

   msgbuf = encode_setVisualizationMinHeight(&len, viz_min);
   gettimeofday(&now, NULL);
   if (d_connection && msgbuf)
   {
      retval = d_connection->pack_message(len, now, d_createScreenImage_type,
                                          d_myId, msgbuf,
                                          vrpn_CONNECTION_RELIABLE);
      if (retval)
         fprintf(stderr, "nmg_Graphics_Remote::setVisualizationMinHeight:  "
                         "Couldn't pack message to send to server.\n");
   }

   if (msgbuf)
      delete [] msgbuf;
}

void nmg_Graphics_Remote::setVisualizationMaxHeight(float viz_max)
{
   struct timeval now;
   char *msgbuf;
   int len;
   int retval;

   msgbuf = encode_setVisualizationMinHeight(&len, viz_max);
   gettimeofday(&now, NULL);
   if (d_connection && msgbuf)
   {
      retval = d_connection->pack_message(len, now, d_createScreenImage_type,
                                          d_myId, msgbuf,
                                          vrpn_CONNECTION_RELIABLE);
      if (retval)
         fprintf(stderr, "nmg_Graphics_Remote::setVisualizationMaxHeight:  "
                         "Couldn't pack message to send to server.\n");
   }

   if (msgbuf)
      delete [] msgbuf;
}

void nmg_Graphics_Remote::setVisualizationAlpha(float viz_alpha)
{
   struct timeval now;
   char *msgbuf;
   int len;
   int retval;

   msgbuf = encode_setVisualizationMinHeight(&len, viz_alpha);
   gettimeofday(&now, NULL);
   if (d_connection && msgbuf)
   {
      retval = d_connection->pack_message(len, now, d_createScreenImage_type,
                                          d_myId, msgbuf,
                                          vrpn_CONNECTION_RELIABLE);
      if (retval)
         fprintf(stderr, "nmg_Graphics_Remote::setVisualizationAlpha:  "
                         "Couldn't pack message to send to server.\n");
   }

   if (msgbuf)
      delete [] msgbuf;
}
 
void nmg_Graphics_Remote::setViztexScale (float scale) {
  struct timeval now;
  char * msgbuf;
  int len;
  int retval;

  msgbuf = encode_setViztexScale(&len, scale);
  gettimeofday(&now, NULL);
  if (d_connection && msgbuf) {
    retval = d_connection->pack_message(len, now, d_setViztexScale_type,
                           d_myId, msgbuf, vrpn_CONNECTION_RELIABLE);
    if (retval) {
      fprintf(stderr, "nmg_Graphics_Remote::setViztexScale:  "
                      "Couldn't pack message to send to server.\n");
    }
  }
  if (msgbuf)
    delete [] msgbuf;
}

// ACCESSORS
void nmg_Graphics_Remote::getLightDirection (q_vec_type * v) const {
  q_vec_copy(*v, (double *) d_lightDirection);
}

int nmg_Graphics_Remote::getHandColor (void) const {
  return d_handColor;
}

int nmg_Graphics_Remote::getSpecularity (void) const {
  return d_specularity;
}

const double * nmg_Graphics_Remote::getMinColor (void) const {
  return d_minColor;
}


const double * nmg_Graphics_Remote::getMaxColor (void) const {
  return d_maxColor;
}



