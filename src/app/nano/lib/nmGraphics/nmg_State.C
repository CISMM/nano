/*===3rdtech===
  Copyright (c) 2000 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#include "nmg_State.h"
#include "nmg_Graphics.h" // for enums
#include "nmg_Surface.h"

class BCGrid;  // from BCGrid.h
class nmb_Subgrid;  // from nmb_Subgrid.h

// Attention:
// NO Tclvars in this file!  Nothing that needs to be shared outside!


nmg_State::nmg_State ()
{
    alpha_r = 0.0f;
    alpha_g = 1.0f;
    alpha_b = 0.0f;
    alpha_slider_min = 0.0f;
    alpha_slider_max = 1.0f;

    colorMapDir = NULL;
    curColorMap = NULL;

    just_color = 0;

    color_min = 0; // nmb_ColorMap lower and upper bounds for color
    color_max = 1.0;

    displayIndexList = NULL;

    data_min_norm = 0; // nmb_ColorMap lower and upper bounds for data, normalized
    data_max_norm = 1.0;
    data_min = 0; // data min/max, real units.
    data_max = 1.0;

    opacity_slider_min = 0.0f;
    opacity_slider_max = 1.0f;

    textureDir = NULL;

    config_chartjunk = 1;
    config_measurelines = 1;
    config_planeonly = 0;
    config_filled_polygons = 1;
    config_smooth_shading = 1;
    config_trueTip = 0;
    config_enableUber = 1;

    contour_r = 255;
    contour_g = 55;
    contour_b = 55;
    contour_width = 10.0f;

    hand_color = 0;  
    null_data_alpha_toggle = 0;
    icon_scale = 0.25f;

    position_collab_hand = 0;
    draw_collab_hand = 0;
    collabMode = 0;

    surfaceColor[0] = 1.0f;
    surfaceColor[1] = 0.88f;
    surfaceColor[2] = 0.04f;
    surfaceColor[3] = 1.0f;

    // Realigning Textures:
    realign_textures_curColorMap = NULL;
    realign_textures_data_min = 0;
    realign_textures_data_max = 1.0;
    realign_textures_color_min = 0;
    realign_textures_color_max = 1.0;
    realign_texture_name[0] = '\0';

    prerendered_grid = NULL;
    prerenderedChange = NULL;

    translate_textures = 0;
    scale_textures = 0;
    shear_textures = 0;
    rotate_textures = 0;

    tex_coord_center_x = 0.5;
    tex_coord_center_y = 0.5;
    tex_range_x = 1.0;
    tex_range_y = 1.0;
    tex_theta_cumulative = 0.0;
    translate_tex_x = 0.0;
    translate_tex_y = 0.0;
    scale_tex_x = 1000.0;
    scale_tex_y = 1000.0;
    shear_tex_x = 0.0;
    shear_tex_y = 0.0;


    translate_tex_dx = 0;
    translate_tex_dy = 0;
    scale_tex_dx = 0;
    scale_tex_dy = 0;
    shear_tex_dx = 0;
    shear_tex_dy = 0;
    rotate_tex_theta = 0;

    // Registration
    texture_transform[0] = 0.001;
    texture_transform[1] = 0;
    texture_transform[2] = 0;
    texture_transform[3] = 0;
    texture_transform[4] = 0;
    texture_transform[5] = 0.001;
    texture_transform[6] = 0;
    texture_transform[7] = 0;
    texture_transform[8] = 0;
    texture_transform[9] = 0;
    texture_transform[10] = 1;
    texture_transform[11] = 0;
    texture_transform[12] = 0;
    texture_transform[13] = 0;
    texture_transform[14] = 0;
    texture_transform[15] = 1;

    scanlinePt [0] = 0.0;
    scanlinePt [1] = 0.0;
    scanlinePt [2] = 0.0;
    scanlinePt [3] = 0.0;
    scanlinePt [4] = 0.0;
    scanlinePt [5] = 0.0;
    scanline_display_enabled = 0;

    rulergrid_xoffset = 0.0f;
    rulergrid_yoffset = 0.0f;
    rulergrid_scale = 500.0f;
    ruler_r = 255;
    ruler_g = 255;
    ruler_b = 55;

    //set default rulergrid opacity to 178 (70%) instead of 255 (100%)
    ruler_opacity = 178.0f;
    ruler_width_x = 1.0f;
    ruler_width_y = 1.0f;
    rulergrid_sin = 0.0f;
    rulergrid_cos = 1.0f;
    rulerPPM = NULL;

    // Material properties of the surface
    shiny = 55;
    local_viewer = vrpn_TRUE;
    diffuse = .50;
    specular_color = 1.0;

    surface_alpha = 1.0;

    sphere_scale = 12.5f;
    stride = 1;

    xs_state[0].enabled = 0;
    xs_state[1].enabled = 0;
    xs_state[2].enabled = 0;

    texture_displayed = nmg_Graphics::NO_TEXTURES;
    texture_transform_mode = nmg_Graphics::RULERGRID_COORD;

    texture_mode = GL_FALSE;

    texture_scale = 10.0f;
    trueTipScale = 1.0f;

    PRERENDERED_COLORS = 0;  // only used by remote rendering clients
    PRERENDERED_TEXTURE = 0;  // only used by remote rendering clients
    PRERENDERED_DEPTH = 0;  // only used by remote rendering clients

    inputGrid = NULL;

    mask = ENABLE_MASK;
    transparent = 0;
    viztex_scale = 500.0f;

    vizPPM = NULL;

    //////////////////////////////////////////////////////////////////
    // Variables that define what GL can and can't do on the machine
    // we are running on.
    //////////////////////////////////////////////////////////////////

    VERTEX_ARRAY = 0;

    //////////////////////////////////////////////////////////////////
    // End GL capabilities
    //////////////////////////////////////////////////////////////////
}

// Local Variables:
// mode:c++
// End:
