#!/bin/sh
# ----------------------------------------------------------------------
#  reg interface
# ----------------------------------------------------------------------
#\
exec wish "$0" ${3+"$@"}

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

# When the entry value has not been finalized by hitting "Enter",
# the entry background will be red.

# called when the C code sets the tcl variable
proc reg_updateEntry {entry var name element op} {
    puts "reg_updateEntry"
    upvar #0 $entry.textbg mybg
    upvar #0 $var varval
    if { $varval != [$entry get] } {
    $entry delete 0 end
    $entry insert 0 $varval
    }
    $entry configure -textbackground $mybg
}

proc reg_setEntry {entry var } {
    puts "reg_setEntry"
    upvar #0 $entry.textbg mybg
    upvar #0 $var varval
    if { $varval != [$entry get] } {
    set varval [$entry get]
    }
    $entry configure -textbackground $mybg
}

#Called when the user types a new character in the entry widget
# Warns them that their changes have not been finalized.
proc reg_setWarnBackground { entry name element op } {
    $entry configure -textbackground LightPink1
}

# Create an integer entry field.
proc reg_entry { name var label validation} {
    upvar #0 $var varval
    iwidgets::entryfield "$name" \
        -command "reg_setEntry $name $var" \
        -validate $validation \
        -textvariable "$name.gvar" \
        -labeltext "$label" \
        -labelpos n
    #set the initial contents.
    $name insert 0 $varval

    global $name.textbg $name.gvar

    #Store the default background so we can use it later
    set $name.textbg [$name cget -textbackground]

    # Find out when the user types in the entry field
#    trace variable $name.gvar w "reg_setWarnBackground $name"

    # Find out when the global variable connected with the entry
    # gets set by someone else.
    trace variable varval w "reg_updateEntry $name $var "

    return $name
}

# GLOBAL variables
# ----------------------------------------------------------------------
# If set to 1, the C code will handle associated cleanup
global reg_window_open
set reg_window_open 0

# Default padding around the entry widgets, in pixels.
set def_pad 1

# reg_widgets is a global array for containing widget paths and names.
# reg is a global array for containing control variables,
#   especially those linked to C++ code

# reg_win should be set to the path where these controls will
# show up. In a standalone application, do something like this:
# set reg_win [toplevel .reg_win]
# But if you have another place to put it, do:
# set reg_win [frame .path.reg_win]
# pack $reg_win
# ------------------------------------------------- end GLOBAL variables

pack [frame $reg_win.left] -side left -expand yes -fill both -padx 10
#-----------------------------
iwidgets::labeledframe $reg_win.left.reg_frame -labeltext \
    "Registration Controls"
set reg_widgets(reg_frame) [$reg_win.left.reg_frame childsite]

pack $reg_win.left.reg_frame -side top -fill both

# Selector to choose height field data
set reg_widgets(selection3D) [frame $reg_widgets(reg_frame).selection3D]
pack $reg_widgets(selection3D)

# Selector to choose texture-mapped data
set reg_widgets(selection2D) [frame $reg_widgets(reg_frame).selection2D]
pack $reg_widgets(selection2D)

# controls that modify the registration algorithm's behavior
set reg_widgets(rotate3D_enable) [checkbutton \
         $reg_widgets(reg_frame).rotate3D_enable -text "3D Rotation" \
         -variable reg(rotate3D_enable) ]
pack $reg_widgets(rotate3D_enable) -anchor w

set reg_widgets(register) [button \
        $reg_widgets(reg_frame).register -text "Register" \
        -command { set reg(registration_needed) 1}]
pack $reg_widgets(register) -anchor w
 
set reg_widgets(display_texture) [checkbutton \
        $reg_widgets(reg_frame).display_texture \
        -text "Display Texture" -variable reg(display_texture) ]
pack $reg_widgets(display_texture) -anchor w

set reg(resample_plane_name) ""
set reg_widgets(resample_plane) [frame $reg_widgets(reg_frame).resample_plane \
              -relief raised -bd 4]
   set reg_widgets(resample_plane_choice) [frame \
        $reg_widgets(resample_plane).choice]
   set reg_widgets(resample_plane_name) [frame \
        $reg_widgets(resample_plane).name]
   pack $reg_widgets(resample_plane_choice)
   pack $reg_widgets(resample_plane_name)
   label $reg_widgets(resample_plane_name).label -text "Resample plane name"
   pack $reg_widgets(resample_plane_name).label
   newlabel_dialogue reg(resample_plane_name) $reg_widgets(resample_plane_name)
pack $reg_widgets(resample_plane) -fill both

if { 0==[info exists reg(resample_resolution_x)] } \
          { set reg(resample_resolution_x) 0 }
set reg_widgets(resample_resolution_x) \
    [reg_entry $reg_widgets(reg_frame).resample_resolution_x \
     reg(resample_resolution_x) "resample x resolution" integer ]
pack $reg_widgets(resample_resolution_x) -padx $def_pad -pady $def_pad

if { 0==[info exists reg(resample_resolution_y)] } \
          { set reg(resample_resolution_y) 0 } 
set reg_widgets(resample_resolution_y) \
    [reg_entry $reg_widgets(reg_frame).resample_resolution_y \
     reg(resample_resolution_y) "resample y resolution" integer ]
pack $reg_widgets(resample_resolution_y) -padx $def_pad -pady $def_pad

# align labels so it looks nice
iwidgets::Labeledwidget::alignlabels \
    $reg_widgets(resample_resolution_x) \
    $reg_widgets(resample_resolution_y)
 
