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
    set file [tk_getOpenFile -filetypes $types -initialdir $fileinfo(open_dir)]
    if {$file != ""} {
	# setting this variable triggers a callback in C code
	# which saves the file. 
	set open_static_filename $file
	set fileinfo(open_dir) [file dirname $file]
    } 
    # otherwise do nothing.
}

#
################################
#
# Open a stream file (vrpn log file). 
proc open_stream_file {} {
    global open_stream_filename fileinfo
    set types { {"All files" *} }
    set file [tk_getOpenFile -filetypes $types -initialdir $fileinfo(open_dir)]
    if {$file != ""} {
	# setting this variable triggers a callback in C code
	# which saves the file. 
	set open_stream_filename $file
	set fileinfo(open_dir) [file dirname $file]
    } 
    # otherwise do nothing.
}

#
################################
#
# Open a device - SPM.

set deviceNames       { "Local SPM" }
set deviceConnections { "nmm_Microscope@127.0.0.1:4580" }

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
                {"All files" *} }
	set file [tk_getSaveFile -filetypes $types \
		-initialfile "log.nms" -initialdir $fileinfo(save_dir)]    
	if {$file != ""} {
	    set open_spm_log_name $file
	    set fileinfo(save_dir) [file dirname $file]
	} 
        # otherwise do nothing - user pressed cancel or didn't enter file name
}

generic_entry $win.open_logfile open_spm_log_name \
	"Auto lab notebook:" ""
$win.open_logfile configure -width 20
button $win.get_logfile_name -text "Choose..." -command choose_logfile

pack $win.open_logfile $win.get_logfile_name  -side left

# Allow the user to save 
proc open_spm_connection {} {
    global deviceNames deviceConnections 
    global chosen_device_index open_spm_device_name

    if { [.open_device_dialog activate] } {
        # User chose a device, translate it into a vrpn device name for Nano
        set open_spm_device_name [lindex $deviceConnections $chosen_device_index]
        puts "$open_spm_device_name"
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
    global export_plane export_filetype export_filename fileinfo imageNames
    # Trigger the export_filetype widget to display formats for
    # the default selected export_plane.
    set export_plane [lindex $imageNames 0]
    if { [.save_plane_dialog activate] } {
	set types { {"All files" *} }
	set file [tk_getSaveFile -filetypes $types \
		-initialfile "$export_plane.tfr" -initialdir $fileinfo(save_dir)]    
	if {$file != ""} {
	    puts "Save plane data: $file $export_plane $export_filetype"
	    # setting this variable triggers a callback in C code
	    # which saves the file. 
	    set export_filename $file
	    set fileinfo(save_dir) [file dirname $file]
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
    global fileinfo
    .save_screen_dialog deactivate 1
    set types { {"All files" *} }
    set file [tk_getSaveFile -filetypes $types \
		-initialfile screenimage.tif -initialdir $fileinfo(save_dir)] 
    if {$file != ""} {
	puts "Save screenshot: $file $screenImage_format"
	update idletasks
	after idle {
	# Setting this variable triggers a callback which saves the file.
	set screenImage_filename $file
	# Turn the screen information back on whatever happens
	set chart_junk 1
	}
	set fileinfo(save_dir) [file dirname $file]
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
iwidgets::dialog .save_mod_dialog -title "Save modify data" -modality application

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

# Allow the user to save 
proc save_mod_dialog {} {
    global  nmInfo mod_data which_mod_to_save mod_data_time_list 
    global tcl_platform fileinfo

    set mod_data_time_list [array names mod_data]

    if { [.save_mod_dialog activate] } {
	set types { {"All files" *} }
	set file [tk_getSaveFile -filetypes $types \
		-initialfile $which_mod_to_save.mod -initialdir $fileinfo(save_dir)]
	if {$file != ""} {
	    puts $file
	    set modfile [open $file w]
	    puts -nonewline $modfile $mod_data($which_mod_to_save)
	    flush $modfile
	    close $modfile
	    set fileinfo(save_dir) [file dirname $file]
	    if {$tcl_platform(platform) == "unix"} {
		exec unix_to_dos $file
	    }

	} 
	# otherwise do nothing - user pressed cancel or didn't enter file name
    } else {
	# user pressed "cancel" so do nothing
    }
}	


