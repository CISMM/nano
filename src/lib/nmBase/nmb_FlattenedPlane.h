
#ifndef _NMB_FLATTENEDPLANE_H
#define _NMB_FLATTENEDPLANE_H

#include "nmb_CalculatedPlane.h"
#include "nmb_Dataset.h"

#ifdef _WIN32
// turns off warning C4290: C++ Exception Specification ignored
#pragma warning( push )
#pragma warning( disable : 4290 )
#endif


/**
A class representing a new plane that is a flattening of a height plane 
according to the positions of three measure lines (red, green and blue).
The resulting flattened plane is calculated such that the intersections 
of three measure lines with the surface have the same z value.
*/
class nmb_FlattenedPlane 
  : virtual public nmb_CalculatedPlane
{
public:
  // Constructor
  nmb_FlattenedPlane( const char* inputPlaneName,
		      const char* outputPlaneName,
		      float redX, float greenX, float blueX,
		      float redY, float greenY, float blueY,
		      nmb_Dataset* dataset 
		      // Dataset to which this plane will be added.
		      )
    throw( nmb_CalculatedPlaneCreationException );
  
  // Accessor.  Returns that calculated plane.
  BCPlane* getCalculatedPlane( )  { return flatPlane; }

  // returns the name of the calculated plane
  const BCString* getName( )  { return flatPlane->name( ); }

  // Packs up and sends across the connection all the data
  // necessary for the other end to recreate this calculated 
  // plane.
  void sendCalculatedPlane( vrpn_Connection* conn, vrpn_int32 senderID,
			    vrpn_int32 synchCalculatedPlaneMessageType ) const;

  
protected:
  nmb_FlattenedPlane( const char* inputPlaneName,
                      const char* outputPlaneName,
                      nmb_Dataset* dataset )
    throw( nmb_CalculatedPlaneCreationException );

  BCPlane* sourcePlane;
  BCPlane* flatPlane;
  double dx, dy, offset;

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

  // utility function used by the constructor to 
  // calculate and set dx, dy and offset for this flattened plane.
  void calculateDxDyOffset( nmb_Dataset* dataset,
			    float redX, float greenX, float blueX,
			    float redY, float greenY, float blueY )
    throw( nmb_CalculatedPlaneCreationException );

private:
  nmb_FlattenedPlane( ) 
    : sourcePlane( NULL ), flatPlane( NULL ),
      dx( 0 ), dy( 0 ), offset( 0 ),
		nmb_CalculatedPlane( "", NULL )
  {  };

}; // end class nmb_FlattenedPlane


#ifdef _WIN32
#pragma warning( pop )
#endif


#endif // defined _NMB_FLATTENEDPLANE_H
