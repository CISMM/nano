#ifndef CORRESPONDENCE_H
#define CORRESPONDENCE_H

#include <nmb_Types.h>  // for vrpn_bool
#include <BCPlane.h>
#include <nmb_Image.h>

class corr_point_t {
  public:
    corr_point_t(double xp=0.0, double yp=0.0, double radiusp=5.0);
    corr_point_t(double xp, double yp, double zp, double radiusp);
    vrpn_bool is2D;
    double x, y, z, radius;
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
    void clear();
    int addPoint(corr_point_t &p);
    int unpaired_fluoro_addPoint(corr_point_t &p); // new
    int addPoint(corr_point_t *p);
    int unpaired_fluoro_addPoint(corr_point_t *p); // new
	int addCurve(corr_curve_t &c);
    int setPoint(int spaceIndex, int pntIndex, const corr_point_t &p);
	int unpaired_fluoro_setPoint(int spaceIndex, int pntIndex, const corr_point_t &p); // new
    int setPoint(int pntIndex, corr_point_t *p);
	int unpaired_fluoro_setPoint(int pntIndex, corr_point_t *p); // new
    int deletePoint(int pntIndex);
	int unpaired_fluoro_deletePoint(int pntIndex); // new
    vrpn_bool findNearestPoint(int spaceIndex, double x, double y,
        double scaleX, double scaleY, int *pntIndex);
	vrpn_bool unpaired_fluoro_findNearestPoint(int spaceIndex, double x, double y,
        double scaleX, double scaleY, int *pntIndex); // new
    int numPoints() const {return num_points;};
	int unpaired_fluoro_numPoints() const {return unpaired_fluoro_num_points;};// new
    int maxPoints() const {return max_points;};
	int unpaired_fluoro_maxPoints() const {return unpaired_fluoro_max_points;};// new
    int numSpaces() const {return num_spaces;};
	int unpaired_fluoro_numSpaces() const {return unpaired_fluoro_num_spaces;};// new
    int getPoint(int spaceIndex, int pntIndex, corr_point_t *pnt) const;
	int unpaired_fluoro_getPoint(int spaceIndex, int pntIndex, corr_point_t *pnt) const; // new
    int setValuesFromPlane(int spaceIdx, BCPlane *p);
    int setValuesFromImage(int spaceIdx, nmb_Image *im);
    int scalePoints(int spaceIndex, double sx, double sy, double sz);
    Correspondence &operator = (const Correspondence &c);
    vrpn_bool equals(const Correspondence &c);
    void print();

  private:
    unsigned int num_points;
    unsigned int max_points;
    unsigned int num_spaces;
    corr_point_t **pnts;

	unsigned int unpaired_fluoro_num_points; // new
    unsigned int unpaired_fluoro_max_points; // new
    unsigned int unpaired_fluoro_num_spaces; // new
    corr_point_t **unpaired_fluoro_pnts; // new
};

#endif
