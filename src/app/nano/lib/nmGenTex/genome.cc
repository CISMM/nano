#include <stdio.h>
#include <stdlib.h>
#include <iostream.h>
#include <string.h>
#include <ctype.h>

#include "genome.h"
#include "gaFunctions.h"
extern "C" double drand48(void);

char FUNCTION_LIST[28][20] ={ "min", "abs", "sin", "cos", "ceil",
			      "floor", "sqrt", "bias", "gain", "max",
			      "log", "exp", "+", "-", "*", "/", "atan",
			      "acos", "asin", "and", "or", "mod", "tan",
			      "neg", "round", "IF", "noise", "turbulence" };
int NUMBER_OF_FUNCTIONS = 26;

float FN_MUTATION = .4;
float NEW_RANDOM_EXPRESSION = .50;
float VALUE_MUTATION = .3;
float VARIABLE_MUTATION = .3;
float NUMERICAL = .99;
float VARIABLE = .4;


// Randomly construct a genome, based on the list of possible
// functions and variables in gaFunctions.h
//
Genome::Genome( int limit, int count ) {
  new_random_expression( limit, count );
}

// Recursively copy the genome g into this:
Genome::Genome( Genome *g ) {
  expression_ = new char[ strlen( g->expression_ ) + 1 ];
  strcpy( expression_, g->expression_ );
  type_ = g->type_;
  number_of_arguments_ = g->number_of_arguments_;
  if ( number_of_arguments_ > 0 ) {
    arguments_ = new Genome*[ number_of_arguments_ ];
    for ( int i = 0; i < number_of_arguments_; i++ )
      arguments_[i] = new Genome( g->arguments_[i] );
  }
  else
    arguments_ = NULL;
}


// This function takes a lisp expression as an argument, and creates the
// parse tree for that list expression. First the function is determined, and 
// the number of arguments, then the function is called recursively with each
// subexpression. Because it is a recursive function, the terminating
// conditions are handled first.
Genome::Genome( const char *string ) {
  int length = strlen( string );

  // Terminal conditions, numerical and variables are handled first:
  // Numerical value:
  if ( ( string[0] != '(' ) &&  
       ( isdigit( string[0] ) || ( string[0] == '-' ) ) ) {
    expression_ = new char [ length + 1 ];
    strcpy( expression_, string );
    type_ = TERMINAL_NUMERICAL;
    number_of_arguments_ = 0;
    arguments_ = NULL;
  }
  else if ( string[0] != '(' ) {
    expression_ = new char [ length + 1 ];
    strcpy( expression_, string );
    type_ = TERMINAL_VARIABLE;
    number_of_arguments_ = 0;
    arguments_ = NULL;
  }
  else {
  // assume the buffer contains a function of the form: (fn args...)
  // First get the function:
    char *fn = new char[ length + 1 ];
    // Set the current_index into the buffer after the first left paren:
    int i;
    int current_index = 0;
    for ( i = current_index; ( i < length ) && string[ i ] != '('; i++ );
    current_index = i + 1;

    // Set the current_index to the first non-space character:
    for ( i = current_index; ( i < length ) && string[ i ] == ' '; i++ );
    current_index = i;

    // Copy the function name:
    for ( i = current_index; ( i < length ) && string[ i ] != ' '; i++ )
      fn[ i - current_index ] = string[ i ];
    fn[ i - current_index ] = '\0';
    current_index = i; // now the current index is the first space after the fn
    expression_ = new char [ strlen( fn ) + 1 ];
    strcpy( expression_, fn );
    delete [] fn;
    type_ = FUNCTION;
    

  // Next create a child Genome for each argument:
    number_of_arguments_ = args( string );
    arguments_ = new Genome*[ number_of_arguments_ ];

    char *arg_string = new char[ length + 1 ];
    for ( i = 0; i < number_of_arguments_; i++ ) {
      int open = 0, close = 0;
      // the current index is the first non-space after the fn
      // Set the current index to the fist non-space character:

      int j;
      for ( j = current_index; ( j < length ) && string[ j ] == ' '; j++ );
      current_index = j;

      // The argument is a terminal, extract it and recursively create new G:
      if ( string[ current_index ] != '(' ) { 
	for ( j = current_index;
	      ( j < length ) &&
		( (string[ j ] != ' ' ) && (string[ j ] != ')' )); j++ ) {
	  arg_string[j - current_index] = string[ j ];
	}
	arg_string[j - current_index ] = '\0';
	arguments_[ i ] = new Genome( arg_string );
      }
      // The argument is a subfunction, extract it:
      else {
	for ( j = current_index; ( j < length ) &&
		( ( open != close ) || ( open == 0 ) ); 
	      j++ ) {
	  arg_string[ j - current_index ] = string[ j ];
	  if ( string[ j ] == '(' )
	    open++;
	  else if ( string[ j ] == ')' )
	    close++;
	}
	arg_string[j - current_index ] = '\0';
	arguments_[i] = new Genome( arg_string );
      }
      current_index = j;
    }
    delete [] arg_string;
  }
}


Genome::~Genome() {
  delete [] expression_;
  if ( number_of_arguments_ > 0 ) {
    for ( int i = 0; i < number_of_arguments_; i++ )
      delete arguments_[i];
    delete [] arguments_;
  }
}

// Recursively print the parse-tree:
void Genome::print() {
  if ( type_ == FUNCTION )
    cout <<"(";
  cout <<" " <<expression_ <<" ";
  for ( int i = 0; i < number_of_arguments_; i++ )
    arguments_[i]->print();
  if ( type_ == FUNCTION )
    cout <<")";
}

// Recursively count the nodes in the parse-tree:
void Genome::count( int &result ) {
  result++;
  for ( int i = 0; i < number_of_arguments_; i++ )
    arguments_[i]->count( result );
}

void Genome::count( const char *string, int &result ) {
  if ( !strcmp( string, expression_ ) )
    result++;
  for ( int i = 0; i < number_of_arguments_; i++ )
    arguments_[i]->count( string, result );
}

void Genome::new_random_expression( int limit, int count ) {
  int index;
  if ( ( drand48() > .85 ) || ( count >= limit ) ) { // become a varaible:
    index = rand() % GA_NUMBER_OF_VARIABLES;
    expression_ = new char[ strlen( GA_VARIABLE_LIST[ index ] ) + 1 ];
    strcpy( expression_, GA_VARIABLE_LIST[ index ] );
    type_ = TERMINAL_VARIABLE;
    number_of_arguments_ = 0;
    arguments_ = NULL;
  }
  else if ( drand48() > .95 ) { // become a constant:
    char num[30];
    sprintf( num, "%f", drand48() );
    expression_ = new char[ strlen( num ) + 1 ];
    strcpy( expression_, num );
    type_ = TERMINAL_NUMERICAL;
    number_of_arguments_ = 0;
    arguments_ = NULL;
  }
  else {
    int function = rand() % NUMBER_OF_FUNCTIONS; // pick random function:
    expression_ = new char[ strlen( FUNCTION_LIST[ function ] ) + 1 ];
    strcpy( expression_, FUNCTION_LIST[ function ] );
    type_ = FUNCTION;
    number_of_arguments_ = args_sure( FUNCTION_LIST[ function ] );
    arguments_ = new Genome*[ number_of_arguments_ ];
  
    for ( int i = 0; i < number_of_arguments_; i++ ) {
      arguments_[i] = new Genome( limit, count + 1 );
    }
  }
}

// Mutate the fn
// 1). No Mutation.
// 2). Function changes to new random function.
// 3). Function changes to one of arguments.
// 4). Function becomes the argument to a new random expression
// 5). Function changes into a new function
Genome *Genome::mutate() {
  // 1). no mutation, recurse on children
  if ( drand48() < FN_MUTATION ) {
    for ( int i = 0; i < number_of_arguments_; i++ ) {
      Genome *g;
      if ( ( g = arguments_[i]->mutate() ) != NULL ) {
	delete arguments_[i];
	arguments_[i] = g;
      }
    }
  }
  // 2). change to a random expression.
  else if ( drand48() > NEW_RANDOM_EXPRESSION ) { 
//     cout <<"mutation" <<endl;
    return new Genome( 5, 0 );
  }  
  // 3). change to one of arguments:
  else if ( drand48() > .85 ) {  
//     cout <<"mutation" <<endl;
    for ( int i = 0; i < number_of_arguments_; i++ )
      if ( drand48() < 1.0/(number_of_arguments_ - i) )
	return new Genome( arguments_[i] );
  }
  // 4). Function becomes the argument to a new random expression
//   else if ( drand48() > .25 ) {  
//     Genome *g = new Genome( 2, 0 );
//     int num;
//     g->count( num );
//     if ( num > 1 ) {
//       g->replace_delete( this, 1 );
//       return g;
//     }
//     else
//       delete g;
//   }

  return NULL;
}

// Recursive find the child to replace assuming an inorder numbering
// of the tree, then replace the child with g.
int Genome::replace( Genome *g, int &a ) {
  if ( a == 0 )             // terminating condition
    return 1;
  else { // recursively search subchildren
    for ( int i = 0; i < number_of_arguments_; i++ )
      if ( arguments_[i]->replace( g, --a ) == 1 ) {
	arguments_[i] = g;
	return 0;
      }
  }
  return 0;
}

// Recursive find the child to replace assuming an inorder numbering
// of the tree, then delete the child and replace with g.
int Genome::replace_delete( Genome *g, int &a ) {
  if ( a == 0 )             // terminating condition
    return 1;
  else { // recursively search subchildren
    for ( int i = 0; i < number_of_arguments_; i++ )
      if ( arguments_[i]->replace( g, --a ) == 1 ) {
	delete arguments_[i];
	arguments_[i] = g;
	return 0;
      }
  }
  return 0;
}

// Recursive find the child a assuming an inorder numbering
Genome *Genome::retrieve( int &a ) {
  if ( a == 0 )             // terminating condition
    return this;
  else { // recursively search subchildren
    Genome *g;
    for ( int i = 0; i < number_of_arguments_; i++ ) {
      if ( ( g = arguments_[i]->retrieve( --a ) ) != NULL )
	return g;
    }
  }
  return NULL;
}

float Genome::interpret( int i_index, int j_index ) {
  if ( type_ == TERMINAL_NUMERICAL ) {
    return atof( expression_ );
  }
  else if ( type_ == TERMINAL_VARIABLE ) {
    return evaluate_variable( expression_, i_index, j_index );
  }
  else if ( type_ == FUNCTION ) {
    float *args = new float[ number_of_arguments_ ];
    for ( int i = 0; i < number_of_arguments_; i++ )
      args[i] = arguments_[i]->interpret( i_index, j_index );
    float value = evaluate_function( expression_, args );
    delete [] args;
    return value;
  }
  return 0;
}

// Assume that string is a string of the form (fn args...)
// Where the number of arguments is unlimited, and may contain subfunctions.
// Parse the arguments, and return the number:
int Genome::args( const char *string ) {
  int current_index = 0;
  int length = strlen( string );
  int i;
  // Find the first paren, and skip it:
  for ( i = 0; ( i < length ) && ( string[i] != '(' ); i++ );
  current_index = i+1;

  // Find the function name, skiping spaces:
  for ( i = current_index; ( i < length ) && ( string[i] == ' ' ); i++ );
  current_index = i;
  
  // At the function name, skip it:
  for ( i = current_index; ( i < length ) && ( string[i] != ' ' ); i++ );
  current_index = i;

  // Find the first non-space char:
  for ( i = current_index; ( i < length ) && ( string[i] == ' ' ); i++ );
  current_index = i;

  // Now current_index is at the first character of the first argument:
  // (fn arg1 arg2 arg3 ... )
  //     ^
  // Count the arguments:
  int number_of_args = 0;
  while ( current_index < ( length - 1 ) ) {
    cout <<current_index <<endl;
    // at non-space char, either terminal or subexpression:
    if ( string[ current_index ] != '(' ) {       // terminal
      // move current_index to end of terminal, and up number_of_args:
      for ( i = current_index;
	    ( i < length ) && ( string[i] != ' ' ); i++ );
      current_index = i;
      number_of_args++;
    }
    else if ( string[ current_index ] == '(' ) {  // subexpression
      // move current_index to end of subexpression, and up number_of_args:
      int open = 0, close = 0;
      for ( i = current_index; ( i < length ) &&
	      ( ( open != close ) || ( open == 0 ) ); 
	    i++ ) {
	if ( string[ i ] == '(' )
	  open++;
	else if ( string[ i ] == ')' )
	  close++;
      }
      current_index = i;
      number_of_args++;
    }
    // Find the first non-space char:
    for ( i = current_index; ( i < length ) && ( string[i] == ' ' ); i++ );
    current_index = i;
  }
  return number_of_args;
}
