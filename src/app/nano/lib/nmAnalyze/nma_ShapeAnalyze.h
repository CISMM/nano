#ifndef NM_ANALYZE
#define NM_ANALYZE

class nmb_PlaneSelection;
class CNT_IA;

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

};




//class added by andrea hilchey 6/14/01
//adds a new height plane when you select "ShapeAnalysis" from the Analysis heading
//the new plane shows where nanotubes have been identified
class nma_ShapeIdentifiedPlane 
  : virtual public nmb_CalculatedPlane
{
public:
	friend class nmb_CalculatedPlane;

        nma_ShapeIdentifiedPlane(BCPlane * sourcePlane, nmb_Dataset * dataset, 
				 char* outputPlaneName, double * cntMask);
        ~nma_ShapeIdentifiedPlane();
  
	// Accessor.  Returns that calculated plane.
	BCPlane* getCalculatedPlane(){return NULL;}//fill these in later

	// returns the name of the calculated plane
	const BCString* getName(){return NULL;}//fill in later

	// Packs up and sends across the connection all the data
	// necessary for the other end to recreate this calculated 
	// plane.
	void sendCalculatedPlane( vrpn_Connection* /* conn */, 
				  vrpn_int32 /* senderID */,
				  vrpn_int32 /*synchCalcdPlaneMessageType */ )
	  const {}

protected:
	// create a new plane according to the data from vrpn
	static nmb_CalculatedPlane*
	  _handle_PlaneSynch( vrpn_HANDLERPARAM /* p */, 
			      nmb_Dataset* /* dataset */ )
	  throw( nmb_CalculatedPlaneCreationException )
	  {return NULL;}

	// Update the calculated plane for changes in the source plane
	static void sourcePlaneChangeCallback( BCPlane* /* plane */, 
					       int /* x */, int /* y */,
					       void* /* userdata */ )
	  { }
private:
	void create_ShapeIdentifiedPlane();  //creates new plane

	BCPlane * d_sourcePlane;       //source plane the new plane came from
	BCPlane * d_outputPlane;       //new plane
	nmb_Dataset * d_dataset;       //the dataset we belong to
	char * d_outputPlaneName;      //what to call the plane
	double * d_cntMask;            //the array of values (0 or 255) determining what is a nanotube (object)
	                               //and what is not (255 is a nanotube)
};

#endif
