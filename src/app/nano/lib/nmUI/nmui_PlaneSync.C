#include "nmui_PlaneSync.h"


nmui_PlaneSync::nmui_PlaneSync (nmb_Dataset * d, AFMDataset * afmd, 
				vrpn_Connection * c) :
    d_server (c),
    d_peer (NULL),
    d_dataset (d),
    d_afmdata (afmd),
    d_sync_hostname (NULL),
    d_accepting (VRPN_FALSE),
    d_outgoing_flat_list (NULL),
    d_incoming_flat_list (NULL)

{

  d_myId = c->register_sender("nmui Plane Sync");

  d_flatPlaneSync_type = c->register_message_type("nmui PS Flat Plane");

  c->register_handler(d_flatPlaneSync_type, handle_flatPlaneSync,
                      this, d_myId);

  d->registerFlatPlaneCallback((void *)this, queueFlatPlaneForSync);

}

nmui_PlaneSync::~nmui_PlaneSync (void) {
  if (d_sync_hostname)
      delete [] d_sync_hostname;
}


void nmui_PlaneSync::addPeer (vrpn_Connection * c,
			      const char * new_sync_hostname) {

  d_peer = c;
  if (new_sync_hostname) {
      if (d_sync_hostname) {
	  delete [] d_sync_hostname;
      }
      d_sync_hostname = new char [strlen(new_sync_hostname) +1];
      strcpy(d_sync_hostname, new_sync_hostname);
  }
//fprintf(stderr, "nmui_PlaneSync::addPeer().\n");
  d_myId = d_peer->register_sender("nmui Plane Sync");
  d_flatPlaneSync_type = d_peer->register_message_type("nmui PS Flat Plane");

}


// static
int nmui_PlaneSync::handle_flatPlaneSync (void * userdata,
                                          vrpn_HANDLERPARAM p) {
  nmui_PlaneSync * it = (nmui_PlaneSync *) userdata;
  char outputPlane [256];
  char inputPlane [256];
  const char * bp = p.buffer;
  vrpn_int32 outputPlaneLen;
  vrpn_int32 inputPlaneLen;
  vrpn_float64 dx;
  vrpn_float64 dy;
  vrpn_float64 offset;

  vrpn_unbuffer(&bp, &dx);
  vrpn_unbuffer(&bp, &dy);
  vrpn_unbuffer(&bp, &offset);
  vrpn_unbuffer(&bp, &outputPlaneLen);
  vrpn_unbuffer(&bp, &inputPlaneLen);
  vrpn_unbuffer(&bp, outputPlane, outputPlaneLen);
  vrpn_unbuffer(&bp, inputPlane, inputPlaneLen);
  outputPlane[outputPlaneLen] = 0;
  inputPlane[inputPlaneLen] = 0;

//fprintf(stderr, "In nmui_PlaneSync::handle_flatPlaneSync. out %s in %s\n",
//	outputPlane, inputPlane);

//   char new_outputPlane[256];
//   if (it->d_sync_hostname) {
//       sprintf(new_outputPlane, "%s from %s", outputPlane, it->d_sync_hostname);
//   } else {
//       sprintf(new_outputPlane, "%s from ??", outputPlane);
//   }
  if (it->d_accepting) {
      // Add the host name to the plane name so we can distinguish
      // where the plane came from
      it->d_dataset->computeFlattenedPlane(outputPlane, inputPlane,
					   dx, dy, offset);
      it->d_afmdata->inputPlaneNames.addEntry(outputPlane);
  } else {
      // Create the flatten plane, but don't tell the user about it. 
      // This is cribbed from nmb_Dataset.C::computeFlattenPlane
      BCPlane * plane = it->d_dataset->inputGrid->getPlaneByName(inputPlane);
      if (plane == NULL) {
	  fprintf(stderr,
		  "nmui_PlaneSync::handle_flatPlaneSync(): could not get height plane!\n");
	  return -1;
      }
      char        newunits [1000];
      sprintf(newunits, "%s_flat", plane->units()->Characters());
      BCPlane * outplane = it->d_dataset->inputGrid->addNewPlane(outputPlane, newunits, NOT_TIMED);
      if (outplane == NULL) {
          fprintf(stderr,
            "nmui_PlaneSync::handle_flatPlaneSync(): Can't make plane %s\n",outputPlane);
          return -1;
      }
      it->d_dataset->dataImages->addImage(new nmb_ImageGrid(outplane));

      //Add flat plane to the list. It will be computed if/when the user
      // decides to sync with their collaborator. 
      flatten_node * new_node = new flatten_node;
      new_node->next = NULL;
      new_node->data = new flatten_data;
      new_node->data->dx = dx;
      new_node->data->dy = dy;
      new_node->data->offset = offset;
      new_node->data->from_plane = plane;
      new_node->data->flat_plane = outplane;

      // add to the end of the list, because plane creation order
      // may be important!!!
      if (it->d_incoming_flat_list==NULL) {
	  it->d_incoming_flat_list = new_node;
      } else {
	  flatten_node * list_itr =it->d_incoming_flat_list; 
	  while (list_itr->next !=NULL) {
	      list_itr = list_itr->next;
	  }
	  list_itr->next = new_node;
      }
  }
  return 0;
}


// static
void nmui_PlaneSync::queueFlatPlaneForSync (void * userdata, const flatten_data * f) {

  nmui_PlaneSync * it = (nmui_PlaneSync *) userdata;

  // HACK
  // If we don't have a peer, or just generally, we should queue up the
  // plane creation request until the user chooses to synchronize.

  if ((it->d_peer == NULL)||(!it->d_accepting) ) {
//fprintf(stderr, "queueFlatPlaneForSync() with no peer.\n");
      //Add flat plane to the list. It will be communicated if/when the user
      // decides to sync with their collaborator. 
      flatten_node * new_node = new flatten_node;
      new_node->next = NULL;
      new_node->data = new flatten_data;
      new_node->data->dx = f->dx;
      new_node->data->dy = f->dy;
      new_node->data->offset = f->offset;
      new_node->data->from_plane = f->from_plane;
      new_node->data->flat_plane = f->flat_plane;

      // add to the end of the list, because plane creation order
      // may be important!!! 
      if (it->d_outgoing_flat_list==NULL) {
	  it->d_outgoing_flat_list = new_node;
      } else {
	  flatten_node *list_itr =it->d_outgoing_flat_list; 
	  while (list_itr->next !=NULL) {
	      list_itr = list_itr->next;
	  }
	  list_itr->next = new_node;
      }
  } else {
      // Send the plane information right now.
      it->sendFlatMessage(f);
//fprintf(stderr, "nmui_PlaneSync::queueFlatPlaneForSync sent.\n");
  }
}

int nmui_PlaneSync::sendFlatMessage(const flatten_data * f) 
{
  char planemsg [1024];
  timeval now;
  char * bufptr = planemsg;
  vrpn_int32 msglen = 1024;

  vrpn_buffer(&bufptr, &msglen, f->dx);
  vrpn_buffer(&bufptr, &msglen, f->dy);
  vrpn_buffer(&bufptr, &msglen, f->offset);
  vrpn_buffer(&bufptr, &msglen,
                  (vrpn_int32) f->flat_plane->name()->Length());
  vrpn_buffer(&bufptr, &msglen,
                  (vrpn_int32) f->from_plane->name()->Length());
  vrpn_buffer(&bufptr, &msglen, f->flat_plane->name()->Characters(),
	      f->flat_plane->name()->Length());
  vrpn_buffer(&bufptr, &msglen, f->from_plane->name()->Characters(),
	      f->from_plane->name()->Length());
  
  gettimeofday(&now, NULL);
  d_peer->pack_message(1024 - msglen, now, d_flatPlaneSync_type, d_myId,
		       planemsg, vrpn_CONNECTION_RELIABLE);
  return 0;
}

void nmui_PlaneSync::acceptUpdates (void) 
{
    // If there is no peer, this doesn't make sense!
    if (!d_peer) return;

    d_accepting = VRPN_TRUE;
    // send any planes that we have created to our peer
    flatten_node *curr_node = d_outgoing_flat_list;
    flatten_node *last_node;
    while (curr_node != NULL) {
	sendFlatMessage(curr_node->data);
	last_node = curr_node;
	curr_node = curr_node->next;
	delete last_node->data;
	delete last_node;
    }
    d_outgoing_flat_list = NULL;
    // calculate any planes that our peer has send us. 
    curr_node = d_incoming_flat_list;
    while (curr_node != NULL) {
	// We already added a plane with the right name when
	// we received the message, so computing the plane now
	// will fill in that plane, and register callbacks so 
	// it gets updated.
	d_dataset->computeFlattenedPlane(
		 curr_node->data->flat_plane->name()->Characters(), 
		 curr_node->data->from_plane->name()->Characters(),
		 curr_node->data->dx, 
		 curr_node->data->dy, 
		 curr_node->data->offset);
	// Now we want the user to see the plane.
	d_afmdata->inputPlaneNames.addEntry(
		 curr_node->data->flat_plane->name()->Characters());
	last_node = curr_node;
	curr_node = curr_node->next;
	delete last_node->data;
	delete last_node;
    }
    d_incoming_flat_list = NULL;
    return;
}

void nmui_PlaneSync::queueUpdates (void) 
{
    d_accepting = VRPN_FALSE;
}
