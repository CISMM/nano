global reg_window_open registration_needed reg_constrain_to_topography
global reg_invert_warp
global resample_resolution_x resample_resolution_y reg_resample_ratio
global reg_surface_comes_from reg_projection_comes_from
global resample_image_name

set reg_surface_comes_from "none"
set reg_projection_comes_from "none"

set nmInfo(registration) [create_closing_toplevel_with_notify \
                                      registration reg_window_open]


generic_optionmenu $nmInfo(registration).selection3D reg_surface_comes_from \
        "Topography image" imageNames
pack $nmInfo(registration).selection3D -anchor nw -padx 3 -pady 3

generic_optionmenu $nmInfo(registration).selection2D reg_projection_comes_from \
        "Projection image" imageNames
pack $nmInfo(registration).selection2D -anchor nw -padx 3 -pady 3

# make radio button group for transform options (2D-2D only, 3D-2D, etc)

# radio button group for resample options (surface region only), preserve
# entire texture, 

# checkbox option for resampling: blend using transparency and colormap 
# displayed in graphics window

checkbutton $nmInfo(registration).constrain_to_topography \
       -text "Topography Region Only" -variable reg_constrain_to_topography \
       -anchor nw
pack $nmInfo(registration).constrain_to_topography

checkbutton $nmInfo(registration).invert_warp \
       -text "Resample Warped Topography (instead of Projection)" \
       -variable reg_invert_warp \
       -anchor nw
pack $nmInfo(registration).invert_warp

floatscale $nmInfo(registration).resample_ratio 0 1 101 1 1 \
       reg_resample_ratio "Resample Mixing Ratio"
pack $nmInfo(registration).resample_ratio

button $nmInfo(registration).register -text "Register" -command \
    { set registration_needed 1 }
pack $nmInfo(registration).register

checkbutton $nmInfo(registration).display_texture \
       -text "Display Texture" -variable reg_display_texture -anchor nw
pack $nmInfo(registration).display_texture

# stuff for creating resampled data:
set resample_image_name ""
#set resample_from ""
frame $nmInfo(registration).resample_image -relief raised -bd 4
frame $nmInfo(registration).resample_image.choice
frame $nmInfo(registration).resample_image.name
pack $nmInfo(registration).resample_image.choice -side left
pack $nmInfo(registration).resample_image.name
label $nmInfo(registration).resample_image.name.label \
      -text "Resample image name"
pack $nmInfo(registration).resample_image.name.label
newlabel_dialogue resample_image_name $nmInfo(registration).resample_image.name
pack $nmInfo(registration).resample_image -fill both

intscale $nmInfo(registration).resolution_x 10 10000 9991 1 1 \
		resample_resolution_x "x resolution"

intscale $nmInfo(registration).resolution_y 10 10000 9991 1 1 \
		resample_resolution_y "y resolution"

pack $nmInfo(registration).resolution_x $nmInfo(registration).resolution_y \
	-fill both
