#include <vrpn_Connection.h>
#include <iostream.h>
#include <stdlib.h>
#include <stdio.h>
#ifndef _WIN32
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#endif
#ifndef _WIN32
#include <netinet/in.h>
#endif

#include "gaEngine.h"
#include "gaFunctions.h"

//static
const unsigned int gaEngine::defaultPort = 4503;

gaEngine::gaEngine (vrpn_Connection *c) : my_connection( c ) {

  myId = -1;

  reevaluateDataset_messagetype =
    my_connection->register_message_type( "gaEngine reevaluateDataset" );
  selectGenomes_messagetype =
    my_connection->register_message_type( "gaEngine selectGenomes" );
  texture_messagetype =
    my_connection->register_message_type( "gaEngine texture" );
  dimensions_messagetype =
    my_connection->register_message_type( "gaEngine dimensions" );
  dataSet_messagetype      =
    my_connection->register_message_type( "gaEngine dataSet" );
  number_of_variables_messagetype  =
    my_connection->register_message_type( "gaEngine number_of_variables" );
  variableList_messagetype  =
    my_connection->register_message_type( "gaEngine variableList" );
  evaluationComplete_messagetype =
    my_connection->register_message_type( "gaEngine evaluateComplete" );
  connectionComplete_messagetype =
    my_connection->register_message_type( "gaEngine connectionComplete" );

  evaluation_complete = NULL;
  connection_complete = NULL;
  data = NULL;
}

gaEngine::~gaEngine (void) {
  gaEngine_CompleteHandler *tmp1, *tmp2;
  tmp1 = evaluation_complete;
  while ( tmp1 ) {
    tmp2 = tmp1->next;
    delete tmp1;
    tmp1 = tmp2;
  }

  tmp1 = connection_complete;
  while ( tmp1 ) {
    tmp2 = tmp1->next;
    delete tmp1;
    tmp1 = tmp2;
  }
}

void gaEngine::mainloop (void) {
  if (my_connection)
    my_connection->mainloop();
}


// Registers a function with one void * parameter returning int to
// be called whenever texture evaluation completes.  Second parameter
// is the data to be passed to the function when it is invoked.
// It should return zero on success;  nonzero return values will
// cause an error to be reported.
void gaEngine::registerEvaluationCompleteHandler (int (*f) (void * ), void *d) {

  if ( evaluation_complete == NULL ) {
    evaluation_complete = new gaEngine_CompleteHandler;
    evaluation_complete->next = NULL;
    evaluation_complete->function = f;
    evaluation_complete->userdata = d;
  }
  else {
    gaEngine_CompleteHandler *tmp = new gaEngine_CompleteHandler;
    tmp->next = evaluation_complete;
    tmp->function = f;
    tmp->userdata = d;
    evaluation_complete = tmp;
  }
}


// Registers a function with one void * parameter returning int to
// be called whenever the gaImplementationc connection is complete.
// Second parameter is the data to be passed to the function when 
// it is invoked. It should return zero on success;  nonzero return 
// values will cause an error to be reported.
void gaEngine::registerConnectionCompleteHandler (int (*f) (void * ), void *d) {

  if ( connection_complete == NULL ) {
    connection_complete = new gaEngine_CompleteHandler;
    connection_complete->next = NULL;
    connection_complete->function = f;
    connection_complete->userdata = d;
  }
  else {
    gaEngine_CompleteHandler *tmp = new gaEngine_CompleteHandler;
    tmp->next = connection_complete;
    tmp->function = f;
    tmp->userdata = d;
    connection_complete = tmp;
  }
}

int gaEngine::evaluationComplete ( ) {
  gaEngine_CompleteHandler *tmp;
  tmp = gaEngine::evaluation_complete;
  while ( tmp ) {
    tmp->function( tmp->userdata );
    tmp = tmp->next;
  }
  return 0;
}

int gaEngine::connectionComplete ( ) {
  gaEngine_CompleteHandler *tmp;
  tmp = gaEngine::connection_complete;
  while ( tmp ) {
    tmp->function( tmp->userdata );
    tmp = tmp->next;
  }
  return 0;
}

// Each encode_ routine will allocate a new char [] to hold
// the appropriate encoding of its arguments, and write the
// length of that buffer into its first argument.
// It is the caller's responsibility to delete [] this buffer!
// Please forgive...

// (There is no encode_ routine for messages whose only argument
// is a single const char *, since they might as well send the array.
// A NULL array should be sent as a 0-length message.)
// (There is also no encode_ routine for messages with no arguments.)


// Takes the number of variables and encodes it into a 
// bufer for network transmission.  Returns a newly 
// allocated buffer and copies the length of the buffer into its first argument.
char * gaEngine::encode_number_of_variables (int * len, int n ) {

  char * buf = NULL;

  if (!len) return NULL;

  *len = sizeof(int);
  buf = new char [*len];
  if (!buf) {
    fprintf(stderr, "gaEngine::encode_number_of_variables:  "
	    "Out of memory.\n");
    *len = 0;
  } else
    *((int *) buf) = htonl(n);

  return buf;
}

// Extracts the number of variables from the buffer into the argument.
void gaEngine::decode_number_of_variables (const char * buf, int *n) {
  if (!buf || !n)
    return;
  *n = ntohl(*((int *) buf));
}

// Encodes the names of the variables
// into a bufer for network transmission.  Returns a newly 
// allocated buffer and copies the length of the buffer into its first argument.
char * gaEngine::encode_variableList (int * len, int n, char **vl) {

  if ( !len )
    return NULL;

  int i;
  *len = 0;
  for ( i = 0; i < n; i++ )
    *len += strlen( vl[i] ) + 1;

  char *buf = new char[ *len ]; // plus room for int.
  *len = 0;
  for ( i = 0; i < n; i++ ) {
    sprintf( buf + *len, "%s ", vl[i] );
    *len = strlen( buf );
  }
  *len = strlen( buf ) + 1;

  return buf;
}
  
// Extracts the names of the variables from the buffer into the arguments.
void gaEngine::decode_variableList (const char * buf) {
  if ( !buf || !GA_VARIABLE_LIST )
    return;
  
  int len = 0;
  for ( int i = 0; i < GA_NUMBER_OF_VARIABLES; i++ ) {
    if ( !GA_VARIABLE_LIST[i] ) return;
    sscanf( buf + len, "%s ", GA_VARIABLE_LIST[i] );
    len += strlen( GA_VARIABLE_LIST[i] ) + 1;
  }
}


// Takes the number and size of the datasets and 
// encodes them into a bufer for network transmission.  Returns a newly allocated
// buffer and copies the length of the buffer into its first argument.
char * gaEngine::encode_dimensions (int * len, int n, int w, int h ) {

  char  * buf = NULL;
  int   * fb;
  
  if (!len) return NULL;
  
  *len = 3 * sizeof(int);
  buf = new char [*len];
  if (!buf) {
    fprintf(stderr, "gaEngine::encode_dimensions:  "
	    "Out of memory.\n");
    *len = 0;
  } else {
    fb = (int *) buf;
    fb[0] = htonl(n);
    fb[1] = htonl(w);
    fb[2] = htonl(h);
  }
  return buf;
}

// Extracts the number and size of the datasets from the buffer
// into the arguments.
void gaEngine::decode_dimensions ( const char * buf, int *n ) {
  if (! buf || !n )
    return;

  *n     = ntohl(((int *) buf)[0]);
  width  = ntohl(((int *) buf)[1]);
  height = ntohl(((int *) buf)[2]);
}

// Takes one row of the dataset and encodes it into the buffer, 
// then copies the length of the buffer into its first argument.
// Returns the row of the dataset as a char * for network transmission.
//
// Total pain in the ass to send floats accross the network in a
// byte-ordering transparent way.
// First an integer pointer is assigned to the location of the float
// in memory, so that the bytes of the float won't be cast to an int
// before the htonl call is made.
// The bytes are then copied from the int * to an int which is passed to
// the htonl call which encodes it for network transmission.
// This wouldn't be necessary if there were an equivalent float version
// of htonl and ntohl.
char * gaEngine::encode_dataSet (int * len, int row, float **d) {
  
  *len = sizeof( float ) * width;
  char * buf = new char[ *len ];


  int *fb; // int pointed to the buffer to be sent accross network
  int *tmp, tmpv; // temporary variables so the input float
                  // can be treated as an integer without casting to int.
                  // this way htonl can be called on the data for
                  // proper byte-ordering.

  float tmpf;     // temporary copy of data.
  if ( !buf ) {
    fprintf(stderr, "gaEngine::encode_dataSet:  "
	    "Out of memory.\n");
    *len = 0;
  } else {
    fb = ( int *) buf;
    for ( int i = 0; i < width; i++ ) {

      tmpf = d[row][i];                  // copy into local memory
      tmp = ( int * )(&tmpf);            // assign integer pointer to addr. of float
      tmpv = *tmp;                       // copies float bytes into int w/out cast
      fb[ i ] = htonl( tmpv );           // conversion for network transmission.
    }
  }
  
  return buf;
}

// Extracts the dataset from the buffer
// reverse of encode_dataSet... see above
void gaEngine::decode_dataSet (const char * buf, int row, int set) {
  if ( !buf )
    return;

  float *tmp, tmpv;
  int tmpi;
  for ( int i = 0; i < width; i++ ) {
    tmpi = ntohl( ((int *)buf)[i] );     // conversion from network transmission
    tmp = ( float * )(&tmpi);            // assign float pointer to address of int
    tmpv = *tmp;                         // copies the int bytes into float w/out cast
    GA_INPUTS[set][row][i] = tmpv;       // copies float into final resting place. :-)
  }
}


// Returns the texture as a char * for network transmission.
// copies the length of the buffer into its first argument.
char * gaEngine::encode_texture (int * len, int which, int row) {
  *len = sizeof( float ) * 512 * 3;
  char *buf = new char[ *len ];

  int *fb; // int pointed to the buffer to be sent accross network
  int *tmp, tmpv; // temporary variables so the input float
                  // can be treated as an integer without casting to int.
                  // this way htonl can be called on the data for
                  // proper byte-ordering.

  float tmpf;     // temporary copy of data.
  if ( !buf ) {
    fprintf(stderr, "gaEngine::encode_dataSet:  "
	    "Out of memory.\n");
    *len = 0;
  } else {
    fb = ( int *) buf;
    for ( int i = 0; i < 512; i++ ) {
      for ( int j = 0; j < 3; j++ ) {
	tmpf = data[which][row * 512 * 3 + i * 3 + j]; // copy into local memory
	tmp = ( int * )(&tmpf);              // assign integer pointer to addr. of float
	tmpv = *tmp;                         // copies float bytes into int w/out cast
	fb[ i * 3 + j ] = htonl( tmpv );     // conversion for network transmission.
      }
    }
  }
  return buf;
}

// Extracts the texture from the buffer
void gaEngine::decode_texture (const char * buf, int row) {
  if ( !buf )
    return;

  float *tmp, tmpv;
  int tmpi;
  for ( int i = 0; i < 512; i++ ) {
    for ( int j = 0; j < 3; j++ ) {
      tmpi = ntohl( ((int *)buf)[i * 3 + j] );     // conversion from network transmission
      tmp = ( float * )(&tmpi);                // assign float pointer to address of int
      tmpv = *tmp;                             // copies the int bytes into float w/out cast
      data[0][row * 512 * 3 + i * 3 + j] = tmpv;
    }
  }
}

// Takes the genome indices and encodes them into a buffer for
// network transmission.  Returns a newly allocated
// buffer and copies the length of the buffer into its first argument.
char * gaEngine::encode_selectGenomes (int * len, int i, int j ) {

  char  * buf = NULL;
  int   * fb;
  
  if (!len) return NULL;
  
  *len = 2 * sizeof(int);
  buf = new char [*len];
  if (!buf) {
    fprintf(stderr, "gaEngine::encode_selectGenomes:  "
	    "Out of memory.\n");
    *len = 0;
  } else {
    fb = (int *) buf;
    fb[0] = htonl(i);
    fb[1] = htonl(j);
  }
  return buf;
}

// Extracts the indices of the genes to crossbreed from the buffer
// into the arguments.
void gaEngine::decode_selectGenomes ( const char * buf, int *i, int *j ) {
  if (! buf || !i || !j )
    return;

  *i     = ntohl(((int *) buf)[0]);
  *j     = ntohl(((int *) buf)[1]);
}
