#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "cnt_ia.h"
#include "cnt_ps.h"
#include "ppm.h"


/**************************************************************************************/
// GLOBAL VARIABLES
CNT_IA cntRec;		// structure for recognizing CNT (carbon nanotubes) in images

//int foundTubeCount;		// number of tubes found in this image
//#define MAX_TUBE   100
//CNT_cnt foundTube[MAX_TUBE];	// array of structs holding tube parameters


/**************************************************************************************/
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

	cnt_tubes = NULL;		// pointer to CNTs ( array )
}


CNT_IA::~CNT_IA()
{
}


void CNT_IA::cnt_image_read(char *imgFile, double pitch_x, double pitch_y, double pitch_z, double sigma, double aspect, double intensity, double correlate)
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

// initialize parameters

	cnt_image_x = imgX;
	cnt_image_y = imgY;

	cnt_scale_x = pitch_x;
	cnt_scale_y = pitch_y;
	cnt_scale_z = pitch_z;

	cnt_sigma = sigma;
	cnt_aspect = aspect;
	cnt_intensity = intensity;
	cnt_correlate = correlate;
}


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


void CNT_IA::cnt_image_flat(void)
{
	int x, y;
	long N;
	double Sx, Sy, Sz, Sxx, Sxy, Syy, Sxz, Syz;
	double Lxx, Lxy, Lyy, Lxz, Lyz;
	double a, b, c, nm, nx, ny, nz;
	double pixel, maxI, minI, denom, *imgTmp;

	N = cnt_image_x * cnt_image_y;
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
	      cnt_image[y*cnt_image_x+x] = 255.0 * sqrt((imgTmp[y*cnt_image_x+x]-minI) / denom);
	   }
	}
}


void CNT_IA::cnt_image_filter(void)
{
	long N = cnt_image_x * cnt_image_y;

// allocate image arrays

	cnt_image_Blr = new double [N];
	cnt_image_Rdg = new double [N];
	cnt_image_Med = new double [N];
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

	cnt_image_Tid = new long[N];

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


void CNT_IA::cnt_image_medial(void)
{
	int x, y;
	long index;
	double hxx, hxy, hyx, hyy, x1, y1, x2, y2;
	double dxy, S, vectx, vecty, norm;
	double Xpr, Ypr, Zpr;

// solve for Heissian eigen-values and eigen-vectors

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

	
int CNT_IA::cnt_image_pattern(double x, double y, double z)
{
	if ( (x < 0.0) && (fabs(y) < 10.0) && (z >= cnt_intensity) )
		return(1);
	else
		return(0);
}


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

	for ( y=0; y<cnt_image_y; y++ ) {		// reset mask
		for ( x=0; x<cnt_image_x; x++ ) {
			cnt_image_Msk[y*cnt_image_x+x] = 0.0;
		}
	}

	for ( y=0; y<cnt_image_y; y++ ) {
		for ( x=0; x<cnt_image_x; x++ ) {

			index1 = y * cnt_image_x + x;
			
			if ( cnt_image_Med[index1] > 1.0 ) {

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


void CNT_IA::cnt_image_label(void)
{
	int runmore = 0;
	int x, y, i, j, i0, i1, j0, j1;
	long id0, id1, index, index1, index2;


	for ( y=0; y<cnt_image_y; y++ ) {
		for ( x=0; x<cnt_image_x; x++ ) {

			index = y * cnt_image_x + x;
			
			if ( cnt_image_Msk[index] > 1.0 )
				cnt_image_Tid[index] = index;// assign pixel ID
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


void CNT_IA::cnt_image_select(void)
{
	int x, y, i, j;
	long id0, id1, index;
	FILE *fp;
	
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
				
				else {
					// A tube has been recognized.
					// Pass on the parameters describing the tube.
					fprintf(fp, "===== CNT ID = %d =====\n", id0);
					fprintf(fp, "Position : X = %8.3lf Y = %8.3lf\n", COMx, COMy);
					fprintf(fp, "Orientation : %8.3lf degree\n", yaw);
					fprintf(fp, "Length = %8.3lf Width = %8.3lf\n\n", La, Lb);



#if 0
					// Increment the tube counter and store tube params to array.
					int i = foundTubeCount;
					foundTubeCount++;	
					
					foundTube[i].cnt_id       = i;

					foundTube[i].cnt_center_x = COMx;
					foundTube[i].cnt_center_y = COMy;
					foundTube[i].cnt_center_z = 0.;

					foundTube[i].cnt_orient   = yaw * (3.141592 / 180.); // radians

					foundTube[i].cnt_length   = La;
					foundTube[i].cnt_width    = Lb;

					foundTube[i].cnt_on_border = 0;
					foundTube[i].cnt_touch_id  = NULL;
#endif


				}
			}
		}
	}

//	printf( "Tubes found: %d\n", foundTubeCount );		// added by WR

    fclose(fp);
}
