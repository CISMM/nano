#ifndef EDGETABLE_H
#define EDGETABLE_H

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

#endif
