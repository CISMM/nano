#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include <Tcl_Linkvar.h>

#include "Tcl_Interpreter.h"
#include "errorDisplay.h"

/* Bug note. I started with const char *__format as the argument, 
 and it caused a segfault when used near a printf!!!! Don't use
 args starting with double underscore - they are reserved! */

void display_warning_dialog (const char *format, ...) 
{
  va_list argptr;
  char txtptr[2000];
  char msgptr[2050]; 
  /* Print text in a string */
  va_start(argptr, format);
  vsprintf (txtptr, format, argptr);
  va_end(argptr);

  if (Tcl_Interpreter::getInterpreter() == NULL) {
      fprintf(stderr, "Warning: %s\n", txtptr);
  } else {
      sprintf(msgptr, "display_warning_dialog {%s}", txtptr);
      TCLEVALCHECK2(Tcl_Interpreter::getInterpreter(), msgptr);
  }
}

void display_error_dialog (const char *format, ...)
{
  va_list argptr;
  char txtptr[2000];
  char msgptr[2050]; 
  /* Print text in a string */
  va_start(argptr, format);
  vsprintf (txtptr, format, argptr);
  va_end(argptr);

  if (Tcl_Interpreter::getInterpreter() == NULL) {
      fprintf(stderr, "ERROR: %s\n", txtptr);
  } else {
      sprintf(msgptr, "display_error_dialog {%s}", txtptr);
      TCLEVALCHECK2(Tcl_Interpreter::getInterpreter(), msgptr);
  }
}

void display_fatal_error_dialog (const char *format, ...)
{
  va_list argptr;
  char txtptr[2000];
  char msgptr[2050]; 
  /* Print text in a string */
  va_start(argptr, format);
  vsprintf (txtptr, format, argptr);
  va_end(argptr);

  if (Tcl_Interpreter::getInterpreter() == NULL) {
      fprintf(stderr, "FATAL ERROR: %s\n", txtptr);
      // XXX Is this the right thing? Will it attempt to save stream?
      exit(-1);
  } else {
      sprintf(msgptr, "display_fatal_error_dialog {%s}", txtptr);
      TCLEVALCHECK2(Tcl_Interpreter::getInterpreter(), msgptr);
  }
}
