#include "URSpider.h"
#include "URPolygon.h"

URSpider::URSpider():URPolygon() {
	obj_type = URSPIDER;
}

URSpider::~URSpider(){}


void URSpider::SaveSpider(const char* filename) {
	ofstream writefile;
	int i;

	writefile.open(filename);
	assert(writefile);

	if (writefile.bad()) {
		cerr << "Unable to open output file" << endl;
		return;
	}

	// write the filename to the file
	writefile << filename << endl << endl;

	// write spider data to the file
	for (i = 0; i < spider_legs; i++) {
		writefile << "Leg " << i + 1 << endl
				  << "\tLength\t" << spider_length[i] << endl
				  << "\tWidth\t\t" << spider_width[i] << endl
				  << "\tThickness\t" << spider_thick[i] << endl
				  << "\tBegin Curvature\t" << Q_RAD_TO_DEG(spider_beg_curve[i]) << endl
                  << "\tEnd Curvature\t" << Q_RAD_TO_DEG(spider_end_curve[i]) << endl << endl;
	}

	writefile.close();
}
