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

  // This allocates the string, and passes it to us. 
  string * text = me->d_pointlist.outputToText();
  me->d_pointlist.clear();

  me->RememberPointList(text);
  delete text;

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

/** Put the text of the modfile into the tcl interpreter, then tell the
 interpreter it is there so the use can save it later.
  */
void ModFile::RememberPointList (string * text) {
    if( text == NULL ) return;
    d_interp = Tcl_Interpreter::getInterpreter();
    char * c_text = new char[strlen(text->c_str()) +1];
    strcpy(c_text, text->c_str());
    // Can't use tclvar_string, because it has a length limit of 128 char. 
    Tcl_SetVar(d_interp,"modfile_text",c_text,TCL_GLOBAL_ONLY);
    delete [] c_text;

    char command [100];
    // This command includes a timestamp so we know which mod it is. 
    sprintf(command, "remember_mod_data %ld", d_start_mod_time);
    TCLEVALCHECK2(d_interp, command);
}

