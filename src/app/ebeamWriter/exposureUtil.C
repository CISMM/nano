#include "exposureUtil.h"

double ExposureUtil::computeMinLinearExposure(double minDwellTime_sec,
                                         double dotSpacing_nm,
                                         double current_picoAmps)
{
  double exposure_pCoul_per_cm = 
       1e7*current_picoAmps*minDwellTime_sec/dotSpacing_nm;
  return exposure_pCoul_per_cm;
}

double ExposureUtil::computeMinAreaExposure(double minDwellTime_sec,
                                       double dotSpacing_nm,
                                       double lineSpacing_nm,
                                       double current_picoAmps)
{
  double exposure_uCoul_per_square_cm = 
      1e8*current_picoAmps*minDwellTime_sec/(dotSpacing_nm*lineSpacing_nm);
  return exposure_uCoul_per_square_cm;
}


double ExposureUtil::computeLineDwellTime(double dotSpacing_nm,
                                       double current_picoAmps,
                                       double exposure_picoCoul_per_cm)
{
  double dwellTime_sec = 
         (1e-7)*exposure_picoCoul_per_cm*dotSpacing_nm/current_picoAmps;
  return dwellTime_sec;
}

double ExposureUtil::computeAreaDwellTime(double dotSpacing_nm,
                                     double lineSpacing_nm,
                                     double current_picoAmps,
                                     double exposure_uCoul_per_square_cm)
{
  double dwellTime_sec = 
       (1e-8)*exposure_uCoul_per_square_cm*dotSpacing_nm*lineSpacing_nm/
       current_picoAmps;
  return dwellTime_sec;
}
