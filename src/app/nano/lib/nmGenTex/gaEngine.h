#ifndef _gaEngine_h_
#define _gaEngine_h_

#include <vrpn_Connection.h>


class gaEngine {
  
public:
  
  gaEngine (vrpn_Connection *);
  virtual ~gaEngine (void);
  
  virtual void mainloop (void);
  
  virtual void dimensions (int, int, int ) = 0;
  // the dataSets

  virtual void dataSet (int, int, float **) = 0;
  // the sends one row of data.

  virtual void number_of_variables (int) = 0;
  // which variables to use

  virtual void variableList (int, char **) = 0;
  // which variables to use

  virtual void selectGenomes (int, int) = 0;
  // Selects genomes to be crossed for the next generation.
						     
  virtual void reevaluateDataset (void) = 0;
  // Informs the GA engine that the dataset has changed and should
  // be reevaluated.
  
  virtual void texture ( int, int ) = 0;
  //

  virtual int evaluationComplete ( );
  //

  virtual int connectionComplete ( );
  //


  void registerEvaluationCompleteHandler (int (*) (void * ), void *);
  // Registers a function with one void * parameter returning int to
  // be called whenever texture evaluation completes.  Second parameter
  // is the data to be passed to the function when it is invoked.
  // It should return zero on success;  nonzero return values will
  // cause an error to be reported.

  void registerConnectionCompleteHandler (int (*) (void * ), void *);
  // Registers a function with one void * parameter returning int to
  // be called whenever the gaImplementationc connection is complete.
  // Second parameter is the data to be passed to the function when 
  // it is invoked. It should return zero on success;  nonzero return 
  // values will cause an error to be reported.
  
  float **data;
  int width, height, number;

  static const unsigned int defaultPort;
    // Default port for servers to listen on

protected:
  
  // keep track of VRPN stuff
  
  vrpn_Connection * my_connection;
  
  // sender ID for VRPN
  int myId;
  
  // message types for VRPN
  int reevaluateDataset_messagetype;
  int selectGenomes_messagetype;
  int texture_messagetype;
  int dimensions_messagetype;
  int dataSet_messagetype;
  int number_of_variables_messagetype;
  int variableList_messagetype;
  int evaluationComplete_messagetype;
  int connectionComplete_messagetype;
  
  // keep track of handlers that have been registered
  struct gaEngine_CompleteHandler {
    gaEngine_CompleteHandler * next;
    int (* function) (void *);
    void * userdata;
  };
  
  gaEngine_CompleteHandler * evaluation_complete;
  gaEngine_CompleteHandler * connection_complete;
  
  // Each encode_ routine will allocate a new char [] to hold
  // the appropriate encoding of its arguments, and write the
  // length of that buffer into its first argument.
  // It is the caller's responsibility to delete [] this buffer!
  // Please forgive...
  
  // (There is no encode_ routine for messages whose only argument
  // is a single const char *, since they might as well send the array.
  // A NULL array should be sent as a 0-length message.)
  // (There is also no encode_ routine for messages with no arguments.)


  char * encode_number_of_variables (int * len, int n );
  // Takes the number of variables and encodes it into a 
  // bufer for network transmission.  Returns a newly 
  // allocated buffer and copies the length of the buffer into its first argument.

  void decode_number_of_variables (const char * buf, int *n);
  // Extracts the number of variables from the buffer into the argument.

  
  char * encode_variableList (int * len, int, char **);
  // Encodes the names of the variables
  // into a bufer for network transmission.  Returns a newly 
  // allocated buffer and copies the length of the buffer into its first argument.
  
  void decode_variableList (const char * buf );
  // Extracts the names of the variables from the buffer into the arguments.

  char * encode_dimensions (int * len, int n, int w, int h );
  // Takes the number and size of the datasets and 
  // encodes them into a bufer for network transmission.  Returns a newly allocated
  // buffer and copies the length of the buffer into its first argument.

  void decode_dimensions ( const char * buf, int *n );
  // Extracts the number and size of the datasets from the buffer
  // into the arguments.

  char * encode_dataSet (int * len, int, float **d);
  // Returns the dataset as a char * for network transmission.
  // copies the length of the buffer into its first argument.

  void decode_dataSet (const char * buf, int, int);
    // Extracts the dataset from the buffer

  char * encode_texture (int * len, int, int);
  // Returns the texture as a char * for network transmission.
  // copies the length of the buffer into its first argument.

  void decode_texture (const char * buf, int);
  // Extracts the texture from the buffer


  char * encode_selectGenomes (int * len, int, int);
  // Returns ints as a char * for network transmission.
  // copies the length of the buffer into its first argument.

  void decode_selectGenomes (const char * buf, int *, int *);
  // Extracts the texture from the buffer


};

#endif
