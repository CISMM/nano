#include "nmui_PlaneSync.h"
#include <microscape.h>

// static class members
const char nmui_PlaneSync::
vrpnSenderType[] = "Plane Synch";

const char nmui_PlaneSync::
vrpnMessageType[] = "Plane Synch Calculated Plane";


nmui_PlaneSync::nmui_PlaneSync( vrpn_Connection * conn ) :
    d_server (conn),
    d_peer (NULL),
    d_accepting (VRPN_FALSE),
    incomingCalcdPlaneList (NULL),
    outgoingCalcdPlaneList (NULL)
{
  d_senderID = conn->register_sender( vrpnSenderType );
  d_synchCalculatedPlaneMessageType 
    = conn->register_message_type( vrpnMessageType );
  conn->register_handler(d_synchCalculatedPlaneMessageType, 
		      handleCalculatedPlaneSync,
                      this, d_senderID);
  nmb_CalculatedPlane::registerNewCalculatedPlaneCallback( (void *) this, 
						queueCalculatedPlaneForSync );
} // end nmui_PlaneSync( ... )



nmui_PlaneSync::~nmui_PlaneSync( void ) 
{
  // remove any items from our lists of calculated planes
  if( incomingCalcdPlaneList != NULL )
    {
      vrpn_HANDLERPARAM_node* pnode = incomingCalcdPlaneList;
      while( pnode != NULL )
	{
	  delete (void*) pnode->data->buffer;
	  delete pnode->data;
	  incomingCalcdPlaneList = pnode->next;
	  delete pnode;
	  
	  pnode = incomingCalcdPlaneList;
	}
    } 

  if( outgoingCalcdPlaneList != NULL )
    {
      nmb_CalculatedPlaneNode* cnode = outgoingCalcdPlaneList;
      while( cnode != NULL )
	{
	  outgoingCalcdPlaneList = cnode->next;
	  delete cnode;
	  cnode = outgoingCalcdPlaneList;
	}
    }

  // unregister our callback
  nmb_CalculatedPlane::removeNewCalculatedPlaneCallback( (void*) this,
					     queueCalculatedPlaneForSync );
} // end ~nmui_PlaneSynch
 



void nmui_PlaneSync::changePeer( vrpn_Connection * c ) 
{
  d_peer = c;
  d_senderID = d_peer->register_sender( vrpnSenderType );
  d_synchCalculatedPlaneMessageType 
    = d_peer->register_message_type( vrpnMessageType );
  
} // end addPeer( ... )



// static
int nmui_PlaneSync::
handleCalculatedPlaneSync( void * userdata, vrpn_HANDLERPARAM p ) 
{
  nmui_PlaneSync * it = (nmui_PlaneSync *) userdata;

  if( it == NULL || dataset == NULL )
    {
      fprintf( stderr, "ERROR:  handleCalculatedPlaneSync called with "
	       "invalid arguments.  I can't do anything.\n" );
      return -1;
    }

  if (it->d_accepting) 
    {
      nmb_CalculatedPlane* newPlane = NULL;
      try
	{
	  newPlane = 
	    nmb_CalculatedPlane::receiveCalculatedPlane( p, dataset );
	}
      catch( nmb_CalculatedPlaneCreationException e )
	{

	}

      if( newPlane == NULL )
	{
	  fprintf( stderr, "nmui_PlaneSync::Unable to create "
		   "plane from remote.\n" );
	}
    } 
  else // not accepting updates
    {
      // copy vrpn_HANDLERPARAM p and use it when we are accepting
      vrpn_HANDLERPARAM* newParam = new vrpn_HANDLERPARAM;
      newParam->type = p.type;
      newParam->sender = p.sender;
      newParam->msg_time = p.msg_time;
      newParam->payload_len = p.payload_len;
      char* newBuffer = new char[ p.payload_len * sizeof( char ) ];
      memcpy( newBuffer, p.buffer, p.payload_len );
      newParam->buffer = newBuffer;

      // put it at the end of the list, since plane creation
      // order may be important!
      vrpn_HANDLERPARAM_node* newNode = new vrpn_HANDLERPARAM_node;
      newNode->data = newParam;
      newNode->next = NULL;
      if( it->incomingCalcdPlaneList == NULL )
	it->incomingCalcdPlaneList = newNode;
      else
	{
	  for( vrpn_HANDLERPARAM_node* last = it->incomingCalcdPlaneList;
	       last->next != NULL; last = last->next );
	  last->next = newNode;
	}
    }
  return 0;
} // end handleCalculatedPlaneSynch



// static
void nmui_PlaneSync::
queueCalculatedPlaneForSync( void* userdata, const nmb_CalculatedPlane* newPlane )
{
  nmui_PlaneSync * it = (nmui_PlaneSync *) userdata;

  if( it->d_peer != NULL && it->d_accepting ) // just send the plane now
    {
      newPlane->sendCalculatedPlane( it->d_peer, it->d_senderID, 
				     it->d_synchCalculatedPlaneMessageType );
    }
  else  // queue the plane for synch later
    {
      nmb_CalculatedPlaneNode* node = new nmb_CalculatedPlaneNode;
      node->data = (nmb_CalculatedPlane*) newPlane;
      node->next = NULL;
      
      // add plane to the end of the list, as plane-creation
      // order is likely important
      if( it->outgoingCalcdPlaneList == NULL )
	it->outgoingCalcdPlaneList = node;
      else
	{
	  for( nmb_CalculatedPlaneNode* last = it->outgoingCalcdPlaneList;
	       last->next != NULL; last = last->next );
	  last->next = node;
	}
    }
} // end queueCalculatedPlaneForSync( ... )



void nmui_PlaneSync::acceptUpdates (void) 
{
    if (!d_peer) return;
    d_accepting = VRPN_TRUE;

    // send any planes that we have created to our peer
    nmb_CalculatedPlaneNode* cnode = outgoingCalcdPlaneList;
    while( cnode != NULL )
      {
	queueCalculatedPlaneForSync( this, cnode->data );

	// remove the node now.
	outgoingCalcdPlaneList = cnode->next;
	delete cnode;
	cnode = outgoingCalcdPlaneList;
      }

    // create any planes that our peer sent us. 
    vrpn_HANDLERPARAM_node* pnode = incomingCalcdPlaneList;
    while( pnode != NULL )
      {
	handleCalculatedPlaneSync( this, *(pnode->data) );
	
	// remove the node now.
	delete (void*) pnode->data->buffer;
	delete pnode->data;
	incomingCalcdPlaneList = pnode->next;
	delete pnode;
	
	pnode = incomingCalcdPlaneList;
      }
} // end acceptUpdates()


void nmui_PlaneSync::queueUpdates (void) 
{
    d_accepting = VRPN_FALSE;
}
