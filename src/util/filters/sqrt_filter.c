/****************************************************************************
				sqrt.c

	Reads in a bunch of raw floats.  Takes the sqrt() of each.  Writes
 out the new raw floats.  Produces no other output to stdout.  This is
 intended as a filter for datasets from the nanoManipulator.
	Puts in 0 for square root of negative numbers.
	Reads from standard input and writes to standard output.
	Exits with value 0 if it found an integral number of floats in the
 input file.  Exits with value -1 if not.
 ****************************************************************************/

#include	<math.h>
#include	<stdlib.h>
#include	<stdio.h>

const	int	BLOCKSZ = 1000;

main(void)
{
	float	buf[BLOCKSZ];
	int	count;
	int	i;
	int	retcode = 0;

	do {
		// Read in blocks of BLOCKSZ to decrease read/write overhead.
		count = fread(buf, sizeof(float), BLOCKSZ, stdin);

		// Take the square root of the ones we got.
		for (i = 0; i < count; i++) {
		  if (buf[i] < 0) {
			buf[i] = 0.0;
		  } else {
			buf[i] = sqrt(buf[i]);
		  }
		}

		// Write the ones we got back out.
		if (fwrite(buf, sizeof(float), count, stdout) != count) {
			perror("sqrt: Can't write output");
			retcode = -1;
			break;	// Break out of the 'do' loop
		}

	} while (count == BLOCKSZ);

	exit(retcode);
}

