#include <iostream.h>
#include <stdlib.h>
#include <stdio.h>
#ifndef _WIN32
#include <unistd.h>
#include <sys/types.h>

#include <vrpn_Shared.h> // include here to fix time stuff

#include <sys/time.h>
#endif
#include <vrpn_Connection.h>

#include "gaEngine_Implementation.h"
#include "gaFunctions.h"

gaEngine_Implementation::gaEngine_Implementation (vrpn_Connection *c) :
  gaEngine( c ) {

  if ( my_connection ) {
    my_connection->register_handler( number_of_variables_messagetype,
				     handle_number_of_variables,
				     this, vrpn_ANY_SENDER );
    my_connection->register_handler( variableList_messagetype,
				     handle_variableList,
				     this, vrpn_ANY_SENDER );
    my_connection->register_handler( dimensions_messagetype,
				     handle_dimensions,
				     this, vrpn_ANY_SENDER );
    my_connection->register_handler( dataSet_messagetype,
				     handle_dataSet,
				     this, vrpn_ANY_SENDER );
    my_connection->register_handler( selectGenomes_messagetype,
				     handle_selectGenomes,
				     this, vrpn_ANY_SENDER );
    my_connection->register_handler( reevaluateDataset_messagetype,
				     handle_reevaluateDataset,
				     this, vrpn_ANY_SENDER );
    myId = my_connection->register_sender("gaEngine Implementation" );
  }

  genes = NULL;
  data = new float*[ 12 ];
  for ( int i = 0; i < 12; i++ )
    data[i] = new float[ 512 * 512 * 3 ];



  struct timeval now;
  int retval = 0;
  gettimeofday( &now, NULL );
  if ( my_connection ) {
    retval = my_connection->pack_message( 0, now,
					  connectionComplete_messagetype, myId,
					  NULL, vrpn_CONNECTION_RELIABLE );
    if ( retval )  {
      fprintf(stderr, "gaEngine_Implementation:  "
	      "Couldn't pack message to send to remote.\n");
    }
  }
  else {
    fprintf(stderr, "gaEngine_Implementation:  "
	    "No connection.\n");
  }
}

gaEngine_Implementation::~gaEngine_Implementation (void) {}
  
void gaEngine_Implementation::mainloop (void) {
  gaEngine::mainloop();
}
  
void gaEngine_Implementation::selectGenomes (int i, int j) {
//   cout <<"Implementation:: Inside Select Genomes. i = " <<i <<" j = " <<j <<endl;

  if ( genes ) {
    genes->crossover( i, j );
  }
//   cout <<"Implementation:: DONE Select Genomes. i = " <<i <<" j = " <<j <<endl;
}

void gaEngine_Implementation::reevaluateDataset (void) {
//   cout <<"Implementation:: Inside Reevaluate Dataset" <<endl;

  float red, green, blue;
  if ( genes ) {
    for ( int i = 0; i < 12; i++ ) {
      for (  int i_index = 0; i_index < height; i_index++ )
	for ( int j_index = 0; j_index < width; j_index++ ) {
	  red   = genes->interpret( i, 0, i_index, j_index );
	  green = genes->interpret( i, 1, i_index, j_index );
	  blue  = genes->interpret( i, 2, i_index, j_index );
	  if ( red > 1.0 ) red = 1.0;
	  if ( red < 0.0 ) red = 0.0;

	  if ( green > 1.0 ) green = 1.0;
	  if ( green < 0.0 ) green = 0.0;

	  if ( blue > 1.0 ) blue = 1.0;
	  if ( blue < 0.0 ) blue = 0.0;
	  data[i][ i_index * 512 * 3 + j_index * 3 + 0] = red;
	  data[i][ i_index * 512 * 3 + j_index * 3 + 1] = green;
 	  data[i][ i_index * 512 * 3 + j_index * 3 + 2] = blue;
	    
	}
    }
  }
//   cout <<"Implementation:: DONE Reevaluate Dataset" <<endl;
}
  
void gaEngine_Implementation::number_of_variables ( int n ) {
//   cout <<"Implementation:: Inside number_of_variables " <<endl;

  GA_VARIABLE_LIST = new char*[ n ];
  for ( int i = 0; i < n; i++ )
    GA_VARIABLE_LIST[i] = new char[100]; // variable names !> 100

//   cout <<"Implementation:: DONE number_of_variables " <<endl;
}
  
void gaEngine_Implementation::variableList ( int n, char **vl ) {
//   cout <<"Implementation:: Inside variableList " <<endl;

  if ( n >= GA_NUMBER_OF_VARIABLES )
    GA_NUMBER_OF_VARIABLES = n;
  for ( int i = 0; i < n; i++ ) {
    strcpy( GA_VARIABLE_LIST[i], vl[i] );
  }
  this->initialize_genes();
//   cout <<"Implementation:: DONE variableList " <<endl;
}


void gaEngine_Implementation::dimensions ( int n, int w, int h ) {
// cout <<"Implementation:: Inside dimensions " <<n <<" " <<w <<" " <<h <<endl;

  GA_NUMBER_OF_INPUTS = n;
  width = w;
  height = h;

  GA_INPUTS = new float **[ n ];
  for ( int i = 0; i < n; i++ ) {
    GA_INPUTS[i] = new float*[h];
    for ( int j = 0; j < h; j++ )
      GA_INPUTS[i][j] = new float[w];
  }
//   cout <<"Implementation:: DONE dimensions" <<endl;
}

void gaEngine_Implementation::dataSet ( int set, int row, float **d ) {
//   cout <<"Implementation:: Inside dataSet " <<endl;

  if ( (set >= GA_NUMBER_OF_INPUTS) || ( row >= height ) )
    return;

  for ( int i = 0; i < width; i++ )
    GA_INPUTS[ set ][ row ][i] = d[row][i];

//   cout <<"Implementation:: DONE dataSet" <<endl;
}

void gaEngine_Implementation::texture ( int which, int row ) {
//   cout <<"gaEngine_Implementation inside texture" <<endl;
  
  // Pass message to Implementation:
  struct timeval now;
  int retval = 0;
  int len;
  gettimeofday( &now, NULL );
  char *buf = encode_texture( &len, which, row );
  if ( my_connection ) {
    retval = my_connection->pack_message( len, now,
					  texture_messagetype, myId, buf,
					  vrpn_CONNECTION_RELIABLE );    
    if (retval) {
      fprintf(stderr, "gaEngine_Implementation::texture:  "
	      "Couldn't pack message to send to client.\n");
    }
  }
  else
    fprintf(stderr, "gaEngine_Implementation::texture:  "
	    "No connection.\n");
  
  delete [] buf;
//   cout <<"gaEngine_Implementation DONE texture" <<endl;
}

  
//static
int gaEngine_Implementation::handle_number_of_variables (void *userdata, vrpn_HANDLERPARAM p) {
  gaEngine_Implementation *it = (gaEngine_Implementation *)(userdata);
  it->decode_number_of_variables( p.buffer, &GA_NUMBER_OF_VARIABLES );
  it->number_of_variables( GA_NUMBER_OF_VARIABLES );
  return 0;
}

//static
int gaEngine_Implementation::handle_variableList (void *userdata, vrpn_HANDLERPARAM p) {
  gaEngine_Implementation *it = (gaEngine_Implementation *)(userdata);
  it->decode_variableList( p.buffer );
  it->initialize_genes();
  return 0;
}

//static
int gaEngine_Implementation::handle_dimensions (void *userdata, vrpn_HANDLERPARAM p) {
  gaEngine_Implementation *it = (gaEngine_Implementation *)(userdata);

  it->decode_dimensions( p.buffer, &GA_NUMBER_OF_INPUTS );
  it->dimensions( GA_NUMBER_OF_INPUTS, it->width, it->height );
  return 0;
}


//static
int gaEngine_Implementation::handle_dataSet (void *userdata, vrpn_HANDLERPARAM p) {
  static int row = 0, set = 0;
  gaEngine_Implementation *it = (gaEngine_Implementation *)(userdata);

  it->decode_dataSet( p.buffer, row, set );
  if ( ( row + 1 ) < it->height ) {
    row = row + 1;
  }
  else if ( ( row + 1 ) == it->height ) {
    row = 0;
    set = set + 1;
  }
  if ( set == GA_NUMBER_OF_INPUTS ) {
    set = 0;
    row = 0;
    it->reevaluateDataset();
    it->gaEngine::evaluationComplete();
  }
  return 0;
}

//static
int gaEngine_Implementation::handle_selectGenomes (void *userdata, 
						   vrpn_HANDLERPARAM p) {
  
  gaEngine_Implementation *it = (gaEngine_Implementation *)(userdata);

  int i, j;
  it->decode_selectGenomes( p.buffer, &i, &j );
  it->selectGenomes( i, j );
  return 1;
}

//static
int gaEngine_Implementation::handle_reevaluateDataset (void *userdata, 
						       vrpn_HANDLERPARAM) {
  
  gaEngine_Implementation *it = (gaEngine_Implementation *)(userdata);
  it->reevaluateDataset( );
  return 1;
}

void gaEngine_Implementation::evaluate() {
  if ( genes ) {
    genes->evaluate();
    this->reevaluateDataset();
    this->gaEngine::evaluationComplete();
  }
}

void gaEngine_Implementation::evaluate2() {
  if ( genes ) {
    genes->evaluate2();
    this->reevaluateDataset();
    this->gaEngine::evaluationComplete();
  }
}

void gaEngine_Implementation::random() {
  initialize_genes();
  this->reevaluateDataset();
  this->gaEngine::evaluationComplete();
}


void gaEngine_Implementation::initialize_genes (  ) {
  if ( genes )
    delete genes;
  genes = new Genetic( 12 );
}

