#include "nmg_Funclist.h"

#include <stdlib.h>  // for NULL
#include <stdio.h>


nmg_Funclist::nmg_Funclist (int _id, object_ptr _function, void * _data,
                            nmg_Funclist * _next,
                            const char * _name) :
  id (_id),
  function (_function),
  data (_data),
  next (_next),
  name (_name) {

}

nmg_Funclist::~nmg_Funclist (void) {

}

int addFunctionToFunclist (nmg_Funclist ** list,
                           object_ptr func, void * data,
                           const char * name) {
  nmg_Funclist * head,
               * prev;

  head = *list;
  if (!head) {
    head = new nmg_Funclist (0, func, data, NULL, name);
    *list = head;
    return head->id;
  }

  prev = head;
  while (head) {
    prev = head;
    head = head->next;
  }

  head = new nmg_Funclist (prev->id + 1, func, data, NULL, name);
  prev->next = head;

  return head->id;
}

int removeFunctionFromFunclist (nmg_Funclist ** list, int id) {
  nmg_Funclist * head,
               * prev;

  head = *list;
  prev = head;
  while (head && (id != head->id)) {
    prev = head;
    head = head->next;
  }

  // list empty or off right
  if (!head) {
    if (!prev)
      fprintf(stderr, "removeFunctionFromFunclist:  Already empty!\n");
    else
      fprintf(stderr, "removeFunctionFromFunclist:  "
                      "No function in list with id %d.\n", id);
    return -1;
  }

  if (head == prev) {
    delete head;
    *list = NULL;
    return 0;
  }

  prev->next = head->next;
  delete head;
  return 0;
}


int changeDataInFunclist (nmg_Funclist * list, int id, void * newData) {
  nmg_Funclist * head;

  head = list;
  while (head && (id != head->id))
    head = head->next;

  if (!head) {
    fprintf(stderr, "changeDataInFunclist:  "
                    "No function in list with id %d.\n", id);
    return -1;
  }

  head->data = newData;
  return 0;
}




