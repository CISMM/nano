#include	<stdlib.h>
#include	<stdio.h>
#include	<math.h>

#define	RETURN_RGB(x,y,z) {*r=x; *g=y; *b=z; break;}

// From Foley & van Dam
void	hsv_to_rgb(float h, float s, float v, float *r, float *g, float *b)
{
        // H is given on [0, 360]. S and V are given on [0, 1]. 
        // RGB are each returned on [0, 1]. 
        float f,p,q,t; 
        int i; 

	h /= 60;		// Range of h to [0-6]
	if (h == 6) {h = 0;};	// Range of h to [0-6>

	i = (int)floor(h);	// Which sextant is it in?
	f = h-i;		// Fractional part within sextant
	p = v*(1-(s));
	q = v*(1-(s*f));
	t = v*(1-(s*(1-f)));
	switch (i) {
	  case 0: RETURN_RGB(v,t,p);
	  case 1: RETURN_RGB(q,v,p);
	  case 2: RETURN_RGB(p,v,t);
	  case 3: RETURN_RGB(p,q,v);
	  case 4: RETURN_RGB(t,p,v);
	  case 5: RETURN_RGB(v,p,q);
	}
}

void	main(unsigned argc, char *argv[])
{
	float	r,g,b, h,s,v;
	float	i;
	int	COUNT = 200;

	// Print header
	printf("Colormap\n");
	printf("Value\tRed\tGreen\tBlue\tAlpha\n");
	printf("-----------------------------------------------------\n");

	// Print a random-hue colormap
	s = 1.0;	// Completely saturated
	v = 1.0;	// Constant intensity
	for (i = 0; i < COUNT; i++) {
		h = rand()/(32767.0)*255;
		hsv_to_rgb(h,s,v, &r,&g,&b);
		printf("%7.3g\t%d\t%d\t%d\t%d\n", i/(COUNT-1),
			(int)(r*255),(int)(g*255),(int)(b*255), 255);
	}
}

