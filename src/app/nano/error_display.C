#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include <Tcl_Linkvar.h>

#include "tcl_tk.h"
#include "error_display.h"

// TCH Dissertation July 2001 HACK HACK HACK
int disable_dialogs = 1;

/* Bug note. I started with const char *__format as the argument, 
 and it caused a segfault when used near a printf!!!! Don't use
 args starting with double underscore - they are reserved! */

void display_warning_dialog (const char *format, ...) 
{
  va_list argptr;
  char txtptr[2000];
  char msgptr[2050]; 

  if (disable_dialogs) return;

  /* Print text in a string */
  va_start(argptr, format);
  vsprintf (txtptr, format, argptr);
  va_end(argptr);

  if (get_the_interpreter() == NULL) {
      fprintf(stderr, "Warning: %s\n", txtptr);
  } else {
      sprintf(msgptr, "nano_warning {%s}", txtptr);
      TCLEVALCHECK2(get_the_interpreter(), msgptr);
  }
}

void display_error_dialog (const char *format, ...)
{
  va_list argptr;
  char txtptr[2000];
  char msgptr[2050]; 

  if (disable_dialogs) return;

  /* Print text in a string */
  va_start(argptr, format);
  vsprintf (txtptr, format, argptr);
  va_end(argptr);

  if (get_the_interpreter() == NULL) {
      fprintf(stderr, "ERROR: %s\n", txtptr);
  } else {
      sprintf(msgptr, "nano_error {%s}", txtptr);
      TCLEVALCHECK2(get_the_interpreter(), msgptr);
  }
}

void display_fatal_error_dialog (const char *format, ...)
{
  va_list argptr;
  char txtptr[2000];
  char msgptr[2050]; 

  if (disable_dialogs) return;

  /* Print text in a string */
  va_start(argptr, format);
  vsprintf (txtptr, format, argptr);
  va_end(argptr);

  if (get_the_interpreter() == NULL) {
      fprintf(stderr, "FATAL ERROR: %s\n", txtptr);
      // XXX Is this the right thing? Will it attempt to save stream?
      exit(-1);
  } else {
      sprintf(msgptr, "nano_fatal_error {%s}", txtptr);
      TCLEVALCHECK2(get_the_interpreter(), msgptr);
  }
}
