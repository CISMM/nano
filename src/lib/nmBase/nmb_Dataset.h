/*===3rdtech===
  Copyright (c) 2000 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#ifndef NMB_DATASET_H
#define NMB_DATASET_H

class BCGrid;  // from BCGrid.h
class BCPlane;  // from BCPlane.h
class nmb_String;  // from nmb_String.h

#include "nmb_Types.h"  // for PointType
#include "nmb_Image.h"  // for nmb_ImageLIst
#include "vrpn_Types.h" // for vrpn_bool

#ifndef NMB_SUBGRID_H
#include "nmb_Subgrid.h"  // for range_of_change
#endif

#include "Topo.h"
#include "nmb_CalculatedPlane.h"

// class nmb_Dataset
//
// Tom Hudson, November 1997

// struct sum_node;

/**
   Contains a (pointer to a) BCGrid with all known data from microscopes,
 and an nmb_Subgrid used to record the portions of that grid that have
 changed.
   Contains the names of BCPlanes in the BCGrid that are mapped
 to specific visualization techniques or parameters.
*/
class nmb_Dataset {

  public:

    // Constructor.
    nmb_Dataset (vrpn_bool useFileResolution, int xSize, int ySize,
                 float xMin, float xMax, float yMin, float yMax,
                 int readMode, const char ** fileNames, int numFiles,
		 const char ** imageFileNames, int numImageFiles,
		 nmb_String * hostname, 
                 nmb_String * (* string_allocator) (const char *),
                 nmb_ListOfStrings * (* list_of_strings_allocator) (),
                 TopoFile &topoFile);

    // Destructor.
    ~nmb_Dataset (void);
    
    	//height-plane and colorplane to try and initialize to
	static char initHeight[256];
	static char initColorPlane[256];
	static bool doInitHeight,doInitColorPlane;
    

    BCGrid * inputGrid;
        ///< incoming data from microscope (data may be changed 
	///< during acquisition)

    nmb_ListOfStrings * inputPlaneNames;
      ///< lists the names of all planes of data

    nmb_ListOfStrings * imageNames;
        ///< List of names of image in dataImages list.
    nmb_ImageList * dataImages;
	///< static data plus data currently being acquired or
	///< overwritten (i.e., including inputGrid)

    nmb_Subgrid range_of_change;
        ///< portion of inputGrid that changed since the last render

    nmb_String * alphaPlaneName;
      ///< name of the plane whose data should control
      ///< alphablending of a texture
    nmb_String * colorPlaneName;
      ///< name of the plane to map color from
    nmb_String * colorMapName;
      ///< name of the color map to use
    nmb_String * contourPlaneName;
      ///< name of the plane to render contours for
    nmb_String * opacityPlaneName;
      ///< name of the plane to render opacity for
    nmb_String * heightPlaneName;
      ///< name of the plane to render as height (Z)
    nmb_String * transparentPlaneName;
      ///< plane that defines the alpha values for all
      ///< the points on the surface
    nmb_String * maskPlaneName;
      ///< name of the plane whose data should control
      ///< what to draw for the various visualizations
    nmb_String * vizPlaneName;
      ///< name of the plane to control display of visualizations

    vrpn_bool done;
      ///< 1 if a subroutine has failed and directs us to exit



    // MANIPULATORS
    int loadFile(const char* file_name, TopoFile &topoFile);
      ///< Load file with the same grid size/region into this grid.
      ///< Load file with any grid size/region into image list. 
    int addImageToGrid(nmb_ImageGrid * new_image) ;
    ///< Add an nmb_Image to existing grid, if it fits. 

    int setGridSize(int x, int y);
    ///< Change the grid size of the input grid.

    BCPlane * ensureHeightPlane (void);
    /**< Call to make sure there is a height plane in inputGrid.
      This is defined as a plane with "nm" units, or (if none exists)
        a plane named "Topography-Forward".
      If none exists, one will be created (with name "Topography-Forward"
      and units "nm"). */

    int computeFilteredPlane (const char * outputPlane,
                              const char * inputPlane,
                              const char * filterPath,
                              const char * filterName,
                              float scale, float angle,
                              const char * filterParameters);
      ///<   Runs the specified filter program on the inputPlane with
      ///< a rotation and scaling and creates outputPlane with the results.

    int computeAdhesionFromDeflection
                             (const char * outputPlane,
                              const char * firstInputPlane,
                              const char * lastInputPlane,
                              int numberToAverage);
      ///<   Estimates adhesion from a series of planes with numeric
      ///< names containing deflection data.


    float getFirstLineAvg(BCPlane *);
    ///< Computes average of the first scan line in the plane provided. 


    const char* getHostname( ) const;
    ///< returns what this Dataset currently thinks the hostname is.


   void addNewCalculatedPlane( nmb_CalculatedPlane* plane );
   ///< adds a new plane to the list of calculated planes and calls
   ///< any callbacks registered to listen for new (calc'd) plane creation.

private:

    int mapInputToInputNormalized (const char * inputName,
                                   const char * ouputName);

    nmb_CalculatedPlaneNode* calculatedPlane_head;

    nmb_String * d_hostname;
};


#endif  // NMB_DATASET_H
