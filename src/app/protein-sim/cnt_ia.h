#ifndef __CNT_IA_H
#define __CNT_IA_H 1


class CNT_cnt {				// parameters of a single CNT
public :

	int cnt_id;				// ID of current CNT
	double cnt_center_x;	// center of CNT
	double cnt_center_y;
	double cnt_center_z;
	double cnt_width;		// CNT width
	double cnt_length;		// CNT length
	double cnt_orient;		// CNT orientation ( in degree )

	char cnt_on_border;		// flag indicating CNT partially in image
	int *cnt_touch_id;		// pointer to an array, which holds ID of all CNTs in touch
};


class CNT_IA {				// image analysis to find CNT
private :

	int cnt_image_x;		// image size X
	int cnt_image_y;		// image size Y

	double cnt_scale_x;		// pixel <-> nano
	double cnt_scale_y;
	double cnt_scale_z;

	double cnt_sigma;		// parameter for image blurring
	double cnt_aspect;		// aspect ratio threshold
	double cnt_intensity;		// intensity threshold
	double cnt_correlate;		// correlation threshold

	CNT_cnt *cnt_tubes;		// pointer to CNTs ( array )

public : // following are Image Analysis functions

	double *cnt_image;		// image array
	double *cnt_image_Blr;	// smoothed image
	double *cnt_image_Rdg;	// ridge operation
	double *cnt_image_Med;	// medial axes image
	double *cnt_image_Msk;	// CNT mask
	long *cnt_image_Tid;	// CNT ID

	double *cnt_image_H1x;	// derivatives
	double *cnt_image_H1y;
	double *cnt_image_H2x;
	double *cnt_image_H2y;

	double *cnt_image_Hxx;	// Heissian elements
	double *cnt_image_Hxy;
	double *cnt_image_Hyx;
	double *cnt_image_Hyy;

	double *cnt_image_Hpp;	// Heissian eigen-values and eigen-vectors
	double *cnt_image_Hqq;
	double *cnt_image_Vpp;
	double *cnt_image_Vqq;

	CNT_IA();

	void findTubesInImage( void );						// added by WR
	int cnt_image_pattern(double x, double y, double z);    // added by WR

	void cnt_image_read(char *imgFile, double pitch_x, double pitch_y, double pitch_z, double sigma, double aspect, double intensity, double correlate);
									// read in image array
	void cnt_image_write(char *imgFile, double *imgSrc);
									// write out image array in PPM format
	void cnt_image_flat(void);		// flattenning image
	void cnt_image_filter(void);	// blurring image
	void cnt_image_medial(void);	// finding medial axes
	void cnt_image_fit(void);		// picking tube from medial axes info
	void cnt_image_label(void);		// labelling CNT id
	void cnt_image_select(void);	// applying thresholds to pick out CNTs
	~CNT_IA();



protected:	// filter related variables

	double a0,a1,b0,b1,c0,c1,w0,w1; // Parameter of exponential serie
	double K;						// Normalization factor

private:	// filter related variables and functions

	double n00,n11,n22,n33;			// Causal coefficients
	double d11,d22,d33,d44;			// Causal coefficients == Anticausal coeff.
	double m11,m22,m33,m44;			// AntiCausal coefficients (for symmetrical case)

	void calcIIRfilterX(double *inpImg, double *outImg);
	void calcIIRfilterY(double *inpImg, double *outImg);
	void filterIIR(double *outs, double *data, int ln);
	void calcIIRcoefficients(int symmetric);
	void setupF1(double dd1);
	void setupF2(double dd2);
	void setupF3(double dd3);

	void filterH1x(double *inpImg, double *outImg);
	void filterH2x(double *inpImg, double *outImg);
	void filterH3x(double *inpImg, double *outImg);
	void filterH1y(double *inpImg, double *outImg);
	void filterH2y(double *inpImg, double *outImg);
	void filterH3y(double *inpImg, double *outImg);
};



// GLOBAL VARIABLE DEFINITIONS
extern CNT_IA cntRec;	// structure for recognizing CNT (carbon nanotubes) in images



#endif
