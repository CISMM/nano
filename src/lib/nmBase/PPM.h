#ifndef	PPM_H
#define PPM_H

typedef	unsigned char	P_COLOR[3];	/* Red, Green, Blue */
typedef	P_COLOR		*PPMROW;

class	PPM {
    public:
	int	valid;		// Is this a valid one?

	int	nx,ny;		// Number of pixels in X and Y
	PPMROW	*pixels;	// The rows of pixel values

	int	Tellppm(int x, int y, int *red, int *green, int *blue);
	int	Putppm(int x, int y,  int red, int green, int blue);

	PPM (const char * filename = NULL);
	PPM (FILE *file);

	int	Value_at_normalized(double nx, double ny, int *red,
			int *green, int *blue);
	int	Write_to(char *filename = NULL);

    protected:

    private:
	int	read_P6_body(int infile);
	int	read_P3_body(int infile, int maxc);
	int	read_P5_body(int infile);
	int	read_P2_body(int infile, int maxc);
};

#endif

