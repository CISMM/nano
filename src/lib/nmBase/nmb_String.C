#include "nmb_String.h"

#include <string.h>  // strcmp(), strncmp(), strncpy()
#include <stdio.h>  // fprintf()

nmb_ListOfStrings::nmb_ListOfStrings (void) :
    d_numEntries (0) {
    for(int i =0; i< NUM_ENTRIES; i++) {
	d_entries[i] = new char[nmb_STRING_LENGTH + 1];
    }
}

// virtual
nmb_ListOfStrings::~nmb_ListOfStrings (void) {

//int i;
//fprintf(stderr, "Deleting list of strings:\n");
//for (i = 0; i < d_numEntries; i++)
//fprintf(stderr, "    %s\n", d_entries[i]);

  while (d_numEntries && !deleteEntry(d_entries[0])) ;
  for(int i =0; i< NUM_ENTRIES; i++) {
      delete [] d_entries[i];
  }
  
}
/** Returns an entry in this list, by numerical index.
Returns NULL if index is not valid for this list.
*/
const char * nmb_ListOfStrings::entry (int i) const
{ 
    if ((i <0) || (i>=d_numEntries)) return NULL;
    return d_entries[i]; 
}

/** Checks to see if the character string /a name is 
in this list. If it is, it returns it's numerical index.
If not, returns -1
*/
int nmb_ListOfStrings::getIndex (const char * name) const
{ 
    for (int i = 0; i < d_numEntries; i++) {
        if (strncmp(name, d_entries[i], nmb_STRING_LENGTH) == 0) {
            return i;
        }
    }
    return -1;
}

int nmb_ListOfStrings::clearList () 
{
    d_numEntries = 0;
    return 0;
}

int nmb_ListOfStrings::addEntry (const char * newEntry) 
{
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

  strncpy(d_entries[d_numEntries], newEntry, nmb_STRING_LENGTH);
  d_numEntries++;

  return 0;
}

int nmb_ListOfStrings::deleteEntry (const char * oldEntry) {
  int i;
  int ret = 0;

  // Find it.
  for (i = 0; (i < d_numEntries) &&
              strncmp(d_entries[i], oldEntry, nmb_STRING_LENGTH); i++) ;
  if (i == d_numEntries) {
    return 1;
  }

  strncpy(d_entries[i], d_entries[d_numEntries - 1],
          nmb_STRING_LENGTH);
  d_numEntries--;

  return ret;
}

int nmb_ListOfStrings::copyList(nmb_ListOfStrings * newList)
{
  int i;
  for (i = 0; i < newList->d_numEntries; i++) {
      strncpy(d_entries[i], newList->d_entries[i], 
	      nmb_STRING_LENGTH);
  }
  d_numEntries = newList->d_numEntries;
  return 0;

}

nmb_ListOfStrings * allocate_nmb_ListOfStrings() {
  return new nmb_ListOfStrings ();
}


nmb_String::nmb_String (const char * initialValue) 
{
  Set(initialValue);

}

// virtual
nmb_String::~nmb_String (void) {

}

nmb_String::operator const char * (void) const {
  return d_myString;
}

const char * nmb_String::string (void) const {
  return d_myString;
}

const char * nmb_String::lastString (void) const {
  return d_myLastString;
}

//void nmb_String::setCallback

/*
// virtual
int nmb_String::bindList (nmb_ListOfStrings * list) {

  // Can be used to set NULL => non-NULL
  //          or to set non-NULL => NULL,
  //            but not non-NULL => non-NULL.

  if (list && d_myList) {
    fprintf(stderr, "nmb_String::bindList:  Called twice.\n");
    return -1;
  }

  d_myList = list;

  if (list)
    list->addString(this);

  return 0;
}
*/

// virtual
const char * nmb_String::operator = (const char * v) {
  if (!v) {
    d_myString[0] = 0;
  } else {
    strncpy(d_myString, v, nmb_STRING_LENGTH);
  }

  return d_myString;
}
  

// Same implementation, but different signature...

// virtual
const char * nmb_String::operator = (char * v) {
  if (!v) {
    d_myString[0] = 0;
  } else {
    strncpy(d_myString, v, nmb_STRING_LENGTH);
  }

  return d_myString;
}
  

// virtual
void nmb_String::Set (const char * value) {
  // Need to invoke operator = in case it's been redefined
  // by derived class.
  operator = (value);
  strncpy(d_myLastString, value, nmb_STRING_LENGTH);
}


nmb_String * allocate_nmb_String (const char * iv) {
  return new nmb_String (iv);
}

