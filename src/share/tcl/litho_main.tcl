#!/bin/sh
# the next line restarts using wishx (see man wish) \
	exec wish "$0" ${1+"$@"}
# This is a tcl/tk script to create the user interface to the
# ebeam lithography system.  
#

# Import Itcl and Iwidgets, for the tabnotebook widget and others we
# will use. This import method is a BUG - we shouldn't have to import
# the itcl and itk namespaces, but at least we can use it...
catch { set auto_path [lappend auto_path $env(ITCL_LIBRARY)] }

package require Itcl 
catch {namespace import itcl::*}
package require Itk 
catch {namespace import itk::*}
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
# If variable is not set, 
# assume files are in the current directory. Allows
# "wish litho_main.tcl" to bring up the interface.
if {![info exists tcl_script_dir] } {
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
source [file join ${tcl_script_dir} colormap.tcl]
source [file join ${tcl_script_dir} registration.tcl]

# frames - overall layout of the screen
frame .menubar -relief raised -bd 4

# for overall appearance of user interface
frame .main

# controls is for stuff that stay on the screen all the time
frame .main.controls -relief raised -bd 2

#set up window
pack .menubar .main -side top -fill x

pack .main.controls -side top -fill x

#initialize frame names
set controls .main.controls


# menu bar
menu .menu -tearoff 0

#### FILE menu globals, windows and procedures #####################

# globals for remembering where the user opens and saves files
set fileinfo(open_dir) ""
set fileinfo(save_dir) ""

#
################################
#
# This allows the user to export a file that
# holds the values for a given plane, in any of several formats
#

#Initial dialog which allows user to choose which plane
#of data to save.
iwidgets::dialog .save_plane_dialog -title "Save plane data"\
  -modality application

.save_plane_dialog hide Help
.save_plane_dialog buttonconfigure OK -text "Save" -command {
    .save_plane_dialog deactivate 1
}
.save_plane_dialog hide Apply
# "Cancel" button is already set up correctly

set win [.save_plane_dialog childsite]
generic_optionmenu $win.export_plane export_plane \
        "Plane to extract data from:" imageNames
pack $win.export_plane -anchor nw

set save_image_format_list {none}
generic_optionmenu $win.save_image_filetype save_image_filetype \
        "Format for saved data:" save_image_format_list
pack $win.save_image_filetype -anchor nw

# Allow the user to save
proc save_image_file {} {
    global export_plane save_image_filetype save_image_filename \
           fileinfo imageNames
    # Trigger the export_filetype widget to display formats for
    # the default selected export_plane.
    set export_plane [lindex $imageNames 0]
    if { [.save_plane_dialog activate] } {
        set types { {"All files" *}
        { "ThermoMicroscopes" ".tfr" }
        { "TIFF Image" ".tif" }
        { "TIFF" ".tif" }
        { "TIF" ".tif" }
        { "JPG" ".jpg" }
        { "JPEG" ".jpg" }
        { "BMP" ".bmp" }
        { "PGM" ".pgm" }
        { "PPM" ".ppm" }
        { "Text(MathCAD)" ".txt" }
        { "PPM Image" ".ppm" }
        { "SPIP" ".spip" }
        { "UNCA Image" ".ima" } }

        # Set the file extension correctly
        set def_file_exten ".tif"
        #puts $save_image_filetype
        foreach item $types {
            #puts "[lindex $item 0] [lindex $item 1]"
            if { [string compare $save_image_filetype [lindex $item 0]] == 0} {
                set def_file_exten [lindex $item 1]
            }
        }

        # Let the user choose a file to save the data in.
        set filename [tk_getSaveFile -filetypes $types \
                -initialfile "${export_plane}$def_file_exten" \
                -initialdir $fileinfo(save_dir)\
                -title "Save plane data"]
        if {$filename != ""} {
            #puts "Save plane data: $save_image_filename $export_plane $save_image_filetype"
            # setting this variable triggers a callback in C code
            # which saves the file.

            # Dialog checks for writeable directory, and asks about
            # replacing existing files.
            set save_image_filename $filename
            set fileinfo(save_dir) [file dirname $save_image_filename]
        }
        # otherwise do nothing - user pressed cancel or didn't enter file name
    } else {
        # user pressed "cancel" so do nothing
    }
}

#
################################
#
# This allows the user to export a file that
# is a snapshot of the window on the screen, in any of several formats
#

iwidgets::dialog .save_buffer_dialog -title "Save buffer image"
#-modality application
.save_buffer_dialog hide Help
.save_buffer_dialog hide Apply
.save_buffer_dialog buttonconfigure OK -text "Save" -command {
    global fileinfo bufferImage_format
    .save_buffer_dialog deactivate 1
    set types { {"All files" *}
    {"TIFF" ".tif" }
    {"TIF" ".tif" }
    {"JPEG" ".jpg" }
    {"JPG" ".jpg" }
    {"BMP" ".bmp" }
    {"PGM" ".pgm" }
    {"PPM" ".ppm" }}

    # Set the file extension correctly
    set def_file_exten ".tif"
    #puts $bufferImage_format
    foreach item $types {
        #puts "[lindex $item 0] [lindex $item 1]"
        if { [string compare $bufferImage_format [lindex $item 0]] == 0} {
            set def_file_exten [lindex $item 1]
        }
    }
    
    set filename [tk_getSaveFile -filetypes $types \
                -initialfile aligned_data$def_file_exten \
                -initialdir $fileinfo(save_dir) \
                -title "Save buffer image"]
    if {$filename != ""} {
        #puts "Save buffer image: $filename $bufferImage_format"
        update idletasks
        after idle {
        # Setting this variable triggers a callback which saves the file.
            # Dialog checks for writeable directory, and asks about
            # replacing existing files.
        set bufferImage_filename $filename
        }
        set fileinfo(save_dir) [file dirname $filename]
    } else {
        # otherwise do nothing - user pressed cancel or didn't enter file name
    }

}

.save_buffer_dialog buttonconfigure Cancel -command {
    .save_buffer_dialog deactivate 0
}

set win [.save_buffer_dialog childsite]
set bufferImage_format_list {}
generic_optionmenu $win.bufferImage_format bufferImage_format \
        "Format for saved picture:" bufferImage_format_list
pack $win.bufferImage_format -anchor nw

# Allow the user to open a file
proc open_image_file {} {
    global open_image_filename fileinfo
    set types { {"All files" *} }
    set filename [tk_getOpenFile -filetypes $types \
            -initialdir $fileinfo(open_dir) \
            -title "Open an image file"]
    if {$filename != ""} {
        # setting this variable triggers a callback in C code
        # which saves the file.
        # dialog check whether file exists.
        set open_image_filename $filename
        set fileinfo(open_dir) [file dirname $filename]
    }
    # otherwise do nothing.
}

# Allow the user to save
proc save_editor_window {} {
    global  bufferImage_format bufferImage_filename
    # All activity is done in the button commands defined above.
    .save_buffer_dialog activate
}

proc open_pattern_file {} {
    global open_pattern_filename fileinfo
    set types { {"Pattern Files" ".spf"} }
    set filename [tk_getOpenFile -filetypes $types \
            -initialdir $fileinfo(open_dir) \
            -title "Open a pattern file"]
    if {$filename != ""} {
        # setting this variable triggers a callback in C code
        # which saves the file.
        # dialog checks whether file exists.
        set open_pattern_filename $filename
        set fileinfo(open_dir) [file dirname $filename]
    }
    # otherwise do nothing.
}

proc save_pattern_file {} {
    global save_pattern_filename fileinfo
    set types { {"Pattern Files" ".spf"} }
    # Let the user choose a file to save the data in.
    set filename [tk_getSaveFile -filetypes $types \
                -initialfile "pattern.spf" \
                -initialdir $fileinfo(save_dir)\
                -title "Save pattern"]
    if {$filename != ""} {
        # setting this variable triggers a callback in C code
        # which saves the file.

        # Dialog checks for writeable directory, and asks about
        # replacing existing files.
        set save_pattern_filename $filename
        set fileinfo(save_dir) [file dirname $save_pattern_filename]
    }
    # otherwise do nothing - user pressed cancel or didn't enter file name
}

#### FILE menu commands ####################

set filemenu .menu.file
menu $filemenu -tearoff 0
.menu add cascade -label "File" -menu $filemenu -underline 0

$filemenu add command -label "Open Image File..." -underline 0 \
    -command "open_image_file"

$filemenu add command -label "Open Pattern File..." -underline 0 \
    -command "open_pattern_file"

$filemenu add command -label "Save Editor Window..." -underline 0 \
    -command "save_editor_window"

$filemenu add command -label "Save Image File..." -underline 5 -command \
    "save_image_file"

$filemenu add command -label "Save Pattern File..." -underline 5 -command \
    "save_pattern_file"

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
.menu add cascade -label "Setup" -menu $setupmenu -underline 0 -columnbreak 0

#$setupmenu add command -label "Drawing" -underline 0 \
#	-command "show.drawing_parameters"

#$setupmenu add command -label "Display" -underline 0 \
#        -command "show.display_parameters"

$setupmenu add command -label "Alignment" -underline 0 \
        -command "show.registration"

#### SEM menu ###############################
set SEM_menu .menu.sem
menu $SEM_menu -tearoff 0
.menu add cascade -label "SEM" -menu $SEM_menu -underline 0 -columnbreak 0

$SEM_menu add command -label "Image Acquisition" -underline 0 \
        -command "show.sem_image_acquisition"

#$SEM_menu add command -label "Expose Pattern" -underline 0 \
#        -command "show.sem_beam_expose_control"


# This command attaches the menu to the main window
# and allows you to use "alt-KEY" to access the menus.
. config -menu .menu


# Invoke File.. Exit when destroying window for clean exit.
wm protocol . WM_DELETE_WINDOW {$filemenu invoke "Quit"}

#################################################
# keeps track of how many controls are in an array we will use to
# store which controls should be disabled during long operations
set numProtectedControls 0


######### Drawing Parameters Control Panel ###########################
#set drawing_parameters_win \
#         [create_closing_toplevel drawing_parameters "Drawing Parameters"]

frame .main.controls.drawing_parameters -bd 3 -relief groove
pack .main.controls.drawing_parameters -side top -fill x
set drawing_parameters_win .main.controls.drawing_parameters
label $drawing_parameters_win.main_label -text "Drawing Parameters"
pack $drawing_parameters_win.main_label -side top

set line_width1_nm 1000
set line_width2_nm 400
set area_exposure_uCoulombs_per_square_cm 300
set line_exposure_pCoulombs_per_cm 150
set drawing_tool 1
set clear_drawing 0

set pattern_color_changed 0
set pattern_r 255
set pattern_g 255
set pattern_b 255

frame $drawing_parameters_win.tool -bd 3 -relief groove
label $drawing_parameters_win.tool.tool_label -text "Tool:"
pack $drawing_parameters_win.tool.tool_label -side top

frame $drawing_parameters_win.tool.thinpolyline -bd 3 -relief groove
pack $drawing_parameters_win.tool.thinpolyline -side top
radiobutton $drawing_parameters_win.tool.thinpolyline.button \
      -variable drawing_tool \
      -value 1 -text "one-pass polyline" -justify left
generic_entry $drawing_parameters_win.tool.thinpolyline.line_exposure \
    line_exposure_pCoulombs_per_cm "linear exposure (pCoul/cm)" real
pack $drawing_parameters_win.tool.thinpolyline.line_exposure \
        -anchor ne -padx 3 -pady 3
button $drawing_parameters_win.tool.thinpolyline.add_test_grid \
    -text "Add Test Grid" -command { set add_test_grid 1 }
pack $drawing_parameters_win.tool.thinpolyline.add_test_grid -side top

frame $drawing_parameters_win.tool.area_tools -bd 3 -relief groove
pack $drawing_parameters_win.tool.area_tools -side top
generic_entry $drawing_parameters_win.tool.area_tools.area_exposure \
    area_exposure_uCoulombs_per_square_cm "area exposure (uCoul/cm^2)" real
pack $drawing_parameters_win.tool.area_tools.area_exposure -anchor ne \
    -padx 3 -pady 3

frame $drawing_parameters_win.tool.area_tools.thickpolyline -bd 3 -relief groove
pack $drawing_parameters_win.tool.area_tools.thickpolyline -side right
radiobutton $drawing_parameters_win.tool.area_tools.thickpolyline.button \
      -variable drawing_tool \
      -value 2 -text "thick polyline" -justify left

frame $drawing_parameters_win.tool.area_tools.thickpolyline.line_width1
radiobutton $drawing_parameters_win.tool.area_tools.thickpolyline.line_width1.button \
	-variable width_value -value 1
generic_entry $drawing_parameters_win.tool.area_tools.thickpolyline.line_width1.entry \
    line_width1_nm "line width 1 (nm)" real
frame $drawing_parameters_win.tool.area_tools.thickpolyline.line_width2
radiobutton $drawing_parameters_win.tool.area_tools.thickpolyline.line_width2.button \
	-variable width_value -value 2
generic_entry $drawing_parameters_win.tool.area_tools.thickpolyline.line_width2.entry \
    line_width2_nm "line width 2 (nm)" real

pack $drawing_parameters_win.tool.area_tools.thickpolyline.line_width1 
pack $drawing_parameters_win.tool.area_tools.thickpolyline.line_width1.button \
	-side left
pack $drawing_parameters_win.tool.area_tools.thickpolyline.line_width1.entry \
	-side right
pack $drawing_parameters_win.tool.area_tools.thickpolyline.line_width2
pack $drawing_parameters_win.tool.area_tools.thickpolyline.line_width2.button \
	-side left
pack $drawing_parameters_win.tool.area_tools.thickpolyline.line_width2.entry \
	-side right

radiobutton $drawing_parameters_win.tool.area_tools.polygon \
      -variable drawing_tool -value 3 -text "polygon" -justify left
radiobutton $drawing_parameters_win.tool.dump_point -variable drawing_tool \
      -value 4 -text "dump point" -justify left
#radiobutton $drawing_parameters_win.tool.select -variable drawing_tool \
#      -value 5 -text "select" -justify left

pack $drawing_parameters_win.tool -side left

pack $drawing_parameters_win.tool.thinpolyline.button \
     $drawing_parameters_win.tool.area_tools.thickpolyline.button \
     $drawing_parameters_win.tool.area_tools.polygon \
     $drawing_parameters_win.tool.dump_point -side top

#     $drawing_parameters_win.tool.select -side top

generic_optionmenu $drawing_parameters_win.canvas_image canvas_image \
        "Canvas Image" imageNames
pack $drawing_parameters_win.canvas_image -anchor nw \
       -padx 3 -pady 3 -side top


frame $drawing_parameters_win.clear_pattern
pack $drawing_parameters_win.clear_pattern -anchor nw -side top 
button $drawing_parameters_win.clear_pattern.button -text "Clear Pattern" -command \
    { set clear_pattern 1 }
pack $drawing_parameters_win.clear_pattern.button -side left
checkbutton $drawing_parameters_win.clear_pattern.confirm \
   -text "Sure?" -variable clear_pattern_confirm
pack $drawing_parameters_win.clear_pattern.confirm -anchor w

button $drawing_parameters_win.undo_shape -text "Undo Shape" -command \
    { set undo_shape 1 }
pack $drawing_parameters_win.undo_shape -anchor nw -side top

button $drawing_parameters_win.undo_point -text "Undo Point" -command \
    { set undo_point 1 }
pack $drawing_parameters_win.undo_point -anchor nw -side top

frame $drawing_parameters_win.segment_length -relief solid \
      -borderwidth 2
label $drawing_parameters_win.segment_length.title -text "length:"
label $drawing_parameters_win.segment_length.value \
      -textvariable segment_length -width 10 -anchor e -justify left

pack $drawing_parameters_win.segment_length 
pack $drawing_parameters_win.segment_length.title -side left
pack $drawing_parameters_win.segment_length.value -side left

set pattern_color [format #%02x%02x%02x $pattern_r $pattern_g $pattern_b]

proc set_pattern_color {} {
    global pattern_r pattern_g pattern_b pattern_color pattern_color_changed
    # Extract three component colors of pattern_color
    # and save into pattern_r g b
    scan $pattern_color #%02x%02x%02x pattern_r pattern_g pattern_b
    set pattern_color_changed 1
}

proc update_pattern_color_sample {name el op} {
    global pattern_r pattern_g pattern_b pattern_color drawing_parameters_win
    set pattern_color [format #%02x%02x%02x $pattern_r $pattern_g $pattern_b]
    $drawing_parameters_win.pattern_color.colorsample configure -bg $pattern_color
}

trace variable pattern_color_changed w update_pattern_color_sample

frame $drawing_parameters_win.pattern_color
button $drawing_parameters_win.pattern_color.set_color -text "Set pattern color" \
   -command {
     choose_color pattern_color "Choose pattern color" $drawing_parameters_win
     $drawing_parameters_win.pattern_color.colorsample configure -bg $pattern_color
     set_pattern_color
}

button $drawing_parameters_win.pattern_color.colorsample -relief groove -bd 2 \
   -bg $pattern_color -command {
           $drawing_parameters_win.pattern_color.set_color invoke
}

pack $drawing_parameters_win.pattern_color -side top -fill x
pack $drawing_parameters_win.pattern_color.set_color -side left -fill x
pack $drawing_parameters_win.pattern_color.colorsample -side left -fill x \
     -expand yes

######### End of Drawing Parameters Control Panel ####################

######### Display Parameters Control Panel ###########################

#parameters with C counterparts (in fact, each image has its own set of these
# and the C code is responsible for swapping in the right values when you
# change the current image)
set image_color_changed 0
set image_r 255
set image_g 255
set image_b 120
set image_opacity 50.0
set image_magnification 1000.0
set hide_other_images 0
set enable_image_display 0
set current_image "none"

#set display_parameters_win \
#        [create_closing_toplevel display_parameters "Display Parameters"]
frame .main.controls.display_parameters -bd 3 -relief groove
pack .main.controls.display_parameters -side top -fill x
set display_parameters_win .main.controls.display_parameters
label $display_parameters_win.main_label -text "Image Display Parameters"
pack $display_parameters_win.main_label -side top

generic_optionmenu $display_parameters_win.current_image current_image \
        "Image" imageNames
pack $display_parameters_win.current_image -anchor nw \
     -padx 3 -pady 3 -side left

set image_color [format #%02x%02x%02x $image_r $image_g $image_b]

proc set_image_color {} {
    global image_r image_g image_b image_color image_color_changed
    # Extract three component colors of image_color
    # and save into image_r g b
    scan $image_color #%02x%02x%02x image_r image_g image_b
    set image_color_changed 1
}

proc update_color_sample {name el op} {
    global image_r image_g image_b image_color display_parameters_win
    set image_color [format #%02x%02x%02x $image_r $image_g $image_b]
    $display_parameters_win.image_color.colorsample configure -bg $image_color
}

trace variable image_color_changed w update_color_sample

frame $display_parameters_win.image_color
button $display_parameters_win.image_color.set_color -text "Set image color" \
   -command {
     choose_color image_color "Choose image color" $display_parameters_win
     $display_parameters_win.image_color.colorsample configure -bg $image_color
     set_image_color
}

button $display_parameters_win.image_color.colorsample -relief groove -bd 2 \
   -bg $image_color -command {
           $display_parameters_win.image_color.set_color invoke
}

pack $display_parameters_win.image_color -side top -fill x
pack $display_parameters_win.image_color.set_color -side left -fill x
pack $display_parameters_win.image_color.colorsample -side left -fill x \
     -expand yes

generic_entry $display_parameters_win.image_opacity image_opacity \
     "Image opacity" real

pack $display_parameters_win.image_opacity -side top -fill x -padx 3 -pady 3

generic_entry $display_parameters_win.magnification image_magnification \
     "Image magnification" real

pack $display_parameters_win.magnification -side top -fill x -padx 3 -pady 3

checkbutton $display_parameters_win.hide_others_check \
   -text "Hide Others" -variable hide_other_images
   
pack $display_parameters_win.hide_others_check -anchor nw -padx 3 -pady 3 -side right

checkbutton $display_parameters_win.enable_display_check \
   -text "Enabled" -variable enable_image_display

pack $display_parameters_win.enable_display_check -anchor nw -padx 3 -pady 3
######### End of Drawing Parameters Control Panel ####################

######### SEM Image Acquisition ######################################
global sem_window_open sem_acquire_image sem_acquire_continuous \
       sem_pixel_integration_time_nsec sem_inter_pixel_delay_time_nsec \
       sem_resolution sem_overwrite_old_data sem_data_buffer \
       sem_bufferImageNames sem_acquisition_magnification \
       sem_beam_blank_enable sem_horiz_retrace_delay sem_vert_retrace_delay \
       sem_x_dac_gain sem_x_dac_offset sem_y_dac_gain sem_y_dac_offset \
       sem_z_adc_gain sem_z_adc_offset sem_external_scan_control_enable
       

set sem_pixel_integration_time_nsec 0
set sem_inter_pixel_delay_time_nsec 0
set sem_resolution 1
set sem_acquire_continuous 0
set sem_overwrite_old_data 1
set sem_data_buffer "none"
set sem_bufferImageNames {none}
set sem_acquisition_magnification 1000
set sem_beam_blank_enable 0
set sem_horiz_retrace_delay_nsec 0
set sem_vert_retrace_delay_nsec 0
set sem_x_dac_gain 16384
set sem_x_dac_offset 0
set sem_y_dac_gain 16384
set sem_y_dac_offset 0
set sem_z_adc_gain 16384
set sem_z_adc_offset 0
set sem_external_scan_control_enable 0

set sem_win  \
   [create_closing_toplevel_with_notify sem_image_acquisition sem_window_open \
              "SEM Image Acquisition"]


# controls whether we have any scan control at all
checkbutton $sem_win.external_scan_control \
   -text "Enable Scan Control" -variable sem_external_scan_control_enable
pack $sem_win.external_scan_control -anchor w -padx 3 -pady 3

# Button to start the scan
button $sem_win.acquire_image -text "Acquire Image" \
        -command { set sem_acquire_image 1 }
pack $sem_win.acquire_image -anchor w -padx 3 -pady 3

set protectedControls($numProtectedControls) $sem_win.acquire_image
incr numProtectedControls

# Controls whether to scan continuously or not
checkbutton $sem_win.acquire_continuous \
        -text "Acquire Continuously" \
        -variable sem_acquire_continuous
checkbutton $sem_win.overwrite_old_data \
        -text "Overwrite Old Data" \
        -variable sem_overwrite_old_data

# this could be made selectable but its a bit complicated and unnecessary
#generic_optionmenu $sem_win.buffer_select sem_data_buffer \
#        "data buffer" sem_bufferImageNames
frame $sem_win.data_buf_label

label $sem_win.data_buf_label.descr -text "Data Buffer: "
label $sem_win.data_buf_label.value -textvariable sem_data_buffer \
      -fg blue -bg white

generic_entry $sem_win.magnification sem_acquisition_magnification \
        "Magnification (for 12.8 cm wide display)" integer
checkbutton $sem_win.beam_blank_enable \
        -text "Blank Beam Between Points" \
        -variable sem_beam_blank_enable
generic_entry $sem_win.horiz_retrace sem_horiz_retrace_delay_nsec \
        "Horiz. Retrace Delay (nsec)" integer
generic_entry $sem_win.vert_retrace sem_vert_retrace_delay_nsec \
        "Vert. Retrace Delay (nsec)" integer

pack $sem_win.acquire_continuous -anchor w -padx 3 -pady 3
pack $sem_win.overwrite_old_data -anchor w -padx 3 -pady 3
#pack $sem_win.buffer_select -anchor w -padx 3 -pady 3
pack $sem_win.data_buf_label -anchor w -padx 3 -pady 3
pack $sem_win.data_buf_label.descr -padx 3 -pady 3 -side left
pack $sem_win.data_buf_label.value -padx 3 -pady 3 -side left
pack $sem_win.magnification -anchor w -padx 3 -pady 3
pack $sem_win.beam_blank_enable -anchor w -padx 3 -pady 3
pack $sem_win.horiz_retrace -anchor w -padx 3 -pady 3
pack $sem_win.vert_retrace -anchor w -padx 3 -pady 3

frame $sem_win.res -bd 3 -relief groove
pack $sem_win.res -side top
label $sem_win.res.res_label -text "Resolution"
pack $sem_win.res.res_label -side top
radiobutton $sem_win.res.res1 -variable sem_resolution \
     -value 0 -text "50x64" -justify left -anchor nw
radiobutton $sem_win.res.res2 -variable sem_resolution \
     -value 1 -text "100x128" -justify left -anchor nw
radiobutton $sem_win.res.res3 -variable sem_resolution \
     -value 2 -text "200x256" -justify left -anchor nw
radiobutton $sem_win.res.res4 -variable sem_resolution \
     -value 3 -text "400x512" -justify left -anchor nw
radiobutton $sem_win.res.res5 -variable sem_resolution \
     -value 4 -text "800x1024" -justify left -anchor nw
radiobutton $sem_win.res.res6 -variable sem_resolution \
     -value 5 -text "1600x2048" -justify left -anchor nw
radiobutton $sem_win.res.res7 -variable sem_resolution \
     -value 6 -text "3200x4096" -justify left -anchor nw

pack $sem_win.res.res1 $sem_win.res.res2 \
     $sem_win.res.res3 $sem_win.res.res4 \
     $sem_win.res.res5 $sem_win.res.res6 \
     $sem_win.res.res7 -fill x

generic_entry $sem_win.pixel_integration_time_nsec \
    sem_pixel_integration_time_nsec \
    "Pixel Integration (nsec)" integer
pack $sem_win.pixel_integration_time_nsec -padx 3 -pady 3
generic_entry $sem_win.inter_pixel_delay_time_nsec \
    sem_inter_pixel_delay_time_nsec \
    "Inter Pixel Delay (nsec)" integer
pack $sem_win.inter_pixel_delay_time_nsec -padx 3 -pady 3

frame $sem_win.dac_settings -bd 3 -relief groove
pack $sem_win.dac_settings -side top -fill x
label $sem_win.dac_settings.header_label -text "DAC Settings"
pack $sem_win.dac_settings.header_label -side top
generic_entry $sem_win.dac_settings.x_gain sem_x_dac_gain "X gain" integer
generic_entry $sem_win.dac_settings.x_offset sem_x_dac_offset "X offset" integer
generic_entry $sem_win.dac_settings.y_gain sem_y_dac_gain "Y gain" integer
generic_entry $sem_win.dac_settings.y_offset sem_y_dac_offset "Y offset" integer
generic_entry $sem_win.dac_settings.z_gain sem_z_adc_gain "Z gain" integer
generic_entry $sem_win.dac_settings.z_offset sem_z_adc_offset "Z offset" integer

pack $sem_win.dac_settings.x_gain \
     $sem_win.dac_settings.x_offset \
     $sem_win.dac_settings.y_gain \
     $sem_win.dac_settings.y_offset \
     $sem_win.dac_settings.z_gain \
     $sem_win.dac_settings.z_offset -fill x

set protectedControls($numProtectedControls) $sem_win.dac_settings.x_gain
incr numProtectedControls
set protectedControls($numProtectedControls) $sem_win.dac_settings.x_offset
incr numProtectedControls
set protectedControls($numProtectedControls) $sem_win.dac_settings.y_gain
incr numProtectedControls
set protectedControls($numProtectedControls) $sem_win.dac_settings.y_offset
incr numProtectedControls
set protectedControls($numProtectedControls) $sem_win.dac_settings.z_gain
incr numProtectedControls
set protectedControls($numProtectedControls) $sem_win.dac_settings.z_offset
incr numProtectedControls

######### End Image Acquisition ###########################

######### Beam Control Panel ##############################
global sem_exposure_magnification sem_beam_width_nm sem_beam_current_picoAmps \
       sem_beam_expose_now sem_dot_spacing_nm sem_line_spacing_nm \
       sem_beam_expose_enabled sem_do_timing_test

set sem_exposure_magnification 10000
set sem_dot_spacing_nm 0
set sem_line_spacing_nm 0
set sem_beam_current_picoAmps 0
set sem_beam_expose_enabled 0
set sem_beam_expose_now 0
set sem_do_timing_test 0

#set sem_beam_win \
#   [create_closing_toplevel sem_beam_expose_control "SEM Control"]

frame .main.controls.sem_beam_expose_control -bd 3 -relief groove
pack .main.controls.sem_beam_expose_control -side top -fill x
set sem_beam_win .main.controls.sem_beam_expose_control
label $sem_beam_win.main_label -text "SEM Beam Control"
pack $sem_beam_win.main_label -side top

frame $sem_beam_win.calibration -relief raised -bd 3

pack $sem_beam_win.calibration -anchor w -padx 3 -pady 3

label $sem_beam_win.calibration.calibration_label -text "Calibration Parameters"
pack $sem_beam_win.calibration.calibration_label \
         -padx 3 -pady 3

generic_entry $sem_beam_win.calibration.magnification \
    sem_exposure_magnification \
    "Magnification (for 12.8 cm wide display)" integer
pack $sem_beam_win.calibration.magnification -anchor w -padx 3 -pady 3

frame $sem_beam_win.calibration.spacing
pack $sem_beam_win.calibration.spacing -anchor w
generic_entry $sem_beam_win.calibration.spacing.dot_spacing \
    sem_dot_spacing_nm \
    "Dot Spacing (nm)" real
pack $sem_beam_win.calibration.spacing.dot_spacing -anchor w -padx 3 -pady 3 -side left

generic_entry $sem_beam_win.calibration.spacing.line_spacing \
    sem_line_spacing_nm \
    "Line Spacing (nm)" real
pack $sem_beam_win.calibration.spacing.line_spacing -anchor w -padx 3 -pady 3 -side right

generic_entry $sem_beam_win.calibration.beam_current \
    sem_beam_current_picoAmps \
    "Beam Current (picoAmps)" real
pack $sem_beam_win.calibration.beam_current -anchor w -padx 3 -pady 3

set sem_min_lin_exposure "0"
set sem_min_area_exposure "0"

frame $sem_beam_win.calibration.min_lin_exposure -relief solid -borderwidth 2
pack $sem_beam_win.calibration.min_lin_exposure -padx 3 -pady 3 -side right
label $sem_beam_win.calibration.min_lin_exposure.label -text "min. lin. exp.:"
label $sem_beam_win.calibration.min_lin_exposure.value \
     -textvariable sem_min_lin_exposure
pack $sem_beam_win.calibration.min_lin_exposure.label -side left
pack $sem_beam_win.calibration.min_lin_exposure.value -side left

frame $sem_beam_win.calibration.min_area_exposure -relief solid -borderwidth 2
pack $sem_beam_win.calibration.min_area_exposure -padx 3 -pady 3
label $sem_beam_win.calibration.min_area_exposure.label -text "min. area exp.:"
label $sem_beam_win.calibration.min_area_exposure.value \
     -textvariable sem_min_area_exposure
pack $sem_beam_win.calibration.min_area_exposure.label -side left
pack $sem_beam_win.calibration.min_area_exposure.value -side left

button $sem_beam_win.do_timing_test -text "Timing Test" \
    -command "set sem_do_timing_test 1" 
pack $sem_beam_win.do_timing_test -padx 3 -pady 3 -side left

set protectedControls($numProtectedControls) $sem_beam_win.do_timing_test
incr numProtectedControls

button $sem_beam_win.expose_now -text "EXPOSE" \
    -command { set sem_beam_expose_now 1 }
pack $sem_beam_win.expose_now -padx 3 -pady 3 -side right

$sem_beam_win.expose_now configure -state disabled

checkbutton $sem_beam_win.enable_point_reporting \
   -text "Display Beam Dwell Points" -variable sem_point_report_enable
pack $sem_beam_win.enable_point_reporting -padx 3 -pady 3

set sem_exposure_status "N/A"

frame $sem_beam_win.expose_progress -relief solid -borderwidth 2
pack $sem_beam_win.expose_progress -padx 3 -pady 3
label $sem_beam_win.expose_progress.label -text "status:"
label $sem_beam_win.expose_progress.value \
      -textvariable sem_exposure_status

pack $sem_beam_win.expose_progress.label -side left
pack $sem_beam_win.expose_progress.value -side left

# allow C code to control whether expose button is enabled:
proc handle_expose_enable { nm el op } {
  global sem_beam_expose_enabled
  global sem_beam_win
  if { $sem_beam_expose_enabled != 0 } {
    $sem_beam_win.expose_now configure -state normal -fg White -bg Red
  } else {
    $sem_beam_win.expose_now configure -state disabled -fg Black -bg LemonChiffon1
  }
}
trace variable sem_beam_expose_enabled w handle_expose_enable

######### End Beam ########################################

set sem_controls_enabled 1

proc handle_sem_controls_enable { nm el op } {
  global sem_controls_enabled
  global sem_beam_expose_enabled
  global sem_beam_win
  global numProtectedControls
  global protectedControls
  if { $sem_controls_enabled == 0 } {
    # do this one specially
    $sem_beam_win.expose_now configure -state disabled
    # now do all the normal cases
    for {set i 0} {$i < $numProtectedControls} {incr i} {
      $protectedControls($i) configure -state disabled
    }
  } else {
    # do this one specially 
    if { $sem_beam_expose_enabled != 0 } {
      $sem_beam_win.expose_now configure -state normal
    }
    for {set i 0} {$i < $numProtectedControls} {incr i} {
      $protectedControls($i) configure -state normal
    }
  }
}

trace variable sem_controls_enabled w handle_sem_controls_enable


################ Error Handling #############################
### Message dialog for warnings and errors.
# Designed to be called from C code, as well.

# The iwidgets message box had a problem when reporting Phantom errors,
# for some reason - Tcl_Eval failed in that situation.
# Switched to tk_messageBox procedure, and things work fine.

#iwidgets::messagedialog .error_dialog -title "Program Error" \
#    -bitmap error -text "Error" -modality application

#.error_dialog hide Help
#.message_dialog buttonconfigure OK -text "Yes"
#.error_dialog hide Cancel
proc display_fatal_error_dialog {msg } {
    global quit_program_now
    tk_messageBox -message "$msg" -title "Seegerizer Fatal Error" \
            -type ok -icon error
    set quit_program_now 1
}
proc display_error_dialog {msg } {
    tk_messageBox -message "$msg" -title "Seegerizer Error" \
            -type ok -icon error
}
proc display_warning_dialog {msg } {
    toplevel .warning
    label .warning.label -text "$msg"
    button .warning.button -text "OK" -command {destroy .warning}
    pack .warning.label .warning.button
}
# bgerror is a special name, provided by tcl/tk, called if there
# is a background error in the script. Don't necessarily want
# these to be fatal.
proc bgerror {msg} {
    display_error_dialog "Internal tcl error: $msg"
}
######################

#----------------
# Setup window positions and geometries to be convenient and pleasant!
# We do this at the end so we can find out the requested size of
# all the widgets we created.

after idle {
  update idletasks
  # Find out about the main window geometry.
  scan [wm geometry .] %dx%d+%d+%d width height main_xpos main_ypos
  puts "$width, $height, $main_xpos, $main_ypos"
  # wm rootx seems to tell us how big the border of the window is.
  set winborder [expr [winfo rootx .] - $main_xpos]
  set main_width [expr $width + 2* $winborder ]
  set main_height [expr $height + ([winfo rooty .] - $main_ypos) + $winborder ]

  # Keep track of where the next window should appear
  set next_left_pos [expr $main_xpos +$main_width]
}

# puts the focus on the main window, instead of any other windows
# which have been created.
after idle {
  wm deiconify .
}
