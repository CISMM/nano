/******************************************************************************
				slope_filter.c
Reads a block of X by Y floats into an array.  The code computes the slope at
each element of the block.  This is accomplished by subtracting the element to
the left of a given element from the one on the right of the same element.
This value is then squared.  Then the element above the given element is
subtracted from the one below the given element.  Then this value is squared.
Finally, the two squared values are added together.  This is done for the
entire block of floats.  On the edges, the slope is computed using 3 neighbors,
and in the corners it is computed with only 2. 

The parameters are nx, ny (number of samples in x and y, respectively).
The code exits with no output and status -1 if it can't read all of its input.
It also exits with -1 if it can't write all of its output.
If everything works, it exits with code 0.
*******************************************************************************/

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

//takes a float as a parameter and returns the square of its value
float square(float num)
{
	return num*num;
}

//line is either a column or a row vector.  num is the length of the vector.
//diffSq subtracts the entry in the vector that is one smaller than the current
//entry from the one that has a position that is one greater.  Then this value
//is divided by the stepsize, which in this case is 1, resulting in calculating
//a normalized slope.
   
void diffSq(float *line, int num)
{
	float temp[num];
	for (int i = 0; i < num; i++) {
		if (i==0) {
			temp[i] = square(line[i] / 2); 
		}
		else if (i == num - 1) {
			temp[i] = square(line[i] / 2);
		}
		else {
			temp[i] = square(line[i+1] - line[i-1]);
		}
	}
	for (int i = 0; i < num; i++) {
		line[i] = temp[i];
	}
}

//produces an error message and returns -1 if something goes wrong

void Usage(char *s)
{
	fprintf(stderr,"Usage: %s nx ny\n",s);
	fprintf(stderr,"   nx ny: Number of samples in x and y\n");
	exit(-1);
}

int main(unsigned argc, char *argv[])
{
	int nx, ny;	//Number of samples in x and y
	int ret;
	float *M, *TC, *TR, *sqCols, *sqRows;	

	//Parse the command line
	if (argc != 3) { Usage(argv[0]); }
	nx = atoi(argv[1]); //number of data entries in x
	ny = atoi(argv[2]); //number of data entries in y
	if ((nx < 1) || (ny < 1)) { Usage(argv[0]); }

	//Allocate arrays
	if (((M = new float[nx*ny]) == NULL) ||
	    ((TC = new float[ny]) == NULL) ||
	    ((TR = new float[nx]) == NULL) ||
	    ((sqCols = new float[nx*ny]) == NULL) ||
	    ((sqRows = new float[nx*ny]) == NULL)) {
		fprintf(stderr,"%s: Out of memory\n",argv[0]);
		return(-1);
	}

	//Read the data into M(x,y)
	if ((ret = fread(M, sizeof(float), nx*ny, stdin)) != nx*ny) {
		fprintf(stderr, "%s: Can't read %d values for %dx%d grid, got %d\n", argv[0], nx*ny, ret);
		return(-1);
	}

	for (int x = 0; x < nx; x++) {
		//Get a column of M into TC.  (TC is temporary column.)
		for (int y = 0; y < ny; y++) {
			TC[y] = M[x+y*nx];
		}

		//Get a row of M into TR.  (TR is temporary row.)
		for (int y = 0; y < ny; y++) {
			TR[y] = M[x*ny+y];
		}

		diffSq(TC, ny);
		diffSq(TR, nx);

		//Move TC into sqCols and TR into sqRows
		for (int y = 0; y < ny; y++) {
			sqCols[x+y*nx] = TC[y];
			sqRows[x*ny+y] = TR[y];
		}
	}
	
	//Add sqCols and sqRows and take sqrt of sum.  Assign the result to M.
	for (int i = 0; i < nx*ny; i++) {
		M[i] = sqrt(sqCols[i] + sqRows[i]);
	}	

	//Write the output data
	if(fwrite(M, sizeof(float), nx*ny, stdout) != nx*ny) {
		perror("slope: Can't write plane");
		return(-1);
	}

	return(0);
}
	
