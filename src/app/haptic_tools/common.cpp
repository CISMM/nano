/*

Kent Rosenkoetter

*/


#include "common.h"


#include <cassert>
#include <cmath>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <limits>
#include <algorithm>
#include <numeric>
#include <functional>


using namespace std;


datum_list read_path(const string & filename, double dead_time)
{
	datum_list path;
	double previous_t(-1);

	ifstream file;
	file.open(filename.c_str());

	if (!file.is_open())
		throw string("not open");

	datum_list::size_type last_change(0);
	//datum_list::iterator last_change(path.begin());
	bool test_for_stall(false);
	bool found_stall(false);

	int lead_char(file.peek());
	while (!file.eof()) {
		if (!file)
			throw string("read error");
		if (lead_char == '#') {
			file.ignore(256, '\n');
		} else {
			double t;
			file >> t;
			if (!file.eof()) {
				if (t <= previous_t)
					break;
				previous_t = t;
				datum_type d(4);
				d[0] = t;
				file >> d[1];
				file >> d[2];
				file >> d[3];
				path.push_back(d);
				//cout << d << endl;

				//cout << last_change << endl;
				//if (test_for_stall)
				//	cout << path[last_change] << endl;
				//cout << endl;

				if (test_for_stall
					&& equal(d.begin() + 1, d.end(),
							 path[last_change].begin() + 1)) {
					//&& equal(d.begin() + 1, d.end(),
					//		 last_change->begin() + 1)) {
					if ((d.front() - path[last_change].front()) >= dead_time) {
					//if ((d.front() - last_change->front()) >= dead_time) {
						//path.resize(last_change);
						//break;
						found_stall = true;
						//cout << "found stall" << endl;
					}
				} else {
					if (found_stall) {
						//cout << "erasing" << endl;
						// clip
						//path.erase(last_change, path.end() - 1);
						path.erase(path.begin() + last_change, path.end() - 1);
						//cout << "erased" << endl;
						//last_change = path.end() - 1;
						//last_change = path.begin() + (path.size() - 1);
						last_change = path.size() - 1;
						//cout << "new start" << endl;
						found_stall = false;
					} else {
						last_change = path.size() - 1;
						//last_change = path.end() - 1;
					}
				}

				test_for_stall = true;
			}
		}
		lead_char = file.peek();
	}
	if (found_stall) {
		//path.erase(last_change, path.end());
		path.resize(last_change);
	}

	file.close();

	return path;
}

template<class T>
static inline vector<T> sum(const vector<T> & a, const vector<T> & b)
{
	assert(b.size() >= a.size());
	vector<T> c(a.size());
	transform(a.begin(), a.end(), b.begin(), c.begin(), plus<T>());
	return c;
}

template<class T>
static inline vector<T> difference(const vector<T> & a, const vector<T> & b)
{
	assert(b.size() >= a.size());
	vector<T> c(a.size());
	transform(a.begin(), a.end(), b.begin(), c.begin(), minus<T>());
	return c;
}

template<class T>
static inline vector<T> scale(const vector<T> & a, T s)
{
	vector<T> ret(a.size());
	transform(a.begin(), a.end(), ret.begin(),
			  bind2nd(multiplies<T>(), s));
	return ret;
}

template<class T>
static inline T dot_product(const vector<T> & a, const vector<T> & b)
{
	assert(b.size() >= a.size());
	return inner_product(a.begin(), a.end(), b.begin(), T(0));
}

template<class T>
static inline T length_squared(const vector<T> & a)
{
	return dot_product(a, a);
}

template<class T>
static inline T length(const vector<T> & v)
{
	return sqrt(length_squared(v));
}

template<class T>
static inline vector<T> normalize(const vector<T> & n)
{
	return scale(n, T(1) / length(n));
}

struct datum_difference : public binary_function<datum_type, datum_type, datum_type>
{
	inline datum_type operator()(const datum_type & a, const datum_type & b) const
	{
		assert(a.size() == 4);
		assert(b.size() == 4);
		point_type diff(3);
		transform(a.begin() + 1, a.end(), b.begin() + 1, diff.begin(),
				  minus<double>());
		return diff;
	}
};

vector<double> step_length_list(const datum_list & path)
{
	//assert(!path.empty());
	vector<double> step_length(path.size());
	datum_list step_diff(path.size());
	adjacent_difference(path.begin(), path.end(), step_diff.begin(),
						datum_difference());
	transform(step_diff.begin(), step_diff.end(), step_length.begin(),
			  ptr_fun(length<double>));
	return step_length;
}

double path_length(const datum_list & path)
{
	if (path.empty())
		return 0;
	vector<double> step_length(step_length_list(path));
	return accumulate(step_length.begin() + 1, step_length.end(), double(0));
}

double ratio_true(const vector<bool> & vec)
{
	if (vec.empty())
		return 0;
	return double(count(vec.begin(),
						vec.end(),
						true))
		/ double(vec.size());
}

struct is_beyond_pivot : public binary_function<datum_type, double, bool>
{
	inline bool operator()(const datum_type & datum, double pivot) const
	{
		assert(datum.size() == 4);
		return (datum[3] < pivot);
	}
};

vector<bool> depth_time_list(const datum_list & path, double pivot)
{
	vector<bool> behind(path.size());
	transform(path.begin(), path.end(), behind.begin(),
			  bind2nd(is_beyond_pivot(), pivot));
	return behind;
}

double depth_time(const datum_list & path, double pivot)
{
	return ratio_true(depth_time_list(path, pivot));
}

template<class T, class Pred>
static inline vector<bool> touching_list(const vector<T> & path,
										 Pred predicate)
{
	vector<bool> touching(path.size());
	transform(path.begin(), path.end(), touching.begin(),
			  predicate);
	return touching;
}

struct polygon_distance : public binary_function<polygon_type, axis_type, double>
{
private:
	const point_list points;
	const point_type point;

public:
	polygon_distance(const point_list & ps,
					 const point_type & pt) : points(ps),
											  point(pt)
	{
		if (points.size() == 0)
			throw string("no points");
	}

	double operator()(const polygon_type & polygon,
					  const axis_type & axis) const
	{
		return distance_squared(points, polygon, axis, point);
	}
};

struct touching_shape : public binary_function<datum_type, double, bool>
{
private:
	const transformed_shape_type shape;

public:
	touching_shape(const transformed_shape_type & s) : shape(s)
	{
		if (shape.first.first.empty())
			throw string("no points");
		if (shape.first.second.empty())
			throw string("no polygons");
		assert(shape.first.second.size() == shape.second.size());
	}

	bool operator()(const datum_type & datum, double threshold) const
	{
		assert(datum.size() == 4);
		point_type p(3);
		copy(datum.begin() + 1, datum.end(), p.begin());

		vector<double> distances(shape.first.second.size());
		transform(shape.first.second.begin(), shape.first.second.end(),
				  shape.second.begin(),
				  distances.begin(),
				  polygon_distance(shape.first.first, p));
		double dist(*min_element(distances.begin(), distances.end()));

		return (dist <= threshold * threshold);
	}
};

vector<bool> touching_shape_list(const datum_list & path,
								 const transformed_shape_type & shape,
								 double threshold)
{
	return touching_list(path,
						 bind2nd(touching_shape(shape),
								 threshold));
}

double time_touching_shape(const datum_list & path,
						   const transformed_shape_type & shape,
						   double threshold)
{
	return ratio_true(touching_shape_list(path, shape, threshold));
}

struct touching_sphere : public binary_function<datum_type, double, bool>
{
private:
	const point_type center;
	const double radius;

public:
	touching_sphere(const point_type & cen,
					double rad) : center(cen),
								  radius(rad) {}

	bool operator()(const datum_type & datum, double threshold) const
	{
		assert(datum.size() == 4);
		point_type p(3);
		transform(datum.begin() + 1, datum.end(), center.begin(), p.begin(),
				  minus<double>());
		return (abs(length(p) - radius) <= threshold);
	}
};

vector<bool> touching_sphere_list(const datum_list & path,
								  const point_type & center, double radius,
								  double threshold)
{
	return touching_list(path,
						 bind2nd(touching_sphere(center, radius),
								 threshold));
}

double time_touching_sphere(const datum_list & path,
							const point_type & center, double radius,
							double threshold)
{
	return ratio_true(touching_sphere_list(path, center, radius, threshold));
}

struct touching_torus : public binary_function<datum_type, double, bool>
{
private:
	const point_type center;
	const double inner_radius, outer_radius;
	const double major_radius, minor_radius;

public:
	touching_torus(const point_type & cen,
				   double _ir,
				   double _or) : center(cen),
								 inner_radius(_ir),
								 outer_radius(_or),
								 major_radius((_or + _ir) / double(2)),
								 minor_radius((_or - _ir) / double(2)) {}

	bool operator()(const datum_type & datum, double threshold) const
	{
		assert(datum.size() == 4);
		point_type p(3);
		//copy(d.begin() + 1, d.end(), p.begin());
		transform(datum.begin() + 1, datum.end(), center.begin(), p.begin(),
				  minus<double>());

		point_type flat_p(p);
		// project point onto x-z plane
		flat_p[1] = 0;
		// find vector to center of torus
		flat_p = scale(normalize(flat_p), major_radius);
		// find distance to center of torus
		point_type tube_diff(difference(p, flat_p));
		double dist(abs(length(tube_diff) - minor_radius));

		return (dist <= threshold);
	}
};

vector<bool> touching_torus_list(const datum_list & path,
								 const point_type & center,
								 double inner_radius, double outer_radius,
								 double threshold)
{
	return touching_list(path,
						 bind2nd(touching_torus(center,
												inner_radius,
												outer_radius),
								 threshold));
}

double time_touching_torus(const datum_list & path,
						   const point_type & center,
						   double inner_radius, double outer_radius,
						   double threshold)
{
	return ratio_true(touching_torus_list(path,
										  center,
										  inner_radius,
										  outer_radius,
										  threshold));
}

transformed_shape_type read_shape(const string & filename)
{
	ifstream file;
	file.open(filename.c_str());
	// read the number of vertices
	int vertices;
	file >> vertices;
	// read the number of values per vert (dimensions)
	int dimensions;
	file >> dimensions;
	point_list points(vertices, point_type(dimensions));
	// read each vertex
	//for (int i = 0; i < vertices; i++) {
	point_list::iterator i(points.begin());
	while (i != points.end()) {
		//datum_type t(dimensions);
		//for (int j = 0; j < dimensions; j++) {
		point_type::iterator j(i->begin());
		while (j != i->end()) {
			//file >> points[i][j];
			file >> *j;
			(*j) *= double(4) / double(5);
			j++;
		}
		// This is VERY IMPORTANT!!!
		//transform(i->begin(), i->end(), i->begin(),
		//		  bind2nd(multiplies<double>(), double(4) / double(5)));
		//points.push_back(t);
		i++;
	}

	// read the number of polygons
	int polygons;
	file >> polygons;
	// read the number of sides per polygon
	int sides;
	file >> sides;
	polygon_list polys(polygons, polygon_type(sides));
	// read each polygon vertex index
	//for (int j = 0; j < polygons; j++) {
	polygon_list::iterator k(polys.begin());
	while (k != polys.end()) {
		//polygon_type p(sides);
		//for (int k = 0; k < sides; k++) {
		polygon_type::iterator l(k->begin());
		while (l != k->end()) {
			//file >> p[k];
			file >> *l;
			// This is VERY IMPORTANT!!!
			//p[k]--;
			--(*l);
			l++;
		}
		//polys.push_back(p);
		k++;
	}
	//cout << string("bumps") << endl;

	// read in the bumps
	if (true) {
		char axis;
		do {
			file >> axis;
			//cout << axis << flush;
		} while (axis != 'x' && axis != 'y' && axis != 'z');
		float x, y, z;
		file >> x >> y >> z;
	}
	//cout << string("rotate") << endl;

	// read in the transformation stuff
	datum_list matrix(identity_matrix());
	char axis;
	float angle;
	file >> axis;
	while (file && !file.eof()) {
		//cout << axis << flush;
		if (axis == 'x' || axis == 'y' || axis == 'z') {
			//cout << "char: " << axis << endl;
			file >> angle;
			//cout << "#: " << angle << endl;
			//if (file.eof())
			//	break;
			cout << "Rotating " << angle
				 << " degrees about the " << axis
				 << " axis" << endl;
			point_type normal(3);
			normal[axis - 'x'] = 1;
			matrix = multiply_matrix(matrix, rotate_matrix(angle, normal));
			//transform_points(points, rotate_matrix(angle, normal));
		}
		file >> axis;
	}
	transform_points(points, matrix);

	file.close();

	//return shape_type(points, polys);
	shape_type shape(points, polys);

	axis_list axis_l;

	polygon_list::const_iterator iter(polys.begin());
	while (iter != polys.end()) {
		axis_type axis(find_axis(points, *iter++));
		axis_l.push_back(axis);
	}

	return transformed_shape_type(shape, axis_l);
}

static inline point_type cross_product(const point_type & a,
									   const point_type & b)
{
	assert(a.size() == 3);
	assert(b.size() == 3);
	//assert(a.size() >= 3 && b.size() >= 3);
	point_type c(3);
	c[0] = a[1] * b[2] - a[2] * b[1];
	c[1] = a[2] * b[0] - a[0] * b[2];
	c[2] = a[0] * b[1] - a[1] * b[0];
	return c;
}

static inline point_type project(const point_type & p,
								 const point_type & u,
								 const point_type & v,
								 const point_type & w)
{
	// transforms the point p into the coordinate system
	// defined by u, v, w
	assert(p.size() == 3);
	assert(u.size() == 3);
	assert(v.size() == 3);
	assert(w.size() == 3);
	//assert(p.size() >= 3 && u.size() >= p.size()
	//	   && v.size() >= p.size() && w.size() >= p.size());
	point_type n(3);
	n[0] = dot_product(p, u);
	n[1] = dot_product(p, v);
	n[2] = dot_product(p, w);
	return n;
}

static inline point_type project_helper(const point_type & p,
										const axis_type & axis)
{
	assert(axis.size() == 3);
	//point_type n(3);
	//transform(axis.begin(), axis.end(), n.begin(),
	//		  bind1st(ptr_fun(dot_product<double>), p));
	//return n;
	return project(p, axis[0], axis[1], axis[2]);
}

axis_type find_axis(const point_list & points, const polygon_type & polygon)
{
	if (polygon.size() < 3)
		throw string("unfinished polygon");
	if (polygon.size() != 3)
		throw string("non-triangle surfaces not implemented");

	if (polygon[0] >= points.size()
		|| polygon[1] >= points.size()
		|| polygon[2] >= points.size())
		throw string("insufficient points");
	// get three points
	vector<point_type> abc(3);
	abc[0] = points[polygon[0]];
	abc[1] = points[polygon[1]];
	abc[2] = points[polygon[2]];
	axis_type uvw(3);
	adjacent_difference(abc.begin(), abc.end(), uvw.begin(),
						ptr_fun(difference<double>));

	// get two planar vectors
	// uvw[1]
	// uvw[2]
	// find the normal
	uvw[0] = cross_product(uvw[1], uvw[2]);
	// make sure that the axis are fully orthogonal
	uvw[1] = cross_product(uvw[2], uvw[0]);
	rotate(uvw.begin(), uvw.begin() + 1, uvw.end());
	// normalize the axis
	transform(uvw.begin(), uvw.end(), uvw.begin(), ptr_fun(normalize<double>));

	return uvw;
}

static inline point_type perpendicular(const point_type & point)
{
	point_type p(2);
	p[0] = -point[1];
	p[1] = point[0];
	return p;
}

static inline bool inside(const point_type & p,
						  const point_type & a,
						  const point_type & b,
						  const point_type & c)
{
	assert(p.size() >= 2);
	assert(a.size() >= 2);
	assert(b.size() >= 2);
	assert(c.size() >= 2);

	point_type u(difference(b, a));
	point_type v(difference(c, b));
	point_type w(difference(a, c));

	u = perpendicular(u);
	v = perpendicular(v);
	w = perpendicular(w);

	point_type np(p);
	np.resize(2);

	bool ba = dot_product(difference(np, a), u) > 0;
	bool bb = dot_product(difference(np, b), v) > 0;
	bool bc = dot_product(difference(np, c), w) > 0;

	return ba == bb && bb == bc;
}

double distance_squared(const point_list & points,
						const polygon_type & polygon,
						const axis_type & axis,
						const point_type & point)
{
	assert(point.size() == 3);
	assert(axis.size() == 3);
	assert(polygon.size() >= 3);
	if (polygon.size() != 3)
		throw string("non-triangles not implemented");
	if (polygon[0] >= points.size()
		|| polygon[1] >= points.size()
		|| polygon[2] >= points.size())
		throw string("polygon references non-existant points");

	// at this point, uvw contains the coordinate axis for the polygon
	// uv are planar, w is the normal
	point_type a(points[polygon[0]]);
	point_type b(points[polygon[1]]);
	point_type c(points[polygon[2]]);
	point_type diff(difference(point, a));
	//cout << "Difference: " << diff << endl << endl;
	point_type new_point(project_helper(diff, axis));

	// TODO: this is unclean. only works with triangles
	// also, doesn't measure distance if not perpendicularly over a triangle
	// but it's good enough for the current use. with the threshold, it works
	if (!inside(new_point,
				project_helper(difference(a, a), axis),
				project_helper(difference(b, a), axis),
				project_helper(difference(c, a), axis)))
		return numeric_limits<double>::max();

	// get the vector to the non-planar point
	point_type normal(axis[2]);
	// distance is the vector to the non-planar point projected onto the normal
	return abs(dot_product(diff, normal));
}

struct z_comp : public unary_function<point_type, double>
{
	inline double operator()(const point_type & p) const
	{
		assert(p.size() == 3);
		return p[2];
	}
};

double find_pivot(const point_list & points)
{
	//assert(!points.empty());
	if (points.empty())
		return 0;
	vector<double> z(points.size());
	transform(points.begin(), points.end(), z.begin(),
			  z_comp());
	double minimum(*min_element(z.begin(), z.end()));
	double maximum(*max_element(z.begin(), z.end()));
	return (minimum + maximum) / double(2);
}

void transform_points(point_list & points, const datum_list & matrix)
{
	assert(matrix.size() == 4);
	assert(matrix[0].size() == 4);
	assert(matrix[1].size() == 4);
	assert(matrix[2].size() == 4);
	assert(matrix[3].size() == 4);
	point_list::iterator i(points.begin());
	while (i != points.end()) {
		point_type point(*i);
		assert(point.size() == 3);
		// extend to 4-elements
		point.push_back(1);
		point_type p(4);
		p[0] = dot_product(matrix[0], point);
		p[1] = dot_product(matrix[1], point);
		p[2] = dot_product(matrix[2], point);
		p[3] = dot_product(matrix[3], point);
		//transform(matrix.begin(), matrix.end(), p.begin(),
		//		  bind2nd(ptr_fun(dot_product<double>), point));
		p[0] /= p[3];
		p[1] /= p[3];
		p[2] /= p[3];
		//transform(p.begin(), p.end(), p.begin(),
		//		  bind2nd(divides<double>(), p.back()));
		p.resize(3);
		*i++ = p;
	}
}

datum_list identity_matrix(void)
{
	datum_list matrix(4, datum_type(4));
	matrix[0][0] = 1;
	matrix[1][1] = 1;
	matrix[2][2] = 1;
	matrix[3][3] = 1;
	return matrix;
}

template<class T>
static inline T to_radians(T degrees)
{
	return degrees * 3.14159265358979 / T(180);
}

datum_list rotate_matrix(double angle, const point_type & normal)
{
	assert(normal.size() == 3);
	point_type n(normalize(normal));
	datum_list matrix(4, datum_type(4));
	angle = to_radians(-angle);
	double c(cos(angle));
	double _c(1 - c);
	double s(sin(angle));
	double x(n[0]);
	double y(n[1]);
	double z(n[2]);
	matrix[0][0] = x*x*_c+c;
	matrix[0][1] = y*x*_c+z*s;
	matrix[0][2] = z*x*_c-y*s;
	matrix[0][3] = 0;
	matrix[1][0] = x*y*_c-z*s;
	matrix[1][1] = y*y*_c+c;
	matrix[1][2] = z*y*_c+x*s;
	matrix[1][3] = 0;
	matrix[2][0] = x*z*_c+y*s;
	matrix[2][1] = y*z*_c-x*s;
	matrix[2][2] = z*z*_c+c;
	matrix[2][3] = 0;
	matrix[3][0] = 0;
	matrix[3][1] = 0;
	matrix[3][2] = 0;
	matrix[3][3] = 1;
	return matrix;
}

datum_list multiply_matrix(const datum_list & a, const datum_list & b)
{
	datum_list c(4, datum_type(4));
	for (int x = 0; x < 4; x++)
		for (int y = 0; y < 4; y++)
			for (int z = 0; z < 4; z++)
				c[x][y] += a[x][z] * b[z][y];
	return c;
}

// This function does absolutely nothing.
// It is here so that the broken Microsoft compiler (6.0) will
// link in normalize<double> for use in other places.
static double useless(point_type trash)
{
	normalize(trash);
	return length(trash);
}
