#include <stdio.h>
#include <stdlib.h>


#include <BCPlane.h>
#include <BCString.h>
#include <nmb_PlaneSelection.h>
#include "nma_ShapeAnalyze.h"
#include "cnt_ia.h"
#include <nmb_Dataset.h>

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
	strcpy(d_desiredFilename, file);

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
imageAnalyze(nmb_PlaneSelection planeSelection, nmb_Dataset * dataset) //*
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

	BCGrid * currentGrid = dataset->inputGrid;//for right now just keep as dataset->inputGrid, but
	//can change it later to get planes in other related BCGrids as well

	d_cntRec->cnt_image_select(d_txtFile, imagePlane->name()->Characters(), currentGrid, imagePlane);
	
	//		d_cntRec.cnt_image_write("blur.ppm", d_cntRec.cnt_image_Blr);
	//		d_cntRec.cnt_image_write("medial.ppm", d_cntRec.cnt_image_Med);
	if (d_maskWrite) {
	  d_cntRec->cnt_image_write(d_imgMaskFile/*image file (name)*/, d_cntRec->cnt_image_Msk/*data array*/);

          nma_ShapeIdentifiedPlane shapePlane(imagePlane, dataset, d_desiredFilename, d_cntRec->cnt_image_Msk);
	}
	if (d_ordWrite) {
		d_cntRec->cnt_image_write(d_imgOrdFile, d_cntRec->cnt_image_Ord);
	}
}







nma_ShapeIdentifiedPlane::
	  nma_ShapeIdentifiedPlane(BCPlane * sourcePlane, nmb_Dataset * dataset, char* outputPlaneName, double * cntMask)
	    : d_sourcePlane(sourcePlane), d_dataset(dataset), d_outputPlaneName(outputPlaneName),d_cntMask(cntMask)
{
    create_ShapeIdentifiedPlane();

}


void nma_ShapeIdentifiedPlane::
create_ShapeIdentifiedPlane()
{

  //local variables for code simplification
  BCGrid* sourcePlaneParentGrid = d_sourcePlane->GetGrid();

  //check if other plane with name we want to give it is already in the grid
  //see if we can name the new plane with "d_imgMaskFile" as a name, will be NULL if we can
  d_outputPlane = sourcePlaneParentGrid->getPlaneByName(d_outputPlaneName);  
  if( d_outputPlane != NULL )
    {
      // a plane already exists by this name, and we disallow that.
      char s[] = "Cannot create new height plane.  A plane already exists of the name:  ";
      char msg[1024];
      sprintf( msg, "%s%s.", s, d_outputPlaneName );
    }  

  //adding new plane  
  char newunits[1000];
  sprintf(newunits, "boolean");
  d_outputPlane 
    = sourcePlaneParentGrid->addNewPlane(d_outputPlaneName, newunits, NOT_TIMED);
  if( d_outputPlane == NULL ) 
    {
      char s[] = "Could not create new height plane.  Can't make plane:  ";
      char msg[1024];
      sprintf( msg, "%s%s.", s, d_outputPlaneName );
    }

  
  //add image to image list so on menu
  TopoFile tf;
  
  nmb_Image *im = d_dataset->dataImages->getImageByPlane(d_sourcePlane);
  nmb_Image *output_im = new nmb_ImageGrid(d_outputPlane);
  if( im != NULL ) 
    {
      im->getTopoFileInfo(tf);
      output_im->setTopoFileInfo(tf);
    } 
  else 
    {
      fprintf(stderr, "Height Plane: Warning, "
  	      "input image not in list\n");
    }
  
  d_dataset->dataImages->addImage(output_im);

  short rowlength = sourcePlaneParentGrid->numX();
  short columnheight = sourcePlaneParentGrid->numY();

  // fill in the new plane.
  for(int y = 0; y <= columnheight - 1; y++) 
    {
      for( int x = 0; x <= rowlength - 1; x++) 
  	{
	  d_outputPlane->setValue(x, (columnheight - y),(float)(d_cntMask[y*rowlength + x]));  
	  //the array d_cntMask fills from the top down when you "chunk" the array
	  //into pieces of length rowlength.  However, traditional y values used
	  //in setValue treat lower valued y's as at the bottom of the image, and
	  //larger values at the top.  the above line accounts for this.
	}
    }

  // add ourselves to the dataset
  d_dataset->addNewCalculatedPlane( this );

  //add to list of names so show up in the pull-down menu of avail. planes
  d_dataset->inputPlaneNames->addEntry(d_outputPlaneName);
  
  // register ourselves to receive plane updates
  d_sourcePlane->add_callback(sourcePlaneChangeCallback, this);

  // let interested parties know a new plane has been created.
  nmb_CalculatedPlane::addNewCalculatedPlane(this);
 
}

