#include <stdio.h>
#include <stdlib.h>
//#include <nmg_Graphics.h>


#include <BCPlane.h>
#include <BCString.h>
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
		delete shapePlane;
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
	d_cntRec->cnt_image_order(d_txtFile);

	BCGrid * currentGrid = dataset->inputGrid;//for right now just keep as 
        //dataset->inputGrid, but can change it later to get planes in other 
        //related BCGrids as well

	d_cntRec->cnt_image_select(d_txtFile, imagePlane->name()->Characters());
		
	//d_cntRec->cnt_image_Msk = NULL;
	//set to NULL for testing without calling cnt_image_select
	
	//		d_cntRec.cnt_image_write("blur.ppm", d_cntRec.cnt_image_Blr);
	//		d_cntRec.cnt_image_write("medial.ppm", d_cntRec.cnt_image_Med);
	if (d_maskWrite) {
	  d_cntRec->cnt_image_write(d_imgMaskFile/*image file (name)*/, d_cntRec->cnt_image_Msk/*data array*/);
	  
	  //reverse the array so when flipped in nma_ShapeIdentifiedPlane, turns out upright
	  int x_dim = d_cntRec->cnt_image_x;
	  int y_dim = d_cntRec->cnt_image_y;
	  reverse_array = new double[x_dim*y_dim];	  
	  for(int j = 0;j < y_dim;++j){
		  for(int i = 0;i < x_dim;++i){
			  reverse_array[i+(y_dim-j-1)*x_dim] = d_cntRec->cnt_image_Msk[i+j*x_dim];
		  }
	  }
	
	  shapePlane = new nma_ShapeIdentifiedPlane(imagePlane, dataset, d_desiredFilename, reverse_array);
		 		  
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
  firstblur = true;
  storeddataline = new double[d_rowlength];

  l_sim_x = 0;
  h_sim_x = 1;
  placeholder_x = 0;
  //because starting at beginning of row

  l_sim_y = -1;
  h_sim_y = 0;
  placeholder_y = 0;
  //init. l_sim_y and h_sim_y to -1 and 0 because will be incremented at beginning of interpolation code
  //and want to have 0 and 1 for first row processed (y=0 from simulator)

  nano_y = 0;

  d_array_size = create_ShapeIdentifiedPlane();
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
  storeddataline = new double[d_rowlength];
  
  l_sim_x = 0;
  h_sim_x = 1;
  placeholder_x = 0;
  //because starting at beginning of row

  l_sim_y = -1;
  h_sim_y = 0;
  placeholder_y = 0;
  //init. l_sim_y and h_sim_y to -1 and 0 because will be incremented at beginning of interpolation code
  //and want to have 0 and 1 for first row processed (y=0 from simulator)

  nano_y = 0;
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
	  l_sim_x = 0;
	  h_sim_x = 1;
	  placeholder_x = 0;
  }//always reset these back to values they should be for the start of a row of data

  if(d_dataArray == NULL){//make it not NULL by filling in with dummy values
        d_array_size = create_ShapeIdentifiedPlane();//need to initialize array_size if first time
  }

  int sourceplane_size = d_rowlength*d_columnheight;
  if(d_array_size == sourceplane_size){//already filled once, array_size starts out at zero
	  if(d_rowlength >= datain_rowlen){
		  stepsize = ((double)((double)datain_rowlen - 1))/((double)((double)d_rowlength - 1));
		  blur_data_up(&dataline,y,datain_rowlen);
	  }
	  else if (datain_rowlen > d_rowlength){
		  blur_plane_up(&dataline,y,datain_rowlen);
	  }//blur current dataline up or down to fit required rowlength (filling in points in between if
	   //d_rowlength is larger, and blur between current row of data and previous row
  }
  else{//array_size != preexisting_size
    cerr << "Sizes do not match" << endl << "Cannot update with current scan" << endl;
    return;
  } 
  delete [] dataline;
  
}


//used by UpdateDataArray; blurs current dataline up to fit required rowlength 
//(filling in points in between since d_rowlength is larger, and blur between current row 
//of data and previous row)
void nma_ShapeIdentifiedPlane::
blur_data_up(double** dataline, int& y, int& datain_rowlen){

	double * newdataline = new double[d_rowlength];
	//holds "new" line of data consisting of old data with interpolated values inbetween
	//now we have an incoming data line of the appropriate length
	double placeholder_val;//value at (placeholder_x,y) in incoming line of data (from simulator)
	double percent_between;

	newdataline[0] = (*dataline)[0];
	placeholder_x += stepsize;
	int nano_x = 0;//tracks where we are in nano's x-space, range of [0,d_rowlength]
	do{
		if((placeholder_x >= l_sim_x) && (placeholder_x <= h_sim_x)){
			percent_between = placeholder_x - l_sim_x;
			placeholder_val = percent_between * ((*dataline)[h_sim_x] - (*dataline)[l_sim_x]) + (*dataline)[l_sim_x];
			//linear interpolation scheme based on percentage between l_sim_x and h_sim_x that placeholder_x falls
			newdataline[nano_x] = placeholder_val;
			
			++nano_x;//increment to indicate another value in the nano-length row filled in
			placeholder_x += stepsize;

		}
		else if(placeholder_x > h_sim_x){
			l_sim_x++;
			h_sim_x++;
		}//placeholder_x has passed out of the current range [l_sim_x,h_sim_x], and thus
		 //to find the value at placeholder_x, we need to interpolate between [h_sim_x,h_sim_x + 1], so increment
		else{
			return;
		}//should never do this else, but just in case...		
	}while(nano_x <= (d_rowlength - 1));

	
	//clean up--change datain_rowlen to reflect current for row and set datain_numrows to equal datain_rowlen
	//since grid is square from simulator
	int datain_numrows = datain_rowlen;
	datain_rowlen = d_rowlength;//(returned by reference)


	//fill in rows (step through y)
	l_sim_y = (++l_sim_y)%datain_numrows;//always want l_sim_y to be within the range [0,datain_numrows-1]
	h_sim_y = l_sim_y +1;
	//can increment these two at beginning of function because initialized to -1 and 0

	int y_flipped;//flip y for display:  array is filled in such that the top of the grid corresponds to 
	//lower numbered rows and the bottom of the grid is higher numbered rows--however, the plane fills 
	//in with lowered numbered rows corresponding to the bottom of the grid.  so, we store top down
	//but must "flip" y to display the plane
	int index;//current index we are on now

	//nano_y starts at 0
	while(nano_y <= (d_columnheight-1)){
		//handle first row
		if(placeholder_y == 0){//just insert row--no interpolation between this row and previous row
			double val;

			int y = d_columnheight - placeholder_y - 1;
			for(int i = 0; i < d_rowlength; ++i){
				d_dataArray[i] = newdataline[i];
				val = d_dataArray[i];
				calculatedPlane->setValue(i,y,val);
				d_dataset->range_of_change.AddPoint(i,y);
			}
			for(int i = 0; i < d_rowlength; i++){
				storeddataline[i] = newdataline[i];
			}//now this is our old row			
			--l_sim_y;
			--h_sim_y;//so that can cover placeholder_y values between 0 and 1 next time around
			placeholder_y = (placeholder_y + stepsize);
			++nano_y;
			if(firstblur) firstblur = false;//we have entered a row for the first time (necessary that y = 0)
			return;//wait until we get another row of data in to do anything else
		}
		//handle all rows between (and including) 1 and d_columnheight - 1 (in nano y-space)
		else if((placeholder_y >= l_sim_y) && (placeholder_y <= h_sim_y)){//interpolate between l_sim_y and h_sim_y
			percent_between = placeholder_y - l_sim_y;//= incremental distance past l_sim_y
			for(int i = 0;i < d_rowlength; ++i){
				placeholder_val = percent_between*(newdataline[i] - storeddataline[i]) + storeddataline[i];	
				//linear interpolation scheme based on percentage between l_sim_y and h_sim_y that placeholder_y falls
				index = nano_y*datain_rowlen + i;
				d_dataArray[index] = placeholder_val;
				y_flipped = d_columnheight - nano_y - 1;
				calculatedPlane->setValue(i,y_flipped,placeholder_val);
				d_dataset->range_of_change.AddPoint(i,y_flipped);				
			}
			placeholder_y = (placeholder_y + stepsize);
			++nano_y;
			//increment placeholder_y and nano_y for next insertion (whether on this function call or next)
			if(nano_y == d_columnheight){
				placeholder_y = 0;
				nano_y = 0;
				l_sim_y = -1;
				h_sim_y = 0;
			}//zero placeholder_y and nano_y and re-initialize l_sim_y and h_sim_y so that, on next run-through,
			 //they are incremented to 0 and 1 (which occurs at the beginning of the function)
		}
		//prepare to handle next line of data if have handled all the points we can with current line
		else if(placeholder_y > h_sim_y){//we need to wait for a new line of data to be fed in
			for(int i = 0; i < d_rowlength; i++){
				storeddataline[i] = newdataline[i];
			}//store the current data line so can interpolate between it and new data line next time around

			//keep placeholder_y and thus nano_y as they are
			return;//wait for next dataline
		}
		else if(placeholder_y < l_sim_y){
			return;
		}//shouldn't ever run through this else if statement but, just in case...
	}

}


void nma_ShapeIdentifiedPlane::
blur_plane_up(double ** dataline, int& y, int& datain_rowlen){

//not implemented yet...	

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
  sprintf(newunits, "nm");//assume always nanometers
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

