#include <stdlib.h>
#include <stdio.h>
#include <iostream.h>
#include <fstream.h>
#include <string.h>

#include "gaFunctions.h"
#include "noise.h"

char **GA_VARIABLE_LIST;
int GA_NUMBER_OF_VARIABLES = 0;
int GA_NUMBER_OF_INPUTS = 0;
float ***GA_INPUTS;

float evaluate_variable( const char *string, int i_index, int j_index ) {
  int index;
  for ( int i = 0; i < GA_NUMBER_OF_VARIABLES; i++ ) {
    if ( strstr( string, GA_VARIABLE_LIST[i] ) ) {
      if ( strstr( string, "X" ) ) {
	return i_index;
      }
      else if ( strstr( string, "Y" ) ) {
	return j_index;
      }
      else {
	index = i - (GA_NUMBER_OF_VARIABLES - GA_NUMBER_OF_INPUTS);
	return GA_INPUTS[index][i_index][j_index];
      }
    }
  }
  return 0.0;
}

float evaluate_function( const char *string, float args[] ) {
  float value = 0;
  // return the result of this function on the arguments:
  if ( !strcmp( string, "abs" ) )
    value = fabs( args[0] );
  else if ( !strcmp( string, "exp" ) )
    value = exp( args[0] );
  else if ( !strcmp( string, "log" ) ) {
    if ( args[0] != 0 )
      value = log( fabs( args[0] ) );
    else
      value = 0;
  }
  else if ( !strcmp( string, "neg" ) )
    value = - args[0];
  else if ( !strcmp( string, "sqrt" ) )
    value = sqrt( fabs(args[0] ) );
  else if ( !strcmp( string, "sin" ) )
    value = sin( args[0] );
  else if ( !strcmp( string, "cos" ) )
    value = cos( args[0] );
  else if ( !strcmp( string, "tan" ) )
    value = tan( args[0] );
  else if ( !strcmp( string, "atan" ) )
    value = atan( args[0] );
  else if ( !strcmp( string, "acos" ) )
    value = acos( args[0] );
  else if ( !strcmp( string, "asin" ) )
    value = asin( args[0] );
  else if ( !strcmp( string, "ceil" ) )
    value = ceil( args[0] );
  else if ( !strcmp( string, "floor" ) )
    value = floor( args[0] );
  else if ( !strcmp( string, "round" ) )
    value = floor( args[0] + .5 );

  else if ( !strcmp( string, "+" ) )
    value = args[0] + args[1];
  else if ( !strcmp( string, "-" ) )
    value = args[0] - args[1];
  else if ( !strcmp( string, "*" ) )
    value = args[0] * args[1];
  else if ( !strcmp( string, "/" ) ) {
    if ( args[1] != 0 )
      value = args[0] / args[1];
    else
      value = args[0];
  }
  else if ( !strcmp( string, "mod" ) ) {
    if ( int(args[1]) != 0 )
      value = int(args[0]) % int(args[1]);
    else
      value = int(args[0]);
  }
  else if ( !strcmp( string, "min" ) )
    value = (args[0] < args[1]) ? args[0] : args[1];
  else if ( !strcmp( string, "max" ) )
    value = (args[0] > args[1]) ? args[0] : args[1];
  else if ( !strcmp( string, "and" ) )
    value = int(args[0]) & int(args[1]);
  else if ( !strcmp( string, "or" ) )
    value = int(args[0]) | int(args[1]);
  else if ( !strcmp( string, "bias" ) )
    value = bias(args[0], args[1]);
  else if ( !strcmp( string, "gain" ) )
    value = gain(args[0], args[1]);

  else if ( !strcmp( string, "noise" ) )
    value = noise( args[0], args[1], args[2] );
  else if ( !strcmp( string, "turbulence" ) )
    value = turbulence( args[0], args[1], args[2]);
  else if ( !strcmp( string, "IF" ) )
    value = ( args[0] ) ? args[1] : args[2];

  return value;
}


int args_sure( const char *function ) {
  if (
      !strcmp( function, "round" ) ||
      !strcmp( function, "abs" ) ||
      !strcmp( function, "exp" ) ||
      !strcmp( function, "log" ) ||
      !strcmp( function, "sin" ) ||
      !strcmp( function, "cos" ) ||
      !strcmp( function, "atan" ) ||
      !strcmp( function, "acos" ) ||
      !strcmp( function, "asin" ) ||
      !strcmp( function, "ceil" ) ||
      !strcmp( function, "floor" ) ||
      !strcmp( function, "sqrt" ) ||
      !strcmp( function, "tan" ) ||
      !strcmp( function, "neg" ) )
    return 1;
  else if ( !strcmp( function, "noise" ) ||
	    !strcmp( function, "turbulence" ) ||
	    !strcmp( function, "IF" ) )
    return 3;
  else if ( !strcmp( function, "RD" ) )
    return 6;
  return 2;
}



void gaInitialize(  ) {
  initialize_p();
  initialize_g();
}
