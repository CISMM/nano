#include <iostream.h>
#include <stdlib.h>
#include <stdio.h>
#ifndef _WIN32
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <vrpn_Connection.h>

#include "gaEngine_Remote.h"

gaEngine_Remote::gaEngine_Remote (vrpn_Connection *c) : gaEngine( c ) {

  if ( my_connection ) {
    my_connection->register_handler( connectionComplete_messagetype,
				     handle_connectionComplete,
				     this, vrpn_ANY_SENDER );
    my_connection->register_handler( texture_messagetype,
				     handle_texture,
				     this, vrpn_ANY_SENDER );
    myId = my_connection->register_sender("gaEngine Remote" );
  }
  else
    cerr <<"No connection yet" <<endl;
  data = new float*[ 1 ];
  for ( int i = 0; i < 1; i++ )
    data[i] = new float[ 512 * 512 * 3 ];
}

gaEngine_Remote::~gaEngine_Remote (void) {}
  
void gaEngine_Remote::mainloop (void) {
  gaEngine::mainloop();
}
  
void gaEngine_Remote::selectGenomes (int i, int j) {
//   cout <<"gaEngine_Remote inside select Genomes " <<i <<" " <<j <<endl;

  struct timeval now;
  int retval = 0;
  int len;
  char *buf = encode_selectGenomes( &len, i, j );
  gettimeofday( &now, NULL );
  if ( my_connection ) {
    retval = my_connection->pack_message( len, now,
					  selectGenomes_messagetype, myId, buf,
					  vrpn_CONNECTION_RELIABLE );    
    if (retval) {
      fprintf(stderr, "gaEngine_Remote::selectGenomes:  "
	      "Couldn't pack message to send to server.\n");
    }
  }
  else
    fprintf(stderr, "gaEngine_Remote::selectGenomes:  "
	    "No connection.\n");

  delete [] buf;

//   cout <<"gaEngine_Remote DONE select Genomes" <<endl;
}

void gaEngine_Remote::reevaluateDataset (void) {
//   cout <<"gaEngine_Remote inside reevaluate dataset" <<endl;

  struct timeval now;
  int retval = 0;
  gettimeofday( &now, NULL );
  if ( my_connection ) {
    retval = my_connection->pack_message( 0, now,
					  reevaluateDataset_messagetype,
					  myId, NULL,
					  vrpn_CONNECTION_RELIABLE );    
    if (retval) {
      fprintf(stderr, "gaEngine_Remote::reevaluateDataset:  "
	      "Couldn't pack message to send to server.\n");
    }
  }
  else
    fprintf(stderr, "gaEngine_Remote::reevaluateDataset:  "
	    "No connection.\n");

//   cout <<"gaEngine_Remote DONE reevaluate dataset" <<endl;
}

void gaEngine_Remote::number_of_variables ( int n ) {
//   cout <<"gaEngine_Remote inside number_of_variables" <<endl;

  struct timeval now;
  int retval = 0;
  int len;
  char *buf = encode_number_of_variables( &len, n );
  gettimeofday( &now, NULL );
  if ( my_connection ) {
    retval = my_connection->pack_message( len, now,
					  number_of_variables_messagetype, myId, buf,
					  vrpn_CONNECTION_RELIABLE );    
    if (retval) {
      fprintf(stderr, "gaEngine_Remote::number_of_variables:  "
	      "Couldn't pack message to send to server.\n");
    }
  }
  else
    fprintf(stderr, "gaEngine_Remote::number_of_variables:  "
	    "No connection.\n");

  delete [] buf;
//   cout <<"gaEngine_Remote DONE number_of_variables" <<endl;
}

void gaEngine_Remote::variableList ( int n, char **vl ) {
//   cout <<"gaEngine_Remote inside variableList" <<endl;

  struct timeval now;
  int retval = 0;
  int len;
  char *buf = encode_variableList( &len, n, vl );
  gettimeofday( &now, NULL );
  if ( my_connection ) {
    retval = my_connection->pack_message( len, now,
					  variableList_messagetype, myId, buf,
					  vrpn_CONNECTION_RELIABLE );    
    if (retval) {
      fprintf(stderr, "gaEngine_Remote::variableList:  "
	      "Couldn't pack message to send to server.\n");
    }
  }
  else
    fprintf(stderr, "gaEngine_Remote::variableList:  "
	    "No connection.\n");

  delete [] buf;
//   cout <<"gaEngine_Remote DONE variableList" <<endl;
}

void gaEngine_Remote::dimensions ( int n, int w, int h ) {
//   cout <<"gaEngine_Remote inside dimensions" <<endl;

  width = w;
  height = h;

  // Pass message to Implementation:
  struct timeval now;
  int retval = 0;
  int len;

  gettimeofday( &now, NULL );
  char *buf = encode_dimensions( &len, n, w, h );
  if ( my_connection ) {
    retval = my_connection->pack_message( len, now,
					  dimensions_messagetype, myId, buf,
					  vrpn_CONNECTION_RELIABLE );    
    if (retval) {
      fprintf(stderr, "gaEngine_Remote::dimensions:  "
	      "Couldn't pack message to send to server.\n");
    }
  }
  else
    fprintf(stderr, "gaEngine_Remote::dimensions:  "
	    "No connection.\n");

  delete [] buf;
//   cout <<"gaEngine_Remote DONE dimensions" <<endl;
}

void gaEngine_Remote::dataSet ( int /* set */, int row, float **d ) {
//   cout <<"gaEngine_Remote inside dataSet" <<endl;


  // Pass message to Implementation:
  struct timeval now;
  int retval = 0;
  int len;
  gettimeofday( &now, NULL );
  char *buf = encode_dataSet( &len, row, d );
  if ( my_connection ) {
    retval = my_connection->pack_message( len, now,
					  dataSet_messagetype, myId, buf,
					  vrpn_CONNECTION_RELIABLE );    
    if (retval) {
      fprintf(stderr, "gaEngine_Remote::dataSet:  "
	      "Couldn't pack message to send to server.\n");
    }
  }
  else
    fprintf(stderr, "gaEngine_Remote::dataSet:  "
	    "No connection.\n");

  delete [] buf;
//   cout <<"gaEngine_Remote DONE dataSet" <<endl;
}

void gaEngine_Remote::texture (int /* which */, int /* row */) {
//   cout <<"gaEngine_Remote inside texture" <<endl;

//   cout <<"gaEngine_Remote DONE texture" <<endl;
}

//calls user-defined handlers that have been registered.
//static
int gaEngine_Remote::handle_connectionComplete (void *userdata,
						vrpn_HANDLERPARAM) {
  gaEngine_Remote *it = (gaEngine_Remote *)(userdata);
  return it->gaEngine::connectionComplete( );
}


//calls user-defined handlers that have been registered.
//static
int gaEngine_Remote::handle_texture (void *userdata, vrpn_HANDLERPARAM p) {
  static int row = 0;
  static int complete = 0;

  gaEngine_Remote *it = (gaEngine_Remote *)(userdata);

  it->decode_texture( p.buffer, row );
  if ( (row + 1) == 512 ) {
    row = 0;
    complete = 1;
  }
  else {
    row = row + 1;
    complete = 0;
  }
  it->texture( 0, row );
  if ( complete )
    return it->gaEngine::evaluationComplete( );
  return 0;
}
