#ifndef NMG_HEIGHTFIELD_H
#define NMG_HEIGHTFIELD_H

//#include <stl_config.h> // for bool
#include "nmg_Point.h"
#include "nmg_Triangle.h"
#include <assert.h>
#include "nmg_Types.h"
#include <stdlib.h>
#include "nmb_Image.h"

class nmg_HeightField {
 public:
  nmg_HeightField(nmb_Image *image, double sideLength,
                               double zScale);
  nmg_HeightField(int num_x, int num_y, double sideLength);
  nmg_HeightField(nmg_HeightField *surface,
                 int i_min, int j_min, int i_max, int j_max);
  nmg_HeightField(const nmg_HeightField &surface);
  void doBasicInitialization();
  int memoryUsed() {return d_memoryUsed;}
  ~nmg_HeightField();

  void set(int x, int y, double height)
  {
    int index = computeIndex(x,y);
    d_points[index].z = height;
  }

  double get(int x, int y)
         {return d_points[computeIndex(x,y)].z;}

  double height(double x, double y);
  nmg_Vector_3d normal(double x, double y);

  bool contains(nmg_Point_3d &p);
  bool triangleContains(int triangleIndex, nmg_Point_3d &p);

  /* Intersection test routines
  */
  inline bool mayIntersectRaySegment(nmg_Float z_start, nmg_Float z_end);
  inline bool mayIntersectRaySegment(nmg_Point start, nmg_Point end);
  bool intersectsRay(nmg_Ray &ray, bool startingInside, double &minT,
                     nmg_Vector_3d &normal);
  bool intersectsRaySegment(nmg_RaySegment &raySegment, bool startingInside,
                            double &minT, nmg_Vector_3d &normal,
                            bool mayIntersectRaySegmentJustCalled = false);

  void setVerbosity(int v);
  void rebuildTriangles();

  inline int getSurfaceRegionID(double x, double y);
  inline bool correctSurfaceRegionID(double x, double y, int regionID);
  inline double heightAboveSurface(nmg_Point point, int regionID);

  double minHeightForSubregion(nmg_Plane &referencePlane, 
                               double minX, double minY, 
                               double maxX, double maxY);
  nmg_Vector_3d estimateSurfaceNormal(double minX, double minY,
                                    double maxX, double maxY);

  double deltaX() {return d_deltaX;}
  double deltaY() {return d_deltaY;}

  double minX() { return -d_centerX;}
  double maxX() { return d_sizeX-d_centerX;}
  double minY() { return -d_centerY;}
  double maxY() { return d_sizeY-d_centerY;}

 protected:
  inline void computeSquareIndex(double x, double y,
                                 int &sqXindex, int &sqYindex);
  
  inline void computeSquareIndex(double x, double y,
                                 int &sqXindex, int &sqYindex,
                                 double &xfract, double &yfract);

  inline int computeTriangleIndex(double x, double y, int sqX, int sqY);
  inline int computeTriangleIndex(double x, double y);
  int computeIndex(int x, int y) 
      {return x + y*d_numX;}
  inline bool testRaySquareIntersection(bool startingInside, 
                                        bool &foundIntersection,
                                        double &minT, nmg_Ray &ray,
                                        int squareIndexX, int squareIndexY, 
                                        nmg_Vector_3d &normal);
  inline bool checkSquareIndex(int x, int y);

  // ============  Data ==================
  int d_memoryUsed;
  int d_numX;
  int d_numY;
  double d_sizeX;
  double d_sizeY;
  double d_deltaX;
  double d_deltaY;
  double d_diagonalSlope;

  double *d_x;
  double *d_y;

  // for tri-mesh:
  int d_numPoints;
  nmg_Point_3d *d_points;

  int d_numTriangles;
  nmg_Triangle *d_triangles;
 
  double d_maxHeight;
  double d_minHeight;
 
  int d_verbosity;

  double d_centerX;
  double d_centerY;

};

//inline 
int nmg_HeightField::getSurfaceRegionID(double x, double y)
{
  return computeTriangleIndex(x, y);
}

//inline 
bool nmg_HeightField::correctSurfaceRegionID(double x, double y, int regionID)
{
  if (regionID == -1) {
    if (x > (d_sizeX-d_centerX) || x < -d_centerX ||
        y > (d_sizeY-d_centerY) || y < -d_centerY) {
      return true;
    } else {
      return false;
    }
  } else {
    double minX, minY, maxX, maxY, b;
    if (regionID % 2 == 0) {
      minX = d_triangles[regionID].C()->x;
      if (x < minX) return false;
      minY = d_triangles[regionID].B()->y;
      if (y < minY) return false;
      maxX = d_triangles[regionID].B()->x;
      if (x > maxX) return false;
      maxY = d_triangles[regionID].C()->y;
      if (y > maxY) return false;
      b = minX + minY + d_deltaY;
      if (-d_diagonalSlope*x + b < y) {
        return false;
      }
    } else {
      minX = d_triangles[regionID].C()->x;
      if (x < minX) return false;
      minY = d_triangles[regionID].A()->y;
      if (y < minY) return false;
      maxX = d_triangles[regionID].A()->x;
      if (x > maxX) return false;
      maxY = d_triangles[regionID].C()->y;
      if (y > maxY) return false;
      b = minX + minY + d_deltaY;
      if (-d_diagonalSlope*x + b >= y) {
        return false;
      }
    }
  }
  return true;
}

//inline 
double nmg_HeightField::heightAboveSurface(nmg_Point point, int regionID)
{
  double result;
  if (regionID == -1) {
    result = -1.0;
  } else {
    result = d_triangles[regionID].plane().height(point);
  }
  return result;
}

void nmg_HeightField::computeSquareIndex(double x, double y, 
                                        int &sqXindex, int &sqYindex)
{
  if (x > (d_sizeX-d_centerX) || x < -d_centerX ||
      y > (d_sizeY-d_centerY) || y < -d_centerY) {
    sqXindex = -1;
    sqYindex = -1;
  } else {
    double numDeltaX, numDeltaY;
    numDeltaX = (x + d_centerX)/d_deltaX;
    numDeltaY = (y + d_centerY)/d_deltaY;
    sqXindex = (int)(numDeltaX);
    if (sqXindex == d_numX-1) sqXindex = d_numX-2;
    sqYindex = (int)(numDeltaY);
    if (sqYindex == d_numY-1) sqYindex = d_numY-2;
  }
  return;
}

void nmg_HeightField::computeSquareIndex(double x, double y,
                                        int &sqXindex, int &sqYindex, 
                                        double &xfract, double &yfract)
{
  if (x > (d_sizeX-d_centerX) || x < -d_centerX ||
      y > (d_sizeY-d_centerY) || y < -d_centerY) {
    sqXindex = -1;
    sqYindex = -1;
  } else {
    double numDeltaX, numDeltaY;
    numDeltaX = (x + d_centerX)/d_deltaX;
    numDeltaY = (y + d_centerY)/d_deltaY;
    sqXindex = (int)(numDeltaX);
    if (sqXindex == d_numX-1) {
      sqXindex = d_numX-2;
      xfract = 1.0;
    } else {
      xfract = numDeltaX - sqXindex;
    }
    sqYindex = (int)(numDeltaY);
    if (sqYindex == d_numY-1) {
      sqYindex = d_numY-2;
      yfract = 1.0;
    } else {
      yfract = numDeltaY - sqYindex;
    }
  }
  return;
}

// inline
int nmg_HeightField::computeTriangleIndex(double x, double y, int sqX, int sqY)
{
  int result = 2*(sqX+sqY*(d_numX-1));
  double minX = sqX*d_deltaX-d_centerX;
  double minY = sqY*d_deltaY-d_centerY;
  double b = minX + minY + d_deltaY;
  if (-d_diagonalSlope*x + b < y) {
    result++;
  }
  return result;
}

// inline
int nmg_HeightField::computeTriangleIndex(double x, double y)
{
  int result;
  int sqXindex, sqYindex;
  computeSquareIndex(x, y, sqXindex, sqYindex);
  if (sqXindex < 0 || sqYindex < 0) {
    result = -1;
  } else {
    result = computeTriangleIndex(x, y, sqXindex, sqYindex);
  }
  return result;
}

bool nmg_HeightField::testRaySquareIntersection(bool startingInside,
                 bool &foundIntersection,
                 double &minT, nmg_Ray &ray,
                 int squareIndexX, int squareIndexY,
                 nmg_Vector_3d &normal)
{

  if (squareIndexX < 0 || squareIndexX > d_numX-2 ||
      squareIndexY < 0 || squareIndexY > d_numY-2) {
    printf("testRaySquareIntersection: Error, invalid index\n");
    exit(-1);
  }

//  printf("testing square: (%d, %d)\n", squareIndexX, squareIndexY);
  double testT = 0.0;
  int tri0 = 2*(squareIndexX + squareIndexY*(d_numX-1));
  int tri1 = tri0+1;
  bool hit0, hit1, hit2;
  if (d_triangles[tri0].intersectsRay(ray, startingInside, testT,
                                      hit0, hit1, hit2)) {
    if (!foundIntersection) {
      minT = testT;
      normal = *(d_triangles[tri0].normal());
      foundIntersection = true;
    } else if (testT < minT) {
      minT = testT;
      normal = *(d_triangles[tri0].normal());
    }
/*
    } else if (testT < minT + d_epsilonDist) {
      if (minT > testT+d_epsilonDist) {
        minT = testT;
      }
    }
*/
  }
  if (d_triangles[tri1].intersectsRay(ray, startingInside, testT,
                                      hit0, hit1, hit2)) {
    if (!foundIntersection) {
      minT = testT;
      normal = *(d_triangles[tri1].normal());
      foundIntersection = true;
    } else if (testT < minT) {
      minT = testT;
      normal = *(d_triangles[tri1].normal());
    }
/*
    } else if (testT < minT+d_epsilonDist) {
      if (minT > testT+d_epsilonDist) {
        minT = testT;
      }
    }
*/
  }
  return foundIntersection;
}

bool nmg_HeightField::checkSquareIndex(int x, int y) 
{
  return (x > -1 && x < d_numX-1 && y > -1 && y < d_numY-1);
}

bool nmg_HeightField::mayIntersectRaySegment(nmg_Float z_start, nmg_Float z_end)
{
  if (z_start < d_minHeight && 
      z_end < d_minHeight) return false;
  return true;
}

bool nmg_HeightField::mayIntersectRaySegment(nmg_Point start, nmg_Point end)
{
  if (start.z < d_minHeight && end.z < d_minHeight) return false;

  double rx0 = (start.x+d_centerX)/d_deltaX;
  double ry0 = (start.y+d_centerY)/d_deltaY;
  double rx1 = (end.x+d_centerX)/d_deltaX;
  double ry1 = (end.y+d_centerY)/d_deltaY;

  int x0 = (int)(rx0);
  int y0 = (int)(ry0);
  int x1 = (int)(rx1);
  int y1 = (int)(ry1);

  int startTri = computeTriangleIndex(start.x, start.y, x0, y0);
  int endTri = computeTriangleIndex(end.x, end.y, x1, y1);
  if (startTri == endTri) {
    double safety = 0.1*fabs(end.z - start.z);
    // we add in 0.1*length to be a little cautious so we don't reject
    // something that would have been considered an intersection by the
    // slightly different code below
    if ((start.z+safety) < d_triangles[startTri].plane().z(start.x, start.y) &&
        (end.z+safety) < d_triangles[endTri].plane().z(end.x, end.y)) {
      return false;
    }
  }
  return true;
}

#endif
