#include <stdio.h>
#include <stdlib.h>

#include <BCPlane.h>
#include <BCString.h>
#include <nmb_PlaneSelection.h>
#include "nma_ShapeAnalyze.h"
#include "cnt_ia.h"

nma_ShapeAnalyze::
nma_ShapeAnalyze()
	:	d_maskWrite(1), d_ordWrite(0)
{
	strcpy(d_imgMaskFile, "mask.ppm");
	strcpy(d_imgOrdFile, "order.ppm");
	strcpy(d_txtFile, "mask");

	d_cntRec = new CNT_IA();
}

void nma_ShapeAnalyze::
setScale(float xscale, float yscale, float zscale)
{
	// set PIXEL <-> nm conversion scale factor (x, y, z)
	d_cntRec->cnt_image_setScal(xscale, yscale, zscale);													
}

void nma_ShapeAnalyze::
setBlur(float sigma)
{
	// set SIGMA for image blurring -- default=1.5
	d_cntRec->cnt_image_setSigm(sigma);
}

void nma_ShapeAnalyze::
setCorrelation(float correlation)
{
	// set CORRELATION  factor for CNT fitting -- default=0.6
	d_cntRec->cnt_image_setCorr(correlation);	
}

void nma_ShapeAnalyze::
setAspectRatio(float aspect)
{
	// set ASPECT ratio for CNT recognition -- default=2.0
	d_cntRec->cnt_image_setAspt(aspect);			
}

void nma_ShapeAnalyze::
setThresholdInten(float thresholdInten)
{
	// set INTENSITY threshold for CNT recognition -- default=0.6
	d_cntRec->cnt_image_setIntn(thresholdInten);	
}

void nma_ShapeAnalyze::
setPreFlatten(int preFlatten)
{
	// set PRE-FLATTENING flag -- default=1
	d_cntRec->cnt_image_setFlat(preFlatten);		
}

void nma_ShapeAnalyze::
setAutoAdapt(int autoAdapt)
{
	// set AUTO-ADAPTION flag -- default=1
	d_cntRec->cnt_image_setAuto(autoAdapt);		
}

void nma_ShapeAnalyze::
setMaskWrite(int maskWrite)
{
	d_maskWrite = maskWrite;
}

void nma_ShapeAnalyze::
setOrderWrite(int ordWrite)
{
	d_ordWrite = ordWrite;
}

void nma_ShapeAnalyze::
setMaskFile(const char *file)
{
	int len = strlen(file);

	strcpy(d_imgMaskFile, file);
	strcpy(d_txtFile, file);
#ifdef _WIN32
	if (_stricmp(file + (len-4), ".ppm") != 0) {
#else
	if (strcasecmp(file + (len-4), ".ppm") != 0) {
#endif
		strcat(d_imgMaskFile, ".ppm");
	}

}

void nma_ShapeAnalyze::
setOrderFile(const char *file)
{
	int len = strlen(file);

	strcpy(d_imgOrdFile, file);
#ifdef _WIN32
	if (_stricmp(file + (len-4), ".ppm") != 0) {
#else
	if (strcasecmp(file + (len-4), ".ppm") != 0) {
#endif
		strcat(d_imgOrdFile, ".ppm");
	}
}


void nma_ShapeAnalyze::
imageAnalyze(nmb_PlaneSelection planeSelection) //*
{
	BCPlane *imagePlane;
	imagePlane = planeSelection.height;

	d_cntRec->cnt_image_read(imagePlane);			// read image from BCPlane //*
	
	d_cntRec->cnt_image_flat();
	d_cntRec->cnt_image_filter();
	d_cntRec->cnt_image_medial();
	d_cntRec->cnt_image_fit();
	d_cntRec->cnt_image_label();
//	d_cntRec->cnt_image_order(d_txtFile);
	d_cntRec->cnt_image_select(d_txtFile, imagePlane->name()->Characters());
	
	//		d_cntRec.cnt_image_write("blur.ppm", d_cntRec.cnt_image_Blr);
	//		d_cntRec.cnt_image_write("medial.ppm", d_cntRec.cnt_image_Med);
	if (d_maskWrite) {
		d_cntRec->cnt_image_write(d_imgMaskFile, d_cntRec->cnt_image_Msk);
	}
	if (d_ordWrite) {
		d_cntRec->cnt_image_write(d_imgOrdFile, d_cntRec->cnt_image_Ord);
	}
}

