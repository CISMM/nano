#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <string.h>

#include <vector>

#ifdef sgi
using std::vector;
#endif

#include <nmb_Dataset.h>

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


extern nmb_Dataset * dataset;



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

typedef vector<cylinder> cylinders;


void BuildList(URender *Pobject, GLuint dl, verts vs, int & count);




// This function reads Shape Analysis output file
// it will create an openGL display list for each logical object
int TubeFileGenerator::Load(URender *Pobject, GLuint *&Dlist_array)
{
	char buffer[MAXLENGTH];
	char* token;
	ifstream readfile;

	int i;
	float radius, x, y, z, az, alt;
	double theta;
	double minX, maxX, minY, maxY, z_value;
	int imageX, imageY;
	int cur_step = 0;
	double scale_factor;
	bool skip_first;		// skips first point--sometimes the azimuth is off...

	// pointer to the current plane
	BCPlane *height = dataset->inputGrid->getPlaneByName(dataset->heightPlaneName->string());

//	tubes t;				// gives a bunch of warnings because the name is too big...
	verts t[10];			// go with static number for now...  10 tubes allowed per file
	verts vs;
	vertex v;

	cylinder c;
	cylinders cs;

	q_vec_type p1;
	q_vec_type p2;
	q_type q;

	q_type coord_fix;
	q_vec_type trans;

	int tess = Pobject->GetTess();

	int numtubes = 0;

	bool newtube;

	GLuint dl;


	q_from_euler(coord_fix, -PI / 2, 0, 0);		// coordinate systems are different
												// this fixes it

	// set up min and max values for the current plane
	minX = height->minX();
	maxX = height->maxX();
	minY = height->minY();
	maxY = height->maxY();

	// set up z offset
	z_value = height->scale() * 0.5 * (height->maxNonZeroValue() + height->minNonZeroValue());
//	z_value = height->minNonZeroValue();
//	z_value = height->maxNonZeroValue();

/*
printf("minX = %f\n", minX);
printf("maxX = %f\n", maxX);
printf("minY = %f\n", minY);
printf("maxY = %f\n", maxY);
printf("z_value = %f\n", z_value);
printf("scale = %f\n", height->scale());
printf("minnzv = %f\n", height->minNonZeroValue());
printf("maxnzv = %f\n", height->maxNonZeroValue());
printf("maxav = %f\n", height->maxAttainableValue());
printf("%f\n", height->maxNonZeroValueComputedLast());
printf("%f\n", height->maxValue());
printf("%f\n", height->scaledMaxValue());
*/


	// set up translation to correct place in height plane
	q_vec_set(trans, minX, maxY, z_value);

	readfile.open(filename);
    assert(readfile);

    if(readfile.bad()) {
		cerr << "Unable to open input file" << endl;
        return 0;
    }
	while(!readfile.eof()) {
		readfile.getline(buffer, MAXLENGTH);

		token = strtok(buffer, " \t\n");
		if (token != NULL) {
			if (*token == 'X') { 
				// do nothing
			}
			else if (strcmp(token, "image") == 0) {
				// get image dimensions
				token = strtok(NULL, ":");

				token = strtok(NULL, " \t\n");
				imageX = atoi(token);

				token = strtok(NULL, " \t\n");

				token = strtok(NULL, " \t\n");
				imageY = atoi(token);

				scale_factor = (maxX - minX) / imageX;
			}
			else if (strcmp(token, "radius") == 0) {
				// if not first tube, add to tubes
				if (vs.size() != 0) {
					t[numtubes++] = vs;
					vs.clear();
				}

				// get rid of last cylinder--it is bogus
				if (cs.size() != 0 ) {
					// get rid of last cylinder--it is bogus
					cs.pop_back();
				}
				newtube = true;
	
				// get the radius of the new tube
				token = strtok(NULL, " =\t\n");
				radius = atof(token);

				// sometimes shape analysis gives a radius of zero--quick hack to fix this
				if (radius == 0.0) radius = 5.0;

				// scale to correct size
				radius *= scale_factor;

				// set skip_first
				skip_first = true;

			}
			else {			// should contain numerical data
				if (skip_first) {	// skip the first point--sometimes gives bogus azimuth
					skip_first = false;
				}
				else {
					if (cur_step++ % Pobject->GetAxisStep() == 0) {
						cur_step = 1;

						/* 
						// THIS CODE IS FOR USING X3D, Y3D, Z3d

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

						*/

						// THIS CODE IS FOR USING X and Y (constant Z)


						x = atof(token);
						// scale to correct size
						x *= scale_factor;

						token = strtok(NULL, " \t\n");
						y = atof(token);
						// scale to correct size
						y *= scale_factor;

						z = radius;
//						z = 0.0;


						// skip X3D, Y3D, Z3D
						token = strtok(NULL, " \t\n");
						token = strtok(NULL, " \t\n");
						token = strtok(NULL, " \t\n");


						token = strtok(NULL, " \t\n");
						az = atof(token);
						token = strtok(NULL, " \t\n");
						// buggy alt output from shape analysis...just set to zero for now
	//					alt = atof(token);
						alt = 0.0;

						// set up medial axis point
						q_vec_set(p1, x, y, z);

						// coordinate systems are different
						// this fixes it
						q_xform(p1, coord_fix, p1);
						q_vec_add(p1, p1, trans);	
						
						// set up rotation quat
						q_from_euler(q, az, 0, alt);

						// get vertices
						theta = 0.0;
						for (i = 0; i < tess; i++) {
							v.clear();

							// set point
							q_vec_set(p2,	p1[0] + radius * cos(theta),
											p1[1],
											p1[2] + radius * sin(theta));

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
							theta += 2 * PI / tess;

							Pobject->num_triangles += 2;	// two triangles per vertex
						}
						// do cylinder stuff
						// translates back to origin from correct place in height plane

/*
						c.x1 = p1[0] - minX;
						c.y1 = p1[1] - minY;
						c.z1 = p1[2] - z_value;
*/


/*
						c.x1 = p1[0];
						c.y1 = p1[1];
						c.z1 = p1[2];
*/
						c.x1 = p1[0];
						c.y1 = p1[1];
						c.z1 = p1[2] - z_value;

/*
	printf("%f\n", c.x1);
	printf("%f\n", c.y1);
	printf("%f\n\n", c.z1);
*/
						if (!newtube) {	
							// fill in last guys second point
							cs.back().x2 = p1[0];
							cs.back().y2 = p1[1];
							cs.back().z2 = p1[2];
							// fill in length
							cs.back().length = sqrt((cs.back().x2 - cs.back().x1) * (cs.back().x2 - cs.back().x1) +
													(cs.back().y2 - cs.back().y1) * (cs.back().y2 - cs.back().y1) +
													(cs.back().z2 - cs.back().z1) * (cs.back().z2 - cs.back().z1));
																						
						}
						c.radius = radius;
						c.az = az + PI / 2;
						c.alt = alt;
						cs.push_back(c);
						newtube = false;
					}
				}
			}
		}
	}
	// add last tube
	t[numtubes++] = vs;

	// get rid of last cylinder--it is bogus
	cs.pop_back();


	// create space for triangles in Pobject
	Pobject->num_triangles -= numtubes * tess * 2;
	Pobject->triangles = new float* [Pobject->num_triangles * 3];
	for (i = 0; i < Pobject->num_triangles * 3; i++) {
		Pobject->triangles[i] = new float[4];
	}

	// copy cylinders to Pobjects cylinder list
	Pobject->num_cylinders = cs.size();
	Pobject->cylinders = new cylinder[Pobject->num_cylinders];
	for (i = 0; i < Pobject->num_cylinders; i++) {
		memcpy(&Pobject->cylinders[i], &cs[i], sizeof(c));
	}

/*
	for (i = 0; i < Pobject->num_cylinders; i++) {
		printf("x1 = %f\n", Pobject->cylinders[i].x1);
		printf("y1 = %f\n", Pobject->cylinders[i].y1);
		printf("z1 = %f\n", Pobject->cylinders[i].z1);
		printf("x2 = %f\n", Pobject->cylinders[i].x2);
		printf("y2 = %f\n", Pobject->cylinders[i].y2);
		printf("z2 = %f\n", Pobject->cylinders[i].z2);
		printf("length = %f\n", Pobject->cylinders[i].length);
		printf("radius = %f\n", Pobject->cylinders[i].radius);
		printf("azimuth = %f\n", Pobject->cylinders[i].az);
		printf("altitude = %f\n\n", Pobject->cylinders[i].alt);
	}
*/



	// create geometry from list of vertices

	//	set up display list id's
	Dlist_array = new GLuint[numtubes];
    dl = glGenLists(numtubes);
    if (dl == 0 || Dlist_array == NULL) { 
		cerr << "Bad Display List generation\n"; 
        //kill(getpid(),SIGINT); 
        return 0;
	}

	int count = 0;
	for (i = 0; i < numtubes; i++){  // dtm
		//BuildList actually builds the geometry from
		//the data structures previously built
        BuildList(Pobject, dl + i, t[i], count);
		Dlist_array[i] = dl + i;
	}

	readfile.close();

	return numtubes;  // should be number of display lists
}



void BuildList(URender *Pobject, GLuint dl, verts vs, int & count) {
	float v1[4];
	float v2[4];
	float v3[4];
	float v4[4];

	int i, k;

	q_vec_type n1;
	q_vec_type n2;

	int tess = Pobject->GetTess();

	glNewList(dl, GL_COMPILE);	// init display list

	for (i = 0; i < vs.size() - tess; i++) {
		glBegin(GL_TRIANGLES);
		for (k = 0; k < 3; k++) {
			// special case for last two triangles per segment
			if ((i + 1) >= tess && (i + 1) % tess == 0) {
				v1[k] = vs[i][k];
				v2[k] = vs[i + tess][k];
				v3[k] = vs[i + 1][k];
				v4[k] = vs[i - tess + 1][k];
			}
			else {
				v1[k] = vs[i][k];
				v2[k] = vs[i + tess][k];
				v3[k] = vs[i + tess + 1][k];
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
//glNormal3f((float) sin(count), (float) cos(count), (float) sin(count));
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

//glNormal3f((float) sin(count), (float) cos(count), (float) sin(count));
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
