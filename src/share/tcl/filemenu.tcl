#
################################
#
# Open a static file. 
# It should open any type of file handled by BCGrid
# Eventually, it should allow opening of stream files. 
proc open_static_file {} {
    global open_static_filename
    set types { {"All files" *} }
    set file [tk_getOpenFile -filetypes $types -initialfile plane.tfr ]    
    if {$file != ""} {
	# setting this variable triggers a callback in C code
	# which saves the file. 
	set open_static_filename $file
    } 
    # otherwise do nothing.
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
    global export_plane export_filetype export_filename
    if { [.save_plane_dialog activate] } {
	set types { {"All files" *} }
	set file [tk_getSaveFile -filetypes $types \
		-initialfile "$export_plane.tfr" ]    
	if {$file != ""} {
	    puts "Save plane data: $file $export_plane $export_filetype"
	    # setting this variable triggers a callback in C code
	    # which saves the file. 
	    set export_filename $file
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
	.save_screen_dialog deactivate 1
    set types { {"All files" *} }
    set file [tk_getSaveFile -filetypes $types \
		-initialfile screenimage.tif ] 
    if {$file != ""} {
	puts "Save screenshot: $file $screenImage_format"
	update idletasks
	after idle {
	# Setting this variable triggers a callback which saves the file.
	set screenImage_filename $file
	# Turn the screen information back on whatever happens
	set chart_junk 1
	}

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
    global tcl_platform

    set mod_data_time_list [array names mod_data]

    if { [.save_mod_dialog activate] } {
	set types { {"All files" *} }
	set file [tk_getSaveFile -filetypes $types \
		-initialfile $which_mod_to_save.mod ]
	if {$file != ""} {
	    puts $file
	    set modfile [open $file w]
	    puts -nonewline $modfile $mod_data($which_mod_to_save)
	    flush $modfile
	    close $modfile
	    if {$tcl_platform(platform) == "unix"} {
		exec unix_to_dos $filename
	    }

	} 
	# otherwise do nothing - user pressed cancel or didn't enter file name
    } else {
	# user pressed "cancel" so do nothing
    }
}	


