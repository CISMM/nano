/*===3rdtech===
  Copyright (c) 2000 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#ifndef NMB_IMAGE_H
#define NMB_IMAGE_H

#include <vrpn_Shared.h>
#include "BCString.h"
#include "BCPlane.h"
#include "BCGrid.h"
#include "nmb_String.h"
#include <math.h>
#include "Topo.h"
#include <assert.h>

#if !(defined(__CYGWIN__) || defined(_WIN32))
#include <values.h>	// for MAXSHORT - probably different on windows
#else
// MAXSHORT taken care of by vrpn_Shared.h above - it's in a windows header.
//#include <limits.h>
//#ifndef MAXSHORT
//#define MAXSHORT SHRT_MAX
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

typedef enum {NMB_FLOAT32, NMB_UINT8, NMB_UINT16}  nmb_PixelType;

class nmb_ImageBounds {
  public:
	nmb_ImageBounds();
	nmb_ImageBounds(double x0,double y0,double x1,double y1);
    /// the MIN/MAX distinction is based on how the pixels in the image
    /// are indexed (e.g.,the corner at the pixel with index (0,0) is
    /// referenced by MIN_X_MIN_Y; particular values are assigned so that
    /// the enum values can be used as indexes for the x,y arrays
	enum ImageBoundPoint {MIN_X_MIN_Y = 0, MIN_X_MAX_Y = 1, 
				MAX_X_MIN_Y = 2, MAX_X_MAX_Y = 3};

	void setX(ImageBoundPoint ibp, double xb) {x[ibp] = xb;}
	void setY(ImageBoundPoint ibp, double yb) {y[ibp] = yb;}
	double getX(ImageBoundPoint ibp) const { return x[ibp]; }
	double getY(ImageBoundPoint ibp) const { return y[ibp]; }
        double minX() const {return min(min(x[0], x[1]), min(x[2], x[3]));}
        double minY() const {return min(min(y[0], y[1]), min(y[2], y[3]));}
        double maxX() const {return max(max(x[0], x[1]), max(x[2], x[3]));}
        double maxY() const {return max(max(y[0], y[1]), max(y[2], y[3]));}
        double area() const {
           double a1 = (x[MAX_X_MIN_Y] - x[MIN_X_MIN_Y])*
                       (y[MIN_X_MAX_Y] - y[MIN_X_MIN_Y]) -
                       (x[MIN_X_MAX_Y] - x[MIN_X_MIN_Y])*
                       (y[MAX_X_MIN_Y] - y[MIN_X_MIN_Y]);
           double a2 = (x[MAX_X_MIN_Y] - x[MAX_X_MAX_Y])*
                       (y[MIN_X_MAX_Y] - y[MAX_X_MAX_Y]) -
                       (x[MIN_X_MAX_Y] - x[MAX_X_MAX_Y])*
                       (y[MAX_X_MIN_Y] - y[MAX_X_MAX_Y]);
           return 0.5*(fabs(a1) + fabs(a2));

        }

  private:
    double x[4], y[4];
};

class nmb_ImageList;
/**
	nmb_ImageList provides a way to unite images from different BCGrids
	into a single list. An nmb_Image base class and container class for
	BCPlane were added to allow code to work with data structures other
	than BCPlane more easily
*/
class nmb_Image {
        friend class nmb_ImageList;
  public:
	nmb_Image():is_height_field(VRPN_FALSE), num_referencing_lists(0),
                d_worldToImageMatrixSet(VRPN_FALSE),
                d_imagePosition(0.0, 0.0, 1.0, 1.0),
                d_units_scale(1.0), d_units_offset(0.0)
        {
           for (int i = 0; i < 4; i++){
              for (int j = 0; j < 4; j++) {
                 d_worldToImageMatrix[i*4+j] = (i==j?1:0);
              }
           }
        };
        nmb_Image(nmb_Image *) {};
	virtual ~nmb_Image (void);
	virtual int width() const = 0;
	virtual int height() const = 0;
	virtual float getValue(int i, int j) const = 0;
	virtual void setValue(int i, int j, float val) = 0;
        /// min and max of values actually in the image:
	virtual float maxValue() = 0;
	virtual float minValue() = 0;
        virtual int normalize() = 0;
        // min and max of values that have been set
        virtual float maxValidValue() = 0;
        virtual float minValidValue() = 0;
        virtual float maxNonZeroValue() = 0;
        virtual float minNonZeroValue() = 0;
        /// min and max representable values:
	virtual float minAttainableValue() const = 0;
        virtual float maxAttainableValue() const = 0;
	virtual int validDataRange(short* o_top, short* o_left, 
                                   short* o_bottom, short*o_right) = 0;

	float getValueInterpolated(double i, double j) const;
	float getValueInterpolatedNZ(double i, double j) const;

	/// bounds of image in whatever units the image is in 
	/// (note: this is more general than the current BCGrid interface since
	///  it allows for rotation of the image coordinate system (in which we
	///  have image positions in terms of pixel (i,j) coordinates) relative
	///  to the world coordinate system)
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

	/// convert a position in an image given as a pixel location into a
	/// position in the world coordinate system for the image
	void pixelToWorld(const double i, const double j, 
 		  double &x, double &y) const;

	/// WARNING: assumes image axes are orthogonal in the world
	void worldToPixel(const double x, const double y,
			double &i, double &j) const;

        void getWorldToImageTransform(double *matrix44);
        void setWorldToImageTransform(double *matrix44);

        double areaInWorld();

        virtual void setTopoFileInfo(TopoFile &tf) = 0;
        virtual void getTopoFileInfo(TopoFile &tf) = 0;

	virtual BCString *name() = 0;
	virtual BCString *unitsX() = 0;
	virtual BCString *unitsY() = 0;
	virtual BCString *unitsValue() = 0;
        virtual double valueOffset() {return d_units_offset;}
        virtual double valueScale() {return d_units_scale;}
	vrpn_bool isHeightField() const {return is_height_field;}
	void setHeightField(vrpn_bool flag) {is_height_field = flag;}

	/// gives address of an array of pixels in the order
        /// row0, row1, row2, ... row<height-1>
	virtual void *pixelData() = 0;

        /// gives the border widths for data returned by pixelData
        virtual int borderXMin() = 0;
        virtual int borderXMax() = 0;
        virtual int borderYMin() = 0;
        virtual int borderYMax() = 0;

        /// tells you the size of the array returned by pixelData()
        virtual int arrayLength() = 0;

        /// tells you what type of data is returned by pixelData
        virtual nmb_PixelType pixelType() = 0;

	virtual int numExportFormats() = 0;
	virtual const char *exportFormatType(int type) = 0;
	virtual nmb_ListOfStrings *exportFormatNames() = 0;
	virtual int exportToFile(FILE *f, const char *export_type) = 0;

  protected:
	vrpn_bool is_height_field;
        int num_referencing_lists;

        /// has d_worldToImageMatrix been set?
        vrpn_bool d_worldToImageMatrixSet;
        /// the transformation matrix returned by getWorldToImageTransform
        /// if its been set through setWorldToImageTransform
        double d_worldToImageMatrix[16];
        /// position of the corners of the image in the world
        nmb_ImageBounds d_imagePosition;
        /// (value in specified units) = 
        ///               (array value)*d_units_scale+d_units_offset
        double d_units_scale;
        double d_units_offset;
};

/// container class for BCGrid/BCPlane-based images
class nmb_ImageGrid : public nmb_Image{
  public:
	nmb_ImageGrid(const char *name, const char *units, short x, short y);
        // makes a wrapper object which is responsible for deallocating 
        // the grid and plane
        nmb_ImageGrid(BCGrid *g);
        // makes a wrapper object which doesn't do any deallocation of the
        // plane or grid
	nmb_ImageGrid(BCPlane *p);
        nmb_ImageGrid(nmb_Image *);
	virtual ~nmb_ImageGrid();
	virtual int width() const;
	virtual int height() const;
	virtual float getValue(int i, int j) const;
	virtual void setValue(int i, int j, float val);
        virtual int validDataRange(short* o_top, short* o_left,
                                   short* o_bottom, short*o_right);
	virtual float maxValue();
	virtual float minValue();
        virtual int normalize();
        virtual float maxValidValue();
        virtual float minValidValue();
        virtual float maxNonZeroValue();
        virtual float minNonZeroValue();
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
        virtual void setTopoFileInfo(TopoFile &tf);
        virtual void getTopoFileInfo(TopoFile &tf);
        virtual void *pixelData();
        virtual int borderXMin();
        virtual int borderXMax();
        virtual int borderYMin();
        virtual int borderYMax();
        virtual int arrayLength();
	virtual nmb_PixelType pixelType();
        virtual int numExportFormats();
	virtual nmb_ListOfStrings *exportFormatNames();
        virtual const char *exportFormatType(int type); 
        virtual int exportToFile(FILE *f, const char *export_type);

	typedef int (*FileExportingFunction) (FILE *file, nmb_ImageGrid *im);

        /// functions for reading one or more images out of a file
        /// these are in this class so we can leverage the code in BCGrid
        /// for reading topometrix files
        static int openFile(const char *filename);
        static nmb_ImageGrid *getNextImage();

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
	BCGrid *grid;  ///< this is NULL if we are not the allocator of
			///< the grid (important for destructor)
	BCString units_x;
	BCString units_y;
        short min_x_set, min_y_set, max_x_set, max_y_set;

        /// the topo file information for this particular image
        TopoFile d_topoFileDefaults;

        /// the topo file information for the last opened file
        static TopoFile s_openFileTopoHeader;
        static BCPlane *s_openFilePlane;
        static BCGrid *s_openFileGrid;
        
        vrpn_bool d_imagePositionSet;
};

/* This was originally written as a template with variable type for the pixel
   data but something about this mixture of templates and inheritance seems
   to cause the sgi CC to do something really nasty. -- AAS
*/

class nmb_ImageArray : public nmb_Image {
  public:
    nmb_ImageArray(const char *name, const char *units, short x, short y,
        nmb_PixelType pixType = NMB_FLOAT32);
    nmb_ImageArray(nmb_Image *);
    virtual ~nmb_ImageArray();
    virtual int width() const;
    virtual int height() const;

    virtual float getValue(int i, int j) const;
    virtual void setValue(int i, int j, float val);
    /// min and max of values actually in the image:
    virtual float maxValue();
    virtual float minValue();
    virtual int normalize();
    // min and max of values that have been set
    virtual float maxValidValue();
    virtual float minValidValue();
    virtual float maxNonZeroValue();
    virtual float minNonZeroValue();
    /// min and max representable values:
    virtual float minAttainableValue() const;
    virtual float maxAttainableValue() const;
    virtual int validDataRange(short* o_top, short* o_left,
                               short* o_bottom, short*o_right);

    virtual double boundX(nmb_ImageBounds::ImageBoundPoint ibp) const;
    virtual double boundY(nmb_ImageBounds::ImageBoundPoint ibp) const;
    virtual void setBoundX(nmb_ImageBounds::ImageBoundPoint ibp, double x);
    virtual void setBoundY(nmb_ImageBounds::ImageBoundPoint ibp, double y);
    virtual void getBounds(nmb_ImageBounds &ib) const;
    virtual void setBounds(const nmb_ImageBounds &ib);

    virtual void setTopoFileInfo(TopoFile &) {
      fprintf(stderr,
          "Warning: nmb_ImageArray::setTopoFileInfo not implemented\n");
    }
    virtual void getTopoFileInfo(TopoFile &) {
      fprintf(stderr,
          "Warning: nmb_ImageArray::getTopoFileInfo not implemented\n");
    }

    virtual BCString *name();
    virtual BCString *unitsX();
    virtual BCString *unitsY();
    virtual BCString *unitsValue();

    /// gives address of an array of pixels in the order
    /// row0, row1, row2, ... row<height-1>
    virtual void *pixelData();

    /// gives the border width for data returned by pixelData
    virtual int borderXMin();
    virtual int borderXMax();
    virtual int borderYMin();
    virtual int borderYMax();

    /// tells you the size of the array returned by pixelData()
    virtual int arrayLength();

    /// tells you what type of data is returned by pixelData
    virtual nmb_PixelType pixelType();

    virtual int numExportFormats();
    virtual const char *exportFormatType(int type);
    virtual nmb_ListOfStrings *exportFormatNames();
    virtual int exportToFile(FILE *f, const char *export_type);


    virtual void setLine(int line, void *line_data);
    virtual void setImage(void *newdata);

    typedef int (*FileExportingFunction) 
            (FILE *file, nmb_ImageArray *im);
  protected:
    int arrIndex(int i, int j) const
      { return (i+d_borderXMin+(j+d_borderYMin)*
                               (num_x+d_borderXMin+d_borderXMax));}
    int pixelSize()
    {
      switch(d_pixelType) {
        case NMB_FLOAT32:
          return sizeof(vrpn_float32);
        case NMB_UINT8:
          return sizeof(vrpn_uint8);
        case NMB_UINT16:
          return sizeof(vrpn_uint16);
        default:
          return 0;
      }
    }

    vrpn_float32 *fData;
    vrpn_uint8 *ucData;
    vrpn_uint16 *usData;

    void *data;
    short num_x, num_y;
    short d_borderXMin, d_borderXMax, d_borderYMin, d_borderYMax;
    BCString units_x, units_y, units, my_name;
    short min_x_set, min_y_set, max_x_set, max_y_set;

    static const int     num_export_formats;
    static const char    *export_formats_list[];
    nmb_ListOfStrings formatNames;
    static const FileExportingFunction file_exporting_function[];

    vrpn_bool d_minNonZeroValueComputed;
    float d_minNonZeroValue;
    vrpn_bool d_minValueComputed;
    float d_minValue;
    vrpn_bool d_maxValueComputed;
    float d_maxValue;
    vrpn_bool d_maxValidValueComputed;
    float d_maxValidValue;
    vrpn_bool d_minValidValueComputed;
    float d_minValidValue;

    nmb_PixelType d_pixelType;
};

#define NMB_MAX_IMAGELIST_LENGTH 100

class nmb_ImageList {
  public:
	nmb_ImageList(nmb_ListOfStrings *namelist);
        ~nmb_ImageList();
	nmb_ImageList(nmb_ListOfStrings *namelist,
                      const char **file_names, int num_files,
                      TopoFile &topoFile);
	int addImage(nmb_Image *im);
	nmb_ListOfStrings *imageNameList() {return imageNames;}
	nmb_Image *getImageByName(BCString name) {
		int i;
		return getImageByName(name, i);
	}
	nmb_Image *removeImageByName(BCString name);
        int numImages() {
            return num_images;
        }
        nmb_Image *getImage(int index) {
            return images[index];
        }

  private:
	nmb_Image *getImageByName(BCString name, int &index);

	int num_images;
        nmb_Image *images[NMB_MAX_IMAGELIST_LENGTH];
	nmb_ListOfStrings *imageNames;
};


#endif
