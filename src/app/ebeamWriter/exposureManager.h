#ifndef EXPOSUREMANAGER_H
#define EXPOSUREMANAGER_H

#include "patternEditor.h"
#include "nmm_Microscope_SEM_Remote.h"

class ExposureManager {
 public:
  ExposureManager();

  void exposePattern(list<PatternShape> shapes, 
                     nmm_Microscope_SEM_Remote *sem, int mag);
  void setExposure(double uCoul_per_cm);
  void setMinDwellTime(double t);
  int initShape(PatternShape &shape);
  int getNextPoint(PatternPoint &point, double &time);

 private:
  // exposure-dependent parameters
  double d_exposure_uCoul_per_cm2;
  double d_fill_dwell_time_sec;
  double d_line_dwell_time_sec;
  double d_fill_inter_dot_dist_nm;
  double d_line_inter_dot_dist_nm;

  // exposure-independent parameters
  double d_min_exposure_per_point_uCoul_per_cm2;
  double d_min_dwell_time_sec;
  double d_nm_per_pixel;
  double d_beam_width_nm;
  double d_beam_current_picoAmps;

  // some parameters that need to be tuned
  double d_line_overlap_factor;
  double d_area_overlap_factor;

  // other
  PatternShape *d_currShape;
  list<PatternPoint>::iterator d_pointListPtr; 
  PatternPoint d_nextExposePoint;
};

#endif
