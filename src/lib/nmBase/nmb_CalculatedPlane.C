
#ifdef sgi
#include <unistd.h>  // for gethostname
#endif
#ifdef _WIN32
#include <winsock2.h>  // for gethostname
#endif

#include "nmb_Dataset.h"
#include "nmb_CalculatedPlane.h"
#include "nmb_FlattenedPlane.h"
#include "nmb_LBLFlattenedPlane.h"
#include "nmb_SummedPlane.h"

#ifdef _WIN32
// turns off warning C4290: C++ Exception Specification ignored
#pragma warning( push )
#pragma warning( disable : 4290 )
#endif


/* static */ 
const int nmb_CalculatedPlane::
FLATTENED_PLANE_TYPE = 1;

/* static */
const int nmb_CalculatedPlane::
LBL_FLATTENED_PLANE_TYPE = 2;

/* static */
const int nmb_CalculatedPlane::
SUMMED_PLANE_TYPE = 3;

/* static */
NewCalculatedPlaneCallbackNode* nmb_CalculatedPlane::
calculatedPlaneCB_head = NULL;

// Constructor
nmb_CalculatedPlane::
nmb_CalculatedPlane( const char* planeName, nmb_Dataset* dataset )
  throw( nmb_CalculatedPlaneCreationException )
  : calculatedPlane( NULL ),
    calculatedPlaneName( NULL )
{
  ///////
  // set calculatedPlaneName
#ifndef thirdtech
  if( strstr( planeName, " from " ) == NULL ) // not a plane from remote
  {
    // Add the host name to the plane name so we can distinguish
    // where the plane came from
    char hostname[256];
    calculatedPlaneName = new char[ 257 + strlen( planeName ) ];
    gethostname( hostname, 256 );
    sprintf( calculatedPlaneName, "%s from %s", planeName, hostname );
    // NB:  change the conditional above if you change the plane
    //      naming scheme.
  }
  else  // plane already has " from " in it
  {
    calculatedPlaneName = new char[ strlen( planeName ) + 1 ];
    sprintf( calculatedPlaneName, "%s", planeName );
  }
#else
  // 3rdTech only - no weird plane names.
  calculatedPlaneName = new char[ strlen( planeName ) + 1 ];
  sprintf( calculatedPlaneName, "%s", planeName );
  // YOUR CLOSE-MINDED PLANE-NAMING TECHINIQUES WILL BE YOUR
  // DOWNFALL WHEN THE REVOLUTION COMES!!1!11!  ALL PLANE
  // NAMES ARE BEAUTIFUL!!!!!
#endif
  // calculatedPlaneName is now set

  
  BCPlane* calculatedPlane 
     = dataset->inputGrid->getPlaneByName( calculatedPlaneName );
  
  if( calculatedPlane != NULL )
  {
     // a plane already exists by this name, and we disallow that.
     char s[] = "Cannot create flattened plane.  "
        "A plane already exists of the name:  ";
     char msg[1024];
     sprintf( msg, "%s%s.", s, calculatedPlaneName );
     throw nmb_CalculatedPlaneCreationException( msg );
  }
} // end nmb_CalculatedPlane( const char* )


nmb_CalculatedPlane::
~nmb_CalculatedPlane( )
{
	if( calculatedPlaneName != NULL )
		delete calculatedPlaneName;
}


BCPlane* nmb_CalculatedPlane::
getCalculatedPlane( )
{ return calculatedPlane; }

const BCString* nmb_CalculatedPlane:: 
getName( )
{
   if( calculatedPlane != NULL )
      return calculatedPlane->name( );
   else
      return NULL;
}



void nmb_CalculatedPlane:: 
createCalculatedPlane( char* units, BCPlane* sourcePlane, nmb_Dataset* dataset )
  throw( nmb_CalculatedPlaneCreationException )
{
  calculatedPlane 
    = dataset->inputGrid->addNewPlane( calculatedPlaneName, units, NOT_TIMED);
  if( calculatedPlane == NULL ) 
    {
      char s[] = "Could not create calculated plane.  Can't make plane:  ";
      char msg[1024];
      sprintf( msg, "%s%s.", s, calculatedPlaneName );
      throw nmb_CalculatedPlaneCreationException( msg );
    }
  
  TopoFile tf;
  nmb_Image* im = dataset->dataImages->getImageByPlane( sourcePlane );
  nmb_Image* output_im = new nmb_ImageGrid( calculatedPlane );
  if( im != NULL ) 
    {
      im->getTopoFileInfo(tf);
      output_im->setTopoFileInfo(tf);
    } 
  else 
    {
      fprintf(stderr, "nmb_FlattenedPlane: Warning, "
	      "input image not in list\n");
    }
  dataset->dataImages->addImage(output_im);

  // add new calculated plane to the dataset
  dataset->addNewCalculatedPlane( this );
  
  addNewCalculatedPlane( this );  
} // end createCalculatedPlane


/* static */
nmb_CalculatedPlane* nmb_CalculatedPlane::
receiveCalculatedPlane( vrpn_HANDLERPARAM p, nmb_Dataset* dataset )
    throw( nmb_CalculatedPlaneCreationException )
{
  const char * bp = p.buffer;
  vrpn_int32 planeType;
  vrpn_unbuffer( &bp, &planeType );

  nmb_CalculatedPlane* newPlane;
  switch( planeType )
    {
    case FLATTENED_PLANE_TYPE:
      newPlane = nmb_FlattenedPlane::_handle_PlaneSynch( p, dataset );
      break;
    case LBL_FLATTENED_PLANE_TYPE:
      newPlane = nmb_LBLFlattenedPlane::_handle_PlaneSynch( p, dataset );
      break;
    case SUMMED_PLANE_TYPE:
      newPlane = nmb_SummedPlane::_handle_PlaneSynch( p, dataset );
      break;
    default:
      newPlane = NULL;
      break;
    }

  return newPlane;
}



void nmb_CalculatedPlane::
addNewCalculatedPlane( nmb_CalculatedPlane* plane )
{
  // notify all parties who registered a callback for new planes.
  NewCalculatedPlaneCallbackNode* cbnode = calculatedPlaneCB_head;
  while( cbnode != NULL )
    {
      cbnode->callback( cbnode->userdata, plane );
      cbnode = cbnode->next;
    }

} // end addNewCalculatedPlane( ... )



/* static */
void nmb_CalculatedPlane::
registerNewCalculatedPlaneCallback( void * userdata,
				    NewCalculatedPlaneCallback* callback ) 
{
  NewCalculatedPlaneCallbackNode* node = new NewCalculatedPlaneCallbackNode;
  if( !node ) 
    {
      fprintf(stderr, "nmb_CalculatedPlane::" 
	      "registerFlatPlaneCallback:  Out of memory.\n");
      return;
    }

  node->userdata = userdata;
  node->callback = callback;
  node->next = calculatedPlaneCB_head;
  calculatedPlaneCB_head = node;
} // end registerNewCalculatedPlaneCallback( ... )



/* static */
void nmb_CalculatedPlane::
removeNewCalculatedPlaneCallback( void* userdata,
				  NewCalculatedPlaneCallback* callback )
{
  NewCalculatedPlaneCallbackNode* node = calculatedPlaneCB_head;
  NewCalculatedPlaneCallbackNode* prev = NULL;

  while( node != NULL
	 && node->userdata != userdata
	 && node->callback != callback )
    { 
      prev = node;
      node = node->next; 
    }
  if( node != NULL )
    {
      if( prev )
	prev->next = node->next;
      else
	calculatedPlaneCB_head = node->next;
      delete node;
    }
  
} // end removeNewCalculatedPlaneCallback( ... )



nmb_CalculatedPlaneCreationException::
nmb_CalculatedPlaneCreationException( char* msgString )
{
  if( msgString == NULL )
    {
      this->msgString = NULL;
    }
  else
    {
      this->msgString = new char[ strlen( msgString ) + 1 ];
      strcpy( this->msgString, msgString );
    }
}



nmb_CalculatedPlaneCreationException::
nmb_CalculatedPlaneCreationException( nmb_CalculatedPlaneCreationException& e )
{
  if( e.msgString == NULL )
    {
      this->msgString = NULL;
    }
  else
    {
      this->msgString = new char[ strlen( e.msgString ) + 1];
      strcpy( this->msgString, e.msgString );
    }
}



#ifdef _WIN32
#pragma warning( pop )
#endif

      
