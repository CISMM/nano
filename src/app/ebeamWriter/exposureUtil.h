#ifndef EXPOSUREUTIL_H
#define EXPOSUREUTIL_H

class ExposureUtil {
 public:
  // in pCoul_per_cm
  static double computeMinLinearExposure(double minDwellTime_sec,
                                         double dotSpacing_nm,
                                         double current_picoAmps);

  // in uCoul_per_cm2
  static double computeMinAreaExposure(double minDwellTime_sec,
                                       double dotSpacing_nm,
                                       double lineSpacing_nm,
                                       double current_picoAmps); 
  
  // in nanoseconds
  static double computeLineDwellTime(double dotSpacing_nm,
                                       double current_picoAmps,
                                       double exposure_picoAmps_per_cm);

  // in nanoseconds
  static double computeAreaDwellTime(double dotSpacing_nm,
                                     double lineSpacing_nm,
                                     double current_picoAmps,
                                     double exposure_uAmps_per_square_cm);
};

#endif
