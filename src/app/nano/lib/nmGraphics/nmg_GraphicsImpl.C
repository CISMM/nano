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
#include <nmb_ColorMap.h>  // for ColorMap::lookup()
#include <nmb_ImgMagick.h>

#include <nmb_Dataset.h>
#include <nmb_Globals.h>
#include "nmg_Surface.h"
#include "nmg_SurfaceRegion.h"

#include "surface_util.h"
#include "openGL.h"  // for check_extension(), display_lists_in_x
#include "globjects.h"  // for replaceDefaultObjects()
#include "nmg_State.h"
#include "nmg_Globals.h"

#include "Timer.h"

// UGRAPHICS GLOBAL DEFINED IN MICROSCAPE.C
#include <UTree.h>
#include <URender.h>
extern UTree World;

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
    const int surfaceColor [3],
    const char * rulergridName,
    const char * vizName,
    vrpn_Connection * connection,
    unsigned int /*portNum*/ )

  : nmg_Graphics (connection, "nmg Graphics Implementation GL"),
    d_dataset (data),
    state (new nmg_State), 
    d_displayIndexList (new v_index [NUM_USERS]),
    d_textureTransformMode(RULERGRID_COORD),
    d_last_region(-1)
{
    if (d_dataset == NULL) {
        grid_size_x = 12;
        grid_size_y = 12;
    } else {
        grid_size_x = d_dataset->inputGrid->numX();
        grid_size_y = d_dataset->inputGrid->numY();
    }

    if (state == NULL) {
        fprintf(stderr, "ERROR: nmg_Graphics_Implementation: no memory available, abort.\n");
        exit (-1);
    }

    //fprintf(stderr,
    //"In nmg_Graphics_Implementation::nmg_Graphics_Implementation()\n");

    int i = 0;

    state->inputGrid = data->inputGrid;

    // This is risky...
    state->displayIndexList = d_displayIndexList;

    if (d_dataset == NULL) {
        state->inputGrid = NULL;
        strcpy(state->alphaPlaneName, "none");
        strcpy(state->colorPlaneName, "none");
        strcpy(state->contourPlaneName, "none");
        strcpy(state->heightPlaneName, "none");
        strcpy(state->opacityPlaneName, "none");
        strcpy(state->maskPlaneName, "none");
    } else {
        state->inputGrid = data->inputGrid;
        strcpy(state->alphaPlaneName, data->alphaPlaneName->string());
        strcpy(state->colorPlaneName, data->colorPlaneName->string());
        strcpy(state->contourPlaneName, data->contourPlaneName->string());
        strcpy(state->heightPlaneName, data->heightPlaneName->string());
        strcpy(state->opacityPlaneName, data->opacityPlaneName->string());
        strcpy(state->maskPlaneName, data->maskPlaneName->string());
    }
    
    //Figure out what capabilities we have.
    determine_GL_capabilities(state);
    
    /* initialize graphics  */
    //printf("Initializing graphics...\n");

    if ( v_open() != V_OK ) {
        exit(-1);
    }

    initDisplays();

    // make sure everything is going into the right gl context
    v_gl_set_context_to_vlib_window();
    
    /* Set up the viewing info */
  
    /* Set initial user mode */
    state->user_mode = USER_GRAB_MODE;

    /* set up user and object trees     */
    //printf("Creating the world...\n");
    v_create_world(NUM_USERS, d_displayIndexList);

    /* Set the background color to light blue */
    glClearColor(0.3, 0.3, 0.7,  0.0);
    
    setSurfaceColor(surfaceColor);
    
    initialize_globjects(state, NULL);  // load default font
    
    /* user routine to define user objects and override default objects */
    replaceDefaultObjects(state);
    
    v_replace_drawfunc(i, V_WORLD, draw_world, state);
    v_replace_lightingfunc(i, setup_lighting, state);    
    

    state->surface = new nmg_Surface;
    state->surface->changeDataset(data);
    
    //////////////////////////////////////////////////////////////////////
    // Build the display lists we'll need to draw the data, etc		    //
    //									                                //
    // Note:  This code removed as all grid building needs to happen	//
    //        through nmg_Surface classes				                //
    //////////////////////////////////////////////////////////////////////
    
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
    state->rulerPPM = new PPM (infile);
    if (!state->rulerPPM) {
      fprintf(stderr, "nmg_GraphicsImplementation:  Out of memory.\n");
      return;
    }
    if (!state->rulerPPM->valid) {
      fprintf(stderr, "Cannot read rulergrid PPM %s.\n", rulergridName);

      delete state->rulerPPM;  // plug memory leak?
      state->rulerPPM = NULL;  // show default ruler image instead
    }
  }

  if (vizName) {
    FILE *infile = fopen(vizName, "rb");
    if (!infile) {
      fprintf(stderr, "nmg_GraphicsImplementation:  no such file: %s\n",
		rulergridName);
      return;
    }
    state->vizPPM = new PPM (infile);
    if (!state->vizPPM) {
      fprintf(stderr, "nmg_GraphicsImplementation:  Out of memory.\n");
      return;
    }
    if (!state->vizPPM->valid) {
      fprintf(stderr, "Cannot read visualization PPM %s.\n", vizName);

      delete state->vizPPM;  // plug memory leak?
      state->vizPPM = NULL;
    }
  }

  initializeTextures();
  setupMaterials(); // this needs to come after initializeTextures because
		// shader initialization depends on texture ids


    // Even though we may not be using the vertex array extension, we still
    // use the vertex array to cache calculated normals
  if (state->surface->init(grid_size_x, grid_size_y) ) {
      fprintf(stderr," initialization of surface: out of memory [call 0].\n");
      exit(0);
  }

  state->positionList = new Position_list;
  state->positionListL = new Position_list;
  state->positionListR = new Position_list;

  // This section initializes the user mode to GRAB and sets an initial
  // position for the aimLine...
  BCPlane* plane = dataset->inputGrid->getPlaneByName
    (dataset->heightPlaneName->string());
  decoration->aimLine.moveTo(plane->minX(), plane->maxY(), plane);
  init_world_modechange( state, state->user_mode, 0, 0 );

  if (!connection) return;

  connection->register_handler(d_resizeViewport_type,
                               handle_resizeViewport,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_loadRulergridImage_type,
                               handle_loadRulergridImage,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_loadVizImage_type,
                               handle_loadVizImage,
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
/*
  connection->register_handler(d_enableUber_type,
                               handle_enableUber,
                               this, vrpn_ANY_SENDER);
*/
  connection->register_handler(d_enableTrueTip_type,
                               handle_enableTrueTip,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_setAlphaColor_type,
                               handle_setAlphaColor,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_setAlphaSliderRange_type,
                               handle_setAlphaSliderRange,
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
  connection->register_handler(d_setContourColor_type,
                               handle_setContourColor,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_setContourWidth_type,
                               handle_setContourWidth,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_setHandColor_type,
                               handle_setHandColor,
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
  connection->register_handler(d_setMaskPlaneName_type,
			   handle_setMaskPlaneName,
			   this, vrpn_ANY_SENDER);
  connection->register_handler(d_setSurfaceColor_type,
                               handle_setSurfaceColor,
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

  // Colormap Texture Handlers:
  connection->register_handler( d_createColormapTexture_type,
				handle_createColormapTexture,
				this, vrpn_ANY_SENDER);

  // Texture Colormap and Alpha Handlers:
  connection->register_handler( d_setTextureColormapSliderRange_type,
				handle_setTextureColormapSliderRange,
				this, vrpn_ANY_SENDER);
  connection->register_handler( d_setTextureColormapConversionMap_type,
				handle_setTextureColormapConversionMap,
				this, vrpn_ANY_SENDER);
  connection->register_handler( d_setTextureAlpha_type,
                handle_setTextureAlpha,
                this, vrpn_ANY_SENDER);

  connection->register_handler( d_updateTexture_type,
				handle_updateTexture,
				this, vrpn_ANY_SENDER);

  connection->register_handler( d_setTextureTransform_type,
				handle_setTextureTransform,
				this, vrpn_ANY_SENDER);
  connection->register_handler( d_setViewTransform_type,
				handle_setViewTransform,
				this, vrpn_ANY_SENDER);
  connection->register_handler( d_createScreenImage_type,
				handle_createScreenImage,
				this, vrpn_ANY_SENDER);
  connection->register_handler(d_setViztexScale_type,
                               handle_setViztexScale,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_setRegionMaskHeight_type,
                               handle_setRegionMaskHeight,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_setRegionControlPlaneName_type,
                               handle_setRegionControlPlaneName,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_createRegion_type,
                               handle_createRegion,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_destroyRegion_type,
                               handle_destroyRegion,
                               this, vrpn_ANY_SENDER); 
  connection->register_handler(d_associateAlpha_type,
                               handle_associateAlpha,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_associateFilledPolygons_type,
                               handle_associateFilledPolygons,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_associateStride_type,
                               handle_associateStride,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_associateTextureDisplayed_type,
                               handle_associateTextureDisplayed,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_associateTextureMode_type,
                               handle_associateTextureMode,
                               this, vrpn_ANY_SENDER);
  connection->register_handler(d_associateTextureTransformMode_type,
                               handle_associateTextureTransformMode,
                               this, vrpn_ANY_SENDER);

}

nmg_Graphics_Implementation::~nmg_Graphics_Implementation (void) {

  int i;

  for (i = 0; i < NUM_USERS; i++) {
    v_close_display(d_displayIndexList[i]);
  }

  delete state->surface;
/*
  if (sem_data) {
    delete [] sem_data;
  }
*/
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

/** Get the total graphics state from this object */
nmg_State * nmg_Graphics_Implementation::getState()
{
    return state;
}

/** If we load a different stream file or connect to a new AFM,
 * graphics needs to start watching a new dataset.
 */
void nmg_Graphics_Implementation::changeDataset( nmb_Dataset * data)
{
  d_dataset = data;
  grid_size_x = d_dataset->inputGrid->numX();
  grid_size_y = d_dataset->inputGrid->numY();

  state->inputGrid = data->inputGrid;
  strcpy(state->alphaPlaneName, data->alphaPlaneName->string());
  strcpy(state->colorPlaneName, data->colorPlaneName->string());
  strcpy(state->contourPlaneName, data->contourPlaneName->string());
  strcpy(state->heightPlaneName, data->heightPlaneName->string());
  strcpy(state->maskPlaneName, data->maskPlaneName->string());

  state->surface->changeDataset(data);
  state->colormapTexture.changeDataset(data);
  state->rulergridTexture.changeDataset(data);
  state->videoTexture.changeDataset(data);
  state->remoteDataTexture.changeDataset(data);
  state->visualizationTexture.changeDataset(data);

  // If there is an existing region, it won't be applicable to new 
  // data set. 
  if (d_last_region != -1) {
      state->surface->destroyRegion(d_last_region);
      d_last_region = -1;
  }

  if (d_nulldata_region != -1) {
      state->surface->destroyRegion(d_nulldata_region);
      d_nulldata_region = -1;
  }

  BCPlane * plane = d_dataset->inputGrid->getPlaneByName(d_dataset->heightPlaneName->string());
  if (plane) {
      // EXPERIMENTAL. Mask out null data on default surface region
      d_nulldata_region = state->surface->createNewRegion();
      state->surface->setRegionControl(plane, d_nulldata_region);
      state->surface->deriveMaskPlane(d_nulldata_region);
  }

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
#ifdef V_GLUT
  v_gl_set_context_to_vlib_window();
  glutReshapeWindow(width, height);
#endif
}

void nmg_Graphics_Implementation::getViewportSize(int *width, int *height) {
//fprintf(stderr, "ngi get viewport size\n");
   *width  = v_display_table[d_displayIndexList[0]].viewports[0].fbExtents[0];
   *height = v_display_table[d_displayIndexList[0]].viewports[0].fbExtents[1];
//fprintf(stderr, "ngi get viewport size done (%d, %d)\n", *width, *height);
}

void nmg_Graphics_Implementation::positionWindow(int x, int y) {
  // make sure gl calls are directed to the right context
  v_gl_set_context_to_vlib_window();
#ifdef V_GLUT
  // Doesn't take into account window title bar and borders. Add
  // the right numbers for win2k
    glutPositionWindow(x+3,y+23);
#endif
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
	if (infile) {
		state->rulerPPM = new PPM (infile);
		if (!state->rulerPPM->valid) {
			fprintf(stderr, "Cannot read rulergrid PPM %s.\n", name);
		} else {
			state->rulergridTexture.setImage(state->rulerPPM);
		}
		fclose(infile);
	} else {
		fprintf(stderr, "nmg_Graphics_Implementation: can't find file: %s\n",
		name);
	}
}

void nmg_Graphics_Implementation::loadVizImage (const char * name) {
//fprintf(stderr, "nmg_Graphics_Implementation::loadRulergridImage().\n");
	if (!name) {
		return;
	}
	FILE *infile = fopen(name, "rb");
	if (infile) {
		state->vizPPM = new PPM (infile);
		if (!state->vizPPM->valid) {
			fprintf(stderr, "Cannot read visualization PPM %s.\n", name);
		} else {
			state->visualizationTexture.setImage(state->vizPPM);
		}
		fclose(infile);
	} else {
		fprintf(stderr, 
			"nmg_Graphics_Implementation: can't find file: %s\n", 
			name);
	}
}

/*
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
  const int texture_size 
    = int (0.5 + pow( (float) 2, ceil(log( (float) ts_orig_) / log(2.0f))));

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
        if (state->tex_blend_func[RULERGRID_TEX_ID] == GL_MODULATE) {
          texture[ texel( y, x, 0) ] = 255;
          texture[ texel( y, x, 1) ] = 255;
          texture[ texel( y, x, 2) ] = 255;
          texture[ texel( y, x, 3) ] = 255;
        } else if (state->tex_blend_func[RULERGRID_TEX_ID] == GL_BLEND) {
          texture[ texel( y, x, 0) ] = 0;
          texture[ texel( y, x, 1) ] = 0;
          texture[ texel( y, x, 2) ] = 0;
          texture[ texel( y, x, 3) ] = 255;
        } else if (state->tex_blend_func[RULERGRID_TEX_ID] == GL_DECAL){
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
        if (state->tex_blend_func[RULERGRID_TEX_ID] == GL_BLEND) {
          texture[ texel( (myPPM->ny-1)-y, x, 0) ] = 
		(GLubyte)((float)r*state->ruler_opacity/255.0);
          texture[ texel( (myPPM->ny-1)-y, x, 1) ] = 
		(GLubyte)((float)g*state->ruler_opacity/255.0);
          texture[ texel( (myPPM->ny-1)-y, x, 2) ] = 
		(GLubyte)((float)b*state->ruler_opacity/255.0);
          texture[ texel( (myPPM->ny-1)-y, x, 3) ] = (GLubyte) 255;
        } else if (state->tex_blend_func[RULERGRID_TEX_ID] == GL_MODULATE) {
          texture[ texel( (myPPM->ny-1)-y, x, 0) ] =
                (GLubyte)((float)r*state->ruler_opacity/255.0);
          texture[ texel( (myPPM->ny-1)-y, x, 1) ] =
                (GLubyte)((float)g*state->ruler_opacity/255.0);
          texture[ texel( (myPPM->ny-1)-y, x, 2) ] =
                (GLubyte)((float)b*state->ruler_opacity/255.0);
          texture[ texel( (myPPM->ny-1)-y, x, 3) ] = (GLubyte) 255;
        } else { //if (state->tex_blend_func[RULERGRID_TEX_ID] == GL_DECAL){
          texture[ texel( (myPPM->ny-1)-y, x, 3) ] = (GLubyte)state->ruler_opacity;
          texture[ texel( (myPPM->ny-1)-y, x, 0) ] = r;
          texture[ texel( (myPPM->ny-1)-y, x, 1) ] = g;
          texture[ texel( (myPPM->ny-1)-y, x, 2) ] = b;
        }
    }
  }

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  glBindTexture(GL_TEXTURE_2D, state->tex_ids[RULERGRID_TEX_ID]);

  glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
  glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
  glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

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
  state->tex_installed_width[RULERGRID_TEX_ID] = texture_size;
  state->tex_installed_height[RULERGRID_TEX_ID] = texture_size;
  state->tex_image_width[RULERGRID_TEX_ID] = myPPM->nx;
  state->tex_image_height[RULERGRID_TEX_ID] = myPPM->ny;
  state->tex_image_offsetx[RULERGRID_TEX_ID] = 0;
  state->tex_image_offsety[RULERGRID_TEX_ID] = 0;

  delete [] texture;
}

void nmg_Graphics_Implementation::makeAndInstallVizImage(PPM *myPPM)
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
  const int texture_size 
    = int (0.5 + pow( (float) 2, ceil( log( (float) ts_orig_) / log(2.0f))));
  
  if (texture_size > 512) {
      fprintf (stderr,
               "Using viz texture of size %dx%d, "
               "which is larger than previously allowed\n",
               texture_size, texture_size);
  }
  
#if 0 // old error message for static-sized array
  if (texture_size > 512) {
      fprintf(stderr,"Not enough space for %dx%d viz texture\n",
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
          if (state->tex_blend_func[VISUALIZATION_TEX_ID] == GL_MODULATE) {
              texture[ texel( y, x, 0) ] = 255;
              texture[ texel( y, x, 1) ] = 255;
              texture[ texel( y, x, 2) ] = 255;
              texture[ texel( y, x, 3) ] = 0;
          } else if (state->tex_blend_func[VISUALIZATION_TEX_ID] == GL_BLEND) {
              texture[ texel( y, x, 0) ] = 0;
              texture[ texel( y, x, 1) ] = 0;
              texture[ texel( y, x, 2) ] = 0;
              texture[ texel( y, x, 3) ] = 0;
          } else if (state->tex_blend_func[VISUALIZATION_TEX_ID] == GL_DECAL){
              texture[ texel( y, x, 3) ] = 0;
          } else {
              texture[ texel( y, x, 0) ] = 255;
              texture[ texel( y, x, 1) ] = 255;
              texture[ texel( y, x, 2) ] = 255;
              texture[ texel( y, x, 3) ] = 0;
          }
      }
  }
  
  // Fill in the part of the texture that the PPM file covers
  // Invert Y because the coordinate system in the PPM file starts
  // in the upper left corner, and our coordinate system in the lower
  // left.
  for (x = 0; x < myPPM->nx; x++) {
      for (y = 0; y < myPPM->ny; y++) {
          myPPM->Tellppm(x,y, &r, &g, &b);
          
          //Assuming the file is just a gray scale image.  And that
          //the intensity denotes the "opacity".
          
          //Since it is assumed to be gray scale, r,g, and b should
          //be equal
          int alpha = r;
          
          texture[ texel( (myPPM->ny-1)-y, x, 0) ] = 255;
          texture[ texel( (myPPM->ny-1)-y, x, 1) ] = 255;
          texture[ texel( (myPPM->ny-1)-y, x, 2) ] = 255;
          texture[ texel( (myPPM->ny-1)-y, x, 3) ] = (GLubyte) alpha;
      }
  }
  
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  
  glBindTexture(GL_TEXTURE_2D, state->tex_ids[VISUALIZATION_TEX_ID]);
  
  glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
  glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
  glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

#ifdef  FLOW
//    glTexImage2D(GL_TEXTURE_2D, 0, 4,
//                 texture_size, texture_size,
//                 0, GL_RGBA, GL_UNSIGNED_BYTE,
//                 (const GLvoid*)texture);
//    if (glGetError()!=GL_NO_ERROR) {
//     printf(" Error making ruler texture.\n");
//    }
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
          printf(" Error making viz texture.\n");
      }
  }
#endif
  state->tex_installed_width[VISUALIZATION_TEX_ID] = texture_size;
  state->tex_installed_height[VISUALIZATION_TEX_ID] = texture_size;
  state->tex_image_width[VISUALIZATION_TEX_ID] = myPPM->nx;
  state->tex_image_height[VISUALIZATION_TEX_ID] = myPPM->ny;
  state->tex_image_offsetx[VISUALIZATION_TEX_ID] = 0;
  state->tex_image_offsety[VISUALIZATION_TEX_ID] = 0;

  delete [] texture;
}
*/

/**
 * Next display loop, regenerate vertex colors for the surface. 
 * Use cached normals and vertex coordinates. Provides moderate
 * speed up over calling causeGridRedraw. 
 */
void nmg_Graphics_Implementation::causeGridReColor (void) {
    state->surface->recolorSurface();
  // Don't cause geometry to be re-calculated !!
  //causeGridRedraw();
}

/**
 * Next display loop, regenerate all display lists for the surface. 
 * Probably a bad name - "Rebuild" instead? 
 */
void nmg_Graphics_Implementation::causeGridRedraw (void) {
  //fprintf(stderr, "nmg_Graphics_Implementation::causeGridRedraw().\n");
  BCPlane * plane = state->inputGrid->getPlaneByName(state->heightPlaneName);

  if (state->surface->redrawSurface(state)) {
    fprintf(stderr,
            "nmg_Graphics_Implementation::causeGridRebuild():  "
            "Couldn't build grid display lists\n");
    dataset->done = V_TRUE;
  }
  if (plane) {
    decoration->red.normalize(plane);
    decoration->green.normalize(plane);
    decoration->blue.normalize(plane);
    decoration->aimLine.normalize(plane);
  }
}

/** 
 * Re-allocate the vertex array that draws the surface. Call _ONLY_ 
 * if the resolution of the surface changes. Bad name - should be
 * called ReallocateGrid. 
 */
void nmg_Graphics_Implementation::causeGridRebuild (void) {
  // Even though we may not be using the vertex array extension, we still
  // use the vertex array to cache calculated normals
  if (state->surface->init(d_dataset->inputGrid->numX(),
                       d_dataset->inputGrid->numY()) )
  {
      fprintf(stderr," initialization of surface: out of memory [call 1].\n");
      exit(0);
  }

  grid_size_x = d_dataset->inputGrid->numX();
  grid_size_y = d_dataset->inputGrid->numY();

  if (state->surface->rebuildSurface(state)) {
    fprintf(stderr,
            "nmg_Graphics_Implementation::causeGridRebuild():  "
            "Couldn't build grid display lists\n");
    dataset->done = V_TRUE;
  }
}

void nmg_Graphics_Implementation::enableChartjunk (int on) {
//fprintf(stderr, "nmg_Graphics_Implementation::enableChartjunk().\n");
  if ( state->config_chartjunk == on )
    return;
  state->config_chartjunk = on;
}

void nmg_Graphics_Implementation::enableFilledPolygons (int on, int region) {
//fprintf(stderr, "nmg_Graphics_Implementation::enableFilledPolygons().\n");
  state->surface->setFilledPolygons(on, region);
}

void nmg_Graphics_Implementation::enableSmoothShading (int on) {
//fprintf(stderr, "nmg_Graphics_Implementation::enableSmoothShading().\n");
  state->config_smooth_shading = on;
}

void nmg_Graphics_Implementation::enableUber (int on) {
//fprintf(stderr, "nmg_Graphics_Implementation::enableUber(%d).\n", on);
  state->config_enableUber = on;
}

void nmg_Graphics_Implementation::enableTrueTip (int on) {
//fprintf(stderr, "nmg_Graphics_Implementation::enableTrueTip().\n");
  state->config_trueTip = on;
  fprintf(stderr, "Set state->config_trueTip to %d.\n", on);
}




void nmg_Graphics_Implementation::setAlphaColor (float r, float g, float b) {
//fprintf(stderr, "nmg_Graphics_Implementation::setAlphaColor().\n");
  state->alpha_r = r;
  state->alpha_g = g;
  state->alpha_b = b;
  makeCheckImage(state);
  buildAlphaTexture(state);
}

void nmg_Graphics_Implementation::setAlphaSliderRange (float low, float high) {
//fprintf(stderr, "nmg_Graphics_Implementation::setAlphaSliderRange().\n");
  state->alpha_slider_min = low;
  state->alpha_slider_max = high;
  causeGridRedraw();
}

void nmg_Graphics_Implementation::setColorMapDirectory (const char * dir) {
//fprintf(stderr, "nmg_Graphics_Implementation::setColorMapDirectory().\n");
  if (state->colorMapDir) {
    delete [] state->colorMapDir;
  }
  if (!dir) {
    state->colorMapDir = NULL;
    return;
  }
  state->colorMapDir = new char [1 + strlen(dir)];

  strcpy(state->colorMapDir, dir);
}

void nmg_Graphics_Implementation::setTextureDirectory (const char * dir) {
//fprintf(stderr, "nmg_Graphics_Implementation::setTextureDirectory().\n");
  if (state->textureDir) {
    delete [] state->textureDir;
  }
  if (!dir) {
    state->textureDir = NULL;
    return;
  }
  state->textureDir = new char [1 + strlen(dir)];
  strcpy(state->textureDir, dir);
}

void nmg_Graphics_Implementation::setColorMapName (const char * name) {
//fprintf(stderr, "nmg_Graphics_Implementation::setColorMapName().\n");

  if (strcmp(name, "none") == 0) {
    state->curColorMap = NULL;
  } else {
    state->colorMap.load_from_file(name, state->colorMapDir);
    state->curColorMap = &state->colorMap;
  }
  causeGridReColor();
}

void nmg_Graphics_Implementation::setColorMinMax (float low, float high) {
//fprintf(stderr, "nmg_Graphics_Implementation::setColorMinMax().\n");
  if ( (state->color_min != low) || (state->color_max != high) ) {
    state->color_min = low;
    state->color_max = high;
    causeGridReColor();
  }
}

void nmg_Graphics_Implementation::setDataColorMinMax (float low, float high) {
//fprintf(stderr, "nmg_Graphics_Implementation::setDataColorMinMax().\n");
  if ( (state->data_min_norm != low) || (state->data_max_norm != high) ) {
    state->data_min_norm = low;
    state->data_max_norm = high;
    causeGridReColor();
  }
}

void nmg_Graphics_Implementation::setOpacitySliderRange (float low, float high) {
  state->opacity_slider_min = low;
  state->opacity_slider_max = high;
  causeGridReColor();
}

void nmg_Graphics_Implementation::setContourColor (int r, int g, int b) {
//fprintf(stderr, "nmg_Graphics_Implementation::setContourColor().\n");
  state->contour_r = r;
  state->contour_g = g;
  state->contour_b = b;
  buildContourTexture(state);
  causeGridRedraw();
}

void nmg_Graphics_Implementation::setContourWidth (float x) {
//fprintf(stderr, "nmg_Graphics_Implementation::setContourWidth().\n");
  state->contour_width = x;
  buildContourTexture(state);
  causeGridRedraw();
}

void nmg_Graphics_Implementation::setHandColor (int c) {
    //fprintf(stderr, "nmg_Graphics_Implementation::setHandColor().\n");
  state->hand_color = c;
}

void nmg_Graphics_Implementation::setIconScale (float scale) {
//fprintf(stderr, "nmg_Graphics_Implementation::setIconScale().\n");
  state->icon_scale = scale;
}

void nmg_Graphics_Implementation::enableCollabHand (vrpn_bool on) {
  ::enableCollabHand(state, on);  // from globjects.c
  state->draw_collab_hand = on;
}

void nmg_Graphics_Implementation::setCollabHandPos(double pos[], double quat[])
{
    //fprintf(stderr, "nmg_Graphics_Implementation::setCollabHandPos().\n");
  int i;
  for (i = 0; i < 3; i++) {
    state->collabHandPos[i] = pos[i];
    state->collabHandQuat[i] = quat[i];
  }
  state->collabHandQuat[3] = quat[3];

  state->position_collab_hand = 1;
}

void nmg_Graphics_Implementation::setCollabMode(int mode)
{
    //fprintf(stderr, "nmg_Graphics_Implementation::setCollabMode().\n");
  state->collabMode = mode;
}


// virtual
void nmg_Graphics_Implementation::setAlphaPlaneName (const char * n) {
  strcpy(state->alphaPlaneName, n);
}

// virtual
void nmg_Graphics_Implementation::setColorPlaneName (const char * n) {
  strcpy(state->colorPlaneName, n);

  // This function is called when the user chooses a new colormap plane,
  // or when the use hits the button "Autoscale". These are the
  // only times we want to change what "0" and "1" mean for
  // the state->data_min_norm and state->data_max_norm variables. So we set
  // state->data_m* here. 
  nmb_PlaneSelection planes;  planes.lookup(d_dataset);

  if (planes.color != NULL) {
      state->data_min = planes.color->minNonZeroValue();
      state->data_max = planes.color->maxNonZeroValue();

      // Colormap drift compensation. Reset the first line average numbers so
      // the next time we start a scan, we can drift-compensate the color map.
      decoration->first_line_avg = 0;
      decoration->first_line_avg_prev = d_dataset->getFirstLineAvg(planes.color);
  }

  causeGridReColor();

}

// virtual
void nmg_Graphics_Implementation::setContourPlaneName (const char * n) {
  strcpy(state->contourPlaneName, n);
  causeGridRedraw();
}

// virtual
void nmg_Graphics_Implementation::setHeightPlaneName (const char * n) {
  strcpy(state->heightPlaneName, n);
  BCPlane * plane = d_dataset->inputGrid->getPlaneByName(n);
  if (!plane) return;
  // EXPERIMENTAL. Mask out null data on default surface region
  if (d_nulldata_region == -1) {
      d_nulldata_region = state->surface->createNewRegion();
  }
  state->surface->setRegionControl(plane, d_nulldata_region);
  state->surface->deriveMaskPlane(d_nulldata_region);
  // Grid size can change if we are replacing the empty height plane. 
  if ( (grid_size_x != d_dataset->inputGrid->numX()) ||
       (grid_size_y != d_dataset->inputGrid->numY())    ) {
    causeGridRebuild();
  } else {
    causeGridRedraw();
  }
 
}

// virtual
void nmg_Graphics_Implementation::setOpacityPlaneName (const char * n) {
  strcpy(state->opacityPlaneName, n);
  causeGridRedraw();
}

void nmg_Graphics_Implementation::setMaskPlaneName (const char * n) {
  strcpy(state->maskPlaneName, n);
}

void nmg_Graphics_Implementation::setRegionControlPlaneName (const char * n, int region) {
  BCPlane * plane = dataset->inputGrid->getPlaneByName(n);
  state->surface->setRegionControl(plane, region);
}

void nmg_Graphics_Implementation::setSurfaceColor (const double c [4]) {
//fprintf(stderr, "nmg_Graphics_Implementation::setSurfaceColor().\n");
  memcpy(state->surfaceColor, c, 3 * sizeof(double));
}

void nmg_Graphics_Implementation::setSurfaceColor (const int c [4]) {
//fprintf(stderr, "nmg_Graphics_Implementation::setSurfaceColor().\n");
  state->surfaceColor[0] = c[0] / 255.0;
  state->surfaceColor[1] = c[1] / 255.0;
  state->surfaceColor[2] = c[2] / 255.0;
  //use alpha value from Opacity scrollbar
  state->surfaceColor[3] = state->surface_alpha;
}

//
// Colormap Texture Functions
//

/**
This function takes the name of a dataset and creates a texture map
out of the data in the dataset. 
The texture map is created by mapping data values to color values
based on the conversion map selected (i.e. blackbody, inverse rainbow,...)
Or in none is selected, it just does some simple mapping.

*/
void nmg_Graphics_Implementation::createColormapTexture( const char *name ) {
  // inputGrid is replaced with dataImages in this function so that
  // we don't have separate functions for building textures from ppm images
  // and AFM data (these changes allow this function to do what 
  // makeAndInstallRulerImage() was used for previously)

  nmb_Image *im = d_dataset->dataImages->getImageByName(name);
  if (!im) {
    fprintf(stderr, 
	"nmg_Graphics_Impl::createColormapTexture: image %s not found\n", name);
    return;
  }

  v_gl_set_context_to_vlib_window();
  state->colormapTexture.setImage(im);
  state->colormapTexture.setUpdateColorMap(true);
  state->colormapTexture.createTexture(false);

  return;
}

//
// Sets the conversion map used to convert data values to 
// color values when making the colormap textures.
//
void nmg_Graphics_Implementation::
setTextureColormapConversionMap( int which, const char *map, const char* /*mapdir*/ ) {
    if ( !strcmp( map, "none" ) ) {
        if (which == nmg_Graphics::COLORMAP) {
			state->colormapTexture.setColorMap(NULL);
			state->colormapTexture.setUpdateColorMap(true);
		} else if (which == nmg_Graphics::VIDEO) {
            state->videoTexture.setColorMap(NULL);
            state->videoTexture.setUpdateColorMap(true);
        }
    }
    else {
        if (which == nmg_Graphics::COLORMAP) {
			nmb_ColorMap colormap;
			colormap.load_from_file(map, state->colorMapDir);
			state->colormapTexture.setColorMap(&colormap);
			state->colormapTexture.setUpdateColorMap(true);
		} else if (which == nmg_Graphics::VIDEO) {
			nmb_ColorMap colormap;
			colormap.load_from_file(map, state->colorMapDir);
			state->videoTexture.setColorMap(&colormap);
			state->videoTexture.setUpdateColorMap(true);
		}
    }
}

//
// Sets the colormap texture colormap slider range:
//
void nmg_Graphics_Implementation::setTextureColormapSliderRange (
    int which,
    float data_min,
    float data_max,
    float color_min,
    float color_max) {

    if (which == nmg_Graphics::COLORMAP) {
		state->colormapTexture.setColorMapMinMax(data_min, data_max, color_min, color_max);
		state->colormapTexture.setUpdateColorMap(true);	
	} else if (which == nmg_Graphics::VIDEO) {
		state->videoTexture.setColorMapMinMax(data_min, data_max, color_min, color_max);
		state->videoTexture.setUpdateColorMap(true);
	}
}

//
// Sets the colormap texture colormap alpha:
//
void nmg_Graphics_Implementation::setTextureAlpha (
    int which,
    float alpha) {
  if (which == nmg_Graphics::COLORMAP) {
    state->colormapTexture.setOpacity(alpha);
  }
  else if (which == nmg_Graphics::VIDEO) {
    state->videoTexture.setOpacity(alpha);
  }
}

void nmg_Graphics_Implementation::initializeTextures(void)
{
  //fprintf(stderr, "initializing textures\n");

  // make sure gl calls are directed to the right context
  v_gl_set_context_to_vlib_window();

  //glGenTextures(1, &(state->contourTextureID));
  state->contourTextureID = 0;
  //buildContourTexture(state);

  //fprintf(stderr, "Initializing checkerboard texture.\n");
  //glGenTextures(1, &(state->alphaTextureID));
  state->alphaTextureID = 0;
  makeCheckImage(state);
  //buildAlphaTexture(state);

  //fprintf(stderr, "Initializing ruler texture.");
  if (state->rulerPPM) {
	  state->rulergridTexture.setImage(state->rulerPPM);
  } else {
    makeRulerImage(state);
    buildRulergridTexture(state);
    //fprintf(stderr, " Using default grid.\n");
  }
  if (report_gl_errors()) {
      printf(" Error making ruler texture.\n");
  }

  if (state->vizPPM) {
	  state->visualizationTexture.setImage(state->vizPPM);
  }

  if (report_gl_errors()) {
      printf(" Error making viz texture.\n");
  }
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
	state->videoTexture.setImage(im);
	state->videoTexture.doFastUpdates(true);
}

void nmg_Graphics_Implementation::updateTexture(int which_texture,
       const char *image_name,
       int start_x, int start_y,
       int /*end_x*/, int /*end_y*/)
{
    switch(which_texture) {
	case VIDEO:
       loadRawDataTexture(which_texture, image_name, start_x, start_y);
	   break;
    default:
       printf("nmg_Graphics_Implementation::updateTexture - don't know this"
		" texture id: %d", which_texture);
	   break;
    }
}

void nmg_Graphics_Implementation::setTextureTransform(double *xform){
    int i;
/*
    printf("got TextureTransform\n");
    printf("was: ");
    for (i = 0; i < 16; i++)
        printf("%g ", state->surfaceModeTextureTransform[i]);
    printf("\n");
*/

    for (i = 0; i < 16; i++)
	state->surfaceModeTextureTransform[i] = xform[i];
/*
    printf("new: ");
    for (i = 0; i < 16; i++)
        printf("%g ", state->surfaceModeTextureTransform[i]);
    printf("\n");
*/
}


void nmg_Graphics_Implementation::setRulergridAngle (float v) {
//fprintf(stderr, "nmg_Graphics_Implementation::setRulergridAngle().\n");
  state->rulergrid_sin = sin(-v / 180.0 * M_PI);
  state->rulergrid_cos = cos(-v / 180.0 * M_PI);
  //causeGridRedraw();
}

void nmg_Graphics_Implementation::setRulergridColor (int r, int g, int b) {
//fprintf(stderr, "nmg_Graphics_Implementation::setRulergridColor().\n");
  state->ruler_r = r;
  state->ruler_g = g;
  state->ruler_b = b;

  // if they've specified a ruler image, keep that instead of replacing
  // it with the default set of orthogonal lines
  if (state->rulerPPM)
    state->rulergridTexture.setImage(state->rulerPPM);
  else {
    makeRulerImage(state);
    buildRulergridTexture(state);
  }
}

void nmg_Graphics_Implementation::setRulergridOffset (float x, float y) {
//fprintf(stderr, "nmg_Graphics_Implementation::setRulergridOffset().\n");
  state->rulergrid_xoffset = x;
  state->rulergrid_yoffset = y;
  //causeGridRedraw();
}

void nmg_Graphics_Implementation::setNullDataAlphaToggle( int v ) {
  state->null_data_alpha_toggle = v;
  causeGridReColor();
}

void nmg_Graphics_Implementation::setRulergridOpacity (float alpha) {
//fprintf(stderr, "nmg_Graphics_Implementation::setRulergridOpacity().\n");
  state->ruler_opacity = alpha;

  // if they've specified a ruler image, keep that instead of replacing
  // it with the default set of orthogonal lines
  if (state->rulerPPM)
    state->rulergridTexture.setImage(state->rulerPPM);
  else {
    makeRulerImage(state);
    buildRulergridTexture(state);
  }
}

void nmg_Graphics_Implementation::setRulergridScale (float s) {
//fprintf(stderr, "nmg_Graphics_Implementation::setRulergridScale().\n");
  state->rulergrid_scale = s;
  //causeGridRedraw();
}

void nmg_Graphics_Implementation::setRulergridWidths (float x, float y) {
//fprintf(stderr, "nmg_Graphics_Implementation::setRulergridWidths().\n");
  state->ruler_width_x = x;
  state->ruler_width_y = y;

  // if they've specified a ruler image, keep that instead of replacing
  // it with the default set of orthogonal lines
  if (state->rulerPPM)
    state->rulergridTexture.setImage(state->rulerPPM);
  else {
    makeRulerImage(state);
    buildRulergridTexture(state);
  }
  causeGridRedraw();
}

void nmg_Graphics_Implementation::setSpecularity (int s) {
//fprintf(stderr, "nmg_Graphics_Implementation::setSpecularity().\n");
  state->shiny = s;
  //causeGridRedraw();
}

void nmg_Graphics_Implementation::setLocalViewer (vrpn_bool lv) {
//fprintf(stderr, "nmg_Graphics_Implementation::setLocalViewer().\n");
  state->local_viewer = lv;
  //causeGridRedraw();
}


void nmg_Graphics_Implementation::setDiffusePercent (float d) {
//fprintf(stderr, "nmg_Graphics_Implementation::setDiffusePercent().\n");
  state->diffuse = d;
  //causeGridRedraw();
}

void nmg_Graphics_Implementation::setSurfaceAlpha (float a, int region) {
//fprintf(stderr, "nmg_Graphics_Implementation::setSurfaceAlpha().\n");
  state->surface->setAlpha(a, region);
}

void nmg_Graphics_Implementation::setSpecularColor (float s) {
//fprintf(stderr, "nmg_Graphics_Implementation::setSpecularColor().\n");
  state->specular_color = s;
  //causeGridRedraw();
}

void nmg_Graphics_Implementation::setSphereScale (float s) {
//fprintf(stderr, "nmg_Graphics_Implementation::setSphereScale().\n");
  state->sphere_scale = s;
}

void nmg_Graphics_Implementation::setTesselationStride (int s, int region) {
//fprintf(stderr, "nmg_Graphics_Implementation::setTesselationStride(%d).\n", s);

  state->surface->setStride(s, region);
}

void nmg_Graphics_Implementation::setTextureMode (TextureMode m,
	TextureTransformMode xm, int region) {
//fprintf(stderr, "nmg_Graphics_Implementation::setTextureMode().\n");

  switch (m) {

    case NO_TEXTURES:
//      fprintf(stderr, "nmg_Graphics_Implementation:  no textures.\n");
      state->texture_mode = GL_FALSE;
	  World.Do(&URender::SetProjTextureAll, NULL);
	  state->currentProjectiveTexture = NULL;
      break;
    case CONTOUR:
      state->texture_mode = GL_TEXTURE_1D;
      break;
    case RULERGRID:
//        fprintf(stderr,"nmg_Graphics_Implementation: entering RULERGRID mode.\n");
		state->texture_mode = GL_TEXTURE_2D;
		World.Do(&URender::SetProjTextureAll, &(state->rulergridTexture));
		state->currentProjectiveTexture = &(state->rulergridTexture);
		break;
    case ALPHA:
#ifndef _WIN32
//        fprintf(stderr, "nmg_Graphics_Implementation:  entering ALPHA mode.\n");
		state->texture_mode = GL_TEXTURE_3D_EXT;
#else
		fprintf(stderr, "nmg_Graphics_Implementation:"
			" ALPHA mode not available under WIN32\n");
		state->texture_mode = GL_FALSE;
#endif
      break;
    case COLORMAP:
//    fprintf(stderr, "nmg_Graphics_Implementation: entering COLORMAP mode.\n");
		state->texture_mode = GL_TEXTURE_2D;
		World.Do(&URender::SetProjTextureAll, &(state->colormapTexture));
		state->currentProjectiveTexture = &(state->colormapTexture);
		break;
    case VIDEO:
//    fprintf(stderr, "nmg_Graphics_Implementation: entering SEM_DATA mode.\n");
		state->texture_mode = GL_TEXTURE_2D;
		World.Do(&URender::SetProjTextureAll, &(state->videoTexture));
		state->currentProjectiveTexture = &(state->videoTexture);
		break;
    case REMOTE_DATA:
//fprintf(stderr, "nmg_Graphics_Implementation:  entering REMOTE_DATA mode.\n");
		state->texture_mode = GL_TEXTURE_2D;
		World.Do(&URender::SetProjTextureAll, &(state->remoteDataTexture));
		state->currentProjectiveTexture = &(state->remoteDataTexture);
		break;
    case VISUALIZATION:
        state->texture_mode = GL_TEXTURE_2D;
		World.Do(&URender::SetProjTextureAll, &(state->visualizationTexture));
		state->currentProjectiveTexture = &(state->visualizationTexture);
        break;
    default:
		fprintf(stderr, "nmg_Graphics_Implementation::setTextureMode:  "
					  "Unknown texture mode %d.\n", m);
		state->texture_mode = GL_FALSE;
		break;
  }
  switch (xm) {
    case RULERGRID_COORD:
//    fprintf(stderr, "nmg_Graphics_Implementation: RULERGRID_COORD mode\n");
      break;
    case VIZTEX_COORD:
      break;
    case SURFACE_REGISTRATION_COORD:
//    fprintf(stderr, "nmg_Graphics_Implementation: SURFACE_REGISTRATION_COORD mode\n");
      break;
	case MODEL_REGISTRATION_COORD:
//    fprintf(stderr, "nmg_Graphics_Implementation: MODEL_REGISTRATION_COORD mode\n");
      break;
    default:
      fprintf(stderr, "nmg_Graphics_Implementation::setTextureMode:  "
		"Unknown texture transform mode %d.\n", xm);
      break;
  }
  d_textureMode = m;
  d_textureTransformMode = xm;
  // communicate to non nmg_Graphics code:
  state->texture_displayed = m;
  state->texture_transform_mode = xm;

  state->surface->setTextureDisplayed(m, region);
  state->surface->setTextureMode(state->texture_mode, region);
  state->surface->setTextureTransformMode(xm, region);
}

void nmg_Graphics_Implementation::setTextureScale (float f) {
//fprintf(stderr, "nmg_Graphics_Implementation::setTextureScale().\n");
  state->texture_scale = f;
  causeGridRedraw();
}

void nmg_Graphics_Implementation::setTrueTipScale (float f) {
//fprintf(stderr, "nmg_Graphics_Implementation::setTrueTipScale().\n");
  state->trueTipScale = f;
}


void nmg_Graphics_Implementation::setUserMode (int oldMode, int oldStyle,
					       int newMode, int style,
					       int tool) {
//fprintf(stderr, "nmg_Graphics_Implementation::setUserMode() old %d %d new %d %d.\n", oldMode, oldStyle, newMode, style);
  clear_world_modechange(state, oldMode, oldStyle, tool);
  init_world_modechange(state, newMode, style, tool);
  state->user_mode = newMode;
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

  return make_rubber_line_point(state, point, state->positionList);
}

void nmg_Graphics_Implementation::emptyPolyline (void) {
//fprintf(stderr, "nmg_Graphics_Implementation::emptyPolyline().\n");

    empty_rubber_line(state->positionList);
    empty_rubber_line(state->positionListL, 0);
    empty_rubber_line(state->positionListR, 1);
}

void nmg_Graphics_Implementation::setRubberLineStart (float p0, float p1) {
//fprintf(stderr, "nmg_Graphics_Implementation::setRubberLineStart().\n");
  state->rubberPt[0] = p0;
  state->rubberPt[1] = p1;
  // Now we find the corresponding z value.
  BCPlane* plane = state->inputGrid->getPlaneByName
                    (state->heightPlaneName);
  if (plane == NULL) {
      fprintf(stderr, "Error in setRubberLineStart: could not get plane!\n");
      return ;
  }
  plane->valueAt(&state->rubberPt[3], p0, p1);
  state->rubberPt[3] *= plane->scale();
}

void nmg_Graphics_Implementation::setRubberLineEnd (float p2, float p3 ) {
//fprintf(stderr, "nmg_Graphics_Implementation::setRubberLineEnd().\n");
  state->rubberPt[4] = p2;
  state->rubberPt[5] = p3;
  // Now we find the corresponding z value.
  BCPlane* plane = state->inputGrid->getPlaneByName
                    (state->heightPlaneName);
  if (plane == NULL) {
      fprintf(stderr, "Error in setRubberLineEnd: could not get plane!\n");
      return ;
  }
  plane->valueAt(&state->rubberPt[6], p2, p3);
  state->rubberPt[6] *= plane->scale();
}

void nmg_Graphics_Implementation::setRubberLineStart (const float p [2]) {
//fprintf(stderr, "nmg_Graphics_Implementation::setRubberLineStart().\n");
  state->rubberPt[0] = p[0];
  state->rubberPt[1] = p[1];
  // Now we find the corresponding z value.
  BCPlane* plane = state->inputGrid->getPlaneByName
                    (state->heightPlaneName);
  if (plane == NULL) {
      fprintf(stderr, "Error in setRubberLineStart: could not get plane!\n");
      return ;
  }
  plane->valueAt(&state->rubberPt[3], p[0], p[1]);
  state->rubberPt[3] *= plane->scale();
}

void nmg_Graphics_Implementation::setRubberLineEnd (const float p [2]) {
//fprintf(stderr, "nmg_Graphics_Implementation::setRubberLineEnd().\n");
  state->rubberPt[4] = p[0];
  state->rubberPt[5] = p[1];
  // Now we find the corresponding z value.
  BCPlane* plane = state->inputGrid->getPlaneByName
                    (state->heightPlaneName);
  if (plane == NULL) {
      fprintf(stderr, "Error in setRubberLineEnd: could not get plane!\n");
      return ;
  }
  plane->valueAt(&state->rubberPt[6], p[0], p[1]);
  state->rubberPt[6] *= plane->scale();
}

void nmg_Graphics_Implementation::setRubberSweepLineStart (const PointType Left,
							   const PointType Right) {
//fprintf(stderr, "nmg_Graphics_Implementation::setRubberLineStart().\n");
  state->rubberSweepPts[0][0] = Left[0];
  state->rubberSweepPts[0][1] = Left[1];
  // Now we find the corresponding z value.
  BCPlane* plane = state->inputGrid->getPlaneByName
                    (state->heightPlaneName);
  if (plane == NULL) {
      fprintf(stderr, "Error in setRubberSweepLineStart: could not get plane!\n");
  } else {
      plane->valueAt(&state->rubberSweepPts[0][2], Left[0], Left[1]);
      state->rubberSweepPts[0][2] *= plane->scale();
  }
  state->rubberSweepPts[1][0] = Right[0];
  state->rubberSweepPts[1][1] = Right[1];
  // Now we find the corresponding z value.
  if (plane) {
      plane->valueAt(&state->rubberSweepPts[1][2], Right[0], Right[1]);
      state->rubberSweepPts[1][2] *= plane->scale();
  }
}

void nmg_Graphics_Implementation::setRubberSweepLineEnd (const PointType Left,
							 const PointType Right) {
//fprintf(stderr, "nmg_Graphics_Implementation::setRubberLineEnd().\n");
    PointType avg_old, avg_new, dif;

    BCPlane* plane = state->inputGrid->getPlaneByName
                    (state->heightPlaneName);
    if (plane == NULL) {
        fprintf(stderr, "Error in setRubberSweepLineEnd: could not get plane!\n");
    }
    avg_old[0] = (state->rubberSweepPts[0][0] + state->rubberSweepPts[1][0])/2;
    avg_old[1] = (state->rubberSweepPts[0][1] + state->rubberSweepPts[1][1])/2;

    avg_new[0] = (Left[0] + Right[0])/2;
    avg_new[1] = (Left[1] + Right[1])/2;

    dif[0] = avg_new[0] - Left[0];
    dif[1] = avg_new[1] - Left[1];

    // adjust old points:
    state->rubberSweepPts[0][0] = avg_old[0] + dif[0]; //Left
    state->rubberSweepPts[0][1] = avg_old[1] + dif[1];

    state->rubberSweepPts[1][0] = avg_old[0] - dif[0]; //Right
    state->rubberSweepPts[1][1] = avg_old[1] - dif[1];
    if (plane) {
        // Set the Z values
      plane->valueAt(&state->rubberSweepPts[0][2], 
                     state->rubberSweepPts[0][0],
                     state->rubberSweepPts[0][1]);
      state->rubberSweepPts[0][2] *= plane->scale();
      plane->valueAt(&state->rubberSweepPts[1][2], 
                     state->rubberSweepPts[1][0],
                     state->rubberSweepPts[1][1]);
      state->rubberSweepPts[1][2] *= plane->scale();
    }
    PointType vr, vl;
    float d1, d2;
    vr[0] = Right[0] - state->rubberSweepPts[1][0];
    vr[1] = Right[1] - state->rubberSweepPts[1][1];
    vr[2] = 0;
    d1 = sqrt( vr[0]*vr[0] + vr[1]*vr[1]);

    vl[0] = Left[0] - state->rubberSweepPts[1][0];
    vl[1] = Left[1] - state->rubberSweepPts[1][1];
    vl[2] = 0;
    d2 = sqrt( vl[0]*vl[0] + vl[1]*vl[1]);

    if ( d1 < d2 ) {
	state->rubberSweepPts[0][3] = Left[0];
	state->rubberSweepPts[0][4] = Left[1];
	
	state->rubberSweepPts[1][3] = Right[0];
	state->rubberSweepPts[1][4] = Right[1];
    }
    else {
	state->rubberSweepPts[0][3] = Right[0];
	state->rubberSweepPts[0][4] = Right[1];
	
	state->rubberSweepPts[1][3] = Left[0];
	state->rubberSweepPts[1][4] = Left[1];
    }
    if (plane) {
        // Set the Z values
      plane->valueAt(&state->rubberSweepPts[0][5], 
                     state->rubberSweepPts[0][3],
                     state->rubberSweepPts[0][4]);
      state->rubberSweepPts[0][5] *= plane->scale();
      plane->valueAt(&state->rubberSweepPts[1][5], 
                     state->rubberSweepPts[1][3],
                     state->rubberSweepPts[1][4]);
      state->rubberSweepPts[1][5] *= plane->scale();
    }

    state->rubberSweepPtsSave[0][3] = state->rubberSweepPts[0][0];
    state->rubberSweepPtsSave[0][4] = state->rubberSweepPts[0][1];
    state->rubberSweepPtsSave[0][5] = state->rubberSweepPts[0][2];

    state->rubberSweepPtsSave[1][3] = state->rubberSweepPts[1][0];
    state->rubberSweepPtsSave[1][4] = state->rubberSweepPts[1][1];
    state->rubberSweepPtsSave[1][5] = state->rubberSweepPts[1][2];
}


void nmg_Graphics_Implementation::positionSweepLine (const PointType topL,
                                                     const PointType bottomL,
                                                     const PointType topR,
                                                     const PointType bottomR) {
//fprintf(stderr, "nmg_Graphics_Implementation::positionSweepLine().\n");
  make_sweep(state, topL, bottomL, topR, bottomR);
}

int nmg_Graphics_Implementation::addPolySweepPoints (const PointType /*topL*/,
                                                     const PointType /*bottomL*/,
                                                     const PointType /*topR*/,
                                                     const PointType /*bottomR*/) {
//fprintf(stderr, "nmg_Graphics_Implementation::addPolySweepPoints().\n");
    // Values passed in are ignored - rubber line drawn from
    // points in state->rubberSweepPts instead. 
    make_rubber_line_point(state, state->positionListL, 0);
    make_rubber_line_point(state, state->positionListR, 1);
    return 0;
}

void nmg_Graphics_Implementation::setScanlineEndpoints(const float p0[3],
		const float p1[3]){
//fprintf(stderr, "nmg_Graphics_Implementation::setScanlineEndpoints().\n");
  state->scanlinePt[0] = p0[0]; state->scanlinePt[1] = p0[1]; state->scanlinePt[2] = p0[2];
  state->scanlinePt[3] = p1[0]; state->scanlinePt[4] = p1[1]; state->scanlinePt[5] = p1[2];
}

void nmg_Graphics_Implementation::displayScanlinePosition(const int enable)
{
//fprintf(stderr, "nmg_Graphics_Implementation::displayScanlinePosition().\n");
    enableScanlinePositionDisplay(state, enable);
}

void nmg_Graphics_Implementation::positionAimLine (const PointType top,
                                                   const PointType bottom) {
//fprintf(stderr, "nmg_Graphics_Implementation::positionAimLine().\n");
  make_aim(top, bottom);
}

void nmg_Graphics_Implementation::positionRubberCorner
    (float minx, float miny, float maxx, float maxy, int highlight_mask) {
//fprintf(stderr, "nmg_Graphics_Implementation::positionRubberCorner().\n");
  make_rubber_corner(state, minx, miny, maxx, maxy, highlight_mask);
}

void nmg_Graphics_Implementation::positionRegionBox
  (float center_x,float center_y, float width,float height, float angle, 
   int highlight_mask) {
//fprintf(stderr, "nmg_Graphics_Implementation::positionRegionBox().\n");

    // Always draw the icon representing the current region. 
    make_region_box(state, center_x, center_y, width, height, 
                    angle, highlight_mask);

    // If we don't have a region yet, make one. 
    if ((d_last_region == -1) && (highlight_mask != REG_DEL)) {
        d_last_region = state->surface->createNewRegion();
        state->surface->associateStride(VRPN_FALSE, d_last_region);
        // Make this new region, outside of the box, blurry by setting
        // stride to 5. 
        setTesselationStride(5, d_last_region);
        // Always derive the mask plane the first time. 
        state->surface->deriveMaskPlane(center_x, center_y, width, 
                                   height, angle, d_last_region);    
    } else {
        // Don't derive a mask plane while dimensions are changing.
        // Only derive it if user has released widget. 
        // And state->surface knows not to re-derive the same surface. 
        // Note: highlight_mask is supposed to be a bitmask,
        // but is being interpreted here as a single value. 
        if (highlight_mask == REG_NULL || 
            highlight_mask == REG_PREP_TRANSLATE || 
            highlight_mask == REG_PREP_SIZE_WIDTH || 
            highlight_mask == REG_PREP_SIZE_HEIGHT || 
            highlight_mask == REG_PREP_SIZE) {
            // Create new mask plane for this region. Region is "on"
            // outside the box. Default region is on inside. 
            state->surface->deriveMaskPlane(center_x, center_y, width, 
                                       height, angle, d_last_region);    
        } else if (highlight_mask == REG_DEL) {
            state->surface->destroyRegion(d_last_region);
            d_last_region = -1;
        }
    }
}

void nmg_Graphics_Implementation::positionCrossSection
  (int id, int enable, float center_x,float center_y, 
   float widthL, float widthR,
   float angle, int highlight_mask) {
//fprintf(stderr, "nmg_Graphics_Implementation::positionRegionBox().\n");

    // In globjects.c
    move_cross_section(state, id, enable, center_x, center_y, 
                       widthL, widthR, angle, highlight_mask);
}

void nmg_Graphics_Implementation::hideCrossSection(int id ) {
    hide_cross_section( state,  id ) ;
}



void nmg_Graphics_Implementation::positionSphere (float x, float y, float z) {
//fprintf(stderr, "nmg_Graphics_Implementation::positionSphere().\n");
  position_sphere(state, x, y, z);
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

void nmg_Graphics_Implementation::
createScreenImage( const char* filename,
		   const char* /*type*/ )
{
  int w, h;
  unsigned char * pixels = NULL;

  screenCapture(&w, &h, &pixels, vrpn_FALSE);
  if (!pixels) {
    return;
  }

  if(nmb_ImgMagick::writeFileMagick(filename, NULL, w, h, 3, pixels)) {
      fprintf(stderr, "Failed to write screen to '%s'!\n", filename);
  }
  delete [] pixels;
}

void nmg_Graphics_Implementation::setViztexScale (float s) {
  state->viztex_scale = s;
}

void nmg_Graphics_Implementation::setRegionMaskHeight (float min_height,
                                                       float max_height,
                                                       int region) 
{
    state->surface->deriveMaskPlane(min_height, max_height, region);
}

int nmg_Graphics_Implementation::createRegion()
{
    return state->surface->createNewRegion();
}

void nmg_Graphics_Implementation::destroyRegion(int region)
{
    state->surface->destroyRegion(region);
}

void nmg_Graphics_Implementation::associateAlpha(vrpn_bool associate, int region)
{
    state->surface->associateAlpha(associate, region);
}

void nmg_Graphics_Implementation::associateFilledPolygons(vrpn_bool associate, int region)
{
    state->surface->associateFilledPolygons(associate, region);
}

void nmg_Graphics_Implementation::associateTextureDisplayed(vrpn_bool associate, int region)
{
    state->surface->associateTextureDisplayed(associate, region);
}

void nmg_Graphics_Implementation::associateTextureMode(vrpn_bool associate, int region)
{
    state->surface->associateTextureMode(associate, region);
}

void nmg_Graphics_Implementation::associateTextureTransformMode(vrpn_bool associate, int region)
{
    state->surface->associateTextureTransformMode(associate, region);
}

void nmg_Graphics_Implementation::associateStride(vrpn_bool associate, int region)
{
    state->surface->associateStride(associate, region);
}


void nmg_Graphics_Implementation::getLightDirection (q_vec_type * v) const {
//fprintf(stderr, "nmg_Graphics_Implementation::getLightDirection().\n");
  ::getLightDirection(v);
}

int nmg_Graphics_Implementation::getHandColor (void) const {
//fprintf(stderr, "nmg_Graphics_Implementation::getHandColor().\n");
  return state->hand_color;
}

int nmg_Graphics_Implementation::getSpecularity (void) const {
//fprintf(stderr, "nmg_Graphics_Implementation::getSpecularity().\n");
  return state->shiny;
}

/*
float nmg_Graphics_Implementation::getDiffusePercent (void) const {
  return state->diffuse;
}
*/

const double * nmg_Graphics_Implementation::getSurfaceColor (void) const {
//fprintf(stderr, "nmg_Graphics_Implementation::getSurfaceColor().\n");
  return state->surfaceColor;
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
      exit(-1);
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
  *minX = state->minChangedX;
  *maxX = state->maxChangedX;
  *minY = state->minChangedY;
  *maxY = state->maxChangedY;

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
int nmg_Graphics_Implementation::handle_loadVizImage
                                 (void * userdata, vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;

  it->loadVizImage(p.buffer);
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
  int region;

  CHECKF(it->decode_enableFilledPolygons(p.buffer, &value, &region), "handle_enableFilledPolygons");
  it->enableFilledPolygons(value, region);
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

/*
// static
int nmg_Graphics_Implementation::handle_enableUber
                                 (void * userdata, vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  int value;

  CHECKF(it->decode_enableUber(p.buffer, &value), "handle_enableUber");
  it->enableUber(value);

  return 0;
}
*/

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

int nmg_Graphics_Implementation::handle_setMaskPlaneName (void * userdata,
							     vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  it->setMaskPlaneName(p.buffer);
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_setHeightPlaneName (void * userdata,
                                           vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;

  it->setHeightPlaneName(p.buffer);
  return 0;
}

int nmg_Graphics_Implementation::handle_setRegionControlPlaneName (void * userdata,
                                                    vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  char *name;
  int region;

  CHECKF(it->decode_setRegionControlPlaneName(p.buffer, &name, &region), 
      "handle_setRegionControlPlaneName");
  it->setRegionControlPlaneName(name, region);
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_setSurfaceColor
                                 (void * userdata, vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  double c [3];

  CHECKF(it->decode_setSurfaceColor(p.buffer, c), "handle_setSurfaceColor");
  it->setSurfaceColor(c);
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
   int region;

   CHECKF(it->decode_setSurfaceAlpha(p.buffer, &surface_alpha, &region), "handle_setSurfaceAlpha");
   it->setSurfaceAlpha(surface_alpha, region);
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
  int stride, region;

  CHECKF(it->decode_setTesselationStride(p.buffer, &stride, &region), "handle_setTesselationStride");
  it->setTesselationStride(stride, region);
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_setTextureMode
                                 (void * userdata, vrpn_HANDLERPARAM p) {
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  TextureMode m;
  TextureTransformMode xm;
  int region;

  CHECKF(it->decode_setTextureMode(p.buffer, &m, &xm, &region), "handle_setTextureMode");
  it->setTextureMode(m, xm, region);
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
  int oldMode, oldStyle, newMode, style, tool;

  CHECK(it->decode_setUserMode(p.buffer, &oldMode, &oldStyle, &newMode, &style, &tool));
  it->setUserMode(oldMode, oldStyle, newMode, style, tool);
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
  it->positionRubberCorner(x0, y0, x1, y1, 0);
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


//
// Colormap Texture Handlers
//

// static
int nmg_Graphics_Implementation::handle_createColormapTexture (void *userdata,
						       vrpn_HANDLERPARAM p) {

  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation * )userdata;
  it->createColormapTexture( p.buffer );
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_setTextureColormapSliderRange
( void *userdata, vrpn_HANDLERPARAM p) {
  int which;
  float data_min,data_max,color_min, color_max;
  
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation * )userdata;
  CHECKF(it->decode_setTextureColormapSliderRange(p.buffer, &which, &data_min,&data_max,&color_min, &color_max), 
	  "handle_setTextureColormapSliderRange");
  it->setTextureColormapSliderRange( which, data_min,data_max,color_min, color_max );
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_setTextureAlpha
( void *userdata, vrpn_HANDLERPARAM p) {
  int which;
  float alpha;
  
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation * )userdata;
  CHECKF(it->decode_setTextureAlpha(p.buffer, &which, &alpha), 
	  "handle_setTextureAlpha");
  it->setTextureAlpha( which, alpha );
  return 0;
}

// static
int nmg_Graphics_Implementation::handle_setTextureColormapConversionMap
( void *userdata, vrpn_HANDLERPARAM p) {
  
  int which;
  char *map = new char[100], *mapdir = new char[100];
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation * )userdata;
  CHECKF(it->decode_setTextureColormapConversionMap(p.buffer, &which, &map, &mapdir), "handle_setTextureConversionMap");
  it->setTextureColormapConversionMap( which, map, mapdir );

  if (map) {
    delete [] map;
  }
  if (mapdir) {
    delete [] mapdir;
  }
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
int nmg_Graphics_Implementation::
handle_setTextureTransform (void *userdata, vrpn_HANDLERPARAM p) {
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
int nmg_Graphics_Implementation::
handle_createScreenImage( void *userdata, vrpn_HANDLERPARAM  p )
{
   char *filename = new char[512];
   char* type = new char[512];

   nmg_Graphics_Implementation *it = (nmg_Graphics_Implementation *)userdata;
   it->decode_createScreenImage(p.buffer, &filename, &type);
   it->createScreenImage(filename, type);

   if (filename) {
     delete [] filename;
   }

   return 0;
}

// static
int nmg_Graphics_Implementation::
handle_setViztexScale( void * userdata, vrpn_HANDLERPARAM p ) 
{
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  float scale;

  CHECKF(it->decode_setViztexScale(p.buffer, &scale), "handle_setViztexScale");
  it->setViztexScale(scale);
  return 0;
}


int nmg_Graphics_Implementation::
handle_setRegionMaskHeight( void * userdata, vrpn_HANDLERPARAM p ) 
{
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  float min_height, max_height;
  int region;

  CHECKF(it->decode_setRegionMaskHeight(p.buffer, &min_height, &max_height, &region), 
      "handle_setRegionMaskHeight");
  it->setRegionMaskHeight(min_height, max_height, region);
  return 0;
}

int nmg_Graphics_Implementation::
handle_createRegion( void * userdata, vrpn_HANDLERPARAM /*p*/ ) 
{
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;

  it->createRegion();
  return 0;
}

int nmg_Graphics_Implementation::
handle_destroyRegion( void * userdata, vrpn_HANDLERPARAM p ) 
{
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  int region;

  CHECKF(it->decode_destroyRegion(p.buffer, &region), 
      "handle_destroyRegion");
  it->destroyRegion(region);
  return 0;
}

int nmg_Graphics_Implementation::
handle_associateAlpha(void *userdata, vrpn_HANDLERPARAM p)
{
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  vrpn_bool associate;
  int region;

  CHECKF(it->decode_associate(p.buffer, &associate, &region), 
      "handle_associateAlpha");
  it->associateAlpha(associate, region);
  return 0;
}

int nmg_Graphics_Implementation::
handle_associateFilledPolygons(void *userdata, vrpn_HANDLERPARAM p)
{
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  vrpn_bool associate;
  int region;

  CHECKF(it->decode_associate(p.buffer, &associate, &region), 
      "handle_associateFilledPolygons");
  it->associateFilledPolygons(associate, region);
  return 0;
}

int nmg_Graphics_Implementation::
handle_associateStride(void *userdata, vrpn_HANDLERPARAM p)
{
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  vrpn_bool associate;
  int region;

  CHECKF(it->decode_associate(p.buffer, &associate, &region), 
      "handle_associateStride");
  it->associateStride(associate, region);
  return 0;
}

int nmg_Graphics_Implementation::
handle_associateTextureDisplayed(void *userdata, vrpn_HANDLERPARAM p)
{
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  vrpn_bool associate;
  int region;

  CHECKF(it->decode_associate(p.buffer, &associate, &region), 
      "handle_associateTextureDisplayed");
  it->associateTextureDisplayed(associate, region);
  return 0;
}

int nmg_Graphics_Implementation::
handle_associateTextureMode(void *userdata, vrpn_HANDLERPARAM p)
{
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  vrpn_bool associate;
  int region;

  CHECKF(it->decode_associate(p.buffer, &associate, &region), 
      "handle_associateTextureMode");
  it->associateTextureMode(associate, region);
  return 0;
}

int nmg_Graphics_Implementation::
handle_associateTextureTransformMode(void *userdata, vrpn_HANDLERPARAM p)
{
  nmg_Graphics_Implementation * it = (nmg_Graphics_Implementation *) userdata;
  vrpn_bool associate;
  int region;

  CHECKF(it->decode_associate(p.buffer, &associate, &region), 
      "handle_associateTextureTransformMode");
  it->associateTextureTransformMode(associate, region);
  return 0;
}
