#include <stdio.h>  // for vfprintf()
#include <stdarg.h>  // for va_start(), va_end(), va_list

int spm_verbosity;
int collab_verbosity;

void collabVerbose (int level, char * msg, ...) {
  va_list argptr;

  if (collab_verbosity >= level) {
    va_start(argptr, msg);
    vfprintf(stderr, msg, argptr);
    va_end(argptr);
  }

}


