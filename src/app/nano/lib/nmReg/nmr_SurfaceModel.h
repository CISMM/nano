#ifndef NMR_SURFACEMODEL_H
#define NMR_SURFACEMODEL_H

class nmr_SurfaceModel {
 public:
  // solve for the minimal t such that 
  // (x,y,z) + t*(vx,vy,vz) lies in the surface
  virtual void surfaceRayIntersection(double x, double y, double z,
                                      double vx, double vy, double vz,
                                      double &t) = 0;
};

// plane defined by the equation: nx*x + ny*y + nz*z + d = 0
class nmr_SurfaceModelPlane : public nmr_SurfaceModel {
 public:
  nmr_SurfaceModelPlane(double nx=0, double ny=0, double nz=1, double d=0):
    d_nx(nx), d_ny(ny), d_nz(nz), d_d(d) {}
  virtual void surfaceRayIntersection(double x, double y, double z,
                                      double vx, double vy, double vz, 
                                      double &t)
  {
    // nx*x + ny*y + nz*z + d = 0
    // nx*(x+t*vx) + ny*(y+t*vy) + nz*(z+t*vz) + d = 0
    // t*(vx*nx + vy*ny + vz*nz) = -(nx*x + ny*y + nz*z + d)
    t = -(d_nx*x + d_ny*y + d_nz*z + d_d)/(vx*d_nx + vy*d_ny + vz*d_nz);
    return;
  }
 private:
  double d_nx, d_ny, d_nz, d_d;
};

#endif