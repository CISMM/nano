#ifndef EXPOSUREMANAGER_H
#define EXPOSUREMANAGER_H

#include "patternEditor.h"
#include "nmm_Microscope_SEM_Remote.h"

class EdgeTableEntry {
 public:
  EdgeTableEntry(int ymax = 0, double xmin = 0.0, double deltaX = 1.0):
        d_yMax(ymax), d_xMin(xmin), d_deltaX(deltaX) {}
  EdgeTableEntry(const EdgeTableEntry &ete):
        d_yMax(ete.d_yMax), d_xMin(ete.d_xMin), d_deltaX(ete.d_deltaX){}
  
  int operator== (const EdgeTableEntry &ete) {
    return (d_yMax == ete.d_yMax && 
            d_xMin == ete.d_xMin && 
            d_deltaX == ete.d_deltaX);
  }
    
  int operator< (const EdgeTableEntry &ete) {
    if (d_xMin == ete.d_xMin) {
      return (d_deltaX < ete.d_deltaX);
    }
    return (d_xMin < ete.d_xMin);
  }
  int d_yMax;  // in units of d_area_inter_dot_dist_nm
  double d_xMin; // in nm
  double d_deltaX; // in nm per d_area_inter_dot_dist_nm
};

class ExposureManager {
 public:
  ExposureManager();

  void exposePattern(list<PatternShape> shapes, 
                     list<PatternPoint> dump_points,
                     nmm_Microscope_SEM_Remote *sem, int mag);
  void setExposure(double uCoul_per_cm);
  void setColumnParameters(double minDwellTime_sec,
                           double beamWidth_nm,
                           double current_picoAmps);
  int initShape(PatternShape &shape);
  vrpn_bool getNextPoint(PatternPoint &point, double &time);
  void convert_nm_to_DAC(const double x_nm, const double y_nm, 
                         int &xDAC, int &yDAC);

 private:
  // helper function for initShape and getNextPoint
  void initThickLineSegment(vrpn_bool firstSegment);
  void initThinLineSegment();
  int initPolygon();

  // exposure-dependent parameters
  double d_exposure_uCoul_per_cm2;
  double d_area_dwell_time_sec;
  double d_line_dwell_time_sec;
  double d_area_inter_dot_dist_nm;
  double d_line_inter_dot_dist_nm;

  // exposure-independent parameters
  double d_min_exposure_per_point_uCoul_per_cm2;
  double d_min_dwell_time_sec;
  double d_nm_per_pixel;
  double d_beam_width_nm;
  double d_beam_current_picoAmps;
  int d_xSpan_DACunits;
  int d_ySpan_DACunits;
  double d_xSpan_nm;
  double d_ySpan_nm;

  // some parameters that need to be tuned
  double d_line_overlap_factor;
  double d_area_overlap_factor;

  // other
  PatternShape *d_currShape;
  list<PatternPoint>::iterator d_pointListPtr; 
  PatternPoint d_nextExposePoint;

  // additional state for doing thick polylines
  int d_currThickLineRow;
  int d_numThickLineRows;
  double d_segmentStartFirstRowX, d_segmentStartFirstRowY;
  double d_segmentStartLastRowX, d_segmentStartLastRowY;
  double d_segmentEndFirstRowX, d_segmentEndFirstRowY;
  double d_segmentEndLastRowX, d_segmentEndLastRowY;
  double d_halfWidth;

  // additional state for doing polygons
  int d_currPolygonScanline;
  int d_numPolygonScanlines;
  double d_polygonMinYScan;
  list<EdgeTableEntry> *d_edgeTable; // ith element corresponds to the scanline
                  // at y= d_polygonMinY + i*d_area_inter_dot_dist_nm

  list<EdgeTableEntry> d_activeEdgeTable;
  list<EdgeTableEntry>::iterator d_activeEdgeBegin, d_activeEdgeEnd;
};

#endif
