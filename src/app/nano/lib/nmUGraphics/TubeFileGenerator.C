#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <string.h>

#include <vector>

#ifdef sgi
using std::vector;
#endif


#include "URender.h"
#include "TubeFileGenerator.h"

#include "quat.h"

// Created by Borland 02/21/02
// Handles the creation of objects from Shape Analysis output files


#ifdef sgi
// isspace and isalnum are found in ctype.h on the sgi's
#include <ctype.h>
#endif

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

#define MAXLENGTH	512		// buffer size

const double PI = 3.1415926535;
const int TESS = 6;		// number of points around the axis--should be user-controlled?




TubeFileGenerator::TubeFileGenerator(const char* fname) 
	: FileGenerator(fname, "txt")
{
}


int TubeFileGenerator::ReLoad(URender *Pobject, GLuint *&Dlist_array) {
	return Load(Pobject, Dlist_array);
}


typedef vector<float> vertex;
typedef vector<vertex> verts;
//typedef vector<verts> tubes;


void BuildList(URender *Pobject, GLuint dl, verts vs);




// This function reads Shape Analysis output file
// it will create an openGL display list for each logical object
int TubeFileGenerator::Load(URender *Pobject, GLuint *&Dlist_array)
{
	char buffer[MAXLENGTH];
	char* token;
	ifstream readfile;

	float radius, x, y, z, az, alt;
	double theta;
//	tubes t;				// gives a bunch of warnings because the name is too big...
	verts t[10];			// go with static number for now...
	verts vs;
	vertex v;

	q_vec_type p1;
	q_vec_type p2;
	q_type q;

	int numtubes = 0;

	GLuint dl;


	readfile.open(filename);
    assert(readfile);

    if(readfile.bad()) {
		cerr << "Unable to open input file" << endl;
        return 0;
    }
	readfile.getline(buffer, MAXLENGTH);	// skip first line, it doesn't give any data
	while(!readfile.eof()) {
		readfile.getline(buffer, MAXLENGTH);

		if (*buffer != '\0') {
			token = strtok(buffer, " \t\n");
 
			if (strcmp(token, "radius") == 0) {
				// if not first tube, add to tubes
				if (vs.size() != 0) {
					t[numtubes++] = vs;
					vs.clear();
				}
	
				// get the radius of the new tube
				token = strtok(NULL, " =\t\n");
				radius = atof(token);
			}
			else {			// should contain numerical data
				token = strtok(NULL, " \t\n");		// skip X and Y
				token = strtok(NULL, " \t\n");

				x = atof(token);
				token = strtok(NULL, " \t\n");
				y = atof(token);
				token = strtok(NULL, " \t\n");
				z = atof(token);
				token = strtok(NULL, " \t\n");
				az = atof(token);
				token = strtok(NULL, " \t\n");
				alt = atof(token);

				// set up medial axis point
				q_vec_set(p1, x, y, z);

				// set up rotation quat
				q_from_euler(q, az + PI / 2, 0, alt);

				// get vertices
				theta = 0.0;
				for (int i = 0; i < TESS; i++) {
					v.clear();

					// set point
					q_vec_set(p2,	x + radius * cos(theta),
									y,
									z + radius * sin(theta));

					// translate point to origin
					q_vec_subtract(p2, p2, p1);
					
					// rotate
					q_xform(p2, q, p2);

					// translate back
					q_vec_add(p2, p2, p1);

					v.push_back(p2[0]);
					v.push_back(p2[1]);
					v.push_back(p2[2]);

					vs.push_back(v);
					theta += 2 * PI / TESS;

					Pobject->num_triangles += 2;	// two triangles per vertex
				}
			}
		}
	}
	// add last tube
	t[numtubes++] = vs;


	// create space for triangles in Pobject
	Pobject->num_triangles -= numtubes * TESS * 2;
	Pobject->triangles = new float* [Pobject->num_triangles * 3];
	for (int i = 0; i < Pobject->num_triangles * 3; i++) {
		Pobject->triangles[i] = new float[4];
	}





	// create geometry from list of vertices

	//	set up display list id's
	Dlist_array = new GLuint[numtubes];
    dl = glGenLists(numtubes);
    if (dl == 0 || Dlist_array == NULL) { 
		cerr << "Bad Display List generation\n"; 
        //kill(getpid(),SIGINT); 
        return 0;
	}

	for (i = 0; i < numtubes; i++){
		//BuildList actually builds the geometry from
		//the data structures previously built
        BuildList(Pobject, dl + i, t[i]);
		Dlist_array[i] = dl + i;
	}




	for (i = 0; i < Pobject->num_triangles * 3; i++) {
		for (int j = 0; j < 3; j++) {
			printf("%f\t", Pobject->triangles[i][j]);
		}
		printf("\n");
	}

	readfile.close();

	return numtubes;  // should be number of display lists
}



void BuildList(URender *Pobject, GLuint dl, verts vs) {
	float v1[4];
	float v2[4];
	float v3[4];
	float v4[4];

	q_vec_type n1;
	q_vec_type n2;

	static int count = 0;

	glNewList(dl, GL_COMPILE);	// init display list

	for (int i = 0; i < vs.size() - TESS; i++) {
		glBegin(GL_TRIANGLES);
		for (int k = 0; k < 3; k++) {
			// special case for last two triangles per segment
			if ((i + 1) >= TESS && (i + 1) % TESS == 0) {
				v1[k] = vs[i][k];
				v2[k] = vs[i + TESS][k];
				v3[k] = vs[i + 1][k];
				v4[k] = vs[i - TESS + 1][k];
			}
			else {
				v1[k] = vs[i][k];
				v2[k] = vs[i + TESS][k];
				v3[k] = vs[i + TESS + 1][k];
				v4[k] = vs[i + 1][k];
			}
		}
		v1[k] = v2[k] = v3[k] = v4[k] = 1.0;


		// Compute the normal...only flat shading for now...should be good enough???
		q_vec_set(n1, v4[0] - v2[0],
						v4[1] - v2[1],
						v4[2] - v2[2]);

		q_vec_set(n2, v1[0] - v4[0],
						v1[1] - v4[1],
						v1[2] - v4[2]);
		

		q_vec_cross_product(n1, n2, n1);

		q_vec_normalize(n1, n1);
	
		glNormal3f(n1[0], n1[1], n1[2]);
		glVertex4fv(v1);
		glVertex4fv(v2);
		glVertex4fv(v4);


		Pobject->triangles[count][0] = v1[0];
		Pobject->triangles[count][1] = v1[1];
		Pobject->triangles[count][2] = v1[2];
		Pobject->triangles[count++][3] = 1.0;

		Pobject->triangles[count][0] = v2[0];
		Pobject->triangles[count][1] = v2[1];
		Pobject->triangles[count][2] = v2[2];
		Pobject->triangles[count++][3] = 1.0;

		Pobject->triangles[count][0] = v4[0];
		Pobject->triangles[count][1] = v4[1];
		Pobject->triangles[count][2] = v4[2];
		Pobject->triangles[count++][3] = 1.0;

		// Compute the normal...only flat shading for now...should be good enough???
		q_vec_set(n1, v4[0] - v3[0],
						v4[1] - v3[1],
						v4[2] - v3[2]);

		q_vec_set(n2, v2[0] - v4[0],
						v2[1] - v4[1],
						v2[2] - v4[2]);
		

		q_vec_cross_product(n1, n2, n1);

		q_vec_normalize(n1, n1);
	
		glNormal3f(n1[0], n1[1], n1[2]);
		glVertex4fv(v2);
		glVertex4fv(v3);
		glVertex4fv(v4);


		Pobject->triangles[count][0] = v2[0];
		Pobject->triangles[count][1] = v2[1];
		Pobject->triangles[count][2] = v2[2];
		Pobject->triangles[count++][3] = 1.0;

		Pobject->triangles[count][0] = v3[0];
		Pobject->triangles[count][1] = v3[1];
		Pobject->triangles[count][2] = v3[2];
		Pobject->triangles[count++][3] = 1.0;

		Pobject->triangles[count][0] = v4[0];
		Pobject->triangles[count][1] = v4[1];
		Pobject->triangles[count][2] = v4[2];
		Pobject->triangles[count++][3] = 1.0;



		Pobject->UpdateBoundsWithPoint(v1[0], v1[1], v1[2]);  // should do for all points
		glEnd();
	}
	glEndList();
}
