#ifndef NMM_GLOBALS_H
#define NMM_GLOBALS_H

// nmm_Globals
//
// Microscope objects visible *outside* this module.



// If defined, uses the VRPN microscope class (nmm_Microscope/_Remote);
// otherwises, uses the old Microscope class with a custom network layer.

//#define USE_VRPN_MICROSCOPE

#ifdef USE_VRPN_MICROSCOPE

class nmm_Microscope_Remote;
extern nmm_Microscope_Remote * microscope;

#else

class Microscope;
extern Microscope * microscope;

#endif

#endif  // NMM_GLOBALS_H


