#include	<stdio.h>
#include	<sys/types.h>

#ifdef  _WIN32
#	include        <io.h>
#else
#	include	<unistd.h>
#endif

#include	<stdlib.h>
#include	<fcntl.h>
#include	<string.h>
#include	<ctype.h>
#include	"PPM.h"

/**	This routine will read an integer from the file whose descriptor
 * that is passed to it.  White space before the integer will be skipped.
 * One character past the integer will be read, so it will hopefully be
 * white space that is not needed.
 *	This routine will skip anything between a '#' character and the
 * end of the line on which the character is found - this information is
 * a comment in the ppm file.
 *	This routine returns 0 on success and -1 on failure. */

static	int	read_int(FILE *infile, int *i)
{
	int	temp = 0;
	char	c;
	int	done_skipping = 0;

	/* Look for the first character, skipping any white space or comments */
	do {
		do {
			fread(&c, 1, 1, infile);
		} while ( isspace(c) );

		/* If this is the comment character, skip to the end of line */
		if (c == '#') {	/* Comment - skip this line and start on next */
			do {
				if (fread(&c, 1, 1, infile) != 1) {
					return(-1);
				}
			} while (c != '\n');
		} else {	/* No comment - this better be the integer! */
			done_skipping = 1;
		}
	} while (!done_skipping);
	if (!isdigit(c)) return(-1);

	/* Read the characters in and make the integer */
	while (isdigit(c)) {
		temp = temp*10 + (c - '0');
		fread(&c,1, 1, infile);
	}

	*i = temp;
	return(0);
}

PPM::PPM (const char *filename)
{
    FILE *infile = fopen(filename, "rb");
    if (!infile) {
	fprintf(stderr, "PPM::PPM(%s): file not found\n", filename);
	valid = 0;
	return;
    }
    PPM((FILE *)infile);
}

/**	This routine will read a ppm file from the file whose descriptor
 * is passed to it into an internal structure. */

PPM::PPM (FILE * infile)
{
	char		magic[10];
	int		maxc;		/* Maximum color value */

	/* Scan the header information from the ppm file */
	if (fread(magic, 2, 1, infile) != 1) {
		perror("Could not read magic number from ppm file");
		perror("Could not open ppm file");
		valid = 0;
		return;
	}
	if (read_int(infile,&nx)) {
		perror("Could not read number of x pixels");
		valid = 0;
		return;
	}
	if (read_int(infile,&ny)) {
		perror("Could not read number of y pixels");
		valid = 0;
		return;
	}
	if (read_int(infile,&maxc)) {
		perror("Could not read maximum value");
		valid = 0;
		return;
	}

	// Read in the body of the file
	if (strncmp(magic,"P6",2) == 0) {	// Binary PPM file

		if (maxc != 255) {
		    fprintf(stderr,"Max color not 255.  Can't handle it.\n");
		    valid = 0;
		    return;
		}

		if (read_P6_body(infile)) {
			valid = 0;
			return;
		}

	} else if (strncmp(magic,"P5",2) == 0) { // Binary PGM file

		if (maxc != 255) {
		    fprintf(stderr,"Max color not 255.  Can't handle it.\n");
		    valid = 0;
		    return;
		}

		if (read_P5_body(infile)) {
			valid = 0;
			return;
		}

	} else if (strncmp(magic,"P3",2) ==0) {	// ASCII PPM file
		if (read_P3_body(infile,maxc)) {
			valid = 0;
			return;
		}

	} else if (strncmp(magic,"P2",2) ==0) {	// ASCII PGM file
		if (read_P3_body(infile,maxc)) {
			valid = 0;
			return;
		}

	} else {
		fprintf(stderr,"Magic number not P3 or P6 (not PPM file)\n");
		valid = 0;
		return;
	}

	valid = 1;
} // end of PPM::PPM()

int	PPM::read_P5_body(FILE *infile)
{
	unsigned char	*pgmrow;		/* One row from the pgm file */
	int		x,y;
        int 	n_values_read;

	/* Set up the buffer for one row of the image */
	if ( (pgmrow = (unsigned char *)malloc(nx)) == NULL) {
		fprintf(stderr,"Not enough memory for pgm row buffer\n");
		return -1;
	}

	/* Allocate the ppm info structure and fill it in */
	if ((pixels = (PPMROW*)malloc(ny*sizeof(PPMROW*))) == NULL) {
		fprintf(stderr,"Not enough memory for ppm info buffer\n");
		return -1;
	}
	for (y = 0; y < ny; y++) {
		if ((pixels[y] =
			(PPMROW)malloc(nx*sizeof(P_COLOR))) == NULL) {
			fprintf(stderr,"Not enough memory for ppm pixels\n");
			return -1;
		}
	}

	for (y = 0; y < ny; y++) {

	  /* Read one row of values from the file */
	  n_values_read = fread((char*)pgmrow, 1, nx, infile);
          if (n_values_read != nx) {
                perror("PPM::read_P5_body read error:");
                fprintf(stderr, "Cannot read line %d of pixels from pgm file\n",
                        y);
                fprintf(stderr, "read %d values, expected %d of them\n",
                        n_values_read, nx);
                return -1;
          }

	  // Copy the values into the texture pixels
	  // This is a graymap -- copy same color into R,G and B
	  for (x = 0; x < nx; x++) {
	    ((pixels)[y])[x][0] = pgmrow[x];
	    ((pixels)[y])[x][1] = pgmrow[x];
	    ((pixels)[y])[x][2] = pgmrow[x];
	  }

	}

	free(pgmrow);

	return 0;

} // End of PPM::read_P5_body()

int	PPM::read_P6_body(FILE *infile)
{
	PPMROW		ppmrow;		/* One row from the ppm file */
	int		x,y;

	/* Set up the buffer for one row of the image */
	if ( (ppmrow = (P_COLOR *)malloc(nx*sizeof(P_COLOR))) == NULL) {
		fprintf(stderr,"Not enough memory for ppm row buffer\n");
		return -1;
	}

	/* Allocate the ppm info structure and fill it in */
	if ((pixels = (PPMROW*)malloc(ny*sizeof(PPMROW*))) == NULL) {
		fprintf(stderr,"Not enough memory for ppm info buffer\n");
		return -1;
	}
	for (y = 0; y < ny; y++) {
		if ((pixels[y] =
			(PPMROW)malloc(nx*sizeof(P_COLOR))) == NULL) {
			fprintf(stderr,"Not enough memory for ppm pixels\n");
			return -1;
		}
	}

	for (y = 0; y < ny; y++) {

	  /* Read one row of values from the file */
	  if (fread((char*)ppmrow, 1, nx*sizeof(P_COLOR), infile) !=
	      (unsigned)(nx*sizeof(P_COLOR))) {
		perror("Cannot read line of pixels from ppm file");
		return -1;
	  }

	  // Copy the values into the texture pixels
	  for (x = 0; x < nx; x++) {
	    ((pixels)[y])[x][0] = ppmrow[x][0];
	    ((pixels)[y])[x][1] = ppmrow[x][1];
	    ((pixels)[y])[x][2] = ppmrow[x][2];
	  }

	}

	free(ppmrow);

	return 0;

} // End of PPM::read_P6_body()

int	PPM::read_P2_body(FILE *infile, int maxc)
{
	double	scale = 256.0/( ((double)maxc)+1);
	int	x,y;
	int	V;

	/* Allocate the ppm info structure and fill it in */
	if ((pixels = (PPMROW*)malloc(ny*sizeof(PPMROW*))) == NULL) {
		fprintf(stderr,"Not enough memory for ppm info buffer\n");
		return -1;
	}
	for (y = 0; y < ny; y++) {
		if ((pixels[y] =
			(PPMROW)malloc(nx*sizeof(P_COLOR))) == NULL) {
			fprintf(stderr,"Not enough memory for ppm pixels\n");
			return -1;
		}
	}

	// Read in the value for each pixel.  Convert each from the range
	// 0..maxc into the range 0..255, then assign to each of R,G,B
	// the same color (this is a grayscale file).
	for (y = 0; y < ny; y++) {
	  for (x = 0; x < nx; x++) {
	    if (read_int(infile,&V)) {
		fprintf(stderr,"Error reading value!\n");
		return -1;
	    } else {
		((pixels)[y])[x][0] = (int)(V*scale);
		((pixels)[y])[x][1] = (int)(V*scale);
		((pixels)[y])[x][2] = (int)(V*scale);
	    }
	  }

	}

	return 0;

} // End of PPM::read_P2_body()

int	PPM::read_P3_body(FILE *infile, int maxc)
{
	double	scale = 256.0/( ((double)maxc)+1);
	int	x,y;
	int	R,G,B;

	/* Allocate the ppm info structure and fill it in */
	if ((pixels = (PPMROW*)malloc(ny*sizeof(PPMROW*))) == NULL) {
		fprintf(stderr,"Not enough memory for ppm info buffer\n");
		return -1;
	}
	for (y = 0; y < ny; y++) {
		if ((pixels[y] =
			(PPMROW)malloc(nx*sizeof(P_COLOR))) == NULL) {
			fprintf(stderr,"Not enough memory for ppm pixels\n");
			return -1;
		}
	}

	// Read in the RGB values for each pixel.  Convert each from the range
	// 0..maxc into the range 0..255.
	for (y = 0; y < ny; y++) {
	  for (x = 0; x < nx; x++) {
	    if (read_int(infile,&R) || read_int(infile,&G) ||
			read_int(infile,&B)) {
		fprintf(stderr,"Error reading value!\n");
		return -1;
	    } else {
		((pixels)[y])[x][0] = (int)(R*scale);
		((pixels)[y])[x][1] = (int)(G*scale);
		((pixels)[y])[x][2] = (int)(B*scale);
	    }
	  }

	}

	return 0;

} // End of PPM::read_P3_body()


/**	This routine will return the given element from the ppm info
 * structure.  The red, green, and blue values are filled in.
 *	0 is returned on success and -1 on failure. */

int	PPM::Tellppm(int x,int y, int *red,int *green,int *blue)
{
	if ( (x < 0) || (x >= nx) ) return(-1);
	if ( (y < 0) || (y >= ny) ) return(-1);

	*red = pixels[y][x][0];
	*green = pixels[y][x][1];
	*blue = pixels[y][x][2];

	return(0);
}


/**	This routine will set the given element in the ppm info
 * structure.  The red, green, and blue values are filled in.
 *	0 is returned on success and -1 on failure. */

int	PPM::Putppm(int x,int y, int red,int green,int blue)
{
	if ( (x < 0) || (x >= nx) ) return(-1);
	if ( (y < 0) || (y >= ny) ) return(-1);

	pixels[y][x][0] = red;
	pixels[y][x][1] = green;
	pixels[y][x][2] = blue;

	return(0);
}


/**	This routine will return the given element from the ppm info
 * structure.  The red, green, and blue values are filled in.
 *	This routine takes a float from -1 to 1 in x and y and maps this into
 * the whole range of x and y for the ppm file.
 *	Bilinear interpolation is performed between the pixel values to
 * keep from getting horrible aliasing when the picture is larger than
 * the texture map.
 *	This routine inverts y so that the ppm image comes up rightside-
 * up.
 *	0 is returned on success and -1 on failure. */

int	PPM::Value_at_normalized(double normx,double normy, int *red,int *green,int *blue)
{
	float	sx = normx, sy = normy;	/* Scaled to 0-->1 */
	int x,y;			/* Truncated integer coordinate */
	double	dx,dy;			/* Delta from the integer coord */

	/* Map normx, normy to 0-->1 range */
	sx = (sx+1)/2;
	sy = 1-(sy+1)/2;

	if ( (sx < 0) || (sx >= 1) ) {
		*red = *green = *blue = 0;
		return(0);
	}
	if ( (sy < 0) || (sy >= 1) ) {
		*red = *green = *blue = 0;
		return(0);
	}

	x = (int)( sx * (nx - 1) );
	y = (int)( sy * (ny - 1) );

	/* Find the normalized distance from the integer coordinate
	 * (the d's will be from 0 to 1 and will represent weighting) */

	dx = sx * (nx - 1) - x;
	dy = sy * (ny - 1) - y;

	/* If we are at the right or bottom edge, no interpolation */

	if ( (x == (nx - 1)) || (y == (ny - 1)) ) {
		*red = pixels[y][x][0];
		*green = pixels[y][x][1];
		*blue = pixels[y][x][2];

	/* Otherwise, perform interpolation based on the d's */
	} else {
		*red = (int)( pixels[y][x][0] * (1-dx)*(1-dy) +
			pixels[y][x+1][0] * dx*(1-dy) +
			pixels[y+1][x][0] * (1-dx)*dy +
			pixels[y+1][x+1][0] * dx*dy );
		*green=(int)( pixels[y][x][1] * (1-dx)*(1-dy) +
			pixels[y][x+1][1] * dx*(1-dy) +
			pixels[y+1][x][1] * (1-dx)*dy +
			pixels[y+1][x+1][1] * dx*dy );
		*blue =(int)( pixels[y][x][2] * (1-dx)*(1-dy) +
			pixels[y][x+1][2] * dx*(1-dy) +
			pixels[y+1][x][2] * (1-dx)*dy +
			pixels[y+1][x+1][2] * dx*dy );
	}

	return(0);
}

int	PPM::Write_to(FILE *outfile)
{
	int	y;

	/* Write the header information to the ppm file */
	if (fprintf(outfile,"P6\n%d %d\n255\n",nx,ny) <= 0) {
		perror("PPM:Write_to(): Could write PPM header");
		fclose(outfile);
		return(-1);
	}

	for (y = 0; y < ny; y++) {

	  /* Write one row of values to the file */
	  if (fwrite(pixels[y], sizeof(P_COLOR),nx,outfile) != (unsigned)nx) {
		perror("PPM:Write_to(): Cannot write line of pixels");
		fclose(outfile);
		return(-1);
	  }
	}

	if (fclose(outfile) == EOF) {
		perror("PPM:Write_to(): Cannot close file");
		return(-1);
	}

	return(0);
}
