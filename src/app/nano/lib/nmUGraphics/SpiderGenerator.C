#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <nmb_Dataset.h>

#include "URender.h"
#include "SpiderGenerator.h"

#include "quat.h"

// Created by Borland 04/26/02
// Handles the creation of spiders


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



SpiderGenerator::SpiderGenerator(const char *fname) : FileGenerator(fname, "spi")
{
}


void BuildList(URender *Pobject, GLuint dl);


int SpiderGenerator::ReLoad(URender *Pobject, GLuint *&Dlist_array) {
	// basically the same as Load, but doesn't set the translation stuff
	GLuint dl;

	//	set up display list id
	Dlist_array = new GLuint[1];
    dl = glGenLists(1);
    if (dl == 0 || Dlist_array == NULL) { 
		cerr << "Bad Display List generation\n"; 
        //kill(getpid(),SIGINT); 
        return 0;
	}
        
	BuildList(Pobject, dl);
		
	Dlist_array[0] = dl;

	return 1;	// number of display lists
}



// This function creates a spider
int SpiderGenerator::Load(URender *Pobject, GLuint *&Dlist_array)
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
        
	BuildList(Pobject, dl);
		
	Dlist_array[0] = dl;

	// add minimum extents of the height plane
	Pobject->GetLocalXform().SetXOffset(minX);
	Pobject->GetLocalXform().SetYOffset(minY);
	Pobject->GetLocalXform().SetZOffset(z_value);
	Pobject->GetLocalXform().SetTranslate(0, 0, 0);

	return 1;	// number of display lists
}



void BuildList(URender *Pobject, GLuint dl) {
	int i, j, k;
	q_vec_type p[8];	// 8 points per section
	q_vec_type n;
	q_type q;

	int tess = Pobject->GetSpiderTess();
	double x = Pobject->GetSpiderLength();
	double y = Pobject->GetSpiderWidth() / 2;
	double z = Pobject->GetSpiderThick() / 2;

	double rot_step = Q_PI / 4;
	double rot;

/**********************************************************************************/
	
	// special case for curve == 0.0
	// don't want to divide by zero, and can calculate much more easily
	if (Pobject->GetSpiderCurve() == 0.0) {
		glNewList(dl, GL_COMPILE);
		rot = 0.0;
		for (i = 0; i < 4; i++) {
			glBegin(GL_QUADS);

			// set up middle points
			q_from_euler(q, rot, 0, 0);

			q_vec_set(p[0], -x, y, -z);
			q_vec_set(p[1], -x, -y, -z);
			q_vec_set(p[2], -x, -y, z);
			q_vec_set(p[3], -x, y, z);
			q_vec_set(p[4], x, y, -z);
			q_vec_set(p[5], x, -y, -z);
			q_vec_set(p[6], x, -y, z);
			q_vec_set(p[7], x, y, z);

			for (j = 0; j < 8; j++) {
				q_xform(p[j], q, p[j]);
			}

			// bottom
			q_vec_set(n, 0, 0, -1);
			q_xform(n, q, n);
			q_vec_normalize(n, n);
			glNormal3f(n[0], n[1], n[2]);

			glVertex3f(p[0][0], p[0][1], p[0][2]);
			glVertex3f(p[1][0], p[1][1], p[1][2]);
			glVertex3f(p[5][0], p[5][1], p[5][2]);
			glVertex3f(p[4][0], p[4][1], p[4][2]);

			// side 
			q_vec_set(n, 0, -1, 0);
			q_xform(n, q, n);
			q_vec_normalize(n, n);
			glNormal3f(n[0], n[1], n[2]);

			glVertex3f(p[1][0], p[1][1], p[1][2]);
			glVertex3f(p[5][0], p[5][1], p[5][2]);
			glVertex3f(p[6][0], p[6][1], p[6][2]);
			glVertex3f(p[2][0], p[2][1], p[2][2]);

			// top 
			q_vec_set(n, 0, 0, 1);
			q_xform(n, q, n);
			q_vec_normalize(n, n);
			glNormal3f(n[0], n[1], n[2]);

			glVertex3f(p[3][0], p[3][1], p[3][2]);
			glVertex3f(p[2][0], p[2][1], p[2][2]);
			glVertex3f(p[6][0], p[6][1], p[6][2]);
			glVertex3f(p[7][0], p[7][1], p[7][2]);

			// side 
			q_vec_set(n, 0, 1, 0);
			q_xform(n, q, n);
			q_vec_normalize(n, n);
			glNormal3f(n[0], n[1], n[2]);

			glVertex3f(p[0][0], p[0][1], p[0][2]);
			glVertex3f(p[3][0], p[3][1], p[3][2]);
			glVertex3f(p[7][0], p[7][1], p[7][2]);
			glVertex3f(p[4][0], p[4][1], p[4][2]);

			// end1
			q_vec_set(n, -1, 0, 0);
			q_xform(n, q, n);
			q_vec_normalize(n, n);
			glNormal3f(n[0], n[1], n[2]);

			glVertex3f(p[0][0], p[0][1], p[0][2]);
			glVertex3f(p[1][0], p[1][1], p[1][2]);
			glVertex3f(p[2][0], p[2][1], p[2][2]);
			glVertex3f(p[3][0], p[3][1], p[3][2]);

			// end2
			q_vec_set(n, 1, 0, 0);
			q_xform(n, q, n);
			q_vec_normalize(n, n);
			glNormal3f(n[0], n[1], n[2]);

			glVertex3f(p[4][0], p[4][1], p[4][2]);
			glVertex3f(p[7][0], p[7][1], p[7][2]);
			glVertex3f(p[6][0], p[6][1], p[6][2]);
			glVertex3f(p[5][0], p[5][1], p[5][2]);

			for (k = 0; k < 8; k++) {
				Pobject->UpdateBoundsWithPoint(p[k][0], p[k][1], p[k][2]);
			}

			rot += rot_step;
			glEnd();
		}
		glEndList();
		return;
	}

/**********************************************************************************/


	// curve not equal to zero


	double curve = Pobject->GetSpiderCurve() / tess;
	double radius = x / Pobject->GetSpiderCurve();

	double cur_curve;

	glNewList(dl, GL_COMPILE);	// init display list
	rot = 0.0;
	for (i = 0; i < 8; i++) {
		glBegin(GL_QUADS);

		// set up middle points
		q_from_euler(q, rot, 0, 0);

		q_vec_set(p[0], 0, y, radius - z);
		q_vec_set(p[1], 0, -y, radius - z);
		q_vec_set(p[2], 0, -y, radius + z);
		q_vec_set(p[3], 0, y, radius + z);

		for (k = 0; k < 4; k++) {
			q_xform(p[k], q, p[k]);
		}

		for (k = 0; k < 4; k++) {
			p[k][2] -= radius;
		}

		cur_curve = curve;	
		for (j = 0; j < tess; j++) {
			q_from_euler(q, rot, cur_curve, 0);

			q_vec_set(p[4], 0, y, radius - z);
			q_vec_set(p[5], 0, -y, radius - z);
			q_vec_set(p[6], 0, -y, radius + z);
			q_vec_set(p[7], 0, y, radius + z);

			for (k = 4; k < 8; k++) {
				q_xform(p[k], q, p[k]);
			}

			for (k = 4; k < 8; k++) {
				p[k][2] -= radius;
			}

			// bottom
			q_vec_set(n, 0, 0, -1);
			q_xform(n, q, n);
			q_vec_normalize(n, n);
			glNormal3f(n[0], n[1], n[2]);

			glVertex3f(p[0][0], p[0][1], p[0][2]);
			glVertex3f(p[1][0], p[1][1], p[1][2]);
			glVertex3f(p[5][0], p[5][1], p[5][2]);
			glVertex3f(p[4][0], p[4][1], p[4][2]);

			// side 
			q_vec_set(n, 0, -1, 0);
			q_xform(n, q, n);
			q_vec_normalize(n, n);
			glNormal3f(n[0], n[1], n[2]);

			glVertex3f(p[1][0], p[1][1], p[1][2]);
			glVertex3f(p[5][0], p[5][1], p[5][2]);
			glVertex3f(p[6][0], p[6][1], p[6][2]);
			glVertex3f(p[2][0], p[2][1], p[2][2]);

			// top 
			q_vec_set(n, 0, 0, 1);
			q_xform(n, q, n);
			q_vec_normalize(n, n);
			glNormal3f(n[0], n[1], n[2]);

			glVertex3f(p[3][0], p[3][1], p[3][2]);
			glVertex3f(p[2][0], p[2][1], p[2][2]);
			glVertex3f(p[6][0], p[6][1], p[6][2]);
			glVertex3f(p[7][0], p[7][1], p[7][2]);

			// side 
			q_vec_set(n, 0, 1, 0);
			q_xform(n, q, n);
			q_vec_normalize(n, n);
			glNormal3f(n[0], n[1], n[2]);

			glVertex3f(p[0][0], p[0][1], p[0][2]);
			glVertex3f(p[3][0], p[3][1], p[3][2]);
			glVertex3f(p[7][0], p[7][1], p[7][2]);
			glVertex3f(p[4][0], p[4][1], p[4][2]);

			// set up for next segment
			for (k = 0; k < 4; k++) {
				q_vec_copy(p[k], p[k + 4]);
			}

			cur_curve += curve;
		}
		// end
		q_vec_set(n, 1, 0, 0);
		q_xform(n, q, n);
		q_vec_normalize(n, n);
		glNormal3f(n[0], n[1], n[2]);

		glVertex3f(p[0][0], p[0][1], p[0][2]);
		glVertex3f(p[3][0], p[3][1], p[3][2]);
		glVertex3f(p[2][0], p[2][1], p[2][2]);
		glVertex3f(p[1][0], p[1][1], p[1][2]);

		for (k = 0; k < 4; k++) {
			Pobject->UpdateBoundsWithPoint(p[k][0], p[k][1], p[k][2]);
		}

		rot += rot_step;
		glEnd();
	}
	glEndList();






// THIS WAY LOOKS GOOD, BUT I DON'T THINK HANDLES THE CURVATURE PROPERLY

/*
	int i, j, k;
	q_vec_type p[8];	// 8 points per section
	q_vec_type mid;
	q_vec_type n;
	q_type q;

	int tess = Pobject->GetSpiderTess();
	double y = Pobject->GetSpiderWidth() / 2;
	double z = Pobject->GetSpiderThick() / 2;
	double curve = Pobject->GetSpiderCurve() / tess;
	double seg_length = Pobject->GetSpiderLength() / tess;

	double cur_curve;
	double x;

	double rot_step = Q_PI / 4;
	double rot;

	glNewList(dl, GL_COMPILE);	// init display list
	rot = 0.0;
    for (i = 0; i < 8; i++) {
		glBegin(GL_QUADS);

		// set up middle points
		q_from_euler(q, rot, 0, 0);

		q_vec_set(p[0], 0, y, -z);
		q_xform(p[0], q, p[0]);
	
		q_vec_set(p[1], 0, -y, -z);
		q_xform(p[1], q, p[1]);

		q_vec_set(p[2], 0, -y, z);
		q_xform(p[2], q, p[2]);

		q_vec_set(p[3], 0, y, z);
		q_xform(p[3], q, p[3]);

		q_vec_set(mid, 0, 0, 0);

		cur_curve = curve;
		x = seg_length;
		for (j = 0; j < tess; j++) {
			q_from_euler(q, rot, cur_curve, 0);

			q_vec_set(p[4], x, y, -z);
			q_xform(p[4], q, p[4]);
	
			q_vec_set(p[5], x, -y, -z);
			q_xform(p[5], q, p[5]);

			q_vec_set(p[6], x, -y, z);
			q_xform(p[6], q, p[6]);

			q_vec_set(p[7], x, y, z);
			q_xform(p[7], q, p[7]);

			// bottom
			q_vec_set(n, 0, 0, -1);
			q_xform(n, q, n);
			q_vec_normalize(n, n);
			glNormal3f(n[0], n[1], n[2]);

			glVertex3f(p[0][0], p[0][1], p[0][2]);
			glVertex3f(p[1][0], p[1][1], p[1][2]);
			glVertex3f(p[5][0], p[5][1], p[5][2]);
			glVertex3f(p[4][0], p[4][1], p[4][2]);

			// side 
			q_vec_set(n, 0, -1, 0);
			q_xform(n, q, n);
			q_vec_normalize(n, n);
			glNormal3f(n[0], n[1], n[2]);

			glVertex3f(p[1][0], p[1][1], p[1][2]);
			glVertex3f(p[5][0], p[5][1], p[5][2]);
			glVertex3f(p[6][0], p[6][1], p[6][2]);
			glVertex3f(p[2][0], p[2][1], p[2][2]);

			// top 
			q_vec_set(n, 0, 0, 1);
			q_xform(n, q, n);
			q_vec_normalize(n, n);
			glNormal3f(n[0], n[1], n[2]);

			glVertex3f(p[3][0], p[3][1], p[3][2]);
			glVertex3f(p[2][0], p[2][1], p[2][2]);
			glVertex3f(p[6][0], p[6][1], p[6][2]);
			glVertex3f(p[7][0], p[7][1], p[7][2]);

			// side 
			q_vec_set(n, 0, 1, 0);
			q_xform(n, q, n);
			q_vec_normalize(n, n);
			glNormal3f(n[0], n[1], n[2]);

			glVertex3f(p[0][0], p[0][1], p[0][2]);
			glVertex3f(p[3][0], p[3][1], p[3][2]);
			glVertex3f(p[7][0], p[7][1], p[7][2]);
			glVertex3f(p[4][0], p[4][1], p[4][2]);

			// set up for next segment
			for (k = 0; k < 4; k++) {
				q_vec_copy(p[k], p[k + 4]);
			}

			cur_curve += curve;
			x += seg_length;
		}
		// end
		q_vec_set(n, 1, 0, 0);
		q_xform(n, q, n);
		q_vec_normalize(n, n);
		glNormal3f(n[0], n[1], n[2]);

		glVertex3f(p[0][0], p[0][1], p[0][2]);
		glVertex3f(p[3][0], p[3][1], p[3][2]);
		glVertex3f(p[2][0], p[2][1], p[2][2]);
		glVertex3f(p[1][0], p[1][1], p[1][2]);

		for (k = 0; k < 4; k++) {
			Pobject->UpdateBoundsWithPoint(p[k][0], p[k][1], p[k][2]);
		}

		rot += rot_step;
		glEnd();
	}
	glEndList();
*/
}