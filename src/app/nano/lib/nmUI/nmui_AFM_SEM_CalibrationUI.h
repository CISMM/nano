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
#include "URVector.h"
#include "nmui_ColorMap.h"

class nmui_AFM_SEM_CalibrationUI {
  public:

    nmui_AFM_SEM_CalibrationUI(nmr_Registration_Proxy *aligner,
		UTree *world);
    ~nmui_AFM_SEM_CalibrationUI();

	void setSPM(nmm_Microscope_Remote *scope);
	void setSEM(nmm_Microscope_SEM_Remote *sem);
    void setTipRenderer(URender *tipRenderer);

    void changeDataset(nmb_ImageManager *dataset);

    // generate synthetic data based on the specified transformations
    void createTestImages();
    void addContactPoint(double *afm_pnt, double *sem_pnt, nmb_Image *semImage);
    void addFreePoint(double *afm_pnt, double *sem_pnt, nmb_Image *semImage);

    void clearContactPoints();
    void clearFreePoints();
  private:
    int drawTestImages(const ImageViewerDisplayData &data);
    static int testImageWindowDisplayHandler
		(const ImageViewerDisplayData &data, void *ud);

    static void correspondenceEditorChangeHandler(void *ud,
                  const nmr_ProxyChangeHandlerData &info);

    static void handle_windowOpen_change(vrpn_int32 value, void *ud);

    // radio button - three modes: model_SEM, AFM_SEM_contact, AFM_SEM_free
    static void handle_registrationMode_change(vrpn_int32 value, void *ud);

    void setFiducial();

    // model_SEM selectors
    static void handle_modelToSEM_model_change(const char *name, void *ud);
    static void handle_modelToSEM_SEMImage_change(const char *name, void *ud);
    static void handle_updateModel_change(vrpn_int32 value, void *ud);
    static void handle_updateSEMImage_change(vrpn_int32 value, void *ud);

    // AFM_SEM_contact
    // buttons
    static void handle_AddContactPoint(vrpn_int32 value, void *ud);
    void addLatestDataAsContactPoint();
    static void handle_DeleteContactPoint(vrpn_int32 value, void *ud);
    // point selector
    static void handle_currentContactPoint_change(const char *name, void *ud);

    // AFM_SEM_free
    // buttons
    static void handle_AddFreePoint(vrpn_int32 value, void *ud);
    void addLatestDataAsFreePoint();
    static void handle_DeleteFreePoint(vrpn_int32 value, void *ud);
    // point selector
    static void handle_currentFreePoint_change(const char *name, void *ud);

    static void handle_updateSolution_change(vrpn_int32 value, void *ud);
    static void handle_drawSurface_change(vrpn_int32 value, void *ud);
    static void handle_drawSurfaceTexture_change(vrpn_int32 value, void *ud);
    static void handle_liveSEMTexture_change(vrpn_int32 value, void *ud);
    static void handle_textureOpacity_change(vrpn_float64 value, void *ud);
    static void handle_ZScale_change(vrpn_float64 value, void *ud);
	    //   for colormap
    static void handle_colormap_change(const char *name, void *_ud);
    //   for colormap minmax
    static void handle_colormap_minmax_change(vrpn_float64, void *_ud);
    
    static void handle_generateTestData_change(
	    vrpn_int32 value, void *ud);

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
    Tclvar_int d_updateModel;
    Tclvar_string d_modelToSEM_SEMImageName;
    Tclvar_int d_updateSEMImage;

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

    Tclvar_int d_updateSolution;

    Tclvar_int d_drawSurface;
    Tclvar_int d_drawSurfaceTexture;
    Tclvar_int d_liveSEMTexture;

    Tclvar_float d_textureOpacity;
    Tclvar_float d_ZScale;

    Tclvar_string d_semColormapImageName;
    Tclvar_string d_semColormap;

    Tclvar_int d_generateTestData;

    nmui_ColorMap* d_colorMapUI;

    // AFM
    nmm_Microscope_Remote *d_AFM;
    int finishedFreehandHandler();
    static int finishedFreehandHandler(void *ud);
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
    static int VRPN_CALLBACK semSynchHandler (void *ud,
                            const nmb_SynchMessage *data);
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
    URVector d_textureProjectionDirectionRenderer;
    URProjectiveTexture d_SEMTexture;
    int d_surfaceStride;
    UTree *d_world;
    nmr_SurfaceModelHeightField *d_heightField;
    vrpn_bool d_heightFieldNeedsUpdate;
    URender *d_tipRenderer;

	double d_SEMfromAFM[16];
	double d_SEMfromModel[16];
	double d_AFMfromModel[16];


	double d_SEMfromAFM_test[16];
	double d_AFMfromModel_test[16];
	int d_testImageWinID;
	bool d_testImageWinCreated;
	bool d_testImageWinNeedsUpdate;

	bool d_modelNeedsUpdate;
	bool d_SEMNeedsUpdate;
};


#endif
