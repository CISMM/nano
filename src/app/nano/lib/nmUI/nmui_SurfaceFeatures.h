#ifndef NMUI_SURFACE_FEATURES_H
#define NMUI_SURFACE_FEATURES_H

#include <vrpn_Types.h>  // for vrpn_bool

class nmb_Dataset;  // from <nmb_Dataset.h>
class nmm_Microscope_Remote;  // from <nmm_MicroscopeRemote.h>
class nmui_HSCanned;  // from "nmui_HapticSurface.h"

// Second pass design:
//   Put Point, Grid onto strategy classes & run them behind the scenes.
//   Make nmui_SurfaceFeatures concrete and instantiate it.
// * Still need a better way to instantiate & select strategy classes.
// * We need buzzingEnabled(), bumpsEnabled() because the "magic numbers"
//   that get thrown at them may include an offset as well as a scale,
//   so passing 0 back from scaledPointBuzzAmplitude() *doesn't* guarantee
//   that buzzing will be off if magicBuzzAmplitude() is obnoxious enough.


class nmui_SurfaceFeatureStrategy {

  public:

    nmui_SurfaceFeatureStrategy (void);
    virtual ~nmui_SurfaceFeatureStrategy (void) = 0;

    // ACCESSORS

    virtual vrpn_bool buzzingEnabled (void) const;
      ///< Default implementation returns vrpn_false;
    virtual vrpn_bool bumpsEnabled (void) const;
      ///< Default implementation returns vrpn_false;

    // MANIPULATORS
    // These could be declared const, but I don't want to constrain
    // future implementations.

    virtual double pointAdhesionValue (void);
      ///< Default implementation returns 0.
    virtual double scaledPointBuzzAmplitude (void);
      ///< Default implementation returns 0.
    virtual double scaledPointBumpSizeValue (void);
      ///< Default implementation returns 0.
    virtual double pointComplianceValue (void);
      ///< Default implementation returns the conventional default/error value:
      ///< max(0.0, Arm_knobs[FORCE_KNOB]).
    virtual double pointFrictionValue (void);
      ///< Default implementation returns the conventional default/error value:
      ///< max(0.0, Arm_knobs[FRICTION_KNOB]).

    virtual double magicBuzzAmplitude (double) = 0;
      ///< Scale bump wavelength by context-dependent magic numbers.
    virtual double magicBumpSize (double) = 0;
      ///< Scale bump wavelength by context-dependent magic numbers.

    virtual double dynamicFrictionKspring (double staticFrictionKspring);
      ///< Default implementation returns 0.5 * kSpring.



};


class nmui_SurfaceFeatures {

  public:

    nmui_SurfaceFeatures (void);
    virtual ~nmui_SurfaceFeatures (void);

    // ACCESSORS




    // MANIPULATORS

    void update (void);

    void setSurfaceFeatureStrategy (nmui_SurfaceFeatureStrategy *);

    void useLinearBuzzing (vrpn_bool);
    void useLinearBumps (vrpn_bool);
    void useLinearFriction (vrpn_bool);


    void setBuzzFrequency (double = 115.0);
      ///< Defaults to 115.0 Hz.
    void setComplianceGain (double = 1.3);
      ///< Compensate for nonunity gain in the Phantom server -
      ///< defaults to 1.3.
    void setAdhesionConstant (double = 12000.0);
      ///< Defaults to 12,000.

  protected:

    void specifyAdhesion (void);
    void specifyBuzzAmplitude (void);
    void specifyBumpSize (void);
    void specifyCompliance (void);
    void specifyFriction (void);
      ///< Takes the local value of friction, scales it with respect
      ///< to friction_slider, linearizes it if necessary, and passes
      ///< the result to setStaticFriction() and setDynamicFriction().

    virtual void setFriction (double kS, double dynamic_kS);
      ///< Default implementation calls forceDevice->setSurfaceFstatic(kS)
      ///< and setSurfaceFdynamic(dynamic_kS).
    void setSurfaceTexture (double wavelength);
      ///< Calls forceDevice->setSurfaceTextureWavelength(wavelength)
      ///< and setSurfaceTextureAmplitude(0.1 * wavelength).
    void setBuzzing (double amp);
      ///< Calls forceDevice->setSurfaceBuzzFrequency(d_buzzFrequency)
      ///< and setSurfaceBuzzAmplitude(amp).
    void setCompliance (double kS);
      ///< Calls forceDevice->setSurfaceKspring(kS).


    vrpn_bool d_firstExecution;
    vrpn_bool d_useLinearBuzzing;
    vrpn_bool d_useLinearBumps;
    vrpn_bool d_useLinearFriction;

    double d_adhesionConstant;
    double d_buzzFrequency;
    double d_complianceGain;

    nmui_SurfaceFeatureStrategy * d_strategy;

};

/// @class nmui_PointFeatures
/// Used by touch_live_to_plane_fit_to_line and anything else that wants
/// to make the surface feel like it does at a point returned by
/// the microscope.

class nmui_PointFeatures : public nmui_SurfaceFeatureStrategy {

  public:

    nmui_PointFeatures (nmm_Microscope_Remote * scope);
    virtual ~nmui_PointFeatures (void);

    // ACCESSORS

    virtual vrpn_bool buzzingEnabled (void) const;
    virtual vrpn_bool bumpsEnabled (void) const;

    // MANIPULATORS

    virtual double pointAdhesionValue (void);
      ///< Looks up the adhesion in microscope->state.data.inputPoint.
    virtual double scaledPointBuzzAmplitude (void);
      ///< Looks up the buzz size in microscope->state.data.inputPoint
      ///< and scales by (buzz_slider_max - buzz_slider_min).
    virtual double scaledPointBumpSizeValue (void);
      ///< Looks up the bump size in microscope->state.data.inputPoint
      ///< and scales by (bump_slider_max - bump_slider_min).
    virtual double pointFrictionValue (void);
      ///< Looks up the friction value in microscope->state.data.inputPoint.



    virtual double magicBuzzAmplitude (double amp);
      ///< Returns 0.002 * amp.
    virtual double magicBumpSize (double wavelength);
      ///< Returns 0.01 * wavelength to scale from centimeters to meters.


    virtual double dynamicFrictionKspring (double kS);
      ///< Dynamic friction at points on the grid is fixed to the value of
      ///< max(0.0, Arm_knobs[FRICTION_KNOB]), so we ignore kS and use
      ///< nmui_SurfaceFeatureStrategy::pointFrictionValue() instead.

    nmm_Microscope_Remote * d_microscope;
};



/// @class nmui_GridFeatures
/// Used by touch_canned_from_plane and anything else that wants
/// to make the surface feel like data in the grid.

class nmui_GridFeatures : public nmui_SurfaceFeatureStrategy {

  public:

    nmui_GridFeatures (nmui_HSCanned * surface,
                       nmb_Dataset * dataset);
    virtual ~nmui_GridFeatures (void);

    // ACCESSORS

    virtual vrpn_bool buzzingEnabled (void) const;
    virtual vrpn_bool bumpsEnabled (void) const;

    // MANIPULATORS

    virtual double pointAdhesionValue (void);
      ///< Looks up plane->value(x, y).
    virtual double scaledPointBumpSizeValue (void);
      ///< Looks up plane->value(x, y) and scales by
      ///< (plane->maxValue() - plane->minValue()).
    virtual double scaledPointBuzzAmplitude (void);
    virtual double pointComplianceValue (void);
      ///< Looks up plane->value(x, y).
    virtual double pointFrictionValue (void);
      ///< Looks up plane->value(x, y).


    virtual double magicBuzzAmplitude (double amp);
      ///< Returns 0.0001 + .0009 * amp.
    virtual double magicBumpSize (double wavelength);
      ///< Returns 0.0004 + 0.004 * wavelength - old magic numbers.


    nmui_HSCanned * d_hapticSurface;
    nmb_Dataset * d_dataset;

};


#endif  // NMUI_SURFACE_FEATURES_H

