#ifndef _gaFunctions_h_
#define _gaFunctions_h_

// data input to the genetic algorithm... from nanomanipulator data

extern char **GA_VARIABLE_LIST;
extern int GA_NUMBER_OF_VARIABLES;
extern int GA_NUMBER_OF_INPUTS;
extern float ***GA_INPUTS;


// functions for setting up the variable list and input data:
void gaInitialize(  );

// functions for evaluating/manipulating data:
float evaluate_variable( const char *string, int i_index, int j_index );
float evaluate_function( const char *string, float *args );
int args_sure( const char *function );



#endif
