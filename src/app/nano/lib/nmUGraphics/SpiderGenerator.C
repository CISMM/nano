#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <Tcl_Linkvar.h>
#include <Tcl_Netvar.h>

#include <nmb_Dataset.h>

#include "URender.h"
#include "URSpider.h"
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

#define MAXLENGTH 512


extern nmb_Dataset * dataset;



SpiderGenerator::SpiderGenerator(const char *fname) : FileGenerator(fname, "spi")
{
}


void BuildListSpider(URSpider *Pobject, GLuint dl);


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
        
	BuildListSpider((URSpider*)Pobject, dl);
		
	Dlist_array[0] = dl;

	return 1;	// number of display lists
}


// parse the info in the .spi file
void ParseFileSpider(URender *Pobject, const char* filename) {
	char buffer[512];
	char* token;
	ifstream readfile;

	extern int current_leg;		// found in import.C

	URSpider* spider = (URSpider*)Pobject;

	readfile.open(filename);
    assert(readfile);

    if(readfile.bad()) {
		cerr << "Unable to open input file" << endl;
        return;
    }

	current_leg = -1;
	while(!readfile.eof()) {
		readfile.getline(buffer, 512);

		token = strtok(buffer, " \t\n");

		if (token != NULL) {
			if (strcmp(token, "Leg") == 0) {
				current_leg++;
			}
			else if (strcmp(token, "Length") == 0) {
				token = strtok(NULL, " \t\n");
				spider->SetSpiderLength(current_leg, atof(token));
			}
			else if (strcmp(token, "Width") == 0) {
				token = strtok(NULL, " \t\n");
				spider->SetSpiderWidth(current_leg, atof(token));
			}
			else if (strcmp(token, "Thickness") == 0) {
				token = strtok(NULL, " \t\n");
				spider->SetSpiderThick(current_leg, atof(token));
			}
			else if (strcmp(token, "Begin") == 0) {
				token = strtok(NULL, " \t\n");
				spider->SetSpiderBegCurve(current_leg, Q_DEG_TO_RAD(atof(token)));
			}
            else if (strcmp(token, "End") == 0) {
				token = strtok(NULL, " \t\n");
				spider->SetSpiderEndCurve(current_leg, Q_DEG_TO_RAD(atof(token)));
			}
		}
	}
	spider->SetSpiderLegs(current_leg + 1);
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

	// if not the default /spider.spi, we are loading from a file, so parse the file
	if (strcmp(filename, "/spider.spi") != 0) {
		ParseFileSpider(Pobject, filename);
	}

	//	set up display list id
	Dlist_array = new GLuint[1];
    dl = glGenLists(1);
    if (dl == 0 || Dlist_array == NULL) { 
		cerr << "Bad Display List generation\n"; 
        //kill(getpid(),SIGINT); 
        return 0;
	}
        
	BuildListSpider((URSpider*)Pobject, dl);
		
	Dlist_array[0] = dl;

	// add minimum extents of the height plane
	Pobject->GetLocalXform().SetXOffset(minX);
	Pobject->GetLocalXform().SetYOffset(minY);
	Pobject->GetLocalXform().SetZOffset(z_value);
	Pobject->GetLocalXform().SetTranslate(0, 0, 0);

	return 1;	// number of display lists
}



void BuildListSpider(URSpider *Pobject, GLuint dl) {
	int i, j, k;
	q_vec_type p[8];	// 8 points per section
	q_vec_type n;
    q_vec_type trans;
	q_type q;

	int legs = Pobject->GetSpiderLegs();
	int tess;
	double x;
	double y;
	double z;

	double rot_step = 2 * Q_PI / legs;
	double rot;

	double beg_curve;
    double end_curve;
    double curve_step;

	double cur_curve;

	glNewList(dl, GL_COMPILE);	// init display list
	rot = 0.0;
	for (i = 0; i < legs; i++) {
		tess = Pobject->GetSpiderTess(i);
		x = Pobject->GetSpiderLength(i) / tess;
		y = Pobject->GetSpiderWidth(i) / 2;
		z = Pobject->GetSpiderThick(i) / 2;

		glBegin(GL_QUADS);

		beg_curve = Pobject->GetSpiderBegCurve(i) / tess;
        end_curve = Pobject->GetSpiderEndCurve(i) / tess;
        curve_step = (end_curve - beg_curve) / tess;

		// set up middle points
		q_from_euler(q, rot, 0, 0);

        q_vec_set(p[0], 0, y, -z);
		q_vec_set(p[1], 0, -y, -z);
		q_vec_set(p[2], 0, -y, z);
		q_vec_set(p[3], 0, y, z);

		for (k = 0; k < 4; k++) {
			q_xform(p[k], q, p[k]);
		}

		// end1
		q_vec_set(n, -1, 0, 0);
		q_xform(n, q, n);
		q_vec_normalize(n, n);
		glNormal3f(n[0], n[1], n[2]);

		glVertex3f(p[0][0], p[0][1], p[0][2]);
		glVertex3f(p[1][0], p[1][1], p[1][2]);
		glVertex3f(p[2][0], p[2][1], p[2][2]);
		glVertex3f(p[3][0], p[3][1], p[3][2]);

		cur_curve = beg_curve;	
        q_vec_set(trans, 0, 0, 0);
		for (j = 0; j < tess; j++) {
			q_from_euler(q, rot, cur_curve, 0);

			q_vec_set(p[4], x, y, -z);
			q_vec_set(p[5], x, -y, -z);
			q_vec_set(p[6], x, -y, z);
			q_vec_set(p[7], x, y, z);

			for (k = 4; k < 8; k++) {
				q_xform(p[k], q, p[k]);
			}

            for (k = 4; k < 8; k++) {
                q_vec_add(p[k], p[k], trans);
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
            q_vec_set(trans, 0, 0, 0);
			for (k = 0; k < 4; k++) {
				q_vec_copy(p[k], p[k + 4]);
                q_vec_add(trans, trans, p[k]);
			}

            // get translation vector
            q_vec_set(trans, trans[0] / 4, trans[1] / 4, trans[2] / 4);

			cur_curve += beg_curve + (j + 1) * curve_step;
		}
		// end2
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
}
