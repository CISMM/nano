#ifndef _noise_h_
#define _noise_h_

#include <stdlib.h>
#include <math.h>

double bias( double b, double t );
double gain( double g, double t );
double noise( double x, double y, double z );
double turbulence( double x, double y, double z );
double sigma( int i, int j, int k, double u, double v, double w );
double omega( double t );
void gradient( int i, int j, int k, double g[] );
int p( int i );
void initialize_p();
void permute();
void swap( int i, int j );
void initialize_g();
double distance( double f[] );

#endif
