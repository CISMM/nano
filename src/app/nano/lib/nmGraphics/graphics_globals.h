#ifndef GRAPHICS_GLOBALS_H
#define GRAPHICS_GLOBALS_H

// graphics_globals.h

// File for anything that needs to be visible to every file in this module
// but not outside.

// DO NOT put variables that need to be shared outside this module here!
// That means NO Tclvars!

#ifndef INCLUDED_V_H
#include <v.h>  // for GLfloat, GLenum
#define INCLUDED_V_H
#endif

#include "vrpn_Types.h"

#include <colormap.h>
#include "nmg_Funclist.h"

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

#ifdef FLOW
#define PUSH_ATTRIB(x)
#define POP_ATTRIB()
#else
#define PUSH_ATTRIB(x) glPushAttrib(x)
#define POP_ATTRIB() glPopAttrib()
#endif  // FLOW

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
extern float g_color_slider_min;
extern float g_color_slider_max;

extern char * g_textureDir;

extern float g_compliance_slider_min;
extern float g_compliance_slider_max;

extern int g_config_chartjunk;
extern int g_config_measurelines;
extern int g_config_planeonly;
extern int g_config_filled_polygons;
extern int g_config_smooth_shading;
extern int g_config_trueTip;

extern int g_contour_r;
extern int g_contour_g;
extern int g_contour_b;
extern float g_contour_width;

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
extern double g_collabHandPos[3];
extern double g_collabHandQuat[4];
extern int g_draw_collab_hand;
extern int make_collab_hand_icon(double pos[], double quat[], vrpn_int32 mode);
extern vrpn_int32 g_collabMode;

// Realigning Textures:
extern ColorMap *g_realign_textures_curColorMap;
extern float g_realign_textures_slider_min;
extern float g_realign_textures_slider_max;

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
extern int g_tex_image_width;
extern int g_tex_image_height;
extern int g_tex_installed_width;
extern int g_tex_installed_height;
extern int g_tex_sem_installed_width;
extern int g_tex_sem_installed_height;

extern float g_rubberPt [4];
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

extern int g_user_mode [NUM_USERS];
extern int g_VERTEX_ARRAY;

extern int g_PRERENDERED_COLORS;
extern int g_PRERENDERED_TEXTURE;
extern int g_PRERENDERED_DEPTH;

extern Position_list * g_positionList;

#define N_TEX 7
#define CONTOUR_1D_TEX_ID       0
#define ALPHA_3D_TEX_ID         1
#define RULERGRID_TEX_ID        2
#define GENETIC_TEX_ID          3
#define COLORMAP_TEX_ID         4
#define SEM_DATA_TEX_ID         5
#define REMOTE_DATA_TEX_ID      6

extern GLuint tex_ids [N_TEX];
extern GLubyte * sem_data;
extern GLubyte * remote_data;
// note: shader textures managed separately below

extern char g_alphaPlaneName [128];
extern char g_colorPlaneName [128];
extern char g_contourPlaneName [128];
extern char g_heightPlaneName [128];

extern BCGrid * g_inputGrid;

#ifdef FLOW

/*****************************************************************
 This part should be the same as nM_global.h.
 It will match which shader to turn on and which texture_id is
 mapped to where.
 *****************************************************************/

// total number of textures used by shaders
#define N_SHADER_TEX 7

// texture id num
#define BUMP_TEX_ID          0
#define PATTERN_TEX_ID       1
#define HATCH_NOISE_TEX_ID   2

//data texture id num
#define BUMP_DATA_TEX_ID         3
#define PATTERN_DATA_TEX_ID      4
#define HATCH_DATA_TEX_ID        5
#define ANI_CONTOUR_DATA_TEX_ID  6

// active shader bit for shader_mask
#define NO_SHADER      0
#define PATTERN_BIT    1
#define HATCH_BIT      2
#define BUMP_BIT       4
#define ANI_CONTOUR    8

// constant for which spot noise to use for the spot noise shader 
#define HATCH_ROTATE 0       // default
#define GUASS_BOX    1
#define GUASS_HATCH  2
#define GUASS_PLUS   3
#define GUASS_SIZE   4
#define PLUS_X       5
#define PLUS_RING    6
#define WOBBLE_PLUS  7
#define GAUSS_DISK   8

/******************************************************************/


  //shaders
  extern GLuint nM_shader;
  extern GLuint nM_diffuse;     // just shows the diffuse color shader

  // texture id for shaders
  extern GLuint shader_tex_ids[N_SHADER_TEX];

  // data set values
  extern GLubyte *pattern_data;  // data_tex_size*data_tex_size*3
  extern GLubyte *hatch_data;    // data_tex_size*data_tex_size*3
  extern GLubyte *bump_data;     // data_tex_size*data_tex_size*3

  // which shaders to turn on
  extern unsigned char shader_mask;

  // for time-varying phenomena
  extern float px_framenum;

  //=====Bump Shader=====
  extern float bumpiness;          // max amount of bumpiness
  extern float bump_pulserate;   // bump pulse rate
  extern float bump_scale;       // scale of bump texture

  //=====Pattern Blend Shader=====
  extern float pattern_blend;     // max amount of blending
  extern float pattern_color[3];  // color of the pattern to blend
  extern float pattern_scale;     // scale of the pattern texture
  extern float pattern_pulserate; // pattern pulse rate
  
  //=====Spot Noise Shader=====
  extern float spotnoise_tex_size;      // spot noise texture size
  extern float spot_pulserate;          // spot noise pulse rate
  extern float spot_blend;              // max amont of spot noise to blend in
  extern unsigned char whichspot;       // which spot noise to use
  extern float contrast;                // to adjust contrast on noise functions
  extern float rotate_amount;           // amount to rotate the kernel
  extern unsigned char swap_kernel; // switch kernels used for high and low data
  extern unsigned char kernel_size;     // size of the gaussian shaders
  extern unsigned char dense;           //sparse or dense noise
  extern float wobblerate;              // rate of wobbling for wobble_plus 

  // size of the data texture.
  extern int g_data_tex_size;

/******************************************************************/

#endif  // FLOW

#endif  // GRAPHICS_GLOBALS_H











