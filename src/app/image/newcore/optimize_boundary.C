// Modified 10/98 by Mark Foskey

#include "core_ops.h"
#include "cimage_filter.h"
#include <math.h>

void 
optimize_boundary_sites(CorePoint C,   // A point in scale space
				       //   implying a pair of boundary
				       //   points
			int polarity,  // light on dark or vice versa
			cimage im,     // The image being analyzed
			CorePoint *B1, // The output points.
			CorePoint *B2, 
			float ratio)   // ???
{

    float ratio_s;   /* scale for boundary site */

    float t0, t1, t2; /* t0 is (d sigma (s)/ds)^2 */

    float theta;  /* BASOC direction angle */

    /* CorePoint P = core_point_get(C, CoreIndex); */

    CorePoint P = C; /* structure of the core */

    float site_x = P.x;  /* x coordinate of the site */
    float site_y = P.y;  /* y coordinate of the site */
    float site_s = P.s;  /* scale of the site */
    float dx = P.dx;  /* delta of the x in one step */
    float dy = P.dy;  /* delta of the y in one step */
    float ds = P.ds;  /* delta of the scale in one step */

    float length = sqrt(dx*dx+dy*dy); /* length of the direction vector */

    ratio_s = site_s/ratio; /* setup the Bdry site scale */

    /* find the BASOC direction */



    /*  theta = asin(1/(1+ (d(sigma(s)/d(s))**2 ); */

    t0 = (ds*ds/(dx*dx+dy*dy)); /* the denominator for theta */
    theta = acos(1.0/(1.0+t0));

    t1 = cos (theta);
    t2 = sin (theta);

    float max = 0.0;

    for (int i=0; i<=10; i++) {
	float t = (float) i*0.025; 
	float x1 = site_x - (t+0.875)*(dy*t1 - dx*t2)*site_s/length;
	float y1 = site_y + (t+0.875)*(dy*t2+ dx*t1)*site_s/length;
	float s = (t+0.875)*ratio_s;

	if (s < 1.0) s = 1.0; 

	// first_dir_deriv() computes the first derivative of the
	// intensity function  im  at the point (x1, y1) at the
	// scale  s  in the direction given by the last two params.
	// Defined in CIMAGE/src/cimage/filter/Deriv.c.
	float val = first_dir_deriv(im, x1, y1, s, 
				    (-polarity)*(x1-site_x), 
				    (-polarity)*(y1-site_y));

	float x2 = site_x - (t+0.875)*(-dy*t1 - dx*t2)*site_s/length;
        float y2 = site_y + (t+0.875)*(dy*t2 - dx*t1)*site_s/length;

	val += first_dir_deriv(im, x2, y2, s, 
			       (-polarity)*(x2-site_x), 
			       (-polarity)*(y2-site_y));
	if (val > max) {
	    max = val;
	    B1->x = x1;
	    B1->y = y1;
	    B1->s = s;
	    B2->x = x2;
	    B2->y = y2;
	    B2->s = s;
	}
    }
/*
  max = 0.0;
  for (int i=0; i<=10; i++) {
  float t = (float) i*0.025;
  float x2 = site_x - (t+0.875)*(-dy*t1 - dx*t2)*site_s/length;
  float y2 = site_y + (t+0.875)*(dy*t2 - dx*t1)*site_s/length;
  float s = (t+0.875)*ratio_s;

  if (s < 1.0) s = 1.0;

  float val = first_dir_deriv(im, x2, y2, s, x2-site_x, y2-site_y);

  if (val > max) {
  max = val;
  B2->x = x2;
  B2->y = y2;
  B2->s = s;
  }
  }
  */

}

