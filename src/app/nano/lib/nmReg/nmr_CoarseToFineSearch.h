#ifndef NMR_COARSETOFINESEARCH_H
#define NMR_COARSETOFINESEARCH_H

#include "nmr_AlignerMI.h"

/* nmr_CoarseToFineSearch
  This class is responsible for aligning 2 images using a 
  coarse to fine search
  Think of this as a level just above nmr_AlignerMI
  nmr_AlignerMI provides most of the tools for calculating the
  objective function and derivatives of that objective function 
  but doesn't really tell us how to use them to find the solution
  This class provides a strategy and management of the 
  necessary information for finding the solution
*/

// return values of takeStep()
enum {NMR_SEARCH_ERROR = -1,
      NMR_SEARCH_LEVEL_IN_PROGRESS = 0,
      NMR_SEARCH_LEVEL_FINISHED = 1,
      NMR_SEARCH_FINISHED = 2};

enum {NMR_TRANSLATE_X,
      NMR_TRANSLATE_Y, 
      NMR_SCALE_X,
      NMR_SCALE_Y,
      NMR_ROTATE_Z,
      NMR_SHEAR};

class nmr_CoarseToFineSearch {
  public:
    nmr_CoarseToFineSearch();
    ~nmr_CoarseToFineSearch();
    int initImages(nmb_Image *reference, nmb_Image *test,
                   nmb_ImageList *monitorList = NULL);
    void setTransformation(double *T);
    void getTransformation(double *T);

    /* set/get parameter value: the meaning of these depends on the current
	resolution level since the scale of the parameter is designed to 
	adjust to the size of a pixel
        The idea behind this is that change of this parameter should be
	commensurate with the size of the capture region.
        Another way of putting this is that the same size change at any 
	resolution level should result in the same maximum displacement
	(measured in pixels) at any place in the image
    */
    void setParameterValue(double value);
    double getParameterValue(vrpn_bool resolutionIndependent = VRPN_FALSE);
    int takeGradientStep();
    int optimizeInDirection(int direction, double &change, 
                            int numStepsUp = 10, int numStepsDown = 10,
                            double stepSizeFactor = 1.0);
    void plotObjective(int direction, int numValues, FILE *fout);
    int getJointHistogram(int level,
                           nmb_Image *histogram, vrpn_bool blur, 
                           vrpn_bool setRefScale,
                           float min_ref, float max_ref,
                           vrpn_bool setTestScale,
                           float min_test, float max_test);

  protected:
    // helper functions
    int initializeSearchAtLevel(int level, double *T);
    void finishSearchAtCurrentLevel();
    void convertTransformToPixelUnits(double *T1, double *T2);
    void convertTransformToImageUnits(double *T1, double *T2);
    vrpn_bool directionValid(int direction);

    int d_numResolutionLevels;
    double d_resolutionScaleFactor;
    nmb_Image **d_refImages;
    nmb_Image **d_testImages;
    nmb_Image **d_testGradXImages;
    nmb_Image **d_testGradYImages;
    double *d_minSqrTestGradMagnitude;
    nmr_AlignerMI d_alignerMI;
    
    int d_currentLevel;
    // this is what nmr_AlignerMI expects to get (something that takes
    // a given pixel location in one image to a pixel location in another image
    double d_transformPixelUnits[6];

    // this is the representation that is preserved across resolutions
    // it assumes coordinates in both images range from 0..1 in both
    // x and y
    double d_transformImageUnits[6];

    // parameters for Parzen windowing which we optimize with the help
    // of nmr_AlignerMI
    double d_sigmaRef;
    double d_sigmaTest;
    double d_sigmaRefRef, d_sigmaTestTest;

    // update rate for d_transformPixelUnits
    double d_transformUpdateRate;

    // update rate for Parzen window parameters
    double d_sigmaUpdateRate;

    // threshold for switching to the next resolution
    double d_levelSwitchThreshold;

    // in takeStep is this the first time at this level
    vrpn_bool d_firstStepAtLevel;

    // stuff for direct stepping
    int d_lastDirection;
    int d_lastDirectionSign;
    double d_lastObjFuncValue;
    double d_currObjFuncValue;
    double d_lastStepSize;
    double d_startStepSize;


    int d_stepsAtLevel;

    int d_direction;
    vrpn_bool d_takeRandomSamples;
};

#endif
