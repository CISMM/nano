
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <vrpn_Connection.h>
#include <vrpn_FileConnection.h>


// Written 10/27/03 by David Marshburn for the
// nanoEducation project to clip stream files
// in preparation for upcoming experiments
// at schools.


static 	vrpn_Connection * connection = NULL;
static   vrpn_File_Connection *fcon = NULL;
static int default_sender_id = -1;

static char outputStreamName[128];
static int isWritingStreamFile = 0;
static int isReadingStreamFile = 0;
static char * device_name = NULL;
static int endTimeSec = -1;

static struct timeval initialTime = { 0, 0 };
static bool gotInitialTime = false;

//--------------------------------------------------------------------------

static void handle_cntl_c(int )
{
    fprintf(stderr, "Received ^C signal, shutting down and saving log file\n");
    if (connection) 
        delete connection; // needed to make stream file write out
    exit(-1);
}


// This handler gets every message, presumably from a stream file
int handle_any_print (void * userdata, vrpn_HANDLERPARAM p) 
{
    // Don't want to look at messages we just sent. 
    if (p.sender == default_sender_id) 
	{
		// printf( "handle_any_print:  default sender\n;" );
		return 0;
	}
	
    vrpn_Connection * c = (vrpn_Connection *) userdata;
    struct timeval el;
    if(fcon) {
		fcon->time_since_connection_open(&el);
    } else {
		connection->time_since_connection_open(&el);
    }
	
	if( !gotInitialTime )
	{
		initialTime.tv_sec = el.tv_sec;
		initialTime.tv_usec = el.tv_usec;
		printf( "initial time:  %d.%d\n", initialTime.tv_sec, initialTime.tv_usec );
		gotInitialTime = true;
	}
	
	/*
	printf("Msg \"%s\" from \"%s\" time %ld.%ld timestamp %ld.%ld\n",
	        c->message_type_name(p.type), c->sender_name(p.sender),
	 	    el.tv_sec, el.tv_usec, p.msg_time.tv_sec, p.msg_time.tv_usec);
	*/
	// clip any messages past the desired end time
	if( el.tv_sec - initialTime.tv_sec > endTimeSec )
		return 0;

	// Use our new sender ID so we don't get callback loops, and
	// converted stream files are identified. 
	connection->pack_message(p.payload_len, p.msg_time, p.type, default_sender_id, 
		p.buffer, vrpn_CONNECTION_RELIABLE);
	return 0;
}


// Argument handling
void usage(char *program_name)
{
	fprintf(stderr, "Error: bad or incomlete arguments.\n");
	fprintf(stderr, "usage: %s -o out-streamfile -i in-streamfile "
					"-t end-time\n", program_name);
	fflush( stderr );
	exit(-1);
}


void parseArguments(int argc, char **argv)
{
	int i;
	if( argc != 7 ) usage( argv[0] );

	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-o"))
		{
			if (++i >= argc) usage(argv[0]);
			isWritingStreamFile = 1;
			strcpy(outputStreamName, argv[i]);
		}
		else if (!strcmp(argv[i], "-i"))
		{
			if (++i >= argc) usage(argv[0]);
			isReadingStreamFile = 1;
			device_name = new char[14 + strlen(argv[i])+1];
			sprintf(device_name,"file://%s", argv[i]);
		}
		else if( !strcmp( argv[i], "-t" ) )
		{
			if( ++i >= argc ) usage( argv[0] );
			endTimeSec = atoi( argv[i] );
		}
		else
			usage(argv[0]);
	}
	printf("Device_name is %s\n", device_name);
}


int	main(unsigned argc, char *argv[])
{
    parseArguments(argc, argv);
    
    // Initialize our connections to the things we are going to control.
    if (device_name == NULL) {  return -1;  }

	printf( "device to open:  %s\n", device_name );

    if( (connection = vrpn_get_connection_by_name(device_name, NULL,
		  (isWritingStreamFile ? outputStreamName: (char *)NULL))) == NULL) {
       // connection failed. VRPN prints error message.
      return -1;
    }

	fcon = connection->get_File_Connection();
	if (fcon==NULL) 
	{
		fprintf(stderr, "Error: expected but didn't get file connection\n");
		exit(-1);
	}
	fcon->set_replay_rate(200.0);
	printf("Stream file %s\n", outputStreamName);

	signal(SIGINT, handle_cntl_c);
	
	
	default_sender_id = connection->register_sender("streamClip");
	
	// DEBUG print all messages, incoming and outgoing, 
	// sent over our VRPN connection.
	connection->register_handler(vrpn_ANY_TYPE, handle_any_print,
		connection);
	
	bool done = false;
	while (!done) 
	{
		//------------------------------------------------------------
		// Send/receive message from our vrpn connections.
		connection->mainloop();
		//fprintf(stderr, "After mainloop, fcon is doing %d.\n", fcon->doing_okay());
		if (fcon) 
		{
			if (fcon->eof()) done = true;
		}
		vrpn_SleepMsecs( 10 );      
	}
	
	if (connection) 
		delete connection; // needed to make stream file write out
	
	return 0;
}



