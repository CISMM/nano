/*===3rdtech===
  Copyright (c) 2000 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#ifndef MOD_FILE_H
#define MOD_FILE_H

#include <Point.h>


struct Tcl_Interp;  // from <tcl.h>
/** Tom Hudson

 This is a type of logfile other than the streamfile:
 it logs a set of modifications made to the sample.
 Low-level IO is actually carried out by functions on
 Point_list (Point.h)
*/
class ModFile {

  public:

    ModFile (void);
    ~ModFile (void);

    static int EnterModifyMode (void *);
    static int EnterImageMode (void *);
    ///< Should be called every time we enter the appropriate mode

    static int ReceiveNewPoint (void *, const Point_results *);
      ///< Should be called with every modification result received
      ///< from the microscope.

  private:

    int d_lastmode;
    long d_start_mod_time;
    Tcl_Interp * d_interp;
    Point_list d_pointlist;

    void RememberPointList (void);

    void ShowModFile (void);
};

#endif  // MOD_FILE_H

