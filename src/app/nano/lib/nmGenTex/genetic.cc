#include <iostream.h>
#include <stdlib.h>
#include "genetic.h"
#include "gaFunctions.h"

Genetic::Genetic( int size ) {
  gaInitialize(  );

  population_size_ = size;
  population_ = new Genome**[ population_size_];
  for ( int i = 0; i < population_size_; i++ ) {
    population_[i] = new Genome*[3];
    for ( int j = 0; j < 3; j++ ) {
      int limit = rand() % 5;
      population_[i][j] = new Genome( limit, 0 );
    }
  }
}

Genetic::~Genetic( ) {
  for ( int i = 0; i < population_size_; i++ ) {
    for ( int j = 0; j < 3; j++ ) 
      delete population_[i][j];
    delete [] population_[i];
  }
  delete [] population_;
}

void Genetic::crossover( int p1, int p2 ) {
  cout <<"Crossover: " <<p1 <<" " <<p2 <<endl;
  for ( int i = 0; i < population_size_; i++ ) {
    if ( ( i != p1 ) && ( i != p2 ) ) {
      for ( int j = 0; j < 3; j++ ) {
	int c1, c2;
	int a, b;
	c1 = 0;
	c2 = 0;
	population_[p1][j]->count( c1 );
	population_[p2][j]->count( c2 );
	a = ( rand()%( c1 ) );
	b = ( rand()%( c2 ) );
	if ( a == 0 ) a++;
	if ( b == 0 ) b++;
	if ( a == c1 ) a--;
	if ( b == c2 ) b--;

	Genome *g2 = new Genome( population_[p2][j]->retrieve( b ) );

	delete population_[i][j];
	population_[i][j] = new Genome( population_[p1][j] );

	population_[i][j]->replace_delete( g2, a );
	if ( (g2 = population_[i][j]->mutate()) ) {
	  delete population_[i][j];
	  population_[i][j] = g2;
	}
      }
    }
  }
}

/*
void Genetic::crossover( int i1, int j1, int i2, int j2 ) {
  cout <<"Crossover: " <<i1 <<"," <<j1 <<" " <<i2 <<"," <<j2 <<endl;

  Genome *pg1 = new Genome( population_[i1][j1] );
  Genome *pg2 = new Genome( population_[i2][j2] );
  for ( int i = 0; i < population_size_; i++ ) {
    if ( ( i != i1 ) && ( i != i2 ) ) {
      for ( int j = 0; j < 3; j++ ) {
	if ( ( j != j1 ) && ( j != j2 ) ) {
	  int c1, c2;
	  int a, b;
	  c1 = 0;
	  c2 = 0;
	  population_[i1][j1]->count( c1 );
	  population_[i2][j2]->count( c2 );
	  a = ( rand()%( c1 ) );
	  b = ( rand()%( c2 ) );
	  if ( a == 0 ) a++;
	  if ( b == 0 ) b++;
	  if ( a == c1 ) a--;
	  if ( b == c2 ) b--;
	  
	  Genome *g2 = new Genome( population_[i2][j2]->retrieve( b ) );
	  
	  delete population_[i][j];
	  population_[i][j] = new Genome( population_[i1][j1] );
	  
	  population_[i][j]->replace_delete( g2, a );
	  if ( (g2 = population_[i][j]->mutate()) ) {
	    delete population_[i][j];
	    population_[i][j] = g2;
	  }
	}
      }
    }
  }
}
*/

void Genetic::crossover( int i1, int j1, int i2, int j2 ) {
  cout <<"Crossover: " <<i1 <<"," <<j1 <<" " <<i2 <<"," <<j2 <<endl;

  Genome *pg1 = new Genome( population_[i1][j1] );
  Genome *pg2 = new Genome( population_[i2][j2] );
  for ( int i = 0; i < population_size_; i++ ) {
    for ( int j = 0; j < 3; j++ ) {
      int c1, c2;
      int a, b;
      c1 = 0;
      c2 = 0;
      pg1->count( c1 );
      pg2->count( c2 );
      a = ( rand()%( c1 ) );
      b = ( rand()%( c2 ) );
      if ( a == 0 ) a++;
      if ( b == 0 ) b++;
      if ( a == c1 ) a--;
      if ( b == c2 ) b--;
      
      Genome *g2 = new Genome( pg2->retrieve( b ) );
	  
      delete population_[i][j];
      population_[i][j] = new Genome( pg1 );
	  
      population_[i][j]->replace_delete( g2, a );
      if ( (g2 = population_[i][j]->mutate()) ) {
	delete population_[i][j];
	population_[i][j] = g2;
      }
    }
  }
}

float Genetic::interpret( int i, int j, int i_index, int j_index ) {
  return population_[i][j]->interpret( i_index, j_index );
}

/*
void Genetic::evaluate2() {
  int max1i, max2i;
  int max1 = 0, max2 = 0;
  int tf;
  int c;
  int ***fitness = new int**[population_size_];
  for ( int i = 0; i < population_size_; i++ ) {
    int j;
    fitness[i] = new int*[3];
    for (  j = 0; j < 3; j++ ) {
      fitness[i][j] = new int[GA_NUMBER_OF_VARIABLES];
      int k;
      for ( k = 0; k < GA_NUMBER_OF_VARIABLES; k++ ) {
	c = 0;
	population_[i][j]->count( GA_VARIABLE_LIST[k], c );
	if ( c > 0 )
	  fitness[i][j][k] = 1;
      }
    }
    tf = 0;
    for ( j = 0; j < 3; j++ ) {
      for ( int k = 0; k < GA_NUMBER_OF_VARIABLES; k++ ) {
	tf += fitness[i][j][k];
      }
    }
    if ( tf > max1 ) {
      max2 = max1;
      max1 = tf;
      max2i = max1i;
      max1i = i;
    }
    else if ( tf > max2 ) {
      max2 = tf;
      max2i = i;
    }
  }
  crossover( max2i, max1i );
}
*/

void Genetic::evaluate2() {
  int max1i, max2i;
  int max1 = 0, max2 = 0;
  int tf;
  int c;
  int ***fitness = new int**[population_size_];

  int *rg = new int[ GA_NUMBER_OF_VARIABLES ];
  int *rb = new int[ GA_NUMBER_OF_VARIABLES ];
  int *gb = new int[ GA_NUMBER_OF_VARIABLES ];
  
  int i;
  for ( i = 0; i < population_size_; i++ ) {
    int j;
    fitness[i] = new int*[3];
    for (  j = 0; j < 3; j++ ) {
      fitness[i][j] = new int[GA_NUMBER_OF_VARIABLES];
      int k;
      for ( k = 0; k < GA_NUMBER_OF_VARIABLES; k++ ) {
	c = 0;
	population_[i][j]->count( GA_VARIABLE_LIST[k], c );
	if ( c > 0 )
	  fitness[i][j][k] = 1;
      }
    }
    tf = 0;
    for ( int k = 0; k < GA_NUMBER_OF_VARIABLES; k++ ) {
      rg[k] = fitness[i][0][k] ^ fitness[i][1][k];
      rb[k] = fitness[i][0][k] ^ fitness[i][2][k];
      rg[k] = fitness[i][1][k] ^ fitness[i][2][k];
      if ( rg[k] )
	tf += 1;
      if ( rb[k] )
	tf += 1;
      if ( gb[k] )
	tf += 1;
    }
    if ( tf > max1 ) {
      max2 = max1;
      max1 = tf;
      max2i = max1i;
      max1i = i;
    }
    else if ( tf > max2 ) {
      max2 = tf;
      max2i = i;
    }
  }
  crossover( max2i, max1i );

  delete [] rg;
  delete [] rb;
  delete [] gb;
  for ( i = 0; i < population_size_; i++ ) {
    for ( int j = 0; j < 3; j++ ) {
      delete [] fitness[i][j];
    }
    delete [] fitness[i];
  }  
  delete [] fitness;
}


void Genetic::evaluate() {
  int max1i, max1j, max2i, max2j;
  int max1 = 0, max2 = 0;
  int tf;
  int c;
  int ***fitness = new int**[population_size_];
  for ( int i = 0; i < population_size_; i++ ) {
    fitness[i] = new int*[3];
    for ( int j = 0; j < 3; j++ ) {
      fitness[i][j] = new int[GA_NUMBER_OF_VARIABLES];
      int k;
      for ( k = 0; k < GA_NUMBER_OF_VARIABLES; k++ ) {
	c = 0;
	population_[i][j]->count( GA_VARIABLE_LIST[k], c );
	if ( c > 0 )
	  fitness[i][j][k] = 1;
      }
      tf = 0;
      for ( k = 0; k < GA_NUMBER_OF_VARIABLES; k++ ) {
	tf += fitness[i][j][k];
      }
      if ( tf > max1 ) {
	max2 = max1;
	max1 = tf;
	max2i = max1i;
	max2j = max1j;
	max1i = i;
	max1j = j;
      }
      else if ( tf > max2 ) {
	max2 = tf;
	max2i = i;
	max2j = j;
      }
    }
  }
  crossover( max2i, max2j, max1i, max1j );
}

void Genetic::print( int i ) {
  cout <<endl <<"Genome[" <<i <<"]" <<endl;
  for ( int j = 0; j < 3; j++ ) {
    if ( j == 0 )
      cout <<"Red:   ";
    else if ( j == 1 )
      cout <<"Green: ";
    else if ( j == 2 )
      cout <<"Blue:  ";
    population_[i][j]->print();
    cout <<endl;
  }
}

void Genetic::replace( int i ) {
  for ( int j = 0; j < 3; j++ ) {
    delete population_[i][j];
    population_[i][j] = new Genome( rand()%5, 0 );
  }  
}

