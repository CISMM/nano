#include "exposureManager.h"
#include "delay.h"

#ifndef M_PI
#define M_PI           3.14159265358979323846
#endif

#define MAX_DWELL_TIME_SEC (2.14) // since we encode it in vrpn_int32/nsec

//#define USE_POINT_MESSAGES

ExposureManager::ExposureManager():
  d_exposure_uCoul_per_cm2(0),
  d_area_dwell_time_sec(0.000005),
  d_line_dwell_time_sec(0.000005),
  d_area_inter_dot_dist_nm(50),
  d_line_inter_dot_dist_nm(50),

  d_min_exposure_per_point_uCoul_per_cm2(0),
  d_min_dwell_time_sec(0),
  d_nm_per_pixel(0),
  d_beam_width_nm(0),
  d_beam_current_picoAmps(0),

  d_line_overlap_factor(2),
  d_area_overlap_factor(4),
  d_timingCalibrationDone(vrpn_FALSE),
  d_overhead_per_point_msec(0)
{
}

#pragma optimize("",off)
void ExposureManager::convert_nm_to_DAC(
     const double x_nm, const double y_nm, int &xDAC, int &yDAC)
{
  xDAC = (int)floor(x_nm*(double)d_xSpan_DACunits/d_xSpan_nm);
  yDAC = d_ySpan_DACunits - 
         (int)floor(y_nm*(double)d_ySpan_DACunits/d_ySpan_nm)-1;
  return;
}
#pragma optimize("",on)

void ExposureManager::getDwellTimes(double &line_sec, double &area_sec)
{
	line_sec = d_line_dwell_time_sec;
	area_sec = d_area_dwell_time_sec;
	return;
}

void ExposureManager::exposePattern(list<PatternShape> shapes,
                     nmm_Microscope_SEM_Remote *sem, int mag, 
                     int &numPointsGenerated, double &totalExposureTimeSec,
                     ExposeMode mode, int recursion_level)
{
#ifdef USE_POINT_MESSAGES

  list<PatternShape>::iterator shape;
  PatternPoint pnt_nm;
  double dwell_time_sec;
  vrpn_int32 dwell_time_nsec;
  int x_DAC, y_DAC;

  sem->getScanRegion_nm(d_xSpan_nm, d_ySpan_nm);
  sem->getMaxScan(d_xSpan_DACunits, d_ySpan_DACunits);

  // only printout timing for the complete list of shapes and not for sublists
  if (recursion_level == 0) {
    numPointsGenerated = 0;
    totalExposureTimeSec = 0;
    printf("starting test\n");
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);
    exposePattern(shapes, sem, mag, numPointsGenerated,
                  totalExposureTimeSec, vrpn_FALSE, 1);
    gettimeofday(&end_time, NULL);
    struct timeval elapsed_time = vrpn_TimevalDiff(end_time, start_time);
    double elapsed_time_msec = vrpn_TimevalMsecs(elapsed_time);
    double elapsed_time_sec = elapsed_time_msec*0.001;
    double time_per_point_msec = elapsed_time_msec/(double)numPointsGenerated;
    printf("ending test\n");
    printf("%d points generated at %g msec per point\n",
        numPointsGenerated, time_per_point_msec);
  }

  for (shape = shapes.begin();
       shape != shapes.end(); shape++) {
    if ((*shape).d_type == PS_COMPOSITE) {
      exposePattern((*shape).d_shapes, sem, mag, 
                    numPointsGenerated, totalExposureTimeSec,
                    mode, recursion_level+1);
    } else {
      printf("Starting shape\n");
      initShape(*shape);
      while (getNextPoint(pnt_nm, dwell_time_sec)) {
        numPointsGenerated++;
        dwell_time_nsec = (vrpn_int32)floor(dwell_time_sec*1e9);
        totalExposureTimeSec += dwell_time_sec;
        convert_nm_to_DAC(pnt_nm.d_x, pnt_nm.d_y, x_DAC, y_DAC);
        if (mode == NORMAL_EXPOSE) {
          sem->setPointDwellTime(dwell_time_nsec);
          sem->goToPoint(x_DAC, y_DAC);
          if (numPointsGenerated % 200 == 0) {
            sem->mainloop();
          }
        }
      }
      if (mode == NORMAL_EXPOSE) {
        sem->mainloop();
      }
      printf("Finished with shape\n");
    }
  }
#else 

  list<PatternShape>::iterator shape;
  list<PatternPoint>::iterator point;
  vrpn_int32 numPointsInShape;
  vrpn_float32 exposure_uCoul_per_cm2;
  vrpn_float32 lineWidth_nm;
  vrpn_float32 *x_nm, *y_nm;
  int i;

  if (recursion_level == 0) {
    if (mode == NORMAL_EXPOSE) {
      sem->setBeamCurrent(d_beam_current_picoAmps);
      sem->setBeamWidth(d_beam_width_nm);
      sem->clearExposePattern();
    }
    numPointsGenerated = 0;
    totalExposureTimeSec = 0.0;
  }

  for (shape = shapes.begin();
       shape != shapes.end(); shape++) {

    // convert the point list into a pair of arrays if this is a simple shape
    if ((*shape).d_type != PS_COMPOSITE) {
      numPointsInShape = 0;
      for (point = (*shape).pointListBegin();
           point != (*shape).pointListEnd(); point++) {
        numPointsInShape++;
      }
      x_nm = new vrpn_float32[numPointsInShape];
      y_nm = new vrpn_float32[numPointsInShape];
      exposure_uCoul_per_cm2 = (*shape).d_exposure_uCoulombs_per_square_cm;
      lineWidth_nm = (*shape).d_lineWidth_nm;
      i = 0;
      for (point = (*shape).pointListBegin();
           point != (*shape).pointListEnd(); point++) {
        x_nm[i] = (*point).d_x;
        y_nm[i] = (*point).d_y;
        i++;
      }
      setExposure(exposure_uCoul_per_cm2);
      double dwell_time_sec = 0;
      initShape(*shape);
      PatternPoint pnt_nm;

      while (getNextPoint(pnt_nm, dwell_time_sec)) {
        numPointsGenerated++;
        totalExposureTimeSec += dwell_time_sec;
      }
    }

    if ((*shape).d_type == PS_POLYLINE && (mode == NORMAL_EXPOSE)) {
      sem->addPolyline(exposure_uCoul_per_cm2, lineWidth_nm, numPointsInShape,
                         x_nm, y_nm);
    } else if ((*shape).d_type == PS_POLYGON && (mode == NORMAL_EXPOSE)){
      sem->addPolygon(exposure_uCoul_per_cm2, numPointsInShape, x_nm, y_nm);
    } else if ((*shape).d_type == PS_DUMP && (mode == NORMAL_EXPOSE)){
      sem->addDumpPoint(x_nm[0], y_nm[0]);
    } else if ((*shape).d_type == PS_COMPOSITE) {
      exposePattern((*shape).d_shapes, sem, mag,
                    numPointsGenerated, totalExposureTimeSec,
                    mode, recursion_level+1);
    }

    if ((*shape).d_type != PS_COMPOSITE) {
      delete [] x_nm;
      delete [] y_nm;
    }
  }
  if (recursion_level == 0 && (mode == NORMAL_EXPOSE)) {
    sem->exposePattern();
  }

#endif

}

#pragma optimize("",off)
void ExposureManager::exposePattern(list<PatternShape> shapes,
                     nmm_Microscope_SEM_EDAX *sem, int mag,
                     int &numPointsGenerated, double &totalExposureTimeSec,
                     ExposeMode mode, int recursion_level)
{
  list<PatternShape>::iterator shape;
  PatternPoint pnt_nm;
  double dwell_time_sec;
  vrpn_int32 dwell_time_nsec;
  int x_DAC, y_DAC;
  struct timeval t_start, t_end;

  sem->getScanRegion_nm(d_xSpan_nm, d_ySpan_nm);
  sem->getMaxScan(d_xSpan_DACunits, d_ySpan_DACunits);

  // search the list for a dump point to use for timing test
  int x_DACdump0, y_DACdump0;
  shape = shapes.begin();
  list<PatternPoint>::iterator dumpPnt;
  vrpn_bool dumpPointFound = vrpn_FALSE;
  while (shape != shapes.end()) {
    if ((*shape).d_type == PS_DUMP) {
      dumpPnt = (*shape).pointListBegin();
      if (dumpPnt != (*shape).pointListEnd()) {
        convert_nm_to_DAC((*dumpPnt).d_x, (*dumpPnt).d_y, 
                          x_DACdump0, y_DACdump0);
        dumpPointFound = vrpn_TRUE;
        break;
      }
    }
    shape++;
  }

  if (!dumpPointFound) {
    printf("Warning: no dump point found to be used for timing test\n");
    x_DACdump0 = 0;
    y_DACdump0 = 0;
  }

  if (recursion_level == 0) {
    Delay::beginRealTimeSection();
    d_numPointsTotal = 0;
    d_exposureTimeTotal_sec = 0.0;
    numPointsGenerated = 0;
    totalExposureTimeSec = 0.0;
    
    if (!d_timingCalibrationDone) {
      printf("starting timing calibration test\n");
      struct timeval start_time, end_time;

      gettimeofday(&start_time, NULL);
      exposePattern(shapes, sem, mag, d_numPointsTotal, 
                    d_exposureTimeTotal_sec, DUMP_EXPOSE_MIN_DWELL, 1);

      gettimeofday(&end_time, NULL);
      struct timeval elapsed_time = vrpn_TimevalDiff(end_time, start_time);
      double elapsed_time_msec = vrpn_TimevalMsecs(elapsed_time);
      d_overhead_per_point_msec = 
                        elapsed_time_msec/(double)d_numPointsTotal;
      printf("ending timing calibration test\n");
      printf("%d points generated in %g msec; %g msec per point\n",
          d_numPointsTotal, elapsed_time_msec, d_overhead_per_point_msec);
      d_timingCalibrationDone = vrpn_TRUE;
    } else {
      exposePattern(shapes, sem, mag, d_numPointsTotal, d_exposureTimeTotal_sec,
                    NO_SEM_COMMANDS, 1);
    }
    gettimeofday(&t_start, NULL);
  }

  for (shape = shapes.begin();
       shape != shapes.end(); shape++) {

    if ((*shape).d_type != PS_COMPOSITE) {
      printf("Starting shape\n");
      initShape(*shape);
      while (getNextPoint(pnt_nm, dwell_time_sec)) {
        dwell_time_nsec = (vrpn_int32)floor(dwell_time_sec*1e9);
        dwell_time_sec = dwell_time_nsec*(1e-9); 
        if (mode == NORMAL_EXPOSE) {
          sem->setPointDwellTime(dwell_time_nsec, vrpn_FALSE);
        } else if (mode == DUMP_EXPOSE_MIN_DWELL) {
          sem->setPointDwellTime(0, vrpn_FALSE);
        }
        convert_nm_to_DAC(pnt_nm.d_x, pnt_nm.d_y, x_DAC, y_DAC);
        vrpn_bool shouldReportStatus = (totalExposureTimeSec >=
                     ceil(totalExposureTimeSec)-dwell_time_sec);
        if (mode == NORMAL_EXPOSE) {
          sem->goToPoint(x_DAC, y_DAC, vrpn_FALSE, d_overhead_per_point_msec);
          if (shouldReportStatus) {
            sem->reportExposureStatus(d_numPointsTotal,
                                      numPointsGenerated,
                                      d_exposureTimeTotal_sec,
                                      totalExposureTimeSec);
          }
        } else if (mode == DUMP_EXPOSE_MIN_DWELL) {
          sem->goToPoint(x_DACdump0, y_DACdump0, vrpn_FALSE, 0);
        }
        numPointsGenerated++;
        totalExposureTimeSec += dwell_time_sec;
      }
      printf("Finished with shape\n");
    } else {
      exposePattern((*shape).d_shapes, sem, mag, 
           numPointsGenerated, totalExposureTimeSec, 
           mode, recursion_level+1);
    }
  }

  if (recursion_level == 0) {
    gettimeofday(&t_end, NULL);
    Delay::endRealTimeSection();
    double total_elapsed_time = vrpn_TimevalMsecs(t_end) - 
                                vrpn_TimevalMsecs(t_start);
    printf("actual total exposure time = %g msec\n", 
           total_elapsed_time);
    sem->reportExposureStatus(d_numPointsTotal, d_numPointsTotal,
                              d_exposureTimeTotal_sec, d_exposureTimeTotal_sec);
    numPointsGenerated = d_numPointsTotal;
    totalExposureTimeSec = d_exposureTimeTotal_sec;

  }

}
#pragma optimize("",on)

/* this version is disabled for testing initShape and getNextPoint
void ExposureManager::exposePattern(list<PatternShape> shapes,
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
      dwell_time_nsec = (vrpn_int32)floor(dwell_time_sec*1e9);

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

  double beam_area_nm2 = 0.25*M_PI*d_beam_width_nm*d_beam_width_nm;
  double beam_area_cm2 = beam_area_nm2*(1e-14);

  d_line_dwell_time_sec = d_exposure_uCoul_per_cm2*beam_area_cm2/
                          (d_line_overlap_factor*d_beam_current_picoAmps*1e-6);

  d_area_dwell_time_sec = d_exposure_uCoul_per_cm2*beam_area_cm2/
                          (d_area_overlap_factor*d_beam_current_picoAmps*1e-6);
 
  if (d_line_dwell_time_sec > MAX_DWELL_TIME_SEC) {
    double line_overlap_factor_to_use = 
          d_line_overlap_factor*(d_line_dwell_time_sec)/MAX_DWELL_TIME_SEC;
    d_line_inter_dot_dist_nm = d_beam_width_nm/line_overlap_factor_to_use;
    d_line_dwell_time_sec = MAX_DWELL_TIME_SEC;
  } else {
    d_line_inter_dot_dist_nm = d_beam_width_nm/d_line_overlap_factor;
  }
  if (d_area_dwell_time_sec > MAX_DWELL_TIME_SEC) {
    double area_overlap_factor_to_use = 
          d_area_overlap_factor*(d_area_dwell_time_sec)/MAX_DWELL_TIME_SEC;
    d_area_inter_dot_dist_nm = beam_area_nm2/area_overlap_factor_to_use;
    d_area_inter_dot_dist_nm = sqrt(d_area_inter_dot_dist_nm);
    d_area_dwell_time_sec = MAX_DWELL_TIME_SEC;
  } else {
    d_area_inter_dot_dist_nm = beam_area_nm2/d_area_overlap_factor;
    d_area_inter_dot_dist_nm = sqrt(d_area_inter_dot_dist_nm);
  }
 
  // now we know the dwell time and the dot spacing 
  printf("line dwell: %g usec; line dist: %g nm\n",
         d_line_dwell_time_sec*1e6, d_line_inter_dot_dist_nm); 
  printf("area dwell: %g usec; area dist: %g nm\n",
         d_area_dwell_time_sec*1e6, d_area_inter_dot_dist_nm);

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

// setup for exposure of the specified shape (for simple shapes only)
int ExposureManager::initShape(PatternShape &shape)
{
  if (shape.d_type == PS_COMPOSITE) {
    return -1;
  }
  setExposure(shape.d_exposure_uCoulombs_per_square_cm);


  if (shape.d_type == PS_POLYLINE){
    d_currShape = new PatternShape(shape);
    d_pointListPtr = d_currShape->pointListBegin();
    d_numThickLineRows = (int)floor(d_currShape->d_lineWidth_nm/
                                    d_area_inter_dot_dist_nm);
    if (d_numThickLineRows == 0) {
       d_halfWidth = 0;
    } else {
       d_halfWidth = 0.5*(double)(d_numThickLineRows-1)*
                                 d_area_inter_dot_dist_nm;
    }

    if (d_halfWidth > 0) {
      list<PatternPoint>::iterator nextPnt = d_pointListPtr;
      nextPnt++;
      if (nextPnt != d_currShape->pointListEnd()) {
        initThickLineSegment(vrpn_TRUE);
      } else {
        fprintf(stderr, "Warning: only one point found in thick polyline\n");
        delete d_currShape;
        d_currShape = NULL;
        return -1;
      }
    } else {
      initThinLineSegment();
    }
    d_pointListPtr++;
  } else if (shape.d_type == PS_POLYGON) {
    d_currShape = new PatternShape(shape);
    if (initPolygon()) {
      delete d_currShape;
      d_currShape = NULL;
      return -1;
    }
  } else {
    printf("this shouldn't happen\n");
    return -1;
  }
  return 0;
}


int ExposureManager::initPolygon()
{
  vrpn_bool swapOrder;
  double fymin, fymax;
  int ymin, ymax;
  double xmin;
  double deltaX;
  EdgeTableEntry ete;

  double deltaYTotal = d_currShape->maxY() - d_currShape->minY();

  d_numPolygonScanlines = (int)ceil(deltaYTotal/d_area_inter_dot_dist_nm)+1;
  d_polygonMinYScan = d_currShape->minY();
  d_edgeTable = new list<EdgeTableEntry>[d_numPolygonScanlines];

  list<PatternPoint>::iterator p0 = d_currShape->pointListBegin();
  list<PatternPoint>::iterator p1 = p0;

  p1++;
  if (p0 == d_currShape->pointListEnd() ||
      p1 == d_currShape->pointListEnd()) {
    return -1;
  }
  while (p0 != d_currShape->pointListEnd()) {
    if (p1 == d_currShape->pointListEnd()) {
      p1 = d_currShape->pointListBegin();
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
    ymin = (int)floor((fymin-d_polygonMinYScan)/d_area_inter_dot_dist_nm);
    ymax = (int)floor((fymax-d_polygonMinYScan)/d_area_inter_dot_dist_nm);
    if (ymax == ymin) {
      // horizontal line, don't do anything
    } else {
      deltaX = d_area_inter_dot_dist_nm*((*p1).d_x-(*p0).d_x)/(fymax-fymin);
      if (swapOrder) {
         deltaX *= -1;
      }
      ete = EdgeTableEntry(ymax, xmin, deltaX);
      // insert this at ymin
      assert(ymin <= d_numPolygonScanlines);
      d_edgeTable[ymin].push_back(ete);
    }
    p0++;
    p1++;
  }
  int i;
  for (i = 0; i < d_numPolygonScanlines; i++) {
     d_edgeTable[i].sort();
  }
  d_currPolygonScanline = 0;
  d_activeEdgeTable = d_edgeTable[0];
  d_activeEdgeBegin = d_activeEdgeTable.begin();
  d_activeEdgeEnd = d_activeEdgeBegin;
  d_activeEdgeEnd++;
  d_nextExposePoint = 
         PatternPoint((*d_activeEdgeBegin).d_xMin+d_area_inter_dot_dist_nm, 
                      d_polygonMinYScan);
  return 0;
}

void ExposureManager::initThinLineSegment()
{
  d_nextExposePoint = (*d_pointListPtr);
}

void ExposureManager::initThickLineSegment(vrpn_bool firstSegment)
{
  double start_x, start_y, end_x, end_y;
  double dxA, dyA, dxB, dyB, dist, dx_avg, dy_avg;
  double cprod, widthCorrection;
  double os_x0, os_x1, os_y0, os_y1;

  list<PatternPoint>::iterator pnt0 = d_pointListPtr;
  list<PatternPoint>::iterator pnt1 = pnt0;
  pnt1++;
  list<PatternPoint>::iterator pnt2 = pnt1;
  pnt2++;

  // assertion: pnt0,pnt1 != d_currShape->pointListEnd()
  end_x = (*pnt1).d_x;
  end_y = (*pnt1).d_y;
  start_x = (*pnt0).d_x;
  start_y = (*pnt0).d_y;
  dxA = end_x - start_x;
  dyA = end_y - start_y;
  dist = sqrt(dxA*dxA + dyA*dyA);
  dxA /= dist;
  dyA /= dist;

  if (firstSegment) {
    os_x0 = -d_halfWidth*dyA;
    os_y0 = d_halfWidth*dxA;
  } else {
    os_x0 = 0.5*(d_segmentEndLastRowX - d_segmentEndFirstRowX);
    os_y0 = 0.5*(d_segmentEndLastRowY - d_segmentEndFirstRowY);
  }

  if (pnt2 != d_currShape->pointListEnd()){
    dxB = (*pnt2).d_x - end_x;
    dyB = (*pnt2).d_y - end_y;
    dist = sqrt(dxB*dxB + dyB*dyB);
    dxB /= dist;
    dyB /= dist;
    dx_avg = 0.5*(dxA + dxB);
    dy_avg = 0.5*(dyA + dyB);
    os_x1 = -d_halfWidth*dy_avg;
    os_y1 = d_halfWidth*dx_avg;
    cprod = dxA*os_y1 - dyA*os_x1;
    if (cprod < 0) {
      os_x1 = -os_x1;
      os_y1 = -os_y1;
    }
    // project onto segment perpendicular - should get something equal to
    // d_halfWidth
    widthCorrection = (d_halfWidth)/(os_x1*(-dyA) + os_y1*dxA);
    if (widthCorrection > 2.0) widthCorrection = 2.0;
    os_x1 *= widthCorrection;
    os_y1 *= widthCorrection;
  } else { // the last segment
    os_x1 = -d_halfWidth*dyA;
    os_y1 = d_halfWidth*dxA;
  }

  d_segmentStartFirstRowX = start_x-os_x0;
  d_segmentStartFirstRowY = start_y-os_y0;
  d_segmentStartLastRowX = start_x+os_x0;
  d_segmentStartLastRowY = start_y+os_y0;
  d_segmentEndFirstRowX = end_x-os_x1;
  d_segmentEndFirstRowY = end_y-os_y1;
  d_segmentEndLastRowX = end_x+os_x1;
  d_segmentEndLastRowY = end_y+os_y1;

  d_currThickLineRow = 0;
  d_nextExposePoint = PatternPoint(d_segmentStartFirstRowX,
                                   d_segmentStartFirstRowY);
}

// get the next exposure point for the current shape
vrpn_bool ExposureManager::getNextPoint(PatternPoint &point, double &time)
{
  if (d_currShape == NULL) return vrpn_FALSE; // error, no shape being drawn

  if (d_currShape->d_type == PS_POLYLINE) {
    double end_x, end_y, dx, dy, distToEnd;


    if (d_halfWidth == 0) {
      point = d_nextExposePoint;
      time = d_line_dwell_time_sec;

      if (d_pointListPtr == d_currShape->pointListEnd()) {
        delete d_currShape;
        d_currShape = NULL;
        // we're done with the shape after this (returning the last point)
        return vrpn_TRUE;
      }

      end_x = (*d_pointListPtr).d_x;
      end_y = (*d_pointListPtr).d_y;
      dx = end_x - d_nextExposePoint.d_x;
      dy = end_y - d_nextExposePoint.d_y;
      distToEnd = sqrt(dx*dx + dy*dy);

      if (distToEnd < d_line_inter_dot_dist_nm) {
        // a little extra exposure at the corner but I don't think it will
        // be significant
        d_nextExposePoint = PatternPoint(end_x, end_y);
        d_pointListPtr++;
      } else {
        d_nextExposePoint.d_x += d_line_inter_dot_dist_nm*dx/distToEnd;
        d_nextExposePoint.d_y += d_line_inter_dot_dist_nm*dy/distToEnd;
      }
    } else {
      point = d_nextExposePoint;
      time = d_area_dwell_time_sec;

      if (d_pointListPtr == d_currShape->pointListEnd()) {
        delete d_currShape;
        d_currShape = NULL;
        // we're done with the shape after this (returning the last point)
        return vrpn_TRUE;
      }

      end_x = d_segmentEndFirstRowX + 
              (d_segmentEndLastRowX - d_segmentEndFirstRowX)*
              (double)d_currThickLineRow/(double)(d_numThickLineRows-1);
      end_y = d_segmentEndFirstRowY +
              (d_segmentEndLastRowY - d_segmentEndFirstRowY)*
              (double)d_currThickLineRow/(double)(d_numThickLineRows-1);
      dx = end_x - d_nextExposePoint.d_x;
      dy = end_y - d_nextExposePoint.d_y;
      distToEnd = sqrt(dx*dx + dy*dy);
      if (distToEnd < d_area_inter_dot_dist_nm) {
        d_currThickLineRow++;
        if (d_currThickLineRow < d_numThickLineRows) {
          d_nextExposePoint.d_x = d_segmentStartFirstRowX +
                     (d_segmentStartLastRowX - d_segmentStartFirstRowX)*
                     (double)d_currThickLineRow/(double)(d_numThickLineRows-1);
          d_nextExposePoint.d_y = d_segmentStartFirstRowY +
                     (d_segmentStartLastRowY - d_segmentStartFirstRowY)*
                     (double)d_currThickLineRow/(double)(d_numThickLineRows-1);
        } else {
          list<PatternPoint>::iterator nextPnt = d_pointListPtr;
          nextPnt++;
          if (nextPnt != d_currShape->pointListEnd()) {
             initThickLineSegment(vrpn_FALSE);
          }
          d_pointListPtr++;
        }
      } else {
        d_nextExposePoint.d_x += d_area_inter_dot_dist_nm*dx/distToEnd;
        d_nextExposePoint.d_y += d_area_inter_dot_dist_nm*dy/distToEnd;
      }
    }
  } else {
    point = d_nextExposePoint;
    time = d_area_dwell_time_sec;
    if ((d_currPolygonScanline == (d_numPolygonScanlines-1) &&
        d_activeEdgeBegin == d_activeEdgeTable.end()) ||
        d_activeEdgeTable.empty()){
      delete d_currShape;
      d_activeEdgeTable.clear();
      delete [] d_edgeTable;
      return vrpn_FALSE;
    } else if (d_nextExposePoint.d_x > (*d_activeEdgeEnd).d_xMin){
      // step to the next segment in this scanline
      d_activeEdgeBegin = d_activeEdgeEnd;
      d_activeEdgeBegin++;
      d_activeEdgeEnd = d_activeEdgeBegin;
      d_activeEdgeEnd++;
      // if we're done with the scanline, increment to the next scanline
      if (d_activeEdgeBegin == d_activeEdgeTable.end()) {
        d_currPolygonScanline++;
        // remove edges with yMax == d_currPolygonScanline
        list<EdgeTableEntry>::iterator test = d_activeEdgeTable.begin();
        list<EdgeTableEntry>::iterator victim;
        while (test != d_activeEdgeTable.end()) {
          if ((*test).d_yMax == d_currPolygonScanline) {
              victim = test;
              test++;
              d_activeEdgeTable.remove(*victim);
          } else {
              (*test).d_xMin += (*test).d_deltaX;
              test++;
          }
        }
        // add edges in d_edgeTable[d_currPolygonScanline] to the active edge
        // table
        d_activeEdgeTable.merge(d_edgeTable[d_currPolygonScanline]);
        d_activeEdgeBegin = d_activeEdgeTable.begin();
        d_activeEdgeEnd = d_activeEdgeBegin;
        d_activeEdgeEnd++;

/*
        printf("%d: active edges:", d_currPolygonScanline);
        for (list<EdgeTableEntry>::iterator edge = d_activeEdgeTable.begin();
             edge != d_activeEdgeTable.end(); edge++) {
           printf("%g, ", (*edge).d_xMin);
        }
        printf("\n");
*/
      } 
      // init d_nextExposePoint to ((d_activeEdgeBegin).d_xMin, 
      // d_currPolygonScanline*d_area_inter_dot_dist_nm + d_polygonMinYScan)
      if (!d_activeEdgeTable.empty()){
        d_nextExposePoint.d_x = (*d_activeEdgeBegin).d_xMin;
        d_nextExposePoint.d_y = d_polygonMinYScan +
           d_currPolygonScanline*d_area_inter_dot_dist_nm;
      }
    } else {
      // increment d_nextExposePoint.d_x
      d_nextExposePoint.d_x += d_area_inter_dot_dist_nm;
    }
  }
  return vrpn_TRUE; // shape is in progress
}
