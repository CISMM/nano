#!/bin/sh
# ----------------------------------------------------------------------
#  AFM tip display interface
# ----------------------------------------------------------------------
#\
exec wish "$0" ${2+"$@"}

# Import Itcl and Iwidgets
# This import method is a BUG - we shouldn't have to import
# the itcl and itk namespaces, but at least we can use it...
package require Itcl 
catch {namespace import itcl::*}
package require Itk 
catch {namespace import itk::*}
package require Iwidgets
# the extended file selection dialog does not work properly if 
# the iwidget namespace is imported.
#catch {namespace import -force iwidgets::*}
# Unfortunately, iwidgets and BLT have a namespace conflict 
# with the command "clock" - requires the -force option to be successful.

# GLOBAL variables
# ----------------------------------------------------------------------
global afm_sem_window_open 

set nmInfo(afm_sem) [create_closing_toplevel_with_notify \
                            afm_sem_win afm_sem_window_open]

set afm_sem_registration_mode_model_SEM 1
set afm_sem_registration_mode_AFM_SEM_contact 2
set afm_sem_registration_mode_AFM_SEM_free 3

set afm_sem_registration_mode 1

set contact_point_list {none}
set free_point_list {none}

set afm_sem_more_data_needed 1
set afm_sem_model_sem_points_needed 1
set afm_sem_contact_points_needed 1
set afm_sem_free_points_needed 1
# ------------------------------------------------- end GLOBAL variables

iwidgets::labeledframe $nmInfo(afm_sem).calibration -labeltext "Calibration"
pack $nmInfo(afm_sem).calibration -anchor nw -padx 3 -pady 3

set calibration [$nmInfo(afm_sem).calibration childsite]

frame $calibration.status -relief solid -borderwidth 2
label $calibration.status.title -text "Summary Status: "
label $calibration.status.variable -text ""

set afm_sem_status_label $calibration.status.variable
pack $calibration.status -anchor nw -padx 3 -pady 3 -fill x
pack $calibration.status.title -side left -fill x
pack $calibration.status.variable -side left -fill x

# set up the three registration mode frames:
iwidgets::labeledframe $calibration.model_sem \
        -labeltext "model<-->SEM"
pack $calibration.model_sem -anchor nw -padx 3 -pady 3 -fill x

set model_sem [$calibration.model_sem childsite]

iwidgets::labeledframe $calibration.afm_sem_contact \
        -labeltext "AFM<-->SEM contact"
pack $calibration.afm_sem_contact -anchor nw -padx 3 -pady 3 -fill x

set afm_sem_contact [$calibration.afm_sem_contact childsite]

iwidgets::labeledframe $calibration.afm_sem_free \
        -labeltext "AFM<-->SEM free"
pack $calibration.afm_sem_free -anchor nw -padx 3 -pady 3 -fill x

set afm_sem_free [$calibration.afm_sem_free childsite]

button $calibration.generate_test_data \
      -text "Make Test Data" -command {set afm_sem_generate_test_data 1}
pack $calibration.generate_test_data -fill x

button $calibration.update_solution \
      -text "Update Solution" -command {set afm_sem_update_solution 1}
pack $calibration.update_solution -fill x

###################################################################
# model_sem controls
#-------------------
radiobutton $model_sem.mode_enable_button \
      -variable afm_sem_registration_mode -value 1 -justify left -relief solid
pack $model_sem.mode_enable_button -side left -fill y

generic_optionmenu \
  $model_sem.model_selector afm_sem_model "model" imageNames
pack $model_sem.model_selector

generic_optionmenu \
  $model_sem.sem_image_selector afm_sem_sem_image "SEM image" imageNames
pack $model_sem.sem_image_selector

frame $model_sem.status
label $model_sem.status.title -text "Status: "
label $model_sem.status.variable -text ""
set afm_sem_model_sem_status_label $model_sem.status.variable
pack $model_sem.status -anchor nw -padx 3 -pady 3
pack $model_sem.status.title -side left
pack $model_sem.status.variable -side left

###################################################################
# afm_sem_contact controls
#-------------------------
radiobutton $afm_sem_contact.mode_enable_button \
      -variable afm_sem_registration_mode -value 2 -justify left -relief solid
pack $afm_sem_contact.mode_enable_button -side left -fill y

button $afm_sem_contact.add_point_button \
      -text "Add Point" -command { set add_contact_point 1 }
button $afm_sem_contact.delete_point_button \
      -text "Delete Point" -command { set delete_contact_point 1 }
pack $afm_sem_contact.add_point_button $afm_sem_contact.delete_point_button

generic_optionmenu \
  $afm_sem_contact.point_selector afm_sem_current_contact_point \
      "Current point" contact_point_list

pack $afm_sem_contact.point_selector

frame $afm_sem_contact.status
label $afm_sem_contact.status.title -text "Status: "
label $afm_sem_contact.status.variable -text ""
set afm_sem_contact_status_label $afm_sem_contact.status.variable
pack $afm_sem_contact.status -anchor nw -padx 3 -pady 3
pack $afm_sem_contact.status.title -side left
pack $afm_sem_contact.status.variable -side left

###################################################################
# afm_sem_free controls
#----------------------
radiobutton $afm_sem_free.mode_enable_button \
      -variable afm_sem_registration_mode -value 3 -justify left -relief solid
pack $afm_sem_free.mode_enable_button -side left -fill y

button $afm_sem_free.add_point_button \
      -text "Add Point" -command { set add_free_point 1 }
button $afm_sem_free.delete_point_button \
      -text "Delete Point" -command { set delete_free_point 1 }
pack $afm_sem_free.add_point_button $afm_sem_free.delete_point_button

generic_optionmenu \
  $afm_sem_free.point_selector afm_sem_current_free_point \
      "Current point" free_point_list

pack $afm_sem_free.point_selector

frame $afm_sem_free.status
label $afm_sem_free.status.title -text "Status: "
label $afm_sem_free.status.variable -text ""
set afm_sem_free_status_label $afm_sem_free.status.variable
pack $afm_sem_free.status -anchor nw -padx 3 -pady 3
pack $afm_sem_free.status.title -side left
pack $afm_sem_free.status.variable -side left

###################################################################

proc handle_sufficient_data_change {label varname element op} {
  upvar $varname statusVar
  if {$statusVar== 0} {
    $label configure -text "Sufficient Data" -fg green
  } else {
    $label configure -text "Insufficient Data" -fg red
  }
}

trace variable afm_sem_model_sem_points_needed \
   w "handle_sufficient_data_change $afm_sem_model_sem_status_label"
trace variable afm_sem_contact_points_needed \
   w "handle_sufficient_data_change $afm_sem_contact_status_label"
trace variable afm_sem_free_points_needed \
   w "handle_sufficient_data_change $afm_sem_free_status_label"
trace variable afm_sem_more_data_needed \
   w "handle_sufficient_data_change $afm_sem_status_label"

set afm_sem_model_sem_points_needed 1
set afm_sem_contact_points_needed 1
set afm_sem_free_points_needed 1
set afm_sem_more_data_needed 1 
