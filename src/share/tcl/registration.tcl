#/*===3rdtech===
#  Copyright (c) 2001 by 3rdTech, Inc.
#  All Rights Reserved.
#
#  This file may not be distributed without the permission of 
#  3rdTech, Inc. 
#  ===3rdtech===*/

set reg_surface_comes_from "none"
set reg_projection_comes_from "none"

set spotTrackerFunctions { "None" "Local Max" "Cone" "Disk" "FIONA" "Symmetric" }

# Main dialog, for interaction to create a registered image. 
set nmInfo(registration) [create_closing_toplevel_with_notify \
                                      registration reg_window_open]
wm title $nmInfo(registration) "Registration"

########################

iwidgets::Labeledframe $nmInfo(registration).topographyControls \
	-labeltext "Topograpy Image" \
	-labelpos nw
set nmInfo(topographyControls) [$nmInfo(registration).topographyControls childsite]
pack $nmInfo(registration).topographyControls -anchor nw -expand true -fill x

frame $nmInfo(topographyControls).row1
frame $nmInfo(topographyControls).row2
frame $nmInfo(topographyControls).row3

pack $nmInfo(topographyControls).row1 $nmInfo(topographyControls).row2 $nmInfo(topographyControls).row3 -anchor nw

generic_optionmenu $nmInfo(topographyControls).row1.selection3D \
        reg_surface_cm(color_comes_from) \
        "Image" imageNames

button $nmInfo(topographyControls).row1.colormap3D -text "Colormap..."  \
    -command "show.reg_surf_colorscale"

button $nmInfo(topographyControls).row1.refresh -text "Refresh" \
    -command "set reg_refresh_3D 1"

pack $nmInfo(topographyControls).row1.selection3D \
	$nmInfo(topographyControls).row1.colormap3D \
	$nmInfo(topographyControls).row1.refresh \
	-anchor nw -side left -pady 1

generic_optionmenu $nmInfo(topographyControls).row2.spotTracker \
    reg_fiducial_spot_tracker_3D "Tracker" spotTrackerFunctions

checkbutton $nmInfo(topographyControls).row2.optimize_radius_checkbutton \
    -text "Optimize radius" -variable reg_spot_tracker_optimize_radius_3D -anchor nw

pack $nmInfo(topographyControls).row2.spotTracker \
	$nmInfo(topographyControls).row2.optimize_radius_checkbutton \
	-anchor w -side left -pady 1

floatscale $nmInfo(topographyControls).row3.radius 0 50 1000 1 1 \
	reg_spot_tracker_radius_3D "Radius"

floatscale $nmInfo(topographyControls).row3.radius_accuracy 0.001 1 1000 1 1 \
	reg_spot_tracker_radius_accuracy_3D "Radius accuracy"

floatscale $nmInfo(topographyControls).row3.pixel_accuracy 0.001 1 1000 1 1 \
	reg_spot_tracker_pixel_accuracy_3D "Pixel accuracy"

pack $nmInfo(topographyControls).row3.radius \
	$nmInfo(topographyControls).row3.radius_accuracy \
	$nmInfo(topographyControls).row3.pixel_accuracy \
	-anchor w -side left -pady 1


# Colormap controls, using routines from colormap.tcl
set nmInfo(reg_surf_colorscale) [create_closing_toplevel reg_surf_colorscale "Registration Topography Color Map" ]

# create the colormap controls. 
colormap_controls $nmInfo(reg_surf_colorscale) reg_surface_cm \
        reg_surface_cm(color_comes_from) "Topography image" imageNames

########################
iwidgets::Labeledframe $nmInfo(registration).projectionControls \
	-labeltext "Projection Image" \
	-labelpos nw
set nmInfo(projectionControls) [$nmInfo(registration).projectionControls childsite]
pack $nmInfo(registration).projectionControls -anchor nw -expand true -fill x

frame $nmInfo(projectionControls).row1
frame $nmInfo(projectionControls).row2
frame $nmInfo(projectionControls).row3
frame $nmInfo(projectionControls).row4

pack $nmInfo(projectionControls).row1 \
	$nmInfo(projectionControls).row2 \
	$nmInfo(projectionControls).row3 \
	$nmInfo(projectionControls).row4 \
	-anchor nw

generic_optionmenu $nmInfo(projectionControls).row1.selection2D \
        reg_projection_cm(color_comes_from) \
        "Image" imageNames

button $nmInfo(projectionControls).row1.colormap2D -text "Colormap..."  \
    -command "show.reg_proj_colorscale"

button $nmInfo(projectionControls).row1.refresh -text "Refresh" \
    -command "set reg_refresh_2D 1"

pack $nmInfo(projectionControls).row1.selection2D \
	$nmInfo(projectionControls).row1.colormap2D \
	$nmInfo(projectionControls).row1.refresh \
	-anchor nw -side left -pady 1


checkbutton $nmInfo(projectionControls).row2.flip_projection_image_x_checkbutton \
    -text "Flip image in X" -variable reg_proj_flipX -anchor nw

pack $nmInfo(projectionControls).row2.flip_projection_image_x_checkbutton \
	-anchor w -side left -pady 1

generic_optionmenu $nmInfo(projectionControls).row3.spotTracker \
    reg_fiducial_spot_tracker_2D "Tracker" spotTrackerFunctions

checkbutton $nmInfo(projectionControls).row3.optimize_radius_checkbutton \
    -text "Optimize radius" -variable reg_spot_tracker_optimize_radius_2D -anchor nw

pack $nmInfo(projectionControls).row3.spotTracker \
	$nmInfo(projectionControls).row3.optimize_radius_checkbutton \
	-anchor w -side left -pady 1

floatscale $nmInfo(projectionControls).row4.radius 0 50 1000 1 1 \
	reg_spot_tracker_radius_2D "Radius"

floatscale $nmInfo(projectionControls).row4.radius_accuracy 0.001 1 1000 1 1 \
	reg_spot_tracker_radius_accuracy_2D "Radius accuracy"

floatscale $nmInfo(projectionControls).row4.pixel_accuracy 0.001 1 1000 1 1 \
	reg_spot_tracker_pixel_accuracy_2D "Pixel accuracy"

pack $nmInfo(projectionControls).row4.radius \
	$nmInfo(projectionControls).row4.radius_accuracy \
	$nmInfo(projectionControls).row4.pixel_accuracy \
	-anchor w -side left -pady 1


# Colormap controls, using routines from colormap.tcl
set nmInfo(reg_proj_colorscale) [create_closing_toplevel reg_proj_colorscale "Registration Projection Color Map" ]

# create the colormap controls. 
colormap_controls $nmInfo(reg_proj_colorscale) reg_projection_cm \
        reg_projection_cm(color_comes_from) "Projection image" imageNames

########################
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

iwidgets::Labeledframe $nmInfo(registration).auto_align \
	-labeltext "Automatic Alignment" \
	-labelpos nw
set nmInfo(auto_align) [$nmInfo(registration).auto_align childsite]
pack $nmInfo(registration).auto_align -anchor nw -expand true -fill x

frame $nmInfo(auto_align).row1
frame $nmInfo(auto_align).row2

pack $nmInfo(auto_align).row1 $nmInfo(auto_align).row2 -anchor nw

button $nmInfo(auto_align).row1.auto_align -text "Auto Align" -command \
    { set auto_align_requested 1 }

generic_entry $nmInfo(auto_align).row1.num_iteration_entry \
        auto_align_num_iterations "# iterations" numeric

generic_entry $nmInfo(auto_align).row1.step_size_entry auto_align_step_size \
        "Step size (0,1)" real

pack $nmInfo(auto_align).row1.auto_align \
	$nmInfo(auto_align).row1.num_iteration_entry \
	$nmInfo(auto_align).row1.step_size_entry \
	-anchor w -side left -pady 1

generic_optionmenu $nmInfo(auto_align).row2.resolution_selector \
        auto_align_resolution \
        "Resolution level" auto_align_resolution_list

generic_optionmenu $nmInfo(auto_align).row2.mode_selector \
        auto_align_mode \
        "Mode" auto_align_mode_list

pack $nmInfo(auto_align).row2.resolution_selector \
	$nmInfo(auto_align).row2.mode_selector \
	-anchor w -side left

################ end of controls for automatic alignment

iwidgets::Labeledframe $nmInfo(registration).initialPoints \
	-labeltext "Save/Load Registration Markers" \
	-labelpos nw 
set nmInfo(initialPoints) [$nmInfo(registration).initialPoints childsite]
pack $nmInfo(registration).initialPoints -anchor nw -expand true -fill x

frame $nmInfo(initialPoints).row1

pack $nmInfo(initialPoints).row1 -anchor nw

button $nmInfo(initialPoints).row1.save -text "Save Markers"  \
    -command "set save_registration_markers 1"

button $nmInfo(initialPoints).row1.load -text "Load Markers"  \
    -command "set load_registration_markers 1"

pack $nmInfo(initialPoints).row1.save \
	$nmInfo(initialPoints).row1.load \
	-anchor nw -side left -pady 1
################ end of controls for saving points


iwidgets::Labeledframe $nmInfo(registration).report \
	-labeltext "Report" \
	-labelpos nw 
set nmInfo(report) [$nmInfo(registration).report childsite]
pack $nmInfo(registration).report -anchor nw -expand true -fill x

frame $nmInfo(report).row1

pack $nmInfo(report).row1 -anchor nw

button $nmInfo(report).row1.createReport -text "Create Report"  \
    -command "set save_report 1"


pack $nmInfo(report).row1.createReport \
	-anchor nw -side left -pady 1
################ end of controls for methods section

iwidgets::Labeledframe $nmInfo(registration).ransac \
	-labeltext "Find Correspondence" \
	-labelpos nw 
set nmInfo(ransac) [$nmInfo(registration).ransac childsite]
pack $nmInfo(registration).ransac -anchor nw -expand true -fill x

frame $nmInfo(ransac).row1
frame $nmInfo(ransac).row2
frame $nmInfo(ransac).row3
frame $nmInfo(ransac).row4

pack $nmInfo(ransac).row1 $nmInfo(ransac).row2 $nmInfo(ransac).row3 $nmInfo(ransac).row4 -anchor nw

button $nmInfo(ransac).row1.calculatePoints -text "Calculate Points"  \
    -command "set run_calculatePoints 1"

pack $nmInfo(ransac).row1.calculatePoints \
	-anchor nw -side left -pady 1

button $nmInfo(ransac).row2.drawTopo -text "Draw Topography Points"  \
    -command "set run_drawTopographyPoints 1"

button $nmInfo(ransac).row2.saveTopo -text "Save Topography Points"  \
    -command "set run_saveTopographyPoints 1"

pack $nmInfo(ransac).row2.drawTopo \
	$nmInfo(ransac).row2.saveTopo \
	-anchor w -side left -pady 1

button $nmInfo(ransac).row3.drawProj -text "Draw Projection Points"  \
    -command "set run_drawProjectionPoints 1"

button $nmInfo(ransac).row3.saveProj -text "Save Projection Points"  \
    -command "set run_saveProjectionPoints 1"

pack $nmInfo(ransac).row3.drawProj \
	$nmInfo(ransac).row3.saveProj \
	-anchor w -side left -pady 1

button $nmInfo(ransac).row4.run_calcCorr -text "Calculate Correspondences"  \
    -command "set run_ransac 1"

button $nmInfo(ransac).row4.run_drawCorr -text "Draw Correspondences"  \
    -command "set draw_ransac 1"


pack $nmInfo(ransac).row4.run_calcCorr \
	$nmInfo(ransac).row4.run_drawCorr \
	-anchor nw -side left -pady 1

################ end of controls for ransac

################ some manually-adjustable transformation parameters #####

iwidgets::Labeledframe $nmInfo(registration).transformParameters \
	-labeltext "Manual Alignment Parameters" \
	-labelpos nw
set nmInfo(transformParameters) [$nmInfo(registration).transformParameters childsite]
#pack $nmInfo(registration).transformParameters -anchor nw -expand true -fill x

#generic_entry $nmInfo(registration).transformParameters.shearZ \
#        reg_shearZ "shear Z" real
#pack $nmInfo(registration).transformParameters.shearZ -anchor nw

frame $nmInfo(transformParameters).row1
frame $nmInfo(transformParameters).row2
frame $nmInfo(transformParameters).row3
pack $nmInfo(transformParameters).row1 \
	$nmInfo(transformParameters).row2 \
	$nmInfo(transformParameters).row3 \
	-anchor nw -side top

generic_entry $nmInfo(transformParameters).row1.translateX \
	reg_translateX "Translate X" real

generic_entry $nmInfo(transformParameters).row1.translateY \
	reg_translateY "Translate Y" real

#generic_entry $nmInfo(registration).transformParameters.translateZ \
#        reg_translateZ "translate Z" real
#pack $nmInfo(registration).transformParameters.translateZ -anchor nw

generic_entry $nmInfo(transformParameters).row1.scaleX \
	reg_scaleX "Scale X" real

generic_entry $nmInfo(transformParameters).row1.scaleY \
	reg_scaleY "Scale Y" real


#pack $nmInfo(transformParameters).row1.translateX \
#	$nmInfo(transformParameters).row1.translateY \
#	$nmInfo(transformParameters).row1.scaleX \
#	$nmInfo(transformParameters).row1.scaleY \
#	-anchor nw -side left

generic_entry $nmInfo(transformParameters).row2.rotate2D_Z \
	reg_rotate2D_Z "2D rotation" real

#pack $nmInfo(transformParameters).row2.rotate2D_Z \
#	-anchor nw -side left

generic_entry $nmInfo(transformParameters).row3.rotate3D_X \
        reg_rotate3D_X "Altitude angle (deg.)" real

generic_entry $nmInfo(transformParameters).row3.rotate3D_Z \
        reg_rotate3D_Z "Azimuth angle (deg.)" real

#pack $nmInfo(transformParameters).row3.rotate3D_X \
#	$nmInfo(transformParameters).row3.rotate3D_Z \
#	-anchor nw -side left


#################################################################
set reg_transformation_source_list {none}

iwidgets::Labeledframe $nmInfo(registration).texture \
	-labeltext "Image Overlay" \
	-labelpos nw
set nmInfo(texture_controls) [$nmInfo(registration).texture childsite]
pack $nmInfo(registration).texture -anchor nw -expand true -fill x

frame $nmInfo(texture_controls).row1
frame $nmInfo(texture_controls).row2
pack $nmInfo(texture_controls).row1 $nmInfo(texture_controls).row2 -anchor nw

generic_optionmenu $nmInfo(texture_controls).row1.source_selector \
        reg_transformation_source \
        "Source" reg_transformation_source_list

floatscale $nmInfo(texture_controls).row1.texture_alpha 0 1 1000 1 1 \
    reg_texture_alpha "Texture alpha value"

checkbutton $nmInfo(texture_controls).row1.display_texture \
    -text "Display" -variable reg_display_texture -anchor nw

pack $nmInfo(texture_controls).row1.source_selector \
	$nmInfo(texture_controls).row1.texture_alpha \
	-side left -pady 1 -fill x

pack $nmInfo(texture_controls).row1.display_texture -side left -pady 1 -fill x


iwidgets::Labeledframe $nmInfo(registration).rsplane \
	-labeltext "Create Plane (match topography image region)" \
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
$nmInfo(reg_image).constrain_to_topography configure -orient horizontal

pack $nmInfo(reg_image).constrain_to_topography -anchor nw

generic_radiobox $nmInfo(reg_image).invert_warp \
	reg_invert_warp  \
	"When resampling..." { "Warp projection" "Warp topography" }
$nmInfo(reg_image).invert_warp configure -orient horizontal

pack $nmInfo(reg_image).invert_warp -anchor nw

floatscale $nmInfo(reg_image).resample_ratio 0 1 101 1 1 \
       reg_resample_ratio "Portion unwarped image intensity mixed"
pack $nmInfo(reg_image).resample_ratio -anchor nw


# stuff for creating resampled data:
set resample_image_name ""

frame $nmInfo(reg_image).row1
frame $nmInfo(reg_image).row2
pack $nmInfo(reg_image).row1 $nmInfo(reg_image).row2 -anchor nw -expand true -fill x

generic_entry $nmInfo(reg_image).row1.resolution_x \
        resample_resolution_x "X resolution (10, 10000)" numeric

generic_entry $nmInfo(reg_image).row1.resolution_y \
		resample_resolution_y "Y resolution (10, 10000)" numeric

pack $nmInfo(reg_image).row1.resolution_x $nmInfo(reg_image).row1.resolution_y \
	 -fill x -anchor nw -side left

generic_entry $nmInfo(reg_image).row2.resample_image resample_image_name \
	"Resample image name" ""
pack $nmInfo(reg_image).row2.resample_image  -fill x -anchor nw
