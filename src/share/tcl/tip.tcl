#!/bin/sh
# ----------------------------------------------------------------------
#  AFM tip display interface
# ----------------------------------------------------------------------
#\
exec wish "$0" ${2+"$@"}

# Import Itcl and Iwidgets
# This import method is a BUG - we shouldn't have to import
# the itcl and itk namespaces, but at least we can use it...
package require Itcl 
catch {namespace import itcl::*}
package require Itk 
catch {namespace import itk::*}
package require Iwidgets
# the extended file selection dialog does not work properly if 
# the iwidget namespace is imported.
#catch {namespace import -force iwidgets::*}
# Unfortunately, iwidgets and BLT have a namespace conflict 
# with the command "clock" - requires the -force option to be successful.

# GLOBAL variables
# ----------------------------------------------------------------------
global tip_window_open tip_display_enable tip_display_texture_enable \
       tip_model_mode_image tip_model_mode_conesphere tip_model_mode \
       tip_cone_sphere_radius tip_cone_sphere_angle tip_cone_sphere_height

set nmInfo(tip) [create_closing_toplevel_with_notify \
                            tip_win tip_window_open]
set tip_model_mode_image 1
set tip_model_mode_conesphere 2
set tip_model_mode 2
# ------------------------------------------------- end GLOBAL variables

iwidgets::labeledframe $nmInfo(tip).display -labeltext "AFM tip display"
pack $nmInfo(tip).display -anchor nw -padx 3 -pady 3

set nmInfo(tip_display) [$nmInfo(tip).display childsite]

# Controls whether or not SEM image of tip is displayed
checkbutton $nmInfo(tip_display).display_texture_checkbutton \
      -text "Display Texture" -variable tip_display_texture_enable
pack $nmInfo(tip_display).display_texture_checkbutton -anchor w

# Controls whether or not tip geometry is displayed
checkbutton $nmInfo(tip_display).display_geometry_checkbutton \
        -text "Display Geometry" -variable tip_display_enable
pack $nmInfo(tip_display).display_geometry_checkbutton -anchor w

frame $nmInfo(tip_display).tip_model -bd 3 -relief groove
pack $nmInfo(tip_display).tip_model -side left

label $nmInfo(tip_display).tip_model.frame_label -text "Tip Model"
pack $nmInfo(tip_display).tip_model.frame_label -side top

frame $nmInfo(tip_display).tip_model.topography_image -bd 3 -relief groove
pack $nmInfo(tip_display).tip_model.topography_image -side top

radiobutton $nmInfo(tip_display).tip_model.topography_image.button \
  -variable tip_model_mode \
  -value 1 -text "use topography image" -justify left
pack $nmInfo(tip_display).tip_model.topography_image.button -side top

generic_optionmenu \
   $nmInfo(tip_display).tip_model.topography_image.image_selector \
   tip_topography_image \
   "tip topography image" imageNames
pack $nmInfo(tip_display).tip_model.topography_image.image_selector

frame $nmInfo(tip_display).tip_model.cone_sphere -bd 3 -relief groove
pack $nmInfo(tip_display).tip_model.cone_sphere -side left -anchor w

radiobutton $nmInfo(tip_display).tip_model.cone_sphere.button \
  -variable tip_model_mode \
  -value 2 -text "use cone-sphere" -justify left
pack $nmInfo(tip_display).tip_model.cone_sphere.button -side top

generic_entry $nmInfo(tip_display).tip_model.cone_sphere.radius \
  tip_cone_sphere_radius "radius (nm)" numeric

generic_entry $nmInfo(tip_display).tip_model.cone_sphere.angle \
  tip_cone_sphere_angle "angle (deg)" numeric

generic_entry $nmInfo(tip_display).tip_model.cone_sphere.height \
  tip_cone_sphere_height "height (nm)" numeric

pack $nmInfo(tip_display).tip_model.cone_sphere.radius \
     $nmInfo(tip_display).tip_model.cone_sphere.angle \
     $nmInfo(tip_display).tip_model.cone_sphere.height

# sends current tip location as a fiducial to the alignment function
button $nmInfo(tip_display).send_fiducial_button \
        -text "Send Fiducial" -command { set send_fiducial_requested 1 }
pack $nmInfo(tip_display).send_fiducial_button
