#ifndef NM_ANALYZE
#define NM_ANALYZE


#ifdef _WIN32
// turns off warning C4290: C++ Exception Specification ignored
#pragma warning( push )
#pragma warning( disable : 4290 )
#endif


class nmb_PlaneSelection;
class CNT_IA;
class nma_ShapeIdentifiedPlane;
class nmm_SimulatedMicroscope_Remote;

#include "nmb_CalculatedPlane.h"
#include <vrpn_Connection.h>


/////////////////////////////////////////////////////////////////
// Class:       nma_ShapeAnalyze
// Description: Interface class to the shape analysis code to
//              allow nano and that code to evolve independently
/////////////////////////////////////////////////////////////////
class nma_ShapeAnalyze
{
public:
	nma_ShapeAnalyze();
        ~nma_ShapeAnalyze();

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

	void imageAnalyze(nmb_PlaneSelection planeSelection, 
			  nmb_Dataset * dataset);
	//pass in dataset so that it is current when image analysis is done

        static int nma_ShapeAnalyzeCounter;
private:
	char d_imgMaskFile[255];  //255 characters should be plenty for any file name
	char d_desiredFilename[255];
	
	char d_imgOrdFile[255];
	char d_txtFile[255];

	int d_maskWrite;
	int d_ordWrite;

	CNT_IA *d_cntRec;
	nma_ShapeIdentifiedPlane *shapePlane;
	double * reverse_array;

};




//class added by andrea hilchey 6/14/01
//adds a new height plane when you select "ShapeAnalysis" from the Analysis heading
//the new plane shows where nanotubes have been identified
//also used by nmm_SimulatedMicroscope_Remote
//can update a plane with lines of data, and, if the data is not the same size as the
//source plane (of the ShapeIdentifiedPlane), the data will be expanded by interpolating
//between the existing data points and thus blurring the image to look smooth and be the
//appropriate size
class nma_ShapeIdentifiedPlane 
  : virtual public nmb_CalculatedPlane
{
public:
	friend class nmb_CalculatedPlane;
	friend class nmm_SimulatedMicroscope_Remote;

    nma_ShapeIdentifiedPlane(BCPlane * sourcePlane, nmb_Dataset * dataset, 
				 const char* outputPlaneName, double * cntMask, void * creator = NULL);
	nma_ShapeIdentifiedPlane(BCPlane * sourcePlane, nmb_Dataset * dataset, 
				 const char* outputPlaneName);
    ~nma_ShapeIdentifiedPlane();

	//updates d_cntMask when new information is received
	void UpdateDataArray(double * dataline, const int y, int datain_rowlen);
  
        // Accessor.  Returns true if this calc'd plane depend on
        // (is calculated from) the specified plane.
        bool dependsOnPlane( const BCPlane* const plane );
        bool dependsOnPlane( const char* planeName );

	// Accessor.  Returns that calculated plane.
	BCPlane* getCalculatedPlane(){return NULL;}//fill these in later

	// returns the name of the calculated plane
	const string* getName(){return NULL;}//fill in later

	// Packs up and sends across the connection all the data
	// necessary for the other end to recreate this calculated 
	// plane.
	void sendCalculatedPlane( vrpn_Connection* /* conn */, 
				  vrpn_int32 /* senderID */,
				  vrpn_int32 /*synchCalcdPlaneMessageType */ )const {}

protected:

	// Check for changes in the source plane
	static void sourcePlaneChangeCallback( BCPlane* plane, int x, int y,
				 void* userdata );

	// non-static member function to handle changes in the source plane
	void _handleSourcePlaneChange( int x, int y );

	//used by UpdateDataArray; blurs current dataline up or down to fit required rowlength 
	//(filling in points in between if d_rowlength is larger, and blur between current row 
	//of data and previous row
	//void nonblur(double ** dataline, int& y, int& datain_rowlen);
	void blur_data_up(double ** dataline, int& y, int& datain_rowlen);
	void blur_plane_up(double ** dataline, int& y, int& datain_rowlen);

private:
	int create_ShapeIdentifiedPlane();  //creates new plane

	BCPlane * d_sourcePlane;		//source plane the new plane came from
	BCPlane * d_outputPlane;		//new plane
	nmb_Dataset * d_dataset;		//the dataset we belong to
	const char * d_outputPlaneName;	//what to call the plane
	double * d_dataArray;			//the array of values 
	double * storeddataline;		//the last row of data we have received in (interpolated)
									//used for interpolation of rows between last row and the current row
	int d_array_size;				//size of the source plane
	short d_rowlength;				//length of row in source plane
    short d_columnheight;			//height of column in source plane
	double stepsize;				//keeps track of the increment in one direction in the simulator
									//x or y space one has to go to get to the next place in that space
									//to pull a value from--sim x-space divided up into (d_rowlength - 1) 
									//chunks of size stepsize and sim y-space divided up into 
									//(d_columnheight - 1) chunks of size stepsize
	bool firstblur;					//set to true initially, changed to false once source plane filled in
									//with updated values for the first time, ensures that interpolation
									//code always starts with the y=0 row sent over from the simulator by
									//keeping firstblur = true until a y=0 row is sent->triggers interpolation
									//code
	double placeholder_x;			//incremented by stepsize, holds the place in sim x-space that the current
									//value needs to be pulled from
	double placeholder_y;			//incremented by stepsize, holds the place that the current row should fall at
									//in sim y-space
	int l_sim_x;					//lower point of two points in sim x-space to interpolate between to get value
									//at placeholder_x
	int h_sim_x;					//upper (higher) point in sim x-space to interpolate between
	int l_sim_y;					//lower rownumber of two rownumbers in sim y-space to interpolate between to get
									//values for rownumber placeholder_y
	int h_sim_y;					//upper rownumber in sim y-space to interpolate between
	int nano_y;						//y value in the source plane array at which the new point is being inserted
	nmm_SimulatedMicroscope_Remote* remoteEroderConnObj;	//pointer to simulated microscope object that created
															//this shape identified plane object, if there is one
	FILE * outfile;					//for reading out data array

};


#ifdef _WIN32
#pragma warning( pop )
#endif



#endif
