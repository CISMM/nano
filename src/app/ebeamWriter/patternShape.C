#include "patternShape.h"
#include "edgeTable.h"
#include "math.h"
#include "GL/gl.h"
#include "GL/glut_UNC.h"
#include "stdio.h"
#include "nmm_Microscope_SEM_Remote.h"
#include "nmm_Microscope_SEM_EDAX.h"
#include "exposureUtil.h"

PatternShapeColorMap::PatternShapeColorMap(): 
   d_numLineExposureLevels(0),
   d_numAreaExposureLevels(0),
   d_colorMode(PS_COLOR_BY_EXPOSURE)
{
  int i;
  for (i = 0; i < 3; i++) {
    d_minLineExposureColor[i] = 0.0;
    d_maxLineExposureColor[i] = 1.0;
    d_minAreaExposureColor[i] = 0.0;
    d_maxAreaExposureColor[i] = 1.0;
  }
}

// static
void PatternShapeColorMap::exposureColor(double exposure, double *color,
                          list<double> &levels, int numLevels,
                       double *minColor, double *maxColor)
{
  double interp;
  list<double>::iterator level = levels.begin();

  if (numLevels <= 1) {
    interp = 0;
  } else {
    int exposureLevel = 0;
    while (level != levels.end()) {
      if ((*level) == exposure) break;
      exposureLevel++;
      level++;
    }
    interp = (double)exposureLevel/(double)(numLevels - 1);
  }
  color[0] = interp*maxColor[0] + (1-interp)*minColor[0];
  color[1] = interp*maxColor[1] + (1-interp)*minColor[1];
  color[2] = interp*maxColor[2] + (1-interp)*minColor[2];
}

void PatternShapeColorMap::linearExposureColor(double exposure, double *color)
{
  exposureColor(exposure, color, d_lineExposureLevels, d_numLineExposureLevels,
                d_minLineExposureColor, d_maxLineExposureColor);
}

void PatternShapeColorMap::areaExposureColor(double exposure, double *color)
{
  exposureColor(exposure, color, d_areaExposureLevels, d_numAreaExposureLevels,
                d_minAreaExposureColor, d_maxAreaExposureColor);
}

void PatternShapeColorMap::draw(double x, double y,
                       double units_per_pixel_x,
                       double units_per_pixel_y)
{
  char str[128];
  list<double>::iterator level;
  level = d_lineExposureLevels.begin();
  int value;
  double color[3];
  while (level != d_lineExposureLevels.end()) {
    value = (int)floor(*level);
    linearExposureColor((*level), color);
    glColor4f(color[0], color[1], color[2], 1.0);
    sprintf(str, "%d pCoul/cm", value);
    glRasterPos2d(x, y);
    int i;
    for (i = 0; i < strlen(str); i++) {
      glutBitmapCharacter(GLUT_BITMAP_8_BY_13, str[i]);
    }
    y -= 15.0*units_per_pixel_y; // 15 pixels
    level++;
  }
  level = d_areaExposureLevels.begin();
  while (level != d_areaExposureLevels.end()) {
    value = (int)floor(*level);
    areaExposureColor((*level), color);
    glColor4f(color[0], color[1], color[2], 1.0);
    sprintf(str, "%d uCoul/cm2", value);
    glRasterPos2d(x, y);
    int i;
    for (i = 0; i < strlen(str); i++) {
      glutBitmapCharacter(GLUT_BITMAP_8_BY_13, str[i]);
    }
    y -= 15.0*units_per_pixel_y; // 15 pixels
    level++;
  }
}

int PatternShape::s_nextID = 1;

PatternShape::PatternShape(ShapeType type):
  d_ID(s_nextID),
  d_shapeType(type),
  d_numReferences(1)
{
  s_nextID++;
}

PatternShapeListElement::PatternShapeListElement(PatternShape *ps): 
  d_shape(ps)
{
  d_shape->d_numReferences++;
}

PatternShapeListElement::PatternShapeListElement(
  const PatternShapeListElement &psle):
  d_shape(psle.d_shape)
{
  d_shape->d_numReferences++;
}

PatternShapeListElement::~PatternShapeListElement()
{
  if (d_shape->d_numReferences == 1) {
    delete d_shape;
  }
  d_shape->d_numReferences--;
}

int PatternShapeListElement::operator== 
             (const PatternShapeListElement &psle) const
{
  if (!d_shape || !psle.d_shape) {
    return vrpn_FALSE;
  } else {
    return (*d_shape == *psle.d_shape);
  }
}

PolylinePatternShape::PolylinePatternShape(double lineWidth, double line_dose,
                                           double area_dose):
  PatternShape(PS_POLYLINE),
  d_sidePointsNeedUpdate(vrpn_FALSE),
  d_leftSidePoints(NULL),
  d_rightSidePoints(NULL),
  d_numPoints(0),
  d_lineWidth_nm(lineWidth),
  d_exposure_uCoulombs_per_square_cm(area_dose),
  d_exposure_pCoulombs_per_cm(line_dose)
{
  
}

PolylinePatternShape::PolylinePatternShape(const PolylinePatternShape &pps):
  PatternShape(PS_POLYLINE),
  d_sidePointsNeedUpdate(pps.d_numPoints > 0),
  d_leftSidePoints(NULL),
  d_rightSidePoints(NULL),
  d_numPoints(pps.d_numPoints),
  d_lineWidth_nm(pps.d_lineWidth_nm),
  d_exposure_uCoulombs_per_square_cm(pps.d_exposure_uCoulombs_per_square_cm),
  d_exposure_pCoulombs_per_cm(pps.d_exposure_pCoulombs_per_cm)
{
  d_points = pps.d_points;
}

void PolylinePatternShape::drawToDisplay(double units_per_pixel_x,
                               double units_per_pixel_y,
                               PatternShapeColorMap &map)
{
  if (d_numPoints == 0) return;

  if (d_lineWidth_nm == 0) {
    drawToDisplayZeroWidth(units_per_pixel_x,
                                            units_per_pixel_y,
                                            map);
  } else {

    if (d_sidePointsNeedUpdate) {
      computeSidePoints();
    }

    double color[3];
    map.areaExposureColor(d_exposure_uCoulombs_per_square_cm, color);

    glLineWidth(1);
    glColor4f(color[0], color[1], color[2], 1.0);

    if (d_numPoints == 1) {
      list<PatternPoint>::iterator pntIter;
      pntIter = d_points.begin();
      glBegin(GL_POINTS);
      glVertex3f((*pntIter).d_x, (*pntIter).d_y, 0.0);
      glEnd();
      return;
    } else {
      glBegin(GL_LINE_LOOP);
      int i;
      for (i = 0; i < d_numPoints; i++) {
        glVertex2d(d_leftSidePoints[i].d_x, d_leftSidePoints[i].d_y);
      }
      for (i = d_numPoints-1; i >= 0; i--) {
        glVertex2d(d_rightSidePoints[i].d_x, d_rightSidePoints[i].d_y);
      }
      glEnd();
    }
  }
}

void PolylinePatternShape::drawToSEM(nmm_Microscope_SEM_Remote *sem)
{
  if (d_numPoints <= 1) return;

  vrpn_float32 *x_nm, *y_nm;
  x_nm = new vrpn_float32[d_numPoints];
  y_nm = new vrpn_float32[d_numPoints];

  list<PatternPoint>::iterator point;
  int i = 0;
  for (point = d_points.begin(); point != d_points.end(); point++) {
    x_nm[i] = (*point).d_x;
    y_nm[i] = (*point).d_y;
    i++;
  }

  sem->addPolyline(d_exposure_pCoulombs_per_cm,
                   d_exposure_uCoulombs_per_square_cm,
                   d_lineWidth_nm, d_numPoints, x_nm, y_nm);
}

void PolylinePatternShape::drawToSEM(nmm_Microscope_SEM_EDAX *sem,
            double current, double dotSpacing, double lineSpacing,
            int &numPoints, double &expTime)
{
  generateExposurePoints(sem, current, dotSpacing, lineSpacing,
          numPoints, expTime);
}

void PolylinePatternShape::computeExposureStatistics(int &numPoints,
      double &expTime,
      double current, double dotSpacing, double lineSpacing)
{
  // this is a simple way to keep things consistent - we just run the
  // same code used for the exposure without sending commands to the sem
  // and keep track of how many points get generated and the total dwell time
  generateExposurePoints(NULL, current, dotSpacing, lineSpacing,
          numPoints, expTime);
}

void PolylinePatternShape::generateExposurePoints(nmm_Microscope_SEM_EDAX *sem,
            double current, double dotSpacing, double lineSpacing,
            int &numPoints, double &expTime)
{
  numPoints = 0;
  expTime = 0.0;

  if (d_numPoints <= 1) return;

  if (d_lineWidth_nm == 0) {
    generateExposurePointsZeroWidth(sem, current, dotSpacing, lineSpacing,
                                    numPoints, expTime);
  } else {

    if (d_sidePointsNeedUpdate) {
      computeSidePoints();
    }

    double invDotSpacing = 1.0/dotSpacing;
    double invLineSpacing = 1.0/lineSpacing;
    double exposure = d_exposure_uCoulombs_per_square_cm;
    int x_DAC, y_DAC;
    double dwellTime_nsec =
        1e9*ExposureUtil::computeAreaDwellTime(dotSpacing, lineSpacing, 
                                              current, exposure);

    if (sem) {
      sem->setPointDwellTime((int)floor(dwellTime_nsec), vrpn_FALSE);
    }

    double leftSegmentLength, rightSegmentLength;
    int numSegmentPoints;
    double x_nm, y_nm;
    double deltaLeft_x, deltaLeft_y, deltaRight_x, deltaRight_y;
    double incr_x, incr_y;
    double dir_x, dir_y;

    int numLines = floor(d_lineWidth_nm*invLineSpacing);
    double edgeOffsetFraction = 0.5*(d_lineWidth_nm - numLines*lineSpacing)/
                              d_lineWidth_nm;
    double interp_incr = 1.0/(double)(numLines-1);
    int i, j, k;
    int numSegments = d_numPoints-1;

    double leftStartX, rightStartX, leftStartY, rightStartY;
    double leftEndX, rightEndX, leftEndY, rightEndY;

    for (i = 0; i < numSegments; i++) {
      deltaLeft_x = d_leftSidePoints[i+1].d_x - d_leftSidePoints[i].d_x;
      deltaLeft_y = d_leftSidePoints[i+1].d_y - d_leftSidePoints[i].d_y; 
      deltaRight_x = d_rightSidePoints[i+1].d_x - d_rightSidePoints[i].d_x;
      deltaRight_y = d_rightSidePoints[i+1].d_y - d_rightSidePoints[i].d_y;
      leftSegmentLength = sqrt(deltaLeft_x*deltaLeft_x + 
                               deltaLeft_y*deltaLeft_y);
      leftStartX = edgeOffsetFraction*
                 (d_rightSidePoints[i].d_x - d_leftSidePoints[i].d_x) +
                 d_leftSidePoints[i].d_x;
      rightStartX = edgeOffsetFraction*
                 (d_leftSidePoints[i].d_x - d_rightSidePoints[i].d_x) +
                 d_rightSidePoints[i].d_x;
      leftStartY = edgeOffsetFraction*
                 (d_rightSidePoints[i].d_y - d_leftSidePoints[i].d_y) +
                 d_leftSidePoints[i].d_y;
      rightStartY = edgeOffsetFraction*
                 (d_leftSidePoints[i].d_y - d_rightSidePoints[i].d_y) +
                 d_rightSidePoints[i].d_y;
      leftEndX = edgeOffsetFraction*
                 (d_rightSidePoints[i+1].d_x - d_leftSidePoints[i+1].d_x) +
                 d_leftSidePoints[i+1].d_x;
      rightEndX = edgeOffsetFraction*
                 (d_leftSidePoints[i+1].d_x - d_rightSidePoints[i+1].d_x) +
                 d_rightSidePoints[i+1].d_x;
      leftEndY = edgeOffsetFraction*
                 (d_rightSidePoints[i+1].d_y - d_leftSidePoints[i+1].d_y) +
                 d_leftSidePoints[i+1].d_y;
      rightEndY = edgeOffsetFraction*
                 (d_leftSidePoints[i+1].d_y - d_rightSidePoints[i+1].d_y) +
                 d_rightSidePoints[i+1].d_y;
      dir_x = deltaLeft_x/leftSegmentLength;
      dir_y = deltaLeft_y/leftSegmentLength;
      incr_x = dir_x*dotSpacing;
      incr_y = dir_y*dotSpacing;

      double interp;
      for (j = 0, interp = 0.0; j < numLines; j++, interp += interp_incr) {
        double startX = interp*(rightStartX - leftStartX) + leftStartX;
        double startY = interp*(rightStartY - leftStartY) + leftStartY;
        double endX = interp*(rightEndX - leftEndX) + leftEndX;
        double endY = interp*(rightEndY - leftEndY) + leftEndY;
        double deltaX = endX - startX, deltaY = endY - startY;
        double segmentLength = deltaX*dir_x + deltaY*dir_y;
        int numLinePoints = floor(segmentLength*invDotSpacing);
      
        for (k = 0, x_nm = startX, y_nm = startY; k < numLinePoints;
             k++, x_nm += incr_x, y_nm += incr_y) {
          if (sem) {
            sem->convert_nm_to_DAC(x_nm, y_nm, x_DAC, y_DAC);
            sem->goToPoint(x_DAC, y_DAC, PS_POLYLINE);
          }
          numPoints++;
        }
      }
    }
    expTime = numPoints*dwellTime_nsec*1e-9;
  }
}

double PolylinePatternShape::minX()
{
  if (d_numPoints == 0) return 0.0;

  if (d_lineWidth_nm == 0) {
    return minXZeroWidth();
  }

  list<PatternPoint>::iterator point = d_points.begin();
  double result = (*point).d_x;

  if (d_numPoints == 1) {
    return result;
  } 

  if (d_sidePointsNeedUpdate) {
    computeSidePoints();
  }

  int i;
  for (i = 0; i < d_numPoints; i++) {
    if (d_leftSidePoints[i].d_x < result) {
      result = d_leftSidePoints[i].d_x;
    }
    if (d_rightSidePoints[i].d_x < result) {
      result = d_rightSidePoints[i].d_x;
    }
  }
  return result;
}

double PolylinePatternShape::minY()
{
  if (d_numPoints == 0) return 0.0;

  if (d_lineWidth_nm == 0) {
    return minYZeroWidth();
  }

  list<PatternPoint>::iterator point = d_points.begin();
  double result = (*point).d_y;

  if (d_numPoints == 1) {
    return result;
  }
  
  if (d_sidePointsNeedUpdate) {
    computeSidePoints();
  }

  int i;
  for (i = 0; i < d_numPoints; i++) {
    if (d_leftSidePoints[i].d_y < result) {
      result = d_leftSidePoints[i].d_y;
    }
    if (d_rightSidePoints[i].d_y < result) {
      result = d_rightSidePoints[i].d_y;
    }
  }
  return result;
}

double PolylinePatternShape::maxX()
{
  if (d_numPoints == 0) return 0.0;

  if (d_lineWidth_nm == 0) {
    return maxXZeroWidth();
  }

  list<PatternPoint>::iterator point = d_points.begin();
  double result = (*point).d_x;

  if (d_numPoints == 1) {
    return result;
  }
  
  if (d_sidePointsNeedUpdate) {
    computeSidePoints();
  }

  int i;
  for (i = 0; i < d_numPoints; i++) {
    if (d_leftSidePoints[i].d_x > result) {
      result = d_leftSidePoints[i].d_x;
    }
    if (d_rightSidePoints[i].d_x > result) {
      result = d_rightSidePoints[i].d_x;
    }
  }
  return result;
}

double PolylinePatternShape::maxY()
{
  if (d_numPoints == 0) return 0.0;

  if (d_lineWidth_nm == 0) {
    return maxYZeroWidth();
  }

  list<PatternPoint>::iterator point = d_points.begin();
  double result = (*point).d_y;

  if (d_numPoints == 1) {
    return result;
  }
  
  if (d_sidePointsNeedUpdate) {
    computeSidePoints();
  }

  int i;
  for (i = 0; i < d_numPoints; i++) {
    if (d_leftSidePoints[i].d_y > result) {
      result = d_leftSidePoints[i].d_y;
    }
    if (d_rightSidePoints[i].d_y > result) {
      result = d_rightSidePoints[i].d_y;
    }
  }
  return result;
}

vrpn_bool PolylinePatternShape::minLinearExposure(
                double &exposure_pCoul_per_cm)
{
  if (d_lineWidth_nm == 0) {
    exposure_pCoul_per_cm = d_exposure_pCoulombs_per_cm;
    return vrpn_TRUE;
  } else {
    return vrpn_FALSE;
  }
}

vrpn_bool PolylinePatternShape::minAreaExposure(
                double &exposure_uCoul_per_sq_cm)
{
  if (d_lineWidth_nm == 0) {
    return vrpn_FALSE;
  } else {
    exposure_uCoul_per_sq_cm = d_exposure_uCoulombs_per_square_cm;
    return vrpn_TRUE;
  }
}

void PolylinePatternShape::setExposure(double linearExposure, 
                                            double areaExposure)
{
  d_exposure_pCoulombs_per_cm = linearExposure;
  d_exposure_uCoulombs_per_square_cm = areaExposure;
}

void PolylinePatternShape::getExposureLevels(list<double> &linearLevels,
                                   list<double> &areaLevels)
{
  linearLevels.clear();
  areaLevels.clear();
  if (d_lineWidth_nm == 0) {
    linearLevels.push_back(d_exposure_pCoulombs_per_cm);
  } else {
    areaLevels.push_back(d_exposure_uCoulombs_per_square_cm);
  }
}

void PolylinePatternShape::setLineWidth(double width_nm)
{
  d_lineWidth_nm = width_nm;
}

double PolylinePatternShape::getLineWidth()
{
  return d_lineWidth_nm;
}

void PolylinePatternShape::setPoints(list<PatternPoint> &points)
{
  d_points = points;
  d_numPoints = d_points.size();
  d_sidePointsNeedUpdate = vrpn_TRUE;
}

void PolylinePatternShape::getPoint(int index, double &x, double &y)
{
  list<PatternPoint>::iterator point = d_points.begin();
  int i;
  for (i = 0; i < index; i++) 
     point++;
  x = (*point).d_x;
  y = (*point).d_y;
}

void PolylinePatternShape::setPoint(int index, double x, double y)
{
  list<PatternPoint>::iterator point = d_points.begin();
  int i;
  for (i = 0; i < index; i++) {
     point++;
  }
  (*point) = PatternPoint(x, y);
  d_sidePointsNeedUpdate = vrpn_TRUE;
}

void PolylinePatternShape::addPoint(double x, double y)
{
  d_points.push_back(PatternPoint(x,y));
  d_numPoints++;
  d_sidePointsNeedUpdate = vrpn_TRUE;
}

void PolylinePatternShape::removePoint()
{
  d_points.pop_back();
  d_numPoints--;
  d_sidePointsNeedUpdate = vrpn_TRUE;
}

void PolylinePatternShape::clearPoints()
{
  d_points.clear();
  d_numPoints = 0;
  d_sidePointsNeedUpdate = vrpn_TRUE;
}

void PolylinePatternShape::computeSidePoints()
{
  if (!d_sidePointsNeedUpdate) {
    return;
  }
  if (d_leftSidePoints) {
    delete [] d_leftSidePoints;
    d_leftSidePoints = NULL;
  }
  if (d_rightSidePoints) {
    delete [] d_rightSidePoints;
    d_rightSidePoints = NULL;
  }
  if (d_numPoints == 0) {
    return;
  }
  d_leftSidePoints = new PatternPoint[d_numPoints];
  d_rightSidePoints = new PatternPoint[d_numPoints];

  if (d_numPoints <= 1) return;

  list<PatternPoint>::iterator segmentStart, segmentEnd, nextSegmentEnd;
  segmentStart = d_points.begin();
  segmentEnd = segmentStart;
  segmentEnd++;
  nextSegmentEnd = segmentEnd;
  nextSegmentEnd++;

  double segmentLength;

  double widthCorrection;
  // offsets used to generate new start and end points for individual lines
  double sideVec_x, sideVec_y;

  double currSegment_dx, currSegment_dy;
  double lastSegment_dx, lastSegment_dy;
  double avgSegment_dx, avgSegment_dy;
  double halfWidth = 0.5*d_lineWidth_nm;
  int i;

  double cent_x, cent_y;
  currSegment_dx = (*segmentEnd).d_x - (*segmentStart).d_x;
  currSegment_dy = (*segmentEnd).d_y - (*segmentStart).d_y;
  segmentLength = sqrt(currSegment_dx*currSegment_dx + 
                       currSegment_dy*currSegment_dy);
  currSegment_dx /= segmentLength;
  currSegment_dy /= segmentLength;

  sideVec_x = -halfWidth*currSegment_dy;
  sideVec_y = halfWidth*currSegment_dx;
  cent_x = (*segmentStart).d_x;
  cent_y = (*segmentStart).d_y;

  d_leftSidePoints[0] = PatternPoint(cent_x + sideVec_x, cent_y + sideVec_y);
  d_rightSidePoints[0] = PatternPoint(cent_x - sideVec_x, cent_y - sideVec_y);

  int numSidePointsComputed = 1;

  while (segmentEnd != d_points.end()) {
    if (nextSegmentEnd == d_points.end()) { // we're near the end
      sideVec_x = -halfWidth*currSegment_dy;
      sideVec_y = halfWidth*currSegment_dx;
    } else { // do what happens in the middle parts (something complicated)
      lastSegment_dx = currSegment_dx;
      lastSegment_dy = currSegment_dy;
      currSegment_dx = (*nextSegmentEnd).d_x - (*segmentEnd).d_x;
      currSegment_dy = (*nextSegmentEnd).d_y - (*segmentEnd).d_y;
      segmentLength = sqrt(currSegment_dx*currSegment_dx + 
                           currSegment_dy*currSegment_dy);
      currSegment_dx /= segmentLength;
      currSegment_dy /= segmentLength;
      avgSegment_dx = 0.5*(currSegment_dx + lastSegment_dx);
      avgSegment_dy = 0.5*(currSegment_dy + lastSegment_dy);
      sideVec_x = -halfWidth*avgSegment_dy;
      sideVec_y = halfWidth*avgSegment_dx;

      widthCorrection = (halfWidth)/(sideVec_x*(-lastSegment_dy) + 
                                       sideVec_y*lastSegment_dx);
/*
      if (widthCorrection > 2.0) widthCorrection = 2.0;
*/
      sideVec_x *= widthCorrection;
      sideVec_y *= widthCorrection;

    }
    cent_x = (*segmentEnd).d_x;
    cent_y = (*segmentEnd).d_y;
    
    d_leftSidePoints[numSidePointsComputed] = 
            PatternPoint(cent_x + sideVec_x, cent_y + sideVec_y);
    d_rightSidePoints[numSidePointsComputed] = 
            PatternPoint(cent_x - sideVec_x, cent_y - sideVec_y);
    numSidePointsComputed++;

    segmentStart = segmentEnd;
    segmentEnd = nextSegmentEnd;
    nextSegmentEnd++;
  }
  d_sidePointsNeedUpdate = vrpn_FALSE;
}

void PolylinePatternShape::drawToDisplayZeroWidth(double units_per_pixel_x,
                               double units_per_pixel_y,
                               PatternShapeColorMap &map)
{
  if (d_points.empty()) return;

  double color[3];
  map.linearExposureColor(d_exposure_pCoulombs_per_cm, color);

  double x, y;
  double x_max, y_min;
  glLineWidth(1);
  glColor4f(color[0], color[1], color[2], 1.0);

  list<PatternPoint>::iterator pntIter = d_points.begin();

  x = (*pntIter).d_x;
  y = (*pntIter).d_y;
  x_max = x;
  y_min = y;
  pntIter++;
  if (pntIter == d_points.end()) {
    glBegin(GL_POINTS);
    glVertex3f(x,y,0.0);
    glEnd();
  } else {
    glBegin(GL_LINE_STRIP);
    glVertex3f(x,y,0.0);
    while (pntIter != d_points.end()) {
      x = (*pntIter).d_x;
      y = (*pntIter).d_y;
      if (x > x_max) {
        x_max = x;
        y_min = y;
      }
      glVertex3f(x,y,0.0);
      pntIter++;
    }
    glEnd();
  }
}

void PolylinePatternShape::generateExposurePointsZeroWidth(
        nmm_Microscope_SEM_EDAX *sem,
        double current, double dotSpacing, double lineSpacing,
        int &numPoints, double &expTime)
{
  numPoints = 0;
  expTime = 0.0;
  if (d_numPoints == 0) return;

  list<PatternPoint>::iterator segmentStart, segmentEnd;
  segmentStart = d_points.begin();
  segmentEnd = segmentStart;
  segmentEnd++;

  double invDotSpacing = 1.0/dotSpacing;
  double exposure = d_exposure_pCoulombs_per_cm;
  int x_DAC, y_DAC;
  double dwellTime_nsec =
        1e9*ExposureUtil::computeLineDwellTime(dotSpacing, current, exposure);

  if (sem) {
    sem->setPointDwellTime((int)floor(dwellTime_nsec), vrpn_FALSE);
  }


  double segmentLength;
  int numSegmentPoints;
  double x_nm, y_nm;
  double delta_x, delta_y;
  double incr_x, incr_y;
  while (segmentEnd != d_points.end()) {
    delta_x = (*segmentEnd).d_x - (*segmentStart).d_x;
    delta_y = (*segmentEnd).d_y - (*segmentStart).d_y;
    segmentLength = sqrt(delta_x*delta_x + delta_y*delta_y);
    incr_x = delta_x*dotSpacing/segmentLength;
    incr_y = delta_y*dotSpacing/segmentLength;
    numSegmentPoints =
            (int)floor(segmentLength*invDotSpacing);
    x_nm = (*segmentStart).d_x;
    y_nm = (*segmentStart).d_y;
    int i;
    for (i = 0; i < numSegmentPoints; i++) {
      if (sem) {
        sem->convert_nm_to_DAC(x_nm, y_nm, x_DAC, y_DAC);
        sem->goToPoint(x_DAC, y_DAC, PS_POLYLINE);
      }
      x_nm += incr_x;
      y_nm += incr_y;
      numPoints++;
    }
    segmentStart = segmentEnd;
    segmentEnd++;
  }
  // draw the last point in the polyline
  x_nm = (*segmentStart).d_x;
  y_nm = (*segmentStart).d_y;
  if (sem) {
    sem->convert_nm_to_DAC(x_nm, y_nm, x_DAC, y_DAC);
    sem->goToPoint(x_DAC, y_DAC, PS_POLYLINE);
  }
  numPoints++;
  expTime = dwellTime_nsec*1e-9*numPoints;
}

double PolylinePatternShape::minXZeroWidth()
{
  if (d_points.empty()) return 0.0;

  double result;
  list<PatternPoint>::iterator point;
  point = d_points.begin();
  result = (*point).d_x;
  while (point != d_points.end()) {
    if ((*point).d_x < result) {
      result = (*point).d_x;
    }
    point++;
  }
  return result;
}

double PolylinePatternShape::minYZeroWidth()
{
  if (d_points.empty()) return 0.0;

  double result;
  list<PatternPoint>::iterator point;
  point = d_points.begin();
  result = (*point).d_y;
  while (point != d_points.end()) {
    if ((*point).d_y < result) {
      result = (*point).d_y;
    }
    point++;
  }
  return result;
}

double PolylinePatternShape::maxXZeroWidth()
{
  if (d_points.empty()) return 0.0;

  double result;
  list<PatternPoint>::iterator point;
  point = d_points.begin();
  result = (*point).d_x;
  while (point != d_points.end()) {
    if ((*point).d_x > result) {
      result = (*point).d_x;
    }
    point++;
  }
  return result;
}

double PolylinePatternShape::maxYZeroWidth()
{
  if (d_points.empty()) return 0.0;

  double result;
  list<PatternPoint>::iterator point;
  point = d_points.begin();
  result = (*point).d_y;
  while (point != d_points.end()) {
    if ((*point).d_y > result) {
      result = (*point).d_y;
    }
    point++;
  }
  return result;
}

PolygonPatternShape::PolygonPatternShape(double area_dose):
  PatternShape(PS_POLYGON),
  d_numPoints(0.0),
  d_exposure_uCoulombs_per_square_cm(area_dose)
{
}

PolygonPatternShape::PolygonPatternShape(const PolygonPatternShape &pps):
  PatternShape(PS_POLYGON),
  d_numPoints(pps.d_numPoints),
  d_exposure_uCoulombs_per_square_cm(pps.d_exposure_uCoulombs_per_square_cm),
  d_points(pps.d_points)
{

}

void PolygonPatternShape::drawToDisplay(double units_per_pixel_x,
                               double units_per_pixel_y,
                               PatternShapeColorMap &map)
{
  if (d_points.empty()) return;


  double color[3];
  map.areaExposureColor(d_exposure_uCoulombs_per_square_cm, color);
  float x, y;
  glLineWidth(1);
  glColor4f(color[0], color[1], color[2], 1.0);

  list<PatternPoint>::iterator pntIter = d_points.begin();

  x = (*pntIter).d_x;
  y = (*pntIter).d_y;
  pntIter++;
  if (pntIter == d_points.end()) {
    glBegin(GL_POINTS);
    glVertex3f(x,y,0.0);
    glEnd();
  } else {
    glBegin(GL_LINE_LOOP);
    glVertex3f(x,y,0.0);
    while (pntIter != d_points.end()) {
      x = (*pntIter).d_x;
      y = (*pntIter).d_y;
      glVertex3f(x,y,0.0);
      pntIter++;
    }
    glEnd();
  }
}

void PolygonPatternShape::drawToSEM(nmm_Microscope_SEM_Remote *sem)
{
  if (d_numPoints == 0) return;

  vrpn_float32 *x_nm, *y_nm;
  x_nm = new vrpn_float32[d_numPoints];
  y_nm = new vrpn_float32[d_numPoints];

  list<PatternPoint>::iterator point;
  int i = 0;
  for (point = d_points.begin(); point != d_points.end(); point++) {
    x_nm[i] = (*point).d_x;
    y_nm[i] = (*point).d_y;
    i++;
  }

  sem->addPolygon(d_exposure_uCoulombs_per_square_cm, d_numPoints, x_nm, y_nm);
}

void PolygonPatternShape::drawToSEM(nmm_Microscope_SEM_EDAX *sem,
      double current, double dotSpacing, double lineSpacing,
      int &numPoints, double &expTime)
{
  generateExposurePoints(sem, current, dotSpacing, lineSpacing,
                         numPoints, expTime);
}

void PolygonPatternShape::computeExposureStatistics(int &numPoints, 
   double &expTime,
   double current, double dotSpacing, double lineSpacing)
{
  generateExposurePoints(NULL, current, dotSpacing, lineSpacing,
                         numPoints, expTime);
}

void PolygonPatternShape::generateExposurePoints(nmm_Microscope_SEM_EDAX *sem,
      double current, double dotSpacing, double lineSpacing,
      int &numPoints, double &expTime)
{
  numPoints = 0;
  expTime = 0.0;

  double invDotSpacing = 1.0/dotSpacing;
  double invLineSpacing = 1.0/lineSpacing;
  double exposure = d_exposure_uCoulombs_per_square_cm;
  int x_DAC, y_DAC;
  double dwellTime_nsec =
        1e9*ExposureUtil::computeAreaDwellTime(dotSpacing, lineSpacing,
                                              current, exposure);

  if (sem) {
    sem->setPointDwellTime((int)floor(dwellTime_nsec), vrpn_FALSE);
  }

  list<EdgeTableEntry> *edgeTable; // ith element corresponds to the scanline
                  // at y= d_polygonMinY + i*lineSpacing
  int currPolygonScanline, numPolygonScanlines;
  double polygonMinYScan;
  list<EdgeTableEntry> activeEdgeTable;
  list<EdgeTableEntry>::iterator activeEdgeBegin, activeEdgeEnd;

  vrpn_bool swapOrder;
  double fymin, fymax;
  int ymin, ymax;
  double xmin;
  double deltaX;
  EdgeTableEntry ete;

  double deltaYTotal = maxY() - minY();

  numPolygonScanlines = (int)ceil(deltaYTotal*invLineSpacing)+1;
  polygonMinYScan = minY();
  edgeTable = new list<EdgeTableEntry>[numPolygonScanlines];

  list<PatternPoint>::iterator p0 = d_points.begin();
  list<PatternPoint>::iterator p1 = p0;

  p1++;
  if (p0 == d_points.end() ||
      p1 == d_points.end()) {
    return;
  }
  while (p0 != d_points.end()) {
    if (p1 == d_points.end()) {
      p1 = d_points.begin();
    }
    if ((*p0).d_y < (*p1).d_y) {
      fymin = (*p0).d_y;
      fymax = (*p1).d_y;
      xmin = (*p0).d_x;
      swapOrder = vrpn_FALSE;
    } else {
      fymin = (*p1).d_y;
      fymax = (*p0).d_y;
      xmin = (*p1).d_x;
      swapOrder = vrpn_TRUE;
    }
    ymin = (int)floor((fymin-polygonMinYScan)*invLineSpacing);
    ymax = (int)floor((fymax-polygonMinYScan)*invLineSpacing);
    if (ymax == ymin) {
      // horizontal line, don't do anything
    } else {
      deltaX = lineSpacing*((*p1).d_x-(*p0).d_x)/(fymax-fymin);
      if (swapOrder) {
         deltaX *= -1;
      }
      ete = EdgeTableEntry(ymax, xmin, deltaX);
      // insert this at ymin
      assert(ymin <= numPolygonScanlines);
      edgeTable[ymin].push_back(ete);
    }
    p0++;
    p1++;
  }
  int i;
  for (i = 0; i < numPolygonScanlines; i++) {
     edgeTable[i].sort();
  }
  currPolygonScanline = 0;
  activeEdgeTable = edgeTable[0];
  activeEdgeBegin = activeEdgeTable.begin();
  activeEdgeEnd = activeEdgeBegin;
  activeEdgeEnd++;

  double x_nm, y_nm;
  x_nm = (*activeEdgeBegin).d_xMin;
  y_nm = polygonMinYScan;

  // expose the point
  if (sem) {
    sem->convert_nm_to_DAC(x_nm, y_nm, x_DAC, y_DAC);
    sem->goToPoint(x_DAC, y_DAC, PS_POLYGON);
  }
  numPoints++;

  // start exposing points
  while (!((currPolygonScanline == (numPolygonScanlines-1) &&
           activeEdgeBegin == activeEdgeTable.end()) ||
           activeEdgeTable.empty())) {

    // check if we need to step to the next segment in this scanline
    if (x_nm > (*activeEdgeEnd).d_xMin){ // need to step to next segment
      activeEdgeBegin = activeEdgeEnd;
      activeEdgeBegin++;
      activeEdgeEnd = activeEdgeBegin;
      activeEdgeEnd++;

      // check if we're done with the scanline and if so
      // increment to the next scanline and update the active edge table
      if (activeEdgeBegin == activeEdgeTable.end()) {
        currPolygonScanline++;

        // remove edges with yMax == currPolygonScanline
        list<EdgeTableEntry>::iterator test = activeEdgeTable.begin();
        list<EdgeTableEntry>::iterator victim;
        while (test != activeEdgeTable.end()) {
          if ((*test).d_yMax == currPolygonScanline) {
              victim = test;
              test++;
              activeEdgeTable.remove(*victim);
          } else {
              (*test).d_xMin += (*test).d_deltaX;
              test++;
          }
        }

        // add edges in edgeTable[currPolygonScanline] to the active edge
        // table
        activeEdgeTable.merge(edgeTable[currPolygonScanline]);
        activeEdgeBegin = activeEdgeTable.begin();
        activeEdgeEnd = activeEdgeBegin;
        activeEdgeEnd++;
      } // done updating the active edge table

      // the next point will be ((activeEdgeBegin).d_xMin,
      // currPolygonScanline*lineSpacing + polygonMinYScan)
      if (!activeEdgeTable.empty()){
        x_nm = (*activeEdgeBegin).d_xMin;
        y_nm = polygonMinYScan + currPolygonScanline*lineSpacing;
        if (sem) {
          sem->convert_nm_to_DAC(x_nm, y_nm, x_DAC, y_DAC);
          sem->goToPoint(x_DAC, y_DAC, PS_POLYGON);
        }
        numPoints++;
      }
    } else { // just step along in the same segment of the same scanline
      // increment nextExposePoint.d_x
      x_nm += dotSpacing;
      if (sem) {
        sem->convert_nm_to_DAC(x_nm, y_nm, x_DAC, y_DAC);
        sem->goToPoint(x_DAC, y_DAC, PS_POLYGON);
      }
      numPoints++;
    }
  }
  expTime = numPoints*dwellTime_nsec*1e-9;
}

double PolygonPatternShape::minX()
{
  if (d_points.empty()) return 0.0;

  double result;
  list<PatternPoint>::iterator point;
  point = d_points.begin();
  result = (*point).d_x;
  while (point != d_points.end()) {
    if ((*point).d_x < result) {
      result = (*point).d_x;
    }
    point++;
  }
  return result;
}

double PolygonPatternShape::minY()
{
  if (d_points.empty()) return 0.0;

  double result;
  list<PatternPoint>::iterator point;
  point = d_points.begin();
  result = (*point).d_y;
  while (point != d_points.end()) {
    if ((*point).d_y < result) {
      result = (*point).d_y;
    }
    point++;
  }
  return result;
}

double PolygonPatternShape::maxX()
{
  if (d_points.empty()) return 0.0;

  double result;
  list<PatternPoint>::iterator point;
  point = d_points.begin();
  result = (*point).d_x;
  while (point != d_points.end()) {
    if ((*point).d_x > result) {
      result = (*point).d_x;
    }
    point++;
  }
  return result;
}

double PolygonPatternShape::maxY()
{
  if (d_points.empty()) return 0.0;

  double result;
  list<PatternPoint>::iterator point;
  point = d_points.begin();
  result = (*point).d_y;
  while (point != d_points.end()) {
    if ((*point).d_y > result) {
      result = (*point).d_y;
    }
    point++;
  }
  return result;
}

vrpn_bool PolygonPatternShape::minLinearExposure(double &exposure_pCoul_per_cm)
{
  return vrpn_FALSE;
}

vrpn_bool PolygonPatternShape::minAreaExposure(double &exposure_uCoul_per_sq_cm)
{
  exposure_uCoul_per_sq_cm = d_exposure_uCoulombs_per_square_cm;
  return vrpn_TRUE;
}

void PolygonPatternShape::setExposure(double linExposure, double areaExposure)
{
  d_exposure_uCoulombs_per_square_cm = areaExposure;
}

void PolygonPatternShape::getExposureLevels(list<double> &linearLevels,
                                   list<double> &areaLevels)
{
  linearLevels.clear();
  areaLevels.clear();
  areaLevels.push_back(d_exposure_uCoulombs_per_square_cm);
}

void PolygonPatternShape::setPoints(list<PatternPoint> &points)
{
  d_points = points;
  d_numPoints = d_points.size();
}

void PolygonPatternShape::getPoint(int index, double &x, double &y)
{
  list<PatternPoint>::iterator point = d_points.begin();
  int i;
  for (i = 0; i < index; i++)
     point++;
  x = (*point).d_x;
  y = (*point).d_y;
}

void PolygonPatternShape::setPoint(int index, double x, double y)
{
  list<PatternPoint>::iterator point = d_points.begin();
  int i;
  for (i = 0; i < index; i++)
     point++;
  (*point) = PatternPoint(x, y);
}

void PolygonPatternShape::addPoint(double x, double y)
{
  d_points.push_back(PatternPoint(x,y));
  d_numPoints++;
}

void PolygonPatternShape::removePoint() {
  d_points.pop_back();
  d_numPoints--;
}

void PolygonPatternShape::clearPoints() {
  d_points.clear();
  d_numPoints = 0;
}

CompositePatternShape::CompositePatternShape():
  PatternShape(PS_COMPOSITE)
{
}

CompositePatternShape::CompositePatternShape(const CompositePatternShape &cps):
  PatternShape(PS_COMPOSITE),
  d_subShapes(cps.d_subShapes)
{
}

void CompositePatternShape::drawToDisplay(double units_per_pixel_x,
                               double units_per_pixel_y,
                               PatternShapeColorMap &color)
{
  list<PatternShapeListElement>::iterator shape;
  for (shape = d_subShapes.begin(); shape != d_subShapes.end(); shape++) {
    (*shape).drawToDisplay(units_per_pixel_x, units_per_pixel_y, color);
  }
}

void CompositePatternShape::drawToSEM(nmm_Microscope_SEM_Remote *sem)
{
  list<PatternShapeListElement>::iterator shape;
  for (shape = d_subShapes.begin(); shape != d_subShapes.end(); shape++) {
    (*shape).drawToSEM(sem);
  }
}

void CompositePatternShape::drawToSEM(nmm_Microscope_SEM_EDAX *sem,
      double current, double dotSpacing, double lineSpacing,
      int &numPoints, double &expTime)
{
  int numShapePoints;
  double shapeExpTime;
  numPoints = 0;
  expTime = 0.0;

  list<PatternShapeListElement>::iterator shape;
  for (shape = d_subShapes.begin(); shape != d_subShapes.end(); shape++) {
    (*shape).drawToSEM(sem, current, dotSpacing, lineSpacing,
                       numShapePoints, shapeExpTime);
    numPoints += numShapePoints;
    expTime += shapeExpTime;
  }
}

void CompositePatternShape::computeExposureStatistics(int &numPoints, 
           double &expTime,
           double current, double dotSpacing, double lineSpacing)
{
  int numShapePoints;
  double shapeExpTime;
  numPoints = 0;
  expTime = 0.0;
  list<PatternShapeListElement>::iterator shape;
  for (shape = d_subShapes.begin(); shape != d_subShapes.end(); shape++) {
    (*shape).computeExposureStatistics(numShapePoints, shapeExpTime, 
                                       current, dotSpacing, lineSpacing);
    numPoints += numShapePoints;
    expTime += shapeExpTime;
  }
}

double CompositePatternShape::minX()
{
  if (d_subShapes.empty()) return 0.0;

  double result;
  double test;
  list<PatternShapeListElement>::iterator shape;
  shape = d_subShapes.begin();
  result = (*shape).minX();
  shape++;
  while (shape != d_subShapes.end()) {
    test = (*shape).minX();
    if (test < result) {
      result = test;
    }
    shape++;
  }
  return result;
}

double CompositePatternShape::minY()
{
  if (d_subShapes.empty()) return 0.0;

  double result;
  double test;
  list<PatternShapeListElement>::iterator shape;
  shape = d_subShapes.begin();
  result = (*shape).minY();
  shape++;
  while (shape != d_subShapes.end()) {
    test = (*shape).minY();
    if (test < result) {
      result = test;
    }
    shape++;
  }
  return result;
}

double CompositePatternShape::maxX()
{
  if (d_subShapes.empty()) return 0.0;

  double result;
  double test;
  list<PatternShapeListElement>::iterator shape;
  shape = d_subShapes.begin();
  result = (*shape).maxX();
  shape++;
  while (shape != d_subShapes.end()) {
    test = (*shape).maxX();
    if (test > result) {
      result = test;
    }
    shape++;
  }
  return result;
}

double CompositePatternShape::maxY()
{
  if (d_subShapes.empty()) return 0.0;

  double result;
  double test;
  list<PatternShapeListElement>::iterator shape;
  shape = d_subShapes.begin();
  result = (*shape).maxY();
  shape++;
  while (shape != d_subShapes.end()) {
    test = (*shape).maxY();
    if (test > result) {
      result = test;
    }
    shape++;
  }
  return result;
}

vrpn_bool CompositePatternShape::minLinearExposure(
                         double &exposure_pCoul_per_cm)
{
  vrpn_bool foundLinExposure = vrpn_FALSE;
  double minResult, testResult;
  list<PatternShapeListElement>::iterator shape;
  for (shape = d_subShapes.begin(); shape != d_subShapes.end(); shape++) {
    vrpn_bool hasLinExposure = (*shape).minLinearExposure(testResult);
    if (hasLinExposure) {
      if (!foundLinExposure || (testResult < minResult)) {
        minResult = testResult;
        foundLinExposure = vrpn_TRUE;
      }
    }
  }
  if (foundLinExposure) {
    exposure_pCoul_per_cm = minResult;
  }
  return foundLinExposure;
}

vrpn_bool CompositePatternShape::minAreaExposure(
                         double &exposure_uCoul_per_sq_cm)
{
  vrpn_bool foundAreaExposure = vrpn_FALSE;
  double minResult, testResult;
  list<PatternShapeListElement>::iterator shape;
  for (shape = d_subShapes.begin(); shape != d_subShapes.end(); shape++) {
    vrpn_bool hasAreaExposure = (*shape).minAreaExposure(testResult);
    if (hasAreaExposure) {
      if (!foundAreaExposure || (testResult < minResult)) {
        minResult = testResult;
        foundAreaExposure = vrpn_TRUE;
      }
    }
  }
  if (foundAreaExposure) {
    exposure_uCoul_per_sq_cm = minResult;
  }
  return foundAreaExposure;
}

void CompositePatternShape::setExposure(double linearExposure, 
                                        double areaExposure)
{
  list<PatternShapeListElement>::iterator shape;
  for (shape = d_subShapes.begin(); shape != d_subShapes.end(); shape++) {
    (*shape).setExposure(linearExposure, areaExposure);
  }
}

void CompositePatternShape::getExposureLevels(list<double> &linearLevels,
                                   list<double> &areaLevels)
{
  linearLevels.clear();
  areaLevels.clear();

  list<double> shapeLinLevels, shapeAreaLevels;
  list<PatternShapeListElement>::iterator shape;
  for (shape = d_subShapes.begin(); shape != d_subShapes.end(); shape++) {
    (*shape).getExposureLevels(shapeLinLevels, shapeAreaLevels);
    linearLevels.merge(shapeLinLevels);
    areaLevels.merge(shapeAreaLevels);
  }
}

DumpPointPatternShape::DumpPointPatternShape(double x_nm, double y_nm):
  PatternShape(PS_DUMP),
  d_location(x_nm, y_nm),
  d_dwellTime_sec(1e-3)
{
}

DumpPointPatternShape::DumpPointPatternShape(const DumpPointPatternShape &dpps):
  PatternShape(PS_DUMP),
  d_location(dpps.d_location),
  d_dwellTime_sec(dpps.d_dwellTime_sec)
{
}

/* we ignore the color map parameter here since the exposure (which we
   are assuming maps to color) is irrelevant for a dump point
*/
void DumpPointPatternShape::drawToDisplay(double units_per_pixel_x,
                               double units_per_pixel_y,
                               PatternShapeColorMap &color)
{
  glColor4f(1.0, 1.0, 0.0, 1.0);

  double size = 1.5;
  float xmin, xmax, ymin, ymax;

  xmin = d_location.d_x - size*units_per_pixel_x;
  ymin = d_location.d_y - size*units_per_pixel_y;
  xmax = d_location.d_x + size*units_per_pixel_x;
  ymax = d_location.d_y + size*units_per_pixel_y;
  float x[4] = {xmin, xmin, xmax, xmax};
  float y[4] = {ymin, ymax, ymax, ymin};
  glBegin(GL_LINE_LOOP);
    glVertex3f(x[0], y[0], 0.0);
    glVertex3f(x[1], y[1], 0.0);
    glVertex3f(x[2], y[2], 0.0);
    glVertex3f(x[3], y[3], 0.0);
  glEnd();
}

void DumpPointPatternShape::drawToSEM(nmm_Microscope_SEM_Remote *sem)
{
  sem->addDumpPoint(d_location.d_x, d_location.d_y);
}

void DumpPointPatternShape::drawToSEM(nmm_Microscope_SEM_EDAX *sem,
     double current, double dotSpacing, double lineSpacing,
     int &numPoints, double &expTime)
{
  int x_DAC, y_DAC;
  double dwellTime_nsec = 1e9*d_dwellTime_sec;
  if (sem) {
    sem->setPointDwellTime((int)floor(dwellTime_nsec), vrpn_FALSE);
    sem->convert_nm_to_DAC(d_location.d_x, d_location.d_y, x_DAC, y_DAC);
    sem->goToPoint(x_DAC, y_DAC, PS_DUMP);
  }
}

void DumpPointPatternShape::computeExposureStatistics(int &numPoints, 
      double &expTime,
      double current, double dotSpacing, double lineSpacing)
{
  numPoints = 1;
  expTime = d_dwellTime_sec;
}

double DumpPointPatternShape::minX()
{
  return d_location.d_x;
}

double DumpPointPatternShape::minY()
{
  return d_location.d_y;
}

double DumpPointPatternShape::maxX()
{
  return d_location.d_x;
}

double DumpPointPatternShape::maxY()
{
  return d_location.d_y;
}

