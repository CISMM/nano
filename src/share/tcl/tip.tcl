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
global tip_window_open tip_display_enable tip_display_texture_enable

set nmInfo(tip) [create_closing_toplevel_with_notify \
                            tip_win tip_window_open]
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
pack $nmInfo(tip_display).display_geometry_checkbutton

