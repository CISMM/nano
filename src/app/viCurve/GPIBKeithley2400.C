
#include <signal.h>
#include <stdio.h>

// must come before tcl/tk includes.
#include <vrpn_Connection.h>
#include <vrpn_FileConnection.h>

extern "C" {
#include <tcl.h>
#include <tk.h>
#include <itcl.h>
#include <itk.h>
#include <blt.h>
}

#include "Tcl_Linkvar.h"
#include "Tcl_Interpreter.h"

#include "nma_Keithley2400_ui.h"

///////////////
// pre-declared functions
static void parseArguments(int argc, char **argv);
static void handle_cntl_c(int );
static void handle_quit_button(vrpn_int32, void *);
static void handle_rewind_stream_change( vrpn_int32 new_value, void * userdata );
static void handle_replay_rate_change( vrpn_int32 new_value, void *userdata );
static void handle_set_stream_time_change( vrpn_float64 new_value, void * );
///////////////

///////////////
// global variables
static nma_Keithley2400_ui* keithley2400_ui;
static vrpn_Connection* connection;
char *tcl_script_dir = NULL, * nano_root = NULL;

static Tcl_Interp* tcl_interp;
static Tclvar_int* tcl_quit_button_pressed;
static char tcl_default_dir [] = "./";

static char outputStreamName[128];
static int isWritingStreamFile = 0;
static int isReadingStreamFile = 0;
static int isReadingDevice = 0;
static char* vi_device_name = NULL;

Tclvar_int replay_rate( "stream_replay_rate", 1, handle_replay_rate_change, NULL );
Tclvar_int rewind_stream( "rewind_stream",0, handle_rewind_stream_change, NULL );
Tclvar_float set_stream_time( "set_stream_time", 0, handle_set_stream_time_change, NULL );
///////////////


// Handler for set_stream_time_now, NOT set_stream_time. 
static void handle_set_stream_time_change( vrpn_float64, void * ) 
{
  struct timeval newStreamTime;
  newStreamTime.tv_sec = (long) ((double) set_stream_time);
  newStreamTime.tv_usec = 999999L;

  vrpn_File_Connection* fcon;
  if (isReadingStreamFile)
  {
    fcon = connection->get_File_Connection();
    if (!fcon) {
      fprintf(stderr, "Error: expected but didn't get file connection\n");
      exit(-1);
    }
    
    // if we're jumping back in time, reset the interface
    // since vrpn will rewind
    struct timeval currentTime;
    fcon->time_since_connection_open( &currentTime );
    if( newStreamTime.tv_sec < currentTime.tv_sec )
      keithley2400_ui->reset( );
    
    // go to the new time
    fcon->play_to_time( newStreamTime );
  }
}


static void handle_rewind_stream_change (vrpn_int32 /*new_value*/, 
					     void * /*userdata*/)
{
  if( keithley2400_ui != NULL )
    keithley2400_ui->reset( );

    // force time to zero
    set_stream_time = 0;
}



static void handle_replay_rate_change (vrpn_int32 value, void *) 
{
  if (isReadingStreamFile)
  {
    vrpn_File_Connection* fcon = connection->get_File_Connection();
    if (!fcon) {
      fprintf(stderr, "Error: expected but didn't get file connection\n");
      exit(-1);
    }
    fcon->set_replay_rate( (float) value );
  }
}	



int init_Tk( char* tcl_script_dir )
{
  tcl_interp = Tcl_Interpreter::getInterpreter();
  // this does all the initialization when creating the interpreter

  // tell Tcl where to find our scripts
  Tcl_SetVar( tcl_interp, "tcl_script_dir", tcl_script_dir, TCL_GLOBAL_ONLY);
     
  return(0);
}


int init_Tk_controls ()
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
int handle_any_print (void * userdata, vrpn_HANDLERPARAM p)
{
  vrpn_Connection * c = (vrpn_Connection *) userdata;

  fprintf(stderr, "Got message \"%s\" from \"%s\".\n",
          c->message_type_name(p.type), c->sender_name(p.sender));

  return 0;  // non-error completion
}


// Argument handling
void usage(char *program_name)
{
	fprintf(stderr, "Error: bad arguments.\n");
	fprintf(stderr, "usage: %s [-o streamfile] [-i streamfile]",program_name);
	fprintf(stderr, " [-d ohmmeter_device]\n");
	exit(-1);
}


void parseArguments(int argc, char **argv)
{
  int i;
  for (i = 1; i < argc; i++){
    if (!strcmp(argv[i], "-o")){
      if (++i >= argc) usage(argv[0]);
      isWritingStreamFile = 1;
      strcpy(outputStreamName, argv[i]);
    }
    else if (!strcmp(argv[i], "-d")){
      if (++i >= argc) usage(argv[0]);
      isReadingDevice = 1;
      if( vi_device_name != NULL ) 
        delete vi_device_name;
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



int main(unsigned argc, char *argv[])
{
  unsigned short def_port_no = 4545;
  
  // -------------------------------------------
  // get environment variables
  char* tempstring = NULL;
  nano_root = tcl_script_dir = vi_device_name = NULL;
  tempstring = getenv( "NANO_ROOT" );
  if( tempstring != NULL )
  {
    nano_root = new char[ strlen( tempstring ) + 1 ];
    strcpy( nano_root, tempstring );
  }

  tempstring = getenv( "NM_TCL_DIR" );
  if( tempstring != NULL )
  {
    tcl_script_dir = new char[ strlen( tempstring ) + 1 ];
    strcpy( tcl_script_dir, tempstring );
  }

  tempstring = getenv( "NM_VICRVE" );
  if( tempstring != NULL )
  {
    vi_device_name = new char[ strlen( tempstring ) + 1 ];
    strcpy( vi_device_name, tempstring );
  }
  

  // --------------------------------------------------------------
  // set tcl_script_dir
  if(  tcl_script_dir == NULL )
  {
    if( nano_root != NULL )
    {
      tcl_script_dir = new char[ strlen( nano_root ) + 100 ];
      sprintf( tcl_script_dir, "%s/share/tcl/", nano_root );
    }
    else // no NANO_ROOT
    {
      if( tcl_script_dir == NULL )
        tcl_script_dir = tcl_default_dir;
    }
  }
  
  // -----------------------------------------------
  // get program arguments
  parseArguments(argc, argv);
  
  //------------------------------------------------------------------
  // Generic Tcl startup.  Getting an interpreter and mainwindow.
  Tcl_FindExecutable(argv[0]);
  init_Tk( tcl_script_dir );
  init_Tk_controls( );

  // tell keithley2400.tcl we're running as a standalone app
  char msg[] = "set vi_standalone 1";
  TCLEVALCHECK(tcl_interp, msg);

  // Initialize our connections to the things we are going to control.
  // defaults to port 4545
  char con_name [256];
  if (vi_device_name == NULL)
    {
      fprintf( stderr, "Error:  no device or streamfile specified.\n" );
      usage( argv[0] );
    }
  else if(isReadingStreamFile)
    // No port number while reading a stream.
    sprintf(con_name, "%s",vi_device_name);
  else 
    sprintf(con_name, "%s:%d",vi_device_name, def_port_no);
  
  connection 
    = vrpn_get_connection_by_name( con_name,
				   (isWritingStreamFile ? outputStreamName 
				    : (char*) NULL ) );
  if( connection == NULL) {
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

    // try to get tcl to show the streamfile control
    //char msg[] = "if { [info exist streamfile] } { show.streamfile }";
    char msg[] = "show.streamfile";
    TCLEVALCHECK(tcl_interp, msg);
  }
  
  signal(SIGINT, handle_cntl_c);
  
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
  
} // end main


void do_shutdown( )
{
  if (keithley2400_ui)
    delete keithley2400_ui;
  if (connection)
    delete connection; // needed to make stream file write out
  if( nano_root != NULL )
    delete nano_root;
  if( tcl_script_dir != NULL )
    delete tcl_script_dir;
  if( vi_device_name != NULL )
    delete vi_device_name;
}


void handle_quit_button(vrpn_int32, void *)
{
  if ((*tcl_quit_button_pressed) != 1) return;
  fprintf(stderr, "Shutting down and saving log file\n");
  do_shutdown( );
  exit(0);
}


void handle_cntl_c(int )
{
  fprintf(stderr, "Received ^C signal, shutting down and saving log file\n");
  do_shutdown( );
  exit(-1);
}
