#include "nm_Tip.h"
#include <GL/glut_UNC.h>
#include <math.h>

#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif

char *nm_TipDisplayControls::s_renderName = "AFM Tip";

nm_TipDisplayControls::nm_TipDisplayControls(nmm_Microscope_Remote *spm,
                                             nmr_Registration_Proxy *aligner):
  d_AFM(NULL),
  d_tipModel(),
  d_tipModelModeImage("tip_model_mode_image"),
  d_tipModelModeConeSphere("tip_model_mode_conesphere"),
  d_aligner(aligner),
  d_enableDisplay("tip_display_enable", 0),
  d_enableTexture("tip_display_texture_enable", 0),
  d_tipModelMode("tip_model_mode", 2),
  d_tipTopographyImage("tip_topography_image", "none"),
  d_tipConeSphereRadius("tip_cone_sphere_radius", 200.),
  d_tipConeSphereAngle("tip_cone_sphere_angle", 40.),
  d_tipConeSphereHeight("tip_cone_sphere_height", 450.0),
  d_sendFiducialRequested("send_fiducial_requested", 0),
  d_tipRenderer(&d_tipModel)
{
  setSPM(spm);
  World.TAddNode(&d_tipRenderer, s_renderName);
  d_enableDisplay.addCallback(handleEnableDisplayChange, this);
  d_enableTexture.addCallback(handleEnableTextureChange, this);

  d_tipModelMode.addCallback(handleTipModelModeChange, this);
  d_tipTopographyImage.addCallback(handleTipTopographyImageChange, this);
  d_tipConeSphereRadius.addCallback(handleConeSphereChange, this);
  d_tipConeSphereAngle.addCallback(handleConeSphereChange, this);
  d_tipConeSphereHeight.addCallback(handleConeSphereChange, this);

  d_tipModel.d_radius_nm = d_tipConeSphereRadius;
  d_tipModel.d_coneAngle_deg = d_tipConeSphereAngle;
  d_tipModel.d_totalHeight_nm = d_tipConeSphereHeight;

  d_tipRenderer.buildDisplayList();
  d_sendFiducialRequested.addCallback(handleSendFiducialRequested, this);
}

nm_TipDisplayControls::~nm_TipDisplayControls()
{
  UTree *myNode = World.TGetNodeByName(s_renderName);
  World.TRemoveTreeNode(myNode);
}

void nm_TipDisplayControls::handleEnableDisplayChange (vrpn_int32 newval, 
                                                       void *userdata)
{
  nm_TipDisplayControls *me = (nm_TipDisplayControls *)userdata;
  me->setDisplayEnable(newval);
}

void nm_TipDisplayControls::handleEnableTextureChange (vrpn_int32 newval, 
                                                       void *userdata)
{
  nm_TipDisplayControls *me = (nm_TipDisplayControls *)userdata;
  me->setTextureEnable(newval);
}

void nm_TipDisplayControls::handleTipModelModeChange(vrpn_int32 newval, 
                                                     void *ud)
{
  nm_TipDisplayControls *me = (nm_TipDisplayControls *)ud;
  if (newval == (vrpn_int32)(me->d_tipModelModeImage)) {
  } else if (newval == (vrpn_int32)(me->d_tipModelModeConeSphere)) {
  }
}

void nm_TipDisplayControls::handleTipTopographyImageChange(const char *name, 
                                                           void *ud)
{
  nm_TipDisplayControls *me = (nm_TipDisplayControls *)ud;
}

void nm_TipDisplayControls::handleConeSphereChange(vrpn_float64 newval, 
                                                        void *ud)
{
  nm_TipDisplayControls *me = (nm_TipDisplayControls *)ud;
  me->d_tipModel.d_radius_nm = me->d_tipConeSphereRadius;
  me->d_tipModel.d_coneAngle_deg = me->d_tipConeSphereAngle;
  me->d_tipModel.d_totalHeight_nm = me->d_tipConeSphereHeight;
  me->d_tipRenderer.buildDisplayList();
}

void nm_TipDisplayControls::handleSendFiducialRequested (vrpn_int32 newval,
                                                         void *userdata)
{
  nm_TipDisplayControls *me = (nm_TipDisplayControls *)userdata;
  if (newval) {
    me->sendFiducial();
    me->d_sendFiducialRequested = 0;
  }
}

// non-static
int nm_TipDisplayControls::pointDataHandler(const Point_results *pr)
{
  d_tipModel.setPosition(pr->x(), pr->y(), pr->z());
  return 0;
}

// static
int nm_TipDisplayControls::pointDataHandler(void *ud, const Point_results *pr)
{
  nm_TipDisplayControls *me = (nm_TipDisplayControls *)ud;
  return me->pointDataHandler(pr);
}

void nm_TipDisplayControls::setSPM(nmm_Microscope_Remote *scope) 
{
  if (d_AFM) {
    d_AFM->unregisterPointDataHandler(pointDataHandler, this);
  }
  d_AFM = scope;
  if (d_AFM) {
    d_AFM->registerPointDataHandler(pointDataHandler, this);
  }
}

void nm_TipDisplayControls::setDisplayEnable(int enable)
{
  d_tipRenderer.SetVisibility(enable);
}

void nm_TipDisplayControls::setTextureEnable(int enable)
{
  if (enable != 0){
    d_tipRenderer.setTextureEnable(true); 
  } else {
    d_tipRenderer.setTextureEnable(false);
  }
}

void nm_TipDisplayControls::sendFiducial()
{
  double minX, maxX, minY, maxY;
  double scanWidthX_nm = 1.0, scanWidthY_nm = 1.0;
  if (d_AFM) {
	minX = d_AFM->Data()->inputGrid->minX();
	maxX = d_AFM->Data()->inputGrid->maxX();
	minY = d_AFM->Data()->inputGrid->minY();
	maxY = d_AFM->Data()->inputGrid->maxY();
	scanWidthX_nm = maxX - minX;
	scanWidthY_nm = maxY - minY;
  } else {
    fprintf(stderr, "nm_TipDisplayControls::sendFiducial: Error, no AFM\n");
  }
  double fiducialX, fiducialY, fiducialZ;
  fiducialX = (d_tipModel.d_pos[0]-minX)/scanWidthX_nm;
  fiducialY = (d_tipModel.d_pos[1]-minY)/scanWidthY_nm;
  fiducialZ = d_tipModel.d_pos[2];
  if (d_aligner) {
    printf("Sending tip location as fiducial: (%g,%g): %g nm\n",
          fiducialX, fiducialY, fiducialZ);
    d_aligner->sendFiducial(fiducialX, fiducialY, fiducialZ,
                            fiducialX, fiducialY, fiducialZ);
  } else {
    fprintf(stderr, "nm_TipDisplayControls::sendFiducial: Error, no aligner\n");
  }
}

nm_TipModel::nm_TipModel()
{
  d_pos[0] = 0; d_pos[1] = 0; d_pos[2] = 0;
  d_tipSurface = NULL;
  d_rotation_deg = 0;
  d_offset[0] = 0; d_offset[1] = 0; d_offset[2] = 0;
  d_radius_nm = 50;
  d_coneAngle_deg = 20;
  d_totalHeight_nm = 200;
}

nm_TipModel::~nm_TipModel()
{

}

void nm_TipModel::setPosition(double x, double y, double z)
{
  d_pos[0] = x; d_pos[1] = y; d_pos[2] = z;
}

void nm_TipModel::getPosition(double &x, double &y, double &z)
{
  x = d_pos[0]; y = d_pos[1]; z = d_pos[2];
}

nm_TipRenderer::nm_TipRenderer(nm_TipModel *tipModel):
  URender(), d_drawConeSphere(vrpn_TRUE), 
  d_drawSurface(vrpn_FALSE), d_drawTexture(vrpn_FALSE),
  d_tipModel(tipModel), d_lastInstallProjImCount(0), d_displayListID(0)
{
  d_quadric = gluNewQuadric();
  gluQuadricDrawStyle(d_quadric, GLU_FILL);
  gluQuadricNormals(d_quadric, GLU_FLAT);
  buildDisplayList();
  visible = false;
}

nm_TipRenderer::~nm_TipRenderer()
{
  if (d_quadric) {
    gluDeleteQuadric(d_quadric);
  }
  if (d_displayListID != 0) {
    glDeleteLists(d_displayListID, 1);
  }
}

void nm_TipRenderer::setTextureEnable(bool enable)
{
  d_drawTexture = enable;
}

void nm_TipRenderer::buildDisplayList()
{
  double coneAngle_rad = d_tipModel->d_coneAngle_deg*M_PI/180.0;
  double sinTheta = sin(coneAngle_rad);
  double cosTheta = cos(coneAngle_rad);
  double tanTheta = sinTheta/cosTheta;
  double radius = d_tipModel->d_radius_nm;
  double coneBottomZ = radius - radius*sinTheta;
  double coneBottomRadius = radius*cosTheta;
  double coneTopZ = d_tipModel->d_totalHeight_nm;
  double coneTopRadius = radius + 
           (d_tipModel->d_totalHeight_nm + (sinTheta-1)*radius)*tanTheta;
  
  if (d_displayListID != 0) {
    glDeleteLists(d_displayListID, 1);
  }
  d_displayListID = glGenLists(1);
  if (d_displayListID == 0) {
    fprintf(stderr, "nm_TipRenderer::buildDisplayList: glGenLists failed\n");
    return;
  }
  glNewList(d_displayListID, GL_COMPILE);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glTranslatef(0.0, 0.0, radius);
  glColor3f(1.0, 0.0, 0.0);
  gluSphere(d_quadric, radius, 30, 30);
  glTranslatef(0, 0, -radius*sinTheta);
  gluCylinder(d_quadric, coneBottomRadius, coneTopRadius, coneTopZ-coneBottomZ,
              30, 30);
  glTranslatef(0.0, 0.0, coneTopZ-coneBottomZ);
  gluDisk(d_quadric, 0.0, coneTopRadius, 30, 1);
  glPopMatrix();
  glEndList();
}

// virtual
int nm_TipRenderer::Render(void * /*userdata*/)
{
  if (visible) {
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    GetLocalXform().Push_As_OGL();
    double tx, ty, tz;
    d_tipModel->getPosition(tx, ty, tz);
    glTranslatef(tx, ty, tz);
    glPushAttrib(GL_TEXTURE_BIT | GL_ENABLE_BIT);
    if (d_drawTexture) {
      glEnable(GL_TEXTURE_2D);
      glEnable(GL_TEXTURE_GEN_S);
      glEnable(GL_TEXTURE_GEN_T);
      glEnable(GL_TEXTURE_GEN_R);
      glEnable(GL_TEXTURE_GEN_Q);
    } else {
      glDisable(GL_TEXTURE_2D);
      glDisable(GL_TEXTURE_GEN_S);
      glDisable(GL_TEXTURE_GEN_T);
      glDisable(GL_TEXTURE_GEN_R);
      glDisable(GL_TEXTURE_GEN_Q);
    }
    if (d_drawConeSphere) {
      if (d_displayListID != 0) {
        glCallList(d_displayListID);
      }
    }
    glPopAttrib();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

  }
  if (recursion) {
    return ITER_CONTINUE;
  } else {
    return ITER_STOP;
  }
}
