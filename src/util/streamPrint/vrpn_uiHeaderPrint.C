// must come before tcl/tk includes.
#include <vrpn_Connection.h>
#include <vrpn_FileConnection.h>
#include <vrpn_Shared.h>

#ifdef sgi
#include <unistd.h>
#endif
#include	<stdlib.h>
#include <signal.h>
#include	<stdio.h>
#include	<string.h>


static void parseArguments(int argc, char **argv);
static void handle_cntl_c(int );


static 	vrpn_Connection * connection = NULL;
static   vrpn_File_Connection *fcon = NULL;

static vrpn_int32 vsufs;
static vrpn_int32 vsufr;

//--------------------------------------------------------------------------
// Handles VRPN messages

int handle_any_print (void * userdata, vrpn_HANDLERPARAM p) {
  vrpn_Connection * c = (vrpn_Connection *) userdata;
 struct timeval el;
 connection->time_since_connection_open(&el);
  printf("Msg \"%s\" from \"%s\" at %ld.%ld (%ld.%ld) - %d bytes\n",
          c->message_type_name(p.type), c->sender_name(p.sender),
	  el.tv_sec, el.tv_usec, p.msg_time.tv_sec, p.msg_time.tv_usec,
          p.payload_len);

  return 0;  // non-error completion
}

int handle_int_print (void * userdata, vrpn_HANDLERPARAM p) {
  vrpn_Connection * c = (vrpn_Connection *) userdata;
 struct timeval el;
 connection->time_since_connection_open(&el);
  vrpn_int32 v;
  const char ** bp = &p.buffer;
  vrpn_unbuffer(bp, &v);
  printf("Set %s to %d at %ld.%ld.\n", c->sender_name(p.sender), v,
         el.tv_sec, el.tv_usec);

  return 0;  // non-error completion
}

int handle_float_print (void * userdata, vrpn_HANDLERPARAM p) {
  vrpn_Connection * c = (vrpn_Connection *) userdata;
 struct timeval el;
 connection->time_since_connection_open(&el);
  vrpn_float64 v;
  const char ** bp = &p.buffer;
  vrpn_unbuffer(bp, &v);
  printf("Set %s to %.5f at %ld.%ld.\n", c->sender_name(p.sender), v,
         el.tv_sec, el.tv_usec);

  return 0;  // non-error completion
}

int handle_string_print (void * userdata, vrpn_HANDLERPARAM p) {
  vrpn_Connection * c = (vrpn_Connection *) userdata;
 struct timeval el;
 connection->time_since_connection_open(&el);
  char buf [1024];
  const char ** bp = &p.buffer;
  vrpn_unbuffer(bp, buf, p.payload_len);
  printf("Set %s to \"%s\" at %ld.%ld.\n", c->sender_name(p.sender), buf,
         el.tv_sec, el.tv_usec);

  return 0;  // non-error completion
}

// Argument handling
void usage(char *program_name){
	fprintf(stderr, "Error: bad arguments.\n");
	fprintf(stderr, "usage: %s [-i streamfile]",program_name);
	fprintf(stderr, " [-d device]\n");
	exit(-1);
}

void registerFloat (vrpn_int32 sender) {
  connection->register_handler(vsufs, handle_float_print, connection, sender);
  connection->register_handler(vsufr, handle_float_print, connection, sender);
}

void registerInt (vrpn_int32 sender) {
  connection->register_handler(vsufs, handle_int_print, connection, sender);
  connection->register_handler(vsufr, handle_int_print, connection, sender);
}

void registerString (vrpn_int32 sender) {
  connection->register_handler(vsufs, handle_string_print, connection, sender);
  connection->register_handler(vsufr, handle_string_print, connection, sender);
}


static int isReadingStreamFile = 0;
static char * device_name = NULL;
void parseArguments(int argc, char **argv){
	int i;
	for (i = 1; i < argc; i++){
	    if (!strcmp(argv[i], "-d")){
			if (++i >= argc) usage(argv[0]);
			device_name = strdup(argv[i]);
		}
		else if (!strcmp(argv[i], "-i")){
			if (++i >= argc) usage(argv[0]);
			isReadingStreamFile = 1;
			device_name = new char[14 + strlen(argv[i])+1];
			sprintf(device_name,"file://%s", argv[i]);
		}
		else
			usage(argv[0]);
	}
	printf( "Device_name is %s\n", device_name);
}

int	main(unsigned argc, char *argv[])
{
    
    parseArguments(argc, argv);
    
    // Initialize our connections to the things we are going to control.
    if (device_name == NULL) {
	return -1;
    } 
    if( (connection = vrpn_get_connection_by_name(device_name)) == NULL) {
       // connection failed. VRPN prints error message.
      return -1;
    }

   if (isReadingStreamFile){
       fcon = connection->get_File_Connection();
       if (fcon==NULL) {
	   fprintf(stderr, "Error: expected but didn't get file connection\n");
	   exit(-1);
       }
       fcon->set_replay_rate(1000.0);
   }

   signal(SIGINT, handle_cntl_c);


	// DEBUG print all messages, incoming and outgoing, 
	// sent over our VRPN connection
 	//connection->register_handler(vrpn_ANY_TYPE, handle_any_print,
 				    //connection);
         // defaults to vrpn_SENDER_ANY
// 	struct timeval newStreamTime;
// 	newStreamTime.tv_sec = 500;
// 	newStreamTime.tv_usec = 0L;
	//fcon->play_to_time(newStreamTime);

     connection->register_message_type
         ("clock query");
     connection->register_message_type
         ("clock reply");

  vsufs =
    connection->register_message_type ("vrpn Shared update from server");
  vsufr =
    connection->register_message_type ("vrpn Shared update from remote");

  vrpn_int32 vsbs =
    connection->register_message_type ("vrpn Shared become serializer");

  vrpn_int32 csmin = 
    connection->register_sender ("vrpn Shared float64 color_slider_min");
    registerFloat(csmin);
  vrpn_int32 csmax = 
    connection->register_sender ("vrpn Shared float64 color_slider_max");
    registerFloat(csmax);
  vrpn_int32 srr = 
    connection->register_sender ("vrpn Shared int32 stream_replay_rate");
    registerInt(srr);
  vrpn_int32 rs = 
    connection->register_sender ("vrpn Shared int32 rewind_stream");
    registerInt(rs);
  vrpn_int32 sst = 
    connection->register_sender ("vrpn Shared int32 set_stream_time");
    registerInt(sst);
  vrpn_int32 zs = 
    connection->register_sender ("vrpn Shared float64 z_scale");
    registerFloat(zs);
  vrpn_int32 mrx = 
    connection->register_sender ("vrpn Shared float64 measure_red_x");
    registerFloat(mrx);
  vrpn_int32 mry = 
    connection->register_sender ("vrpn Shared float64 measure_red_y");
    registerFloat(mry);
  vrpn_int32 mgx = 
    connection->register_sender ("vrpn Shared float64 measure_green_x");
    registerFloat(mgx);
  vrpn_int32 mgy = 
    connection->register_sender ("vrpn Shared float64 measure_green_y");
    registerFloat(mgy);
  vrpn_int32 mbx = 
    connection->register_sender ("vrpn Shared float64 measure_blue_x");
    registerFloat(mbx);
  vrpn_int32 mby = 
    connection->register_sender ("vrpn Shared float64 measure_blue_y");
    registerFloat(mby);
  vrpn_int32 rt = 
    connection->register_sender ("vrpn Shared float64 recovery_time");
    registerFloat(rt);
  vrpn_int32 rgpl = 
    connection->register_sender ("vrpn Shared int32 rulergrid_position_line");
    registerInt(rgpl);
  vrpn_int32 rgol = 
    connection->register_sender ("vrpn Shared int32 rulergrid_orient_line");
    registerInt(rgol);
  vrpn_int32 rgx = 
    connection->register_sender ("vrpn Shared float64 rulergrid_xoffset");
    registerFloat(rgx);
  vrpn_int32 rgy = 
    connection->register_sender ("vrpn Shared float64 rulergrid_yoffset");
    registerFloat(rgy);
  vrpn_int32 rgs = 
    connection->register_sender ("vrpn Shared float64 rulergrid_scale");
    registerFloat(rgs);
  vrpn_int32 rga = 
    connection->register_sender ("vrpn Shared float64 rulergrid_angle");
    registerFloat(rga);
  vrpn_int32 rgwx = 
    connection->register_sender ("vrpn Shared float64 ruler_width_x");
    registerFloat(rgwx);
  vrpn_int32 rgwy = 
    connection->register_sender ("vrpn Shared float64 ruler_width_y");
    registerFloat(rgwy);
  vrpn_int32 rgo = 
    connection->register_sender ("vrpn Shared float64 ruler_opacity");
    registerFloat(rgo);
  vrpn_int32 rgr = 
    connection->register_sender ("vrpn Shared int32 ruler_r");
    registerInt(rgr);
  vrpn_int32 rgg = 
    connection->register_sender ("vrpn Shared int32 ruler_g");
    registerInt(rgg);
  vrpn_int32 rgb = 
    connection->register_sender ("vrpn Shared int32 ruler_b");
    registerInt(rgb);
  vrpn_int32 rgc = 
    connection->register_sender ("vrpn Shared int32 rulergrid_changed");
    registerInt(rgc);
  vrpn_int32 rge = 
    connection->register_sender ("vrpn Shared int32 rulergrid_enabled");
    registerInt(rge);
  vrpn_int32 s = 
    connection->register_sender ("vrpn Shared int32 shiny");
    registerInt(s);
  vrpn_int32 d = 
    connection->register_sender ("vrpn Shared float64 diffuse");
    registerFloat(d);
  vrpn_int32 sa = 
    connection->register_sender ("vrpn Shared float64 surface_alpha");
    registerFloat(sa);
  vrpn_int32 sc = 
    connection->register_sender ("vrpn Shared float64 specular_color");
    registerFloat(sc);
  vrpn_int32 ts = 
    connection->register_sender ("vrpn Shared float64 texture_scale");
    registerFloat(ts);
  vrpn_int32 cw = 
    connection->register_sender ("vrpn Shared float64 contour_width");
    registerFloat(cw);
  vrpn_int32 cr = 
    connection->register_sender ("vrpn Shared int32 contour_r");
    registerInt(cr);
  vrpn_int32 cg = 
    connection->register_sender ("vrpn Shared int32 contour_g");
    registerInt(cg);
  vrpn_int32 cb = 
    connection->register_sender ("vrpn Shared int32 contour_b");
    registerInt(cb);
  vrpn_int32 cc = 
    connection->register_sender ("vrpn Shared int32 contour_changed");
    registerInt(cc);
  vrpn_int32 tcls = 
    connection->register_sender ("vrpn Shared int32 tclstride");
    registerInt(tcls);

  vrpn_int32 acf =
    connection->register_sender ("vrpn Shared String alpha_comes_from");
    registerString(acf);
  vrpn_int32 cm =
    connection->register_sender ("vrpn Shared String color_map");
    registerString(cm);
  vrpn_int32 ccf =
    connection->register_sender ("vrpn Shared String color_comes_from");
    registerString(ccf);
  vrpn_int32 zcf =
    connection->register_sender ("vrpn Shared String z_comes_from");
    registerString(zcf);

  vrpn_int32 sss =
    connection->register_sender ("vrpn Shared int32 share_sync_state");
    registerInt(sss);
  vrpn_int32 cis =
    connection->register_sender ("vrpn Shared int32 copy_inactive_state");
    registerInt(cis);
  vrpn_int32 cmn =
    connection->register_sender ("vrpn Shared String collab_machine_name");
    registerString(cmn);

  vrpn_int32 tldx =
    connection->register_sender ("vrpn Shared float64 tcl_lightDirX");
    registerFloat(tldx);
  vrpn_int32 tldy =
    connection->register_sender ("vrpn Shared float64 tcl_lightDirY");
    registerFloat(tldy);
  vrpn_int32 tldz =
    connection->register_sender ("vrpn Shared float64 tcl_lightDirZ");
    registerFloat(tldz);
  vrpn_int32 twxx =
    connection->register_sender ("vrpn Shared float64 tcl_wfr_xlate_X");
    registerFloat(twxx);
  vrpn_int32 twxy =
    connection->register_sender ("vrpn Shared float64 tcl_wfr_xlate_Y");
    registerFloat(twxy);
  vrpn_int32 twxz =
    connection->register_sender ("vrpn Shared float64 tcl_wfr_xlate_Z");
    registerFloat(twxz);
  vrpn_int32 twr0 =
    connection->register_sender ("vrpn Shared float64 tcl_wfr_rot_0");
    registerFloat(twr0);
  vrpn_int32 twr1 =
    connection->register_sender ("vrpn Shared float64 tcl_wfr_rot_1");
    registerFloat(twr1);
  vrpn_int32 twr2 =
    connection->register_sender ("vrpn Shared float64 tcl_wfr_rot_2");
    registerFloat(twr2);
  vrpn_int32 twr3 =
    connection->register_sender ("vrpn Shared float64 tcl_wfr_rot_3");
    registerFloat(twr3);
  vrpn_int32 tws =
    connection->register_sender ("vrpn Shared float64 tcl_wfr_scale");
    registerFloat(tws);

  connection->register_sender ("ccs0");
  vrpn_int32 tpq =
  connection->register_message_type ("Tracker Pos/Quat");
  vrpn_int32 tv =
  connection->register_message_type ("Tracker Velocity");
  vrpn_int32 ta =
  connection->register_message_type ("Tracker Acceleration");
  vrpn_int32 ttr =
  connection->register_message_type ("Tracker To Room");
  vrpn_int32 uts =
  connection->register_message_type ("Unit To Sensor");
  vrpn_int32 rttr =
  connection->register_message_type ("Request Tracker To Room");
  vrpn_int32 ruts =
  connection->register_message_type ("Request Unit To Sensor");
  vrpn_int32 tw =
  connection->register_message_type ("Tracker Workspace");
  vrpn_int32 rtw =
  connection->register_message_type ("Request Tracker Workspace");
  vrpn_int32 vtsur =
  connection->register_message_type ("vrpn Tracker set update rate");
  vrpn_int32 ro =
  connection->register_message_type ("Reset Origin");

  connection->register_sender ("nM coord change");
  vrpn_int32 css =
  connection->register_message_type ("change sync status");

  connection->register_sender ("Cmode0");
  vrpn_int32 ac =
  connection->register_message_type ("Analog Channel");

  connection->register_sender ("nmui Component View Plane");
  connection->register_sender ("nmui Component View Color");
  connection->register_sender ("nmui Component View Measure");
  connection->register_sender ("nmui Component View Lighting");
  connection->register_sender ("nmui Component View Contour");
  connection->register_sender ("nmui Component View Grid");
  connection->register_sender ("nmui Component View");
  connection->register_sender ("nmui Component Stream");
  connection->register_sender ("nmui Component ROOT");
  vrpn_int32 cnrs =
  connection->register_message_type ("nmui Component request sync");
  vrpn_int32 ncsc =
  connection->register_message_type ("nmui Component sync complete");

  connection->register_sender ("nmui Plane Sync");
  vrpn_int32 npsfp =
  connection->register_message_type ("nmui PS Flat Plane");

  //c->register_sender ("vrpn Shared float64 x");
  //c->register_sender ("vrpn Shared int32 x");
  //c->register_sender ("vrpn Shared String x");

     int done = 0;
     while (!done) {
	    sleep(0.01);
      
	    //------------------------------------------------------------
	    // Send/receive message from our vrpn connections.
	    connection->mainloop();
	    //fprintf(stderr, "After mainloop, fcon is doing %d.\n", fcon->doing_okay());
	    if (fcon) {
		if (fcon->eof())
		    done = 1;
	    }
     }
    if (connection) 
        delete connection; // needed to make stream file write out
     
     return 0;
}


void handle_cntl_c(int ){
    fprintf(stderr, "Received ^C signal, shutting down and saving log file\n");
    if (connection) 
        delete connection; // needed to make stream file write out

    exit(-1);
}
