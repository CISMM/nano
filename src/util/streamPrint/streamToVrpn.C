#include "nmm_MicroscopeTranslator.h"
#include <stdio.h>
#include <stdlib.h>

// streamToNewVrpn
//
// Tom Hudson, May 1999
// Program takes a stm_cmd stream and translates it to the NEW
// VRPN message spec by Aron Helser.

// This is not very efficient;  we copy buffers over and over
// even when the format is unchanged because while copying we
// determine the length of the buffer.  Ugh.

// connection we're writing the output file on
vrpn_Connection * connection;
nmm_Microscope_Translator * translator;

void Usage(char *s)
{
  fprintf(stderr,"Usage: %s instream outputfile\n",s);
  fprintf(stderr,"       instream: Input stream file\n");
  exit(-1);
}

void	main(unsigned argc, char *argv[])
{
  int	ret;
  char	instream_name[255];
  char	outstream_name[255];
  stm_stream	*instream;

  /* Parse the command line */
  switch (argc) {
  case 3:
    strcpy(instream_name, argv[1]);
    strcpy(outstream_name, argv[2]);
    break;
  default:
    Usage(argv[0]);
  }

  /* Open the input stream files */
  fprintf(stderr, "Reading the input stream file...\n");
  if ((instream = stm_open_datastream_for_read(instream_name)) == NULL){
    fprintf(stderr, "Couldn't open the input stream file\n");
    exit(-1);
  }

  // don't connect to anything, just log
  connection = new vrpn_Synchronized_Connection
    ("localhost", 4500, outstream_name, vrpn_LOG_OUTGOING);

  char *myName = "nmm_Microscope";
  translator = new nmm_Microscope_Translator(myName, connection);

  /* Scan in the input file and translate to the output file until
   * the end of the input file is reached. */
  fprintf(stderr, "Converting the stream file...\n");
  while ( (ret = translator->translate_packet(instream)) == 1) 
  {
  }
  if (ret == -1) {
    fprintf(stderr,"Error during translation!\n");
  }

  timeval elapsed = translator->getTimeElapsed();
  printf("Elapsed time in stream was %ld seconds\n", elapsed.tv_sec);

  /* Close the stream file */
  if (stm_close_stream(instream)) {
    fprintf(stderr, "Couldn't close the input stream file\n");
    exit(-1);
  }

  delete translator;
  delete connection;
}

