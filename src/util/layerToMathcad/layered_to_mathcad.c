#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <dirent.h>
#include "BCDebug.h"
#include "BCGrid.h"
#include "BCPlane.h"

/*******************
 * Global definitions
 *******************/

const int MAXFILES = 1000;

/*********
 * Functions defined in this file (added by KPJ to satisfy g++)...
 *********/

void Usage(char* s);
int main(int argc, char* argv[]);


void Usage(char* s)
{
  fprintf(stderr,"Usage: %s [-l lines] filename [filename ...]\n",s);
  fprintf(stderr,"     Converts each filename to filename.prn\n");
  fprintf(stderr,"     (From topometrix 3.0.6 binary to MathCAD)\n");
  fprintf(stderr,"       -l: Only translates the number of lines specified\n");
  exit(-1);
}

int translate_file(char *filename, int lines)
{
    BCGrid	inputGrid(512,512, 0,0,100,100, 0.0, READ_FILE,
			  &filename,1);	// Read the Grid from file
    BCPlane	*p;
    char	outname[1000];
    FILE	*outfile;
    int		x,y;

    // If the user specified a number of lines to do, only do this many.
    // If they didn't, lines will be 0, so set it to all.
    if (lines > 0) {
	if (lines > inputGrid.numY()) {	// Don't do more lines than there are
		lines = inputGrid.numY();
	}
    } else {
	lines = inputGrid.numY();
    }

    // Open the output file

    strncpy(outname, filename, sizeof(outname)-10);
    strcat(outname, ".prn");
    printf("Writing %s\n",outname);
    if ( (outfile = fopen(outname, "w")) == NULL) {
	perror("Cannot open output file for write");
	return(-1);
    }

    // Write the planes out to MATHCAD in the correct order.
    // This is one line in the file per layer, with the values laid out
    // along the line (x varying fastest).

    for (p = inputGrid.head(); p != NULL; p = p->next()) {
      for (y = 0; y < lines; y++) {
	for (x = 0; x < inputGrid.numX(); x++) {
		if (fprintf(outfile,"%f ",p->value(x,y)) < 0) {
			perror("Can't write output file");
			fclose(outfile);
			return(-1);
		}
	}
      }
      // Put the carriage return at the end of the line
      if (fprintf(outfile,"\r\n") < 0) {
		perror("Can't write newline to output file");
		fclose(outfile);
		return(-1);
      }
   }

   // ^Z at the end of the file
   if (fprintf(outfile,"\032") < 0) {
	perror("Can't write newline to output file");
	fclose(outfile);
	return(-1);
   }
   
   if (fclose(outfile) < 0) {
	perror("Error closing output file");
	return(-1);
   }

   return 0;
}

int main(int argc, char* argv[])
{
    BCDebug	debug("main");
    int		lines = 0;	// The number of lines to actually do

    char	*stm_file_names[MAXFILES];	/* Files to load */
    int		num_stm_files = 0;		/* How many to load */
    int		i;

    int	real_params = 0;		/* Non-flag parameters */

    debug.turnOff();

    /* Parse the command line */
    i = 1;
    while (i < argc) {

	if (strcmp(argv[i],"-l") == 0) {
		if (++i > argc) Usage(argv[0]);
		lines = atoi(argv[i]);
	} else if (argv[i][0] == '-') {
	    Usage(argv[0]);
	} else {	// Get the next file name
	    if (num_stm_files >= MAXFILES) {
		fprintf(stderr,"Only %d files allowed, ignoring %s\n",
			MAXFILES,argv[i]);
	    }
	    stm_file_names[num_stm_files] = argv[i];
	    num_stm_files++;

	    real_params++;
        }
	i++;
    }
    if (real_params == 0) Usage(argv[0]);

    for (i = 0; i < num_stm_files; i++) {
	if (translate_file(stm_file_names[i], lines)) {
		fprintf(stderr,"Failed on file %s\n",stm_file_names[i]);
		return -1;
	}
    }

    return 0;
}

