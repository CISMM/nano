#ifndef NMB_SELECTOR_H
#define NMB_SELECTOR_H

#include <stdlib.h>  // for NULL
#include <stdio.h>  // for NULL

// Originally from Tcl_Linkvar
//
// Abstracted here so we could remove Base's dependance
// on Tcl.

typedef void (nmb_SELECTOR_CALLBACK) (const char * newValue, void * userdata);

#define nmb_STRING_LENGTH 128

class nmb_ListOfStrings {

  friend class nmb_Selector;
    // Expose addSelector only to Selector so that we can
    // guarantee consistency of our circular pointers.
    // I'm sure there's a better way in one of the books.

  public:

    nmb_ListOfStrings (void);
    virtual ~nmb_ListOfStrings (void);

    int numEntries (void) const { return d_numEntries; }
    const char * entry (int i) const { return d_entries[i]; }

    // Return nonzero on failure.  Do consistency, legality checks.
    // Actual work is performed by really{Add,Delete}Entry,
    // which are virtual and may be overridden by derived classes.
    int addEntry (const char *);
    int deleteEntry (const char *);


    enum { NUM_ENTRIES = 100 };

  protected:

    // Return nonzero on failure.
    int addSelector (nmb_Selector *);
    int deleteSelector (nmb_Selector *);

    int d_numSelectors;
    nmb_Selector * d_selector [NUM_ENTRIES];
      // Which selectors use this List?

    int d_numEntries;
    char d_entries [NUM_ENTRIES][nmb_STRING_LENGTH + 1];


};

class nmb_Selector {

  friend class nmb_ListOfStrings;

  public:

    nmb_Selector (nmb_ListOfStrings * = NULL, const char * initialValue = "");
    virtual ~nmb_Selector (void);

    // ACCESSORS

    operator const char * (void) const;
    const char * string (void) const;

    // MANIPULATORS

    virtual int bindList (nmb_ListOfStrings *);

    virtual const char * operator = (const char *);
    virtual const char * operator = (char *);
      // aargh!  SGI compiler doesn't treat the (const char *) version
      // right;  we end up invoking the plainest Tclvar_selector
      // constructor.  Maybe we ought to get rid of it...

    virtual void Set (const char *);

  protected:

    virtual int addEntry (const char *);
    virtual int deleteEntry (const char *);

    char d_myString [nmb_STRING_LENGTH + 1];
    char d_myLastString [nmb_STRING_LENGTH + 1];

    nmb_ListOfStrings * d_myList;

};

nmb_Selector * allocate_nmb_Selector (const char * initialValue);


#endif  // NMB_SELECTOR_H

