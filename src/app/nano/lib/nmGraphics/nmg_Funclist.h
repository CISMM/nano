#ifndef NMG_FUNCLIST_H
#define NMG_FUNCLIST_H

#include "nmg_Types.h"

#include <stdlib.h>  // for NULL

struct nmg_Funclist {

  nmg_Funclist (int id, object_ptr function, void * userdata,
                nmg_Funclist * next, const char * name = NULL);
  ~nmg_Funclist (void);

  int id;
  object_ptr function;
  void * data;
  nmg_Funclist * next;
  const char * name;
};

int addFunctionToFunclist (nmg_Funclist ** flist,
                           object_ptr function, void * userdata,
                           const char * name = NULL);
int removeFunctionFromFunclist (nmg_Funclist ** flist, int id);
int changeDataInFunclist (nmg_Funclist * flist, int id, void * userdata);



#endif // NMG_FUNCLIST_H

