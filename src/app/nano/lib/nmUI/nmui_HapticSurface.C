#include "nmui_HapticSurface.h"

#include <quat.h>

#include <v.h>  // BUG - for VectorType

#include <BCPlane.h>  // needed by normal.h

#include "normal.h"  // BUG - replace with quatlib!

#include <nmb_Dataset.h>
#include <nmb_Decoration.h>

#ifndef USE_VRPN_MICROSCOPE
#include <Microscope.h>
#else
#include <nmm_MicroscopeRemote.h>
#endif



nmui_HapticSurface::nmui_HapticSurface (void) {

}

// virtual
nmui_HapticSurface::~nmui_HapticSurface (void) {

}

// virtual
void nmui_HapticSurface::getLocation (q_vec_type x) const {
  q_vec_copy(x, (double *) d_handPosMS);
}

// virtual
void nmui_HapticSurface::getOutputPlane (q_vec_type n, double * d) const {

  n[0] = d_currentPlaneNormal[0];
  n[1] = d_currentPlaneNormal[1];
  n[2] = d_currentPlaneNormal[2];
  *d = d_currentPlaneParameter;

}

// virtual
double nmui_HapticSurface::distanceFromSurface (void) const {
  return d_distanceFromPlane;
}


// virtual
void nmui_HapticSurface::setLocation (q_vec_type x) {

  d_handPosMS[0] = x[0];
  d_handPosMS[1] = x[1];
  d_handPosMS[2] = x[2];

}

// virtual
void nmui_HapticSurface::setLocation (double x, double y, double z) {

  d_handPosMS[0] = x;
  d_handPosMS[1] = y;
  d_handPosMS[2] = z;

}

// virtual
void nmui_HapticSurface::enable (void) {
  d_enabled = VRPN_TRUE;
}

// virtual
void nmui_HapticSurface::disable (void) {
  d_enabled = VRPN_FALSE;
}




// static
void nmui_HapticSurface::pointToTrackerFromWorld (q_vec_type out,
                                                  const q_vec_type in) {
  v_xform_type WorldFromTracker;
  v_xform_type TrackerFromWorld;

  v_x_compose(&WorldFromTracker,
          &v_world.users.xforms[0],
          &v_users[0].xforms[V_ROOM_FROM_HAND_TRACKER]);

  v_x_invert(&TrackerFromWorld, &WorldFromTracker);

  v_x_xform_vector(out, &TrackerFromWorld, (double *) in);

}

// static
void nmui_HapticSurface::vectorToTrackerFromWorld (q_vec_type out,
                                                   const q_vec_type in) {
  v_xform_type WorldFromTracker;
  v_xform_type TrackerFromWorld;

  v_x_compose(&WorldFromTracker,
          &v_world.users.xforms[0],
          &v_users[0].xforms[V_ROOM_FROM_HAND_TRACKER]);

  v_x_invert(&TrackerFromWorld, &WorldFromTracker);

  q_xform(out, TrackerFromWorld.rotate, (double *) in);
}

// virtual
void nmui_HapticSurface::computeDistanceFromPlane (void) {

  // Signed distance from plane (plane in world space)
  // This is the dot product of the vector from the plane's point to the
  // hand location and the plane's unit normal vector.

  q_vec_type handInTracker;
  q_vec_type planeToHandVector;

  vectorToTrackerFromWorld(handInTracker, d_handPosMS);
  q_vec_subtract( planeToHandVector, handInTracker, d_planePosPH );
  d_distanceFromPlane = q_vec_dot_product(planeToHandVector,
                                           d_currentPlaneNormal);


};





nmui_HSCanned::nmui_HSCanned (nmb_Dataset * dset) :
    nmui_HapticSurface (),
    d_dataset (dset) {


}

// virtual
nmui_HSCanned::~nmui_HSCanned (void) {

}

int nmui_HSCanned::getGridX (void) const {
  return d_gridX;
}

int nmui_HSCanned::getGridY (void) const {
  return d_gridY;
}



// virtual
void nmui_HSCanned::update (void) {

  BCPlane * plane = d_dataset->inputGrid->getPlaneByName
                     (d_dataset->heightPlaneName->string());
  if (!plane) {
      fprintf(stderr, "nmui_HSCanned::getOutputPlane:  "
                      "could not get plane!\n");
      return;
  }

  vrpn_bool zero_xnorm = VRPN_FALSE;
  vrpn_bool zero_ynorm = VRPN_FALSE;

  VectorType Norm00, Norm01, Norm10, Norm11;

  d_planePosPH[0] = d_handPosMS[0];
  d_planePosPH[1] = d_handPosMS[1];

  // Translate microscope-space coordinates to (integer and fractional)
  // grid position.

  double rangeX = plane->maxX() - plane->minX();
  double rangeY = plane->maxY() - plane->minY();

  double rx = ((d_handPosMS[0] - plane->minX()) / rangeX * (plane->numX() - 1));
  double ry = ((d_handPosMS[1] - plane->minY()) / rangeY * (plane->numY() - 1));

  d_gridX = (int) rx;
  d_gridY = (int) ry;

  double a = rx - d_gridX;
  double b = ry - d_gridY;

  if (d_gridX >= plane->numX() - 1) {
    d_gridX = plane->numX() - 2;
    zero_xnorm = VRPN_TRUE;
    d_planePosPH[0] = d_gridX * rangeX * (plane->numX() - 1) + plane->minX();
    a = 1.0;
  }
  if (d_gridY >= plane->numY() - 1) {
    d_gridY = plane->numY() - 2;
    zero_ynorm = VRPN_TRUE;
    d_planePosPH[1] = d_gridY * rangeY * (plane->numY() - 1) + plane->minY();
    b = 1.0;
  }
  if (d_gridX <= 0) {
    d_gridX = 0;
    zero_xnorm = VRPN_TRUE;
    d_planePosPH[0] = d_gridX * rangeX * (plane->numX() - 1) + plane->minX();
    a = 0.0;
  }
  if (d_gridY <= 0) {
    d_gridY = 0;
    zero_ynorm = VRPN_TRUE;
    d_planePosPH[1] = d_gridY * rangeY * (plane->numY() - 1) + plane->minY();
    b = 0.0;
  }

  d_planePosPH[2] = 
      plane->scaledValue(d_gridX,   d_gridY  ) * (1-a) * (1-b)
    + plane->scaledValue(d_gridX+1, d_gridY  ) * (  a) * (1-b)
    + plane->scaledValue(d_gridX,   d_gridY+1) * (1-a) * (  b)
    + plane->scaledValue(d_gridX+1, d_gridY+1) * (  a) * (  b);


  // BUG - uses normal.h but we ought to be using quatlib normals.
  // Comptue the normals at the four nearest grid points.

  Compute_Norm(d_gridX,   d_gridY,   Norm00, plane);
  Compute_Norm(d_gridX+1, d_gridY,   Norm10, plane);
  Compute_Norm(d_gridX+1, d_gridY+1, Norm11, plane);
  Compute_Norm(d_gridX,   d_gridY+1, Norm01, plane);

  // Average the four normals together with appropriate weighting.

  VectorScale(Norm00, (1-a)*(1-b));
  VectorCopy(d_currentPlaneNormal, Norm00);
  VectorScale(Norm10, (  a)*(1-b));
  VectorAdd(d_currentPlaneNormal, Norm10, d_currentPlaneNormal);
  VectorScale(Norm01, (1-a)*(  b));
  VectorAdd(d_currentPlaneNormal, Norm01, d_currentPlaneNormal);
  VectorScale(Norm11, (  a)*(  b));
  VectorAdd(d_currentPlaneNormal, Norm11, d_currentPlaneNormal);

  if (zero_xnorm) d_currentPlaneNormal[0] = 0.0;
  if (zero_ynorm) d_currentPlaneNormal[1] = 0.0;

  pointToTrackerFromWorld(d_planePosPH, d_planePosPH);
  vectorToTrackerFromWorld(d_currentPlaneNormal, d_currentPlaneNormal);

  VectorNormalize(d_currentPlaneNormal);

  d_currentPlaneParameter = - q_vec_dot_product(d_currentPlaneNormal,
                                                d_planePosPH);

  computeDistanceFromPlane();
};



nmui_HSMeasurePlane::nmui_HSMeasurePlane (nmb_Dataset * dset,
                                          nmb_Decoration * dec) :
    nmui_HapticSurface (),
    d_dataset (dset),
    d_decoration (dec) {

}

// virtual
nmui_HSMeasurePlane::~nmui_HSMeasurePlane (void) {

}

// virtual
void nmui_HSMeasurePlane::update (void) {

  //---------------------------------------------------------------------
  // Get the height plane, which we'll use to find the height at the
  // locations of the measure lines

  BCPlane * plane = d_dataset->inputGrid->getPlaneByName
                     (d_dataset->heightPlaneName->string());
  if (!plane) {
      fprintf(stderr, "nmui_HSMeasurePlane::update:  could not get plane!\n");
      return;
  }

  //---------------------------------------------------------------------
  // Find the location of the three measure lines (where they
  // intersect with the plane we are feeling on).

  q_vec_type    red, green, blue;

  d_decoration->red.getIntercept(red, plane);
  d_decoration->green.getIntercept(green, plane);
  d_decoration->blue.getIntercept(blue, plane);

  //---------------------------------------------------------------------
  // Pick one of the points (red) as the plane origin, then find the
  // normal using cross-products for the vectors to the other two
  // points. Make sure we get the up-pointing normal.

  q_vec_type    r_to_g, r_to_b;
  d_planePosPH[0] = red[0];
  d_planePosPH[1] = red[1];
  d_planePosPH[2] = red[2];

  q_vec_subtract(r_to_g, green, red);
  q_vec_subtract(r_to_b, blue, red);
  if ((q_vec_magnitude(r_to_g) == 0.0) || (q_vec_magnitude(r_to_b) == 0.0)) {
        fprintf(stderr,"nmui_HSMeasurePlane::update:  "
                "two measure lines are coincident\n");
        return;
  }
  q_vec_cross_product(d_currentPlaneNormal, r_to_g, r_to_b);
  if (d_currentPlaneNormal[2] < 0) {   // Flip it over if it is pointing down
        q_vec_scale(d_currentPlaneNormal, -1.0, d_currentPlaneNormal);
  }

  //---------------------------------------------------------------------
  // Rotate, Xlate and scale point from world space to ARM space.
  // The normal direction just needs rotating. Compute the plane
  // equation that corresponds to the given point and normal.

  pointToTrackerFromWorld(d_planePosPH, d_planePosPH);
  vectorToTrackerFromWorld(d_currentPlaneNormal, d_currentPlaneNormal);

  VectorNormalize(d_currentPlaneNormal);

  d_currentPlaneParameter = - q_vec_dot_product(d_currentPlaneNormal,
                                                d_planePosPH);

  computeDistanceFromPlane();

}







nmui_HSLivePlane::nmui_HSLivePlane (nmb_Dataset * dset,

#ifndef USE_VRPN_MICROSCOPE
                                    Microscope * scope) :
#else
                                    nmm_Microscope_Remote * scope) :
#endif

    nmui_HapticSurface (),
    d_dataset (dset),
    d_microscope (scope) {

  // Default normal is up.
  d_UP[0] = 0.0;
  d_UP[1] = 0.0;
  d_UP[2] = 0.0;
  q_vec_copy(d_lastNormal, d_UP);

}

// virtual
nmui_HSLivePlane::~nmui_HSLivePlane (void) {


}


// virtual
void nmui_HSLivePlane::update (void) {

  q_vec_type                 at;

  d_planePosPH[0] = d_microscope->state.data.inputPoint->x();
  d_planePosPH[1] = d_microscope->state.data.inputPoint->y();

  // Scale the Z value by the scale factor of the currently-displayed
  // data set.  XXX This assumes that the one mapped to height display is
  // also mapped in touch mode, and that mapping for this has been
  // set up.

  BCPlane * plane = d_dataset->inputGrid->getPlaneByName
                  (d_dataset->heightPlaneName->string());
  if (!plane) {
      fprintf(stderr, "nmui_HSLivePlane::update:  could not get plane!\n");
      return;
  }

  Point_value * value =
     d_microscope->state.data.inputPoint->getValueByPlaneName
                  (d_dataset->heightPlaneName->string());

  if (!value) {
      fprintf(stderr, "nmui_HSLivePlane::update:  could not get value!\n");
      return;
  }

  d_planePosPH[2] = value->value() * plane->scale();

  q_vec_copy(d_currentPlaneNormal, d_lastNormal);

  if (!d_initialized) {

    // start off with the force normal assumed straight up
    q_vec_copy(d_lastPoint, d_planePosPH);
    d_initialized = VRPN_TRUE;

  } else {

    // don't let the reuse of variables fool you. the normal is:
    // norm = ((p-last_p)X(0,0,1))X(p-last_p).
    // if we don't have a last point or haven't moved, leave the
    // normal direction alone.

    q_vec_subtract(at, d_planePosPH, d_lastPoint);
    if (10.0 < (at[0] * at[0] + at[1] * at[1])) {
        q_vec_cross_product(d_currentPlaneNormal, at, d_UP);
        q_vec_cross_product(d_currentPlaneNormal, d_currentPlaneNormal, at);
        q_vec_copy(d_lastPoint, d_planePosPH);
    }
  }

  VectorNormalize(d_currentPlaneNormal);

  q_vec_copy(d_lastNormal, d_currentPlaneNormal);
  d_initialized = VRPN_TRUE;

  // Got (x,y,z) and normal in microscope space;  XForm into ARM space.

  pointToTrackerFromWorld(d_planePosPH, d_planePosPH);
  vectorToTrackerFromWorld(d_currentPlaneNormal, d_currentPlaneNormal);

  VectorNormalize(d_currentPlaneNormal);

  d_currentPlaneParameter = - q_vec_dot_product(d_currentPlaneNormal,
                                                d_planePosPH);

  computeDistanceFromPlane();
}


