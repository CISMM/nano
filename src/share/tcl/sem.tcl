#!/bin/sh
# ----------------------------------------------------------------------
#  sem interface
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

# When the entry value has not been finalized by hitting "Enter",
# the entry background will be red.

# called when the C code sets the tcl variable
proc sem_updateEntry {entry var name element op} {
    puts "sem_updateEntry"
    upvar #0 $entry.textbg mybg
    upvar #0 $var varval
    if { $varval != [$entry get] } {
    $entry delete 0 end
    $entry insert 0 $varval
    }
    $entry configure -textbackground $mybg
}

# Called when the user hits "enter" in the entry widget
proc sem_setEntry {entry var } {
    puts "sem_setEntry"
    upvar #0 $entry.textbg mybg
    upvar #0 $var varval
    if { $varval != [$entry get] } {
    set varval [$entry get]
    }
    $entry configure -textbackground $mybg
}

#Called when the user types a new character in the entry widget
# Warns them that their changes have not been finalized.
proc sem_setWarnBackground { entry name element op } {
    $entry configure -textbackground LightPink1
}

# Create an integer entry field.
proc mysem_entry { name var label validation} {
    upvar #0 $var varval
    iwidgets::entryfield "$name" \
        -command "sem_setEntry $name $var" \
        -validate $validation \
        -textvariable "$name.gvar" \
        -labeltext "$label"
    #set the initial contents.
    $name insert 0 $varval

    global $name.textbg $name.gvar

    #Store the default background so we can use it later
    set $name.textbg [$name cget -textbackground]

    # Find out when the user types in the entry field
#    trace variable $name.gvar w "sem_setWarnBackground $name"

    # Find out when the global variable connected with the entry
    # gets set by someone else.
    trace variable varval w "sem_updateEntry $name $var "

    return $name
}

# GLOBAL variables
# ----------------------------------------------------------------------
# If set to 1, the C code will handle associated cleanup
global sem_window_open
set sem_window_open 0

# Default padding around the entry widgets, in pixels. 
set def_pad 1


# semwidgets is a global array for containing widget paths and names.
# sem is a global array for containing control variables, 
#   especially those linked to C++ code. 

# sem_win should be set to the path where these controls will 
# show up. In a standalone application, do something like this:
# set sem_win [toplevel .sem_win] 
# But if you have another place to put it, do:
# set sem_win [frame .path.sem_win]
# pack $sem_win 
# ------------------------------------------------- end GLOBAL variables

# Menu bar
#frame $sem_win.mbar -borderwidth 2 -relief raised
#pack $sem_win.mbar -fill x
#menubutton $sem_win.mbar.main -text "Main" -menu $sem_win.mbar.main.m
#pack $sem_win.mbar.main -side left

#menu $sem_win.mbar.main.m
#$sem_win.mbar.main.m add command -label "Quit" -command exit
#button $sem_win.mbar.close -text "Close" -command { set sem_quit 1 }
#pack $sem_win.mbar.close -anchor nw

pack [frame $sem_win.left] -side left -expand yes -fill both -padx 10
#-----------------------------
iwidgets::labeledframe $sem_win.left.sem_frame -labeltext "SEM controls"
set semwidgets(sem_frame) [$sem_win.left.sem_frame childsite]


pack $sem_win.left.sem_frame -side top -fill both

# Button to start the scan
set semwidgets(acquire_image) [button $semwidgets(sem_frame).acquire_image -text "Acquire Image" \
	-command { set sem(acquire_image) 1 } ]
pack $semwidgets(acquire_image) -anchor w

# Controls whether to scan continuously or not
set semwidgets(acquire_continuous) [checkbutton \
        $semwidgets(sem_frame).acquire_continuous -text "Acquire Continuously" \
	-variable sem(acquire_continuous) ]
pack $semwidgets(acquire_continuous) -anchor w

# Controls whether or not image is displayed as a surface texture
set semwidgets(display_texture) [checkbutton $semwidgets(sem_frame).display_texture \
      -text "Display Texture" -variable sem(display_texture) ]
pack $semwidgets(display_texture) -anchor w

# Controls whether to update image data and displays
set semwidgets(network_test) [checkbutton \
        $semwidgets(sem_frame).network_test -text "Do not update images" \
        -variable sem(network_test) ]
pack $semwidgets(network_test) -anchor w


# Resolution selector
set semwidgets(resolution) [iwidgets::radiobox $semwidgets(sem_frame).resolution \
     -labeltext "Resolution:" -labelpos nw -command \
     {if {[$semwidgets(resolution) get]!= ""} {set sem(resolution) [$semwidgets(resolution) get]} }]
pack $semwidgets(resolution) -padx $def_pad -pady $def_pad -fill both 

$semwidgets(resolution) add 0 -text "50 x 64"
$semwidgets(resolution) add 1 -text "100 x 128"
$semwidgets(resolution) add 2 -text "200 x 256"
$semwidgets(resolution) add 3 -text "400 x 512"
$semwidgets(resolution) add 4 -text "800 x 1024"
$semwidgets(resolution) add 5 -text "1600 x 2048"
$semwidgets(resolution) add 6 -text "3200 x 4096"

$semwidgets(resolution) select 1
#frame $semwidgets(sem_frame).resolution
#pack $semwidgets(sem_frame).resolution


if { 0==[info exists sem(pixel_integration_time_nsec)] } \
          { set sem(pixel_integration_time_nsec) 0 }
set semwidgets(pixel_integration_time_nsec) \
        [mysem_entry $semwidgets(sem_frame).pixel_integration_time_nsec \
        sem(pixel_integration_time_nsec) \
				  "Pixel Integration (nsec)" integer ]
pack $semwidgets(pixel_integration_time_nsec) -padx $def_pad -pady $def_pad

if { 0==[info exists sem(inter_pixel_delay_time_nsec)] } \
          { set sem(inter_pixel_delay_time_nsec) 0 }
set semwidgets(inter_pixel_delay_time_nsec) \
        [mysem_entry $semwidgets(sem_frame).inter_pixel_delay_time_nsec \
        sem(inter_pixel_delay_time_nsec) \
			      "Inter Pixel Delay (nsec)" integer ]
pack $semwidgets(inter_pixel_delay_time_nsec) -padx $def_pad -pady $def_pad

# align all the labels so it looks nice
iwidgets::Labeledwidget::alignlabels \
    $semwidgets(pixel_integration_time_nsec) \
    $semwidgets(inter_pixel_delay_time_nsec)





