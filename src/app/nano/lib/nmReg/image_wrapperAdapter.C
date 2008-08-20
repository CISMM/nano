#include "image_wrapperAdapter.h"

image_wrapperAdapter::image_wrapperAdapter() {
	image = NULL;
}

image_wrapperAdapter::image_wrapperAdapter(const nmb_Image *image) {
	this->image = const_cast<nmb_Image*>(image);
}

image_wrapperAdapter::~image_wrapperAdapter() {
	image = NULL;
}

void image_wrapperAdapter::read_range(int &minx, int &maxx, int &miny, int &maxy) const {
	// This is assuming the returned values mark the inclusive range of valid indices in
	// each dimension.
	if (image) {
		minx = 0; maxx = image->width()-1; miny = 0; maxy = image->height()-1;
	} else {
		minx = 0; maxx = -1; miny = 0; maxy = -1;
	}
}

bool image_wrapperAdapter::read_pixel(int x, int y, double	&result, unsigned rgb) const {
	int index = y * image->width() + x;
	if (index >= 0 && index < image->width() * image->height()) {
		result = (double) image->getValue(x, y);
		return true;
	}

	// Out of range, return false.
	return false;
}

double image_wrapperAdapter::read_pixel_nocheck(int x, int y, unsigned rgb) const {
	if (image == NULL) return 0.0;

	return static_cast<double>(image->getValue(x, y));
}