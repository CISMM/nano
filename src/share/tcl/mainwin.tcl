#!/bin/sh
# the next line restarts using wishx (see man wish) \
	exec wish "$0" "$@"
#	exec ${PXFL_TOOLS_HOME:?}/bin/wishx "$0" "$@"
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
option add *background blanchedAlmond startupFile
option add *highlightBackground blanchedAlmond startupFile

global knobs
global  user_0_mode
global  maxR maxG maxB minR minG minB ruler_r ruler_g ruler_b
global  color_flag polish region_changed term_input surface_changed

# where do I find supporting files?
set tcl_script_dir $env(NM_TCL_DIR)

#appearance variables
    # fore color
set fc tan
    # back color
set bc tan
    # vertical padding between floatscales - image and modify windows
set fspady 5

#intialize stuff
set color_flag "Maximum"
set term_input ""

# contains definition of float_scale and minmax_scale
source ${tcl_script_dir}russ_widgets.tcl
source ${tcl_script_dir}panel_tools.tcl

# frames - overall layout of the screen
. configure 
frame .menubar -relief raised -bd 4 

# for overall appearance of user interface
frame .main -bg $bc

# w1 is for stuff that stay on the screen all the time
frame .main.w1 -bg $bc
# w2 is for stuff that changes depending on what you are doing - 
#   see procedure flip_w2
frame .main.w2 -bg $bc

#set up window
pack .menubar .main -side top -fill x 

pack .main.w1 .main.w2 -side top -fill x

#initialize frame names
set w1 .main.w1
set w2 .main.w2


#
# Get all info about View control box.
#
# variable "view" determines what frame the View control
#     box will appear in.
#     It will stay all the time so it's in w1
set view $w1.view

source ${tcl_script_dir}view.tcl

#
# Show the w1 frame
pack $view -side top  -fill both
# -padx 1m -pady 1m

#
# Get all info about Image mode control box.
#
# variable "image" determines what frame the Image mode control
#     box will appear in.
set image $w2.image
source ${tcl_script_dir}image.tcl

#
# Get all info about Modify mode control box.
#
# variable "modify" determines what frame Modify mode control box
#     will appear in.
set modify $w2.modify
source ${tcl_script_dir}modify.tcl

#
# Get all info about Button box simulator.
#
# variable "buttons" determines what frame the Button box simulator
#     will appear in.
set buttons $w2.buttons
source ${tcl_script_dir}buttons.tcl

#
# Get all info about Knob box simulator.
#
# variable "knbs" determines what frame the Knob box simulator
#     will appear in. NOTE: "knobs" is used elsewhere!!!

set knbs $w2.knobs
source ${tcl_script_dir}knobs.tcl

#
# Get all info about Dataset control box.
#
# variables "dataset1", 2 and 3 determine what frame the Dataset control box
#     will appear in.
# We split it up so it will fit on an 800x600 laptop screen.

set dataset1 $w2.dataset1
set dataset2 $w2.dataset2
set dataset3 $w2.dataset3
source ${tcl_script_dir}dataset.tcl

#
# Get all info about Latency compensation control.
#

set latency $w2.latency
source ${tcl_script_dir}latency.tcl

set scanline $w2.scanline
source ${tcl_script_dir}scanline.tcl


# make a other useful windows, which can be shown later when needed.
source ${tcl_script_dir}toplevels.tcl

# make a stripchart window, which can be shown later when needed.
source ${tcl_script_dir}stripchart.tcl

#make an import objects window, which can be shown later when needed.
source ${tcl_script_dir}import_tubes.tcl

# make a modfile window, which can be shown later when needed.
source ${tcl_script_dir}modfile.tcl

# make a streamfile control window, which can be shown later when needed
source ${tcl_script_dir}streamfile.tcl

# make a data registration control panel, which can be shown later when needed
source ${tcl_script_dir}registration.tcl

# make a window for widgets associated with sharing a pointer
source ${tcl_script_dir}shared_ptr.tcl

#
# Show the initial setup in the w2 frame
#
#pack $buttons $knobs -side left -padx 3m -pady 3m -fill both
pack $dataset1 -side top -padx 1m -pady 3 -fill both

# menu bar
menubutton .menubar.top -text "Menus" -menu .menubar.top.menu 
set topmenu .menubar.top.menu
menu $topmenu 
     $topmenu add cascade -label "File" -menu $topmenu.file 
     $topmenu add cascade -label "Modes" -menu $topmenu.modes
     $topmenu add cascade -label "STM" -menu $topmenu.stm 
     $topmenu add cascade -label "Display" -menu $topmenu.display
     $topmenu add cascade -label "Utility" -menu $topmenu.utility
     $topmenu add cascade -label "Playback" -menu $topmenu.playback
menu $topmenu.file 
        $topmenu.file add command -label "Take snapshot" -command \
		"set term_input s"
        $topmenu.file add command -label "Write UNCA" -command \
		"set term_input u"
        $topmenu.file add command -label "Write ascii" -command \
		"set term_input a"
        $topmenu.file add command -label "Info dump" -command \
		"set term_input I"
        $topmenu.file add command -label "Store screen" -command  \
		"set term_input \"\$\" " 
#        $topmenu.file add command -label "NNN by NNN snapshot" \
#		-command not_implemented
        $topmenu.file add command -label "Quit" -command \
		"set term_input q"
        $topmenu.file add command -label "Quit without saving" \
		-command "set term_input Q" 
menu $topmenu.modes 
        $topmenu.modes add radiobutton -label "Grab" -variable \
		user_0_mode -value 1
#        $topmenu.modes add radiobutton -label "Pulse" -variable \
#		user_0_mode -value 5 
        $topmenu.modes add radiobutton -label "Sweep" -variable \
		user_0_mode -value 8
#        $topmenu.modes add radiobutton -label "Comb" -variable \
#		user_0_mode -value 14
        $topmenu.modes add radiobutton -label "Line" -variable \
		user_0_mode -value 7 
        $topmenu.modes add radiobutton -label "Measure" -variable \
		user_0_mode -value 9
        $topmenu.modes add radiobutton -label "Scale UP" -variable \
		user_0_mode -value 2 
        $topmenu.modes add radiobutton -label "Scale DOWN" -variable \
		user_0_mode -value 3 
#        $topmenu.modes add radiobutton -label "Feel with blunt tip (LIVE)" \
#                 -variable user_0_mode -value 13
        $topmenu.modes add radiobutton -label "Feel with sharp tip (LIVE)" \
                 -variable user_0_mode -value 12
        $topmenu.modes add radiobutton -label "Feel canned" -variable \
		user_0_mode -value 11 
        $topmenu.modes add command -label "Toggle data" -command \
		"set term_input \"\x23\" "
        $topmenu.modes add radiobutton -label "Fly" -variable \
		user_0_mode -value 0 
        $topmenu.modes add radiobutton -label "Select" -variable \
		user_0_mode -value 4 
        $topmenu.modes add radiobutton -label "Height of measure grid" \
                 -variable user_0_mode -value 6
menu $topmenu.stm 
        $topmenu.stm add command -label "Select all" -command \
		"set term_input A"
        $topmenu.stm add command -label "Scan in x" -command \
		"set term_input x"
        $topmenu.stm add command -label "Scan in y" -command \
		"set term_input y"
        $topmenu.stm add command -label "Boustrophedonic scan" \
		-command "set term_input B"
        $topmenu.stm add command -label "Raster scan" -command \
		"set term_input R"
        $topmenu.stm add command -label "Show + raster" -command \
		"set term_input \"\+\" "
        $topmenu.stm add command -label "Show - raster" -command \
		"set term_input \"\-\" "
        $topmenu.stm add command -label "Toggle piezo relaxation" \
		-command "set term_input \"\=\" "
        $topmenu.stm add command -label "Toggle drift" -command \
		"set term_input \"\~\" "
        $topmenu.stm add command -label "Toggle Ohmmeter" -command \
		"set term_input O"
#        $topmenu.stm add command -label "Set imaging setpoint" \
#		-command not_implemented
menu $topmenu.display 
        $topmenu.display add command -label "Visible pulses" \
		-command "set term_input V"
        $topmenu.display add command -label "Invisible pulses" \
		-command "set term_input v"
        $topmenu.display add command -label "Clear pulses" \
		-command "set term_input C"
        $topmenu.display add command -label "Visible measuring grid" \
		-command  "set term_input m"
        $topmenu.display add command -label "Visible control panels" \
		-command "set term_input Z"
        $topmenu.display add command -label "Invisible control panels" \
		-command "set term_input z"
        $topmenu.display add command -label "Lock head position" \
		-command "set term_input L"
        $topmenu.display add command -label "Unlock head position" \
		-command "set term_input l"
        $topmenu.display add command -label "Refresh display from grid" \
                -command "set term_input \"\.\" "
        $topmenu.display add radiobutton -label "Position light" \
		-variable user_0_mode -value 10
        $topmenu.display add command -label "Graph mode" -command \
		"set term_input w"
        $topmenu.display add command -label "Center" -command \
		"set term_input \"\^\" "
menu $topmenu.utility 
        $topmenu.utility add command -label "Close and re-open ARM" \
		-command "set term_input X" 
menu $topmenu.playback 
        $topmenu.playback add command -label "Rate of playback 0-9" \
		-command not_implemented 
        $topmenu.playback add command -label "Where in stream file" \
		-command "set term_input ?" 
        $topmenu.playback add command -label "Advance 1 frame" \
		-command "set term_input \"\>\" " 
        $topmenu.playback add command -label "Back 1 frame" \
		-command "set term_input \"\<\" "
#        $topmenu.playback add command -label "Goto nth byte" \
#		-command not_implemented 

#
# QUIT button
#
button .quit_button -text "QUIT nM" -command "set term_input q" -bg red 
#if the window is destroyed by some other means, still invoke quit 
wm protocol . WM_DELETE_WINDOW {.quit_button invoke}
#
#setup menubar
#
# DISABLE menu button, because no one ever uses it
# re-enable only if someone complains.
#pack .menubar.top -side left

pack .quit_button -in .menubar -side right -anchor e -ipadx 3m

set disp_frame 0
trace variable disp_frame w flip_w2

#
# setup UI changing button bar
#
radiobutton .menubar.data1 -text "Data Mapping" -variable disp_frame -value 0
radiobutton .menubar.data2 -text "Data Sets" -variable disp_frame -value 1
radiobutton .menubar.image -text "Image" -variable disp_frame -value 2
radiobutton .menubar.modify -text "Modify" -variable disp_frame -value 3
radiobutton .menubar.scanline -text "Line Scan" -variable disp_frame -value 4
radiobutton .menubar.buttons -text "Button & Knob Box" -variable disp_frame -value 5
radiobutton .menubar.data3 -text "Haptic" -variable disp_frame -value 6
radiobutton .menubar.latency -text "Latency" -variable disp_frame -value 7

pack .menubar.data1 .menubar.data2 .menubar.image .menubar.modify \
    .menubar.scanline .menubar.buttons .menubar.data3 .menubar.latency \
    -side left -padx 10 -ipadx 3

set dm1_list "$dataset1"
set dm2_list "$dataset2"
set dm3_list "$dataset3"
set im_list "$image"
set md_list "$modify"
set bk_list "$buttons $knbs"
set lat_list "$latency"
set sl_list "$scanline"

# flips frames inside $w2
proc flip_w2 {disp_frame element op} {
    global w2
    global dm1_list dm2_list dm3_list
    global im_list
    global md_list
    global bk_list
    global lat_list
    global sl_list

    upvar $disp_frame k

    if {$k==0} {
        # selected data sets 1
	set plist [lrange [pack slaves $w2] 0 end] 
	foreach widg $plist {pack forget $widg}
	foreach widg $dm1_list {pack $widg -side top -padx 1m -pady 3 -fill both}
    } elseif {$k==1} {
        # selected data sets 2
	set plist [lrange [pack slaves $w2] 0 end] 
	foreach widg $plist {pack forget $widg}
	foreach widg $dm2_list {pack $widg -side top -padx 1m -pady 3 -fill both}
    } elseif {$k==2} {
	# selected image
	set plist [lrange [pack slaves $w2] 0 end] 
	foreach widg $plist {pack forget $widg}
	foreach widg $im_list {pack $widg -side top -padx 1m -pady 3 -fill both}
    } elseif {$k==3} {
	# selected modify
	set plist [lrange [pack slaves $w2] 0 end] 
	foreach widg $plist {pack forget $widg}
	foreach widg $md_list {pack $widg -side top -padx 1m -pady 3 -fill both}
    } elseif {$k==4} {
        # selected scanline
        set plist [lrange [pack slaves $w2] 0 end]
        foreach widg $plist {pack forget $widg}
        foreach widg $sl_list {pack $widg -side left -padx 1m -pady 3 -fill both}
    } elseif {$k==5} {
	# selected buttons and knobs
	set plist [lrange [pack slaves $w2] 0 end] 
	foreach widg $plist {pack forget $widg}
	foreach widg $bk_list {pack $widg -side left -padx 1m -pady 3 -fill both}
    } elseif {$k==6} {
        # selected haptic
        set plist [lrange [pack slaves $w2] 0 end]
        foreach widg $plist {pack forget $widg}
        foreach widg $dm3_list {pack $widg -side left -padx 1m -pady 3 -fill both -expand yes }
    } elseif {$k==7} {
        # selected latency
        set plist [lrange [pack slaves $w2] 0 end]
        foreach widg $plist {pack forget $widg}
        foreach widg $lat_list {pack $widg -side left -padx 1m -pady 3 -fill both -expand yes }
    }
}

# for any item not implemented.
proc not_implemented {} {
    toplevel .msg -bg grey50
    label   .msg.line1 -text "Function not accessible" -bg grey50
    label   .msg.line2 -text "from this control panel." -bg grey50
    button .msg.ok -bg yellow -text "OK" -command "destroy .msg"
    pack .msg.line1 .msg.line2 -side top -ipadx 3m
    pack .msg.ok -side top -ipadx 3m -ipady 3m -pady 3m
}

