/*===3rdtech===
  Copyright (c) 2000 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#include "noise.h"
#include <stdio.h>

static int        P[256];
static double     G[256][3];

#ifdef _WIN32
double drand48() { printf("XXX drand48 not on _WIN32\n"); return 0.0; }
#else
extern "C" double drand48(void);
#endif

double bias( double b, double t ) {
  if ( t < 0 )
    return 0;
  return pow( t, ( log( b ) / log( 0.5 ) ) );
}

double gain( double g, double t ) {
  if ( t < .5 )
    return ( bias( 1 - g, ( 2.0 * t ) ) / 2.0 ) ;
  else
    return ( 1 - ( bias( 1 - g, ( 2.0 - 2.0 * t ) ) / 2.0 ) );
}

double noise( double x, double y, double z ) {
  double result = 0.0;
  for ( int i = ((int)( floor( x ) )); i <= ( floor( x ) + 1 ); i++ )
    for ( int j = ((int)( floor( y ) ));j <= (floor( y ) + 1 ); j++ )
      for ( int k = ((int)( floor( z )));k <= (floor(z) + 1 ); k++ )
	result += sigma( i, j, k, x - i, y - j, z - k );
  return fabs( result );
}

double turbulence( double x, double y, double z ) {
  double result = 0.0;
  for ( int i = 0; i < 25; i++ ) {
    double r = pow( 2.0, i );
    x = x * r;
    y = y * r;
    z = z * r;
    result += fabs( ( 1.0 / r ) ) * noise( x, y, z );
  }
  return result;
}


double sigma( int i, int j, int k, double u, double v, double w ) {
  double f[3];
  gradient( i, j, k, f );
  double dot = f[0] * u +  f[1] * v +  f[2] * w;

  return (  omega( u ) * omega( v ) * omega( w ) * dot );
}

double omega( double t ) {
  double abst = fabs( t );
  if ( abst < 1.0 )
    return 2.0 * pow( abst, 3.0 ) - 3.0 * pow( abst, 2.0 ) + 1.0;
  return 0.0;
}

void gradient( int i, int j, int k, double g[3] ) {
  for ( int a = 0; a < 3; a++ )
    g[a] = G[ p( i + p( j + p( k ) ) ) ][a];
}

int p( int i ) {
  return P[ abs(i)%256 ];
}

void initialize_p() {
  for ( int i = 0; i < 256; i++ )
    P[i] = i;

  permute();
  permute();
  permute();
}

void permute() {
  double pow_max = pow( 2.0, 15.0 )-1;

  int j, k;
  double a, b;
  for ( int i = 0; i < 256; i++ ) {
    a = drand48() / ( pow_max );
    b = drand48() / ( pow_max );
    j = ((int)( a * 255 ));
    k = ((int)( b * 255 ));
    swap( j, k );
  }
}

void swap( int i, int j ) {
  int tmp = P[i];
  P[i] = P[j];
  P[j] = tmp;
}

void initialize_g() {
  for ( int i = 0; i < 256; i++ ) {
    G[i][0] = ( ( rand() / float(RAND_MAX) ) * pow( -1.0, (double)rand() ) );
    G[i][1] = ( ( rand() / float(RAND_MAX) ) * pow( -1.0, (double)rand() ) );
    G[i][2] = ( ( rand() / float(RAND_MAX) ) * pow( -1.0, (double)rand() ) );
    double d = distance( G[i] );
    while ( fabs( d ) > 1.0 ) {
      G[i][0] = ( ( rand() / float(RAND_MAX) ) * pow( -1.0, (double)rand() ) );
      G[i][1] = ( ( rand() / float(RAND_MAX) ) * pow( -1.0, (double)rand() ) );
      G[i][2] = ( ( rand() / float(RAND_MAX) ) * pow( -1.0, (double)rand() ) );
      d = distance( G[i] );
    }
    //    normalize( G[i] );
    d = sqrt( d );
    if ( d != 0 ) {
      G[i][0] /= d;
      G[i][1] /= d;
      G[i][2] /= d;
    }
  }
}

double distance( double f[] ) {
  return f[0] * f[0] + f[1] * f[1] + f[2] * f[2];
}
