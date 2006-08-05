#include "nmui_HapticSurface.h"

#include <quat.h>

#include <v.h>  // BUG - for VectorType

#include <vrpn_ForceDevice.h>

#include <BCPlane.h>  // needed by normal.h

#include "normal.h"  // BUG - replace with quatlib!

#include <nmb_Dataset.h>
#include <nmb_Decoration.h>

#include <nmm_MicroscopeRemote.h>
#include <nmm_Sample.h>

#include "microscape.h"  // for directz_force_scale

//JM 
#include <nmg_haptic_graphics.h>


nmui_HapticSurface::nmui_HapticSurface (void) :
    d_distanceFromPlane (0.0) {

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

//fprintf(stderr, "nmui_HapticSurface::setLocation() to <%.5f, %.5f, %.5f>\n",
//x[0], x[1], x[2]);

}

// virtual
void nmui_HapticSurface::setLocation (double x, double y, double z) {

  d_handPosMS[0] = x;
  d_handPosMS[1] = y;
  d_handPosMS[2] = z;

//fprintf(stderr, "nmui_HapticSurface::setLocation() to <%.5f, %.5f, %.5f>\n",
//d_handPosMS[0], d_handPosMS[1], d_handPosMS[2]);
}

// virtual
void nmui_HapticSurface::sendForceUpdate (vrpn_ForceDevice_Remote * device) {

    q_vec_type currentPlaneNormalMS;
    vectorToWorldFromTracker(currentPlaneNormalMS, d_currentPlaneNormal);
    
    haptic_graphics->setFeelPlane(d_handPosMS, currentPlaneNormalMS);
    
    //TODO: find absolute number
    static q_vec_type vertices [50];
    
    vrpn_int32 xside, yside;
    vrpn_int32 i, j;
    
    BCPlane * plane = dataset->inputGrid->getPlaneByName
        (dataset->heightPlaneName->string());
	
	xside = 2;
	yside = 2;
    int xsize = 6;
    int ysize = 6;

	int numEntries = (xsize) * (ysize);

    d_planePosPH[0] = d_handPosMS[0];
    d_planePosPH[1] = d_handPosMS[1];
    
    // Translate microscope-space coordinates to (integer and fractional)
    // grid position.
    
    double rangeX = plane->maxX() - plane->minX();
    double rangeY = plane->maxY() - plane->minY();
    
    double rx = ((d_handPosMS[0] - plane->minX()) / rangeX * (plane->numX() - 1));
    double ry = ((d_handPosMS[1] - plane->minY()) / rangeY * (plane->numY() - 1));
    


	int current_x = plane->xInGrid(d_handPosMS[0]);
	int current_y = plane->yInGrid(d_handPosMS[1]);
	int current_z_value = plane->valueInWorld(current_x,current_y);
	
	double i_temp, j_temp, value_temp;
	q_vec_type qm;
    int s =0;
    for (i = -xside; i <= (xside + 1); i++) {
        i_temp = plane->xInWorld(current_x + i);
        for (j = -yside; j <= (yside + 1); j++) {
            j_temp = plane->xInWorld(current_y + j);
            plane->valueAt(&value_temp,i_temp,j_temp);
            
            qm[0] = i_temp;
            qm[1] = j_temp;
            qm[2] = value_temp;
            
            q_vec_copy(vertices[s], qm);
            s++;
        }
    }
    haptic_graphics->setFeelGrid(xsize,ysize,vertices);    
    
    
    if (device) {
        device->set_plane(d_currentPlaneNormal[0],
            d_currentPlaneNormal[1],
            d_currentPlaneNormal[2],
            d_currentPlaneParameter);
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



//JM from TCH branch 11/02
// static
void nmui_HapticSurface::pointToWorldFromTracker (q_vec_type out,
                                                  const q_vec_type in) {
  v_xform_type WorldFromTracker;

  v_x_compose(&WorldFromTracker,
          &v_world.users.xforms[0],
          &v_users[0].xforms[V_ROOM_FROM_HAND_TRACKER]);


  v_x_xform_vector(out, &WorldFromTracker, (double *) in);

}

//JM from TCH branch 11/02
// static
void nmui_HapticSurface::vectorToWorldFromTracker (q_vec_type out,
                                                   const q_vec_type in) {
  v_xform_type WorldFromTracker;

  v_x_compose(&WorldFromTracker,
          &v_world.users.xforms[0],
          &v_users[0].xforms[V_ROOM_FROM_HAND_TRACKER]);

  q_xform(out, WorldFromTracker.rotate, (double *) in);
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





nmui_HSCanned::nmui_HSCanned () :
    nmui_HapticSurface () 
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
void nmui_HSCanned::update (nmb_Dataset * dataset, nmm_Microscope_Remote * scope) {
	if(use_grid_mesh == 1) {
		updateModel(dataset);
		return;
	} 

  BCPlane * plane = dataset->inputGrid->getPlaneByName
                     (dataset->heightPlaneName->string());
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

    //JM
    q_vec_type currentPlaneNormalMS;
    vectorToWorldFromTracker(currentPlaneNormalMS, d_currentPlaneNormal);
        
    haptic_graphics->setFeelPlane(d_handPosMS, currentPlaneNormalMS);
}



nmui_HSMeasurePlane::nmui_HSMeasurePlane (nmb_Decoration * dec) :
    nmui_HapticSurface (),
    d_decoration (dec) {

}

// virtual
nmui_HSMeasurePlane::~nmui_HSMeasurePlane (void) {

}

// virtual
void nmui_HSMeasurePlane::update (nmb_Dataset * dataset, nmm_Microscope_Remote * scope) {

  //---------------------------------------------------------------------
  // Get the height plane, which we'll use to find the height at the
  // locations of the measure lines

  BCPlane * plane = dataset->inputGrid->getPlaneByName
                     (dataset->heightPlaneName->string());
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







nmui_HSLivePlane::nmui_HSLivePlane () :
    nmui_HapticSurface ()
{

  // Default normal is up.
  d_UP[0] = 0.0;
  d_UP[1] = 0.0;
  d_UP[2] = 1.0;
  q_vec_copy(d_lastNormal, d_UP);

}

// virtual
nmui_HSLivePlane::~nmui_HSLivePlane (void) {


}


// virtual
void nmui_HSLivePlane::update (nmb_Dataset * dataset, nmm_Microscope_Remote * scope) {

  q_vec_type                 at;

  d_planePosPH[0] = scope->state.data.inputPoint->x();
  d_planePosPH[1] = scope->state.data.inputPoint->y();

  // Scale the Z value by the scale factor of the currently-displayed
  // data set.  XXX This assumes that the one mapped to height display is
  // also mapped in touch mode, and that mapping for this has been
  // set up.

  BCPlane * plane = dataset->inputGrid->getPlaneByName
                  (dataset->heightPlaneName->string());
  if (!plane) {
      fprintf(stderr, "nmui_HSLivePlane::update:  could not get plane!\n");
      return;
  }

  Point_value * value =
     scope->state.data.inputPoint->getValueByPlaneName
                  (dataset->heightPlaneName->string());

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




nmui_HSFeelAhead::nmui_HSFeelAhead (void) :
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
void nmui_HSFeelAhead::update (nmb_Dataset * dataset, nmm_Microscope_Remote * m) {

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
  Point_list * l;
  const Point_results * p;
  vrpn_int32 xside, yside, start;
  vrpn_int32 i, j, k;

  if (!d_microscope || !d_device) {
    // TODO
    // Mark this update as pending and submit it in update()/sendForceUpdate()
    // once we have all the data we need.
    return;
  }

  // Send a trimesh up to the Phantom
  // For now hackishly assumes we've got a sample grid.
  // Probably the nmm_Sample subclass should be responsible for
  // converting the Point_list into some sort of mesh, but we
  // probably don't want to use a BCGrid to hold it...

  // Send the vertices

  l = &d_microscope->state.data.receivedPointList;
  for (i = 0; i < l->numEntries(); i++) {
    p = l->entry(i);
    d_device->setVertex(i, p->x(), p->y(), p->z());
  }

  // Find out the dimensions of the grid sent.  Note that x and y are
  // arbitrary axes, not necessarily aligned with x and y in world space
  // or any other space.

  //side = sqrt(l->numEntries());
  xside = d_microscope->state.data.receivedAlgorithm.numx;
  yside = d_microscope->state.data.receivedAlgorithm.numy;

  // Triangulate.
  // 0 - 1 - 2 - 3 - 4
  // | / | / | / | / |  triangles 0 / 1 | 2 / 3 | 4 / 5 | 6 / 7
  // 5 - 6 - 7 - 8 - 9
  // ...

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
    }
  }

  // Make the Phantom pay attention to what we've just done.

  d_device->updateTrimeshChanges();

}

// static
int nmui_HSFeelAhead::newPointListReceivedCallback (void * userdata) {

  nmui_HSFeelAhead * it = (nmui_HSFeelAhead *) userdata;

  it->updateModel();

  return 0;
}






nmui_HSDirectZ::nmui_HSDirectZ (nmb_Dataset * dataset,
                                nmm_Microscope_Remote * scope) :
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
void nmui_HSDirectZ::update (nmb_Dataset * dataset, nmm_Microscope_Remote * scope) {
  BCPlane * plane;
  Point_value * value;
  Point_value * forcevalue;
  double currentForce;

  plane = dataset->inputGrid->getPlaneByName
               (dataset->heightPlaneName->string());

  value = scope->state.data.inputPoint->getValueByPlaneName
               (dataset->heightPlaneName->string());

  if (!plane || !value) {
    fprintf(stderr, "nmui_HSDirectZ::update():  NULL value.\n");
    return;
  }

  d_planePosPH[0] = d_handPosMS[0];
  d_planePosPH[1] = d_handPosMS[1];
  d_planePosPH[2] = value->value() * plane->scale();

  // Get the current value of the error signal, which tells us
  // the force the tip is experiencing.

  const char *error_channel;
  if (nmb_MicroscopeFlavor == Topometrix) {
    error_channel = "Internal Sensor";
  } else if (nmb_MicroscopeFlavor == Asylum) {
    if (scope->state.image.mode == TAPPING) {
      error_channel = "Amplitude";
    } else {
      error_channel = "Deflection";
    }
  } else {
    fprintf(stderr,"nmui_HSDirectZ::update(): Unknown microscope flavor\n");
  }
  forcevalue = scope->state.data.inputPoint->getValueByName(error_channel);

  if (!forcevalue) {
    fprintf(stderr, "nmui_HSDirectZ::update():  NULL force value.\n");
    return;
  }

  currentForce = forcevalue->value();

  // Calculate the difference from the free-space value of
  // error recorded when we entered direct-z control
  d_force = currentForce
             - scope->state.modify.freespace_normal_force;


  // Got (x,y,z) and an up vector in microscope space;  XForm into ARM space.

  pointToTrackerFromWorld(d_planePosPH, d_planePosPH);
  vectorToTrackerFromWorld(d_currentPlaneNormal, d_UP);
}

// virtual
void nmui_HSDirectZ::sendForceUpdate (vrpn_ForceDevice_Remote * device) {

// The force should be a force-field, proportional to the difference between
// the free-space error signal and the current internal-sensor, scaled by
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


nmui_HSDirectZPlane::nmui_HSDirectZPlane (nmb_Dataset * dataset, nmb_Decoration * dec) :
nmui_HapticSurface (),
d_dataset (dataset),
d_decoration(dec) {
}

nmui_HSDirectZPlane::~nmui_HSDirectZPlane(void) {
	
}

//this mode is used when we have exceeded the force setpoint limit in direct Z
void nmui_HSDirectZPlane::update(nmb_Dataset * dataset, nmm_Microscope_Remote * scope) {
	
	// set up force plane whose origin is at the last point set by directZ
	// and whose normal is up
	BCPlane * plane = dataset->inputGrid->getPlaneByName
                     (dataset->heightPlaneName->string());
  if (!plane) {
      fprintf(stderr, "nmui_HSDirectZPlane::update:  could not get plane!\n");
      return;
  }

  //force is up
  q_vec_type		up = { 0.0, 0.0, 1.0 };
  
  if(offset > 0) { offset = offset - 5; }

  d_planePosPH[0] = last_point[0];
  d_planePosPH[1] = last_point[1];
  d_planePosPH[2] = last_point[2] - offset;

 //---------------------------------------------------------------------
  // Rotate, Xlate and scale point from world space to ARM space.
  // The normal direction just needs rotating. Compute the plane
  // equation that corresponds to the given point and normal.

  pointToTrackerFromWorld(d_planePosPH, d_planePosPH);
  vectorToTrackerFromWorld(d_currentPlaneNormal, up);

  VectorNormalize(d_currentPlaneNormal);

  d_currentPlaneParameter = - q_vec_dot_product(d_currentPlaneNormal,
                                                d_planePosPH);

  computeDistanceFromPlane();
  

}

void nmui_HSDirectZPlane::set_direct_z_plane(double x, double y, double z) {

	BCPlane * plane = dataset->inputGrid->getPlaneByName
		(dataset->heightPlaneName->string());
	if (!plane) {
		fprintf(stderr, "nmui_HSDirectZPlane::update:  could not get plane!\n");
		return;
	}
	
	last_point[0] = x;
	last_point[1] = y;
	last_point[2] = z;

	offset = 50;
	
}

//for feeling from a grid. send points to the phantom, and
//to graphics to let it know what to draw.
void nmui_HSCanned::updateModel(nmb_Dataset * dataset) {
	
    vrpn_int32 xside, yside, start;
    vrpn_int32 i, j, k;
    
    BCPlane * plane = dataset->inputGrid->getPlaneByName
        (dataset->heightPlaneName->string());
    

    static q_vec_type vertices [50];
    xside = 2;
	yside = 2;
    int xsize = 6;
    int ysize = 6;

	int numEntries = (xsize) * (ysize);

    d_planePosPH[0] = d_handPosMS[0];
    d_planePosPH[1] = d_handPosMS[1];
    
    // Translate microscope-space coordinates to (integer and fractional)
    // grid position.
    
    double rangeX = plane->maxX() - plane->minX();
    double rangeY = plane->maxY() - plane->minY();
    
    double rx = ((d_handPosMS[0] - plane->minX()) / rangeX * (plane->numX() - 1));
    double ry = ((d_handPosMS[1] - plane->minY()) / rangeY * (plane->numY() - 1));
    


	int current_x = plane->xInGrid(d_handPosMS[0]);
	int current_y = plane->yInGrid(d_handPosMS[1]);
	int current_z_value = plane->valueInWorld(current_x,current_y);

	if (!d_device) {
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
	
	
	d_device->clearTrimesh();
	
	double i_temp, j_temp, value_temp;
	q_vec_type qm,qph;
    int s =0;
    for (i = -xside; i <= (xside + 1); i++) {
        i_temp = plane->xInWorld(current_x + i);
        for (j = -yside; j <= (yside + 1); j++) {
            j_temp = plane->xInWorld(current_y + j);
            plane->valueAt(&value_temp,i_temp,j_temp);
            
            qm[0] = i_temp;
            qm[1] = j_temp;
            qm[2] = value_temp;
            
            q_vec_copy(vertices[s], qm);
            s++;

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
		for (i = 0; i < xsize; i++) {
			start = i * xsize;
			for (j = 0; j < ysize; j++) {
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
		d_device->updateTrimeshChanges();
		
		// Dec 2001
		// Draw a grid on-screen so we can see if this is where it ought
		// to be.  (This uses microscope coordinates, not phantom coords,
		// but it's one more point in the process that we can verify.)
		// Hacked, of course.
		
		/*
		for (i = 0; (i < numEntries) && (i < 100); i++) {
		vertices[i][0] = l->entry(i)->x();
		vertices[i][1] = l->entry(i)->y();
		vertices[i][2] = l->entry(i)->z();
		}
		/*
		  if (d_graphics) {
		  d_graphics->setFeelGrid(xside, yside, vertices);
		  }
          */
        haptic_graphics->setFeelGrid(xsize,ysize,vertices);
          
	}
}

void nmui_HSCanned::sendForceUpdate(vrpn_ForceDevice_Remote * device) {

	if(use_grid_mesh == 1) {
		d_device = device;
	} else {
		nmui_HapticSurface::sendForceUpdate(device);
	}

}
