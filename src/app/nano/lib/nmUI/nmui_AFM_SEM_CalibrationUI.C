#include <GL/glut_UNC.h>
#include "nmui_AFM_SEM_CalibrationUI.h"
#include "error_display.h"

#define SCAN_COMPLETION_MESSAGE "scan complete"

nmui_AFM_SEM_CalibrationUI::nmui_AFM_SEM_CalibrationUI(
    nmr_Registration_Proxy *aligner,
	UTree *world): 

	d_aligner(aligner), 
	d_windowOpen("afm_sem_window_open", 0),
	d_registrationMode_model_SEM("afm_sem_registration_mode_model_SEM",1),
	d_registrationMode_AFM_SEM_contact("afm_sem_registration_mode_AFM_SEM_contact",2),
	d_registrationMode_AFM_SEM_free("afm_sem_registration_mode_AFM_SEM_free",3),
	d_registrationMode("afm_sem_registration_mode",1),
	d_modelToSEM_modelImageName("afm_sem_model", "none"),
	d_updateModel("afm_sem_update_model", 0),
	d_modelToSEM_SEMImageName("afm_sem_sem_image", "none"),
	d_updateSEMImage("afm_sem_update_sem_image", 0),
	d_addContactPoint("add_contact_point", 0),
	d_deleteContactPoint("delete_contact_point", 0),
	d_currentContactPoint("afm_sem_current_contact_point", "none"),
	d_contactPointList("contact_point_list"),
	d_addFreePoint("add_free_point", 0),
	d_deleteFreePoint("delete_free_point", 0),
	d_currentFreePoint("afm_sem_current_free_point", "none"),
	d_freePointList("free_point_list"),
	d_moreDataNeeded("afm_sem_more_data_needed", 1),
	d_modelSEMPointsNeeded("afm_sem_model_sem_points_needed", 1),
	d_contactPointsNeeded("afm_sem_contact_points_needed", 1),
    d_freePointsNeeded("afm_sem_free_points_needed", 1),
	d_updateSolution("afm_sem_update_solution", 0),
	d_drawSurface("afm_sem_draw_surface", 0),
	d_drawSurfaceTexture("afm_sem_draw_surface_texture", 0),
	d_liveSEMTexture("afm_sem_live_sem_texture", 0),
	d_textureOpacity("afm_sem_texture_opacity", 0.5),
	d_ZScale("afm_sem_z_scale", 1.0),
    d_semColormapImageName("afm_sem_cm(color_comes_from)", "none"),
    d_semColormap("afm_sem_cm(color_map)", "none"),
	d_generateTestData("afm_sem_generate_test_data", 0),
	d_AFM(NULL),
	d_SEM(NULL),
	d_dataset(NULL),
	d_tipPositionAcquiredSinceLastAdd(vrpn_FALSE),
	d_semImageAcquiredSinceLastAdd(vrpn_FALSE),
	d_calibration(NULL),
	d_surfaceModelRenderer(),
	d_textureProjectionDirectionRenderer(),
	d_world(world),
	d_surfaceStride(3),
	d_heightField(NULL),
	d_heightFieldNeedsUpdate(vrpn_TRUE),
    d_tipRenderer(NULL),
    d_testImageWinCreated(false),
	d_testImageWinNeedsUpdate(false)
{
  d_tipPosition[0] = 0;
  d_tipPosition[1] = 0;
  d_tipPosition[2] = 0;
  d_numModelSEMPoints = 0;
  d_numContactPoints = 0;
  d_numFreePoints = 0;
  d_modelToSEM_modelImage = NULL;
  d_modelToSEM_SEMImage = NULL;

  d_colorMapUI = new nmui_ColorMap("afm_sem_cm",
                            &d_semColormapImageName,
                            &d_semColormap);
  d_colorMapUI->setSurfaceColor(255, 255, 255);

  int i, j;
  nmr_CalibrationPoint defaultPointValue = {0,0,0,1};
  for (i = 0; i < NMR_MAX_FIDUCIAL; i++) {
	d_contactImages[i] = NULL;
	d_freeImages[i] = NULL;
	for (j = 0; j < 4; j++) {
		d_modelToSEM_modelPoints[i][j] = defaultPointValue[j];
		d_modelToSEM_SEMPoints[i][j] = defaultPointValue[j];
		d_contact_AFMPoints[i][j] = defaultPointValue[j];
		d_contact_SEMPoints[i][j] = defaultPointValue[j];
		d_free_AFMPoints[i][j] = defaultPointValue[j];
		d_free_SEMPoints[i][j] = defaultPointValue[j];
	}
  }

  d_windowOpen.addCallback(handle_windowOpen_change, this);
  d_registrationMode.addCallback(handle_registrationMode_change, this);
  d_modelToSEM_modelImageName.addCallback(handle_modelToSEM_model_change, 
			this);
  d_updateModel.addCallback(handle_updateModel_change, this);
  d_modelToSEM_SEMImageName.addCallback(handle_modelToSEM_SEMImage_change, 
			this);
  d_updateSEMImage.addCallback(handle_updateSEMImage_change, this);
  d_addContactPoint.addCallback(handle_AddContactPoint, this);
  d_deleteContactPoint.addCallback(handle_DeleteContactPoint, this);
  d_currentContactPoint.addCallback(handle_currentContactPoint_change, this);
  d_addFreePoint.addCallback(handle_AddFreePoint, this);
  d_deleteFreePoint.addCallback(handle_DeleteFreePoint, this);
  d_currentFreePoint.addCallback(handle_currentFreePoint_change, this);
  d_updateSolution.addCallback(handle_updateSolution_change, this);

  d_drawSurface.addCallback(handle_drawSurface_change, this);
  d_drawSurfaceTexture.addCallback(handle_drawSurfaceTexture_change, this);
  d_liveSEMTexture.addCallback(handle_liveSEMTexture_change, this);
  d_textureOpacity.addCallback(handle_textureOpacity_change, this);
  d_ZScale.addCallback(handle_ZScale_change, this);

  d_semColormap.addCallback(handle_colormap_change, this);
  d_colorMapUI->addMinMaxCallback(handle_colormap_minmax_change, this);

  d_generateTestData.addCallback(handle_generateTestData_change, this);

  d_aligner->registerChangeHandler(this, correspondenceEditorChangeHandler);

  if (d_world) {
	d_world->TAddNode(&d_surfaceModelRenderer, "AFM-SEM Surface Model");
	d_world->TAddNode(&d_textureProjectionDirectionRenderer, 
                          "AFM-SEM Texture Projection Direction");
  }
  d_surfaceModelRenderer.SetProjTexture(&d_SEMTexture);
}

void nmui_AFM_SEM_CalibrationUI::setSPM(nmm_Microscope_Remote *scope) 
{
  if (d_AFM) {
    d_AFM->unregisterPointDataHandler(pointDataHandler, this);
	d_AFM->unregisterFinishedFreehandHandler(finishedFreehandHandler, this);
  }
  d_AFM = scope;
  if (d_AFM) {
    d_AFM->registerPointDataHandler(pointDataHandler, this);
	d_AFM->registerFinishedFreehandHandler(finishedFreehandHandler, this);
  }
}

void nmui_AFM_SEM_CalibrationUI::setSEM(nmm_Microscope_SEM_Remote *sem)
{
  if (d_SEM) {
    d_SEM->unregisterChangeHandler(this, semDataHandler);
    d_SEM->unregisterSynchHandler(semSynchHandler, this);
  }
  d_SEM = sem;
  if (d_SEM) {
    d_SEM->registerChangeHandler(this, semDataHandler);
    d_SEM->registerSynchHandler(semSynchHandler, this);
  }
}

void nmui_AFM_SEM_CalibrationUI::setTipRenderer(URender *renderer)
{
  d_tipRenderer = renderer;
  if (d_tipRenderer) {
	d_tipRenderer->SetProjTexture(&d_SEMTexture);
  }
}

void nmui_AFM_SEM_CalibrationUI::changeDataset(nmb_ImageManager *dataset)
{
  d_dataset = dataset;
	if (d_dataset && (vrpn_int32)d_windowOpen &&
		(vrpn_int32)(d_registrationMode) == 
		(vrpn_int32)(d_registrationMode_model_SEM)) {
		d_modelToSEM_modelImage = 
			d_dataset->dataImages()->getImageByName(
							d_modelToSEM_modelImageName.string());
		double xmin, ymin, xmax, ymax;
		d_modelToSEM_modelImage->getWorldRectangle(xmin, ymin, xmax, ymax);
		xmax -= xmin;
		ymax -= ymin;
		xmin = 0.0;
		ymin = 0.0;
		nmb_Image *scaledImage = new nmb_ImageGrid(d_modelToSEM_modelImage);
		scaledImage->normalize();
		int i,j;
		for (i = 0; i < scaledImage->width(); i++) {
			for (j = 0; j < scaledImage->height(); j++) {
				scaledImage->setValue(i, j, scaledImage->getValue(i,j)*d_ZScale);
			}
		}
		d_surfaceModelRenderer.setSurface(scaledImage, 
			xmin, ymin, xmax, ymax,
			d_surfaceStride);
		nmb_Image::deleteImage(scaledImage);

		d_modelToSEM_SEMImage = 
			d_dataset->dataImages()->getImageByName(
                            d_modelToSEM_SEMImageName.string());
		d_heightFieldNeedsUpdate = vrpn_TRUE;
		d_aligner->setImage(NMR_SOURCE, d_modelToSEM_modelImage, 
				vrpn_FALSE, vrpn_FALSE);
		d_aligner->setImage(NMR_TARGET, d_modelToSEM_SEMImage, 
				vrpn_FALSE, vrpn_FALSE);
		if (d_AFM && d_AFM->Data() && d_AFM->Data()->inputGrid) {
			double minX, minY, maxX, maxY;
			BCGrid *inputGrid = d_AFM->Data()->inputGrid;
			minX = inputGrid->minX();
			minY = inputGrid->minY();
			maxX = inputGrid->maxX();
			maxY = inputGrid->maxY();
			d_surfaceModelRenderer.setSurfaceRegion(minX, minY, maxX, maxY);
			printf("setting surfacemodel to match AFM region (%g,%g)-(%g,%g)\n",
				minX, minY, maxX, maxY);
		}
	}
}

int nmui_AFM_SEM_CalibrationUI::testImageWindowDisplayHandler
		(const ImageViewerDisplayData &data, void *ud)
{
    nmui_AFM_SEM_CalibrationUI *me = (nmui_AFM_SEM_CalibrationUI *)ud;
	int result = me->drawTestImages(data);
	return result;
}

int nmui_AFM_SEM_CalibrationUI::drawTestImages(const ImageViewerDisplayData &data)
{
	if (!d_testImageWinNeedsUpdate) {
		return 0;
	}
	d_testImageWinNeedsUpdate = false;
    ImageViewer *imageViewer = ImageViewer::getImageViewer();
	imageViewer->setGraphicsContext(data.winID);

	glDrawBuffer(GL_BACK);
	glViewport(0, 0, data.winWidth, data.winHeight);
	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, 1.0, 0.0, 1.0, -2.0, 2.0);

	// here we are pretending the model and afm surfaces are the 
	// same and just transform the model appropriately given that 
	// it is specified in model coordinates

	// URHeightField builds its display list for the
	// vlib context only so we can't use it for this
	// context
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glLoadMatrixd(d_SEMfromAFM_test);
	glMultMatrixd(d_AFMfromModel_test);

	double matrix[16];
	glGetDoublev(GL_MODELVIEW_MATRIX, matrix);
	printf("modelview (sem_from_model) matrix:\n");
	nmb_TransformMatrix44 temp;
	temp.setMatrix(matrix);
	temp.print();

	double minX, minY, maxX, maxY;
	d_modelToSEM_modelImage->getWorldRectangle(minX, minY, maxX, maxY);
	maxX -= minX;
	maxY -= minY;
	minX = 0;
	minY = 0;
	d_surfaceModelRenderer.renderWithoutDisplayList(
		d_modelToSEM_modelImage, minX, minY, maxX, maxY, 2);
	glPopMatrix();

	glReadBuffer(GL_BACK);
	nmb_Image *image = NULL;
	char imageName[128];
	sprintf(imageName, "simulated_SEM_for_%s", d_modelToSEM_modelImage->name()->c_str());
	image = new nmb_ImageArray(imageName, "units", data.winWidth, data.winHeight,
			NMB_FLOAT32);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_SKIP_PIXELS, image->borderXMin());
	glPixelStorei(GL_PACK_SKIP_ROWS, image->borderYMin());
	glPixelStorei(GL_PACK_ROW_LENGTH, 
		image->width() + image->borderXMin() + image->borderXMax());
	glReadPixels(0, 0, image->width(), image->height(), 
		GL_RED, GL_FLOAT, image->pixelData());
	d_dataset->dataImages()->removeImageByName(imageName);
	d_dataset->dataImages()->addImage(image);

	nmb_TransformMatrix44 SEM_from_AFM;
	SEM_from_AFM.setMatrix(d_SEMfromAFM_test);
	nmb_TransformMatrix44 AFM_from_model;
	AFM_from_model.setMatrix(d_AFMfromModel_test);

	double maxIntensity = image->maxValue();
	double minIntensity = image->minValue();
	double tipIntensity = 0.5*(maxIntensity + minIntensity);
	//printf("createTestImage: min = %g, max = %g\n",
	//	image->minValue(), image->maxValue());
	
	// now draw the SEM images for each of the tip positions
	// add the tip positions and the corresponding images as
	// correspondence points to
	// d_contact_AFMPoints, d_contactImages
	// d_free_AFMPoints, d_freeImages

	// pick contact points by specifying three points on the
	// model and transforming these into the afm and sem 
	// coordinate systems
	const int numCont = 3;
	const int numFree = 1;
	double model_cont_pnt[numCont][4] = {{0.3, 0.3, 0.0, 1.0},
											{0.7, 0.3, 0.0, 1.0},
											{0.5, 0.7, 0.0, 1.0}};
	double model_free_pnt[numFree][4] = {{0.5, 0.5, 0.0, 1.0}};
	double afm_cont_pnt[numCont][4], afm_free_pnt[numFree][4];
	double sem_cont_pnt[numCont][4], sem_free_pnt[numFree][4];

	int pntIndex;
	double i, j;
	double widthWorld = d_modelToSEM_modelImage->widthWorld();
	double heightWorld = d_modelToSEM_modelImage->heightWorld();
	for (pntIndex = 0; pntIndex < numCont; pntIndex++) {
		i = model_cont_pnt[pntIndex][0]*(double)(d_modelToSEM_modelImage->width());
		j = model_cont_pnt[pntIndex][1]*(double)(d_modelToSEM_modelImage->height());

		model_cont_pnt[pntIndex][0] *= widthWorld;
		model_cont_pnt[pntIndex][1] *= heightWorld;
		model_cont_pnt[pntIndex][2] = d_modelToSEM_modelImage->getValueInterpolated(i, j);
		AFM_from_model.transform(model_cont_pnt[pntIndex], afm_cont_pnt[pntIndex]);
		SEM_from_AFM.transform(afm_cont_pnt[pntIndex], sem_cont_pnt[pntIndex]);
	}
	for (pntIndex = 0; pntIndex < numFree; pntIndex++) {
		i = model_free_pnt[pntIndex][0]*(double)(d_modelToSEM_modelImage->width());
		j = model_free_pnt[pntIndex][1]*(double)(d_modelToSEM_modelImage->height());

		model_free_pnt[pntIndex][0] *= widthWorld;
		model_free_pnt[pntIndex][1] *= heightWorld;
		model_free_pnt[pntIndex][2] = d_modelToSEM_modelImage->getValueInterpolated(i, j);
		model_free_pnt[pntIndex][2] += 0.2*widthWorld;
		AFM_from_model.transform(model_free_pnt[pntIndex], afm_free_pnt[pntIndex]);
		SEM_from_AFM.transform(afm_free_pnt[pntIndex], sem_free_pnt[pntIndex]);
	}

	for (pntIndex = 0; pntIndex < numCont; pntIndex++) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		glLoadMatrixd(d_SEMfromAFM_test);
		glMultMatrixd(d_AFMfromModel_test);
		d_surfaceModelRenderer.renderWithoutDisplayList(
			d_modelToSEM_modelImage, minX, minY, maxX, maxY, 2);
		glPopMatrix();

		glColor4f(tipIntensity, 1.0, 1.0, 1.0);
		glBegin(GL_TRIANGLES);
		glVertex3d(sem_cont_pnt[pntIndex][0], sem_cont_pnt[pntIndex][1], sem_cont_pnt[pntIndex][2]);
		glVertex3d(sem_cont_pnt[pntIndex][0]-0.3, sem_cont_pnt[pntIndex][1]+1.0, sem_cont_pnt[pntIndex][2]);
		glVertex3d(sem_cont_pnt[pntIndex][0]+0.3, sem_cont_pnt[pntIndex][1]+1.0, sem_cont_pnt[pntIndex][2]);
		glEnd();
		char imageName[128];
		sprintf(imageName, "simulated_SEM_for_contactPoint%d", pntIndex);
		image = new nmb_ImageArray(imageName, "units", data.winWidth, data.winHeight,
				NMB_FLOAT32);
		glReadPixels(0, 0, image->width(), image->height(), 
			GL_RED, GL_FLOAT, image->pixelData());
		addContactPoint(afm_cont_pnt[pntIndex], sem_cont_pnt[pntIndex], image);
	}

	for (pntIndex = 0; pntIndex < numFree; pntIndex++) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		glLoadMatrixd(d_SEMfromAFM_test);
		glMultMatrixd(d_AFMfromModel_test);
		d_surfaceModelRenderer.renderWithoutDisplayList(
			d_modelToSEM_modelImage, minX, minY, maxX, maxY, 2);
		glPopMatrix();

		glColor4f(tipIntensity, 1.0, 1.0, 1.0);
		glBegin(GL_TRIANGLES);
		glVertex3d(sem_free_pnt[pntIndex][0], sem_free_pnt[pntIndex][1], sem_free_pnt[pntIndex][2]);
		glVertex3d(sem_free_pnt[pntIndex][0]-0.3, sem_free_pnt[pntIndex][1]+1.0, sem_free_pnt[pntIndex][2]);
		glVertex3d(sem_free_pnt[pntIndex][0]+0.3, sem_free_pnt[pntIndex][1]+1.0, sem_free_pnt[pntIndex][2]);
		glEnd();
		char imageName[128];
		sprintf(imageName, "simulated_SEM_for_freePoint%d", pntIndex);
		image = new nmb_ImageArray(imageName, "units", data.winWidth, data.winHeight,
				NMB_FLOAT32);
		glReadPixels(0, 0, image->width(), image->height(), 
			GL_RED, GL_FLOAT, image->pixelData());
		addFreePoint(afm_free_pnt[pntIndex], sem_free_pnt[pntIndex], image);
	}

	return 0;
}

void nmui_AFM_SEM_CalibrationUI::createTestImages()
{
	clearFreePoints();
	clearContactPoints();

	d_testImageWinNeedsUpdate = true;
	ImageViewer *imageViewer = ImageViewer::getImageViewer();
	if (d_testImageWinCreated) {
		imageViewer->dirtyWindow(d_testImageWinID);
		return;
	}
	/* 
	Creates a separate graphics context be creating a new window
	and draws everything into that so we don't need to worry about
	GL state in windows being used for other stuff. 
	The window is destroyed at the end after the pixels have been 
	transferred to main memory.
	*/
    char *display_name;
    display_name = (char *)getenv("V_X_DISPLAY");
    if (!display_name) {
       display_name = (char *)getenv("DISPLAY");
       if (!display_name) {
          display_name = "unix:0";
       }
    }
	d_testImageWinID = 
		imageViewer->createWindow(display_name, 10, 10, 
		500, 500, "modelRender");


	imageViewer->setWindowDisplayHandler(d_testImageWinID, 
		testImageWindowDisplayHandler, this);


	imageViewer->showWindow(d_testImageWinID);
	d_testImageWinCreated = true;
}

void nmui_AFM_SEM_CalibrationUI::addContactPoint(
	double *afm_pnt, double *sem_pnt, nmb_Image *semImage)
{
	char selectorEntry[128];
	int k;
	if (afm_pnt) {
		for (k = 0; k < 3; k++) {
			d_contact_AFMPoints[d_numContactPoints][k] = afm_pnt[k];
		}
		d_contact_AFMPoints[d_numContactPoints][3] = 1.0;
	}
	if (sem_pnt) {
		for (k = 0; k < 3; k++) {
			d_contact_SEMPoints[d_numContactPoints][k] = sem_pnt[k];
		}
		d_contact_SEMPoints[d_numContactPoints][3] = 1.0;
	}

	d_contactImages[d_numContactPoints] = semImage;
	sprintf(selectorEntry, "%d", d_numContactPoints);
	d_numContactPoints++;
	if (d_numContactPoints == 1) {
		d_contactPointList.clearList();
	}
	d_contactPointList.addEntry(selectorEntry);
	d_currentContactPoint.Set(selectorEntry);
	updateInputStatus();
}

void nmui_AFM_SEM_CalibrationUI::addFreePoint(
	double *afm_pnt, double *sem_pnt, nmb_Image *semImage)
{
	char selectorEntry[128];
	int k;
	if (afm_pnt) {
		for (k = 0; k < 3; k++) {
			d_free_AFMPoints[d_numFreePoints][k] = afm_pnt[k];
		}
		d_free_AFMPoints[d_numFreePoints][3] = 1.0;
	}
	if (sem_pnt) {
		for (k = 0; k < 3; k++) {
			d_free_SEMPoints[d_numFreePoints][k] = sem_pnt[k];
		}
		d_free_SEMPoints[d_numFreePoints][3] = 1.0;
	}

	d_freeImages[d_numFreePoints] = semImage;
	sprintf(selectorEntry, "%d", d_numFreePoints);
	d_numFreePoints++;
	if (d_numFreePoints == 1) {
		d_freePointList.clearList();
	}
	d_freePointList.addEntry(selectorEntry);
	d_currentFreePoint.Set(selectorEntry);
	updateInputStatus();
}

void nmui_AFM_SEM_CalibrationUI::clearContactPoints()
{
	int i;
	for (i = 0; i < d_numContactPoints; i++) {
		nmb_Image::deleteImage(d_contactImages[i]);
		d_contactImages[i] = 0;
	}
	d_numContactPoints = 0;
	d_contactPointList.clearList();
}

void nmui_AFM_SEM_CalibrationUI::clearFreePoints()
{
	int i;
	for (i = 0; i < d_numFreePoints; i++) {
		nmb_Image::deleteImage(d_freeImages[i]);
		d_freeImages[i] = 0;
	}
	d_numFreePoints = 0;
	d_freePointList.clearList();
}

//static 
void nmui_AFM_SEM_CalibrationUI::correspondenceEditorChangeHandler(void *ud,
                  const nmr_ProxyChangeHandlerData &info)
{
	int i;
	nmui_AFM_SEM_CalibrationUI *me = (nmui_AFM_SEM_CalibrationUI *)ud;
	if (info.msg_type == NMR_FIDUCIAL) {
		vrpn_int32 replace, num;
		vrpn_float32 x_src[NMR_MAX_FIDUCIAL], y_src[NMR_MAX_FIDUCIAL],
                 z_src[NMR_MAX_FIDUCIAL];
		vrpn_float32 x_tgt[NMR_MAX_FIDUCIAL], y_tgt[NMR_MAX_FIDUCIAL],
                 z_tgt[NMR_MAX_FIDUCIAL];
		info.aligner->getFiducial(replace, num, x_src, y_src, z_src, 
									x_tgt, y_tgt, z_tgt);
		if ((vrpn_int32)(me->d_registrationMode) == 
			(vrpn_int32)(me->d_registrationMode_AFM_SEM_contact)) {
			if (me->d_numContactPoints > 0 && num == 1) {
				int index = atoi(me->d_currentContactPoint.string());
				me->d_contact_SEMPoints[index][0] = x_tgt[0];
				me->d_contact_SEMPoints[index][1] = y_tgt[0];
				me->d_contact_SEMPoints[index][2] = z_tgt[0];
				printf("received fiducial for contact point\n");
			} else if (!(me->d_numContactPoints == 0 && num == 0)){
				printf("this shouldn't happen\n");
			}
		} else if ((vrpn_int32)(me->d_registrationMode) == 
			(vrpn_int32)(me->d_registrationMode_AFM_SEM_free)) {
			if (me->d_numFreePoints > 0 && num == 1) {
				int index = atoi(me->d_currentFreePoint.string());
				me->d_free_SEMPoints[index][0] = x_tgt[0];
				me->d_free_SEMPoints[index][1] = y_tgt[0];
				me->d_free_SEMPoints[index][2] = z_tgt[0];
				printf("received fiducial for free point\n");
			} else if (!(me->d_numFreePoints == 0 && num == 0)){
				printf("this shouldn't happen\n");
			}
		} else if ((vrpn_int32)(me->d_registrationMode) == 
			(vrpn_int32)(me->d_registrationMode_model_SEM)) {
			me->d_numModelSEMPoints = num;
			double widthWorld = 1, heightWorld = 1;
			int numX = 0, numY = 0;
			if (me->d_modelToSEM_modelImage) {
				widthWorld = me->d_modelToSEM_modelImage->widthWorld();
				heightWorld = me->d_modelToSEM_modelImage->heightWorld();
				numX = me->d_modelToSEM_modelImage->width();
				numY = me->d_modelToSEM_modelImage->height();
			}
			for (i = 0; i < num; i++) {
				me->d_modelToSEM_modelPoints[i][0] = x_src[i]*widthWorld;
				me->d_modelToSEM_modelPoints[i][1] = y_src[i]*heightWorld;
				if (me->d_modelToSEM_modelImage) {
					me->d_modelToSEM_modelPoints[i][2] = 
						me->d_modelToSEM_modelImage->getValueInterpolated(
						(double)(x_src[i]*(double)numX),
						(double)(y_src[i]*(double)numY));
				} else {
					me->d_modelToSEM_modelPoints[i][2] = z_src[i];
				}
				me->d_modelToSEM_SEMPoints[i][0] = x_tgt[i];
				me->d_modelToSEM_SEMPoints[i][1] = y_tgt[i];
				me->d_modelToSEM_SEMPoints[i][2] = z_tgt[i];
			}
			printf("received fiducials for model-SEM points\n");
		}
		me->updateInputStatus();
	}
}

void nmui_AFM_SEM_CalibrationUI::handle_windowOpen_change(
			 vrpn_int32 value, void *ud)
{
	nmui_AFM_SEM_CalibrationUI *me = (nmui_AFM_SEM_CalibrationUI *)ud;
    if (value) {
		// tell the aligner not to send registration results - we will do the
		// necessary computation on the client side using the fiducials
		// selected by the user and sent back to us from the server
		me->d_aligner->enableAutoUpdate(vrpn_FALSE);
		handle_registrationMode_change((vrpn_int32)(me->d_registrationMode),ud);
		if (me->d_modelNeedsUpdate) {
			nmui_AFM_SEM_CalibrationUI::handle_updateModel_change(1, me);
		}
		if (me->d_SEMNeedsUpdate) {
			nmui_AFM_SEM_CalibrationUI::handle_updateSEMImage_change(1, me);
		}
	} else {
		me->d_aligner->setGUIEnable(vrpn_FALSE, NMR_ALLWINDOWS);
		me->d_aligner->setEditEnable(vrpn_TRUE, vrpn_TRUE);
		// tell the aligner to send registration results - this should be the
		// default behavior because its expected by the normal 
        // registration code
		me->d_aligner->enableAutoUpdate(vrpn_TRUE);
	}
}

// radio button - three modes: model_SEM, AFM_SEM_contact, AFM_SEM_free
// static 
void nmui_AFM_SEM_CalibrationUI::handle_registrationMode_change(
			 vrpn_int32 value, void *ud)
{
	nmb_Image *currentImage = NULL;
	int pointIndex;
    nmui_AFM_SEM_CalibrationUI *me = (nmui_AFM_SEM_CalibrationUI *)ud;
	if (!(vrpn_int32)(me->d_windowOpen)) {
		return;
	}
	if (value == (int)(me->d_registrationMode_model_SEM)) {
		printf("changed to model-SEM mode\n");
		// send the images for model<-->SEM
		if (me->d_dataset) {
			me->d_modelToSEM_modelImage = me->d_dataset->dataImages()->
				getImageByName(me->d_modelToSEM_modelImageName.string());
			me->d_modelToSEM_SEMImage = me->d_dataset->dataImages()->
				getImageByName(me->d_modelToSEM_SEMImageName.string());
			me->d_heightFieldNeedsUpdate = vrpn_TRUE;
		}
		me->d_aligner->setImage(NMR_SOURCE, me->d_modelToSEM_modelImage, 
			vrpn_FALSE, vrpn_FALSE);
		me->d_aligner->setImage(NMR_TARGET, me->d_modelToSEM_SEMImage, 
			vrpn_FALSE, vrpn_FALSE);
		me->setFiducial();
		me->d_aligner->setGUIEnable(vrpn_TRUE, NMR_ALLWINDOWS);
		me->d_aligner->setEditEnable(vrpn_TRUE, vrpn_TRUE);
	} else if (value == (int)(me->d_registrationMode_AFM_SEM_contact)) {
		printf("changed to contact mode\n");
		if (me->d_numContactPoints > 0) {
			pointIndex = atoi(me->d_currentContactPoint);
			currentImage = me->d_contactImages[pointIndex];
			me->d_aligner->setImage(NMR_TARGET, currentImage, 
				vrpn_FALSE, vrpn_FALSE);
		} else {
			me->d_aligner->setImage(NMR_TARGET, NULL, vrpn_FALSE, vrpn_FALSE);
		}
		me->setFiducial();
		me->d_aligner->setGUIEnable(vrpn_FALSE, NMR_SOURCEWINDOW);
		me->d_aligner->setGUIEnable(vrpn_TRUE, NMR_TARGETWINDOW);
		me->d_aligner->setEditEnable(vrpn_FALSE, vrpn_TRUE);
	} else if (value == (int)(me->d_registrationMode_AFM_SEM_free)) {
		printf("changed to free mode\n");
		if (me->d_numFreePoints > 0) {
			pointIndex = atoi(me->d_currentFreePoint);
			currentImage = me->d_freeImages[pointIndex];
			me->d_aligner->setImage(NMR_TARGET, currentImage, 
				vrpn_FALSE, vrpn_FALSE);
		} else {
			me->d_aligner->setImage(NMR_TARGET, NULL, vrpn_FALSE, vrpn_FALSE);
		}
		me->setFiducial();
		me->d_aligner->setGUIEnable(vrpn_FALSE, NMR_SOURCEWINDOW);
		me->d_aligner->setGUIEnable(vrpn_TRUE, NMR_TARGETWINDOW);
		me->d_aligner->setEditEnable(vrpn_FALSE, vrpn_TRUE);
	} else {
		fprintf(stderr, "Error: unknown registration mode\n");
	}
	fprintf(stderr, "registrationMode: %d\n", value);
}

void nmui_AFM_SEM_CalibrationUI::setFiducial()
{
	if ((vrpn_int32)(d_registrationMode) == 
		(vrpn_int32)(d_registrationMode_AFM_SEM_contact)) {
		if (d_numContactPoints > 0) {
			int index = atoi(d_currentContactPoint.string());
			printf("sending 1 contact fiducial for point %d\n",
				index);
			d_aligner->setFiducial(vrpn_TRUE, 1, 
				&d_contact_AFMPoints[index][0],
				&d_contact_AFMPoints[index][1], 
				&d_contact_AFMPoints[index][2],
				&d_contact_SEMPoints[index][0], 
				&d_contact_SEMPoints[index][1], 
				&d_contact_SEMPoints[index][2]);
		} else {
			printf("sending 0 contact fiducial\n");
			d_aligner->setFiducial(vrpn_TRUE, 0, NULL, NULL, NULL,
				NULL, NULL, NULL);
		}
	} else if ((vrpn_int32)(d_registrationMode) == 
		(vrpn_int32)(d_registrationMode_AFM_SEM_free)) {
		printf("sending free fiducial\n");
		if (d_numFreePoints > 0) {
			int index = atoi(d_currentFreePoint.string());
			d_aligner->setFiducial(vrpn_TRUE, 1, 
				&d_free_AFMPoints[index][0],
				&d_free_AFMPoints[index][1], 
				&d_free_AFMPoints[index][2],
				&d_free_SEMPoints[index][0], 
				&d_free_SEMPoints[index][1], 
				&d_free_SEMPoints[index][2]);
		} else {
			d_aligner->setFiducial(vrpn_TRUE, 0, NULL, NULL, NULL,
				NULL, NULL, NULL);
		}
	} else if ((vrpn_int32)(d_registrationMode) == 
		(vrpn_int32)(d_registrationMode_model_SEM)) {
		printf("sending model-SEM fiducials\n");
		int num = d_numModelSEMPoints;
		int i;
		vrpn_float32 x_src[NMR_MAX_FIDUCIAL], y_src[NMR_MAX_FIDUCIAL],
                 z_src[NMR_MAX_FIDUCIAL];
		vrpn_float32 x_tgt[NMR_MAX_FIDUCIAL], y_tgt[NMR_MAX_FIDUCIAL],
                 z_tgt[NMR_MAX_FIDUCIAL];
		double widthWorld = 1, heightWorld = 1;
		if (d_modelToSEM_modelImage) {
			widthWorld = d_modelToSEM_modelImage->widthWorld();
			heightWorld = d_modelToSEM_modelImage->heightWorld();
		}
		for (i = 0; i < num; i++) {
			x_src[i] = d_modelToSEM_modelPoints[i][0]/widthWorld;
			y_src[i] = d_modelToSEM_modelPoints[i][1]/heightWorld;
			z_src[i] = d_modelToSEM_modelPoints[i][2];
			x_tgt[i] = d_modelToSEM_SEMPoints[i][0];
			y_tgt[i] = d_modelToSEM_SEMPoints[i][1];
			z_tgt[i] = d_modelToSEM_SEMPoints[i][2];
		}
		d_aligner->setFiducial(vrpn_TRUE, num, 
			x_src, y_src, z_src,
			x_tgt, y_tgt, z_tgt);
	}
}

// model_SEM selectors
// static 
void nmui_AFM_SEM_CalibrationUI::handle_modelToSEM_model_change(
			const char *name, void *ud)
{
	nmui_AFM_SEM_CalibrationUI *me = (nmui_AFM_SEM_CalibrationUI *)ud;
	if (!(vrpn_int32)(me->d_windowOpen)) {
		me->d_modelNeedsUpdate = true;
		return;
	}
	printf("modelToSEM_model: %s\n", name);
	if (me->d_dataset) {
		me->d_modelToSEM_modelImage = 
			me->d_dataset->dataImages()->getImageByName(name);
		if (!me->d_modelToSEM_modelImage) {
			fprintf(stderr, "nmui_AFM_SEM_CalibrationUI::handle_modelToSEM_model_change: \n"
				"image not found: %s\n", name);
			me->d_modelNeedsUpdate = true;
			return;
		}
		double modelMinX, modelMinY, modelMaxX, modelMaxY;
		me->d_modelToSEM_modelImage->getWorldRectangle(
			modelMinX, modelMinY, modelMaxX, modelMaxY);
		modelMaxX -= modelMinX;
		modelMaxY -= modelMinY;
		modelMinX = 0.0;
		modelMinY = 0.0;

		nmb_Image *scaledImage = new nmb_ImageGrid(me->d_modelToSEM_modelImage);
		scaledImage->normalize();
		int i,j;
		for (i = 0; i < scaledImage->width(); i++) {
			for (j = 0; j < scaledImage->height(); j++) {
				scaledImage->setValue(i, j, scaledImage->getValue(i,j)*me->d_ZScale);
			}
		}
		
		me->d_surfaceModelRenderer.setSurface(scaledImage,
			modelMinX, modelMinY, modelMaxX, modelMaxY,
			me->d_surfaceStride);
		nmb_Image::deleteImage(scaledImage);
		// find out where the surface model renderer will put the surface
		// and scale and translate this so it fits where the AFM thinks the
		// scan range is and store this transformation in d_AFMfromModel
		if (me->d_AFM && me->d_AFM->Data() && me->d_AFM->Data()->inputGrid) {
			double minX, minY, maxX, maxY;
			BCGrid *inputGrid = me->d_AFM->Data()->inputGrid;
			minX = inputGrid->minX();
			minY = inputGrid->minY();
			maxX = inputGrid->maxX();
			maxY = inputGrid->maxY();
			me->d_surfaceModelRenderer.setSurfaceRegion(minX, minY, maxX, maxY);
		}
	}
	
	if ((vrpn_int32)(me->d_registrationMode) == 
		(vrpn_int32)(me->d_registrationMode_model_SEM) &&
		(vrpn_int32)(me->d_windowOpen)) {
		me->d_aligner->setImage(NMR_SOURCE, me->d_modelToSEM_modelImage, 
				vrpn_FALSE, vrpn_FALSE);
	}
	me->updateInputStatus();
}

// static
void nmui_AFM_SEM_CalibrationUI::handle_updateModel_change(
			vrpn_int32 value, void *ud)
{
	nmui_AFM_SEM_CalibrationUI *me = (nmui_AFM_SEM_CalibrationUI *)ud;
	if (!value) {
		return;
	}
	printf("modelToSEM_model: %s\n", 
		me->d_modelToSEM_modelImageName.string());
	if (me->d_dataset) {
		me->d_modelToSEM_modelImage = me->d_dataset->dataImages()->
			getImageByName(me->d_modelToSEM_modelImageName.string());
		if (!me->d_modelToSEM_modelImage) {
			fprintf(stderr, "nmui_AFM_SEM_CalibrationUI::handle_modelToSEM_model_change: \n"
				"image not found: %s\n", 
				me->d_modelToSEM_modelImageName.string());
			return;
		}
		double modelMinX, modelMinY, modelMaxX, modelMaxY;
		me->d_modelToSEM_modelImage->getWorldRectangle(
			modelMinX, modelMinY, modelMaxX, modelMaxY);
		modelMaxX -= modelMinX;
		modelMaxY -= modelMinY;
		modelMinX = 0;
		modelMinY = 0;
		nmb_Image *scaledImage = new nmb_ImageGrid(me->d_modelToSEM_modelImage);
		scaledImage->normalize();
		int i,j;
		for (i = 0; i < scaledImage->width(); i++) {
			for (j = 0; j < scaledImage->height(); j++) {
				scaledImage->setValue(i, j, scaledImage->getValue(i,j)*me->d_ZScale);
			}
		}

		me->d_surfaceModelRenderer.setSurface(scaledImage,
			modelMinX, modelMinY, modelMaxX, modelMaxY,
			me->d_surfaceStride);
		nmb_Image::deleteImage(scaledImage);

		// find out where the surface model renderer will put the surface
		// and scale and translate this so it fits where the AFM thinks the
		// scan range is and store this transformation in d_AFMfromModel
		if (me->d_AFM && me->d_AFM->Data() && me->d_AFM->Data()->inputGrid) {
			double minX, minY, maxX, maxY;
			BCGrid *inputGrid = me->d_AFM->Data()->inputGrid;
			minX = inputGrid->minX();
			minY = inputGrid->minY();
			maxX = inputGrid->maxX();
			maxY = inputGrid->maxY();
			me->d_surfaceModelRenderer.setSurfaceRegion(minX, minY, maxX, maxY);
		}
	}
	
	if ((vrpn_int32)(me->d_registrationMode) == 
		(vrpn_int32)(me->d_registrationMode_model_SEM) &&
		(vrpn_int32)(me->d_windowOpen)) {
		me->d_aligner->setImage(NMR_SOURCE, me->d_modelToSEM_modelImage, 
				vrpn_FALSE, vrpn_FALSE);
	}
	me->updateInputStatus();
	me->d_modelNeedsUpdate = false;
}

// static 
void nmui_AFM_SEM_CalibrationUI::handle_modelToSEM_SEMImage_change(
			const char *name, void *ud)
{
	nmui_AFM_SEM_CalibrationUI *me = (nmui_AFM_SEM_CalibrationUI *)ud;
	if (!(vrpn_int32)(me->d_windowOpen)) {
		me->d_SEMNeedsUpdate = true;
		return;
	}
	printf("modelToSEM_SEMImage: %s\n", name);
	if (me->d_dataset) {
		me->d_modelToSEM_SEMImage = 
			me->d_dataset->dataImages()->getImageByName(name);
		if (!me->d_modelToSEM_modelImage) {
			fprintf(stderr, "nmui_AFM_SEM_CalibrationUI::handle_modelToSEM_SEMImage_change: \n"
				"image not found: %s\n", name);
			me->d_SEMNeedsUpdate = true;
			return;
		}
		me->d_heightFieldNeedsUpdate = vrpn_TRUE;
	}
	if ((vrpn_int32)(me->d_registrationMode) == 
		(vrpn_int32)(me->d_registrationMode_model_SEM) &&
		(vrpn_int32)(me->d_windowOpen)) {
		me->d_aligner->setImage(NMR_TARGET, me->d_modelToSEM_SEMImage, 
					vrpn_FALSE, vrpn_FALSE);
	}
	me->d_SEMTexture.setImage(me->d_modelToSEM_SEMImage);
	me->updateInputStatus();
}

// static
void nmui_AFM_SEM_CalibrationUI::handle_updateSEMImage_change(
			vrpn_int32 value, void *ud)
{
	nmui_AFM_SEM_CalibrationUI *me = (nmui_AFM_SEM_CalibrationUI *)ud;
	if (!value) {
		return;
	}
	printf("modelToSEM_SEMImage: %s\n", 
		me->d_modelToSEM_SEMImageName.string());
	if (me->d_dataset) {
		me->d_modelToSEM_SEMImage = me->d_dataset->dataImages()->
			getImageByName(me->d_modelToSEM_SEMImageName.string());
		if (!me->d_modelToSEM_modelImage) {
			fprintf(stderr, "nmui_AFM_SEM_CalibrationUI::handle_modelToSEM_SEMImage_change: \n"
				"image not found: %s\n", 
				me->d_modelToSEM_SEMImageName.string());
			return;
		}
		me->d_heightFieldNeedsUpdate = vrpn_TRUE;
	}
	if ((vrpn_int32)(me->d_registrationMode) == 
		(vrpn_int32)(me->d_registrationMode_model_SEM) &&
		(vrpn_int32)(me->d_windowOpen)) {
		me->d_aligner->setImage(NMR_TARGET, me->d_modelToSEM_SEMImage, 
					vrpn_FALSE, vrpn_FALSE);
	}
	me->d_SEMTexture.setImage(me->d_modelToSEM_SEMImage);
	me->updateInputStatus();
	me->d_SEMNeedsUpdate = false;
}

// AFM_SEM_contact
// buttons
// static 
void nmui_AFM_SEM_CalibrationUI::handle_AddContactPoint(
			vrpn_int32 value, void *ud)
{
	nmui_AFM_SEM_CalibrationUI *me = (nmui_AFM_SEM_CalibrationUI *)ud;
	// switch mode
	me->addLatestDataAsContactPoint();
}

void nmui_AFM_SEM_CalibrationUI::addLatestDataAsContactPoint()
{
	if (!d_SEM) {
		display_error_dialog("Add Contact Point: No SEM connected");
		return;
	}
	nmb_ImageArray *image;
	d_SEM->getImageData(&image);
	if (!image) {
		display_error_dialog("Add Contact Point: No SEM Data");
		return;
	}
	addContactPoint(d_tipPosition, NULL, new nmb_ImageArray(image));
	d_tipPositionAcquiredSinceLastAdd = vrpn_FALSE;
	d_semImageAcquiredSinceLastAdd = vrpn_FALSE;
	updateInputStatus();	
}

// static 
void nmui_AFM_SEM_CalibrationUI::handle_DeleteContactPoint(
		vrpn_int32 value, void *ud)
{
	int i;
	nmui_AFM_SEM_CalibrationUI *me = (nmui_AFM_SEM_CalibrationUI *)ud;
	printf("deleteContactPoint\n");
	int index = 0;
	int replacementIndex = me->d_numContactPoints-1;
	if (me->d_numContactPoints > 0) {
		index = atoi(me->d_currentContactPoint.string());
		nmb_Image::deleteImage(me->d_contactImages[index]);
		if (index != me->d_numContactPoints-1) {
			me->d_contactImages[index] = me->d_contactImages[replacementIndex];
			for (i = 0; i < 4; i++) {
				me->d_contact_AFMPoints[index][i] = 
					me->d_contact_AFMPoints[replacementIndex][i];
				me->d_contact_SEMPoints[index][i] = 
					me->d_contact_SEMPoints[replacementIndex][i];
			}
		}
		me->d_numContactPoints--;
		char selectorEntry[128];
		me->d_contactPointList.clearList();
		if (me->d_numContactPoints > 0) {
			for (i = 0; i < me->d_numContactPoints; i++) {
				sprintf(selectorEntry, "%d", i);
				me->d_contactPointList.addEntry(selectorEntry);
			}
		} else {
			me->d_contactPointList.addEntry("none");
		}
	}
	me->updateInputStatus();
}

// point selector
// static 
void nmui_AFM_SEM_CalibrationUI::handle_currentContactPoint_change(
		const char *name, void *ud)
{
	nmui_AFM_SEM_CalibrationUI *me = (nmui_AFM_SEM_CalibrationUI *)ud;
	printf("currentContactPoint: %s\n", name);

	if ((vrpn_int32)(me->d_registrationMode) == 
			(vrpn_int32)(me->d_registrationMode_AFM_SEM_contact)) {
		if (me->d_numContactPoints == 0) {
			me->d_aligner->setImage(NMR_TARGET, NULL, vrpn_FALSE, vrpn_FALSE);
			me->d_aligner->setFiducial(vrpn_TRUE, 0, NULL, NULL, NULL,
				NULL, NULL, NULL);
		} else {
			int index = atoi(name);

			printf("sending image and fiducial for contact point %d\n",index);
			me->d_aligner->setImage(NMR_TARGET, 
              me->d_contactImages[index],vrpn_FALSE, vrpn_FALSE);
			  me->d_aligner->setFiducial(vrpn_TRUE, 1, 
				&me->d_contact_AFMPoints[index][0],
				&me->d_contact_AFMPoints[index][1], 
				&me->d_contact_AFMPoints[index][2],
				&me->d_contact_SEMPoints[index][0], 
				&me->d_contact_SEMPoints[index][1], 
				&me->d_contact_SEMPoints[index][2]);
		}
	}
}

// AFM_SEM_free
// buttons
// static 
void nmui_AFM_SEM_CalibrationUI::handle_AddFreePoint(
		vrpn_int32 value, void *ud)
{
	nmui_AFM_SEM_CalibrationUI *me = (nmui_AFM_SEM_CalibrationUI *)ud;
	// switch mode
	me->addLatestDataAsFreePoint();
}

void nmui_AFM_SEM_CalibrationUI::addLatestDataAsFreePoint()
{
	if (!d_SEM) {
		display_error_dialog("Add Free Point: No SEM connected");
		return;
	}
	nmb_ImageArray *image;
	d_SEM->getImageData(&image);
	if (!image) {
		display_error_dialog("Add Free Point: No SEM Data");
		return;
	}
	addFreePoint(d_tipPosition, NULL, new nmb_ImageArray(image));
	d_tipPositionAcquiredSinceLastAdd = vrpn_FALSE;
	d_semImageAcquiredSinceLastAdd = vrpn_FALSE;
	updateInputStatus();	
}

// static 
void nmui_AFM_SEM_CalibrationUI::handle_DeleteFreePoint(
	vrpn_int32 value, void *ud)
{
	int i;
	nmui_AFM_SEM_CalibrationUI *me = (nmui_AFM_SEM_CalibrationUI *)ud;
	printf("deleteFreePoint\n");
	int index = 0;
	int replacementIndex = me->d_numFreePoints-1;
	if (me->d_numFreePoints > 0) {
		index = atoi(me->d_currentFreePoint.string());
		nmb_Image::deleteImage(me->d_freeImages[index]);
		if (index != me->d_numFreePoints-1) {
			me->d_freeImages[index] = me->d_freeImages[replacementIndex];
			for (i = 0; i < 4; i++) {
				me->d_free_AFMPoints[index][i] = 
					me->d_free_AFMPoints[replacementIndex][i];
				me->d_free_SEMPoints[index][i] = 
					me->d_free_SEMPoints[replacementIndex][i];
			}
		}
		me->d_numFreePoints--;
		char selectorEntry[128];
		me->d_freePointList.clearList();
		if (me->d_numFreePoints > 0) {
			for (i = 0; i < me->d_numFreePoints; i++) {
				sprintf(selectorEntry, "%d", i);
				me->d_freePointList.addEntry(selectorEntry);
			}
		} else {
			me->d_freePointList.addEntry("none");
		}
	}
	me->updateInputStatus();
}

// point selector
// static 
void nmui_AFM_SEM_CalibrationUI::handle_currentFreePoint_change(
	const char *name, void *ud)
{
	nmui_AFM_SEM_CalibrationUI *me = (nmui_AFM_SEM_CalibrationUI *)ud;
	printf("currentFreePoint: %s\n", name);

	if ((vrpn_int32)(me->d_registrationMode) == 
			(vrpn_int32)(me->d_registrationMode_AFM_SEM_free)) {
		if (me->d_numFreePoints == 0) {
			me->d_aligner->setImage(NMR_TARGET, NULL, vrpn_FALSE, vrpn_FALSE);
			me->d_aligner->setFiducial(vrpn_TRUE, 0, NULL, NULL, NULL,
				NULL, NULL, NULL);
		} else {
			int index = atoi(name);

			printf("sending image and fiducial for free point %d\n",index);
			me->d_aligner->setImage(NMR_TARGET, 
            me->d_freeImages[index],vrpn_FALSE, vrpn_FALSE);
			me->d_aligner->setFiducial(vrpn_TRUE, 1, 
				&me->d_free_AFMPoints[index][0],
				&me->d_free_AFMPoints[index][1], 
            	&me->d_free_AFMPoints[index][2],
				&me->d_free_SEMPoints[index][0], 
				&me->d_free_SEMPoints[index][1], 
				&me->d_free_SEMPoints[index][2]);
		}
	}
}

void nmui_AFM_SEM_CalibrationUI::handle_updateSolution_change(
		vrpn_int32 value, void *ud)
{
	nmui_AFM_SEM_CalibrationUI *me = (nmui_AFM_SEM_CalibrationUI *)ud;
	if (value) {
		me->updateSolution();
	}
}

void nmui_AFM_SEM_CalibrationUI::handle_drawSurface_change(
		vrpn_int32 value, void *ud)
{
	nmui_AFM_SEM_CalibrationUI *me = (nmui_AFM_SEM_CalibrationUI *)ud;
	bool enable = (value != 0);
	me->d_surfaceModelRenderer.SetVisibility(enable);
	me->d_textureProjectionDirectionRenderer.SetVisibility(enable);
}

void nmui_AFM_SEM_CalibrationUI::handle_drawSurfaceTexture_change(
		vrpn_int32 value, void *ud)
{
	nmui_AFM_SEM_CalibrationUI *me = (nmui_AFM_SEM_CalibrationUI *)ud;
	bool enable = (value != 0);
	me->d_surfaceModelRenderer.setTextureEnable(enable);
	if ((int)(me->d_liveSEMTexture) && me->d_SEM) {
		me->d_SEMTexture.doFastUpdates(true);
		nmb_ImageArray *image;
		me->d_SEM->getImageData(&image);
		if (image) {
			me->d_SEMTexture.setImage(image);
		}
	} else {
		me->d_SEMTexture.doFastUpdates(false);
		me->d_SEMTexture.setImage(me->d_modelToSEM_SEMImage);
	}
}

void nmui_AFM_SEM_CalibrationUI::handle_liveSEMTexture_change(
		vrpn_int32 value, void *ud)
{
	nmui_AFM_SEM_CalibrationUI *me = (nmui_AFM_SEM_CalibrationUI *)ud;
	bool enable = (value != 0);
	if(me->d_SEM) {
		if (enable) {
			me->d_SEMTexture.doFastUpdates(true);
			nmb_ImageArray *image;
			me->d_SEM->getImageData(&image);
			if (image) {
				me->d_SEMTexture.setImage(image);
			}
		} else {
			me->d_SEMTexture.doFastUpdates(false);
			me->d_SEMTexture.setImage(me->d_modelToSEM_SEMImage);
		}
	}
}

void nmui_AFM_SEM_CalibrationUI::handle_textureOpacity_change(
		vrpn_float64 value, void *ud)
{
	nmui_AFM_SEM_CalibrationUI *me = (nmui_AFM_SEM_CalibrationUI *)ud;

	double alpha = value;
	if (alpha > 1.0) alpha = 1.0;
	if (alpha < 0.0) alpha = 0.0;
	me->d_SEMTexture.setOpacity(alpha);
}

void nmui_AFM_SEM_CalibrationUI::handle_ZScale_change(
		vrpn_float64 value, void *ud)
{
	nmui_AFM_SEM_CalibrationUI *me = (nmui_AFM_SEM_CalibrationUI *)ud;
	handle_updateModel_change(1, me);
	me->d_heightFieldNeedsUpdate = true;
	me->updateSolution();
	
}

// static
void nmui_AFM_SEM_CalibrationUI::handle_colormap_change(const char* name, void* _ud)
{
    nmui_AFM_SEM_CalibrationUI *me = (nmui_AFM_SEM_CalibrationUI *)_ud;

    nmb_ColorMap* cmap = me->d_colorMapUI->currentColorMap();

	me->d_SEMTexture.setColorMap(cmap);
	me->d_SEMTexture.setUpdateColorMap(true);
}

// static
void nmui_AFM_SEM_CalibrationUI::handle_colormap_minmax_change(vrpn_float64, void* _ud)
{
//printf("handle_colormap_minmax_change\n");

    nmui_AFM_SEM_CalibrationUI *me = (nmui_AFM_SEM_CalibrationUI *)_ud;
    double dmin,dmax,cmin,cmax;

    me->d_colorMapUI->getDataColorMinMax(&dmin, &dmax, &cmin, &cmax);

    me->d_SEMTexture.setColorMapMinMax(dmin, dmax, cmin, cmax);
	me->d_SEMTexture.setUpdateColorMap(true);
}

void nmui_AFM_SEM_CalibrationUI::handle_generateTestData_change(
		vrpn_int32 value, void *ud)
{
	nmui_AFM_SEM_CalibrationUI *me = (nmui_AFM_SEM_CalibrationUI *)ud;
	if (value) {
		if (me->d_modelToSEM_modelImage) {
			double widthWorld = me->d_modelToSEM_modelImage->widthWorld();
			double minX = 0;//me->d_modelToSEM_modelImage->boundX(MIN_X_MIN_Y);
			double minY = 0;//me->d_modelToSEM_modelImage->boundY(MIN_X_MIN_Y);
			nmb_Transform_TScShR SEM_from_AFM;
			double angle = 0.7854;
			SEM_from_AFM.setScale(NMB_X, 1.0/widthWorld);
			SEM_from_AFM.setScale(NMB_Y, 1.0/widthWorld);
			SEM_from_AFM.setScale(NMB_Z, 1.0/widthWorld);
			SEM_from_AFM.setRotation(NMB_THETA, -angle);
			SEM_from_AFM.setRotation(NMB_PHI, angle);

			SEM_from_AFM.getMatrix(me->d_SEMfromAFM_test);

			nmb_Transform_TScShR AFM_from_model;
			AFM_from_model.setTranslation(NMB_Z, widthWorld*0.4);
			AFM_from_model.setTranslation(NMB_Y, -widthWorld*0.4-minY);
			AFM_from_model.setTranslation(NMB_X, -minX);
			//fprintf(stderr, "surface minX=%g, minY=%g\n", minX, minY);

			AFM_from_model.getMatrix(me->d_AFMfromModel_test);

			nmb_TransformMatrix44 SEM_from_AFM44, AFM_from_model44;
			SEM_from_AFM44.setMatrix(me->d_SEMfromAFM_test);
			printf("SEM_from_AFM:\n");
			SEM_from_AFM44.print();
			AFM_from_model44.setMatrix(me->d_AFMfromModel_test);
			printf("AFM_from_model:\n");
			AFM_from_model44.print();
			SEM_from_AFM44.compose(AFM_from_model44);
			printf("SEM_from_model:\n");
			SEM_from_AFM44.print();
			me->createTestImages();
		} else {
			printf("Error, can't make test data before selecting a model image\n");
		}
	}
}

int nmui_AFM_SEM_CalibrationUI::finishedFreehandHandler()
{
	fprintf(stderr, "nmui_AFM_SEM_CalibrationUI::finishedFreehandHandler()\n");
	if (d_windowOpen) {
	  fprintf(stderr, "nmui_AFM_SEM_CalibrationUI: auto-acquiring calibration point\n");
	  if (d_SEM) {
        vrpn_int32 scanPreviouslyEnabled;
        d_SEM->getExternalScanControlEnable(scanPreviouslyEnabled);
        if (!scanPreviouslyEnabled) {
          d_SEM->setExternalScanControlEnable(1);
        }
		d_SEM->requestScan(1);
		// send a synchronization message so we know when the scan has completed
		d_SEM->requestSynchronization(0, 0, SCAN_COMPLETION_MESSAGE);
        if (!scanPreviouslyEnabled) {
          d_SEM->setExternalScanControlEnable(0);
        }
	  }
	}
	return 0;
}

// static
int nmui_AFM_SEM_CalibrationUI::finishedFreehandHandler(void *ud)
{
  nmui_AFM_SEM_CalibrationUI *me = (nmui_AFM_SEM_CalibrationUI *)ud;
  return me->finishedFreehandHandler();
}

// non-static
int nmui_AFM_SEM_CalibrationUI::pointDataHandler(const Point_results *pr)
{
  Point_value *heightData = pr->getValueByPlaneName(
	  d_AFM->Data()->heightPlaneName->string());
  if (!heightData) 
  {
	  if( (vrpn_int32) d_windowOpen )
	  {
		  display_error_dialog("Missing height data in point channels");
		  return -1;
	  }
	  else return 0;
  }

  d_tipPosition[0] = pr->x();
  d_tipPosition[1] = pr->y();
  d_tipPosition[2] = heightData->value();
  d_tipPositionAcquiredSinceLastAdd = vrpn_TRUE;
  return 0;
}

// static
int nmui_AFM_SEM_CalibrationUI::pointDataHandler(void *ud, 
                                                const Point_results *pr)
{
  nmui_AFM_SEM_CalibrationUI *me = (nmui_AFM_SEM_CalibrationUI *)ud;
  return me->pointDataHandler(pr);
}

// static
void nmui_AFM_SEM_CalibrationUI::semDataHandler(void *userdata,
                          const nmm_Microscope_SEM_ChangeHandlerData &info)
{
	nmui_AFM_SEM_CalibrationUI *me = (nmui_AFM_SEM_CalibrationUI *)userdata;
	if (info.msg_type == nmm_Microscope_SEM::SCANLINE_DATA &&
		info.sem->lastScanMessageCompletesImage())
	{
		me->d_semImageAcquiredSinceLastAdd = vrpn_TRUE;
		if (me->d_liveSEMTexture) {
			nmb_ImageArray *image;
			me->d_SEM->getImageData(&image);
			me->d_SEMTexture.setImage(image);
		}
	}
}

// static
int nmui_AFM_SEM_CalibrationUI::semSynchHandler (void *userdata,
                                        const nmb_SynchMessage *data)
{
	fprintf(stderr, "nmui_AFM_SEM_CalibrationUI::semSynchHandler\n");
	nmui_AFM_SEM_CalibrationUI *me = (nmui_AFM_SEM_CalibrationUI *)userdata;
	if (strcmp(data->comment, SCAN_COMPLETION_MESSAGE) == 0) {
		if (me->d_registrationMode == me->d_registrationMode_AFM_SEM_contact) {
			me->addLatestDataAsContactPoint();
		} else if (me->d_registrationMode == me->d_registrationMode_AFM_SEM_free) {
			me->addLatestDataAsFreePoint();
		}
	}
	return 0;
}

void nmui_AFM_SEM_CalibrationUI::updateInputStatus()
{
	if (d_numModelSEMPoints < 3) {
		d_modelSEMPointsNeeded = 1;
	} else {
		d_modelSEMPointsNeeded = 0;
	}
	if (d_numContactPoints < 3) {
		d_contactPointsNeeded = 1;
	} else {
		d_contactPointsNeeded = 0;
	}
	if (d_numFreePoints < 1) {
		d_freePointsNeeded = 1;
	} else {
		d_freePointsNeeded = 0;
	}
	if (d_modelSEMPointsNeeded || d_contactPointsNeeded ||
		d_freePointsNeeded) {
		d_moreDataNeeded = 1;
	} else {
		d_moreDataNeeded = 0;
	}
}

void nmui_AFM_SEM_CalibrationUI::updateSolution()
{
	updateInputStatus();
	// determine whether or not we have sufficient data 
	// and update status variables to indicate what is needed to the user
	// the data requirements are actually a bit more flexible than what
	// is advertised to the user but its done this way for simplicity -
	// for example, we could trade off free points for contact points

	// if we have sufficient data then compute a new solution and display it

	if (!((vrpn_int32)d_moreDataNeeded) && d_modelToSEM_modelImage) {
		if (d_calibration) {
			delete d_calibration;
			d_calibration = NULL;
		}
		if (d_heightFieldNeedsUpdate) {
			fprintf(stderr, "Recomputing the surface geometry...");
			if (d_heightField) {
				delete d_heightField;
			}
			double minX, minY, maxX, maxY;
			d_modelToSEM_modelImage->getWorldRectangle(
				minX, minY, maxX, maxY);
			maxX -= minX;
			maxY -= minY;
			minX = 0.0;
			minY = 0.0;
			d_heightField = new nmr_SurfaceModelHeightField(
				d_modelToSEM_modelImage, minX, minY, maxX, maxY, d_ZScale);
			d_heightFieldNeedsUpdate = vrpn_FALSE;
			if (!d_heightField) {
				printf("Error, out of memory\n");
				return;
			}
			fprintf(stderr, "Done\n");
		}
		fprintf(stderr, "Creating calibration object...");
		d_calibration = new nmr_AFM_SEM_Calibration(d_heightField);
		if (!d_calibration) {
			printf("Error, out of memory\n");
			return;
		}
		// pass some more data to d_calibration, call update function
		// and get results
		d_calibration->setContactTipCorrespondence(d_contact_AFMPoints, d_contact_SEMPoints, 
			(int)d_numContactPoints);


		int numFreePoints = 0;
		nmr_CalibrationPoint free_AFMPoints[NMR_MAX_FIDUCIAL];
		nmr_CalibrationPoint free_SEMPoints[NMR_MAX_FIDUCIAL];
		int i, j;
		for (i = 0; i < d_numFreePoints; i++) {
			for (j = 0; j < 4; j++) {
				free_AFMPoints[numFreePoints][j] = d_free_AFMPoints[i][j];
				free_SEMPoints[numFreePoints][j] = d_free_SEMPoints[i][j];
			}
			numFreePoints++;
		}
		for (i = 0; i < d_numContactPoints; i++) {
			for (j = 0; j < 4; j++) {
				free_AFMPoints[numFreePoints][j] = d_contact_AFMPoints[i][j];
				free_SEMPoints[numFreePoints][j] = d_contact_SEMPoints[i][j];
			}
			numFreePoints++;
		}

		d_calibration->setFreeTipCorrespondence(free_AFMPoints, free_SEMPoints,
			numFreePoints);

		d_calibration->setSurfaceFeatureCorrespondence(d_modelToSEM_modelPoints,
			d_modelToSEM_SEMPoints, (int)d_numModelSEMPoints);

		fprintf(stderr, "Done\n");
		fprintf(stderr, "Computing calibration solution...");
		d_calibration->updateSolution();
		fprintf(stderr, "Done\n");
		d_calibration->getSEMfromAFMMatrix(d_SEMfromAFM);
		d_calibration->getSEMfromModel3DMatrix(d_SEMfromModel);
		d_calibration->getAFMfromModel3DMatrix(d_AFMfromModel);

		nmb_TransformMatrix44 temp;
		printf("SEM_from_AFM found:\n");
		temp.setMatrix(d_SEMfromAFM);
		temp.print();
		printf("SEM_from_Model found\n");
		temp.setMatrix(d_SEMfromModel);
		temp.print();
		printf("AFM_from_Model found\n");
		temp.setMatrix(d_AFMfromModel);
		temp.print();
		
		updateSolutionDisplay();
	}
}

void nmui_AFM_SEM_CalibrationUI::updateSolutionDisplay()
{
#ifdef _WIN32
    int i; 
	for (i = 0; i < 16; i++) {
		if (!_finite(d_AFMfromModel[i]) ||
			!_finite(d_SEMfromModel[i]) ||
			!_finite(d_SEMfromAFM[i])) {
			display_error_dialog(
				"Error, undefined or infinite values in solution");
			return;
		}
	}
#endif
	// display plan:
	//   display model surface as an uberGraphics object
	//   set transform for model according to afm<-->model 
    //     coordinate transformation
	//   display SEM image as a projective texture on the model using the
	//   appropriate texture projection matrix - either from the 
	//   afm<-->sem or model<-->sem coordinate transformation
	d_surfaceModelRenderer.setWorldFromObjectTransform(d_AFMfromModel);
	double projDirX, projDirY, projDirZ;
	d_calibration->getSEMProjectionDirectionInAFM(projDirX, projDirY, projDirZ);
	double xScale = sqrt(d_AFMfromModel[0]*d_AFMfromModel[0] + 
		d_AFMfromModel[1]*d_AFMfromModel[1]);
	double scale = 0.1*d_modelToSEM_modelImage->widthWorld()*xScale;
	if (scale < 0) {
		scale = -scale;
	}
	projDirX *= scale; projDirY *= scale; projDirZ *= scale;
	nmb_TransformMatrix44 worldFromObject;
	worldFromObject.setMatrix(d_AFMfromModel);
	double surfCenter[4] = {0,0,0,1};
	double minX, minY, maxX, maxY;
	d_modelToSEM_modelImage->getWorldRectangle(minX, minY, maxX, maxY);
	maxX -= minX;
	maxY -= minY;
	minX = 0.0;
	minY = 0.0;
	surfCenter[0] = 0.5*(minX + maxX);
	surfCenter[1] = 0.5*(minY + maxY);
	int w = d_modelToSEM_modelImage->width();
	int h = d_modelToSEM_modelImage->height();
	surfCenter[2] = d_modelToSEM_modelImage->getValue(w/2, h/2);
	worldFromObject.transform(surfCenter);
	d_textureProjectionDirectionRenderer.set(
		surfCenter[0], surfCenter[1], surfCenter[2],
		projDirX, projDirY, projDirZ);
	d_surfaceModelRenderer.SetTextureTransform(d_SEMfromModel);
	d_surfaceModelRenderer.SetTextureCoordinatesInWorld(false);
	if (d_tipRenderer) {
		d_tipRenderer->SetTextureTransform(d_SEMfromAFM);
	}
}
