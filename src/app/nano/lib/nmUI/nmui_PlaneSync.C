#include "nmui_PlaneSync.h"


nmui_PlaneSync::nmui_PlaneSync (nmb_Dataset * d, vrpn_Connection * c) :
    d_server (c),
    d_peer (NULL),
    d_dataset (d) {

  d_myId = c->register_sender("nmui Plane Sync");

  d_flatPlaneSync_type = c->register_message_type("nmui PS Flat Plane");

  c->register_handler(d_flatPlaneSync_type, handle_flatPlaneSync,
                      this, d_myId);

  d->registerFlatPlaneCallback((void *)this, queueFlatPlaneForSync);
}

nmui_PlaneSync::~nmui_PlaneSync (void) {


}


void nmui_PlaneSync::addPeer (vrpn_Connection * c) {

  d_peer = c;

//fprintf(stderr, "nmui_PlaneSync::addPeer().\n");

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

//fprintf(stderr, "In nmui_PlaneSync::handle_flatPlaneSync.\n");

  it->d_dataset->computeFlattenedPlane(outputPlane, inputPlane,
                                       dx, dy, offset);

  return 0;
}


// static
void nmui_PlaneSync::queueFlatPlaneForSync (void * userdata, const flatten_data * f) {

  nmui_PlaneSync * it = (nmui_PlaneSync *) userdata;
  char planemsg [1024];
  timeval now;
  char * bufptr = planemsg;
  vrpn_int32 msglen = 1024;
  vrpn_int32 myId;
  vrpn_int32 flatPlaneSync_type;

  // HACK
  // If we don't have a peer, or just generally, we should queue up the
  // plane creation request until the user chooses to synchronize.

  if (!it->d_peer) {
//fprintf(stderr, "queueFlatPlaneForSync() with no peer.\n");
    return;
  }

  myId = it->d_peer->register_sender("nmui Plane Sync");
  flatPlaneSync_type = it->d_peer->register_message_type("nmui PS Flat Plane");

  vrpn_buffer(&bufptr, &msglen, f->dx);
  vrpn_buffer(&bufptr, &msglen, f->dy);
  vrpn_buffer(&bufptr, &msglen, f->offset);
  vrpn_buffer(&bufptr, &msglen, f->flat_plane->name()->Length());
  vrpn_buffer(&bufptr, &msglen, f->from_plane->name()->Length());
  vrpn_buffer(&bufptr, &msglen, f->flat_plane->name()->Characters(),
              f->flat_plane->name()->Length());
  vrpn_buffer(&bufptr, &msglen, f->from_plane->name()->Characters(),
              f->from_plane->name()->Length());

  gettimeofday(&now, NULL);
  it->d_peer->pack_message(1024 - msglen, now, flatPlaneSync_type, myId,
                           planemsg, vrpn_CONNECTION_RELIABLE);

//fprintf(stderr, "nmui_PlaneSync::queueFlatPlaneForSync sent.\n");

}



