#ifndef _gaEngine_Remote_h_
#define _gaEngine_Remote_h_

#include "gaEngine.h"

class gaEngine_Remote : public gaEngine {
  
public:
  
  gaEngine_Remote (vrpn_Connection *);
  virtual ~gaEngine_Remote (void);
  
  virtual void mainloop (void);
  
  virtual void texture ( int, int );
  virtual void dimensions (int, int, int);
  virtual void dataSet (int, int, float **);
  virtual void number_of_variables (int);
  virtual void variableList (int, char **);
  virtual void selectGenomes (int, int);
  virtual void reevaluateDataset (void);

protected:
  
  static int handle_connectionComplete (void *, vrpn_HANDLERPARAM);
  static int handle_texture (void *, vrpn_HANDLERPARAM );
};

#endif
