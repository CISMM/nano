#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <GL/glut_UNC.h>

#include <nmb_Dataset.h>

#include "URender.h"
#include "ProjTextObjGenerator.h"

#include "quat.h"

// Created by Borland 04/26/02
// Handles the creation of Projective Texture Objects.  In essence, we use the UberGraphics functionality to 
// be able to control the transformation of projective textures through the object loading interface


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


extern nmb_Dataset * dataset;



ProjTextObjGenerator::ProjTextObjGenerator(const char *fname) : FileGenerator(fname, "ptx")
{
}


void BuildListProjTextObj(URender *Pobject, GLuint dl);


int ProjTextObjGenerator::ReLoad(URender *Pobject, GLuint *&Dlist_array) {
	return Load(Pobject, Dlist_array);
}



// This function creates a Projective Texture Object, used to control the transfrom for the Projective Texture
int ProjTextObjGenerator::Load(URender *Pobject, GLuint *&Dlist_array)
{
	GLuint dl;

	double minX, minY, maxX, maxY, z_value;

	// pointer to the current plane
	BCPlane *height = dataset->inputGrid->getPlaneByName(dataset->heightPlaneName->string());

	// set up min and max values for the current plane
	minX = height->minX();
	minY = height->minY();
	maxX = height->maxX();
	maxY = height->maxY();
	z_value = height->minNonZeroValue();

	//	set up display list id
	Dlist_array = new GLuint[1];
    dl = glGenLists(1);
    if (dl == 0 || Dlist_array == NULL) { 
		cerr << "Bad Display List generation\n"; 
        //kill(getpid(),SIGINT); 
        return 0;
	}
        
	BuildListProjTextObj(Pobject, dl);
		
	Dlist_array[0] = dl;

	// add minimum extents of the height plane
	Pobject->GetLocalXform().SetXOffset(minX);
	Pobject->GetLocalXform().SetYOffset(minY);
	Pobject->GetLocalXform().SetZOffset(z_value);

	Pobject->GetLocalXform().SetTranslate((maxX - minX) / 2, (maxY - minY) / 2, 0);

	return 1;	// number of display lists
}



void BuildListProjTextObj(URender *Pobject, GLuint dl) {
	// Create an object that will point in the direction of the projective texture...
	double radius = 200;
	double height = 800;
	double slices = 200;
	double stacks = 200;

	double width = radius / 8;
	double length = radius + 80.0;

	glNewList(dl, GL_COMPILE);	// init display list
	
	glColor3f(0.9, 0.4, 0.0);

	// draw cone
	glutSolidCone(radius, height, slices, stacks);

	// draw sight
	glBegin(GL_QUADS);
	// right side
	glVertex3d(width, 0.0, 0.0);
	glVertex3d(width, length, 0.0);
	glVertex3d(width, length, radius);
	glVertex3d(width, 0.0, radius);

	// left side
	glVertex3d(-width, 0.0, 0.0);
	glVertex3d(-width, length, 0.0);
	glVertex3d(-width, length, radius);
	glVertex3d(-width, 0.0, radius);

	// back
	glVertex3d(-width, 0.0, 0.0);
	glVertex3d(width, 0.0, 0.0);
	glVertex3d(width, 0.0, radius);
	glVertex3d(-width, 0.0, radius);

	// front
	glVertex3d(-width, length, 0.0);
	glVertex3d(width, length, 0.0);
	glVertex3d(width, length, radius);
	glVertex3d(-width, length, radius);

	// top
	glVertex3d(-width, 0.0, radius);
	glVertex3d(width, 0.0, radius);
	glVertex3d(width, length, radius);
	glVertex3d(-width, length, radius);
	glEnd();

	// draw center line
	glBegin(GL_LINES);
	glVertex3d(0.0, 0.0, 10000.0);
	glVertex3d(0.0, 0.0, -10000.0);
	glEnd();

	glEndList();
}
