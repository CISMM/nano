#ifndef NMB_DATASET_H
#define NMB_DATASET_H

class BCGrid;  // from BCGrid.h
class BCPlane;  // from BCPlane.h
class nmb_Selector;  // from nmb_Selector.h

#include "nmb_Types.h"  // for PointType
#include "nmb_Image.h"  // for nmb_ImageLIst
#include "vrpn_Types.h" // for vrpn_bool

#ifndef NMB_SUBGRID_H
#include "nmb_Subgrid.h"  // for range_of_change
#endif

// class nmb_Dataset
//
// Tom Hudson, November 1997

//    Contains a (pointer to a) BCGrid with all known data from microscopes,
//  and an nmb_Subgrid used to record the portions of that grid that have
//  changed.
//    Contains the names of BCPlanes in the BCGrid that are mapped
//  to specific visualization techniques or parameters.

class nmb_Dataset {

  public:

    nmb_Dataset (vrpn_bool useFileResolution, int xSize, int ySize,
                 float xMin, float xMax, float yMin, float yMax,
                 int readMode, const char ** fileNames, int numFiles,
		 const char ** imageFileNames, int numImageFiles,
                 nmb_Selector * (* allocator) (const char *));
      // Constructor.

    ~nmb_Dataset (void);
      // Destructor.

    BCGrid * inputGrid;
        // incoming data from microscope (data may be changed 
	// during acquisition)
    nmb_ImageList * dataImages;
	// static data plus data currently being acquired or
	// overwritten (i.e., including inputGrid)

    nmb_Subgrid range_of_change;
        // portion of inputGrid that changed since the last render

    nmb_Selector * alphaPlaneName;
      // name of the plane whose data should control
      // alphablending of a texture
    nmb_Selector * colorPlaneName;
      // name of the plane to map color from
    nmb_Selector * colorMapName;
      // name of the color map to use
    nmb_Selector * contourPlaneName;
      // name of the plane to render contours for
    nmb_Selector * heightPlaneName;
      // name of the plane to render as height (Z)

    vrpn_bool done;
      // 1 if a subroutine has failed and directs us to exit


    // MANIPULATORS


    BCPlane * ensureHeightPlane (void);
      // Call to make sure there is a height plane in inputGrid.
      // This is defined as a plane with "nm" units, or (if none exists)
      //   a plane named "Topography-Forward".
      // If none exists, one will be created (with name "Topography-Forward"
      //   and units "nm").

    int computeFilteredPlane (const char * outputPlane,
                              const char * inputPlane,
                              const char * filterPath,
                              const char * filterName,
                              float scale, float angle,
                              const char * filterParameters);
      //   Runs the specified filter program on the inputPlane with
      // a rotation and scaling and creates outputPlane with the results.

    int computeAdhesionFromDeflection
                             (const char * outputPlane,
                              const char * firstInputPlane,
                              const char * lastInputPlane,
                              int numberToAverage);
      //   Estimates adhesion from a series of planes with numeric
      // names containing deflection data.

    int computeSumPlane (const char * outputPlane,
                         const char * firstInputPlane,
                         const char * secondInputPlane,
                         double scale);
      // Creates outputPlane as firstPlane + scale * secondPlane.
      // Output plane is updated as first and second plane change.


    int computeFlattenedPlane (const char * outputPlane,
                               const char * inputPlane,
                 float redX, float greenX, float blueX,
                 float redY, float greenY, float blueY);
      //   Projects (x1, y1)...(x3, y3) onto the inputPlane,
      // solves for the transformation that moves the corresponding points
      // into a flat plane, and creates outputPlane from inputPlane via that
      // transformation.  Output plane is updated as input plane changes.

    int computeLBLFlattenedPlane (const char * outputPlane,
				  const char * inputPlane);


  private:

    int mapInputToInputNormalized (const char * inputName,
                                   const char * ouputName);

    static void updateFlattenOnPlaneChange (BCPlane *, int x, int y,
                                            void * userdata);
    static void updateSumOnPlaneChange (BCPlane *, int x, int y,
                                        void * userdata);
    static void updateLBLFlattenOnPlaneChange (BCPlane *, int x, int y,
					       void * userdata);

};


#endif  // NMB_DATASET_H
