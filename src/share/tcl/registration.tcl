#/*===3rdtech===
#  Copyright (c) 2001 by 3rdTech, Inc.
#  All Rights Reserved.
#
#  This file may not be distributed without the permission of 
#  3rdTech, Inc. 
#  ===3rdtech===*/

# Not needed.
#global reg_window_open registration_needed reg_constrain_to_topography
#global reg_invert_warp
#global resample_resolution_x resample_resolution_y reg_resample_ratio
#global reg_surface_comes_from reg_projection_comes_from
#global resample_image_name

set reg_surface_comes_from "none"
set reg_projection_comes_from "none"

# Main dialog, for interaction to create a registered image. 
set nmInfo(registration) [create_closing_toplevel_with_notify \
                                      registration reg_window_open]

generic_optionmenu $nmInfo(registration).selection3D \
        reg_surface_cm(color_comes_from) \
        "Topography image" imageNames
button $nmInfo(registration).colormap3D -text "Colormap..."  \
    -command "show.reg_surf_colorscale"
#generic_optionmenu $nmInfo(registration).colormap3D \
#	reg_surface_colormap_from \
#	"Colormap" colorMapNames
pack $nmInfo(registration).selection3D $nmInfo(registration).colormap3D -anchor nw -pady 3

# Colormap controls, using routines from colormap.tcl
set nmInfo(reg_surf_colorscale) [create_closing_toplevel reg_surf_colorscale "Registration Topography Color Map" ]

# create the colormap controls. 
colormap_controls $nmInfo(reg_surf_colorscale) reg_surface_cm \
        reg_surface_cm(color_comes_from) "Topography image" imageNames

generic_optionmenu $nmInfo(registration).selection2D \
        reg_projection_cm(color_comes_from) \
        "Projection image" imageNames
button $nmInfo(registration).colormap2D -text "Colormap..."  \
    -command "show.reg_proj_colorscale"
#generic_optionmenu $nmInfo(registration).colormap2D \
#	reg_projection_colormap_from \
#	"Colormap" colorMapNames
pack $nmInfo(registration).selection2D $nmInfo(registration).colormap2D -anchor nw -pady 3

#proc printvar {fooa element op} {
#    global reg_projection_cm
#    puts "XXXX new  $reg_projection_cm(color_comes_from)"
#}
#trace variable reg_projection_cm(color_comes_from) w printvar

# Colormap controls, using routines from colormap.tcl
set nmInfo(reg_proj_colorscale) [create_closing_toplevel reg_proj_colorscale "Registration Projection Color Map" ]

# create the colormap controls. 
colormap_controls $nmInfo(reg_proj_colorscale) reg_projection_cm \
        reg_projection_cm(color_comes_from) "Projection image" imageNames

#make sure colormap windows close along with the rest of the interface. 
proc reg_close_cmap_windows {name el op} {
    global reg_window_open
    if {$reg_window_open == 0 } {
        hide.reg_surf_colorscale
        hide.reg_proj_colorscale
    }
}
trace variable reg_window_open w reg_close_cmap_windows

###########################################
# controls to align automatically using mutual information starting from the
# currently set correspondence
set auto_align_resolution_list {none}
set auto_align_mode_list {none}

button $nmInfo(registration).auto_align -text "Auto Align" -command \
    { set auto_align_requested 1 }
pack $nmInfo(registration).auto_align -anchor nw

generic_entry $nmInfo(registration).num_iteration_entry \
        auto_align_num_iterations "# iterations" numeric
pack $nmInfo(registration).num_iteration_entry -anchor nw

generic_entry $nmInfo(registration).step_size_entry auto_align_step_size \
        "step size (0,1)" real
pack $nmInfo(registration).step_size_entry -anchor nw

generic_optionmenu $nmInfo(registration).resolution_selector \
        auto_align_resolution \
        "resolution level" auto_align_resolution_list
pack $nmInfo(registration).resolution_selector -anchor nw

generic_optionmenu $nmInfo(registration).mode_selector \
        auto_align_mode \
        "mode" auto_align_mode_list
pack $nmInfo(registration).mode_selector -anchor nw

################ end of controls for automatic alignment

################ some manually-adjustable transformation parameters #####

frame $nmInfo(registration).transformParameters -bd 3 -relief groove
pack $nmInfo(registration).transformParameters -anchor nw

generic_entry $nmInfo(registration).transformParameters.scaleX \
        reg_scaleX "scale X" real
pack $nmInfo(registration).transformParameters.scaleX -anchor nw

generic_entry $nmInfo(registration).transformParameters.scaleY \
        reg_scaleY "scale Y" real
pack $nmInfo(registration).transformParameters.scaleY -anchor nw

generic_entry $nmInfo(registration).transformParameters.translateX \
        reg_translateX "translate X" real
pack $nmInfo(registration).transformParameters.translateX -anchor nw

generic_entry $nmInfo(registration).transformParameters.translateY \
        reg_translateY "translate Y" real
pack $nmInfo(registration).transformParameters.translateY -anchor nw

generic_entry $nmInfo(registration).transformParameters.rotateX \
        reg_rotateX "rotate X" real
pack $nmInfo(registration).transformParameters.rotateX -anchor nw

generic_entry $nmInfo(registration).transformParameters.rotateY \
        reg_rotateY "rotate Y" real
pack $nmInfo(registration).transformParameters.rotateY -anchor nw

generic_entry $nmInfo(registration).transformParameters.rotateZ \
        reg_rotateZ "rotate Z" real
pack $nmInfo(registration).transformParameters.rotateZ -anchor nw

generic_entry $nmInfo(registration).transformParameters.shearZ \
        reg_shearZ "shear Z" real
pack $nmInfo(registration).transformParameters.shearZ -anchor nw

#################################################################
set reg_transformation_source_list {none}

frame $nmInfo(registration).texture -bd 3 -relief groove
pack $nmInfo(registration).texture -anchor nw

generic_optionmenu $nmInfo(registration).texture.source_selector \
        reg_transformation_source \
        "source" reg_transformation_source_list
pack $nmInfo(registration).texture.source_selector -anchor nw

checkbutton $nmInfo(registration).texture.display_texture \
    -text "Display in Surface View" -variable reg_display_texture -anchor nw

pack $nmInfo(registration).texture.display_texture -anchor nw

iwidgets::Labeledframe $nmInfo(registration).rsplane \
	-labeltext "Create Plane (match Topog image region)" \
	-labelpos nw
set nmInfo(reg_plane) [$nmInfo(registration).rsplane childsite]
pack $nmInfo(registration).rsplane -fill x -anchor nw

generic_entry $nmInfo(reg_plane).resample_plane resample_plane_name \
	"Resample plane name" ""
pack $nmInfo(reg_plane).resample_plane -fill x -anchor nw

# make radio button group for transform options (2D-2D only, 3D-2D, etc)

# radio button group for resample options (surface region only), preserve
# entire texture, 

# checkbox option for resampling: blend using transparency and colormap 
# displayed in graphics window

# Secondary frame, for saving the resampled image. 
iwidgets::Labeledframe $nmInfo(registration).rsimage \
	-labeltext "Create Image (flexible region)" \
	-labelpos nw
set nmInfo(reg_image) [$nmInfo(registration).rsimage childsite]
pack $nmInfo(registration).rsimage  -fill x -anchor nw

generic_radiobox $nmInfo(reg_image).constrain_to_topography \
	reg_constrain_to_topography  \
	"Extent of image includes..." { "Whole projection image" \
        "Topography region only" }
pack $nmInfo(reg_image).constrain_to_topography -fill x -anchor nw

generic_radiobox $nmInfo(reg_image).invert_warp \
	reg_invert_warp  \
	"Registered image gets..." { "Reprojected" "Resampled" }

pack $nmInfo(reg_image).invert_warp -fill x -anchor nw

floatscale $nmInfo(reg_image).resample_ratio 0 1 101 1 1 \
       reg_resample_ratio "Portion Topog intensity mixed"
pack $nmInfo(reg_image).resample_ratio -anchor nw


# stuff for creating resampled data:
set resample_image_name ""

generic_entry $nmInfo(reg_image).resolution_x \
        resample_resolution_x "x resolution (10, 10000)" numeric

generic_entry $nmInfo(reg_image).resolution_y \
		resample_resolution_y "y resolution (10, 10000)" numeric

pack $nmInfo(reg_image).resolution_x $nmInfo(reg_image).resolution_y \
	 -fill x -anchor nw

generic_entry $nmInfo(reg_image).resample_image resample_image_name \
	"Resample image name" ""
pack $nmInfo(reg_image).resample_image  -fill x -anchor nw
