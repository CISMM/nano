global reg_window_open registration_needed reg_constrain_to_topography
global reg_invert_warp
global resample_resolution_x resample_resolution_y reg_resample_ratio
global reg_surface_comes_from reg_projection_comes_from
global resample_image_name

set reg_surface_comes_from "none"
set reg_projection_comes_from "none"

# Main dialog, for interaction to create a registered image. 
set nmInfo(registration) [create_closing_toplevel_with_notify \
                                      registration reg_window_open]

generic_optionmenu $nmInfo(registration).selection3D reg_surface_comes_from \
        "Topography image" imageNames
pack $nmInfo(registration).selection3D -anchor nw -pady 3

generic_optionmenu $nmInfo(registration).selection2D reg_projection_comes_from \
        "Projection image" imageNames
pack $nmInfo(registration).selection2D -anchor nw -pady 3

button $nmInfo(registration).register -text "Register" -command \
    { set registration_needed 1 }
pack $nmInfo(registration).register -anchor nw

checkbutton $nmInfo(registration).display_texture \
       -text "Display in Surface View" -variable reg_display_texture -anchor nw
pack $nmInfo(registration).display_texture -anchor nw

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
