set nmInfo(import_objects) [create_closing_toplevel import_objects "Import Objects" ]

set import_dir ""
set import_file_label "none"
set imported_objects {none }

set import_transx 0
set import_lock_transx 0
set import_transy 0
set import_lock_transy 0
set import_transz 0
set import_lock_transz 0
set import_tune_trans 0
set import_scale 1
set import_rotx 0
set import_lock_rotx 0
set import_roty 0
set import_lock_roty 0
set import_rotz 0
set import_lock_rotz 0
set import_tune_rot 0
set import_type ""
set import_visibility 1
set import_proj_text 1
set import_CCW 1
set import_tess 10
set import_axis_step 10
set import_lock_object 0
set import_lock_texture 0
set import_update_AFM 0
set import_grab_object 0

set spider_which_leg {none }
set spider_length 5
set spider_width 2
set spider_thick 0.1
set spider_tess 10
set spider_curve 0
set spider_legs 8
set spider_filename ""

set import_color gray
set import_r 192
set import_g 192
set import_b 192
set import_color_changed 0

iwidgets::Labeledframe $nmInfo(import_objects).basics \
	-labeltext "Import Models" \
	-labelpos nw
set nmInfo(basic_options) [$nmInfo(import_objects).basics childsite]

pack $nmInfo(import_objects).basics -side top -fill x

frame $nmInfo(basic_options).file
frame $nmInfo(basic_options).file.buttons -bd 3 -relief groove
frame $nmInfo(basic_options).file.buttons.f1
frame $nmInfo(basic_options).file.buttons.f1.f1

generic_optionmenu $nmInfo(basic_options).file.buttons.imported_files current_object\
	"Current Object" imported_objects

#label $nmInfo(basic_options).file.import_file_label -justify left -text \
#	"Current Imported file: $import_file_label"

button $nmInfo(basic_options).file.buttons.f1.import_button -text "Import File" -command open_import_file
radiobutton $nmInfo(basic_options).file.buttons.f1.f1.ccw -text "CCW" -variable import_CCW -value 1
radiobutton $nmInfo(basic_options).file.buttons.f1.f1.cw -text "CW" -variable import_CCW -value 0
button $nmInfo(basic_options).file.buttons.f1.close_button -text "Close File" -command close_import_file
#button $nmInfo(basic_options).file.buttons.close_button -text "Close File" import_close_file 

# Spider Stuff
button $nmInfo(basic_options).file.buttons.spider_create_button -text "Create Spider" \
	-command create_spider

set nmInfo(spider_control) [create_closing_toplevel spider_control "Spider Control" ]

generic_optionmenu $nmInfo(spider_control).spider_which_leg_menu spider_current_leg \
	"Current Leg" spider_which_leg

floatscale $nmInfo(spider_control).spider_length_slide 0 20 1000 1 1 \
	spider_length "Spider Length"

floatscale $nmInfo(spider_control).spider_width_slide 0 10 1000 1 1 \
	spider_width "Spider Width"

floatscale $nmInfo(spider_control).spider_thick_slide 0 5 1000 1 1 \
	spider_thick "Spider Thickness"

intscale $nmInfo(spider_control).spider_tess_slide 1 50 1000 1 1 \
	spider_tess "Spider Tesselation"

floatscale $nmInfo(spider_control).spider_curve_slide 0 90 1000 1 1 \
	spider_curve "Spider Curvature"

intscale $nmInfo(spider_control).spider_legs_slide 1 8 1000 1 1 \
	spider_legs "Spider Legs"

button $nmInfo(spider_control).spider_save_to_file -text "Save Spider" \
        -command save_spider

#

generic_entry $nmInfo(basic_options).file.buttons.import_tess import_tess \
     "Tube Loading Tesselation" integer
generic_entry $nmInfo(basic_options).file.buttons.import_axis_step import_axis_step \
     "Tube Loading Axis Step" integer






#generic_entry $nmInfo(basic_options).modelFile modelFile "Enter the file to import:" ""

frame $nmInfo(basic_options).f1
frame $nmInfo(basic_options).f2
frame $nmInfo(basic_options).f1.f1 -bd 3 -relief groove
generic_entry $nmInfo(basic_options).f1.import_scale import_scale \
     "Scale" real 
floatscale $nmInfo(basic_options).f1.import_scale_slide 0.1 1000.0 100 1 1 \
	import_scale "Scale"

#generic_entry $nmInfo(basic_options).f1.import_transx import_transx \
#     "X Translation" real
floatscale $nmInfo(basic_options).f1.import_transx_slide -1000.0 6000.0 100 1 1 \
	import_transx "X Translation"
checkbutton $nmInfo(basic_options).f1.import_lock_transx_button \
    -text "Lock X Translation" -variable import_lock_transx
#generic_entry $nmInfo(basic_options).f1.import_transy import_transy \
#     "Y Translation" real
floatscale $nmInfo(basic_options).f1.import_transy_slide -1000.0 6000.0 100 1 1 \
	import_transy "Y Translation"
checkbutton $nmInfo(basic_options).f1.import_lock_transy_button \
    -text "Lock Y Translation" -variable import_lock_transy
#generic_entry $nmInfo(basic_options).f1.import_transz import_transz \
#     "Z Translation" real
floatscale $nmInfo(basic_options).f1.import_transz_slide -1000.0 6000.0 100 1 1 \
	import_transz "Z Translation"
checkbutton $nmInfo(basic_options).f1.import_lock_transz_button \
    -text "Lock Z Translation" -variable import_lock_transz
checkbutton $nmInfo(basic_options).f1.import_tune_trans_button \
    -text "Fine Tune Translations" -variable import_tune_trans
button $nmInfo(basic_options).f1.import_lock_transall_button \
    -text "Lock All Translations" -command lock_transall


checkbutton $nmInfo(basic_options).f1.import_update_AFM \
     -text "Update AFM" -variable import_update_AFM
checkbutton $nmInfo(basic_options).f1.import_grab_object \
     -text "Grab Object" -variable import_grab_object


#generic_entry $nmInfo(basic_options).f2.import_rotx import_rotx \
#     "X Rotation" real
floatscale $nmInfo(basic_options).f2.import_rotx_slide -360 360 1000 1 1 \
	import_rotx "X Rotation"
checkbutton $nmInfo(basic_options).f2.import_lock_rotx_button \
    -text "Lock X Rotation" -variable import_lock_rotx
#generic_entry $nmInfo(basic_options).f2.import_roty import_roty \
#     "Y Rotation" real
floatscale $nmInfo(basic_options).f2.import_roty_slide -360 360 1000 1 1 \
	import_roty "Y Rotation"
checkbutton $nmInfo(basic_options).f2.import_lock_roty_button \
    -text "Lock Y Rotation" -variable import_lock_roty
#generic_entry $nmInfo(basic_options).f2.import_rotz import_rotz \
#     "Z Rotation" real
floatscale $nmInfo(basic_options).f2.import_rotz_slide -360 360 1000 1 1 \
	import_rotz "Z Rotation"
checkbutton $nmInfo(basic_options).f2.import_lock_rotz_button \
    -text "Lock Z Rotation" -variable import_lock_rotz
checkbutton $nmInfo(basic_options).f2.import_tune_rot_button \
    -text "Fine Tune Rotations" -variable import_tune_rot
button $nmInfo(basic_options).f2.import_lock_rotall_button \
    -text "Lock All Rotations" -command lock_rotall
button $nmInfo(basic_options).f2.import_reset_object -text "Reset Object" -command reset_object


button $nmInfo(basic_options).f1.f1.visibility_button -text "Hide" -command change_visibility
checkbutton $nmInfo(basic_options).f1.f1.proj_text_button \
    -text "Show Projective Texture" -variable import_proj_text
checkbutton $nmInfo(basic_options).f1.f1.lock_object_button \
    -text "Lock Object to Projective Texture" -variable import_lock_object
checkbutton $nmInfo(basic_options).f1.f1.lock_texture_button \
    -text "Lock Projective Texture to Object" -variable import_lock_texture
button $nmInfo(basic_options).f1.f1.set_color \
        -text "Set color" -command {
            if {[choose_color import_color "Choose color" $nmInfo(basic_options)] } {
                $nmInfo(basic_options).f1.f1.colorsample configure -bg $import_color 
                set_import_color
            }
        }  
button $nmInfo(basic_options).f1.f1.colorsample \
        -relief groove -bd 2 -bg $import_color \
        -command { $nmInfo(basic_options).f1.f1.set_color invoke}


#pack $nmInfo(basic_options).modelFile -side top -anchor w -padx 1m -pady 1m -fill x
pack $nmInfo(basic_options).file -anchor w -fill x
#pack $nmInfo(basic_options).file.import_file_label -anchor w -side left
pack $nmInfo(basic_options).file.buttons -anchor w -fill x
pack $nmInfo(basic_options).file.buttons.imported_files -anchor nw -padx 1m -pady 1m
pack $nmInfo(basic_options).file.buttons.f1 -anchor nw
pack $nmInfo(basic_options).file.buttons.f1.import_button -anchor nw -side left -padx 1m -pady 1m
pack $nmInfo(basic_options).file.buttons.f1.f1 -anchor nw -side left
pack $nmInfo(basic_options).file.buttons.f1.f1.ccw -anchor nw 
pack $nmInfo(basic_options).file.buttons.f1.f1.cw -anchor nw 
pack $nmInfo(basic_options).file.buttons.f1.close_button -anchor nw -padx 1m -pady 1m
# Spider Stuff
pack $nmInfo(basic_options).file.buttons.spider_create_button -anchor nw -padx 1m -pady 1m

pack $nmInfo(spider_control).spider_which_leg_menu -padx 1m -pady 1m -anchor nw
pack $nmInfo(spider_control).spider_length_slide -padx 1m -pady 1m -anchor nw
pack $nmInfo(spider_control).spider_width_slide -padx 1m -pady 1m -anchor nw
pack $nmInfo(spider_control).spider_thick_slide -padx 1m -pady 1m -anchor nw
pack $nmInfo(spider_control).spider_tess_slide -padx 1m -pady 1m -anchor nw
pack $nmInfo(spider_control).spider_curve_slide -padx 1m -pady 1m -anchor nw
pack $nmInfo(spider_control).spider_legs_slide -padx 1m -pady 1m -anchor nw
pack $nmInfo(spider_control).spider_save_to_file -padx 1m -pady 1m -anchor nw
#
pack $nmInfo(basic_options).file.buttons.import_tess -anchor nw -padx 1m -pady 1m
pack $nmInfo(basic_options).file.buttons.import_axis_step -anchor nw -padx 1m -pady 1m

pack $nmInfo(basic_options).f1 -anchor nw -side left -pady 7m -fill x
pack $nmInfo(basic_options).f2 -anchor nw -pady 22m -fill x

#pack $nmInfo(basic_options).f1.import_scale -anchor nw -padx 1m -pady 1m
pack $nmInfo(basic_options).f1.import_scale_slide -anchor nw -padx 1m -pady 1m
#pack $nmInfo(basic_options).f1.import_transx -anchor nw -padx 1m -pady 1m
pack $nmInfo(basic_options).f1.import_transx_slide -anchor nw -padx 1m -pady 1m
pack $nmInfo(basic_options).f1.import_lock_transx_button -anchor nw -padx 1m -pady 1m
#pack $nmInfo(basic_options).f1.import_transy -anchor nw -padx 1m -pady 1m
pack $nmInfo(basic_options).f1.import_transy_slide -anchor nw -padx 1m -pady 1m
pack $nmInfo(basic_options).f1.import_lock_transy_button -anchor nw -padx 1m -pady 1m
#pack $nmInfo(basic_options).f1.import_transz -anchor nw -padx 1m -pady 1m
pack $nmInfo(basic_options).f1.import_transz_slide -anchor nw -padx 1m -pady 1m
pack $nmInfo(basic_options).f1.import_lock_transz_button -anchor nw -padx 1m -pady 1m
pack $nmInfo(basic_options).f1.import_tune_trans_button -anchor nw -padx 1m -pady 1m
pack $nmInfo(basic_options).f1.import_lock_transall_button -anchor nw -padx 1m -pady 1m
pack $nmInfo(basic_options).f1.import_update_AFM -anchor sw -padx 1m -pady 1m -fill x
pack $nmInfo(basic_options).f1.import_grab_object -anchor sw -padx 1m -pady 1m -fill x
pack $nmInfo(basic_options).f1.f1 -anchor nw -padx 1m -pady 1m -fill x
pack $nmInfo(basic_options).f1.f1.visibility_button -anchor nw -padx 1m -pady 1m -fill x
pack $nmInfo(basic_options).f1.f1.proj_text_button -anchor nw -padx 1m -pady 1m -fill x
pack $nmInfo(basic_options).f1.f1.lock_object_button -anchor sw -padx 1m -pady 1m -fill x
pack $nmInfo(basic_options).f1.f1.lock_texture_button -anchor sw -padx 1m -pady 1m -fill x
pack $nmInfo(basic_options).f1.f1.set_color -anchor nw -side left -fill x
pack $nmInfo(basic_options).f1.f1.colorsample -anchor nw -side left -fill x -expand yes


#pack $nmInfo(basic_options).f2.import_rotx -anchor nw -padx 1m -pady 1m
pack $nmInfo(basic_options).f2.import_rotx_slide -anchor nw -padx 1m -pady 1m
pack $nmInfo(basic_options).f2.import_lock_rotx_button -anchor nw -padx 1m -pady 1m
#pack $nmInfo(basic_options).f2.import_roty -anchor nw -padx 1m -pady 1m
pack $nmInfo(basic_options).f2.import_roty_slide -anchor nw -padx 1m -pady 1m
pack $nmInfo(basic_options).f2.import_lock_roty_button -anchor nw -padx 1m -pady 1m
#pack $nmInfo(basic_options).f2.import_rotz -anchor nw -padx 1m -pady 1m
pack $nmInfo(basic_options).f2.import_rotz_slide -anchor nw -padx 1m -pady 1m
pack $nmInfo(basic_options).f2.import_lock_rotz_button -anchor nw -padx 1m -pady 1m
pack $nmInfo(basic_options).f2.import_tune_rot_button -anchor nw -padx 1m -pady 1m
pack $nmInfo(basic_options).f2.import_lock_rotall_button -anchor nw -padx 1m -pady 1m
pack $nmInfo(basic_options).f2.import_reset_object -anchor sw -padx 1m -pady 1m -fill x



trace variable import_type w SetupOptions

#
################################
# Open an import file. 
proc open_import_file {} {
    global modelFile nmInfo import_dir import_file_label current_object current_object_new
    set types { {"Wave Front files" ".obj" } 
                {"MSI files" ".msi" } 
		{"Text files" ".txt" }
	{"NAMS spider files" ".spi" }
                {"All files" *} }
    set filename [tk_getOpenFile -filetypes $types \
            -initialdir $import_dir \
            -title "Import a file"]
    if {$filename != ""} {
	# setting this variable triggers a callback in C code
	# which saves the file. 
        # dialog check whether file exists.
        set modelFile $filename
        set import_dir [file dirname $filename]

        regexp -indices {[^/]*$} $modelFile model_ind
        set m_start [lindex [split $model_ind] 0]
        set m_end [lindex [split $model_ind] 1]
    
        set import_file_label [string range $modelFile $m_start $m_end]
    } else {
        set modelFile ""
        set import_file_label "NONE"
    }
#    $nmInfo(basic_options).file.import_file_label configure -text \
#	"Current Imported file: $import_file_label"

    if { [string first $filename ".spi"] != 0 } {
        "show.spider_control"
    }


 	set current_object_new $import_file_label
}


# Create a spider
proc create_spider {} {
	global modelFile import_file_label current_object current_object_new

	"show.spider_control"

	set filename "/spider.spi"

      set modelFile $filename

	set current_object_new "spider.spi"
}

# Save spider data
proc save_spider {} {
        global fileinfo spider_filename reg_surface_cm color_comes_from
        
        set initfile [string range $reg_surface_cm(color_comes_from) 0 end-4]
        set ext ".spi"
        set types { {"TAMS spider" ".spi"} }

        # Let the user choose a file to save the data in.
        # Set the default name to the current registration image 
	set filename [tk_getSaveFile -filetypes $types \
		-initialfile "$initfile$ext"\
                -initialdir $fileinfo(save_dir)\
                -title "Save spider data"]
	if {$filename != ""} { 
	    set spider_filename $filename
	    set fileinfo(save_dir) [file dirname $filename]
        }
}


proc close_import_file {} {
    global modelFile nmInfo import_file_label

    set modelFile ""
    set import_file_label "all"

#    $nmInfo(basic_options).file.import_file_label configure -text \
#	"Current Imported file: $import_file_label"

#    destroy $nmInfo(import_objects).options
}

proc reset_object {} {
	global nmInfo
	global import_scale
	global import_transx
	global import_transy
	global import_transz
	global import_rotx
	global import_roty
	global import_rotz

	set import_scale 1
	set import_transx 0
	set import_transy 0
	set import_transz 0
	set import_rotx 0
	set import_roty 0
	set import_rotz 0
}

proc lock_transall {} {
	global import_lock_transx
	global import_lock_transy
	global import_lock_transz

	set import_lock_transx 1
	set import_lock_transy 1
	set import_lock_transz 1
}

proc lock_rotall {} {
	global import_lock_rotx
	global import_lock_roty
	global import_lock_rotz

	set import_lock_rotx 1
	set import_lock_roty 1
	set import_lock_rotz 1
}




proc set_import_color {} {
    global import_r import_g import_b import_color_changed import_color
    global nmInfo

    # Extract three component colors of contour_color 
    # and save into import_r g b
    scan $import_color #%02x%02x%02x import_r import_g import_b
    set import_color_changed 1
}


##This procedure changes the visibility_mode from hide to show (set it up
# so that the object will be drawn)
proc change_visibility {} {
  global nmInfo
  global import_visibility
   
  if { $import_visibility == 1 } {
    ##Change the hide button to a show button
    $nmInfo(basic_options).f1.f1.visibility_button configure -text "Show"
    set import_visibility 0
  } else { 
    ##Change the show button to a hide button  
    $nmInfo(basic_options).f1.f1.visibility_button configure -text "Hide"
    set import_visibility 1
  }
}

proc SetupOptions {name el op} {
    global import_type
    global nmInfo

    destroy $nmInfo(import_objects).options

    if { [string compare $import_type "msi"] == 0 } {
        StandardFileOptions
        MSIFileOptions
    } 
}

proc StandardFileOptions {} {
    global nmInfo

    iwidgets::Labeledframe $nmInfo(import_objects).options \
	    -labeltext "Options" \
	    -labelpos nw
    set nmInfo(adv_options) [$nmInfo(import_objects).options childsite]

    pack $nmInfo(import_objects).options -side top -fill x
}

# MSI Specific variables

set msi_bond_mode 0
set msi_atom_radius 1

proc MSIFileOptions {} {
    global nmInfo
    global msi_color
    global msi_atom_radius
    global msi_bond_mode
    
    #Reset some variables
    #set msi_atom_radius 1
    set msi_bond_mode 0
        
    frame $nmInfo(adv_options).f
        
    button $nmInfo(adv_options).f.bond_mode -text "Atoms" -command change_bond_mode

    pack $nmInfo(adv_options).f -side top -anchor nw
    pack $nmInfo(adv_options).f.bond_mode -side left -padx 1m
}

##This procedure changes the import_mode from spheres to bonds (set it up
# so that the nanotube's bonds will be drawn, rather than its atoms)
proc change_bond_mode {} {
  global nmInfo
  global msi_bond_mode
  global msi_atom_radius

  if { $msi_bond_mode == 0 } {
    ##Change the bonds button to an atoms button
    set msi_bond_mode 1

    $nmInfo(adv_options).f.bond_mode configure -text "Bonds"
    generic_entry $nmInfo(adv_options).f.atom_radius msi_atom_radius "Atom Radius" real
    pack $nmInfo(adv_options).f.atom_radius -side left -padx 1m

  } else {
    set msi_bond_mode 0
    set msi_switch_bond_mode 0

    ##Change the atoms button to an bonds button
    $nmInfo(adv_options).f.bond_mode configure -text "Atoms"

    destroy $nmInfo(adv_options).f.atom_radius
  }
}
