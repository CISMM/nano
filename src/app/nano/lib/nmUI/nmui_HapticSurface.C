#include "nmui_HapticSurface.h"

#include <quat.h>

#include <v.h>  // BUG - for VectorType

#include <vrpn_ForceDevice.h>

#include <BCPlane.h>  // needed by normal.h

#include <normal.h>  // BUG - replace with quatlib!

#include <nmb_Dataset.h>
#include <nmb_Decoration.h>

#include <nmm_MicroscopeRemote.h>
#include <nmm_Sample.h>

#include <microscape.h>  // for directz_force_scale

#include <nmg_Graphics.h>  // for FeelGrid diagnostics

nmui_HapticSurface::nmui_HapticSurface (nmg_Graphics * g) :
    d_distanceFromPlane (0.0),
    d_graphics (g) {

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

  //n[0] = d_currentPlaneNormal[0];
  //n[1] = d_currentPlaneNormal[1];
  //n[2] = d_currentPlaneNormal[2];
  q_vec_copy(n, (double *) d_currentPlaneNormal);
  *d = d_currentPlaneParameter;

}

// virtual
double nmui_HapticSurface::distanceFromSurface (void) const {
  return d_distanceFromPlane;
}


// virtual
void nmui_HapticSurface::setLocation (q_vec_type x) {

  //d_handPosMS[0] = x[0];
  //d_handPosMS[1] = x[1];
  //d_handPosMS[2] = x[2];
  q_vec_copy(d_handPosMS, x);

//fprintf(stderr, "nmui_HapticSurface::setLocation() to <%.5f, %.5f, %.5f>\n",
//x[0], x[1], x[2]);

  if (d_graphics) {
    d_graphics->setFeelPlane(d_handPosMS, d_currentPlaneNormal);
  }
}

// virtual
void nmui_HapticSurface::setLocation (double x, double y, double z) {

  //d_handPosMS[0] = x;
  //d_handPosMS[1] = y;
  //d_handPosMS[2] = z;
  q_vec_set(d_handPosMS, x, y, z);

  if (d_graphics) {
    d_graphics->setFeelPlane(d_handPosMS, d_currentPlaneNormal);
  }

//fprintf(stderr, "nmui_HapticSurface::setLocation() to <%.5f, %.5f, %.5f>\n",
//d_handPosMS[0], d_handPosMS[1], d_handPosMS[2]);
}

// virtual
void nmui_HapticSurface::sendForceUpdate (vrpn_ForceDevice_Remote * device) {
  if (device) {
    device->set_plane(d_currentPlaneNormal[0],
                      d_currentPlaneNormal[1],
                      d_currentPlaneNormal[2],
                      d_currentPlaneParameter);
  }
  if (d_graphics) {
    d_graphics->setFeelPlane(d_handPosMS, d_currentPlaneNormal);
  }
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

  pointToTrackerFromWorld(handInTracker, d_handPosMS);
  q_vec_subtract( planeToHandVector, handInTracker, d_planePosPH );

//fprintf(stderr, "planeToHandVector <%.2f, %.2f, %.2f>\n",
//planeToHandVector[0], planeToHandVector[1], planeToHandVector[2]);
//fprintf(stderr, "d_currentPlaneNormal <%.2f, %.2f, %.2f>\n",
//d_currentPlaneNormal[0], d_currentPlaneNormal[1], d_currentPlaneNormal[2]);

  d_distanceFromPlane = q_vec_dot_product(planeToHandVector,
                                           d_currentPlaneNormal);

//fprintf(stderr, "nmui_HapticSurface::computeDistanceFromPlane() "
//"set to %.5lf\n", d_distanceFromPlane);

}





nmui_HSCanned::nmui_HSCanned (nmg_Graphics * g) :
    nmui_HapticSurface (g) 
{

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
void nmui_HSCanned::update (nmm_Microscope_Remote * scope) {

  BCPlane * plane = scope->Data()->inputGrid->getPlaneByName
                     (scope->Data()->heightPlaneName->string());
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

//fprintf(stderr, "d_planePosPH is <%.5f, %.5f, %.5f>\n",
//d_planePosPH[0], d_planePosPH[1], d_planePosPH[2]);

  // BUG - uses normal.h but we ought to be using quatlib normals.
  // Comptue the normals at the four nearest grid points.

  Compute_Norm(d_gridX,   d_gridY,   Norm00, plane);
  Compute_Norm(d_gridX+1, d_gridY,   Norm10, plane);
  Compute_Norm(d_gridX+1, d_gridY+1, Norm11, plane);
  Compute_Norm(d_gridX,   d_gridY+1, Norm01, plane);

  // Average the four normals together with appropriate weighting.

  VectorScale(Norm00, (1-a) * (1-b));
  VectorCopy(d_currentPlaneNormal, Norm00);
  VectorScale(Norm10, (  a) * (1-b));
  VectorAdd(d_currentPlaneNormal, Norm10, d_currentPlaneNormal);
  VectorScale(Norm01, (1-a) * (  b));
  VectorAdd(d_currentPlaneNormal, Norm01, d_currentPlaneNormal);
  VectorScale(Norm11, (  a) * (  b));
  VectorAdd(d_currentPlaneNormal, Norm11, d_currentPlaneNormal);

  if (zero_xnorm) d_currentPlaneNormal[0] = 0.0;
  if (zero_ynorm) d_currentPlaneNormal[1] = 0.0;

  pointToTrackerFromWorld(d_planePosPH, d_planePosPH);
  vectorToTrackerFromWorld(d_currentPlaneNormal, d_currentPlaneNormal);

  VectorNormalize(d_currentPlaneNormal);

  d_currentPlaneParameter = - q_vec_dot_product(d_currentPlaneNormal,
                                                d_planePosPH);

  computeDistanceFromPlane();
}



nmui_HSMeasurePlane::nmui_HSMeasurePlane (nmb_Decoration * dec,
                                          nmg_Graphics * g) :
    nmui_HapticSurface (g),
    d_decoration (dec) {

}

// virtual
nmui_HSMeasurePlane::~nmui_HSMeasurePlane (void) {

}

// virtual
void nmui_HSMeasurePlane::update (nmm_Microscope_Remote * scope) {

  //---------------------------------------------------------------------
  // Get the height plane, which we'll use to find the height at the
  // locations of the measure lines

  BCPlane * plane = scope->Data()->inputGrid->getPlaneByName
                     (scope->Data()->heightPlaneName->string());
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

  // Scale the z value into world coordinates:
  red[2] *= plane->scale();
  green[2] *= plane->scale();
  blue[2] *= plane->scale();

  //---------------------------------------------------------------------
  // Pick one of the points (red) as the plane origin, then find the
  // normal using cross-products for the vectors to the other two
  // points. Make sure we get the up-pointing normal.

  q_vec_type    r_to_g, r_to_b;
  //d_planePosPH[0] = red[0];
  //d_planePosPH[1] = red[1];
  //d_planePosPH[2] = red[2];
  q_vec_copy(d_planePosPH, red);

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







nmui_HSLivePlane::nmui_HSLivePlane (nmg_Graphics * g) :
    nmui_HapticSurface (g)
{

  // Default normal is up.
  //d_UP[0] = 0.0;
  //d_UP[1] = 0.0;
  //d_UP[2] = 1.0;
  q_vec_set(d_UP, 0.0, 0.0, 1.0);
  q_vec_copy(d_lastNormalMS, d_UP);

}

// virtual
nmui_HSLivePlane::~nmui_HSLivePlane (void) {


}


// virtual
void nmui_HSLivePlane::update (nmm_Microscope_Remote * scope) {

  getSamplePosMS(scope);
  computeUpdate();
}

void nmui_HSLivePlane::getSamplePosMS (nmm_Microscope_Remote * scope) {

  d_samplePosMS[0] = scope->state.data.inputPoint->x();
  d_samplePosMS[1] = scope->state.data.inputPoint->y();

  // Scale the Z value by the scale factor of the currently-displayed
  // data set.  XXX This assumes that the one mapped to height display is
  // also mapped in touch mode, and that mapping for this has been
  // set up.

  BCPlane * plane = scope->Data()->inputGrid->getPlaneByName
                  (scope->Data()->heightPlaneName->string());
  if (!plane) {
      fprintf(stderr, "nmui_HSLivePlane::update:  could not get plane!\n");
      return;
  }

  Point_value * value =
     scope->state.data.inputPoint->getValueByPlaneName
                  (scope->Data()->heightPlaneName->string());

  if (!value) {
      fprintf(stderr, "nmui_HSLivePlane::update:  could not get value!\n");
      return;
  }

  d_samplePosMS[2] = value->value() * plane->scale();
}

void nmui_HSLivePlane::computeUpdate (void) {

  q_vec_type at;

  q_vec_copy(d_currentPlaneNormalMS, d_lastNormalMS);

  if (!d_initialized) {

    // start off with the force normal assumed straight up
    q_vec_copy(d_lastPointMS, d_samplePosMS);
    d_initialized = VRPN_TRUE;

  } else {

    // don't let the reuse of variables fool you. the normal is:
    // norm = ((p-last_p)X(0,0,1))X(p-last_p).
    // if we don't have a last point or haven't moved, leave the
    // normal direction alone.

    q_vec_subtract(at, d_samplePosMS, d_lastPointMS);
    if (10.0 < (at[0] * at[0] + at[1] * at[1])) {
        q_vec_cross_product(d_currentPlaneNormalMS, at, d_UP);
        q_vec_cross_product(d_currentPlaneNormalMS,
                            d_currentPlaneNormalMS, at);
        q_vec_copy(d_lastPointMS, d_samplePosMS);
    }
  }

  VectorNormalize(d_currentPlaneNormalMS);

  q_vec_copy(d_lastNormalMS, d_currentPlaneNormalMS);
  d_initialized = VRPN_TRUE;

  // Got (x,y,z) and normal in microscope space;  XForm into ARM space.

  pointToTrackerFromWorld(d_samplePosPH, d_samplePosMS);
  vectorToTrackerFromWorld(d_currentPlaneNormal, d_currentPlaneNormalMS);

  VectorNormalize(d_currentPlaneNormal);

  d_currentPlaneParameter = - q_vec_dot_product(d_currentPlaneNormal,
                                                d_samplePosPH);

/*
fprintf(stderr, "Plane is %.3f %.3f %.3f, %.3f\n",
      d_currentPlaneNormal[0],
      d_currentPlaneNormal[1],
      d_currentPlaneNormal[2],
      d_currentPlaneParameter);
fprintf(stderr, "SamplePosPH is %.3f %.3f %.3f\n",
      d_samplePosPH[0],
      d_samplePosPH[1],
      d_samplePosPH[2]);
*/
  computeDistanceFromPlane();
}






nmui_HSWarpedPlane::nmui_HSWarpedPlane (nmg_Graphics * g) :
    nmui_HSLivePlane (g),
    d_rttEstimate (0.0) {

}


// virtual
nmui_HSWarpedPlane::~nmui_HSWarpedPlane (void) {

}



// virtual
void nmui_HSWarpedPlane::update (nmm_Microscope_Remote * scope) {

  float totalTime;
  float networkFraction;
    ///< The (estimated) fraction of total system time to get this data that
    ///< was consumed by the network.

  q_vec_type phantomMovement;
  q_vec_type trueSamplePoint;

  getSamplePosMS(scope);

  // Find the vector to the current PHANTOM position from the
  // position where the sample was requested.

  q_vec_subtract(phantomMovement, d_handPosMS, d_samplePosMS);

  // Find the total time for this update.
  totalTime = vrpn_TimevalMsecs
     (vrpn_TimevalDiff(scope->state.data.inputPoint->timeReceived(),
                       scope->state.data.inputPoint->timeRequested()));

  // Estimated network round-trip time is d_rttEstimate.

  // Scale the movement vector by the ratio of rtt to system time.

  networkFraction = d_rttEstimate / totalTime;
  q_vec_scale(d_currentWarpVector, networkFraction, phantomMovement);

  // Add the scaled vector to the current (and last) point.

  q_vec_copy(trueSamplePoint, d_samplePosMS);
  q_vec_add(d_lastPointMS, d_lastPointMS, d_currentWarpVector);
  q_vec_add(d_samplePosMS, d_samplePosMS, d_currentWarpVector);

  // Compute the plane as if we were doing a standard LivePlane
  // at that point.

  computeUpdate();

  // Restore an unwarped value for d_lastPoint.

  q_vec_copy(d_lastPointMS, trueSamplePoint);
}

void nmui_HSWarpedPlane::setMicroscopeRTTEstimate (double t) {
  d_rttEstimate = t;
}






nmui_HSFeelAhead::nmui_HSFeelAhead (nmg_Graphics * g) :
    nmui_HapticSurface (g),
    d_device (NULL),
    d_microscope (NULL) {

  // Set up default sampling algorithm
  d_sampleAlgorithm.numx = 5;
  d_sampleAlgorithm.numy = 5;
  d_sampleAlgorithm.dx = 5.0;
  d_sampleAlgorithm.dy = 5.0;
  d_sampleAlgorithm.orientation = 0.0;
}

// virtual
nmui_HSFeelAhead::~nmui_HSFeelAhead (void) {

}

/** @function nmui_HSFeelAhead::distanceFromSurface
 * Returns distance from "surface", in this case from the
 * nearest point on the trimesh constructed from the last
 * complete set of samples.
 * TODO - currently returns 0.
 */

// virtual
double nmui_HSFeelAhead::distanceFromSurface (void) const {

  return 0.0;
}

/** @function nmui_HSFeelAhead::update
 * Instead of sending control to the Phantom every frame,
 * we send a new trimesh every time we get a complete set of samples
 * back from the microscope - qv updateModel().
 */

// virtual
void nmui_HSFeelAhead::update (nmm_Microscope_Remote * m) {

  d_microscope = m;

  // TODO:  update sample for current network conditions &
  // current scan size

  m->SetSampleMode(&d_sampleAlgorithm);
}

/** @function nmui_HSFeelAhead::sendForceUpdate
 * Instead of sending control to the Phantom every frame,
 * we send a new trimesh every time we get a complete set of samples
 * back from the microscope - qv updateModel().
 */

// virtual
void nmui_HSFeelAhead::sendForceUpdate (vrpn_ForceDevice_Remote * d) {

  d_device = d;
}



void nmui_HSFeelAhead::updateModel (void) {

  static q_vec_type vertices [100];

  Point_list * l;
  const Point_results * p, * p0, * p1, * p2;
  vrpn_int32 xside, yside, start;
  vrpn_int32 i, j, k;

  if (!d_microscope || !d_device) {
    // TODO
    // Mark this update as pending and submit it in update()/sendForceUpdate()
    // once we have all the data we need.
//fprintf(stderr, "Feelahead has ms %d, dev %d so deferring.\n",
//d_microscope, d_device);
    return;
  }

//fprintf(stderr, "FA updateModel\n");

  // Send a trimesh up to the Phantom
  // For now hackishly assumes we've got a sample grid.
  // Probably the nmm_Sample subclass should be responsible for
  // converting the Point_list into some sort of mesh, but we
  // probably don't want to use a BCGrid to hold it...

  // Send the vertices

  l = &d_microscope->state.data.receivedPointList;
  xside = d_microscope->state.data.receivedAlgorithm.numx;
  yside = d_microscope->state.data.receivedAlgorithm.numy;

  if (l->numEntries() != xside * yside) {
    fprintf(stderr, "nmui_HSFeelAhead::updateModel():  "
                    "Didn't get enough data from microscope\n"
                    "to reconstruct surface.\n");
    fprintf(stderr, "  Got %d entries for %d x %d.\n", l->numEntries(),
            xside, yside);
    return;
  }

  d_device->clearTrimesh();
  for (i = 0; i < l->numEntries(); i++) {
    q_vec_type qm, qph;
    p = l->entry(i);

    // T. Hudson Dec 2001
    // Convert to Phantom coordinates
    //d_device->setVertex(i, p->x(), p->y(), p->z());

    qm[0] = p->x();
    qm[1] = p->y();
    qm[2] = p->z();
    pointToTrackerFromWorld(qph, qm);
fprintf(stderr, " mx %.2f %.2f %.2f, px %.5f %.5f %.5f\n",
qm[0], qm[1], qm[2], qph[0], qph[1], qph[2]);
    d_device->setVertex(i, qph[0], qph[1], qph[2]);
  }

  // Find out the dimensions of the grid sent.  Note that x and y are
  // arbitrary axes, not necessarily aligned with x and y in world space
  // or any other space.

  //side = sqrt(l->numEntries());

  // Triangulate.
  // 0 - 1 - 2 - 3 - 4
  // | / | / | / | / |  triangles 0 / 1 | 2 / 3 | 4 / 5 | 6 / 7
  // 5 - 6 - 7 - 8 - 9
  // ...
  // Don't really consistently know handedness of all this...

  // border conditions
  if ((xside <= 1) || (yside <= 1)) {
    fprintf(stderr, "nmui_HSFeelAhead::updateModel:  grid too small.\n");
    return;
  }

  k = 0;
  for (i = 0; i < xside - 1; i++) {
    start = i * xside;
    for (j = 0; j < yside - 1; j++) {
      d_device->setTriangle(k++, start + j, start + j + 1, start + j + xside);
      d_device->setTriangle(k++, start + j + 1, start + j + xside + 1,
                            start + j + xside);
      // swapped order 15 Dec 2001, to no apparent effect...
      // below is (believed) right-handed, above left-handed
      d_device->setTriangle(k++, start + j, start + j + xside, start + j + 1);
      d_device->setTriangle(k++, start + j + 1, start + j + xside,
                            start + j + xside + 1);
//p0 = l->entry(start + j);
//p1 = l->entry(start + j + xside);
//p2 = l->entry(start + j + 1);
//fprintf(stderr, "    <%.2f %.2f %.2f> <%.2f %.2f %.2f> <%.2f %.2f %.2f>\n",
//p0->x(), p0->y(), p0->z(), p1->x(), p1->y(), p1->z(),
//p2->x(), p2->y(), p2->z());
    }
  }

  // Make the Phantom pay attention to what we've just done.

  // HACK
  // TCH Dissn Jan 2002
  //d_device->updateTrimeshChanges();

  // Dec 2001
  // Draw a grid on-screen so we can see if this is where it ought
  // to be.  (This uses microscope coordinates, not phantom coords,
  // but it's one more point in the process that we can verify.)
  // Hacked, of course.
  
  for (i = 0; (i < l->numEntries()) && (i < 100); i++) {
     vertices[i][0] = l->entry(i)->x();
     vertices[i][1] = l->entry(i)->y();
     vertices[i][2] = l->entry(i)->z();
  }

  if (d_graphics) {
    d_graphics->setFeelGrid(xside, yside, vertices);
  }

}

// static
int nmui_HSFeelAhead::newPointListReceivedCallback (void * userdata) {

  nmui_HSFeelAhead * it = (nmui_HSFeelAhead *) userdata;

  it->updateModel();

  return 0;
}




nmui_HSPseudoFA::nmui_HSPseudoFA (nmg_Graphics * g) :
    nmui_HapticSurface (g) {

}

// virtual
nmui_HSPseudoFA::~nmui_HSPseudoFA (void) {

}

// virtual
double nmui_HSPseudoFA::distanceFromSurface (void) const {
  // TODO
  return 0.0;
}

// MANIPULATORS


// virtual
void nmui_HSPseudoFA::update (nmm_Microscope_Remote * m) {

  d_microscope = m;

  // TODO:  update sample for current network conditions &
  // current scan size

  m->SetSampleMode(&d_sampleAlgorithm);
}

// virtual
void nmui_HSPseudoFA::sendForceUpdate (vrpn_ForceDevice_Remote * device) {

  static q_vec_type vertices [100];

  Point_list * l;
  q_vec_type p0, p1, p2, e0, e1;
  int v0, v1, v2;
  int i, j, k, start;
  int xside, yside;
  vrpn_bool in; 

  // The Phantom is at d_handPosMS.

  if (!d_microscope) {
    fprintf(stderr, "nmui_HSPseudoFA::sendForceUpdate:  no microscope!\n");
    return;
  }

  l = &d_microscope->state.data.receivedPointList;
  xside = d_microscope->state.data.receivedAlgorithm.numx;
  yside = d_microscope->state.data.receivedAlgorithm.numy;

  if ((xside <= 1) || (yside <= 1)) {
    fprintf(stderr, "nmui_HSPseudoFA::sendForceUpdate:  grid too small.\n");
    return;
  }

  if (l->numEntries() != xside * yside) {
    fprintf(stderr, "nmui_HSPseudoFA::sendForceUpdate():  "
                    "Didn't get enough data from microscope\n"
                    "to reconstruct surface.\n");
    fprintf(stderr, "  Got %d entries for %d x %d.\n", l->numEntries(),
            xside, yside);
    return;
  }

  // Find the closest facet.
  // Naive approach:  work in the XY plane.
  //   do 3 * # triangles half-plane tests
  //   (but then ordering matters!)

  // Slightly better approach?  Shoot ray and count edge-crossings
  // From www.ecse.rpi.edu/Homepages/wrf/research/geom/pnpoly.html
  // William Randolph Franklin, comp.graphics.algorithms FAQ

  // Each set of 4 points defines *2* triangles, so there's
  // semiduplicate code in this loop.  I don't move it into a function
  // because the "temporary variables" v0, v1, v2 get reused later.

  // All this is done in microscope space!

  k = 0;  // polygon number
  in = VRPN_FALSE;
  for (i = 0; (i < xside - 1) && !in; i++) {
    start = i * xside;
    for (j = 0; (j < yside - 1) && !in; j++) {

      v0 = start + j;
      v1 = start + j + xside;
      v2 = start + j + 1;

      in = testTriangle (v0, v1, v2);

      if (in) {
        // in triangle k!
        // force out of inner loop, fall out of outer loop and on to resolution
        break;
      } 

      // not in this triangle;  try the next one
      k++;

      v0 = start + j + 1;
      v1 = start + j + xside;
      v2 = start + j + xside + 1;

      in = testTriangle (v0, v1, v2);

      if (in) {
        // in triangle k!
        // force out of inner loop, fall out of outer loop and on to resolution
        break;
      } 

      // not in this triangle;  try the next one
      k++;
    }
  }

  // If all of those fail, you're outside the mesh...
  //   How do we handle that?

  if (!in) {

    // TODO
    // HACK
    // For now, just leave the last known plane active

  }

  // Send that plane back to the Phantom.
  // Relies on v0, v1, v2 being kept intact - we care about that,
  // and would have to regenerate them if we'd just kept track of k.

  if (in) {
    // Normal is cross product of two edges.
    // Parameter is dot product of any point on plane with normal.
    p0[0] = l->entry(v0)->x();
    p0[1] = l->entry(v0)->y();
    p0[2] = l->entry(v0)->z();
    p1[0] = l->entry(v1)->x();
    p1[1] = l->entry(v1)->y();
    p1[2] = l->entry(v1)->z();
    p2[0] = l->entry(v2)->x();
    p2[1] = l->entry(v2)->y();
    p2[2] = l->entry(v2)->z();

    pointToTrackerFromWorld(p0, p0);
    pointToTrackerFromWorld(p1, p1);
    pointToTrackerFromWorld(p2, p2);

    q_vec_subtract(e0, p1, p0);
    q_vec_subtract(e1, p2, p0);
    q_vec_cross_product(d_currentPlaneNormal, e0, e1);
    d_currentPlaneParameter = - q_vec_dot_product (d_currentPlaneNormal, p0);

    nmui_HapticSurface::sendForceUpdate(device);
  }

  // Draw a grid on-screen so we can see if this is where it ought
  // to be.  (This uses microscope coordinates, not phantom coords,
  // but it's one more point in the process that we can verify.)
  // Hacked, of course.
  
  for (i = 0; (i < l->numEntries()) && (i < 100); i++) {
     vertices[i][0] = l->entry(i)->x();
     vertices[i][1] = l->entry(i)->y();
     vertices[i][2] = l->entry(i)->z();
  }

  if (d_graphics) {
    d_graphics->setFeelGrid(xside, yside, vertices);
  }

}

    
vrpn_bool nmui_HSPseudoFA::testTriangle (int v0, int v1, int v2) {
  vrpn_bool in;

  in = testEdge (v0, v1, VRPN_FALSE);
  in = testEdge (v1, v2, in);
  in = testEdge (v2, v0, in);

  return in;
}

vrpn_bool nmui_HSPseudoFA::testEdge (int l, int m, vrpn_bool in) {
  Point_list * list;
  const Point_results * pl, * pm;

  list = &d_microscope->state.data.receivedPointList;
  pl = list->entry(l);
  pm = list->entry(m);

  // Shoot ray and count edge-crossings.
  // From www.ecse.rpi.edu/Homepages/wrf/research/geom/pnpoly.html
  // William Randolph Franklin, comp.graphics.algorithms FAQ

  if ((((pl->y() <= d_handPosMS[1]) && (d_handPosMS[1] < pm->y())) ||
       ((pm->y() <= d_handPosMS[1]) && (d_handPosMS[1] < pl->y()))) &&
      (d_handPosMS[0] < ((pm->x() - pl->x()) * (d_handPosMS[1] - pl->y())
                         / (pm->y() - pl->y()) + pl->x()))) {
    in = !in;
  }

  return in;
}


nmui_HSDirectZ::nmui_HSDirectZ (nmb_Dataset * dataset,
                                nmm_Microscope_Remote * scope,
                                nmg_Graphics * g) :
    nmui_HapticSurface (g),
    d_dataset (dataset),
    d_microscope (scope),
    d_force (0.0),
    d_validRadius (0.02) {

  d_UP[0] = 0.0;
  d_UP[1] = 0.0;
  d_UP[2] = 1.0;

}

// virtual
nmui_HSDirectZ::~nmui_HSDirectZ (void) {

}

    // MANIPULATORS

// virtual
void nmui_HSDirectZ::update (nmm_Microscope_Remote * scope) {
  BCPlane * plane;
  Point_value * value;
  Point_value * forcevalue;
  double currentForce;

  plane = scope->Data()->inputGrid->getPlaneByName
               (scope->Data()->heightPlaneName->string());

  value = scope->state.data.inputPoint->getValueByPlaneName
               (scope->Data()->heightPlaneName->string());

  if (!plane || !value) {
    fprintf(stderr, "nmui_HSDirectZ::update():  NULL value.\n");
    return;
  }

  d_planePosPH[0] = d_handPosMS[0];
  d_planePosPH[1] = d_handPosMS[1];
  d_planePosPH[2] = value->value() * plane->scale();

  // Get the current value of the internal sensor, which tells us
  // the force the tip is experiencing.

  forcevalue = scope->state.data.inputPoint->getValueByName
                       ("Internal Sensor");

  if (!forcevalue) {
    fprintf(stderr, "nmui_HSDirectZ::update():  NULL force value.\n");
    return;
  }

  currentForce = forcevalue->value();

  // Calculate the difference from the free-space value of
  // internal sensor recorded when we entered direct-z control
  d_force = currentForce
             - scope->state.modify.freespace_normal_force;


  // Got (x,y,z) and an up vector in microscope space;  XForm into ARM space.

  pointToTrackerFromWorld(d_planePosPH, d_planePosPH);
  vectorToTrackerFromWorld(d_currentPlaneNormal, d_UP);
}

// virtual
void nmui_HSDirectZ::sendForceUpdate (vrpn_ForceDevice_Remote * device) {

// The force should be a force-field, proportional to the difference between
// the free-space internal sensor and the current internal-sensor, scaled by
// the direct_z_force_scale, a tclvar float whose widget should be in the live
// modify controls dialog.

  double scale = directz_force_scale;

  if (device) {

    // This should be where the user is currently located.
    device->setFF_Origin(d_handPosMS[Q_X], d_handPosMS[Q_Y],
                         d_handPosMS[Q_Z]);

    // This is the force measured by the microscope, * scale factor,
    // * the direction (up = positive z).
    device->setFF_Force(d_force * d_currentPlaneNormal[Q_X] * scale,
                        d_force * d_currentPlaneNormal[Q_Y] * scale,
                        d_force * d_currentPlaneNormal[Q_Z] * scale);

    // Force field does not change as we move around.
    device->setFF_Jacobian(0,0,0,  0,0,0,  0,0,0);
    device->setFF_Radius(d_validRadius);

  }

}

