
#ifndef _NMB_MORPHOLOGYPLANE_H
#define _NMB_MORPHOLOGYPLANE_H

#include "nmb_CalculatedPlane.h"
#include "nmb_Dataset.h"

#ifdef _WIN32
// turns off warning C4290: C++ Exception Specification ignored
#pragma warning( push )
#pragma warning( disable : 4290 )
#endif


/**
   A class representing a data plane computed from the weighted sum of 
   two other planes.  The formula used is:  
     output_plane = first_plane + ( scale * second_plane )
   To do the subtraction of two planes, the scale should be -1.
*/
class nmb_MorphologyPlane 
  : public nmb_CalculatedPlane
{
public:
  // Constructor
  nmb_MorphologyPlane( const char* inputPlaneName,
           const char* tipImageName,
           int morphologySelect,
		   const char* outputPlaneName,
		   nmb_Dataset* dataset 
		   // Dataset to which this plane will be added.
		   )
    throw( nmb_CalculatedPlaneCreationException );
  
  virtual ~nmb_MorphologyPlane( );

  // Accessor.  Returns true if this calc'd plane depend on
  // (is calculated from) the specified plane.
  virtual bool dependsOnPlane( const BCPlane* const plane );
  virtual bool dependsOnPlane( const char* planeName );

  // Packs up and sends across the connection all the data
  // necessary for the other end to recreate this calculated 
  // plane.
  void sendCalculatedPlane( vrpn_Connection* conn, vrpn_int32 senderID,
			    vrpn_int32 synchCalculatedPlaneMessageType ) const;
  
protected:

  BCPlane* sourcePlane;
  nmb_Image* tipImage;
  int morphologySelect;

  // constants indicating Mathematical Morphology operations
  static const int ERODE_TYPE;
  static const int DILATE_TYPE;
  static const int OPEN_TYPE;
  static const int CLOSE_TYPE;

  // create a new plane according to the data from vrpn
  static nmb_CalculatedPlane*
  _handle_PlaneSynch( vrpn_HANDLERPARAM p, nmb_Dataset* dataset )
    throw( nmb_CalculatedPlaneCreationException );
  friend class nmb_CalculatedPlane;

  // Update the calculated plane for changes in the source plane
  static void sourcePlaneChangeCallback( BCPlane* plane, int x, int y,
					 void* userdata );
  
  // non-static member function to handle changes in the source plane
  void _handleSourcePlaneChange( int x, int y );

private:
	nmb_MorphologyPlane( ) : nmb_CalculatedPlane( "", NULL )
	{ }

}; // end class nmb_MorphologyPlane


#ifdef _WIN32
#pragma warning( pop )
#endif

#endif // _NMB_MORPHOLOGYPLANE_H
