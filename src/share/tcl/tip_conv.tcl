global data_comes_from tip_comes_from conv_window_open
global result_image_name


set data_comes_from "none"
set tip_comes_from "none"

set nmInfo(tip_conv) [create_closing_toplevel_with_notify  \
                       tip_conv conv_window_open]

generic_optionmenu $nmInfo(tip_conv).selection_data data_comes_from  \
        "Image Data Name" imageNames
pack $nmInfo(tip_conv).selection_data -anchor nw -padx 3 -pady 3
generic_optionmenu $nmInfo(tip_conv).selection_tip tip_comes_from  \
        "Tip Image Name" imageNames
pack $nmInfo(tip_conv).selection_tip -anchor nw -padx 3 -pady 3

set result_image_name ""


#set resample_from ""
frame $nmInfo(tip_conv).result_image -relief raised -bd 4
frame $nmInfo(tip_conv).result_image.choice
frame $nmInfo(tip_conv).result_image.name
pack $nmInfo(tip_conv).result_image.choice -side left
pack $nmInfo(tip_conv).result_image.name
label $nmInfo(tip_conv).result_image.name.label -text "Output image name"
pack $nmInfo(tip_conv).result_image.name.label
newlabel_dialogue result_image_name $nmInfo(tip_conv).result_image.name
pack $nmInfo(tip_conv).result_image -fill both



#frame $nmInfo(tip_conv).result_image -relief raised -bd 4
#label $nmInfo(tip_conv).result_image_label -text "Output Image Name"
#pack $nmInfo(tip_conv).result_image_label -side left
#entry $nmInfo(tip_conv).resultimage -relief sunken -bd 2 -textvariable result_image_name
#pack $nmInfo(tip_conv).resultimage




