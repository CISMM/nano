#include "nmg_HeightField.h"
#include "nmg_Plane.h"
#include "PPM.h"

nmg_HeightField::nmg_HeightField(nmb_Image *image, double sideLength,
                               double zScale) 
{
  d_memoryUsed = 0;
  d_numX = image->width();
  d_numY = image->height();
  d_sizeX = sideLength*(d_numX-1);
  d_sizeY = sideLength*(d_numY-1);
  d_verbosity = 0;
  d_deltaX = sideLength;
  d_deltaY = sideLength;
  doBasicInitialization();

  float min, max, value;

  min = image->getValue(0,0);
  max = min;

  int i,j;
  int pointIndex;
  /*
  for (i = 0; i < d_numX; i++) {
    for (j = 0; j < d_numY; j++) {
		value = image->getValue(i,j);
		if (value < min) min = value;
		else if (value > max) max = value; 
    }
  }

  if (max == min) max += 1.0;
  */

  double heightValue;
  for (i = 0; i < d_numX; i++) {
    for (j = 0; j < d_numY; j++) {
      value = image->getValue(i,j);
      heightValue = zScale*value;
	  //zScale*(double)(value-min)/(double)(max-min);
      pointIndex = computeIndex(i, j);
      d_points[pointIndex].z = heightValue;
    }
  }
  rebuildTriangles();
}

nmg_HeightField::nmg_HeightField(int num_x, int num_y, 
                               double sideLength):
  d_numX(num_x), d_numY(num_y), d_sizeX(sideLength*(num_x-1)), 
  d_sizeY(sideLength*(num_y-1)),
  d_verbosity(0)
{
  d_memoryUsed = 0;
  d_deltaX = sideLength;
  d_deltaY = sideLength;
  doBasicInitialization();
  rebuildTriangles();
}
 
nmg_HeightField::nmg_HeightField(nmg_HeightField *surface,
                      int i_min, int j_min, int i_max, int j_max):
  d_numX(i_max-i_min+1), d_numY(j_max-j_min+1), 
  d_sizeX((surface->d_deltaX)*(d_numX-1)),
  d_sizeY((surface->d_deltaY)*(d_numY-1)),
  d_verbosity(0), 
  d_deltaX(surface->d_deltaX), d_deltaY(surface->d_deltaY)
{
  d_memoryUsed = 0;
  doBasicInitialization();
  int i, j;
  for (i = i_min; i <= i_max; i++) {
    for (j = j_min; j <= j_max; j++) {
      set(i-i_min, j-j_min, surface->get(i,j));
    }
  }
  rebuildTriangles();
}

nmg_HeightField::nmg_HeightField(const nmg_HeightField &s)
{
  d_numX = s.d_numX;
  d_numY = s.d_numY;
  d_sizeX = s.d_sizeX;
  d_sizeY = s.d_sizeY;
  d_deltaX = s.d_deltaX;
  d_deltaY = s.d_deltaY;
  d_diagonalSlope = s.d_diagonalSlope;
  d_verbosity = s.d_verbosity;
  doBasicInitialization();
  int i;
  for (i = 0; i < d_numPoints; i++) {
    d_points[i].z = s.d_points[i].z;
  }
  rebuildTriangles();
}

void nmg_HeightField::doBasicInitialization()
{
  d_numPoints = d_numX*d_numY;
  d_numTriangles = 2*(d_numX-1)*(d_numY-1);

  d_centerX = 0;
  d_centerY = 0;

  d_x = new double[d_numX];
  d_y = new double[d_numY];
  d_points = new nmg_Point_3d[d_numPoints];
  d_triangles = new nmg_Triangle[d_numTriangles];
  d_memoryUsed += d_numX*sizeof(double) + d_numY*sizeof(double) +
                  d_numPoints*sizeof(nmg_Point_3d) + 
                  d_numTriangles*sizeof(nmg_Triangle);

  if (!d_x || !d_y || !d_points || 
      !d_triangles) {
    if (d_x) delete [] d_x; d_x = NULL;
    if (d_y) delete [] d_y; d_y = NULL;
    if (d_points) delete [] d_points; d_points = NULL;
    if (d_triangles) delete [] d_triangles; d_triangles = NULL;
    fprintf(stderr, "nmg_HeightField::ctor: Error, out of memory\n");
    exit(-1);
  }
  int i, j;

  double x = -d_centerX;//-0.5*d_sizeX;
  for (i = 0; i < d_numX; i++) {
    d_x[i] = x;
    x += d_deltaX;
  }
  double y = -d_centerY;//-0.5*d_sizeY;
  for (j = 0; j < d_numY; j++) {
    d_y[j] = y;
    y += d_deltaY;
  }

  int pointIndex;
  for (i = 0; i < d_numX; i++) {
    for (j = 0; j < d_numY; j++) {
      pointIndex = computeIndex(i, j);
      d_points[pointIndex].x = d_x[i];
      d_points[pointIndex].y = d_y[j];
      d_points[pointIndex].z = 0.0;
    }
  }

  d_diagonalSlope = d_deltaY/d_deltaX;
  d_maxHeight = 0; d_minHeight = 0;
}

nmg_HeightField::~nmg_HeightField()
{
  delete [] d_x;
  delete [] d_y;
  delete [] d_points;
  delete [] d_triangles;
}

/*

p01       p11
   _______
  |\      |
  |  \  1 |
  | 0  \  |
  |______\|
p00       p10

*/
void nmg_HeightField::rebuildTriangles() 
{
  nmg_Point_3d *p00, *p10, *p01, *p11;
  int triangleIndex = 0; // x+y*(d_numX-1)
  int squareX, squareY; // 2D index through the quads 

  for (squareY = 0; squareY < d_numY-1; squareY++) {
    for (squareX = 0; squareX < d_numX-1; squareX++) {
      p00 = &d_points[computeIndex(squareX,squareY)];
      p10 = &d_points[computeIndex(squareX+1,squareY)];
      p01 = &d_points[computeIndex(squareX,squareY+1)];
      p11 = &d_points[computeIndex(squareX+1, squareY+1)];
      // lower left triangle (p00, p10, p01)
      d_triangles[triangleIndex] = nmg_Triangle(p00, p10, p01);
      triangleIndex++;
      // upper right triangle (p10, p11, p01)
      d_triangles[triangleIndex] = nmg_Triangle(p10, p11, p01);
      triangleIndex++;
    }
  }
  int i;
  d_maxHeight = d_points[0].z;
  d_minHeight = d_maxHeight;
  for (i = 1; i < d_numPoints; i++) {
    if (d_points[i].z > d_maxHeight) {
      d_maxHeight = d_points[i].z;
    } else if (d_points[i].z < d_minHeight) {
      d_minHeight = d_points[i].z;
    }
  }

  // add a little offset so we don't intersect exactly on the surface 
  // and end up going through the surface when doing ray intersections
  double difference = d_maxHeight - d_minHeight;
  d_maxHeight += 0.001*difference;
  d_minHeight -= 0.001*difference;
}

bool nmg_HeightField::contains(nmg_Point_3d &p)
{
  bool result = false;
  int triangleIndex = computeTriangleIndex(p.x, p.y);
  if (d_verbosity) printf("contains:\n triangleIndex: %d\n", triangleIndex);
  if (triangleIndex >= 0) {
    nmg_Vector_3d *norm = d_triangles[triangleIndex].normal();
    double d = (*norm)*(*(d_triangles[triangleIndex].A()));
    result = ((*norm)*p - d <= 0);
  }

  return result;
}

/*
triangle enumeration:
    ---------
    |\ 5|\ 7|
    |4 \|6 \|
    ---------
 ^  |\ 1|\ 3|
y|  |0 \|2 \|
 |  ---------
   ---> x
*/
bool nmg_HeightField::triangleContains(int triangleIndex, nmg_Point_3d &p)
{
  bool result = false;
  double minX, maxX, minY, maxY;
  int squareIndex = triangleIndex/2;
  int squareIndexY = squareIndex/(d_numX-1);
  int squareIndexX = squareIndex % (d_numX-1);

  minX = d_x[squareIndexX];
  maxX = d_x[squareIndexX+1];
  minY = d_y[squareIndexY];
  maxY = d_y[squareIndexY+1];

  double b = minX + minY + d_deltaY;
  double m = -d_diagonalSlope;
  // diagonal for the square is:
  // y = m*x + b so 
  // for lower triangle we want m*x + b - y <= 0
  // for upper triangle we want m*x + b - y >= 0

  if (triangleIndex%2 == 0) { // lower left
    result = (p.x >= minX &&
              p.y >= minY &&
              m*p.x + b - p.y >= 0); // y <= mx + b
  } else { // upper right
    result = (p.x <= maxX &&
              p.y <= maxY &&
              m*p.x + b - p.y <= 0); // y >= mx + b
  }
  return result;
}

double nmg_HeightField::height(double x, double y) 
{
  double result;
  int tri = computeTriangleIndex(x, y);
  if (tri < 0) {
    printf("Warning, tried to get height outside of surface\n");
    result = 0.0;
  } else {
    result = d_triangles[tri].plane().z(x, y);
  }
  return result;
}

nmg_Vector_3d nmg_HeightField::normal(double x, double y)
{
  nmg_Vector_3d result;
  int tri = computeTriangleIndex(x, y);
  if (tri < 0) {
    printf("Warning, tried to get height outside of surface\n");
    result = nmg_Vector_3d(0,0,1.0);
  } else {
    result = d_triangles[tri].plane().normal();
  }
  return result;
}

/* (3/14/02) note to self:
   I intend to have referencePlane goes through the center point
   (x=0.5*(maxX + minX), y=0.5*(maxY + minY), height(x, y))
*/
double nmg_HeightField::minHeightForSubregion(nmg_Plane &referencePlane,
                                             double minX, double minY,
                                             double maxX, double maxY)
{
  // look at heights at corners, heights at intersections of edges with
  // subregion edges and heights at contained vertices and take the minimum
  // of these
  int minSqIndexX, minSqIndexY, maxSqIndexX, maxSqIndexY;
  computeSquareIndex(minX, minY, minSqIndexX, minSqIndexY);
  computeSquareIndex(maxX, maxY, maxSqIndexX, maxSqIndexY);

  minSqIndexX++;
  minSqIndexY++;

  int i, j;
  double x, y, z_surf, z_plane, z_diff;
  double minHeight = 0.0;

  // set minHeight for first corner
  z_surf = height(minX, minY);
  z_plane = referencePlane.z(minX, minY);
  minHeight = z_surf - z_plane;

  // check the other three corners
  z_surf = height(maxX, minY);
  z_plane = referencePlane.z(maxX, minY);
  z_diff = z_surf - z_plane;
  if (z_diff < minHeight) minHeight = z_diff;
  z_surf = height(minX, maxY);
  z_plane = referencePlane.z(minX, maxY);
  z_diff = z_surf - z_plane;
  if (z_diff < minHeight) minHeight = z_diff;
  z_surf = height(maxX, maxY);
  z_plane = referencePlane.z(maxX, maxY);
  z_diff = z_surf - z_plane;
  if (z_diff < minHeight) minHeight = z_diff;

  // check intersection of vertical edges with horizontal subregion edges
  x = -d_centerX+minSqIndexX*d_deltaX;
  for (i = minSqIndexX; i <= maxSqIndexX; i++) {
    z_surf = height(x, minY);
    z_plane = referencePlane.z(x, minY);
    z_diff = z_surf - z_plane;
    if (z_diff < minHeight) minHeight = z_diff;

    z_surf = height(x, maxY);
    z_plane = referencePlane.z(x, maxY);
    z_diff = z_surf - z_plane;
    if (z_diff < minHeight) minHeight = z_diff;

    x += d_deltaX;
  }
  
  // check intersection of horizontal edges with vertical subregion edges
  y = -d_centerY+minSqIndexY*d_deltaY;
  for (j = minSqIndexY; j <= maxSqIndexY; j++) {
    z_surf = height(minX, y);
    z_plane = referencePlane.z(minX, y);
    z_diff = z_surf - z_plane;
    if (z_diff < minHeight) minHeight = z_diff;

    z_surf = height(maxX, y);
    z_plane = referencePlane.z(maxX, y);
    z_diff = z_surf - z_plane;
    if (z_diff < minHeight) minHeight = z_diff;

    y += d_deltaY;
  }

  // check vertices contained in the subregion
  x = -d_centerX+minSqIndexX*d_deltaX;
  for (i = minSqIndexX; i <= maxSqIndexX; i++) {
    y = -d_centerY+minSqIndexY*d_deltaY;
    for (j = minSqIndexY; j <= maxSqIndexY; j++) {
      z_surf = get(i, j);
      z_plane = referencePlane.z(x, y);
      z_diff = z_surf - z_plane;
      if (z_diff < minHeight) {
        minHeight = z_diff;
      }
      y += d_deltaY;
    }
    x += d_deltaX;
  }

  return minHeight;
}

nmg_Vector_3d nmg_HeightField::estimateSurfaceNormal(double minX, double minY, 
                                    double maxX, double maxY)
{
  if (minX == maxX || minY == maxY) {
    int tri0 = computeTriangleIndex(minX, minY);
    int tri1 = computeTriangleIndex(maxX, maxY);
    nmg_Vector_3d result = (*(d_triangles[tri0].normal()) +
                           *(d_triangles[tri1].normal()))*0.5;
    return result;
  } else {
    nmg_Point_3d p00(minX, minY, height(minX, minY));
    nmg_Point_3d p10(maxX, minY, height(maxX, minY));
    nmg_Point_3d p01(minX, maxY, height(minX, maxY));
    nmg_Point_3d p11(maxX, maxY, height(maxX, maxY));

    nmg_Vector_3d normal0 = (p10 - p00)/(p01 - p00);
    normal0.Normalize();
    nmg_Vector_3d normal1 = (p01 - p11)/(p10 - p11);
    normal1.Normalize();

    nmg_Vector_3d result = (normal0 + normal1)*0.5;
    return result;
  } 
}

bool nmg_HeightField::intersectsRaySegment(nmg_RaySegment &raySegment, 
                                          bool startingInside, 
                                          double &minT, nmg_Vector_3d &normal,
                                          bool mayIntersectRaySegmentJustCalled)
{
  if (d_verbosity) printf("intersectsSegment\n");
  bool foundIntersection = false;

  nmg_Point_3d start = raySegment.start();
  nmg_Point_3d end = raySegment.start() + raySegment.dir()*raySegment.length();

  // we could traverse the whole surface looking for intersections but
  // for efficiency, we will only look at those triangles that contain
  // parts of the segment
  // I originally thought of this as being like scan-converting a
  // line segment but then realized that boundary cases made this very
  // messy - we really want to scan-convert a fat line segment so I 
  // simplified it to search the entire surface in a rectangular bounding
  // box for the line segment - in practice the behavior should be
  // almost identical because the region shouldn't exceed 4 squares 
  // (8 triangles) of the height field with the expected input
  // (very short segments)

  double rx0, ry0, rx1, ry1;
  int x0, y0; // start square index
  int x1, y1; // end square index

  // if the user called the quick rejection test routine just before calling
  // this function and that returned no rejection then take the values
  // calculated in that function to save time
  if (mayIntersectRaySegmentJustCalled) {
    rx0 = (start.x+d_centerX)/d_deltaX;
    ry0 = (start.y+d_centerY)/d_deltaY;
    rx1 = (end.x+d_centerX)/d_deltaX;
    ry1 = (end.y+d_centerY)/d_deltaY;

    x0 = (int)(rx0);
    y0 = (int)(ry0);
    x1 = (int)(rx1);
    y1 = (int)(ry1);
  } else { // do all the quick rejection stuff in here
    if (start.z < d_minHeight && end.z < d_minHeight) return false;

    rx0 = (start.x+d_centerX)/d_deltaX;
    ry0 = (start.y+d_centerY)/d_deltaY;
    rx1 = (end.x+d_centerX)/d_deltaX;
    ry1 = (end.y+d_centerY)/d_deltaY;

    x0 = (int)(rx0);
    y0 = (int)(ry0);
    x1 = (int)(rx1);
    y1 = (int)(ry1);

    // quick test 2:
    int startTri = computeTriangleIndex(start.x, start.y, x0, y0);
    int endTri = computeTriangleIndex(end.x, end.y, x1, y1);
    if (startTri == endTri) {
      double safety = 0.1*raySegment.length();
      // we add in 0.1*length to be a little cautious so we don't reject 
      // something that would have been considered an intersection by the 
      // slightly different code below
      if ((start.z+safety) < d_triangles[startTri].plane().z(start.x, start.y) 
          &&
          (end.z+safety) < d_triangles[endTri].plane().z(end.x, end.y)) {
        return false;
      }
    }
  }
  // now we do the full test:

  if (rx0 - x0 < 0.0001) {
    if (rx0 <= rx1) {
      x0--;
    }
  } else if (rx0 - x0 > 0.9999) {
    if (rx0 > rx1) {
      x0++;
    }
  }
  if (rx1 - x1 < 0.0001) {
    if (rx1 < rx0) {
      x1--;
    }
  } else if (rx1 - x1 > 0.9999) {
    if (rx1 >= rx0) {
      x1++;
    }
  }
  if (ry0 - y0 < 0.0001) { 
    if (ry0 <= ry1) {
      y0--;
    }
  } else if (ry0 - y0 > 0.9999) { 
    if (ry0 > ry1) {
      y0++;
    }
  }
  if (ry1 - y1 < 0.0001) { 
    if (ry1 < ry0) {
      y1--;
    }
  } else if (ry1 - y1 > 0.9999) {
    if (ry1 >= ry0) {
      y1++;
    }
  }

  if (x0 < 0) x0 = 0;
  else if (x0 > d_numX-2) x0 = d_numX-2;
  if (x1 < 0) x1 = 0;
  else if (x1 > d_numX-2) x1 = d_numX-2;
  if (y0 < 0) y0 = 0;
  else if (y0 > d_numY-2) y0 = d_numY-2;
  if (y1 < 0) y1 = 0;
  else if (y1 > d_numY-2) y1 = d_numY-2;

  int squareIndexX = x0, squareIndexY = y0;
  int tri0, tri1;

  double testT;

  int temp = (int)2*( fabs( (double) (x0-x1) )+1)*( fabs( (double) (y0-y1))+1);

  bool hitedge0, hitedge1, hitedge2;

  // this is the expected common case:
  if (x0 == x1 && y0 == y1) {
    squareIndexX = x0; squareIndexY = y0;
    tri0 = 2*(squareIndexX + squareIndexY*(d_numX-1));
    if (d_triangles[tri0].intersectsRaySegment(raySegment, startingInside,
                   testT, hitedge0, hitedge1, hitedge2)) {
      if (!foundIntersection) {
        minT = testT;
        normal = *(d_triangles[tri0].normal());
        foundIntersection = true;
      } else if (testT <= minT) {
        if (testT < minT) {
          minT = testT;
          normal = *(d_triangles[tri0].normal());
        }
      }
    }
    tri1 = tri0+1;
    if (d_triangles[tri1].intersectsRaySegment(raySegment, startingInside,
                              testT, hitedge0, hitedge1, hitedge2)) {
      if (!foundIntersection) {
        minT = testT;
        normal = *(d_triangles[tri1].normal());
        foundIntersection = true;
      } else if (testT <= minT) {
        if (testT < minT) {
          minT = testT;
          normal = *(d_triangles[tri1].normal());
        }
      }
    }
  
    if (d_verbosity) printf("%d, %d\n", squareIndexX, squareIndexY);
    if (d_verbosity) printf("tri: %d, %d\n", tri0, tri1);

  } else { // a more exhaustive search for occasions expected to be rare:
    int indexX_incr;
    int indexX_bound;
    int indexY_incr;
    int indexY_bound;
    if (x1 < x0) {
      indexX_incr = -1;
      indexX_bound = -1;
    } else {
      indexX_incr = 1;
      indexX_bound = d_numX-1;
    }
    if (y1 < y0) {
      indexY_incr = -1;
      indexY_bound = -1;
    } else {
      indexY_incr = 1;
      indexY_bound = d_numY-1;
    }

    indexX_bound = x1 + indexX_incr;
    indexY_bound = y1 + indexY_incr;


    for (squareIndexX = x0; squareIndexX != indexX_bound;
                                          squareIndexX += indexX_incr) {
      for (squareIndexY = y0; squareIndexY != indexY_bound;
                                            squareIndexY += indexY_incr) {
        tri0 = 2*(squareIndexX + squareIndexY*(d_numX-1));
        if (d_triangles[tri0].intersectsRaySegment(raySegment, 
                 startingInside, testT, hitedge0, hitedge1, hitedge2)) {
          if (!foundIntersection) {
            minT = testT;
            normal = *(d_triangles[tri0].normal());
            foundIntersection = true;
          } else if (testT <= minT) {
           if (testT < minT) {
              normal = *(d_triangles[tri0].normal());
              minT = testT;
            }
          }
        }
        tri1 = tri0+1;
        if (d_triangles[tri1].intersectsRaySegment(raySegment, 
                 startingInside, testT, hitedge0, hitedge1, hitedge2)){
          if (!foundIntersection) {
            minT = testT;
            normal = *(d_triangles[tri1].normal());
            foundIntersection = true;
          } else if (testT <= minT) {
            if (testT < minT) {
              normal = *(d_triangles[tri1].normal());
              minT = testT;
            }
          }
        }
      }
    }
  }

  return foundIntersection;
}

bool nmg_HeightField::intersectsRay(nmg_Ray &ray, bool startingInside, 
                          double &minT, nmg_Vector_3d &normal)
{
  if (d_verbosity) printf("intersectsSegment\n");
  bool foundIntersection = false;
  minT = 0.0; // the value doesn't matter at this point because 
              // foundIntersection is false

  nmg_Point start = ray.start();
  nmg_Vector dir = ray.dir();

  if (dir.z > 0) {
    if (start.z > d_maxHeight) {
      return false;
    } else if (start.z < d_minHeight) {
      // find where we hit the z=d_minHeight plane and start from there
      double dxdz = dir.x/dir.z;
      double dydz = dir.y/dir.z;
      double x_intercept = start.x - dxdz*start.z;
      double y_intercept = start.y - dydz*start.z;
      start.x = dxdz*d_minHeight + x_intercept;
      start.y = dydz*d_minHeight + y_intercept;
      start.z = d_minHeight;
    }
  } else if (dir.z < 0) {
    if (start.z < d_minHeight) {
      return false;
    } else if (start.z > d_maxHeight) {
      // find where we hit the z=d_maxHeight plane and start from there
      double dxdz = dir.x/dir.z;
      double dydz = dir.y/dir.z;
      double x_intercept = start.x - dxdz*start.z;
      double y_intercept = start.y - dydz*start.z;
      start.x = dxdz*d_maxHeight + x_intercept;
      start.y = dydz*d_maxHeight + y_intercept;
      start.z = d_maxHeight;
    }
  } else { // dir.z == 0
    if (start.z <= d_minHeight || start.z >= d_maxHeight) {
      return false;
    }
  }

  // if the ray starts outside the grid then find where it first
  // enters the grid if at all
  if (start.x < -d_centerX || start.x > (d_sizeX-d_centerX) || 
      start.y < -d_centerY || start.y > (d_sizeY-d_centerY)) {
    double minT2, testT;
    double x_isect, y_isect;
    bool foundIntersection = false;
    // x = start.x + dir.x*t;
    // y = start.y + dir.y*t;
    if (dir.x != 0) {
      // find intersection with x = -d_centerX
      testT = (-d_centerX - start.x)/dir.x;
      y_isect = start.y + dir.y*testT;
      if (y_isect >= -d_centerY && y_isect <= (d_sizeY-d_centerY) && testT >= 0) {
        minT2 = testT;
        foundIntersection = true;
      }
      // find intersection with x = d_sizeX-d_centerX
      testT = (d_sizeX-d_centerX - start.x)/dir.x;
      y_isect = start.y + dir.y*testT;
      if (y_isect >= -d_centerY && y_isect <= (d_sizeY-d_centerY) && testT >= 0) {
        if (foundIntersection) {
          if (testT < minT2) {
            minT2 = testT;
          }
        } else {
          minT2 = testT;
          foundIntersection = true;
        }
      }
    }
    if (dir.y != 0) {
      // find intersection with y = -d_centerY
      testT = (-d_centerY - start.y)/dir.y;
      x_isect = start.x + dir.x*testT;
      if (x_isect >= -d_centerX && x_isect <= (d_sizeX-d_centerX) && testT >= 0) {
        if (foundIntersection) {
          if (testT < minT2) {
            minT2 = testT;
          }
        } else {
          minT2 = testT;
          foundIntersection = true;
        }
      }
      // find intersection with y = d_sizeY - d_centerY
      testT = (d_sizeY - d_centerY - start.y)/dir.y;
      x_isect = start.x + dir.x*testT;
      if (x_isect >= -d_centerX && x_isect <= (d_sizeX-d_centerX) && testT >= 0) {
        if (foundIntersection) {
          if (testT < minT2) {
            minT2 = testT;
          }
        } else {
          minT2 = testT;
          foundIntersection = true;
        }
      }
    }

    if (foundIntersection) {
      double invmag = 1.0/sqrt(dir.x*dir.x + dir.y*dir.y);
      start.x += dir.x*minT2;
      start.y += dir.y*minT2;
    } else {
      return false;
    }
  }

  // this is like Bresenham line-drawing with a thick line to make sure
  // nothing slips through the cracks due to round-off floating point error

  double rx0, ry0;
  int x0, y0; // start square index

  // real-valued grid coordinates for start position
  rx0 = (start.x+d_centerX)/d_deltaX;
  ry0 = (start.y+d_centerY)/d_deltaY;

  x0 = (int)floor(rx0);
  y0 = (int)floor(ry0);

  int minXindex, maxXindex, minYindex, maxYindex;
  double minX, maxX, minY, maxY;
  minX = x0*d_deltaX-d_centerX; maxX = minX + d_deltaX;
  minY = y0*d_deltaY-d_centerY; maxY = minY + d_deltaY;

  minXindex = x0; maxXindex = x0;
  minYindex = y0; maxYindex = y0;

  if (rx0 - x0 < 0.0001) {
    if (x0 > 0) {
      minX -= d_deltaX;
      minXindex--;
    }
  } else if (rx0 - x0 > 0.9999) {
    if (x0 < d_numX-2) {
      maxX += d_deltaX;
      maxXindex++;
    }
  }
  if (ry0 - y0 < 0.0001) {
    if (y0 > 0) {
      minY -= d_deltaY;
      minYindex--;
    }
  } else if (ry0 - y0 > 0.9999) {
    if (y0 < d_numY-2) {
      maxY += d_deltaY;
      maxYindex++;
    }
  }

  if (minXindex < 0) {
    minXindex = 0;
  } else if (minXindex > d_numX-2) {
    minXindex = d_numX-2;
  }
  if (maxXindex < 0) {
    maxXindex = 0;
  } else if (maxXindex > d_numX-2) {
    maxXindex = d_numX-2;
  }
  if (minYindex < 0) {
    minYindex = 0;
  } else if (minYindex > d_numY-2) {
    minYindex = d_numY-2;
  }
  if (maxYindex < 0) {
    maxYindex = 0;
  } else if (maxYindex > d_numY-2) {
    maxYindex = d_numY-2;
  }

  // assert: minXindex, minYindex, maxXindex, maxYindex are valid

  // check the initial set of squares
  // (minXindex, minYindex)
     
  testRaySquareIntersection(startingInside, foundIntersection, minT, ray,
          minXindex, minYindex, normal);

  // we still may need to test up to three other squares if we were near
  // a square boundary

  if (maxXindex != minXindex) {
    if (maxYindex != minYindex) {
      // 3 squares to check
      // (minXindex, maxYindex)
      // (maxXindex, minYindex)
      // (maxXindex, maxYindex)
      testRaySquareIntersection(startingInside, foundIntersection, minT, ray,
        minXindex, maxYindex, normal);
      testRaySquareIntersection(startingInside, foundIntersection, minT, ray,
        maxXindex, minYindex, normal);
      testRaySquareIntersection(startingInside, foundIntersection, minT, ray,
        maxXindex, maxYindex, normal);
    } else {
      // 1 squares to check
      // (maxXindex, minYindex)
      testRaySquareIntersection(startingInside, foundIntersection, minT, ray,
        maxXindex, minYindex, normal);
    }
  } else if (maxYindex != minYindex) {
    // 1 square to check
    // (minXindex, maxYindex)
    testRaySquareIntersection(startingInside, foundIntersection, minT, ray,
      minXindex, maxYindex, normal);
  }

  if (foundIntersection) {
    return foundIntersection;
  }

  int frontierXindex, frontierYindex, tailXindex, tailYindex;
  int  preLimitXindex, preLimitYindex, Xindex_incr, Yindex_incr;
  double frontierX, frontierY, tailX, tailY, X_incr, Y_incr;

  // now figure out what direction we will be going for all values
  // and how we will check if we've gone too far in that direction
  if (dir.x > 0) {
    frontierXindex = maxXindex;
    tailXindex = minXindex;
    frontierX = maxX;
    tailX = minX;
    preLimitXindex = d_numX-2;
    Xindex_incr = 1;
    X_incr = d_deltaX;
  } else {
    frontierXindex = minXindex;
    tailXindex = maxXindex;
    frontierX = minX;
    tailX = maxX;
    preLimitXindex = 0;
    Xindex_incr = -1;
    X_incr = -d_deltaX;
  }
  if (dir.y > 0) {
    frontierYindex = maxYindex;
    tailYindex = minYindex;
    frontierY = maxY;
    tailY = minY;
    preLimitYindex = d_numY-2;
    Yindex_incr = 1;
    Y_incr = d_deltaY;
  } else {
    frontierYindex = minYindex;
    tailYindex = maxYindex;
    frontierY = minY;
    tailY = maxY;
    preLimitYindex = 0;
    Yindex_incr = -1;
    Y_incr = -d_deltaY;
  }

  bool nearVertical = (fabs(dir.y) > 2*fabs(dir.x));
  bool nearHorizontal = (fabs(dir.y) < 0.5*fabs(dir.x));

  // assert: frontierXindex, frontierYindex, tailXindex, tailYindex are valid

  bool incrX, incrY;

  // these are used to calculate a boundary value for either
  // x or y such that when the ray crosses that boundary, we increment
  // the square index in the x or y direction respectively
  // I think that the frontierWeightFactor can theoretically be set anywhere 
  // between 0 and 1 but it is best to set it somewhat close to 1 without 
  // being too close to cause late increments due to roundoff error.
  // We are probably safe at 0.75 and tuning this parameter a little 
  // closer to 1.0 may improve performance in that in most cases it 
  // increases the delay before we start testing a line with a width of 2.
  // the correctness should probably be retested if you go much above 0.99
  // and the savings decrease beyond this point anyway
  const double frontierWeightFactor = 0.75;
  const double tailWeightFactor = 1.0 - frontierWeightFactor;

  if (nearVertical) {

    // x = dxdy*y + x_intercept
    double dxdy = dir.x/dir.y;
    double x_intercept = start.x - dxdy*start.y;
    double dzdy = dir.z/dir.y;
    double z_intercept = start.z - dzdy*start.y;
    double incrXtestLimit = frontierWeightFactor*frontierX + 
                            tailWeightFactor*tailX;
    while (!foundIntersection && frontierXindex != preLimitXindex &&
                                 frontierYindex != preLimitYindex) {
      double z_at_intersection = dzdy*tailY + z_intercept;
      // find intersection with y=frontierY
      double x_at_intersection = dxdy*frontierY + x_intercept;
 
      if (Xindex_incr > 0) {
        if (x_at_intersection > incrXtestLimit) {
          incrX = true;
          incrY = true;
        } else {
          incrX = false;
          incrY = true;
        }
      } else {
        if (x_at_intersection < incrXtestLimit) {
          incrX = true;
          incrY = true;
        } else {
          incrX = false;
          incrY = true;
        }
      }
      if (incrX) {
        // 3 squares to check
        testRaySquareIntersection(startingInside, foundIntersection, minT, ray,
              frontierXindex, frontierYindex+Yindex_incr, normal);
        testRaySquareIntersection(startingInside, foundIntersection, minT, ray,
              frontierXindex+Xindex_incr, frontierYindex, normal);
        testRaySquareIntersection(startingInside, foundIntersection, minT, ray,
              frontierXindex+Xindex_incr, frontierYindex+Yindex_incr, normal);
        if (frontierXindex != tailXindex) {
          tailXindex += Xindex_incr;
          tailX += X_incr;
          incrXtestLimit += X_incr;
        } else {
          incrXtestLimit += frontierWeightFactor*X_incr;
        }

        frontierXindex += Xindex_incr;
        frontierX += X_incr;
        frontierYindex += Yindex_incr;
        frontierY += Y_incr;
        tailYindex += Yindex_incr;
        tailY += Y_incr;
      } else {
        // increment Y only: 2 squares to check
        testRaySquareIntersection(startingInside, foundIntersection, minT, ray,
              tailXindex, frontierYindex+Yindex_incr, normal);
        if (frontierXindex != tailXindex) {
          testRaySquareIntersection(startingInside, 
                foundIntersection, minT, ray,
                frontierXindex, frontierYindex+Yindex_incr, normal);
        }
        frontierYindex += Yindex_incr;
        frontierY += Y_incr;
        tailYindex += Yindex_incr;
        tailY += Y_incr;
      }
      if (!foundIntersection && 
          ((dir.z > 0 && z_at_intersection > d_maxHeight) ||
           (dir.z < 0 && z_at_intersection < d_minHeight))) {
        return false;
      }

    } // end while

    if (foundIntersection) return foundIntersection;

    // find intersection with y=frontierY
    double x_at_intersection = dxdy*frontierY + x_intercept;
    if (Xindex_incr > 0) {
      if (x_at_intersection > incrXtestLimit) {
        incrX = true;
        incrY = true;
      } else {
        incrX = false;
        incrY = true;
      }
    } else {
      if (x_at_intersection < incrXtestLimit) {
        incrX = true;
        incrY = true;
      } else {
        incrX = false;
        incrY = true;
      }
    }
    if (incrX) {
      // 3 squares but only 0 or 1 are actually in the grid
      if (frontierXindex != preLimitXindex) {
        testRaySquareIntersection(startingInside, foundIntersection, minT, ray,
              frontierXindex+Xindex_incr, frontierYindex, normal);
      } 
      if (frontierYindex != preLimitYindex) {
        testRaySquareIntersection(startingInside, foundIntersection, minT, ray,
              frontierXindex, frontierYindex+Yindex_incr, normal);
      }
    } else {
      // 2 squares but only 0 or 2 are actually in the grid
      if (frontierYindex != preLimitYindex) {
        testRaySquareIntersection(startingInside, foundIntersection, minT, ray,
              tailXindex, frontierYindex+Yindex_incr, normal);
        if (frontierXindex != tailXindex) {
          testRaySquareIntersection(startingInside, 
                foundIntersection, minT, ray,
                frontierXindex, frontierYindex+Yindex_incr, normal);
        }
      }
    }
  } else if (nearHorizontal) {

    // y = dydx*x + y_intercept
    double dydx = dir.y/dir.x;
    double y_intercept = start.y - dydx*start.x;
    double dzdx = dir.z/dir.x;
    double z_intercept = start.z - dzdx*start.x;
    double incrYtestLimit = frontierWeightFactor*frontierY + 
                            tailWeightFactor*tailY;
    while (!foundIntersection && frontierXindex != preLimitXindex &&
                                 frontierYindex != preLimitYindex) {

      double z_at_intersection = dzdx*tailX + z_intercept;
      // find intersection with x=frontierX
      double y_at_intersection = dydx*frontierX + y_intercept;
      if (Yindex_incr > 0) {
        if (y_at_intersection > incrYtestLimit) {
          incrX = true;
          incrY = true;
        } else {
          incrX = true;
          incrY = false;
        }
      } else {
        if (y_at_intersection < incrYtestLimit) {
          incrX = true;
          incrY = true;
        } else {
          incrX = true;
          incrY = false;
        }
      }
      if (incrY) {
        // 3 squares to check
        testRaySquareIntersection(startingInside, foundIntersection, minT, ray,
              frontierXindex, frontierYindex+Yindex_incr, normal);
        testRaySquareIntersection(startingInside, foundIntersection, minT, ray,
              frontierXindex+Xindex_incr, frontierYindex, normal);
        testRaySquareIntersection(startingInside, foundIntersection, minT, ray,
              frontierXindex+Xindex_incr, frontierYindex+Yindex_incr, normal);
        if (frontierYindex != tailYindex) {
          tailYindex += Yindex_incr;
          tailY += Y_incr;
          incrYtestLimit += Y_incr;
        } else {
          incrYtestLimit += frontierWeightFactor*Y_incr;
        }
        frontierXindex += Xindex_incr;
        frontierX += X_incr;
        tailXindex += Xindex_incr;
        tailX += X_incr;
        frontierYindex += Yindex_incr;
        frontierY += Y_incr;
      } else {
        // increment X only: 2 squares to check

        testRaySquareIntersection(startingInside, foundIntersection, minT, ray,
              frontierXindex+Xindex_incr, tailYindex, normal);
        if (frontierYindex != tailYindex) {
          testRaySquareIntersection(startingInside, 
                foundIntersection, minT, ray,
                frontierXindex+Xindex_incr, frontierYindex, normal);
        }

        frontierXindex += Xindex_incr;
        tailXindex += Xindex_incr;
        frontierX += X_incr;
        tailX += X_incr;

      }
      if (!foundIntersection &&   
          ((dir.z > 0 && z_at_intersection > d_maxHeight) ||
           (dir.z < 0 && z_at_intersection < d_minHeight))) {
        return false;
      }
    } // end while

    if (foundIntersection) return foundIntersection;

    // find intersection with x=frontierY
    double y_at_intersection = dydx*frontierX + y_intercept;
    if (Yindex_incr > 0) {
      if (y_at_intersection > incrYtestLimit) {
        incrX = true;
        incrY = true;
      } else {
        incrX = true;
        incrY = false;
      }
    } else {
      if (y_at_intersection < incrYtestLimit) {
        incrX = true;
        incrY = true;
      } else {
        incrX = true;
        incrY = false;
      }
    }

    if (incrY) {
      // 3 squares but only 0 or 1 are actually in the grid
      if (frontierXindex != preLimitXindex) {
        testRaySquareIntersection(startingInside, foundIntersection, minT, ray,
              frontierXindex+Xindex_incr, frontierYindex, normal);
      }
      if (frontierYindex != preLimitYindex) {
        testRaySquareIntersection(startingInside, foundIntersection, minT, ray,
              frontierXindex, frontierYindex+Yindex_incr, normal);
      }
    } else {
      // 2 squares but only 0 or 2 are actually in the grid
      if (frontierXindex != preLimitXindex) {
        testRaySquareIntersection(startingInside, foundIntersection, minT, ray,
              frontierXindex+Xindex_incr, tailYindex, normal);
        if (frontierYindex != tailYindex) {
          testRaySquareIntersection(startingInside, 
                foundIntersection, minT, ray,
                frontierXindex+Xindex_incr, frontierYindex, normal);
        }
      }
    }

  } else { // then its close to diagonal
    // x = dxdy*y + x_intercept
    // y = dydx*x + y_intercept
    double dxdy = dir.x/dir.y;
    double x_intercept = start.x - dxdy*start.y;
    double dydx = dir.y/dir.x;
    double y_intercept = start.y - dydx*start.x;
    double dzdx = dir.z/dir.x;
    double z_intercept = start.z - dzdx*start.x;
    double incrXtestLimit = frontierWeightFactor*frontierX + 
                            tailWeightFactor*tailX;
    double incrYtestLimit = frontierWeightFactor*frontierY + 
                            tailWeightFactor*tailY;
    while (!foundIntersection && frontierXindex != preLimitXindex &&
                                 frontierYindex != preLimitYindex) {
      double z_at_intersection = dzdx*tailX + z_intercept;
      // first find intersection with x=frontierX
      double y_at_intersection = dydx*frontierX + y_intercept;
      if (((Yindex_incr > 0) && 
           (y_at_intersection < incrYtestLimit)) ||
          ((Yindex_incr < 0) &&
           (y_at_intersection > incrYtestLimit))) {
        incrX = true;
        incrY = false;
      } else {
        // find intersection with y=frontierY
        double x_at_intersection = dxdy*frontierY + x_intercept;
        if (((Xindex_incr > 0) &&
             (x_at_intersection < incrXtestLimit)) ||
            ((Xindex_incr < 0) &&
             (x_at_intersection > incrXtestLimit))) {
          incrX = false;
          incrY = true;
        } else {
          incrX = true;
          incrY = true;
        }
      }
      if (!incrX) {
        // increment Y only: 2 squares to check
        testRaySquareIntersection(startingInside, foundIntersection, minT, ray,
              tailXindex, frontierYindex+Yindex_incr, normal);
        if (frontierXindex != tailXindex) {
          testRaySquareIntersection(startingInside, 
                foundIntersection, minT, ray,
                frontierXindex, frontierYindex+Yindex_incr, normal);
        }

        frontierYindex += Yindex_incr;
        frontierY += Y_incr;
        tailYindex += Yindex_incr;
        tailY += Y_incr;
        incrYtestLimit += Y_incr;
      } else if (!incrY) {
        // increment X only: 2 squares to check
        testRaySquareIntersection(startingInside, foundIntersection, minT, ray,
              frontierXindex+Xindex_incr, tailYindex, normal);
        if (frontierYindex != tailYindex) {
          testRaySquareIntersection(startingInside, 
                foundIntersection, minT, ray,
                frontierXindex+Xindex_incr, frontierYindex, normal);
        }

        frontierXindex += Xindex_incr;
        tailXindex += Xindex_incr;
        frontierX += X_incr;
        tailX += X_incr;
        incrXtestLimit += X_incr;
      } else {
        // 3 squares to check
        testRaySquareIntersection(startingInside, foundIntersection, minT, ray,
              frontierXindex, frontierYindex+Yindex_incr, normal);
        testRaySquareIntersection(startingInside, foundIntersection, minT, ray,
              frontierXindex+Xindex_incr, frontierYindex, normal);
        testRaySquareIntersection(startingInside, foundIntersection, minT, ray,
              frontierXindex+Xindex_incr, frontierYindex+Yindex_incr, normal);
        if (frontierXindex != tailXindex) {
          tailXindex += Xindex_incr;
          tailX += X_incr; 
          incrXtestLimit += X_incr;
        } else {
          incrXtestLimit += frontierWeightFactor*X_incr;
        }
        if (frontierYindex != tailYindex) {
          tailYindex += Yindex_incr;
          tailY += Y_incr; 
          incrYtestLimit += Y_incr;
        } else {
          incrYtestLimit += frontierWeightFactor*Y_incr;
        }
        frontierXindex += Xindex_incr;
        frontierX += X_incr;
        frontierYindex += Yindex_incr;
        frontierY += Y_incr;
      }
      if (incrX && !foundIntersection &&   
          ((dir.z > 0 && z_at_intersection > d_maxHeight) ||
           (dir.z < 0 && z_at_intersection < d_minHeight))) {
        return false;
      }
    } // end while

    if (foundIntersection) return foundIntersection;

    // find intersection with x=frontierX
    double y_at_intersection = dydx*frontierX + y_intercept;
    if (((Yindex_incr > 0) &&
         (y_at_intersection < incrYtestLimit)) ||
        ((Yindex_incr < 0) &&
         (y_at_intersection > incrYtestLimit))) {
      incrX = true;
      incrY = false;
    } else {
      // find intersection with y=frontierY
      double x_at_intersection = dxdy*frontierY + x_intercept;
      if (((Xindex_incr > 0) &&
           (x_at_intersection < incrXtestLimit)) ||
          ((Xindex_incr < 0) &&
           (x_at_intersection > incrXtestLimit))) {
        incrX = false;
        incrY = true;
      } else {
        incrX = true;
        incrY = true;
      }
    }
    if (!incrX) {
      // 2 squares but only 0 or 2 are actually in the grid
      if (frontierYindex != preLimitYindex) {
        testRaySquareIntersection(startingInside, foundIntersection, minT, ray,
              tailXindex, frontierYindex+Yindex_incr, normal);
        if (frontierXindex != tailXindex) {
          testRaySquareIntersection(startingInside,
                foundIntersection, minT, ray,
                frontierXindex, frontierYindex+Yindex_incr, normal);
        }
      }
    } else if (!incrY) {
      // 2 squares but only 0 or 2 are actually in the grid
      if (frontierXindex != preLimitXindex) {
        testRaySquareIntersection(startingInside, foundIntersection, minT, ray,
              frontierXindex+Xindex_incr, tailYindex, normal);
        if (frontierYindex != tailYindex) {
          testRaySquareIntersection(startingInside, 
                foundIntersection, minT, ray,
                frontierXindex+Xindex_incr, frontierYindex, normal);
        }
      }
    } else {
      // 3 squares but only 0 or 1 are actually in the grid
      if (frontierXindex != preLimitXindex) {
        testRaySquareIntersection(startingInside, foundIntersection, minT, ray,
              frontierXindex+Xindex_incr, frontierYindex, normal);
      }
      if (frontierYindex != preLimitYindex) {
        testRaySquareIntersection(startingInside, foundIntersection, minT, ray,
              frontierXindex, frontierYindex+Yindex_incr, normal);
      }
    }
  }

  return foundIntersection;
}

void nmg_HeightField::setVerbosity(int v) {
  d_verbosity = v;
}

