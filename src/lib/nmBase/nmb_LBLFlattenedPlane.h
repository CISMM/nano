
#ifndef _NMB_LBLFLATTENEDPLANE_H
#define _NMB_LBLFLATTENEDPLANE_H

#include "nmb_CalculatedPlane.h"
#include "nmb_Dataset.h"

#ifdef _WIN32
// turns off warning C4290: C++ Exception Specification ignored
#pragma warning( push )
#pragma warning( disable : 4290 )
#endif


/** a class representing a data plane calculated by shifting each
    line of the plnae in Z so that the line's average height is the
    same as that of the first line of the plane
*/
class nmb_LBLFlattenedPlane
  : virtual public nmb_CalculatedPlane
{
public:
  // Constructor
  nmb_LBLFlattenedPlane( const char* inputPlaneName,
			 const char* outputPlaneName,
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

  BCPlane* sourcePlane;
  BCPlane* flatPlane;
  float firstLineAvg;

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

  // utility function used by the constructor 
  BCPlane* createLBLFlattenedPlane( nmb_Dataset* dataset,
				    const char* outputPlaneName )
    throw( nmb_CalculatedPlaneCreationException );

private:
  nmb_LBLFlattenedPlane( ) 
    : sourcePlane( NULL ), flatPlane( NULL ),
      firstLineAvg( 0 )
  { };

}; // end class nmb_LBLFlattenedPlane

#ifdef _WIN32
#pragma warning( pop )
#endif


#endif // defined _NMB_LBLFLATTENEDPLANE_H
