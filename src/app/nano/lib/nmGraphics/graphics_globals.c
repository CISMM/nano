#include "graphics_globals.h"

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
float g_color_slider_min = 0.0f;
float g_color_slider_max = 1.0f;

char * g_textureDir = NULL;

int g_config_chartjunk = 1;
int g_config_filled_polygons = 1;
int g_config_smooth_shading = 1;
int g_config_trueTip = 0;

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

float g_icon_scale = 0.25f;

//to keep track of collaborator's hand position/orientation
int g_draw_collab_hand = 0;
double g_collabHandPos[3];  //no initial value
double g_collabHandQuat[4];  //no initial value
vrpn_int32 g_collabMode = 0;

//initially set alpha value to 0.0
double g_minColor [4] = { 0.0f, 0.0f, 0.0f, 0.0f };
//initially set alpha value to 1.0
double g_maxColor [4] = { 1.0f, 0.88f, 0.04f, 1.0f };

// Genetic Textures
int g_genetic_textures_enabled = 0;

// Realigning Textures:
ColorMap *g_realign_textures_curColorMap = NULL;
float g_realign_textures_slider_min = 0;
float g_realign_textures_slider_max = 1.0;

int g_realign_textures_enabled = 0;
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

int g_registration_enabled = 0;
double g_texture_transform[16] = {0.001,0,0,0,0,0.001,0,0,0,0,1,0,0,0,0,1};
int g_tex_image_width = 512;
int g_tex_image_height = 512;
int g_tex_installed_width = 512;
int g_tex_installed_height = 512;

float g_rubberPt [4];
float g_scanlinePt [6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
int g_scanline_display_enabled = 0;
int g_rulergrid_enabled = 0;
float g_rulergrid_xoffset = 0.0f;
float g_rulergrid_yoffset = 0.0f;
float g_rulergrid_scale = 500.0f;
int g_ruler_r = 255;
int g_ruler_g = 255;
int g_ruler_b = 55;

//set default rulergrid opacity to 128 (50%) instead of 255 (100%)
float g_ruler_opacity = 128.0f;
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
GLenum g_texture_mode = GL_TEXTURE_2D;
float g_texture_scale = 10.0f;
float g_trueTipLocation [3];
float g_trueTipScale = 1.0f;

int g_user_mode [NUM_USERS];
int g_VERTEX_ARRAY = 0;

Position_list * g_positionList;

#ifdef FLOW

int g_data_tex_size;

  // shaders
  GLuint nM_shader;
  GLuint nM_diffuse;     // just shows the diffuse color shader

  // texture id for shaders
  GLuint tex_ids[NTEX];

  // data set values
  GLubyte *pattern_data;  // g_data_tex_size * g_data_tex_size * 3
  GLubyte *hatch_data;    // g_data_tex_size * g_data_tex_size * 3
  GLubyte *bump_data;     // g_data_tex_size * g_data_tex_size * 3

  // which shader to turn on
  unsigned char shader_mask = NO_SHADER;

  // for time-varying phenomena
  float px_framenum = 1.0;

  //=====Bump Shader=====
  float bumpiness = 1.0;          // max amount of bumpiness
  float bump_pulserate = 0.0;   // bump pulse rate
  float bump_scale = 1.0;       // scale of bump texture

  //=====Pattern Blend Shader=====
  float pattern_blend = 1.0;     // max amount of blending
  float pattern_color[3] = {0.995, 0.0, 0.0};  // color of the pattern to blend
  float pattern_scale = 1.0;     // scale of the pattern texture
  float pattern_pulserate = 0.0; // pattern pulse rate

  //=====Spot Noise Shader=====
  float spotnoise_tex_size = 256.0;  // spot noise texture size
  float spot_pulserate = 0.0;        // spot noise pulse rate
  float spot_blend = 1.0;            // max amont of spot noise to blend in
  unsigned char whichspot = PLUS_X;  // which spot noise to use
  float contrast = 2.5;              // to adjust contrast on noise functions
  float rotate_amount = 75.0;        // amount to rotate the kernel
  unsigned char swap_kernel = 0;   // switch kernels used for high and low data
  unsigned char kernel_size = 9;   // size of the gaussian shaders
  unsigned char dense = 1;         //sparse or dense noise
  float wobblerate = 4.0;          // rate of wobbling for wobble_plus

#endif


