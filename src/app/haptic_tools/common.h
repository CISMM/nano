/*

Kent Rosenkoetter

*/


#ifndef COMMON_H
#define COMMON_H


#include <iterator>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <utility>
#include <algorithm>


typedef std::vector<double> datum_type;
typedef std::vector<double> point_type;
typedef std::vector<unsigned int> polygon_type;

typedef std::vector<datum_type> datum_list;

typedef std::vector<point_type> point_list;
typedef std::vector<polygon_type> polygon_list;
typedef std::vector<point_type> axis_type;
typedef std::vector<axis_type> axis_list;

typedef std::pair<point_list, polygon_list> shape_type;
typedef std::pair<shape_type, axis_list> transformed_shape_type;


template<class T>
static std::ostream & operator<<(std::ostream & out,
								 const std::vector<T> & p);
template<class S, class T>
static std::ostream & operator<<(std::ostream & out,
								 const std::pair<S, T> & p);


datum_list read_path(const std::string & filename, double dead_time);
transformed_shape_type read_shape(const std::string & filename);


std::vector<double> step_length_list(const datum_list &);
double path_length(const datum_list &);

std::vector<bool> depth_time_list(const datum_list &, double pivot);
double depth_time(const datum_list &, double pivot);

double ratio_true(const std::vector<bool> &);

std::vector<bool> touching_shape_list(const datum_list &,
									  const transformed_shape_type &,
									  double threshold);
double time_touching_shape(const datum_list &,
						   const transformed_shape_type &,
						   double threshold);
std::vector<bool> touching_sphere_list(const datum_list &,
									   const point_type & center,
									   double radius,
									   double threshold);
double time_touching_sphere(const datum_list &,
							const point_type & center, double radius,
							double threshold);
std::vector<bool> touching_torus_list(const datum_list &,
									  const point_type & center,
									  double inner_radius, double outer_radius,
									  double threshold);
double time_touching_torus(const datum_list &,
						   const point_type & center,
						   double inner_radius, double outer_radius,
						   double threshold);

double find_pivot(const point_list &);

double distance_squared(const point_list &,
						const polygon_type & polygon,
						const axis_type &,
						const point_type &);

axis_type find_axis(const point_list & points,
					const polygon_type & polygon);

void transform_points(point_list & points, const datum_list & matrix);
datum_list identity_matrix(void);
datum_list rotate_matrix(double angle, const point_type & normal);
datum_list multiply_matrix(const datum_list & a, const datum_list & b);


template<class T>
static std::string to_string(const T & a)
{
	std::stringstream strm;
	strm << a << std::flush;
	return strm.str();
}

template<class T>
static std::ostream & operator<<(std::ostream & out, const std::vector<T> & p)
{
	out << '(';
	copy(p.begin(), p.end(), std::ostream_iterator<T>(out, " "));
	out << ')';
	return out;
}

template<class S, class T>
static std::ostream & operator<<(std::ostream & out, const std::pair<S, T> & p)
{
	out << '(';
	out << p.first;
	out << ", ";
	out << p.second;
	out << ')';
	return out;
}


#endif
