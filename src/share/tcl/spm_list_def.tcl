#/*===3rdtech===
#  Copyright (c) 2000 by 3rdTech, Inc.
#  All Rights Reserved.
#
#  This file may not be distributed without the permission of 
#  3rdTech, Inc. 
#  ===3rdtech===*/

# spm_list_def.tcl contains the default list of SPMs.
# The NanoManipulator install program copies that file automatically
# to spm_list.tcl, which is used by the NanoManipulator in the OpenSPMConnection 
# dialog box. 
#
# Edit spm_list.tcl to change the list of SPMs available inside the
# NanoManipulator program. For each name in the deviceNames list, 
# an entry of the form "nmm_Microscope@<IP address or hostname>" should appear
# in the deviceConnections list. 
set deviceNames       { "Explorer" }
set deviceConnections { "nmm_Microscope@10.9.9.9" }


