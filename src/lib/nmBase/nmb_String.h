#ifndef NMB_STRING_H
#define NMB_STRING_H

#include <stdlib.h>  // for NULL

typedef void (nmb_STRING_CALLBACK) (const char * newValue, void * userdata);

#define nmb_STRING_LENGTH 128

/**
Originally from Tcl_Linkvar
Abstracted here so we could remove Base's dependance
on Tcl.
  nmb_String and nmb_ListOfStrings objects contain information
which will be changed in the user interface. By putting them
in an object with virtual accesssor functions, we can write
classes which inherit, which change the user interface and 
respond to changes in the user interface.
Look at Tcl_Linkvar.[Ch] to see how this is done.
*/
class nmb_ListOfStrings {

  friend class nmb_String;
    // Expose addString only to String so that we can
    // guarantee consistency of our circular pointers.
    // I'm sure there's a better way in one of the books.

  public:

    nmb_ListOfStrings (void);
    virtual ~nmb_ListOfStrings (void);

    int numEntries (void) const { return d_numEntries; }
    const char * entry (int i) const { return d_entries[i]; }

    // Return nonzero on failure.  Do consistency, legality checks.
    virtual int clearList ();
    virtual int addEntry (const char *);
    virtual int deleteEntry (const char *);

    virtual int copyList(nmb_ListOfStrings * newList);

    enum { NUM_ENTRIES = 100 };

  protected:

    int d_numEntries;
    char *d_entries [NUM_ENTRIES];


};

nmb_ListOfStrings * allocate_nmb_ListOfStrings();

class nmb_String {

  public:

    nmb_String (const char * initialValue = "");
    virtual ~nmb_String (void);

    // ACCESSORS

    operator const char * (void) const;
    const char * string (void) const;

    const char * lastString (void) const;

    // MANIPULATORS

    virtual const char * operator = (const char *);
    virtual const char * operator = (char *);
      // aargh!  SGI compiler doesn't treat the (const char *) version
      // right;  we end up invoking the plainest Tclvar_string
      // constructor.  Maybe we ought to get rid of it...

    virtual void Set (const char *);

  protected:

    char d_myString [nmb_STRING_LENGTH + 1];
    char d_myLastString [nmb_STRING_LENGTH + 1];

};

nmb_String * allocate_nmb_String (const char * initialValue);


#endif  // NMB_STRING_H

