#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <Tcl_Linkvar.h>
#include <Tcl_Netvar.h>

#include <nmb_Dataset.h>

#include "URender.h"
#include "DsAxisGenerator.h"

#include "quat.h"

// Created by Jameson
// Handles the creation of axis for direct Step


#ifdef __CYGWIN__
// taken from WaveFrontFileGenerator.C
// XXX juliano 9/19/99
//       this was implicitly declared.  Needed to add decl.
//       getpid comes from unistd.h
#include <sys/types.h>  // for pid_t
extern "C" {
pid_t getpid();
}
#endif

#define MAXLENGTH 512


extern nmb_Dataset * dataset;
DsAxisGenerator::DsAxisGenerator(const char *fname) : FileGenerator(fname, "dsa")
{
}


int DsAxisGenerator::Load(URender *Pobject, GLuint *&Dlist_array)
{
	GLuint dl;

	double minX, minY, z_value;

	// pointer to the current plane
	BCPlane *height = dataset->inputGrid->getPlaneByName(dataset->heightPlaneName->string());

	// set up min and max values for the current plane
	minX = height->minX();
	minY = height->minY();
	z_value = height->minNonZeroValue();

	//	set up display list id
	Dlist_array = new GLuint[1];
    dl = glGenLists(1);
    if (dl == 0 || Dlist_array == NULL) { 
		cerr << "Bad Display List generation\n"; 
        //kill(getpid(),SIGINT); 
        return 0;
	}
        
	BuildListDsAxis(Pobject, dl);
		
	Dlist_array[0] = dl;

	// add minimum extents of the height plane
	Pobject->GetLocalXform().SetXOffset(minX);
	Pobject->GetLocalXform().SetYOffset(minY);
	Pobject->GetLocalXform().SetZOffset(z_value);
	Pobject->GetLocalXform().SetTranslate(0, 0, 0);

	Pobject->GetLocalXform().SetScale(350);

	return 1;	// number of display lists
}

int DsAxisGenerator::ReLoad(URender *Pobject, GLuint *&Dlist_array) {
return 0;
}

void DsAxisGenerator::BuildListDsAxis(URender *Pobject, GLuint dl) {

	glNewList(dl, GL_COMPILE);	// init display list
	
	glBegin(GL_LINES);
	glColor3f(1.0, 0.0, 0.0);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(1.0, 0.0, 0.0);
	glEnd();

	glBegin(GL_LINES);
	glColor3f(0.0, 1.0, 0.0);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(0.0, 1.0, 0.0);
	glEnd();

	glBegin(GL_LINES);
	glColor3f(0.0, 0.0, 1.0);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(0.0, 0.0, 1.0);
	glEnd();

	glEndList();

	return;
}
