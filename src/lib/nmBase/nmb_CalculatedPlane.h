
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


////////////////
// Structure used to make lists of CalculatedPlanes
struct nmb_CalculatedPlaneNode
{
  nmb_CalculatedPlane* data;
  nmb_CalculatedPlaneNode* next;
};


/////////////////
// object thrown when an exception occurs during creation
// of a calculated plane
class nmb_CalculatedPlaneCreationException 
{ 
public:
  nmb_CalculatedPlaneCreationException( char* msgString = NULL );
  nmb_CalculatedPlaneCreationException( const nmb_CalculatedPlaneCreationException& e );

  ~nmb_CalculatedPlaneCreationException( )
  { if( msgString ) delete msgString; }

  const char* getMsgString( ) const
  { return msgString; }

protected:
  char* msgString;
};



/* virtual */ 
class nmb_CalculatedPlane
{
public:

  // Accessor.  Returns the plane calculated.
  BCPlane* getCalculatedPlane( );

  // Accessor.  Returns the name of the calculated plane.
  const string* getName( );

  // Accessor.  Returns true if this calc'd plane depend on
  // (is calculated from) the specified plane.
  virtual bool dependsOnPlane( const BCPlane* const plane ) = 0;
  virtual bool dependsOnPlane( const char* planeName ) = 0;

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
  
  // Returns a pointer to the calculated plane of the specified
  // name, or NULL if no such plane exists.
  static nmb_CalculatedPlane* 
  getCalculatedPlane( char* calculatedPlaneName );

  // removes this plane and all planes that depend on it.
  virtual ~nmb_CalculatedPlane( );

protected:
  //////////////
  // the following are member variables of all calculated planes

  // The calculated plane
  BCPlane* calculatedPlane;

  // constants indicating types of CalculatedPlanes
  static const int FLATTENED_PLANE_TYPE;
  static const int LBL_FLATTENED_PLANE_TYPE;
  static const int SUMMED_PLANE_TYPE;
  static const int MORPHOLOGY_PLANE_TYPE;


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
  // This is reasonably a virtual function. (but things can't be
  // both virtual and static).
  static nmb_CalculatedPlane*
    _handle_PlaneSynch( vrpn_HANDLERPARAM p, nmb_Dataset* dataset )
    throw( nmb_CalculatedPlaneCreationException );
  
private:
  // dataset.  keep this around to use in the destructor.
  // set this every time we are passed in a dataset argument.
  nmb_Dataset* dataset;
  
  // default constructor
  nmb_CalculatedPlane( );
  
  // copy constructor
  nmb_CalculatedPlane( nmb_CalculatedPlane& );

  // The desired name of the calculated plane
  char* calculatedPlaneName;

  struct NewCalculatedPlaneCallbackNode 
  {
    void* userdata;
    NewCalculatedPlaneCallback* callback;
    NewCalculatedPlaneCallbackNode* next;
  };
  
  static NewCalculatedPlaneCallbackNode* calculatedPlaneCB_head; 
  static nmb_CalculatedPlaneNode* calculatedPlaneList_head;

  // calls any callbacks registered to listen for new 
  // calculated plane creation.
  static void addNewCalculatedPlane( nmb_CalculatedPlane* plane );

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
//  4) Call nmb_CalculatedPlane::createCalculatedPlane(...) in the 
//     constructor(s) for the new plane type.  This will create and
//     initialize the calculatedPlane data member.

#ifdef _WIN32
#pragma warning( pop )
#endif



#endif
