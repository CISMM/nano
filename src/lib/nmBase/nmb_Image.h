#ifndef NMB_IMAGE_H
#define NMB_IMAGE_H

#include "BCString.h"
#include "BCPlane.h"
#include "BCGrid.h"
#include "vrpn_Types.h"
#include "nmb_String.h"
#include <math.h>

#ifndef __CYGWIN__
#include <values.h>	// for MAXSHORT - probably different on windows
#else
#include <limits.h>
#ifndef MAXSHORT
#define MAXSHORT SHRT_MAX

#endif
#endif

// min and max also in Topo.h
#ifdef MAX
#undef MAX
#endif
#define MAX(x,y)        (((x)>(y))?(x):(y))

#ifdef MIN
#undef MIN
#endif
#define MIN(x,y)        (((x)<=(y))?(x):(y))

class nmb_ImageBounds {
  public:
	nmb_ImageBounds();
	nmb_ImageBounds(double x0,double y0,double x1,double y1);
	// the MIN/MAX distinction is based on how the pixels in the image
	// are indexed (e.g.,the corner at the pixel with index (0,0) is
	// referenced by MIN_X_MIN_Y; particular values are assigned so that
	// the enum values can be used as indexes for the x,y arrays
	enum ImageBoundPoint {MIN_X_MIN_Y = 0, MIN_X_MAX_Y = 1, 
				MAX_X_MIN_Y = 2, MAX_X_MAX_Y = 3};

	void setX(ImageBoundPoint ibp, double xb) {x[ibp] = xb;}
	void setY(ImageBoundPoint ibp, double yb) {y[ibp] = yb;}
	double getX(ImageBoundPoint ibp) const { return x[ibp]; }
	double getY(ImageBoundPoint ibp) const { return y[ibp]; }

  private:
    double x[4], y[4];
};

/*
	nmb_ImageList provides a way to unite images from different BCGrids
	into a single list. An nmb_Image base class and container class for
	BCPlane were added to allow code to work with data structures other
	than BCPlane more easily
*/

// base class for image
class nmb_Image {
  public:
	nmb_Image():is_height_field(VRPN_FALSE)
            {};
	virtual ~nmb_Image() {};
	virtual int width() const = 0;
	virtual int height() const = 0;
	virtual float getValue(int i, int j) const = 0;
	virtual void setValue(int i, int j, float val) = 0;
        // min and max of values actually in the image:
	virtual float maxValue() const = 0;
	virtual float minValue() const = 0;
        // min and max representable values:
	virtual float minAttainableValue() const = 0;
        virtual float maxAttainableValue() const = 0;
	virtual int validDataRange(short* o_top, short* o_left, 
                                   short* o_bottom, short*o_right) = 0;

	float getValueInterpolated(double i, double j) const;

	// bounds of image in whatever units the image is in 
	// (note: this is more general than the current BCGrid interface since
	//  it allows for rotation of the image coordinate system (in which we
	//  have image positions in terms of pixel (i,j) coordinates) relative
	//  to the world coordinate system)
	virtual double boundX(nmb_ImageBounds::ImageBoundPoint ibp) const = 0;
	virtual double boundY(nmb_ImageBounds::ImageBoundPoint ibp) const = 0;
	virtual void setBoundX(nmb_ImageBounds::ImageBoundPoint ibp, double x) 
             = 0;
	virtual void setBoundY(nmb_ImageBounds::ImageBoundPoint ibp, double y) 
             = 0;
	virtual void getBounds(nmb_ImageBounds &ib) const = 0;
	virtual void setBounds(const nmb_ImageBounds &ib) = 0;

	double widthWorld() const;
	double heightWorld() const;

	// convert a position in an image given as a pixel location into a
	// position in the world coordinate system for the image
	void pixelToWorld(const double i, const double j, 
 		  double &x, double &y) const;

	// WARNING: assumes image axes are orthogonal in the world
	void worldToPixel(const double x, const double y,
			double &i, double &j) const;

	virtual BCString *name() = 0;
	virtual BCString *unitsX() = 0;
	virtual BCString *unitsY() = 0;
	virtual BCString *unitsValue() = 0;
	vrpn_bool isHeightField() const {return is_height_field;}
	void setHeightField(vrpn_bool flag) {is_height_field = flag;}

	// gives address of an array of unsigned chars
	virtual vrpn_uint8 *rawDataUnsignedByte() = 0;

	virtual int numExportFormats() = 0;
	virtual const char *exportFormatType(int type) = 0;
	virtual nmb_ListOfStrings *exportFormatNames() = 0;
	virtual int exportToFile(FILE *f, const char *export_type) = 0;

  protected:
	vrpn_bool is_height_field;
};

// container class for BCGrid/BCPlane-based images
class nmb_ImageGrid : public nmb_Image{
  public:
	nmb_ImageGrid(const char *name, const char *units, short x, short y);
	nmb_ImageGrid(BCPlane *p);
	virtual ~nmb_ImageGrid();
	virtual int width() const;
	virtual int height() const;
	virtual float getValue(int i, int j) const;
	virtual void setValue(int i, int j, float val);
        virtual int validDataRange(short* o_top, short* o_left,
                                   short* o_bottom, short*o_right);
	virtual float maxValue() const;
	virtual float minValue() const;
        virtual float minAttainableValue() const;
        virtual float maxAttainableValue() const;
	virtual double boundX(nmb_ImageBounds::ImageBoundPoint ibp) const;
	virtual double boundY(nmb_ImageBounds::ImageBoundPoint ibp) const;
	virtual void setBoundX(nmb_ImageBounds::ImageBoundPoint ibp, double x);
	virtual void setBoundY(nmb_ImageBounds::ImageBoundPoint ibp, double y);
	virtual void getBounds(nmb_ImageBounds &ib)  const;
	virtual void setBounds(const nmb_ImageBounds &ib);
	virtual BCString *name();
	virtual BCString *unitsValue();
	virtual BCString *unitsX();
	virtual BCString *unitsY();

	virtual vrpn_uint8 *rawDataUnsignedByte();

        virtual int numExportFormats();
	virtual nmb_ListOfStrings *exportFormatNames();
        virtual const char *exportFormatType(int type); 
        virtual int exportToFile(FILE *f, const char *export_type);

	typedef int (*FileExportingFunction) (FILE *file, nmb_ImageGrid *im);
  protected:
	static const int     num_export_formats;
        static const char    *export_formats_list[];
	nmb_ListOfStrings formatNames;
	static const FileExportingFunction file_exporting_function[];
        static int writeTopoFile(FILE *file, nmb_ImageGrid *im);
	static int writeTextFile(FILE *file, nmb_ImageGrid *im);
        static int writePPMFile(FILE *file, nmb_ImageGrid *im);
        static int writeSPIPFile(FILE *file, nmb_ImageGrid *im);
	static int writeUNCAFile(FILE *file, nmb_ImageGrid *im);

	BCPlane *plane;
	BCGrid *grid;  // this is NULL if we are not the allocator of
			// the grid (important for destructor)
	BCString units_x;
	BCString units_y;
        short min_x_set, min_y_set, max_x_set, max_y_set;

};

class nmb_Image8bit : public nmb_Image {
  public:
    nmb_Image8bit(const char *name, const char *units, short x, short y);
    virtual ~nmb_Image8bit();
    virtual int width() const;
    virtual int height() const;
    virtual vrpn_uint8 *rawDataUnsignedByte();
    virtual float getValue(int i, int j) const;
    virtual void setValue(int i, int j, float val);
    virtual void setLine(int line, vrpn_uint8 *line_data);
    virtual void setImage(vrpn_uint8 *newdata);
    virtual int validDataRange(short* o_top, short* o_left,
                                   short* o_bottom, short*o_right);
    virtual float maxValue() const;
    virtual float minValue() const;
    virtual float maxAttainableValue() const;
    virtual float minAttainableValue() const;
    virtual double boundX(nmb_ImageBounds::ImageBoundPoint ibp) const;
    virtual double boundY(nmb_ImageBounds::ImageBoundPoint ibp) const;
    virtual void setBoundX(nmb_ImageBounds::ImageBoundPoint ibp, double x);
    virtual void setBoundY(nmb_ImageBounds::ImageBoundPoint ibp, double y);
    virtual void getBounds(nmb_ImageBounds &ib) const;
    virtual void setBounds(const nmb_ImageBounds &ib);
    virtual BCString *name();
    virtual BCString *unitsValue();
    virtual BCString *unitsX();
    virtual BCString *unitsY();
     
    virtual int numExportFormats();
    virtual nmb_ListOfStrings *exportFormatNames();
    virtual const char *exportFormatType(int type);
    virtual int exportToFile(FILE *f, const char *export_type);

    typedef int (*FileExportingFunction) (FILE *file, nmb_Image8bit *im);
  protected:
    vrpn_uint8 *data;
    short num_x, num_y;
    BCString units_x, units_y, units, my_name;
    short min_x_set, min_y_set, max_x_set, max_y_set;

        static const int     num_export_formats;
        static const char    *export_formats_list[];
        nmb_ListOfStrings formatNames;
        static const FileExportingFunction file_exporting_function[];
};

#define NMB_MAX_IMAGELIST_LENGTH 100

class nmb_ImageList {
  public:
	nmb_ImageList() : num_images(0) {};
        ~nmb_ImageList();
	nmb_ImageList(const char **file_names, int num_files);
	int addImage(nmb_Image *im);
	nmb_ListOfStrings *imageNameList() {return &imageNames;}
	nmb_Image *getImageByName(BCString name) {
		int i;
		return getImageByName(name, i);
	}
	nmb_Image *removeImageByName(BCString name);

  private:
	nmb_Image *getImageByName(BCString name, int &index);

	int num_images;
        nmb_Image *images[NMB_MAX_IMAGELIST_LENGTH];
	nmb_ListOfStrings imageNames;
};


#endif
