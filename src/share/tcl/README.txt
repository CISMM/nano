
Notes on how it all works (found out the hard way):

* When a user changes the modify (also image) parameter foo,
  it is actually stored in a variable named newfoo.

* When a user clicks on 'ACCEPT' for modify (also image) variables,
  acceptModifyVars { modifylist } is called.  Those variables foo
  listed in modifylist have their values copied from newfoo.
  This triggers the Tcl_TraceVar() executed in Tclvar_init(),
  and the value crosses the border from tcl to C.

* From time to time, for reasons unknown (every time the variable
  is accessed, because of the "trace variable" statements?), "updateFromC"
  (in image.tcl) is called.  This gets the current C value of the variable
  foo and copies it into both foo and newfoo.



