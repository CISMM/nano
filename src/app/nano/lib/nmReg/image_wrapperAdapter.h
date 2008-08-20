#ifndef _IMAGE_WRAPPER_ADAPTER_
#define _IMAGE_WRAPPER_ADAPTER_

#include "base_camera_server.h"
#include "nmb_Image.h"

//Adapter class for allowing spot_trackers to operate on nmb_Images.
class image_wrapperAdapter : public image_wrapper {

public:
	image_wrapperAdapter();
	image_wrapperAdapter(const nmb_Image *image);
	virtual ~image_wrapperAdapter();

	void setImage(const nmb_Image *image) { this->image = const_cast<nmb_Image*>(image); }

	nmb_Image* getImage() { return this->image; }

	virtual void read_range(int &minx, int &maxx, int &miny, int &maxy) const;

	virtual unsigned int get_num_colors(void) const { return 1; }

	virtual bool read_pixel(int x, int y, double &result, unsigned rgb = 0) const;

	virtual double read_pixel_nocheck(int x, int y, unsigned rgb = 0) const;

private:
	nmb_Image *image;

};

#endif // _IMAGE_WRAPPER_ADAPTER_