#!/bin/sh
# ----------------------------------------------------------------------
#  tem interface
# ----------------------------------------------------------------------
#\exec wish "$0" ${2+"$@"}


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

set nmInfo(tem) [create_closing_toplevel tem_win "TEM"]
# ------------------------------------------------- end GLOBAL variables

iwidgets::labeledframe $nmInfo(tem).acquisition -labeltext "TEM acquisition"
pack $nmInfo(tem).acquisition -anchor nw -padx 3 -pady 3

set nmInfo(tem_acq) [$nmInfo(tem).acquisition childsite]
# Button to start the scan
button $nmInfo(tem_acq).acquire_image -text "Acquire Image" \
	-command { set tem_acquire_image 1 }
pack $nmInfo(tem_acq).acquire_image -anchor w

