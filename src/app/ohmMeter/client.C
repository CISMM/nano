/*
	This is a stand-alone application for reading from a vrpn_Ohmmeter.
	It allows you to read and write log files and specify the vrpn_Ohmmeter
	server name either as a command line argument or by default in an
	environment variable. Reading from a log file and getting data from a
	server are mutually exclusive.

	WARNING: if you kill the program by typing control-C then data may not
	be stored in the log file.

	author: Adam Seeger, 5/27/99
*/

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <tcl.h>
#include <tk.h>

#include <blt.h>
#include <itcl.h>
#include <itk.h>

#include "ohmmeter.h"
#include "vrpn_Connection.h"
#include "vrpn_FileConnection.h"

static char tcl_default_dir [] = "/afs/cs.unc.edu/project/stm/bin/";
static Tcl_Interp *tk_control_interp;

// for some reason blt.h doesn't declare this procedure.
// I copied this prototype from bltUnixMain.c, but I had to add
// the "C" so it would link with the library.
extern "C" int Blt_Init (Tcl_Interp *interp);

static int init_Tk();
static void parseArguments(int argc, char **argv);
static void handle_cntl_c(int );
static char outputStreamName[128];
static int isWritingStreamFile = 0;
static int isReadingStreamFile = 0;
static char *ohmmeter_device_name = NULL;
vrpn_Connection *ohmmeter_connection = NULL;
vrpn_Ohmmeter_Remote *ohmmeter_device = NULL;

int main(int argc, char **argv) {
        Ohmmeter *ohmmeter_controls;
	char *tcl_script_dir;
	char command[128];
        char *quitButtonText = "quit";
	
	parseArguments(argc, argv);

	if ( (tcl_script_dir=getenv("NM_TCL_DIR")) == NULL) {
		tcl_script_dir=tcl_default_dir;
	}

	if (!init_Tk()){
	  if (!ohmmeter_device_name){
		ohmmeter_device_name = getenv("OHMMETER");
	  }
	  Tclvar_init(tk_control_interp);
	  if (ohmmeter_device_name != NULL && 
		strcmp(ohmmeter_device_name, "null")) {
		printf("main: attempting to connect to ohmmeter: %s\n",
			ohmmeter_device_name);

		ohmmeter_connection = 
			vrpn_get_connection_by_name(
		  	ohmmeter_device_name, 
		   	(isWritingStreamFile ? outputStreamName
			   : (char *)NULL), vrpn_LOG_INCOMING);
		ohmmeter_device = new vrpn_Ohmmeter_Remote(ohmmeter_device_name,
				ohmmeter_connection);

	  }
	  // Hide the main window.
	  sprintf(command, "wm withdraw .");
	  TCLEVALCHECK(tk_control_interp, command);

          // source the french_ohmmeter.tcl file
          sprintf(command, "source %s%s",tcl_script_dir,"french_ohmmeter.tcl");
          if (Tcl_Eval(tk_control_interp, command) != TCL_OK) {
              fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command,
                    tk_control_interp->result);
              return 0;
          }

          ohmmeter_controls = new Ohmmeter (tk_control_interp,
                                tcl_script_dir, ohmmeter_device);

          ohmmeter_controls->openWindow();

	  vrpn_File_Connection *fcon;
	  if (isReadingStreamFile){
		fcon = ohmmeter_connection->get_File_Connection();
		if (!fcon) {
		  fprintf(stderr, "Error: expected but didn't get file connection\n");
		  exit(-1);
		}
		fcon->set_replay_rate(1.0);
	  }

	  signal(SIGINT, handle_cntl_c);

	  while(ohmmeter_controls->windowOpen()) {
		if (ohmmeter_device)
			ohmmeter_device->mainloop();
		while (Tk_DoOneEvent(TK_DONT_WAIT)) {};
		// Call the Tclvar's main loop to allow them to send new values
		// to the Tcl variables if they need to. 
		if (Tclvar_mainloop()) {
			fprintf(stderr, "main: Tclvar_mainloop error\n");
			return -1;
		}
	  }
	}
	else {
		fprintf(stderr, "error initializing tcl/tk\n");
		return -1;
	}
        if (ohmmeter_connection)
            delete ohmmeter_connection; // needed to make stream file write out
        if (ohmmeter_device)
            delete ohmmeter_device;

	return 0;
}

void usage(char *program_name){
	fprintf(stderr, "Error: bad arguments.\n");
	fprintf(stderr, "usage: %s [-o streamfile] [-i streamfile]",program_name);
	fprintf(stderr, " [-d ohmmeter_device]\n");
	exit(-1);
}

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
			ohmmeter_device_name = strdup(argv[i]);
		}
		else if (!strcmp(argv[i], "-i")){
			if (++i >= argc) usage(argv[0]);
			isReadingStreamFile = 1;
			ohmmeter_device_name = new char[14 + strlen(argv[i])+1];
			sprintf(ohmmeter_device_name,"Ohmmeter@file:%s", argv[i]);
		}
		else
			usage(argv[0]);
	}
}

int init_Tk(){
	tk_control_interp = Tcl_CreateInterp();
	printf("init_Tk(): just created the tcl/tk interpreter\n");
	/* Start a Tcl interpreter */
	if (Tcl_Init(tk_control_interp) == TCL_ERROR) {
		fprintf(stderr, "Tcl_Init failed: %s\n",tk_control_interp->result);
		return -1;
	}
	/* Initialize Tk using the Tcl interpreter */
	if (Tk_Init(tk_control_interp) == TCL_ERROR) {
		fprintf(stderr, "Tk_Init failed: %s\n",tk_control_interp->result);
		return -1;
	}
	if (Blt_Init(tk_control_interp) == TCL_ERROR) {
		fprintf(stderr, "Package_Init failed: %s\n",tk_control_interp->result);
		return -1;
	}
	if (Itcl_Init(tk_control_interp) == TCL_ERROR) {
		fprintf(stderr, "Package_Init failed: %s\n",tk_control_interp->result);
		return -1;
	}
        if (Itk_Init(tk_control_interp) == TCL_ERROR) {
                fprintf(stderr,
                        "Package_Init failed: %s\n",tk_control_interp->result);
                return(-1);
        }
        Tcl_StaticPackage(tk_control_interp, "Itcl", Itcl_Init, Itcl_SafeInit);
        Tcl_StaticPackage(tk_control_interp, "Itk", Itk_Init, 
                          (Tcl_PackageInitProc *) NULL);

	return 0;
}

void handle_cntl_c(int ){
	fprintf(stderr, "Received ^C signal, shutting down and saving log file\n");
    if (ohmmeter_connection) 
        delete ohmmeter_connection; // needed to make stream file write out
    if (ohmmeter_device) 
        delete ohmmeter_device;
	exit(0);
}
