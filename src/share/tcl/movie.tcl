#Initial dialog which allows user to choose which plane
#of data to save.
iwidgets::dialog .save_sequence_dialog -title "Save sequence image" 
#-modality application

.save_sequence_dialog hide Help
.save_sequence_dialog hide Apply
.save_sequence_dialog buttonconfigure OK -text "Go" -command {
#    .save_sequence_dialog deactivate 1
    global num_images_so_far time_so_far base_time end_time 
    global num_images time_step sequenceImage_basefile 

    set num_images_so_far 0
    set time_so_far $base_time
    set num_images [expr ceil(($end_time - $base_time)/$time_step)]
    puts " num img $num_images"
    if {$num_images <= 0} {
        set end_time [expr $base_time + 1]
        nano_warning "End time must be greater than start time."
        return;
    }
    if {$sequenceImage_basefile == "" } {
        nano_warning "Please choose a base file to save to."
        return;
    }  

    save_an_image
}

.save_sequence_dialog buttonconfigure Cancel -text "Done" -command {
    .save_sequence_dialog deactivate 0
    # Cancel any pending image saves, does nothing if "Go" hasn't been pressed.
    after cancel save_an_image 
    # Turn the screen information back on whatever happens
    set chart_junk 1
}

set sequenceImage_types { {"All files" *} 
    {"TIFF" ".tif" }
    {"JPG" ".jpg" }
    {"BMP" ".bmp" }
    {"PGM" ".pgm" } 
    {"PPM" ".ppm" } }

set sequenceImage_basefile ""
proc choose_basefile {} {
    global fileinfo sequenceImage_types sequenceImage_format 
    global sequenceImage_basefile

        # Set the file extension correctly
        set def_file_exten ".tif"
        foreach item $sequenceImage_types {
            if { [string compare $sequenceImage_format [lindex $item 0]] == 0} {
                set def_file_exten [lindex $item 1]
            }
        }

    set filename [tk_getSaveFile -filetypes $sequenceImage_types \
		-initialfile sequenceimage$def_file_exten \
                -initialdir $fileinfo(save_dir) \
                -title "Save Image Sequence"] 
    if {$filename != ""} {
	#puts "Save sequence image: $filename $sequenceImage_format"
        # Dialog checks for writeable directory, and asks about
        # replacing existing files. 
        # Must append extension for save to work!
        set sequenceImage_basefile "[file rootname $filename]$def_file_exten"
	set fileinfo(save_dir) [file dirname $filename]
    } else {
	# otherwise do nothing - user pressed cancel or didn't enter file name
    }
}

set win [.save_sequence_dialog childsite]
# Toggle to determine whether the screen labels and decorations
# are visible when the sequence shot is saved. 
set chart_junk 1
set screenImage_format_list {}
checkbutton $win.chart_junk -variable chart_junk \
	-text "Show screen text"
pack $win.chart_junk -anchor nw

set num_images 0
set time_step 1
generic_entry $win.time_step time_step "Time step (sec)" real
set base_time 0
set end_time 100
set time_so_far 0
# Stream must be at the starting time when image save starts. 
# Otherwise incremental update creates bad pictures. 
generic_entry $win.base_time base_time "Start time (sec)" numeric \
        { set set_stream_time $base_time }
generic_entry $win.end_time end_time "Ending time (sec)" numeric
generic_optionmenu $win.sequenceImage_format sequenceImage_format \
	"Format for saved pictures:" screenImage_format_list
pack $win.time_step $win.base_time $win.end_time $win.sequenceImage_format \
        -anchor nw

iwidgets::Labeledwidget::alignlabels \
        $win.sequenceImage_format $win.time_step $win.base_time $win.end_time 

generic_entry $win.sequenceImage_basefile sequenceImage_basefile \
	"Base filename:" ""
$win.sequenceImage_basefile configure -width 20
button $win.get_file_name -text "Choose..." -command choose_basefile
pack $win.sequenceImage_basefile $win.get_file_name -side left

# Make sure chosen filename extension tracks file format changes. 
proc change_sequenceImage_ext {nm el op } {
    global sequenceImage_basefile sequenceImage_types sequenceImage_format

    # Set the file extension correctly
    set def_file_exten ".tif"
    foreach item $sequenceImage_types {
        if { [string compare $sequenceImage_format [lindex $item 0]] == 0} {
            set def_file_exten [lindex $item 1]
        }
    }
    if { $sequenceImage_basefile != "" } {
        set sequenceImage_basefile "[file rootname $sequenceImage_basefile]$def_file_exten"
    }
}
trace variable sequenceImage_format w change_sequenceImage_ext

# Allow the user to save 
proc save_sequenceImage {} {
    global num_images_so_far time_so_far base_time stream_replay_rate 

    set stream_replay_rate 0

    set num_images_so_far 0
    set time_so_far $base_time
    # All activity is done in the button commands defined above.
    .save_sequence_dialog activate
}	

proc save_an_image {} {
    global screenImage_filename num_images time_step sequenceImage_basefile \
            num_images_so_far time_so_far set_stream_time set_stream_time_now

    set stream_replay_rate 0
    if { $num_images_so_far >= $num_images } return;

    set fdir [file dirname $sequenceImage_basefile]
    set froot [file rootname $sequenceImage_basefile]
    set fext [file extension $sequenceImage_basefile]
    set filename [file join $fdir ${froot}[format %04d $num_images_so_far]${fext}]
    puts "$filename "
    update idletasks

    # Setting this variable triggers a callback which saves the file.
    set screenImage_filename $filename

    set time_so_far [expr $time_so_far + $time_step]
    set set_stream_time $time_so_far
    #set set_stream_time_now 1

    incr num_images_so_far
    after 500 save_an_image 
}