#ifndef NM_ANALYZE
#define NM_ANALYZE

class nmb_PlaneSelection;
class CNT_IA;

/////////////////////////////////////////////////////////////////
// Class:       nma_ShapeAnalyze
// Description: Interface class to the shape analysis code to
//              allow nano and that code to evolve independently
/////////////////////////////////////////////////////////////////
class nma_ShapeAnalyze {
public:
	nma_ShapeAnalyze();

	void setScale(float, float, float);
	void setBlur(float);
	void setCorrelation(float);
	void setAspectRatio(float);
	void setThresholdInten(float);

	void setPreFlatten(int);
	void setAutoAdapt(int);
	void setMaskWrite(int);
	void setOrderWrite(int);

	void setMaskFile(const char *);
	void setOrderFile(const char *);

	void imageAnalyze(nmb_PlaneSelection planeSelection);

private:
	char d_imgMaskFile[255];  //255 characters should be plenty for any file name
	char d_imgOrdFile[255];
	char d_txtFile[255];

	int d_maskWrite;
	int d_ordWrite;

	CNT_IA *d_cntRec;
};

#endif
