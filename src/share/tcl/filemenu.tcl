#  ===3rdtech===
#  Copyright (c) 2000 by 3rdTech, Inc.
#  All Rights Reserved.
#
#  This file may not be distributed without the permission of 
#  3rdTech, Inc. 
#  ===3rdtech===
# Globals for remembering where the user opens and saves files.
set fileinfo(open_dir) ""
set fileinfo(save_dir) ""

#
################################
#
# Open a static file. 
# It should open any type of file handled by BCGrid
proc open_static_file {} {
    global open_static_filename fileinfo
    set types { {"All files" *} }
    set filename [tk_getOpenFile -filetypes $types \
            -initialdir $fileinfo(open_dir) \
            -title "Open a static file"]
    if {$filename != ""} {
	# setting this variable triggers a callback in C code
	# which saves the file. 
        # dialog check whether file exists.
        set open_static_filename $filename
        set fileinfo(open_dir) [file dirname $filename]
        # Pop up the height-plane setup dialog. 
        show.z_mapping
    } 
    # otherwise do nothing.
}

#
################################
#
# Open a stream file (vrpn log file). 
proc open_stream_file {} {
    global open_stream_filename fileinfo
    set types { {"Lab Notebook files" ".nms" } 
                {"Lab Notebook files" ".nm" } 
                {"All files" *} }
    set filename [tk_getOpenFile -filetypes $types \
            -initialdir $fileinfo(open_dir)\
            -title "Open a stream file"]
    if {$filename != ""} {
	# setting this variable triggers a callback in C code
	# which saves the file. 
        set open_stream_filename $filename
        set fileinfo(open_dir) [file dirname $filename]
    } 
    # otherwise do nothing.
}

#
################################
#
# Open a device - SPM.
if { $thirdtech_ui } {
    set deviceNames {}
    set deviceConnections {} 
} else {
# Maintain list of all UNC SPMs available here. Don't add UNC SPMs to spm_list_def.tcl
set deviceNames       { "Local SPM" "Nano Demo" "Black Box"}
set deviceConnections { "nmm_Microscope@127.0.0.1" \
        "nmm_Microscope@172.18.2.241" \
        "nmm_Microscope@172.18.2.251" }
}
# Dialog which allows user to choose which device
# and which log file. 
iwidgets::dialog .open_device_dialog -title "Open SPM Connection" -modality application

.open_device_dialog hide Help
.open_device_dialog buttonconfigure OK -text "Open" -command {
    .open_device_dialog deactivate 1
}
.open_device_dialog hide Apply
# "Cancel" button is already set up correctly

set win [.open_device_dialog childsite]
generic_optionmenu_with_index $win.device_name chosen_device_index \
	"SPM to open:" deviceNames
pack $win.device_name -anchor nw -side top

proc choose_logfile { } {
    global fileinfo open_spm_log_name
    set types { {"Lab Notebook files" ".nms" } 
                {"Lab Notebook files" ".nm" } 
                {"All files" *} }
    set filename [tk_getSaveFile -filetypes $types \
            -initialfile "log.nms" -initialdir $fileinfo(save_dir)\
            -title "Choose a lab notebook file"]
    if {$filename != ""} {
        # No error checking here - do that below 
        # in open_spm_connection

        # Make sure filename ends in .nms extension. tk_getSaveFile dialog
        # will strip it, if "hide known file extensions" is on in NT Explorer. 
        if { [string compare -nocase [file extension $filename] ".nms"] != 0 } {
            append filename ".nms"
        }
        set open_spm_log_name $filename
        set fileinfo(save_dir) [file dirname $filename]
    }
    # otherwise do nothing - user pressed cancel or didn't enter file name
}

set open_spm_log_name ""
generic_entry $win.open_logfile open_spm_log_name \
	"Auto lab notebook:" ""
$win.open_logfile configure -width 20
button $win.get_logfile_name -text "Choose..." -command choose_logfile

pack $win.open_logfile $win.get_logfile_name  -side left

# Allow the user to save 
proc open_spm_connection {} {
    global deviceNames deviceConnections 
    global chosen_device_index open_spm_device_name open_spm_log_name

    if { [.open_device_dialog activate] } {
        # Make sure the logfile is OK - MUST be able to write log before 
        # connection opens!

        # directory must exist, must be writable. File must not exist.
        if {![file exists [file dirname $open_spm_log_name]]} {
            nano_error "Cannot save streamfile $open_spm_log_name\nDirectory doesn't exist!"
            return;
        }
        if {![file writable [file dirname $open_spm_log_name]]} {
            nano_error "Cannot save streamfile $open_spm_log_name\nCan't create a file in this directory!"
            return;
        }
        if {[file exists $open_spm_log_name]} {
            nano_error "Cannot save streamfile $open_spm_log_name\nFile already exists!"
            return;
        }
                
        # User chose a device, translate it into a vrpn device name for Nano
        set open_spm_device_name [lindex $deviceConnections $chosen_device_index]
        #puts "$open_spm_device_name"
    } else {
        # user pressed "cancel" so do nothing
    }
}

#
################################
#
# This allows the user to export a file that
# holds the values for a given plane, in any of several formats
#

#Initial dialog which allows user to choose which plane
#of data to save.
iwidgets::dialog .save_plane_dialog -title "Save plane data" -modality application

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

set export_formats {none}
generic_optionmenu $win.export_filetype export_filetype \
	"Format for saved data:" export_formats
pack $win.export_filetype -anchor nw

# Allow the user to save 
proc save_plane_data {} {
    global export_plane export_filetype export_filename fileinfo imageNames z_comes_from
    # Trigger the export_filetype widget to display formats for
    # the default selected export_plane.
    #set export_plane [lindex $imageNames 0]
    set export_plane $z_comes_from
    if { [.save_plane_dialog activate] } {
	set types { {"All files" *} 
        { "ThermoMicroscopes" ".tfr" }
        { "Text(MathCAD)" ".txt" }
        { "PPM Image" ".ppm" }
        { "SPIP" ".spip" }
        { "UNCA Image" ".ima" } }

        # Set the file extension correctly
        set def_file_exten ".tfr"
        #puts $export_filetype
        foreach item $types {
            #puts "[lindex $item 0] [lindex $item 1]"
            if { [string compare $export_filetype [lindex $item 0]] == 0} {
                set def_file_exten [lindex $item 1]
            }
        }

        # Let the user choose a file to save the data in. 
	set filename [tk_getSaveFile -filetypes $types \
		-initialfile "${export_plane}$def_file_exten" \
                -initialdir $fileinfo(save_dir)\
                -title "Save plane data"]
	if {$filename != ""} {
	    #puts "Save plane data: $filename $export_plane $export_filetype"
	    # setting this variable triggers a callback in C code
	    # which saves the file. 

            # Dialog checks for writeable directory, and asks about
            # replacing existing files. 
	    set export_filename $filename
	    set fileinfo(save_dir) [file dirname $filename]
	}
	# otherwise do nothing - user pressed cancel or didn't enter file name
    } else {
	# user pressed "cancel" so do nothing
    }
}

#Initial dialog which allows user to choose which plane
#of data to save.
iwidgets::dialog .save_screen_dialog -title "Save screen image" 
#-modality application

.save_screen_dialog hide Help
.save_screen_dialog hide Apply
.save_screen_dialog buttonconfigure OK -text "Save" -command {
    global fileinfo screenImage_format
    .save_screen_dialog deactivate 1
    set types { {"All files" *} 
    {"TIFF" ".tif" }
    {"PPM/PGM" ".pgm" } 
    {"PPM/PGM" ".ppm" } }

        # Set the file extension correctly
        set def_file_exten ".tif"
        foreach item $types {
            if { [string compare $screenImage_format [lindex $item 0]] == 0} {
                set def_file_exten [lindex $item 1]
            }
        }

    set filename [tk_getSaveFile -filetypes $types \
		-initialfile screenimage$def_file_exten \
                -initialdir $fileinfo(save_dir) \
                -title "Save screen image"] 
    if {$filename != ""} {
	#puts "Save screen image: $filename $screenImage_format"
	update idletasks
	after idle {
	# Setting this variable triggers a callback which saves the file.
            # Dialog checks for writeable directory, and asks about
            # replacing existing files. 
	set screenImage_filename $filename
	# Turn the screen information back on whatever happens
	set chart_junk 1
	}
	set fileinfo(save_dir) [file dirname $filename]
    } else {
	# otherwise do nothing - user pressed cancel or didn't enter file name
	# Turn the screen information back on whatever happens
	set chart_junk 1
    }
}

.save_screen_dialog buttonconfigure Cancel -command {
    .save_screen_dialog deactivate 0
    # Turn the screen information back on whatever happens
    set chart_junk 1
}

set win [.save_screen_dialog childsite]
# Toggle to determine whether the screen labels and decorations
# are visible when the screen shot is saved. 
set chart_junk 1
set screenImage_format_list {}
checkbutton $win.chart_junk -variable chart_junk \
	-text "Show screen text"
generic_optionmenu $win.screenImage_format screenImage_format \
	"Format for saved picture:" screenImage_format_list
pack $win.chart_junk $win.screenImage_format -anchor nw

# Allow the user to save 
proc save_screenImage {} {
    global  screenImage_format screenImage_filename
    # All activity is done in the button commands defined above.
    .save_screen_dialog activate
}	


#Initial dialog which allows user to choose which plane
#of data to save.
iwidgets::dialog .save_mod_dialog -title "Save modification data" -modality application

.save_mod_dialog hide Help
.save_mod_dialog buttonconfigure OK -text "Save" -command {
    .save_mod_dialog deactivate 1
}
.save_mod_dialog hide Apply
# "Cancel" button is already set up correctly

set nmInfo(save_mod_dialog) [.save_mod_dialog childsite]
set mod_data_time_list {}
generic_optionmenu $nmInfo(save_mod_dialog).which_mod_to_save \
	which_mod_to_save \
	"Choose which modification to save:" mod_data_time_list 
pack $nmInfo(save_mod_dialog).which_mod_to_save -anchor nw

# XXX Hack to allow the current code to write the mod data where we
# can see it
# This frame is NOT packed - users don't want to see mod data, they just
# want to save it. 
frame .mod
text .mod.text 

# Whenever we get done with a mod, the C code should call this function
# after it has filled in the .mod.text widget. 
# The timestamp should be time the modify started, in seconds
# (our standard time measure).
proc remember_mod_data { time_stamp} {
    global mod_data
    set mod_data(mod-$time_stamp) [.mod.text get 1.0 end] 

}

# Allows the C code, microscape.c, to clear saved modifications from
# the list when streamfiles or connections change. 

proc forget_mod_data { } {
    global mod_data
    if { [info exists mod_data] } {
        unset mod_data
    }
}

# Allow the user to save 
proc save_mod_dialog {} {
    global  nmInfo mod_data which_mod_to_save mod_data_time_list 
    global tcl_platform fileinfo

    if { [info exists mod_data] } {
        set mod_data_time_list [lsort [array names mod_data]]
    } else {
        nano_warning "No modification data exists to save at this time"
        return;
    }
    if { [.save_mod_dialog activate] } {
        # If data doesn't exist, display an error
        if { ![info exists mod_data($which_mod_to_save)] } {
            nano_error "Data from modification at time $which_mod_to_save is not accessible. \nPlease replay stream file to try again"
            return;
        }
	set types { {"All files" *} }
	set filename [tk_getSaveFile -filetypes $types \
		-initialfile $which_mod_to_save.mod \
                -initialdir $fileinfo(save_dir)\
                -title "Save modification data"]
	if {$filename != ""} {
	    #puts $filename
            # Dialog checks for writeable directory, and asks about
            # replacing existing files. 
	    set modfile [open $filename w]
	    puts -nonewline $modfile $mod_data($which_mod_to_save)
	    flush $modfile
	    close $modfile
	    set fileinfo(save_dir) [file dirname $filename]
	    if {$tcl_platform(platform) == "unix"} {
		exec unix_to_dos $filename
	    }

	} 
	# otherwise do nothing - user pressed cancel or didn't enter file name
    } else {
	# user pressed "cancel" so do nothing
    }
}	


