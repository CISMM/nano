
Notes on how image.tcl and modify.tcl work (found out the hard way):

* In the "Full param" dialog, when a user changes the modify (also
  image) parameter foo, it is actually stored in a variable named
  newfoo.

* When a user clicks on 'ACCEPT' for modify (also image) variables,
  acceptModifyVars { modifylist } is called.  Those variables
  modifyp_foo listed in modifylist have their values copied from
  newmodifyp_foo.  This triggers the Tcl_TraceVar() executed in
  Tclvar_init(), and the value crosses the border from tcl to C. Any
  callback registered for a specific tclvar in C is triggered.

* Every time the variable foo is written from C, or set by one of the
  "Quick" widgets, "updateFromC" (in image.tcl) is called.  This gets
  the current C value of the variable modifyp_foo and copies it into
  both modifyp_foo and newmodifyp_foo. This is set up using the "trace
  variable w" statements (w is for write).

Notes on spm_list.tcl:
.cvsignore is set up to ignore spm_list.tcl. That is because it is
copied from spm_list_def.tcl by the NanoManipulator install
program. 

The list of UNC SPMs should be updated in filemenu.tcl. If the install
program is used at UNC, then delete the file spm_list.tcl after the
installer runs.

spm_list_def.tcl contains the default list of SPMs for 3rdtech users -
one SPM at a fake IP address. The installer copies this file to
spm_list.tcl, where a commercial user or non-UNC user can edit it if
necessary. 


