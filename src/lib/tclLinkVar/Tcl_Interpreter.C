#include "Tcl_Interpreter.h"

#include <tcl.h>
#include <tk.h>

#include <blt.h>

#ifndef NO_ITCL
#include <itcl.h>
#include <itk.h>
#endif

// for some reason blt.h doesn't declare this procedure.
// I copied this prototype from bltUnixMain.c, but I had to add
// the "C" so it would link with the library.
extern "C" int Blt_Init (Tcl_Interp *interp);
extern "C" int Blt_SafeInit(Tcl_Interp *interp);

Tcl_Interp *Tcl_Interpreter::tk_control_interp = NULL;

Tcl_Interp *Tcl_Interpreter::getInterpreter()
{
  if (!tk_control_interp) {
    if (initTclTk()) {
      Tcl_DeleteInterp(tk_control_interp);
      tk_control_interp = NULL;
    }
  }
  return tk_control_interp; 
}

int Tcl_Interpreter::initTclTk()
{
  Tk_Window       tk_control_window;

  Tcl_Interp * my_tk_control_interp = Tcl_CreateInterp();

#if defined (_WIN32) && !defined (__CYGWIN__)
  if (Tcl_InitStubs(my_tk_control_interp, TCL_VERSION, 1) == NULL) {
      fprintf(stderr, "Non matching version of tcl and tk\n");
      return -1;
  }
#endif
  /* Start a Tcl interpreter */
  if (Tcl_Init(my_tk_control_interp) == TCL_ERROR) {
      fprintf(stderr,
         "Tcl_Init failed: %s\n",Tcl_GetStringResult(my_tk_control_interp));
      return -1;
  }

  /* Initialize Tk using the Tcl interpreter */
  if (Tk_Init(my_tk_control_interp) == TCL_ERROR) {
      fprintf(stderr,
          "Tk_Init failed: %s\n",Tcl_GetStringResult(my_tk_control_interp));
      return -1;
  }
  Tcl_StaticPackage(my_tk_control_interp, "Tk", Tk_Init, Tk_SafeInit);

  /* Initialize Tcl packages */
  if (Blt_Init(my_tk_control_interp) == TCL_ERROR) {
      fprintf(stderr,
        "Package_Init failed: %s\n",Tcl_GetStringResult(my_tk_control_interp));
      return -1;
  }
  Tcl_StaticPackage(my_tk_control_interp, "Blt", Blt_Init, Blt_SafeInit);

#ifndef NO_ITCL
  if (Itcl_Init(my_tk_control_interp) == TCL_ERROR) {
      fprintf(stderr,
        "Package_Init failed: %s\n",Tcl_GetStringResult(my_tk_control_interp));
      return(-1);
  }
  if (Itk_Init(my_tk_control_interp) == TCL_ERROR) {
      fprintf(stderr,
         "Package_Init failed: %s\n",Tcl_GetStringResult(my_tk_control_interp));
      return(-1);
  }
  Tcl_StaticPackage(my_tk_control_interp, "Itcl", Itcl_Init, Itcl_SafeInit);
  Tcl_StaticPackage(my_tk_control_interp, "Itk", Itk_Init, 
                    (Tcl_PackageInitProc *) NULL);
#endif 
  // Check to see if we have a Tk main window.
  tk_control_window = Tk_MainWindow(my_tk_control_interp);
  if (tk_control_window == NULL) {
      fprintf(stderr,"Tk can't get main window: %s\n",
              Tcl_GetStringResult(my_tk_control_interp));
      return(-1);
  }
  tk_control_interp = my_tk_control_interp;

  return 0;
}

