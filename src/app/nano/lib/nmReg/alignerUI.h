#ifndef ALIGNERUI_H
#define ALIGNERUI_H

#include "Tcl_Linkvar.h"
#include "Tcl_Netvar.h"
#include "nmg_Graphics.h"
#include "nmb_Image.h"
#include "correspondenceEditor.h"
#include "aligner.h"

class AlignerUI {
  public:
    AlignerUI(nmg_Graphics *g, nmb_ImageList *im);
	int addImage(nmb_Image *im);

	void mainloop() {ce->mainloop();};

	// user interface callback routines:
	static void handle_registration_enabled_change(vrpn_int32, void *);
	static void handle_registration_dataset3D_change(const char *, void *);
	static void handle_registration_dataset2D_change(const char *, void *);
	static void handle_registration_change(vrpn_int32, void *);
	static void handle_registration_type_change(vrpn_int32, void *);
	static void handle_resamplePlaneName_change(const char *, void *);

  private:

	// control panel variables
	Tclvar_selector datasetRegistrationPlaneName3D;
	Tclvar_selector datasetRegistrationPlaneName2D;
	Tclvar_selector newResamplePlaneName;
	Tclvar_int datasetRegistrationEnabled;
	Tclvar_int datasetRegistrationNeeded;
	Tclvar_int_with_button datasetRegistrationRotate3DEnabled;
	Tclvar_int resampleResolutionX;
	Tclvar_int resampleResolutionY;

	const int num_image_windows; // equals ce->numImages()
	// these are the indices of the images represented by
    // datasetRegistrationPlaneName3D and datasetRegistrationPlaneName2D
    // passed to the CorrespondenceEditor. The CorrespondenceEditor then
    // uses the same indices in the resulting Correspondence so we
    // know which points go with which image
    const int height_image;   // index from 0..ce->numImages()
    const int texture_image;  // index from 0..ce->numImages()

	CorrespondenceEditor *ce;	// ui to let user set landmarks in
					// a set of images

	Aligner *aligner;               // where the work gets done, contains
					// information required for doing
					// alignment (Correspondence object),
					// references to images and also
					// the results (ImageTransform object)
								
	nmb_ImageList *images;	// list of images that can be manipulated
				// including references to AFM or
				// SEM data if it exists,
				// contains reference to string list 
				// variable displayed by the selectors
				// datasetRegistrationPlaneName3D,
				// datasetRegistrationPlaneName2D so that
				// changes here are reflected in the
				// control panel

	nmg_Graphics *graphics_display;	// need this so we can send requests to
				// update display of registration results
};

#endif
