#include "exposurePattern.h"
#include "GL/gl.h"
#include "math.h"

int PatternShape::s_nextID = 0;

PatternShape::PatternShape(double lw, double exp,
                          ShapeType type): d_ID(s_nextID),
             d_lineWidth_nm(lw),
             d_exposure_uCoulombs_per_square_cm(exp),
             d_type(type), d_trans_x(0.0), d_trans_y(0.0)
{
  s_nextID++;
}

PatternShape::PatternShape(const PatternShape &sh): d_ID(s_nextID),
   d_lineWidth_nm(sh.d_lineWidth_nm),
   d_exposure_uCoulombs_per_square_cm(sh.d_exposure_uCoulombs_per_square_cm),
   d_type(sh.d_type), d_trans_x(sh.d_trans_x), d_trans_y(sh.d_trans_y)
{
  d_points = sh.d_points;
  d_shapes = sh.d_shapes;
  s_nextID++;
}

void PatternShape::addPoint(double x, double y)
{
  d_points.push_back(PatternPoint(x, y));
}

void PatternShape::addSubShape(const PatternShape &sh)
{
  d_shapes.push_back(sh);
}

void PatternShape::removePoint()
{
  if (!d_points.empty())
    d_points.pop_back();
  return;
}

void PatternShape::clearPoints()
{
  d_points.clear();
}

void PatternShape::drawDumpPoint(double units_per_pixel_x,
                                 double units_per_pixel_y)
{
  if (d_points.empty()) return;
  glColor4f(1.0, 1.0, 0.0, 1.0);

  list<PatternPoint>::iterator pntIter = d_points.begin();

  double size = 1.5;
  float xmin, xmax, ymin, ymax;
  
  xmin = (*pntIter).d_x - size*units_per_pixel_x;
  ymin = (*pntIter).d_y - size*units_per_pixel_y;
  xmax = (*pntIter).d_x + size*units_per_pixel_x;
  ymax = (*pntIter).d_y + size*units_per_pixel_y;
  float x[4] = {xmin, xmin, xmax, xmax};
  float y[4] = {ymin, ymax, ymax, ymin};
  glBegin(GL_LINE_LOOP);
    glVertex3f(x[0], y[0], 0.0);
    glVertex3f(x[1], y[1], 0.0);
    glVertex3f(x[2], y[2], 0.0);
    glVertex3f(x[3], y[3], 0.0);
  glEnd();
}

void PatternShape::drawThinPolyline(double units_per_pixel_x,
                                    double units_per_pixel_y)
{
  if (d_points.empty()) return;

  double x, y;
  double x_max, y_min;
  glLineWidth(1);
  glColor4f(1.0, 0.0, 0.0, 1.0);

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

/*
  glColor4f(1.0, 1.0, 1.0, 1.0);
  glBegin(GL_LINE_STRIP);
  glVertex3f(x_max, y_min, 0.0);
  glVertex3f(x_max+units_per_pixel_x*10.0, y_min-units_per_pixel_y*10.0, 0.0);
  glEnd();
*/
}

void PatternShape::drawPolygon(double units_per_pixel_x,
                               double units_per_pixel_y)
{
  if (d_points.empty()) return;

  double x, y;
  glLineWidth(1);
  glColor4f(1.0, 0.0, 0.0, 1.0);

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


void PatternShape::drawThickPolyline(double units_per_pixel_x,
                                     double units_per_pixel_y)
{
  if (d_points.empty()) return;

  double x_start, y_start;
  double x0, y0, x1, y1, x2, y2;
  double lenA, lenB;
  double os_x0 = 0.0, os_y0 = 0.0, os_x1 = 0.0, os_y1 = 0.0;
  double dxA, dyA, dxB, dyB, dx_avg, dy_avg, lenAvg;
  double widthCorrection = 1.0;
  double cprod;

  glLineWidth(1);
  glColor4f(1.0, 0.0, 0.0, 1.0);

  list<PatternPoint>::iterator pntIter;
  pntIter = d_points.begin();
  x2 = (*pntIter).d_x;
  y2 = (*pntIter).d_y;
  x_start = x2;
  y_start = y2;
  pntIter++;
  if (pntIter == d_points.end()) {
    // just draw one point here
    glBegin(GL_POINTS);
    glVertex3f(x2, y2, 0.0);
    glEnd();
    return;
  } else {
    x1 = x2;
    y1 = y2;
    x2 = (*pntIter).d_x;
    y2 = (*pntIter).d_y;
    if (d_lineWidth_nm > 0 && d_type == PS_POLYLINE) {
      dxB = x2-x1; dyB = y2-y1;
      lenB = sqrt(dxB*dxB + dyB*dyB);
      os_x0 = -0.5*d_lineWidth_nm*dyB/lenB;
      os_y0 = 0.5*d_lineWidth_nm*dxB/lenB;
    }
    pntIter++;
  }
  while (pntIter != d_points.end()) {
    x0 = x1;
    y0 = y1;
    x1 = x2;
    y1 = y2;
    x2 = (*pntIter).d_x;
    y2 = (*pntIter).d_y;
    if (d_lineWidth_nm > 0 && d_type == PS_POLYLINE) {
      lenA = lenB;
      dxA = dxB, dyA = dyB;
      dxB = x2-x1; dyB = y2-y1;
      lenB = sqrt(dxB*dxB + dyB*dyB);
      dx_avg = 0.5*(dxA/lenA + dxB/lenB);
      dy_avg = 0.5*(dyA/lenA + dyB/lenB);
      lenAvg = sqrt(dx_avg*dx_avg + dy_avg*dy_avg);
      os_x1 = -0.5*d_lineWidth_nm*dy_avg/lenAvg;
      os_y1 = 0.5*d_lineWidth_nm*dx_avg/lenAvg;
      cprod = dxA*os_y1 - dyA*os_x1;
      if (cprod < 0) {
        os_x1 = -os_x1;
        os_y1 = -os_y1;
      }
      // project onto segment perpendicular - should get something equal to
      // 0.5*d_lineWidth_nm
      widthCorrection = (0.5*d_lineWidth_nm*lenA)/
                        (os_x1*(-dyA) + os_y1*dxA);
      if (widthCorrection > 2.0) widthCorrection = 2.0;
      os_x1 *= widthCorrection;
      os_y1 *= widthCorrection;

      // draw the segment from (x0,y0) to (x1,y1) with offsets os_x0,os_y0,
      // and os_x1, os_y1
/*
      printf("line: (%g,%g):(%g,%g) to (%g,%g):(%g,%g)\n",
              x0, y0, os_x0, os_y0, x1, y1, os_x1, os_y1);
*/
      glBegin(GL_LINE_LOOP);
      glVertex3f(x0+os_x0, y0+os_y0, 0.0);
      glVertex3f(x1+os_x1, y1+os_y1, 0.0);
      glVertex3f(x1-os_x1, y1-os_y1, 0.0);
      glVertex3f(x0-os_x0, y0-os_y0, 0.0);
      glEnd();
      os_x0 = os_x1;
      os_y0 = os_y1;

    } else {

      glBegin(GL_LINES);
      glVertex3f(x0, y0, 0.0);
      glVertex3f(x1, y1, 0.0);
      glEnd();
    }
    pntIter++;
  }

  if (d_lineWidth_nm > 0 && d_type == PS_POLYLINE) {
    os_x1 = -0.5*d_lineWidth_nm*dyB/lenB;
    os_y1 = 0.5*d_lineWidth_nm*dxB/lenB;

    // draw the segment from (x1,y1) to (x2,y2)
    glBegin(GL_LINE_LOOP);
    glVertex3f(x1+os_x0, y1+os_y0, 0.0);
    glVertex3f(x2+os_x1, y2+os_y1, 0.0);
    glVertex3f(x2-os_x1, y2-os_y1, 0.0);
    glVertex3f(x1-os_x0, y1-os_y0, 0.0);
    glEnd();

  } else {

    glBegin(GL_LINES);
    glVertex3f(x1, y1, 0.0);
    glVertex3f(x2, y2, 0.0);

    if (d_type == PS_POLYGON) {
      glVertex3f(x2, y2, 0.0);
      glVertex3f(x_start, y_start, 0.0);
    }
    glEnd();
  }
}

void PatternShape::draw(double units_per_pixel_x,
                        double units_per_pixel_y) {
  if (d_type == PS_POLYGON) {
    drawPolygon(units_per_pixel_x, units_per_pixel_y);
  } else if (d_type == PS_POLYLINE) {
    if (d_lineWidth_nm > 0) {
      drawThickPolyline(units_per_pixel_x, units_per_pixel_y);
    } else {
      drawThinPolyline(units_per_pixel_x, units_per_pixel_y);
    }
  } else if (d_type == PS_COMPOSITE){
    list<PatternShape>::iterator shape = d_shapes.begin();
    while (shape != d_shapes.end()) {
      (*shape).draw(units_per_pixel_x, units_per_pixel_y);
      shape++;
    }
  } else if (d_type == PS_DUMP){
    drawDumpPoint(units_per_pixel_x, units_per_pixel_y);
  }
}

list<PatternPoint>::iterator PatternShape::pointListBegin()
{
  return d_points.begin();
}

list<PatternPoint>::iterator PatternShape::pointListEnd()
{
  return d_points.end();
}

double PatternShape::minY()
{
  double result;
  if (d_type != PS_COMPOSITE) {
    if (d_points.empty()) return 0.0;
    list<PatternPoint>::iterator pnt = d_points.begin();
    result = (*pnt).d_y;
    pnt++;
    while (pnt != d_points.end()){
      if ((*pnt).d_y < result) {
        result = (*pnt).d_y;
      }
      pnt++;
    }
  } else {
    if (d_shapes.empty()) return 0.0;
    list<PatternShape>::iterator shape = d_shapes.begin();
    result = (*shape).minY();
    shape++;
    double test;
    while (shape != d_shapes.end()) {
      test = (*shape).minY();
      if (test < result) {
        result = test;
      }
    }
  }
  return result;
}

double PatternShape::maxY()
{
  double result;
  if (d_type != PS_COMPOSITE) {
    if (d_points.empty()) return 0.0;
    list<PatternPoint>::iterator pnt = d_points.begin();
    result = (*pnt).d_y;
    pnt++;
    while (pnt != d_points.end()){
      if ((*pnt).d_y > result) {
        result = (*pnt).d_y;
      }
      pnt++;
    }
  } else {
    if (d_shapes.empty()) return 0.0;
    list<PatternShape>::iterator shape = d_shapes.begin();
    result = (*shape).minY();
    shape++;
    double test;
    while (shape != d_shapes.end()) {
      test = (*shape).minY();
      if (test > result) {
        result = test;
      }
    }
  }
  return result;
}
