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
option add *menu*background grey80 startupFile

# This needs to be made dependent on how big the font is on the screen.
catch { option add *font {helvetica -15 } startupFile}
catch { option add *Font {helvetica -15 } startupFile}

#global knobs
#global  user_0_mode
#global  maxR maxG maxB minR minG minB ruler_r ruler_g ruler_b
#global  color_flag polish region_changed term_input surface_changed

# where do I find supporting files?
# If the environment variable NM_TCL_DIR is not set, 
# assume files are in the current directory. Allows
# "wish mainwin.tcl" to bring up the interface.
if {[catch {set tcl_script_dir $env(NM_TCL_DIR) }] } {
    set tcl_script_dir .
}

#appearance variables
    # vertical padding between floatscales - image and modify windows
set fspady 3

# List of the available planes of data in the input grid. Should be set from C.
set inputPlaneNames {none }

# List of available images including those not in the input grid
set imageNames {none }

#intialize stuff
#set color_flag "Maximum"
set term_input ""


#Make the window appear in the upper left corner
wm geometry . +0+0

# Give it a title
wm title . "NanoManipulator"

# Make the main graphics window appear in the lower right corner
# XXX assume a 1280x1024 screen with a 1024x768 vlib window
# XXX doesn't seem to work properly. Not communicated to vlib. 
#set env(V_SCREEN_OFFSET) "252 224"

# contains definition of useful widgets for the 
# Tcl_Linkvar library.
source [file join ${tcl_script_dir} Tcl_Linkvar_widgets.tcl]
source [file join ${tcl_script_dir} panel_tools.tcl]

# frames - overall layout of the screen
frame .menubar -relief raised -bd 4 

# for overall appearance of user interface
frame .main 

# w1 is for stuff that stay on the screen all the time
frame .main.w1 
# w2 is for stuff that changes depending on what you are doing - 
#   see procedure flip_w2
frame .main.w2 

#set up window
pack .menubar .main -side top -fill x 

pack .main.w1 .main.w2 -side top -fill x

#initialize frame names
set w1 .main.w1
set w2 .main.w2

# menu bar
menu .menu -tearoff 0


#### FILE menu #############################
set filemenu .menu.file
menu $filemenu -tearoff 0
.menu add cascade -label "File" -menu $filemenu -underline 0

$filemenu add command -label "Open static file..." -underline 0 \
	-command "open_static_file"
#        $filemenu add command -label "Close..." -underline 0 -command \
#		{.message_dialog activate}
#        $filemenu add separator
$filemenu add command -label "Save Screen..." -command \
	"save_screenImage"
$filemenu add command -label "Save Plane Data..." -command \
	"save_plane_data"
$filemenu add command -label "Save Modification Data..." -command \
	"save_mod_dialog"
$filemenu add command -label "Export Scene..." -command \
	"export_scene"
$filemenu add separator
$filemenu add command -label "Exit" -underline 1 -command \
	"set term_input q"

#### SETUP menu #############################
set setupmenu .menu.setup
menu $setupmenu -tearoff 0
.menu add cascade -label "Setup" -menu $setupmenu -underline 0

$setupmenu add command -label "Data Sets..." \
	-command "show.data_sets"
$setupmenu add command -label "Height Plane..." -command \
	"show.z_mapping"
$setupmenu add command -label "Color Map..." -command \
	"show.colorscale"
$setupmenu add command -label "Contour Lines..." -command \
	"show.contour_lines"
$setupmenu add command -label "Texture Blend..." -command \
	"show.alphascale"
$setupmenu add command -label "Haptic..." -command \
	"show.haptic"
$setupmenu add command -label "Preferences..." -command \
		"show.preferences"
#	{.message_dialog activate}



#### TIPCONTROL menu #############################
set tipcontrolmenu .menu.tipcontrol
menu $tipcontrolmenu -tearoff 0
.menu add cascade -label "Tipcontrol" -menu $tipcontrolmenu -underline 0

$tipcontrolmenu add command -label "Image Params..." \
	-command "show.image"
$tipcontrolmenu add command -label "Modify Params..." \
	-command "show.modify"
$tipcontrolmenu add command -label "Modify Live Controls..." \
	-command "show.modify_live"


#### ANALYSIS menu #############################
set analysismenu .menu.analysis
menu $analysismenu -tearoff 0
.menu add cascade -label "Analysis" -menu $analysismenu -underline 0

$analysismenu add command -label  "Rulergrid..."  \
    -command "show.rulergrid"
$analysismenu add radiobutton -label "Measure Lines" \
    -variable user_0_mode -value 9 
$analysismenu add command -label "Data Registration..." \
    -command "show.registration"
#        $analysismenu add command -label  "Import Objects..." -command \
#		{.message_dialog activate}
#        $analysismenu add command -label  "Filter a plane..." -command \
#		{.message_dialog activate}
#        $analysismenu add command -label  "Sum Plane..." -command \
#		{.message_dialog activate}



#### TOOLS menu #############################
set toolmenu .menu.tool
menu $toolmenu -tearoff 0
.menu add cascade -label "Tools" -menu $toolmenu -underline 1
        
$toolmenu add command -label "Phantom" \
    -command "show.phantom_win"

$toolmenu add command -label "Navigate" \
    -command "show.nav_win"

$toolmenu add command -label "Stripchart" \
	-command "show.stripchart"

$toolmenu add command -label "Replay Control" \
	-command "show.streamfile"

$toolmenu add command -label "Collaboration" \
	-command "show.sharedptr"

$toolmenu add command -label  "Ohmmeter"  \
    -command "show.french_ohmmeter"

$toolmenu add command -label "VI Curve" \
    -command "show.vi_win"

$toolmenu add command -label "Latency Adaptation" \
    -command "show.latency"


$toolmenu add command -label "SEM" \
    -command "show.sem_win"


#### HELP menu #############################
#set helpmenu .menu.help
#menu $helpmenu -tearoff 0
#.menu add cascade -label "Help" -menu $helpmenu -underline 0
#        $helpmenu add command -label "Help..." -command \
#		{.message_dialog activate}

# This command attaches the menu to the main window
# and allows you to use "alt-KEY" to access the menus. 
. config -menu .menu


# Uncomment when done debugging...
#wm protocol . WM_DELETE_WINDOW {$filemenu invoke "Exit"}


#
# Get all info about View control box.
#
# variable "view" determines what frame the View control
#     box will appear in.
#     It will stay all the time so it's in w1
set view $w1.view

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

# Anchor south to make the labels sit on the bottom with the slider.
pack $w2.toolbar.detail $w2.toolbar.speed_detail1 $w2.toolbar.speed_detail2 \
	$w2.toolbar.speed_detail3 $w2.toolbar.speed_detail4 \
	$w2.toolbar.speed_detail5 $w2.toolbar.speed \
	-side left 

radiobutton $w2.toolbar.demotouch -text "Touch Surface" \
	-variable user_0_mode -value 11 
pack $w2.toolbar.demotouch -side left -padx 5

#File menu commands
source [file join ${tcl_script_dir} filemenu.tcl]

#Setup menu commands
source [file join ${tcl_script_dir} setupmenu.tcl]

# tipcontrol menu commands
source [file join ${tcl_script_dir} image.tcl]
source [file join ${tcl_script_dir} modify.tcl]

# Analysis menu commands.
source [file join ${tcl_script_dir} analysismenu.tcl]

#
###############
# The toolbar window fits along the left side of the screen, next to
# the graphics window.
# source first so we can place other windows (streamfile) around it.
source [file join ${tcl_script_dir} toolbar.tcl]

#Tools menu commands - split among several files:

# The stripchart, for graphing the results of modifications
source [file join ${tcl_script_dir} stripchart.tcl]
# Streamfile replay controls. Position depends on toolbar window. 
source [file join ${tcl_script_dir} streamfile.tcl]
#Shared resource controls. Synchronize two copies of nM running
# on different machines. Position depends on streamfile window.
source [file join ${tcl_script_dir} shared_ptr.tcl]
#Registration tool. Align two data sets with each other
source [file join ${tcl_script_dir} registration.tcl]
# Dialogs accessed from the menus, like  vi_win, 
# and nav_win.
source [file join ${tcl_script_dir} toplevels.tcl]
# SEM control panel
source [file join ${tcl_script_dir} sem.tcl]
# French Ohmmeter control panel
source [file join ${tcl_script_dir} french_ohmmeter.tcl]

### Message dialog for any simple messages generated by menu items
# The only one so far is "not implemented"
iwidgets::messagedialog .message_dialog -title "Not implemented" \
    -bitmap info -text "This menu item is not yet implemented."

.message_dialog hide Help
#.message_dialog buttonconfigure OK -text "Yes"
.message_dialog hide Cancel 

#----------------
# Setup window positions and geometries to be convenient and pleasant!
# We do this at the end so we can find out the requested size of 
# all the widgets we created. 

# Pulling it out of individual files also makes more obvious 
# the relationship between window positions.
after idle {

    # First, the toolbar window
    #Make the window appear on the left edge below the main window
    update idletasks
    # the root window, ".", seems to need special handling.
    #set width [winfo reqwidth .] Doesn't include borders.
    #set height [winfo reqheight .] Doesn't include the menu bar and title bar!

    scan [wm geometry .] %dx%d+%d+%d width height main_xpos main_ypos
    # wm rootx seems to tell us how big the border of the window is.
    set main_width [expr $width + 2* ([winfo rootx .] - $main_xpos) ]
    set main_height [expr $height + ([winfo rooty .] - $main_ypos) + \
	   ([winfo rootx .] - $main_xpos) ]
    puts " mainwin $width $height $main_xpos $main_ypos [wm geometry .]"

    set toolbar_req_width  [winfo reqwidth .toolbar] 
    set toolbar_req_height [winfo reqheight .toolbar] 

    wm geometry .toolbar +${main_xpos}+[expr $main_ypos +$main_height]

    # The stripchart window.
    #Make the window appear on the top next to the main window
    wm geometry $graphmod(sc) +[expr $main_xpos + $width ]+$main_ypos

    # Next, the stream file window
    # Make the window appear on the left edge below the toolbar window
    update idletasks
    set width [winfo reqwidth .toolbar]
    set height [winfo reqheight .toolbar]
    set xpos [winfo rootx .toolbar]
    set ypos [winfo rooty .toolbar]
    puts " toolbar $width $height $xpos $ypos [wm geometry .toolbar]"
    # check to make sure we aren't off the bottom of the screen. 
    set my_ypos [expr $ypos +$height]
    # wm maxsize . gives us the size of the available space. 
    # Make sure the window doesn't appear off the bottom of the screen.
    set req_height [winfo reqheight .streamfile] 
    if { $my_ypos > [lindex [wm maxsize .] 1] } {
	set my_ypos [expr [lindex [wm maxsize .] 1] - $req_height - 50]
    }

    wm geometry .streamfile ${toolbar_req_width}x${req_height}+${main_xpos}+$my_ypos

    # Finally the shared_ptr window, for collaboration.
    #Make the window appear on the left edge below the streamfile window
    update idletasks
    set width [winfo reqwidth .streamfile]
    set height [winfo reqheight .streamfile]
    set xpos [winfo rootx .streamfile]
    set ypos [winfo rooty .streamfile]
        puts " streamfile $width $height $xpos $ypos [wm geometry .streamfile]"
    # .streamfile is the window we want to relate our position to. 
    #scan [wm geometry .streamfile] %dx%d+%d+%d width height xpos ypos
    set my_ypos [expr $ypos +$height]
    # wm maxsize . gives us the size of the available space. 
    # Make sure the window doesn't appear off the bottom of the screen.
    set req_height [winfo reqheight .sharedptr] 
    if { $my_ypos > [lindex [wm maxsize .] 1] } {
	set my_ypos [expr [lindex [wm maxsize .] 1] - $req_height]
    }

#  Can't specify the window size because we want it to change when we pack
# the finegrained coupling controls.
#    wm geometry .sharedptr ${toolbar_req_width}x${req_height}+${main_xpos}+$my_ypos
    wm geometry .sharedptr +${main_xpos}+$my_ypos


    # Make the modify live window appear at the same position as the toolbar
    wm geometry .modify_live +${main_xpos}+[expr $main_ypos +$main_height + 20]

}


# Hack to allow microscape to run!
frame .sliders

set bc tan
set fc tan

set dataset1 $w2.dataset1
set dataset2 $w2.dataset2
set dataset3 $w2.dataset3
source [file join ${tcl_script_dir} dataset.tcl]

#source [file join ${tcl_script_dir} modfile.tcl]

source [file join ${tcl_script_dir} latency.tcl]

# puts the focus on the main window, instead of any other windows 
# which have been created. 
wm deiconify .

#Debugging printouts:
#set collab_machine_name "a"
#button $w2.toolbar.testprint -text "printstuff" -command {
#puts "TCL: z dataset: $z_comes_from"
#puts "TCL: replay rate: $stream_replay_rate"
#puts "TCL: collab machine: $collab_machine_name"
#}

#pack $w2.toolbar.testprint
#$w2.toolbar.testprint invoke
