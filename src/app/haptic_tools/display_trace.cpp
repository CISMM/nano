/*

Kent Rosenkoetter

*/


#include <iterator>
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <limits>
#include <functional>

#include <cassert>

#include <GL/glut.h>
#include <GL/gl.h>


#include "common.h"


using namespace std;


typedef vector<transformed_shape_type>::size_type index_t;


int glut_window;
GLint shape_display_list;
GLint trace_display_list;
vector<transformed_shape_type> shapes(21);
vector<double> pivots(23);
vector<string> filenames;
string current_filename;
vector<string>::const_iterator filename_index;
vector<index_t> which_shape;
index_t current_shape_index;
vector<index_t>::const_iterator which_shape_index;
double roll(0), pitch(0), yaw(0), size(1);
const double angle(5), scale(1.08);
bool show_shape(true), show_trace(true);

double threshold(2);
double dead_time(3);

void build_shape_display_list(index_t shape_index);
void build_trace_display_list(const datum_list & trace,
							  const vector<bool> & touching);
vector<bool> touching_list(const datum_list & trace,
						   index_t shape_index);
bool advanceTrace(void);

void display(void);
void keyboard(unsigned char, int, int);
void special(int, int, int);

int main(int argc, char ** argv)
{
	glutInitWindowSize(600, 600);
	glutInitDisplayMode(GLUT_RGBA | GLUT_SINGLE | GLUT_DEPTH);
	glutInit(&argc, argv);
	glut_window = glutCreateWindow("Trace Display");
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(special);
	shape_display_list = glGenLists(1);
	trace_display_list = glGenLists(1);

	string subject("Subject");
	string session("_Session");
	string s1("session_one_shapes");
	string s2("session_two_shapes");
	string shape("shape");

	string alexandra("alexandra");
	string Alexandra("Alexandra");
	string atsuko("atsuko");
	string Atsuko("Atsuko");
	string tom("tom");
	string Tom("Tom");
	string trace("_explore_viewer_trace_");
	string txt(".txt");
	unsigned int first_subject(1);
	unsigned int last_subject(111);
	index_t s;

	vector<string> people_list;
	people_list.push_back(alexandra);
	people_list.push_back(Alexandra);
	people_list.push_back(atsuko);
	people_list.push_back(Atsuko);
	people_list.push_back(tom);
	people_list.push_back(Tom);

	for (s = 1; s <= 10; s++) {
		string filename(s1 + '/' + shape + to_string(s) + txt);
		shapes[s-1] = read_shape(filename);
		pivots[s-1] = find_pivot(shapes[s-1].first.first);
		cout << filename << " = " << pivots[s-1] << endl;
	}
	for (s = 1; s <= 11; s++) {
		string filename(s2 + '/' + shape + to_string(s) + txt);
		shapes[s+9] = read_shape(filename);
		pivots[s+9] = find_pivot(shapes[s+9].first.first);
		cout << filename << " = " << pivots[s+9] << endl;
	}
	pivots[21] = pivots[22] = 0;

	for (unsigned int sub = first_subject; sub <= last_subject; sub ++) {
		if (sub % 3 != 0)
			continue;
		string dir(subject + to_string(sub) + session);

		string dir1(dir + to_string(1) + '/');
		// training shape is a cube 75x75x75
		for (s = 1; s <= 10; s++) {
			vector<string>::const_iterator pp(people_list.begin());
			while (pp != people_list.end()) {
				string person(*pp++);
				string filename(dir1 + person + trace + to_string(s) + txt);
				filenames.push_back(filename);
				which_shape.push_back(s - 1);
			}
		}
		// shape 11 is a cube 60x60x60
		// rotated 45 degrees about (0, 1, 0)
		// rotated 45 degrees about (0, 0, 1)

		// shape 12 is a sphere of radius 45

		// shape 13 is a torus
		// major radius 40
		// minor radius 20

		string dir2(dir + to_string(2) + '/');
		for (s = 1; s <= 13; s++) {
			vector<string>::const_iterator pp(people_list.begin());
			while (pp != people_list.end()) {
				string person(*pp++);
				string filename(dir2 + person + trace + to_string(s) + txt);
				filenames.push_back(filename);
				which_shape.push_back(s + 9);
			}
		}
	}

	filename_index = filenames.begin();
	which_shape_index = which_shape.begin();
	advanceTrace();

	// The haptic experiment display was done with
	// gluPerspective(15, 1, 1.5, 1000);
	// and
	// gluLookAt(-175, 225, 700, 0, 0, 0, 0, 1, 0);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-200, 200, -200, 200, 150, 650);
	//glFrustum(-200, 200, -200, 200, 150, 650);
	glMatrixMode(GL_MODELVIEW);

	glEnable(GL_DEPTH_TEST);
	glPolygonOffset(-0.5, -1.0);
	glEnable(GL_POLYGON_OFFSET_LINE);

	glutMainLoop();

	return 0;
}

vector<bool> touching_list(const datum_list & trace,
						   index_t shape_index)
{
	if (shape_index < shapes.size())
		return touching_shape_list(trace, shapes[shape_index], threshold);
	else if (shape_index == 21)
		return touching_sphere_list(trace, point_type(3), 45, threshold);
	else if (shape_index == 22)
		return touching_torus_list(trace, point_type(3), 20, 40, threshold);
	throw string("invalid shape index");
}

bool advanceTrace(void)
{
	while (filename_index != filenames.end()) {
		current_filename = *filename_index++;
		current_shape_index = *which_shape_index++;
		try {
			datum_list path(read_path(current_filename, dead_time));
			cout << current_filename << endl;
			vector<bool> touching(touching_list(path, current_shape_index));
			build_trace_display_list(path, touching);
			build_shape_display_list(current_shape_index);
			cout << "Path length: " << path_length(path) << endl;
			cout << "Time past depth " << pivots[current_shape_index] << ": "
				 << 100 * depth_time(path, pivots[current_shape_index])
				 << endl;
			cout << "Time touching: "
				 << 100 * ratio_true(touching)
				 << endl;
			return true;
		} catch (string st) {
			if (st.compare(string("not open")) != 0)
				cerr << "failed to read file " << current_filename
					 << " : " << st << endl;
		}
	}

	return false;
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();
	glTranslated(0, 0, -400);
	glRotated(yaw, 0, 1, 0);
	glRotated(pitch, 0, 0, 1);
	glRotated(roll, 1, 0, 0);
	glScaled(size, size, size);

	if (show_shape)
		glCallList(shape_display_list);
	if (show_trace)
		glCallList(trace_display_list);

	glFlush();
	//cout << glGetError() << endl;

	glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 'q':
		exit(0); break;
	case 's':
	case 'S':
		show_shape = !show_shape;
		break;
	case 't':
	case 'T':
		show_trace = !show_trace;
		break;
	case ',':
	case '<':
		threshold = max(threshold - 0.5, 0.5);
		{
			cout << "Threshold: " << threshold << endl;
			datum_list path(read_path(current_filename, dead_time));
			vector<bool> touching(touching_list(path, current_shape_index));
			build_trace_display_list(path, touching);
			cout << "Time touching: " << 100 * ratio_true(touching) << endl;
		}
		break;
	case '.':
	case '>':
		threshold = min(threshold + 0.5, 20.0);
		{
			cout << "Threshold: " << threshold << endl;
			datum_list path(read_path(current_filename, dead_time));
			vector<bool> touching(touching_list(path, current_shape_index));
			build_trace_display_list(path, touching);
			cout << "Time touching: " << 100 * ratio_true(touching) << endl;
		}
		break;
	default:
		if (!advanceTrace())
			exit(0);
		break;
	}
	glutPostRedisplay();
}

void special(int key, int x, int y)
{
	switch (key) {
	case GLUT_KEY_UP:
		pitch -= angle;
		break;
	case GLUT_KEY_DOWN:
		pitch += angle;
		break;
	case GLUT_KEY_LEFT:
		yaw -= angle;
		break;
	case GLUT_KEY_RIGHT:
		yaw += angle;
		break;
	case GLUT_KEY_PAGE_UP:
		roll -= angle;
		break;
	case GLUT_KEY_PAGE_DOWN:
		roll += angle;
		break;
	case GLUT_KEY_HOME:
		size *= scale;
		break;
	case GLUT_KEY_END:
		size /= scale;
		break;
	default:
		return;
	}
	glutPostRedisplay();
}

void build_trace_display_list(const datum_list & trace,
							  const vector<bool> & touching)
{
	assert(trace.size() == touching.size());

	glNewList(trace_display_list, GL_COMPILE);
	glBegin(GL_LINE_STRIP);
	datum_list::const_iterator index(trace.begin());
	vector<bool>::const_iterator tindex(touching.begin());
	while (index != trace.end()) {
		datum_type datum(*index++);
		bool touch(*tindex++);
		assert(datum.size() == 4);
		if (touch) {
			glColor3d(1.0, 0.0, 0.0);
		} else {
			glColor3d(1.0, 1.0, 1.0);
		}
		glVertex3d(datum[1], datum[2], datum[3]);
	}
	glEnd();
	glEndList();
}

void build_shape_display_list(const shape_type & shape)
{
	GLdouble * vertices = new GLdouble[3 * shape.first.size()];
	for (shape_type::first_type::size_type i(0); i < shape.first.size(); i++) {
		vertices[3 * i] = shape.first[i][0];
		vertices[3 * i + 1] = shape.first[i][1];
		vertices[3 * i + 2] = shape.first[i][2];
		//cout << vertices[3 * i] << ' '
		//	 << vertices[3 * i + 1] << ' '
		//	 << vertices[3 * i + 2] << endl;
	}
	//cout << endl;
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_DOUBLE, 0, vertices);
	glNewList(shape_display_list, GL_COMPILE);
	glColor3d(0.5, 0.0, 1.0);
	glBegin(GL_TRIANGLES);
	polygon_list::const_iterator poly_index(shape.second.begin());
	while (poly_index != shape.second.end()) {
		polygon_type poly(*poly_index++);
		glArrayElement(poly[0]);
		glArrayElement(poly[1]);
		glArrayElement(poly[2]);
		//cout << poly[0] << ' '
		//	 << poly[1] << ' '
		//	 << poly[2] << endl;
	}
	glEnd();
	//cout << endl;
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glColor3d(1.0, 0.5, 0.0);
	glBegin(GL_TRIANGLES);
	poly_index = shape.second.begin();
	while (poly_index != shape.second.end()) {
		polygon_type poly(*poly_index++);
		glArrayElement(poly[0]);
		glArrayElement(poly[1]);
		glArrayElement(poly[2]);
	}
	glEnd();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEndList();
	glDisableClientState(GL_VERTEX_ARRAY);
	delete [] vertices;
}

void build_sphere_display_list(const point_type & center,
							   double radius)
{
	assert(center.size() == 3);
	glNewList(shape_display_list, GL_COMPILE);
	glColor3d(0.5, 0.0, 1.0);
	glutSolidSphere(radius, 20, 15);
	glEndList();
}

void build_torus_display_list(const point_type & center,
							  double inner_radius, double outer_radius)
{
	assert(center.size() == 3);
	glNewList(shape_display_list, GL_COMPILE);
	glPushMatrix();
	glRotated(90, 1, 0, 0);
	glColor3d(0.5, 0.0, 1.0);
	glutSolidTorus(inner_radius, outer_radius, 25, 15);
	glPopMatrix();
	glEndList();
}

void build_shape_display_list(index_t shape_index)
{
	if (shape_index < shapes.size())
		build_shape_display_list(shapes[shape_index].first);
	else if (shape_index == 21)
		build_sphere_display_list(point_type(3), 45);
	else if (shape_index == 22)
		build_torus_display_list(point_type(3), 20, 40);
}
