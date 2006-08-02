//				nanoNC.C
//
//	This program will read in a GRID file in one of the formats
// known to the nanoManipulator and will then write out a Numerically-
// Controlled machines program to mill a part in the shape of that
// surface.
//

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "BCGrid.h"
#include "BCPlane.h"

#include "MicroscopeFlavors.h"
TopoFile	GTF;	// ??? required by BCGrid.C!!!

void	Usage(char *progname)
{
    fprintf(stderr, "Usage: %s inputfile outputfile.T [-mold] [-s x y z] [-maxcut depth] [-roughskip count] [-zoff offset]\n",progname);
    fprintf(stderr,"        inputfile: File that can be read by a BCGrid\n");
    fprintf(stderr,"        outputfile: NC machining file written\n");
    fprintf(stderr,"        -mold: Make an inverse shape (a mold)\n");
    fprintf(stderr,"        -s: Size (in inches) of the part [max each dim]\n");
    fprintf(stderr,"            [default is 1\"x1\"x1\"]\n");
    fprintf(stderr,"        -maxcut: Maximum depth of cut for each pass\n");
    fprintf(stderr,"            [default is 0.1\"]\n");
    fprintf(stderr,"        -roughskip: count of lines to skip in rough passes\n");
    fprintf(stderr,"            [default is 1 (no skipping)\"]\n");
    fprintf(stderr,"        -zoff: offset into the wood\n");
    fprintf(stderr,"            [default is 0.1\"]\n");
    exit(-1);
}

int	main(int argc, char **argv)
{
	int		real_params = 0;	// Number of non-flag parameters
	char		*infilename, *outfilename;
	FILE 		*outfile;
	BCGrid	*grid;
	BCPlane	*plane;
	double	sizex = 1, sizey = 1, sizez = 1;
	double	maxcut = 0.1;
	int	roughskip = 1;
	double	zoff = 0.1;
	int	invert = 0;		// Invert (make a mold?)
	int		i;

	// Parse the command line
	i = 1;	// Start with the first real argument
	while (i < argc) {
	  if (strcmp(argv[i],"-s") == 0) {
		if (++i > (argc-3) ) { Usage(argv[0]); };
		sizex = atof( argv[i++] );
		sizey = atof( argv[i++] );
		sizez = atof( argv[i++] );
		if ( (sizex <= 0) || (sizey <= 0) || (sizez <= 0) ) {
			fprintf(stderr,"Size must be >0!\n");
			return -1;
		}
	  } else if (strcmp(argv[i],"-mold") == 0) {
		++i;
		invert = 1;
	  } else if (strcmp(argv[i],"-zoff") == 0) {
		if (++i > (argc-1) ) { Usage(argv[0]); };
		zoff = atof( argv[i++] );
		if (zoff <= 0) {
			fprintf(stderr,"Z offset must be >0!\n");
			return -1;
		}
	  } else if (strcmp(argv[i],"-maxcut") == 0) {
		if (++i > (argc-1) ) { Usage(argv[0]); };
		maxcut = atof( argv[i++] );
		if (maxcut <= 0) {
			fprintf(stderr,"Maxcut must be >0!\n");
			return -1;
		}
	  } else if (strcmp(argv[i],"-roughskip") == 0) {
		if (++i > (argc-1) ) { Usage(argv[0]); };
		roughskip = atoi( argv[i++] );
		if (maxcut <= 0) {
			fprintf(stderr,"Roughskip must be >0!\n");
			return -1;
		}
	  } else if (argv[i][0] == '-') {
		Usage(argv[0]);
	  } else switch (real_params) {
		case 0:	infilename = argv[i++]; real_params++; break;
		case 1:	outfilename = argv[i++]; real_params++; break;
		default:
			Usage( argv[0] );
	  }
	}
	if (real_params != 2) Usage(argv[0]);

	// Read the file into the grid
	printf("Reading the Grid from file %s...\n",infilename);
	grid = new BCGrid(512, 512, 0.0, 0.0, 1.0, 1.0, READ_FILE,
		infilename);
	if (grid == NULL) {
		fprintf(stderr, "Cannot create Grid!\n");
		return -1;
	}

	// Select the plane to use (the first height one)
	plane = grid->head();
	while (plane) {
		if (!strcmp(*plane->units(),"nm")) {
			printf("Using plane %s\n",
				plane->name()->c_str());
			break;  // Found one!
		}
		plane = plane->next();
	}
	if (plane == NULL) {
		fprintf(stderr,"Cannot find a height plane in %s\n",
			infilename);
		return -1;
	}

	// Invert the Z values if we want a mold
	if (invert) {
		int x,y;
		double maxz, minz;

		maxz = plane->maxValue();
		minz = plane->minValue();

		printf("Inverting the Z values in the plane to make mold...\n");
		for (x = 0; x < plane->numX(); x++) {
		 for (y = 0; y < plane->numY(); y++) {
			plane->setValue(x,y,
				minz + (maxz - plane->value(x,y)) );
		 }
		}
	}

	// Write the NC milling file
	printf("Writing the NC milling file...\n");
	if ( (outfile = fopen(outfilename, "w")) == NULL) {
		perror("Can't open output file");
		return -1;
	}
	if (grid->writeNCFile(outfile, plane, sizex, sizey, sizez, maxcut,
	    zoff, roughskip)) {
		fprintf(stderr, "Can't write output file\n");
		fclose(outfile);
		return -1;
	}
	if (fclose(outfile)) {
		perror("Error closing output file");
		return -1;
	}

	printf("Done!\n");
	return 0;
}

