#ifndef MOD_FILE_H
#define MOD_FILE_H

#include <Point.h>


// ModFile
//
// Tom Hudson

// This is a type of logfile other than the streamfile:
// it logs a set of modifications made to the sample.
// Low-level IO is actually carried out by functions on
// Point_list (Point.h)

class Tcl_Interp;  // from <tcl.h>

class ModFile {

  public:

    ModFile (void);
    ~ModFile (void);

    static int EnterModifyMode (void *);
    static int EnterImageMode (void *);
      // Should be called every time we enter the appropriate mode

    static int ReceiveNewPoint (void *, const Point_results *);
      // Should be called with every modification result received
      // from the microscope.

  private:

    int d_lastmode;
    Tcl_Interp * d_interp;
    Point_list d_pointlist;

    void ShowModFile (void);
};

#endif  // MOD_FILE_H

