#ifndef NMUI_AFM_SEM_CALIBRATIONUI_H
#define NMUI_AFM_SEM_CALIBRATIONUI_H

#include "nmr_Registration_Proxy.h"
#include "nmm_MicroscopeRemote.h"
#include "nmm_Microscope_SEM_Remote.h"
#include "Tcl_Linkvar.h"
#include "nmr_AFM_SEM_Calibration.h"
#include "nmb_ImageManager.h"
#include "nmb_Image.h"
#include "nmg_ImageDisplayProjectiveTexture.h"
#include "UTree.h"
#include "URHeightField.h"

class nmui_AFM_SEM_CalibrationUI {
  public:

    nmui_AFM_SEM_CalibrationUI(nmr_Registration_Proxy *aligner,
		UTree *world,
		nmg_ImageDisplayProjectiveTexture *textureDisplay);
    ~nmui_AFM_SEM_CalibrationUI();

	void setSPM(nmm_Microscope_Remote *scope);
	void setSEM(nmm_Microscope_SEM_Remote *sem);

	void changeDataset(nmb_ImageManager *dataset);

	// generate synthetic data based on the specified transformations
	void createTestImages(double *SEM_from_AFM, double *AFM_from_model);
	void addContactPoint(double *afm_pnt, double *sem_pnt, nmb_Image *semImage);
	void addFreePoint(double *afm_pnt, double *sem_pnt, nmb_Image *semImage);

  private:
	static void correspondenceEditorChangeHandler(void *ud,
                  const nmr_ProxyChangeHandlerData &info);

	static void handle_windowOpen_change(vrpn_int32 value, void *ud);

    // radio button - three modes: model_SEM, AFM_SEM_contact, AFM_SEM_free
    static void handle_registrationMode_change(vrpn_int32 value, void *ud);

	void setFiducial();

    // model_SEM selectors
    static void handle_modelToSEM_model_change(const char *name, void *ud);
    static void handle_modelToSEM_SEMImage_change(const char *name, void *ud);

    // AFM_SEM_contact
    // buttons
    static void handle_AddContactPoint(vrpn_int32 value, void *ud);
    static void handle_DeleteContactPoint(vrpn_int32 value, void *ud);
    // point selector
    static void handle_currentContactPoint_change(const char *name, void *ud);

    // AFM_SEM_free
    // buttons
    static void handle_AddFreePoint(vrpn_int32 value, void *ud);
    static void handle_DeleteFreePoint(vrpn_int32 value, void *ud);
    // point selector
    static void handle_currentFreePoint_change(const char *name, void *ud);

	static void handle_generateTestData_change(
		vrpn_int32 value, void *ud);
	static void handle_updateSolution_change(vrpn_int32 value, void *ud);

	void updateInputStatus();
	void updateSolution();
	void updateSolutionDisplay();

	Tclvar_int d_windowOpen;
    // values for the acquisition mode variable
    Tclvar_int d_registrationMode_model_SEM;
    Tclvar_int d_registrationMode_AFM_SEM_contact;
    Tclvar_int d_registrationMode_AFM_SEM_free;

    Tclvar_int d_registrationMode;

    Tclvar_string d_modelToSEM_modelImageName;
    Tclvar_string d_modelToSEM_SEMImageName;

    Tclvar_int d_addContactPoint;
    Tclvar_int d_deleteContactPoint;

    Tclvar_string d_currentContactPoint;
    Tclvar_list_of_strings d_contactPointList;

    Tclvar_int d_addFreePoint;
    Tclvar_int d_deleteFreePoint;

    Tclvar_string d_currentFreePoint;
    Tclvar_list_of_strings d_freePointList;

	Tclvar_int d_moreDataNeeded;
	Tclvar_int d_modelSEMPointsNeeded;
	Tclvar_int d_contactPointsNeeded;
	Tclvar_int d_freePointsNeeded;

	Tclvar_int d_generateTestData;
	Tclvar_int d_updateSolution;
 
    // AFM
    nmm_Microscope_Remote *d_AFM;
    int pointDataHandler(const Point_results *pr);
    static int pointDataHandler(void *ud, const Point_results *pr);
	// stores the latest position reported by the AFM
    double d_tipPosition[3]; 
	vrpn_bool d_tipPositionAcquiredSinceLastAdd;

    // SEM - this object already manages the last image acquired
    // so we just use this to get a reference to the image
	vrpn_bool d_semImageAcquiredSinceLastAdd;
	static void semDataHandler(void *userdata,
                          const nmm_Microscope_SEM_ChangeHandlerData &info);
    nmm_Microscope_SEM_Remote *d_SEM;

    // alignment module
    nmr_Registration_Proxy *d_aligner;

	// correspondence information
	int d_numModelSEMPoints;
	nmr_CalibrationPoint d_modelToSEM_modelPoints[NMR_MAX_FIDUCIAL];
	nmr_CalibrationPoint d_modelToSEM_SEMPoints[NMR_MAX_FIDUCIAL];
	nmb_Image *d_modelToSEM_modelImage;
	nmb_Image *d_modelToSEM_SEMImage;

	int d_numContactPoints;
	nmr_CalibrationPoint d_contact_AFMPoints[NMR_MAX_FIDUCIAL];
	nmr_CalibrationPoint d_contact_SEMPoints[NMR_MAX_FIDUCIAL];
	nmb_Image *d_contactImages[NMR_MAX_FIDUCIAL];

	int d_numFreePoints;
	nmr_CalibrationPoint d_free_AFMPoints[NMR_MAX_FIDUCIAL];
	nmr_CalibrationPoint d_free_SEMPoints[NMR_MAX_FIDUCIAL];
	nmb_Image *d_freeImages[NMR_MAX_FIDUCIAL];

	nmb_ImageManager *d_dataset;

	// solver module with current solution
	nmr_AFM_SEM_Calibration *d_calibration;

	// data needed for display of solution
	URHeightField d_surfaceModelRenderer;
	int d_surfaceStride;
	UTree *d_world;
	nmg_ImageDisplayProjectiveTexture *d_textureDisplay;
	nmr_SurfaceModelHeightField *d_heightField;
	vrpn_bool d_heightFieldNeedsUpdate;

	double d_SEMfromAFM[16];
	double d_SEMfromModel[16];
	double d_AFMfromModel[16];

};


#endif
