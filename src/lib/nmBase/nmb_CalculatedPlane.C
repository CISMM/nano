
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
const int nmb_CalculatedPlane::FLATTENED_PLANE_TYPE = 1;

/* static */
const int nmb_CalculatedPlane::LBL_FLATTENED_PLANE_TYPE = 2;

/* static */
const int nmb_CalculatedPlane::SUMMED_PLANE_TYPE = 3;

/* static */
NewCalculatedPlaneCallbackNode* 
nmb_CalculatedPlane::calculatedPlaneCB_head = NULL;


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

      
