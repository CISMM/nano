#ifndef URSPIDER_H
#define URSPIDER_H

#include "URPolygon.h"

class URSpider : public URPolygon {
private:
	double spider_length[8];
	double spider_width[8];
	double spider_thick[8];
	int spider_tess[8];
	double spider_beg_curve[8];
    double spider_end_curve[8];
    double spider_leg_x[8];
    double spider_leg_y[8];
    double spider_leg_rot[8];
	int spider_legs;

public:
	// constructor destructor
	URSpider();
	~URSpider();

	// Spider specific functions
	void SetSpiderLength(int i, double l) { spider_length[i] = l; }
	void SetSpiderWidth(int i, double w) { spider_width[i] = w; }
	void SetSpiderThick(int i, double t) { spider_thick[i] = t; }
	void SetSpiderTess(int i, int t) { spider_tess[i] = t; }
	void SetSpiderBegCurve(int i, double b) { spider_beg_curve[i] = b; }
    void SetSpiderEndCurve(int i, double e) { spider_end_curve[i] = e; }
    void SetSpiderLegX(int i, double x) { spider_leg_x[i] = x; }
    void SetSpiderLegY(int i, double y) { spider_leg_y[i] = y; }
    void SetSpiderLegRot(int i, double r) { spider_leg_rot[i] = r; }
	void SetSpiderLegs(int l) { spider_legs = l; }

	double GetSpiderLength(int i) { return spider_length[i]; }
	double GetSpiderWidth(int i) { return spider_width[i]; }
	double GetSpiderThick(int i) { return spider_thick[i]; }
	int GetSpiderTess(int i) { return spider_tess[i]; }
	double GetSpiderBegCurve(int i) { return spider_beg_curve[i]; }
    double GetSpiderEndCurve(int i) { return spider_end_curve[i]; }
    double GetSpiderLegX(int i) { return spider_leg_x[i]; }
    double GetSpiderLegY(int i) { return spider_leg_y[i]; }
    double GetSpiderLegRot(int i) { return spider_leg_rot[i]; }
	int GetSpiderLegs() { return spider_legs; }
	void SaveSpider(const char*);
};

#endif