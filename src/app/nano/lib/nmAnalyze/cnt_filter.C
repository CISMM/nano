#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "cnt_ia.h"


void CNT_IA::calcIIRfilterX(double *inpImg, double *outImg)
{
	int x, y;
	double *inps, *outs;

	inps = new double[cnt_image_x];
	outs = new double[cnt_image_x];

	for(y=0; y<cnt_image_y; y++) {

		for(x=0; x<cnt_image_x; x++)
			inps[x] = inpImg[y*cnt_image_x+x];

		filterIIR(outs, inps, cnt_image_x);

		for(x=0; x<cnt_image_x; x++ )
			outImg[y*cnt_image_x+x] = outs[x];
	}

	if( outs ) delete [] outs;
	if( inps ) delete [] inps;
}


void CNT_IA::calcIIRfilterY(double *inpImg, double *outImg)
{
	int x, y;
	double *inps, *outs;

	inps = new double[cnt_image_y];
	outs = new double[cnt_image_y];

	for(x=0; x<cnt_image_x; x++) {

		for(y=0; y<cnt_image_y; y++)
			inps[y] = inpImg[y*cnt_image_x+x];

		filterIIR(outs, inps, cnt_image_y);

		for(y=0; y<cnt_image_y; y++ )
			outImg[y*cnt_image_x+x] = outs[y];
	}

	if( outs ) delete [] outs;
	if( inps ) delete [] inps;
}


void CNT_IA::filterIIR(double *outs, double *data, int ln)
{

	int i;
	double *s1, *s2;

	if( !outs || !data ) return;

	s1 = new double[ln];
	s2 = new double[ln];

	s1[0] = (double)(n00*data[0] + n11*data[1] + n22*data[2] + n33*data[3]);
	s1[1] = (double)(n00*data[1] + n11*data[0] + n22*data[1] + n33*data[2] - d11*s1[0]);
	s1[2] = (double)(n00*data[2] + n11*data[1] + n22*data[0] + n33*data[1] - d11*s1[1] - d22*s1[0]);
	s1[3] = (double)(n00*data[3] + n11*data[2] + n22*data[1] + n33*data[0]);
	s1[3] += (double)(- d11*s1[2] - d22*s1[1] - d33*s1[0]);

	for (i=4; i<ln; i++) {
		s1[i]  = (double)(n00*data[i] + n11*data[i-1] + n22*data[i-2] + n33*data[i-3]);
		s1[i] -= (double)(d11*s1[i-1] + d22*s1[i-2] + d33*s1[i-3] + d44*s1[i-4]);
	}

	s2[ln-1] = (double)(m11*data[ln-2] + m22*data[ln-3] + m33*data[ln-4]);
	s2[ln-2] = (double)(m11*data[ln-1] + m22*data[ln-2] + m33*data[ln-3] - d11*s2[ln-1]);
	s2[ln-3] = (double)(m11*data[ln-2] + m22*data[ln-1] + m33*data[ln-2] - d11*s2[ln-2] - d22*s2[ln-1]);
	s2[ln-4] = (double)(m11*data[ln-3] + m22*data[ln-2] + m33*data[ln-1]);
	s2[ln-4] -= (double)(d11*s2[ln-3] + d22*s2[ln-2] + d33*s2[ln-1]);

	for(i=(ln-5); i>=0; i--) {
		s2[i]  = (double)(m11*data[i+1] + m22*data[i+2] + m33*data[i+3] + m44*data[i+4]);
		s2[i] -= (double)(d11*s2[i+1] + d22*s2[i+2] + d33*s2[i+3] + d44*s2[i+4]);
	}

	for(i=0; i<ln; i++ ) {
		outs[i] = (double)(K * (s1[i]+s2[i]));
	}

	if( s1 ) delete [] s1;
	if( s2 ) delete [] s2;
}


void CNT_IA::calcIIRcoefficients(int symmetric)
{
	n00  = a0 + c0;
	n11  = exp(-b1/cnt_sigma)*(c1*sin(w1/cnt_sigma)-(c0+2*a0)*cos(w1/cnt_sigma)); 
	n11 += exp(-b0/cnt_sigma)*(a1*sin(w0/cnt_sigma)-(a0+2*c0)*cos(w0/cnt_sigma)); 
	n22  = ((a0+c0)*cos(w1/cnt_sigma)*cos(w0/cnt_sigma));
	n22	-= (a1*cos(w1/cnt_sigma)*sin(w0/cnt_sigma)+c1*cos(w0/cnt_sigma)*sin(w1/cnt_sigma));
	n22	*= 2*exp(-(b0+b1)/cnt_sigma);
	n22	+= c0*exp(-2*b0/cnt_sigma) + a0*exp(-2*b1/cnt_sigma);
	n33  = exp(-(b1+2*b0)/cnt_sigma)*(c1*sin(w1/cnt_sigma)-c0*cos(w1/cnt_sigma));
	n33 += exp(-(b0+2*b1)/cnt_sigma)*(a1*sin(w0/cnt_sigma)-a0*cos(w0/cnt_sigma));

	d44  = exp(-2*(b0+b1)/cnt_sigma);
	d33  = -2*cos(w0/cnt_sigma)*exp(-(b0+2*b1)/cnt_sigma);
	d33 += -2*cos(w1/cnt_sigma)*exp(-(b1+2*b0)/cnt_sigma);
	d22  =  4*cos(w1/cnt_sigma)*cos(w0/cnt_sigma)*exp(-(b0+b1)/cnt_sigma);
	d22 +=  exp(-2*b1/cnt_sigma)+exp(-2*b0/cnt_sigma);
	d11  =  -2*exp(-b1/cnt_sigma)*cos(w1/cnt_sigma)-2*exp(-b0/cnt_sigma)*cos(w0/cnt_sigma);
	
	if( symmetric ) {
		m11 = n11-d11*n00;
		m22 = n22-d22*n00;
		m33 = n33-d33*n00;
		m44 = -d44*n00;
	}
	else {
		m11 = -(n11-d11*n00);
		m22 = -(n22-d22*n00);
		m33 = -(n33-d33*n00);
		m44 = d44*n00;
	}
}


void CNT_IA::setupF1(double dd)
{
	int symmetric;
	double sigmad;

	a0 = 1.680;
	a1 = 3.735;
	b0 = 1.783;
	b1 = 1.723;
	c0 =-0.6803;
	c1 =-0.2598;
	w0 = 0.6318;
	w1 = 1.9970;

	sigmad = cnt_sigma / dd;
	K = 1.0/(sigmad*sqrt(2.0*(4.0*atan(1.0))));

	symmetric = 1;
	calcIIRcoefficients(symmetric);
}


void CNT_IA::setupF2(double dd)
{
	int symmetric;
	double sigmad;

	a0 = -0.6472;
	a1 = -4.5310;
	b0 =  1.5270;
	b1 =  1.5160;
	c0 =  0.6494;
	c1 =  0.9557;
	w0 =  0.6719;
	w1 =  2.0720;

	sigmad = cnt_sigma / dd;
	K = 1.0/(sigmad*sqrt(2.0*(4.0*atan(1.0))));

	symmetric = 1;
	calcIIRcoefficients( !symmetric );
}


void CNT_IA::setupF3(double dd)
{
	int symmetric;
	double sigmad;

	a0 =-1.3310;
	a1 = 3.6610;
	b0 = 1.2400;
	b1 = 1.3140;
	c0 = 0.3225;
	c1 =-1.7380;
	w0 = 0.7480;
	w1 = 2.1660;

	sigmad = cnt_sigma / dd;
	K = 1.0/(sigmad*sqrt(2.0*(4.0*atan(1.0))));
	
	symmetric = 1;
	calcIIRcoefficients( symmetric );
}


void CNT_IA::filterH1x(double *inpImg, double *outImg)
{
	setupF1( 1.0 );
	calcIIRfilterX(inpImg, outImg);
}


void CNT_IA::filterH2x(double *inpImg, double *outImg)
{
	setupF2( 1.0 );
	calcIIRfilterX(inpImg, outImg);
}


void CNT_IA::filterH3x(double *inpImg, double *outImg)
{
	setupF3( 1.0 );
	calcIIRfilterX(inpImg, outImg);
}


void CNT_IA::filterH1y(double *inpImg, double *outImg)
{
	setupF1( 1.0 );
	calcIIRfilterY(inpImg, outImg);
}


void CNT_IA::filterH2y(double *inpImg, double *outImg)
{
	setupF2( 1.0 );
	calcIIRfilterY(inpImg, outImg);
}


void CNT_IA::filterH3y(double *inpImg, double *outImg)
{
	setupF3( 1.0 );
	calcIIRfilterY(inpImg, outImg);
}
