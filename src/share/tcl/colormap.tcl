#/*===3rdtech===
#  Copyright (c) 2000 by 3rdTech, Inc.
#  All Rights Reserved.
#
#  This file may not be distributed without the permission of 
#  3rdTech, Inc. 
#  ===3rdtech===*/


#################################    
#
# This script brings up sliders to control the # mapping of values into colors on the surface.
# A colorbar is displayed which shows the current colormap being applied to the surface
# four sliders control the mapping of data values to color.
# On the right are the upper and lower bounds on the colormap: which data points
# are represented by the maximum and minimum values in the colormap. These controls
# can be used to clamp values above or below a certain point in the data.
# On the left are the upper and lower bounds of the applied colormap, these arrows
# indicate the range of the colormap that is in use. They are used to limit the applied
# color to a subrange of the complete colormap. For example they enable the user to
# apply only the red to white range of a blackbody colormap to the data.
#

set nmInfo(colorscale) [create_closing_toplevel colorscale "Color Map Setup" ]

# Make a frame to hold the pull-down menu that selects from the list
frame $nmInfo(colorscale).scales
frame $nmInfo(colorscale).pickframe

pack $nmInfo(colorscale).pickframe $nmInfo(colorscale).scales -side left -fill both

# Pop-up menu to choose which plane is displayed as a color map.
generic_optionmenu $nmInfo(colorscale).pickframe.colormap_plane \
	color_comes_from \
	"Colormap plane" inputPlaneNames

set colorMapNames {none }
generic_optionmenu $nmInfo(colorscale).pickframe.colormap \
	color_map \
	"Colormap" colorMapNames

iwidgets::Labeledwidget::alignlabels \
	$nmInfo(colorscale).pickframe.colormap_plane \
	$nmInfo(colorscale).pickframe.colormap

pack $nmInfo(colorscale).pickframe.colormap_plane \
	$nmInfo(colorscale).pickframe.colormap -anchor nw -fill x


# Make it easy to create a flat plane to use for a color map.
button $nmInfo(colorscale).pickframe.calc_planes -text "Calculate Data Planes..."  \
    -command "show.calc_planes"
pack $nmInfo(colorscale).pickframe.calc_planes -side bottom -anchor nw

# Reset the colormap sliders:
button $nmInfo(colorscale).pickframe.autoscale -text "AutoScale"  \
    -command "adjust_color_min_max autoscale autoscale autoscale"
pack $nmInfo(colorscale).pickframe.autoscale -side left -anchor nw

set surface_r 192
set surface_g 192
set surface_b 192
set surface_color_changed 0

button $nmInfo(colorscale).set_color -text "Set surface color" -command {
    choose_color surface_color "Choose surface color"
    set_surface_color
}

# this sets the color of the sample frame to the color of the scales
set surface_color [format #%02x%02x%02x $surface_r $surface_g $surface_b]
pack $nmInfo(colorscale).set_color -side left -anchor nw

proc set_surface_color {} {
    global surface_r surface_g surface_b surface_color_changed surface_color
    global nmInfo

    # Extract three component colors of contour_color 
    # and save into surface_r g b
    scan $surface_color #%02x%02x%02x surface_r surface_g surface_b
    set surface_color_changed 1
}

#set these so we can see do " wishx <mainwin.tcl" and test interface
set color_min 0
set color_max 1.0

#
# The color_*_limit variables are the minimum and maximum data values
# in the current colorplane:
set color_min_limit 0
set color_max_limit 1
trace variable color_min_limit w adjust_color_min_max
trace variable color_max_limit w adjust_color_min_max

proc adjust_color_min_max {name element op} {
    global color_min_limit color_max_limit cmColor cmData
    set cmData(min_changed) 1
    set cmData(min_value) 0
    set cmData(max_changed) 1
    set cmData(max_value) 1.0
    set cmColor(min_changed) 0
    set cmColor(min_value) $color_min_limit
    set cmColor(max_changed) 0
    set cmColor(max_value) $color_max_limit
}

canvas $nmInfo(colorscale).canvas -background LemonChiffon1 -height 270
pack $nmInfo(colorscale).canvas
set triangle_width 10
set triangle_height 8
set box_width 6
set box_height 8

set canvas_lower_bound 9
set canvas_upper_bound 265

set image_x 200 
set image_y 265
set image_width 32
set image_height 256
set image_heightf 256.0

# The data_x, data_y_* variables represent the (x,y) coordinates of the 
# data triangles in canvas coordinates:
set data_x [expr $image_x - $image_width - 10]
set data_y_low $image_y
set data_y_high [expr $image_y - $image_height]

# The color_x, color_y_* variables represent the (x,y) coordinates of the 
# color triangles in canvas coordinates:
set color_x [expr $image_x + 10]
set color_y_low $image_y
set color_y_high [expr $image_y - $image_height]

#
# cmData contains the values displayed in the data text boxes.
# The values range from 0 - 1. The *_changed variables are used
# to indicate whether the values were changed by interactively sliding
# the triangle, or by typing in the textbox.
set cmData(min_value) 0
set cmData(max_value) 1
set cmData(min_changed) 0
set cmData(max_changed) 0

#
# cmColor contains the values displayed in the color text boxes.
# The values range from the minimum data value to the maximum data value. 
# contained in the current colorplane as scaled by the cmData triangle range.
# (see below). The *_changed variables are used
# to indicate whether the values were changed by interactively sliding
# the triangle, by typing in the textbox, or because the data triangles
# changed.
set cmColor(min_value) 0
set cmColor(max_value) 1
set cmColor(min_changed) 0
set cmColor(max_changed) 0

# The cmColor(min_value) and cmColor(max_value) are related to the  
# cmData(min_value) and cmData(max_value) through a scaling operation. 
# For example the cmColor(max_value)
# cmColor(max_value) = (colorplane data range)/(data arrows range) *  color_max +
#                      ((color_min * cmData(max_value) - (color_max * cmData(min_value))/
#                      (data arrows range)


# Draws a triangle pointing right:
proc left_triangle { x y } {
    global triangle_width triangle_height
    return [list [expr $x + $triangle_width] $y \
	    [expr $x - $triangle_width] [expr $y + $triangle_height] \
	    [expr $x - $triangle_width] [expr $y - $triangle_height]]
}

# Draws a triangle pointing left:
proc right_triangle { x y } {
    global triangle_width triangle_height
    return [list [expr $x - $triangle_width] $y \
	    [expr $x + $triangle_width] [expr $y + $triangle_height] \
	    [expr $x + $triangle_width] [expr $y - $triangle_height]]
}
proc draw_line { x y1 y2 } {
    return [list  $x [expr $y2] $x [expr $y1] ]
}

proc draw_rectangle { x y1 y2 } {
    global box_width box_height

    return [list \
	    [expr $x - $box_width]\
	    [expr $y1 + ($y2 - $y1)/2 - $box_height] \
	    [expr $x  + $box_width]\
	    [expr $y1 + ($y2 - $y1)/2 + $box_height] \
	    ]
}
###
### The triangle and entry widget for the Data Min selector
### the motion of the triangle is restricted to up/down.
###
eval $nmInfo(colorscale).canvas create polygon [left_triangle $data_x $data_y_low] \
	-fill blue4 -tags data_min_tri
$nmInfo(colorscale).canvas bind data_min_tri <B1-Motion>\
	"colormap_set_data_min $nmInfo(colorscale) %y"
generic_entry $nmInfo(colorscale).data_min_entry cmData(min_value) "Data Min" real
trace variable cmData(min_value) w "adjust_data_min $nmInfo(colorscale)"
$nmInfo(colorscale).canvas create window [expr $data_x - $triangle_width] $data_y_low\
	-anchor e -window $nmInfo(colorscale).data_min_entry -tags data_min_entry

###
### The triangle and entry widget for the Data Max selector
### the motion of the triangle is restricted to up/down.
###
eval $nmInfo(colorscale).canvas create polygon [left_triangle $data_x $data_y_high]\
	-fill blue4 -tags data_max_tri
$nmInfo(colorscale).canvas bind data_max_tri <B1-Motion>\
	"colormap_set_data_max $nmInfo(colorscale) %y"
generic_entry $nmInfo(colorscale).data_max_entry cmData(max_value) "Data Max" real
trace variable cmData(max_value) w "adjust_data_max $nmInfo(colorscale)"
$nmInfo(colorscale).canvas create window [expr $data_x - $triangle_width] $data_y_high\
	-anchor e -window $nmInfo(colorscale).data_max_entry -tags data_max_entry

eval $nmInfo(colorscale).canvas create line [draw_line $data_x $data_y_low $data_y_high ] \
	-width 3 -fill blue4 -tags data_line

eval $nmInfo(colorscale).canvas create rectangle \
	[draw_rectangle $data_x $data_y_low $data_y_high]\
	-fill blue4 -outline blue4 -tags data_line_box
$nmInfo(colorscale).canvas bind data_line_box <B1-Motion>\
	"colormap_set_data_line $nmInfo(colorscale) %y"

###
### The triangle and entry widget for the Color Min selector
### the motion of the triangle is restricted to up/down.
###
eval $nmInfo(colorscale).canvas create polygon [right_triangle $color_x $color_y_low]\
	-fill blue4 -tags color_min_tri
$nmInfo(colorscale).canvas bind color_min_tri <B1-Motion> \
	"colormap_set_color_min $nmInfo(colorscale) %y"
generic_entry $nmInfo(colorscale).color_min_entry cmColor(min_value) "Color Min" real
trace variable cmColor(min_value) w "adjust_color_min $nmInfo(colorscale)"
$nmInfo(colorscale).canvas create window [expr $color_x + $triangle_width] $color_y_low\
	-anchor w -window $nmInfo(colorscale).color_min_entry -tags color_min_entry

###
### The triangle and entry widget for the Color Max selector
### the motion of the triangle is restricted to up/down.
###
eval $nmInfo(colorscale).canvas create polygon [right_triangle $color_x $color_y_high]\
	-fill blue4 -tags color_max_tri
$nmInfo(colorscale).canvas bind color_max_tri <B1-Motion> \
	"colormap_set_color_max $nmInfo(colorscale) %y"
generic_entry $nmInfo(colorscale).color_max_entry cmColor(max_value) "Color Max" real
trace variable cmColor(max_value) w "adjust_color_max $nmInfo(colorscale)"
$nmInfo(colorscale).canvas create window [expr $color_x + $triangle_width] $color_y_high\
	-anchor w -window $nmInfo(colorscale).color_max_entry -tags color_max_entry

eval $nmInfo(colorscale).canvas create line [draw_line $color_x $color_y_low $color_y_high]\
	-width 3 -fill blue4 -tags color_line

eval $nmInfo(colorscale).canvas create rectangle \
	[draw_rectangle $color_x $color_y_low $color_y_high] \
	-fill blue4 -outline blue4 -tags color_line_box
$nmInfo(colorscale).canvas bind color_line_box <B1-Motion> \
	"colormap_set_color_line $nmInfo(colorscale) %y"

set imh [image create photo "colormap_image" -height $image_height -width $image_width]
$nmInfo(colorscale).canvas create image $image_x $image_y -anchor se -image $imh

## This function gets called when the user moves the data_min triangle
## It makes sure the triangle is w/in the bounds of the canvas and
## transforms the value from canvas coordinates to between 0-1.
## It then sets the data variables data_min, which is used in unix,
## and cmData(min_value/changed) which are used to display in tcl.
## Once cmData(min_value) is set a callback is activated. See adjust_data_min.
proc colormap_set_data_min { win y } {
    global canvas_lower_bound canvas_upper_bound image_heightf
    global cmData cmColor
    global data_min

    set y1 $y
    if { $y1 <= $canvas_lower_bound } {
	set y1 $canvas_lower_bound
    } elseif { $y1 >= $canvas_upper_bound } {
	set y1 $canvas_upper_bound
    }

    set data_min [expr 1.0 - (($y1 - $canvas_lower_bound)/$image_heightf)]
    set cmData(min_changed) 1
    set cmData(min_value) $data_min
}

## This function gets called whenever cmData(data_min) is changed,
## either by moving the data_min triangle, by typing into the textbox,
## or by pressing the autoscale button. 
## In all cases the triangle, line, and box icons are repositioned.
proc adjust_data_min { win name element op } {
    global data_y_low data_y_high data_x
    global canvas_lower_bound canvas_upper_bound triangle_width
    global image_heightf
    global color_min_limit color_max_limit color_max color_min
    global cmData cmColor
    global data_min

    ## Makes sure the values are w/in range:
    if { $cmData(min_value) < 0 } {
	set cmData(min_value) 0
    } elseif { $cmData(min_value) > 1.0 } {
	set cmData(min_value) 1.0
    }

    ## Changes the cmColor(max_value) variable to correctly represent
    ## the new data range. This is used when the data_min/max are
    ## set so that only a subrange of the colormap is applied to the
    ## data, in that case the cmColor(*_value) displayed are no longer
    ## w/in the range of the min/max data points in the colorplane
    ## but actually extend outside the colorplane range.
    ## if $cmData(min_changed) ==2 then the values have already been set.
    if { $cmData(min_changed) != 2 } {
	set cmColor(min_changed) 2
	set cmColor(min_value) [scale_color_min $color_max_limit $color_min_limit \
		$color_min $cmData(max_value) $cmData(min_value)]
    }
    ##
    ## Calculate where to position arrow and box:
    ##
    set y1 [expr 1.0 - $cmData(min_value)]
    set y1 [expr $y1 * ($canvas_upper_bound - $canvas_lower_bound)]
    set y1 [expr $y1 + $canvas_lower_bound]
    
    if { $y1 < $canvas_lower_bound } {
	set y1 $canvas_lower_bound
    } elseif { $y1 > $canvas_upper_bound } {
	set y1 $canvas_upper_bound
    }
    set data_min [expr 1.0 - (($y1 - $canvas_lower_bound)/$image_heightf)]
    set data_y_low $y1
    
    ## reposition arrow:
    eval $win.canvas coords data_min_tri [left_triangle $data_x $data_y_low]
    $win.canvas coords data_min_entry [expr $data_x - $triangle_width] $data_y_low
    
    ## reposition line:
    eval $win.canvas coords data_line [draw_line $data_x $data_y_low  $data_y_high]
    
    ## reposition box:
    eval $win.canvas coords data_line_box [draw_rectangle $data_x $data_y_low $data_y_high]
}

## This function gets called when the user moves the data_max triangle
## It makes sure the triangle is w/in the bounds of the canvas and
## transforms the value from canvas coordinates to between 0-1.
## It then sets the data variables data_max, which is used in unix,
## and cmData(max_value/changed) which are used to display in tcl.
## Once cmData(max_value) is set a callback is activated. See adjust_data_max.
proc colormap_set_data_max { win y } {
    global data_max
    global canvas_lower_bound canvas_upper_bound
    global image_heightf
    global color_min_limit color_max_limit
    global cmData cmColor

    set y1 $y
    if { $y1 < $canvas_lower_bound } {
	set y1 $canvas_lower_bound
    } elseif { $y1 > $canvas_upper_bound } {
	set y1 $canvas_upper_bound
    }
    set data_max [expr 1.0 - (($y1 - $canvas_lower_bound)/$image_heightf)]
    set cmData(max_changed) 1
    set cmData(max_value) $data_max
}

## This function gets called whenever cmData(data_max) is changed,
## either by moving the data_max triangle, by typing into the textbox,
## or by pressing the autoscale button. 
## In all cases the triangle, line, and box icons are repositioned.
proc adjust_data_max { win name element op } {
    global data_max data_y_low data_y_high data_x
    global canvas_lower_bound canvas_upper_bound triangle_width
    global image_heightf
    global color_min_limit color_max_limit color_max color_min
    global cmData cmColor

    ## Makes sure the number typed in is w/in range:
    if { $cmData(max_value) < 0 } {
	set cmData(max_value) 0
    } elseif { $cmData(max_value) > 1.0 } {
	set cmData(max_value) 1.0
    }

    ## Changes the cmColor(max_value) variable to correctly represent
    ## the new data range. This is used when the data_min/max are
    ## set so that only a subrange of the colormap is applied to the
    ## data, in that case the cmColor(*_value) displayed are no longer
    ## w/in the range of the min/max data points in the colorplane
    ## but actually extend outside the colorplane range.
    ## if $cmData(max_changed) ==2 then the values have already been set.
    if { $cmData(max_changed) != 2} {
	set cmColor(max_changed) 2
	set cmColor(max_value) [scale_color_max $color_max_limit $color_min_limit \
		$color_max $cmData(max_value) $cmData(min_value)]
    }
    ##
    ## Calculate where to position arrow and box:
    ##
    set y1 [expr 1.0 - $cmData(max_value)]
    set y1 [expr $y1 * ($canvas_upper_bound - $canvas_lower_bound)]
    set y1 [expr $y1 + $canvas_lower_bound]
    
    if { $y1 < $canvas_lower_bound } {
	set y1 $canvas_lower_bound
    } elseif { $y1 > $canvas_upper_bound } {
	set y1 $canvas_upper_bound
    }
    set data_max [expr 1.0 - (($y1 - $canvas_lower_bound)/$image_heightf)]
    set data_y_high $y1
    
    ## reposition arrow:
    eval $win.canvas coords data_max_tri [left_triangle $data_x $data_y_high]
    $win.canvas coords data_max_entry [expr $data_x - $triangle_width] $data_y_high
    
    ## reposition line:
    eval $win.canvas coords data_line [draw_line $data_x $data_y_low  $data_y_high]
    
    ## reposition box:
    eval $win.canvas coords data_line_box [draw_rectangle $data_x $data_y_low $data_y_high]
}

## This function gets called when the box icon connecting the data triangles
## is moved. In that case both triangles move as a unit.
## Both callbacks get called as well, repositioning the icons and changing
## the cmColor(*_values) displayed.
proc colormap_set_data_line { win y } {
    global data_max data_min data_y_low data_y_high
    global canvas_lower_bound canvas_upper_bound
    global image_heightf
    global color_min_limit color_max_limit color_max color_min
    global cmData cmColor

    set y1 $y
    if { $y1 < $canvas_lower_bound } {
	set y1 $canvas_lower_bound
    } elseif { $y1 > $canvas_upper_bound } {
	set y1 $canvas_upper_bound
    }

    set dif [expr (($data_y_high - $y1) - ($y1 - $data_y_low))/2]

    set data_y_low_tmp [expr $data_y_low - $dif] 
    set data_y_high_tmp [expr $data_y_high - $dif]
    set size [expr $data_y_low_tmp - $data_y_high_tmp]
    
    ## Checks that the values are w/in the range of the canvas
    ## allows for swapping of the min/max triangles.
    if { $data_y_high_tmp <= $canvas_lower_bound } {
	set data_y_high $canvas_lower_bound
	set data_y_low [expr $data_y_high + $size]
    } elseif { $data_y_high_tmp >= $canvas_upper_bound } {
	set data_y_high $canvas_upper_bound
	set data_y_low [expr $data_y_high - $size]
    } elseif { $data_y_low_tmp <= $canvas_lower_bound } {
	set data_y_low $canvas_lower_bound
	set data_y_high [expr $data_y_low + $size]
    } elseif { $data_y_low_tmp >= $canvas_upper_bound } {
	set data_y_low $canvas_upper_bound
	set data_y_high [expr $data_y_low - $size]
    } else {
	set data_y_low $data_y_low_tmp
	set data_y_high $data_y_high_tmp
    }

    set data_min [expr 1.0 - ($data_y_low - $canvas_lower_bound)/$image_heightf]
    set data_max [expr 1.0 - ($data_y_high  - $canvas_lower_bound)/$image_heightf]


    ## Changes the cmColor(*_value) variables to correctly represent
    ## the new data range. This is used when the data_min/max are
    ## set so that only a subrange of the colormap is applied to the
    ## data, in that case the cmColor(*_value) displayed are no longer
    ## w/in the range of the min/max data points in the colorplane
    ## but actually extend outside the colorplane range.

    set cmColor(max_changed) 1
    set cmColor(max_value) [scale_color_max $color_max_limit $color_min_limit \
	    $color_max $data_max $data_min]
    set cmColor(min_changed) 1
    set cmColor(min_value) [scale_color_min $color_max_limit $color_min_limit \
	    $color_min $data_max $data_min]

    set cmData(min_changed) 2
    set cmData(min_value) $data_min
    set cmData(max_changed) 2
    set cmData(max_value) $data_max
}

## This function gets called when the user moves the color_min
## triangle. It makes sure the values are w/in the range of the canvas
## then converts from canvas coordinates to the min-max range
## of the colorplane (taking into account any scaling by the data triangles)
proc colormap_set_color_min { win y } {
    global color_min color_y_high color_y_low
    global canvas_lower_bound canvas_upper_bound image_heightf
    global color_min_limit color_max_limit
    global cmColor cmData

    set y1 $y
    if { $y1 < $color_y_high } {
	set y1 $color_y_high
    } elseif { $y1 > $canvas_upper_bound } {
	set y1 $canvas_upper_bound
    }
    set color_min [expr 1.0 - (($y1 - $canvas_lower_bound)/$image_heightf)]
    set color_y_low $y1

    set cmColor(min_changed) 1
    set cmColor(min_value) [scale_color_min $color_max_limit $color_min_limit \
	    $color_min $cmData(max_value) $cmData(min_value)]
}

## This function gets called whenever the cmColor(data_min) value is 
## set, whether through the autoscale button, the slider, or by changing
## cmData(data_min). Depending on how the value changed different things
## need to be recalculated.
proc adjust_color_min { win name element op } {
    global color_min color_y_low color_y_high color_x
    global canvas_lower_bound canvas_upper_bound triangle_width
    global image_heightf
    global color_min_limit color_max_limit
    global cmColor

    ## If the value changed only because cmData(min_value) changed,
    ## then it is not necessary to reposition the arrow icon. Otherwise,
    ## if the color_min slider moved, or the text box was typed into, reposition the arrow:
    if { $cmColor(min_changed) != 2 } {
	## == 0 means the textbox was typed into:
	if { $cmColor(min_changed) == 0 } {
	    set y1 [expr ($cmColor(min_value) - $color_min_limit)/ \
		    ($color_max_limit - $color_min_limit)]
	    set y1 [expr 1.0 - $y1]
	    set y1 [expr $y1 * ($canvas_upper_bound - $canvas_lower_bound)]
	    set y1 [expr $y1 + $canvas_lower_bound]
	    
	    if { $y1 < $color_y_high } {
		set y1 $color_y_high
	    } elseif { $y1 > $canvas_upper_bound } {
		set y1 $canvas_upper_bound
	    }
	    set color_min [expr 1.0 - (($y1 - $canvas_lower_bound)/$image_heightf)]
	    set color_y_low $y1
	}
	## reposition arrow:
	eval $win.canvas coords color_min_tri [right_triangle $color_x $color_y_low]
	$win.canvas coords color_min_entry [expr $color_x + $triangle_width] $color_y_low
	
	## reposition line:
	eval $win.canvas coords color_line [draw_line $color_x $color_y_low  $color_y_high]
	
	## reposition box:
	eval $win.canvas coords color_line_box \
		[draw_rectangle $color_x $color_y_low $color_y_high]
    }
    set cmColor(min_changed) 0
}

## This function gets called when the user moves the color_max
## triangle. It makes sure the values are w/in the range of the canvas
## then converts from canvas coordinates to the min-max range
## of the colorplane (taking into account any scaling by the data triangles)
proc colormap_set_color_max { win y } {
    global canvas_lower_bound canvas_upper_bound image_heightf color_y_low color_y_high
    global color_min_limit color_max_limit
    global color_max
    global cmData cmColor

    set y1 $y
    if { $y1 < $canvas_lower_bound } {
	set y1 $canvas_lower_bound
    } elseif { $y1 > $color_y_low } {
	set y1 $color_y_low
    }
    set color_y_high $y1
    set color_max [expr 1.0 - (($y1 - $canvas_lower_bound)/$image_heightf)]
    set cmColor(max_changed) 1
    set cmColor(max_value) [scale_color_max $color_max_limit $color_min_limit \
	    $color_max $cmData(max_value) $cmData(min_value)]
}

## This function gets called whenever the cmColor(data_max) value is 
## set, whether through the autoscale button, the slider, or by changing
## cmData(data_max). Depending on how the value changed different things
## need to be recalculated.
proc adjust_color_max { win name element op } {
    global color_max color_y_low color_y_high color_x
    global canvas_lower_bound canvas_upper_bound triangle_width
    global image_heightf
    global color_min_limit color_max_limit
    global cmColor cmData

    ## If the value changed only because cmData(max_value) changed,
    ## then it is not necessary to reposition the arrow icon. Otherwise,
    ## if the color_min slider moved, or the text box was typed into, reposition the arrow:
    if { $cmColor(max_changed) != 2 } {
	## == 0 means the textbox was typed into:
	if { $cmColor(max_changed) == 0 } {
	    set y1 [expr ($cmColor(max_value) - $color_min_limit)/ \
		    ($color_max_limit - $color_min_limit)]
	    set y1 [expr 1.0 - $y1]
	    set y1 [expr $y1 * ($canvas_upper_bound - $canvas_lower_bound)]
	    set y1 [expr $y1 + $canvas_lower_bound]
	    
	    if { $y1 < $canvas_lower_bound } {
		set y1 $canvas_lower_bound
	    } elseif { $y1 > $color_y_low } {
		set y1 $color_y_low
	    }
	    set color_max [expr 1.0 - (($y1 - $canvas_lower_bound)/$image_heightf)]
	    set color_y_high $y1
	} 
	## reposition arrow:
	eval $win.canvas coords color_max_tri [right_triangle $color_x $color_y_high]
	$win.canvas coords color_max_entry [expr $color_x + $triangle_width] $color_y_high
	
	## reposition line:
	eval $win.canvas coords color_line [draw_line $color_x $color_y_low  $color_y_high]
	
	## reposition box:
	eval $win.canvas coords color_line_box \
		[draw_rectangle $color_x $color_y_low $color_y_high]
    }
    set cmColor(max_changed) 0
}

## This function gets called when the box icon for the color_min/max arrows
## is moved. It calculates the correct range and moves the arrows as a unit.
proc colormap_set_color_line { win y } {
    global color_max color_min color_y_high color_y_low color_x
    global canvas_lower_bound canvas_upper_bound
    global triangle_width triangle_height box_width box_height
    global image_heightf
    global color_min_limit color_max_limit
    global cmData cmColor

    set y1 $y
    if { $y1 < $canvas_lower_bound } {
	set y1 $canvas_lower_bound
    } elseif { $y1 > $canvas_upper_bound } {
	set y1 $canvas_upper_bound
    }

    set dif [expr (($color_y_high - $y1) - ($y1 - $color_y_low))/2]

    set color_y_low_tmp [expr $color_y_low - $dif] 
    set color_y_high_tmp [expr $color_y_high - $dif]
    set size [expr $color_y_low_tmp - $color_y_high_tmp]
    
    if { $color_y_high_tmp < $canvas_lower_bound } {
	set color_y_high $canvas_lower_bound
	set color_y_low [expr $color_y_high + $size]
    } elseif { $color_y_high_tmp > $canvas_upper_bound } {
    } elseif { $color_y_low_tmp < $canvas_lower_bound } {
    } elseif { $color_y_low_tmp > $canvas_upper_bound } {
	set color_y_low $canvas_upper_bound
	set color_y_high [expr $color_y_low - $size]
    } else {
	set color_y_low $color_y_low_tmp
	set color_y_high $color_y_high_tmp
    }

    ## Set the minimum value displayed in the text box:
    set color_min [expr 1.0 - ($color_y_low - $canvas_lower_bound)/$image_heightf]

    ## Set the maximum value displayed in the text box:
    set color_max [expr 1.0 - ($color_y_high - $canvas_lower_bound)/$image_heightf]

    set cmColor(max_changed) 1
    set cmColor(max_value) [scale_color_max $color_max_limit $color_min_limit \
	    $color_max $cmData(max_value) $cmData(min_value)]

    set cmColor(min_changed) 1
    set cmColor(min_value) [scale_color_min $color_max_limit $color_min_limit \
	    $color_min $cmData(max_value) $cmData(min_value)]
}

## Changes the cmColor(max_value) variable to correctly represent
## the new data range. This is used when the data_min/max are
## set so that only a subrange of the colormap is applied to the
## data, in that case the cmColor(*_value) displayed are no longer
## w/in the range of the min/max data points in the colorplane
## but actually extend outside the colorplane range.
## if $cmData(max_changed) ==2 then the values have already been set.
proc scale_color_max { c_max_limit c_min_limit c_max d_max d_min} {
    if { $d_max != $d_min } {
	set tmp [expr (($c_max_limit - $c_min_limit) / ($d_max - $d_min) * $c_max) + \
		((($c_min_limit * $d_max) - ($c_max_limit * $d_min))/($d_max - $d_min))]
    } else {
	## Only happens if the user wants to swap the min/max colors, and is temporary...
	set tmp [expr (($c_max_limit - $c_min_limit) / ($d_max - $d_min + .001) * $c_max) + \
		((($c_min_limit * $d_max) - ($c_max_limit * $d_min))/($d_max - $d_min + .001))]
    }
    return $tmp
}

## Changes the cmColor(max_value) variable to correctly represent
## the new data range. This is used when the data_min/max are
## set so that only a subrange of the colormap is applied to the
## data, in that case the cmColor(*_value) displayed are no longer
## w/in the range of the min/max data points in the colorplane
## but actually extend outside the colorplane range.
## if $cmData(min_changed) ==2 then the values have already been set.
proc scale_color_min { c_max_limit c_min_limit c_min d_max d_min} {
    if { $d_max != $d_min } {
	set tmp [expr (($c_max_limit - $c_min_limit) / ($d_max - $d_min) * $c_min) + \
		((($c_min_limit * $d_max) - ($c_max_limit * $d_min))/($d_max - $d_min))]
    } else {
	## Only happens if the user wants to swap the min/max colors, and is temporary...
	set tmp [expr (($c_max_limit - $c_min_limit) / ($d_max - $d_min + .001) * $c_min) + \
		((($c_min_limit * $d_max) - ($c_max_limit * $d_min))/($d_max - $d_min + .001))]
    }
    return $tmp
}

##########
# end ColorMap
