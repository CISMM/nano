/*===3rdtech===
  Copyright (c) 2000 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#include <Tcl_Interpreter.h>
#include <Tcl_Linkvar.h>
#include <tcl.h>

#include <string.h>  // for strlen()
#include <stdio.h>

#include <nmb_Globals.h> // for decoration
#include <nmb_Decoration.h>  //for decoration->elapsedTime
#include "ModFile.h"

static const int IMAGEMODE = 0;
static const int MODMODE = 1;


// ModFile
//
// Tom Hudson
// Code from modfile_ functions and globals in animate.c



Tclvar_int modfile_hasWindow ("modfile_hasWindow", 0); 
                                // it won't work the other way unless
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
  me->d_start_mod_time = decoration->elapsedTime;

  return 0;
}

// static
int ModFile::EnterImageMode (void * userdata) {
  ModFile * me = (ModFile *) userdata;

  if (me->d_lastmode == IMAGEMODE) return 0;

  me->d_interp = Tcl_Interpreter::getInterpreter();
  if (!modfile_hasWindow) {
    me->ShowModFile();
    modfile_hasWindow = 1;
  }
  me->d_pointlist.writeToTclWindow(me->d_interp);
  me->d_pointlist.clear();

  me->RememberPointList();

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
    // Turns out we don't want to pop up the modfile window. Ever.
    /*
   char command [100];
   sprintf(command, "show_mod_win");
   TCLEVALCHECK2(d_interp, command);
    */
}

void ModFile::RememberPointList (void) {
   char command [100];
   d_interp = Tcl_Interpreter::getInterpreter();
   // This command includes a timestamp so we know which mod it is. 
   sprintf(command, "remember_mod_data %ld", d_start_mod_time);
   TCLEVALCHECK2(d_interp, command);
}

