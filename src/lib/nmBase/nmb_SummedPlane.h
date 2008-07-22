
#ifndef _NMB_SUMMEDPLANE_H
#define _NMB_SUMMEDPLANE_H

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
class nmb_SummedPlane 
  : public nmb_CalculatedPlane
{
public:
  // Constructor
  nmb_SummedPlane( const char* inputPlaneName1,
		   const char* inputPlaneName2,
		   double scale,
		   const char* outputPlaneName,
		   nmb_Dataset* dataset 
		   // Dataset to which this plane will be added.
		   )
    throw( nmb_CalculatedPlaneCreationException );
  
  virtual ~nmb_SummedPlane( );

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

  BCPlane* sourcePlane1;
  BCPlane* sourcePlane2;
  double scale;

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
	nmb_SummedPlane( ) : nmb_CalculatedPlane( "", NULL )
	{ }

}; // end class nmb_SummedPlane


#ifdef _WIN32
#pragma warning( pop )
#endif

#endif // _NMB_SUMMEDPLANE_H
