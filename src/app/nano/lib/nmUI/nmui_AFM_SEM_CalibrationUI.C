#include <GL/glut_UNC.h>
#include "nmui_AFM_SEM_CalibrationUI.h"

nmui_AFM_SEM_CalibrationUI::nmui_AFM_SEM_CalibrationUI(
    nmr_Registration_Proxy *aligner,
	UTree *world,
	nmg_ImageDisplayProjectiveTexture *textureDisplay): 

	d_aligner(aligner), 
	d_windowOpen("afm_sem_window_open", 0),
	d_registrationMode_model_SEM("afm_sem_registration_mode_model_SEM",1),
	d_registrationMode_AFM_SEM_contact("afm_sem_registration_mode_AFM_SEM_contact",2),
	d_registrationMode_AFM_SEM_free("afm_sem_registration_mode_AFM_SEM_free",3),
	d_registrationMode("afm_sem_registration_mode",1),
	d_modelToSEM_modelImageName("afm_sem_model", "none"),
	d_modelToSEM_SEMImageName("afm_sem_sem_image", "none"),
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
	d_generateTestData("afm_sem_generate_test_data", 0),
	d_updateSolution("afm_sem_update_solution", 0),
	d_AFM(NULL),
	d_SEM(NULL),
	d_dataset(NULL),
	d_tipPositionAcquiredSinceLastAdd(vrpn_FALSE),
	d_semImageAcquiredSinceLastAdd(vrpn_FALSE),
	d_calibration(NULL),
	d_surfaceModelRenderer(),
	d_world(world),
	d_textureDisplay(textureDisplay),
	d_surfaceStride(3),
	d_heightField(NULL),
	d_heightFieldNeedsUpdate(vrpn_TRUE)
{
  d_tipPosition[0] = 0;
  d_tipPosition[1] = 0;
  d_tipPosition[2] = 0;
  d_numModelSEMPoints = 0;
  d_numContactPoints = 0;
  d_numFreePoints = 0;
  d_modelToSEM_modelImage = NULL;
  d_modelToSEM_SEMImage = NULL;

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
  d_modelToSEM_SEMImageName.addCallback(handle_modelToSEM_SEMImage_change, 
			this);
  d_addContactPoint.addCallback(handle_AddContactPoint, this);
  d_deleteContactPoint.addCallback(handle_DeleteContactPoint, this);
  d_currentContactPoint.addCallback(handle_currentContactPoint_change, this);
  d_addFreePoint.addCallback(handle_AddFreePoint, this);
  d_deleteFreePoint.addCallback(handle_DeleteFreePoint, this);
  d_currentFreePoint.addCallback(handle_currentFreePoint_change, this);
  d_generateTestData.addCallback(handle_generateTestData_change, this);
  d_updateSolution.addCallback(handle_updateSolution_change, this);

  d_aligner->registerChangeHandler(this, correspondenceEditorChangeHandler);

  if (d_world) {
	d_world->TAddNode(&d_surfaceModelRenderer, "AFM-SEM Surface Model");
  }
}

void nmui_AFM_SEM_CalibrationUI::setSPM(nmm_Microscope_Remote *scope) 
{
  if (d_AFM) {
    d_AFM->unregisterPointDataHandler(pointDataHandler, this);
  }
  d_AFM = scope;
  if (d_AFM) {
    d_AFM->registerPointDataHandler(pointDataHandler, this);
  }
}

void nmui_AFM_SEM_CalibrationUI::setSEM(nmm_Microscope_SEM_Remote *sem)
{
  if (d_SEM) {
    d_SEM->unregisterChangeHandler(this, semDataHandler);
  }
  d_SEM = sem;
  if (d_SEM) {
    d_SEM->registerChangeHandler(this, semDataHandler);
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
		d_surfaceModelRenderer.setSurface(d_modelToSEM_modelImage, d_surfaceStride);
		d_modelToSEM_SEMImage = 
			d_dataset->dataImages()->getImageByName(
                            d_modelToSEM_SEMImageName.string());
		d_heightFieldNeedsUpdate = vrpn_TRUE;
		d_aligner->setImage(NMR_SOURCE, d_modelToSEM_modelImage, 
				vrpn_FALSE, vrpn_FALSE);
		d_aligner->setImage(NMR_TARGET, d_modelToSEM_SEMImage, 
				vrpn_FALSE, vrpn_FALSE);
	}
}

void nmui_AFM_SEM_CalibrationUI::createTestImages(double *SEM_from_AFM_matrix, double *AFM_from_model_matrix)
{
	/* plan:
	base all images on the model
	synthesize SEM image of surface
	pick a reasonable set of contact/free points
	synthesize SEM images showing a representation of the tip
	
	let the user pick the model<-->sem points
	*/
	ImageViewer *imageViewer = ImageViewer::getImageViewer();
    char *display_name;
    display_name = (char *)getenv("V_X_DISPLAY");
    if (!display_name) {
       display_name = (char *)getenv("DISPLAY");
       if (!display_name) {
          display_name = "unix:0";
       }
    }
	int renderWindowID = 
		imageViewer->createWindow(display_name, 10, 10, 500, 500, "modelRender");

	imageViewer->setGraphicsContext(renderWindowID);

	glDrawBuffer(GL_BACK);
	glViewport(0, 0, 500, 500);
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
	glLoadMatrixd(SEM_from_AFM_matrix);
	glMultMatrixd(AFM_from_model_matrix);

	double matrix[16];
	glGetDoublev(GL_MODELVIEW_MATRIX, matrix);
	printf("modelview (sem_from_model) matrix:\n");
	nmb_TransformMatrix44 temp;
	temp.setMatrix(matrix);
	temp.print();

	d_surfaceModelRenderer.renderWithoutDisplayList(
		d_modelToSEM_modelImage, 2);
	glPopMatrix();

	glReadBuffer(GL_BACK);
	nmb_Image *image = NULL;
	image = new nmb_ImageArray("SEM_from_ModelTest", "units", 500, 500,
			NMB_FLOAT32);
	glPixelStorei(GL_PACK_SKIP_PIXELS, image->borderXMin());
	glPixelStorei(GL_PACK_SKIP_ROWS, image->borderYMin());
	glPixelStorei(GL_PACK_ROW_LENGTH, 
		image->width() + image->borderXMin() + image->borderXMax());
	glReadPixels(0, 0, image->width(), image->height(), 
		GL_RED, GL_FLOAT, image->pixelData());
	d_dataset->dataImages()->addImage(image);

	nmb_TransformMatrix44 SEM_from_AFM;
	SEM_from_AFM.setMatrix(SEM_from_AFM_matrix);
	nmb_TransformMatrix44 AFM_from_model;
	AFM_from_model.setMatrix(AFM_from_model_matrix);

	double maxIntensity = image->maxValue();
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
		glLoadMatrixd(SEM_from_AFM_matrix);
		glMultMatrixd(AFM_from_model_matrix);
		d_surfaceModelRenderer.renderWithoutDisplayList(
			d_modelToSEM_modelImage, 2);
		glPopMatrix();

		glColor4f(maxIntensity, 1.0, 1.0, 1.0);
		glBegin(GL_TRIANGLES);
		glVertex3d(sem_cont_pnt[pntIndex][0], sem_cont_pnt[pntIndex][1], sem_cont_pnt[pntIndex][2]);
		glVertex3d(sem_cont_pnt[pntIndex][0]-0.3, sem_cont_pnt[pntIndex][1]+1.0, sem_cont_pnt[pntIndex][2]);
		glVertex3d(sem_cont_pnt[pntIndex][0]+0.3, sem_cont_pnt[pntIndex][1]+1.0, sem_cont_pnt[pntIndex][2]);
		glEnd();
		image = new nmb_ImageArray("SEM_from_AFMtest", "units", 500, 500,
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
		glLoadMatrixd(SEM_from_AFM_matrix);
		glMultMatrixd(AFM_from_model_matrix);
		d_surfaceModelRenderer.renderWithoutDisplayList(
			d_modelToSEM_modelImage, 2);
		glPopMatrix();

		glColor4f(maxIntensity, 1.0, 1.0, 1.0);
		glBegin(GL_TRIANGLES);
		glVertex3d(sem_free_pnt[pntIndex][0], sem_free_pnt[pntIndex][1], sem_free_pnt[pntIndex][2]);
		glVertex3d(sem_free_pnt[pntIndex][0]-0.3, sem_free_pnt[pntIndex][1]+1.0, sem_free_pnt[pntIndex][2]);
		glVertex3d(sem_free_pnt[pntIndex][0]+0.3, sem_free_pnt[pntIndex][1]+1.0, sem_free_pnt[pntIndex][2]);
		glEnd();
		image = new nmb_ImageArray("SEM_from_AFMtest", "units", 500, 500,
				NMB_FLOAT32);
		glReadPixels(0, 0, image->width(), image->height(), 
			GL_RED, GL_FLOAT, image->pixelData());
		addFreePoint(afm_free_pnt[pntIndex], sem_free_pnt[pntIndex], image);
	}

	imageViewer->destroyWindow(renderWindowID);
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
	updateInputStatus();
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
		me->d_surfaceModelRenderer.SetVisibility(1);
	} else {
		me->d_aligner->setGUIEnable(vrpn_FALSE, NMR_ALLWINDOWS);
		me->d_aligner->setEditEnable(vrpn_TRUE, vrpn_TRUE);
		// tell the aligner to send registration results - this should be the
		// default behavior because its expected by the normal 
        // registration code
		me->d_aligner->enableAutoUpdate(vrpn_TRUE);
		me->d_surfaceModelRenderer.SetVisibility(0);
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
			x_src[i] = d_modelToSEM_SEMPoints[i][0];
			y_src[i] = d_modelToSEM_SEMPoints[i][1];
			z_src[i] = d_modelToSEM_SEMPoints[i][2];
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
	printf("modelToSEM_model: %s\n", name);
	if (me->d_dataset) {
		me->d_modelToSEM_modelImage = 
			me->d_dataset->dataImages()->getImageByName(name);
		me->d_surfaceModelRenderer.setSurface(me->d_modelToSEM_modelImage, 
			me->d_surfaceStride);
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
void nmui_AFM_SEM_CalibrationUI::handle_modelToSEM_SEMImage_change(
			const char *name, void *ud)
{
	nmui_AFM_SEM_CalibrationUI *me = (nmui_AFM_SEM_CalibrationUI *)ud;
	if (!(vrpn_int32)(me->d_windowOpen)) {
		return;
	}
	printf("modelToSEM_SEMImage: %s\n", name);
	if (me->d_dataset) {
		me->d_modelToSEM_SEMImage = 
			me->d_dataset->dataImages()->getImageByName(name);
		me->d_heightFieldNeedsUpdate = vrpn_TRUE;
	}
	if ((vrpn_int32)(me->d_registrationMode) == 
		(vrpn_int32)(me->d_registrationMode_model_SEM) &&
		(vrpn_int32)(me->d_windowOpen)) {
		me->d_aligner->setImage(NMR_TARGET, me->d_modelToSEM_SEMImage, 
					vrpn_FALSE, vrpn_FALSE);
	}
	me->updateInputStatus();
}

// AFM_SEM_contact
// buttons
// static 
void nmui_AFM_SEM_CalibrationUI::handle_AddContactPoint(
			vrpn_int32 value, void *ud)
{
	nmui_AFM_SEM_CalibrationUI *me = (nmui_AFM_SEM_CalibrationUI *)ud;
	// switch mode
	if (me->d_tipPositionAcquiredSinceLastAdd && 
		me->d_semImageAcquiredSinceLastAdd) {
		nmb_ImageArray *image;
		me->d_SEM->getImageData(&image);
		me->addContactPoint(me->d_tipPosition, NULL, new nmb_ImageArray(image));
		me->d_tipPositionAcquiredSinceLastAdd = vrpn_FALSE;
		me->d_semImageAcquiredSinceLastAdd = vrpn_FALSE;
		me->updateInputStatus();
	} else {
		fprintf(stderr, "Error, need to acquire data first\n");
	}
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
	if (me->d_tipPositionAcquiredSinceLastAdd && 
        me->d_semImageAcquiredSinceLastAdd) {
		nmb_ImageArray *image;
		me->d_SEM->getImageData(&image);
		me->addFreePoint(me->d_tipPosition, NULL, new nmb_ImageArray(image));
		me->d_tipPositionAcquiredSinceLastAdd = vrpn_FALSE;
		me->d_semImageAcquiredSinceLastAdd = vrpn_FALSE;
		me->updateInputStatus();
	} else {
		fprintf(stderr, "Error, need to acquire data first\n");
	}
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

void nmui_AFM_SEM_CalibrationUI::handle_generateTestData_change(
		vrpn_int32 value, void *ud)
{
	nmui_AFM_SEM_CalibrationUI *me = (nmui_AFM_SEM_CalibrationUI *)ud;
	if (value) {
		if (me->d_modelToSEM_modelImage) {
			double widthWorld = me->d_modelToSEM_modelImage->widthWorld();
			nmb_Transform_TScShR SEM_from_AFM;
			double angle = 0.7854;
			SEM_from_AFM.setScale(NMB_X, 1.0/widthWorld);
			SEM_from_AFM.setScale(NMB_Y, 1.0/widthWorld);
			SEM_from_AFM.setScale(NMB_Z, 1.0/widthWorld);
			SEM_from_AFM.setRotation(NMB_THETA, -angle);
			double sem_afm_matrix[16];
			SEM_from_AFM.getMatrix(sem_afm_matrix);

			nmb_Transform_TScShR AFM_from_model;
			AFM_from_model.setTranslation(NMB_Z, widthWorld*0.2);
			AFM_from_model.setTranslation(NMB_X, widthWorld*0.1);
			double afm_model_matrix[16];
			AFM_from_model.getMatrix(afm_model_matrix);

			nmb_TransformMatrix44 SEM_from_AFM44, AFM_from_model44;
			SEM_from_AFM44.setMatrix(sem_afm_matrix);
			printf("SEM_from_AFM:\n");
			SEM_from_AFM44.print();
			AFM_from_model44.setMatrix(afm_model_matrix);
			printf("AFM_from_model:\n");
			AFM_from_model44.print();
			SEM_from_AFM44.compose(AFM_from_model44);
			printf("SEM_from_model:\n");
			SEM_from_AFM44.print();

			me->createTestImages(sem_afm_matrix, afm_model_matrix);
		} else {
			printf("Error, can't make test data before selecting a model image\n");
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

// non-static
int nmui_AFM_SEM_CalibrationUI::pointDataHandler(const Point_results *pr)
{
  d_tipPosition[0] = pr->x();
  d_tipPosition[1] = pr->y();
  d_tipPosition[2] = pr->z();
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
	}
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
			if (d_heightField) {
				delete d_heightField;
			}
			d_heightField = new nmr_SurfaceModelHeightField(
				d_modelToSEM_modelImage);
			d_heightFieldNeedsUpdate = vrpn_FALSE;
			if (!d_heightField) {
				printf("Error, out of memory\n");
				return;
			}
		}
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
		d_calibration->updateSolution();
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
	// display plan:
	//   display model surface as an uberGraphics object
	//   set transform for model according to afm<-->model 
    //     coordinate transformation
	//   display SEM image as a projective texture on the model using the
	//   appropriate texture projection matrix - either from the 
	//   afm<-->sem or model<-->sem coordinate transformation
	d_surfaceModelRenderer.setWorldFromObjectTransform(d_AFMfromModel);
}

