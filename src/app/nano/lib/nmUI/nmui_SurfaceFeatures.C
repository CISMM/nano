#include "nmui_SurfaceFeatures.h"

#include <nmb_Dataset.h>

#include <nmm_MicroscopeRemote.h>

#include "nmui_HapticSurface.h"

// BUG - violates librariness
#include "microscape.h"  // for adhesion_slider_min &c, MIN_K and MAX_K
#include "interaction.h"  // for knob #defines
#include "globals.h"  // for Arm_knobs[]

#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif








nmui_SurfaceFeatureStrategy::nmui_SurfaceFeatureStrategy (void) {

}

// virtual
nmui_SurfaceFeatureStrategy::~nmui_SurfaceFeatureStrategy (void) {

}

//virtual
vrpn_bool nmui_SurfaceFeatureStrategy::buzzingEnabled (nmm_Microscope_Remote *) const {
  return vrpn_false;
}

//virtual
vrpn_bool nmui_SurfaceFeatureStrategy::bumpsEnabled (nmm_Microscope_Remote *) const {
  return vrpn_false;
}

// virtual
double nmui_SurfaceFeatureStrategy::pointAdhesionValue (nmm_Microscope_Remote * ) {
  return 0.0;
}

// virtual
double nmui_SurfaceFeatureStrategy::scaledPointBuzzAmplitude (nmm_Microscope_Remote * ) {
  return 0.0;
}

// virtual
double nmui_SurfaceFeatureStrategy::scaledPointBumpSizeValue (nmm_Microscope_Remote * ) {
  return 0.0;
}

// virtual
double nmui_SurfaceFeatureStrategy::pointComplianceValue (nmm_Microscope_Remote * ) {
  return max( 0.01, min(1.0, (double)default_spring_k) );
}

// virtual
double nmui_SurfaceFeatureStrategy::pointFrictionValue (nmm_Microscope_Remote * ) {
  return max(0.0, Arm_knobs[FRICTION_KNOB]);
}

// virtual
double nmui_SurfaceFeatureStrategy::dynamicFrictionKspring
        (double staticFrictionKspring, nmm_Microscope_Remote *) {
  return 0.5 * staticFrictionKspring;
}


nmui_SurfaceFeatures::nmui_SurfaceFeatures (void) :
    d_firstExecution (vrpn_true),
    d_useLinearBuzzing (vrpn_false),
    d_useLinearBumps (vrpn_false),
    d_useLinearFriction (vrpn_false),
    d_adhesionConstant (12000.0),
    d_buzzFrequency (115.0),
    d_complianceGain (1.3),
    d_strategy (NULL) {

}

nmui_SurfaceFeatures::~nmui_SurfaceFeatures (void) {

}

void nmui_SurfaceFeatures::update (nmm_Microscope_Remote * scope) {

  if (d_strategy) {

    specifyAdhesion(scope);
    specifyBuzzAmplitude(scope);
    specifyBumpSize(scope);
    specifyCompliance(scope);
    specifyFriction(scope);

  }

  d_firstExecution = vrpn_false;
}

void nmui_SurfaceFeatures::setSurfaceFeatureStrategy
        (nmui_SurfaceFeatureStrategy * fs) {
  d_strategy = fs;
}



void nmui_SurfaceFeatures::useLinearBuzzing (vrpn_bool on) {
  d_useLinearBuzzing = on;
}

void nmui_SurfaceFeatures::useLinearBumps (vrpn_bool on) {
  d_useLinearBumps = on;
}

void nmui_SurfaceFeatures::useLinearFriction (vrpn_bool on) {
  d_useLinearFriction = on;
}



void nmui_SurfaceFeatures::setBuzzFrequency (double hz) {
  d_buzzFrequency = hz;
}

void nmui_SurfaceFeatures::setComplianceGain (double gain) {
  d_complianceGain = gain;
}

void nmui_SurfaceFeatures::setAdhesionConstant (double kAdh) {
  d_adhesionConstant = kAdh;
}



void nmui_SurfaceFeatures::specifyAdhesion (nmm_Microscope_Remote * scope) {
  double adhesion_range;
  double adh;

  adh = d_strategy->pointAdhesionValue(scope);

  if (adhesion_slider_min != adhesion_slider_max) {
    adhesion_range = adhesion_slider_max - adhesion_slider_min;
    adh = (adh - adhesion_slider_min) / adhesion_range;

    adh = min(1.0, adh);
    adh = max(0.0, adh);

    adh *= d_adhesionConstant;
  }

  // do nothing
}

void nmui_SurfaceFeatures::specifyBuzzAmplitude (nmm_Microscope_Remote * scope) {
  double amp;

  if (!d_strategy->buzzingEnabled(scope)) {
    setBuzzing(0.0);
    return;
  }

  amp = d_strategy->scaledPointBuzzAmplitude(scope);

  // Percieved magnitude of buzzing is proportional to x^0.95
  if (d_useLinearBuzzing) {
    amp = log(amp) / log(0.95);
  }

  // Limit to [0, 1]
  amp = min(1.0, amp);
  amp = max(0.0, amp);

  // Magic numbers from previous code
  amp = d_strategy->magicBuzzAmplitude(amp);

  setBuzzing(amp);
}

void nmui_SurfaceFeatures::specifyBumpSize (nmm_Microscope_Remote * scope) {
  double wavelength;

  if (!d_strategy->bumpsEnabled(scope)) {
    setSurfaceTexture(0.0);
    return;
  }

  wavelength = d_strategy->scaledPointBumpSizeValue(scope);

  // Percieved magnitude of bumps is proportional to lambda^0.86
  if (d_useLinearBumps) {
    wavelength = log(wavelength) / log(0.86);
  }

  // Limit to [0, 1]
  wavelength = min(1.0, wavelength);
  wavelength = max(0.0, wavelength);

  // Magic numbers from previous code
  wavelength = d_strategy->magicBumpSize(wavelength);

  setSurfaceTexture(wavelength);

}

void nmui_SurfaceFeatures::specifyCompliance (nmm_Microscope_Remote * scope) {
  double compliance_range;
  double kS;

  kS = d_strategy->pointComplianceValue(scope);

  if (compliance_slider_max != compliance_slider_min) {
    compliance_range = compliance_slider_max - compliance_slider_min;
    kS = (kS - compliance_slider_min) / compliance_range;

    // Compliance is perceptually linear, so we don't need to take logs.

    // Limit to [0, 1]
    kS = min(1.0, kS);
    kS = max(0.0, kS);

    // Gain compensation should almost certainly be AFTER we scale by
    // MIN_K and MAX_K, but this is a refactoring, not a bugfix.
    kS *= d_complianceGain;
    kS = kS * (MAX_K - MIN_K) + MIN_K;
  }

//fprintf(stderr, "Compliance is %.5f\n", kS);

  setCompliance(kS);
}

void nmui_SurfaceFeatures::specifyFriction (nmm_Microscope_Remote * scope) {
  double friction_range;
  double localValue;
  double kS;
  double dynamic_kS;

  localValue = d_strategy->pointFrictionValue(scope);

  if (friction_slider_max != friction_slider_min) {
    friction_range = friction_slider_max - friction_slider_min;
    kS = (localValue - friction_slider_min) / friction_range;

    // Percieved magnitude of friction is proportional to x^1.5
    if (d_useLinearFriction) {
      kS = log(kS) / log(1.5);
    }

    // Limit to [0, 1]
    kS = min(1.0, kS);
    kS = max(0.0, kS);

  } else {

    kS = max(0.0, Arm_knobs[FRICTION_KNOB]);

  }

  dynamic_kS = d_strategy->dynamicFrictionKspring(kS, scope);
  setFriction(kS, dynamic_kS);
}






// virtual
void nmui_SurfaceFeatures::setFriction (double kS, double dynamic_kS) {
  if (forceDevice) {
    forceDevice->setSurfaceFstatic(kS);
    forceDevice->setSurfaceFdynamic(dynamic_kS);
  }
}

void nmui_SurfaceFeatures::setSurfaceTexture (double wavelength) {
  if (forceDevice) {
    forceDevice->setSurfaceTextureWavelength(wavelength);
    forceDevice->setSurfaceTextureAmplitude(0.1 * wavelength);
  }
}

void nmui_SurfaceFeatures::setBuzzing (double amp) {
  if (forceDevice) {
    forceDevice->setSurfaceBuzzFrequency(d_buzzFrequency);
    forceDevice->setSurfaceBuzzAmplitude(amp);
  }
}

void nmui_SurfaceFeatures::setCompliance (double kS) {
  if (forceDevice) {
    forceDevice->setSurfaceKspring(kS);
  }
}







nmui_PointFeatures::nmui_PointFeatures () :
    nmui_SurfaceFeatureStrategy () {

}

// virtual
nmui_PointFeatures::~nmui_PointFeatures (void) {

}



// virtual
vrpn_bool nmui_PointFeatures::buzzingEnabled (nmm_Microscope_Remote * scope) const {
  Point_value * value =
    scope->state.data.inputPoint->getValueByPlaneName
         (buzzPlaneName.string());
  double buzz_range = buzz_slider_max - buzz_slider_min;

  if (!buzz_range || !value) {
    return vrpn_false;
  }
  return vrpn_true;
}

// virtual
vrpn_bool nmui_PointFeatures::bumpsEnabled (nmm_Microscope_Remote * scope) const {
  Point_value * value =
    scope->state.data.inputPoint->getValueByPlaneName
         (bumpPlaneName.string());
  if ((bump_slider_max == bump_slider_min) || !value) {
    return vrpn_false;
  }

  return vrpn_true;
}



// virtual
double nmui_PointFeatures::pointAdhesionValue (nmm_Microscope_Remote * scope) {
  Point_value * value =
    scope->state.data.inputPoint->getValueByPlaneName
         (adhesionPlaneName.string());

  if (!value) {
    return nmui_SurfaceFeatureStrategy::pointAdhesionValue(scope);
  }

  return value->value();
}

// virtual
double nmui_PointFeatures::scaledPointBuzzAmplitude (nmm_Microscope_Remote * scope) {
  Point_value * value =
    scope->state.data.inputPoint->getValueByPlaneName
         (buzzPlaneName.string());
  double buzz_range = buzz_slider_max - buzz_slider_min;

  return (value->value() - buzz_slider_min) / buzz_range;
}

// virtual
double nmui_PointFeatures::pointFrictionValue (nmm_Microscope_Remote * scope) {
  Point_value * value =
    scope->state.data.inputPoint->getValueByPlaneName
         (frictionPlaneName.string());

  if (!value) {
    return nmui_SurfaceFeatureStrategy::pointFrictionValue(scope);
  }

  return value->value();
}

// virtual
double nmui_PointFeatures::scaledPointBumpSizeValue (nmm_Microscope_Remote * scope) {
  Point_value * value =
    scope->state.data.inputPoint->getValueByPlaneName
         (bumpPlaneName.string());
  double bump_range = bump_slider_max - bump_slider_min;

  if (bump_range && value) {
    return (value->value() - bump_slider_min) / bump_range;
  } else {
    return 0.0;
  }
}


// virtual
double nmui_PointFeatures::magicBuzzAmplitude (double amp) {
  return 0.002 * amp;
}

// virtual
double nmui_PointFeatures::magicBumpSize (double wavelength) {
  return 0.01 * wavelength;
}



// virtual
double nmui_PointFeatures::dynamicFrictionKspring (double /*kS*/, nmm_Microscope_Remote * scope) {
  return nmui_SurfaceFeatureStrategy::pointFrictionValue(scope);
}






nmui_GridFeatures::nmui_GridFeatures (nmui_HSCanned * surface) :
    nmui_SurfaceFeatureStrategy (),
    d_hapticSurface (surface) {

}

// virtual
nmui_GridFeatures::~nmui_GridFeatures (void) {

}



// virtual
vrpn_bool nmui_GridFeatures::buzzingEnabled (nmm_Microscope_Remote * scope) const {
  BCPlane * plane =
    scope->Data()->inputGrid->getPlaneByName(buzzPlaneName.string());

  if (!plane || (buzz_slider_max == buzz_slider_min)) {
    return vrpn_false;
  }

  return vrpn_true;
}

// virtual
vrpn_bool nmui_GridFeatures::bumpsEnabled (nmm_Microscope_Remote * scope) const {
  BCPlane * plane =
    scope->Data()->inputGrid->getPlaneByName(bumpPlaneName.string());

  if ((bump_slider_max == bump_slider_min) || !plane) {
    return vrpn_false;
  }

  return vrpn_true;
}

// virtual
double nmui_GridFeatures::pointAdhesionValue (nmm_Microscope_Remote * scope) {
  BCPlane * plane =
    scope->Data()->inputGrid->getPlaneByName(adhesionPlaneName.string());

  if (!plane) {
    return nmui_SurfaceFeatureStrategy::pointAdhesionValue(scope);
  }

  return plane->value(d_hapticSurface->getGridX(),
                      d_hapticSurface->getGridY());
}

// virtual
double nmui_GridFeatures::scaledPointBuzzAmplitude (nmm_Microscope_Remote * scope) {
  BCPlane * plane =
    scope->Data()->inputGrid->getPlaneByName(buzzPlaneName.string());
  double val;

  val = plane->value(d_hapticSurface->getGridX(),
                    d_hapticSurface->getGridY());
  return (val - plane->minValue()) /
            (plane->maxValue() - plane->minValue());
}

// virtual
double nmui_GridFeatures::pointComplianceValue (nmm_Microscope_Remote * scope) {
  BCPlane * plane =
    scope->Data()->inputGrid->getPlaneByName(compliancePlaneName.string());

  if (!plane) {
    return nmui_SurfaceFeatureStrategy::pointComplianceValue(scope);
  }

  return plane->value(d_hapticSurface->getGridX(),
                      d_hapticSurface->getGridY());
}

// virtual
double nmui_GridFeatures::pointFrictionValue (nmm_Microscope_Remote * scope) {
  BCPlane * plane = 
    scope->Data()->inputGrid->getPlaneByName(frictionPlaneName.string());

  if (!plane) {
    return nmui_SurfaceFeatureStrategy::pointFrictionValue(scope);
  }

  return plane->value(d_hapticSurface->getGridX(),
                      d_hapticSurface->getGridY());
}

// virtual
double nmui_GridFeatures::scaledPointBumpSizeValue (nmm_Microscope_Remote * scope) {
  BCPlane * plane =
    scope->Data()->inputGrid->getPlaneByName(bumpPlaneName.string());
  double val;

  val = plane->value(d_hapticSurface->getGridX(),
                     d_hapticSurface->getGridY());
  return (val - plane->minValue()) /
            (plane->maxValue() - plane->minValue());
}



// virtual
double nmui_GridFeatures::magicBuzzAmplitude (double amp) {
  return 0.0001 + 0.0009 * amp;
}

// virtual
double nmui_GridFeatures::magicBumpSize (double wavelength) {
  return 0.0004 + 0.004 * wavelength;
}





