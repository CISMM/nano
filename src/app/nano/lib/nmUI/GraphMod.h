/*===3rdtech===
  Copyright (c) 2000 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#ifndef GRAPH_MOD_H
#define GRAPH_MOD_H

// for Point_results, Point_value
#include <Point.h>
// for Scanline_results
#include <Scanline.h>
// for TclNet_int
#include <Tcl_Netvar.h>
#include "nmui_Component.h"

#define MAX_SCANLINE_GRAPH_PTS (300)	// don't draw too many points (50 is
					// a good number - set to 300 for
					// testing)
#define MAX_SCANLINES (3)	// it would be better to have this
		// value controlled by the user - but since the user
		// can enable or disable each of the scanlines individually
		// the user can acheive the same effect less elegantly

#define MAX_DATA_VECTORS (32)

struct Tcl_Interp;  // from <tcl.h>

class GraphMod {

  public:

    GraphMod (void);
    ~GraphMod (void);

    static int EnterModifyMode (void *);
    /**< Tells the stripchart to start graphing points whenever
     * it receives them. */
    static int EnterImageMode (void *);
    /**< Tells the stripchart to stop graphing points. It can still
     * receive points if the user is feeling the surface (I think). */
    static int EnterScanlineMode (void *);
    /**< Should be called every time we enter the appropriate mode
       (See MicroscopeRcv.C) */
    static int ReceiveNewPoint (void *, const Point_results *);
    /**< Should be called with every modification result received
       from the microscope. */

    static int ReceiveNewScanline(void *, const Scanline_results *);
    /**< Should be called with every scanline result received
       from the microscope. */

    static void handle_MaxPointsChange(vrpn_int32, void *);
    /**< Should be called when user changes max points control */
    static void handle_StrideChange(vrpn_int32, void *);
    /**< Should be called when user changes stride control */

    void gm_SetupSynchronization(nmui_Component * container);
    /**< Sets up synchronization on the TclNets */
    void gm_TeardownSynchronization(nmui_Component * container);
    /**< Tears down synchronization on the TclNets */

    private:
    int d_currentmode;
    int d_lastmode;
    // This is a bad idea - should get from a globally maintained var. 
    Tcl_Interp * d_interp;
    
    int d_total_num_points; /**< Total number of points received. */
    int d_num_points_graphed; 
      /**< Number of points actually graphed. Some can be skipped if
	 stride is > 1. This count does not get decremented if d_max_points
	 is exceeded and some points are removed from the graph.
      */
    TclNet_int d_max_points; 
      /**< maximum number of points to graph per channel. */
    TclNet_int d_stride; 
      /**< Only graph every d_stride points. */
    
    void ShowStripchart (const char * tcl_script_dir);
    /**< Pops the stripchart window up. Catches the user's attention. */

    // data vector names
    int d_numDataVectors;
    char d_dataVectorNames[MAX_DATA_VECTORS][128];

    int d_is3D; ///< did the current mod include Z data as well?

    // stuff for scanline display:
    int d_num_scanlines;
    int d_numscanline_channels;
    int d_scanlength;
    Scanline_results *d_scanlines[MAX_SCANLINES];
};

#endif  // GRAPH_MOD_H

