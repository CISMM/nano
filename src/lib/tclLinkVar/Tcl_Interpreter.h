#ifndef TCL_INTERPRETER_H
#define TCL_INTERPRETER_H

#include <tcl.h>

class Tcl_Interpreter {
 public:
  static Tcl_Interp *getInterpreter();

 protected:
  static int initTclTk();
  static Tcl_Interp *tk_control_interp;

};

#endif
