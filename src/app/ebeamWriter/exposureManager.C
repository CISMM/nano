#include "exposureManager.h"

#ifndef M_PI
#define M_PI           3.14159265358979323846
#endif

ExposureManager::ExposureManager():
  d_exposure_uCoul_per_cm2(0),
  d_fill_dwell_time_sec(0.000005),
  d_line_dwell_time_sec(0.000005),
  d_fill_inter_dot_dist_nm(50),
  d_line_inter_dot_dist_nm(50),

  d_min_exposure_per_point_uCoul_per_cm2(0),
  d_min_dwell_time_sec(0),
  d_nm_per_pixel(0),
  d_beam_width_nm(0),
  d_beam_current_picoAmps(0),

  d_line_overlap_factor(0.7),
  d_area_overlap_factor(0.7)
{

}

void ExposureManager::exposePattern(list<PatternShape> shapes,
                     list<PatternPoint> /*dump_points*/,
                     nmm_Microscope_SEM_Remote *sem, int mag)
{
  list<PatternShape>::iterator shape;
  list<PatternPoint>::iterator point;
  vrpn_bool firstTime = vrpn_TRUE;
  int i;
  double start_x_nm, start_y_nm, end_x_nm, end_y_nm;
  double start_x_DAC, start_y_DAC, end_x_DAC, end_y_DAC;
  int x_DAC, y_DAC;
  double dx, dy, delta_x, delta_y;
  double imageRegionWidth = 12.8e7/(double)(mag);
  vrpn_int32 xDACspan, yDACspan;
  sem->getMaxScan(xDACspan, yDACspan);

  double imageRegionHeight = imageRegionWidth*(double)yDACspan/
                                              (double)xDACspan;
  double dist;
  int numSteps;

  for (shape = shapes.begin();
       shape != shapes.end(); shape++) {
    for (point = (*shape).pointListBegin();
         point != (*shape).pointListEnd(); point++) {
      if (firstTime) {
         start_x_nm = (*point).d_x;
         start_y_nm = (*point).d_y;
         // convert from world to DAC units
         start_x_DAC = start_x_nm*(double)xDACspan/imageRegionWidth;
         start_y_DAC = start_y_nm*(double)yDACspan/imageRegionHeight;
         firstTime = vrpn_FALSE;
      } else {
         end_x_nm = (*point).d_x;
         end_y_nm = (*point).d_y;
         // convert from world to DAC units
         end_x_DAC = end_x_nm*(double)xDACspan/imageRegionWidth;
         end_y_DAC = end_y_nm*(double)yDACspan/imageRegionHeight;
         // now draw line going from start to end
         delta_x = end_x_nm - start_x_nm;
         delta_y = end_y_nm - start_y_nm;
         dist = sqrt(delta_x*delta_x + delta_y*delta_y);
         numSteps = dist/d_line_inter_dot_dist_nm;
         dx = (end_x_DAC - start_x_DAC)/(double)numSteps;
         dy = (end_y_DAC - start_y_DAC)/(double)numSteps;
         for (i = 0; i < numSteps; i++) {
           x_DAC = start_x_DAC + dx*i;
           y_DAC = start_y_DAC + dy*i;
           sem->goToPoint(x_DAC, y_DAC);
         }
         sem->goToPoint(0,0); // dump location
         sem->mainloop(); // do the line segment and wait at the dump location
                          // for additional commands
         start_x_DAC = end_x_DAC;
         start_y_DAC = end_y_DAC;
         start_x_nm = end_x_nm;
         start_y_nm = end_y_nm;
      }
    }
  }
}

void ExposureManager::setExposure(double uCoul_per_cm2)
{
  d_exposure_uCoul_per_cm2 = uCoul_per_cm2;

  double exposure_number = 
              d_exposure_uCoul_per_cm2/d_min_exposure_per_point_uCoul_per_cm2;

  // assuming we overlap the dots by some factor, 
  // how long should dwell time be?

  // use a one-dimensional overlap factor for lines 
  d_line_inter_dot_dist_nm = d_beam_width_nm/d_line_overlap_factor;
  // and a two-dimensional overlap factor for areas
  d_fill_inter_dot_dist_nm = 0.25*M_PI*d_beam_width_nm*d_beam_width_nm/
                             d_area_overlap_factor;
  d_fill_inter_dot_dist_nm = sqrt(d_fill_inter_dot_dist_nm);

  double line_time_factor = exposure_number/d_line_overlap_factor;
  double area_time_factor = exposure_number/d_area_overlap_factor;

  d_line_dwell_time_sec = line_time_factor*d_min_dwell_time_sec;
  d_fill_dwell_time_sec = area_time_factor*d_min_dwell_time_sec;
  
  // now we know the dwell time and the dot spacing 
  
}

void ExposureManager::setMinDwellTime(double t)
{
  d_min_dwell_time_sec = t;
  double beam_area_cm2 = 0.25*d_beam_width_nm*d_beam_width_nm*(1e-14)*M_PI;
  d_min_exposure_per_point_uCoul_per_cm2 = 
           (1e-6)*d_min_dwell_time_sec*d_beam_current_picoAmps/beam_area_cm2;

}

// setup for exposure of the specified shape
int ExposureManager::initShape(PatternShape &shape)
{
  if (shape.d_type != PS_POLYLINE || shape.d_lineWidth_nm > d_beam_width_nm){
    printf("polygons and thick polylines not implemented so don't\n"
           "expect to see anything\n");
    return -1;
  }
  d_currShape = new PatternShape(shape);
  double start_x, start_y;
  
  setExposure(d_currShape->d_exposure_uCoulombs_per_square_cm);
  
  d_pointListPtr = d_currShape->d_points.begin();
  start_x = (*d_pointListPtr).d_x;
  start_y = (*d_pointListPtr).d_y;
  d_pointListPtr++;

  d_nextExposePoint = PatternPoint(start_x, start_y);
  return 0;
}

// get the next exposure point for the current shape
int ExposureManager::getNextPoint(PatternPoint &point, double &time)
{
  if (d_currShape == NULL) return -1; // error, no shape being drawn

  point = d_nextExposePoint;
  time = d_line_dwell_time_sec;

  list<PatternPoint>::iterator testPtr = d_pointListPtr;
  testPtr++;
  if (testPtr == d_currShape->pointListEnd()) {
    delete d_currShape;
    d_currShape = NULL;
    return 1; // indicate we are done with the current shape
  }

  double end_x = (*d_pointListPtr).d_x;
  double end_y = (*d_pointListPtr).d_y;
  double dx = end_x - d_nextExposePoint.d_x;
  double dy = end_y - d_nextExposePoint.d_y;
  double distToEnd = sqrt(dx*dx + dy*dy);
  if (distToEnd < d_line_inter_dot_dist_nm) {
    // a little extra exposure at the corner but I don't think it will
    // be significant
    d_nextExposePoint = PatternPoint(end_x, end_y);
    d_pointListPtr++;
  } else {
    d_nextExposePoint.d_x += d_line_inter_dot_dist_nm*dx/distToEnd;
    d_nextExposePoint.d_y += d_line_inter_dot_dist_nm*dy/distToEnd;
  }
  return 0; // shape is in progress
}
