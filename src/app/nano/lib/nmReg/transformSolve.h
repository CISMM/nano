
// Given a Correspondence and two image indices to select images
// in the correspondence, this function computes the 4x4 transformation matrix
// that maps points in im0 to points in im1 with minimal error in the 
// specified corresondence.

#include "correspondence.h"
#include "nmb_Types.h"       // for vrpn_bool

int transformSolver(double *xform_matrix, double *error,
	Correspondence &c, int im0, int im1,
	vrpn_bool rotation3D = VRPN_FALSE);

typedef enum {ZERO_ROTATION_ZERO_SHEAR_UNIFORM_SCALE_2D2D,
	ZERO_SHEAR_UNIFORM_SCALE_2D2D, GENERAL_2D2D,
	ZERO_ROTATION_2D2D, GENERAL_3D2D} xform_solution_t;

int transformSolver(double *xform_matrix, double *error,
	Correspondence &c, int im0, int im1, 
	double r_x, double r_y, double r_z, double angle, 
	xform_solution_t type = GENERAL_2D2D,
	double shear_x = 0.0, double shear_y = 0.0,
	double scale_ratio_x_to_y = 1.0);

// This function takes the output from transformSolve and gives you a
// direction of projection if the input matrix is an orthogonal projection
// which is the case for SOLVE_3D_TO_2D or SOLVE_2D_TO_2D
int computeProjectionVector(double *xform_matrix, double *vec);
