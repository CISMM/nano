#ifndef	PPM_H
#define PPM_H

typedef	unsigned char	P_COLOR[3];	/**< Red, Green, Blue */
typedef	P_COLOR		*PPMROW;

class	PPM {
    public:
	int	valid;		///< Is this a valid one?

	int	nx,ny;		///< Number of pixels in X and Y
	PPMROW	*pixels;	///< The rows of pixel values

	int	Tellppm(int x, int y, int *red, int *green, int *blue);
	int	Putppm(int x, int y,  int red, int green, int blue);

	PPM (FILE *file = NULL);
        PPM (const char *filename = NULL);

	int	Value_at_normalized(double nx, double ny, int *red,
			int *green, int *blue);
	int	Write_to(FILE *file);

    protected:

    private:
	void	constructor_worker(FILE * infile);
	int	read_P6_body(FILE *infile);
	int	read_P3_body(FILE *infile, int maxc);
	int	read_P5_body(FILE *infile);
	int	read_P2_body(FILE *infile, int maxc);
};

#endif

