/*===3rdtech===
  Copyright (c) 2000 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#include "nmg_GraphicsImpl.h"

#include <GL/gl.h>
#if defined(sgi) || defined(_WIN32)
#include <GL/glu.h>
#endif
#include <v.h>

#include <Position.h>
#include <PPM.h>
#include <BCPlane.h>
#include <BCGrid.h>
#include <nmb_Image.h>
#include <colormap.h>  // for ColorMap::lookup()

#include <nmb_Dataset.h>
#include <nmb_Globals.h>

#include "graphics.h"
#include "openGL.h"  // for check_extension(), display_lists_in_x
#include "globjects.h"  // for replaceDefaultObjects()
#include "graphics_globals.h"
#include "spm_gl.h"  // for init_vertexArray()

#include "Timer.h"

// M_PI not defined for VC++, for some reason. 
#ifndef M_PI
#define M_PI		3.14159265358979323846
#endif

#undef min  // XXX this is kind of a hack; should have a better fix
#define min(a,b) ((a)<(b)?(a):(b))

#undef max  // XXX this is kind of a hack; should have a better fix
#define max(a,b) ((a)>(b)?(a):(b))

#define CHECK(a) if (a == -1) return -1
#define CHECKF(a,b) if (a == -1) { fprintf(stderr, "Error: %s\n", b); return -1; }

nmg_Graphics_Implementation::nmg_Graphics_Implementation(
    nmb_Dataset * data,
    const int minColor [3],
    const int maxColor [3],
    const char * rulergridName,
    vrpn_Connection * connection,
    unsigned int portNum)

  : nmg_Graphics (connection, "nmg Graphics Implementation GL"),
    d_dataset (data),
    d_displayIndexList (new v_index [NUM_USERS]),
    d_textureTransformMode(RULERGRID_COORD)
{
    if (d_dataset == NULL) {
        grid_size_x = 12;
        grid_size_y = 12;
    } else {
        grid_size_x = d_dataset->inputGrid->numX();
        grid_size_y = d_dataset->inputGrid->numX();
    }

    //fprintf(stderr,
    //"In nmg_Graphics_Implementation::nmg_Graphics_Implementation()\n");

    int i;

    g_inputGrid = data->inputGrid;
    strcpy(g_opacityPlaneName, data->opacityPlaneName->string());

    if (d_dataset == NULL) {
        g_inputGrid = NULL;
        strcpy(g_alphaPlaneName, "none");
        strcpy(g_colorPlaneName, "none");
        strcpy(g_contourPlaneName, "none");
        strcpy(g_heightPlaneName, "none");
    } else {
        g_inputGrid = data->inputGrid;
        strcpy(g_alphaPlaneName, data->alphaPlaneName->string());
        strcpy(g_colorPlaneName, data->colorPlaneName->string());
        strcpy(g_contourPlaneName, data->contourPlaneName->string());
        strcpy(g_heightPlaneName, data->heightPlaneName->string());
    }

#ifdef FLOW
    g_data_tex_size = max(grid_size_x, grid_size_y);
    g_data_tex_size = (int) pow(2, ceil(log2(g_data_tex_size)));
#endif

    /* initialize graphics  */
    //printf("Initializing graphics...\n");

    if ( v_open() != V_OK ) {
        exit(V_ERROR);
    }

    initDisplays();

    // make sure everything is going into the right gl context
    v_gl_set_context_to_vlib_window();
    
    /* Set up the viewing info */
  
    /* Set initial user mode */
    for (i = 0; i < NUM_USERS; i++) {
        g_user_mode[i] = USER_GRAB_MODE;
    }
    /* set up user and object trees     */
    //printf("Creating the world...\n");
    v_create_world(NUM_USERS, d_displayIndexList);

    /* Set the background color to light blue */
    glClearColor(0.3, 0.3, 0.7,  0.0);
    
    setMinColor(minColor);
    setMaxColor(maxColor);
    
    initialize_globjects(NULL);  // load default font
    
    /* user routine to define user objects and override default objects */
    replaceDefaultObjects();
    
    v_replace_drawfunc(i, V_WORLD, draw_world);
    v_replace_lightingfunc(i, setup_lighting);
    
    /********************************************************************/
    /* Build the display lists we'll need to draw the data, etc */
    /********************************************************************/
    
    //fprintf(stderr,"Building display lists...\n");
    
    /* added by qliu for texture mapping*/
    /*initialize the texture mapping*/
    // Texture mapping is not enabled here, but when a data set is mapped*/

  if (rulergridName) {
    FILE *infile = fopen(rulergridName, "rb");
    if (!infile) {
      fprintf(stderr, "nmg_GraphicsImplementation:  no such file: %s\n",
		rulergridName);
      return;
    }
    g_rulerPPM = new PPM (infile);
    if (!g_rulerPPM) {
      fprintf(stderr, "nmg_GraphicsImplementation:  Out of memory.\n");
      return;
    }
    if (!g_rulerPPM->valid) {
      fprintf(stderr, "Cannot read rulergrid PPM %s.\n", rulergridName);

      delete g_rulerPPM;  // plug memory leak?
      g_rulerPPM = NULL;  // show default ruler image instead
    }
  }

  initializeTextures();
  setupMaterials(); // this needs to come after initializeTextures because
		// shader initialization depends on texture ids


  const GLubyte * exten;
  exten = glGetString(GL_EXTENSIONS);

#if ( defined(linux) || defined(hpux) || defined(_WIN32) )
  // RMT The accelerated X server seems to be telling us that we have
  // vertex arrays, but then they are not drawn; ditto for MesaGL on
  // HPUX
  g_VERTEX_ARRAY = 0;
#else
  g_VERTEX_ARRAY = check_extension(exten); //"EXT_vertex_array"
#endif

//    if (g_VERTEX_ARRAY) {
//      fprintf(stderr,"Vertex Array extension used.\n");
//    } else {
//       fprintf(stderr,"Vertex Array extension not supported.\n");
//    }

    // Even though we may not be using the vertex array extension, we still
    // use the vertex array to cache calculated normals
    if (!init_vertexArray(grid_size_x,
			  grid_size_y) ) {
      fprintf(stderr," init_vertexArray: out of memory.\n");
      exit(0);
    }

  /* Build display lists for surface scanning in X fastest, since
   * that is the way the SPM will start scanning.
   * There is one list for each row of data points except for
   * the last.  Since these are for rows scanning in X fastest,
   * we have one per Y index. */
  nmb_PlaneSelection planes; planes.lookup(data);
  if (build_grid_display_lists(planes, 1, &grid_list_base,
                                &num_grid_lists, g_minColor, g_maxColor)) {
     fprintf(stderr,"ERROR: Could not build grid display lists\n");
     d_dataset->done = 1;
   }

  g_positionList = new Position_list;
  g_positionListL = new Position_list;
  g_positionListR = new Position_list;

  // This section initializes the user mode to GRAB and sets an initial
  // position for the aimLine...
  BCPlane* plane = dataset->inputGrid->getPlaneByName
    (dataset->heightPlaneName->string());
  decoration->aimLine.moveTo(plane->minX(), plane->maxY(), plane);
  init_world_modechange( USER_GRAB_MODE, 0 );


  if (!connection) return;

  connection->register_handler(d_resizeViewport_type,
                               handle_resizeViewport,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_loadRulergridImage_type,
                               handle_loadRulergridImage,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_causeGridRedraw_type,
                               handle_causeGridRedraw,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_causeGridRebuild_type,
                               handle_causeGridRebuild,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_enableChartjunk_type,
                               handle_enableChartjunk,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_enableFilledPolygons_type,
                               handle_enableFilledPolygons,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_enableSmoothShading_type,
                               handle_enableSmoothShading,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_enableTrueTip_type,
                               handle_enableTrueTip,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_setAdhesionSliderRange_type,
                               handle_setAdhesionSliderRange,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_setAlphaColor_type,
                               handle_setAlphaColor,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_setAlphaSliderRange_type,
                               handle_setAlphaSliderRange,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_setBumpMapName_type,
                               handle_setBumpMapName,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_setColorMapDirectory_type,
                               handle_setColorMapDirectory,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_setColorMapName_type,
                               handle_setColorMapName,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_setColorMinMax_type,
                               handle_setColorMinMax,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_setDataColorMinMax_type,
                               handle_setDataColorMinMax,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_setOpacitySliderRange_type,
			       handle_setOpacitySliderRange,
			       this, vrpn_ANY_SENDER);
  connection->register_handler(d_setComplianceSliderRange_type,
                               handle_setComplianceSliderRange,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_setContourColor_type,
                               handle_setContourColor,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_setContourWidth_type,
                               handle_setContourWidth,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_setFrictionSliderRange_type,
                               handle_setFrictionSliderRange,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_setBumpSliderRange_type,
                               handle_setBumpSliderRange,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_setBuzzSliderRange_type,
                               handle_setBuzzSliderRange,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_setHandColor_type,
                               handle_setHandColor,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_setHatchMapName_type,
                               handle_setHatchMapName,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_setAlphaPlaneName_type,
                               handle_setAlphaPlaneName,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_setColorPlaneName_type,
                               handle_setColorPlaneName,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_setContourPlaneName_type,
                               handle_setContourPlaneName,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_setOpacityPlaneName_type,
			       handle_setOpacityPlaneName,
			       this, vrpn_ANY_SENDER);
  connection->register_handler(d_setHeightPlaneName_type,
                               handle_setHeightPlaneName,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_setMinColor_type,
                               handle_setMinColor,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_setMaxColor_type,
                               handle_setMaxColor,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_setPatternMapName_type,
                               handle_setPatternMapName,
                               this, vrpn_ANY_SENDER);
/*
  connection->register_handler(d_enableRulergrid_type,
                               handle_enableRulergrid,
                               this, vrpn_ANY_SENDER);
*/

  connection->register_handler(d_setRulergridAngle_type,
                               handle_setRulergridAngle,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_setRulergridColor_type,
                               handle_setRulergridColor,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_setRulergridOffset_type,
                               handle_setRulergridOffset,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_setNullDataAlphaToggle_type,
			       handle_setNullDataAlphaToggle,
			       this, vrpn_ANY_SENDER);
  connection->register_handler(d_setRulergridOpacity_type,
                               handle_setRulergridOpacity,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_setRulergridScale_type,
                               handle_setRulergridScale,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_setRulergridWidths_type,
                               handle_setRulergridWidths,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_setSpecularity_type,
                               handle_setSpecularity,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_setSpecularColor_type,
                               handle_setSpecularColor,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_setDiffusePercent_type,
			       handle_setDiffusePercent,
			       this, vrpn_ANY_SENDER);
  connection->register_handler(d_setSurfaceAlpha_type,
                               handle_setSurfaceAlpha,
                               this, vrpn_ANY_SENDER); 
  connection->register_handler(d_setSphereScale_type,
                               handle_setSphereScale,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_setTesselationStride_type,
                               handle_setTesselationStride,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_setTextureMode_type,
                               handle_setTextureMode,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_setTextureScale_type,
                               handle_setTextureScale,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_setTrueTipScale_type,
                               handle_setTrueTipScale,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_setUserMode_type,
                               handle_setUserMode,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_setLightDirection_type,
                               handle_setLightDirection,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_resetLightDirection_type,
                               handle_resetLightDirection,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_addPolylinePoint_type,
                               handle_addPolylinePoint,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_emptyPolyline_type,
                               handle_emptyPolyline,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_setScanlineEndpoints_type,
				handle_setScanlineEndpoints,
				this, vrpn_ANY_SENDER);
  connection->register_handler(d_displayScanlinePosition_type,
				handle_displayScanlinePosition,
				this, vrpn_ANY_SENDER);
  connection->register_handler(d_positionAimLine_type,
                               handle_positionAimLine,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_positionRubberCorner_type,
                               handle_positionRubberCorner,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_positionSweepLine_type,
                               handle_positionSweepLine,
                               this, vrpn_ANY_SENDER);

  connection->register_handler(d_addPolySweepPoints_type,
                               handle_addPolySweepPoints,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_setRubberSweepLineStart_type,
                               handle_setRubberSweepLineStart,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_setRubberSweepLineEnd_type,
                               handle_setRubberSweepLineEnd,
                               this, vrpn_ANY_SENDER);

  connection->register_handler(d_setRubberLineStart_type,
                               handle_setRubberLineStart,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_setRubberLineEnd_type,
                               handle_setRubberLineEnd,
                               this, vrpn_ANY_SENDER);

  connection->register_handler(d_positionSphere_type,
                               handle_positionSphere,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_enableCollabHand_type,
			       handle_enableCollabHand,
			       this, vrpn_ANY_SENDER);
  connection->register_handler(d_setCollabHandPos_type,
			       handle_setCollabHandPos,
			       this, vrpn_ANY_SENDER);
  connection->register_handler(d_setCollabMode_type,
			       handle_setCollabMode,
			       this, vrpn_ANY_SENDER);

  // Realign Textures Handlers:
  connection->register_handler( d_createRealignTextures_type,
				handle_createRealignTextures,
				this, vrpn_ANY_SENDER);
  connection->register_handler( d_setRealignTextureSliderRange_type,
				handle_setRealignTextureSliderRange,
				this, vrpn_ANY_SENDER);
  connection->register_handler( d_setRealignTexturesConversionMap_type,
				handle_setRealignTexturesConversionMap,
				this, vrpn_ANY_SENDER);
  connection->register_handler( d_computeRealignPlane_type,
				handle_computeRealignPlane,
				this, vrpn_ANY_SENDER);
/*
  connection->register_handler( d_enableRealignTextures_type,
				handle_enableRealignTextures,
				this, vrpn_ANY_SENDER);
*/
  connection->register_handler( d_translateTextures_type,
				handle_translateTextures,
				this, vrpn_ANY_SENDER);
  connection->register_handler( d_scaleTextures_type,
				handle_scaleTextures,
				this, vrpn_ANY_SENDER);
  connection->register_handler( d_shearTextures_type,
				handle_shearTextures,
				this, vrpn_ANY_SENDER);
  connection->register_handler( d_rotateTextures_type,
				handle_rotateTextures,
				this, vrpn_ANY_SENDER);
  connection->register_handler( d_setTextureCenter_type,
				handle_setTextureCenter,
				this, vrpn_ANY_SENDER);
  connection->register_handler( d_updateTexture_type,
				handle_updateTexture,
				this, vrpn_ANY_SENDER);
/*
  connection->register_handler( d_enableRegistration_type,
				handle_enableRegistration,
				this, vrpn_ANY_SENDER);
*/
  connection->register_handler( d_setTextureTransform_type,
				handle_setTextureTransform,
				this, vrpn_ANY_SENDER);
  connection->register_handler( d_setViewTransform_type,
				handle_setViewTransform,
				this, vrpn_ANY_SENDER);
  connection->register_handler( d_createScreenImage_type,
				handle_createScreenImage,
				this, vrpn_ANY_SENDER);

}

nmg_Graphics_Implementation::~nmg_Graphics_Implementation (void) {

  int i;

  for (i = 0; i < NUM_USERS; i++)
    v_close_display(d_displayIndexList[i]);

#ifdef FLOW
  // free the dynamic memory allocated for date textures
  if (bump_data) {
    delete [] bump_data;
  }
  if (pattern_data) {
    delete [] pattern_data;
  }
  if (hatch_data) {
    delete [] hatch_data;
  }
#endif
  if (sem_data) {
    delete [] sem_data;
  }

  v_close();
}



void nmg_Graphics_Implementation::mainloop (void) {

  
//fprintf(stderr, "nmg_Graphics_Implementation::mainloop().\n");

  // BUG:  do we have one frame of latency here?

  // make sure gl calls are directed to the right context
  v_gl_set_context_to_vlib_window();

  switch (decoration->mode) {
    case nmb_Decoration::IMAGE:
      glClearColor(0.3, 0.3, 0.7, 0.0);  // blue
      break;
    case nmb_Decoration::FEEL:
      glClearColor(0.4, 0.4, 0.2, 0.0);  // yellow
      break;
    case nmb_Decoration::MODIFY:
      glClearColor(0.7, 0.3, 0.3, 0.0);  // red
      break;
    case nmb_Decoration::SCANLINE:
      glClearColor(0.3, 0.7, 0.3, 0.0);  // green (kind of looks like some
      break;				// green tea ice-cream I once had)
    default:
      fprintf(stderr, "Illegal or unknown value for mode of microscope "
                      "to display in v_draw_objects().\n");
      break;
  }


  if ( (grid_size_x != d_dataset->inputGrid->numX()) ||
       (grid_size_y != d_dataset->inputGrid->numY())    ) {
    causeGridRebuild();
  }

  TIMERVERBOSE(5, mytimer, "GImainloop: nmg_Graphics::mainloop");

  nmg_Graphics::mainloop();

  TIMERVERBOSE(5, mytimer, "GImainloop: end nmg_Graphics::mainloop");
  TIMERVERBOSE(5, mytimer, "GImainloop: v_update_displays");

  v_update_displays(d_displayIndexList);

  TIMERVERBOSE(5, mytimer, "GImainloop: end v_update_displays");

}

/** If we load a different stream file or connect to a new AFM,
 * graphics needs to start watching a new dataset.
 */
void nmg_Graphics_Implementation::changeDataset( nmb_Dataset * data)
{
  d_dataset = data;
  grid_size_x = d_dataset->inputGrid->numX();
  grid_size_y = d_dataset->inputGrid->numX();

  g_inputGrid = data->inputGrid;
  strcpy(g_alphaPlaneName, data->alphaPlaneName->string());
  strcpy(g_colorPlaneName, data->colorPlaneName->string());
  strcpy(g_contourPlaneName, data->contourPlaneName->string());
  strcpy(g_heightPlaneName, data->heightPlaneName->string());

#ifdef FLOW
    g_data_tex_size = max(grid_size_x, grid_size_y);
    g_data_tex_size = (int) pow(2, ceil(log2(g_data_tex_size)));
#endif

    causeGridRebuild();
 
}

// functions to replace code in microscape.c - AAS
void nmg_Graphics_Implementation::resizeViewport(int width, int height) {
//fprintf(stderr, "nmg_Graphics_Implementation::resizeViewport().\n");
  v_display_type *displayPtr;
  v_viewport_type *windowPtr;

  // DEBUG
  //fprintf(stderr, "DEBUG nmg_Graphics_Impl::resizeViewport, width %d, height %d\n",
  //  width, height);
  displayPtr=&v_display_table[d_displayIndexList[0]];
  windowPtr=&(displayPtr->viewports[0]);
  windowPtr->fbExtents[0] = width;
  windowPtr->fbExtents[1] = height;

}

void nmg_Graphics_Implementation::getViewportSize(int *width, int *height) {
   *width  = v_display_table[d_displayIndexList[0]].viewports[0].fbExtents[0];
   *height = v_display_table[d_displayIndexList[0]].viewports[0].fbExtents[1];
}

void nmg_Graphics_Implementation::getDisplayPosition (q_vec_type &ll,
        q_vec_type &ul, q_vec_type &ur)
{
//fprintf(stderr, "nmg_Graphics_Implementation::getDisplayPosition().\n");
    q_vec_copy(ll,
        v_display_table[d_displayIndexList[0]].viewports[0].screenLowerLeft);
    q_vec_copy(ul,
        v_display_table[d_displayIndexList[0]].viewports[0].screenUpperLeft);
    q_vec_copy(ur,
        v_display_table[d_displayIndexList[0]].viewports[0].screenUpperRight);
// DEBUG
//   fprintf(stderr, "g>DEBUG nmg_Graphics_Impl::getDisplayPosition "
//   "ll %f %f %f ul %f %f %f ur %f %f %f\n",
// 	  ll[0], ll[1], ll[2], 
// 	  ul[0], ul[1], ul[2], 
// 	  ur[0], ur[1], ur[2]);
}
// end functions to replace stuff in microscape.c

void nmg_Graphics_Implementation::loadRulergridImage (const char * name) {
//fprintf(stderr, "nmg_Graphics_Implementation::loadRulergridImage().\n");
  if (!name) {
    return;
  }
  FILE *infile = fopen(name, "rb");
  if (!infile) {
    fprintf(stderr, "nmg_Graphics_Implementation: can't find file: %s\n",
	name);
  }
  g_rulerPPM = new PPM (infile);
  if (!g_rulerPPM->valid) {
    fprintf(stderr, "Cannot read rulergrid PPM %s.\n", name);
  } else {
    makeAndInstallRulerImage(g_rulerPPM);
  }
}


// Tell how to index a given element.  Parameters are y,x,color
#define texel(j,i,c) ( (c) + 4 * ( (i) + (j) * texture_size))

void nmg_Graphics_Implementation::makeAndInstallRulerImage(PPM *myPPM)
{
  // code taken from graphics.c::makeAndInstallRulerImage():
  int x,y;
  int r,g,b;

  // make sure gl calls are directed to the right context
  v_gl_set_context_to_vlib_window();

  // texture_size is the length of one side of the square texture.
  // Find out the smallest power-of-2 texture region we can use.
  // Remember that float->int conversion truncates, so add 0.5 for rounding
  // Make sure it is not too big.
  int ts_orig_ = max(myPPM->nx, myPPM->ny);
  const int texture_size = int (0.5 + pow (2, ceil(log(ts_orig_)/log(2))));

  if (texture_size > 512) {
      fprintf (stderr,
               "Using ruler texture of size %dx%d, "
               "which is larger than previously allowed\n",
               texture_size, texture_size);
  }
  
#if 0 // old error message for static-sized array
  if (texture_size > 512) {
        fprintf(stderr,"Not enough space for %dx%d ruler texture\n",
                texture_size, texture_size);
        return;
  }
#endif

  // multiply by 4 so we can store 4 values at each texel
  GLubyte * texture = new GLubyte [4 * texture_size * texture_size];

  // Fill the whole texture with black.  This will make a border around
  // any area not filled by the PPM file.
  for (x = 0; x < texture_size; x++) {
    for (y = 0; y < texture_size; y++) {
        if (g_tex_blend_func[RULERGRID_TEX_ID] == GL_MODULATE) {
          texture[ texel( y, x, 0) ] = 255;
          texture[ texel( y, x, 1) ] = 255;
          texture[ texel( y, x, 2) ] = 255;
          texture[ texel( y, x, 3) ] = 255;
        } else if (g_tex_blend_func[RULERGRID_TEX_ID] == GL_BLEND) {
          texture[ texel( y, x, 0) ] = 0;
          texture[ texel( y, x, 1) ] = 0;
          texture[ texel( y, x, 2) ] = 0;
          texture[ texel( y, x, 3) ] = 255;
        } else if (g_tex_blend_func[RULERGRID_TEX_ID] == GL_DECAL){
          texture[ texel( y, x, 3) ] = 0;
        } else {
          texture[ texel( y, x, 0) ] = 255;
          texture[ texel( y, x, 1) ] = 255;
          texture[ texel( y, x, 2) ] = 255;
          texture[ texel( y, x, 3) ] = 255;
        }
    }
  }

  // Fill in the part of the texture that the PPM file covers
  // Invert Y because the coordinate system in the PPM file starts
  // in the upper left corner, and our coordinate system in the lower
  // left.
  for (x = 0; x < myPPM->nx; x++) {
    for (y = 0; y < myPPM->ny; y++) {
//printf("XXX Filling %3d,%3d (%ld)\n", x, (myPPM->ny-1)-y,
//              (long)texel((myPPM->ny-1)-y,x,0));
        myPPM->Tellppm(x,y, &r, &g, &b);
        if (g_tex_blend_func[RULERGRID_TEX_ID] == GL_BLEND) {
          texture[ texel( (myPPM->ny-1)-y, x, 0) ] = 
		(GLubyte)((float)r*g_ruler_opacity/255.0);
          texture[ texel( (myPPM->ny-1)-y, x, 1) ] = 
		(GLubyte)((float)g*g_ruler_opacity/255.0);
          texture[ texel( (myPPM->ny-1)-y, x, 2) ] = 
		(GLubyte)((float)b*g_ruler_opacity/255.0);
          texture[ texel( (myPPM->ny-1)-y, x, 3) ] = (GLubyte) 255;
        } else if (g_tex_blend_func[RULERGRID_TEX_ID] == GL_MODULATE) {
          texture[ texel( (myPPM->ny-1)-y, x, 0) ] =
                (GLubyte)((float)r*g_ruler_opacity/255.0);
          texture[ texel( (myPPM->ny-1)-y, x, 1) ] =
                (GLubyte)((float)g*g_ruler_opacity/255.0);
          texture[ texel( (myPPM->ny-1)-y, x, 2) ] =
                (GLubyte)((float)b*g_ruler_opacity/255.0);
          texture[ texel( (myPPM->ny-1)-y, x, 3) ] = (GLubyte) 255;
        } else { //if (g_tex_blend_func[RULERGRID_TEX_ID] == GL_DECAL){
          texture[ texel( (myPPM->ny-1)-y, x, 3) ] = (GLubyte)g_ruler_opacity;
          texture[ texel( (myPPM->ny-1)-y, x, 0) ] = r;
          texture[ texel( (myPPM->ny-1)-y, x, 1) ] = g;
          texture[ texel( (myPPM->ny-1)-y, x, 2) ] = b;
        }
    }
  }

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  glBindTexture(GL_TEXTURE_2D, tex_ids[RULERGRID_TEX_ID]);

  glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
  glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
  glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

#ifdef  FLOW
  /**********************-_________________________
    glTexImage2D(GL_TEXTURE_2D, 0, 4,
                 texture_size, texture_size,
                 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 (const GLvoid*)texture);
    if (glGetError()!=GL_NO_ERROR) {
      printf(" Error making ruler texture.\n");
    }
_______________________________********************/
#endif

#if defined(sgi) || defined(_WIN32)
  // Build the texture map and set the mode for 2D textures
  if (gluBuild2DMipmaps(GL_TEXTURE_2D,4, texture_size, texture_size,
                        GL_RGBA, GL_UNSIGNED_BYTE, texture) != 0) {
           printf(" Error making mipmaps, using texture instead.\n");
           glTexImage2D(GL_TEXTURE_2D, 0, 4,
                        texture_size, texture_size,
                        0, GL_RGBA, GL_UNSIGNED_BYTE,
                        texture);
           if (glGetError()!=GL_NO_ERROR) {
                printf(" Error making ruler texture.\n");
           }
  }
#endif
  g_tex_installed_width[RULERGRID_TEX_ID] = texture_size;
  g_tex_installed_height[RULERGRID_TEX_ID] = texture_size;
  g_tex_image_width[RULERGRID_TEX_ID] = myPPM->nx;
  g_tex_image_height[RULERGRID_TEX_ID] = myPPM->ny;

  delete [] texture;
}

void nmg_Graphics_Implementation::causeGridReColor (void) {
  g_just_color = 1;
  causeGridRedraw();
}


void nmg_Graphics_Implementation::causeGridRedraw (void) {
  //fprintf(stderr, "nmg_Graphics_Implementation::causeGridRedraw().\n");
  BCPlane * plane = g_inputGrid->getPlaneByName(g_heightPlaneName);

  d_dataset->range_of_change.ChangeAll();
  if (plane) {
    decoration->red.normalize(plane);
    decoration->green.normalize(plane);
    decoration->blue.normalize(plane);
    decoration->aimLine.normalize(plane);
  }
}

void nmg_Graphics_Implementation::causeGridRebuild (void) {
//fprintf(stderr, "nmg_Graphics_Implementation::causeGridRebuild().\n");

  g_just_color = 0;
  // Rebuilds the texture coordinate array:
  nmb_PlaneSelection planes;  planes.lookup(d_dataset);

  // Even though we may not be using the vertex array extension, we still
  // use the vertex array to cache calculated normals
  if (!init_vertexArray(d_dataset->inputGrid->numX(),
                          d_dataset->inputGrid->numY()) ) {
          fprintf(stderr," init_vertexArray: out of memory.\n");
          exit(0);
   }

  grid_size_x = d_dataset->inputGrid->numX();
  grid_size_y = d_dataset->inputGrid->numX();

  if (build_grid_display_lists(planes,
                             display_lists_in_x, &grid_list_base,
                             &num_grid_lists, g_minColor, g_maxColor)) {
    fprintf(stderr,
          "nmg_Graphics_Implementation::causeGridRebuild():  "
          "Couldn't build grid display lists\n");
    d_dataset->done = V_TRUE;
  }
}

void nmg_Graphics_Implementation::enableChartjunk (int on) {
//fprintf(stderr, "nmg_Graphics_Implementation::enableChartjunk().\n");
  if ( g_config_chartjunk == on )
    return;
  g_config_chartjunk = on;
  if ( on == 1 ) {
    init_world_modechange( USER_GRAB_MODE, 0);
  }
  else if ( on == 0 ) {
    clear_world_modechange( USER_MEASURE_MODE, 0);
    clear_world_modechange( USER_GRAB_MODE, 0);
  }
}

void nmg_Graphics_Implementation::enableFilledPolygons (int on) {
//fprintf(stderr, "nmg_Graphics_Implementation::enableFilledPolygons().\n");
  g_config_filled_polygons = on;
}

void nmg_Graphics_Implementation::enableSmoothShading (int on) {
//fprintf(stderr, "nmg_Graphics_Implementation::enableSmoothShading().\n");
  g_config_smooth_shading = on;
}

void nmg_Graphics_Implementation::enableTrueTip (int on) {
//fprintf(stderr, "nmg_Graphics_Implementation::enableTrueTip().\n");
  g_config_trueTip = on;
  fprintf(stderr, "Set g_config_trueTip to %d.\n", on);
}




void nmg_Graphics_Implementation::setAdhesionSliderRange (float low,
                                                          float high) {
//fprintf(stderr, "nmg_Graphics_Implementation::setAdhesionSliderRange().\n");
  g_adhesion_slider_min = low;
  g_adhesion_slider_max = high;
}

void nmg_Graphics_Implementation::setAlphaColor (float r, float g, float b) {
//fprintf(stderr, "nmg_Graphics_Implementation::setAlphaColor().\n");
  g_alpha_r = r;
  g_alpha_g = g;
  g_alpha_b = b;
  makeCheckImage();
  buildAlphaTexture();
}

void nmg_Graphics_Implementation::setAlphaSliderRange (float low, float high) {
//fprintf(stderr, "nmg_Graphics_Implementation::setAlphaSliderRange().\n");
  g_alpha_slider_min = low;
  g_alpha_slider_max = high;
  causeGridRedraw();
}

void nmg_Graphics_Implementation::setBumpMapName (const char * /*name*/)
{
//fprintf(stderr, "nmg_Graphics_Implementation::setBumpMapName().\n");

#ifdef FLOW
  BCPlane * plane = g_inputGrid->getPlaneByName(name);

  double value;
  GLubyte c;
  int x, y;

  // make sure gl calls are directed to the right context
  v_gl_set_context_to_vlib_window();

  if (plane) {
    setTextureMode(BUMPMAP, RULERGRID_COORD);

    for (x = 0; x < plane->numX(); x++)
      for (y = 0; y < plane->numY(); y++) {
        value = (plane->value(x, y) - g_friction_slider_min) /
                 (g_friction_slider_max - g_friction_slider_min);
        value = min(1.0, value);
        value = max(0.0, value);
        c = (GLubyte)((int) (value * 255.0 + 0.5));

        bump_data[(y * g_data_tex_size + x) * 3] = (GLubyte) c;
        bump_data[(y * g_data_tex_size + x) * 3 + 1] = (GLubyte) c;
        bump_data[(y * g_data_tex_size + x) * 3 + 2] = (GLubyte) c;
      }

    shader_mask = shader_mask | BUMP_BIT;

    // bind active mask
    glBoundMaterialiEXT(nM_shader,
                        glGetMaterialParameterNameEXT("active_mask"),
                        shader_mask);

    glBindTexture(GL_TEXTURE_2D, shader_tex_ids[BUMP_DATA_TEX_ID]);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, g_data_tex_size, g_data_tex_size,
                 0, GL_RGB, GL_BYTE, (const GLvoid *) bump_data);
  } else {
    for(x = 0; x < g_data_tex_size; x++)
      for(y = 0; y < g_data_tex_size; y++) {
        bump_data[(y * g_data_tex_size + x) * 3] = (GLubyte) 0;
        bump_data[(y * g_data_tex_size + x) * 3 + 1] = (GLubyte) 0;
        bump_data[(y * g_data_tex_size + x) * 3 + 2] = (GLubyte) 0;
      }
    shader_mask = shader_mask & (~BUMP_BIT);

    // bind active mask
    glBoundMaterialiEXT(nM_shader,
                        glGetMaterialParameterNameEXT("active_mask"),
                        shader_mask);

    glBindTexture(GL_TEXTURE_2D, shader_tex_ids[BUMP_DATA_TEX_ID]);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, g_data_tex_size, g_data_tex_size, 0,
                 GL_RGB, GL_BYTE, (const GLvoid *) bump_data);
  }

#endif

}

void nmg_Graphics_Implementation::setColorMapDirectory (const char * dir) {
//fprintf(stderr, "nmg_Graphics_Implementation::setColorMapDirectory().\n");
  if (g_colorMapDir) {
    delete [] g_colorMapDir;
  }
  if (!dir) {
    g_colorMapDir = NULL;
    return;
  }
  g_colorMapDir = new char [1 + strlen(dir)];

  strcpy(g_colorMapDir, dir);
}

void nmg_Graphics_Implementation::setTextureDirectory (const char * dir) {
//fprintf(stderr, "nmg_Graphics_Implementation::setTextureDirectory().\n");
  if (g_textureDir) {
    delete [] g_textureDir;
  }
  if (!dir) {
    g_textureDir = NULL;
    return;
  }
  g_textureDir = new char [1 + strlen(dir)];
  strcpy(g_textureDir, dir);
}

void nmg_Graphics_Implementation::setColorMapName (const char * name) {
//fprintf(stderr, "nmg_Graphics_Implementation::setColorMapName().\n");

  if (strcmp(name, "none") == 0) {
    g_curColorMap = NULL;
  } else {
    g_colorMap.load_from_file(name, g_colorMapDir);
    g_curColorMap = &g_colorMap;
  }
  causeGridReColor();
}

void nmg_Graphics_Implementation::setColorMinMax (float low, float high) {
//fprintf(stderr, "nmg_Graphics_Implementation::setColorMinMax().\n");
  if ( (g_color_min != low) || (g_color_max != high) ) {
    g_color_min = low;
    g_color_max = high;
    causeGridReColor();
  }
}

void nmg_Graphics_Implementation::setDataColorMinMax (float low, float high) {
//fprintf(stderr, "nmg_Graphics_Implementation::setDataColorMinMax().\n");
  if ( (g_data_min != low) || (g_data_max != high) ) {
    g_data_min = low;
    g_data_max = high;
    causeGridReColor();
  }
}

void nmg_Graphics_Implementation::setOpacitySliderRange (float low, float high) {
  g_opacity_slider_min = low;
  g_opacity_slider_max = high;
  causeGridRedraw();
}

void nmg_Graphics_Implementation::setComplianceSliderRange (float low, float high) {
//fprintf(stderr, "nmg_Graphics_Implementation::setComplianceSliderRange().\n");
  g_compliance_slider_min = low;
  g_compliance_slider_max = high;
  causeGridRedraw();
}

void nmg_Graphics_Implementation::setContourColor (int r, int g, int b) {
//fprintf(stderr, "nmg_Graphics_Implementation::setContourColor().\n");
  g_contour_r = r;
  g_contour_g = g;
  g_contour_b = b;
  buildContourTexture();
  causeGridRedraw();
}

void nmg_Graphics_Implementation::setFrictionSliderRange (float low,
                                                          float high) {
//fprintf(stderr, "nmg_Graphics_Implementation::setFrictionSliderRange().\n");
  g_friction_slider_min = low;
  g_friction_slider_max = high;
  causeGridRedraw();
}

void nmg_Graphics_Implementation::setBumpSliderRange (float low,
                                                          float high) {
//fprintf(stderr, "nmg_Graphics_Implementation::setBumpSliderRange().\n");
  g_bump_slider_min = low;
  g_bump_slider_max = high;
  causeGridRedraw();
}

void nmg_Graphics_Implementation::setBuzzSliderRange (float low,
                                                          float high) {
//fprintf(stderr, "nmg_Graphics_Implementation::setBuzzSliderRange().\n");
  g_buzz_slider_min = low;
  g_buzz_slider_max = high;
  causeGridRedraw();
}

void nmg_Graphics_Implementation::setHandColor (int c) {
    //fprintf(stderr, "nmg_Graphics_Implementation::setHandColor().\n");
  g_hand_color = c;
}

void nmg_Graphics_Implementation::setIconScale (float scale) {
//fprintf(stderr, "nmg_Graphics_Implementation::setIconScale().\n");
  g_icon_scale = scale;
}

void nmg_Graphics_Implementation::enableCollabHand (vrpn_bool on) {
  ::enableCollabHand(on);  // from globjects.c
  g_draw_collab_hand = on;
}

void nmg_Graphics_Implementation::setCollabHandPos(double pos[], double quat[])
{
    //fprintf(stderr, "nmg_Graphics_Implementation::setCollabHandPos().\n");
  int i;
  for (i = 0; i < 3; i++) {
    g_collabHandPos[i] = pos[i];
    g_collabHandQuat[i] = quat[i];
  }
  g_collabHandQuat[3] = quat[3];

  g_position_collab_hand = 1;
}

void nmg_Graphics_Implementation::setCollabMode(int mode)
{
    //fprintf(stderr, "nmg_Graphics_Implementation::setCollabMode().\n");
  g_collabMode = mode;
}

void nmg_Graphics_Implementation::setHatchMapName (const char * /*name*/)
{
//fprintf(stderr, "nmg_Graphics_Implementation::setHatchMapName().\n");

#ifdef FLOW
  BCPlane * plane = g_inputGrid->getPlaneByName(name);

  double value;
  GLubyte c;
  int x, y;

  // make sure gl calls are directed to the right context
  v_gl_set_context_to_vlib_window();

  if (plane) {
    setTextureMode(HATCHMAP, RULERGRID_COORD);

    for (x = 0; x < plane->numX(); x++)
      for (y = 0; y < plane->numY(); y++) {
        value = (plane->value(x, y) - g_adhesion_slider_min) /
                 (g_adhesion_slider_max - g_adhesion_slider_min);
        value = min(1.0, value);
        value = max(0.0, value);
        c = (GLubyte)((int) (value * 255.0 + 0.5));

        hatch_data[(y * g_data_tex_size + x) * 3] = (GLubyte) c;
        hatch_data[(y * g_data_tex_size + x) * 3 + 1] = (GLubyte) c;
        hatch_data[(y * g_data_tex_size + x) * 3 + 2] = (GLubyte) c;
      }

    shader_mask = shader_mask | HATCH_BIT;

    // bind active mask
    glBoundMaterialiEXT(nM_shader,
                        glGetMaterialParameterNameEXT("active_mask"),
                        shader_mask);

    glBindTexture(GL_TEXTURE_2D, shader_tex_ids[HATCH_DATA_TEX_ID]);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, g_data_tex_size, g_data_tex_size,
                 0, GL_RGB, GL_BYTE, (const GLvoid *) hatch_data);
  } else {
    for(x = 0; x < g_data_tex_size; x++)
      for(y = 0; y < g_data_tex_size; y++) {
        hatch_data[(y * g_data_tex_size + x) * 3] = (GLubyte) 0;
        hatch_data[(y * g_data_tex_size + x) * 3 + 1] = (GLubyte) 0;
        hatch_data[(y * g_data_tex_size + x) * 3 + 2] = (GLubyte) 0;
      }
    shader_mask = shader_mask & (~HATCH_BIT);

    // bind active mask
    glBoundMaterialiEXT(nM_shader,
                        glGetMaterialParameterNameEXT("active_mask"),
                        shader_mask);

    glBindTexture(GL_TEXTURE_2D, shader_tex_ids[HATCH_DATA_TEX_ID]);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, g_data_tex_size, g_data_tex_size, 0,
                 GL_RGB, GL_BYTE, (const GLvoid *) hatch_data);
  }

#endif  // FLOW

}


// virtual
void nmg_Graphics_Implementation::setAlphaPlaneName (const char * n) {
  strcpy(g_alphaPlaneName, n);
}

// virtual
void nmg_Graphics_Implementation::setColorPlaneName (const char * n) {
  strcpy(g_colorPlaneName, n);
}

// virtual
void nmg_Graphics_Implementation::setContourPlaneName (const char * n) {
  strcpy(g_contourPlaneName, n);
}

// virtual
void nmg_Graphics_Implementation::setHeightPlaneName (const char * n) {
  strcpy(g_heightPlaneName, n);
}

// virtual
void nmg_Graphics_Implementation::setOpacityPlaneName (const char * n) {
  strcpy(g_opacityPlaneName, n);
}


void nmg_Graphics_Implementation::setContourWidth (float x) {
//fprintf(stderr, "nmg_Graphics_Implementation::setContourWidth().\n");
  g_contour_width = x;
  buildContourTexture();
  causeGridRedraw();
}

void nmg_Graphics_Implementation::setMinColor (const double c [4]) {
//fprintf(stderr, "nmg_Graphics_Implementation::setMinColor().\n");
  memcpy(g_minColor, c, 3 * sizeof(double));
}

void nmg_Graphics_Implementation::setMaxColor (const double c [4]) {
//fprintf(stderr, "nmg_Graphics_Implementation::setMaxColor().\n");
  memcpy(g_maxColor, c, 3 * sizeof(double));
}

void nmg_Graphics_Implementation::setMinColor (const int c [4]) {
//fprintf(stderr, "nmg_Graphics_Implementation::setMinColor().\n");
  g_minColor[0] = c[0] / 255.0;
  g_minColor[1] = c[1] / 255.0;
  g_minColor[2] = c[2] / 255.0;
  //use alpha value from Opacity scrollbar
  g_minColor[3] = g_surface_alpha;
}

void nmg_Graphics_Implementation::setMaxColor (const int c [4]) {
//fprintf(stderr, "nmg_Graphics_Implementation::setMaxColor().\n");
  g_maxColor[0] = c[0] / 255.0;
  g_maxColor[1] = c[1] / 255.0;
  g_maxColor[2] = c[2] / 255.0;
  //use alpha value from Opacity scrollbar
  g_maxColor[3] = g_surface_alpha;
}

void nmg_Graphics_Implementation::setPatternMapName (const char * /*name*/)
{
//fprintf(stderr, "nmg_Graphics_Implementation::setPatternMapName().\n");

#ifdef FLOW
  BCPlane * plane = g_inputGrid->getPlaneByName(name);

  GLubyte c;
  double value;
  int x, y;

  // make sure gl calls are directed to the right context
  v_gl_set_context_to_vlib_window();

  if (plane) {
    setTextureMode(PATTERNMAP, RULERGRID_COORD);
    for (x = 0; x < plane->numX(); x++)
      for (y = 0; y < plane->numY(); y++) {
        value = (plane->value(x, y) - g_alpha_slider_min) /
                (g_alpha_slider_max - g_alpha_slider_min);
        value = min(1.0, value);
        value = max(0.0, value);
        c = (GLubyte) ((int) (value * 255.0 + 0.5));

        pattern_data[(y * g_data_tex_size + x) * 3] = c;
        pattern_data[(y * g_data_tex_size + x) * 3 + 1] = c;
        pattern_data[(y * g_data_tex_size + x) * 3 + 2] = c;
      }

    shader_mask |= PATTERN_BIT;

    // bind active mask

    glBoundMaterialiEXT(nM_shader,
                 glGetMaterialParameterNameEXT("active_mask"),
                 shader_mask);

    glBindTexture(GL_TEXTURE_2D, shader_tex_ids[PATTERN_DATA_TEX_ID]);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, g_data_tex_size, g_data_tex_size,
                 0, GL_RGB, GL_BYTE, (const GLvoid *) pattern_data);
  } else {
    for (x = 0; x < g_data_tex_size; x++)
      for (y = 0; y < g_data_tex_size; y++) {
        pattern_data[(y * g_data_tex_size + x) * 3] = 0;
        pattern_data[(y * g_data_tex_size + x) * 3 + 1] = 255;
        pattern_data[(y * g_data_tex_size + x) * 3 + 2] = 0;
      }

    shader_mask &= (~PATTERN_BIT);

    glBoundMaterialiEXT(nM_shader,
                 glGetMaterialParameterNameEXT("active_mask"),
                 shader_mask);

    glBindTexture(GL_TEXTURE_2D, shader_tex_ids[PATTERN_DATA_TEX_ID]);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, g_data_tex_size, g_data_tex_size,
                 0, GL_RGB, GL_BYTE, (const GLvoid *) pattern_data);
  }

#endif  // FLOW

  causeGridRedraw();
}

//
// Realign Texture Functions
//

//
// This function takes the name of a dataset and creates a texture map
// out of the data in the dataset. 
// The texture map is created by mapping data values to color values
// based on the conversion map selected (i.e. blackbody, inverse rainbow,...)
// Or in none is selected, it just does some simple mapping.
//
// Texture maps must be in units of powers of 2,
// So a texture map of 512x512 is allocated (hopefully this is big enough)
//   -JFJ: correction, it's now a dynamically-allocated array of any
//         power-of-2 size.
//
void nmg_Graphics_Implementation::createRealignTextures( const char *name ) {
  // inputGrid is replaced with dataImages in this function so that
  // we don't have separate functions for building textures from ppm images
  // and AFM data (these changes allow this function to do what 
  // makeAndInstallRulerImage() was used for previously)
  strcpy(g_realign_texture_name, name);
  nmb_Image *im = d_dataset->dataImages->getImageByName(name);
  if (!im) {
    fprintf(stderr, 
	"nmg_Graphics_Impl::createRealignTextures: image not found\n");
    return;
  }
//  printf("nmg_Graphics_Implementation: creating realign texture %s\n", name);

  
  int image_width = im->width();
  int image_height = im->height();

  /* values used by registration code - since non-graphics code assumes
	that the texture coordinates go from 0..1 in the texture image and not
	0..<some value smaller than one> as a result of our need to use a
	power of 2 - the different sizes need to be stored so that we can
	compute the right scaling factor to compensate when loading the texture
	transformation
  */
  int stride_x = 1, stride_y = 1;
  if (image_width > g_tex_installed_width[COLORMAP_TEX_ID]){
	stride_x = (int)floor((double)image_width/
			     (double)g_tex_installed_width[COLORMAP_TEX_ID]);
  }
  if (image_height > g_tex_installed_height[COLORMAP_TEX_ID]) {
	stride_y = (int)floor((double)image_height/
			     (double)g_tex_installed_height[COLORMAP_TEX_ID]);
  }

  g_tex_image_width[COLORMAP_TEX_ID] = image_width/stride_x;
  g_tex_image_height[COLORMAP_TEX_ID] = image_height/stride_y;

  if (image_width > g_tex_installed_width[COLORMAP_TEX_ID] ||
		image_height > g_tex_installed_height[COLORMAP_TEX_ID]) {
	fprintf(stderr, "nmg::createRealignTextures, Warning: large texture"
	 ", stride reduction: (%d,%d)/(%d,%d)->(%d,%d)\n",
	image_width, image_height, stride_x, stride_y, 
        g_tex_image_width[COLORMAP_TEX_ID],
        g_tex_image_height[COLORMAP_TEX_ID]);
  }

  float min = im->minNonZeroValue();
  float max = im->maxValue();
  int red_index, grn_index, blu_index, alph_index;
  // we leave a 1 pixel border at the edge so that texture color where there
  // is no texture gets the border color
  int border = 1;
  int nc = 4;// num channels
  for ( int j = border,j_im= border*stride_y; 
        j < g_tex_installed_height[COLORMAP_TEX_ID]-border; 
        j++,j_im+=stride_y ) {
    red_index = j*g_tex_installed_width[COLORMAP_TEX_ID]*nc + 0 + border*nc;
    grn_index = j*g_tex_installed_width[COLORMAP_TEX_ID]*nc + 1 + border*nc;
    blu_index = j*g_tex_installed_width[COLORMAP_TEX_ID]*nc + 2 + border*nc;
    alph_index = j*g_tex_installed_width[COLORMAP_TEX_ID]*nc + 3 + border*nc;
    for (int k = border,k_im= border*stride_x;
         k < g_tex_installed_width[COLORMAP_TEX_ID]-border; 
         k++, k_im+=stride_x, 
         red_index+=nc, grn_index+=nc, blu_index+=nc, alph_index+=nc ) {
      // this condition actually chops off a pixel on each side of the 
      // texture so that the clamped texture coordinates map to 
      // completely transparent pixels
      // really, we'd like to just shift the image by 1 pixel so we don't
      // lose anything but that would introduce a translation which
      // we'd need to compensate for when using the registration result
      // and I don't feel like figuring that out at the moment
      if ((j < g_tex_image_height[COLORMAP_TEX_ID]) && 
          (k < g_tex_image_width[COLORMAP_TEX_ID])) {
        // if we're inside the image region

         if (g_realign_textures_curColorMap) {
           // Map data to color based on conversion map:
           double scale = (im->getValue(k_im,j_im) -
                        g_realign_textures_slider_min) /
              (g_realign_textures_slider_max - g_realign_textures_slider_min);
           scale = min(1.0, scale);
           scale = max(0.0, scale);

           float r, g, b, a;
           g_realign_textures_curColorMap->lookup(scale, &r, &g, &b, &a);
	   realign_data[red_index] = r;
           realign_data[grn_index] = g;
           realign_data[blu_index] = b;
         } 
         else { // Otherwise simple data to greyscale color mapping 
           float val = ( im->getValue( k_im, j_im ) - min )/( max - min );
           if (val < 0.0) val = 0.0;
           realign_data[red_index] = val;
           realign_data[grn_index] = val;
           realign_data[blu_index] = val;
         }

         // Here we adjust intensity of the texture:
         // XXX should use its own variable rather than g_ruler_opacity
         if (g_tex_blend_func[COLORMAP_TEX_ID] == GL_MODULATE) {
             realign_data[red_index] += 
		(1.0- realign_data[red_index])*
		(255.0 - (float)g_ruler_opacity)/255.0;
             realign_data[grn_index] += 
		(1.0- realign_data[grn_index])*
		(255.0 - (float)g_ruler_opacity)/255.0;
             realign_data[blu_index] += 
		(1.0- realign_data[blu_index])*
		(255.0 - (float)g_ruler_opacity)/255.0;
             realign_data[alph_index] = 1.0;
         } else if (g_tex_blend_func[COLORMAP_TEX_ID] == GL_BLEND) {
             realign_data[red_index] *= (float)g_ruler_opacity/255.0;
             realign_data[grn_index] *= (float)g_ruler_opacity/255.0;
             realign_data[blu_index] *= (float)g_ruler_opacity/255.0;
             realign_data[alph_index] = 1.0;
         } else if (g_tex_blend_func[COLORMAP_TEX_ID] == GL_DECAL) {
             realign_data[alph_index]  = (float)g_ruler_opacity/255.0;
         }

      }
      else { // handles parts of texture that extend beyond image
        if (g_tex_blend_func[COLORMAP_TEX_ID] == GL_MODULATE) {
            realign_data[red_index] = 1.0;
            realign_data[grn_index] = 1.0;
            realign_data[blu_index] = 1.0;
            realign_data[alph_index] = 1.0;
        } else if (g_tex_blend_func[COLORMAP_TEX_ID] == GL_BLEND) {
	    realign_data[red_index] = 0.0;
	    realign_data[grn_index] = 0.0;
	    realign_data[blu_index] = 0.0;
            realign_data[alph_index] = 1.0;
        } else if (g_tex_blend_func[COLORMAP_TEX_ID] == GL_DECAL) {
            realign_data[alph_index] = 0.0;
        }
      }
    }
  }

  //
  // Create/Setup the Texture in GL:
  //
  // make sure gl calls are directed to the right context
  v_gl_set_context_to_vlib_window();

  glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );
  
  glBindTexture(GL_TEXTURE_2D, tex_ids[COLORMAP_TEX_ID]);
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
 
  float bord_col[4] = {0.0, 0.0, 0.0, 1.0};
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, bord_col);

#if defined(sgi)
  if (gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA,
                   g_tex_installed_width[COLORMAP_TEX_ID],
                   g_tex_installed_height[COLORMAP_TEX_ID],
                   GL_RGBA, GL_FLOAT, realign_data) != 0) {
    printf(" Error making mipmaps, using texture instead.\n");

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                   g_tex_installed_width[COLORMAP_TEX_ID],
                   g_tex_installed_height[COLORMAP_TEX_ID],
                0, GL_RGBA, GL_FLOAT, realign_data);
    if (glGetError()!=GL_NO_ERROR) {
      printf(" Error making realign texture.\n");
    }

  }
#else

#if defined(_WIN32)

/*
  if (gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, 
                   g_tex_installed_width[COLORMAP_TEX_ID], 
                   g_tex_installed_height[COLORMAP_TEX_ID],
                   GL_RGBA, GL_FLOAT, realign_data) != 0) { 
    printf(" Error making mipmaps, using texture instead.\n");
*/
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 
                   g_tex_installed_width[COLORMAP_TEX_ID], 
                   g_tex_installed_height[COLORMAP_TEX_ID],
		   0, GL_RGBA, GL_FLOAT, realign_data);
    if (glGetError()!=GL_NO_ERROR) {
      printf(" Error making realign texture.\n");
    }

//  }

#else
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 
                   g_tex_installed_width[COLORMAP_TEX_ID], 
                   g_tex_installed_height[COLORMAP_TEX_ID],
	       0, GL_RGBA, GL_FLOAT, realign_data);
 #ifndef FLOW
  if (glGetError()!=GL_NO_ERROR) {
    printf(" Error making realign texture.\n");
  }
 #endif
#endif
#endif
}

//
// Sets the conversion map used to convert data values to 
// color values when realigning textures.
//
void nmg_Graphics_Implementation::
setRealignTexturesConversionMap( const char *map, const char *mapdir ) {
  if ( !strcmp( map, "none" ) ) {
    if (g_realign_textures_curColorMap) {
      delete g_realign_textures_curColorMap;
    }
    g_realign_textures_curColorMap = NULL;
  }
  else {
    if ( !g_realign_textures_curColorMap )
      g_realign_textures_curColorMap = new ColorMap;
    g_realign_textures_curColorMap->load_from_file( map, mapdir );
  }
}

//
// Sets the realign texture colormap slider range:
//
void nmg_Graphics_Implementation::setRealignTextureSliderRange (float low,
								float high) {
  g_realign_textures_slider_min = low;
  g_realign_textures_slider_max = high;
}


//
// Resamples the original data set based on the transformed
// texture coordinates, creating a new data array.
// Uses bi-linear interpolation for points that don't fall
// exactly on the grid.
//
void nmg_Graphics_Implementation::computeRealignPlane( const char *name,
						       const char *newname ) {
  
  // New data Array 
  BCPlane *plane = g_inputGrid->getPlaneByName(name);
  if ( !plane )
    return;
  
  char newunits [1000];
  sprintf(newunits, "%s_realign", plane->units()->Characters());

  BCPlane *newplane = g_inputGrid->getPlaneByName(newname);
  if (!newplane) {
     g_inputGrid->addNewPlane( newname, newunits, NOT_TIMED );
     if ( newplane == NULL ) {
        fprintf( stderr, "compute realign plane: Can't make plane %s\n", 
		newname );
    	return;
     }
     d_dataset->dataImages->addImage(new nmb_ImageGrid(newplane));
  }

  // Resample the old dataset based on the realigned texture coordinates
  // Uses bi-linear interpolation for points that don't fall
  // exactly on the grid.

  int numx = plane->numX();
  int numy = plane->numY();
  for ( int i = 0; i < numy; i++ )
    for ( int j = 0; j < numx; j++ ) {

      // Transformed Texture Coordinate
      float u = vertexptr[i][j * 2].Texcoord[1];
      float v = vertexptr[i][j * 2].Texcoord[2];

      // X, Y coordinate in original dataset
      float x = u * 512; // Maximum texture size
      float y = v * 512; // Maximum texture size

      // interpolation values
      float s = x - floor( x );
      float t = y - floor( y );

      // indices of the four data values to interpolate:
      int x1 = (int)floor( x );
      int y1 = (int)floor( y );
      int x2 = (int)ceil( x );
      int y2 = (int)ceil( y );

      // lookup data values to interpolate:
      float a, b, c, d;
      if ( (x1 >= 0) && (x1 < numx) && (y1 >= 0) && (y1 < numy) )
	a = plane->value( x1, y1 );
      else
	a = 0;
      if ( (x2 >= 0) && (x2 < numx) && (y1 >= 0) && (y1 < numy) )
	b = plane->value( x2, y1 );
      else
	b = 0;
      if ( (x1 >= 0) && (x1 < numx) && (y2 >= 0) && (y2 < numy) )
	c = plane->value( x1, y2 );
      else
	c = 0;
      if ( (x2 >= 0) && (x2 < numx) && (y2 >= 0) && (y2 < numy) )
	d = plane->value( x2, y2 );
      else d = 0;

      // interpolate first along x:
      float e = (1.0 - s) * a + s * b;
      float f = (1.0 - s) * c + s * d;

      // interpolate interpolated values along y:
      float value = (1.0 - t) * e + t * f;
      
      // assign the value to the new dataset
      newplane->setValue( j, i, value );
    }
}

//
// Translates the texture based on the mouse movements:
//
void nmg_Graphics_Implementation::translateTextures ( int on,
						      float dx, float dy ) {
  g_translate_textures = on;
  g_scale_textures = 0;
  g_shear_textures = 0;
  g_rotate_textures = 0;

  //
  // First the direction (dx, dy) is rotated to match the
  // rotated texture coordinates. (So after rotating the texture,
  // translating up and down still matches the mouse)...
  //
  float u = ( dx / g_tex_range_x ) * cos( g_tex_theta_cumulative ) + 
    ( dy / g_tex_range_y ) * sin( g_tex_theta_cumulative );
  float v = ( dx / g_tex_range_x ) * -sin( g_tex_theta_cumulative ) +
    ( dy / g_tex_range_y ) * cos( g_tex_theta_cumulative );

  g_translate_tex_dx = u;
  g_translate_tex_dy = v;

#ifdef PROJECTIVE_TEXTURE
  g_translate_tex_x -= dx;
  g_translate_tex_y -= dy;
#endif

  // Rebuilds the texture coordinate array:
  causeGridRebuild();
}

//
// Scales the texture based on the mouse movements:
//
void nmg_Graphics_Implementation::scaleTextures ( int on,
						  float dx, float dy ) {
  g_translate_textures = 0;
  g_scale_textures = on;
  g_shear_textures = 0;
  g_rotate_textures = 0;

  //
  // First the direction (dx, dy) is rotated to match the
  // rotated texture coordinates. (So after rotating the texture,
  // scaling left and right still matches the mouse)...
  //
  float u = ( dx / 100.0 ) * cos( g_tex_theta_cumulative ) + 
    ( dy / 100.0 ) * sin( g_tex_theta_cumulative );
  float v = ( dx / 100.0 ) * -sin( g_tex_theta_cumulative ) +
    ( dy / 100.0 ) * cos( g_tex_theta_cumulative );

  g_scale_tex_dx = 1.0 + u;
  g_scale_tex_dy = 1.0 + v;

  g_tex_range_x /= g_scale_tex_dx;
  g_tex_range_y /= g_scale_tex_dy;

#ifdef PROJECTIVE_TEXTURE
  if (g_scale_tex_dx > 0.5 && g_scale_tex_dx < 2.0)
      g_scale_tex_x *= g_scale_tex_dx;
  if (g_scale_tex_dy > 0.5 && g_scale_tex_dy < 2.0)
      g_scale_tex_y *= g_scale_tex_dy;
#endif

  // Rebuilds the texture coordinate array:
  causeGridRebuild();
}

//
// Shear the texture based on the mouse movements:
//
void nmg_Graphics_Implementation::shearTextures ( int on,
						  float dx, float dy ) {
  g_translate_textures = 0;
  g_scale_textures = 0;
  g_shear_textures = on;
  g_rotate_textures = 0;

  //
  // First the direction (dx, dy) is rotated to match the
  // rotated texture coordinates. (So after rotating the texture,
  // shearing left and right still matches the mouse)...
  //
  float u = ( dx/(g_tex_range_x ) ) * cos( g_tex_theta_cumulative ) + 
    ( dy/( g_tex_range_y ) ) * sin( g_tex_theta_cumulative );
  float v = ( dx/(g_tex_range_x ) ) * -sin( g_tex_theta_cumulative ) +
    ( dy/( g_tex_range_y ) ) * cos( g_tex_theta_cumulative );

  g_shear_tex_dx = u;
  g_shear_tex_dy = v;

#ifdef PROJECTIVE_TEXTURE
  // not sure why this is more sensitive but it is so we scale down the
  // change a bit
  g_shear_tex_x += g_shear_tex_dx/100.0;
  g_shear_tex_y += g_shear_tex_dy/100.0;
#endif

  // Rebuilds the texture coordinate array:
  causeGridRebuild();
}

//
// Shear the texture based on the mouse movements:
//
void nmg_Graphics_Implementation::rotateTextures ( int on, float theta ) {
  g_translate_textures = 0;
  g_scale_textures = 0;
  g_shear_textures = 0;
  g_rotate_textures = on;

  g_rotate_tex_theta = theta / 10.0;
  g_tex_theta_cumulative += g_rotate_tex_theta;

  // Rebuilds the texture coordinate array:
  causeGridRebuild();
}

//
// Set the center of texture realign transformations:
// A red "center sphere" follows the mouse:
//
void nmg_Graphics_Implementation::setTextureCenter( float dx, float dy ) {

  int numx = g_inputGrid->numX();
  int numy = g_inputGrid->numY();

#ifdef PROJECTIVE_TEXTURE

  // (g_translate_tex = g_translate_tex_x,g_translate_tex_y)
  // g_translate_tex represents the translation of the grid to the
  // point about which the texture is rotated, scaled, and sheared
  // so to move this point we must adjust g_translate_tex but then
  // we need to compute the corresponding new location in texture coordinates
  // so that this can be subtracted out properly by compute_texture_matrix() 
  // so that the texture itself doesn't move

  BCPlane *plane = g_inputGrid->getPlaneByName(g_heightPlaneName);
  if (plane == NULL){
      fprintf(stderr, "Error in setTextureCenter: could not get plane!\n");
      return; 
  }

  // we use current texture matrix to compute how the change in position of
  // the center on the grid (g_tex_grid_center) gets mapped to a change in 
  // the position of the center in the texture (g_tex_coord_center)
  double tex_mat[16];

  // note: it is important that we call this here before we
  // change g_translate_tex because otherwise g_translate_tex and
  // g_tex_coord_center would be inconsistent and the texture would move
  compute_texture_matrix(g_translate_tex_x, g_translate_tex_y,
                g_tex_theta_cumulative, g_scale_tex_x,
                g_scale_tex_y, g_shear_tex_x, g_shear_tex_y,
                g_tex_coord_center_x, g_tex_coord_center_y,
                tex_mat); 

  double pl_len_x = plane->maxX()-plane->minX();
  double pl_len_y = plane->maxY()-plane->minY();

  // this just scales the mouse movements by a reasonable value to map them
  // to nm:
  g_translate_tex_x += dx*pl_len_x/(double)numx;
  g_translate_tex_y += dy*pl_len_y/(double)numy;

  if (g_translate_tex_x > pl_len_x)
     g_translate_tex_x = pl_len_x;
  else if (g_translate_tex_x < 0)
     g_translate_tex_x = 0;
  if (g_translate_tex_y > pl_len_y)
     g_translate_tex_y = pl_len_y;
  else if (g_translate_tex_y < 0)
     g_translate_tex_y = 0;

  float x = 0, y = 0, z = 0;
  // x,y are in nm
  x = g_translate_tex_x;
  y = g_translate_tex_y;
 
  // rx,ry are in grid lattice units 
  double rx = x*(double)numx/pl_len_x;
  double ry = y*(double)numy/pl_len_y;
  z = plane->interpolatedValue(rx, ry)*(plane->scale());

  position_sphere(x,y,z);

  // now that we have the new position of the center in grid coordinates, we
  // compute the new position of the center in texture coordinates using the
  // _old_ texture matrix 
  double grid_pnt[4] = {x, y, z, 1.0};
  double texture_pnt[2];
  int i,j;
  for (i = 0; i < 2; i++){
      texture_pnt[i] = 0.0;
      for (j = 0; j < 4; j++)
          texture_pnt[i] += tex_mat[i + j*4]*grid_pnt[j];
  }
  g_tex_coord_center_x = texture_pnt[0];
  g_tex_coord_center_y = texture_pnt[1];

#else	// NOT PROJECTIVE_TEXTURE

  // rotate:
  float u = ( dx/(2.0 * g_tex_range_x ) ) * cos( g_tex_theta_cumulative ) + 
    ( dy/( 2.0 * g_tex_range_y ) ) * sin( g_tex_theta_cumulative );
  float v = ( dx/(2.0 * g_tex_range_x ) ) * -sin( g_tex_theta_cumulative ) +
    ( dy/( 2.0 * g_tex_range_y ) ) * cos( g_tex_theta_cumulative );

  u = g_tex_coord_center_x + u;
  v = g_tex_coord_center_y + v;
  
  float x = 0, y = 0, z = 0;
  float min = 3;
  float x_dis, y_dis;

  for ( int i = 0; i < numy; i++ )
    for ( int j = 0; j < numx; j++ ) {
      x_dis = (vertexptr[i][j * 2].Texcoord[1] - u)*
	(vertexptr[i][j * 2].Texcoord[1] - u);
      y_dis = (vertexptr[i][j * 2].Texcoord[2] - v)*
	(vertexptr[i][j * 2].Texcoord[2] - v);
      if ( sqrt( x_dis + y_dis ) < min ) {
	min = sqrt( x_dis + y_dis );
	x = vertexptr[i][j * 2].Vertex[0];
	y = vertexptr[i][j * 2].Vertex[1];
	z = vertexptr[i][j * 2].Vertex[2];
	g_tex_coord_center_x = vertexptr[i][j * 2].Texcoord[1];
	g_tex_coord_center_y = vertexptr[i][j * 2].Texcoord[2];
	position_sphere(x, y, z);
      }
    }
#endif
}

//
// End of Realign Texture Functions.
//

void nmg_Graphics_Implementation::initializeTextures(void)
{
  int i,j, k;
  //fprintf(stderr, "initializing textures\n");

  for (i = 0; i < N_TEX; i++) {
     g_tex_image_width[i] = NMG_DEFAULT_IMAGE_WIDTH;
     g_tex_image_height[i] = NMG_DEFAULT_IMAGE_HEIGHT;
     g_tex_installed_width[i] = NMG_DEFAULT_IMAGE_WIDTH;
     g_tex_installed_height[i] = NMG_DEFAULT_IMAGE_HEIGHT;
#ifdef _WIN32
     g_tex_blend_func[i] = CYGWIN_TEXTURE_FUNCTION;
#else
     g_tex_blend_func[i] = GL_DECAL;
#endif
  }
  g_tex_blend_func[SEM_DATA_TEX_ID] = GL_MODULATE;

  g_tex_env_color[SEM_DATA_TEX_ID][0] = 1.0;
  g_tex_env_color[SEM_DATA_TEX_ID][1] = 1.0;
  g_tex_env_color[SEM_DATA_TEX_ID][2] = 1.0;
  g_tex_env_color[SEM_DATA_TEX_ID][3] = 1.0;

  g_tex_env_color[COLORMAP_TEX_ID][0] = 1.0;
  g_tex_env_color[COLORMAP_TEX_ID][1] = 1.0;
  g_tex_env_color[COLORMAP_TEX_ID][2] = 1.0;
  g_tex_env_color[COLORMAP_TEX_ID][3] = 1.0;

  // make sure gl calls are directed to the right context
  v_gl_set_context_to_vlib_window();

  glGenTextures(N_TEX, tex_ids);

#ifdef FLOW
  glGenTextures(N_SHADER_TEX, shader_tex_ids);

  PPMImageRec *image = new PPMImageRec;

  // initialize pattern texture
  glBindTexture(GL_TEXTURE_2D, shader_tex_ids[PATTERN_TEX_ID]);
  // Load textures from the specified directory.
  sprintf(filename,"%schecker", g_textureDir);
  PPMImageLoad(image, filename);
  glTexImage2D(GL_TEXTURE_2D, 0, 3, image->width, image->height, 0,
               GL_RGB, GL_BYTE, (const GLvoid *)image->data);

  // initialize bump texture
  glBindTexture(GL_TEXTURE_2D, shader_tex_ids[BUMP_TEX_ID]);
  sprintf(filename,"%sstripes-bump", g_textureDir);
  PPMImageLoad(image, filename);
  glTexImage2D(GL_TEXTURE_2D, 0, 3, image->width, image->height, 0,
               GL_RGB, GL_BYTE, (const GLvoid *)image->data);

  glBindTexture(GL_TEXTURE_2D, shader_tex_ids[HATCH_NOISE_TEX_ID]);
  sprintf(filename,"%suniform_noise2", g_textureDir);
  PPMImageLoad(image, filename);
  glTexImage2D(GL_TEXTURE_2D, 0, 3, image->width, image->height, 0,
               GL_RGB, GL_BYTE, (const GLvoid *)image->data);
  spotnoise_tex_size = min(image->width, image->height);

  PPMImageDelete(image);

  // initiailize bump data
  //bump_data = new GLuint [g_data_tex_size * g_data_tex_size * 3];
  bump_data = new GLubyte [g_data_tex_size * g_data_tex_size * 3];
  for (i = 0; i < g_data_tex_size * g_data_tex_size * 3; i++)
    bump_data[i] = 0;
  glBindTexture(GL_TEXTURE_2D, shader_tex_ids[BUMP_DATA_TEX_ID]);
  glTexImage2D(GL_TEXTURE_2D, 0, 3, g_data_tex_size, g_data_tex_size, 0,
               GL_RGB, GL_BYTE, (const GLvoid *)bump_data);

  // initialize pattern data
  //pattern_data = new GLuint [g_data_tex_size * g_data_tex_size * 3];
  pattern_data = new GLubyte [g_data_tex_size * g_data_tex_size * 3];
  for (i = 0; i < g_data_tex_size * g_data_tex_size * 3; i++)
    pattern_data[i] = 0;
  glBindTexture(GL_TEXTURE_2D, shader_tex_ids[PATTERN_DATA_TEX_ID]);
  glTexImage2D(GL_TEXTURE_2D, 0, 3, g_data_tex_size, g_data_tex_size, 0,
               GL_RGB, GL_BYTE, (const GLvoid *)pattern_data)

  // initialize hatch data
  //hatch_data = new GLuint [g_data_tex_size * g_data_tex_size * 3];
  hatch_data = new GLubyte [g_data_tex_size * g_data_tex_size * 3];
  for (i = 0; i < g_data_tex_size * g_data_tex_size * 3; i++)
    hatch_data[i] = 0;
  glBindTexture(GL_TEXTURE_2D, shader_tex_ids[HATCH_DATA_TEX_ID]);
  glTexImage2D(GL_TEXTURE_2D, 0, 3, g_data_tex_size, g_data_tex_size, 0,
               GL_RGB, GL_BYTE, (const GLvoid *)hatch_data);
#endif

  buildContourTexture();

#if defined(sgi) || defined(FLOW) || defined(_WIN32)
  //fprintf(stderr, "Initializing checkerboard texture.\n");
  makeCheckImage();
  buildAlphaTexture();

  //fprintf(stderr, "Initializing ruler texture.");
  if (g_rulerPPM) {
    makeAndInstallRulerImage(g_rulerPPM);
  } else {
    makeRulerImage();
    buildRulergridTexture();
    //fprintf(stderr, " Using default grid.\n");
  }
  if (glGetError()!=GL_NO_ERROR) {
      printf(" Error making ruler texture.\n");
  }

  int sem_tex_size = g_tex_installed_width[SEM_DATA_TEX_ID] *
                          g_tex_installed_height[SEM_DATA_TEX_ID];
  sem_data = new GLubyte [sem_tex_size]; 
  k = 0;
  
  for (i = 0; i < g_tex_installed_width[SEM_DATA_TEX_ID]; i++){
    for (j = 0; j < g_tex_installed_height[SEM_DATA_TEX_ID]; j++){
      sem_data[k] = 255;//20*((i/30+j/30)%2);
      k++;
    }
  }
  glBindTexture(GL_TEXTURE_2D, tex_ids[SEM_DATA_TEX_ID]);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 
                g_tex_installed_width[SEM_DATA_TEX_ID],
		g_tex_installed_height[SEM_DATA_TEX_ID], 
                0, GL_LUMINANCE, GL_UNSIGNED_BYTE, sem_data);

  if (report_gl_errors()) {
     fprintf(stderr, "Error initializing sem texture\n");
  }
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

  int realign_tex_size = 4*g_tex_installed_width[COLORMAP_TEX_ID] *
                             g_tex_installed_height[COLORMAP_TEX_ID];
  realign_data = new float [realign_tex_size];
  glBindTexture(GL_TEXTURE_2D, tex_ids[COLORMAP_TEX_ID]);
  int ri,gi,bi,ai;
  if (g_tex_blend_func[COLORMAP_TEX_ID] == GL_BLEND) {
    for (ri=0,gi=1,bi=2,ai=3; ai < realign_tex_size; ri+=4,gi+=4,bi+=4,ai+=4){
      realign_data[ri] = 0.0;
      realign_data[gi] = 0.0;
      realign_data[bi] = 0.0;
      realign_data[ai] = 1.0;
    }
    float bord_col[4] = {0.0, 0.0, 0.0, 1.0};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, bord_col);
  } else if (g_tex_blend_func[COLORMAP_TEX_ID] == GL_MODULATE) {
    for (ri=0,gi=1,bi=2,ai=3; ai < realign_tex_size; ri+=4,gi+=4,bi+=4,ai+=4){
      realign_data[ri] = 1.0;
      realign_data[gi] = 1.0;
      realign_data[bi] = 1.0;
      realign_data[ai] = 1.0;
    }
    float bord_col[4] = {1.0, 1.0, 1.0, 1.0};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, bord_col);
  } else {
    for (ri=0,gi=1,bi=2,ai=3; ai < realign_tex_size; ri+=4,gi+=4,bi+=4,ai+=4){
      realign_data[ri] = 0.0;
      realign_data[gi] = 0.0;
      realign_data[bi] = 0.0;
      realign_data[ai] = 0.0;
    }
    float bord_col[4] = {0.0, 0.0, 0.0, 0.0};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, bord_col);
  }
  glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );

#ifdef _WIN32
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
       g_tex_installed_width[COLORMAP_TEX_ID],
       g_tex_installed_height[COLORMAP_TEX_ID],
       0, GL_RGBA, GL_FLOAT, realign_data);
#else
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
       g_tex_installed_width[COLORMAP_TEX_ID],
       g_tex_installed_height[COLORMAP_TEX_ID],
       0, GL_RGBA, GL_FLOAT, realign_data);
#endif

  if (report_gl_errors()) {
     fprintf(stderr, "Error initializing realign texture\n");
  }
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

#endif

}

/*
This function will load data from an nmb_Image which contains 
an array of unsigned bytes - we do it this way so it will be as fast
as possible (don't want to do unnecessary copying or scaling operations)
*/
void nmg_Graphics_Implementation::loadRawDataTexture(const int /*which*/,
       const char *image_name,
       const int start_x, const int start_y)
{
    nmb_Image *im = d_dataset->dataImages->getImageByName(image_name);

    if (!im) {
        fprintf(stderr, "nmg_GraphImp::loadRawDataTexture:"
                        " Error, couldn't find image: %s\n", image_name);
        return;
    }
    void *data = NULL;
    data = im->pixelData();
    if (!data) {
        fprintf(stderr, "nmg_GraphImp::loadRawDataTexture:"
                   " Error, image exists but pixel array isn't available\n");
        return;
    }
/*    printf("loadRawDataTexture: start=(%d,%d), size=(%d,%d)\n",
           start_x, start_y, im->width(), im->height());
*/

    // make sure gl calls are directed to the right context
    v_gl_set_context_to_vlib_window();

    if (im->width() <= g_tex_installed_width[SEM_DATA_TEX_ID] && 
	im->height() <= g_tex_installed_height[SEM_DATA_TEX_ID]) {
        int pixelType;
        if (im->pixelType() == NMB_UINT8){
            pixelType = GL_UNSIGNED_BYTE;
        } else if (im->pixelType() == NMB_UINT16){
            pixelType = GL_UNSIGNED_SHORT;
        } else if (im->pixelType() == NMB_FLOAT32){
            pixelType = GL_FLOAT;
        } else {
            fprintf(stderr, "nmb_GraphicsImp::loadRawDataTexture:"
                 "can't handle pixel type\n"); 
            return;      
        }
        glBindTexture(GL_TEXTURE_2D, tex_ids[SEM_DATA_TEX_ID]);
        glTexSubImage2D(GL_TEXTURE_2D, 0, start_x, start_y,
               im->width(), im->height(), GL_LUMINANCE, pixelType, 
	       data);
        g_tex_image_width[SEM_DATA_TEX_ID] = im->width();
        g_tex_image_height[SEM_DATA_TEX_ID] = im->height();
	if (report_gl_errors()) {
	  printf(" Error loading sem texture.\n");
	}
    } else {
      fprintf(stderr, "Error, sem image size exceeds limit\n");
    }
}

void nmg_Graphics_Implementation::updateTexture(int which_texture,
       const char *image_name,
       int start_x, int start_y,
       int /*end_x*/, int /*end_y*/)
{
    if (which_texture == SEM_DATA){
       loadRawDataTexture(which_texture, image_name, start_x, start_y);
    } else {
       printf("nmg_Graphics_Implementation::updateTexture - don't know this"
		" texture id: %d", which_texture);
    }
}

void nmg_Graphics_Implementation::setTextureTransform(double *xform){
    int i;
    printf("got TextureTransform\n");
    printf("was: ");
    for (i = 0; i < 16; i++)
        printf("%g ", g_texture_transform[i]);
    printf("\n");

    for (i = 0; i < 16; i++)
	g_texture_transform[i] = xform[i];

    printf("new: ");
    for (i = 0; i < 16; i++)
        printf("%g ", g_texture_transform[i]);
    printf("\n");
}


void nmg_Graphics_Implementation::setRulergridAngle (float v) {
//fprintf(stderr, "nmg_Graphics_Implementation::setRulergridAngle().\n");
  g_rulergrid_sin = sin(-v / 180.0 * M_PI);
  g_rulergrid_cos = cos(-v / 180.0 * M_PI);
  causeGridRedraw();
}

void nmg_Graphics_Implementation::setRulergridColor (int r, int g, int b) {
//fprintf(stderr, "nmg_Graphics_Implementation::setRulergridColor().\n");
  g_ruler_r = r;
  g_ruler_g = g;
  g_ruler_b = b;

  // if they've specified a ruler image, keep that instead of replacing
  // it with the default set of orthogonal lines
  if (g_rulerPPM)
    makeAndInstallRulerImage(g_rulerPPM);
  else {
    makeRulerImage();
    buildRulergridTexture();
  }
}

void nmg_Graphics_Implementation::setRulergridOffset (float x, float y) {
//fprintf(stderr, "nmg_Graphics_Implementation::setRulergridOffset().\n");
  g_rulergrid_xoffset = x;
  g_rulergrid_yoffset = y;
  causeGridRedraw();
}

void nmg_Graphics_Implementation::setNullDataAlphaToggle( int v ) {
  g_null_data_alpha_toggle = v;
  causeGridRedraw();
}

void nmg_Graphics_Implementation::setRulergridOpacity (float alpha) {
//fprintf(stderr, "nmg_Graphics_Implementation::setRulergridOpacity().\n");
  g_ruler_opacity = alpha;

  // if they've specified a ruler image, keep that instead of replacing
  // it with the default set of orthogonal lines
  if (g_rulerPPM)
    makeAndInstallRulerImage(g_rulerPPM);
  else {
    makeRulerImage();
    buildRulergridTexture();
  }
  if (g_realign_texture_name[0]) {
    createRealignTextures(g_realign_texture_name);
  }
}

void nmg_Graphics_Implementation::setRulergridScale (float s) {
//fprintf(stderr, "nmg_Graphics_Implementation::setRulergridScale().\n");
  g_rulergrid_scale = s;
  causeGridRedraw();
}

void nmg_Graphics_Implementation::setRulergridWidths (float x, float y) {
//fprintf(stderr, "nmg_Graphics_Implementation::setRulergridWidths().\n");
  g_ruler_width_x = x;
  g_ruler_width_y = y;

  // if they've specified a ruler image, keep that instead of replacing
  // it with the default set of orthogonal lines
  if (g_rulerPPM)
    makeAndInstallRulerImage(g_rulerPPM);
  else {
    makeRulerImage();
    buildRulergridTexture();
  }
  causeGridRedraw();
}

void nmg_Graphics_Implementation::setSpecularity (int s) {
//fprintf(stderr, "nmg_Graphics_Implementation::setSpecularity().\n");
  g_shiny = s;
  causeGridRedraw();
}


void nmg_Graphics_Implementation::setDiffusePercent (float d) {
//fprintf(stderr, "nmg_Graphics_Implementation::setDiffusePercent().\n");
  g_diffuse = d;
  causeGridRedraw();
}

void nmg_Graphics_Implementation::setSurfaceAlpha (float a) {
//fprintf(stderr, "nmg_Graphics_Implementation::setSurfaceAlpha().\n");
  g_surface_alpha = a;
  causeGridRedraw();
}

void nmg_Graphics_Implementation::setSpecularColor (float s) {
//fprintf(stderr, "nmg_Graphics_Implementation::setSpecularColor().\n");
  g_specular_color = s;
  causeGridRedraw();
}

void nmg_Graphics_Implementation::setSphereScale (float s) {
//fprintf(stderr, "nmg_Graphics_Implementation::setSphereScale().\n");
  g_sphere_scale = s;
}

void nmg_Graphics_Implementation::setTesselationStride (int s) {
//fprintf(stderr, "nmg_Graphics_Implementation::setTesselationStride(%d).\n", s);

  g_stride = s;

  // Destroy the old triangle strips, then rebuilt the correct number
  // of new ones.
  causeGridRebuild();
}

void nmg_Graphics_Implementation::setTextureMode (TextureMode m,
	TextureTransformMode xm) {
//fprintf(stderr, "nmg_Graphics_Implementation::setTextureMode().\n");

// on FLOW, do we need to not change out of TEXTURE_2D?
#ifdef FLOW
  m = RULERGRID;
#endif

  switch (m) {

    case NO_TEXTURES:
//      fprintf(stderr, "nmg_Graphics_Implementation:  no textures.\n");
      g_texture_mode = GL_FALSE;
      break;
    case CONTOUR:
// This mode is possible if we can reimplement contour texture stuff
// not using glXXXPointerEXT() and glDrawArraysEXT()
//#ifndef _WIN32
        //fprintf(stderr, "nmg_Graphics_Implementation:  entering CONTOUR mode.\n");
      g_texture_mode = GL_TEXTURE_1D;
/*#else
      fprintf(stderr, "nmg_Graphics_Implementation: " 
		"CONTOUR mode not available in win32\n");
      g_texture_mode = GL_FALSE;
#endif
*/
      break;
    case BUMPMAP:
//        fprintf(stderr,
//          "nmg_Graphics_Implementation: entering BUMPMAP mode.\n");
      g_texture_mode = GL_TEXTURE_2D;
      break;
    case HATCHMAP:
//        fprintf(stderr,
//          "nmg_Graphics_Implementation: entering HATCHMAP mode.\n");
      g_texture_mode = GL_TEXTURE_2D;
      break;
    case PATTERNMAP:
//        fprintf(stderr,
//  	"nmg_Graphics_Implementation: entering PATTERNMAP mode.\n");
      g_texture_mode = GL_TEXTURE_2D;
      break;
    case RULERGRID:
//        fprintf(stderr,"nmg_Graphics_Implementation: entering RULERGRID mode.\n");
      g_texture_mode = GL_TEXTURE_2D;
      break;
    case ALPHA:
#ifndef _WIN32
//        fprintf(stderr, "nmg_Graphics_Implementation:  entering ALPHA mode.\n");
      g_texture_mode = GL_TEXTURE_3D_EXT;
#else
      fprintf(stderr, "nmg_Graphics_Implementation:"
		" ALPHA mode not available under WIN32\n");
      g_texture_mode = GL_FALSE;
#endif
      break;
    case COLORMAP:
//    fprintf(stderr, "nmg_Graphics_Implementation: entering COLORMAP mode.\n");
      g_texture_mode = GL_TEXTURE_2D;
      break;
    case SEM_DATA:
//    fprintf(stderr, "nmg_Graphics_Implementation: entering SEM_DATA mode.\n");
      g_texture_mode = GL_TEXTURE_2D;
      break;
    case REMOTE_DATA:
//fprintf(stderr, "nmg_Graphics_Implementation:  entering REMOTE_DATA mode.\n");
      g_texture_mode = GL_TEXTURE_2D;
      break;
    default:
      fprintf(stderr, "nmg_Graphics_Implementation::setTextureMode:  "
                      "Unknown texture mode %d.\n", m);
      g_texture_mode = GL_FALSE;
      break;
  }
  switch (xm) {
    case RULERGRID_COORD:
//    fprintf(stderr, "nmg_Graphics_Implementation: RULERGRID_COORD mode\n");
      break;
    case REGISTRATION_COORD:
//    fprintf(stderr, "nmg_Graphics_Implementation: REGISTRATION_COORD mode\n");
      break;
    case MANUAL_REALIGN_COORD:
      //fprintf(stderr, 
	//"nmg_Graphics_Implementation: MANUAL_REALIGN_COORD mode\n");
      if (d_textureTransformMode != MANUAL_REALIGN_COORD) {
	g_translate_textures  = 0;
        g_scale_textures      = 0;
        g_shear_textures      = 0;
        g_rotate_textures     = 0;

        g_shear_tex_x = 0.0;
        g_shear_tex_y = 0.0;
        g_tex_coord_center_x = 0.0;
        g_tex_coord_center_y = 0.0;
        g_scale_tex_x = 2500.0;
        g_scale_tex_y = 2500.0;
        g_tex_theta_cumulative = 0.0;
        g_translate_tex_x = 0.0;
        g_translate_tex_y = 0.0;
      }
      break;
    default:
      fprintf(stderr, "nmg_Graphics_Implementation::setTextureMode:  "
		"Unknown texture transform mode %d.\n", xm);
      break;
  }
  d_textureMode = m;
  d_textureTransformMode = xm;
  // communicate to non nmg_Graphics code:
  g_texture_displayed = m;
  g_texture_transform_mode = xm;

  causeGridRedraw();
}

void nmg_Graphics_Implementation::setTextureScale (float f) {
//fprintf(stderr, "nmg_Graphics_Implementation::setTextureScale().\n");
  g_texture_scale = f;
  causeGridRedraw();
}

void nmg_Graphics_Implementation::setTrueTipScale (float f) {
//fprintf(stderr, "nmg_Graphics_Implementation::setTrueTipScale().\n");
  g_trueTipScale = f;
}


void nmg_Graphics_Implementation::setUserMode (int oldMode, int oldStyle,
					       int newMode, int style) {
//fprintf(stderr, "nmg_Graphics_Implementation::setUserMode().\n");
  clear_world_modechange(oldMode, oldStyle);
  init_world_modechange(newMode, style);
}


void nmg_Graphics_Implementation::setLightDirection (q_vec_type & v) {
//fprintf(stderr, "nmg_Graphics_Implementation::setLightDirection().\n");
  ::setLightDirection(v);
}

void nmg_Graphics_Implementation::resetLightDirection (void) {
//fprintf(stderr, "nmg_Graphics_Implementation::resetLightDirection().\n");
  ::resetLightDirection();
}


int nmg_Graphics_Implementation::addPolylinePoint( const PointType point[2] ) {
//int nmg_Graphics_Implementation::addPolylinePoint (const float point [2][3]) {
//fprintf(stderr, "nmg_Graphics_Implementation::addPolylinePoint().\n");

  return make_rubber_line_point(point, g_positionList);
}

void nmg_Graphics_Implementation::emptyPolyline (void) {
//fprintf(stderr, "nmg_Graphics_Implementation::emptyPolyline().\n");

    empty_rubber_line(g_positionList);
    empty_rubber_line(g_positionListL, 0);
    empty_rubber_line(g_positionListR, 1);
}

void nmg_Graphics_Implementation::setRubberLineStart (float p0, float p1) {
//fprintf(stderr, "nmg_Graphics_Implementation::setRubberLineStart().\n");
  g_rubberPt[0] = p0;
  g_rubberPt[1] = p1;
}

void nmg_Graphics_Implementation::setRubberLineEnd (float p2, float p3 ) {
//fprintf(stderr, "nmg_Graphics_Implementation::setRubberLineEnd().\n");
  g_rubberPt[2] = p2;
  g_rubberPt[3] = p3;
}

void nmg_Graphics_Implementation::setRubberLineStart (const float p [2]) {
//fprintf(stderr, "nmg_Graphics_Implementation::setRubberLineStart().\n");
  g_rubberPt[0] = p[0];
  g_rubberPt[1] = p[1];
}

void nmg_Graphics_Implementation::setRubberLineEnd (const float p [2]) {
//fprintf(stderr, "nmg_Graphics_Implementation::setRubberLineEnd().\n");
  g_rubberPt[2] = p[0];
  g_rubberPt[3] = p[1];
}

void nmg_Graphics_Implementation::setRubberSweepLineStart (const PointType Left,
							   const PointType Right) {
//fprintf(stderr, "nmg_Graphics_Implementation::setRubberLineStart().\n");
  g_rubberSweepPts[0][0] = Left[0];
  g_rubberSweepPts[0][1] = Left[1];

  g_rubberSweepPts[1][0] = Right[0];
  g_rubberSweepPts[1][1] = Right[1];
}

void nmg_Graphics_Implementation::setRubberSweepLineEnd (const PointType Left,
							 const PointType Right) {
//fprintf(stderr, "nmg_Graphics_Implementation::setRubberLineEnd().\n");
    PointType avg_old, avg_new, dif;

    avg_old[0] = (g_rubberSweepPts[0][0] + g_rubberSweepPts[1][0])/2;
    avg_old[1] = (g_rubberSweepPts[0][1] + g_rubberSweepPts[1][1])/2;

    avg_new[0] = (Left[0] + Right[0])/2;
    avg_new[1] = (Left[1] + Right[1])/2;

    dif[0] = avg_new[0] - Left[0];
    dif[1] = avg_new[1] - Left[1];

    // adjust old points:
    g_rubberSweepPts[0][0] = avg_old[0] + dif[0]; //Left
    g_rubberSweepPts[0][1] = avg_old[1] + dif[1];

    g_rubberSweepPts[1][0] = avg_old[0] - dif[0]; //Right
    g_rubberSweepPts[1][1] = avg_old[1] - dif[1];


    PointType vr, vl;
    float d1, d2;
    vr[0] = Right[0] - g_rubberSweepPts[1][0];
    vr[1] = Right[1] - g_rubberSweepPts[1][1];
    vr[2] = 0;
    d1 = sqrt( vr[0]*vr[0] + vr[1]*vr[1]);

    vl[0] = Left[0] - g_rubberSweepPts[1][0];
    vl[1] = Left[1] - g_rubberSweepPts[1][1];
    vl[2] = 0;
    d2 = sqrt( vl[0]*vl[0] + vl[1]*vl[1]);

    if ( d1 < d2 ) {
	g_rubberSweepPts[0][2] = Left[0];
	g_rubberSweepPts[0][3] = Left[1];
	
	g_rubberSweepPts[1][2] = Right[0];
	g_rubberSweepPts[1][3] = Right[1];
    }
    else {
	g_rubberSweepPts[0][2] = Right[0];
	g_rubberSweepPts[0][3] = Right[1];
	
	g_rubberSweepPts[1][2] = Left[0];
	g_rubberSweepPts[1][3] = Left[1];
    }

    g_rubberSweepPtsSave[0][2] = g_rubberSweepPts[0][0];
    g_rubberSweepPtsSave[0][3] = g_rubberSweepPts[0][1];

    g_rubberSweepPtsSave[1][2] = g_rubberSweepPts[1][0];
    g_rubberSweepPtsSave[1][3] = g_rubberSweepPts[1][1];
}


void nmg_Graphics_Implementation::setScanlineEndpoints(const float p0[3],
		const float p1[3]){
//fprintf(stderr, "nmg_Graphics_Implementation::setScanlineEndpoints().\n");
  g_scanlinePt[0] = p0[0]; g_scanlinePt[1] = p0[1]; g_scanlinePt[2] = p0[2];
  g_scanlinePt[3] = p1[0]; g_scanlinePt[4] = p1[1]; g_scanlinePt[5] = p1[2];
}

void nmg_Graphics_Implementation::displayScanlinePosition(const int enable)
{
//fprintf(stderr, "nmg_Graphics_Implementation::displayScanlinePosition().\n");
    enableScanlinePositionDisplay(enable);
}

void nmg_Graphics_Implementation::positionAimLine (const PointType top,
                                                   const PointType bottom) {
//fprintf(stderr, "nmg_Graphics_Implementation::positionAimLine().\n");
  make_aim(top, bottom);
}

void nmg_Graphics_Implementation::positionRubberCorner
    (float minx, float miny, float maxx, float maxy) {
//fprintf(stderr, "nmg_Graphics_Implementation::positionRubberCorner().\n");
  make_rubber_corner(minx, miny, maxx, maxy);
}

void nmg_Graphics_Implementation::positionSweepLine (const PointType topL,
                                                     const PointType bottomL,
                                                     const PointType topR,
                                                     const PointType bottomR) {
//fprintf(stderr, "nmg_Graphics_Implementation::positionSweepLine().\n");
  make_sweep(topL, bottomL, topR, bottomR);
}

int nmg_Graphics_Implementation::addPolySweepPoints (const PointType topL,
                                                     const PointType bottomL,
                                                     const PointType topR,
                                                     const PointType bottomR) {
//fprintf(stderr, "nmg_Graphics_Implementation::addPolySweepPoints().\n");
    make_rubber_line_point(topL, bottomL, g_positionListL, 0);
    make_rubber_line_point(topR, bottomR, g_positionListR, 1);
    return 0;
}


void nmg_Graphics_Implementation::positionSphere (float x, float y, float z) {
//fprintf(stderr, "nmg_Graphics_Implementation::positionSphere().\n");
  position_sphere(x, y, z);
}



void nmg_Graphics_Implementation::setViewTransform (v_xform_type xform) {

  v_world.users.xforms[0].xlate[0] = xform.xlate[0];
  v_world.users.xforms[0].xlate[1] = xform.xlate[1];
  v_world.users.xforms[0].xlate[2] = xform.xlate[2];
  v_world.users.xforms[0].rotate[0] = xform.rotate[0];
  v_world.users.xforms[0].rotate[1] = xform.rotate[1];
  v_world.users.xforms[0].rotate[2] = xform.rotate[2];
  v_world.users.xforms[0].rotate[3] = xform.rotate[3];
  v_world.users.xforms[0].scale = xform.scale;

}

void nmg_Graphics_Implementation::createScreenImage
(
   const char      *filename,
   const ImageType  type
)
{
//   fprintf(stderr, "DEBUG nmg_Graphics_Impl::createScreenImage '%s' '%s'\n", filename, ImageType_names[type]);
  int w, h;
  unsigned char * pixels = NULL;

  screenCapture(&w, &h, &pixels, vrpn_FALSE);

  if (!pixels) {
    return;
  }

  AbstractImage *ai = ImageMaker(type, h, w, 3, pixels, true);

  delete [] pixels;

  if (ai)
  {
     if (!ai->Write(filename))
        fprintf(stderr, "Failed to write screen to '%s'!\n", filename);

     delete ai;
  }
}

void nmg_Graphics_Implementation::getLightDirection (q_vec_type * v) const {
//fprintf(stderr, "nmg_Graphics_Implementation::getLightDirection().\n");
  ::getLightDirection(v);
}

int nmg_Graphics_Implementation::getHandColor (void) const {
//fprintf(stderr, "nmg_Graphics_Implementation::getHandColor().\n");
  return g_hand_color;
}

int nmg_Graphics_Implementation::getSpecularity (void) const {
//fprintf(stderr, "nmg_Graphics_Implementation::getSpecularity().\n");
  return g_shiny;
}

/*
float nmg_Graphics_Implementation::getDiffusePercent (void) const {
  return g_diffuse;
}
*/

const double * nmg_Graphics_Implementation::getMinColor (void) const {
//fprintf(stderr, "nmg_Graphics_Implementation::getMinColor().\n");
  return g_minColor;
}

const double * nmg_Graphics_Implementation::getMaxColor (void) const {
//fprintf(stderr, "nmg_Graphics_Implementation::getMaxColor().\n");
  return g_maxColor;
}




// PROTECTED

#if 0
nmg_Graphics_Implementation::TextureMode
   nmg_Graphics_Implementation::getTextureMode (void) const {
  return d_textureMode;
}
#endif

void nmg_Graphics_Implementation::initDisplays (void) {
//fprintf(stderr, "nmg_Graphics_Implementation::initDisplays().\n");
  int i;

  /*
   * initialize display for each user
   */

  for ( i = 0; i < NUM_USERS; i++ ) {
    /* open display */
    d_displayIndexList[i] = v_open_display(V_ENV_DISPLAY, i);
    if (d_displayIndexList[i] == V_NULL_DISPLAY) {
      exit(V_ERROR);
    }
    //printf("display = '%s'\n", v_display_name(d_displayIndexList[i]));
  }
}


void nmg_Graphics_Implementation::screenCapture (int * w, int * h,
                                                 unsigned char ** pixels,
                                                 vrpn_bool captureBack) {

  v_display_type *displayPtr;
  v_viewport_type *windowPtr;

  displayPtr=&v_display_table[d_displayIndexList[0]];
  windowPtr=&(displayPtr->viewports[0]);

  *w = windowPtr->fbExtents[0];
  *h = windowPtr->fbExtents[1];

  if (!*pixels) {
    *pixels = new unsigned char [*w * *h * 3];
  }

  if (!*pixels) {
     fprintf(stderr, "nmg_Graphics_Implementation::screenCapture:  "
                     "Insufficient memory to grab screen!\n");
     return;
  }

  // make sure gl calls are directed to the right context
  v_gl_set_context_to_vlib_window();

  if (captureBack) {
    glReadBuffer(GL_BACK); // read the back buffer, no interference from WM
//fprintf(stderr, "nmg_Graphics_Implementation::screenCapture:  BACK BUFFER.\n");
  } else {
    glReadBuffer(GL_FRONT);
//fprintf(stderr, "nmg_Graphics_Implementation::screenCapture:  FRONT BUFFER.\n");
  }

  glPixelStorei(GL_PACK_ALIGNMENT, 1); // byte alignment, slower
  glReadPixels(0, 0, *w, *h, GL_RGB, GL_UNSIGNED_BYTE, *pixels);
  // Do we ever do anything really nonstandard?  do we always clean
  // up and return to defaults between frames?  will the following
  // screw-up some other fancy thing somewhere?  If graphics go wonky
  // these should be queried prior to reading, then restored to what
  // they were after reading the buffer instead of some "defaults".
  glPixelStorei(GL_PACK_ALIGNMENT, 4); // word alignment, GL's default

  glReadBuffer(GL_FRONT); // go back to the front buffer

}

void nmg_Graphics_Implementation::depthCapture (int * w, int * h,
                                                 float ** depths,
                                                 vrpn_bool captureBack) {
  v_display_type *displayPtr;
  v_viewport_type *windowPtr;

  displayPtr=&v_display_table[d_displayIndexList[0]];
  windowPtr=&(displayPtr->viewports[0]);

  *w = windowPtr->fbExtents[0];
  *h = windowPtr->fbExtents[1];

  if (!*depths) {
    *depths = new float [*w * *h];
  }

  if (!*depths) {
     fprintf(stderr, "nmg_Graphics_Implementation::screenCapture:  "
                     "Insufficient memory to grab screen!\n");
     return;
  }

  // make sure gl calls are directed to the right context
  v_gl_set_context_to_vlib_window();

  if (captureBack) {
    glReadBuffer(GL_BACK); // read the back buffer, no interference from WM
  } else {
    glReadBuffer(GL_FRONT);
  }

  glReadPixels(0, 0, *w, *h, GL_DEPTH_COMPONENT, GL_FLOAT, *depths);

  glReadBuffer(GL_FRONT); // go back to the front buffer

}

void nmg_Graphics_Implementation::getLatestGridChange (int * minX, int * maxX,
                                                   int * minY, int * maxY) {
  *minX = g_minChangedX;
  *maxX = g_maxChangedX;
  *minY = g_minChangedY;
  *maxY = g_maxChangedY;

}



// PRIVATE





// static
int nmg_Graphics_Implementation::handle_resizeViewport
                                 (void * userdata, vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  int height, width;

  CHECKF(it->decode_resizeViewport(p.buffer, &height, &width), "handle_resizeViewport");
  it->resizeViewport(height, width);
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_loadRulergridImage
                                 (void * userdata, vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;

  it->loadRulergridImage(p.buffer);
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_causeGridRedraw
                                 (void * userdata, vrpn_HANDLERPARAM /*p*/) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;

  it->causeGridRedraw();
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_causeGridRebuild
                                 (void * userdata, vrpn_HANDLERPARAM /*p*/) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;

  it->causeGridRebuild();
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_enableChartjunk
                                 (void * userdata, vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  int value;

  CHECKF(it->decode_enableChartjunk(p.buffer, &value), "handle_enableChartjunk");
  it->enableChartjunk(value);
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_enableFilledPolygons
                                 (void * userdata, vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  int value;

  CHECKF(it->decode_enableFilledPolygons(p.buffer, &value), "handle_enableFilledPolygons");
  it->enableFilledPolygons(value);
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_enableSmoothShading
                                 (void * userdata, vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  int value;

  CHECKF(it->decode_enableSmoothShading(p.buffer, &value), "handle_enableSmoothShading");
  it->enableSmoothShading(value);

  return 0;
}

// static
int nmg_Graphics_Implementation::handle_enableTrueTip
                                 (void * userdata, vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  int value;

  CHECKF(it->decode_enableTrueTip(p.buffer, &value), "handle_enableTrueTip");
  it->enableTrueTip(value);

  return 0;
}

// static
int nmg_Graphics_Implementation::handle_setAdhesionSliderRange
                                 (void * userdata, vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  float low, hi;

  CHECKF(it->decode_setAdhesionSliderRange(p.buffer, &low, &hi), "handle_setAdhesionSliderRange");
  it->setAdhesionSliderRange(low, hi);
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_setAlphaColor
                                 (void * userdata, vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  float r, g, b;

  CHECKF(it->decode_setAlphaColor(p.buffer, &r, &g, &b), "handle_setAlphaColor");
  it->setAlphaColor(r, g, b);
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_setAlphaSliderRange
                                 (void * userdata, vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  float low, hi;

  CHECKF(it->decode_setAlphaSliderRange(p.buffer, &low, &hi), "handle_setAlphaSliderRange");
  it->setAlphaSliderRange(low, hi);
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_setBumpMapName
                                 (void * userdata, vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;

  it->setBumpMapName(p.buffer);
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_setColorMapDirectory
                                 (void * userdata, vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;

  it->setColorMapDirectory(p.buffer);
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_setTextureDirectory
                                 (void * userdata, vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;

  it->setTextureDirectory(p.buffer);
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_setColorMapName
                                 (void * userdata, vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;

  it->setColorMapName(p.buffer);
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_setColorMinMax
                                 (void * userdata, vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  float low, hi;

  CHECK(it->decode_setColorMinMax(p.buffer, &low, &hi));
  it->setColorMinMax(low, hi);
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_setDataColorMinMax
                                 (void * userdata, vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  float low, hi;

  CHECK(it->decode_setDataColorMinMax(p.buffer, &low, &hi));
  it->setDataColorMinMax(low, hi);
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_setOpacitySliderRange
                                 (void * userdata, vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  float low, hi;
  CHECKF(it->decode_setOpacitySliderRange(p.buffer, &low, &hi), 
	 "handle_setOpacitySliderRange");
  it->setOpacitySliderRange(low, hi);
  return 0;
}


// static
int nmg_Graphics_Implementation::handle_setComplianceSliderRange
                                 (void * userdata, vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  float low, hi;

  CHECKF(it->decode_setComplianceSliderRange(p.buffer, &low, &hi), "handle_setComplianceSliderRange");
  it->setComplianceSliderRange(low, hi);
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_setContourColor
                                 (void * userdata, vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  int r, g, b;

  CHECKF(it->decode_setContourColor(p.buffer, &r, &g, &b), "handle_setContourColor");
  it->setContourColor(r, g, b);
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_setContourWidth
                                 (void * userdata, vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  float width;

  CHECKF(it->decode_setContourWidth(p.buffer, &width), "handle_setContourWidth");
  it->setContourWidth(width);
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_setFrictionSliderRange
                                 (void * userdata, vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  float low, hi;

  CHECKF(it->decode_setFrictionSliderRange(p.buffer, &low, &hi), "handle_setFrictionSliderRange");
  it->setFrictionSliderRange(low, hi);
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_setBumpSliderRange
                                 (void * userdata, vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  float low, hi;

  CHECKF(it->decode_setBumpSliderRange(p.buffer, &low, &hi), "handle_setBumpSliderRange");
  it->setBumpSliderRange(low, hi);
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_setBuzzSliderRange
                                 (void * userdata, vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  float low, hi;

  CHECKF(it->decode_setBuzzSliderRange(p.buffer, &low, &hi), "handle_setBuzzSliderRange");
  it->setBuzzSliderRange(low, hi);
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_setHandColor
                                 (void * userdata, vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  int color;

  CHECKF(it->decode_setHandColor(p.buffer, &color), "handle_setHandColor");
  it->setHandColor(color);
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_enableCollabHand
                                 (void *userdata, vrpn_HANDLERPARAM p)
{
  nmg_Graphics_Implementation *it = (nmg_Graphics_Implementation *)userdata;
  vrpn_bool on;

  CHECKF(it->decode_enableCollabHand(p.buffer, &on), "handle_enableCollabHand");
  it->enableCollabHand(on);
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_setCollabHandPos
                                 (void *userdata, vrpn_HANDLERPARAM p)
{
  nmg_Graphics_Implementation *it = (nmg_Graphics_Implementation *)userdata;
  double pos[3], quat[4];

  CHECKF(it->decode_setCollabHandPos(p.buffer, pos, quat), "handle_setCollabHandPos");
  it->setCollabHandPos(pos, quat);
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_setCollabMode
                                 (void *userdata, vrpn_HANDLERPARAM p)
{
  nmg_Graphics_Implementation *it = (nmg_Graphics_Implementation *)userdata;
  int mode;

  CHECKF(it->decode_setCollabMode(p.buffer, &mode), "handle_setCollabMode");
  it->setCollabMode(mode);
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_setHatchMapName
                                 (void * userdata, vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;

  it->setHatchMapName(p.buffer);
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_setAlphaPlaneName (void * userdata,
                                           vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;

  it->setAlphaPlaneName(p.buffer);
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_setColorPlaneName (void * userdata,
                                           vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;

  it->setColorPlaneName(p.buffer);
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_setContourPlaneName (void * userdata,
                                           vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;

  it->setContourPlaneName(p.buffer);
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_setOpacityPlaneName (void * userdata,
							     vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  it->setOpacityPlaneName(p.buffer);
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_setHeightPlaneName (void * userdata,
                                           vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;

  it->setHeightPlaneName(p.buffer);
  return 0;
}


// static
int nmg_Graphics_Implementation::handle_setMinColor
                                 (void * userdata, vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  double c [3];

  CHECKF(it->decode_setMinColor(p.buffer, c), "handle_setMinColor");
  it->setMinColor(c);
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_setMaxColor
                                 (void * userdata, vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  double c [3];

  CHECKF(it->decode_setMaxColor(p.buffer, c), "handle_setMaxColor");
  it->setMaxColor(c);
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_setPatternMapName
                                 (void * userdata, vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;

  it->setPatternMapName(p.buffer);
  return 0;
}

/*
// static
int nmg_Graphics_Implementation::handle_enableRulergrid
                                 (void * userdata, vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  int value;

  CHECKF(it->decode_enableRulergrid(p.buffer, &value), "handle_enableRulergrid");
  it->enableRulergrid(value);
  return 0;
}
*/

// static
int nmg_Graphics_Implementation::handle_setRulergridAngle
                                 (void * userdata, vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  float angle;

  CHECKF(it->decode_setRulergridAngle(p.buffer, &angle),  "handle_setRulergridAngle");
  it->setRulergridAngle(angle);
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_setRulergridColor
                                 (void * userdata, vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  int r, g, b;

  CHECKF(it->decode_setRulergridColor(p.buffer, &r, &g, &b), "handle_setRulergridColor");
  it->setRulergridColor(r, g, b);
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_setRulergridOffset
                                 (void * userdata, vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  float x, y;

  CHECKF(it->decode_setRulergridOffset(p.buffer, &x, &y), "handle_setRulergridOffset");
  it->setRulergridOffset(x, y);
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_setNullDataAlphaToggle
                                 (void * userdata, vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  int val;

  CHECKF(it->decode_setNullDataAlphaToggle(p.buffer, &val), "handle_setNullDataAlphaToggle");
  it->setNullDataAlphaToggle(val);
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_setRulergridOpacity
                                 (void * userdata, vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  float alpha;

  CHECKF(it->decode_setRulergridOpacity(p.buffer, &alpha), "handle_setRulergridOpacity");
  it->setRulergridOpacity(alpha);
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_setRulergridScale
                                 (void * userdata, vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  float scale;

  CHECKF(it->decode_setRulergridScale(p.buffer, &scale), "handle_setRulergridScale");
  it->setRulergridScale(scale);
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_setRulergridWidths
                                 (void * userdata, vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  float x, y;

  CHECKF(it->decode_setRulergridWidths(p.buffer, &x, &y), "handle_setRulergridWidths");
  it->setRulergridWidths(x, y);
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_setSpecularity
                                 (void * userdata, vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  int shiny;

  CHECKF(it->decode_setSpecularity(p.buffer, &shiny), "handle_setSpecularity");
  it->setSpecularity(shiny);
  return 0;
}


int nmg_Graphics_Implementation::handle_setDiffusePercent
				 (void * userdata, vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  float diffuse;

  CHECKF(it->decode_setDiffusePercent(p.buffer, &diffuse), "handle_setDiffusePercent");
  it->setDiffusePercent(diffuse);
  return 0;
}

int nmg_Graphics_Implementation::handle_setSurfaceAlpha
                                 (void * userdata, vrpn_HANDLERPARAM p) {
   nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
   float surface_alpha;

   CHECKF(it->decode_setSurfaceAlpha(p.buffer, &surface_alpha), "handle_setSurfaceAlpha");
   it->setSurfaceAlpha(surface_alpha);
   return 0;
}

// static
int nmg_Graphics_Implementation::handle_setSpecularColor
                                 (void * userdata, vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  float specular_color;

  CHECKF(it->decode_setSpecularColor(p.buffer, &specular_color), "handle_setSpecularColor");
  it->setSpecularColor(specular_color);
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_setSphereScale
                                 (void * userdata, vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  float scale;

  CHECKF(it->decode_setSphereScale(p.buffer, &scale), "handle_setSphereScale");
  it->setSphereScale(scale);
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_setTesselationStride
                                 (void * userdata, vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  int stride;

  CHECKF(it->decode_setTesselationStride(p.buffer, &stride), "handle_setTesselationStride");
  it->setTesselationStride(stride);
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_setTextureMode
                                 (void * userdata, vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  TextureMode m;
  TextureTransformMode xm;

  CHECKF(it->decode_setTextureMode(p.buffer, &m, &xm), "handle_setTextureMode");
  it->setTextureMode(m, xm);
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_setTextureScale
                                 (void * userdata, vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  float scale;

  CHECKF(it->decode_setTextureScale(p.buffer, &scale), "handle_setTextureScale");
  it->setTextureScale(scale);
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_setTrueTipScale
                                 (void * userdata, vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  float scale;

  CHECKF(it->decode_setTrueTipScale(p.buffer, &scale), "handle_setTrueTipScale");
  it->setTrueTipScale(scale);
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_setUserMode
                                 (void * userdata, vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  int oldMode, oldStyle, newMode, style;

  CHECK(it->decode_setUserMode(p.buffer, &oldMode, &oldStyle, &newMode, &style));
  it->setUserMode(oldMode, oldStyle, newMode, style);
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_setLightDirection
                                 (void * userdata, vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  q_vec_type v;

  CHECKF(it->decode_setLightDirection(p.buffer, v), "handle_setLightDirection");
  it->setLightDirection(v);
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_resetLightDirection
                                 (void * userdata, vrpn_HANDLERPARAM) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;

  it->resetLightDirection();
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_addPolylinePoint
                                 (void * userdata, vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  PointType * pp;

  pp = new PointType [2];

  CHECKF(it->decode_addPolylinePoint(p.buffer, pp), "handle_addPolylinePoint");
  it->addPolylinePoint(pp);
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_addPolySweepPoints
                                 (void * userdata, vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  PointType topL, botL, topR, botR;

  CHECKF(it->decode_addPolySweepPoints(p.buffer, topL, botL, topR, botR), "handle_addPolySweepPoints");
  it->addPolySweepPoints( topL, botL, topR, botR);
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_emptyPolyline
                                 (void * userdata, vrpn_HANDLERPARAM) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;

  it->emptyPolyline();
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_setRubberLineStart
                                 (void * userdata, vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  float pt [2];

  CHECKF(it->decode_setRubberLineStart(p.buffer, pt), "handle_setRubberLineStart");
  it->setRubberLineStart(pt);
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_setRubberLineEnd
                                 (void * userdata, vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  float pt [2];

  CHECKF(it->decode_setRubberLineEnd(p.buffer, pt), "handle_setRubberLineEnd");
  it->setRubberLineEnd(pt);
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_setRubberSweepLineStart
                                 (void * userdata, vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  PointType left, right;

  CHECKF(it->decode_setRubberSweepLineStart(p.buffer, left, right), "handle_setRubberSweepLineStart");
  it->setRubberSweepLineStart(left, right);
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_setRubberSweepLineEnd
                                 (void * userdata, vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  PointType left, right;

  CHECKF(it->decode_setRubberSweepLineEnd(p.buffer, left, right), "handle_setRubberSweepLineEnd");
  it->setRubberSweepLineEnd(left, right);
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_setScanlineEndpoints
                                 (void * userdata, vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  float pt [6];

  CHECKF(it->decode_setScanlineEndpoints(p.buffer, pt), "handle_setRubberSweepLineEnd");
  it->setScanlineEndpoints(&(pt[0]), &(pt[3]));
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_displayScanlinePosition
				(void *userdata, vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  int enable;
  CHECKF(it->decode_displayScanlinePosition(p.buffer, &enable), "handle_displayScanlinePosition");
  it->displayScanlinePosition(enable);
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_positionAimLine
                                 (void * userdata, vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  PointType top, bottom;

  CHECKF(it->decode_positionAimLine(p.buffer, top, bottom), "handle_positionAimLine");
  it->positionAimLine(top, bottom);
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_positionRubberCorner
                                 (void * userdata, vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  float x0, y0, x1, y1;

  CHECKF(it->decode_positionRubberCorner(p.buffer, &x0, &y0, &x1, &y1), "handle_positionRubberCorner");
  it->positionRubberCorner(x0, y0, x1, y1);
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_positionSweepLine
                                 (void * userdata, vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  PointType topL, bottomL, topR, bottomR;

  CHECKF(it->decode_positionSweepLine(p.buffer, topL, bottomL, topR, bottomR), "handle_positionSweepLine");
  it->positionSweepLine(topL, bottomL, topR, bottomR);
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_positionSphere
                                 (void * userdata, vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  float x, y, z;

  CHECKF(it->decode_positionSphere(p.buffer, &x, &y, &z), "handle_positionSphere");
  it->positionSphere(x, y, z);
  return 0;
}

/*
// genetic textures:
// Call back function called when the gaEngine_Remote gets an
// evaluation_complete message. This occurs when the selected texture
// sent by the gaEngine_Implementation had been completely transferred
// accross the network and is ready to be mapped onto the surface.
//static
int nmg_Graphics_Implementation::genetic_textures_ready( void *p ) {
  nmg_Graphics_Implementation * it = (  nmg_Graphics_Implementation * )p;
  
  // make sure gl calls are directed to the right context
  v_gl_set_context_to_vlib_window();

  glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );

  glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, 
               g_tex_blend_func[GENETIC_TEX_ID]);

  glBindTexture(GL_TEXTURE_2D, tex_ids[GENETIC]);
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

#if defined(sgi) || defined(__CYGWIN__)
  if (gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, 512, 512,
                        GL_RGB, GL_FLOAT, it->gaRemote->data[0]) != 0) { 
    printf(" Error making mipmaps, using texture instead.\n");
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
		 512, 512, 0, GL_RGB, GL_FLOAT, it->gaRemote->data[0]);
    if (glGetError()!=GL_NO_ERROR) {
      printf(" Error making genetic texture.\n");
    }
  }
#else
  // Should use glBindTexture so it doesn't conflict w/Rulergrid...
  //   glBindTexture( GL_TEXTURE_2D, &texName );
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
	       512, 512, 0, GL_RGB, GL_FLOAT, it->gaRemote->data[0]);
 #ifndef FLOW
  if (glGetError()!=GL_NO_ERROR) {
    printf(" Error making genetic texture.\n");
  }
 #endif
#endif

  return 0;
}

// Sends the data to the genetic algorithm.
// The variable list is either the entire list of plane names,
// or the selected list from the "Set Genetic Textures Parameter"
// pop-up window. 
// Only the selected datasets are sent to the genetic alg.
void nmg_Graphics_Implementation::sendGeneticTexturesData (
			   int number_of_variables, 
			   char **variable_list ) {
  BCGrid *grid = g_inputGrid;

  gaRemote->number_of_variables( number_of_variables );
  gaRemote->mainloop();
  gaRemote->variableList( number_of_variables, variable_list );
  gaRemote->mainloop();

  int width  = grid->numX();
  int height = grid->numY();
  gaRemote->dimensions( number_of_variables, width, height );
  gaRemote->mainloop();

  float **input = new float*[height];
  if (!input) {
    fprintf(stderr, "nmg_Graphics_Implementation::sendGeneticTexturesData:  "
                    "Out of memory.\n");
    return;
  }
  for ( int i = 0; i < number_of_variables; i++ ) {
    for ( BCPlane *head = grid->head(); head; head = head ->next() ) {
      if ( !strcmp( variable_list[ i ], head->name()->Characters() ) ) {
	float min = head->minValue();
	float max = head->maxValue();
	for ( int j = 0; j < height; j++ ) {
	  input[ j ] = new float[ width ];
          if (!input[j]) {
            fprintf(stderr, "nmg_Graphics_Implementation::"
                            "sendGeneticTexturesData:  "
                            "Out of memory.\n");
            return;
          }
	  for ( int k = 0; k < width; k++ )
	    input[ j ][ k ] = ( head->value( k, j ) - min )/( max - min );
	  gaRemote->dataSet( i, j, input );
	  gaRemote->mainloop();
	  delete [] input[ j ];
	}
      }
    }
  }

  delete [] input;
  causeGridRedraw();
}


// This is a callback function called when the gaEngine_Remote 
// gets a connection. It calls sendGeneticTextureData which sends 
// the number of variables, names of the planes to be used in the genetic alg.
// and all the data sets. 
//
int nmg_Graphics_Implementation::send_genetic_texture_data( void *userdata ) {
  nmg_Graphics_Implementation * it = ( nmg_Graphics_Implementation * )userdata;

  BCGrid *grid = g_inputGrid;

  int number_of_variables  = grid->numPlanes();
  char **variable_list     = new char*[ number_of_variables ];

  BCPlane *head = grid->head();

  if (!variable_list) {
    fprintf(stderr, "nmg_Graphics_Implementation::send_genetic_texture_data:  "
                    "Out of memory.\n");
    return -1;
  }

  int i;
  for ( i = 0; ( i < number_of_variables ) && head; i++ ) {
    variable_list[ i ] = new char[ head->name()->Length() ];
    if (!variable_list[i]) {
      fprintf(stderr, "nmg_Graphics_Implementation::"
                      "send_genetic_texture_data:  "
                      "Out of memory.\n");
      return -1;
    }
    strcpy( variable_list[ i ], head->name()->Characters() );
    head = head->next();
  }

  it->sendGeneticTexturesData ( number_of_variables, variable_list );

  for ( i = 0; i < number_of_variables; i++ )
    delete [] variable_list[ i ];
  delete [] variable_list;

  return 0;
}
*/
// static
/*
int nmg_Graphics_Implementation::handle_enableGeneticTextures (void *userdata,
						       vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = ( nmg_Graphics_Implementation * )userdata;
  int value;
  CHECKF(it->decode_enableGeneticTextures(p.buffer, &value), "handle_enableGeneticTextures");
  it->enableGeneticTextures( value );
  return 0;
}
*/
/*
// static
int nmg_Graphics_Implementation::handle_sendGeneticTexturesData(void *userdata,
						      vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation * )userdata;
  int number_of_variables;
  char **variable_list;
  CHECKF(it->decode_sendGeneticTexturesData
          (p.buffer, &number_of_variables, &variable_list), "handle_sendGeneticTexturesData");
  it->sendGeneticTexturesData( number_of_variables, variable_list );
  for ( int i = 0; i < number_of_variables; i++ ) {
    if (variable_list && variable_list[i]) {
      delete [] variable_list[i];
    }
  }
  if (variable_list) {
    delete [] variable_list;
  }
  return 0;
}
*/

//
// Realign Textures Handlers
//

// static
int nmg_Graphics_Implementation::handle_createRealignTextures (void *userdata,
						       vrpn_HANDLERPARAM p) {

  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation * )userdata;
  it->createRealignTextures( p.buffer );
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_setRealignTextureSliderRange
( void *userdata, vrpn_HANDLERPARAM p) {
  float low, hi;
  
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation * )userdata;
  CHECKF(it->decode_setRealignTextureSliderRange(p.buffer, &low, &hi), "handle_setRealignTextureSliderRange");
  it->setRealignTextureSliderRange( low, hi );
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_setRealignTexturesConversionMap
( void *userdata, vrpn_HANDLERPARAM p) {
  
  char *map = new char[100], *mapdir = new char[100];
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation * )userdata;
  CHECKF(it->decode_two_char_arrays(p.buffer, &map, &mapdir), "handle_setRealignTexturesConversionMap");
  it->setRealignTexturesConversionMap( map, mapdir );

  if (map) {
    delete [] map;
  }
  if (mapdir) {
    delete [] mapdir;
  }
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_computeRealignPlane (void *userdata,
						     vrpn_HANDLERPARAM p) {

  char *name = new char[100], *newname = new char[100];
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation * )userdata;
  CHECKF(it->decode_two_char_arrays(p.buffer, &name, &newname), "handle_computeRealignPlane");
  it->computeRealignPlane( name, newname );

  if (name) {
    delete [] name;
  }
  if (newname) {
    delete [] newname;
  }
  return 0;
}

/*
// static
int nmg_Graphics_Implementation::handle_enableRealignTextures (void *userdata,
						       vrpn_HANDLERPARAM p) {
  int on;
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation * )userdata;
  CHECKF(it->decode_enableRealignTextures(p.buffer, &on), "handle_enableRealignTextures");
  it->enableRealignTextures( on );
  return 0;
}
*/

// static
int nmg_Graphics_Implementation::handle_translateTextures (void *userdata,
						   vrpn_HANDLERPARAM p) {
  
  float dx, dy;
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation * )userdata;
  it->decode_dx_dy( p.buffer, &dx, &dy );
  it->translateTextures( 1, dx, dy );
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_scaleTextures (void *userdata,
					       vrpn_HANDLERPARAM p) {
  float dx, dy;
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation * )userdata;
  it->decode_dx_dy( p.buffer, &dx, &dy );
  it->scaleTextures( 1, dx, dy );
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_shearTextures (void *userdata,
					       vrpn_HANDLERPARAM p) {
  float dx, dy;
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation * )userdata;
  it->decode_dx_dy( p.buffer, &dx, &dy );
  it->shearTextures( 1, dx, dy );
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_rotateTextures (void *userdata,
						vrpn_HANDLERPARAM p) {
  float theta;
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation * )userdata;
  it->decode_rotateTextures( p.buffer, &theta );
  it->rotateTextures( 1, theta );
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_setTextureCenter (void *userdata,
						  vrpn_HANDLERPARAM p) {
  float dx, dy;
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation * )userdata;
  it->decode_dx_dy( p.buffer, &dx, &dy );
  it->setTextureCenter( dx, dy );
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_updateTexture (void *userdata,
                                                       vrpn_HANDLERPARAM p) {
  int whichTexture;
  char *image_name = NULL;
  int start_x, start_y, end_x, end_y;
  
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation * )userdata;
  it->decode_updateTexture( p.buffer, &whichTexture, &image_name, 
	&start_x, &start_y, &end_x, &end_y); // allocates space for image_name
  it->updateTexture(whichTexture, image_name, start_x, start_y,end_x,end_y);
  delete [] image_name;
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_setTextureTransform (void *userdata,
						vrpn_HANDLERPARAM p) {
  double xform[16];
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation * )userdata;
  it->decode_textureTransform( p.buffer, xform);
  it->setTextureTransform( xform );
  return 0;
}


// static
int nmg_Graphics_Implementation::handle_setViewTransform
               (void * userdata, vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation * )userdata;
  v_xform_type xform;
  it->decode_setViewTransform(p.buffer, &xform);
  it->setViewTransform(xform);
  return 0;
}


//
// Save the screen as an image
//



int nmg_Graphics_Implementation::handle_createScreenImage
(
   void              *userdata,
   vrpn_HANDLERPARAM  p
)
{
   char *filename = new char[512];

   ImageType type;

   nmg_Graphics_Implementation *it = (nmg_Graphics_Implementation *)userdata;
   it->decode_createScreenImage(p.buffer, &filename, &type);
   it->createScreenImage(filename, type);

   if (filename) {
     delete [] filename;
   }

   return 0;
}


