#ifndef NMB_IMAGE_H
#define NMB_IMAGE_H

#include "BCString.h"
#include "BCPlane.h"
#include "BCGrid.h"
#include "vrpn_Types.h"
#include "nmb_Selector.h"
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
	nmb_ImageBounds() {
		for (int i = 0; i < 4; i++){
			x[i] = 0.0; y[i] = 0.0;
		}
	}
	nmb_ImageBounds(double x0,double y0,double x1,double y1){
		x[MIN_X_MIN_Y] = x0; x[MIN_X_MAX_Y] = x0;
		y[MIN_X_MIN_Y] = y0; y[MAX_X_MIN_Y] = y0;
		x[MAX_X_MIN_Y] = x1; x[MAX_X_MAX_Y] = x1;
                y[MIN_X_MAX_Y] = y1; y[MAX_X_MAX_Y] = y1;
	}
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

	virtual int numExportFormats() = 0;
	virtual char *exportFormatType(int type) = 0;
	virtual nmb_ListOfStrings *exportFormatNames() = 0;
	virtual int exportToFile(FILE *f, const char *export_type) = 0;

  protected:
	vrpn_bool is_height_field;
};

// container class for BCGrid/BCPlane-based images
class nmb_ImageGrid : public nmb_Image{
  public:
	nmb_ImageGrid(const char *name, const char *units, short x, short y):
	    nmb_Image(),
	    units_x("nm"), units_y("nm")
        {
		BCString name_str(name), units_str(units);
		grid = new BCGrid(x, y, 0.0, 1.0, 0.0, 1.0);
		plane = grid->addNewPlane(name_str, units_str, 0);
                min_x_set = MAXSHORT; min_y_set = MAXSHORT;
                max_x_set = -MAXSHORT; max_y_set = -MAXSHORT;
		for (int i = 0; i < numExportFormats(); i++){
		    BCString name = exportFormatType(i);
		    formatNames.addEntry(name);
		}
	}
	nmb_ImageGrid(BCPlane *p):nmb_Image()
	{
            // WARNING: assumes (non-zero value <==> value was set) as
            // did BCPlane::findValidDataRange()
	    
	    plane = p;
	    grid = NULL;
	    int i,j;
            for (i = 0; i < plane->numX(); i++){
		for (j = 0; j < plane->numY(); j++){
 		    if (plane->value(i,j) != 0.0){
			min_x_set = MIN(min_x_set, i);
                        max_x_set = MAX(max_x_set, i);
                        min_y_set = MIN(min_y_set, j);
                        max_y_set = MAX(max_y_set, j);
                    }
                }
            }
	    for (i = 0; i < numExportFormats(); i++){
                BCString name = exportFormatType(i);
                formatNames.addEntry(name);
            }
	}
	virtual ~nmb_ImageGrid() {if (grid) delete grid;}
	virtual int width() const {return plane->numX();}
	virtual int height() const {return plane->numY();}
	virtual float getValue(int i, int j) const 
             {return plane->value(i,j);}
	virtual void setValue(int i, int j, float val)
        {
             plane->setValue(i,j,val);
             min_x_set = MIN(min_x_set, i);
             max_x_set = MAX(max_x_set, i);
             min_y_set = MIN(min_y_set, j);
             max_y_set = MAX(max_y_set, j);
	}
        virtual int validDataRange(short* o_top, short* o_left,
                                   short* o_bottom, short*o_right){
	     // if no valid data:
             if (min_y_set > max_y_set || min_x_set > max_x_set)
		return -1;
             // otherwise at least one valid data point:
             *o_bottom = min_y_set; *o_top = max_y_set;
             *o_left = min_x_set; *o_right = max_x_set;
	     return 0;
        }
	virtual float maxValue() const {return plane->maxValue();}
	virtual float minValue() const {return plane->minValue();}
        virtual float minAttainableValue() const {
           return plane->minAttainableValue();}
        virtual float maxAttainableValue() const {
           return plane->maxAttainableValue();}
	virtual double boundX(nmb_ImageBounds::ImageBoundPoint ibp) const
	{
		if (ibp == nmb_ImageBounds::MIN_X_MIN_Y || 
			ibp == nmb_ImageBounds::MIN_X_MAX_Y){
			return plane->minX();
		} else {
			return plane->maxX();
                }
	}
	virtual double boundY(nmb_ImageBounds::ImageBoundPoint ibp) const 
	{
		if (ibp == nmb_ImageBounds::MIN_X_MIN_Y || 
            		ibp == nmb_ImageBounds::MAX_X_MIN_Y) {
            		return plane->minY();
        	} else {
            		return plane->maxY();
		}
	}
	virtual void setBoundX(nmb_ImageBounds::ImageBoundPoint ibp, double x)
	{
            // WARNING: this might not do what you think because
            // BCGrid has a less general notion of the image extents
            if (grid == NULL) {
            	fprintf(stderr, 
                	"Warning: nmb_ImageGrid::setBoundX failed\n");
		    return;
            }
	    if (ibp == nmb_ImageBounds::MIN_X_MIN_Y ||
            	ibp == nmb_ImageBounds::MIN_X_MAX_Y) {
            	grid->setMinX(x);
            } else {
            	grid->setMaxX(x);
	    }
	}
	virtual void setBoundY(nmb_ImageBounds::ImageBoundPoint ibp, double y)
	{
            // WARNING: this might not do what you think because
            // BCGrid has a less general notion of the image extents
            if (grid == NULL) {
            	fprintf(stderr, 
                	"Warning: nmb_ImageGrid::setBoundY failed\n");
		return;
            }
            if (ibp == nmb_ImageBounds::MIN_X_MIN_Y ||
            	ibp == nmb_ImageBounds::MAX_X_MIN_Y) {
            	grid->setMinY(y);
            } else {
            	grid->setMaxY(y);
	    }
	}
	virtual void getBounds(nmb_ImageBounds &ib)  const
	{
	    ib = nmb_ImageBounds(plane->minX(), plane->minY(),
				plane->maxX(), plane->maxY());
	}
	virtual void setBounds(const nmb_ImageBounds &ib)
	{
            // WARNING: this might not do what you think because
            // BCGrid has a less general notion of the image extents
	    if (grid == NULL) {
		fprintf(stderr, 
			"Warning: nmb_ImageGrid::setBounds failed\n");
	    }
	    grid->setMinX(ib.getX(nmb_ImageBounds::MIN_X_MIN_Y));
	    grid->setMinY(ib.getY(nmb_ImageBounds::MIN_X_MIN_Y));
	    grid->setMaxX(ib.getX(nmb_ImageBounds::MAX_X_MAX_Y));
	    grid->setMaxY(ib.getY(nmb_ImageBounds::MAX_X_MAX_Y));
	}
	virtual BCString *name() {return plane->name();}
	virtual BCString *unitsValue() {return plane->units();}
	virtual BCString *unitsX() {return &units_x;}
	virtual BCString *unitsY() {return &units_y;}

        virtual int numExportFormats() {return num_export_formats;};
	virtual nmb_ListOfStrings *exportFormatNames() 
		{return &formatNames;}
        virtual char *exportFormatType(int type) 
	    {return (char *)export_formats_list[type];}
        virtual int exportToFile(FILE *f, const char *export_type);

	typedef int (*FileExportingFunction) (FILE *file, nmb_ImageGrid *im);
  private:
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
	nmb_Image *removeImageByName(BCString name) {
		int i;
		nmb_Image *im = getImageByName(name, i);
		if (im == NULL) return NULL;
		// getImageByName() succeeds ==> num_images >= 1
		images[i] = images[num_images-1];
		imageNames.deleteEntry((const char *)(*(im->name())));
		num_images--;
		return im;
	}

  private:
	nmb_Image *getImageByName(BCString name, int &index) {
	  for (int i = 0; i < num_images; i++) {
            if (*(images[i]->name()) == name){
		index = i;
                return images[i];
	    }
          }
          return NULL;
	}

	int num_images;
        nmb_Image *images[NMB_MAX_IMAGELIST_LENGTH];
	nmb_ListOfStrings imageNames;
};


#endif
