#ifndef NMM_SAMPLE_H
#define NMM_SAMPLE_H

class nmb_DeviceSequencer;  // from <nmb_DeviceSequencer.h>

class nmm_Microscope_Remote;  // from "nmm_MicroscopeRemote.h"

/** \class nmm_Sample
 * Set of alternative strategies for sampling a surface with an AFM,
 * presumably in "feel-ahead mode".
 * Has a circular dependency with nmm_Microscope_Remote;  that implies
 * we should redesign.
 */

typedef void (* PostSampleCallback) (void * userdata);

class nmm_Sample {

  public:

    nmm_Sample (nmm_Microscope_Remote *);
    virtual ~nmm_Sample (void) = 0;


    // ACCESSORS


    virtual int numSamples (void) const = 0;


    // MANIPULATORS


    void setMicroscope (nmm_Microscope_Remote *);
      ///< Permits delayed binding of d_scope.

    void sampleAt (float x, float y);
      ///< Commands the microscope to take a sample at (x, y)

    void addPostSampleCallback (PostSampleCallback f, void * userdata);
      ///< Specifies a function to be called after each complete set of samples
      ///< is returned.

  protected:

    virtual void sendSampleRequests (void) = 0;
    virtual void processSampleRequests (void);
      ///< Clean-up routine that should be called after the strategy-specific
      ///< processing is done;  swaps d_scope->state.data.incomingPointList
      ///< with receivedPointList to make it visible and gets rid of stale
      ///< data;  triggers post-sample callbacks.

    static int handleSequencerIteration (void * userdata);
      ///< Once sampleAt() has been invoked, this callback alternately calls
      ///< sendSampleRequests() and processSampleRequests() until one of
      ///< them executes d_sequencer->releaseControl().

    nmm_Microscope_Remote * d_scope;
    nmb_DeviceSequencer * d_sequencer;

    int d_iteration;

    float d_sampleX, d_sampleY;
      ///< Location at which to take sample (after synch completes).

  private:

    void triggerPostSampleCallbacks (void);

    struct pscE {
      PostSampleCallback f;
      void * userdata;
      pscE * next;
    };
    pscE * d_callbacks;
};

/** \class nmm_SampleGrid
 * Samples a surface in a mxn grid centered on the sample point
 */

class nmm_SampleGrid : public nmm_Sample {

  public:

    nmm_SampleGrid (nmm_Microscope_Remote *);
    virtual ~nmm_SampleGrid (void);


    // ACCESSORS


    virtual int numSamples (void) const;


    // MANIPULATORS


    void setGridSize (int x, int y);
      /**< Set number of grid lines at which to sample along each direction. */

    void setGridSpacing (float d);
      /**< Set distance between grid points, in nM. */

  protected:

    virtual void sendSampleRequests (void);
    virtual void processSampleRequests (void);

    void recomputeMeasure (void);

    int d_xSize;
    int d_ySize;
    float d_xDistance;
    float d_yDistance;

    float d_xMeasure;
    float d_yMeasure;

};


#endif  // NMM_SAMPLE_H

