/*===3rdtech===
  Copyright (c) 2000 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#include "graphics_globals.h"
#include "nmg_Graphics.h" // for enums
#include "nmg_Surface.h"

class BCGrid;  // from BCGrid.h
class nmb_Subgrid;  // from nmb_Subgrid.h

float g_adhesion_slider_min = 0.0f;
float g_adhesion_slider_max = 1.0f;

float g_alpha_r = 0.0f;
float g_alpha_g = 1.0f;
float g_alpha_b = 0.0f;
float g_alpha_slider_min = 0.0f;
float g_alpha_slider_max = 1.0f;

ColorMap g_colorMap;
char * g_colorMapDir = NULL;
ColorMap * g_curColorMap = NULL;

int g_just_color = 0;

float g_color_min = 0; // ColorMap lower and upper bounds for color
float g_color_max = 1.0;

v_index * g_displayIndexList = NULL;

float g_data_min_norm = 0; // ColorMap lower and upper bounds for data, normalized
float g_data_max_norm = 1.0;
float g_data_min = 0; // data min/max, real units.
float g_data_max = 1.0;

float g_opacity_slider_min = 0.0f;
float g_opacity_slider_max = 1.0f;

char * g_textureDir = NULL;

int g_config_chartjunk = 1;
int g_config_measurelines = 1;
int g_config_planeonly = 0;
int g_config_filled_polygons = 1;
int g_config_smooth_shading = 1;
int g_config_trueTip = 0;
int g_config_enableUber = 1;

float g_compliance_slider_min = 0.0f;
float g_compliance_slider_max = 1.0f;

int g_contour_r = 255;
int g_contour_g = 55;
int g_contour_b = 55;
float g_contour_width = 10.0f;

float g_friction_slider_min = 0.0f;
float g_friction_slider_max = 1.0f;

float g_bump_slider_min = 0.0f;
float g_bump_slider_max = 1.0f;

float g_buzz_slider_min = 0.0f;
float g_buzz_slider_max = 1.0f;

int g_hand_color;  // XXX no initial value
int g_null_data_alpha_toggle = 0;
float g_icon_scale = 0.25f;

// position:  has it moved?  do we need to generate a new display list
// for it?
// draw:  should we draw the display list at all?
//to keep track of collaborator's hand position/orientation
int g_position_collab_hand = 0;
int g_draw_collab_hand = 0;
double g_collabHandPos[3];  //no initial value
double g_collabHandQuat[4];  //no initial value
vrpn_int32 g_collabMode = 0;

//initially set alpha value to 0.0
double g_minColor [4] = { 0.0f, 0.0f, 0.0f, 0.0f };
//initially set alpha value to 1.0
double g_maxColor [4] = { 1.0f, 0.88f, 0.04f, 1.0f };

int g_minChangedX;
int g_maxChangedX;
int g_minChangedY;
int g_maxChangedY;


// Realigning Textures:
ColorMap *g_realign_textures_curColorMap = NULL;
float g_realign_textures_slider_min = 0;
float g_realign_textures_slider_max = 1.0;
char g_realign_texture_name[128] = {0};

BCGrid * g_prerendered_grid = NULL;
nmb_Subgrid * g_prerenderedChange = NULL;

int g_translate_textures = 0;
int g_scale_textures = 0;
int g_shear_textures = 0;
int g_rotate_textures = 0;

float g_tex_coord_center_x = 0.5;
float g_tex_coord_center_y = 0.5;
float g_tex_range_x = 1.0;
float g_tex_range_y = 1.0;
float g_tex_theta_cumulative = 0.0;
float g_translate_tex_x = 0.0;
float g_translate_tex_y = 0.0;
float g_scale_tex_x = 1000.0;
float g_scale_tex_y = 1000.0;
float g_shear_tex_x = 0.0;
float g_shear_tex_y = 0.0;


float g_translate_tex_dx = 0;
float g_translate_tex_dy = 0;
float g_scale_tex_dx = 0;
float g_scale_tex_dy = 0;
float g_shear_tex_dx = 0;
float g_shear_tex_dy = 0;
float g_rotate_tex_theta = 0;

// Registration
double g_texture_transform[16] = {0.001,0,0,0,0,0.001,0,0,0,0,1,0,0,0,0,1};
  // object space along x or y from 0 to 1000 (units are nM) 
  // goes from 0 to 1 in u or v texture coordinate

float g_rubberPt [4];
float g_slowLine3dPt [6];
float g_rubberSweepPts[2][4];
float g_rubberSweepPtsSave[2][4];
float g_scanlinePt [6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
int g_scanline_display_enabled = 0;

float g_rulergrid_xoffset = 0.0f;
float g_rulergrid_yoffset = 0.0f;
float g_rulergrid_scale = 500.0f;
int g_ruler_r = 255;
int g_ruler_g = 255;
int g_ruler_b = 55;

//set default rulergrid opacity to 178 (70%) instead of 255 (100%)
float g_ruler_opacity = 178.0f;
//float g_ruler_opacity = 255.0f;
float g_ruler_width_x = 1.0f;
float g_ruler_width_y = 1.0f;
float g_rulergrid_sin = 0.0f;
float g_rulergrid_cos = 1.0f;
PPM * g_rulerPPM = NULL;

// Material properties of the surface
// NO Tclvars in this file!  Nothing that needs to be shared outside!
//Tclvar_int g_shiny("shiny", 55);  
int g_shiny = 55;
     // specular shininess
float g_diffuse = .50;
//Tclvar_float g_specular_color("specular_color", 1.0);    
float g_specular_color = 1.0;
     // specular color - always white/gray

float g_surface_alpha = 1.0;

float g_sphere_scale = 12.5f;
int g_stride = 1;

int g_texture_displayed = nmg_Graphics::NO_TEXTURES;
int g_texture_transform_mode = nmg_Graphics::RULERGRID_COORD;

//GLenum g_texture_mode = GL_TEXTURE_2D;
GLenum g_texture_mode = GL_FALSE;
GLuint tex_ids [N_TEX];
//GLubyte * sem_data = NULL;
//float *realign_data = NULL;
GLubyte * remote_data = NULL;

// image size refers to a sub region of the texture passed to openGL that
// actually contains image data
int g_tex_image_width[N_TEX];
int g_tex_image_height[N_TEX];

// installed size refers to size of texture passed to openGL
int g_tex_installed_width[N_TEX];
int g_tex_installed_height[N_TEX];

// position in installed texture array of first pixel from image array
int g_tex_image_offsetx[N_TEX];
int g_tex_image_offsety[N_TEX];

// e.g. GL_MODULATE, GL_DECAL...
int g_tex_blend_func[N_TEX];

float g_tex_env_color[N_TEX][4];

float g_texture_scale = 10.0f;
float g_trueTipLocation [3];
float g_trueTipScale = 1.0f;

int g_user_mode;

int g_PRERENDERED_COLORS = 0;  // only used by remote rendering clients
int g_PRERENDERED_TEXTURE = 0;  // only used by remote rendering clients
int g_PRERENDERED_DEPTH = 0;  // only used by remote rendering clients

Position_list * g_positionList;
Position_list * g_positionListL;  // for the left side of the sweep marker
Position_list * g_positionListR;  // for the right side of the sweep marker

BCGrid * g_inputGrid = NULL;

char g_alphaPlaneName [128];
char g_colorPlaneName [128];
char g_contourPlaneName [128];
char g_heightPlaneName [128];
char g_opacityPlaneName [128];
char g_maskPlaneName [128];
char g_transparentPlaneName [128];
char g_vizPlaneName [128];

int g_mask = ENABLE_MASK;
int g_transparent = 0;
float g_viztex_scale = 500.0f;

nmg_Surface *g_surface;

PPM * g_vizPPM = NULL;

//////////////////////////////////////////////////////////////////
// Variables that define what GL can and can't do on the machine
// we are running on.
//////////////////////////////////////////////////////////////////

int g_VERTEX_ARRAY = 0;

//////////////////////////////////////////////////////////////////
// End GL capabilities
//////////////////////////////////////////////////////////////////

// Local Variables:
// mode:c++
// End:
