#ifndef NMM_SAMPLE_H
#define NMM_SAMPLE_H

class nmm_Microscope_Remote;  // from "nmm_MicroscopeRemote.h"

/** \class nmm_Sample
 * Set of alternative strategies for sampling a surface with an AFM
 * in "feel-ahead mode".
 * Has a circular dependency with nmm_Microscope_Remote;  that implies
 * we should redesign.
 */

class nmm_Sample {

  public:

    nmm_Sample (nmm_Microscope_Remote *);
    virtual ~nmm_Sample (void) = 0;

    virtual void sampleAt (float x, float y) = 0;
      /**< Commands the microscope to take a sample at (x, y) */

  protected:

    nmm_Microscope_Remote * d_scope;

};

/** \class nmm_SampleGrid
 * Samples a surface in a mxn grid centered on the sample point
 */

class nmm_SampleGrid : public nmm_Sample {

  public:

    nmm_SampleGrid (nmm_Microscope_Remote *);
    virtual ~nmm_SampleGrid (void);

    virtual void sampleAt (float x, float y);

    void setGridSize (int x, int y);
      /**< Set number of grid lines at which to sample along each direction. */

    void setGridSpacing (float d);
      /**< Set distance between grid points, in nM. */

  protected:

    void recomputeMeasure (void);

    int d_xSize;
    int d_ySize;
    float d_xDistance;
    float d_yDistance;

    float d_xMeasure;
    float d_yMeasure;

};


#endif  // NMM_SAMPLE_H

