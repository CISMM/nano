#ifndef _genome_h_
#define _genome_h_

enum expression_type {TERMINAL_VARIABLE, TERMINAL_NUMERICAL, FUNCTION};

extern char FUNCTION_LIST[28][20];
extern int NUMBER_OF_FUNCTIONS;

extern float FN_MUTATION;
extern float NEW_RANDOM_EXPRESSION;
extern float VALUE_MUTATION;
extern float VARIABLE_MUTATION;
extern float NUMERICAL;
extern float VARIABLE;

class Genome {
public:  
  Genome( int limit, int count );
  Genome( Genome *g );
  Genome( const char *string );
  ~Genome();
  void print();
  void count( int &result );
  void count( const char *string, int &result );
  Genome *mutate();
  int replace( Genome *g, int &a );
  int replace_delete( Genome *g, int &a );
  Genome *retrieve( int &a );
  float interpret( int i_index, int j_index );

private:
  void new_random_expression( int limit, int count );

  int args( const char *expression );
  char *expression_;
  int expression_length_;
  Genome **arguments_;
  int number_of_arguments_;
  expression_type type_;
};


#endif
