#ifndef VRML_H
#define VRML_H

int write_to_vrml_file (const char * filename, nmb_PlaneSelection planes,
                        GLdouble minColor [3], GLdouble maxColor [3]);

/**	This routine finds the normal to the surface at the given grid
 * point.  It does this by averaging the normals with the four 4-connected
 * points in the grid (one away in x or y.)
 *	This routine takes into account the current stride in x and y
 * between tesselated points and grid points.
 *
 * x and y are the indices of the point.  dx and dy are the distances between
 * points in the X and Y directions.  I think dz is the Z scale.
 *
 *	This routine returns 0 on success and -1 on failure.
 */
int vrml_compute_plane_normal(const BCPlane *plane, int x,int y,
                              double dx,double dy,double dz, GLfloat Normal[3]);

#endif  // VRML_H
