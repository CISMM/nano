#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "cnt_ia.h"
#include "cnt_ps.h"
#include "ppm.h"

#include <BCPlane.h>

CNT_IA::CNT_IA()
{
	cnt_image = NULL;	// image array
	cnt_image_x = 0;	// image size X
	cnt_image_y = 0;	// image size Y

	cnt_scale_x = 0.0;	// pixel <-> nano
	cnt_scale_y = 0.0;
	cnt_scale_z = 0.0;

	cnt_sigma = SIGMA;		// parameter for image blurring
	cnt_aspect = ASP_RAT;	// aspect ratio threshold
	cnt_intensity = INTENS;	// intensity threshold
	cnt_correlate = CORREL;	// correlation threshold

	cnt_autoparam = 0;		// no automatic parameter adaption
	cnt_tubes = NULL;		// pointer to CNTs ( array )
}


CNT_IA::~CNT_IA()
{
}


// read in image array from BCPlane
void CNT_IA::cnt_image_read(BCPlane *imagePlane) //*
{
	int x, y, imgX, imgY;
//	unsigned char *img;

//	img = NULL;
//	LoadPPM(imgFile, img, imgX, imgY);
	imgX = (int)imagePlane->numX();
	imgY = (int)imagePlane->numY();

	cnt_image = new double[imgX * imgY];

	for ( y=0; y<imgY; y++ ) {
		for ( x=0; x<imgX; x++ ) {
			cnt_image[y*imgX+x] = (double)imagePlane->value(x,imgY-1-y);
		}
	}

	cnt_image_x = imgX;
	cnt_image_y = imgY;
	cnt_image_height = imagePlane->maxValue() - imagePlane->minValue();
}


// read in image array from ppm file
void CNT_IA::cnt_image_read(char *imgFile)
{
	int x, y, imgX, imgY;
	unsigned char *img;

	img = NULL;
	LoadPPM(imgFile, img, imgX, imgY);

	cnt_image = new double[imgX * imgY];

	for ( y=0; y<imgY; y++ ) {
		for ( x=0; x<imgX; x++ ) {
			cnt_image[y*imgX+x] = (double)img[3*(y*imgX+x)];
		}
	}

	cnt_image_x = imgX;
	cnt_image_y = imgY;
}


void CNT_IA::cnt_image_setScal(double pitch_x, double pitch_y, double pitch_z)
{
	cnt_scale_x = pitch_x;
	cnt_scale_y = pitch_y;
	cnt_scale_z = pitch_z;
}


void CNT_IA::cnt_image_setSigm(double sigma)
{
	cnt_sigma = sigma;
}


void CNT_IA::cnt_image_setAspt(double aspect)
{
	cnt_aspect = aspect;
}


void CNT_IA::cnt_image_setIntn(double intensity)
{
	cnt_intensity = intensity;
}


void CNT_IA::cnt_image_setCorr(double correlate)
{
	cnt_correlate = correlate;
}


void CNT_IA::cnt_image_setAuto(int autoparam)
{
	cnt_autoparam = autoparam;
}


// write out image array in PPM format
void CNT_IA::cnt_image_write(char *imgFile, double *imgSrc)
{
	int x, y;
	unsigned char *img;
	double pixel, maxI, minI, denom;

	img = new unsigned char[3 * cnt_image_x * cnt_image_y];

	maxI = -1.0E9;
	minI = +1.0E9;

	for ( y=0; y<cnt_image_y; y++ ) {
		for ( x=0; x<cnt_image_x; x++ ) {

			pixel = imgSrc[y*cnt_image_x+x];

			if ( pixel > maxI )
				maxI = pixel;
			else if (pixel < minI )
				minI = pixel;
		}
	}

	denom = maxI - minI;
	if ( denom < 1.0E-9 )
		denom = 1.0E-9;

	for ( y=0; y<cnt_image_y; y++ ) {
		for ( x=0; x<cnt_image_x; x++ ) {
			pixel = 255.0 * (imgSrc[y*cnt_image_x+x] - minI) / denom;
			img[3*(y*cnt_image_x+x)+0] = (unsigned char)(pixel);
			img[3*(y*cnt_image_x+x)+1] = img[3*(y*cnt_image_x+x)+0];
			img[3*(y*cnt_image_x+x)+2] = img[3*(y*cnt_image_x+x)+0];
		}
	}

	WritePPM(imgFile, img, cnt_image_x, cnt_image_y);
}

// flatten a nanotube image by removing any slope in the background
void CNT_IA::cnt_image_flat(void)
{
	int x, y;
	long N;
	double Sx, Sy, Sz, Sxx, Sxy, Syy, Sxz, Syz;
	double Lxx, Lxy, Lyy, Lxz, Lyz;
	double a, b, c, nm, nx, ny, nz;
	double pixel, maxI, minI, denom, *imgTmp;
	int binN, binS, cnt_bin[ADP_BIN];	// bins for intensity adaption

	N = cnt_image_x * cnt_image_y;

	// Allocate image array;
	cnt_image_Hgt = new double [N];

	Sx = 0.0;
	Sy = 0.0;
	Sz = 0.0;
	Sxx = 0.0;
	Sxy = 0.0;
	Syy = 0.0;
	Sxz = 0.0;
	Syz = 0.0;

	for ( y=0; y<cnt_image_y; y++ ) {
		for ( x=0; x<cnt_image_x; x++ ) {

			Sx += x;
			Sy += y;
			Sz += cnt_image[y*cnt_image_x+x];
			Sxx += x * x;
			Sxy += x * y;
			Syy += y * y;
			Sxz += x * cnt_image[y*cnt_image_x+x];
			Syz += y * cnt_image[y*cnt_image_x+x];
		}
	}

	Lxx = Sxx - Sx * Sx / N;
	Lxy = Sxy - Sx * Sy / N;
	Lyy = Syy - Sy * Sy / N;
	Lxz = Sxz - Sx * Sz / N;
	Lyz = Syz - Sy * Sz / N;

	a = (Lyy * Lxz - Lxy * Lyz) / (Lxx * Lyy - Lxy * Lxy);
	b = (Lxx * Lyz - Lxy * Lxz) / (Lxx * Lyy - Lxy * Lxy);
	c = Sz / N - a * (cnt_image_x - 1) * 0.5 - b * (cnt_image_y - 1) * 0.5;
	printf("a = %8.3lf; b = %8.3f; c = %8.3f\n", a, b, c);

	nm = sqrt(a*a + b*b + 1);	// mod of normal vector (a, b, -1)
	nx = -a / nm;
	ny = -b / nm;
	nz = 1.0 / nm;

// scale pixel value to ( 0.0, 255.0 )

	maxI = -1.0E9;
	minI = +1.0E9;
	imgTmp = new double[N];

	for ( y=0; y<cnt_image_y; y++ ) {
		for ( x=0; x<cnt_image_x; x++ ) {

			pixel = nx * x + ny * y + nz * (cnt_image[y*cnt_image_x+x] - c);
			imgTmp[y*cnt_image_x+x] = pixel;

			if ( pixel > maxI )
				maxI = pixel;
			else if (pixel < minI )
				minI = pixel;
		}
	}

	denom = maxI - minI;
	if ( denom < 1.0E-6 )
		denom = 1.0E-6;

	for ( y=0; y<cnt_image_y; y++ ) {
	   for ( x=0; x<cnt_image_x; x++ ) {
	      cnt_image_Hgt[y*cnt_image_x+x] = sqrt((imgTmp[y*cnt_image_x+x]-minI) / denom) * denom;
	      cnt_image[y*cnt_image_x+x] = 255.0 * cnt_image_Hgt[y*cnt_image_x+x] / denom;
	   }
	}

// automatic parameter adaption

	if ( cnt_autoparam ) {

		binS = 256 / ADP_BIN;

		for ( y=0; y<ADP_BIN; y++ )	// reset cnt_bin[]
			cnt_bin[y] = 0;

		for ( y=0; y<cnt_image_y; y++ ) {
			for ( x=0; x<cnt_image_x; x++ ) {
				cnt_bin[(int)(cnt_image[y*cnt_image_x+x]/binS)] ++;
			}
		}

		binN = 0;
		for ( y=1; y<ADP_BIN; y++ ) {
			if ( cnt_bin[0] < cnt_bin[y] ) {
				cnt_bin[0] = cnt_bin[y];
				binN = y;
			}
		}

		cnt_intensity = (binS * binN + 255.0) / 2.0;
		printf("\n\n\n");
		printf("Intensity threshold is set to %lf \n", cnt_intensity);
		printf("\n\n\n");
	}
}


// Gaussian blur the image, storing the blurred image in cnt_Image_Blr
void CNT_IA::cnt_image_filter(void)
{
	long N = cnt_image_x * cnt_image_y;

// allocate image arrays

	cnt_image_Blr = new double [N];
	cnt_image_Rdg = new double [N];
	cnt_image_Med = new double [N];
	cnt_image_Ord = new double [N];
	cnt_image_Msk = new double [N];

	cnt_image_H1x = new double [N];
	cnt_image_H1y = new double [N];
	cnt_image_H2x = new double [N];
	cnt_image_H2y = new double [N];

	cnt_image_Hxx = new double [N];
	cnt_image_Hxy = new double [N];
	cnt_image_Hyx = new double [N];
	cnt_image_Hyy = new double [N];

	cnt_image_Hpp = new double [N];
	cnt_image_Hqq = new double [N];
	cnt_image_Vpp = new double [N];
	cnt_image_Vqq = new double [N];

	cnt_image_Tid = new long [N];

// apply filters

	filterH1x(cnt_image, cnt_image_H1x);
	filterH1y(cnt_image, cnt_image_H1y);
	filterH2x(cnt_image, cnt_image_H2x);
	filterH2y(cnt_image, cnt_image_H2y);

	filterH3x(cnt_image_H1y, cnt_image_Hxx);
	filterH3y(cnt_image_H1x, cnt_image_Hyy);
	filterH2y(cnt_image_H2x, cnt_image_Hxy);
	filterH2x(cnt_image_H2y, cnt_image_Hyx);

	filterH1y(cnt_image_H1x, cnt_image_Blr);
}


// compute which pixels fall along the medial axis of the image
// set cnt_image_Med[i] to 250 if the axis passes through pixel i,
//   0 otherwise
void CNT_IA::cnt_image_medial(void)
{
	int x, y;
	long index;
	double hxx, hxy, hyx, hyy, x1, y1, x2, y2;
	double dxy, S, vectx, vecty, norm;
	double Xpr, Ypr, Zpr;

// solve for Hessian eigen-values and eigen-vectors
	for (y=0; y<cnt_image_y; y++) {
		for(x=0; x<cnt_image_x; x++) {

			index = y * cnt_image_x + x;

			hxx = cnt_image_Hxx[index];
			hxy = cnt_image_Hxy[index];
			hyx = cnt_image_Hyx[index];
			hyy = cnt_image_Hyy[index];

			dxy = hxx - hyy;
			S = sqrt(dxy*dxy + 4.0*hxy*hyx);

			cnt_image_Hpp[index] = ( hxx + hyy - S ) / 2.0;
			cnt_image_Hqq[index] = ( hxx + hyy + S ) / 2.0;

			vectx = hxy;
			vecty = ( - dxy - S ) / 2.0;
			norm  = sqrt( vectx*vectx + vecty*vecty );

			if( norm > 1.0E-9 ) {
				cnt_image_Vpp[index] = vectx / norm;
				cnt_image_Vqq[index] = vecty / norm;
			}
			else {
				cnt_image_Vpp[index] = 0.0;
				cnt_image_Vqq[index] = 0.0;
			}

// ridge computation

			x1 = cnt_image_H2x[index];
			y1 = cnt_image_H2y[index];
			x2 = cnt_image_Vpp[index];
			y2 = cnt_image_Vqq[index];

			cnt_image_Rdg[index] = x1*x2 + y1*y2;

// pattern recognition analysis

			Xpr = cnt_image_Hpp[index];	// x, y, z values in the parameter space
			Ypr = cnt_image_Rdg[index];
			Zpr = cnt_image_Blr[index];

			if ( cnt_image_pattern(Xpr, Ypr, Zpr) ) {
				cnt_image_Med[index] = 250.0;
			}
			else {
				cnt_image_Med[index] = 0.0;
			}
		}
	}
}

	
// pattern recognition helper function, used by cnt_image_medial()
int CNT_IA::cnt_image_pattern(double x, double y, double z)
{
	if ( x > -2.00 )  // NEW -- used to be "x > 0.0".  Now it ignores pretty flat regions.
		return(0);

	else if ( fabs(atan(y/x)) > (10.0*3.14/180.0) )
		return(0);

	else if ( z < cnt_intensity )
		return(0);

	else
		return(1);
}


// compute which pixels fall within a tube, using the medial axis
//   info from cnt_image_Med
// set cnt_image_Msk[i] to 255 if pixel i is determined to fall
//   within a tube, 0 otherwise
void CNT_IA::cnt_image_fit(void)
{
	int x, y, i, j;
	int i0, j0, i1, j1;
	long index1, index2;
	double opt_wid, p, q, pq_mod;
	double corr, max_corr;

	int num;
	double wid, inta, intb, u, v;
	double s_a, s_a2, s_b, s_b2, s_ab;

	// reset mask
	for ( y=0; y<cnt_image_y; y++ ) {
		for ( x=0; x<cnt_image_x; x++ ) {
			cnt_image_Msk[y*cnt_image_x+x] = 0.0;
		}
	}

	for ( y=0; y<cnt_image_y; y++ ) {
		for ( x=0; x<cnt_image_x; x++ ) {

			index1 = y * cnt_image_x + x;
			
			if ( cnt_image_Med[index1] > 1.0 ) {  // medial axis passes through this pixel

		           	p=0.0;
				q=0.0;
        		        opt_wid = 0.1;

				i0 = x - SAM_WID;
				i1 = x + SAM_WID;
				j0 = y - SAM_WID;
				j1 = y + SAM_WID;

				if ( i0<0 ) i0 = 0;
				if ( j0<0 ) j0 = 0;
				if ( i1>=cnt_image_x ) i1 = cnt_image_x-1;
				if ( j1>=cnt_image_y ) j1 = cnt_image_y-1;

				for ( i=i0; i<=i1; i++ ) {
		               		for ( j=j0; j<=j1; j++ ) {
                		  		if ( cnt_image_Med[index1] > 1.0 ) {
							index2 = j*cnt_image_x + i;
                     					p += cnt_image_Vpp[index2] / ( fabs(i-x) + 1 );
                     					q += cnt_image_Vqq[index2] / ( fabs(j-y) + 1 );
						}
					}
				}

				pq_mod = sqrt(p*p + q*q);
				p /= pq_mod;
				q /= pq_mod;

				if ( PQFLAG ) {
					double temp_pq;
					temp_pq = p;
					p = q;
					q = -temp_pq;
				}

				max_corr = cnt_correlate + 1.0E-6;

				for ( int wi=1; wi<TUBEWID; wi++ ) {

					num=0;
					wid = 0.5 * wi;
					s_a=0.0; s_a2=0.0; s_b=0.0; s_b2=0.0; s_ab=0.0;

					i0 = (int)floor(x - wid);
					i1 = (int)ceil(x + wid);
					j0 = (int)floor(y - wid);
					j1 = (int)ceil(y + wid);

					if ( i0<0 ) i0 = 0;
					if ( j0<0 ) j0 = 0;
					if ( i1>=cnt_image_x ) i1 = cnt_image_x-1;
					if ( j1>=cnt_image_y ) j1 = cnt_image_y-1;

					for ( i=i0; i<=i1; i++ ) {
						for ( j=j0; j<=j1; j++ ) {

							u = fabs( p*(i-x) + q*(j-y) );
							v = fabs( q*(i-x) - p*(j-y) );

							index2 = j*cnt_image_x + i;

							if ( u<=(wid/2.0) && v<=(wid/2.0) ) {

								inta = cnt_image_Blr[index1] * exp(-v*v/(2*wid*wid));
			                          		intb = cnt_image_Blr[index2];

								num ++;
								s_a += inta;
								s_b += intb;
								s_a2 += inta * inta;
								s_b2 += intb * intb;
								s_ab += inta * intb;
							}
						}
					}

					corr = sqrt( fabs(num*s_a2 - s_a*s_a) ) * sqrt( fabs(num*s_b2 - s_b*s_b) );
					if ( corr < 1.0E-9 ) corr = 1.0E-9;
					corr = ( num*s_ab - s_a*s_b ) / corr;

					if ( corr > max_corr ) {
						opt_wid = wid;
						max_corr = corr;
					}
				}
//                                if ( opt_wid >= (TUBEWID-1)/2 )// NEW
//                                        printf("Max width hit\n");

				if ( max_corr >= cnt_correlate ) {

					i0 = (int)floor(x - opt_wid);
					i1 = (int)ceil(x + opt_wid);
					j0 = (int)floor(y - opt_wid);
					j1 = (int)ceil(y + opt_wid);

					if ( i0<0 ) i0 = 0;
					if ( j0<0 ) j0 = 0;
					if ( i1>=cnt_image_x ) i1 = cnt_image_x-1;
					if ( j1>=cnt_image_y ) j1 = cnt_image_y-1;

					for ( i=i0; i<=i1; i++ ) {
                 		for ( j=j0; j<=j1; j++ ) {

							u = fabs( p*(i-x) + q*(j-y) );
	                  		v = fabs( q*(i-x) - p*(j-y) );

							index2 = j*cnt_image_x+i;
							
							if ( u<=opt_wid && v<=(opt_wid/1.3) ) {
								cnt_image_Msk[index2] = 255.0;
								if ( cnt_image_Blr[index2] < cnt_intensity )
									cnt_image_Msk[index2] = 0.0;
							}
						}
					}
				}
			}
		}
	}
}


// assign each tube a unique ID number
// set cnt_image_Tid[i] to the given tube's ID if it falls
//   within a tube, -1 otherwise
void CNT_IA::cnt_image_label(void)
{
	int runmore = 0;
	int x, y, i, j, i0, i1, j0, j1;
	long id0, id1, index, index1, index2;


	for ( y=0; y<cnt_image_y; y++ ) {
		for ( x=0; x<cnt_image_x; x++ ) {

			index = y * cnt_image_x + x;
			
			if ( cnt_image_Msk[index] > 1.0 )  // this pixel falls within a tube
				cnt_image_Tid[index] = index;  // assign pixel ID
			else
				cnt_image_Tid[index] = -1;
		}
	}

	do {
		runmore = 0;
		
		for ( y=0; y<cnt_image_y; y++ ) {
			for ( x=0; x<cnt_image_x; x++ ) {

				index1 = y * cnt_image_x + x;
				id0 = cnt_image_Tid[index1];

				if ( id0 > 0 ) {

					i0 = x - 1;
					i1 = x + 1;
					j0 = y - 1;
					j1 = y + 1;

					if ( i0 < 0 ) i0 = 0;
					if ( i1 >= cnt_image_x ) i1 = cnt_image_x-1;
					if ( j0 < 0 ) j0 = 0;
					if ( j1 >= cnt_image_y ) j1 = cnt_image_y-1;
 
					for ( i=i0; i<=i1; i++ ) {
						for ( j=j0; j<=j1; j++ ) {
							
							index2 = j * cnt_image_x + i;
							id1 = cnt_image_Tid[index2];
							
							if ( (id1 > 0) && (id0 > id1) ) {
								cnt_image_Tid[index1] = id1;
								runmore ++;
							}
						}
					}
				}
			}
		}

		for ( y=(cnt_image_y-1); y>=0; y-- ) {
		  	for ( x=(cnt_image_x-1); x>=0; x-- ) {

				index1 = y * cnt_image_x + x;
				id0 = cnt_image_Tid[index1];

				if ( id0 > 0 ) {

					i0 = x - 1;
					i1 = x + 1;
					j0 = y - 1;
					j1 = y + 1;

					if ( i0 < 0 ) i0 = 0;
					if ( i1 >= cnt_image_x ) i1 = cnt_image_x-1;
					if ( j0 < 0 ) j0 = 0;
					if ( j1 >= cnt_image_y ) j1 = cnt_image_y-1;

					for ( i=i0; i<=i1; i++ ) {
						for ( j=j0; j<=j1; j++ ) {

							index2 = j * cnt_image_x + i;
							id1 = cnt_image_Tid[index2];

							if ( (id1 > 0) && (id0 > id1) ) {
								cnt_image_Tid[index1] = id1;
								runmore ++;
							}
						}
					}
				}
			}
		}
	} while ( runmore );
}


// order the medial axis points along each tube
// for each pixel i where cnt_image_Med[i] is labeled as a medial
//   axis point, set cnt_image_Ord[i] to the ordered index of that
//   point along the current tube
void CNT_IA::cnt_image_order(void)
{
	// Find "most medial" point.
	//cnt_image_Med[index] = 50.0 - 2*Xpr + 6*(10-fabs(atan(Ypr/Xpr))*180.0/3.14) + 2*(Zpr-190);
	//if ( cnt_image_Med[index] < 220.0 ) cnt_image_Med[index] = 0.0;

	int n = 0;  // Tube number.
//	int p;		// Medial point number within tube n.
//	double *cnt_image_Tmp = new double [cnt_image_x * cnt_image_y];
//	for ( y=0; y<cnt_image_y; y++ ) {
//		for ( x=0; x<cnt_image_x; x++ ) {
//			id0 = y * cnt_image_x + x;
//			cnt_image_Tmp[id0] = 0.0;
//		}
//	}

	int x, y;
	long id0, id1;

	double max_intens = -10000.0;
	long max_id;
	for ( y=0; y<cnt_image_y; y++ ) {
		for ( x=0; x<cnt_image_x; x++ ) {
			id0 = y * cnt_image_x + x;
			cnt_image_Ord[id0] = 0.0;  // Init cnt_image_Ord to 0.
			if ( (cnt_image_Med[id0] > 1.0) && (cnt_image_Blr[id0] > max_intens) ) {
				max_intens = cnt_image_Blr[id0];
				max_id = id0;
			}
		}
	}

	double id_hi = 0.0;
	double id_lo = 0.0;
	int found, dir, skip, end;
	double theta, dx, dy;
	for ( y=0; y<cnt_image_y; y++ ) {
		for ( x=0; x<cnt_image_x; x++ ) {
			id0 = y * cnt_image_x + x;
			// start medial axis tracking at the tube ID pixel
			if ( id0 == max_id ) {
				id1 = id0;
				end = 0;
				skip = 0;
				do {
					id_hi = id_hi + 1.0;
					found = 0;
					theta = cnt_image_Vpp[id1] * 3.14159265359;
					theta = fabs(theta);
					dx = sin(theta);
					dy = cos(theta);
					// move in x-direction
					if ( fabs(dx) >= fabs(dy) ) {
						if ( dx > 0 ) dir = 1;
						else dir = -1;
						if ( cnt_image_Med[id1+dir] > 1.0 ) {
							found = 1;
							skip = 0;
							id1 += dir;
							cnt_image_Ord[id1] = id_hi;
						}
					}
					// move in y-direction
					if ( (fabs(dx) < fabs(dy)) || (!found) ) {
						if ( dy > 0 ) dir = cnt_image_x;
						else dir = -cnt_image_x;
						if ( cnt_image_Med[id1+dir] > 1.0 ) {
							found = 1;
							skip = 0;
							id1 += dir;
							cnt_image_Ord[id1] = id_hi;
						}
					}
					// move diagonally
					if ( !found ) {
						if ( dy > 0 ) dir = cnt_image_x;
						else dir = -cnt_image_x;
						if ( dx > 0 ) dir += 1;
						else dir -= 1;
						if ( cnt_image_Med[id1+dir] > 1.0 ) {
							found = 1;
							skip = 0;
							id1 += dir;
							cnt_image_Ord[id1] = id_hi;
						}
					}
					if ( !found ) {
						// move to next nearest diagonal
						if ( fabs(dx) >= fabs(dy) ) {
							if ( dy > 0 ) dir = -cnt_image_x;
							else dir = cnt_image_x;
							if ( dx > 0 ) dir += 1;
							else dir -= 1;
							if ( cnt_image_Med[id1+dir] > 1.0 ) {
								found = 1;
								skip = 0;
								id1 += dir;
								cnt_image_Ord[id1] = id_hi;
							}
						}
						if ( !found && !skip ) {
							if ( fabs(dx) >= fabs(dy) ) {
								if ( dx > 0 ) dir = 1;
								else dir = -1;
								skip = 1;
								id1 += dir;
							}
							else {
								if ( dy > 0 ) dir = cnt_image_x;
								else dir = -cnt_image_x;
								skip = 1;
								id1 += dir;
							}
						}
						if ( !found && skip ) {
							end = 1;
						}
					}
				} while ( !end );
				id1 = id0;
				end = 0;
				do {
					id_lo = id_lo - 1.0;
					found = 0;
					theta = cnt_image_Vpp[id1] * 3.14159265359;
					theta = fabs(theta);
					dx = -sin(theta);
					dy = -cos(theta);
					// move in x-direction
					if ( fabs(dx) >= fabs(dy) ) {
						if ( dx > 0 ) dir = 1;
						else dir = -1;
						if ( cnt_image_Med[id1+dir] > 1.0 ) {
							found = 1;
							skip = 0;
							id1 += dir;
							cnt_image_Ord[id1] = id_lo;
						}
					}
					// move in y-direction
					if ( (fabs(dx) < fabs(dy)) || (!found) ) {
						if ( dy > 0 ) dir = cnt_image_x;
						else dir = -cnt_image_x;
						if ( cnt_image_Med[id1+dir] > 1.0 ) {
							found = 1;
							skip = 0;
							id1 += dir;
							cnt_image_Ord[id1] = id_lo;
						}
					}
					// move diagonally
					if ( !found ) {
						if ( dy > 0 ) dir = cnt_image_x;
						else dir = -cnt_image_x;
						if ( dx > 0 ) dir += 1;
						else dir -= 1;
						if ( cnt_image_Med[id1+dir] > 1.0 ) {
							found = 1;
							skip = 0;
							id1 += dir;
							cnt_image_Ord[id1] = id_lo;
						}
					}
					if ( !found ) {
						// move to next nearest diagonal
						if ( fabs(dx) >= fabs(dy) ) {
							if ( dy > 0 ) dir = -cnt_image_x;
							else dir = cnt_image_x;
							if ( dx > 0 ) dir += 1;
							else dir -= 1;
							if ( cnt_image_Med[id1+dir] > 1.0 ) {
								found = 1;
								skip = 0;
								id1 += dir;
								cnt_image_Ord[id1] = id_lo;
							}
						}
						if ( !found && !skip ) {
							if ( fabs(dx) >= fabs(dy) ) {
								if ( dx > 0 ) dir = 1;
								else dir = -1;
								skip = 1;
								id1 += dir;
							}
							else {
								if ( dy > 0 ) dir = cnt_image_x;
								else dir = -cnt_image_x;
								skip = 1;
								id1 += dir;
							}
						}
						if ( !found && skip ) {
							end = 1;
						}
					}
				} while ( !end );

				n++;
			}
		}
	}

	// Find the minimum and maximum order values.
	double mn = 10000.0;
	double mx = -10000.0;
	for ( y=0; y<cnt_image_y; y++ ) {
		for ( x=0; x<cnt_image_x; x++ ) {
			id0 = y * cnt_image_x + x;
			if ( cnt_image_Ord[id0] < mn ) {
				mn = cnt_image_Ord[id0];
			}
			if ( cnt_image_Ord[id0] > mx ) {
				mx = cnt_image_Ord[id0];
			}
		}
	}
	int count = int (mx-mn) + 1;

	// Increment all the order values by -mn+1, so that the min will be 1.
	for ( y=0; y<cnt_image_y; y++ ) {
		for ( x=0; x<cnt_image_x; x++ ) {
			id0 = y * cnt_image_x + x;
			if ( (cnt_image_Ord[id0] != 0.0) || (id0 == max_id) ) {
				cnt_image_Ord[id0] = cnt_image_Ord[id0] - mn + 1.0;
			}
		}
	}

//	// Load auxiliary data.  // NEW (comment only)
//	int imgX, imgY;
//	unsigned char *img;
//	double *aux;
//	img = NULL;
//	LoadPPM("good1.ppm", img, imgX, imgY);
//	aux = new double[imgX * imgY];
//	for ( y=0; y<imgY; y++ ) {
//		for ( x=0; x<imgX; x++ ) {
//			aux[y*imgX+x] = (double)img[3*(y*imgX+x)];
//		}
//	}

        // Compute (x,y) coords for all ordered medial points.  // NEW (comment only)
	int i = 0;
	int *Xs = new int [count];
	int *Ys = new int [count];
	for ( y=cnt_image_y-1; y>=0; y-- ) {
		for ( x=0; x<cnt_image_x; x++ ) {
			id0 = y * cnt_image_x + x;
			if ( cnt_image_Ord[id0] > 0.0 ) {
				Xs[i] = x;
				Ys[i] = y;
				i++;
			}
		}
	}

        // Compute height of nanotube.  // NEW (whole block)
        double heightSum = 0.0;
        double heightMax, heightAve;
//      long idLeft, idRight;
        count = 0;
        for ( y=0; y<cnt_image_y; y++ ) {
                for ( x=0; x<cnt_image_x; x++ ) {
                        id0 = y * cnt_image_x + x;
                        if (cnt_image_Ord[id0] > 0.0) {
                                theta = cnt_image_Vpp[id0] * 3.14159265359;
                                theta = fabs(theta);
                                dx = sin(theta);
                                dy = cos(theta);
                                heightMax = 0;
                                for ( i=-TUBEWID/2; i<=TUBEWID/2; i++) {
                                        id1 = int(y+i*dx+0.5) * cnt_image_x + int(x-i*dy+0.5);
					if ( cnt_image_Hgt[id1] > heightMax )
						heightMax = cnt_image_Hgt[id1];
                                }
//                              idLeft = int(y+dx+0.5) * cnt_image_x + int(x-dy+0.5);
//                              idRight = int(y-dx+0.5) * cnt_image_x + int(x+dy+0.5);
//                              heightMax = 0;//{replace w/ height[id0], from nano info}
//                              if ( 1 )//{height[idLeft] > heightMax}
//                                      heightMax = 1;//{height[idLeft]}
//                              if ( 2 )//{height[idRight] > heightMax}
//                                      heightMax = 2;//{height[idRight]}
                                heightSum += heightMax;
                                count++;
                        }
                }
        }
        heightAve = heightSum / count;
                
        // Compute 3D info.  // NEW (whole block) 
        int prevId, nextId;
        int prevX, prevY, nextX, nextY;
        double prevZ, nextZ, dz, norm_alt, mag;
        double *az = new double [count];
        double *alt = new double [count];
        double *rad_of_curv = new double [count];
        double *X3D = new double [count];
        double *Y3D = new double [count];
        double *Z3D = new double [count];

        for ( i=0; i<count; i++ ) { 
                x = Xs[i];               
                y = Ys[i];
                id0 = y * cnt_image_x + x;

                // Find the azimuth and altitude of the axis direction at the current location.
                if ( i==0 ) {
                        prevX = Xs[i];
                        prevY = Ys[i];
                }
                else {
                        prevX = Xs[i-1];
                        prevY = Ys[i-1];
                }
                if ( i==count-1 ) {
                        nextX = Xs[i];
                        nextY = Ys[i];
                }
                else {
                        nextX = Xs[i+1];
                        nextY = Ys[i+1];
                }
                prevId = prevY * cnt_image_x + prevX;
                nextId = nextY * cnt_image_x + nextX;
                prevZ = cnt_image[prevId];
                nextZ = cnt_image[nextId];
                dx = double (nextX - prevX);
                dy = double (nextY - prevY);
                dz = nextZ - prevZ;
                theta = cnt_image_Vpp[id0] * 3.14159265359;
                theta = fabs(theta);
                az[i] = theta;
                alt[i] = atan2(dz, sqrt(dx*dx+dy*dy));

                // Find the radius of curvature of the tube at the current location.
                if ( fabs(cnt_image_Hpp[id0]) > fabs(cnt_image_Hqq[id0]) )
                        rad_of_curv[i] = fabs(cnt_image_Hpp[id0]);
                else
                        rad_of_curv[i] = fabs(cnt_image_Hqq[id0]);
                rad_of_curv[i] *= cos(alt[i]);           

                // Find the 3D medial axis location corresponding to the current image (tube top) point.
                if ( alt > 0 ) {    
                        norm_alt = alt[i] - 3.14159265359/2;
                        dx = sin(theta);
                        dy = cos(theta);  
                        dz = tan(norm_alt);
                }
                else if ( alt < 0 ) {
                        norm_alt = alt[i] + 3.14159265359/2;
                        dx = -sin(theta);
                        dy = -cos(theta);
                        dz = -tan(theta);
                }
                mag = rad_of_curv[i] / sqrt(1 + dz*dz);
                dx *= mag;
                dy *= mag;
                dz *= mag;
                X3D[i] = double (x) + dx;
                Y3D[i] = double (y) + dy;
                Z3D[i] = cnt_image[id0] + dz;
        }

	double orient, height;
	FILE *fp;
	fp = fopen("medial.dat", "w");
        fprintf(fp, "  s    height   azimuth altitude  radius     x        y     newX     newY     newZ\n\n");  // NEW
	for ( y=cnt_image_y-1; y>=0; y-- ) {
		for ( x=0; x<cnt_image_x; x++ ) {
			id0 = y * cnt_image_x + x;
			if ( cnt_image_Ord[id0] > 0.0 ) {
                                height = cnt_image[id0] * cnt_image_height / 255.0;
				id1 = int (cnt_image_Ord[id0]) - 1; // NEW
                                fprintf(fp, "%3.0lf %8.2lf %8.2lf %8.2lf %8.2lf %8d %8d %8.2lf %8.2lf %8.2lf\n", cnt_image_Ord[id0], height, az[id1], alt[id1], rad_of_curv[id1], x, y, X3D[id1], Y3D[id1], Z3D[id1]);  // NEW
			}
		}
	}
	fclose(fp);

	// NEW -- Clear cnt_image_Ord (undo everything done in cnt_image_order()).
//	for ( y=0; y<cnt_image_y; y++ ) {
//		for ( x=0; x<cnt_image_x; x++) {
//			id0 = x * cnt_image_x + y;
//			cnt_image_Ord[id0] = 0.0;
//		}
//	}
}


// select which "tubes" are actually tubes, and which are irrelevant blobs
// for each tube, write position, orientation, and size to file param.dat
void CNT_IA::cnt_image_select(void)
{
	int x, y, i, j;
	long id0, id1, index;
	FILE *fp;
	
        int count = 0; // NEW

	fp = fopen("param.dat", "w");

	for ( y=0; y<cnt_image_y; y++ ) {
		for ( x=0; x<cnt_image_x; x++ ) {

			id0 = y * cnt_image_x + x;
			
			if ( cnt_image_Tid[id0] == id0 ) {
				
				long num = 0;
				double e1 = 0.0, e2 = 0.0, e3 = 0.0, e4 = 0.0, xb = 0.0, yb = 0.0;
				double COMx, COMy, Ia, Ib, yaw, La, Lb;
			 
				for ( j=0; j<cnt_image_y; j++ ) {
					for ( i=0; i<cnt_image_x; i++ ) {
						
						index = j * cnt_image_x + i;
						id1 = cnt_image_Tid[index];
						
						if ( id0 == id1 ) {
							e1 += i * i;
							e2 += i * j;
							e3 += j * i;
							e4 += j * j;
							xb += i;
							yb += j;
							num ++;
						}
					}
				}

				e1 -= xb * xb / num;
				e2 -= xb * yb / num;
				e3 -= xb * yb / num;
				e4 -= yb * yb / num;

				Ia = 0.5 * fabs( (e1+e4) + sqrt( (e1-e4)*(e1-e4) + 4*e2*e3 ) ) + 0.1;
				Ib = 0.5 * fabs( (e1+e4) - sqrt( (e1-e4)*(e1-e4) + 4*e2*e3 ) ) + 0.1;
				La = exp( log( 144*Ia*Ia*Ia/Ib ) / 8.0 );
				Lb = exp( log( 144*Ib*Ib*Ib/Ia ) / 8.0 );

				yaw = atan((Ia-e1)/(e2+1.0E-6)) * 180.0 / 3.14;

				COMx = xb / num;
				COMy = yb / num;

				// Discard tube
				if ( (La/Lb < cnt_aspect) || (num < 16) ) {  // lower bound of pixels in blob

					for ( j=0; j<cnt_image_y; j++ ) {
						for ( i=0; i<cnt_image_x; i++ ) {

							index = j * cnt_image_x + i;

							if ( cnt_image_Tid[index] == id0 ) {
								cnt_image_Tid[index] = -1;
								cnt_image_Msk[index] = 0.0;
							}
						}
					}
				}

				// Valid tube
				else {

                                        // NEW -- Paint each tube a different color
                                        count++;
                                        for ( j=0; j<cnt_image_y; j++ ) {
                                                for ( i=0; i<cnt_image_x; i++ ) {
                                                        index = j * cnt_image_x + i;
                                                        if ( cnt_image_Tid[index] == id0 ) {
                                                                cnt_image_Ord[index] = 1.0;
                                                        }
                                                }
                                        }
  
					fprintf(fp, "===== CNT ID = %d =====\n", id0);
					fprintf(fp, "Position : X = %8.3lf Y = %8.3lf\n", COMx, COMy);
					fprintf(fp, "Orientation : %8.3lf degree\n", yaw);
					fprintf(fp, "Length = %8.3lf Width = %8.3lf\n\n", La, Lb);
				}
			}
		}
	}

	fclose(fp);
	
}
