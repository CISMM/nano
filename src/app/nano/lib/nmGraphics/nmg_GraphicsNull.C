/*===3rdtech===
  Copyright (c) 2000 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#include "nmg_GraphicsNull.h"

#include <GL/gl.h>
#ifdef sgi
#include <GL/glu.h>
#endif
#include <v.h>

//#include <vrpn_Connection.h>

#include <PPM.h>
#include <BCPlane.h>
#include <BCGrid.h>
#include <nmb_Dataset.h>
#include <nmb_Globals.h>

#include "graphics_globals.h"  // for NUM_USERS, NUM_foo

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)<(b)?(b):(a))


nmg_Graphics_Null::nmg_Graphics_Null
                                (nmb_Dataset * ,
                                 const int [3],
                                 const int [3],
                                 const char *,
                                 vrpn_Connection *) :
    nmg_Graphics (NULL, "nmg Graphics Null GL"),
    d_displayIndexList (new v_index [NUM_USERS]) {

  int i;

  // do pgl_init() stuff

  /* initialize graphics  */
  printf("Initializing graphics...\n");

  if ( v_open() != V_OK )
      exit(V_ERROR);

  fprintf(stderr, "Done.\n");

  /*
   * initialize display for each user
   */
  // OPEN NULL DISPLAYS
  char namelist [1][V_MAX_NAME_LENGTH] = { "null" };

  for ( i = 0; i < NUM_USERS; i++ ) {

    /* open display */
    if ( (d_displayIndexList[i] = v_open_display(namelist, 0)) !=
        V_NULL_DISPLAY )
        exit(V_ERROR);

    //printf("display = '%s'\n", v_display_name(d_displayIndexList[i]));
  }

  /* Set up the viewing info */
  
  /* Set initial user mode */

  /* set up user and object trees     */
  printf("Creating the world...\n");
  v_create_world(NUM_USERS,
                 d_displayIndexList);

  /********************************************************************/
  /* Build the display lists we'll need to draw the data, etc */
  /********************************************************************/

  fprintf(stderr,"Building display lists...\n");

  /* added by qliu for texture mapping*/
  /*initialize the texture mapping*/
  // Texture mapping is not enabled here, but when a data set is mapped*/

  /* Build display lists for surface scanning in X fastest, since
   * that is the way the SPM will start scanning.
   * There is one list for each row of data points except for
   * the last.  Since these are for rows scanning in X fastest,
   * we have one per Y index. */

  fprintf(stderr, "done\n");

  // other (non-pgl_init()) setup

}


nmg_Graphics_Null::~nmg_Graphics_Null (void) {

  v_close();
}



void nmg_Graphics_Null::mainloop (void) {

  nmg_Graphics::mainloop();

}

void nmg_Graphics_Null::resizeViewport (int /* width */, int /* height */) {}
void nmg_Graphics_Null::getDisplayPosition (q_vec_type & /* ll */,
                                            q_vec_type & /* ul */,
                                            q_vec_type & /* ur */) {}

void nmg_Graphics_Null::loadRulergridImage (const char * ) {}
void nmg_Graphics_Null::loadVizImage (const char * ) {}
void nmg_Graphics_Null::enableChartjunk (int ) {}
void nmg_Graphics_Null::enableFilledPolygons (int ) {}
void nmg_Graphics_Null::enableSmoothShading (int ) {}
void nmg_Graphics_Null::setAdhesionSliderRange (float , float ) {}
void nmg_Graphics_Null::setAlphaColor (float , float , float ) {}
void nmg_Graphics_Null::setAlphaSliderRange (float , float ) {}
void nmg_Graphics_Null::setBumpMapName (const char * ) {}
void nmg_Graphics_Null::setColorMapDirectory (const char * ) {}
void nmg_Graphics_Null::setColorMapName (const char * ) {}
void nmg_Graphics_Null::setColorMinMax (float , float ) {}
void nmg_Graphics_Null::setDataColorMinMax (float , float ) {}
void nmg_Graphics_Null::setOpacitySliderRange (float, float ) {}
void nmg_Graphics_Null::setTextureDirectory (const char * ) {}
void nmg_Graphics_Null::setComplianceSliderRange (float , float ) {}
void nmg_Graphics_Null::setContourColor (int , int , int ) {}
void nmg_Graphics_Null::setFrictionSliderRange (float , float ) {}
void nmg_Graphics_Null::setBumpSliderRange (float , float ) {}
void nmg_Graphics_Null::setBuzzSliderRange (float , float ) {}
void nmg_Graphics_Null::setHandColor (int ) {}
void nmg_Graphics_Null::setHatchMapName (const char * ) {}
void nmg_Graphics_Null::setContourWidth (float ) {}
void nmg_Graphics_Null::setMinColor (const double [3]) {}
void nmg_Graphics_Null::setMaxColor (const double [3]) {}
void nmg_Graphics_Null::setMinColor (const int [3]) {}
void nmg_Graphics_Null::setMaxColor (const int [3]) {}
void nmg_Graphics_Null::setPatternMapName (const char * ) {}

// Genetic Textures;
void nmg_Graphics_Null::enableGeneticTextures (int) {}
void nmg_Graphics_Null::sendGeneticTexturesData (int, char **) {}

// Realigning Textures:
void nmg_Graphics_Null::createRealignTextures( const char * ) {}
void nmg_Graphics_Null::setRealignTextureSliderRange (float, float) {}
void nmg_Graphics_Null::setRealignTexturesConversionMap( char *, char * ) {}
void nmg_Graphics_Null::computeRealignPlane( char *, char * ) {}
void nmg_Graphics_Null::enableRealignTextures (int on) {}
void nmg_Graphics_Null::translateTextures ( int on, float dx, float dy ) {}
void nmg_Graphics_Null::scaleTextures ( int on, float dx, float dy ) {}
void nmg_Graphics_Null::shearTextures ( int on, float dx, float dy ) {}
void nmg_Graphics_Null::rotateTextures ( int on, float theta ) {}
void nmg_Graphics_Null::setTextureCenter( float dx, float dy ) {}

void nmg_Graphics_Null::setTextureTransform(double *xform) {}

void nmg_Graphics_Null::enableRulergrid (int ) {}
void nmg_Graphics_Null::setRulergridAngle (float ) {}
void nmg_Graphics_Null::setRulergridColor (int , int , int ) {}
void nmg_Graphics_Null::setRulergridOffset (float , float ) {}
void nmg_Graphics_Null::setNullDataAlphaToggle( int ) {}
void nmg_Graphics_Null::setRulergridOpacity (float ) {}
void nmg_Graphics_Null::setRulergridScale (float ) {}
void nmg_Graphics_Null::setRulergridWidths (float , float ) {}
void nmg_Graphics_Null::setSpecularity (int ) {}
void nmg_Graphics_Null::setSphereScale (float ) {}
void nmg_Graphics_Null::setTesselationStride (int, int) {}
void nmg_Graphics_Null::setTextureMode (TextureMode m, TextureTransformMode, int)
   { d_textureMode = m; }
void nmg_Graphics_Null::setTextureScale (float ) {}
void nmg_Graphics_Null::setUserMode (int , int , int) {}
void nmg_Graphics_Null::setLightDirection (q_vec_type & ) {}
void nmg_Graphics_Null::resetLightDirection (void) {}

//int nmg_Graphics_Null::addPolylinePoint (const float [2][3]) {
int nmg_Graphics_Null::addPolylinePoint (const PointType[2]) {
  return 0;
}

int nmg_Graphics_Null::addSlowLine3dMarker (const float[3]) {
  return 0;
}

void nmg_Graphics_Null::emptyPolyline (void) {}
void nmg_Graphics_Null::setRubberLineStart (float, float) {}
void nmg_Graphics_Null::setRubberLineEnd (float, float) {}
void nmg_Graphics_Null::setRubberLineStart (const float [2]) {}
void nmg_Graphics_Null::setRubberLineEnd (const float [2]) {}
void nmg_Graphics_Null::setScanlineEndpoints (const float [3],
		const float [3]) {}
void nmg_Graphics_Null::displayScanlinePosition (const int) {}
void nmg_Graphics_Null::positionAimLine (const PointType , const PointType ) {}
void nmg_Graphics_Null::positionRubberCorner(float , float , float , float ) {}
void nmg_Graphics_Null::positionSweepLine (const PointType , const PointType){}
void nmg_Graphics_Null::positionSphere (float , float , float ) {}

void nmg_Graphics_Null::createScreenImage
(
   const char      *filename,
   const ImageType  type
) {}

void nmg_Graphics_Null::setRegionMask(BCPlane *mask){}
void nmg_Graphics_Null::setViztexScale (float ) {}
void nmg_Graphics_Null::setRegionMaskHeight (float, float, int) {}
void nmg_Graphics_Null::createRegion (){}
void nmg_Graphics_Null::destroyRegion (int){}

void nmg_Graphics_Null::lockAlpha(vrpn_bool lock, int region){}
void nmg_Graphics_Null::lockFilledPolygons(vrpn_bool lock, int region){}
void nmg_Graphics_Null::lockTextureDisplayed(vrpn_bool lock, int region){}
void nmg_Graphics_Null::lockTextureMode(vrpn_bool lock, int region){}
void nmg_Graphics_Null::lockTextureTransformMode(vrpn_bool lock, int region){}
void nmg_Graphics_Null::lockStride(vrpn_bool lock, int region){}

void nmg_Graphics_Null::getLightDirection (q_vec_type *) const {}

int nmg_Graphics_Null::getHandColor (void) const {
  return 0;
}

int nmg_Graphics_Null::getSpecularity (void) const {
  return 0;
}

const double * nmg_Graphics_Null::getMinColor (void) const {
  return NULL;
}

const double * nmg_Graphics_Null::getMaxColor (void) const {
  return NULL;
}
