/***************************************************************************
				filter.h

	This header file describes the functions that are needed to derive
 a new data set from an old one by passing it through one of the unix filter
 programs (scripts) found in ~stm/etc/filters.  It creates a new plane in the
 inputGrid if and only if the filter succeeds.
 ***************************************************************************/

//#include "BCGrid.h"
//#include "BCPlane.h"

class BCPlane;  // from BCPlane.h
class BCGrid;  // from BCGrid.h
//class nmb_Dataset;  // from nmb_Dataset.h

extern	int	filter_plane(
			const char *filtername,	// Full path name
			BCPlane *oldset,	// Plane to filter
			const char *newset_name,	// Where to put it
			double scale,		// Scale of the filter
			double angle,		// Angle of filter
			const char *otherargs,	// Other arguments (or "")
                        BCGrid * grid);		// Grid to put it in

//#define	defaultFilterDir	"/afs/unc/proj/stm/etc/filters/"

