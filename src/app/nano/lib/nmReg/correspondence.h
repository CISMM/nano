#ifndef CORRESPONDENCE_H
#define CORRESPONDENCE_H

#include <nmb_Types.h>  // for vrpn_bool
#include <BCPlane.h>
#include <nmb_Image.h>

class corr_point_t {
  public:
    corr_point_t(double xp = 0, double yp = 0, double zp = 0){
	x = xp; y = yp; z = zp; };
    double x, y, z;
};

class corr_curve_t {
  public:
    corr_curve_t(int n) { n_pnts = n; curve_pnt = new corr_point_t[n];};
    int n_pnts;
    corr_point_t *curve_pnt;
};

class Correspondence {
  public:
    Correspondence();
    Correspondence(int num_spaces, int max_points);
    void init(int num_spaces, int max_points);
    int addPoint(corr_point_t &p);
    int addCurve(corr_curve_t &c);
    int setPoint(int spaceIndex, int pntIndex, corr_point_t &p);
    int deletePoint(int pntIndex);
    vrpn_bool findNearestPoint(int spaceIndex, double x, double y,
	double x_max, double y_max, int *pntIndex);
    int numPoints() const {return num_points;};
    int maxPoints() const {return max_points;};
    int numSpaces() const {return num_spaces;};
    int getPoint(int spaceIndex, int pntIndex, corr_point_t *pnt) const;
    int setValuesFromPlane(int spaceIdx, BCPlane *p);
	int setValuesFromImage(int spaceIdx, nmb_Image *im);
    Correspondence &operator = (const Correspondence &c);
    void print();

  private:
    unsigned int num_points;
    unsigned int max_points;
    unsigned int num_spaces;
    corr_point_t **pnts;
};

#endif
