/*===3rdtech===
  Copyright (c) 2000 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#include  "nmg_GraphicsTimer.h"

nmg_Graphics_Timer::nmg_Graphics_Timer (nmg_Graphics * imp,
                                        nmb_TimerList * timer) :
  nmg_Graphics (NULL, ""),
  d_timingViewpointChanges (VRPN_FALSE),
  d_imp (imp),
  d_timer (timer) {

}

nmg_Graphics_Timer::~nmg_Graphics_Timer (void) {

}


// PROTECTED

void nmg_Graphics_Timer::activateTimer (void) {
  if (d_timer) {
    d_timer->activate(d_timer->getListHead());
  }
}


void nmg_Graphics_Timer::timeViewpointChanges (vrpn_bool x) {
  d_timingViewpointChanges = x;
}

void nmg_Graphics_Timer::mainloop (void) {

  d_imp->mainloop();
  
}

void nmg_Graphics_Timer::changeDataset( nmb_Dataset * data) {
  d_imp->changeDataset(data);
}

void nmg_Graphics_Timer::resizeViewport (int width, int height) {
  activateTimer();
  d_imp->resizeViewport(width, height);
}

void nmg_Graphics_Timer::getViewportSize(int *width, int * height) {
    d_imp->getViewportSize(width, height);
}

void nmg_Graphics_Timer::getDisplayPosition (q_vec_type &ll,
        q_vec_type &ul, q_vec_type &ur)
{
  activateTimer();
  d_imp->getDisplayPosition(ll, ul, ur);
}

void nmg_Graphics_Timer::loadRulergridImage (const char * name) {
  activateTimer();
  d_imp->loadRulergridImage(name);
}

void nmg_Graphics_Timer::loadVizImage (const char * name) {
  activateTimer();
  d_imp->loadVizImage(name);
}

void nmg_Graphics_Timer::causeGridReColor (void) {
  activateTimer();
  d_imp->causeGridReColor();
}

void nmg_Graphics_Timer::causeGridRedraw (void) {
  activateTimer();
  d_imp->causeGridRedraw();
}

void nmg_Graphics_Timer::causeGridRebuild (void) {
  activateTimer();
  d_imp->causeGridRebuild();
}

void nmg_Graphics_Timer::enableChartjunk (int on) {
  activateTimer();
  d_imp->enableChartjunk(on);
}

void nmg_Graphics_Timer::enableFilledPolygons (int on, int region) {
  activateTimer();
  d_imp->enableFilledPolygons(on, region);
}

void nmg_Graphics_Timer::enableSmoothShading (int on) {
  activateTimer();
  d_imp->enableSmoothShading(on);
}

void nmg_Graphics_Timer::enableTrueTip (int on) {
  activateTimer();
  d_imp->enableTrueTip(on);
}

void nmg_Graphics_Timer::setViztexScale (float s)
{
  activateTimer();
  d_imp->setViztexScale(s);
}

void nmg_Graphics_Timer::setRegionMaskHeight(float min_height, float max_height, int region)
{
  activateTimer();
  d_imp->setRegionMaskHeight(min_height, max_height, region);
}

void nmg_Graphics_Timer::setRegionControlPlaneName(const char *name, int region)
{
  activateTimer();
  d_imp->setRegionControlPlaneName(name, region);
}

int nmg_Graphics_Timer::createRegion()
{
    activateTimer();
    return d_imp->createRegion();
}

void nmg_Graphics_Timer::destroyRegion(int region)
{
    activateTimer();
    d_imp->destroyRegion(region);
}

void nmg_Graphics_Timer::associateAlpha(vrpn_bool associate, int region)
{
    activateTimer();
    d_imp->associateAlpha(associate, region);
}

void nmg_Graphics_Timer::associateFilledPolygons(vrpn_bool associate, int region)
{
    activateTimer();
    d_imp->associateFilledPolygons(associate, region);
}

void nmg_Graphics_Timer::associateTextureDisplayed(vrpn_bool associate, int region)
{
    activateTimer();
    d_imp->associateTextureDisplayed(associate, region);
}

void nmg_Graphics_Timer::associateTextureMode(vrpn_bool associate, int region)
{
    activateTimer();
    d_imp->associateTextureMode(associate, region);
}

void nmg_Graphics_Timer::associateTextureTransformMode(vrpn_bool associate, int region)
{
    activateTimer();
    d_imp->associateTextureTransformMode(associate, region);
}

void nmg_Graphics_Timer::associateStride(vrpn_bool associate, int region)
{
    activateTimer();
    d_imp->associateStride(associate, region);;
}

void nmg_Graphics_Timer::setAdhesionSliderRange (float low,
                                                          float high) {
  activateTimer();
  d_imp->setAdhesionSliderRange(low, high);
}

void nmg_Graphics_Timer::setAlphaColor (float r, float g, float b) {
  activateTimer();
  d_imp->setAlphaColor(r,g,b);
}

void nmg_Graphics_Timer::setAlphaSliderRange (float low, float high) {
  activateTimer();
  d_imp->setAlphaSliderRange(low, high);
}

void nmg_Graphics_Timer::setBumpMapName (const char * name) {
  activateTimer();
  d_imp->setBumpMapName(name);
}

void nmg_Graphics_Timer::setColorMapDirectory (const char * dir) {
  activateTimer();
  d_imp->setColorMapDirectory(dir);
}

void nmg_Graphics_Timer::setTextureDirectory (const char * dir) {
  activateTimer();
  d_imp->setTextureDirectory(dir);
}

void nmg_Graphics_Timer::setColorMapName (const char * name) {
  activateTimer();
  d_imp->setColorMapName(name);
}

void nmg_Graphics_Timer::setColorMinMax (float low, float high) {
  activateTimer();
  d_imp->setColorMinMax(low, high);
}

void nmg_Graphics_Timer::setDataColorMinMax (float low, float high) {
  activateTimer();
  d_imp->setDataColorMinMax(low, high);
}

void nmg_Graphics_Timer::setOpacitySliderRange (float low, float high) {
  activateTimer();
  d_imp->setOpacitySliderRange(low, high);
}

void nmg_Graphics_Timer::setComplianceSliderRange (float low, float high) {
  activateTimer();
  d_imp->setComplianceSliderRange(low, high);
}

void nmg_Graphics_Timer::setContourColor (int r, int g, int b) {
  activateTimer();
  d_imp->setContourColor(r,g,b);
}

void nmg_Graphics_Timer::setFrictionSliderRange (float low, float high) {
  activateTimer();
  d_imp->setFrictionSliderRange(low, high);
}

void nmg_Graphics_Timer::setBumpSliderRange (float low, float high) {
  activateTimer();
  d_imp->setBumpSliderRange(low, high);
}

void nmg_Graphics_Timer::setBuzzSliderRange (float low, float high) {
  activateTimer();
  d_imp->setBuzzSliderRange(low, high);
}

void nmg_Graphics_Timer::setHandColor (int c) {
  activateTimer();
  d_imp->setHandColor(c);
}

void nmg_Graphics_Timer::setIconScale (float scale) {
  activateTimer();
  d_imp->setIconScale(scale);
}

void nmg_Graphics_Timer::enableCollabHand (vrpn_bool on) {
  activateTimer();
  d_imp->enableCollabHand(on);
}

void nmg_Graphics_Timer::setCollabHandPos(double pos[], double quat[])
{
  activateTimer();
  d_imp->setCollabHandPos(pos, quat);
}

void nmg_Graphics_Timer::setCollabMode(int mode)
{
  activateTimer();
  d_imp->setCollabMode(mode);
}

void nmg_Graphics_Timer::setHatchMapName (const char * name) {
  activateTimer();
  d_imp->setHatchMapName(name);
}


void nmg_Graphics_Timer::setAlphaPlaneName (const char * n) {
  activateTimer();
  d_imp->setAlphaPlaneName(n);
}

void nmg_Graphics_Timer::setColorPlaneName (const char * n) {
  activateTimer();
  d_imp->setColorPlaneName(n);
}

void nmg_Graphics_Timer::setContourPlaneName (const char * n) {
  activateTimer();
  d_imp->setContourPlaneName(n);
}

void nmg_Graphics_Timer::setOpacityPlaneName (const char * n) {
  activateTimer();
  d_imp->setOpacityPlaneName(n);
}

void nmg_Graphics_Timer::setMaskPlaneName (const char * n) {
  activateTimer();
  d_imp->setMaskPlaneName(n);
}

void nmg_Graphics_Timer::setHeightPlaneName (const char * n) {
  activateTimer();
  d_imp->setHeightPlaneName(n);
}

void nmg_Graphics_Timer::setContourWidth (float x) {
  activateTimer();
  d_imp->setContourWidth(x);
}

void nmg_Graphics_Timer::setMinColor (const double c [4]) {
  activateTimer();
  d_imp->setMinColor(c);
}

void nmg_Graphics_Timer::setMaxColor (const double c [4]) {
  activateTimer();
  d_imp->setMaxColor(c);
}

void nmg_Graphics_Timer::setMinColor (const int c [4]) {
  activateTimer();
  d_imp->setMinColor(c);
}

void nmg_Graphics_Timer::setMaxColor (const int c [4]) {
  activateTimer();
  d_imp->setMaxColor(c);
}

void nmg_Graphics_Timer::setPatternMapName (const char * name) {
  activateTimer();
  d_imp->setPatternMapName(name);
}

void nmg_Graphics_Timer::createRealignTextures( const char *name ) {
  activateTimer();
  d_imp->createRealignTextures(name);
}

void nmg_Graphics_Timer::
setRealignTexturesConversionMap( const char *map, const char *mapdir ) {
  activateTimer();
  d_imp->setRealignTexturesConversionMap(map, mapdir);
}

void nmg_Graphics_Timer::setRealignTextureSliderRange (float low,
								float high) {
  activateTimer();
  d_imp->setRealignTextureSliderRange(low, high);
}


void nmg_Graphics_Timer::computeRealignPlane( const char *name,
						       const char *newname ) {
  activateTimer();
  d_imp->computeRealignPlane(name, newname);
}

void nmg_Graphics_Timer::translateTextures ( int on,
						      float dx, float dy ) {
  activateTimer();
  d_imp->translateTextures(on, dx, dy);
}

void nmg_Graphics_Timer::scaleTextures ( int on,
						  float dx, float dy ) {
  activateTimer();
  d_imp->scaleTextures(on, dx, dy);
}

void nmg_Graphics_Timer::shearTextures ( int on,
						  float dx, float dy ) {
  activateTimer();
  d_imp->shearTextures(on, dx, dy);
}

void nmg_Graphics_Timer::rotateTextures ( int on, float theta ) {
  activateTimer();
  d_imp->rotateTextures(on, theta);
}

void nmg_Graphics_Timer::setTextureCenter( float dx, float dy ) {
  activateTimer();
  d_imp->setTextureCenter(dx, dy);
}

void nmg_Graphics_Timer::loadRawDataTexture(const int which,
       const char *image_name,
       const int start_x, const int start_y)
{
  activateTimer();
  d_imp->loadRawDataTexture(which, image_name, start_x, start_y);
}

void nmg_Graphics_Timer::updateTexture(int which_texture,
       const char *image_name,
       int start_x, int start_y,
       int end_x, int end_y)
{
  activateTimer();
  d_imp->updateTexture(which_texture, image_name, start_x, start_y,
                       end_x, end_y);
}

void nmg_Graphics_Timer::setTextureTransform(double *xform){
  activateTimer();
  d_imp->setTextureTransform(xform);
}


void nmg_Graphics_Timer::setRulergridAngle (float v) {
  activateTimer();
  d_imp->setRulergridAngle(v);
}

void nmg_Graphics_Timer::setRulergridColor (int r, int g, int b) {
  activateTimer();
  d_imp->setRulergridColor(r, g, b);
}

void nmg_Graphics_Timer::setRulergridOffset (float x, float y) {
  activateTimer();
  d_imp->setRulergridOffset(x, y);
}

void nmg_Graphics_Timer::setNullDataAlphaToggle (int val) {
  activateTimer();
  d_imp->setNullDataAlphaToggle(val);
}

void nmg_Graphics_Timer::setRulergridOpacity (float alpha) {
  activateTimer();
  d_imp->setRulergridOpacity(alpha);
}

void nmg_Graphics_Timer::setRulergridScale (float s) {
  activateTimer();
  d_imp->setRulergridScale(s);
}

void nmg_Graphics_Timer::setRulergridWidths (float x, float y) {
  activateTimer();
  d_imp->setRulergridWidths(x, y);
}

void nmg_Graphics_Timer::setSpecularity (int s) {
  activateTimer();
  d_imp->setSpecularity(s);
}

void nmg_Graphics_Timer::setLocalViewer (vrpn_bool s) {
  activateTimer();
  d_imp->setLocalViewer(s);
}

void nmg_Graphics_Timer::setDiffusePercent (float d) {
  activateTimer();
  d_imp->setDiffusePercent(d);
}

void nmg_Graphics_Timer::setSurfaceAlpha (float a, int region) {
  activateTimer();
  d_imp->setSurfaceAlpha(a, region);
}

void nmg_Graphics_Timer::setSpecularColor (float s) {
  activateTimer();
  d_imp->setSpecularColor(s);
}

void nmg_Graphics_Timer::setSphereScale (float s) {
  activateTimer();
  d_imp->setSphereScale(s);
}

void nmg_Graphics_Timer::setTesselationStride (int s, int region) {
  activateTimer();
  d_imp->setTesselationStride(s, region);
}

void nmg_Graphics_Timer::setTextureMode (TextureMode m,
	TextureTransformMode xm, int region) {
  activateTimer();
  d_imp->setTextureMode(m, xm, region);
  // EXTRA-SPECIAL STUFF - There's a default (nonvirtual)
  // implementation on nmg_Graphics that we can't intercept.
  d_textureMode = m;
}

void nmg_Graphics_Timer::setTextureScale (float f) {
  activateTimer();
  d_imp->setTextureScale(f);
}

void nmg_Graphics_Timer::setTrueTipScale (float f) {
  activateTimer();
  d_imp->setTrueTipScale(f);
}


void nmg_Graphics_Timer::setUserMode (int oldMode, int oldStyle, int newMode,
				      int style, int tool) {
  activateTimer();
  d_imp->setUserMode(oldMode, oldStyle, newMode, style, tool);
}


void nmg_Graphics_Timer::setLightDirection (q_vec_type & v) {
  activateTimer();
  d_imp->setLightDirection(v);
}

void nmg_Graphics_Timer::resetLightDirection (void) {
  activateTimer();
  d_imp->resetLightDirection();
}


//int nmg_Graphics_Timer::addPolylinePoint (const float point [2][3]) {
int nmg_Graphics_Timer::addPolylinePoint (const PointType point[2]) {
  activateTimer();
  return d_imp->addPolylinePoint(point);
}

void nmg_Graphics_Timer::emptyPolyline (void) {
  activateTimer();
  d_imp->emptyPolyline();
}

void nmg_Graphics_Timer::setRubberLineStart (float p0, float p1) {
  activateTimer();
  d_imp->setRubberLineStart(p0, p1);
}

void nmg_Graphics_Timer::setRubberLineEnd (float p2, float p3 ) {
  activateTimer();
  d_imp->setRubberLineEnd(p2, p3);
}

void nmg_Graphics_Timer::setRubberLineStart (const float p [2]) {
  activateTimer();
  d_imp->setRubberLineStart(p);
}

void nmg_Graphics_Timer::setRubberLineEnd (const float p [2] ) {
  activateTimer();
  d_imp->setRubberLineEnd(p);
}

void nmg_Graphics_Timer::setScanlineEndpoints(const float p0[3],
		const float p1[3]){
  activateTimer();
  d_imp->setScanlineEndpoints(p0, p1);
}

void nmg_Graphics_Timer::displayScanlinePosition(const int enable)
{
  activateTimer();
  d_imp->displayScanlinePosition(enable);
}

void nmg_Graphics_Timer::positionAimLine (const PointType top,
                                                   const PointType bottom) {
  activateTimer();
  d_imp->positionAimLine(top, bottom);
}

void nmg_Graphics_Timer::positionRubberCorner
    (float minx, float miny, float maxx, float maxy, int highlight_mask) {
  activateTimer();
  d_imp->positionRubberCorner(minx, miny, maxx, maxy, highlight_mask);
}

void nmg_Graphics_Timer::positionRegionBox
  (float center_x,float center_y, float width,float height, float angle, 
   int highlight_mask) {
  activateTimer();
  d_imp->positionRegionBox(center_x, center_y, width, height, angle, highlight_mask);
}

void nmg_Graphics_Timer::positionSweepLine (const PointType topL,
					    const PointType bottomL,
					    const PointType topR,
					    const PointType bottomR) {
  activateTimer();
  d_imp->positionSweepLine(topL, bottomL, topR, bottomR);
}

int nmg_Graphics_Timer::addPolySweepPoints (const PointType topL,
					    const PointType bottomL,
					    const PointType topR,
					    const PointType bottomR) {
    activateTimer();
    return d_imp->addPolySweepPoints(topL, bottomL, topR, bottomR);
}

void nmg_Graphics_Timer::setRubberSweepLineStart (const PointType left,
						  const PointType right) {
    activateTimer();
    d_imp->setRubberSweepLineStart(left, right);
}
void nmg_Graphics_Timer::setRubberSweepLineEnd (const PointType left,
						const PointType right) {
    activateTimer();
    d_imp->setRubberSweepLineEnd(left, right);
}

void nmg_Graphics_Timer::positionSphere (float x, float y, float z) {
  activateTimer();
  d_imp->positionSphere(x, y, z);
}



void nmg_Graphics_Timer::createScreenImage
  (const char * filename, const ImageType type)
{
  activateTimer();
  d_imp->createScreenImage(filename, type);
}

void nmg_Graphics_Timer::setViewTransform (v_xform_type x) {
  if (d_timingViewpointChanges) {
    activateTimer();
  }
  d_imp->setViewTransform(x);
}


void nmg_Graphics_Timer::setFeelGrid (int x, int y, q_vec_type * v) {
   activateTimer();
   d_imp->setFeelGrid(x, y, v);
}
void nmg_Graphics_Timer::showFeelGrid (vrpn_bool on) {
   activateTimer();
   d_imp->showFeelGrid(on);
}


void nmg_Graphics_Timer::getLightDirection (q_vec_type * v) const {
  d_imp->getLightDirection(v);
}
int nmg_Graphics_Timer::getHandColor (void) const {
  return d_imp->getHandColor();
}
int nmg_Graphics_Timer::getSpecularity (void) const {
  return d_imp->getSpecularity();
}
const double * nmg_Graphics_Timer::getMinColor (void) const {
  return d_imp->getMinColor();
}
const double * nmg_Graphics_Timer::getMaxColor (void) const {
  return d_imp->getMaxColor();
}


