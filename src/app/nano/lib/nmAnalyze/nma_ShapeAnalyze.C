#include <stdio.h>
#include <stdlib.h>


#include <BCPlane.h>
#include <BCString.h>
#include <fstream.h>
#include <nmb_PlaneSelection.h>
#include "nma_ShapeAnalyze.h"
#include "cnt_ia.h"
#include <nmb_Dataset.h>

int nma_ShapeAnalyze::nma_ShapeAnalyzeCounter = 0;

nma_ShapeAnalyze::
nma_ShapeAnalyze()
  :	d_maskWrite(1), d_ordWrite(0)
{
        nma_ShapeAnalyzeCounter = 0;
  
	strcpy(d_imgMaskFile, "mask.ppm");
	strcpy(d_imgOrdFile, "order.ppm");
	strcpy(d_txtFile, "mask");

	d_cntRec = new CNT_IA();

	
}

nma_ShapeAnalyze::
~nma_ShapeAnalyze(){
        delete d_cntRec;
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

	BCGrid * currentGrid = dataset->inputGrid;//for right now just keep as 
        //dataset->inputGrid, but can change it later to get planes in other 
        //related BCGrids as well

	//d_cntRec->cnt_image_select(d_txtFile, imagePlane->name()->Characters());
	//above line commented out because causes system to crash
	d_cntRec->cnt_image_Msk = NULL;
	//set to NULL for testing without calling cnt_image_select
	
	//		d_cntRec.cnt_image_write("blur.ppm", d_cntRec.cnt_image_Blr);
	//		d_cntRec.cnt_image_write("medial.ppm", d_cntRec.cnt_image_Med);
	if (d_maskWrite) {
	  //d_cntRec->cnt_image_write(d_imgMaskFile/*image file (name)*/, d_cntRec->cnt_image_Msk/*data array*/);

          nma_ShapeIdentifiedPlane shapePlane(imagePlane, dataset, 
			  d_desiredFilename, d_cntRec->cnt_image_Msk);
		  /*this code tests the function UpdateDataArray
		  //fill in array
		  double * d_cntMask = new double[512.0*512.0];
		  for(int y = 0; y <= 512 - 1; y++) {
			  for( int x = 0; x <= 512 - 1; x++){
						if(y%2 == 0){
							d_cntMask[y*512 + x] =  256.0;
						}
						else{
							d_cntMask[y*512 + x] =  0.0;
						}
			  }
		  }
		  //update shapePlane
		  shapePlane.UpdateDataArray(d_cntMask, 512.0*512.0);
		  */
	}
	if (d_ordWrite) {
		d_cntRec->cnt_image_write(d_imgOrdFile, d_cntRec->cnt_image_Ord);
	}
}






//constructor for when data is sent across initially
nma_ShapeIdentifiedPlane::
nma_ShapeIdentifiedPlane(BCPlane * sourcePlane, nmb_Dataset * dataset, const char* outputPlaneName, double * cntMask)
  : nmb_CalculatedPlane(outputPlaneName, dataset), d_sourcePlane(sourcePlane), 
  d_dataset(dataset), d_outputPlaneName(outputPlaneName), d_cntMask(cntMask)
{
    array_size = create_ShapeIdentifiedPlane();
}

//'default' constructor for when no data sent across initially
nma_ShapeIdentifiedPlane::
nma_ShapeIdentifiedPlane(BCPlane * sourcePlane, nmb_Dataset * dataset, const char* outputPlaneName)
  : nmb_CalculatedPlane(outputPlaneName, dataset), d_sourcePlane(sourcePlane), 
  d_dataset(dataset), d_outputPlaneName(outputPlaneName), d_cntMask(NULL)
{
  //cout << "In nma_ShapeIdentifiedPlane constructor" << endl;
  array_size = 0;  
}

nma_ShapeIdentifiedPlane::
~nma_ShapeIdentifiedPlane(){
  /*delete d_sourcePlane;
  delete d_outputPlane;
  delete d_dataset;*/
}

//updates d_cntMask when new information is received and fills in d_outputPlane with new values
void nma_ShapeIdentifiedPlane::
UpdateDataArray(double * cntMask, int size){
  //cout << "In nma_ShapeIdentifiedPlane::UpdateDataArray" << endl;

  if(d_cntMask != NULL && array_size == size){//already filled once
    //give d_cntMask new values
    for(int i = 0; i < size; ++i){
      d_cntMask[i] = cntMask[i];
    }//don't know if above loop is necessary--already pointer to array?

    short rowlength = d_sourcePlane->GetGrid()->numX();
    short columnheight = d_sourcePlane->GetGrid()->numY();

    //fill in d_outputPlane with the new values
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

  }
  else if(d_cntMask == NULL){//first time filling it up
    d_cntMask = cntMask;
	cout << "Calling nma_ShapeIdentifiedPlane::create_ShapeIdentifiedPlane()" << endl;
    array_size = create_ShapeIdentifiedPlane();//need to initialize array_size if first time
  }
  else{//d_cntMask != NULL && array_size != size
    cerr << "Sizes do not match" << endl << "Cannot update with current scan" << endl;
    return;
  }  
}

int nma_ShapeIdentifiedPlane::
create_ShapeIdentifiedPlane()
{
  //cout << "In nma_ShapeIdentifiedPlane::create_ShapeIdentifiedPlane()" << endl;

  char uniqueOutputPlaneName[50];
  nma_ShapeAnalyze::nma_ShapeAnalyzeCounter++;
  sprintf(uniqueOutputPlaneName, "file%d_%s", nma_ShapeAnalyze::nma_ShapeAnalyzeCounter, d_outputPlaneName);
  //allows files to be named different things so you can make more than one shape-identified file
  //per run of nano
  

  //check if other plane with name we want to give it is already in the grid
  //see if we can name the new plane with "d_imgMaskFile" as a name, will be NULL if we can
  calculatedPlane = d_sourcePlane->GetGrid()->getPlaneByName(uniqueOutputPlaneName); 
  
  if( calculatedPlane != NULL )
    {
      // a plane already exists by this name, and we disallow that.
      char s[] = "Cannot create new height plane.  A plane already exists of the name:  ";
      char msg[1024];
      sprintf( msg, "%s%s.", s, uniqueOutputPlaneName );
    }  

  //adding new plane  
  char newunits[1000];
  sprintf(newunits, "");//fill in with correct units ANDREA
  calculatedPlane  
    = d_sourcePlane->GetGrid()->addNewPlane(uniqueOutputPlaneName, newunits, NOT_TIMED);
  if( calculatedPlane == NULL ) 
    {
      char s[] = "Could not create new height plane.  Can't make plane:  ";
      char msg[1024];
      sprintf( msg, "%s%s.", s, uniqueOutputPlaneName );
    }

  
  //add image to image list so on menu
  TopoFile tf;
  
  nmb_Image *im = d_dataset->dataImages->getImageByPlane(d_sourcePlane);
  nmb_Image *output_im = new nmb_ImageGrid(calculatedPlane);
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

  short rowlength = d_sourcePlane->GetGrid()->numX();
  short columnheight = d_sourcePlane->GetGrid()->numY();

  int size = columnheight*rowlength;//size of this array--used if first time filling

  // fill in the new plane.
  for(int y = 0; y <= columnheight - 1; y++) {

      for( int x = 0; x <= rowlength - 1; x++){
		  if(d_cntMask == NULL){//to test, since cnt_image_select does not seem to be
								//working...
			  if(y%2 == 0){
				  calculatedPlane->setValue(x, (columnheight - y),
						(float)(256.0)); 
			  }
			  else{
				  calculatedPlane->setValue(x, (columnheight - y),
						(float)(0.0)); 
			  }
		  }
		  else{//normal operation
			  calculatedPlane->setValue(x, (columnheight - y),
					(float)(d_cntMask[y*rowlength + x]));
			  //the array d_cntMask fills from the top down when you "chunk" the array
			  //into pieces of length rowlength.  However, traditional y values used
			  //in setValue treat lower valued y's as at the bottom of the image, and
			  //larger values at the top.  the above line accounts for this.
		  }
	  }
   }

  // add ourselves to the dataset
  d_dataset->addNewCalculatedPlane( this );

  //add to list of names so show up in the pull-down menu of avail. planes
  d_dataset->inputPlaneNames->addEntry(uniqueOutputPlaneName);
  
  // register ourselves to receive plane updates
  d_sourcePlane->add_callback(sourcePlaneChangeCallback, this);

  // let interested parties know a new plane has been created.
  nmb_CalculatedPlane::addNewCalculatedPlane(this);

  return size;
 
}

