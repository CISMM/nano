#/*===3rdtech===
#  Copyright (c) 2000 by 3rdTech, Inc.
#  All Rights Reserved.
#
#  This file may not be distributed without the permission of 
#  3rdTech, Inc. 
#  ===3rdtech===*/
#!/bin/sh
# the next line restarts using wishx (see man wish) \
	exec wish "$0" ${1+"$@"}
# This is a tcl/tk script to create the user interface to the
# Nanomanipulator system.  
# It sets up:
#      a menu bar, with a "quit" button
#      an Image mode area, where the user controls the properties of 
#             Image mode.
#      a Modify mode area, where the user controls the properties of 
#             Modify mode.
#      a duplicate of the physical button box and dial box currently used.
#      a panel for latency compensation techniques
#
# The variables 'mode_change' and
# 'user_0_mode' are assumed to be linked with the application
# variables mode_change and user_mode[0], both of which are integers.
# 

# Import Itcl and Iwidgets, for the tabnotebook widget and others we
# will use. This import method is a BUG - we shouldn't have to import
# the itcl and itk namespaces, but at least we can use it...
catch { set auto_path [lappend auto_path $env(ITCL_LIBRARY)] }

package require Itcl 
namespace import itcl::*
package require Itk 
namespace import itk::*
package require Iwidgets

# Include the BLT package, which provides vectors, stripchart and
# graph widgets
package require BLT
namespace import blt::*
namespace import -force blt::tile::*

#set up some options for all the widgets in microscape
#option add *background blanchedAlmond startupFile
#option add *highlightBackground blanchedAlmond startupFile
option add *background LemonChiffon1 startupFile
option add *highlightBackground LemonChiffon1 startupFile
option add *menu*background grey75 startupFile

# This needs to be made dependent on how big the font is on the screen.
catch { option add *font {helvetica -15 } startupFile}
catch { option add *Font {helvetica -15 } startupFile}

#global knobs
#global  user_0_mode
#global  maxR maxG maxB minR minG minB ruler_r ruler_g ruler_b
#global  color_flag polish region_changed surface_changed

#
# 3rdTech modifications:
#   There are several parts of the interface we don't expose in the
#   commercial version. This flag turns them off.
#
set thirdtech_ui 0

# We will also watch for "viewer_only" to be set, indicating this
# version can't connect to live SPMs.
if {![info exists viewer_only] } { set viewer_only 0 }
 
# where do I find supporting files?
# If the environment variable NM_TCL_DIR is not set, 
# check for NANO_ROOT. If that is not set, 
# assume files are in the current directory. Allows
# "wish mainwin.tcl" to bring up the interface.
if {[info exists env(NM_TCL_DIR)] } {
    set tcl_script_dir $env(NM_TCL_DIR)
} elseif {[info exists env(NANO_ROOT)]} {
    if {$viewer_only} {
        set tcl_script_dir [file join $env(NANO_ROOT) share tcl_view]
    } else {
        set tcl_script_dir [file join $env(NANO_ROOT) share tcl]
    }
} else {
    set tcl_script_dir .
}
### ks
#if {[string match -nocase "*wish*" [info nameofexecutable]] } {
if {[string match "*wish*" [info nameofexecutable]] } {
    set tcl_script_dir .
}

#appearance variables
    # vertical padding between floatscales - image and modify windows
set fspady 3

# List of the available planes of data in the input grid. Should be set from C.
set inputPlaneNames {none }


# List of the available external filters found in the NM_FILTER_DIR env. 
# variable directory
set filter_names {none }

# List of available images including those not in the input grid
set imageNames {none }

# Hide until set up.
wm withdraw .

#Make the window appear in the upper left corner
wm geometry . +0+0

# Give it a title
wm title . "NMViewer"
if { !$viewer_only } {
wm title . "NanoManipulator"
}
# contains definition of useful widgets for the 
# Tcl_Linkvar library.
source [file join ${tcl_script_dir} Tcl_Linkvar_widgets.tcl]
source [file join ${tcl_script_dir} panel_tools.tcl]

################ Error Handling #############################
### Message dialog for warnings and errors. 
# Designed to be called from C code, as well. 

# The iwidgets message box had a problem when reporting Phantom errors,
# for some reason - Tcl_Eval failed in that situation.
# Switched to tk_messageBox procedure, and things work fine. 

#iwidgets::messagedialog .error_dialog -title "NanoManipulator Error" \
#    -bitmap error -text "Error" -modality application

#.error_dialog hide Help
#.message_dialog buttonconfigure OK -text "Yes"
#.error_dialog hide Cancel 
proc nano_fatal_error {msg } {
    global quit_program_now
    tk_messageBox -message "$msg" -title "NanoManipulator Fatal Error" \
            -type ok -icon error 
    set quit_program_now 1 
#    .error_dialog config -text "$msg" -title "NanoManipulator Fatal Error" \
#            -bitmap error 
#    .error_dialog buttonconfigure OK -command " set quit_program_now 1; .error_dialog deactivate 1"
#    .error_dialog activate
}
proc nano_error {msg } {
    tk_messageBox -message "$msg" -title "NanoManipulator Error" \
            -type ok -icon error 
#    .error_dialog config -text "$msg" -title "NanoManipulator Error" \
#            -bitmap error 
#    .error_dialog buttonconfigure OK -command ".error_dialog deactivate 1"
#    .error_dialog activate
}
proc nano_warning {msg } {
    tk_messageBox -message "$msg" -title "NanoManipulator Warning" \
            -type ok -icon warning 
#    .error_dialog config -text "$msg" -title "NanoManipulator Warning" \
#            -bitmap warning
#    .error_dialog buttonconfigure OK -command ".error_dialog deactivate 1"
#    .error_dialog activate
}

# bgerror is a special name, provided by tcl/tk, called if there
# is a background error in the script. Don't necessarily want
# these to be fatal. 
proc bgerror {msg} {
    nano_error "Internal tcl error: $msg"
}
######################


# frames - overall layout of the screen
frame .menubar -relief raised -bd 4 

# for overall appearance of user interface
frame .main 

# w1 is for stuff that stay on the screen all the time
frame .main.w1 -relief raised -bd 2

# w2 is for stuff that changes depending on what you are doing - 
#   see procedure flip_w2
frame .main.w2 

#set up window
pack .menubar .main -side top -fill x 


pack .main.w1 -side top -fill x
pack .main.w2 -side bottom -fill x


#initialize frame names
set w1 .main.w1
set w2 .main.w2


# menu bar
menu .menu -tearoff 0

# Array to allow controls to be enabled and disabled 
# based on spm_read_mode - reading a file, a streamfile, or a device?
set device_only_controls ""
set stream_only_controls ""
set stream_and_device_only_controls ""


#### FILE menu #############################
set filemenu .menu.file
menu $filemenu -tearoff 0
.menu add cascade -label "File" -menu $filemenu -underline 0

$filemenu add command -label "Open Static File..." -underline 12 \
	-command "open_static_file"
$filemenu add command -label "Open Stream File..." -underline 0 \
	-command "open_stream_file"
if { !$viewer_only } {
$filemenu add command -label "Open SPM Connection..." -underline 9 \
	-command "open_spm_connection"
}
$filemenu add command -label "Close..." -underline 0 -command \
        "set close_microscope 1"
$filemenu add separator
$filemenu add command -label "Save Screen..." -underline 0 -command \
	"save_screenImage"
$filemenu add command -label "Save Plane Data..." -underline 5 -command \
	"save_plane_data"
$filemenu add command -label "Save Modification Data..." -underline 5 \
         -command "save_mod_dialog"
$filemenu add command -label "Export Scene..." -command \
	"export_scene"
$filemenu add separator
$filemenu add command -label "Exit" -underline 1 -command {
    if {[string match "*wish*" [info nameofexecutable]] } {
        # we ran the file using wish, for testing. Kill app.
        exit 0
    } else {
        # we ran using NanoManipulator. Tell main app to exit. 
        set quit_program_now 1
    }
}

lappend stream_and_device_only_controls \
        [list $filemenu entryconfigure "Save Modification Data..."]


#### SETUP menu #############################
set setupmenu .menu.setup
menu $setupmenu -tearoff 0
.menu add cascade -label "Setup" -menu $setupmenu -underline 0

if { !$viewer_only } {
$setupmenu add command -label "Data Sets..." -underline 0 \
	-command "show.data_sets"
}
$setupmenu add command -label "Height Plane..." -underline 0 -command \
	"show.z_mapping"
$setupmenu add command -label "Visualization Settings..." -underline 0 -command \
	"show.visualizations"
$setupmenu add command -label "Color Map..." -underline 0 -command \
	"show.colorscale"
$setupmenu add command -label "Contour Lines..." -underline 8 -command \
	"show.contour_lines"
if { !$thirdtech_ui } {
$setupmenu add command -label "Texture Blend..." -underline 0 -command \
	"show.alphascale"
$setupmenu add command -label "Haptic..." -underline 2 -command \
	"show.haptic"
$setupmenu add command -label "External Filters..." -underline 0 -command \
	"show.external_filters"
}
$setupmenu add command -label "Display Settings..." -underline 8 -command \
		"show.display_settings"

if { !$viewer_only } {
lappend device_only_controls \
        [list $setupmenu entryconfigure "Data Sets..."]
}
#### TIPCONTROL menu #############################
set tipcontrolmenu .menu.tipcontrol
menu $tipcontrolmenu -tearoff 0
.menu add cascade -label "Tipcontrol" -menu $tipcontrolmenu -underline 0

$tipcontrolmenu add command -label "Image Params..." -underline 0 \
	-command "show.image"
$tipcontrolmenu add command -label "Modify Params..." -underline 0 \
	-command "show.modify"
if { !$viewer_only } {
$tipcontrolmenu add command -label "Modify Live Controls..." -underline 7 \
	-command "show.modify_live"
}

$tipcontrolmenu add command -label "Guarded Scan..." -underline 0 -command {
    show.guarded_win
}


#### ANALYSIS menu #############################
set analysismenu .menu.analysis
menu $analysismenu -tearoff 0
.menu add cascade -label "Analysis" -menu $analysismenu -underline 0

$analysismenu add command -label  "Calculate Data Planes..." -underline 0 \
    -command "show.calc_planes"
$analysismenu add command -label  "Rulergrid..." -underline 0  \
    -command "show.rulergrid"
$analysismenu add radiobutton -label "Measure Lines" -underline 0 \
    -variable user_0_mode -value 9 
$analysismenu add command -label "Shape Analysis..." -underline 0 \
    -command "show.shape_analysis"
if { !$thirdtech_ui } {
$analysismenu add command -label "Data Registration..." -underline 0 \
    -command "show.registration"
$analysismenu add command -label "Tip Convolution..." \
    -command "show.tip_conv"
}


#### TOOLS menu #############################
set toolmenu .menu.tool
menu $toolmenu -tearoff 0
.menu add cascade -label "Tools" -menu $toolmenu -underline 1

set show_mode_buttons 1
$toolmenu add checkbutton -label "Show Mode Buttons" -underline 10 \
        -variable show_mode_buttons \
        -command {
    if { $show_mode_buttons } {
        pack $view -side top -fill both
    } else {
        pack forget $view
    }
}


#Only use the Mouse Phantom if the hand-tracker is null
if {(![info exist env(TRACKER)]) \
        || ([string compare [lindex $env(TRACKER) 1] "null"] == 0) \
        || ($viewer_only)} {
    $toolmenu add command -label "Mouse Phantom" -underline 6 \
            -command "show.mouse_phantom_win"
} else {
    # Use a real phantom.
    # must be left-justified so strip_unc program can remove it.
if { !$viewer_only } {
    $toolmenu add command -label "Phantom" -underline 0 \
            -command "show.phantom_win"
}
}


$toolmenu add command -label "Magellan" -underline 0 \
    -command "show.magellan_win"

$toolmenu add command -label "Navigate" -underline 0 \
    -command "show.nav_win"

$toolmenu add command -label "Stripchart" -underline 0 \
	-command "show.stripchart"

$toolmenu add command -label "Replay Control" -underline 0 \
	-command "show.streamfile"

if { !$thirdtech_ui } {
$toolmenu add command -label "Collaboration" -underline 0 \
	-command "show.sharedptr"

$toolmenu add command -label  "Ohmmeter" -underline 0  \
    -command "show.french_ohmmeter"

$toolmenu add command -label "VI Curve" -underline 0 \
    -command "show.vi_win"

$toolmenu add command -label "Latency Adaptation" -underline 0 \
    -command "show.latency"

$toolmenu add command -label "SEM" -underline 1 \
    -command "show.sem_win"

$toolmenu add command -label  "Import Objects..." -underline 0 \
    -command "show.import_objects"
}

lappend stream_only_controls [list $toolmenu entryconfigure "Replay Control"]


#### HELP menu #############################
#set helpmenu .menu.help
#menu $helpmenu -tearoff 0
#.menu add cascade -label "Help" -menu $helpmenu -underline 0
#        $helpmenu add command -label "Help..." -command \
#		{.message_dialog activate}

# This command attaches the menu to the main window
# and allows you to use "alt-KEY" to access the menus. 
. config -menu .menu


# Invoke File.. Exit when destroying window for clean exit. 
wm protocol . WM_DELETE_WINDOW {$filemenu invoke "Exit"}


#
# Get all info about View control box.
#
# variable "view" determines what frame the View control
#     box will appear in.
#     It will stay all the time so it's in w1
set view $w1


source [file join ${tcl_script_dir} view.tcl]


#
# Show the w1 frame
pack $view -side top  -fill both

# Fill in the rest of the main window.
frame $w2.toolbar 
pack $w2.toolbar -side left
label $w2.toolbar.speed -text "More Speed\nLess Detail"
label $w2.toolbar.detail -text "More Detail\nLess Speed"
# Scale is a non-intuitive interface. Let's try a row of radio-buttons.
#scale $w2.toolbar.speed_detail -from 1 -to 6 -resolution 1 -orient horizontal \
#	-variable tesselation_stride
radiobutton $w2.toolbar.speed_detail1 -variable tesselation_stride \
	-value 1
radiobutton $w2.toolbar.speed_detail2 -variable tesselation_stride \
	-value 2
radiobutton $w2.toolbar.speed_detail3 -variable tesselation_stride \
	-value 3
radiobutton $w2.toolbar.speed_detail4 -variable tesselation_stride \
	-value 4
radiobutton $w2.toolbar.speed_detail5 -variable tesselation_stride \
	-value 5

pack $w2.toolbar.detail $w2.toolbar.speed_detail1 $w2.toolbar.speed_detail2 \
	$w2.toolbar.speed_detail3 $w2.toolbar.speed_detail4 \
	$w2.toolbar.speed_detail5 $w2.toolbar.speed \
	-side left 


if { !$viewer_only } {
set spm_scanning 1
# toggle scanning flag. 
button $w2.toolbar.pause_scan -text "Stop\nScan" \
	-command { set spm_scanning [expr !$spm_scanning] }

# keep button label consistent with value of global variable.
# After all, it may be set from C code. 
proc scan_button_label { name el op } {
    global spm_scanning w2
    if {$spm_scanning} {
        $w2.toolbar.pause_scan configure -text "Stop\nScan"
    } else {
        $w2.toolbar.pause_scan configure -text "Start\nScan"
    }
}
trace variable spm_scanning w scan_button_label

button $w2.toolbar.withdraw_tip -text "Withdraw\nTip" \
        -command "set withdraw_tip 1"
pack $w2.toolbar.pause_scan $w2.toolbar.withdraw_tip -side left -padx 5

# These two button should only be available when reading a DEVICE
lappend device_only_controls $w2.toolbar.pause_scan $w2.toolbar.withdraw_tip
}
#File menu commands
source [file join ${tcl_script_dir} filemenu.tcl]

# Get the list of SPM generated by the setup program, but only if it exists. 
set spm_list_filename [file join ${tcl_script_dir} spm_list.tcl]
if { [file exists "$spm_list_filename"] } {
    source "$spm_list_filename"
}

#Setup menu commands
source [file join ${tcl_script_dir} setupmenu.tcl]

# Colormap widget and window.
source [file join ${tcl_script_dir} colormap.tcl]

# tipcontrol menu commands
source [file join ${tcl_script_dir} image.tcl]
source [file join ${tcl_script_dir} modify.tcl]
source [file join ${tcl_script_dir} guard.tcl]

# Analysis menu commands.
source [file join ${tcl_script_dir} analysismenu.tcl]

#
###############
#Tools menu commands - split among several files:

# The stripchart, for graphing the results of modifications
source [file join ${tcl_script_dir} stripchart.tcl]
# Streamfile replay controls. Position depends on image window. 
source [file join ${tcl_script_dir} streamfile.tcl]

#Shared resource controls. Synchronize two copies of nM running
# on different machines. Position depends on streamfile window.
# Always sourced so mutex messages handled without error. 
source [file join ${tcl_script_dir} shared_ptr.tcl]

if { !$thirdtech_ui } {
#source [file join ${tcl_script_dir} shared_ptr.tcl]
#Registration tool. Align two data sets with each other
source [file join ${tcl_script_dir} registration.tcl]
source [file join ${tcl_script_dir} tip_conv.tcl]
source [file join ${tcl_script_dir} import.tcl]
}
# Dialogs accessed from the menus, like  vi_win, 
# and nav_win.
source [file join ${tcl_script_dir} toplevels.tcl]

if { !$thirdtech_ui } {
# SEM control panel
source [file join ${tcl_script_dir} sem.tcl]
# French Ohmmeter control panel
source [file join ${tcl_script_dir} french_ohmmeter.tcl]
}

#----------------
# Try and make widgets active based on what we are doing.
# We only want to be able to operate widgets if they are appropriate
# for the current activity.
# First version based totally on whether we are reading files, a stream,
# or a device.  Ran into race conditions with
# disable_device_widgets_for_commands_suspended when we tried to generalize that
# to non-device contexts.

set READ_DEVICE 0
set READ_FILE 1
set READ_STREAM 2
if {![info exists spm_read_mode] } { set spm_read_mode $READ_FILE }

proc disable_widgets_for_readmode { name el op } {
    global device_only_controls stream_only_controls stream_and_device_only_controls 
    global spm_read_mode toolmenu

    # Note: $ substitution for the patterns won't work because
    # the switch body is in brackets. 
    switch $spm_read_mode {
        0 {
            # READ_DEVICE
            hide.streamfile
            show.image
            show.modify
            #show.modify_live
            foreach widget $device_only_controls {
                if {([llength $widget] > 1) } {
                    # some widget have a special "configure" command 
                    # saved in the list with the widget name, like 
                    # ".a.rb buttonconfigure 0 -state normal"
                    eval $widget -state normal
                } else {
                    $widget configure -state normal
                }
            }
            foreach widget $stream_only_controls {
                if {([llength $widget] > 1) } {
                    # some widget have a special "configure" command 
                    # saved in the list with the widget name, like 
                    # ".a.rb buttonconfigure 0 -state normal"
                    eval $widget -state disabled
                } else {
                    $widget configure -state disabled
                }
            }
            foreach widget $stream_and_device_only_controls {
                if {([llength $widget] > 1) } {
                    # some widget have a special "configure" command 
                    # saved in the list with the widget name, like 
                    # ".a.rb buttonconfigure 0 -state normal"
                    eval $widget -state normal
                } else {
                    $widget configure -state normal
                }
            }
        }
        1 {
            # READ_FILE
            hide.streamfile
            #hide.image
            #hide.modify
            #hide.modify_live
            foreach widget $device_only_controls {
                if {([llength $widget] > 1) } {
                    # some widget have a special "configure" command 
                    # saved in the list with the widget name, like 
                    # ".a.rb buttonconfigure 0 -state normal"
                    eval $widget -state disabled
                } else {
                    $widget configure -state disabled
                }
            }
            foreach widget $stream_only_controls {
                if {([llength $widget] > 1) } {
                    # some widget have a special "configure" command 
                    # saved in the list with the widget name, like 
                    # ".a.rb buttonconfigure 0 -state normal"
                    eval $widget -state disabled
                } else {
                    $widget configure -state disabled
                }
            }
            foreach widget $stream_and_device_only_controls {
                if {([llength $widget] > 1) } {
                    # some widget have a special "configure" command 
                    # saved in the list with the widget name, like 
                    # ".a.rb buttonconfigure 0 -state normal"
                    eval $widget -state disabled
                } else {
                    $widget configure -state disabled
                }
            }
        }
        2 {
            # READ_STREAM
            show.streamfile
            show.image
            show.modify
            hide.modify_live
            foreach widget $device_only_controls {
                if { ([llength $widget] > 1) } {
                    # some widget have a special "configure" command 
                    # saved in the list with the widget name, like 
                    # ".a.rb buttonconfigure 0 -state normal"
                    eval $widget -state disabled
                } else {
                    $widget configure -state disabled
                }
            }
            foreach widget $stream_only_controls {
                if {([llength $widget] > 1) } {
                    # some widget have a special "configure" command 
                    # saved in the list with the widget name, like 
                    # ".a.rb buttonconfigure 0 -state normal"
                    eval $widget -state normal
                } else {
                    $widget configure -state normal
                }
            }
            foreach widget $stream_and_device_only_controls {
                if {([llength $widget] > 1) } {
                    # some widget have a special "configure" command 
                    # saved in the list with the widget name, like 
                    # ".a.rb buttonconfigure 0 -state normal"
                    eval $widget -state normal
                } else {
                    $widget configure -state normal
                }
            }
        }
        default {
            nano_error "Internal tcl: unknown read mode"
        }
    }
}
trace variable spm_read_mode w disable_widgets_for_readmode

# Helps with Thermo Image Analysis mode. When in this mode, most of 
# the widgets/dialogs that Nano needs to control the SPM aren't available
# So we avoid issuing any commands to Thermo, by disabling all device 
# controls. 
if {![info exists spm_commands_suspended] } { set spm_commands_suspended 0 }
if {![info exists readmode_device_commands_suspended] } \
                    { set readmode_device_commands_suspended 0 }

proc disable_widgets_for_commands_suspended { name el op } {
    global device_only_controls  
    global spm_commands_suspended
    global collab_commands_suspended
    global readmode_device_commands_suspended
    global spm_read_mode READ_DEVICE

    # Don't do anything if we aren't talking to a device
    if { $spm_read_mode != $READ_DEVICE } { return; }

    set commands_suspended 0
    if $spm_commands_suspended {set commands_suspended 1}
    if $collab_commands_suspended {set commands_suspended 1}
    if $readmode_device_commands_suspended {set commands_suspended 1}

    # Note: $ substitution for the patterns won't work because
    # the switch body is in brackets. 
    switch $commands_suspended {
        0 {
            # Commands aren't suspended, enable controls
            foreach widget $device_only_controls {
                if {([llength $widget] > 1) } {
                    # some widget have a special "configure" command 
                    # saved in the list with the widget name, like 
                    # ".a.rb buttonconfigure 0 -state normal"
                    eval $widget -state normal
                } else {
                    $widget configure -state normal
                }
            }
        }
        1 {
            # Commands are suspended, disable controls
            foreach widget $device_only_controls {
                if { ([llength $widget] > 1) } {
                    # some widget have a special "configure" command 
                    # saved in the list with the widget name, like 
                    # ".a.rb buttonconfigure 0 -state normal"
                    eval $widget -state disabled
                } else {
                    $widget configure -state disabled
                }
            }
        }
    }
}
trace variable spm_commands_suspended w disable_widgets_for_commands_suspended

trace variable readmode_device_commands_suspended w \
                    disable_widgets_for_commands_suspended


#----------------
# Setup window positions and geometries to be convenient and pleasant!
# We do this at the end so we can find out the requested size of 
# all the widgets we created. 

# Pulling it out of individual files also makes more obvious 
# the relationship between window positions.
after idle {

    # Find out about the main window geometry.
    update idletasks
    # the root window, ".", seems to need special handling.
    #set width [winfo reqwidth .] Doesn't include borders.
    #set height [winfo reqheight .] Doesn't include the menu bar and title bar!

    scan [wm geometry .] %dx%d+%d+%d width height main_xpos main_ypos
    # wm rootx seems to tell us how big the border of the window is.
    set winborder [expr [winfo rootx .] - $main_xpos]
    set main_width [expr $width + 2* $winborder ]
    set main_height [expr $height + ([winfo rooty .] - $main_ypos) + \
	   $winborder ]
#    puts " mainwin $width $height $main_xpos $main_ypos [wm geometry .]"

    # The stripchart window.
    #Make the window appear on the top next to the main window
    #wm geometry $graphmod(sc) +[expr $main_xpos + $width + 2*$winborder]+$main_ypos
    # Nah. Top right corner.
    wm geometry $graphmod(sc) -0+0

    # The colormap window - about the same as the stripchart window.
#    wm geometry $nmInfo(colorscale) +[expr $main_xpos + $width \
#            + 2*$winborder + 10]+[expr $main_ypos + 10]
    wm geometry $nmInfo(colorscale) -0+[expr $main_ypos + 10]
    # The image window
    # Make the window appear on the left edge below the main window
    
    # Make the left strip all the same width - same as the image windows.
    set left_strip_width  [winfo reqwidth .image] 

    # Keep track of where the next window should appear
    set next_left_pos [expr $main_ypos +$main_height]

    wm geometry .image +${main_xpos}+$next_left_pos

    
    # Next, the modify window
    # Make the window appear on the left edge below the image window
    update idletasks

    # find out how big the title bar is. 
    scan [wm geometry .image] %dx%d+%d+%d width height xpos ypos
    #puts "image geometry $width $height $xpos $ypos"
    set titleborder [expr [winfo rooty .image] - $ypos]
    #puts "image root pos [winfo rootx .image] [winfo rooty .image]"
    # check to make sure we aren't off the bottom of the screen. 
    set next_left_pos [expr $next_left_pos  +[winfo reqheight .image] \
            + $winborder + $titleborder]
    # wm maxsize . gives us the size of the available space. 
    # Make sure the window doesn't appear off the bottom of the screen.
    set req_height [winfo reqheight .modify] 
    if { $next_left_pos > [lindex [wm maxsize .] 1] } {
	set next_left_pos [expr $main_ypos +$main_height]
    }

    wm geometry .modify +${main_xpos}+$next_left_pos

    # Next, the stream file window
    # Make the window appear on the left edge below the image window
    update idletasks

    # check to make sure we aren't off the bottom of the screen. 
    set next_left_pos [expr $next_left_pos  +[winfo reqheight .modify] \
            + $winborder + $titleborder]
    # wm maxsize . gives us the size of the available space. 
    # Make sure the window doesn't appear off the bottom of the screen.
    set req_height [winfo reqheight .streamfile] 
    if { $next_left_pos > [lindex [wm maxsize .] 1] } {
	set next_left_pos [expr $main_ypos +$main_height]
    }

    wm geometry .streamfile ${left_strip_width}x${req_height}+${main_xpos}+$next_left_pos

if { !$thirdtech_ui } {
    # Finally the shared_ptr window, for collaboration.
    #Make the window appear on the left edge below the streamfile window
    update idletasks
    # check to make sure we aren't off the bottom of the screen. 
    set next_left_pos [expr $next_left_pos  +[winfo reqheight .streamfile] \
            + $winborder + $titleborder]
    # wm maxsize . gives us the size of the available space. 
    # Make sure the window doesn't appear off the bottom of the screen.
    set req_height [winfo reqheight .sharedptr] 
    if { [expr $next_left_pos + $req_height] > [lindex [wm maxsize .] 1] } {
	set next_left_pos [expr $main_ypos +$main_height]
    }

#  Can't specify the window size because we want it to change when we pack
# the finegrained coupling controls.
    wm geometry .sharedptr +${main_xpos}+$next_left_pos
}

    # Make the modify live window appear at the same position as the image
    wm geometry .modify_live +${main_xpos}+[expr $main_ypos +$main_height + 20]

}


#set dataset1 $w2.dataset1
#set dataset2 $w2.dataset2
#set dataset3 $w2.dataset3
#source [file join ${tcl_script_dir} dataset.tcl]

#source [file join ${tcl_script_dir} modfile.tcl]

if { !$thirdtech_ui } {
  source [file join ${tcl_script_dir} latency.tcl]
}

# Prevent changes in size of the main window by user.
wm resizable . 0 0 
# puts the focus on the main window, instead of any other windows 
# which have been created. 
after idle {
    wm deiconify .
}
# Just for fun - total number of commands invoked so far
#puts "Command count: [info cmdcount]"

# Put this at the end to make sure things get disabled?

# Helps with Collaboration.  If we don't have the mutex, we should disable
# all the widgets/dialogs that control the microscope.

# Default state:  assume somebody else holds the mutex
#   The C++ code sends a request every time it connects to a
# new microscope to see if we can grab the mutex for this connection.

if {![info exists collab_commands_suspended] } \
                 { set collab_commands_suspended 1; \
                   disable_widgets_for_commands_suspended 0 0 0 }

trace variable collab_commands_suspended w disable_widgets_for_commands_suspended

