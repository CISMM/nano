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

  d_line_overlap_factor(2),
  d_area_overlap_factor(4)
{

}

void ExposureManager::convert_nm_to_DAC(
     const double x_nm, const double y_nm, int &xDAC, int &yDAC)
{
  xDAC = (int)floor(x_nm*(double)d_xSpan_DACunits/d_xSpan_nm);
  //yDAC = d_ySpan_DACunits - 
  yDAC = (int)floor(y_nm*(double)d_ySpan_DACunits/d_ySpan_nm);
  return;
}

void ExposureManager::exposePattern(list<PatternShape> shapes,
                     list<PatternPoint> dump_points,
                     nmm_Microscope_SEM_Remote *sem, int mag)
{
  list<PatternShape>::iterator shape;
  PatternPoint pnt_nm;
  double dwell_time_sec;
  double dwell_time_nsec;
  int x_DAC, y_DAC;

  sem->getScanRegion_nm(d_xSpan_nm, d_ySpan_nm);
  sem->getMaxScan(d_xSpan_DACunits, d_ySpan_DACunits);

  for (shape = shapes.begin();
       shape != shapes.end(); shape++) {
    initShape(*shape);
    while (getNextPoint(pnt_nm, dwell_time_sec)) {
      dwell_time_nsec = (vrpn_int32)floor(d_line_dwell_time_sec*1e9);
      sem->setPointDwellTime(dwell_time_nsec);
      convert_nm_to_DAC(pnt_nm.d_x, pnt_nm.d_y, x_DAC, y_DAC);
      sem->goToPoint(x_DAC, y_DAC);
    }
  }
}

/* this version is disabled for testing initShape and getNextPoint
void ExposureManager::exposePattern(list<PatternShape> shapes,
                     list<PatternPoint> dump_points,
                     nmm_Microscope_SEM_Remote *sem, int mag)
{
  list<PatternShape>::iterator shape;
  list<PatternPoint>::iterator point;
  vrpn_bool firstTime = vrpn_TRUE;
  int i;
  double dump_x_nm = 0, dump_y_nm = 0, dump_x_DAC = 0, dump_y_DAC = 0;
  double start_x_nm, start_y_nm, end_x_nm, end_y_nm;
  int x_DAC, y_DAC;
  double dx, dy, delta_x, delta_y;
  sem->getScanRegion_nm(d_xSpan_nm, d_ySpan_nm);
  sem->getMaxScan(d_xSpan_DACunits, d_ySpan_DACunits);
  vrpn_int32 dwell_time_nsec = 3000000;

  dump_x_DAC = d_xSpan_DACunits/2;
  dump_y_DAC = d_ySpan_DACunits/2;

  double dist;
  int numSteps;
  int numShapes = 0;

  for (shape = shapes.begin();
       shape != shapes.end(); shape++) {
    firstTime = vrpn_TRUE;
    if ((*shape).d_exposure_uCoulombs_per_square_cm == 0) {
      printf("Warning, exposure is 0, not drawing shape\n");
    } else {
      numShapes++;
      setExposure((*shape).d_exposure_uCoulombs_per_square_cm);
      dwell_time_nsec = (vrpn_int32)floor(d_line_dwell_time_sec*1e9);
      sem->setPointDwellTime(dwell_time_nsec);
      for (point = (*shape).pointListBegin();
         point != (*shape).pointListEnd(); point++) {
        if (firstTime) {
         start_x_nm = (*point).d_x;
         start_y_nm = (*point).d_y;
         firstTime = vrpn_FALSE;
        } else {
         end_x_nm = (*point).d_x;
         end_y_nm = (*point).d_y;
         // now draw line going from start to end
         delta_x = end_x_nm - start_x_nm;
         delta_y = end_y_nm - start_y_nm;
         dist = sqrt(delta_x*delta_x + delta_y*delta_y);
         numSteps = dist/d_line_inter_dot_dist_nm;
         dx = (end_x_nm - start_x_nm)/(double)numSteps;
         dy = (end_y_nm - start_y_nm)/(double)numSteps;
         for (i = 0; i < numSteps; i++) {
           convert_nm_to_DAC(start_x_nm + dx*i, start_y_nm + dy*i,
                             x_DAC, y_DAC);
           sem->goToPoint(x_DAC, y_DAC);
           if (i%100 == 0) {
              sem->goToPoint(dump_x_DAC,dump_y_DAC); // dump location
              sem->mainloop();
           }
         }
         sem->goToPoint(dump_x_DAC,dump_y_DAC); // dump location
         sem->mainloop(); // do the line segment and wait at the dump location
                          // for additional commands
         start_x_nm = end_x_nm;
         start_y_nm = end_y_nm;
        }
      }
    }
  }
  printf("Exposure done. %d shapes drawn\n", numShapes);
}
*/

void ExposureManager::setExposure(double uCoul_per_cm2)
{
  d_exposure_uCoul_per_cm2 = uCoul_per_cm2;

  double exposure_number = 
                   (d_exposure_uCoul_per_cm2/
                    d_min_exposure_per_point_uCoul_per_cm2);
  double beam_area_nm2 = 0.25*M_PI*d_beam_width_nm*d_beam_width_nm;
  double beam_area_cm2 = beam_area_nm2*(1e-14);

  // assuming we overlap the dots by some factor, 
  // how long should dwell time be?

  // use a one-dimensional overlap factor for lines 
  d_line_inter_dot_dist_nm = d_beam_width_nm/d_line_overlap_factor;
  // and a two-dimensional overlap factor for areas
  d_fill_inter_dot_dist_nm = beam_area_nm2/d_area_overlap_factor;
  d_fill_inter_dot_dist_nm = sqrt(d_fill_inter_dot_dist_nm);

  double line_time_factor = exposure_number/d_line_overlap_factor;
  double area_time_factor = exposure_number/d_area_overlap_factor;

/*
  d_line_dwell_time_sec = beam_area_cm2*line_time_factor*d_min_dwell_time_sec/
                          (d_beam_current_picoAmps*1e-6);
*/
  d_line_dwell_time_sec = d_exposure_uCoul_per_cm2*beam_area_cm2/
                          (d_line_overlap_factor*d_beam_current_picoAmps*1e-6);

  d_fill_dwell_time_sec = area_time_factor*d_min_dwell_time_sec;
  
  // now we know the dwell time and the dot spacing 
  printf("line dwell time=%g sec\nline interdot dist=%g nm\n",
         d_line_dwell_time_sec, d_line_inter_dot_dist_nm); 

}

void ExposureManager::setColumnParameters(double minDwellTime_sec, 
                                          double beamWidth_nm,
                                          double current_picoAmps)
{
  d_beam_width_nm = beamWidth_nm;
  d_beam_current_picoAmps = current_picoAmps;
  d_min_dwell_time_sec = minDwellTime_sec;
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
  
  d_pointListPtr = d_currShape->pointListBegin();
  start_x = (*d_pointListPtr).d_x;
  start_y = (*d_pointListPtr).d_y;
  d_pointListPtr++;

  d_nextExposePoint = PatternPoint(start_x, start_y);
  return 0;
}

// get the next exposure point for the current shape
vrpn_bool ExposureManager::getNextPoint(PatternPoint &point, double &time)
{
  if (d_currShape == NULL) return vrpn_FALSE; // error, no shape being drawn

  point = d_nextExposePoint;
  time = d_line_dwell_time_sec;

  if (d_pointListPtr == d_currShape->pointListEnd()) {
    delete d_currShape;
    d_currShape = NULL;
    return vrpn_TRUE;
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
  return vrpn_TRUE; // shape is in progress
}
