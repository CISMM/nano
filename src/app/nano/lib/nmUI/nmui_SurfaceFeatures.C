#include "nmui_SurfaceFeatures.h"

#include <nmb_Dataset.h>

#include <nmm_MicroscopeRemote.h>

#include "nmui_HapticSurface.h"

// BUG - violates librariness
#include "microscape.h"  // for adhesion_slider_min &c, MIN_K and MAX_K
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
vrpn_bool nmui_SurfaceFeatureStrategy::buzzingEnabled (void) const {
  return vrpn_false;
}

//virtual
vrpn_bool nmui_SurfaceFeatureStrategy::bumpsEnabled (void) const {
  return vrpn_false;
}

// virtual
double nmui_SurfaceFeatureStrategy::pointAdhesionValue (void) {
  return 0.0;
}

// virtual
double nmui_SurfaceFeatureStrategy::scaledPointBuzzAmplitude (void) {
  return 0.0;
}

// virtual
double nmui_SurfaceFeatureStrategy::scaledPointBumpSizeValue (void) {
  return 0.0;
}

// virtual
double nmui_SurfaceFeatureStrategy::pointComplianceValue (void) {
  return max(0.0, Arm_knobs[FORCE_KNOB]);
}

// virtual
double nmui_SurfaceFeatureStrategy::pointFrictionValue (void) {
  return max(0.0, Arm_knobs[FRICTION_KNOB]);
}

// virtual
double nmui_SurfaceFeatureStrategy::dynamicFrictionKspring
        (double staticFrictionKspring) {
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

void nmui_SurfaceFeatures::update (void) {

  if (d_strategy) {

    specifyAdhesion();
    specifyBuzzAmplitude();
    specifyBumpSize();
    specifyCompliance();
    specifyFriction();

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



void nmui_SurfaceFeatures::specifyAdhesion (void) {
  double adhesion_range;
  double adh;

  adh = d_strategy->pointAdhesionValue();

  if (adhesion_slider_min != adhesion_slider_max) {
    adhesion_range = adhesion_slider_max - adhesion_slider_min;
    adh = (adh - adhesion_slider_min) / adhesion_range;

    adh = min(1.0, adh);
    adh = max(0.0, adh);

    adh *= d_adhesionConstant;
  }

  // do nothing
}

void nmui_SurfaceFeatures::specifyBuzzAmplitude (void) {
  double amp;

  if (!d_strategy->buzzingEnabled()) {
    setBuzzing(0.0);
    return;
  }

  amp = d_strategy->scaledPointBuzzAmplitude();

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

void nmui_SurfaceFeatures::specifyBumpSize (void) {
  double wavelength;

  if (!d_strategy->bumpsEnabled()) {
    setSurfaceTexture(0.0);
    return;
  }

  wavelength = d_strategy->scaledPointBumpSizeValue();

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

void nmui_SurfaceFeatures::specifyCompliance (void) {
  double compliance_range;
  double kS;

  kS = d_strategy->pointComplianceValue();

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

void nmui_SurfaceFeatures::specifyFriction (void) {
  double friction_range;
  double localValue;
  double kS;
  double dynamic_kS;

  localValue = d_strategy->pointFrictionValue();

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

  dynamic_kS = d_strategy->dynamicFrictionKspring(kS);
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







nmui_PointFeatures::nmui_PointFeatures (nmm_Microscope_Remote * scope) :
    nmui_SurfaceFeatureStrategy (),
    d_microscope (scope) {

}

// virtual
nmui_PointFeatures::~nmui_PointFeatures (void) {

}



// virtual
vrpn_bool nmui_PointFeatures::buzzingEnabled (void) const {
  Point_value * value =
    d_microscope->state.data.inputPoint->getValueByPlaneName
         (buzzPlaneName.string());
  double buzz_range = buzz_slider_max - buzz_slider_min;

  if (!buzz_range || !value) {
    return vrpn_false;
  }
  return vrpn_true;
}

// virtual
vrpn_bool nmui_PointFeatures::bumpsEnabled (void) const {
  Point_value * value =
    d_microscope->state.data.inputPoint->getValueByPlaneName
         (bumpPlaneName.string());
  if ((bump_slider_max == bump_slider_min) || !value) {
    return vrpn_false;
  }

  return vrpn_true;
}



// virtual
double nmui_PointFeatures::pointAdhesionValue (void) {
  Point_value * value =
    d_microscope->state.data.inputPoint->getValueByPlaneName
         (adhesionPlaneName.string());

  if (!value) {
    return nmui_SurfaceFeatureStrategy::pointAdhesionValue();
  }

  return value->value();
}

// virtual
double nmui_PointFeatures::scaledPointBuzzAmplitude (void) {
  Point_value * value =
    d_microscope->state.data.inputPoint->getValueByPlaneName
         (buzzPlaneName.string());
  double buzz_range = buzz_slider_max - buzz_slider_min;

  return (value->value() - buzz_slider_min) / buzz_range;
}

// virtual
double nmui_PointFeatures::pointFrictionValue (void) {
  Point_value * value =
    d_microscope->state.data.inputPoint->getValueByPlaneName
         (frictionPlaneName.string());

  if (!value) {
    return nmui_SurfaceFeatureStrategy::pointFrictionValue();
  }

  return value->value();
}

// virtual
double nmui_PointFeatures::scaledPointBumpSizeValue (void) {
  Point_value * value =
    d_microscope->state.data.inputPoint->getValueByPlaneName
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
double nmui_PointFeatures::dynamicFrictionKspring (double /*kS*/) {
  return nmui_SurfaceFeatureStrategy::pointFrictionValue();
}






nmui_GridFeatures::nmui_GridFeatures (nmui_HSCanned * surface,
                                      nmb_Dataset * dataset) :
    nmui_SurfaceFeatureStrategy (),
    d_hapticSurface (surface),
    d_dataset (dataset) {

}

// virtual
nmui_GridFeatures::~nmui_GridFeatures (void) {

}



// virtual
vrpn_bool nmui_GridFeatures::buzzingEnabled (void) const {
  BCPlane * plane =
    d_dataset->inputGrid->getPlaneByName(buzzPlaneName.string());

  if (!plane || (buzz_slider_max == buzz_slider_min)) {
    return vrpn_false;
  }

  return vrpn_true;
}

// virtual
vrpn_bool nmui_GridFeatures::bumpsEnabled (void) const {
  BCPlane * plane =
    d_dataset->inputGrid->getPlaneByName(bumpPlaneName.string());

  if ((bump_slider_max == bump_slider_min) || !plane) {
    return vrpn_false;
  }

  return vrpn_true;
}

// virtual
double nmui_GridFeatures::pointAdhesionValue (void) {
  BCPlane * plane =
    d_dataset->inputGrid->getPlaneByName(adhesionPlaneName.string());

  if (!plane) {
    return nmui_SurfaceFeatureStrategy::pointAdhesionValue();
  }

  return plane->value(d_hapticSurface->getGridX(),
                      d_hapticSurface->getGridY());
}

// virtual
double nmui_GridFeatures::scaledPointBuzzAmplitude (void) {
  BCPlane * plane =
    d_dataset->inputGrid->getPlaneByName(buzzPlaneName.string());
  double val;

  val = plane->value(d_hapticSurface->getGridX(),
                    d_hapticSurface->getGridY());
  return (val - plane->minValue()) /
            (plane->maxValue() - plane->minValue());
}

// virtual
double nmui_GridFeatures::pointComplianceValue (void) {
  BCPlane * plane =
    d_dataset->inputGrid->getPlaneByName(compliancePlaneName.string());

  if (!plane) {
    return nmui_SurfaceFeatureStrategy::pointComplianceValue();
  }

  return plane->value(d_hapticSurface->getGridX(),
                      d_hapticSurface->getGridY());
}

// virtual
double nmui_GridFeatures::pointFrictionValue (void) {
  BCPlane * plane = 
    d_dataset->inputGrid->getPlaneByName(frictionPlaneName.string());

  if (!plane) {
    return nmui_SurfaceFeatureStrategy::pointFrictionValue();
  }

  return plane->value(d_hapticSurface->getGridX(),
                      d_hapticSurface->getGridY());
}

// virtual
double nmui_GridFeatures::scaledPointBumpSizeValue (void) {
  BCPlane * plane =
    d_dataset->inputGrid->getPlaneByName(bumpPlaneName.string());
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





