#ifndef NMUI_HAPTIC_SURFACE_H
#define NMUI_HAPTIC_SURFACE_H

#include <vrpn_Types.h>
#include <quat.h>

class nmb_Dataset;  // from <nmb_Dataset.h>
class nmb_Decoration;  // from <nmb_Decoration.h>

class Microscope;  // from <Microscope.h>
class nmm_Microscope_Remote;  // from <nmm_MicroscopeRemote.h>

class nmui_HapticSurface {

  public:

    nmui_HapticSurface (void);
    virtual ~nmui_HapticSurface (void) = 0;


    // ACCESSORS


    vrpn_bool isEnabled (void) const;

    virtual void getLocation (q_vec_type x) const;
      /**< Returns position last sent to setLocation(), in
       * microscope coordinates.
       */

    virtual void getOutputPlane (q_vec_type n, double * d)
                       const;
      /**< Returns the approximating plane *in Phantom coordinates*
       * for the position specified by
       * the last setLocation() call.
       */

    virtual double distanceFromSurface (void) const;


    // MANIPULATORS


    virtual void setLocation (q_vec_type);
    virtual void setLocation (double x, double y, double z);
      /**< Specify current location of the user's hand,
       * in microscope coordinates (?!).
       */

    virtual void update (void) = 0;
      /**< Recompute all output values based on current state.
       * Necessary because we assume things are dynamic and volatile;
       * we can't assume that nothing's changed since the last call
       * to setLocation() if we've serviced any devices since then.
       */

    virtual void enable (void);
      /**< Turn (this) haptics on. */

    virtual void disable (void);
      /**< Turn (this) haptics off. */

  protected:

    static void pointToTrackerFromWorld (q_vec_type out, const q_vec_type in);
    static void vectorToTrackerFromWorld (q_vec_type out, const q_vec_type in);

    virtual void computeDistanceFromPlane (void);

    // Microscope-space coordinates

    q_vec_type d_handPosMS;
      /**< Position of the user's hand in microscope coordinates. */

    // Phantom-space coordintes

    q_vec_type d_planePosPH;
      /**< Projection of d_handPosMS onto the surface being felt
       * in the coordinates of the haptic device.
       */

    q_vec_type d_currentPlaneNormal;
    double d_currentPlaneParameter;
      /**< Plane equation for approximating plane to surface. */

    double d_distanceFromPlane;
      /**< Distance from d_handPosMS to the current approximating plane
       * (or other surface representation).
       */

    vrpn_bool d_enabled;

};

/** \class nmui_HSCanned
 * Lets the user feel the stored data in a BCPlane, doing "Phong haptic
 * shading" to interpolate the normal from the nearest grid locations.
 * Rewritten from touch_canned_from_plane() by Tom Hudson February 2000
 */

class nmui_HSCanned : public nmui_HapticSurface {

  public:

    nmui_HSCanned (nmb_Dataset *);
    virtual ~nmui_HSCanned (void);

    // ACCESSORS

    int getGridX (void) const;
    int getGridY (void) const;

    // MANIPULATORS

    virtual void update (void);

  protected:

    nmb_Dataset * d_dataset;

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

    nmui_HSMeasurePlane (nmb_Dataset *, nmb_Decoration *);
    virtual ~nmui_HSMeasurePlane (void);

    // MANIPULATORS

    virtual void update (void);

  protected:

    nmb_Dataset * d_dataset;
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

#ifndef USE_VRPN_MICROSCOPE
    nmui_HSLivePlane (nmb_Dataset *, Microscope *);
#else
    nmui_HSLivePlane (nmb_Dataset *, nmm_Microscope_Remote *);
#endif

    virtual ~nmui_HSLivePlane (void);

    // MANIPULATORS

    virtual void update (void);

  protected:

    q_vec_type d_UP;  // would like to be const but can't initialize

    vrpn_bool d_initialized;
    q_vec_type d_lastPoint;
    q_vec_type d_lastNormal;

    nmb_Dataset * d_dataset;

#ifndef USE_VRPN_MICROSCOPE
    Microscope * d_microscope;
#else
    nmm_Microscope_Remote * d_microscope;
#endif
};


/** \class nmui_HSFeelAhead
 * Feels from an interpolated approximation to a set of point results.
 * Written by Tom Hudson February 2000.
 */

class nmui_HSFeelAhead : public nmui_HapticSurface {

  public:

#ifndef USE_VRPN_MICROSCOPE
    nmui_HSFeelAhead (nmb_Dataset *, Microscope *);
#else
    nmui_HSFeelAhead (nmb_Dataset *, nmm_Microscope_Remote *);
#endif
      ///< Set of point results used is
      ///< microscope->state.data.receivedPointList.

    virtual ~nmui_HSFeelAhead (void);

    // MANIPULATORS

    virtual void update (void);



  protected:

    void updateModel (void);
    static void newPointListReceivedCallback (void *);
    ///< Registered on the microscope.

    nmb_Dataset * d_dataset;
#ifndef USE_VRPN_MICROSCOPE
    Microscope * d_microscope;
#else
    nmm_Microscope_Remote * d_microscope;
#endif

};




#endif  // NMUI_HAPTIC_SURFACE_H
