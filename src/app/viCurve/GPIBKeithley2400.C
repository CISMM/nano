// Important note: When printing the value of a Tclvar_int or Tclvar_float
// using *printf, make sure to cast it to the proper type, otherwise you 
// will get a printout of the pointer value!

//#include	<stdlib.h>
#include <signal.h>
#include	<stdio.h>
//#include	<string.h>

// must come before tcl/tk includes.
#include <vrpn_Connection.h>
#include <vrpn_FileConnection.h>

extern "C" {
#include	<tcl.h>
#include	<tk.h>
#include	<itcl.h>
#include	<itk.h>
#include        <blt.h>
}
// for some reason blt.h doesn't declare this procedure.
// I copied this prototype from bltUnixMain.c, but I had to add
// the "C" so it would link with the library.
extern "C" int Blt_Init (Tcl_Interp *interp);

#include	"Tcl_Linkvar.h"


#include "nma_Keithley2400_ui.h"
static void parseArguments(int argc, char **argv);
static void handle_cntl_c(int );
static void handle_quit_button(vrpn_int32, void *);


static nma_Keithley2400_ui * keithley2400_ui;
static 	vrpn_Connection * connection;

static Tcl_Interp	*tcl_interp;
static Tclvar_int *tcl_quit_button_pressed;
static char tcl_default_dir [] = "/afs/cs.unc.edu/project/stm/bin/";

//------------------------------------------------------------
// Duplicate functionality from microscape.
int	init_Tk ()
{
	Tk_Window       tk_control_window;

	tcl_interp = Tcl_CreateInterp();

	printf("init_Tk_control_panels(): just created the tcl/tk interpreter\n");

	/* Start a Tcl interpreter */
	if (Tcl_Init(tcl_interp) == TCL_ERROR) {
		fprintf(stderr,
			"Tcl_Init failed: %s\n",tcl_interp->result);
		return(-1);
	}

	/* Initialize Tk using the Tcl interpreter */
	if (Tk_Init(tcl_interp) == TCL_ERROR) {
		fprintf(stderr,
			"Tk_Init failed: %s\n",tcl_interp->result);
		return(-1);
	}

	/* Initialize Tcl packages */
	if (Blt_Init(tcl_interp) == TCL_ERROR) {
		fprintf(stderr,
			"Package_Init failed: %s\n",tcl_interp->result);
		return(-1);
	}

	if (Itcl_Init(tcl_interp) == TCL_ERROR) {
		fprintf(stderr,
			"Package_Init failed: %s\n",tcl_interp->result);
		return(-1);
	}
	if (Itk_Init(tcl_interp) == TCL_ERROR) {
		fprintf(stderr,
			"Package_Init failed: %s\n",tcl_interp->result);
		return(-1);
	}
	/* Start a Tk mainwindow to hold the widgets */
	tk_control_window = Tk_MainWindow(tcl_interp);
	if (tk_control_window == NULL) {
		fprintf(stderr,"Tk can't make window: %s\n",
			tcl_interp->result);
		return(-1);
	}


	return(0);
}


int	init_Tk_controls ()
{
        char *quitButtonText = "vi_quit";
	char    command[256];

	// Set up the window and variables needed by the tcl script. 

	sprintf(command, "wm withdraw .");
	TCLEVALCHECK(tcl_interp, command);

	sprintf(command, "set vi_win [toplevel .vi_win]");
	TCLEVALCHECK(tcl_interp, command);

	// Set the title
	sprintf(command, "wm title $vi_win {Keithley 2400 Meter}");
	TCLEVALCHECK(tcl_interp, command);

	// Make a button to close the program
	sprintf(command, "button $vi_win.close -text Close -command { set vi_quit 1 }");
	TCLEVALCHECK(tcl_interp, command);
	sprintf(command, "pack $vi_win.close -anchor nw");
	TCLEVALCHECK(tcl_interp, command);

	// Makes sure program exits when window is destroyed.
	sprintf(command, "wm protocol $vi_win WM_DELETE_WINDOW {$vi_win.close invoke}");
	TCLEVALCHECK(tcl_interp, command);

	// Also makes program exit if ctrl-c is pressed in main window. 
	sprintf(command, "bind all <Control-KeyPress-c> { set vi_quit 1 } ");
	TCLEVALCHECK(tcl_interp, command);

	tcl_quit_button_pressed = new Tclvar_int(quitButtonText, 0, handle_quit_button);
	
	/* Initialize the Tclvar variables */
	if (Tclvar_init(tcl_interp)) {
		fprintf(stderr,"Tclvar_init failed.\n");
		return(-1);
	}

	// Handle events caused by initializing tcl.
	while (Tk_DoOneEvent(TK_DONT_WAIT)) {};

	return 0;
}

//--------------------------------------------------------------------------
// Handles VRPN messages

int handle_any_print (void * userdata, vrpn_HANDLERPARAM p) {
  vrpn_Connection * c = (vrpn_Connection *) userdata;

  fprintf(stderr, "Got message \"%s\" from \"%s\".\n",
          c->message_type_name(p.type), c->sender_name(p.sender));

  return 0;  // non-error completion
}

// Argument handling
void usage(char *program_name){
	fprintf(stderr, "Error: bad arguments.\n");
	fprintf(stderr, "usage: %s [-o streamfile] [-i streamfile]",program_name);
	fprintf(stderr, " [-d ohmmeter_device]\n");
	exit(-1);
}

static char outputStreamName[128];
static int isWritingStreamFile = 0;
static int isReadingStreamFile = 0;
static char * vi_device_name = NULL;
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
			vi_device_name = strdup(argv[i]);
		}
		else if (!strcmp(argv[i], "-i")){
			if (++i >= argc) usage(argv[0]);
			isReadingStreamFile = 1;
			vi_device_name = new char[14 + strlen(argv[i])+1];
			sprintf(vi_device_name,"vi_curve@file://%s", argv[i]);
		}
		else
			usage(argv[0]);
	}
}

int	main(unsigned argc, char *argv[])
{
    unsigned short def_port_no = 4545;
    char *tcl_script_dir;
    
    parseArguments(argc, argv);
    
    if ( (tcl_script_dir=getenv("NM_TCL_DIR")) == NULL) {
	tcl_script_dir=tcl_default_dir;
    }

    //------------------------------------------------------------------
    // Generic Tcl startup.  Getting an interpreter and mainwindow.
    init_Tk();
    init_Tk_controls();
    
    // Initialize our connections to the things we are going to control.
    // defaults to port 4545
    char con_name [256];
    if (vi_device_name == NULL)
	sprintf(con_name, "vi_curve@argon.cs.unc.edu:%d", def_port_no);
    else if(isReadingStreamFile)
	// No port number while reading a stream.
	sprintf(con_name, "%s",vi_device_name);
    else 
	sprintf(con_name, "%s:%d",vi_device_name, def_port_no);
    
    if( (connection = vrpn_get_connection_by_name(con_name,
			  (isWritingStreamFile ? outputStreamName
			   : (char *)NULL), vrpn_LOG_INCOMING)) == NULL) {
       // connection failed. VRPN prints error message.
      return -1;
    }
    
   keithley2400_ui = new nma_Keithley2400_ui(tcl_interp, 
					     tcl_script_dir,
					     "vi_curve@dummyname.com", 
					     connection);
   vrpn_File_Connection *fcon;
   if (isReadingStreamFile){
       fcon = connection->get_File_Connection();
       if (!fcon) {
	   fprintf(stderr, "Error: expected but didn't get file connection\n");
	   exit(-1);
       }
       fcon->set_replay_rate(1.0);
   }

   signal(SIGINT, handle_cntl_c);


	// DEBUG print all messages, incoming and outgoing, 
	// sent over our VRPN connection
// 	connection->register_handler(vrpn_ANY_TYPE, handle_any_print,
// 				    connection);
         // defaults to vrpn_SENDER_ANY

	while (1) {
		//------------------------------------------------------------
		// This must be done in any Tcl app, to allow Tcl/Tk to handle
		// events
      
		while (Tk_DoOneEvent(TK_DONT_WAIT)) {};
      
		//------------------------------------------------------------
		// This is called once every time through the main loop.  It
		// pushes changes in the C variables over to Tcl.
      
		if (Tclvar_mainloop()) {
			fprintf(stderr,"Mainloop failed\n");
			return -1;
		}

		// Put in a sleep to free up CPU cycles.
		vrpn_SleepMsecs(1);
		//------------------------------------------------------------
		// Send/receive message from our vrpn connections.
		keithley2400_ui->mainloop();
      
	}
	//return 0;
}

void handle_quit_button(vrpn_int32, void *){
	if ((*tcl_quit_button_pressed) != 1) return;
	fprintf(stderr, "Shutting down and saving log file\n");
	if (keithley2400_ui)
		delete keithley2400_ui;
	if (connection)
		delete connection; // needed to make stream file write out
	exit(0);
}

void handle_cntl_c(int ){
	fprintf(stderr, "Received ^C signal, shutting down and saving log file\n");
    if (connection) 
        delete connection; // needed to make stream file write out
    if (keithley2400_ui) 
        delete keithley2400_ui;
	exit(-1);
}
