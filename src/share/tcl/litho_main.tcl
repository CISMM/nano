#!/bin/sh
# the next line restarts using wishx (see man wish) \
	exec wish "$0" ${1+"$@"}
# This is a tcl/tk script to create the user interface to the
# ebeam lithography system.  
# It sets up:
#      a menu bar
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

#set up some options for all the widgets
option add *background LemonChiffon1 startupFile
option add *highlightBackground LemonChiffon1 startupFile
option add *menu*background grey75 startupFile

# This needs to be made dependent on how big the font is on the screen.
catch { option add *font {helvetica -15 } startupFile}
catch { option add *Font {helvetica -15 } startupFile}

# where do I find supporting files?
# If the environment variable NM_TCL_DIR is not set, 
# check for NANO_ROOT. If that is not set, 
# assume files are in the current directory. Allows
# "wish litho_main.tcl" to bring up the interface.
if {[info exists env(NM_TCL_DIR)] } {
    set tcl_script_dir $env(NM_TCL_DIR)
} elseif {[info exists env(NANO_ROOT)]} {
    set tcl_script_dir [file join $env(NANO_ROOT) share tcl]
} else {
    set tcl_script_dir .
}
### ks
if {[string match "*wish*" [info nameofexecutable]] } {
    set tcl_script_dir .
}

#appearance variables

# vertical padding between floatscales - image and modify windows
set fspady 3

# List of available images
set imageNames {none }

#Make the window appear in the upper left corner
wm geometry . +0+0

set time_to_quit 0

# Give it a title
wm title . "Seegerizer"
wm iconbitmap . error
# contains definition of useful widgets for the 
# Tcl_Linkvar library.
source [file join ${tcl_script_dir} Tcl_Linkvar_widgets.tcl]

# for overall appearance of user interface
frame .main

# frames - overall layout of the screen
frame .menubar -relief raised -bd 4 

#set up window
pack .main .menubar -side top -fill x 

# menu bar
menu .menu -tearoff 0

#### FILE menu #############################
set filemenu .menu.file
menu $filemenu -tearoff 0
.menu add cascade -label "File" -menu $filemenu -underline 0

$filemenu add command -label "Quit" -underline 1 -command {
    if {[string match "*wish*" [info nameofexecutable]] } {
        # we ran the file using wish, for testing. Kill app.
        exit 0
    } else {
        # we ran using the program. Tell main app to exit. 
        set time_to_quit 1 
    }
}

#### SETUP menu #############################
set setupmenu .menu.setup
menu $setupmenu -tearoff 0
.menu add cascade -label "Setup" -menu $setupmenu -underline 0

$setupmenu add command -label "Drawing Parameters" -underline 0 \
	-command "show.drawing_parameters"


# This command attaches the menu to the main window
# and allows you to use "alt-KEY" to access the menus.
. config -menu .menu


# Invoke File.. Exit when destroying window for clean exit.
wm protocol . WM_DELETE_WINDOW {$filemenu invoke "Quit"}

# puts the focus on the main window, instead of any other windows 
# which have been created. 
wm deiconify .


set nmInfo(drawing_parameters) [create_closing_toplevel drawing_parameters "Drawing Parameters"]

set line_width_nm 0
set exposure_uCoulombs_per_square_cm 0
generic_entry $nmInfo(drawing_parameters).line_width line_width_nm \
    "line width (nm)" real
pack $nmInfo(drawing_parameters).line_width -anchor ne -padx 3 -pady 3
generic_entry $nmInfo(drawing_parameters).exposure \
    exposure_uCoulombs_per_square_cm "exposure (uCoul/cm^2)" real
pack $nmInfo(drawing_parameters).exposure -anchor ne -padx 3 -pady 3

