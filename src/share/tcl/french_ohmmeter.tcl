#!/bin/sh
# ----------------------------------------------------------------------
#  ohmmeter interface
# ----------------------------------------------------------------------
#\
exec wish "$0" ${2+"$@"}

catch { set auto_path [lappend auto_path $env(ITCL_LIBRARY)] }
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
# with the command "clock" - requires the -force option to be successful

# This needs to be made dependent on how big the font is on the screen.
catch { option add *font {helvetica -15 } startupFile}
catch { option add *Font {helvetica -15 } startupFile}

if {[catch {set tcl_script_dir $env(NM_TCL_DIR) }] } {
    set tcl_script_dir .
}

source [file join ${tcl_script_dir} Tcl_Linkvar_widgets.tcl]

#
# basic layout:
#  -----------------------------
#  | ch 0 | ch 1 | ch 2 | ch 3 |  
#  |      |      |      |      |
#  |      |      |      |      |
#  -----------------------------

global french_ohmmeter_open

global french_ohmmeter_voltages french_ohmmeter_ranges french_ohmmeter_filters

set french_ohmmeter_voltages {none }
set french_ohmmeter_ranges {none }
set french_ohmmeter_filters {none }

set french_ohmmeter [create_closing_toplevel_with_notify french_ohmmeter french_ohmmeter_open "French Ohmmeter"]

frame $french_ohmmeter.channel_settings

pack $french_ohmmeter.channel_settings -side top

proc indicateError {window name1 name2 op} {
    upvar #0 $name1 errorval
#    puts "indicateError $name1 $errorval $op"
    if {$errorval == 1} {
      $window configure -background LightPink1
    } else {
      $window configure -background grey
    }
}

#######################################################
# Here is what goes in the panel for each channel 
#######################################################

proc create_channel_panel {parent channel_name} {

  global french_ohmmeter_enable_$channel_name \
         french_ohmmeter_autorange_$channel_name \
         french_ohmmeter_voltage_$channel_name \
         french_ohmmeter_range_$channel_name \
         french_ohmmeter_filter_$channel_name \
         french_ohmmeter_resistance_$channel_name \
         french_ohmmeter_status_$channel_name \
         french_ohmmeter_error_$channel_name

  set french_ohmmeter_resistance_$channel_name "0.000"
  set french_ohmmeter_status_$channel_name "unknown"

  set temp [frame $parent.$channel_name -relief raised -borderwidth 3]
  label $parent.$channel_name.label -text $channel_name

  trace variable french_ohmmeter_error_$channel_name w \
   "indicateError $parent.$channel_name"

  checkbutton $parent.$channel_name.selector \
         -variable french_ohmmeter_enable_$channel_name -text "enabled"

  checkbutton $parent.$channel_name.autorange \
         -variable french_ohmmeter_autorange_$channel_name -text "autorange"

  generic_optionmenu_with_index $parent.$channel_name.voltage \
     french_ohmmeter_voltage_$channel_name "voltage" french_ohmmeter_voltages

  generic_optionmenu_with_index $parent.$channel_name.range \
     french_ohmmeter_range_$channel_name "range" french_ohmmeter_ranges

  generic_optionmenu_with_index $parent.$channel_name.filter \
     french_ohmmeter_filter_$channel_name "filter" french_ohmmeter_filters

  frame $parent.$channel_name.resistance -relief solid -borderwidth 2
  label $parent.$channel_name.resistance.label -text "resistance:"
  label $parent.$channel_name.resistance.value \
        -textvariable french_ohmmeter_resistance_$channel_name
  pack $parent.$channel_name.resistance.label -side left
  pack $parent.$channel_name.resistance.value -side left

  frame $parent.$channel_name.status -relief solid -borderwidth 2
  label $parent.$channel_name.status.label -text "status:"
  label $parent.$channel_name.status.value \
        -textvariable french_ohmmeter_status_$channel_name
  pack $parent.$channel_name.status.label -side left
  pack $parent.$channel_name.status.value -side left

  # now pack the main components
  pack $parent.$channel_name.label -side top -anchor n

  pack $parent.$channel_name.selector $parent.$channel_name.autorange \
     $parent.$channel_name.voltage \
     $parent.$channel_name.range \
     $parent.$channel_name.filter -side top -anchor e

  pack $parent.$channel_name.resistance -side top -fill both
  pack $parent.$channel_name.status -side top -fill both

  return $parent.$channel_name
}

set ch0_panel [create_channel_panel $french_ohmmeter.channel_settings ch0]
set ch1_panel [create_channel_panel $french_ohmmeter.channel_settings ch1]
set ch2_panel [create_channel_panel $french_ohmmeter.channel_settings ch2]
set ch3_panel [create_channel_panel $french_ohmmeter.channel_settings ch3]

pack $ch0_panel -side left -fill both -padx 5
pack $ch1_panel -side left -fill both -padx 5
pack $ch2_panel -side left -fill both -padx 5
pack $ch3_panel -side left -fill both -padx 5
