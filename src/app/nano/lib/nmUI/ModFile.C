#include <Tcl_Linkvar.h>
#include <tcl.h>

#include <string.h>  // for strlen()
#include <stdio.h>

#include "ModFile.h"

static const int IMAGEMODE = 0;
static const int MODMODE = 1;


extern Tcl_Interp * get_the_interpreter (void);

// ModFile
//
// Tom Hudson
// Code from modfile_ functions and globals in animate.c



Tclvar_int modfile_hasWindow ("modfile_hasWindow", 0); // it won't work the other way unless
				// we correctly link the Tcl variable with
				// the C variable - AAS
//static int modfile_hasWindow = 0;

ModFile::ModFile (void) :
    d_lastmode (IMAGEMODE),
    d_interp (NULL),
    d_pointlist () {

//fprintf(stderr, "ModFile constructor\n");
}


ModFile::~ModFile (void) {
}

// static
int ModFile::EnterModifyMode (void * userdata) {
  ModFile * me = (ModFile *) userdata;

  //fprintf(stderr, "ModFile::EnterModifyMode().\n");
  me->d_lastmode = MODMODE;

  return 0;
}

// static
int ModFile::EnterImageMode (void * userdata) {
  ModFile * me = (ModFile *) userdata;

  if (me->d_lastmode == IMAGEMODE) return 0;

  if (!modfile_hasWindow) {
    me->d_interp = get_the_interpreter();
    me->ShowModFile();
    modfile_hasWindow = 1;
  }
  me->d_pointlist.writeToTclWindow(me->d_interp);
  me->d_pointlist.clear();

  me->d_lastmode = IMAGEMODE;

  return 0;
}

// static
int ModFile::ReceiveNewPoint (void * userdata, const Point_results * _p) {
  ModFile * me = (ModFile *) userdata;

  if (me->d_lastmode != MODMODE)
    return 0;  // Not an error - this is a feel result

  me->d_pointlist.addEntry(*_p);

  return 0;
}

void ModFile::ShowModFile (void) {
   char command [100];
   sprintf(command, "show_mod_win");
   if (Tcl_Eval(d_interp, command) != TCL_OK) {
      fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command,
	      d_interp->result);
   }	
}


