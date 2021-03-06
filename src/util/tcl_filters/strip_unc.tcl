#/* The nanoManipulator and its source code have been released under the
# * Boost software license when nanoManipulator, Inc. ceased operations on
# * January 1, 2014.  At this point, the message below from 3rdTech (who
# * sublicensed from nanoManipulator, Inc.) was superceded.
# * Since that time, the code can be used according to the following
# * license.  Support for this system is now through the NIH/NIBIB
# * National Research Resource at cismm.org.
#
#Boost Software License - Version 1.0 - August 17th, 2003
#
#Permission is hereby granted, free of charge, to any person or organization
#obtaining a copy of the software and accompanying documentation covered by
#this license (the "Software") to use, reproduce, display, distribute,
#execute, and transmit the Software, and to prepare derivative works of the
#Software, and to permit third-parties to whom the Software is furnished to
#do so, all subject to the following:
#
#The copyright notices in the Software and this entire statement, including
#the above license grant, this restriction and the following disclaimer,
#must be included in all copies of the Software, in whole or in part, and
#all derivative works of the Software, unless such copies or derivative
#works are solely in the form of machine-executable object code generated by
#a source language processor.
#
#THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
#SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
#FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
#ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
#DEALINGS IN THE SOFTWARE.
#*/

#!/bin/bash
# the next line restarts using wishx (see man wish) \
	exec wish83 "$0" "$@"

#  ===3rdtech===
#  Copyright (c) 2000 by 3rdTech, Inc.
#  All Rights Reserved.
#
#  This file may not be distributed without the permission of 
#  3rdTech, Inc. 
#  ===3rdtech===

set line_string_pre "if \{ !\$"
set line_string_post " \} \{"
#puts "$line_string"

# Read a tcl file and output a new tcl file,
# with things inside the "if { !$thirdtech_ui } { ... } stripped.
proc get_lines {fid out_fid targets} {
    global line_string_pre line_string_post
    set my_command ""
    set found 0
    foreach targ $targets {
        lappend line_strings $line_string_pre$targ$line_string_post
    }
    while {[gets $fid line] >= 0} {
        # ignore leading spaces/tabs. 
        set line1 [string trimleft $line]
        foreach line_string $line_strings {
            if {[string compare -length [string length $line_string] $line_string $line1] == 0 } {
                set found 1
            }
        }
        if { $found } {
            append my_command $line 
            # debug printout
            #puts stderr "$my_command"
            while {![info complete $my_command]} {
                if {[gets $fid line] >= 0} {
                    append my_command $line 
                    #puts stderr "$my_command"
                } else {
                    puts "Unexpected end of file, quitting..."
                    close $fid
                    close $out_fid
                    return
                }
            }
            # Done with this block, get ready for the next block. 
            set my_command ""
            set found 0
	} else {
	    puts $out_fid $line
	}
    } 
    #puts stderr "Close files..."
    close $fid
    close $out_fid
    return
}








#wm withdraw .

#puts "$argv0 $argv"

#if { [llength $argv] != 3 } {
#    puts "Usage: $argv0 intcl outtcl "
#    exit
#}


proc open_files { infile outfile targets} {
    # Don't do this to spm_list files
    if {[string compare -length 8 $infile "spm_list"] == 0 } {
        file copy -force $infile $outfile
        return
    }
    #puts stderr "Open files..."
    set out_fid [open "$outfile" "w"]
    
    # Prevent the ^M from showing up when run on PC.
    fconfigure $out_fid -translation binary

    puts $out_fid "#DO NOT EDIT THIS FILE. Stripped of non-3rdtech UI from this tcl file:"
    puts $out_fid "# $infile"

    set fid [open "$infile" "r"]

    get_lines $fid $out_fid $targets

}

#set infile [lindex $argv 0]
#set outfile [lindex $argv 1]
#open_files $infile $outfile

set startdir [pwd]
set indir [lindex $argv 0]
set outdir [lindex $argv 1]
set targets [lrange $argv 2 end]
if {![file isdirectory $outdir] } {
    #puts stderr "Output specified is not a directory"
    exit
}  
# Make an absolute or volume relative path to the output directory
if {[file pathtype $outdir] == "relative"} {
    set outdir [file join $startdir $outdir]
    #puts stderr "Output path is $outdir"
}


if {[catch {cd $indir}] != 0} {
    #puts stderr "Unable to open input directory"
    exit
}

if { [pwd] == $outdir } {
    #puts stderr "Input and output directories are the same - try again."
    exit
}

foreach infile [glob -nocomplain *.tcl] {
    open_files $infile [file join $outdir $infile] $targets
}

exit