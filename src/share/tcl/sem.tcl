#!/bin/sh
# ----------------------------------------------------------------------
#  sem interface
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
global sem_window_open sem_acquire_image sem_acquire_continuous \
       sem_display_texture sem_no_graphics_update \
       sem_pixel_integration_time_nsec sem_inter_pixel_delay_time_nsec \
       sem_resolution

set nmInfo(sem) [create_closing_toplevel_with_notify \
                            sem_win sem_window_open]
# ------------------------------------------------- end GLOBAL variables

iwidgets::labeledframe $nmInfo(sem).acquisition -labeltext "SEM acquisition"
pack $nmInfo(sem).acquisition -anchor nw -padx 3 -pady 3

set nmInfo(sem_acq) [$nmInfo(sem).acquisition childsite]
# Button to start the scan
button $nmInfo(sem_acq).acquire_image -text "Acquire Image" \
	-command { set sem_acquire_image 1 }
pack $nmInfo(sem_acq).acquire_image -anchor w

# Controls whether to scan continuously or not
checkbutton $nmInfo(sem_acq).acquire_continuous \
        -text "Acquire Continuously" \
	-variable sem_acquire_continuous 
pack $nmInfo(sem_acq).acquire_continuous -anchor w

generic_radiobox $nmInfo(sem_acq).resolution \
    sem_resolution \
    "Resolution" \
    {"50 x 64" "100 x 128" "200 x 256" "400 x 512" "800 x 1024" "1600 x 2048" "3200 x 4096"}
pack $nmInfo(sem_acq).resolution

generic_entry $nmInfo(sem_acq).pixel_integration_time_nsec \
    sem_pixel_integration_time_nsec \
    "Pixel Integration (nsec)" integer
pack $nmInfo(sem_acq).pixel_integration_time_nsec
generic_entry $nmInfo(sem_acq).inter_pixel_delay_time_nsec \
    sem_inter_pixel_delay_time_nsec \
    "Inter Pixel Delay (nsec)" integer
pack $nmInfo(sem_acq).inter_pixel_delay_time_nsec

iwidgets::labeledframe $nmInfo(sem).display -labeltext "SEM display"
pack $nmInfo(sem).display -anchor nw -padx 3 -pady 3

set nmInfo(sem_display) [$nmInfo(sem).display childsite]

# Controls whether or not image is displayed as a surface texture
checkbutton $nmInfo(sem_display).display_texture \
      -text "Display Texture" -variable sem_display_texture
pack $nmInfo(sem_display).display_texture -anchor w

# Colormap control
button $nmInfo(sem_display).colormap -text "Colormap..." \
    -command "show.sem_colormap"

pack $nmInfo(sem_display).colormap -anchor w

set nmInfo(sem_colormap) [create_closing_toplevel sem_colormap "SEM Color Map" ]

colormap_controls $nmInfo(sem_colormap) sem_cm \
        sem_cm(color_comes_from) "SEM image" imageNames

# Controls the alpha value of the texture
floatscale $nmInfo(sem_display).texture_alpha 0 1 1000 1 1 \
    video_texture_alpha "Texture Alpha Value"
pack $nmInfo(sem_display).texture_alpha -anchor w


# Controls whether to update image data and displays
checkbutton $nmInfo(sem_display).no_update \
        -text "Do not update images" -variable sem_no_graphics_update
pack $nmInfo(sem_display).no_update -anchor w
