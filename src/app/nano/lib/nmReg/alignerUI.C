#include <microscape.h> // for disableOtherTextures
#include "alignerUI.h"

// from microscape.c - It doesn't make sense to replicate the tcl and c code
// that is linked with this string in order to get these controls since
// realign_textures and registration are mutually exclusive
extern Tclvar_string texturePlaneName;

AlignerUI::AlignerUI(nmg_Graphics *g, nmb_ImageList *im,
    Tcl_Interp *tcl_interp, const char *tcl_script_dir):
	datasetRegistrationPlaneName3D("none"),
	datasetRegistrationPlaneName2D("none"),
	newResamplePlaneName("reg(resample_plane_name)", ""),
	datasetRegistrationEnabled("reg_window_open", 0),
	datasetRegistrationNeeded("reg(registration_needed)", 0),
	datasetRegistrationRotate3DEnabled("reg(rotate3D_enable)", 0),
        textureDisplayEnabled("reg(display_texture)", 0),
	resampleResolutionX("reg(resample_resolution_x)", 100),
	resampleResolutionY("reg(resample_resolution_y)", 100),
	num_image_windows(2),
	height_image(0),
        texture_image(1),
	ce(NULL),
	aligner(NULL),
	images(im),
	graphics_display(g)
{

    char command[256];
    printf("creating the aligner\n");

    /* Load the Tcl script that handles main interface window */
    /*
    sprintf(command, "source %s/%s",tcl_script_dir, ALIGNER_TCL_FILE);
    printf("evaluating %s\n", command);
    TCLEVALCHECK2(tcl_interp, command);
    fprintf(stderr, "done evaluating\n");
    */
	char *reg_win_names[2] = {"registration:topography",
				"registration:texture"};
	ce = new CorrespondenceEditor(num_image_windows, reg_win_names);
	aligner = new Aligner(num_image_windows);

	datasetRegistrationPlaneName3D.initializeTcl
		("topography_data");
	//datasetRegistrationPlaneName3D.bindList(images->imageNameList());
	datasetRegistrationPlaneName2D.initializeTcl
		("texture_data");
	//datasetRegistrationPlaneName2D.bindList(images->imageNameList());

	datasetRegistrationEnabled.addCallback
		(handle_registration_enabled_change, (void *)this);
	datasetRegistrationPlaneName3D.addCallback
		(handle_registration_dataset3D_change, (void *)this);
	datasetRegistrationPlaneName2D.addCallback
		(handle_registration_dataset2D_change, (void *)this);
	datasetRegistrationNeeded.addCallback
		(handle_registration_change, (void *)this);
	datasetRegistrationRotate3DEnabled.addCallback
		(handle_registration_type_change, (void *)this);
        textureDisplayEnabled.addCallback
		(handle_texture_display_change, (void *)this);

	//newResamplePlaneName.bindList(images->imageNameList());
	newResamplePlaneName = "";
	newResamplePlaneName.addCallback
		(handle_resamplePlaneName_change, (void *)this);
        resampleResolutionX = 100;
        resampleResolutionY = 100;

}

AlignerUI::~AlignerUI()
{
   delete aligner;
   delete ce;
}

int AlignerUI::addImage(nmb_Image *im) {
	// adds image to nmb_ImageList --> adds entry to nmb_ListOfStrings -->
	// updates strings which were bound to this list in constructor
	return images->addImage(im);
}

// static
void     AlignerUI::handle_resamplePlaneName_change(const char *, void *ud)
{
	AlignerUI *aui = (AlignerUI *)ud;
	Aligner *aligner = aui->aligner;
	nmb_Image *new_image;
	nmb_Image *im_3D = aui->images->getImageByName
                            (aui->datasetRegistrationPlaneName3D.string());
	nmb_Image *im_2D = aui->images->getImageByName
                            (aui->datasetRegistrationPlaneName2D.string());

	// Make sure we have all the information we need and
    // see if the user has given a name to the resampled plane
    // other than "".  If so, we should create a new plane and set the value
    // back to "".
	if (!im_3D || !im_2D || 
	    (strlen(aui->newResamplePlaneName.string()) == 0)){
		return;
	} else {
		// we might want to use a different resolution here:
		new_image = new nmb_ImageGrid(
			(const char *)(aui->newResamplePlaneName.string()),
			(const char *)(im_2D->unitsValue()),
			aui->resampleResolutionX, aui->resampleResolutionY);
		nmb_ImageBounds im3D_bounds;
		im_3D->getBounds(im3D_bounds);
		new_image->setBounds(im3D_bounds);
		aui->newResamplePlaneName = (const char *) "";
	}
	// get the transformation that takes points from height image to points
	// in the texture image
	ImageTransform *xform = aligner->getTransform(aui->height_image,
		aui->texture_image);

	aligner->resampleImageToDepthImage((*im_2D), (*im_3D), (*xform), 
					(*new_image));
	// now make it available elsewhere:
	aui->images->addImage(new_image);
}

// callbacks for dataset registration control panel

// this tells whether or not the control panel is visible
void    AlignerUI::handle_registration_enabled_change(vrpn_int32 new_value, 
				void *userdata) {
  AlignerUI *aui = (AlignerUI *)userdata;
  nmb_Image *im = aui->images->getImageByName
                        (aui->datasetRegistrationPlaneName2D.string());
  if (new_value){
    aui->ce->show();
    if (im) {
        aui->graphics_display->createRealignTextures(
	    aui->datasetRegistrationPlaneName2D.string());
    }
  }
  else {
    aui->ce->hide();
  }
}

void    AlignerUI::handle_registration_dataset3D_change(const char *, 
				void * userdata)
{
  AlignerUI *aui = ((AlignerUI *)userdata);
  nmb_Image *im = aui->images->getImageByName
                            (aui->datasetRegistrationPlaneName3D.string());

  if (im != NULL) {
    aui->ce->setImage(aui->height_image, im);		// set UI state
	aui->resampleResolutionX = im->width();
	aui->resampleResolutionY = im->height();
  }
}

void    AlignerUI::handle_registration_dataset2D_change(const char *name, 
			void * userdata)
{
	AlignerUI *aui = ((AlignerUI *)userdata);
	nmb_Image *im = aui->images->getImageByName(name);
		//	(aui->datasetRegistrationPlaneName2D.string());

	printf("in handle_registration_dataset2D_change\n");
	if (im != NULL) {
		// set UI state
                // NOTE: call to createRealignTextures is generated in the
                // callback for the variable "texturePlaneName"
		texturePlaneName = 
			(aui->datasetRegistrationPlaneName2D.string());
		aui->ce->setImage(aui->texture_image, im);
		aui->graphics_display->createRealignTextures(name);
		//	aui->datasetRegistrationPlaneName2D.string());
	} else {
	    fprintf(stderr, "AlignerUI::handle_registration_dataset2D_change:"
		" Error, couldn't find image\n");
        }

/*
  if (plane != NULL) {
    ce->setImage(texture_image, im);
    // perhaps there should be a separate function for this
    // especially when graphics keeps track of multiple textures for
    // different modes - but registration and realign should probably
    // share such a function anyway
    graphics->createRealignTextures(datasetRegistrationPlaneName2D.string());
  }
*/
/* This should not be used because its an ugly special case
	(PPM images can just be read into a plane - but then they may be
	 a different resolution from the images in the global dataset!)
  else if (rulerPPMName) {  // set image from rulerimage
    PPM *im = new PPM(rulerPPMName);
    if (im->valid)
        ce->setImageFromPNM(texture_image, im);
    delete im;
    graphics->loadRulergridImage(rulerPPMName);
  }
*/
}

void    AlignerUI::handle_registration_change(vrpn_int32 val, void *userdata) {
  if (val == 0) return;
  printf("AlignerUI::registering\n");
  AlignerUI *aui = ((AlignerUI *)userdata);
  CorrespondenceEditor *ce = aui->ce;
  Aligner *aligner = aui->aligner;
  Correspondence c;

  nmb_Image *h_im = aui->images->getImageByName
                            (aui->datasetRegistrationPlaneName3D.string());
  nmb_Image *t_im = aui->images->getImageByName
		(aui->datasetRegistrationPlaneName2D.string());

  // note: correspondence point coordinates for texture_image are left
  // as normalized coordinates with x,y in [0..1, 0..1]

  // transfer correspondence information set by user into the aligner
  // (may server only as an initial guess so we also pass in the images
  //  to be used for intensity-based registration)
  ce->getCorrespondence(c);
  // set x and y to units for the image and also set z values from
  // image values
  if (h_im != NULL) {
      c.setValuesFromImage(aui->height_image, h_im);
  }
  aligner->setCorrespondence(c);
  aligner->setData(aui->height_image, h_im);
  aligner->setData(aui->texture_image, t_im);
  aligner->updateTransform(aui->height_image, aui->texture_image);

  aui->datasetRegistrationNeeded = 0;
  double xform_matrix[16];

  // this interface will need to be changed if we want a more complicated
  // transformation for computing texture coordinates
  aligner->getGLtransform(aui->height_image, aui->texture_image, xform_matrix);
  aui->graphics_display->setTextureTransform(xform_matrix);

// STUFF IN THIS FUNCTION AFTER THIS LINE CAN PROBABLY BE REPLACED WHEN
// DEBUGGING IS DONE

/*
I don't remember why I put this in here but it seems like it shouldn't
be here; will take out after testing
  else {
      plane = dataset->inputGrid->getPlaneByName
                            (datasetRegistrationPlaneName2D.string());
      if (plane != NULL) {
          c.setValuesFromPlane(texture_image, plane);
      }
  }
*/

/*
  transformSolver(xform_matrix, &error, c, height_image, texture_image,
    (vrpn_bool)datasetRegistrationRotate3DEnabled);

  printf("error = %g\n", error);
*/

/*
  double proj_vec[3] = {0,0,0};
  computeProjectionVector(xform_matrix, proj_vec);
  printf("proj dir: (%g,%g,%g)\n", proj_vec[0], proj_vec[1],proj_vec[2]);
*/
  // add the scaling factor so that texture image is in nanometers
  // instead of from 0..1 for printing out the transformation
  // Also, add in the x,y offset since the texture origin is 0,0 but the
  // plane from which it was constructed may have an offset because it
  // is not at the origin of the scan range
/*
  double nm_xform_matrix[6];
  im = images->getImageByName
    (datasetRegistrationPlaneName2D.string());
  // the transformation converts from nm to pixels if we only have an image
  // so we can't really say what the translation, shear or rotation is
  if (im != NULL) {
     double image_width = im->widthWorld();
     double image_height = im->heightWorld();
     nm_xform_matrix[0] = xform_matrix[0]*image_width;
     nm_xform_matrix[1] = xform_matrix[4]*image_width;
//     nm_xform_matrix[2] = xform_matrix[12]*image_width + plane->minX();
     nm_xform_matrix[3] = xform_matrix[1]*plane_height;
     nm_xform_matrix[4] = xform_matrix[5]*plane_height;
//     nm_xform_matrix[5] = xform_matrix[13]*image_height + plane->minY();
	 nm_xform_matrix[2] = xform_matrix[12]*im->width stopped here
	 pixelToWorld(
     int i;
     printf("homogenous transformation matrix:\n");
     for (i = 0; i < 3; i++)
        printf("%g ", nm_xform_matrix[i]);
     printf("\n");
     for (i = 3; i < 6; i++)
        printf("%g ", nm_xform_matrix[i]);
     printf("\n");

     double scale_x, scale_y, rotation_x, rotation_y, shear;
     scale_x = sqrt(nm_xform_matrix[0]*nm_xform_matrix[0] +
            nm_xform_matrix[3]*nm_xform_matrix[3]);
     scale_y = sqrt(nm_xform_matrix[1]*nm_xform_matrix[1] +
            nm_xform_matrix[4]*nm_xform_matrix[4]);
     rotation_x = (180.0/M_PI)*atan(nm_xform_matrix[3]/nm_xform_matrix[0]);
     rotation_y = (180.0/M_PI)*atan(-nm_xform_matrix[1]/nm_xform_matrix[4]);
     shear = rotation_y - rotation_x;

     printf("********************\n");
     printf("scale_x = %g\n", scale_x);
     printf("scale_y = %g\n", scale_y);
     printf("rotation = %g degrees\n", rotation_x);
     printf("shear = %g degrees\n", shear);
     printf("translation_x = %g\n", nm_xform_matrix[2]);
     printf("translation_y = %g\n", nm_xform_matrix[5]);

  }
*/
/*
  int i;
  for (i = 0; i < 4; i++)
    printf("%g ", xform_matrix[4*i]);
  printf("\n");
  for (i = 0; i < 4; i++)
        printf("%g ", xform_matrix[4*i+1]);
  printf("\n");
*/
}

void    AlignerUI::handle_registration_type_change(vrpn_int32 val, void *ud)
{
  printf("change in registration type\n");
}

void    AlignerUI::handle_texture_display_change(vrpn_int32 val, void *ud)
{
    AlignerUI *aui = ((AlignerUI *)ud);
    if (val) {
      disableOtherTextures(REGISTRATION);
      aui->graphics_display->setTextureMode(nmg_Graphics::COLORMAP,
                                          nmg_Graphics::REGISTRATION_COORD);
    }
    else {
      if (aui->graphics_display->getTextureMode() == nmg_Graphics::COLORMAP) {
        aui->graphics_display->setTextureMode(nmg_Graphics::NO_TEXTURES,
                                            nmg_Graphics::RULERGRID_COORD);
      }
    }
}
