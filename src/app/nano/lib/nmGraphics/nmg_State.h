/*===3rdtech===
  Copyright (c) 2000 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#ifndef NMG_STATE_H
#define NMG_STATE_H

// nmg_State.h

#ifndef INCLUDED_V_H
#include <v.h>  // for GLfloat, GLenum, v_index
#define INCLUDED_V_H
#endif

#include <vrpn_Types.h>

#include <nmb_ColorMap.h>
#include "nmg_Funclist.h"
#include "URProjectiveTexture.h"

#if defined(_WIN32) && !defined(__CYGWIN__)
// bogus double to float conversion warning.
#pragma warning(disable:4244)
#pragma warning(disable:4305)
#endif

class BCGrid;  // from BCGrid.h
class nmb_Subgrid;  // from nmb_Subgrid.h
class PPM;  // from PPM.h
class Position_list;  // from Position.h
class nmg_Surface;

///Debugging printout flag
extern  int     spm_graphics_verbosity;
#define VERBOSE(level,msg) if (spm_graphics_verbosity >= level) fprintf(stderr,"%s\n",msg);

#define TIP_HEIGHT (1.0)
#define NUM_USERS (1)
#define NUM_OBJECTS (100)
#define NUM_ELEMENTS (20)

/// A define allows you to easily disable, was used on PXFL
#define PUSH_ATTRIB(x) glPushAttrib(x)
#define POP_ATTRIB() glPopAttrib()

// Visualization defines
#define DISABLE_MASK 0
#define ENABLE_MASK 1
#define INVERT_MASK -1

typedef struct xs_state {
    int enabled;
    float center_x, center_y;
    float widthL;
    float widthR;
    float angle;
    int highlight_mask;
} crossSectionState;

/**
 Class contains anything that needs to be visible to every file in this module
 but not outside.
 DO NOT put variables that need to be shared outside this module here!
 That means NO Tclvars! Any values should be communicated through 
 nmg_GraphicsImpl, then stored in a variable here. 
*/
class nmg_State {
public:
    nmg_State();
    ~nmg_State() {};
	GLuint alphaTextureID;
	int alphaTextureBlendFunc;
    float alpha_r;  ///< texture blend color
    float alpha_g;  ///< texture blend color
    float alpha_b;  ///< texture blend color
    float alpha_slider_min; ///< texture blend scale
    float alpha_slider_max; ///< texture blend scale

    nmb_ColorMap colorMap;  ///< colormap applied to surface
    char * colorMapDir;
    nmb_ColorMap * curColorMap;

    int just_color;
    float color_min; ///< nmb_ColorMap lower and upper bounds for color
    float color_max;

    float data_min_norm; ///< nmb_ColorMap lower and upper bounds for data
    float data_max_norm; ///< normalized zero to one, set by user

    float data_min; ///< lower and upper bounds for (color map only?) data
    float data_max; ///< real data units (nm, for example), measured from data. 

    float opacity_slider_min;
    float opacity_slider_max;

    char * textureDir;

    int config_chartjunk;
    int config_measurelines;
    int config_planeonly;
    int config_filled_polygons;
    int config_smooth_shading;
    int config_trueTip;
    int config_enableUber;

	GLuint contourTextureID;
	int contourTextureBlendFunc;
    int contour_r;
    int contour_g;
    int contour_b;
    float contour_width;  ///< spacing between contour lines, in real units

    v_index * displayIndexList;

/// color of hand icon in measure mode. red=0, yellow=1, or blue=2. 
    int hand_color;

    float icon_scale;  ///< scale all hand icons. 


    double surfaceColor [4]; ///< color for surface, if no color map. 

    int minChangedX; ///< Grid index bounds of newest data from the microscope.
    int maxChangedX; ///< Grid index bounds of newest data from the microscope.
    int minChangedY; ///< Grid index bounds of newest data from the microscope.
    int maxChangedY; ///< Grid index bounds of newest data from the microscope.

///to keep track of position of collaborator's hand
    double collabHandPos [3];
///to keep track of collaborator's hand orientation
    double collabHandQuat [4];
/// position:  has it moved?  do we need to generate a new display list
/// for it?
    int position_collab_hand;
/// draw:  should we draw the display list at all?
    int draw_collab_hand;
    vrpn_int32 collabMode;

//  Textures:
/*
This is redundant - information is now stored in URProjectiveTexture objects
    nmb_ColorMap * colormap_texture_curColorMap;        // also using for video texture, might want to add a separate one
    float colormap_texture_data_min;
    float colormap_texture_data_max;
    float colormap_texture_color_min;
    float colormap_texture_color_max;
    char colormap_texture_name[128];
	*/

	URProjectiveTexture *currentProjectiveTexture;
	URProjectiveTexture colormapTexture;
	URProjectiveTexture rulergridTexture;
	URProjectiveTexture videoTexture;
	URProjectiveTexture remoteDataTexture;	
	URProjectiveTexture visualizationTexture;
/* replaced:
	RULERGRID_TEX_ID
	SEM_DATA_TEX_ID
	REMOTE_DATA_TEX_ID
	VISUALIZATION_TEX_ID
	*/

    int translate_textures;
    int scale_textures;
    int shear_textures;
    int rotate_textures;

    float tex_coord_center_x;	///< position in texture [0..1]
    float tex_coord_center_y;
    float tex_range_x;
    float tex_range_y;
    float tex_theta_cumulative;
    float translate_tex_x;
    float translate_tex_y;
    float scale_tex_x;
    float scale_tex_y;
    float shear_tex_x;
    float shear_tex_y;

    float translate_tex_dx;
    float translate_tex_dy;
    float scale_tex_dx;
    float scale_tex_dy;
    float shear_tex_dx;
    float shear_tex_dy;
    float rotate_tex_theta;

// Registration:
    double surfaceModeTextureTransform[16];
	double modelModeTextureTransform[16];
  ///< object space along x or y from 0 to 1000 (units are nM) 
  ///< goes from 0 to 1 in u or v texture coordinate

    float rubberPt [6];  ///< endpoints of stretchable "rubber-band" line
    float slowLine3dPt [6];
    float rubberSweepPts[2][6];  ///< begin and end, Left and right points, X,Y,Z
    float rubberSweepPtsSave[2][6];  ///< begin and end, Left and right points, X,Y,Z

    float scanlinePt[6];
    int scanline_display_enabled;

    float rulergrid_xoffset;
    float rulergrid_yoffset;
    float rulergrid_scale;
    float rulergrid_sin;
    float rulergrid_cos;
    int ruler_r;
    int ruler_g;
    int ruler_b;
    int null_data_alpha_toggle;
    float ruler_opacity;
    float ruler_width_x;
    float ruler_width_y;
    PPM * rulerPPM;

    int shiny;
     /// specular shininess exponent 1-128
    vrpn_bool local_viewer;
    float specular_color;
     ///< specular color - always white/gray
    float surface_alpha;
    float diffuse;

    crossSectionState xs_state[2];

    BCGrid * prerendered_grid;
    nmb_Subgrid * prerenderedChange;

    float sphere_scale;
    int stride;

    int texture_displayed;
    int texture_transform_mode;

    GLenum texture_mode;
    float texture_scale;
    float trueTipLocation [3];
    float trueTipScale;

    int user_mode;

    int PRERENDERED_COLORS;
    int PRERENDERED_TEXTURE;
    int PRERENDERED_DEPTH;

    Position_list * positionList;
    Position_list * positionListL;  ///< for the left side of the sweep marker
    Position_list * positionListR;  ///< for the right side of the sweep marker

    char alphaPlaneName [128];
    char colorPlaneName [128];
    char contourPlaneName [128];
    char heightPlaneName [128];
    char opacityPlaneName [128];
    char maskPlaneName [128];
    char transparentPlaneName [128];
    char vizPlaneName[128];
    float viztex_scale;

    BCGrid * inputGrid;
//struct Vertex_Struct;
//    Vertex_Struct **vertexptr;

//////////////////////////////////////////////////////////////////
// Visualization Section
//////////////////////////////////////////////////////////////////

    int mask;
    int transparent;

    nmg_Surface * surface;

    PPM * vizPPM;

//////////////////////////////////////////////////////////////////
// End Visualization Section
//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
// Variables that define what GL can and can't do on the machine
// we are running on.
//////////////////////////////////////////////////////////////////

    int VERTEX_ARRAY;

//////////////////////////////////////////////////////////////////
// End GL capabilities
//////////////////////////////////////////////////////////////////
};
#endif  // GRAPHICS_GLOBALS_H











