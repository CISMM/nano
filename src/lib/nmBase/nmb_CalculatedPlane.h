
///////////
// A virtual base class for all calculated planes
// nmb_CalculatedPlane serves to encapsulate all
// the functionality common to those entities
// reasonably called "calculated planes" and, in
// addition, provides a platform for the communication
// of calculated plane details across a network.


#ifndef NMB_CALCULATEDPLANE_H
#define NMB_CALCULATEDPLANE_H

#ifdef _WIN32
// turns off warning C4290: C++ Exception Specification ignored
#pragma warning( push )
#pragma warning( disable : 4290 )
#endif

#include <BCPlane.h>
class nmb_Dataset; // declare instead of include to avoid circular dependencies
#include <vrpn_Connection.h>

struct nmb_CalculatedPlaneNode;
class nmb_CalculatedPlaneCreationException;
class nmb_CalculatedPlane;



/////////////////
// Typedef for callbacks listening for new calculated planes
// : function with two arguments, returning void.
typedef void (NewCalculatedPlaneCallback) 
  ( void* userdata, const nmb_CalculatedPlane* newPlane );



/////////////////
// object thrown when an exception occurs during creation
// of a calculated plane
class nmb_CalculatedPlaneCreationException 
{ 
public:
  nmb_CalculatedPlaneCreationException( char* msgString = NULL );
  nmb_CalculatedPlaneCreationException( nmb_CalculatedPlaneCreationException& e );

  ~nmb_CalculatedPlaneCreationException( )
  { if( msgString ) delete msgString; }

  const char* getMsgString( ) const
  { return msgString; }

protected:
  char* msgString;
};



////////////////
// Structure used to make lists of CalculatedPlanes
struct nmb_CalculatedPlaneNode
{
  nmb_CalculatedPlane* data;
  nmb_CalculatedPlaneNode* next;
};



struct NewCalculatedPlaneCallbackNode 
{
  void* userdata;
  NewCalculatedPlaneCallback* callback;
  NewCalculatedPlaneCallbackNode* next;
};



/* virtual */ 
class nmb_CalculatedPlane
{
public:

  // Accessor.  Returns that calculated plane.
  BCPlane* getCalculatedPlane( );

  // Accessor.  Returns the name of the calculated plane.
  const BCString* getName( );

  // Packs up and sends across the connection all the data
  // necessary for the other end to recreate this calculated 
  // plane.
  virtual void 
  sendCalculatedPlane( vrpn_Connection* conn, vrpn_int32 senderID,
		       vrpn_int32 synchCalculatedPlaneMessageType ) const = 0;

  // Receive the data necessary to create a new CalculatedPlane,
  // and create this plane.
  static nmb_CalculatedPlane* 
  receiveCalculatedPlane( vrpn_HANDLERPARAM p, nmb_Dataset* dataset )
    throw( nmb_CalculatedPlaneCreationException );

  // Adds a new callback to the list of those wishing to hear about
  // the creation of a new CalculatedPlane.
  static void 
  registerNewCalculatedPlaneCallback( void * userdata,
				      NewCalculatedPlaneCallback* callback );
  
  // Removes a callback from the list of those wishing to hear about
  // the creation of new CalculatedPlanes.  userdata and callback
  // must have the same value as when the callback was registered.
  static void
  removeNewCalculatedPlaneCallback( void* userdata,
				    NewCalculatedPlaneCallback* callback );

  virtual ~nmb_CalculatedPlane( );

protected:
  //////////////
  // the following are member variables of all calculated planes

  // The name of the calculated plane
  char* calculatedPlaneName;

  // The calculated plane
  BCPlane* calculatedPlane;

  // constants indicating types of CalculatedPlanes
  static const int FLATTENED_PLANE_TYPE;
  static const int LBL_FLATTENED_PLANE_TYPE;
  static const int SUMMED_PLANE_TYPE;


  //////////////
  // the following are methods to be used by subclasses
  // implementing a calculated plane
  
  // constructor for nmb_CalculatedPlane
  nmb_CalculatedPlane( const char* calculatedPlaneName, nmb_Dataset* dataset )
    throw( nmb_CalculatedPlaneCreationException );
  
  // utility function to allocate the calculated plane
  // and set it up in dataset.  'units' is the units of the plane.
  // 'sourcePlane' is any plane from which this plane is calculated.
  void createCalculatedPlane( char* units, BCPlane* sourcePlane, 
                              nmb_Dataset* dataset )
    throw( nmb_CalculatedPlaneCreationException );
  
  /////////////
  // function that will be called by nmb_CalculatePlane::
  // handle_CalculatedPlane_synch() for the appropriate subclass.
  // This is reasonably a virtual function.
  static nmb_CalculatedPlane*
  _handle_PlaneSynch( vrpn_HANDLERPARAM p, nmb_Dataset* dataset )
    throw( nmb_CalculatedPlaneCreationException );

  // calls any callbacks registered to listen for new 
  // calculated plane creation.
  static void addNewCalculatedPlane( nmb_CalculatedPlane* plane );

 private:
  // default constructor
  nmb_CalculatedPlane( );

  // copy constructor
  nmb_CalculatedPlane( nmb_CalculatedPlane& );

  static NewCalculatedPlaneCallbackNode* calculatedPlaneCB_head; 
  static nmb_CalculatedPlaneNode* calculatedPlaneList_head;

}; // end class nmb_CalculatedPlane



////////////////
//  To add a new type of Calculated Plane:
//  1) Declare the plane type, which should inherit publicly from 
//     nmb_CalculatedPlane.  Provide implementations for
//     sendCalculatedPlane(...) and _handle_PlaneSynch(...).  
//     The method sendCalculatedPlane() should pack the 
//     plane type's constant (see (2)) as the first 
//     element of a buffer for vrpn and then pack whatever 
//     plane-specific data is necessary.
//  2) Add a constant indicating the new plane type as a 
//     protected data member of class nmb_CalculatedPlane.
//     See, e.g., FLATTENED_PLANE_TYPE.
//  3) Edit the static function nmb_CalculatedPlane::receiveCalculatedPlane
//     to recognize planes of the new type.  This also means
//     that a header for the new plane type needs to be included in
//     nmb_CalculatedPlane.C.


#ifdef _WIN32
#pragma warning( pop )
#endif



#endif
