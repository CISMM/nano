#include <stdlib.h>
#include <stdio.h>
#include <iostream.h>
#include <fstream.h>
#include <string.h>
#include <math.h>

#include "input.h"

char **VARIABLE_LIST;
int NUMBER_OF_VARIABLES = 0;
int NUMBER_OF_INPUTS = 0;
float ***INPUTS;
int DATA_WIDTH;
int DATA_HEIGHT;

void read_inputs( char input_file[] ) {
  FILE *instream = fopen( input_file, "r" );
  if ( !instream ) {
    cerr <<"Cannot open file " <<input_file <<endl;
    exit( 1 );
  }
 
  char *line = new char[84];
  char path_name[200];
  path_name[0] = '\0';
  int index = 0;

  while ( fgets( line, 84, instream ) ) {
    if ( line[0] == '#' );
    else if ( NUMBER_OF_VARIABLES == 0 ) {
      sscanf( line, "%d", &NUMBER_OF_INPUTS );
      NUMBER_OF_VARIABLES = NUMBER_OF_INPUTS;
      if ( strstr( line, "X" ) )	NUMBER_OF_VARIABLES++;
      if ( strstr( line, "Y" ) )	NUMBER_OF_VARIABLES++;

      cout <<"NUMBER_OF_VARIABLES = " <<NUMBER_OF_VARIABLES <<endl;
      VARIABLE_LIST = new char*[NUMBER_OF_VARIABLES];
      for ( int i = 0; i < NUMBER_OF_VARIABLES; i++ )
	VARIABLE_LIST[i] = new char[50];
      index=0;
      if ( strstr( line, "TIME" ) ) {
	strcpy( VARIABLE_LIST[index], "TIME" );
	index++;
      }
      if ( strstr( line, "X" ) ) {
	strcpy( VARIABLE_LIST[index], "X" );
	index++;
      }
      if ( strstr( line, "Y" ) ) {
	strcpy( VARIABLE_LIST[index], "Y" );
	index++;
      }
    }
    else if ( path_name[0] == '\0' ) {
      sscanf( line, "%s", path_name );
      cout <<"path name = " <<path_name <<endl;
    }
    else {
      VARIABLE_LIST[ index ][0] = 'V';
      sscanf( line, "%s", VARIABLE_LIST[ index ] + 1 );
      cout <<"VARIABLE_LIST[" <<index <<"] = " <<VARIABLE_LIST[index] <<endl;
      index++;
    }
  }
  fclose( instream );
  
  delete [] line;

  INPUTS = new float**[ NUMBER_OF_INPUTS ];

  short int width, height;
  float min, max;
  
  char *string;
  int i;
  for (  i = 0; i < NUMBER_OF_INPUTS; i++ ) {
    index = i + NUMBER_OF_VARIABLES - NUMBER_OF_INPUTS;
    string = new char[ strlen( path_name ) + strlen( VARIABLE_LIST[index] ) + 1];
    strcpy( string, path_name );
    strcat( string, VARIABLE_LIST[ index ] +1 );
//     cout <<string <<endl;

    FILE *fp = fopen( string, "rb" );
    if ( !fp ) {
      cerr <<"Cannot open file " <<string <<endl;
      exit( 1 );
    }
    delete [] string;
  
    fread( &width, sizeof( short ), 1, fp );
    fread( &height, sizeof( short ), 1, fp );
    fread( &min, sizeof( float ), 1, fp );
    fread( &max, sizeof( float ), 1, fp );
    DATA_WIDTH = width;
    DATA_HEIGHT = height;
    cout <<DATA_WIDTH <<" " <<DATA_HEIGHT <<endl;
    cout <<min <<" " <<max <<endl;

    INPUTS[i] = new float*[DATA_HEIGHT];
    int j;
    for ( j = 0; j < DATA_HEIGHT; j++ ) {
      INPUTS[i][j] = new float[DATA_WIDTH];
      fread( INPUTS[i][j], sizeof( float ) * DATA_WIDTH, 1, fp );
    }
    fclose( fp );
  
    for ( j = 0; j < DATA_HEIGHT; j++ )
      for ( int k = 0; k < DATA_WIDTH; k++ ) 
	INPUTS[i][j][k] =( INPUTS[i][j][k] - min)/(max-min);
  }
}

void create_input_files( char *input_file, char *datafile ) {
  ofstream output( input_file );
  if ( !output ) {
    cerr <<"Cannot write to output file : " <<input_file <<endl;
    exit( 1 );
  }
  output <<NUMBER_OF_VARIABLES <<endl;
  int i;
  for ( i = 0; i < NUMBER_OF_VARIABLES; i++ ) {
    output <<VARIABLE_LIST[ i ] <<endl;
  }
  output.close();



//   output.open( datafile );
//   if ( !output ) {
//     cerr <<"Cannot write to output file : " <<datafile <<endl;
//     exit( 1 );
//   }
//   output <<NUMBER_OF_INPUTS <<endl;
//   for ( i = 0; i < NUMBER_OF_INPUTS; i++ ) {
//     for ( int j = 0; j < DATA_HEIGHT; j++ ) {
//       for ( int k = 0; k < DATA_WIDTH; k++ )
// 	output <<INPUTS[ i ][ j ][ k ] <<" ";
//     }
//   }
//   output.close();


   FILE *fp = fopen( datafile, "wb" );
   if ( !fp ) {
     cerr <<"Cannot open file " <<datafile <<endl;
     exit( 1 );
   }

   fwrite( &NUMBER_OF_INPUTS, sizeof( int ), 1, fp );
   for ( i = 0; i < NUMBER_OF_INPUTS; i++ ) {
     for ( int j = 0; j < DATA_HEIGHT; j++ ) {
       fwrite( INPUTS[i][j], sizeof( float ) * DATA_WIDTH, 1, fp );
     }
   }
   fclose( fp );
}

#define	RETURN_RGB(x,y,z) {*r=x; *g=y; *b=z; break;}
void hsv_to_rgb(float h, float s, float v, float *r, float *g, float *b)
{
        // H is given on [0, 360]. S and V are given on [0, 1]. 
        // RGB are each returned on [0, 1]. 
        float f,p,q,t; 
        int i; 

	h /= 60;		// Range of h to [0-6]
	if (h == 6) {h = 0;};	// Range of h to [0-6>

	i = (int)floor(h);	// Which sextant is it in?
	f = h-i;		// Fractional part within sextant
	p = v*(1-(s));
	q = v*(1-(s*f));
	t = v*(1-(s*(1-f)));
	switch (i) {
	  case 0: RETURN_RGB(v,t,p);
	  case 1: RETURN_RGB(q,v,p);
	  case 2: RETURN_RGB(p,v,t);
	  case 3: RETURN_RGB(p,q,v);
	  case 4: RETURN_RGB(t,p,v);
	  case 5: RETURN_RGB(v,p,q);
	}
}
