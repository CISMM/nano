/* The nanoManipulator and its source code have been released under the
 * Boost software license when nanoManipulator, Inc. ceased operations on
 * January 1, 2014.  At this point, the message below from 3rdTech (who
 * sublicensed from nanoManipulator, Inc.) was superceded.
 * Since that time, the code can be used according to the following
 * license.  Support for this system is now through the NIH/NIBIB
 * National Research Resource at cismm.org.

Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

/*===3rdtech===
  Copyright (c) 2000 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#ifndef NMUI_CROSS_SECTION_H
#define NMUI_CROSS_SECTION_H

// for TclNet_int
#include <Tcl_Netvar.h>
#include "nmui_Component.h"

#define MAX_DATA_VECTORS (32)

struct Tcl_Interp;  // from <tcl.h>
class BCGrid;
class nmb_ListOfStrings;

class nmui_CrossSection {

  public:

    nmui_CrossSection (void);
    ~nmui_CrossSection (void);

    int ShowCrossSection(BCGrid* grid, 
                         nmb_ListOfStrings * plane_names,
                         int id, int hide,  
                         float center_x, float center_y, 
                         float width1, float width2,
                         float angle);
    /**< Changes display of cross section, given by id */

    static void handle_MaxPointsChange(vrpn_int32, void *);
    /**< Should be called when user changes max points control */
    static void handle_ClearZero(vrpn_int32, void *);
    static void handle_ClearOne(vrpn_int32, void *);

    void SetupSynchronization(nmui_Component * container);
    /**< Sets up synchronization on the TclNets */
    void TeardownSynchronization(nmui_Component * container);
    /**< Tears down synchronization on the TclNets */

    TclNet_int d_snap_to_45; 
    TclNet_int d_vary_width; 
    int d_hide[2];
    int d_first_call[2];

    //private:
    TclNet_int d_max_points; 
      /**< maximum number of points to graph per channel. */
    TclNet_int d_clear_zero; ///< clear cross section graph 
    TclNet_int d_clear_one; ///< clear cross section graph 
    
	TclNet_int d_data_update; ///< trigger data update. 

    int d_length; ///< number of data points per xs. 

    double *data[2][nmb_ListOfStrings::NUM_ENTRIES];
    double *path[2];
    // data vector names
    int d_numDataVectors;
    char d_dataVectorNames[MAX_DATA_VECTORS][128];

};

#endif  // GRAPH_MOD_H

