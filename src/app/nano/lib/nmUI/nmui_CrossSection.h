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
                                int id, int enable,  
                                float center_x,float center_y, float width, 
                                float angle);
    /**< Changes display of cross section, given by id */

    static void handle_MaxPointsChange(vrpn_int32, void *);
    /**< Should be called when user changes max points control */
    static void handle_StrideChange(vrpn_int32, void *);
    /**< Should be called when user changes stride control */

    void SetupSynchronization(nmui_Component * container);
    /**< Sets up synchronization on the TclNets */
    void TeardownSynchronization(nmui_Component * container);
    /**< Tears down synchronization on the TclNets */

    private:
    TclNet_int d_max_points; 
      /**< maximum number of points to graph per channel. */
    TclNet_int d_stride; 
      /**< Only graph every d_stride points. */
    
    double data[nmb_ListOfStrings::NUM_ENTRIES][300];
    double path[300];
    // data vector names
    int d_numDataVectors;
    char d_dataVectorNames[MAX_DATA_VECTORS][128];
    int d_first_call;

};

#endif  // GRAPH_MOD_H

