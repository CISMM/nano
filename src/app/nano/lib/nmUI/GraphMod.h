#ifndef GRAPH_MOD_H
#define GRAPH_MOD_H

// for Point_results, Point_value
#include <Point.h>
// for Scanline_results
#include <Scanline.h>

#define MAX_SCANLINE_GRAPH_PTS (300)	// don't draw too many points (50 is
					// a good number - set to 300 for
					// testing)
#define MAX_SCANLINES (3)	// it would be better to have this
		// value controlled by the user - but since the user
		// can enable or disable each of the scanlines individually
		// the user can acheive the same effect less elegantly

class Tcl_Interp;  // from <tcl.h>

class GraphMod {

  public:

    GraphMod (void);
    ~GraphMod (void);

    static int EnterModifyMode (void *);
    static int EnterImageMode (void *);
    static int EnterScanlineMode (void *);
      // Should be called every time we enter the appropriate mode
      // (See MicroscopeRcv.C)
    static int ReceiveNewPoint (void *, const Point_results *);
      // Should be called with every modification result received
      // from the microscope.

    static int ReceiveNewScanline(void *, const Scanline_results *);

  private:
    int d_currentmode;
    int d_lastmode;
    Tcl_Interp * d_interp;
    int d_first_point;
    void ShowStripchart (const char * tcl_script_dir);

    // stuff for scanline display:
    int d_num_scanlines;
    int d_numscanline_channels;
    int d_scanlength;
    Scanline_results *d_scanlines[MAX_SCANLINES];
};

#endif  // GRAPH_MOD_H

