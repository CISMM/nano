#include "nmb_Selector.h"

#include <string.h>  // strcmp(), strncmp(), strncpy()
#include <stdio.h>  // fprintf()

nmb_ListOfStrings::nmb_ListOfStrings (void) :
    d_numSelectors (0),
    d_numEntries (0) {

}

// virtual
nmb_ListOfStrings::~nmb_ListOfStrings (void) {

//int i;
//fprintf(stderr, "Deleting list of strings:\n");
//for (i = 0; i < d_numEntries; i++)
//fprintf(stderr, "    %s\n", d_entries[i]);

  while (d_numEntries && !deleteEntry(d_entries[0])) ;

  while (d_numSelectors)
    deleteSelector(d_selector[0]);
}

int nmb_ListOfStrings::addEntry (const char * newEntry) {
  int i;

  // Make sure there is enough room for another entry.
  if (d_numEntries >= NUM_ENTRIES) {
    return 1;
  }

  // Make sure the entry is not already on the list
  for (i = 0; i < d_numEntries; i++) {
    if (!strncmp(d_entries[i], newEntry, nmb_STRING_LENGTH)) {
      return 2;
    }
  }

  for (i = 0; i < d_numSelectors; i++) {
    // TODO:  CHECK RETURN VALUES
    d_selector[i]->addEntry(newEntry);
  }

  strncpy(d_entries[d_numEntries], newEntry, nmb_STRING_LENGTH);
  d_numEntries++;

  return 0;
}

int nmb_ListOfStrings::deleteEntry (const char * oldEntry) {
  int i, j;
  int ret = 0;

  // Find it.
  for (i = 0; (i < d_numEntries) &&
              strncmp(d_entries[i], oldEntry, nmb_STRING_LENGTH); i++) ;
  if (i == d_numEntries) {
    return 1;
  }

  for (j = 0; j < d_numSelectors; j++) {
    // TODO:  CHECK RETURN VALUES
    d_selector[j]->deleteEntry(oldEntry);
  }

  strncpy(d_entries[i], d_entries[d_numEntries - 1],
          nmb_STRING_LENGTH);
  d_numEntries--;

  return ret;
}


int nmb_ListOfStrings::addSelector (nmb_Selector * newSelector) {
  int i;

  if (d_numSelectors >= NUM_ENTRIES) {
    return 1;
  }
  for (i = 0; i < d_numSelectors; i++) {
    if (d_selector[i] == newSelector) {
      return 2;
    }
  }

  d_selector[d_numSelectors] = newSelector;
  d_numSelectors++;

  return 0;
}

int nmb_ListOfStrings::deleteSelector (nmb_Selector * oldSelector) {
  int i;

  if (!oldSelector) return 1;

  for (i = 0; (i < d_numSelectors) && (d_selector[i] != oldSelector); i++) ;
  if (i == d_numSelectors) {
    return 1;
  }

  d_selector[i] = d_selector[d_numSelectors - 1];
  d_numSelectors--;

  oldSelector->bindList(NULL);

  return 0;
}



nmb_Selector::nmb_Selector (nmb_ListOfStrings * list,
                            const char * initialValue) :
  d_myList (NULL)
{
  Set(initialValue);

  if (list) {
    bindList(list);
  }
}

// virtual
nmb_Selector::~nmb_Selector (void) {
  if (d_myList)
    d_myList->deleteSelector(this);

}

nmb_Selector::operator const char * (void) const {
  return d_myString;
}

const char * nmb_Selector::string (void) const {
  return d_myString;
}

const char * nmb_Selector::lastString (void) const {
  return d_myLastString;
}

//void nmb_Selector::setCallback

// virtual
int nmb_Selector::bindList (nmb_ListOfStrings * list) {

  // Can be used to set NULL => non-NULL
  //          or to set non-NULL => NULL,
  //            but not non-NULL => non-NULL.

  if (list && d_myList) {
    fprintf(stderr, "nmb_Selector::bindList:  Called twice.\n");
    return -1;
  }

  d_myList = list;

  if (list)
    list->addSelector(this);

  return 0;
}


// virtual
const char * nmb_Selector::operator = (const char * v) {
  if (!v) {
    d_myString[0] = 0;
  } else {
    strncpy(d_myString, v, nmb_STRING_LENGTH);
  }

  return d_myString;
}
  

// Same implementation, but different signature...

// virtual
const char * nmb_Selector::operator = (char * v) {
  if (!v) {
    d_myString[0] = 0;
  } else {
    strncpy(d_myString, v, nmb_STRING_LENGTH);
  }

  return d_myString;
}
  

// virtual
void nmb_Selector::Set (const char * value) {
  // Need to invoke operator = in case it's been redefined
  // by derived class.
  operator = (value);
  if (value) {
    strncpy(d_myLastString, value, nmb_STRING_LENGTH);
  } else {
    d_myLastString[0] = 0;
  }
}


// virtual
int nmb_Selector::addEntry (const char *) {
  // do nothing
  return 0;
}


// virtual
int nmb_Selector::deleteEntry (const char *) {
  // do nothing
  return 0;
}



nmb_Selector * allocate_nmb_Selector (const char * iv) {
  return new nmb_Selector (NULL, iv);
}

