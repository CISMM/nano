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
      // The first id should be set to 1.
      // The problem occurred when items were being deleted from
      // the list which were never added. Since the items were never
      // added to the list the variable used to reference the item
      // (for example trueTip_id in globjects) had a default value of
      // zero this default value was then used to access and delete
      // some other item which had been added to the list and assigned
      // the id zero. This could be solved by a) never deleting items
      // which weren't added or b) setting default values in globjects.c
    head = new nmg_Funclist (1, func, data, NULL, name);
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
//      if (!prev)
//        fprintf(stderr, "removeFunctionFromFunclist:  Already empty!\n");
//      else
//        fprintf(stderr, "removeFunctionFromFunclist:  "
//                        "No function in list with id %d.\n", id);
    return -1;
  }

  if ((head == prev) && (id == head->id)) {
    delete head;
    *list = NULL;
    return 0;
  }
  if ( id == head->id ) {
      prev->next = head->next;
      delete head;
      return 0;
  }
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




