#ifndef _gaEngine_Implementation_h_
#define _gaEngine_Implementation_h_

#include "gaEngine.h"
#include "genetic.h"

class gaEngine_Implementation : public gaEngine {
  
public:
  
  gaEngine_Implementation (vrpn_Connection *);
  virtual ~gaEngine_Implementation (void);
  
  virtual void mainloop (void);
  
  virtual void texture ( int, int );
  virtual void dimensions (int, int, int);
  virtual void dataSet (int, int, float **);
  virtual void number_of_variables (int);
  virtual void variableList (int, char **);
  virtual void selectGenomes (int, int);
  virtual void reevaluateDataset (void);
  
  void initialize_genes();
  void evaluate();
  void evaluate2();
  void random();

protected:
  
  static int handle_number_of_variables (void *, vrpn_HANDLERPARAM);
  static int handle_variableList (void *, vrpn_HANDLERPARAM);
  static int handle_dataSet (void *, vrpn_HANDLERPARAM);
  static int handle_dimensions (void *, vrpn_HANDLERPARAM);
  static int handle_selectGenomes (void *, vrpn_HANDLERPARAM);
  static int handle_reevaluateDataset (void *, vrpn_HANDLERPARAM);
  
private:
  Genetic *genes;

};

#endif
