#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <vector>

#ifdef sgi
using std::vector;
#endif

#include <nmb_Dataset.h>

#include "URender.h"
#include "WaveFrontFileGenerator.h"


// Heavily updated by Borland 1/18/02
// Should be much more robust now, and handle
// objects with polygons of an abritrary degree--
// not just triangles.

#ifdef sgi
// isspace and isalnum are found in ctype.h on the sgi's
#include <ctype.h>
#endif

#ifdef __CYGWIN__
// XXX juliano 9/19/99
//       this was implicitly declared.  Needed to add decl.
//       getpid comes from unistd.h
#include <sys/types.h>  // for pid_t
extern "C" {
pid_t getpid();
}
#endif

#define MAXLENGTH 512		// buffer size


extern nmb_Dataset * dataset;



// auxilliary data structures
typedef vector<float> vertex;

typedef struct {
	vector<int> index;
	vector<int> vert_tex;
	vector<int> vert_norm;

	vertex norm;		// face normal
} face;

typedef struct {
	char name[32];
	vector<int> faces;	//face indeces
} group;

typedef struct {		
	vector<int> faces;		// tells which faces each vertex is incident upon
							// used for calculating normals
} back_point;




void BuildList(URender *Pobject, GLuint dl, 
			   vector<vertex> verts, 
			   vector<vertex> texts, 
			   vector<vertex> norms, 
			   vector<face> faces, 
			   group* g);


WaveFrontFileGenerator::WaveFrontFileGenerator(const char *fname)
    : FileGenerator(fname, "obj")
{
}

int WaveFrontFileGenerator::ReLoad(URender *Pobject, GLuint *&Dlist_array)
{
    return Load(Pobject, Dlist_array);
}





void strip_line(char* string) {
  char *c;
  if (!isalnum(*string)) return;
  c = string + strlen(string);
  while (!isalnum(*c) && c > string) c--;
  *++c = '\n';
}

void calc_face_norm(vector<vertex> verts, face& f) {
	float temp[4], v1[4], v2[4];
	int i;

	for (i = 0; i < 3; i++) {
		temp[i] = verts[f.index[2] - 1][i] - verts[f.index[1] - 1][i];
	}

	for (i = 0; i < 3; i++) {
		v1[i] = temp[i];
	}
  
	for (i = 0; i < 3; i++) {
		temp[i] = verts[f.index[0] - 1][i] - verts[f.index[2] - 1][i];
	}


  
	for (i = 0; i < 3; i++) {
		v2[i] = temp[i];
	}

	// take the cross-product of v1 and v2
	temp[0] = v1[1] * v2[2] - v1[2] * v2[1];
	temp[1] = v1[2] * v2[0] - v1[0] * v2[2];
	temp[2] = v1[0] * v2[1] - v1[1] * v2[0];
	
	// normalize
	float length;
	length = sqrt(temp[0] * temp[0] + temp[1] * temp[1] + temp[2] * temp[2]);
	// check for divide by zero
	if (length == 0.000000) length = 0.000001;
	for (i = 0; i < 3; i++) {
		f.norm.push_back(temp[i] / length);
	}
	f.norm.push_back(1.0);
}

void calc_vertex_norms(vector<vertex> verts, 
					   vector<back_point> back_points,
					   vector<vertex>& norms,
					   vector<face>& faces) {
	int i, j, k;
	float v[4];
	vector<float> n;

	for (i = 0; i < verts.size(); i++) {
		n.clear();
		v[0] = 0.0; v[1] = 0.0; v[2] = 0.0; v[3] = 1.0;
		for (j = 0; j < back_points[i].faces.size(); j++) {
			for (k = 0; k < 3; k++) {
				v[k] += faces[back_points[i].faces[j] - 1].norm[k];
			}
		}
		// normalize
		float length;
		length = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);

		// check for divide by zero
		if (length == 0.000000) length = 0.000001;
		for (k = 0; k < 3; k++) {
			v[k] /= length;
			n.push_back(v[k]);
		}
		n.push_back(1.0);
		norms.push_back(n);
	}

	for (i = 0; i < faces.size(); i++) {
		for (j = 0; j < faces[i].index.size(); j++) {
			faces[i].vert_norm.push_back(faces[i].index[j]);
		}
    }
}




//This function reads a wavefront object file
// it will create an openGL display list for each logical object
int WaveFrontFileGenerator::Load(URender *Pobject, GLuint *&Dlist_array)
{
    char buffer[MAXLENGTH];
	ifstream readfile;

	vertex v;
	back_point b;
	face f;
	group g;

	vector<vertex> verts;				//vertex position   
	vector<back_point> back_points;		//back pointers (one entry per vertex)
	vector<vertex> texts;				//texture coords
	vector<vertex> norms;				//vertex normals
	vector<face> faces;					//faces
	vector<group> groups;				//groups

	double minX, minY, z_value;

    char *buf, *val;
	int i, j, flag;
	int current_group = 0;
	GLuint dl;

	// pointer to the current plane
	BCPlane *height = dataset->inputGrid->getPlaneByName(dataset->heightPlaneName->string());

	// set up min and max values for the current plane
	minX = height->minX();
	minY = height->minY();
	z_value = height->minNonZeroValue();

	g.name[0] = '\0';

	readfile.open(filename);
    assert(readfile);

    if(readfile.bad()) {
		cerr << "Unable to open input file" << endl;
        return 0;
    }
	while(!readfile.eof()) {
		readfile.getline(buffer, MAXLENGTH);
		strip_line(buffer);
		v.clear();

// comment

		if (*buffer == '#') {
			// do nothing
		}

// vertex command

		else if (buffer[0] == 'v' && buffer[1] == ' ') {
			buf = val = buffer;
			flag = 1;



			// find floating point values
			for (i = 0; i < 4; i++) {
				while (isspace(*++buf)) {}
				val = buf;
				while (!isspace(*++val)) {}
				if (*val == '\n') flag = 0;
				*val = '\0';
				v.push_back(atof(buf));				
				buf = val;

				if (!flag) break;
			}

			// if less than 4 floating point values given fill with zero
			if (i < 3) {
				for (j = i + 1; j < 3; j++) {
					v.push_back(0.0);
				}
				v.push_back(1.0);
			}
			// add to the vertex list
			verts.push_back(v);
			// add to the backpointer list
			back_points.push_back(b);
		}

// texture vertex command

		else if (buffer[0] == 'v' && buffer[1] == 't') {
			buf = val = buffer + 1;

			flag = 1;

			// find floating point values
			for (i = 0; i < 3; i++) {
				while (isspace(*++buf)) {}
				val = buf;
				while (!isspace(*++val)) {}
				if (*val == '\n') flag = 0;
				*val = '\0';
				v.push_back(atof(buf));
				buf = val;

				if (!flag) break;
			}

			// fill missing parameters with zero if necessary 
			if (i == 0 || i == 1) v.push_back(0.0);

			v.push_back(1.0);
			
			// add to the texture vertex list
			texts.push_back(v);
		}

// vertex normal command

		else if (buffer[0] == 'v' && buffer[1] =='n') {
			buf = val = buffer + 1;
			flag = 1;

			// find floating point values
			for (i = 0; i < 3; i++) {
				while (isspace(*++buf)) {}
				val = buf;
				while (!isspace(*++val)) {}
				if (*val == '\n') flag = 0;
				*val = '\0';
				v.push_back(atof(buf));
				buf = val;

				if (!flag) break;
			}

			v.push_back(1.0);
			norms.push_back(v);
		}

// new group command 

		else if (buffer[0] == 'g' && buffer[1] == ' ') {
			if (verts.size() != 0) {
				buf = buffer;
				while (isspace(*++buf)) {}
				val = buf;
				while (*++val != '\n') {}
				*val = '\0';

				// search for this name
				flag = 0;
				for (i = 0; i < groups.size(); i++) {
					if (strcmp(buf, groups[i].name) == 0) {
						// group already exists.  add to it
						current_group = i;
						flag = 1;
					}
				}

				if (!flag) {
					// new group
					strcpy(g.name, buf);
					groups.push_back(g);

 					current_group = groups.size() - 1;
				}
			}
		}

// face command

		else if (buffer[0] == 'f') {
			// add new face
			faces.push_back(f);

			buf = val = buffer;
			flag = 1;

			// find the integer values
			while (1) {
				while (isspace(*++buf)) {}
				val = buf;
				while (isdigit(*++val)) {}
				if (*val == '\n') flag = 0;
				else if (*val == '/') flag = 2;
				*val = '\0';
				
				// convert string to integer
				i = atoi(buf);

				// add vertex to face's index list 
				faces.back().index.push_back(i);

	            // add vertex-face back pointer
				back_points[i - 1].faces.push_back(faces.size());

				if (flag == 2) {
					// check for vt
					buf = val + 1;
					if (isdigit(*buf)) {
						// has vt 
						while (isdigit(*++val)) {}
						if (*val == '\n') flag = 0;
						else if (*val == '/') flag = 3;
						*val = '\0';

						// convert string to integer
						i = atoi(buf);

						// add vert_tex to face's vert_tex list 
						faces.back().vert_tex.push_back(i);

						// check for vn
						if (flag == 3) {
							// has vn 
							buf = val + 1;
							while (isdigit(*++val)) {}
							if (*val == '\n') flag = 0;
							*val = '\0';
							
							// convert string to integer
							i = atoi(buf);

							// add vert_norm to face's vert_norm list
							faces.back().vert_norm.push_back(i);
						}
					}
					else {
						// no vt, only vn 
						val = ++buf;
						while (isdigit(*++val)) {}
						if (*val == '\n') flag = 0;
						*val = '\0';
					
						// convert string to integer
						i = atoi(buf);

						// add vert_norm to face's vert_norm list
						faces.back().vert_norm.push_back(i);
					}
				}

				buf = val;

				if (!flag) break;
			}

			// calculate face normal
			calc_face_norm(verts, faces.back());
			
			// if no group has been specified, create group 
			if (groups.size() == 0) {
				strcpy(g.name, "group_1");
				current_group = 0;
				groups.push_back(g);
			}

			// add face number to group face list
			groups[current_group].faces.push_back(faces.size() - 1);
 		}

// group command

		else if (buffer[0] = 'g' && buffer[1] == 'r') {
			buf = buffer + 4;
			while (isspace(*++buf)) {}
			val = buf;
			while (*++val != '\n') {}
			*val = '\0';
			for (i = 0; i < groups.size(); i++) {
				if (strcmp(groups[i].name, buf) == 0) current_group = i;
			}
		}
	} 

	// calculate vertex normals if not given explicitly in file
	if (norms.size() == 0) {
		calc_vertex_norms(verts, back_points, norms, faces);
	}

	// invert normals if wanted
	if (!Pobject->GetCCW()) {
		for (i = 0; i < norms.size(); i++) {
			for (j = 0; j < 3; j++) {
				norms[i][j] *= -1.0;
			}
		}
	}


	//set up display list id's
	Dlist_array = new GLuint[groups.size()];
    dl = glGenLists(groups.size());
    if(dl == 0 || Dlist_array == NULL) { 
		cerr << "Bad Display List generation\n"; 
        //kill(getpid(),SIGINT); 
        return 0;
	}

	for(i = 0; i < groups.size(); i++){
		//BuildList actually builds the geometry from
		//the data structures previously built
        BuildList(Pobject, dl + i, verts, texts, norms, faces, &groups[i]);
		Dlist_array[i] = dl + i;
	}

	readfile.close();

	
	// add minimum extents of the height plane
	Pobject->GetLocalXform().SetXOffset(minX);
	Pobject->GetLocalXform().SetYOffset(minY);
	Pobject->GetLocalXform().SetZOffset(z_value);
	Pobject->GetLocalXform().SetTranslate(0, 0, 0);


	return groups.size();
}


  

void BuildList(URender *Pobject, GLuint dl, 
				vector<vertex> verts, 
				vector<vertex> texts, 
				vector<vertex> norms,
				vector<face> faces, 
				group* g) {
        
	face* f;
	float v[4];

	glNewList(dl, GL_COMPILE);	//init display list
	for (int i = 0; i < g->faces.size(); i++) {
		f = &faces[g->faces[i]];
		glBegin(GL_POLYGON);
		for (int j = 0; j < f->index.size(); j++) {
			if (j < f->vert_norm.size()) {
				// give GL the vertex normals
				// GL needs an array, not a vector
				for (int k = 0; k < 4; k++) {
					v[k] = norms[f->vert_norm[j] - 1][k];
				}
				glNormal3fv(v);
			}
			if (j < f->vert_tex.size()) {
				// give GL the texure coordinates
				// GL needs an array, not a vector
				for (int k = 0; k < 4; k++) {
					v[k] = texts[f->vert_tex[j] - 1][k];
				}
				glTexCoord2fv(v);
			}
			// give GL the vertex
			// GL needs an array, not a vector
			for (int k = 0; k < 4; k++) {
				v[k] = verts[f->index[j] - 1][k]; 
			}
            glVertex4fv(v);
			Pobject->UpdateBoundsWithPoint(v[0], v[1], v[2]);
		}						
		glEnd();
	}
	glEndList();
}






