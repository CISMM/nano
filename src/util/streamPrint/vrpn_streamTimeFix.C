// must come before tcl/tk includes.
#include <vrpn_Connection.h>
#include <vrpn_FileConnection.h>

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

//--------------------------------------------------------------------------
// Handles VRPN messages
static timeval time_offset = {0 , 0};
static timeval last_message_time = {-1, -1};
static int default_sender_id = -1;

// This handler gets every message, presumably from a stream file
// then checks adjusts the timestamp to make sure it is greater than
// the timestamp of the message before, then sends out the same
// message with the new timestamp. 
int handle_any_print (void * userdata, vrpn_HANDLERPARAM p) {
    // Don't want to look at messages we just sent. 
    if (p.sender == default_sender_id) return 0;

    vrpn_Connection * c = (vrpn_Connection *) userdata;
    struct timeval el, adj_time;
    if(fcon) {
	fcon->time_since_connection_open(&el);
    } else {
	connection->time_since_connection_open(&el);
    }
//     printf("Msg \"%s\" from \"%s\" time %ld.%ld timestamp %ld.%ld\n",
//           c->message_type_name(p.type), c->sender_name(p.sender),
// 	  el.tv_sec, el.tv_usec, p.msg_time.tv_sec, p.msg_time.tv_usec);

  if(last_message_time.tv_sec == -1) {
      last_message_time = p.msg_time;
      adj_time = p.msg_time;
  } else {
      // Default thing to do is add the offset.
      adj_time = vrpn_TimevalSum(p.msg_time, time_offset);
      if (vrpn_TimevalGreater(last_message_time, 
			      adj_time)) {
	  // This message's timestamp is before that of the prev message.
	  // offset it forward.
	  time_offset = vrpn_TimevalSum(time_offset, 
				vrpn_TimevalDiff(last_message_time, adj_time));
	  //printf("   new offset time %ld.%ld\n", time_offset.tv_sec, 
	  //  time_offset.tv_usec);


	  // re-adjust the message time to use the new offset. 
	  adj_time = vrpn_TimevalSum(p.msg_time, time_offset);      
      } 
      // Update the previous message time. 
      last_message_time = p.msg_time;
  }
//printf("   adj time %ld.%ld\n", adj_time.tv_sec, adj_time.tv_usec);

 // Use our new sender ID so we don't get callback loops, and
  // converted stream files are identified. 
  connection->pack_message(p.payload_len, adj_time, p.type, default_sender_id, 
			   p.buffer, vrpn_CONNECTION_RELIABLE);
  return 0;  // non-error completion
}

// Argument handling
void usage(char *program_name){
	fprintf(stderr, "Error: bad arguments.\n");
	fprintf(stderr, "usage: %s [-o streamfile] [-i streamfile]",program_name);
	fprintf(stderr, " [-d device]\n");
	exit(-1);
}

static char outputStreamName[128];
static int isWritingStreamFile = 0;
static int isReadingStreamFile = 0;
static char * device_name = NULL;
void parseArguments(int argc, char **argv){
	int i;
	for (i = 1; i < argc; i++){
		if (!strcmp(argv[i], "-o")){
			if (++i >= argc) usage(argv[0]);
			isWritingStreamFile = 1;
			strcpy(outputStreamName, argv[i]);
		}
		else if (!strcmp(argv[i], "-d")){
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
	printf("Device_name is %s\n", device_name);
}

int	main(unsigned argc, char *argv[])
{
    time_offset.tv_sec = 0;
    time_offset.tv_usec=0;

    parseArguments(argc, argv);
    
    // Initialize our connections to the things we are going to control.
    if (device_name == NULL) {
	return -1;
    } 
    if( (connection = vrpn_get_connection_by_name(device_name, NULL,
		  (isWritingStreamFile ? outputStreamName: (char *)NULL))) == NULL) {
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
   if (isWritingStreamFile) {
       printf("Stream file %s\n", outputStreamName);
   }

   signal(SIGINT, handle_cntl_c);


   default_sender_id = connection->register_sender("streamTimeFix");

	// DEBUG print all messages, incoming and outgoing, 
	// sent over our VRPN connection
 	connection->register_handler(vrpn_ANY_TYPE, handle_any_print,
 				    connection);
         // defaults to vrpn_SENDER_ANY
// 	struct timeval newStreamTime;
// 	newStreamTime.tv_sec = 500;
// 	newStreamTime.tv_usec = 0L;
	//fcon->play_to_time(newStreamTime);

     connection->register_message_type
         ("clock query");
     connection->register_message_type
         ("clock reply");

     connection->register_message_type
         ("nmm Microscope SetRegionNM");
     connection->register_message_type
         ("nmm Microscope ScanTo");
     connection->register_message_type
         ("nmm Microscope ScanToZ");
     connection->register_message_type
         ("nmm Microscope ZagTo");
     connection->register_message_type
         ("nmm Microscope SetScanStyle");
     connection->register_message_type
         ("nmm Microscope SetSlowScan");
     connection->register_message_type
         ("nmm Microscope SetStdDelay");
     connection->register_message_type
         ("nmm Microscope SetStPtDelay");
     connection->register_message_type
         ("nmm Microscope SetRelax");
     connection->register_message_type
         ("nmm Microscope RecordResistance");
     connection->register_message_type
         ("nmm Microscope SetStdDevParams");
     connection->register_message_type
         ("nmm Microscope SetScanWindow");
     connection->register_message_type
         ("nmm Microscope ResumeWindowScan");
     connection->register_message_type
         ("nmm Microscope SetGridSize");
     connection->register_message_type
         ("nmm Microscope SetOhmmeterSampleRate");
     connection->register_message_type
         ("nmm Microscope EnableAmp");
     connection->register_message_type
         ("nmm Microscope DisableAmp");
     connection->register_message_type
         ("nmm Microscope EnableVoltsource");
     connection->register_message_type
         ("nmm Microscope DisableVoltsource");
     connection->register_message_type
         ("nmm Microscope SetRateNM");
     connection->register_message_type
         ("nmm Microscope SetMaxMove");
     connection->register_message_type
         ("nmm Microscope SetModForce");
     connection->register_message_type
         ("nmm Microscope DrawSharpLine");
     connection->register_message_type
         ("nmm Microscope DrawSweepLine");
     connection->register_message_type
         ("nmm Microscope DrawSweepArc");
     connection->register_message_type
         ("nmm Microscope GetNewPointDatasets");
     connection->register_message_type
         ("nmm Microscope GetNewScanDatasets");
     connection->register_message_type
         ("nmm Microscope Echo");
	// Tiger	HACK HACK HACK 	added two new message: d_MarkModify_type and d_MarkImage_type
     connection->register_message_type
	("nmm Microscope MarkModify");
     connection->register_message_type
        ("nmm Microscope MarkImage");
     connection->register_message_type
         ("nmm Microscope Shutdown");
     connection->register_message_type
         ("nmm Microscope QueryScanRange");
     connection->register_message_type
         ("nmm Microscope QueryStdDevParams");
     connection->register_message_type
         ("nmm Microscope QueryPulseParams");
     connection->register_message_type
         ("nmm Microscope VoltsourceEnabled");
     connection->register_message_type
         ("nmm Microscope VoltsourceDisabled");
     connection->register_message_type
         ("nmm Microscope AmpEnabled");
     connection->register_message_type
         ("nmm Microscope AmpDisabled");
     connection->register_message_type
         ("nmm Microscope StartingToRelax");
     connection->register_message_type
         ("nmm Microscope RelaxSet");
     connection->register_message_type
         ("nmm Microscope StdDevParameters");
     connection->register_message_type
         ("nmm Microscope WindowLineData");
     connection->register_message_type
         ("nmm Microscope WindowScanNM");
     connection->register_message_type
         ("nmm Microscope WindowBackscanNM");
     connection->register_message_type
         ("nmm Microscope PointResultNM");
     connection->register_message_type
         ("nmm Microscope PointResultData");
     connection->register_message_type
         ("nmm Microscope BottomPunchResultData");
     connection->register_message_type
         ("nmm Microscope TopPunchResultData");
     connection->register_message_type
         ("nmm Microscope ZigResultNM");
     connection->register_message_type
         ("nmm Microscope BluntResultNM");
     connection->register_message_type
         ("nmm Microscope ScanRange");
     connection->register_message_type
         ("nmm Microscope SetRegionCompleted");
     connection->register_message_type
         ("nmm Microscope SetRegionClipped");
     connection->register_message_type
         ("nmm Microscope ResistanceFailure");
     connection->register_message_type
         ("nmm Microscope Resistance");
     connection->register_message_type
         ("nmm Microscope Resistance2");
     connection->register_message_type
         ("nmm Microscope ReportSlowScan");
     connection->register_message_type
         ("nmm Microscope ScanParameters");
     connection->register_message_type
         ("nmm Microscope HelloMessage");
     connection->register_message_type
         ("nmm Microscope ClientHello");
     connection->register_message_type
         ("nmm Microscope ScanDataset");
     connection->register_message_type
         ("nmm Microscope PointDataset");
     connection->register_message_type
         ("nmm Microscope PidParameters");
     connection->register_message_type
         ("nmm Microscope ScanrateParameter");
     connection->register_message_type
         ("nmm Microscope ReportGridSize");
     connection->register_message_type
         ("nmm Microscope ServerPacketTimestamp");
     connection->register_message_type
         ("nmm Microscope TopoFileHeader");
     connection->register_message_type
	 ("nmm Microscope ForceCurveData");

     connection->register_message_type
	 ("nmm Microscope MaxSetpointExceeded");

     connection->register_message_type
         ("nmm Microscope Clark RecvTimestamp");
     connection->register_message_type
         ("nmm Microscope Clark FakeSendTimestamp");
     connection->register_message_type
         ("nmm Microscope Clark UdpSeqNum");

     connection->register_message_type
        ("nmm Microscope AFM EnterTappingMode");
     connection->register_message_type
        ("nmm Microscope AFM EnterContactMode");
     connection->register_message_type
        ("nmm Microscope AFM EnterDirectZControl");
     connection->register_message_type
        ("nmm Microscope AFM EnterSewingStyle");
     connection->register_message_type
	("nmm Microscope AFM EnterSpectroscopyMode");
     connection->register_message_type
        ("nmm Microscope AFM SetContactForce");
     connection->register_message_type
        ("nmm Microscope AFM QueryContactForce");

     connection->register_message_type
        ("nmm Microscope AFM InTappingMode");
     connection->register_message_type
        ("nmm Microscope AFM InContactMode");
     connection->register_message_type
        ("nmm Microscope AFM InDirectZControl");
     connection->register_message_type
        ("nmm Microscope AFM InSewingStyle");
     connection->register_message_type
	("nmm Microscope AFM InSpectroscopyMode");
     connection->register_message_type
        ("nmm Microscope AFM ForceParameters");
     connection->register_message_type
        ("nmm Microscope AFM BaseModParameters");
     connection->register_message_type
        ("nmm Microscope AFM ForceSettings");
     connection->register_message_type
        ("nmm Microscope AFM InModModeT");
     connection->register_message_type
        ("nmm Microscope AFM InImgModeT");
     connection->register_message_type
        ("nmm Microscope AFM InModMode");
     connection->register_message_type
        ("nmm Microscope AFM InImgMode");
     connection->register_message_type
        ("nmm Microscope AFM ModForceSet");
     connection->register_message_type
        ("nmm Microscope AFM ImgForceSet");
     connection->register_message_type
        ("nmm Microscope AFM ModForceSetFailure");
     connection->register_message_type
        ("nmm Microscope AFM ImgForceSetFailure");
     connection->register_message_type
        ("nmm Microscope AFM ModSet");
     connection->register_message_type
        ("nmm Microscope AFM ImgSet");
     connection->register_message_type
        ("nmm Microscope AFM ForceSet");
     connection->register_message_type
        ("nmm Microscope AFM ForceSetFailure");

     connection->register_message_type
        ("nmm Microscope STM SampleApproach");
     connection->register_message_type
        ("nmm Microscope STM SetBias");
     connection->register_message_type
        ("nmm Microscope STM SampleApproachNM");
     connection->register_message_type
        ("nmm Microscope STM SetPulsePeak");
     connection->register_message_type
        ("nmm Microscope STM SetPulseDuration");
     connection->register_message_type
        ("nmm Microscope STM PulsePoint");
     connection->register_message_type
        ("nmm Microscope STM PulsePointNM");

     connection->register_message_type
        ("nmm Microscope STM PulseParameters");
     connection->register_message_type
        ("nmm Microscope STM PulseCompletedNM");
     connection->register_message_type
        ("nmm Microscope STM PulseFailureNM");
     connection->register_message_type
        ("nmm Microscope STM PulseCompleted");
     connection->register_message_type
        ("nmm Microscope STM PulseFailure");
     connection->register_message_type
        ("nmm Microscope STM TunnellingAttained");
     connection->register_message_type
        ("nmm Microscope STM TunnellingAttainedNM");
     connection->register_message_type
        ("nmm Microscope STM TunnellingFailure");
     connection->register_message_type
        ("nmm Microscope STM ApproachComplete");

    // Scanline mode (client-->server)
     connection->register_message_type
	("nmm Microscope EnterScanlineMode");
     connection->register_message_type
        ("nmm Microscope RequestScanLine");

    // Scanline mode (server-->client)
     connection->register_message_type
        ("nmm Microscope InScanlineMode");
     connection->register_message_type
        ("nmm Microscope ScanlineData");
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
