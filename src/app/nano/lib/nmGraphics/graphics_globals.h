/*===3rdtech===
  Copyright (c) 2000 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#ifndef GRAPHICS_GLOBALS_H
#define GRAPHICS_GLOBALS_H

// graphics_globals.h

// File for anything that needs to be visible to every file in this module
// but not outside.

// DO NOT put variables that need to be shared outside this module here!
// That means NO Tclvars!

#ifndef INCLUDED_V_H
#include <v.h>  // for GLfloat, GLenum, v_index
#define INCLUDED_V_H
#endif

#include "vrpn_Types.h"

#include <colormap.h>
#include "nmg_Funclist.h"

#if defined(_WIN32) && !defined(__CYGWIN__)
// bogus double to float conversion warning.
#pragma warning(disable:4244)
#pragma warning(disable:4305)
#endif

class BCGrid;  // from BCGrid.h
class nmb_Subgrid;  // from nmb_Subgrid.h

extern  int     spm_graphics_verbosity;
#define VERBOSE(level,msg) if (spm_graphics_verbosity >= level) fprintf(stderr,"%s\n",msg);

class PPM;  // from PPM.h
class Position_list;  // from Position.h

// HACK
#define TIP_HEIGHT (1.0)
#define NUM_USERS (1)
#define NUM_OBJECTS (100)
#define NUM_ELEMENTS (20)

#define PUSH_ATTRIB(x) glPushAttrib(x)
#define POP_ATTRIB() glPopAttrib()

extern float g_adhesion_slider_min;
extern float g_adhesion_slider_max;

extern float g_alpha_r;
extern float g_alpha_g;
extern float g_alpha_b;
extern float g_alpha_slider_min;
extern float g_alpha_slider_max;

extern ColorMap g_colorMap;
extern char * g_colorMapDir;
extern ColorMap * g_curColorMap;

extern int g_just_color;
extern float g_color_min; // ColorMap lower and upper bounds for color
extern float g_color_max;

extern float g_data_min_norm; // ColorMap lower and upper bounds for data
extern float g_data_max_norm; // normalized zero to one, set by user

extern float g_opacity_slider_min;
extern float g_opacity_slider_max;

extern float g_data_min; // lower and upper bounds for data
extern float g_data_max; // real data units (nm, for example), measured from data. 

extern char * g_textureDir;

extern float g_compliance_slider_min;
extern float g_compliance_slider_max;

extern int g_config_chartjunk;
extern int g_config_measurelines;
extern int g_config_planeonly;
extern int g_config_filled_polygons;
extern int g_config_smooth_shading;
extern int g_config_trueTip;
extern int g_config_enableUber;

extern int g_contour_r;
extern int g_contour_g;
extern int g_contour_b;
extern float g_contour_width;

extern v_index * g_displayIndexList;

extern float g_friction_slider_min;
extern float g_friction_slider_max;

extern float g_bump_slider_min;
extern float g_bump_slider_max;

extern float g_buzz_slider_min;
extern float g_buzz_slider_max;

extern int g_hand_color;

extern float g_icon_scale;


extern double g_minColor [4];
extern double g_maxColor [4];

extern int g_minChangedX;
extern int g_maxChangedX;
extern int g_minChangedY;
extern int g_maxChangedY;

//to keep track of position and orientation of collaborator's hand
extern double g_collabHandPos [3];
extern double g_collabHandQuat [4];
extern int g_position_collab_hand;
extern int g_draw_collab_hand;
extern int make_collab_hand_icon(double pos[], double quat[], vrpn_int32 mode);
extern vrpn_int32 g_collabMode;

// Realigning Textures:
extern ColorMap *g_realign_textures_curColorMap;
extern float g_realign_textures_slider_min;
extern float g_realign_textures_slider_max;
extern char g_realign_texture_name[128];

extern int g_translate_textures;
extern int g_scale_textures;
extern int g_shear_textures;
extern int g_rotate_textures;

extern float g_tex_coord_center_x;	// position in texture [0..1]
extern float g_tex_coord_center_y;
extern float g_tex_range_x;
extern float g_tex_range_y;
extern float g_tex_theta_cumulative;
extern float g_translate_tex_x;
extern float g_translate_tex_y;
extern float g_scale_tex_x;
extern float g_scale_tex_y;
extern float g_shear_tex_x;
extern float g_shear_tex_y;

extern float g_translate_tex_dx;
extern float g_translate_tex_dy;
extern float g_scale_tex_dx;
extern float g_scale_tex_dy;
extern float g_shear_tex_dx;
extern float g_shear_tex_dy;
extern float g_rotate_tex_theta;

// Registration:
extern double g_texture_transform[16];

extern float g_rubberPt [4];
extern float g_slowLine3dPt [6];
extern float g_rubberSweepPts[2][4];
extern float g_rubberSweepPtsSave[2][4];
extern float g_scanlinePt[6];
extern int g_scanline_display_enabled;

extern float g_rulergrid_xoffset;
extern float g_rulergrid_yoffset;
extern float g_rulergrid_scale;
extern float g_rulergrid_sin;
extern float g_rulergrid_cos;
extern int g_ruler_r;
extern int g_ruler_g;
extern int g_ruler_b;
extern int g_null_data_alpha_toggle;
extern float g_ruler_opacity;
extern float g_ruler_width_x;
extern float g_ruler_width_y;
extern PPM * g_rulerPPM;

//extern Tclvar_int g_shiny;
//extern Tclvar_float g_specular_color;
extern int g_shiny;
extern float g_specular_color;
extern float g_surface_alpha;
extern float g_diffuse;

extern BCGrid * g_prerendered_grid;
extern nmb_Subgrid * g_prerenderedChange;

extern float g_sphere_scale;
extern int g_stride;

extern int g_texture_displayed;
extern int g_texture_transform_mode;

extern GLenum g_texture_mode;
extern float g_texture_scale;
extern float g_trueTipLocation [3];
extern float g_trueTipScale;

extern int g_user_mode;

extern int g_PRERENDERED_COLORS;
extern int g_PRERENDERED_TEXTURE;
extern int g_PRERENDERED_DEPTH;

extern Position_list * g_positionList;
extern Position_list * g_positionListL;  // for the left side of the sweep marker
extern Position_list * g_positionListR;  // for the right side of the sweep marker

#define N_TEX 8
#define CONTOUR_1D_TEX_ID       0
#define ALPHA_3D_TEX_ID         1
#define RULERGRID_TEX_ID        2
#define GENETIC_TEX_ID          3
#define COLORMAP_TEX_ID         4  // used for realign
#define SEM_DATA_TEX_ID         5
#define REMOTE_DATA_TEX_ID      6
#define VISUALIZATION_TEX_ID	7

#define NMG_DEFAULT_IMAGE_WIDTH (512)
#define NMG_DEFAULT_IMAGE_HEIGHT (512)

extern GLuint tex_ids [N_TEX];
//extern GLubyte * sem_data;
extern GLubyte * remote_data;
//extern float * realign_data;

// These values are only used for
// the 2D textures but we use the ID values above to index them
// (RULERGRID_TEX_ID, GENETIC_TEX_ID, COLORMAP_TEX_ID, SEM_DATA_TEX_ID, 
//  REMOTE_DATA_TEX_ID)
extern int g_tex_image_width[N_TEX];
extern int g_tex_image_height[N_TEX];
extern int g_tex_installed_width[N_TEX];
extern int g_tex_installed_height[N_TEX];
extern int g_tex_image_offsetx[N_TEX];
extern int g_tex_image_offsety[N_TEX];

// e.g. GL_MODULATE, GL_DECAL...
extern int g_tex_blend_func[N_TEX];

extern float g_tex_env_color[N_TEX][4];

// note: shader textures managed separately below

extern char g_alphaPlaneName [128];
extern char g_colorPlaneName [128];
extern char g_contourPlaneName [128];
extern char g_heightPlaneName [128];
extern char g_opacityPlaneName [128];
extern char g_maskPlaneName [128];
extern char g_transparentPlaneName [128];
extern char g_vizPlaneName[128];

extern BCGrid * g_inputGrid;
//struct Vertex_Struct;
//extern Vertex_Struct **vertexptr;

//////////////////////////////////////////////////////////////////
// Visualization Section
//////////////////////////////////////////////////////////////////

#define DISABLE_MASK 0
#define ENABLE_MASK 1
#define INVERT_MASK -1

extern int g_mask;
extern int g_transparent;

class nmg_Visualization;
extern nmg_Visualization * visualization;

extern PPM * g_vizPPM;

//////////////////////////////////////////////////////////////////
// End Visualization Section
//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
// Variables that define what GL can and can't do on the machine
// we are running on.
//////////////////////////////////////////////////////////////////

extern int g_VERTEX_ARRAY;

//////////////////////////////////////////////////////////////////
// End GL capabilities
//////////////////////////////////////////////////////////////////

#endif  // GRAPHICS_GLOBALS_H











