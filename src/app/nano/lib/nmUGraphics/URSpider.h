#ifndef URSPIDER_H
#define URSPIDER_H

#include "URPolygon.h"

class URSpider : public URPolygon {
private:
	double spider_length[8];
	double spider_width[8];
	double spider_thick[8];
	int spider_tess[8];
	double spider_curve[8];
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
	void SetSpiderCurve(int i, double c) { spider_curve[i] = c; }
	void SetSpiderLegs(int l) { spider_legs = l; }

	double GetSpiderLength(int i) { return spider_length[i]; }
	double GetSpiderWidth(int i) { return spider_width[i]; }
	double GetSpiderThick(int i) { return spider_thick[i]; }
	int GetSpiderTess(int i) { return spider_tess[i]; }
	double GetSpiderCurve(int i) { return spider_curve[i]; }
	int GetSpiderLegs() { return spider_legs; }
	void SaveSpider(const char*);
};

#endif