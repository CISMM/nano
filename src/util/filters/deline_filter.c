/****************************************************************************
				deline.c

	Reads a block of X by Y floats into an array.  This is an image that
 has been scanned and has line artifacts in it.  The code attempts to remove
 the high-frequency components of this scanline noise from the image without
 removing slope or other information from the actual data.
	The algorithm is to assume that the image M(x,y) is composed of two
 signals: S(x,y) is the surface data and N(y) is the noise (assumed to depend
 only on y, not on x).  We apply a low-pass filter to M(x,y) in only the y
 direction to produce L(x,y) [low-pass version].  We form the high-pass
 array as: H(x,y) = M(x,y) - L(x,y).  This gives us a high-pass (in the y
 direction) version of the original function.  The assumption is that the
 high-frequency components of S(x,y) are zero-mean.  Thus, we find the
 function N(y) as the average over x of H(x,y): N(y) = SUMx(H(x,y))/nx.
 We produce the output O(x,y) = M(x,y) - N(y).
	The parameters are nx, ny (number of samples in x and y), f (cutoff
 frequency for low-pass in pixels) and p (number of passes of the low-pass
 filter).
	The code exits with no output and status -1 if it can't read all
 of its input.  It exits with status -1 if it can't write all of its output.
 If things work, it exits with code 0.
 ****************************************************************************/

#include	<math.h>
#include	<string.h>
#include	<stdlib.h>
#include	<stdio.h>

const	int	BLOCKSZ = 1000;

// I got this function from Gary Bishop.  He got it from a NASA tech
// report.  It does a non-causal low-pass filter of a vector in several
// passes, returning the filtered vector.
// The vector (of length 'len') is modified in-place.  The filter cutoff
// is specified in samples/cycle as 'freq'.  The number of passes to make
// with the algorithm are specified in 'passes'.
// Note: Original function specified parameters as dt (distance between
//       sample points [dist/sample]) and fc (frequency of cutoff in Hz
//       [cycles/dist]).  This version calculates those parameters from
//       'freq [sample/cycle]' and runs the original code.

void low_pass_filter_line(double *vec, int len, double freq, int passes)
{
    double dt = 1.0;			// Assume unit sample spacing
    double fc = dt/freq;

    double wc = 2 * M_PI * fc;
    double tau = sqrt(pow(2.0, 1.0/(2*passes)) - 1) / wc;

    double k1 = exp(-dt/tau);
    double k2 = -(k1 - tau/dt * (1 - k1));
    double k3 = 1 - tau/dt * (1 - k1);

    for(int p=0; p<passes; p++) {
        double g0 = (vec[0]+vec[1])/2;
	int i;
        for(i=0; i<len-1; i++) {
            double g1 = k1*g0 + k2*vec[i] + k3*vec[i+1];
            vec[i] = g0;
            g0 = g1;
        }
        vec[len-1] = g0;
        g0 = (vec[len-1]+vec[len-2])/2;
        for(i=len-1; i>0; i--) {
            double g1 = k1*g0 + k2*vec[i] + k3*vec[i-1];
            vec[i] = g0;
            g0 = g1;
        }
        vec[0] = g0;
    }
}

void	Usage(char *s)
{
	fprintf(stderr,"Usage: %s nx ny freq passes\n",s);
	fprintf(stderr,"       nx ny: Number of samples in x and y\n");
	fprintf(stderr,"       freq: pixels/cycle cutoff of filter\n");
	fprintf(stderr,"       passes: number of filter passes\n");
	exit(-1);
}

int main(unsigned argc, char *argv[])
{
	int	nx,ny;		// Number of samples in x and y
	float	freq;		// Samples/cycle cutoff frequency
	int	passes;		// Number of passes for filter
	float	*M, *N, *L, *H;
	double	*T;
	int	x,y;
	int	ret;

	// Parse the command line
	if (argc != 5) { Usage(argv[0]); }
	nx = atoi(argv[1]);
	ny = atoi(argv[2]);
	freq = atof(argv[3]);
	passes = atoi(argv[4]);
	if ( (nx < 1) || (ny < 1) || (freq <= 0) || (passes < 1) ) {
		Usage(argv[0]);
	}

	// Allocate the needed arrays
	if ( ((M = new float[nx*ny]) == NULL) ||
	     ((L = new float[nx*ny]) == NULL) ||
	     ((H = new float[nx*ny]) == NULL) ||
	     ((N = new float[ny]) == NULL) ||
	     ((T = new double[ny]) == NULL) ) {
		fprintf(stderr,"%s: Out of memory\n",argv[0]);
		return(-1);
	}

	// Read the data into M(x,y)
	if ( (ret = fread(M, sizeof(float), nx*ny, stdin)) != nx*ny) {
		fprintf(stderr,
			"%s: Can't read %d values for %dx%d grid, got %d\n",
			argv[0], nx*ny, nx,ny, ret);
		return(-1);
	}

	// Apply the low-pass filter to produce L(x,y) one line at a time
	// Uses T(y) as the temporary vector to hold each for filtering
	for (x = 0; x < nx; x++) {
		// Get a column of M into T
		for (y = 0; y < ny; y++) { T[y] = M[x+y*nx]; }

		// Filter the column
		low_pass_filter_line(T, ny, freq, passes);

		// Move the column into L
		for (y = 0; y < ny; y++) { L[x+y*nx] = T[y]; }
	}

	// Subtract to get H(x,y)
	for (x = 0; x < nx; x++) {
	  for (y = 0; y < ny; y++) {
		H[x+y*nx] = M[x+y*nx] - L[x+y*nx];
	  }
	}

	// Average across rows to find N(y)
	for (y = 0; y < ny; y++) {
		N[y] = 0.0;
		for (x = 0; x < nx; x++) { N[y] += H[x+y*nx]; };
		N[y] /= nx;
	}

	// Subtract N(y) from M(x,y) to produce the output image
	for (x = 0; x < nx; x++) {
	  for (y = 0; y < ny; y++) {
		M[x+y*nx] -= N[y];
	  }
	}

	// Write the output image
	if (fwrite(M, sizeof(float), nx*ny, stdout) != nx*ny) {
		perror("deline: Can't write plane");
		return(-1);
	}

	return(0);
}

