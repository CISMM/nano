#ifndef NMUI_HAPTIC_SURFACE_H
#define NMUI_HAPTIC_SURFACE_H

#include <quat.h>
#include <vrpn_Types.h>

class vrpn_ForceDevice_Remote;  // from <vrpn_ForceDevice.h>

class nmb_Dataset;  // from <nmb_Dataset.h>
class nmb_Decoration;  // from <nmb_Decoration.h>

class Microscope;  // from <Microscope.h>
class nmm_Microscope_Remote;  // from <nmm_MicroscopeRemote.h>
class nmg_Graphics;
#include <nmm_Sample.h>

class nmui_HapticSurface {

  public:

    nmui_HapticSurface (void);
    virtual ~nmui_HapticSurface (void) = 0;


    // ACCESSORS


    vrpn_bool isEnabled (void) const;

    virtual void getLocation (q_vec_type x) const;
      ///< Returns position last sent to setLocation(), in
      ///< microscope coordinates.

    virtual void getOutputPlane (q_vec_type n, double * d) const;
      ///< Returns the approximating plane *in Phantom coordinates*
      ///< for the position specified by the last setLocation() call.
      ///< Undefined if the current mode doesn't create
      ///< an approximating plane.

    virtual double distanceFromSurface (void) const;
      ///< Returns the distance from the position specified in
      ///< the last setLocation() call to the surface/approximating plane,
      ///< or 0 if the current mode doesn't create an approximating plane.


    // MANIPULATORS


    virtual void setLocation (q_vec_type);
    virtual void setLocation (double x, double y, double z);
      ///< Specify current location of the user's hand,
      ///< in microscope coordinates (?!).

    virtual void update (nmm_Microscope_Remote *) =0;
      ///< Recompute all output values (d_currentPlaneNormal and
      ///< d_currentPlaneParameter) based on current state.
      ///< Necessary because we assume things are dynamic and volatile;
      ///< we can't assume that nothing's changed since the last call
      ///< to setLocation() if we've serviced any devices since then.

    virtual void sendForceUpdate (vrpn_ForceDevice_Remote * device);
      ///< Sets up a force update for the indicated force device.
      ///< Default implementation calls device->set_plane() with
      ///< (d_currentPlaneNormal, d_currentPlaneParameter).  The caller
      ///< is responsible for calling send_surface() or start_surface()
      ///< as required.

    virtual void enable (void);
      ///< Turn (this) haptics on.

    virtual void disable (void);
      ///< Turn (this) haptics off.

  protected:

    static void pointToTrackerFromWorld (q_vec_type out, const q_vec_type in);
    static void vectorToTrackerFromWorld (q_vec_type out, const q_vec_type in);

    virtual void computeDistanceFromPlane (void);

    // Microscope-space coordinates

    q_vec_type d_handPosMS;
      ///< Position of the user's hand in microscope coordinates.

    // Phantom-space coordintes

    q_vec_type d_planePosPH;
      ///< Projection of d_handPosMS onto the surface being felt
      ///< in the coordinates of the haptic device.

    q_vec_type d_currentPlaneNormal;
    double d_currentPlaneParameter;
      ///< Plane equation for approximating plane to surface.

    double d_distanceFromPlane;
      ///< Distance from d_handPosMS to the current approximating plane
      ///< (or other surface representation).

    vrpn_bool d_enabled;

};

/** \class nmui_HSCanned
 * Lets the user feel the stored data in a BCPlane, doing "Phong haptic
 * shading" to interpolate the normal from the nearest grid locations.
 * Rewritten from touch_canned_from_plane() by Tom Hudson February 2000
 */

class nmui_HSCanned : public nmui_HapticSurface {

  public:

    nmui_HSCanned (void);
    virtual ~nmui_HSCanned (void);

    // ACCESSORS

    int getGridX (void) const;
    int getGridY (void) const;

    // MANIPULATORS

    virtual void update (nmm_Microscope_Remote *);

  protected:

    int d_gridX;
    int d_gridY;

};


/** \class nmui_HSMeasurePlane
 * Lets the user feel the plane defined by the contact points of the
 * three measure lines.
 * Rewritten from touch_flat_from_measurelines() by Tom Hudson February 2000
 */

class nmui_HSMeasurePlane : public nmui_HapticSurface {

  public:

    nmui_HSMeasurePlane ( nmb_Decoration *);
    virtual ~nmui_HSMeasurePlane (void);

    // MANIPULATORS

    virtual void update (nmm_Microscope_Remote *);

  protected:

    nmb_Decoration * d_decoration;

};


/** \class nmui_HSLivePlane
 * Feel the surface under the microscope tip, using an approximating plane
 * defined by the current sample, the previous sample, and an assumed up
 * vector.
 * Rewritten from touch_live_to_plane_fit_to_line() by Tom Hudson February 2000
 */

class nmui_HSLivePlane : public nmui_HapticSurface {

  public:

    nmui_HSLivePlane (void);

    virtual ~nmui_HSLivePlane (void);

    // MANIPULATORS

    virtual void update (nmm_Microscope_Remote *);
      ///< Calls getSamplePosMS(), then computeUpdate().

  protected:

    void getSamplePosMS (nmm_Microscope_Remote *);
      ///< Sets d_samplePosMS.
    void computeUpdate (void);
      ///< Sets d_initialized, d_lastPointMS, d_lastNormal,
      ///< d_currentPlaneNormal, d_currentPlaneParameter,
      ///< d_samplePosPH, and calls computeDistanceFromPlane().

    q_vec_type d_UP;  // would like to be const but can't initialize

    vrpn_bool d_initialized;
    q_vec_type d_lastNormalMS;
    q_vec_type d_currentPlaneNormalMS;

    q_vec_type d_samplePosMS;
      ///< Position of the sample point being used in microscope coordinates.
    q_vec_type d_lastPointMS;
      ///< Previous sample point used.
    q_vec_type d_samplePosPH;
      ///< Sample point used in Phantom coordinates.

};


/** \class nmui_HSWarpedPlane
 * Feel the surface under the microscope tip, using an approximating plane
 * defined by the current sample, the previous sample, and an assumed up
 * vector.  Compensate for measured latency by shifting position at which
 * the plane is displayed.
 * Tom Hudson February 2001
 */

class nmui_HSWarpedPlane : public nmui_HSLivePlane {

  public:

    nmui_HSWarpedPlane (void);

    virtual ~nmui_HSWarpedPlane (void);

    // MANIPULATORS

    virtual void update (nmm_Microscope_Remote *);
      ///< Wraps extra behavior around getPlanePosPH() and computeUpdate().

    void setMicroscopeRTTEstimate (double);

  protected:

    double d_rttEstimate;
      ///< Current estimate of the network round-trip time we're
      ///< compensating for.

    q_vec_type d_currentWarpVector;
};



/** \class nmui_HSFeelAhead
 * Feels from an interpolated approximation to a set of point results.
 * Written by Tom Hudson February 2000.
 */

class nmui_HSFeelAhead : public nmui_HapticSurface {

  public:

    nmui_HSFeelAhead (nmg_Graphics * = NULL);
      ///< Set of point results used is
      ///< microscope->state.data.receivedPointList.

    virtual ~nmui_HSFeelAhead (void);

    virtual double distanceFromSurface (void) const;
      ///< Return distance from last sampled surface area.
      ///< TODO - currently returns 0.

    // MANIPULATORS

    virtual void update (nmm_Microscope_Remote *);
      ///< Do nothing - handled asynchronously by updateModel().

    virtual void sendForceUpdate (vrpn_ForceDevice_Remote *);
      ///< Do nothing - handled asynchronously by updateModel().


    static int newPointListReceivedCallback (void *);
      ///< To be registered on the microscope.

  protected:

    void updateModel (void);

    vrpn_ForceDevice_Remote * d_device;
    nmm_Microscope_Remote * d_microscope;
    nmg_Graphics * d_graphics;

    nmm_Sample d_sampleAlgorithm;

};


class nmui_HSDirectZ : public nmui_HapticSurface {

  public:

    nmui_HSDirectZ (nmb_Dataset *, nmm_Microscope_Remote *);

    virtual ~nmui_HSDirectZ (void);

    // MANIPULATORS

    virtual void update (nmm_Microscope_Remote *);
    virtual void sendForceUpdate (vrpn_ForceDevice_Remote * device);

  protected:

    nmb_Dataset * d_dataset;
    nmm_Microscope_Remote * d_microscope;

    q_vec_type d_UP;

    double d_force;
    float d_validRadius;
      ///< Radius over which the force field is valid, in m.

};




#endif  // NMUI_HAPTIC_SURFACE_H
