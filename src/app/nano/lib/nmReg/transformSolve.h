#include "correspondence.h"
#include "nmb_Types.h"       // for vrpn_bool
#include "nmr_Registration_Interface.h"

// this tells you how many points you need in the Correspondence which
// gets passed to transformSolver in order to solve for a particular type
// of transformation
int numPointConstraintsToSpecify(nmr_TransformationType type);

// Given a Correspondence and two image indices to select images
// in the correspondence, this function computes the 4x4 transformation matrix
// that maps points in im0 to points in im1 with minimal error in the
// specified corresondence.
int transformSolver(double *xform_matrix, double *error,
        Correspondence &c, int im0, int im1, nmr_TransformationType type);
