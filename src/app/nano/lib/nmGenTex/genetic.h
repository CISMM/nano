#ifndef _genetic_h_
#define _genetic_h_

#include "genome.h"

class Genetic {
public:
  Genetic( int size );
  ~Genetic();
  void crossover( int p1, int p2 );
  void crossover( int i1, int j1, int i2, int j2);
  float interpret( int i, int j, int i_index, int j_index );
  void print( int i );
  void replace( int i );
  void evaluate();
  void evaluate2();

private:
  Genome ***population_;
  int population_size_;
};

#endif
