#include <stdio.h>
#include <stdlib.h>
//#include <nmg_Graphics.h>


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
		  //this code tests the function UpdateDataArray
		  //fill in array
		  double * d_cntMask = new double[512];
		  for(int y = 0; y <= 512 - 1; y++) {
			  for( int x = 0; x <= 512 - 1; x++){
						if(y%2 == 0){
							d_cntMask[x] =  256.0;
						}
						else{
							d_cntMask[x] =  0.0;
						}
			  }
			  shapePlane.UpdateDataArray(d_cntMask, y, 512.0);
		  }
		 
		  //update shapePlane
		 		  
	}
	if (d_ordWrite) {
		d_cntRec->cnt_image_write(d_imgOrdFile, d_cntRec->cnt_image_Ord);
	}
}






//constructor for when data is sent across initially
nma_ShapeIdentifiedPlane::
nma_ShapeIdentifiedPlane(BCPlane * sourcePlane, nmb_Dataset * dataset, const char* outputPlaneName, double * dataArray)
  : nmb_CalculatedPlane(outputPlaneName, dataset), d_sourcePlane(sourcePlane), 
  d_dataset(dataset), d_outputPlaneName(outputPlaneName), d_dataArray(dataArray)
{
	d_rowlength = d_sourcePlane->GetGrid()->numX();
    d_columnheight = d_sourcePlane->GetGrid()->numY();
    d_array_size = create_ShapeIdentifiedPlane();
	firstblur = true;
}

//'default' constructor for when no data sent across initially
nma_ShapeIdentifiedPlane::
nma_ShapeIdentifiedPlane(BCPlane * sourcePlane, nmb_Dataset * dataset, const char* outputPlaneName)
  : nmb_CalculatedPlane(outputPlaneName, dataset), d_sourcePlane(sourcePlane), 
  d_dataset(dataset), d_outputPlaneName(outputPlaneName), d_dataArray(NULL)
{
  d_array_size = 0; 
  d_rowlength = d_sourcePlane->GetGrid()->numX();
  d_columnheight = d_sourcePlane->GetGrid()->numY();
  firstblur = true;
}

nma_ShapeIdentifiedPlane::
~nma_ShapeIdentifiedPlane(){
  
}


bool nma_ShapeIdentifiedPlane::
dependsOnPlane( const BCPlane* const plane )
{
  if( plane == NULL ) return false;
  if( plane == this->d_sourcePlane /* pointer comparison */ )
    return true;
  else
    return false;
}


bool nma_ShapeIdentifiedPlane::
dependsOnPlane( const char* planeName )
{
  if( planeName == NULL ) return false;
  if( strcmp( planeName, this->d_sourcePlane->name()->Characters() ) )
    return true;
  else
    return false;
}


//updates d_dataArray when new information is received and fills in d_outputPlane with new values
void nma_ShapeIdentifiedPlane::
UpdateDataArray(double * dataline, int y, int datain_rowlen){
	if(firstblur && (y != 0) ) return;
	if(!firstblur){
		y = stored_y;
	}

  if(d_dataArray == NULL){//make it not NULL by filling in with dummy values
        d_array_size = create_ShapeIdentifiedPlane();//need to initialize array_size if first time
  }

  int sourceplane_size = d_rowlength*d_columnheight;
  if(d_array_size == sourceplane_size){//already filled once, array_size starts out at zero
	  
	  if(y < d_columnheight){//okay to proceed
			//blur necessary?
			if(d_rowlength > datain_rowlen){
				planepts_per_datapt = d_rowlength/datain_rowlen;
				num_leftovers = d_rowlength%datain_rowlen;
				blur_data_up(&dataline,y,datain_rowlen);
			}
			else if (datain_rowlen > d_rowlength){
				planepts_per_datapt = datain_rowlen/d_rowlength;
				num_leftovers = datain_rowlen%d_rowlength;
				blur_plane_up(&dataline,y,datain_rowlen);
			}//blur current dataline up or down to fit required rowlength (filling in points in between if
			 //d_rowlength is larger, and blur between current row of data and previous row
			else{
				nonblur(&dataline,y,datain_rowlen);
			}
	  }

	  stored_y = (y+planepts_per_datapt)%d_columnheight;
  }
  else{//array_size != preexisting_size
    cerr << "Sizes do not match" << endl << "Cannot update with current scan" << endl;
    return;
  } 
  delete [] dataline;
  
}


//when d_rowlength and datain_rowlen are the same, no need to blur
void nma_ShapeIdentifiedPlane::
nonblur(double** dataline, int& y, int& datain_rowlen){
	int new_index = 0;
	float val;
	
	for(int i = 0; i < datain_rowlen; ++i){
		if(firstblur) firstblur = false;
		if(new_index < d_rowlength){
			d_dataArray[datain_rowlen*y+i] = (*dataline)[i];
			val = d_dataArray[datain_rowlen*y+i];
			calculatedPlane->setValue(i,y,val);
			d_dataset->range_of_change.AddPoint(i,y);
		}

	}
}

//used by UpdateDataArray; blurs current dataline up to fit required rowlength 
//(filling in points in between since d_rowlength is larger, and blur between current row 
//of data and previous row)
void nma_ShapeIdentifiedPlane::
blur_data_up(double** dataline, int& y, int& datain_rowlen){

	double * newdataline = new double[d_rowlength];
	double intermediate = 0.0;
	int new_index = 0;
	int inter_y;

	//create the row of new data, interpolating between x values
	for(int i = 0; i < datain_rowlen; ++i){
		if(new_index < d_rowlength){
			newdataline[new_index++] = (*dataline)[i];
		}

		if(i+1 < datain_rowlen){
			intermediate = ((*dataline)[i+1] - (*dataline)[i])*(1.0/(double)planepts_per_datapt);
			for(int k = 1; k <= planepts_per_datapt-1; k++){
				if(new_index < d_rowlength){
					newdataline[new_index++] = intermediate*k + (*dataline)[i];
				}
			}	
		}
		else{//take care of leftover slots
			int to_do = num_leftovers + 3;
			for(;to_do >= 0;to_do--){
				intermediate = ((*dataline)[i] - (*dataline)[i-1])*(1.0/(double)planepts_per_datapt);
				for(int k = 1; k <= planepts_per_datapt-1; k++){
					if(new_index < d_rowlength){
						newdataline[new_index++] = intermediate*k + (*dataline)[i];
					}
				}
			}
		}
    }//fill in newdataline with values from dataline and values interpolated inbetween (blurring)


	//clean up--change datain_rowlen to reflect current for row and change dataline to point to newdataline
	datain_rowlen = d_rowlength;//(returned by reference)
	delete [] (*dataline);
	(*dataline) = newdataline;//now points to the new array we just made...


	//now fill in the intervening rows by interpolating between previous row and this new row
	new_index = 0;
	float val;
	
	for(i = 0; i < datain_rowlen; ++i){
		if(firstblur) firstblur = false;
		if(new_index < d_rowlength){
			d_dataArray[datain_rowlen*y+i] = (*dataline)[i];
			val = d_dataArray[datain_rowlen*y+i];
			calculatedPlane->setValue(i,y,val);
			d_dataset->range_of_change.AddPoint(i,y);
		}
		if(y-planepts_per_datapt >=0){
			intermediate = (d_dataArray[datain_rowlen*y+i] - d_dataArray[datain_rowlen*(y-planepts_per_datapt)+i])
				*(1.0/(double)planepts_per_datapt);
			for(int k = 1; k <= planepts_per_datapt-1; k++){
				inter_y = (y-planepts_per_datapt) + k;
				if(new_index++ < d_rowlength){
					d_dataArray[datain_rowlen*inter_y+i] = intermediate*k 
						+ d_dataArray[datain_rowlen*(y-planepts_per_datapt)+i];
					val = d_dataArray[datain_rowlen*inter_y+i];
					calculatedPlane->setValue(i,inter_y,val);
					d_dataset->range_of_change.AddPoint(i,inter_y);
				}
			}	
		}

		new_index = 0;
    }

}


void nma_ShapeIdentifiedPlane::
blur_plane_up(double ** dataline, int& y, int& datain_rowlen){

	

}


int nma_ShapeIdentifiedPlane::
create_ShapeIdentifiedPlane()
{
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

  int size = d_columnheight*d_rowlength;//size of this array--used if first time filling

  if(d_dataArray == NULL){//to test, since cnt_image_select does not seem to be
								//working...
	d_dataArray = new double[size];
  // fill in the new plane.
	for(int y = 0; y <= d_columnheight - 1; y++) {

		for( int x = 0; x <= d_rowlength - 1; x++){

			if(y%2 == 0){
				d_dataArray[y*d_columnheight+x] = 0.0;
				calculatedPlane->setValue(x, y,0.0f); 
			}
			else{
				d_dataArray[y*d_columnheight+x] = 0.0;
				calculatedPlane->setValue(x, y,0.0f); 
			}
		}
	}
  }
  else{//normal operation
	  for(int y = 0; y <= d_columnheight - 1; y++) {
		for( int x = 0; x <= d_rowlength - 1; x++){
			calculatedPlane->setValue(x, y,(float)(d_dataArray[y*d_rowlength + x]));
			//the array d_dataArray fills from the bottom up when you "chunk" the array
			//into pieces of length rowlength.  
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

  //nmb_CalculatedPlane::addNewCalculatedPlane(this);

  return size;
 
}

